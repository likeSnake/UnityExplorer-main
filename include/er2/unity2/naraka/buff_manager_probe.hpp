#pragma once

#include "runtime_property_probe.hpp"

#include "../../mem/memory_accessor.hpp"
#include "../../mem/memory_read.hpp"

#include <algorithm>
#include <cstdint>
#include <string>
#include <unordered_map>
#include <vector>

namespace er2 {

struct NarakaBuffManagerOffsets {
  std::uint32_t staticFields = 0xB8;
  std::uint32_t instance = 0x0;
  std::uint32_t characterManagerLocalActor = 0x18;
  std::uint32_t actorFid = 0x340;
  std::uint32_t actorPropertyData = 0x348;
  std::uint32_t actorHeroId = 0x108;
  std::uint32_t recordBuffHandlersDict = 0x10;
  std::uint32_t buffHandlerBuffDict = 0x30;
  std::uint32_t buffHandlerInstanceIdList = 0x38;
  std::uint32_t buffHandlerTidList = 0x40;
  std::uint32_t buffObjectBuffId = 0x20;
  std::uint32_t buffObjectGroupId = 0x24;
  std::uint32_t buffObjectLeftTime = 0x34;
  std::uint32_t buffObjectOwnerFid = 0x7C;
  std::uint32_t buffObjectCasterFid = 0x80;
  std::uint32_t buffObjectInstanceId = 0x84;
  std::uint32_t scanStart = 0x0;
  std::uint32_t scanEnd = 0x180;
  std::uint32_t scanStride = 0x8;
  std::uint32_t dictionaryEntries = 0x18;
  std::uint32_t dictionaryCount = 0x20;
  std::uint32_t dictionaryFirstEntry = 0x20;
  std::uint32_t dictionaryEntryStride = 0x18;
  std::uint32_t dictionaryEntryHash = 0x0;
  std::uint32_t dictionaryEntryKey = 0x8;
  std::uint32_t dictionaryEntryValue = 0x10;
  std::uint32_t listItems = 0x10;
  std::uint32_t listSize = 0x18;
  std::uint32_t listFirstItem = 0x20;
  std::uint32_t listItemStride = 0x8;
};

struct NarakaBuffManagerOptions {
  NarakaManagerRvas rvas;
  NarakaBuffManagerOffsets offsets;
  std::size_t maxContainerCount = 100000;
};

enum class NarakaBuffContainerKind {
  Dictionary,
  List,
};

enum class NarakaBuffContainerOwnerKind {
  Instance,
  StaticFields,
};

struct NarakaBuffContainerCandidate {
  NarakaBuffContainerKind kind = NarakaBuffContainerKind::Dictionary;
  NarakaBuffContainerOwnerKind ownerKind = NarakaBuffContainerOwnerKind::Instance;
  std::uintptr_t owner = 0;
  std::uint32_t ownerOffset = 0;
  std::uintptr_t container = 0;
  std::uintptr_t backingArray = 0;
  std::int32_t count = 0;
  std::string evidence;
};

struct NarakaBuffEntrySample {
  std::uintptr_t container = 0;
  NarakaBuffContainerKind containerKind = NarakaBuffContainerKind::Dictionary;
  std::uintptr_t entryAddress = 0;
  std::uintptr_t valuePtr = 0;
  std::uintptr_t valueKlass = 0;
  std::int32_t key = 0;
  std::int32_t int0 = 0;
  std::int32_t int1 = 0;
  std::int32_t int2 = 0;
  std::int32_t int3 = 0;
  std::uint32_t uint0 = 0;
  float float0 = 0.0f;
};

struct NarakaLocalBuffEntry {
  std::uintptr_t entryAddress = 0;
  std::int32_t key = 0;
  std::uintptr_t buff = 0;
  std::uintptr_t klass = 0;
  std::int32_t buffId = 0;
  std::int32_t groupId = 0;
  std::int32_t instanceId = 0;
  std::int32_t ownerFid = 0;
  std::int32_t casterFid = 0;
  float leftTime = 0.0f;
};

struct NarakaLocalBuffSnapshot {
  std::uintptr_t characterManager = 0;
  std::uintptr_t actorModel = 0;
  std::uintptr_t propertyData = 0;
  std::int32_t fid = 0;
  std::uint32_t heroId = 0;
  std::uintptr_t buffHandlersDict = 0;
  std::uintptr_t buffHandlersEntries = 0;
  std::int32_t buffHandlersCount = 0;
  std::int32_t buffHandlersArrayLength = 0;
  std::int32_t selectedHandlerKey = 0;
  std::uintptr_t buffHandler = 0;
  std::uintptr_t buffTidList = 0;
  std::uintptr_t buffTidItems = 0;
  std::int32_t buffTidCount = 0;
  std::vector<std::int32_t> buffTidValues;
  std::uintptr_t buffInstanceIdList = 0;
  std::uintptr_t buffDict = 0;
  std::uintptr_t buffDictEntries = 0;
  std::int32_t buffDictCount = 0;
  std::int32_t buffDictArrayLength = 0;
  std::vector<NarakaLocalBuffEntry> buffEntries;
  bool hasTargetBuff = false;
  std::int32_t targetBuffId = 0;
  std::string error;
};

struct NarakaBuffManagerSnapshot {
  std::uintptr_t gameAssemblyBase = 0;
  std::uintptr_t buffManagerClass = 0;
  std::uintptr_t staticFields = 0;
  std::uintptr_t instance = 0;
  std::vector<NarakaBuffContainerCandidate> containers;
  std::vector<NarakaBuffEntrySample> entries;
  NarakaLocalBuffSnapshot local;
  std::string error;
};

struct NarakaBuffEntryDiff {
  NarakaBuffEntrySample current;
  bool hadBaseline = false;
  bool added = false;
  bool changed = false;
  std::int32_t baselineInt0 = 0;
  std::int32_t baselineInt1 = 0;
  std::int32_t baselineInt2 = 0;
  std::int32_t baselineInt3 = 0;
};

inline NarakaBuffManagerOptions DefaultNarakaBuffManagerOptions() {
  NarakaBuffManagerOptions options;
  options.rvas = DefaultNarakaManagerRvas();
  return options;
}

inline const char *
NarakaBuffContainerKindName(NarakaBuffContainerKind kind) {
  return kind == NarakaBuffContainerKind::Dictionary ? "dictionary" : "list";
}

inline const char *
NarakaBuffContainerOwnerKindName(NarakaBuffContainerOwnerKind kind) {
  return kind == NarakaBuffContainerOwnerKind::StaticFields ? "static_fields"
                                                           : "instance";
}

inline bool ResolveNarakaBuffManagerPointers(
    const IMemoryAccessor &mem, std::uintptr_t gameAssemblyBase,
    const NarakaBuffManagerOptions &options,
    NarakaBuffManagerSnapshot &out) {
  out.gameAssemblyBase = gameAssemblyBase;
  if (!IsCanonicalUserPtr(gameAssemblyBase)) {
    out.error = "invalid GameAssembly base";
    return false;
  }

  const std::uintptr_t slot =
      gameAssemblyBase + static_cast<std::uintptr_t>(options.rvas.buffManager);
  if (!ReadPtr(mem, slot, out.buffManagerClass) ||
      !IsCanonicalUserPtr(out.buffManagerClass)) {
    out.error = "failed to read BuffManager TypeInfo";
    return false;
  }

  const auto &o = options.offsets;
  if (!ReadPtr(mem, out.buffManagerClass + o.staticFields, out.staticFields) ||
      !IsCanonicalUserPtr(out.staticFields)) {
    out.error = "failed to read BuffManager static_fields";
    return false;
  }

  if (!ReadPtr(mem, out.staticFields + o.instance, out.instance) ||
      !IsCanonicalUserPtr(out.instance)) {
    out.error = "failed to read BuffManager _instance";
    return false;
  }
  return true;
}

inline bool ReadNarakaDictionaryCandidate(
    const IMemoryAccessor &mem, std::uintptr_t owner,
    std::uint32_t ownerOffset, std::uintptr_t container,
    const NarakaBuffManagerOffsets &o, std::size_t maxCount,
    NarakaBuffContainerCandidate &out) {
  std::uintptr_t entries = 0;
  std::int32_t count = 0;
  if (!ReadPtr(mem, container + o.dictionaryEntries, entries) ||
      !ReadValue(mem, container + o.dictionaryCount, count) ||
      !IsCanonicalUserPtr(entries) || count <= 0 ||
      count > static_cast<std::int32_t>(maxCount)) {
    return false;
  }

  std::int32_t length = 0;
  if (!ReadValue(mem, entries + 0x18, length) || length < count ||
      length > 100000) {
    return false;
  }

  out = NarakaBuffContainerCandidate{};
  out.kind = NarakaBuffContainerKind::Dictionary;
  out.owner = owner;
  out.ownerOffset = ownerOffset;
  out.container = container;
  out.backingArray = entries;
  out.count = count;
  out.evidence = "Dictionary entries/count readable";
  return true;
}

inline bool ReadNarakaListCandidate(
    const IMemoryAccessor &mem, std::uintptr_t owner,
    std::uint32_t ownerOffset, std::uintptr_t container,
    const NarakaBuffManagerOffsets &o, std::size_t maxCount,
    NarakaBuffContainerCandidate &out) {
  std::uintptr_t items = 0;
  std::int32_t count = 0;
  if (!ReadPtr(mem, container + o.listItems, items) ||
      !ReadValue(mem, container + o.listSize, count) ||
      !IsCanonicalUserPtr(items) || count <= 0 ||
      count > static_cast<std::int32_t>(maxCount)) {
    return false;
  }

  std::int32_t length = 0;
  if (!ReadValue(mem, items + 0x18, length) || length < count ||
      length > 100000) {
    return false;
  }

  out = NarakaBuffContainerCandidate{};
  out.kind = NarakaBuffContainerKind::List;
  out.owner = owner;
  out.ownerOffset = ownerOffset;
  out.container = container;
  out.backingArray = items;
  out.count = count;
  out.evidence = "List items/size readable";
  return true;
}

inline std::vector<NarakaBuffContainerCandidate>
ScanNarakaBuffContainers(const IMemoryAccessor &mem, std::uintptr_t owner,
                         NarakaBuffContainerOwnerKind ownerKind,
                         const NarakaBuffManagerOptions &options,
                         std::size_t maxCount) {
  std::vector<NarakaBuffContainerCandidate> out;
  if (!IsCanonicalUserPtr(owner)) {
    return out;
  }

  const auto &o = options.offsets;
  for (std::uint32_t offset = o.scanStart; offset <= o.scanEnd;
       offset += o.scanStride) {
    std::uintptr_t ptr = 0;
    if (!ReadPtr(mem, owner + offset, ptr) || !IsCanonicalUserPtr(ptr)) {
      continue;
    }

    NarakaBuffContainerCandidate candidate;
    if (ReadNarakaDictionaryCandidate(mem, owner, offset, ptr, o, maxCount,
                                      candidate)) {
      candidate.ownerKind = ownerKind;
      out.push_back(candidate);
      continue;
    }
    if (ReadNarakaListCandidate(mem, owner, offset, ptr, o, maxCount,
                                candidate)) {
      candidate.ownerKind = ownerKind;
      out.push_back(candidate);
    }
  }

  std::sort(out.begin(), out.end(),
            [](const NarakaBuffContainerCandidate &a,
               const NarakaBuffContainerCandidate &b) {
              if (a.ownerOffset != b.ownerOffset) {
                return a.ownerOffset < b.ownerOffset;
              }
              if (a.ownerKind != b.ownerKind) {
                return a.ownerKind < b.ownerKind;
              }
              return a.container < b.container;
            });
  return out;
}

inline void ReadNarakaBuffEntryValueFields(const IMemoryAccessor &mem,
                                           NarakaBuffEntrySample &entry) {
  if (!IsCanonicalUserPtr(entry.valuePtr)) {
    return;
  }
  (void)ReadPtr(mem, entry.valuePtr + 0x0, entry.valueKlass);

  constexpr std::uintptr_t kIl2CppObjectFields = 0x10;
  (void)ReadValue(mem, entry.valuePtr + kIl2CppObjectFields + 0x0,
                  entry.int0);
  (void)ReadValue(mem, entry.valuePtr + kIl2CppObjectFields + 0x4,
                  entry.int1);
  (void)ReadValue(mem, entry.valuePtr + kIl2CppObjectFields + 0x8,
                  entry.int2);
  (void)ReadValue(mem, entry.valuePtr + kIl2CppObjectFields + 0xC,
                  entry.int3);
  entry.uint0 = static_cast<std::uint32_t>(entry.int0);
  (void)ReadValue(mem, entry.valuePtr + kIl2CppObjectFields + 0x0,
                  entry.float0);
}

inline void CaptureNarakaBuffEntriesFromContainer(
    const IMemoryAccessor &mem, const NarakaBuffContainerCandidate &container,
    const NarakaBuffManagerOffsets &o, std::size_t maxEntries,
    std::vector<NarakaBuffEntrySample> &out) {
  const std::size_t limit = std::min<std::size_t>(
      static_cast<std::size_t>(container.count), maxEntries);

  for (std::size_t i = 0; i < limit; ++i) {
    NarakaBuffEntrySample entry;
    entry.container = container.container;
    entry.containerKind = container.kind;

    if (container.kind == NarakaBuffContainerKind::Dictionary) {
      entry.entryAddress = container.backingArray + o.dictionaryFirstEntry +
                           i * o.dictionaryEntryStride;
      std::int32_t hashCode = 0;
      if (!ReadValue(mem, entry.entryAddress + o.dictionaryEntryHash,
                     hashCode) ||
          hashCode < 0) {
        continue;
      }
      (void)ReadValue(mem, entry.entryAddress + o.dictionaryEntryKey,
                      entry.key);
      (void)ReadPtr(mem, entry.entryAddress + o.dictionaryEntryValue,
                    entry.valuePtr);
    } else {
      entry.entryAddress = container.backingArray + o.listFirstItem +
                           i * o.listItemStride;
      (void)ReadPtr(mem, entry.entryAddress, entry.valuePtr);
      entry.key = static_cast<std::int32_t>(i);
    }

    ReadNarakaBuffEntryValueFields(mem, entry);
    out.push_back(entry);
  }
}

inline bool ReadNarakaIntList(const IMemoryAccessor &mem, std::uintptr_t list,
                              const NarakaBuffManagerOffsets &o,
                              std::size_t maxValues,
                              std::uintptr_t &itemsOut,
                              std::int32_t &countOut,
                              std::vector<std::int32_t> &valuesOut) {
  itemsOut = 0;
  countOut = 0;
  valuesOut.clear();
  if (!IsCanonicalUserPtr(list)) {
    return false;
  }

  if (!ReadPtr(mem, list + o.listItems, itemsOut) ||
      !ReadValue(mem, list + o.listSize, countOut) ||
      !IsCanonicalUserPtr(itemsOut) || countOut < 0 ||
      countOut > static_cast<std::int32_t>(maxValues)) {
    return false;
  }

  std::int32_t length = 0;
  if (!ReadValue(mem, itemsOut + 0x18, length) || length < countOut ||
      length > 100000) {
    return false;
  }

  valuesOut.reserve(static_cast<std::size_t>(countOut));
  for (std::int32_t i = 0; i < countOut; ++i) {
    std::int32_t value = 0;
    if (ReadValue(mem, itemsOut + o.listFirstItem +
                           static_cast<std::uintptr_t>(i) * 4u,
                  value)) {
      valuesOut.push_back(value);
    }
  }
  return true;
}

inline bool CaptureNarakaLocalBuffSnapshot(
    const IMemoryAccessor &mem, std::uintptr_t gameAssemblyBase,
    const NarakaBuffManagerOptions &options, std::size_t maxEntries,
    std::int32_t targetBuffId, NarakaLocalBuffSnapshot &out) {
  out = NarakaLocalBuffSnapshot{};
  out.targetBuffId = targetBuffId;
  const auto &o = options.offsets;

  if (!ResolveNarakaCharacterManagerInstance(mem, gameAssemblyBase,
                                             out.characterManager)) {
    out.error = "failed to resolve CharacterManager instance";
    return false;
  }

  if (!ReadPtr(mem, out.characterManager + o.characterManagerLocalActor,
               out.actorModel) ||
      !IsCanonicalUserPtr(out.actorModel)) {
    out.error = "failed to read local ActorModel";
    return false;
  }

  (void)ReadValue(mem, out.actorModel + o.actorFid, out.fid);
  (void)ReadPtr(mem, out.actorModel + o.actorPropertyData, out.propertyData);
  if (IsCanonicalUserPtr(out.propertyData)) {
    (void)ReadValue(mem, out.propertyData + o.actorHeroId, out.heroId);
  }
  if (out.fid <= 0) {
    out.error = "invalid local ActorModel fid";
    return false;
  }

  NarakaBuffManagerSnapshot manager;
  if (!ResolveNarakaBuffManagerPointers(mem, gameAssemblyBase, options,
                                        manager)) {
    out.error = manager.error.empty() ? "failed to resolve BuffManager"
                                      : manager.error;
    return false;
  }

  if (!ReadPtr(mem, manager.instance + o.recordBuffHandlersDict,
               out.buffHandlersDict) ||
      !IsCanonicalUserPtr(out.buffHandlersDict)) {
    out.error = "failed to read BuffManager._recordBuffHandlersDict";
    return false;
  }

  if (!ReadPtr(mem, out.buffHandlersDict + o.dictionaryEntries,
               out.buffHandlersEntries) ||
      !IsCanonicalUserPtr(out.buffHandlersEntries) ||
      !ReadValue(mem, out.buffHandlersEntries + 0x18,
                 out.buffHandlersArrayLength) ||
      out.buffHandlersArrayLength <= 0 ||
      out.buffHandlersArrayLength > static_cast<std::int32_t>(maxEntries)) {
    out.error = "invalid BuffHandler dictionary";
    return false;
  }
  (void)ReadValue(mem, out.buffHandlersDict + o.dictionaryCount,
                  out.buffHandlersCount);

  for (std::int32_t i = 0; i < out.buffHandlersArrayLength; ++i) {
    const std::uintptr_t entry =
        out.buffHandlersEntries + o.dictionaryFirstEntry +
        static_cast<std::uintptr_t>(i) * o.dictionaryEntryStride;
    std::int32_t hash = -1;
    std::int32_t key = 0;
    std::uintptr_t handler = 0;
    if (!ReadValue(mem, entry + o.dictionaryEntryHash, hash) || hash < 0) {
      continue;
    }
    (void)ReadValue(mem, entry + o.dictionaryEntryKey, key);
    (void)ReadPtr(mem, entry + o.dictionaryEntryValue, handler);
    if (key == out.fid && IsCanonicalUserPtr(handler)) {
      out.selectedHandlerKey = key;
      out.buffHandler = handler;
      break;
    }
  }

  if (!IsCanonicalUserPtr(out.buffHandler)) {
    out.error = "local fid was not found in BuffManager._recordBuffHandlersDict";
    return false;
  }

  (void)ReadPtr(mem, out.buffHandler + o.buffHandlerInstanceIdList,
                out.buffInstanceIdList);
  (void)ReadPtr(mem, out.buffHandler + o.buffHandlerTidList,
                out.buffTidList);
  if (IsCanonicalUserPtr(out.buffTidList)) {
    (void)ReadNarakaIntList(mem, out.buffTidList, o, maxEntries,
                            out.buffTidItems, out.buffTidCount,
                            out.buffTidValues);
  }

  (void)ReadPtr(mem, out.buffHandler + o.buffHandlerBuffDict, out.buffDict);
  if (IsCanonicalUserPtr(out.buffDict)) {
    (void)ReadPtr(mem, out.buffDict + o.dictionaryEntries,
                  out.buffDictEntries);
    (void)ReadValue(mem, out.buffDict + o.dictionaryCount,
                    out.buffDictCount);
    if (IsCanonicalUserPtr(out.buffDictEntries)) {
      (void)ReadValue(mem, out.buffDictEntries + 0x18,
                      out.buffDictArrayLength);
    }
  }

  if (IsCanonicalUserPtr(out.buffDictEntries) &&
      out.buffDictArrayLength > 0 &&
      out.buffDictArrayLength <= static_cast<std::int32_t>(maxEntries)) {
    for (std::int32_t i = 0; i < out.buffDictArrayLength; ++i) {
      const std::uintptr_t entry =
          out.buffDictEntries + o.dictionaryFirstEntry +
          static_cast<std::uintptr_t>(i) * o.dictionaryEntryStride;
      std::int32_t hash = -1;
      if (!ReadValue(mem, entry + o.dictionaryEntryHash, hash) || hash < 0) {
        continue;
      }

      NarakaLocalBuffEntry row;
      row.entryAddress = entry;
      (void)ReadValue(mem, entry + o.dictionaryEntryKey, row.key);
      (void)ReadPtr(mem, entry + o.dictionaryEntryValue, row.buff);
      if (IsCanonicalUserPtr(row.buff)) {
        (void)ReadPtr(mem, row.buff, row.klass);
        (void)ReadValue(mem, row.buff + o.buffObjectBuffId, row.buffId);
        (void)ReadValue(mem, row.buff + o.buffObjectGroupId, row.groupId);
        (void)ReadValue(mem, row.buff + o.buffObjectInstanceId,
                        row.instanceId);
        (void)ReadValue(mem, row.buff + o.buffObjectOwnerFid, row.ownerFid);
        (void)ReadValue(mem, row.buff + o.buffObjectCasterFid, row.casterFid);
        (void)ReadValue(mem, row.buff + o.buffObjectLeftTime, row.leftTime);
      }
      out.buffEntries.push_back(row);
    }
  }

  out.hasTargetBuff =
      std::find(out.buffTidValues.begin(), out.buffTidValues.end(),
                targetBuffId) != out.buffTidValues.end();
  if (!out.hasTargetBuff) {
    out.hasTargetBuff =
        std::any_of(out.buffEntries.begin(), out.buffEntries.end(),
                    [targetBuffId](const NarakaLocalBuffEntry &entry) {
                      return entry.buffId == targetBuffId;
                    });
  }

  return true;
}

inline bool CaptureNarakaBuffManagerSnapshot(
    const IMemoryAccessor &mem, std::uintptr_t gameAssemblyBase,
    const NarakaBuffManagerOptions &options, std::size_t maxEntries,
    NarakaBuffManagerSnapshot &out) {
  out = NarakaBuffManagerSnapshot{};
  if (!ResolveNarakaBuffManagerPointers(mem, gameAssemblyBase, options, out)) {
    return false;
  }

  out.containers = ScanNarakaBuffContainers(
      mem, out.instance, NarakaBuffContainerOwnerKind::Instance, options,
      std::max<std::size_t>(options.maxContainerCount, 1));
  auto staticContainers = ScanNarakaBuffContainers(
      mem, out.staticFields, NarakaBuffContainerOwnerKind::StaticFields,
      options, std::max<std::size_t>(options.maxContainerCount, 1));
  out.containers.insert(out.containers.end(), staticContainers.begin(),
                        staticContainers.end());
  for (const auto &container : out.containers) {
    CaptureNarakaBuffEntriesFromContainer(mem, container, options.offsets,
                                          maxEntries, out.entries);
  }
  (void)CaptureNarakaLocalBuffSnapshot(mem, gameAssemblyBase, options,
                                       maxEntries, 2002103, out.local);
  if (out.containers.empty()) {
    out.error = "no BuffManager container candidates found";
    return false;
  }
  return true;
}

inline std::vector<NarakaBuffEntryDiff>
DiffNarakaBuffManagerSnapshots(const NarakaBuffManagerSnapshot &baseline,
                               const NarakaBuffManagerSnapshot &current) {
  std::unordered_map<std::uintptr_t, const NarakaBuffEntrySample *> byValue;
  byValue.reserve(baseline.entries.size());
  for (const auto &entry : baseline.entries) {
    if (entry.valuePtr) {
      byValue[entry.valuePtr] = &entry;
    }
  }

  std::vector<NarakaBuffEntryDiff> out;
  out.reserve(current.entries.size());
  for (const auto &entry : current.entries) {
    NarakaBuffEntryDiff diff;
    diff.current = entry;
    const auto it = byValue.find(entry.valuePtr);
    if (it == byValue.end() || !it->second) {
      diff.added = true;
      diff.changed = true;
    } else {
      diff.hadBaseline = true;
      diff.baselineInt0 = it->second->int0;
      diff.baselineInt1 = it->second->int1;
      diff.baselineInt2 = it->second->int2;
      diff.baselineInt3 = it->second->int3;
      diff.changed = diff.baselineInt0 != entry.int0 ||
                     diff.baselineInt1 != entry.int1 ||
                     diff.baselineInt2 != entry.int2 ||
                     diff.baselineInt3 != entry.int3;
    }
    out.push_back(diff);
  }
  return out;
}

} // namespace er2
