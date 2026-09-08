#pragma once

// Transport-independent session facade.  The default implementation has no
// VMMDLL dependency: production code supplies a callback/backend adapter for
// MetickAdapter, while tests can use an in-memory fake backend.

#include "session_executor.hpp"

#include <algorithm>
#include <atomic>
#include <cstddef>
#include <cstdint>
#include <exception>
#include <functional>
#include <limits>
#include <memory>
#include <mutex>
#include <optional>
#include <string>
#include <utility>
#include <vector>

namespace unityexplorer::analysis {

enum class SessionState { Idle, Connecting, Ready, Draining, Faulted };

inline const char *ToString(SessionState state) noexcept {
  switch (state) {
  case SessionState::Idle:
    return "idle";
  case SessionState::Connecting:
    return "connecting";
  case SessionState::Ready:
    return "ready";
  case SessionState::Draining:
    return "draining";
  case SessionState::Faulted:
    return "faulted";
  }
  return "unknown";
}

struct ModuleSnapshot {
  std::string name;
  std::string fullName;
  std::uintptr_t baseAddress = 0;
  std::uint32_t imageSize = 0;
  bool isWow64 = false;
};

// A write range is an explicitly approved half-open virtual-address interval
// [base, base + length).  Ranges are intentionally kept as individual
// entries: a single request must fit wholly inside one entry and may not be
// assembled from adjacent ranges.
struct AddressRange {
  std::uintptr_t base = 0;
  std::size_t length = 0;
};

struct ConnectRequest {
  std::string targetProcess;
  std::string mode = "dma";
  std::string unityPlayerName;
  std::string gameAssemblyName;
  std::uint32_t timeoutMs = 30000;
  bool replace = false;
};

struct BackendSnapshot {
  std::uint32_t pid = 0;
  std::string targetProcess;
  std::string runtime = "unknown";
  std::vector<ModuleSnapshot> modules;
};

struct SessionStatus {
  SessionState state = SessionState::Idle;
  std::uint64_t generation = 0;
  std::uint32_t pid = 0;
  std::string targetProcess;
  std::string runtime = "unknown";
  std::vector<ModuleSnapshot> modules;
  std::string lastError;
  bool writeEnabled = false;
  // Policy describes the configured write gate even while the session is
  // idle: disabled, deny_all, or allowlist.
  std::string writePolicy = "disabled";
  std::vector<AddressRange> writeRanges;
  std::size_t pendingJobs = 0;
  bool activeJob = false;
};

struct SessionResult {
  bool ok = false;
  std::string code;
  std::string message;
  std::uint64_t generation = 0;

  static SessionResult Success(std::uint64_t generation = 0,
                               std::string message = {}) {
    SessionResult result;
    result.ok = true;
    result.generation = generation;
    result.message = std::move(message);
    return result;
  }

  static SessionResult Failure(std::string code, std::string message,
                               std::uint64_t generation = 0) {
    SessionResult result;
    result.code = std::move(code);
    result.message = std::move(message);
    result.generation = generation;
    return result;
  }
};

struct ReadResult : SessionResult {
  std::vector<std::uint8_t> bytes;
  std::size_t requested = 0;
  std::size_t actual = 0;
};

struct WriteRequest {
  std::uint64_t expectedGeneration = 0;
  std::uintptr_t address = 0;
  std::vector<std::uint8_t> data;
  std::vector<std::uint8_t> expectedBefore;
  std::vector<std::uint8_t> mask;
  bool verifyReadback = true;
};

struct WriteResult : SessionResult {
  std::vector<std::uint8_t> before;
  std::vector<std::uint8_t> readback;
  bool readbackMatches = false;
};

inline bool IsCanonicalUserAddress(std::uintptr_t address) noexcept {
#if INTPTR_MAX > INT32_MAX
  // The DMA API accepts process virtual addresses, never kernel/non-canonical
  // addresses. Windows user-mode x64 currently uses the low canonical half.
  return static_cast<std::uint64_t>(address) <= 0x00007FFFFFFFFFFFULL;
#else
  return address != 0;
#endif
}

class IMemoryBackend {
public:
  virtual ~IMemoryBackend() = default;

  // Connect must fully validate the target (VMM, PID, CR3 and required
  // modules) before returning true.  On failure it should populate error.
  virtual bool Connect(const ConnectRequest &request, BackendSnapshot &snapshot,
                       std::string &error) = 0;

  // Optional stable error classification for a failed Connect call.  The
  // legacy backend contract only returned a human-readable string, so this is
  // deliberately non-pure to keep existing test/back-end implementations
  // source compatible.  Returning nullptr (or an empty string) makes the
  // session use the generic `dma_init_failed` code.
  virtual const char *LastErrorCode() const noexcept { return nullptr; }
  virtual void Disconnect() noexcept = 0;
  virtual bool IsConnected() const noexcept = 0;
  virtual bool Read(std::uintptr_t address, void *buffer, std::size_t size,
                    bool noCache, std::string &error) = 0;
  virtual bool Write(std::uintptr_t address, const void *buffer,
                     std::size_t size, std::string &error) = 0;
  virtual std::vector<ModuleSnapshot> Modules() const = 0;
};

// Convenient callback adapter for existing MetickAdapter wrappers and test
// doubles.  Empty callbacks fail explicitly instead of pretending readiness.
class CallbackMemoryBackend final : public IMemoryBackend {
public:
  using ConnectFn = std::function<bool(const ConnectRequest &, BackendSnapshot &,
                                       std::string &)>;
  using DisconnectFn = std::function<void()>;
  using ReadFn = std::function<bool(std::uintptr_t, void *, std::size_t, bool,
                                    std::string &)>;
  using WriteFn = std::function<bool(std::uintptr_t, const void *, std::size_t,
                                     std::string &)>;
  using IsConnectedFn = std::function<bool()>;
  using ModulesFn = std::function<std::vector<ModuleSnapshot>()>;

  CallbackMemoryBackend(ConnectFn connect, DisconnectFn disconnect, ReadFn read,
                        WriteFn write, IsConnectedFn connected = {},
                        ModulesFn modules = {})
      : connect_(std::move(connect)), disconnect_(std::move(disconnect)),
        read_(std::move(read)), write_(std::move(write)),
        connected_(std::move(connected)), modules_(std::move(modules)) {}

  bool Connect(const ConnectRequest &request, BackendSnapshot &snapshot,
               std::string &error) override {
    if (!connect_) {
      error = "memory backend has no connect callback";
      return false;
    }
    try {
      const bool connected = connect_(request, snapshot, error);
      connectedState_.store(connected, std::memory_order_release);
      return connected;
    } catch (const std::exception &exception) {
      connectedState_.store(false, std::memory_order_release);
      error = exception.what();
      return false;
    } catch (...) {
      connectedState_.store(false, std::memory_order_release);
      error = "memory backend connect callback threw an unknown exception";
      return false;
    }
  }

  void Disconnect() noexcept override {
    connectedState_.store(false, std::memory_order_release);
    if (disconnect_) {
      try {
        disconnect_();
      } catch (...) {
      }
    }
  }

  bool IsConnected() const noexcept override {
    if (connected_) {
      try {
        return connected_();
      } catch (...) {
        return false;
      }
    }
    return connectedState_.load(std::memory_order_acquire);
  }

  bool Read(std::uintptr_t address, void *buffer, std::size_t size,
            bool noCache, std::string &error) override {
    if (!read_) {
      error = "memory backend has no read callback";
      return false;
    }
    try {
      return read_(address, buffer, size, noCache, error);
    } catch (const std::exception &exception) {
      error = exception.what();
      return false;
    } catch (...) {
      error = "memory backend read callback threw an unknown exception";
      return false;
    }
  }

  bool Write(std::uintptr_t address, const void *buffer, std::size_t size,
             std::string &error) override {
    if (!write_) {
      error = "memory backend has no write callback";
      return false;
    }
    try {
      return write_(address, buffer, size, error);
    } catch (const std::exception &exception) {
      error = exception.what();
      return false;
    } catch (...) {
      error = "memory backend write callback threw an unknown exception";
      return false;
    }
  }

  std::vector<ModuleSnapshot> Modules() const override {
    if (!modules_) {
      return {};
    }
    try {
      return modules_();
    } catch (...) {
      return {};
    }
  }

private:
  ConnectFn connect_;
  DisconnectFn disconnect_;
  ReadFn read_;
  WriteFn write_;
  IsConnectedFn connected_;
  ModulesFn modules_;
  std::atomic<bool> connectedState_{false};
};

struct AnalysisSessionOptions {
  std::size_t maxReadBytes = 64u * 1024u;
  // 4096 (one 4KiB page) allows the CrossFade memory-shock shellcode block
  // (~400 bytes) to be written in a single request without crossing a page.
  // Page-boundary and canonical-address checks still apply per write.
  std::size_t maxWriteBytes = 4096u;
  bool enableWrites = false;
  bool requireExpectedBefore = true;
  bool requireReadback = true;
  std::vector<AddressRange> writeRanges;
  SessionExecutor *executor = nullptr;
};

class AnalysisSession final {
public:
  explicit AnalysisSession(std::shared_ptr<IMemoryBackend> backend,
                           AnalysisSessionOptions options = {})
      : backend_(std::move(backend)), options_(std::move(options)) {
    status_.writePolicy = ConfiguredWritePolicy();
    status_.writeRanges = options_.writeRanges;
  }

  AnalysisSession(const AnalysisSession &) = delete;
  AnalysisSession &operator=(const AnalysisSession &) = delete;

  ~AnalysisSession() { Disconnect(); }

  SessionStatus Status() const {
    // Do not hold stateMutex_ while taking the executor lock.  A shutdown
    // caller can own the executor lock while joining a worker that is reading
    // this session, so state -> executor ordering would deadlock.
    SessionStatus status;
    {
      std::lock_guard<std::mutex> lock(stateMutex_);
      status = status_;
    }
    if (options_.executor) {
      status.pendingJobs = options_.executor->Pending();
      status.activeJob = options_.executor->Active();
    }
    return status;
  }

  SessionResult Connect(const ConnectRequest &request) {
    std::lock_guard<std::mutex> command(commandMutex_);
    if (!backend_) {
      return SetFault("dma_init_failed", "memory backend is not configured");
    }
    if (request.targetProcess.empty()) {
      // Invalid caller input must not poison an existing usable session or
      // leave its backend connected while the public state says faulted.
      return SessionResult::Failure("invalid_argument",
                                    "targetProcess must not be empty",
                                    CurrentGeneration());
    }
    {
      std::lock_guard<std::mutex> lock(stateMutex_);
      if (status_.state == SessionState::Connecting ||
          status_.state == SessionState::Draining) {
        return SessionResult::Failure("busy", "session transition is active",
                                      status_.generation);
      }
      if (status_.state == SessionState::Ready && !request.replace) {
        return SessionResult::Failure(
            "busy", "session is already connected; set replace=true",
            status_.generation);
      }
      if (status_.state == SessionState::Ready ||
          status_.state == SessionState::Faulted) {
        status_.state = SessionState::Draining;
        ++status_.generation; // invalidate all old addresses immediately
      }
      status_.state = SessionState::Connecting;
      status_.targetProcess = request.targetProcess;
      status_.pid = 0;
      status_.runtime = "unknown";
      status_.modules.clear();
      status_.lastError.clear();
    }

    // A backend owns the process-global VMM handle.  Always drain it before a
    // replacement connect; this also makes repeated failed connects clean.
    backend_->Disconnect();

    BackendSnapshot snapshot;
    std::string error;
    if (!backend_->Connect(request, snapshot, error)) {
      if (error.empty()) {
        error = "backend connect failed";
      }
      const char *reportedCode = backend_->LastErrorCode();
      const std::string failureCode =
          reportedCode != nullptr && reportedCode[0] != '\0'
              ? std::string(reportedCode)
              : std::string("dma_init_failed");
      // Backends may have acquired a VMM handle before discovering a target
      // or CR3 failure.  Disconnect on every failed attempt so a retry cannot
      // inherit partial state.
      backend_->Disconnect();
      return SetFault(failureCode, error);
    }
    if (snapshot.pid == 0 || !backend_->IsConnected()) {
      backend_->Disconnect();
      return SetFault("target_not_found",
                      snapshot.pid == 0
                          ? "backend returned an invalid PID"
                          : "backend reported disconnected after connect");
    }

    std::lock_guard<std::mutex> lock(stateMutex_);
    status_.state = SessionState::Ready;
    ++status_.generation; // publish a new, valid generation
    status_.pid = snapshot.pid;
    status_.targetProcess = snapshot.targetProcess.empty()
                                ? request.targetProcess
                                : snapshot.targetProcess;
    status_.runtime = snapshot.runtime.empty() ? "unknown" : snapshot.runtime;
    status_.modules = std::move(snapshot.modules);
    status_.lastError.clear();
    status_.writeEnabled = options_.enableWrites;
    status_.writePolicy = ConfiguredWritePolicy();
    status_.writeRanges = options_.writeRanges;
    return SessionResult::Success(status_.generation);
  }

  SessionResult Disconnect(std::optional<std::uint64_t> expectedGeneration =
                               std::nullopt) {
    std::lock_guard<std::mutex> command(commandMutex_);
    {
      std::lock_guard<std::mutex> lock(stateMutex_);
      if (expectedGeneration.has_value() &&
          expectedGeneration.value() != status_.generation) {
        return SessionResult::Failure("stale_generation",
                                      "expectedGeneration does not match session",
                                      status_.generation);
      }
      if (status_.state == SessionState::Idle) {
        return SessionResult::Success(status_.generation);
      }
      status_.state = SessionState::Draining;
      ++status_.generation;
    }

    if (backend_) {
      backend_->Disconnect();
    }

    std::lock_guard<std::mutex> lock(stateMutex_);
    status_.state = SessionState::Idle;
    status_.pid = 0;
    status_.targetProcess.clear();
    status_.runtime = "unknown";
    status_.modules.clear();
    status_.lastError.clear();
    status_.writeEnabled = false;
    status_.writePolicy = ConfiguredWritePolicy();
    status_.writeRanges = options_.writeRanges;
    return SessionResult::Success(status_.generation);
  }

  ReadResult Read(std::uint64_t expectedGeneration, std::uintptr_t address,
                  std::size_t size, bool noCache = false) {
    ReadResult result;
    result.requested = size;
    std::lock_guard<std::mutex> command(commandMutex_);
    const SessionResult guard = CheckRead(expectedGeneration, address, size);
    if (!guard.ok) {
      result.code = guard.code;
      result.message = guard.message;
      result.generation = guard.generation;
      return result;
    }

    result.bytes.resize(size);
    std::string error;
    if (!backend_->Read(address, result.bytes.data(), size, noCache, error)) {
      result.bytes.clear();
      result.code = "read_failed";
      result.message = error.empty() ? "backend read failed" : error;
      result.generation = CurrentGeneration();
      return result;
    }
    result.ok = true;
    result.actual = size;
    result.generation = CurrentGeneration();
    return result;
  }

  std::pair<SessionResult, std::vector<ModuleSnapshot>> Modules(
      std::uint64_t expectedGeneration) const {
    std::lock_guard<std::mutex> lock(stateMutex_);
    if (expectedGeneration != status_.generation) {
      return {SessionResult::Failure("stale_generation",
                                     "expectedGeneration does not match session",
                                     status_.generation),
              {}};
    }
    if (status_.state != SessionState::Ready) {
      return {SessionResult::Failure("not_connected",
                                     "session is not ready",
                                     status_.generation),
              {}};
    }
    return {SessionResult::Success(status_.generation), status_.modules};
  }

  WriteResult Write(const WriteRequest &request) {
    WriteResult result;
    std::lock_guard<std::mutex> command(commandMutex_);
    {
      std::lock_guard<std::mutex> lock(stateMutex_);
      result.generation = status_.generation;
      if (!options_.enableWrites) {
        result.code = "permission_denied";
        result.message = "memory writes are disabled by session policy";
        return result;
      }
      if (options_.writeRanges.empty()) {
        result.code = "permission_denied";
        result.message =
            "memory writes are denied because no write ranges are configured";
        return result;
      }
      if (request.expectedGeneration != status_.generation) {
        result.code = "stale_generation";
        result.message = "expectedGeneration does not match session";
        return result;
      }
      if (status_.state != SessionState::Ready) {
        result.code = "not_connected";
        result.message = "session is not ready";
        return result;
      }
    }
    if (!request.address || request.data.empty() ||
        request.data.size() > options_.maxWriteBytes) {
      result.code = "invalid_argument";
      result.message = "invalid address or write size";
      return result;
    }
    if (!IsCanonicalUserAddress(request.address)) {
      result.code = "invalid_argument";
      result.message = "write address is not a canonical user address";
      return result;
    }
    if (request.address >
        std::numeric_limits<std::uintptr_t>::max() - request.data.size()) {
      result.code = "invalid_argument";
      result.message = "write address range overflows";
      return result;
    }
    const std::uintptr_t lastAddress =
        request.address + request.data.size() - static_cast<std::size_t>(1);
    if (!IsCanonicalUserAddress(lastAddress)) {
      result.code = "invalid_argument";
      result.message = "write address range is not canonical user memory";
      return result;
    }
    constexpr std::uintptr_t kPageMask = static_cast<std::uintptr_t>(0xFFFu);
    if ((request.address & kPageMask) + request.data.size() >
        static_cast<std::size_t>(0x1000u)) {
      result.code = "invalid_argument";
      result.message = "cross-page writes are not allowed";
      return result;
    }
    if (!IsWriteRangeAllowed(request.address, request.data.size())) {
      result.code = "permission_denied";
      result.message =
          "write range is outside the configured memory write allowlist";
      return result;
    }
    // An expected-before buffer is optional only when the policy allows it,
    // but a supplied buffer must always have exactly the write length.  This
    // prevents the comparison loop below from indexing a short vector when
    // requireExpectedBefore is disabled.
    if ((options_.requireExpectedBefore && request.expectedBefore.empty()) ||
        (!request.expectedBefore.empty() &&
         request.expectedBefore.size() != request.data.size())) {
      result.code = "invalid_argument";
      result.message = "expectedBefore must match data length";
      return result;
    }
    if (!request.mask.empty() && request.mask.size() != request.data.size()) {
      result.code = "invalid_argument";
      result.message = "mask must match data length";
      return result;
    }

    result.before.resize(request.data.size());
    std::string error;
    if (!backend_->Read(request.address, result.before.data(), result.before.size(),
                        true, error)) {
      result.before.clear();
      result.code = "read_failed";
      result.message = error.empty() ? "failed to read expectedBefore" : error;
      return result;
    }
    if (!request.expectedBefore.empty()) {
      for (std::size_t i = 0; i < request.data.size(); ++i) {
        const std::uint8_t mask = request.mask.empty() ? 0xFFu : request.mask[i];
        if (((result.before[i] ^ request.expectedBefore[i]) & mask) != 0) {
          result.code = "compare_failed";
          result.message = "target memory differs from expectedBefore";
          return result;
        }
      }
    }
    if (!backend_->Write(request.address, request.data.data(), request.data.size(),
                         error)) {
      result.code = "write_failed";
      result.message = error.empty() ? "backend write failed" : error;
      return result;
    }
    if (request.verifyReadback || options_.requireReadback) {
      result.readback.resize(request.data.size());
      if (!backend_->Read(request.address, result.readback.data(),
                          result.readback.size(), true, error)) {
        result.code = "readback_failed";
        result.message = error.empty() ? "failed to read write readback" : error;
        return result;
      }
      result.readbackMatches = result.readback == request.data;
      if (!result.readbackMatches) {
        result.code = "readback_mismatch";
        result.message = "write readback differs from requested data";
        return result;
      }
    } else {
      result.readbackMatches = true;
    }
    result.ok = true;
    result.generation = CurrentGeneration();
    return result;
  }

private:
  SessionResult SetFault(const std::string &code, const std::string &message) {
    std::lock_guard<std::mutex> lock(stateMutex_);
    status_.state = SessionState::Faulted;
    status_.lastError = message;
    status_.pid = 0;
    status_.runtime = "unknown";
    status_.modules.clear();
    status_.writeEnabled = false;
    status_.writePolicy = ConfiguredWritePolicy();
    status_.writeRanges = options_.writeRanges;
    return SessionResult::Failure(code, message, status_.generation);
  }

  std::string ConfiguredWritePolicy() const {
    if (!options_.enableWrites) {
      return "disabled";
    }
    return options_.writeRanges.empty() ? "deny_all" : "allowlist";
  }

  bool IsWriteRangeAllowed(std::uintptr_t address,
                           std::size_t size) const noexcept {
    if (size == 0 || options_.writeRanges.empty() ||
        address > std::numeric_limits<std::uintptr_t>::max() - size) {
      return false;
    }
    const std::uintptr_t endExclusive =
        address + static_cast<std::uintptr_t>(size);
    for (const AddressRange &range : options_.writeRanges) {
      if (range.base == 0 || range.length == 0 ||
          !IsCanonicalUserAddress(range.base) ||
          range.base > std::numeric_limits<std::uintptr_t>::max() -
                            range.length) {
        continue;
      }
      const std::uintptr_t rangeEnd =
          range.base + static_cast<std::uintptr_t>(range.length);
      if (!IsCanonicalUserAddress(rangeEnd - 1)) {
        continue;
      }
      if (address >= range.base && endExclusive <= rangeEnd) {
        return true;
      }
    }
    return false;
  }

  SessionResult CheckRead(std::uint64_t expectedGeneration,
                          std::uintptr_t address, std::size_t size) const {
    std::lock_guard<std::mutex> lock(stateMutex_);
    if (expectedGeneration != status_.generation) {
      return SessionResult::Failure("stale_generation",
                                    "expectedGeneration does not match session",
                                    status_.generation);
    }
    if (status_.state != SessionState::Ready || !backend_ ||
        !backend_->IsConnected()) {
      return SessionResult::Failure("not_connected", "session is not ready",
                                    status_.generation);
    }
    if (!address || size == 0 || size > options_.maxReadBytes) {
      return SessionResult::Failure("invalid_argument",
                                    "address or read length is invalid",
                                    status_.generation);
    }
    if (!IsCanonicalUserAddress(address)) {
      return SessionResult::Failure(
          "invalid_argument", "read address is not a canonical user address",
          status_.generation);
    }
    if (address > std::numeric_limits<std::uintptr_t>::max() - size) {
      return SessionResult::Failure("invalid_argument", "address range overflows",
                                    status_.generation);
    }
    const std::uintptr_t lastAddress =
        address + static_cast<std::uintptr_t>(size - 1);
    if (!IsCanonicalUserAddress(lastAddress)) {
      return SessionResult::Failure(
          "invalid_argument",
          "read address range is not canonical user memory",
          status_.generation);
    }
    return SessionResult::Success(status_.generation);
  }

  std::uint64_t CurrentGeneration() const {
    std::lock_guard<std::mutex> lock(stateMutex_);
    return status_.generation;
  }

  std::shared_ptr<IMemoryBackend> backend_;
  AnalysisSessionOptions options_;
  mutable std::mutex stateMutex_;
  mutable std::mutex commandMutex_;
  SessionStatus status_;
};

} // namespace unityexplorer::analysis
