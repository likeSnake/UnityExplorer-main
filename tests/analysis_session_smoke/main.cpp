#include "analysis/analysis_session.hpp"

#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <cstring>
#include <future>
#include <iostream>
#include <limits>
#include <memory>
#include <stdexcept>
#include <string>
#include <thread>
#include <vector>

namespace {

using unityexplorer::analysis::BackendSnapshot;
using unityexplorer::analysis::ConnectRequest;
using unityexplorer::analysis::ExecutorBusy;
using unityexplorer::analysis::ExecutorStopped;
using unityexplorer::analysis::IMemoryBackend;
using unityexplorer::analysis::ModuleSnapshot;
using unityexplorer::analysis::SessionExecutor;

constexpr std::uintptr_t kBase = static_cast<std::uintptr_t>(0x100000u);
constexpr std::size_t kStorageSize = 0x3000u;

bool Expect(bool condition, const char *message) {
  if (condition) {
    return true;
  }
  std::cerr << "[analysis_session] FAIL: " << message << '\n';
  return false;
}

class FakeBackend final : public IMemoryBackend {
public:
  FakeBackend() : memory(kStorageSize, 0) {}

  bool Connect(const ConnectRequest &request, BackendSnapshot &snapshot,
               std::string &error) override {
    ++connectCalls;
    error.clear();
    connected = true;
    snapshot.pid = 1337;
    snapshot.targetProcess = request.targetProcess;
    snapshot.runtime = "il2cpp";
    snapshot.modules = {ModuleSnapshot{"UnityPlayer.dll", "UnityPlayer.dll",
                                       kBase, 0x2000u, false}};
    return true;
  }

  void Disconnect() noexcept override {
    ++disconnectCalls;
    connected = false;
  }

  bool IsConnected() const noexcept override { return connected; }

  bool Read(std::uintptr_t address, void *buffer, std::size_t size,
            bool /*noCache*/, std::string &error) override {
    ++readCalls;
    if (!ValidRange(address, size)) {
      error = "fake read outside storage";
      return false;
    }
    std::memcpy(buffer, memory.data() + (address - kBase), size);
    error.clear();
    return true;
  }

  bool Write(std::uintptr_t address, const void *buffer, std::size_t size,
             std::string &error) override {
    ++writeCalls;
    if (!ValidRange(address, size)) {
      error = "fake write outside storage";
      return false;
    }
    std::memcpy(memory.data() + (address - kBase), buffer, size);
    error.clear();
    return true;
  }

  std::vector<ModuleSnapshot> Modules() const override {
    return {ModuleSnapshot{"UnityPlayer.dll", "UnityPlayer.dll", kBase,
                           0x2000u, false}};
  }

  std::vector<std::uint8_t> memory;
  std::size_t connectCalls = 0;
  std::size_t disconnectCalls = 0;
  std::size_t readCalls = 0;
  std::size_t writeCalls = 0;
  bool connected = false;

private:
  static bool ValidRange(std::uintptr_t address, std::size_t size) noexcept {
    if (address < kBase || size > kStorageSize) {
      return false;
    }
    return address - kBase <= kStorageSize - size;
  }
};

ConnectRequest Request() {
  ConnectRequest request;
  request.targetProcess = "FakeGame.exe";
  return request;
}

bool TestDisabledPolicy() {
  auto backend = std::make_shared<FakeBackend>();
  unityexplorer::analysis::AnalysisSession session(backend);
  const auto connected = session.Connect(Request());
  bool ok = Expect(connected.ok, "disabled policy should connect") &&
            Expect(session.Status().writePolicy == "disabled",
                   "disabled policy should be reported") &&
            Expect(!session.Status().writeEnabled,
                   "disabled policy should not enable writes");

  unityexplorer::analysis::WriteRequest write;
  write.expectedGeneration = connected.generation;
  write.address = kBase + 0x10u;
  write.data = {0xAAu};
  const auto result = session.Write(write);
  ok &= Expect(!result.ok && result.code == "permission_denied",
               "disabled policy should reject writes");
  ok &= Expect(backend->writeCalls == 0,
               "disabled policy should not call backend write");
  return ok;
}

bool TestDenyAllPolicy() {
  auto backend = std::make_shared<FakeBackend>();
  unityexplorer::analysis::AnalysisSessionOptions options;
  options.enableWrites = true;
  unityexplorer::analysis::AnalysisSession session(backend, options);
  const auto connected = session.Connect(Request());
  bool ok = Expect(connected.ok, "deny-all policy should connect") &&
            Expect(session.Status().writePolicy == "deny_all",
                   "empty allowlist should be deny_all");

  unityexplorer::analysis::WriteRequest write;
  write.expectedGeneration = connected.generation;
  write.address = kBase + 0x10u;
  write.data = {0xAAu};
  const auto result = session.Write(write);
  ok &= Expect(!result.ok && result.code == "permission_denied",
               "deny-all policy should reject writes");
  return ok;
}

bool TestAllowlistAndGuards() {
  auto backend = std::make_shared<FakeBackend>();
  backend->memory[0x20] = 0x10u;
  backend->memory[0x21] = 0x20u;
  backend->memory[0x22] = 0x30u;

  unityexplorer::analysis::AnalysisSessionOptions options;
  options.enableWrites = true;
  options.writeRanges = {{kBase, 0x2000u}};
  unityexplorer::analysis::AnalysisSession session(backend, options);
  const auto connected = session.Connect(Request());
  bool ok = Expect(connected.ok, "allowlist policy should connect");

  unityexplorer::analysis::WriteRequest write;
  write.expectedGeneration = connected.generation;
  write.address = kBase + 0x20u;
  write.data = {0xAAu, 0xBBu, 0xCCu};
  write.expectedBefore = {0x10u, 0x20u, 0x30u};
  const auto success = session.Write(write);
  ok &= Expect(success.ok, "matching expectedBefore write should succeed");
  ok &= Expect(success.before == write.expectedBefore,
               "write result should include bytes before write");
  ok &= Expect(success.readback == write.data && success.readbackMatches,
               "write result should verify readback");
  ok &= Expect(backend->writeCalls == 1,
               "successful write should call backend once");

  unityexplorer::analysis::WriteRequest outside = write;
  outside.address = kBase + 0x2000u;
  outside.expectedBefore = {0x00u, 0x00u, 0x00u};
  const auto outsideResult = session.Write(outside);
  ok &= Expect(!outsideResult.ok && outsideResult.code == "permission_denied",
               "write outside allowlist should be rejected");
  ok &= Expect(backend->writeCalls == 1,
               "allowlist rejection should not call backend write");

  unityexplorer::analysis::WriteRequest crossPage = write;
  crossPage.address = kBase + 0x0FFFu;
  crossPage.data = {0x01u, 0x02u};
  crossPage.expectedBefore = {0x00u, 0x00u};
  const auto crossPageResult = session.Write(crossPage);
  ok &= Expect(!crossPageResult.ok && crossPageResult.code == "invalid_argument",
               "cross-page write should be rejected");

  unityexplorer::analysis::WriteRequest mismatch = write;
  mismatch.address = kBase + 0x30u;
  mismatch.data = {0x44u, 0x55u, 0x66u};
  mismatch.expectedBefore = {0xFFu, 0xFFu, 0xFFu};
  const auto mismatchResult = session.Write(mismatch);
  ok &= Expect(!mismatchResult.ok && mismatchResult.code == "compare_failed",
               "mismatched expectedBefore should be rejected");
  ok &= Expect(backend->writeCalls == 1,
               "compare failure should not call backend write");

  unityexplorer::analysis::WriteRequest masked = write;
  masked.address = kBase + 0x40u;
  masked.data = {0xA4u, 0xB5u};
  masked.expectedBefore = {0x1Fu, 0x2Fu};
  masked.mask = {0xF0u, 0xF0u};
  backend->memory[0x40] = 0x10u;
  backend->memory[0x41] = 0x20u;
  const auto maskedResult = session.Write(masked);
  ok &= Expect(maskedResult.ok,
               "masked expectedBefore comparison should ignore unmasked bits");

  unityexplorer::analysis::WriteRequest badMask = write;
  badMask.address = kBase + 0x50u;
  badMask.data = {0x01u, 0x02u};
  badMask.expectedBefore = {0x00u, 0x00u};
  badMask.mask = {0xFFu};
  const auto badMaskResult = session.Write(badMask);
  ok &= Expect(!badMaskResult.ok && badMaskResult.code == "invalid_argument",
               "mask length mismatch should be rejected");

  unityexplorer::analysis::WriteRequest invalidAddress = write;
  invalidAddress.address = 0;
  const auto invalidAddressResult = session.Write(invalidAddress);
  ok &= Expect(!invalidAddressResult.ok &&
                   invalidAddressResult.code == "invalid_argument",
               "zero write address should be rejected");

  if constexpr (sizeof(std::uintptr_t) > 4) {
    unityexplorer::analysis::WriteRequest nonCanonical = write;
    nonCanonical.address = static_cast<std::uintptr_t>(0x0000800000000000ULL);
    const auto nonCanonicalResult = session.Write(nonCanonical);
    ok &= Expect(!nonCanonicalResult.ok &&
                     nonCanonicalResult.code == "invalid_argument",
                 "non-canonical write address should be rejected");

    unityexplorer::analysis::WriteRequest boundary = write;
    boundary.address = static_cast<std::uintptr_t>(0x00007FFFFFFFFFFFULL);
    boundary.data = {0x01u, 0x02u};
    boundary.expectedBefore = {0x00u, 0x00u};
    const auto boundaryResult = session.Write(boundary);
    ok &= Expect(!boundaryResult.ok && boundaryResult.code == "invalid_argument",
                 "write range crossing the canonical boundary should be rejected");
  }

  unityexplorer::analysis::WriteRequest missingBefore = write;
  missingBefore.expectedBefore.clear();
  const auto missingBeforeResult = session.Write(missingBefore);
  ok &= Expect(!missingBeforeResult.ok &&
                   missingBeforeResult.code == "invalid_argument",
               "missing expectedBefore should be rejected by default");

  const auto disconnected = session.Disconnect(connected.generation);
  ok &= Expect(disconnected.ok, "disconnect with current generation should succeed");
  const auto staleRead = session.Read(connected.generation, kBase, 1);
  ok &= Expect(!staleRead.ok && staleRead.code == "stale_generation",
               "old generation read should be rejected");
  const auto staleWrite = session.Write(write);
  ok &= Expect(!staleWrite.ok && staleWrite.code == "stale_generation",
               "old generation write should be rejected");
  return ok;
}

bool TestReadAddressGuards() {
  auto backend = std::make_shared<FakeBackend>();
  unityexplorer::analysis::AnalysisSession session(backend);
  const auto connected = session.Connect(Request());
  bool ok = Expect(connected.ok, "read guard session should connect");

  const auto valid = session.Read(connected.generation, kBase + 0x10u, 4);
  ok &= Expect(valid.ok && valid.actual == 4,
               "valid read should pass address guards");

  const auto zero = session.Read(connected.generation, 0, 1);
  ok &= Expect(!zero.ok && zero.code == "invalid_argument",
               "zero read address should be rejected");

  if constexpr (sizeof(std::uintptr_t) > 4) {
    const auto nonCanonical = session.Read(
        connected.generation,
        static_cast<std::uintptr_t>(0x0000800000000000ULL), 1);
    ok &= Expect(!nonCanonical.ok && nonCanonical.code == "invalid_argument",
                 "non-canonical read address should be rejected");

    const auto boundary = session.Read(
        connected.generation,
        static_cast<std::uintptr_t>(0x00007FFFFFFFFFFFULL), 2);
    ok &= Expect(!boundary.ok && boundary.code == "invalid_argument",
                 "read range crossing the canonical boundary should be rejected");
  }

  const auto overflow = session.Read(
      connected.generation, std::numeric_limits<std::uintptr_t>::max(),
      std::numeric_limits<std::size_t>::max());
  ok &= Expect(!overflow.ok && overflow.code == "invalid_argument",
               "oversized read should be rejected before backend access");
  ok &= Expect(backend->readCalls == 1,
               "invalid reads should not call backend after the valid read");
  return ok;
}

bool TestAllowlistDoesNotJoinAdjacentRanges() {
  auto backend = std::make_shared<FakeBackend>();
  unityexplorer::analysis::AnalysisSessionOptions options;
  options.enableWrites = true;
  options.writeRanges = {{kBase, 0x10u}, {kBase + 0x10u, 0x10u}};
  unityexplorer::analysis::AnalysisSession session(backend, options);
  const auto connected = session.Connect(Request());
  bool ok = Expect(connected.ok, "adjacent range session should connect");

  unityexplorer::analysis::WriteRequest request;
  request.expectedGeneration = connected.generation;
  request.address = kBase + 0x0Cu;
  request.data = {0x01u, 0x02u, 0x03u, 0x04u, 0x05u, 0x06u};
  request.expectedBefore = std::vector<std::uint8_t>(request.data.size(), 0);
  const auto result = session.Write(request);
  ok &= Expect(!result.ok && result.code == "permission_denied",
               "a write spanning adjacent allowlist entries should be denied");
  ok &= Expect(backend->writeCalls == 0,
               "adjacent-range rejection should not call backend write");
  return ok;
}

bool TestGenerationReplacementInvalidatesOldRequests() {
  auto backend = std::make_shared<FakeBackend>();
  unityexplorer::analysis::AnalysisSession session(backend);
  const auto first = session.Connect(Request());
  bool ok = Expect(first.ok, "initial generation should connect");

  ConnectRequest replacement = Request();
  replacement.targetProcess = "Replacement.exe";
  replacement.replace = true;
  const auto second = session.Connect(replacement);
  ok &= Expect(second.ok && second.generation > first.generation,
               "replacement connect should publish a new generation");

  const auto staleRead = session.Read(first.generation, kBase, 1);
  ok &= Expect(!staleRead.ok && staleRead.code == "stale_generation",
               "replacement should reject reads from the old generation");
  const auto currentRead = session.Read(second.generation, kBase, 1);
  ok &= Expect(currentRead.ok,
               "replacement should keep the new generation usable");

  const auto staleDisconnect = session.Disconnect(first.generation);
  ok &= Expect(!staleDisconnect.ok && staleDisconnect.code == "stale_generation",
               "disconnect with an old generation should be rejected");
  ok &= Expect(session.Status().state ==
                   unityexplorer::analysis::SessionState::Ready,
               "stale disconnect must not tear down the current session");
  return ok;
}

bool TestOptionalExpectedBeforeStillValidatesSuppliedLength() {
  auto backend = std::make_shared<FakeBackend>();
  unityexplorer::analysis::AnalysisSessionOptions options;
  options.enableWrites = true;
  options.requireExpectedBefore = false;
  options.writeRanges = {{kBase, 0x1000u}};
  unityexplorer::analysis::AnalysisSession session(backend, options);
  const auto connected = session.Connect(Request());
  bool ok = Expect(connected.ok, "optional expectedBefore session should connect");

  unityexplorer::analysis::WriteRequest request;
  request.expectedGeneration = connected.generation;
  request.address = kBase + 0x60u;
  request.data = {0xAAu, 0xBBu};
  request.expectedBefore = {0x00u};
  const auto result = session.Write(request);
  ok &= Expect(!result.ok && result.code == "invalid_argument",
               "a supplied short expectedBefore must be rejected");
  ok &= Expect(backend->writeCalls == 0,
               "short expectedBefore should not call backend write");
  return ok;
}

bool TestSessionExecutorQueueAndShutdown() {
  SessionExecutor executor(1);
  std::promise<void> startedPromise;
  std::shared_future<void> started = startedPromise.get_future().share();
  std::promise<void> releasePromise;
  std::shared_future<void> release = releasePromise.get_future().share();

  auto first = executor.Submit([&startedPromise, release] {
    startedPromise.set_value();
    release.wait();
    return 7;
  });
  started.wait();

  auto queued = executor.Submit([] { return 9; });
  bool ok = Expect(executor.Active(), "executor should report an active task") &&
            Expect(executor.Pending() == 1,
                   "executor should report one queued task");

  bool busy = false;
  try {
    (void)executor.Submit([] { return 11; });
  } catch (const ExecutorBusy &) {
    busy = true;
  }
  ok &= Expect(busy, "executor should reject submissions at queue capacity");

  releasePromise.set_value();
  ok &= Expect(first.get() == 7, "active executor task should complete");
  ok &= Expect(queued.get() == 9, "queued executor task should run in order");

  executor.Stop();
  bool stopped = false;
  try {
    (void)executor.Submit([] { return 13; });
  } catch (const ExecutorStopped &) {
    stopped = true;
  }
  ok &= Expect(stopped, "executor should reject submissions after Stop");
  return ok;
}

} // namespace

int main() {
  bool ok = true;
  ok &= TestDisabledPolicy();
  ok &= TestDenyAllPolicy();
  ok &= TestAllowlistAndGuards();
  ok &= TestReadAddressGuards();
  ok &= TestAllowlistDoesNotJoinAdjacentRanges();
  ok &= TestGenerationReplacementInvalidatesOldRequests();
  ok &= TestOptionalExpectedBeforeStillValidatesSuppliedLength();
  ok &= TestSessionExecutorQueueAndShutdown();
  if (ok) {
    std::cout << "[analysis_session] PASS\n";
  }
  return ok ? 0 : 1;
}
