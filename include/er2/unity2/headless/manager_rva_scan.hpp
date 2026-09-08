#pragma once

#include "../../core/types.hpp"
#include "../../mem/memory_read.hpp"
#include "../metadata/pe.hpp"

#include <algorithm>
#include <cctype>
#include <cmath>
#include <cstdint>
#include <cstring>
#include <sstream>
#include <string>
#include <vector>

namespace er2 {

enum class ManagerRvaKind {
  CharacterManager,
  EntityManager,
  GlobalTime,
  CharactorSync,
  UserDataManager,
};

struct ManagerRvaScanOptions {
  std::uint32_t maxCandidatesPerKind = 8;
  std::uint32_t maxSectionBytes = 0;
};

struct ManagerRvaCandidate {
  ManagerRvaKind kind = ManagerRvaKind::CharacterManager;
  std::string name;
  std::string section;
  std::string chain;
  std::string evidence;
  std::uint64_t rva = 0;
  std::uintptr_t slotVa = 0;
  std::uintptr_t slotPointer = 0;
  std::uintptr_t staticFields = 0;
  std::uintptr_t resolvedPtr = 0;
  std::uintptr_t auxPtr = 0;
  std::uint32_t count = 0;
  double value = 0.0;
  double observedDelta = 0.0;
  std::uint32_t score = 0;
  bool confirmed = false;
};

struct ManagerRvaScanResult {
  bool ok = false;
  std::string error;
  std::uint32_t imageSize = 0;
  std::uint32_t sectionsVisited = 0;
  std::uint64_t bytesScanned = 0;
  std::uint64_t pointerSlots = 0;
  std::vector<ManagerRvaCandidate> candidates;
};

namespace detail_manager_rva {

struct Seed {
  std::string section;
  std::uint64_t rva = 0;
  std::uintptr_t slotVa = 0;
  std::uintptr_t slotPointer = 0;
  std::uintptr_t rootB8 = 0;
  std::uintptr_t rootC0 = 0;
};

inline const char *KindName(ManagerRvaKind kind) {
  switch (kind) {
  case ManagerRvaKind::CharacterManager:
    return "m_CharacterManager";
  case ManagerRvaKind::EntityManager:
    return "m_EntityManager";
  case ManagerRvaKind::GlobalTime:
    return "m_GlobalTime";
  case ManagerRvaKind::CharactorSync:
    return "m_CharactorSync";
  case ManagerRvaKind::UserDataManager:
    return "m_UserDataManager";
  default:
    return "unknown";
  }
}

inline bool ShouldScanSection(const std::string &name) {
  return name == ".data" || name == ".rdata" || name == "_RDATA" ||
         name == "il2cpp" || name == ".pdata";
}

inline bool ReadCanonicalPtr(const IMemoryAccessor &mem, std::uintptr_t address,
                             std::uintptr_t &out) {
  return ReadPtr(mem, address, out) && IsCanonicalUserPtr(out);
}

inline std::string CountEvidence(const char *label, std::uint32_t count) {
  std::ostringstream ss;
  ss << label << " count=" << count;
  return ss.str();
}

inline bool LooksLikeEntityTypeName(const std::string &text) {
  if (text.size() < 3 || text.size() > 80)
    return false;

  bool hasAlpha = false;
  for (const unsigned char ch : text) {
    if (std::isalpha(ch))
      hasAlpha = true;
    if (!(std::isalnum(ch) || ch == '_' || ch == '.' || ch == '`' ||
          ch == '<' || ch == '>' || ch == '+')) {
      return false;
    }
  }
  return hasAlpha;
}

inline std::uint32_t SampleEntityTypeNames(const IMemoryAccessor &mem,
                                           std::uintptr_t arrayBase,
                                           std::uint32_t count,
                                           std::string &firstType) {
  firstType.clear();
  if (!IsCanonicalUserPtr(arrayBase) || count == 0)
    return 0;

  const std::uint32_t limit = count < 32 ? count : 32;
  std::uint32_t hits = 0;
  for (std::uint32_t i = 0; i < limit; ++i) {
    std::uintptr_t itemAddress = 0, item = 0, typeNamePtr = 0;
    if (!ReadCanonicalPtr(mem, arrayBase + 0x28 + i * 0x10ull, itemAddress))
      continue;
    if (!ReadCanonicalPtr(mem, itemAddress, item))
      continue;
    if (!ReadCanonicalPtr(mem, item + 0x10, typeNamePtr))
      continue;

    std::string typeName;
    if (!ReadCString(mem, typeNamePtr, typeName, 80) ||
        !LooksLikeEntityTypeName(typeName)) {
      continue;
    }

    if (firstType.empty())
      firstType = typeName;
    ++hits;
  }
  return hits;
}

inline ManagerRvaCandidate MakeCandidate(const Seed &seed,
                                         ManagerRvaKind kind) {
  ManagerRvaCandidate c;
  c.kind = kind;
  c.name = KindName(kind);
  c.section = seed.section;
  c.rva = seed.rva;
  c.slotVa = seed.slotVa;
  c.slotPointer = seed.slotPointer;
  c.staticFields = seed.rootB8;
  return c;
}

inline bool ValidateCharacter(const IMemoryAccessor &mem, const Seed &seed,
                              ManagerRvaCandidate &out) {
  if (!IsCanonicalUserPtr(seed.rootB8))
    return false;

  std::uintptr_t mgr = 0, list = 0, array = 0, local = 0, first = 0;
  out = MakeCandidate(seed, ManagerRvaKind::CharacterManager);
  out.chain = "[slot]->+0xB8(static_fields)->+0x8(instance)";
  out.score = 35;

  if (!ReadCanonicalPtr(mem, seed.rootB8 + 0x8, mgr))
    return false;
  out.resolvedPtr = mgr;
  out.score += 20;

  if (ReadCanonicalPtr(mem, mgr + 0x18, local)) {
    out.auxPtr = local;
    out.score += 10;
  }
  if (!ReadCanonicalPtr(mem, mgr + 0x58, list))
    return out.score >= 75;
  out.score += 15;
  if (!ReadCanonicalPtr(mem, list + 0x18, array))
    return out.score >= 75;
  out.auxPtr = array;
  out.score += 15;

  std::uint32_t count = 0;
  if (!ReadValue(mem, array + 0x18, count) || count == 0 || count > 512)
    return out.score >= 75;
  out.count = count;
  out.score += 25;

  if (ReadCanonicalPtr(mem, array + 0x30, first))
    out.score += 10;
  out.evidence = CountEvidence("aliveCharacters", count);
  out.confirmed = out.score >= 100;
  return out.score >= 75;
}

inline bool ValidateEntity(const IMemoryAccessor &mem, const Seed &seed,
                           ManagerRvaCandidate &out) {
  if (!IsCanonicalUserPtr(seed.rootB8))
    return false;

  std::uintptr_t root = 0, entity = 0, array = 0, first = 0;
  out = MakeCandidate(seed, ManagerRvaKind::EntityManager);
  out.chain = "[slot]->+0xB8->+0x8->+0x28(entity list)";
  out.score = 35;

  if (!ReadCanonicalPtr(mem, seed.rootB8 + 0x8, root))
    return false;
  out.score += 15;
  if (!ReadCanonicalPtr(mem, root + 0x28, entity))
    return false;
  out.resolvedPtr = entity;
  out.score += 20;
  if (!ReadCanonicalPtr(mem, entity + 0x18, array))
    return out.score >= 75;
  out.auxPtr = array;
  out.score += 15;

  std::uint32_t count = 0;
  if (!ReadValue(mem, entity + 0x24, count) || count == 0 || count > 200000)
    return out.score >= 75;
  out.count = count;
  out.score += 20;
  if (ReadCanonicalPtr(mem, array + 0x28, first))
    out.score += 10;

  std::string firstType;
  const std::uint32_t typeHits =
      SampleEntityTypeNames(mem, array, count, firstType);
  if (typeHits > 0) {
    out.score += 20;
  }
  std::ostringstream evidence;
  evidence << "entities count=" << count;
  if (typeHits > 0) {
    evidence << "; typeSamples=" << typeHits << "; firstType=" << firstType;
  }
  out.evidence = evidence.str();
  out.confirmed = out.score >= 95;
  return out.score >= 75;
}

inline bool ValidateGlobalTime(const IMemoryAccessor &mem, const Seed &seed,
                               ManagerRvaCandidate &out) {
  if (!IsCanonicalUserPtr(seed.rootB8))
    return false;

  double time = 0.0;
  out = MakeCandidate(seed, ManagerRvaKind::GlobalTime);
  out.chain = "[slot]->+0xB8(GameBaseObject static)->+0x28(double)";
  out.resolvedPtr = seed.rootB8;
  out.score = 55;
  if (!ReadValue(mem, seed.rootB8 + 0x28, time))
    return false;
  if (!std::isfinite(time) || time < 0.001 || time > 1000000000.0)
    return false;

  out.value = time;
  out.score += 30;
  std::ostringstream ss;
  ss << "globalTime=" << time;
  out.evidence = ss.str();
  out.confirmed = false;
  return true;
}

inline bool ValidateCharactorSyncAt(const IMemoryAccessor &mem,
                                    const Seed &seed, std::uint32_t rootOff,
                                    ManagerRvaCandidate &out) {
  if (!IsCanonicalUserPtr(seed.rootB8))
    return false;

  std::uintptr_t sync = 0;
  float shrtt = 0.0f;
  if (!ReadCanonicalPtr(mem, seed.rootB8 + rootOff, sync))
    return false;
  if (!ReadValue(mem, sync + 0x68, shrtt) || !std::isfinite(shrtt) ||
      shrtt < 0.001f || shrtt > 1.0f)
    return false;

  out = MakeCandidate(seed, ManagerRvaKind::CharactorSync);
  out.chain = rootOff == 0 ? "[slot]->+0xB8->+0x0->+0x68(float)"
                           : "[slot]->+0xB8->+0x8->+0x68(float)";
  out.resolvedPtr = sync;
  out.value = shrtt;
  out.score = rootOff == 0 ? 80 : 85;
  if (shrtt >= 0.005f && shrtt <= 0.2f)
    out.score += 20;
  std::ostringstream ss;
  ss << "shrtt=" << shrtt;
  out.evidence = ss.str();
  out.confirmed = true;
  return true;
}

inline bool ValidateUserData(const IMemoryAccessor &mem, const Seed &seed,
                             ManagerRvaCandidate &out) {
  struct Chain {
    std::uintptr_t base = 0;
    std::uint32_t staticOffset = 0;
    const char *name = "";
  };
  const Chain chains[] = {
      {seed.slotPointer, 0xB8, "[slot]->+0xB8(holder)->+0x0(UserData)"},
      {seed.slotPointer, 0xC0, "[slot]->+0xC0(holder)->+0x0(UserData)"},
      {seed.rootB8, 0x0, "[slot]->+0xB8->+0x0(UserData)"},
      {seed.rootC0, 0x0, "[slot]->+0xC0->+0x0(UserData)"},
  };
  for (const Chain &chain : chains) {
    if (!IsCanonicalUserPtr(chain.base))
      continue;

    std::uintptr_t holder = 0, user = 0, battle = 0, skill = 0, dict = 0;
    if (!ReadCanonicalPtr(mem, chain.base + chain.staticOffset, holder))
      continue;
    if (!ReadCanonicalPtr(mem, holder, user))
      continue;

    out = MakeCandidate(seed, ManagerRvaKind::UserDataManager);
    out.chain = chain.name;
    out.staticFields = holder;
    out.resolvedPtr = user;
    out.score = 65;
    if (chain.staticOffset == 0xB8)
      out.score += 10;
    if (ReadCanonicalPtr(mem, user + 0x30, battle)) {
      out.auxPtr = battle;
      out.score += 15;
    }
    if (battle && ReadCanonicalPtr(mem, battle + 0x108, skill) &&
        ReadCanonicalPtr(mem, skill + 0x10, dict)) {
      out.score += 20;
      out.evidence = "battleData->skillData->dict readable";
    }
    out.confirmed = out.score >= 100;
    return out.score >= 75;
  }
  return false;
}

inline void RecordCandidate(ManagerRvaScanResult &result,
                            const ManagerRvaCandidate &candidate,
                            std::uint32_t limit) {
  std::size_t count = 0, worstIndex = result.candidates.size();
  std::uint32_t worstScore = 0xFFFFFFFFu;
  for (std::size_t i = 0; i < result.candidates.size(); ++i) {
    const auto &cur = result.candidates[i];
    if (cur.kind != candidate.kind)
      continue;
    ++count;
    if (cur.score < worstScore) {
      worstScore = cur.score;
      worstIndex = i;
    }
  }
  if (count < limit) {
    result.candidates.push_back(candidate);
  } else if (worstIndex < result.candidates.size() &&
             candidate.score > worstScore) {
    result.candidates[worstIndex] = candidate;
  }
}

inline void ValidateSeed(const IMemoryAccessor &mem, const Seed &seed,
                         ManagerRvaScanResult &result,
                         const ManagerRvaScanOptions &options) {
  ManagerRvaCandidate c;
  if (ValidateCharacter(mem, seed, c))
    RecordCandidate(result, c, options.maxCandidatesPerKind);
  if (ValidateEntity(mem, seed, c))
    RecordCandidate(result, c, options.maxCandidatesPerKind);
  if (ValidateGlobalTime(mem, seed, c))
    RecordCandidate(result, c, options.maxCandidatesPerKind);
  if (ValidateCharactorSyncAt(mem, seed, 0x8, c) ||
      ValidateCharactorSyncAt(mem, seed, 0x0, c))
    RecordCandidate(result, c, options.maxCandidatesPerKind);
  if (ValidateUserData(mem, seed, c))
    RecordCandidate(result, c, options.maxCandidatesPerKind);
}

} // namespace detail_manager_rva

inline void SortManagerRvaCandidates(ManagerRvaScanResult &result) {
  std::sort(result.candidates.begin(), result.candidates.end(),
            [](const ManagerRvaCandidate &a, const ManagerRvaCandidate &b) {
              if (a.kind != b.kind)
                return static_cast<int>(a.kind) < static_cast<int>(b.kind);
              if (a.score != b.score)
                return a.score > b.score;
              return a.rva < b.rva;
            });
}

inline void ApplyManagerRvaRelationshipScores(ManagerRvaScanResult &result) {
  bool hasBestCharacter = false;
  std::uint64_t bestCharacterRva = 0;
  std::uint32_t bestCharacterScore = 0;
  for (const ManagerRvaCandidate &candidate : result.candidates) {
    if (candidate.kind != ManagerRvaKind::CharacterManager)
      continue;
    if (!hasBestCharacter || candidate.score > bestCharacterScore) {
      hasBestCharacter = true;
      bestCharacterRva = candidate.rva;
      bestCharacterScore = candidate.score;
    }
  }

  if (!hasBestCharacter)
    return;

  for (ManagerRvaCandidate &candidate : result.candidates) {
    if (candidate.kind != ManagerRvaKind::CharactorSync)
      continue;

    const std::uint64_t diff =
        candidate.rva > bestCharacterRva ? candidate.rva - bestCharacterRva
                                         : bestCharacterRva - candidate.rva;
    if (diff <= 0x200) {
      candidate.score += 40;
      candidate.evidence += "; near CharacterManager";
    }
  }
}

inline bool ScanGameAssemblyManagerRvas(const IMemoryAccessor &mem,
                                        std::uintptr_t moduleBase,
                                        std::uint32_t moduleSize,
                                        const ManagerRvaScanOptions &options,
                                        ManagerRvaScanResult &out) {
  out = ManagerRvaScanResult{};
  if (!moduleBase || !moduleSize) {
    out.error = "GameAssembly module is not initialized";
    return false;
  }

  std::vector<ModuleSection> sections;
  if (!ReadModuleSections(mem, moduleBase, out.imageSize, sections)) {
    out.error = "failed to read GameAssembly PE sections";
    return false;
  }

  constexpr std::size_t kChunk = 0x200000;
  constexpr std::size_t kPage = 0x1000;
  for (const ModuleSection &s : sections) {
    if (!detail_manager_rva::ShouldScanSection(s.name) || s.size == 0)
      continue;

    ++out.sectionsVisited;
    std::uint64_t sectionSize = s.size;
    if (options.maxSectionBytes && sectionSize > options.maxSectionBytes)
      sectionSize = options.maxSectionBytes;

    const std::uintptr_t secVa = moduleBase + s.rva;
    for (std::uint64_t off = 0; off < sectionSize; off += kChunk) {
      const std::size_t bytes = static_cast<std::size_t>(
          ((sectionSize - off) < kChunk) ? (sectionSize - off) : kChunk);
      const std::size_t pages = (bytes + kPage - 1) / kPage;
      std::vector<std::vector<std::uint8_t>> buffers(pages);
      for (std::size_t p = 0; p < pages; ++p) {
        const std::size_t pageBytes =
            ((p + 1) * kPage <= bytes) ? kPage : bytes - p * kPage;
        buffers[p].resize(pageBytes);
        mem.PrepareScatterRead(secVa + off + p * kPage, buffers[p].data(),
                               pageBytes);
      }
      (void)mem.ExecuteScatterRead();
      out.bytesScanned += bytes;

      std::vector<detail_manager_rva::Seed> seeds;
      for (std::size_t p = 0; p < pages; ++p) {
        for (std::size_t i = 0; i + sizeof(std::uintptr_t) <= buffers[p].size();
             i += sizeof(std::uintptr_t)) {
          std::uintptr_t ptr = 0;
          std::memcpy(&ptr, buffers[p].data() + i, sizeof(ptr));
          if (!IsCanonicalUserPtr(ptr))
            continue;

          detail_manager_rva::Seed seed;
          seed.section = s.name;
          seed.rva = static_cast<std::uint64_t>(s.rva) + off + p * kPage + i;
          seed.slotVa = moduleBase + static_cast<std::uintptr_t>(seed.rva);
          seed.slotPointer = ptr;
          seeds.push_back(seed);
          ++out.pointerSlots;
        }
      }

      for (auto &seed : seeds) {
        mem.PrepareScatterRead(seed.slotPointer + 0xB8, &seed.rootB8,
                               sizeof(seed.rootB8));
        mem.PrepareScatterRead(seed.slotPointer + 0xC0, &seed.rootC0,
                               sizeof(seed.rootC0));
      }
      (void)mem.ExecuteScatterRead();

      for (const auto &seed : seeds) {
        if (!IsCanonicalUserPtr(seed.rootB8) && !IsCanonicalUserPtr(seed.rootC0))
          continue;
        detail_manager_rva::ValidateSeed(mem, seed, out, options);
      }
    }
  }

  ApplyManagerRvaRelationshipScores(out);
  SortManagerRvaCandidates(out);
  out.ok = true;
  return true;
}

inline const ManagerRvaCandidate *
FindBestManagerRvaCandidate(const ManagerRvaScanResult &result,
                            ManagerRvaKind kind) {
  const ManagerRvaCandidate *best = nullptr;
  for (const ManagerRvaCandidate &candidate : result.candidates) {
    if (candidate.kind != kind)
      continue;
    if (!best || candidate.score > best->score)
      best = &candidate;
  }
  return best;
}

inline const ManagerRvaCandidate *
FindBestConfirmedManagerRvaCandidate(const ManagerRvaScanResult &result,
                                     ManagerRvaKind kind) {
  const ManagerRvaCandidate *best = nullptr;
  for (const ManagerRvaCandidate &candidate : result.candidates) {
    if (candidate.kind != kind || !candidate.confirmed)
      continue;
    if (!best || candidate.score > best->score)
      best = &candidate;
  }
  return best;
}

} // namespace er2
