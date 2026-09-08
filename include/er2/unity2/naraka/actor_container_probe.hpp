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

struct NarakaActorContainerOffsets {
  std::uint32_t scanStart = 0x0;
  std::uint32_t scanEnd = 0x1000;
  std::uint32_t scanStride = 0x8;
  std::uint32_t dictionaryEntries = 0x18;
  std::uint32_t dictionaryCount = 0x20;
  std::uint32_t dictionaryFirstEntry = 0x20;
  std::uint32_t dictionaryEntryStride = 0x18;
  std::uint32_t dictionaryEntryHash = 0x0;
  std::uint32_t dictionaryEntryNext = 0x4;
  std::uint32_t dictionaryEntryKey = 0x8;
  std::uint32_t dictionaryEntryValue = 0x10;
  std::uint32_t listItems = 0x10;
  std::uint32_t listSize = 0x18;
  std::uint32_t listFirstItem = 0x20;
  std::uint32_t listItemStride = 0x8;
};

struct NarakaActorContainerOptions {
  NarakaRuntimePropertyOptions actorOptions;
  NarakaActorContainerOffsets offsets;
  std::size_t maxContainerCount = 4096;
};

enum class NarakaActorContainerKind {
  Dictionary,
  List,
};

enum class NarakaActorContainerOwnerKind {
  ActorModel,
  PropertyData,
  RuntimePropertyData,
};

struct NarakaActorContainerCandidate {
  NarakaActorContainerKind kind = NarakaActorContainerKind::Dictionary;
  NarakaActorContainerOwnerKind ownerKind =
      NarakaActorContainerOwnerKind::ActorModel;
  std::uintptr_t owner = 0;
  std::uint32_t ownerOffset = 0;
  std::uintptr_t container = 0;
  std::uintptr_t backingArray = 0;
  std::int32_t count = 0;
  std::string evidence;
};

struct NarakaActorContainerEntry {
  NarakaActorContainerKind containerKind = NarakaActorContainerKind::Dictionary;
  NarakaActorContainerOwnerKind ownerKind =
      NarakaActorContainerOwnerKind::ActorModel;
  std::uint32_t ownerOffset = 0;
  std::uintptr_t container = 0;
  std::uintptr_t entryAddress = 0;
  std::int32_t key = 0;
  std::int32_t hash = 0;
  std::int32_t next = 0;
  std::uintptr_t rawValue = 0;
  std::uintptr_t valuePtr = 0;
  std::uintptr_t valueKlass = 0;
  std::int32_t int0 = 0;
  std::int32_t int1 = 0;
  std::int32_t int2 = 0;
  std::int32_t int3 = 0;
  std::uint32_t uint0 = 0;
  float float0 = 0.0f;
};

struct NarakaActorContainerSnapshot {
  std::uintptr_t actorModel = 0;
  std::uintptr_t propertyData = 0;
  std::uintptr_t runtimePropertyData = 0;
  std::uint32_t heroId = 0;
  std::uint32_t teamId = 0;
  std::vector<NarakaActorContainerCandidate> containers;
  std::vector<NarakaActorContainerEntry> entries;
  std::string error;
};

struct NarakaActorContainerDiff {
  NarakaActorContainerCandidate current;
  bool hadBaseline = false;
  bool added = false;
  bool changed = false;
  std::int32_t baselineCount = 0;
};

struct NarakaActorContainerEntryDiff {
  NarakaActorContainerEntry current;
  bool hadBaseline = false;
  bool added = false;
  bool changed = false;
  std::uintptr_t baselineRawValue = 0;
  std::int32_t baselineInt0 = 0;
  std::int32_t baselineInt1 = 0;
  std::int32_t baselineInt2 = 0;
  std::int32_t baselineInt3 = 0;
};

inline const NarakaActorContainerOptions &
DefaultNarakaActorContainerOptions() {
  static const NarakaActorContainerOptions options{};
  return options;
}

inline const char *
NarakaActorContainerKindName(NarakaActorContainerKind kind) {
  return kind == NarakaActorContainerKind::Dictionary ? "dictionary" : "list";
}

inline const char *
NarakaActorContainerOwnerKindName(NarakaActorContainerOwnerKind kind) {
  switch (kind) {
  case NarakaActorContainerOwnerKind::ActorModel:
    return "actor_model";
  case NarakaActorContainerOwnerKind::PropertyData:
    return "property_data";
  case NarakaActorContainerOwnerKind::RuntimePropertyData:
    return "runtime_property";
  default:
    return "unknown";
  }
}

inline bool ReadNarakaActorDictionaryCandidate(
    const IMemoryAccessor &mem, std::uintptr_t owner,
    std::uint32_t ownerOffset, std::uintptr_t container,
    const NarakaActorContainerOffsets &o, std::size_t maxCount,
    NarakaActorContainerCandidate &out) {
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

  out = NarakaActorContainerCandidate{};
  out.kind = NarakaActorContainerKind::Dictionary;
  out.owner = owner;
  out.ownerOffset = ownerOffset;
  out.container = container;
  out.backingArray = entries;
  out.count = count;
  out.evidence = "Dictionary entries/count readable";
  return true;
}

inline bool ReadNarakaActorListCandidate(
    const IMemoryAccessor &mem, std::uintptr_t owner,
    std::uint32_t ownerOffset, std::uintptr_t container,
    const NarakaActorContainerOffsets &o, std::size_t maxCount,
    NarakaActorContainerCandidate &out) {
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

  out = NarakaActorContainerCandidate{};
  out.kind = NarakaActorContainerKind::List;
  out.owner = owner;
  out.ownerOffset = ownerOffset;
  out.container = container;
  out.backingArray = items;
  out.count = count;
  out.evidence = "List items/size readable";
  return true;
}

inline std::vector<NarakaActorContainerCandidate>
ScanNarakaActorOwnerContainers(const IMemoryAccessor &mem,
                               std::uintptr_t owner,
                               NarakaActorContainerOwnerKind ownerKind,
                               const NarakaActorContainerOptions &options,
                               std::size_t maxCount) {
  std::vector<NarakaActorContainerCandidate> out;
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

    NarakaActorContainerCandidate candidate;
    if (ReadNarakaActorDictionaryCandidate(mem, owner, offset, ptr, o,
                                           maxCount, candidate)) {
      candidate.ownerKind = ownerKind;
      out.push_back(candidate);
      continue;
    }
    if (ReadNarakaActorListCandidate(mem, owner, offset, ptr, o, maxCount,
                                     candidate)) {
      candidate.ownerKind = ownerKind;
      out.push_back(candidate);
    }
  }
  return out;
}

inline void ReadNarakaActorContainerEntryValue(
    const IMemoryAccessor &mem, NarakaActorContainerEntry &entry) {
  entry.valuePtr = IsCanonicalUserPtr(entry.rawValue) ? entry.rawValue : 0;
  if (!entry.valuePtr) {
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

inline void CaptureNarakaActorEntriesFromContainer(
    const IMemoryAccessor &mem, const NarakaActorContainerCandidate &container,
    const NarakaActorContainerOffsets &o, std::size_t maxEntries,
    std::vector<NarakaActorContainerEntry> &out) {
  const std::size_t limit = std::min<std::size_t>(
      static_cast<std::size_t>(container.count), maxEntries);

  for (std::size_t i = 0; i < limit; ++i) {
    NarakaActorContainerEntry entry;
    entry.container = container.container;
    entry.containerKind = container.kind;
    entry.ownerKind = container.ownerKind;
    entry.ownerOffset = container.ownerOffset;

    if (container.kind == NarakaActorContainerKind::Dictionary) {
      entry.entryAddress = container.backingArray + o.dictionaryFirstEntry +
                           i * o.dictionaryEntryStride;
      if (!ReadValue(mem, entry.entryAddress + o.dictionaryEntryHash,
                     entry.hash) ||
          entry.hash < 0) {
        continue;
      }
      (void)ReadValue(mem, entry.entryAddress + o.dictionaryEntryNext,
                      entry.next);
      (void)ReadValue(mem, entry.entryAddress + o.dictionaryEntryKey,
                      entry.key);
      (void)ReadPtr(mem, entry.entryAddress + o.dictionaryEntryValue,
                    entry.rawValue);
    } else {
      entry.entryAddress = container.backingArray + o.listFirstItem +
                           i * o.listItemStride;
      (void)ReadPtr(mem, entry.entryAddress, entry.rawValue);
      entry.key = static_cast<std::int32_t>(i);
      entry.hash = static_cast<std::int32_t>(i);
    }

    ReadNarakaActorContainerEntryValue(mem, entry);
    out.push_back(entry);
  }
}

inline bool CaptureNarakaActorContainerSnapshot(
    const IMemoryAccessor &mem, std::uintptr_t actorModel,
    const NarakaActorContainerOptions &options, std::size_t maxEntries,
    NarakaActorContainerSnapshot &out) {
  out = NarakaActorContainerSnapshot{};
  out.actorModel = actorModel;

  NarakaActorRuntimePropertyCandidate actor;
  if (!ReadNarakaActorRuntimePropertyCandidate(mem, actorModel,
                                               options.actorOptions, actor)) {
    out.error = "failed to read ActorModel roots";
    return false;
  }

  out.propertyData = actor.propertyData;
  out.runtimePropertyData = actor.runtimePropertyData;
  out.heroId = actor.heroId;
  out.teamId = actor.teamId;

  const std::size_t maxCount =
      std::max<std::size_t>(options.maxContainerCount, 1);
  out.containers = ScanNarakaActorOwnerContainers(
      mem, actor.actorModel, NarakaActorContainerOwnerKind::ActorModel,
      options, maxCount);

  auto propertyContainers = ScanNarakaActorOwnerContainers(
      mem, actor.propertyData, NarakaActorContainerOwnerKind::PropertyData,
      options, maxCount);
  out.containers.insert(out.containers.end(), propertyContainers.begin(),
                        propertyContainers.end());

  auto runtimeContainers = ScanNarakaActorOwnerContainers(
      mem, actor.runtimePropertyData,
      NarakaActorContainerOwnerKind::RuntimePropertyData, options, maxCount);
  out.containers.insert(out.containers.end(), runtimeContainers.begin(),
                        runtimeContainers.end());

  std::sort(out.containers.begin(), out.containers.end(),
            [](const NarakaActorContainerCandidate &a,
               const NarakaActorContainerCandidate &b) {
              if (a.ownerKind != b.ownerKind) {
                return a.ownerKind < b.ownerKind;
              }
              if (a.ownerOffset != b.ownerOffset) {
                return a.ownerOffset < b.ownerOffset;
              }
              return a.container < b.container;
            });

  for (const auto &container : out.containers) {
    if (out.entries.size() >= maxEntries) {
      break;
    }
    CaptureNarakaActorEntriesFromContainer(
        mem, container, options.offsets, maxEntries - out.entries.size(),
        out.entries);
  }

  if (out.containers.empty()) {
    out.error = "no ActorModel container candidates found";
    return false;
  }
  return true;
}

inline std::vector<NarakaActorContainerDiff>
DiffNarakaActorContainers(const NarakaActorContainerSnapshot &baseline,
                          const NarakaActorContainerSnapshot &current) {
  std::unordered_map<std::uintptr_t, const NarakaActorContainerCandidate *>
      byContainer;
  byContainer.reserve(baseline.containers.size());
  for (const auto &container : baseline.containers) {
    byContainer[container.container] = &container;
  }

  std::vector<NarakaActorContainerDiff> out;
  out.reserve(current.containers.size());
  for (const auto &container : current.containers) {
    NarakaActorContainerDiff diff;
    diff.current = container;
    const auto it = byContainer.find(container.container);
    if (it == byContainer.end() || !it->second) {
      diff.added = true;
      diff.changed = true;
    } else {
      diff.hadBaseline = true;
      diff.baselineCount = it->second->count;
      diff.changed = diff.baselineCount != container.count;
    }
    out.push_back(diff);
  }
  return out;
}

inline std::uintptr_t
NarakaActorEntryStableKey(const NarakaActorContainerEntry &entry) {
  return entry.valuePtr ? entry.valuePtr : entry.entryAddress;
}

inline std::vector<NarakaActorContainerEntryDiff>
DiffNarakaActorContainerEntries(
    const NarakaActorContainerSnapshot &baseline,
    const NarakaActorContainerSnapshot &current) {
  std::unordered_map<std::uintptr_t, const NarakaActorContainerEntry *> byEntry;
  byEntry.reserve(baseline.entries.size());
  for (const auto &entry : baseline.entries) {
    const std::uintptr_t key = NarakaActorEntryStableKey(entry);
    if (key) {
      byEntry[key] = &entry;
    }
  }

  std::vector<NarakaActorContainerEntryDiff> out;
  out.reserve(current.entries.size());
  for (const auto &entry : current.entries) {
    NarakaActorContainerEntryDiff diff;
    diff.current = entry;
    const auto it = byEntry.find(NarakaActorEntryStableKey(entry));
    if (it == byEntry.end() || !it->second) {
      diff.added = true;
      diff.changed = true;
    } else {
      diff.hadBaseline = true;
      diff.baselineRawValue = it->second->rawValue;
      diff.baselineInt0 = it->second->int0;
      diff.baselineInt1 = it->second->int1;
      diff.baselineInt2 = it->second->int2;
      diff.baselineInt3 = it->second->int3;
      diff.changed = diff.baselineRawValue != entry.rawValue ||
                     diff.baselineInt0 != entry.int0 ||
                     diff.baselineInt1 != entry.int1 ||
                     diff.baselineInt2 != entry.int2 ||
                     diff.baselineInt3 != entry.int3;
    }
    out.push_back(diff);
  }
  return out;
}

} // namespace er2
