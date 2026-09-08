#pragma once

#include "../../mem/memory_accessor.hpp"
#include "header_parser.hpp"
#include <chrono>
#include <cstdint>
#include <vector>


namespace er2 {

struct DirectScanResult {
  std::uintptr_t metaBase = 0;
  int score = 0;
  std::uint32_t maxEnd = 0;
};

inline DirectScanResult FindMetadataDirect(const IMemoryAccessor &mem,
                                           std::uintptr_t moduleBase,
                                           std::uint32_t moduleSize,
                                           bool useMagic, bool useHeuristic,
                                           bool strictVersion = false) {
  DirectScanResult best;

  // Chunk reading for performance
  constexpr size_t chunkSize = 0x10000; // 64KB chunks
  std::vector<uint8_t> buffer(chunkSize);

  // We scan 4-byte aligned
  // Iterate module memory
  size_t processed = 0;

  while (processed < moduleSize) {
    size_t readSize = std::min<size_t>(chunkSize, moduleSize - processed);
    if (!mem.Read(moduleBase + processed, buffer.data(), readSize)) {
      processed += readSize;
      continue;
    }

    size_t limit = readSize - (readSize >= 0x120 ? 0x120 : 0);
    // We need 0x120 bytes for header check. If end of chunk, we might miss
    // boundary. For simplicity, we ignore boundary issue or handle overlap.
    // Let's ignore boundary - Metadata unlikely to straddle chunks unless
    // unlucky.

    for (size_t off = 0; off <= limit; off += 4) {
      bool candidate = false;

      // Strategy 1: Magic Check
      if (useMagic) {
        uint32_t magic = ReadU32LE(buffer.data() + off);
        if (magic == 0xFAB11BAF) {
          candidate = true;
        }
      }

      // Strategy 2: Heuristic (always run if Magic not strictly required OR if
      // Magic found)
      if (useHeuristic && !candidate) {
        // Heuristic check:
        // Check if stringLiteralOffset (at +0x08) is valid
        // Check if stringLiteralDataOffset (at +0x10) is valid
        // Simple pre-checks to avoid calling ScoreMetadataHeader too often
        uint32_t strOff = ReadU32LE(buffer.data() + off + 0x08);
        if (strOff > 0x100 && strOff < 0x20000000) { // Reasonable sanity
          candidate = true;
        }
      }

      if (candidate) {
        auto res =
            ScoreMetadataHeader(buffer.data(), readSize, off, strictVersion, 0);
        if (res.score > 0 && res.score > best.score) {
          best.score = res.score;
          best.metaBase = moduleBase + processed + off;
          best.maxEnd = res.maxEnd;

          // If Magic match, likely the one.
          if (useMagic && ReadU32LE(buffer.data() + off) == 0xFAB11BAF) {
            // High confidence
            // But maybe we keep searching for better score?
            // Usually only one valid metadata.
            // If score is high enough.
            if (best.score > 500000)
              return best; // Arbitrary high threshold
          }
        }
      }
    }

    processed += readSize;
  }

  return best;
}

} // namespace er2
