#pragma once

#include <algorithm>

#include <chrono>

#include <cctype>

#include <cstdint>

#include <cstring>

#include <string>

#include <unordered_map>

#include <utility>

#include <vector>

#include "header_parser.hpp"

#include "pe.hpp"

#include "../init/context.hpp" // For er2::Log

namespace er2

{

struct FoundMetadata

{

  std::uintptr_t ptrAddr = 0;

  std::uintptr_t metaBase = 0;

  std::uint32_t maxEnd = 0;

  int score = 0;
};

inline bool IsSectionWanted(const char *name)

{

  if (!name || !name[0])

  {

    return false;
  }

  std::string s(name);

  for (auto &c : s)

  {

    c = (char)std::tolower((unsigned char)c);
  }

  return s == ".data" || s == ".rdata" || s == ".pdata" || s == ".tls" ||
         s == ".reloc" || s.find("vmp") != std::string::npos ||
         s.find("data") != std::string::npos ||
         s.find("il2cpp") != std::string::npos; // [Fix] Scan il2cpp sections
                                                // for modded games like Naraka
}

inline FoundMetadata FindMetadataPointerByScore(

    const IMemoryAccessor &mem,

    std::uintptr_t moduleBase,

    std::size_t chunkSize,

    std::size_t maxPages,

    double maxSeconds,

    bool strictVersion,

    std::uint32_t requiredVersion)

{

  FoundMetadata best;

  auto start = std::chrono::steady_clock::now();

  std::vector<ModuleSection> sections;

  std::uint32_t sizeOfImage = 0;

  if (!ReadModuleSections(mem, moduleBase, sizeOfImage, sections))

  {

    return best;
  }

  std::vector<ModuleSection> scanSections;

  scanSections.reserve(sections.size());

  // [Debug] Log all sections for diagnostics
  er2::Log("[PointerScan] Module has " + std::to_string(sections.size()) +
           " sections:");
  for (const auto &s : sections) {
    std::string info =
        "  [" + std::string(s.name) + "] RVA=0x" + std::to_string(s.rva) +
        " Size=0x" + std::to_string(s.size) +
        (IsSectionWanted(s.name.c_str()) ? " (WILL SCAN)" : " (SKIP)");
    er2::Log(info);
  }

  for (const auto &s : sections)

  {

    if (!IsSectionWanted(s.name.c_str()))

    {

      continue;
    }

    ModuleSection ms = s;

    if (ms.rva >= sizeOfImage)

    {

      continue;
    }

    std::uint32_t maxSize = sizeOfImage - ms.rva;

    if (ms.size == 0 || ms.size > maxSize)

    {

      ms.size = maxSize;
    }

    if (ms.size == 0)

    {

      continue;
    }

    scanSections.push_back(ms);
  }

  er2::Log("[PointerScan] Will scan " + std::to_string(scanSections.size()) +
           " sections.");

  if (scanSections.empty())

  {
    er2::Log("[PointerScan] ERROR: No valid sections to scan!");
    return best;
  }

  std::unordered_map<std::uintptr_t,
                     std::vector<std::pair<std::uintptr_t, std::uintptr_t>>>
      pageMap;

  pageMap.reserve(maxPages);

  // [Debug] Statistics counters
  std::uint64_t totalCandidates = 0;
  std::uint64_t totalPagesValidated = 0;
  std::uint64_t chunksReadFailed = 0;
  std::uint64_t chunksReadSuccess = 0;

  auto validatePages = [&]()

  {
    if (pageMap.empty())

    {

      return;
    }

    std::vector<std::uint8_t> pageBuf;

    pageBuf.resize(0x1000);

    for (const auto &it : pageMap)

    {

      const std::uintptr_t page = it.first;

      if (!mem.Read(page, pageBuf.data(), pageBuf.size()))

      {

        continue;
      }

      for (const auto &cand : it.second)

      {

        const std::uintptr_t ptrAddr = cand.first;

        const std::uintptr_t ptr = cand.second;

        const std::uintptr_t delta = ptr - page;

        if ((std::size_t)delta + 0x120u > pageBuf.size())

        {

          continue;
        }

        const MetadataScoreResult sr = ScoreMetadataHeader(
            pageBuf.data(), pageBuf.size(), (std::size_t)delta, strictVersion,
            requiredVersion);

        if (sr.score <= 0)

        {

          continue;
        }

        if (sr.score > best.score)

        {

          best.score = sr.score;

          best.maxEnd = sr.maxEnd;

          best.ptrAddr = ptrAddr;

          best.metaBase = ptr;
        }
      }

      if (best.metaBase)

      {

        std::uint8_t tail = 0;

        const std::uintptr_t endMinus1 =
            best.metaBase + (std::uintptr_t)best.maxEnd - 1;

        if (mem.Read(endMinus1, &tail, 1))

        {

          return;
        }
      }
    }
  };

  if (chunkSize < 0x1000)

  {

    return best;
  }

  std::vector<std::uint8_t> chunkBuf;

  chunkBuf.resize(chunkSize);

  for (const auto &sec : scanSections)

  {
    er2::Log("[PointerScan] Scanning section: " + std::string(sec.name) +
             " (Size: 0x" + std::to_string(sec.size) + ")");

    const std::uintptr_t secStart = moduleBase + (std::uintptr_t)sec.rva;

    std::uint32_t remaining = sec.size;

    std::uint32_t offset = 0;

    while (remaining > 0)

    {

      const double elapsed = std::chrono::duration<double>(
                                 std::chrono::steady_clock::now() - start)
                                 .count();

      if (maxSeconds > 0.0 && elapsed > maxSeconds)

      {

        break;
      }

      const std::size_t toRead = remaining > (std::uint32_t)chunkBuf.size()
                                     ? chunkBuf.size()
                                     : remaining;

      const std::uintptr_t chunkAddr = secStart + offset;

      if (!mem.Read(chunkAddr, chunkBuf.data(), toRead))

      {
        // [DMA Fix] Page-by-page fallback for large game modules
        // When a full chunk read fails (common with DMA paging issues),
        // try reading each 4KB page individually. This maximizes the chance
        // of finding the pointer slot even if some pages are swapped out.
        constexpr std::size_t kPageSize = 0x1000; // 4KB
        bool anyPageRead = false;

        for (std::size_t pageOff = 0; pageOff < toRead; pageOff += kPageSize) {
          std::size_t pageToRead = (std::min)(kPageSize, toRead - pageOff);
          if (mem.Read(chunkAddr + pageOff, chunkBuf.data() + pageOff,
                       pageToRead)) {
            anyPageRead = true;
          } else {
            // Zero-fill unreadable page - won't produce valid pointer
            // candidates
            std::memset(chunkBuf.data() + pageOff, 0, pageToRead);
          }
        }

        // If no pages were readable at all, skip this chunk entirely
        if (!anyPageRead) {
          chunksReadFailed++;
          offset += (std::uint32_t)toRead;
          remaining -= (std::uint32_t)toRead;
          continue;
        }
        // Otherwise, fall through to process the partially-read chunk
        chunksReadSuccess++;
      } else {
        chunksReadSuccess++;
      }

      const std::size_t limit = toRead >= 8 ? (toRead - 8) : 0;

      for (std::size_t off = 0; off <= limit; off += 8)

      {

        std::uint64_t ptr = 0;

        std::memcpy(&ptr, chunkBuf.data() + off, sizeof(ptr));

        if (ptr == 0)

        {

          continue;
        }

        if (ptr < 0x0000000000010000ull || ptr > 0x00007FFFFFFFFFFFull)

        {

          continue;
        }

        if ((ptr & 7ull) != 0ull)

        {

          continue;
        }

        const std::uintptr_t page = (std::uintptr_t)ptr & ~0xFFFull;

        const std::uintptr_t ptrAddr = chunkAddr + (std::uintptr_t)off;

        pageMap[page].push_back({ptrAddr, (std::uintptr_t)ptr});
        totalCandidates++;

        if (pageMap.size() >= maxPages)

        {

          validatePages();

          pageMap.clear();

          if (best.metaBase)

          {

            return best;
          }
        }
      }

      validatePages();

      pageMap.clear();

      if (best.metaBase)

      {

        return best;
      }

      offset += (std::uint32_t)toRead;

      remaining -= (std::uint32_t)toRead;
    }
  }

  if (!best.metaBase)

  {

    validatePages();

    pageMap.clear();
  }

  // [Debug] Final scan summary
  er2::Log("[PointerScan] ========== SCAN COMPLETE ==========");
  er2::Log("[PointerScan] Chunks read success: " +
           std::to_string(chunksReadSuccess));
  er2::Log("[PointerScan] Chunks read failed: " +
           std::to_string(chunksReadFailed));
  er2::Log("[PointerScan] Total candidates found: " +
           std::to_string(totalCandidates));
  if (best.metaBase) {
    char buf[256];
    std::snprintf(
        buf, sizeof(buf),
        "[PointerScan] RESULT: MetaBase=0x%llX Score=%d PtrAddr=0x%llX",
        (unsigned long long)best.metaBase, best.score,
        (unsigned long long)best.ptrAddr);
    er2::Log(buf);
  } else {
    er2::Log("[PointerScan] RESULT: No valid metadata found!");
  }
  er2::Log("[PointerScan] =================================");

  return best;
}

} // namespace er2
