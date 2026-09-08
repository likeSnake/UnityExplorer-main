#pragma once

#include "runtime_class_probe.hpp"

#include <array>
#include <cstdint>
#include <string>
#include <vector>

namespace er2 {

struct NativeChainProbeCandidate {
  std::uint32_t managedOffset = 0;
  std::uintptr_t managed = 0;
  std::uintptr_t nativeBackPointer = 0;
  std::uintptr_t klass = 0;
  RuntimeClassProbeResult runtimeClass;
  int score = 0;
  bool valid = false;
  bool managedReadable = false;
  bool hasNativeBackPointer = false;
  bool klassReadable = false;
  std::string failureReason;
};

struct NativeChainProbeResult {
  bool valid = false;
  std::uintptr_t native = 0;
  std::uint32_t suggestedManagedOffset = 0;
  int bestIndex = -1;
  std::vector<NativeChainProbeCandidate> candidates;
  std::string failureReason;

  const NativeChainProbeCandidate &best() const {
    return candidates[static_cast<std::size_t>(bestIndex)];
  }
};

inline const std::array<std::uint32_t, 8> &DefaultManagedPointerOffsets() {
  static constexpr std::array<std::uint32_t, 8> kOffsets = {
      0x20u, 0x28u, 0x30u, 0x38u, 0x40u, 0x48u, 0x50u, 0x58u};
  return kOffsets;
}

inline NativeChainProbeCandidate ProbeNativeObjectChainAtOffset(
    const IMemoryAccessor &mem, std::uintptr_t native,
    const Il2CppLayoutProfile &profile, std::uint32_t managedOffset) {
  NativeChainProbeCandidate candidate;
  candidate.managedOffset = managedOffset;

  if (!ReadPtr(mem, native + managedOffset, candidate.managed) ||
      !IsCanonicalUserPtr(candidate.managed)) {
    candidate.failureReason = "managed pointer is unreadable or invalid";
    return candidate;
  }
  candidate.managedReadable = true;
  candidate.score += 10;

  if (ReadPtr(mem, candidate.managed + 0x10u, candidate.nativeBackPointer) &&
      candidate.nativeBackPointer == native) {
    candidate.hasNativeBackPointer = true;
    candidate.score += 40;
  }

  if (!ReadPtr(mem, candidate.managed, candidate.klass) ||
      !IsCanonicalUserPtr(candidate.klass)) {
    candidate.failureReason = "klass pointer is unreadable or invalid";
    return candidate;
  }
  candidate.klassReadable = true;
  candidate.score += 20;

  if (ProbeRuntimeClass(mem, candidate.klass, profile,
                        candidate.runtimeClass)) {
    candidate.score += 50;
    candidate.valid = true;
    return candidate;
  }

  candidate.failureReason = candidate.runtimeClass.failureReason;
  return candidate;
}

inline bool ProbeNativeObjectChain(const IMemoryAccessor &mem,
                                   std::uintptr_t native,
                                   const Il2CppLayoutProfile &profile,
                                   NativeChainProbeResult &out) {
  out = NativeChainProbeResult{};
  out.native = native;

  if (!IsCanonicalUserPtr(native)) {
    out.failureReason = "native pointer is not a canonical user pointer";
    return false;
  }

  const auto &offsets = DefaultManagedPointerOffsets();
  out.candidates.reserve(offsets.size());

  int bestScore = -1;
  for (std::uint32_t offset : offsets) {
    NativeChainProbeCandidate candidate =
        ProbeNativeObjectChainAtOffset(mem, native, profile, offset);

    const int rankScore = candidate.score + (candidate.valid ? 1000 : 0);
    if (rankScore > bestScore) {
      bestScore = rankScore;
      out.bestIndex = static_cast<int>(out.candidates.size());
    }

    out.candidates.push_back(std::move(candidate));
  }

  if (out.bestIndex < 0 ||
      out.bestIndex >= static_cast<int>(out.candidates.size())) {
    out.failureReason = "no managed offset candidates were evaluated";
    return false;
  }

  const NativeChainProbeCandidate &best = out.best();
  out.suggestedManagedOffset = best.managedOffset;
  out.valid = best.valid;
  if (!out.valid) {
    out.failureReason = best.failureReason.empty()
                            ? "no candidate reached a valid runtime class"
                            : best.failureReason;
  }

  return out.valid;
}

} // namespace er2
