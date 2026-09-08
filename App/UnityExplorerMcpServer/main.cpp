#define _CRT_SECURE_NO_WARNINGS

#include "mcp/http_transport.hpp"
#include "mcp/mcp_server.hpp"
#include "mcp/stdio_transport.hpp"

#include "analysis/analysis_session.hpp"
#include "er2/unity2/init/context.hpp"
#include "er2/unity2/init/module_match.hpp"
#include "App/UnityExplorer/MetickAdapter.hpp"

#include <Windows.h>

#include <algorithm>
#include <array>
#include <atomic>
#include <chrono>
#include <csignal>
#include <cstdint>
#include <cstdlib>
#include <future>
#include <iomanip>
#include <iostream>
#include <limits>
#include <memory>
#include <mutex>
#include <random>
#include <sstream>
#include <string>
#include <thread>
#include <utility>
#include <vector>

namespace {

using unityexplorer::analysis::AnalysisSession;
using unityexplorer::analysis::AnalysisSessionOptions;
using unityexplorer::analysis::AddressRange;
using unityexplorer::analysis::BackendSnapshot;
using unityexplorer::analysis::ConnectRequest;
using unityexplorer::analysis::IMemoryBackend;
using unityexplorer::analysis::IsCanonicalUserAddress;
using unityexplorer::analysis::ModuleSnapshot;
using unityexplorer::analysis::ReadResult;
using unityexplorer::analysis::SessionExecutor;
using unityexplorer::analysis::SessionResult;
using unityexplorer::analysis::SessionStatus;
using unityexplorer::analysis::ToString;
using unityexplorer::analysis::WriteRequest;
using unityexplorer::analysis::WriteResult;
using unityexplorer::mcp::Json;
using unityexplorer::mcp::McpServer;
using unityexplorer::mcp::RequestContext;
using unityexplorer::mcp::ToolDefinition;
using unityexplorer::mcp::ToolResult;

std::atomic<bool> g_stop{false};

void SignalHandler(int) { g_stop.store(true, std::memory_order_relaxed); }

std::string LowerAscii(std::string value) {
  for (char &ch : value) {
    if (ch >= 'A' && ch <= 'Z') {
      ch = static_cast<char>(ch - 'A' + 'a');
    }
  }
  return value;
}

bool ParseUnsigned(const Json &value, std::uint64_t &out) {
  if (value.is_number_unsigned()) {
    out = value.get<std::uint64_t>();
    return true;
  }
  if (value.is_number_integer()) {
    const auto signedValue = value.get<std::int64_t>();
    if (signedValue < 0) {
      return false;
    }
    out = static_cast<std::uint64_t>(signedValue);
    return true;
  }
  if (!value.is_string()) {
    return false;
  }

  std::string text = value.get<std::string>();
  if (text.empty()) {
    return false;
  }
  int base = 10;
  std::size_t offset = 0;
  if (text.size() > 2 && text[0] == '0' &&
      (text[1] == 'x' || text[1] == 'X')) {
    base = 16;
    offset = 2;
  }
  if (offset == text.size()) {
    return false;
  }
  std::uint64_t parsed = 0;
  for (; offset < text.size(); ++offset) {
    const char ch = text[offset];
    unsigned digit = 0;
    if (ch >= '0' && ch <= '9') {
      digit = static_cast<unsigned>(ch - '0');
    } else if (base == 16 && ch >= 'a' && ch <= 'f') {
      digit = static_cast<unsigned>(ch - 'a' + 10);
    } else if (base == 16 && ch >= 'A' && ch <= 'F') {
      digit = static_cast<unsigned>(ch - 'A' + 10);
    } else {
      return false;
    }
    if (digit >= static_cast<unsigned>(base) ||
        parsed > (std::numeric_limits<std::uint64_t>::max() - digit) /
                     static_cast<std::uint64_t>(base)) {
      return false;
    }
    parsed = parsed * static_cast<std::uint64_t>(base) + digit;
  }
  out = parsed;
  return true;
}

bool ParseByteString(const Json &value, std::vector<std::uint8_t> &out) {
  out.clear();
  if (value.is_array()) {
    out.reserve(value.size());
    for (const Json &item : value) {
      std::uint64_t byte = 0;
      if (!ParseUnsigned(item, byte) || byte > 0xFFu) {
        return false;
      }
      out.push_back(static_cast<std::uint8_t>(byte));
    }
    return !out.empty();
  }
  if (!value.is_string()) {
    return false;
  }
  const std::string text = value.get<std::string>();
  if (text.empty() || (text.size() & 1u) != 0) {
    return false;
  }
  out.reserve(text.size() / 2);
  auto hexDigit = [](char ch, unsigned &digit) {
    if (ch >= '0' && ch <= '9') {
      digit = static_cast<unsigned>(ch - '0');
      return true;
    }
    if (ch >= 'a' && ch <= 'f') {
      digit = static_cast<unsigned>(ch - 'a' + 10);
      return true;
    }
    if (ch >= 'A' && ch <= 'F') {
      digit = static_cast<unsigned>(ch - 'A' + 10);
      return true;
    }
    return false;
  };
  for (std::size_t i = 0; i < text.size(); i += 2) {
    unsigned high = 0;
    unsigned low = 0;
    if (!hexDigit(text[i], high) || !hexDigit(text[i + 1], low)) {
      return false;
    }
    out.push_back(static_cast<std::uint8_t>((high << 4u) | low));
  }
  return !out.empty();
}

std::string HexBytes(const std::vector<std::uint8_t> &bytes) {
  std::ostringstream stream;
  stream << std::hex << std::setfill('0');
  for (const std::uint8_t byte : bytes) {
    stream << std::setw(2) << static_cast<unsigned>(byte);
  }
  return stream.str();
}

std::string HexAddress(std::uintptr_t address) {
  std::ostringstream stream;
  stream << "0x" << std::hex << std::uppercase << address;
  return stream.str();
}

// Little-endian interpretation of an 8-byte buffer, as returned by
// unity_memory_read.  Callers that just want a pointer value no longer have
// to reverse the hex string themselves.
std::uint64_t ReadLittleEndian64(const std::vector<std::uint8_t> &bytes) {
  std::uint64_t value = 0;
  const std::size_t count = std::min<std::size_t>(bytes.size(), 8u);
  for (std::size_t i = 0; i < count; ++i) {
    value |= static_cast<std::uint64_t>(bytes[i]) << (8u * i);
  }
  return value;
}

bool IsUserPointer(std::uint64_t value) {
  return value >= 0x10000ull && value < 0x0000800000000000ull;
}

// Best-effort UTF-16LE to UTF-8 conversion.  Returns true when the buffer
// decodes cleanly (even if only partially); invalid pairs become U+FFFD.
bool Utf16LeBytesToUtf8(const std::vector<std::uint8_t> &bytes,
                        std::string &out) {
  out.clear();
  if (bytes.size() < 2u) {
    return false;
  }
  const std::size_t charCount = bytes.size() / 2u;
  std::vector<wchar_t> wide(charCount);
  for (std::size_t i = 0; i < charCount; ++i) {
    wide[i] = static_cast<wchar_t>(
        static_cast<std::uint16_t>(bytes[2u * i]) |
        (static_cast<std::uint16_t>(bytes[2u * i + 1u]) << 8u));
  }
  const int required = WideCharToMultiByte(CP_UTF8, 0, wide.data(),
                                           static_cast<int>(wide.size()),
                                           nullptr, 0, nullptr, nullptr);
  if (required <= 0) {
    return false;
  }
  out.resize(static_cast<std::size_t>(required));
  const int written = WideCharToMultiByte(
      CP_UTF8, 0, wide.data(), static_cast<int>(wide.size()), out.data(),
      required, nullptr, nullptr);
  return written == required;
}

struct RuntimeModuleSelection {
  er2::UnityModuleCandidate selected;
  std::vector<er2::UnityModuleCandidate> candidates;
};

template <typename Predicate>
RuntimeModuleSelection SelectRuntimeModule(
    const std::vector<er2::UnityModuleCandidate> &modules,
    const std::string &requested, Predicate predicate) {
  RuntimeModuleSelection result;
  if (!requested.empty()) {
    const std::string wanted = LowerAscii(requested);
    // Explicit module names are exact (case-insensitive) and are allowed to
    // select a deliberately non-standard runtime image.  This is useful for
    // private Unity builds whose basename does not follow the usual prefix.
    for (const er2::UnityModuleCandidate &module : modules) {
      if (LowerAscii(module.name) == wanted) {
        result.candidates.push_back(module);
      }
    }
    if (result.candidates.size() == 1) {
      result.selected = result.candidates.front();
    }
    return result;
  }

  for (const er2::UnityModuleCandidate &module : modules) {
    if (predicate(module.name)) {
      result.candidates.push_back(module);
    }
  }

  // An implicit choice is safe only when the process exposes one matching
  // runtime image.  Keep the old er2 matcher untouched for other callers;
  // the MCP connection path must make ambiguity explicit to the client.
  if (result.candidates.size() == 1) {
    result.selected = result.candidates.front();
  }
  return result;
}

std::string DescribeRuntimeCandidates(
    const char *role,
    const std::vector<er2::UnityModuleCandidate> &candidates) {
  std::ostringstream stream;
  stream << role << " candidates:";
  for (const er2::UnityModuleCandidate &candidate : candidates) {
    stream << ' ' << candidate.name << '@' << HexAddress(candidate.base)
           << ":0x" << std::hex << std::uppercase << candidate.size;
  }
  return stream.str();
}

Json ModuleJson(const ModuleSnapshot &module) {
  return Json{{"name", module.name},
              {"fullName", module.fullName},
              {"baseAddress", HexAddress(module.baseAddress)},
              {"imageSize", module.imageSize},
              {"isWow64", module.isWow64}};
}

Json AddressRangeJson(const AddressRange &range) {
  return Json{{"base", HexAddress(range.base)},
              {"length", HexAddress(range.length)}};
}

Json StatusJson(const SessionStatus &status) {
  Json modules = Json::array();
  for (const ModuleSnapshot &module : status.modules) {
    modules.push_back(ModuleJson(module));
  }
  Json writeRanges = Json::array();
  for (const AddressRange &range : status.writeRanges) {
    writeRanges.push_back(AddressRangeJson(range));
  }
  return Json{{"state", ToString(status.state)},
              {"generation", status.generation},
              {"pid", status.pid},
              {"targetProcess", status.targetProcess},
              {"runtime", status.runtime},
              {"modules", std::move(modules)},
              {"lastError", status.lastError},
              {"writeEnabled", status.writeEnabled},
              {"writePolicy", status.writePolicy},
              {"writeRanges", std::move(writeRanges)},
              {"pendingJobs", status.pendingJobs},
              {"activeJob", status.activeJob}};
}

ToolResult SessionFailure(const SessionResult &result) {
  return ToolResult::Failure(
      result.code.empty() ? "session_error" : result.code,
      result.message.empty() ? "session operation failed" : result.message,
      Json{{"generation", result.generation}});
}

ToolResult ReadFailure(const ReadResult &result) {
  return ToolResult::Failure(
      result.code.empty() ? "read_failed" : result.code,
      result.message.empty() ? "memory read failed" : result.message,
      Json{{"generation", result.generation}, {"requested", result.requested}});
}

ToolResult WriteFailure(const WriteResult &result) {
  Json data{{"generation", result.generation}};
  if (!result.before.empty()) {
    data["before"] = HexBytes(result.before);
  }
  if (!result.readback.empty()) {
    data["readback"] = HexBytes(result.readback);
  }
  return ToolResult::Failure(
      result.code.empty() ? "write_failed" : result.code,
      result.message.empty() ? "memory write failed" : result.message,
      std::move(data));
}

// The adapter is deliberately hidden behind IMemoryBackend.  This keeps all
// VMMDLL calls on AnalysisSession's executor and makes shutdown ownership
// explicit: dropping the adapter closes its scatter handle and VMM exactly
// once through its RAII destructor.
class DmaMemoryBackend final : public IMemoryBackend {
public:
  const char *LastErrorCode() const noexcept override {
    return lastErrorCode_ != nullptr ? lastErrorCode_ : "";
  }

  bool Connect(const ConnectRequest &request, BackendSnapshot &snapshot,
               std::string &error) override {
    lastErrorCode_ = "dma_init_failed";
    // AnalysisSession normally disconnects before a replacement connect.  Do
    // it here as well so direct backend use cannot retain a stale global er2
    // context or process-wide MemProcFS handle.
    Disconnect();
    adapter_ = std::make_shared<MetickAdapter>();
    if (!adapter_->Initialize(request.targetProcess)) {
      lastErrorCode_ = adapter_->GetLastError().find("not found") !=
                               std::string::npos
                           ? "target_not_found"
                           : "dma_init_failed";
      error = adapter_->GetLastError();
      Disconnect();
      return false;
    }

    if (!adapter_->FixCr3(request.timeoutMs)) {
      lastErrorCode_ = "cr3_failed";
      error = "FixCr3 failed; target DTB could not be validated";
      Disconnect();
      return false;
    }

    const auto dmaModules = adapter_->EnumerateModules();
    std::vector<er2::UnityModuleCandidate> candidates;
    candidates.reserve(dmaModules.size());
    for (const auto &module : dmaModules) {
      candidates.push_back(er2::UnityModuleCandidate{
          module.name, module.baseAddress, module.imageSize});
    }
    const RuntimeModuleSelection unityPlayerSelection = SelectRuntimeModule(
        candidates, request.unityPlayerName,
        [](const std::string &name) { return er2::IsUnityPlayerModuleName(name); });
    const RuntimeModuleSelection gameAssemblySelection = SelectRuntimeModule(
        candidates, request.gameAssemblyName,
        [](const std::string &name) { return er2::IsGameAssemblyModuleName(name); });

    if (unityPlayerSelection.candidates.empty()) {
      lastErrorCode_ = "module_not_found";
      error = "UnityPlayer*.dll was not found in the target process";
      Disconnect();
      return false;
    }

    if (unityPlayerSelection.selected.base == 0) {
      lastErrorCode_ = "module_ambiguous";
      error = DescribeRuntimeCandidates("UnityPlayer",
                                       unityPlayerSelection.candidates);
      Disconnect();
      return false;
    }

    // GameAssembly is optional for Mono, but an explicit name must resolve;
    // an implicit multi-image IL2CPP process is rejected rather than choosing
    // a potentially incompatible RVA image.
    if (!request.gameAssemblyName.empty() &&
        gameAssemblySelection.candidates.empty()) {
      lastErrorCode_ = "module_not_found";
      error = "requested GameAssembly module was not found: " +
              request.gameAssemblyName;
      Disconnect();
      return false;
    }
    if (gameAssemblySelection.candidates.size() > 1 &&
        gameAssemblySelection.selected.base == 0) {
      lastErrorCode_ = "module_ambiguous";
      error = DescribeRuntimeCandidates("GameAssembly",
                                       gameAssemblySelection.candidates);
      Disconnect();
      return false;
    }

    const er2::UnityModuleCandidate unityPlayer =
        unityPlayerSelection.selected;
    const er2::UnityModuleCandidate gameAssembly =
        gameAssemblySelection.selected;
    if (unityPlayer.base == 0) {
      lastErrorCode_ = "module_not_found";
      error = "requested UnityPlayer module was not found";
      Disconnect();
      return false;
    }
    if (!request.gameAssemblyName.empty() && gameAssembly.base == 0) {
      lastErrorCode_ = "module_not_found";
      error = "requested GameAssembly module was not found";
      Disconnect();
      return false;
    }

    if (!ValidatePeHeader(unityPlayer, error) ||
        (gameAssembly.base != 0 && !ValidatePeHeader(gameAssembly, error))) {
      Disconnect();
      return false;
    }

    snapshot.pid = adapter_->GetPID();
    snapshot.targetProcess = request.targetProcess;
    snapshot.runtime = gameAssembly.base != 0 ? "il2cpp" : "mono";
    snapshot.modules.reserve(dmaModules.size());
    for (const auto &module : dmaModules) {
      snapshot.modules.push_back(ModuleSnapshot{module.name,
                                                module.fullName,
                                                module.baseAddress,
                                                module.imageSize,
                                                module.isWow64});
    }
    if (snapshot.pid == 0) {
      error = "adapter returned an invalid target PID";
      Disconnect();
      return false;
    }

    // er2's helpers read the process through its global context.  Bind the
    // already validated DMA accessor only after all connection checks pass;
    // this prevents a partially initialized context from being observed by a
    // subsequent request.  The shared_ptr keeps the accessor alive until
    // Disconnect() clears the context first and then releases the adapter.
    try {
      er2::ResetContext();
      er2::g_ctx.pid = snapshot.pid;
      er2::g_ctx.runtime =
          gameAssembly.base != 0 ? er2::ManagedBackend::Il2Cpp
                                 : er2::ManagedBackend::Mono;
      er2::g_ctx.memory = adapter_;
      er2::g_ctx.unityPlayer =
          er2::ModuleInfo{unityPlayer.base, unityPlayer.size};
      er2::g_ctx.unityPlayerRange.base = unityPlayer.base;
      er2::g_ctx.unityPlayerRange.size = unityPlayer.size;
      er2::g_ctx.gameAssembly =
          er2::ModuleInfo{gameAssembly.base, gameAssembly.size};
    } catch (const std::exception &exception) {
      error = std::string("failed to bind er2 context: ") + exception.what();
      Disconnect();
      return false;
    } catch (...) {
      error = "failed to bind er2 context: unknown exception";
      Disconnect();
      return false;
    }
    lastErrorCode_ = nullptr;
    return true;
  }

  void Disconnect() noexcept override {
    // ResetContext drops its shared_ptr view of adapter_ while the owning
    // member is still alive.  Releasing adapter_ first would leave er2::Mem()
    // with a dangling accessor during context cleanup/reconnect.
    try {
      er2::ResetContext();
    } catch (...) {
      // Disconnect is a noexcept cleanup path.  The adapter reset below still
      // closes the process-wide MemProcFS session even if context reset fails.
    }
    adapter_.reset();
  }

  bool IsConnected() const noexcept override {
    return adapter_ != nullptr && adapter_->IsInitialized() &&
           adapter_->GetVmmHandle() != nullptr && adapter_->GetPID() != 0;
  }

  bool Read(std::uintptr_t address, void *buffer, std::size_t size,
            bool noCache, std::string &error) override {
    if (!IsConnected()) {
      error = "DMA backend is not connected";
      return false;
    }
    adapter_->SetForceNoCache(noCache);
    if (!adapter_->Read(address, buffer, size)) {
      error = "VMMDLL memory read failed";
      return false;
    }
    return true;
  }

  bool Write(std::uintptr_t address, const void *buffer, std::size_t size,
             std::string &error) override {
    if (!IsConnected()) {
      error = "DMA backend is not connected";
      return false;
    }
    if (!adapter_->Write(address, buffer, size)) {
      error = "VMMDLL memory write failed";
      return false;
    }
    return true;
  }

  std::vector<ModuleSnapshot> Modules() const override {
    std::vector<ModuleSnapshot> result;
    if (!IsConnected()) {
      return result;
    }
    const auto modules = adapter_->EnumerateModules();
    result.reserve(modules.size());
    for (const auto &module : modules) {
      result.push_back(ModuleSnapshot{module.name,
                                      module.fullName,
                                      module.baseAddress,
                                      module.imageSize,
                                      module.isWow64});
    }
    return result;
  }

private:
  bool ValidatePeHeader(const er2::UnityModuleCandidate &module,
                        std::string &error) const {
    if (module.base == 0 ||
        module.size < static_cast<std::uint32_t>(sizeof(IMAGE_DOS_HEADER))) {
      error = "invalid PE image bounds for module " + module.name;
      return false;
    }
    IMAGE_DOS_HEADER dos{};
    if (!adapter_->Read(module.base, &dos, sizeof(dos))) {
      error = "failed to read DOS header for module " + module.name;
      return false;
    }
    if (dos.e_magic != IMAGE_DOS_SIGNATURE || dos.e_lfanew < 0) {
      error = "invalid PE DOS header for module " + module.name;
      return false;
    }
    const std::uint64_t ntOffset = static_cast<std::uint64_t>(dos.e_lfanew);
    if (ntOffset > static_cast<std::uint64_t>(module.size) -
                       sizeof(std::uint32_t) ||
        ntOffset > std::numeric_limits<std::uintptr_t>::max() - module.base) {
      error = "invalid PE DOS header for module " + module.name;
      return false;
    }
    std::uint32_t signature = 0;
    const std::uintptr_t ntAddress =
        module.base + static_cast<std::uintptr_t>(ntOffset);
    if (!adapter_->Read(ntAddress, &signature, sizeof(signature)) ||
        signature != IMAGE_NT_SIGNATURE) {
      error = "invalid PE NT signature for module " + module.name;
      return false;
    }
    return true;
  }

  std::shared_ptr<MetickAdapter> adapter_;
  const char *lastErrorCode_ = nullptr;
};

class ServiceState final {
public:
  explicit ServiceState(bool enableWrites,
                        std::vector<AddressRange> writeRanges)
      : executor_(64), backend_(std::make_shared<DmaMemoryBackend>()) {
    AnalysisSessionOptions options;
    options.enableWrites = enableWrites;
    options.requireExpectedBefore = true;
    options.requireReadback = true;
    options.writeRanges = std::move(writeRanges);
    options.executor = &executor_;
    session_ = std::make_unique<AnalysisSession>(backend_, options);
  }

  ~ServiceState() { Shutdown(); }

  ToolResult Status() const {
    std::lock_guard<std::mutex> lock(stateMutex_);
    if (!session_) {
      return ToolResult::Success(
          Json{{"session", Json{{"state", "idle"}}}, {"service", "stopped"}});
    }
    return ToolResult::Success(
        Json{{"session", StatusJson(session_->Status())},
             {"service", stopping_.load(std::memory_order_relaxed)
                              ? "stopping"
                              : "running"}});
  }

  ToolResult Connect(const Json &arguments) {
    ConnectRequest request;
    request.targetProcess = arguments.value("targetProcess", std::string{});
    request.mode = arguments.value("mode", std::string{"dma"});
    request.unityPlayerName =
        arguments.value("unityPlayerName", std::string{});
    request.gameAssemblyName =
        arguments.value("gameAssemblyName", std::string{});
    request.replace = arguments.value("replace", false);
    std::uint64_t timeout = 0;
    if (arguments.contains("timeoutMs") &&
        (!ParseUnsigned(arguments.at("timeoutMs"), timeout) || timeout > 300000)) {
      return ToolResult::Failure("invalid_argument",
                                 "timeoutMs must be an integer in [0, 300000]");
    }
    if (timeout != 0) {
      request.timeoutMs = static_cast<std::uint32_t>(timeout);
    }
    return Submit([this, request] { return ConnectOnExecutor(request); });
  }

  ToolResult Disconnect(const Json &arguments) {
    std::optional<std::uint64_t> expected;
    if (arguments.contains("expectedGeneration")) {
      std::uint64_t generation = 0;
      if (!ParseUnsigned(arguments.at("expectedGeneration"), generation)) {
        return ToolResult::Failure("invalid_argument",
                                   "expectedGeneration must be an integer");
      }
      expected = generation;
    }
    return Submit([this, expected] {
      const SessionResult result = session_->Disconnect(expected);
      return result.ok
                 ? ToolResult::Success(Json{{"state", "idle"},
                                            {"generation", result.generation}})
                 : SessionFailure(result);
    });
  }

  ToolResult Read(const Json &arguments) {
    std::uint64_t generation = 0;
    std::uint64_t address = 0;
    std::uint64_t length = 0;
    if (!ParseUnsigned(arguments.at("expectedGeneration"), generation) ||
        !ParseUnsigned(arguments.at("address"), address) ||
        !ParseUnsigned(arguments.at("length"), length) ||
        length == 0 || length > 64u * 1024u ||
        address > std::numeric_limits<std::uintptr_t>::max()) {
      return ToolResult::Failure("invalid_argument",
                                 "expectedGeneration, address or length is invalid");
    }
    const bool noCache = arguments.value("nocache", false);
    return Submit([this, generation, address, length, noCache] {
      const ReadResult result = session_->Read(
          generation, static_cast<std::uintptr_t>(address),
          static_cast<std::size_t>(length), noCache);
      if (!result.ok) {
        return ReadFailure(result);
      }
      return ToolResult::Success(
          Json{{"generation", result.generation},
               {"address", HexAddress(static_cast<std::uintptr_t>(address))},
               {"requested", result.requested},
               {"actual", result.actual},
               {"encoding", "hex"},
               {"data", HexBytes(result.bytes)}});
    });
  }

  ToolResult Modules(const Json &arguments) {
    std::uint64_t generation = 0;
    if (!ParseUnsigned(arguments.at("expectedGeneration"), generation)) {
      return ToolResult::Failure("invalid_argument",
                                 "expectedGeneration must be an integer");
    }
    return Submit([this, generation] {
      const auto result = session_->Modules(generation);
      if (!result.first.ok) {
        return SessionFailure(result.first);
      }
      Json modules = Json::array();
      for (const ModuleSnapshot &module : result.second) {
        modules.push_back(ModuleJson(module));
      }
      return ToolResult::Success(
          Json{{"generation", generation}, {"modules", std::move(modules)}});
    });
  }

  // Read 8 bytes at address and interpret them as a little-endian pointer.
  // The returned value is already byte-swapped, so AI callers never have to
  // reverse hex strings manually.
  ToolResult ReadPtr(const Json &arguments) {
    std::uint64_t generation = 0;
    std::uint64_t address = 0;
    if (!ParseUnsigned(arguments.at("expectedGeneration"), generation) ||
        !ParseUnsigned(arguments.at("address"), address) ||
        address > std::numeric_limits<std::uintptr_t>::max()) {
      return ToolResult::Failure("invalid_argument",
                                 "expectedGeneration or address is invalid");
    }
    const bool noCache = arguments.value("nocache", false);
    return Submit([this, generation, address, noCache] {
      const ReadResult result = session_->Read(
          generation, static_cast<std::uintptr_t>(address), 8u, noCache);
      if (!result.ok) {
        return ReadFailure(result);
      }
      const std::uint64_t value = ReadLittleEndian64(result.bytes);
      return ToolResult::Success(
          Json{{"generation", result.generation},
               {"address", HexAddress(static_cast<std::uintptr_t>(address))},
               {"raw", HexBytes(result.bytes)},
               {"valueHex", HexAddress(static_cast<std::uintptr_t>(value))},
               {"valueDec", value},
               {"isCanonical",
                IsCanonicalUserAddress(static_cast<std::uintptr_t>(value))},
               {"isUserPointer", IsUserPointer(value)}});
    });
  }

  // Read up to maxBytes at address and decode a NUL-terminated C string.
  // encoding: "ascii" (default, byte string until NUL) or "utf16"
  // (UTF-16LE until double-NUL, converted to UTF-8).
  ToolResult ReadString(const Json &arguments) {
    std::uint64_t generation = 0;
    std::uint64_t address = 0;
    std::uint64_t maxBytes = 256;
    if (!ParseUnsigned(arguments.at("expectedGeneration"), generation) ||
        !ParseUnsigned(arguments.at("address"), address) ||
        address > std::numeric_limits<std::uintptr_t>::max()) {
      return ToolResult::Failure("invalid_argument",
                                 "expectedGeneration or address is invalid");
    }
    if (arguments.contains("maxBytes")) {
      if (!ParseUnsigned(arguments.at("maxBytes"), maxBytes) ||
          maxBytes == 0 || maxBytes > 4096u) {
        return ToolResult::Failure("invalid_argument",
                                   "maxBytes must be in [1, 4096]");
      }
    }
    const std::string encoding = arguments.value("encoding", std::string{"ascii"});
    const bool utf16 = encoding == "utf16";
    const bool noCache = arguments.value("nocache", false);
    return Submit([this, generation, address, maxBytes, utf16, noCache] {
      const ReadResult result = session_->Read(
          generation, static_cast<std::uintptr_t>(address),
          static_cast<std::size_t>(maxBytes), noCache);
      if (!result.ok) {
        return ReadFailure(result);
      }
      std::size_t terminator = result.bytes.size();
      if (utf16) {
        for (std::size_t i = 0; i + 1u < result.bytes.size(); i += 2u) {
          if (result.bytes[i] == 0 && result.bytes[i + 1u] == 0) {
            terminator = i;
            break;
          }
        }
        std::vector<std::uint8_t> wide(result.bytes.begin(),
                                       result.bytes.begin() +
                                           static_cast<std::ptrdiff_t>(terminator));
        if ((terminator & 1u) != 0) {
          wide.pop_back();
        }
        std::string text;
        if (!Utf16LeBytesToUtf8(wide, text)) {
          return ToolResult::Failure("decode_failed",
                                     "utf16 string decode failed",
                                     Json{{"generation", result.generation},
                                          {"address", HexAddress(static_cast<std::uintptr_t>(address))}});
        }
        return ToolResult::Success(
            Json{{"generation", result.generation},
                 {"address", HexAddress(static_cast<std::uintptr_t>(address))},
                 {"encoding", "utf16"},
                 {"actual", result.actual},
                 {"text", std::move(text)},
                 {"truncated", terminator >= result.bytes.size()}});
      }
      for (std::size_t i = 0; i < result.bytes.size(); ++i) {
        if (result.bytes[i] == 0) {
          terminator = i;
          break;
        }
      }
      std::string text(reinterpret_cast<const char *>(result.bytes.data()),
                       terminator);
      return ToolResult::Success(
          Json{{"generation", result.generation},
               {"address", HexAddress(static_cast<std::uintptr_t>(address))},
               {"encoding", "ascii"},
               {"actual", result.actual},
               {"text", std::move(text)},
               {"truncated", terminator >= result.bytes.size()}});
    });
  }

  // Dereference a pointer chain in one executor job: start at base, then for
  // each offset in offsets[], read an 8-byte pointer at
  // current + offset and follow it.  Returns every intermediate hop so the
  // caller can see exactly which step failed.
  ToolResult ResolvePointerChain(const Json &arguments) {
    std::uint64_t generation = 0;
    std::uint64_t base = 0;
    if (!ParseUnsigned(arguments.at("expectedGeneration"), generation) ||
        !ParseUnsigned(arguments.at("base"), base) ||
        base > std::numeric_limits<std::uintptr_t>::max()) {
      return ToolResult::Failure("invalid_argument",
                                 "expectedGeneration or base is invalid");
    }
    if (!arguments.at("offsets").is_array()) {
      return ToolResult::Failure("invalid_argument",
                                 "offsets must be an array of integers");
    }
    std::vector<std::uint64_t> offsets;
    for (const Json &item : arguments.at("offsets")) {
      std::uint64_t offset = 0;
      if (!ParseUnsigned(item, offset)) {
        return ToolResult::Failure("invalid_argument",
                                   "offsets must be non-negative integers");
      }
      offsets.push_back(offset);
    }
    if (offsets.empty()) {
      return ToolResult::Failure("invalid_argument",
                                 "offsets must not be empty");
    }
    const bool noCache = arguments.value("nocache", false);
    return Submit([this, generation, base, offsets = std::move(offsets),
                   noCache]() mutable {
      std::uint64_t current = base;
      Json steps = Json::array();
      for (std::size_t index = 0; index < offsets.size(); ++index) {
        const std::uint64_t hopAddress = current + offsets[index];
        if (hopAddress > std::numeric_limits<std::uintptr_t>::max()) {
          steps.push_back(Json{{"index", index},
                               {"offset", offsets[index]},
                               {"ok", false},
                               {"error", "address overflow"}});
          return ToolResult::Failure(
              "invalid_argument", "pointer chain address overflow",
              Json{{"generation", generation},
                   {"base", HexAddress(static_cast<std::uintptr_t>(base))},
                   {"steps", std::move(steps)}});
        }
        const ReadResult result = session_->Read(
            generation, static_cast<std::uintptr_t>(hopAddress), 8u, noCache);
        if (!result.ok) {
          steps.push_back(Json{{"index", index},
                               {"offset", offsets[index]},
                               {"ok", false},
                               {"address",
                                HexAddress(static_cast<std::uintptr_t>(hopAddress))},
                               {"error", result.message}});
          return ToolResult::Failure(
              result.code.empty() ? "read_failed" : result.code,
              result.message.empty() ? "pointer chain read failed"
                                     : result.message,
              Json{{"generation", result.generation},
                   {"base", HexAddress(static_cast<std::uintptr_t>(base))},
                   {"steps", std::move(steps)}});
        }
        const std::uint64_t value = ReadLittleEndian64(result.bytes);
        steps.push_back(Json{{"index", index},
                             {"offset", offsets[index]},
                             {"ok", true},
                             {"address",
                              HexAddress(static_cast<std::uintptr_t>(hopAddress))},
                             {"valueHex",
                              HexAddress(static_cast<std::uintptr_t>(value))},
                             {"valueDec", value},
                             {"isCanonical",
                              IsCanonicalUserAddress(static_cast<std::uintptr_t>(value))},
                             {"isUserPointer", IsUserPointer(value)}});
        if (value == 0) {
          return ToolResult::Failure(
              "null_pointer", "pointer chain hit a null pointer",
              Json{{"generation", result.generation},
                   {"base", HexAddress(static_cast<std::uintptr_t>(base))},
                   {"steps", std::move(steps)}});
        }
        current = value;
      }
      return ToolResult::Success(
          Json{{"generation", generation},
               {"base", HexAddress(static_cast<std::uintptr_t>(base))},
               {"steps", std::move(steps)},
               {"finalValueHex", HexAddress(static_cast<std::uintptr_t>(current))},
               {"finalValueDec", current}});
    });
  }

  ToolResult Write(const Json &arguments) {
    std::uint64_t generation = 0;
    std::uint64_t address = 0;
    std::vector<std::uint8_t> data;
    std::vector<std::uint8_t> expectedBefore;
    std::vector<std::uint8_t> mask;
    if (!ParseUnsigned(arguments.at("expectedGeneration"), generation) ||
        !ParseUnsigned(arguments.at("address"), address) ||
        address > std::numeric_limits<std::uintptr_t>::max() ||
        !ParseByteString(arguments.at("data"), data) ||
        !ParseByteString(arguments.at("expectedBefore"), expectedBefore)) {
      return ToolResult::Failure("invalid_argument",
                                 "expectedGeneration, address, data or expectedBefore is invalid");
    }
    if (arguments.contains("mask") &&
        !ParseByteString(arguments.at("mask"), mask)) {
      return ToolResult::Failure("invalid_argument",
                                 "mask must be a non-empty hex byte string");
    }
    WriteRequest request;
    request.expectedGeneration = generation;
    request.address = static_cast<std::uintptr_t>(address);
    request.data = std::move(data);
    request.expectedBefore = std::move(expectedBefore);
    request.mask = std::move(mask);
    request.verifyReadback = arguments.value("verifyReadback", true);
    return Submit([this, request = std::move(request)]() mutable {
      const WriteResult result = session_->Write(request);
      if (!result.ok) {
        return WriteFailure(result);
      }
      return ToolResult::Success(
          Json{{"generation", result.generation},
               {"before", HexBytes(result.before)},
               {"readback", HexBytes(result.readback)},
               {"readbackMatches", result.readbackMatches}});
    });
  }

  void Shutdown() noexcept {
    bool expected = false;
    if (!stopping_.compare_exchange_strong(expected, true,
                                           std::memory_order_acq_rel)) {
      return;
    }
    // Main stops accepting transport requests before entering this method.
    // Stop() joins the active DMA job before the adapter/VMM is released.
    executor_.Stop();

    // Do not call AnalysisSession::Disconnect while holding stateMutex_.
    // Disconnect() takes the session command lock and then stateMutex_ again;
    // moving the owners out first keeps shutdown lock-order safe and also
    // makes concurrent status calls observe an empty/stopped service.
    std::unique_ptr<AnalysisSession> session;
    std::shared_ptr<DmaMemoryBackend> backend;
    {
      std::lock_guard<std::mutex> lock(stateMutex_);
      session = std::move(session_);
      backend = std::move(backend_);
    }
    if (session) {
      (void)session->Disconnect();
      session.reset();
    }
    backend.reset();
  }

private:
  ToolResult ConnectOnExecutor(const ConnectRequest &request) {
    if (stopping_.load(std::memory_order_relaxed)) {
      return ToolResult::Failure("service_stopped", "service is stopping");
    }
    const SessionResult result = session_->Connect(request);
    if (!result.ok) {
      return SessionFailure(result);
    }
    return ToolResult::Success(
        Json{{"state", "ready"}, {"generation", result.generation},
             {"session", StatusJson(session_->Status())}});
  }

  template <typename Function> ToolResult Submit(Function &&function) {
    if (stopping_.load(std::memory_order_acquire)) {
      return ToolResult::Failure("service_stopped", "service is stopping");
    }
    try {
      auto future = executor_.Submit(std::forward<Function>(function));
      return future.get();
    } catch (const unityexplorer::analysis::ExecutorBusy &) {
      return ToolResult::Failure("busy", "session executor queue is full");
    } catch (const unityexplorer::analysis::ExecutorStopped &) {
      return ToolResult::Failure("service_stopped", "session executor is stopped");
    } catch (const std::exception &exception) {
      return ToolResult::Failure("internal_error", exception.what());
    }
  }

  SessionExecutor executor_;
  std::shared_ptr<DmaMemoryBackend> backend_;
  std::unique_ptr<AnalysisSession> session_;
  std::atomic<bool> stopping_{false};
  mutable std::mutex stateMutex_;
};

ToolDefinition StatusTool(ServiceState &state) {
  return ToolDefinition{
      "unity_session_status", "Return persistent service/session status",
      Json{{"type", "object"},
           {"properties", Json::object()},
           {"additionalProperties", false}},
      Json{{"type", "object"}}, {"read"},
      [&state](const Json &, const RequestContext &) { return state.Status(); }};
}

ToolDefinition ConnectTool(ServiceState &state) {
  return ToolDefinition{
      "unity_session_connect", "Connect or replace the DMA analysis session",
      Json{{"type", "object"},
           {"properties",
            Json{{"targetProcess", Json{{"type", "string"}}},
                 {"mode", Json{{"type", "string"},
                                {"enum", Json::array({"dma"})}}},
                 {"unityPlayerName", Json{{"type", "string"}}},
                 {"gameAssemblyName", Json{{"type", "string"}}},
                 {"timeoutMs", Json{{"type", "integer"}}},
                 {"replace", Json{{"type", "boolean"}}}}},
           {"required", Json::array({"targetProcess"})},
           {"additionalProperties", false}},
      Json{{"type", "object"}}, {"session.admin"},
      [&state](const Json &arguments, const RequestContext &) {
        return state.Connect(arguments);
      }};
}

ToolDefinition DisconnectTool(ServiceState &state) {
  return ToolDefinition{
      "unity_session_disconnect",
      "Disconnect and invalidate the session generation",
      Json{{"type", "object"},
           {"properties", Json{{"expectedGeneration", Json{{"type", "integer"}}}}},
           {"additionalProperties", false}},
      Json{{"type", "object"}}, {"session.admin"},
      [&state](const Json &arguments, const RequestContext &) {
        return state.Disconnect(arguments);
      }};
}

ToolDefinition ReadTool(ServiceState &state) {
  return ToolDefinition{
      "unity_memory_read", "Read target memory through the DMA session",
      Json{{"type", "object"},
           {"properties", Json{{"expectedGeneration", Json{{"type", "integer"}}},
                                {"address", Json{{"type", "string"}}},
                                {"length", Json{{"type", "integer"}}},
                                {"nocache", Json{{"type", "boolean"}}}}},
           {"required", Json::array({"expectedGeneration", "address", "length"})},
           {"additionalProperties", false}},
      Json{{"type", "object"}}, {"read"},
      [&state](const Json &arguments, const RequestContext &) {
        return state.Read(arguments);
      }};
}

ToolDefinition ReadPtrTool(ServiceState &state) {
  return ToolDefinition{
      "unity_memory_read_ptr",
      "Read an 8-byte pointer at an address and return its little-endian value",
      Json{{"type", "object"},
           {"properties", Json{{"expectedGeneration", Json{{"type", "integer"}}},
                                {"address", Json{{"type", "string"}}},
                                {"nocache", Json{{"type", "boolean"}}}}},
           {"required", Json::array({"expectedGeneration", "address"})},
           {"additionalProperties", false}},
      Json{{"type", "object"}}, {"read"},
      [&state](const Json &arguments, const RequestContext &) {
        return state.ReadPtr(arguments);
      }};
}

ToolDefinition ReadStringTool(ServiceState &state) {
  return ToolDefinition{
      "unity_memory_read_string",
      "Read a NUL-terminated C string (ascii or utf16) at an address",
      Json{{"type", "object"},
           {"properties", Json{{"expectedGeneration", Json{{"type", "integer"}}},
                                {"address", Json{{"type", "string"}}},
                                {"maxBytes", Json{{"type", "integer"}}},
                                {"encoding",
                                 Json{{"type", "string"},
                                      {"enum", Json::array({"ascii", "utf16"})}}},
                                {"nocache", Json{{"type", "boolean"}}}}},
           {"required", Json::array({"expectedGeneration", "address"})},
           {"additionalProperties", false}},
      Json{{"type", "object"}}, {"read"},
      [&state](const Json &arguments, const RequestContext &) {
        return state.ReadString(arguments);
      }};
}

ToolDefinition PointerChainTool(ServiceState &state) {
  return ToolDefinition{
      "unity_pointer_chain_resolve",
      "Dereference a pointer chain: start at base, follow 8-byte pointers at "
      "base+offsets[i] sequentially, return every intermediate hop",
      Json{{"type", "object"},
           {"properties", Json{{"expectedGeneration", Json{{"type", "integer"}}},
                                {"base", Json{{"type", "string"}}},
                                {"offsets", Json{{"type", "array"},
                                                 {"items", Json{{"type", "integer"}}}}},
                                {"nocache", Json{{"type", "boolean"}}}}},
           {"required",
            Json::array({"expectedGeneration", "base", "offsets"})},
           {"additionalProperties", false}},
      Json{{"type", "object"}}, {"read"},
      [&state](const Json &arguments, const RequestContext &) {
        return state.ResolvePointerChain(arguments);
      }};
}

ToolDefinition ModulesTool(ServiceState &state,
                           std::string toolName = "unity_modules_list") {
  return ToolDefinition{
      std::move(toolName), "List module snapshots for a session generation",
      Json{{"type", "object"},
           {"properties", Json{{"expectedGeneration", Json{{"type", "integer"}}}}},
           {"required", Json::array({"expectedGeneration"})},
           {"additionalProperties", false}},
      Json{{"type", "object"}}, {"read"},
      [&state](const Json &arguments, const RequestContext &) {
        return state.Modules(arguments);
      }};
}

ToolDefinition WriteTool(ServiceState &state) {
  return ToolDefinition{
      "unity_memory_write",
      "Conditionally write target memory with expected-before and readback checks",
      Json{{"type", "object"},
           {"properties", Json{{"expectedGeneration", Json{{"type", "integer"}}},
                                {"address", Json{{"type", "string"}}},
                                {"data", Json{{"type", "string"}}},
                                {"expectedBefore", Json{{"type", "string"}}},
                                {"mask", Json{{"type", "string"}}},
                                {"verifyReadback", Json{{"type", "boolean"}}}}},
           {"required", Json::array({"expectedGeneration", "address", "data",
                                      "expectedBefore"})},
           {"additionalProperties", false}},
      Json{{"type", "object"}}, {"memory.write"},
      [&state](const Json &arguments, const RequestContext &) {
        return state.Write(arguments);
      }};
}

struct AppOptions {
  enum class Transport { Http, Stdio };
  Transport transport = Transport::Http;
  std::string bindAddress = "127.0.0.1";
  std::uint16_t port = 18765;
  std::string path = "/mcp";
  std::string bearerToken;
  std::vector<std::string> allowedOrigins;
  std::string targetProcess;
  bool autoConnect = false;
  bool enableWrites = false;
  std::vector<AddressRange> writeRanges;
};

void PrintUsage(std::ostream &out) {
  out << "UnityExplorerMcpServer\n"
      << "  --transport http|stdio     MCP transport (default: http)\n"
      << "  --bind 127.0.0.1           HTTP bind address (loopback only)\n"
      << "  --port 18765               HTTP port\n"
      << "  --token VALUE              HTTP bearer token (or env UNITY_EXPLORER_MCP_TOKEN)\n"
      << "  --allow-origin ORIGIN      Exact HTTP Origin allowlist entry\n"
      << "  --target PROCESS            Target process for --auto-connect\n"
      << "  --auto-connect              Connect before serving; failures remain diagnosable\n"
      << "  --enable-writes             Enable guarded memory write tool (deny-all without ranges)\n"
      << "  --write-range BASE:LENGTH  Allow writes only inside this range (repeatable)\n"
      << "  --help                      Show this help\n";
}

bool ParsePort(const std::string &text, std::uint16_t &port) {
  Json value = text;
  std::uint64_t parsed = 0;
  if (!ParseUnsigned(value, parsed) || parsed == 0 || parsed > 65535) {
    return false;
  }
  port = static_cast<std::uint16_t>(parsed);
  return true;
}

bool ParseAddressRange(const std::string &text, AddressRange &range) {
  const std::size_t separator = text.find(':');
  if (separator == std::string::npos || separator == 0 ||
      separator + 1 >= text.size() ||
      text.find(':', separator + 1) != std::string::npos) {
    return false;
  }
  const std::string baseText = text.substr(0, separator);
  const std::string lengthText = text.substr(separator + 1);
  std::uint64_t base = 0;
  std::uint64_t length = 0;
  if (!ParseUnsigned(Json(baseText), base) ||
      !ParseUnsigned(Json(lengthText), length) || base == 0 || length == 0 ||
      base > std::numeric_limits<std::uintptr_t>::max() ||
      length > std::numeric_limits<std::uintptr_t>::max() -
                   static_cast<std::uintptr_t>(base)) {
    return false;
  }
  const std::uintptr_t nativeBase = static_cast<std::uintptr_t>(base);
  const std::uintptr_t nativeLength = static_cast<std::uintptr_t>(length);
  if (!unityexplorer::analysis::IsCanonicalUserAddress(nativeBase) ||
      !unityexplorer::analysis::IsCanonicalUserAddress(
          nativeBase + nativeLength - static_cast<std::uintptr_t>(1))) {
    return false;
  }
  range.base = nativeBase;
  range.length = static_cast<std::size_t>(nativeLength);
  return true;
}

bool ParseArguments(int argc, char **argv, AppOptions &options,
                    std::ostream &error) {
  for (int i = 1; i < argc; ++i) {
    const std::string argument = argv[i] != nullptr ? argv[i] : "";
    auto nextValue = [&](std::string &value) {
      if (i + 1 >= argc || argv[i + 1] == nullptr) {
        return false;
      }
      value = argv[++i];
      return true;
    };
    if (argument == "--help" || argument == "-h") {
      PrintUsage(error);
      return false;
    }
    if (argument == "--enable-writes") {
      options.enableWrites = true;
      continue;
    }
    std::string value;
    if (argument == "--transport") {
      if (!nextValue(value)) {
        error << "--transport requires a value\n";
        return false;
      }
      if (LowerAscii(value) == "stdio") {
        options.transport = AppOptions::Transport::Stdio;
      } else if (LowerAscii(value) == "http" ||
                 LowerAscii(value) == "streamable-http") {
        options.transport = AppOptions::Transport::Http;
      } else {
        error << "unsupported transport: " << value << "\n";
        return false;
      }
      continue;
    }
    if (argument == "--bind") {
      if (!nextValue(options.bindAddress)) {
        error << "--bind requires a value\n";
        return false;
      }
      continue;
    }
    if (argument == "--port") {
      if (!nextValue(value) || !ParsePort(value, options.port)) {
        error << "--port must be in [1, 65535]\n";
        return false;
      }
      continue;
    }
    if (argument == "--token") {
      if (!nextValue(options.bearerToken) || options.bearerToken.empty()) {
        error << "--token requires a non-empty value\n";
        return false;
      }
      continue;
    }
    if (argument == "--allow-origin") {
      if (!nextValue(value) || value.empty()) {
        error << "--allow-origin requires a non-empty value\n";
        return false;
      }
      options.allowedOrigins.push_back(value);
      continue;
    }
    if (argument == "--target") {
      if (!nextValue(options.targetProcess) || options.targetProcess.empty()) {
        error << "--target requires a process name\n";
        return false;
      }
      continue;
    }
    if (argument == "--write-range") {
      if (!nextValue(value)) {
        error << "--write-range requires BASE:LENGTH\n";
        return false;
      }
      AddressRange range;
      if (!ParseAddressRange(value, range)) {
        error << "--write-range must be a canonical user range BASE:LENGTH\n";
        return false;
      }
      options.writeRanges.push_back(range);
      continue;
    }
    if (argument == "--auto-connect") {
      options.autoConnect = true;
      continue;
    }
    error << "unknown argument: " << argument << "\n";
    return false;
  }
  if (options.autoConnect && options.targetProcess.empty()) {
    error << "--auto-connect requires --target PROCESS\n";
    return false;
  }
  if (!options.writeRanges.empty() && !options.enableWrites) {
    error << "--write-range requires --enable-writes\n";
    return false;
  }
  return true;
}

std::string GenerateToken() {
  std::array<std::uint8_t, 32> bytes{};
  try {
    std::random_device random;
    for (std::uint8_t &byte : bytes) {
      byte = static_cast<std::uint8_t>(random());
    }
  } catch (...) {
    const auto now = static_cast<std::uint64_t>(
        std::chrono::high_resolution_clock::now().time_since_epoch().count());
    for (std::size_t i = 0; i < bytes.size(); ++i) {
      bytes[i] = static_cast<std::uint8_t>((now >> ((i % 8) * 8)) ^
                                           static_cast<std::uint64_t>(i * 37));
    }
  }
  return HexBytes(std::vector<std::uint8_t>(bytes.begin(), bytes.end()));
}

} // namespace

int main(int argc, char **argv) {
  std::signal(SIGINT, SignalHandler);
  std::signal(SIGTERM, SignalHandler);

  AppOptions options;
  if (!ParseArguments(argc, argv, options, std::cerr)) {
    return (argc > 1 && (std::string(argv[1]) == "--help" ||
                         std::string(argv[1]) == "-h"))
               ? 0
               : 2;
  }

  ServiceState state(options.enableWrites, std::move(options.writeRanges));
  McpServer server;
  server.RegisterTool(StatusTool(state));
  server.RegisterTool(ConnectTool(state));
  server.RegisterTool(DisconnectTool(state));
  server.RegisterTool(ReadTool(state));
  server.RegisterTool(ReadPtrTool(state));
  server.RegisterTool(ReadStringTool(state));
  server.RegisterTool(PointerChainTool(state));
  server.RegisterTool(ModulesTool(state));
  // Keep the pre-MCP-plan name as a compatibility alias for existing clients.
  server.RegisterTool(ModulesTool(state, "unity_session_modules"));
  // Keep the tool list deterministic for a given startup policy.  Raw writes
  // are absent unless explicitly enabled, instead of being advertised and
  // then rejected only after a call reaches the handler.
  if (options.enableWrites) {
    server.RegisterTool(WriteTool(state));
  }

  RequestContext context;
  context.clientId = "local";
  context.scopes.insert("read");
  context.scopes.insert("session.admin");
  if (options.enableWrites) {
    context.scopes.insert("memory.write");
  }

  if (options.autoConnect) {
    const ToolResult result =
        state.Connect(Json{{"targetProcess", options.targetProcess},
                           {"mode", "dma"}, {"replace", false}});
    if (result.isError) {
      std::cerr << "[MCP] auto-connect failed: " << result.text << "\n";
    } else {
      std::cerr << "[MCP] auto-connected to " << options.targetProcess << "\n";
    }
  }

  int result = 0;
  if (options.transport == AppOptions::Transport::Stdio) {
    std::cerr << "[MCP] stdio transport is ready; waiting for JSON-RPC frames\n";
    unityexplorer::mcp::StdioTransport transport;
    result = transport.Run(server, context, std::cin, std::cout, std::cerr);
  } else {
    unityexplorer::mcp::HttpTransportOptions httpOptions;
    httpOptions.bindAddress = options.bindAddress;
    httpOptions.port = options.port;
    httpOptions.path = options.path;
    httpOptions.bearerToken = options.bearerToken;
    if (httpOptions.bearerToken.empty()) {
      const char *environmentToken = std::getenv("UNITY_EXPLORER_MCP_TOKEN");
      httpOptions.bearerToken = environmentToken != nullptr ? environmentToken : "";
    }
    if (httpOptions.bearerToken.empty()) {
      httpOptions.bearerToken = GenerateToken();
      std::cerr << "[MCP] generated bearer token: " << httpOptions.bearerToken
                << "\n";
    }
    for (const std::string &origin : options.allowedOrigins) {
      httpOptions.allowedOrigins.insert(origin);
    }
    std::cerr << "[MCP] HTTP endpoint: http://" << httpOptions.bindAddress
              << ':' << httpOptions.port << httpOptions.path << "\n";
    unityexplorer::mcp::StreamableHttpTransport transport(httpOptions);
    std::atomic<bool> transportFinished{false};
    std::thread transportThread([&]() {
      result = transport.Run(server, context, &g_stop, &std::cerr);
      transportFinished.store(true, std::memory_order_release);
    });

    // Run() blocks in accept().  A signal only flips g_stop, so explicitly
    // close the listening socket from this control thread to wake accept and
    // let Run() join its request workers before service teardown.
    while (!transportFinished.load(std::memory_order_acquire)) {
      if (g_stop.load(std::memory_order_relaxed)) {
        transport.Stop();
      }
      std::this_thread::sleep_for(std::chrono::milliseconds(25));
    }
    transport.Stop();
    transportThread.join();
  }

  // Transport has stopped accepting requests and joined HTTP workers before
  // this point.  The service now drains its executor and releases DMA state.
  state.Shutdown();
  return result;
}
