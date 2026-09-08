#pragma once

#include <cstdint>
#include <vector>

#include "../init/context.hpp"
#include "header_parser.hpp"
#include "hint_json.hpp"
#include "scanner_direct.hpp"
#include "scanner_pointer.hpp"

namespace er2 {

// Forward declaration if needed, but we define order correctly.

inline FoundMetadata FindMetadataByScore(const IMemoryAccessor &mem,
                                         std::uintptr_t moduleBase,
                                         std::size_t scanChunkSize,
                                         std::size_t maxPages,
                                         double maxSeconds, bool strictVersion,
                                         std::uint32_t requiredVersion) {
  // 1. Standard Pointer Scan
  FoundMetadata best =
      FindMetadataPointerByScore(mem, moduleBase, scanChunkSize, maxPages,
                                 maxSeconds, strictVersion, requiredVersion);

  if (best.metaBase) {
    er2::Log("[INFO] FindMetadataByScore: SUCCESS (PointerScan). MetaBase: 0x" +
             detail_metadata_hint::HexU64NoPad(best.metaBase));
    return best;
  }

  // 2. Direct Scan (Magic + Heuristic)
  std::vector<ModuleSection> sections;
  std::uint32_t sizeOfImage = 0;
  if (ReadModuleSections(mem, moduleBase, sizeOfImage, sections) &&
      sizeOfImage > 0) {
    // Try Magic
    auto direct = FindMetadataDirect(mem, moduleBase, sizeOfImage, true, false,
                                     strictVersion);
    if (direct.metaBase) {
      best.metaBase = direct.metaBase;
      best.score = direct.score;
      best.maxEnd = direct.maxEnd;
      return best;
    }

    // Try Heuristic
    direct = FindMetadataDirect(mem, moduleBase, sizeOfImage, false, true,
                                strictVersion);
    if (direct.metaBase) {
      best.metaBase = direct.metaBase;
      best.score = direct.score;
      best.maxEnd = direct.maxEnd;
      return best;
    }
  }

  return best;
}

inline bool IsValidMetadataMagicAndVersion(const IMemoryAccessor &mem,
                                           std::uintptr_t metaBase,
                                           std::uint32_t *outMagic = nullptr,
                                           std::uint32_t *outVersion = nullptr) {
  if (outMagic) {
    *outMagic = 0;
  }
  if (outVersion) {
    *outVersion = 0;
  }
  if (!metaBase) {
    return false;
  }

  std::uint32_t magic = 0;
  std::uint32_t version = 0;
  if (!ReadValue(mem, metaBase + 0x00u, magic) ||
      !ReadValue(mem, metaBase + 0x04u, version)) {
    return false;
  }

  if (outMagic) {
    *outMagic = magic;
  }
  if (outVersion) {
    *outVersion = version;
  }

  return magic == 0xFAB11BAFu && version >= 10u && version <= 100u;
}

inline FoundMetadata FindMetadataPointerStrict(const IMemoryAccessor &mem,
                                               std::uintptr_t moduleBase,
                                               std::size_t scanChunkSize,
                                               std::size_t maxPages,
                                               double maxSeconds,
                                               std::uint32_t requiredVersion) {
  FoundMetadata found =
      FindMetadataPointerByScore(mem, moduleBase, scanChunkSize, maxPages,
                                 maxSeconds, true, requiredVersion);
  if (!found.metaBase) {
    return FoundMetadata{};
  }

  std::uint32_t magic = 0;
  std::uint32_t version = 0;
  if (!IsValidMetadataMagicAndVersion(mem, found.metaBase, &magic, &version)) {
    er2::Log("[ERROR] FindMetadataPointerStrict: rejected candidate magic=0x" +
             detail_metadata_hint::HexU64NoPad(magic) + " version=" +
             std::to_string(version));
    return FoundMetadata{};
  }

  return found;
}

inline bool TryCalcMetadataTotalSizeByScore(
    const IMemoryAccessor &mem, std::uintptr_t moduleBase,
    std::size_t scanChunkSize, std::size_t maxPages, double maxSeconds,
    bool strictVersion, std::uint32_t requiredVersion, FoundMetadata &outFound,
    std::uint32_t &outTotalSize) {
  outFound = FoundMetadata{};
  outTotalSize = 0;

  // Use Robust Search
  const FoundMetadata found =
      FindMetadataByScore(mem, moduleBase, scanChunkSize, maxPages, maxSeconds,
                          strictVersion, requiredVersion);

  if (!found.metaBase) {
    return false;
  }

  std::uint32_t totalSize = 0;
  if (!CalcTotalSizeFromHeader(mem, found.metaBase, totalSize)) {
    return false;
  }

  outFound = found;
  outTotalSize = totalSize;
  return true;
}

inline bool ReadMetadataRegion(const IMemoryAccessor &mem, std::uintptr_t base,
                               std::uint32_t size, std::size_t chunkSize,
                               std::vector<std::uint8_t> &out) {
  out.clear();
  if (!base || size == 0) {
    return false;
  }
  if (chunkSize == 0) {
    return false;
  }

  out.reserve(size);

  std::vector<std::uint8_t> buf;
  buf.resize(chunkSize);

  std::uint32_t remaining = size;
  std::uint32_t offset = 0;

  // [Fix] Fault-tolerant read for DMA paging issues
  // Instead of failing the entire export on a single page fault,
  // we warn, zero-fill, and continue.

  while (remaining > 0) {
    const std::size_t toRead =
        remaining > (std::uint32_t)buf.size() ? buf.size() : remaining;

    if (!mem.Read(base + offset, buf.data(), toRead)) {
      // DMA-Friendly: If a chunk is paged out, we log it and fill with 0s
      // preventing the entire export from failing.
      char logBuf[128];
      // Using snprintf or similar if available, otherwise just use er2::Log
      // Since we are in header-only, we use er2::Log directly if possible or
      // std::string Assuming er2::Log takes std::string based on context.
      std::string msg =
          "[WARN] ReadMetadataRegion: Failed to read chunk at offset " +
          std::to_string(offset) + " (paged out?) - Zero-filling " +
          std::to_string(toRead) + " bytes";
      er2::Log(msg);

      std::memset(buf.data(), 0, toRead);
    }

    out.insert(out.end(), buf.begin(), buf.begin() + toRead);
    offset += (std::uint32_t)toRead;
    remaining -= (std::uint32_t)toRead;
  }

  return true;
}

inline bool
ExportMetadataByScore(const IMemoryAccessor &mem, std::uintptr_t moduleBase,
                      std::size_t scanChunkSize, std::size_t maxPages,
                      double maxSeconds, bool strictVersion,
                      std::uint32_t requiredVersion, std::size_t readChunkSize,
                      std::vector<std::uint8_t> &out) {
  out.clear();

  // Use Robust Search
  const FoundMetadata found =
      FindMetadataByScore(mem, moduleBase, scanChunkSize, maxPages, maxSeconds,
                          strictVersion, requiredVersion);

  if (!found.metaBase) {
    er2::Log("[ERROR] ExportMetadataByScore: Metadata not found by score.");
    return false;
  }
  er2::Log("[INFO] ExportMetadataByScore: Metadata found at 0x" +
           detail_metadata_hint::HexU64NoPad(found.metaBase));

  std::uint32_t magic = 0;
  std::uint32_t version = 0;
  if (!IsValidMetadataMagicAndVersion(mem, found.metaBase, &magic, &version)) {
    er2::Log("[ERROR] ExportMetadataByScore: rejected candidate magic=0x" +
             detail_metadata_hint::HexU64NoPad(magic) + " version=" +
             std::to_string(version));
    return false;
  }

  std::uint32_t totalSize = 0;
  if (!CalcTotalSizeFromHeader(mem, found.metaBase, totalSize)) {
    er2::Log("[ERROR] ExportMetadataByScore: Failed to calc total size.");
    return false;
  }

  return ReadMetadataRegion(mem, found.metaBase, totalSize, readChunkSize, out);
}

} // namespace er2
