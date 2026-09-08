#pragma once

#include "../../core/types.hpp"
#include "../../mem/memory_accessor.hpp"
#include "../../mem/memory_read.hpp"

#include <algorithm>
#include <cstdint>
#include <string>
#include <unordered_map>
#include <vector>

namespace er2 {

struct NarakaActorOffsets {
  std::uint32_t runtimePropertyData = 0xA8;
  std::uint32_t propertyData = 0x348;
  std::uint32_t heroId = 0x108;
  std::uint32_t teamId = 0x118;
};

struct NarakaRuntimePropertyOffsets {
  std::uint32_t root = 0x10;
  std::uint32_t idRoot = 0x10;
  std::uint32_t idEntries = 0x18;
  std::uint32_t idCount = 0x18;
  std::uint32_t idFirstEntry = 0x28;
  std::uint32_t idEntryStride = 0x10;
  std::uint32_t idEntryDataId = 0x0;
  std::uint32_t idEntryIndex = 0x4;
  std::uint32_t valuesRoot = 0x28;
  std::uint32_t valuesArray = 0x10;
  std::uint32_t valuesFirst = 0x20;
  std::uint32_t valueStride = 0x4;
};

struct NarakaRuntimePropertyOptions {
  NarakaActorOffsets actor;
  NarakaRuntimePropertyOffsets property;
};

struct NarakaCharacterManagerOffsets {
  std::uint32_t staticFields = 0xB8;
  std::uint32_t instance = 0x8;
  std::uint32_t allAliveCharList = 0x58;
  std::uint32_t actorArray = 0x18;
  std::uint32_t actorCount = 0x18;
  std::uint32_t actorFirst = 0x30;
  std::uint32_t actorStride = 0x18;
  std::uint32_t dictionaryEntries = 0x18;
  std::uint32_t dictionaryCount = 0x20;
  std::uint32_t dictionaryFirstEntry = 0x20;
  std::uint32_t dictionaryEntryStride = 0x18;
  std::uint32_t dictionaryValue = 0x10;
};

enum class GameAssemblyVariant { Default = 0, Super };

struct NarakaManagerRvas {
  std::uint64_t characterManager = 0xe1406f0ull;
  std::uint64_t globalTime      = 0xe142ac8ull;
  std::uint64_t charactorSync   = 0xe106e10ull;
  std::uint64_t entityManager   = 0xe141380ull;
  std::uint64_t userDataManager = 0xe142348ull;
  std::uint64_t buffManager     = 0xE13FC68ull;
  std::uint64_t windowBase      = 0x2795D38ull;
  std::uint64_t transparentTagHandler = 0xDFABA70ull;

  static NarakaManagerRvas ForVariant(GameAssemblyVariant v) {
    NarakaManagerRvas r;
    if (v == GameAssemblyVariant::Super) {
      r.characterManager       = 0x365F150ull;
      r.globalTime             = 0x3668A90ull;
      r.charactorSync          = 0x365F1F8ull;
      r.entityManager          = 0x3668B48ull;
      r.userDataManager        = 0x36849E8ull;
      r.buffManager            = 0x365DD80ull;
      r.windowBase             = 0x3676AE0ull;
      r.transparentTagHandler  = 0x367D078ull;
    }
    return r;
  }
};

struct NarakaRuntimePropertyRow {
  std::int32_t dataId = 0;
  std::int32_t index = 0;
  std::uintptr_t valueAddress = 0;
  std::int32_t intValue = 0;
  std::uint32_t uintValue = 0;
  float floatValue = 0.0f;
  bool valueReadOk = false;
};

struct NarakaRuntimePropertySnapshot {
  std::uintptr_t actorModel = 0;
  std::uintptr_t propertyData = 0;
  std::uintptr_t runtimePropertyData = 0;
  std::uintptr_t propertyRoot = 0;
  std::uintptr_t idEntries = 0;
  std::uintptr_t valuesArray = 0;
  std::int32_t idCount = 0;
  std::uint32_t heroId = 0;
  std::uint32_t teamId = 0;
  std::vector<NarakaRuntimePropertyRow> rows;
  std::string error;
};

struct NarakaRuntimePropertyDiffRow {
  std::int32_t dataId = 0;
  std::int32_t index = 0;
  std::uintptr_t valueAddress = 0;
  bool hadBaseline = false;
  bool changed = false;
  std::int32_t baselineInt = 0;
  std::int32_t currentInt = 0;
  std::int32_t intDelta = 0;
  float baselineFloat = 0.0f;
  float currentFloat = 0.0f;
  float floatDelta = 0.0f;
};

struct NarakaActorRuntimePropertyCandidate {
  std::uintptr_t actorModel = 0;
  std::uintptr_t propertyData = 0;
  std::uintptr_t runtimePropertyData = 0;
  std::uint32_t heroId = 0;
  std::uint32_t teamId = 0;
};

inline const NarakaRuntimePropertyOptions &
DefaultNarakaRuntimePropertyOptions() {
  static const NarakaRuntimePropertyOptions options{};
  return options;
}

inline const NarakaManagerRvas &DefaultNarakaManagerRvas() {
  static const NarakaManagerRvas rvas{};
  return rvas;
}

inline bool ResolveNarakaCharacterManagerInstance(
    const IMemoryAccessor &mem, std::uintptr_t gameAssemblyBase,
    const NarakaManagerRvas &rvas, const NarakaCharacterManagerOffsets &offsets,
    std::uintptr_t &outClass, std::uintptr_t &outStaticFields,
    std::uintptr_t &outInstance) {
  outClass = 0;
  outStaticFields = 0;
  outInstance = 0;
  if (!IsCanonicalUserPtr(gameAssemblyBase)) {
    return false;
  }

  const std::uintptr_t slot =
      gameAssemblyBase + static_cast<std::uintptr_t>(rvas.characterManager);
  if (!ReadPtr(mem, slot, outClass) || !IsCanonicalUserPtr(outClass)) {
    return false;
  }
  if (!ReadPtr(mem, outClass + offsets.staticFields, outStaticFields) ||
      !IsCanonicalUserPtr(outStaticFields)) {
    return false;
  }
  if (!ReadPtr(mem, outStaticFields + offsets.instance, outInstance) ||
      !IsCanonicalUserPtr(outInstance)) {
    return false;
  }
  return true;
}

inline bool ResolveNarakaCharacterManagerInstance(
    const IMemoryAccessor &mem, std::uintptr_t gameAssemblyBase,
    std::uintptr_t &outInstance) {
  std::uintptr_t klass = 0;
  std::uintptr_t staticFields = 0;
  return ResolveNarakaCharacterManagerInstance(
      mem, gameAssemblyBase, DefaultNarakaManagerRvas(), {},
      klass, staticFields, outInstance);
}

inline bool ReadNarakaActorRuntimePropertyCandidate(
    const IMemoryAccessor &mem, std::uintptr_t actorModel,
    const NarakaRuntimePropertyOptions &options,
    NarakaActorRuntimePropertyCandidate &out) {
  out = NarakaActorRuntimePropertyCandidate{};
  if (!IsCanonicalUserPtr(actorModel)) {
    return false;
  }

  out.actorModel = actorModel;
  (void)ReadPtr(mem, actorModel + options.actor.propertyData, out.propertyData);
  (void)ReadPtr(mem, actorModel + options.actor.runtimePropertyData,
                out.runtimePropertyData);

  const std::uintptr_t heroBase =
      IsCanonicalUserPtr(out.propertyData) ? out.propertyData : actorModel;
  (void)ReadValue(mem, heroBase + options.actor.heroId, out.heroId);
  (void)ReadValue(mem, heroBase + options.actor.teamId, out.teamId);
  return IsCanonicalUserPtr(out.runtimePropertyData);
}

inline bool CaptureNarakaRuntimeProperties(
    const IMemoryAccessor &mem, std::uintptr_t actorModel,
    const NarakaRuntimePropertyOptions &options, std::size_t maxRows,
    NarakaRuntimePropertySnapshot &out) {
  out = NarakaRuntimePropertySnapshot{};
  out.actorModel = actorModel;

  NarakaActorRuntimePropertyCandidate candidate;
  if (!ReadNarakaActorRuntimePropertyCandidate(mem, actorModel, options,
                                               candidate)) {
    out.error = "failed to read ActorModel RuntimePropertyData";
    return false;
  }

  out.propertyData = candidate.propertyData;
  out.runtimePropertyData = candidate.runtimePropertyData;
  out.heroId = candidate.heroId;
  out.teamId = candidate.teamId;

  const auto &p = options.property;
  if (!ReadPtr(mem, out.runtimePropertyData + p.root, out.propertyRoot) ||
      !IsCanonicalUserPtr(out.propertyRoot)) {
    out.error = "failed to read RuntimePropertyData root";
    return false;
  }

  std::uintptr_t idRoot = 0;
  if (!ReadPtr(mem, out.propertyRoot + p.idRoot, idRoot) ||
      !IsCanonicalUserPtr(idRoot)) {
    out.error = "failed to read property id root";
    return false;
  }

  if (!ReadPtr(mem, idRoot + p.idEntries, out.idEntries) ||
      !IsCanonicalUserPtr(out.idEntries)) {
    out.error = "failed to read property id entries";
    return false;
  }

  if (!ReadValue(mem, out.idEntries + p.idCount, out.idCount) ||
      out.idCount <= 0 || out.idCount > 4096) {
    out.error = "invalid property id count";
    return false;
  }

  std::uintptr_t valuesRoot = 0;
  if (!ReadPtr(mem, out.propertyRoot + p.valuesRoot, valuesRoot) ||
      !IsCanonicalUserPtr(valuesRoot)) {
    out.error = "failed to read values root";
    return false;
  }

  if (!ReadPtr(mem, valuesRoot + p.valuesArray, out.valuesArray) ||
      !IsCanonicalUserPtr(out.valuesArray)) {
    out.error = "failed to read values array";
    return false;
  }

  const std::size_t rowLimit =
      std::min<std::size_t>(static_cast<std::size_t>(out.idCount), maxRows);
  out.rows.reserve(rowLimit);
  for (std::size_t i = 0; i < rowLimit; ++i) {
    const std::uintptr_t entry =
        out.idEntries + p.idFirstEntry + i * p.idEntryStride;

    NarakaRuntimePropertyRow row;
    if (!ReadValue(mem, entry + p.idEntryDataId, row.dataId) ||
        !ReadValue(mem, entry + p.idEntryIndex, row.index)) {
      continue;
    }
    if (row.dataId <= 0 || row.index < 0 || row.index > 1000000) {
      continue;
    }

    row.valueAddress = out.valuesArray + p.valuesFirst +
                       static_cast<std::uintptr_t>(row.index) * p.valueStride;
    row.valueReadOk = ReadValue(mem, row.valueAddress, row.intValue);
    row.uintValue = static_cast<std::uint32_t>(row.intValue);
    if (row.valueReadOk) {
      (void)ReadValue(mem, row.valueAddress, row.floatValue);
    }
    out.rows.push_back(row);
  }

  std::sort(out.rows.begin(), out.rows.end(),
            [](const NarakaRuntimePropertyRow &a,
               const NarakaRuntimePropertyRow &b) {
              if (a.dataId != b.dataId) {
                return a.dataId < b.dataId;
              }
              return a.index < b.index;
            });
  return true;
}

inline bool CaptureNarakaRuntimeProperties(
    const IMemoryAccessor &mem, std::uintptr_t actorModel, std::size_t maxRows,
    NarakaRuntimePropertySnapshot &out) {
  return CaptureNarakaRuntimeProperties(
      mem, actorModel, DefaultNarakaRuntimePropertyOptions(), maxRows, out);
}

inline std::vector<NarakaRuntimePropertyDiffRow>
DiffNarakaRuntimePropertySnapshots(
    const NarakaRuntimePropertySnapshot &baseline,
    const NarakaRuntimePropertySnapshot &current) {
  auto stableKey = [](std::int32_t dataId, std::int32_t index) {
    const auto hi = static_cast<std::uint64_t>(
        static_cast<std::uint32_t>(dataId));
    const auto lo = static_cast<std::uint32_t>(index);
    return (hi << 32) | lo;
  };

  std::unordered_map<std::uint64_t, const NarakaRuntimePropertyRow *> byDataId;
  byDataId.reserve(baseline.rows.size());
  for (const auto &row : baseline.rows) {
    byDataId[stableKey(row.dataId, row.index)] = &row;
  }

  std::vector<NarakaRuntimePropertyDiffRow> out;
  out.reserve(current.rows.size());
  for (const auto &row : current.rows) {
    NarakaRuntimePropertyDiffRow diff;
    diff.dataId = row.dataId;
    diff.index = row.index;
    diff.valueAddress = row.valueAddress;
    diff.currentInt = row.intValue;
    diff.currentFloat = row.floatValue;

    const auto it = byDataId.find(stableKey(row.dataId, row.index));
    if (it != byDataId.end() && it->second) {
      diff.hadBaseline = true;
      diff.baselineInt = it->second->intValue;
      diff.baselineFloat = it->second->floatValue;
      diff.intDelta = diff.currentInt - diff.baselineInt;
      diff.floatDelta = diff.currentFloat - diff.baselineFloat;
      diff.changed = diff.currentInt != diff.baselineInt;
    } else {
      diff.changed = true;
      diff.intDelta = diff.currentInt;
      diff.floatDelta = diff.currentFloat;
    }
    out.push_back(diff);
  }
  return out;
}

inline bool EnumerateNarakaActorModelsFromCharacterManager(
    const IMemoryAccessor &mem, std::uintptr_t characterManager,
    const NarakaCharacterManagerOffsets &offsets, std::size_t maxActors,
    std::vector<std::uintptr_t> &out, std::string *error = nullptr) {
  out.clear();
  if (error) {
    error->clear();
  }
  if (!IsCanonicalUserPtr(characterManager)) {
    if (error) {
      *error = "invalid CharacterManager pointer";
    }
    return false;
  }

  std::uintptr_t list = 0;
  if (!ReadPtr(mem, characterManager + offsets.allAliveCharList, list) ||
      !IsCanonicalUserPtr(list)) {
    if (error) {
      *error = "failed to read AllAliveCharList";
    }
    return false;
  }

  std::uintptr_t actorArray = 0;
  std::int32_t actorCount = 0;
  if (ReadPtr(mem, list + offsets.actorArray, actorArray) &&
      IsCanonicalUserPtr(actorArray) &&
      ReadValue(mem, actorArray + offsets.actorCount, actorCount) &&
      actorCount > 0 && actorCount <= 4096) {
    const std::size_t limit =
        std::min<std::size_t>(static_cast<std::size_t>(actorCount), maxActors);
    out.reserve(limit);
    for (std::size_t i = 0; i < limit; ++i) {
      std::uintptr_t actor = 0;
      if (ReadPtr(mem, actorArray + offsets.actorFirst +
                           i * offsets.actorStride,
                  actor) &&
          IsCanonicalUserPtr(actor)) {
        out.push_back(actor);
      }
    }
    if (!out.empty()) {
      return true;
    }
  }

  std::uintptr_t entries = 0;
  std::int32_t count = 0;
  if (!ReadPtr(mem, list + offsets.dictionaryEntries, entries) ||
      !ReadValue(mem, list + offsets.dictionaryCount, count) ||
      !IsCanonicalUserPtr(entries) || count <= 0 || count > 4096) {
    if (error) {
      *error = "invalid alive character dictionary";
    }
    return false;
  }

  const std::size_t limit =
      std::min<std::size_t>(static_cast<std::size_t>(count), maxActors);
  out.reserve(limit);
  for (std::size_t i = 0; i < limit; ++i) {
    const std::uintptr_t entry =
        entries + offsets.dictionaryFirstEntry + i * offsets.dictionaryEntryStride;
    std::uintptr_t actor = 0;
    if (ReadPtr(mem, entry + offsets.dictionaryValue, actor) &&
        IsCanonicalUserPtr(actor)) {
      out.push_back(actor);
    }
  }
  return true;
}

} // namespace er2
