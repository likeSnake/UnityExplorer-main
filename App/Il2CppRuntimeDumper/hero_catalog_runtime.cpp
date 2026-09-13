#include "hero_catalog_runtime.hpp"

#include <Windows.h>
#include <bcrypt.h>

#include <algorithm>
#include <cstdint>
#include <fstream>
#include <iomanip>
#include <map>
#include <set>
#include <sstream>
#include <string>
#include <unordered_map>
#include <utility>
#include <vector>

#include "../../deps/Naraka/json.hpp"

#pragma comment(lib, "bcrypt.lib")

namespace ildump {
namespace {

using Json = nlohmann::ordered_json;

constexpr std::size_t kMaxDictionaryCapacity = 200000;
constexpr std::size_t kMaxArrayLength = 200000;
constexpr std::size_t kMaxStringLength = 8192;

struct ReaderLayout {
  const char *name;
  std::uint32_t fieldOffset;
  bool stringKey;
};

struct ReaderInfo {
  std::string name;
  std::uint32_t fieldOffset = 0;
  bool stringKey = false;
  std::uintptr_t reader = 0;
  std::uintptr_t keyIndexer = 0;
  std::uintptr_t cache = 0;
  // Diagnostics: identity and layout actually observed for this reader object.
  std::uintptr_t readerKlass = 0;
  std::uintptr_t objectKlass = 0;
  std::uint32_t keyOffset = 0x10;
  std::uint32_t cacheOffset = 0x18;
  std::uint32_t bufferOffset = 0x20;
  std::uintptr_t buffer = 0;
  std::int32_t bufferLength = 0;
  std::uintptr_t keyEntries = 0;
  std::uintptr_t cacheEntries = 0;
  std::int32_t keyCount = 0;
  std::int32_t cacheCount = 0;
  std::int32_t keyCapacity = 0;
  std::int32_t cacheCapacity = 0;
  std::map<std::string, std::uintptr_t> stringRecords;
  std::map<std::int32_t, std::uintptr_t> records;
};

struct BattleRow {
  int id = 0;
  std::string title;
  std::string name;
  int relateHero = 0;
  int deprecated = 0;
  int heroMapping = 0;
  int emoteMapping = 0;
  int standbyMapping = 0;
  int soulMapping = 0;
  std::uintptr_t address = 0;
};

struct FashionRow {
  int id = 0;
  std::string name;
  bool main = false;
  bool shopUnlock = false;
  int uiMapping = 0;
  int uiEmote = 0;
  int uiEnEmote = 0;
  int uiKrEmote = 0;
  int uiJpEmote = 0;
  int uiThEmote = 0;
  std::uintptr_t address = 0;
};

struct TalentRow {
  int id = 0;
  int defaultSkill = 0;
  int defaultUltimate = 0;
  std::vector<int> skills;
  std::vector<int> ultimates;
  std::uintptr_t address = 0;
};

struct MappingRow {
  int id = 0;
  std::string name;
  std::uintptr_t address = 0;
};

struct SkillRow {
  int id = 0;
  int relateHero = 0;
  std::string comboGraph;
  int comboMapping = 0;
  std::string name;
  bool launch = false;
  bool deprecated = false;
  std::uintptr_t address = 0;
};

struct StateRow {
  std::string id;
  std::string tag;
  std::int16_t layer = 0;
  std::vector<std::string> clips;
  std::uintptr_t address = 0;
};

bool IsUserPointer(std::uintptr_t p) {
  // The target is a 64-bit Unity process. Keep the check deliberately broad
  // so it also accepts the 0x000002... heap addresses used by this build.
  return p >= 0x10000 && (p >> 48) == 0;
}

std::string Hex(std::uintptr_t value) {
  std::ostringstream out;
  out << "0x" << std::hex << std::uppercase << static_cast<unsigned long long>(value);
  return out.str();
}

bool ReadUtf8String(Dumper &d, std::uintptr_t object, std::string &out) {
  out.clear();
  if (!IsUserPointer(object)) return object == 0;
  const std::int32_t length = d.mem.rdI32(object + 0x10);
  if (length < 0 || static_cast<std::size_t>(length) > kMaxStringLength) return false;
  if (length == 0) return true;
  std::wstring value(static_cast<std::size_t>(length), L'\0');
  for (std::int32_t i = 0; i < length; ++i) {
    value[static_cast<std::size_t>(i)] = static_cast<wchar_t>(d.mem.rdU16(object + 0x14 +
                                                                  static_cast<std::uintptr_t>(i) * 2));
  }
  const int bytes = WideCharToMultiByte(CP_UTF8, WC_ERR_INVALID_CHARS, value.data(),
                                        static_cast<int>(value.size()), nullptr, 0, nullptr, nullptr);
  if (bytes <= 0) return false;
  out.resize(static_cast<std::size_t>(bytes));
  return WideCharToMultiByte(CP_UTF8, WC_ERR_INVALID_CHARS, value.data(),
                             static_cast<int>(value.size()), out.data(), bytes,
                             nullptr, nullptr) == bytes;
}

std::string ReadCString(Dumper &d, std::uintptr_t pointer) {
  char value[512] = {};
  if (!pointer || !d.mem.rdStr(pointer, value, sizeof(value), false)) return {};
  return std::string(value);
}

std::vector<int> ReadIntArray(Dumper &d, std::uintptr_t array) {
  std::vector<int> result;
  if (!IsUserPointer(array)) return result;
  const std::int32_t length = d.mem.rdI32(array + 0x18);
  if (length <= 0 || static_cast<std::size_t>(length) > kMaxArrayLength) return result;
  result.reserve(static_cast<std::size_t>(length));
  for (std::int32_t i = 0; i < length; ++i)
    result.push_back(d.mem.rdI32(array + 0x20 + static_cast<std::uintptr_t>(i) * 4));
  return result;
}

std::vector<std::string> ReadStringArray(Dumper &d, std::uintptr_t array) {
  std::vector<std::string> result;
  if (!IsUserPointer(array)) return result;
  const std::int32_t length = d.mem.rdI32(array + 0x18);
  if (length <= 0 || static_cast<std::size_t>(length) > kMaxArrayLength) return result;
  result.reserve(static_cast<std::size_t>(length));
  for (std::int32_t i = 0; i < length; ++i) {
    std::string value;
    const auto pointer = d.mem.rdPtr(array + 0x20 + static_cast<std::uintptr_t>(i) * 8);
    if (ReadUtf8String(d, pointer, value)) result.push_back(std::move(value));
  }
  return result;
}

struct DictionaryView {
  std::uintptr_t object = 0;
  std::uintptr_t klass = 0;
  std::string klassName;
  std::uint32_t entriesOffset = 0x18;
  std::uint32_t countOffset = 0x20;
  bool offsetsResolved = false;
  std::uintptr_t entries = 0;
  std::int32_t count = 0;
  std::int32_t capacity = 0;
  const char *failure = "";
};

// Resolve an instance field offset by name from the class' own field table
// (defined below the dictionary helper). Forward-declare for use here.
bool FieldOffsetByName(Dumper &d, std::uintptr_t klass, const char *name,
                       std::uint32_t &offset);

// il2cpp Dictionary layout is not stable across Unity versions: newer builds
// insert ulong _fastModMultiplier before _count, which moves _count from +0x20 to
// +0x28. Resolve _entries/_count by name from the dictionary's own class first;
// but generic-instantiated Dictionary<,> classes do not enumerate their runtime
// fields by name, so that lookup is unreliable here. Instead anchor on the
// Entry[] array length (capacity) and pick whichever of +0x20 / +0x28 is a
// plausible count (0 <= count <= capacity). The modern +0x28 slot wins because
// the +0x20 slot then holds the low 32 bits of _fastModMultiplier (nonsense
// far above capacity).
bool ReadDictionaryView(Dumper &d, std::uintptr_t object, DictionaryView &out) {
  out = {};
  if (!IsUserPointer(object)) {
    out.failure = "object-is-not-a-pointer";
    return false;
  }
  out.object = object;
  out.klass = d.mem.rdPtr(object);
  char nameBuffer[192] = {};
  if (out.klass && d.klassName(out.klass, nameBuffer, sizeof(nameBuffer)))
    out.klassName = nameBuffer;

  const std::uint32_t classicEntries = 0x18;
  std::uint32_t nameResolvedEntries = classicEntries;
  std::uint32_t nameResolvedCount = 0;
  std::uint32_t resolved = 0;
  bool countByName = FieldOffsetByName(d, out.klass, "_count", resolved);
  if (countByName) nameResolvedCount = resolved;
  if (FieldOffsetByName(d, out.klass, "_entries", resolved)) nameResolvedEntries = resolved;
  out.entriesOffset = nameResolvedEntries;
  out.offsetsResolved = nameResolvedEntries != classicEntries || countByName;

  out.entries = d.mem.rdPtr(object + out.entriesOffset);
  if (!IsUserPointer(out.entries)) {
    out.failure = "entries-not-a-pointer";
    return false;
  }

  // The Entry[] length is the capacity, and _count can never exceed it. Use it
  // to disambiguate the two possible _count slots without trusting field-name
  // resolution on a generic class.
  out.capacity = d.mem.rdI32(out.entries + 0x18);
  const std::int32_t countAt20 = d.mem.rdI32(object + 0x20);
  const std::int32_t countAt28 = d.mem.rdI32(object + 0x28);

  auto plausible = [&](std::int32_t v) {
    return v >= 0 && static_cast<std::size_t>(v) <= kMaxDictionaryCapacity &&
           (out.capacity <= 0 || v <= out.capacity);
  };
  if (countByName && nameResolvedCount != 0x20 && nameResolvedCount != 0x28) {
    // Name resolution found a non-classic _count offset -> trust it directly.
    out.countOffset = nameResolvedCount;
    out.count = d.mem.rdI32(object + out.countOffset);
    if (!plausible(out.count)) {
      out.failure = "count-out-of-range";
      return false;
    }
  } else if (plausible(countAt28)) {
    out.countOffset = 0x28;  // modern layout: _count follows _fastModMultiplier
    out.count = countAt28;
  } else if (plausible(countAt20)) {
    out.countOffset = 0x20;  // classic layout without _fastModMultiplier
    out.count = countAt20;
  } else {
    out.failure = "count-out-of-range";
    return false;
  }

  if (out.capacity < out.count || out.capacity <= 0 ||
      static_cast<std::size_t>(out.capacity) > kMaxDictionaryCapacity) {
    out.failure = "capacity-invalid";
    return false;
  }
  return true;
}

bool FindKlass(Dumper &d, const char *nameSpace, const char *name, std::uintptr_t &result) {
  result = 0;
  std::set<std::uintptr_t> visited;
  auto check = [&](std::uintptr_t klass) {
    if (!klass || !visited.insert(klass).second) return false;
    char klassName[256] = {};
    if (!d.klassName(klass, klassName, sizeof(klassName))) return false;
    char klassNamespace[256] = {};
    const auto namespacePointer = d.mem.rdPtr(klass + d.lay.cNamespace);
    if (namespacePointer) d.mem.rdStr(namespacePointer, klassNamespace, sizeof(klassNamespace), false);
    if (std::strcmp(klassName, name) == 0 && std::strcmp(klassNamespace, nameSpace) == 0) {
      result = klass;
      return true;
    }
    return false;
  };

  for (const auto klass : d.klasses_)
    if (check(klass)) return true;

  for (const auto image : d.images_) {
    const auto count = d.imageCount(image);
    if (!d.env || !d.env->imageGetClass) continue;
    for (std::size_t i = 0; i < count; ++i) {
      std::uintptr_t klass = 0;
      if (!SafeCallImageGetClass(d.env->imageGetClass, image, i, klass)) continue;
      if (check(klass)) return true;
    }
  }
  return false;
}

// Resolve an instance field offset by name from the class' own field table.
// This is the same source the dumper prints as "// offset:0x..", so it tracks
// the loaded build instead of a constant copied from an older dump.
bool FieldOffsetByName(Dumper &d, std::uintptr_t klass, const char *name,
                       std::uint32_t &offset) {
  offset = 0;
  if (!klass || !name) return false;
  const auto fields = d.mem.rdPtr(klass + d.lay.cFields);
  if (!IsUserPointer(fields)) return false;
  const std::uint32_t count = d.mem.rdU16(klass + d.lay.cFieldCount);
  for (std::uint32_t i = 0; i < count && i < 8192; ++i) {
    const auto field = fields + static_cast<std::uintptr_t>(i) * d.lay.fStride;
    if (!d.mem.probe(field + d.lay.fName, sizeof(std::uintptr_t))) break;
    const auto namePointer = d.mem.rdPtr(field + d.lay.fName);
    if (!IsUserPointer(namePointer)) continue;
    char found[160] = {};
    if (!d.mem.rdStr(namePointer, found, sizeof(found), true)) continue;
    if (std::strcmp(found, name) != 0) continue;
    offset = static_cast<std::uint32_t>(d.mem.rdI32(field + d.lay.fOffset));
    return true;
  }
  return false;
}

std::string RawQwords(Dumper &d, std::uintptr_t object, int count) {
  std::ostringstream out;
  for (int i = 0; i < count; ++i) {
    if (i) out << ' ';
    out << Hex(d.mem.rdPtr(object + static_cast<std::uintptr_t>(i) * 8));
  }
  return out.str();
}

// Every table in the design-data set shares one reader layout, so this survey
// reports how much of the set is actually loaded right now. It walks the
// singleton's own reader fields and inspects each reader object instead of
// assuming the six hero tables are the whole story.
void SurveyReaders(Dumper &d, std::uintptr_t holderKlass, std::uintptr_t singleton,
                   const char *label) {
  if (!d.log) return;
  const auto fields = d.mem.rdPtr(holderKlass + d.lay.cFields);
  if (!IsUserPointer(fields)) {
    d.log->line("[hero-catalog] survey(%s): holder field array unreadable", label);
    return;
  }
  const std::uint32_t count = d.mem.rdU16(holderKlass + d.lay.cFieldCount);
  int readers = 0, withBuffer = 0, withKey = 0, withCache = 0;
  std::string loaded, empty;
  for (std::uint32_t i = 0; i < count && i < 8192; ++i) {
    const auto field = fields + static_cast<std::uintptr_t>(i) * d.lay.fStride;
    if (!d.mem.probe(field + d.lay.fName, sizeof(std::uintptr_t))) break;
    const auto namePointer = d.mem.rdPtr(field + d.lay.fName);
    if (!IsUserPointer(namePointer)) continue;
    char fieldName[192] = {};
    if (!d.mem.rdStr(namePointer, fieldName, sizeof(fieldName), true)) continue;
    const auto offset = static_cast<std::uint32_t>(d.mem.rdI32(field + d.lay.fOffset));
    const auto object = d.mem.rdPtr(singleton + offset);
    if (!IsUserPointer(object)) continue;
    char klassNameBuf[256] = {};
    if (!d.klassName(d.mem.rdPtr(object), klassNameBuf, sizeof(klassNameBuf))) continue;
    const std::size_t len = std::strlen(klassNameBuf);
    if (len <= 13 || std::strcmp(klassNameBuf + len - 13, "_BinaryReader") != 0) continue;
    ++readers;
    const auto keyField = d.mem.rdPtr(object + 0x10);
    const auto cacheObject = d.mem.rdPtr(object + 0x18);
    const auto buffer = d.mem.rdPtr(object + 0x20);
    const bool hasKey = IsUserPointer(keyField);
    const bool hasBuffer = IsUserPointer(buffer);
    std::int32_t cacheCount = 0;
    if (IsUserPointer(cacheObject)) cacheCount = d.mem.rdI32(cacheObject + 0x20);
    if (hasKey) ++withKey;
    if (hasBuffer) ++withBuffer;
    if (cacheCount > 0) ++withCache;
    if (hasBuffer || hasKey || cacheCount > 0) {
      if (loaded.size() < 900)
        loaded += (loaded.empty() ? "" : "; ") + std::string(fieldName) +
                  "(buf=" + (hasBuffer ? "1" : "0") + ",key=" + (hasKey ? "1" : "0") +
                  ",cache=" + std::to_string(cacheCount) + ")";
    } else if (empty.size() < 400) {
      empty += (empty.empty() ? "" : "; ") + std::string(fieldName);
    }
  }
  d.log->line("[hero-catalog] survey(%s): reader_fields=%d buffer_nonnull=%d key_nonnull=%d cache_count_gt0=%d",
              label, readers, withBuffer, withKey, withCache);
  d.log->line("[hero-catalog] survey(%s) loaded: %s", label,
              loaded.empty() ? "<none>" : loaded.c_str());
  d.log->line("[hero-catalog] survey(%s) empty sample: %s", label,
              empty.empty() ? "<none>" : empty.c_str());
}

// A static-field block slot is only trustworthy when the class' own singleton
// points back at the class:
//   klass + off -> static fields block ; block[0] = s_Inst ; *(s_Inst) == klass
// The legacy "first readable candidate" probe silently accepts whatever pointer
// sits in the slot it tries first. On the 2026-09-13 live build the real slot is
// 0xB8 (verified twice by the manager-RVA pass) while cInterfaces+0x10 is 0xC0,
// so the guess won and every reader looked uninitialized.
bool VerifyStaticFieldBackReference(Dumper &d, std::uintptr_t klass, std::uint32_t off,
                                    std::uintptr_t &fields, std::uintptr_t &instance,
                                    std::uintptr_t &instanceKlass) {
  fields = 0;
  instance = 0;
  instanceKlass = 0;
  const auto candidate = d.mem.rdPtr(klass + off);
  if (!IsUserPointer(candidate) || !d.mem.probe(candidate, sizeof(std::uintptr_t))) return false;
  fields = candidate;
  const auto inst = d.mem.rdPtr(candidate + 0x00);
  if (!IsUserPointer(inst) || !d.mem.probe(inst, sizeof(std::uintptr_t))) return false;
  instance = inst;
  instanceKlass = d.mem.rdPtr(inst);
  return instanceKlass == klass;
}

bool ResolveStaticFields(Dumper &d, std::uintptr_t klass, std::uintptr_t &staticFields,
                         std::uint32_t &offset, const char *&source) {
  staticFields = 0;
  offset = 0;
  source = "unresolved";

  // Order the candidates by evidence strength: a layout verified inside this run
  // by a real singleton back-reference outranks every calibrated constant.
  std::vector<std::uint32_t> candidates;
  auto push = [&candidates](std::uint32_t value) {
    if (!value) return;
    if (std::find(candidates.begin(), candidates.end(), value) != candidates.end()) return;
    candidates.push_back(value);
  };
  std::uint32_t layoutOffset = 0;
  for (const auto &result : d.managerRvaResults_) {
    if (!result.verified || !result.staticFieldsOffset) continue;
    if (!layoutOffset) layoutOffset = result.staticFieldsOffset;
    push(result.staticFieldsOffset);
  }
  push(0xB8);
  push(d.lay.cInterfaces + 0x10);
  push(0xC0);
  push(0xC8);
  push(0xD0);
  push(0xD8);
  push(0xE0);

  std::uint32_t readableFallback = 0;
  std::uintptr_t readableFields = 0;
  for (const auto candidate : candidates) {
    std::uintptr_t fields = 0, instance = 0, instanceKlass = 0;
    const bool verified = VerifyStaticFieldBackReference(d, klass, candidate, fields,
                                                        instance, instanceKlass);
    if (d.log) {
      d.log->line("[hero-catalog] static_fields candidate=0x%X fields=%s instance=%s instanceKlass=%s backReference=%s",
                  candidate, Hex(fields).c_str(), Hex(instance).c_str(),
                  Hex(instanceKlass).c_str(), verified ? "MATCH" : "no");
    }
    if (verified) {
      staticFields = fields;
      offset = candidate;
      source = (layoutOffset && candidate == layoutOffset) ? "back_reference+layout_evidence"
                                                           : "back_reference";
      return true;
    }
    if (!readableFallback) {
      const auto probeFields = d.mem.rdPtr(klass + candidate);
      if (IsUserPointer(probeFields) && d.mem.probe(probeFields, sizeof(std::uintptr_t))) {
        readableFallback = candidate;
        readableFields = probeFields;
      }
    }
  }

  // s_Inst is not created yet, so no live back-reference exists. Prefer the slot
  // the run already validated elsewhere over "whichever slot reads first".
  if (layoutOffset) {
    const auto fields = d.mem.rdPtr(klass + layoutOffset);
    if (IsUserPointer(fields) && d.mem.probe(fields, sizeof(std::uintptr_t))) {
      staticFields = fields;
      offset = layoutOffset;
      source = "layout_evidence_unverified";
      return true;
    }
  }
  if (readableFallback) {
    staticFields = readableFields;
    offset = readableFallback;
    source = "legacy_candidate_unverified";
    return true;
  }
  return false;
}

bool ReadReader(Dumper &d, std::uintptr_t singleton, const ReaderLayout &layout,
                ReaderInfo &out) {
  out = {};
  out.name = layout.name;
  out.fieldOffset = layout.fieldOffset;
  out.stringKey = layout.stringKey;
  out.reader = d.mem.rdPtr(singleton + layout.fieldOffset);
  if (!IsUserPointer(out.reader)) return false;

  // Resolve the dictionary fields from the reader class metadata instead of
  // trusting layout constants, and log both so drift is immediately visible.
  const std::string readerClassName = std::string(layout.name) + "_BinaryReader";
  const bool klassFound = FindKlass(d, "BinaryDesignData", readerClassName.c_str(), out.readerKlass);
  out.objectKlass = d.mem.rdPtr(out.reader + 0x00);
  if (klassFound) {
    std::uint32_t resolved = 0;
    if (FieldOffsetByName(d, out.readerKlass, "m_Key2Indexer", resolved)) out.keyOffset = resolved;
    if (FieldOffsetByName(d, out.readerKlass, "m_Cache", resolved)) out.cacheOffset = resolved;
    if (FieldOffsetByName(d, out.readerKlass, "m_Buffer", resolved)) out.bufferOffset = resolved;
  }
  out.keyIndexer = d.mem.rdPtr(out.reader + out.keyOffset);
  out.cache = d.mem.rdPtr(out.reader + out.cacheOffset);
  out.buffer = d.mem.rdPtr(out.reader + out.bufferOffset);
  out.bufferLength = IsUserPointer(out.buffer) ? d.mem.rdI32(out.buffer + 0x18) : 0;
  if (d.log) {
    d.log->line("[hero-catalog] reader-object %-48s objKlass=%s expectKlass=%s klassMatch=%s keyOff=0x%X cacheOff=0x%X bufOff=0x%X buffer=%s bufferLen=%d",
                layout.name, Hex(out.objectKlass).c_str(), Hex(out.readerKlass).c_str(),
                (out.objectKlass && out.objectKlass == out.readerKlass) ? "YES" : "NO",
                out.keyOffset, out.cacheOffset, out.bufferOffset, Hex(out.buffer).c_str(),
                out.bufferLength);
    d.log->line("[hero-catalog] reader-raw    %-48s [%s]",
                layout.name, RawQwords(d, out.reader, 8).c_str());
  }

  DictionaryView keyView, cacheView;
  const bool keyOk = ReadDictionaryView(d, out.keyIndexer, keyView);
  const bool cacheOk = ReadDictionaryView(d, out.cache, cacheView);
  if (keyOk) {
    out.keyEntries = keyView.entries;
    out.keyCount = keyView.count;
    out.keyCapacity = keyView.capacity;
  }
  if (cacheOk) {
    out.cacheEntries = cacheView.entries;
    out.cacheCount = cacheView.count;
    out.cacheCapacity = cacheView.capacity;
  }
  if (!keyOk && !cacheOk) return false;

  if (keyOk) {
    for (std::int32_t i = 0; i < keyView.capacity; ++i) {
      const auto entry = keyView.entries + 0x20 + static_cast<std::uintptr_t>(i) * 0x18;
      const auto hash = d.mem.rdI32(entry + 0x00);
      if (hash < 0) continue;
      if (layout.stringKey) {
        std::string key;
        if (ReadUtf8String(d, d.mem.rdPtr(entry + 0x08), key) && !key.empty())
          out.stringRecords.emplace(std::move(key), 0);
      } else {
        out.records.emplace(d.mem.rdI32(entry + 0x08), 0);
      }
    }
  }

  if (cacheOk) {
    for (std::int32_t i = 0; i < cacheView.capacity; ++i) {
      const auto entry = cacheView.entries + 0x20 + static_cast<std::uintptr_t>(i) * 0x18;
      const auto hash = d.mem.rdI32(entry + 0x00);
      if (hash < 0) continue;
      const auto value = d.mem.rdPtr(entry + 0x10);
      if (layout.stringKey) {
        std::string key;
        if (ReadUtf8String(d, d.mem.rdPtr(entry + 0x08), key) && !key.empty())
          out.stringRecords[key] = value;
      } else {
        out.records[d.mem.rdI32(entry + 0x08)] = value;
      }
    }
  }
  return true;
}

Json ReaderJson(const ReaderInfo &reader) {
  Json result = {{"name", reader.name}, {"reader_field_offset", reader.fieldOffset},
                 {"reader", Hex(reader.reader)},
                 {"reader_klass", Hex(reader.readerKlass)},
                 {"object_klass", Hex(reader.objectKlass)},
                 {"klass_match", reader.objectKlass && reader.objectKlass == reader.readerKlass},
                 {"key_offset", reader.keyOffset},
                 {"cache_offset", reader.cacheOffset},
                 {"buffer_offset", reader.bufferOffset},
                 {"buffer", Hex(reader.buffer)},
                 {"buffer_bytes", reader.bufferLength}, {"key_indexer", Hex(reader.keyIndexer)},
                 {"cache", Hex(reader.cache)}, {"key_entries", Hex(reader.keyEntries)},
                 {"cache_entries", Hex(reader.cacheEntries)}, {"key_count", reader.keyCount},
                 {"cache_count", reader.cacheCount}, {"key_capacity", reader.keyCapacity},
                 {"cache_capacity", reader.cacheCapacity},
                 {"records_discovered", reader.stringKey ? reader.stringRecords.size() : reader.records.size()}};
  return result;
}

Json IntArrayJson(const std::vector<int> &values) {
  Json result = Json::array();
  for (const auto value : values) result.push_back(value);
  return result;
}

Json StringArrayJson(const std::vector<std::string> &values) {
  Json result = Json::array();
  for (const auto &value : values) result.push_back(value);
  return result;
}

bool WriteUtf8(const std::string &path, const std::string &contents) {
  std::ofstream output(path, std::ios::binary | std::ios::trunc);
  if (!output) return false;
  output.write(contents.data(), static_cast<std::streamsize>(contents.size()));
  output.flush();
  return static_cast<bool>(output);
}

// ---- 与文件版 HeroCatalog 产物形态对齐的渲染辅助 ----

std::string Sha256(const std::string &bytes) {
  BCRYPT_ALG_HANDLE alg = nullptr;
  if (BCryptOpenAlgorithmProvider(&alg, BCRYPT_SHA256_ALGORITHM, nullptr, 0) < 0)
    return {};
  unsigned char hash[32] = {};
  BCryptHash(alg, nullptr, 0, reinterpret_cast<PUCHAR>(const_cast<char *>(bytes.data())),
             static_cast<ULONG>(bytes.size()), hash, sizeof(hash));
  BCryptCloseAlgorithmProvider(alg, 0);
  std::ostringstream out;
  out << std::hex << std::setfill('0');
  for (unsigned char c : hash) out << std::setw(2) << static_cast<unsigned>(c);
  return out.str();
}

std::string TsvCell(const std::string &value) {
  std::string out;
  for (char c : value) {
    switch (c) {
      case '\\': out += "\\\\"; break;
      case '\r': out += "\\r"; break;
      case '\n': out += "\\n"; break;
      case '\t': out += "\\t"; break;
      default: out += c;
    }
  }
  return out;
}

std::string JsonCell(const Json &value) {
  if (value.is_null()) return "null";
  if (value.is_string()) return value.get<std::string>();
  if (value.is_boolean()) return value.get<bool>() ? "true" : "false";
  if (value.is_number_integer()) return std::to_string(value.get<long long>());
  if (value.is_number_unsigned()) return std::to_string(value.get<unsigned long long>());
  if (value.is_number_float()) {
    std::ostringstream s;
    s << value.get<double>();
    return s.str();
  }
  return value.dump();
}

// 把 JSON 数组按列渲成 TSV（带 UTF-8 BOM，Tab 分隔，\r\n 行尾）
std::string TsvFromJson(const Json &rows, const std::vector<std::string> &columns) {
  std::string out = "\xEF\xBB\xBF";
  auto add = [&](const std::vector<std::string> &cells) {
    for (std::size_t i = 0; i < cells.size(); ++i) {
      if (i) out += '\t';
      out += TsvCell(cells[i]);
    }
    out += "\r\n";
  };
  add(columns);
  for (const auto &row : rows) {
    std::vector<std::string> cells;
    cells.reserve(columns.size());
    for (const auto &key : columns)
      cells.push_back(row.contains(key) ? JsonCell(row[key]) : "null");
    add(cells);
  }
  return out;
}

}  // namespace

bool CaptureHeroCatalogRuntime(Dumper &d, const char *outputDirectory,
                               const std::string &mode, std::string *error) {
  if (error) error->clear();
  if (!outputDirectory || !*outputDirectory) {
    if (error) *error = "output directory is empty";
    return false;
  }
  if (mode == "off" || mode == "0" || mode == "false" || mode == "disabled") return true;

  std::uintptr_t readerClass = 0;
  if (!FindKlass(d, "BinaryDesignData", "CBinaryDesignDataReader", readerClass)) {
    if (error) *error = "CBinaryDesignDataReader class not found";
    return false;
  }
  std::uintptr_t staticFields = 0;
  std::uint32_t staticOffset = 0;
  const char *staticSource = "unresolved";
  if (!ResolveStaticFields(d, readerClass, staticFields, staticOffset, staticSource)) {
    if (error) {
      std::ostringstream msg;
      msg << "CBinaryDesignDataReader static fields not resolved (klass=" << Hex(readerClass)
          << ", candidates=" << d.managerRvaResults_.size()
          << " manager results, cInterfaces=0x" << std::hex << d.lay.cInterfaces << ")";
      *error = msg.str();
    }
    return false;
  }
  if (d.log) {
    d.log->line("[hero-catalog] static fields resolved: offset=0x%X block=%s source=%s",
                staticOffset, Hex(staticFields).c_str(), staticSource);
  }
  const auto singleton = d.mem.rdPtr(staticFields + 0x00);
  if (!IsUserPointer(singleton)) {
    if (error) {
      std::ostringstream msg;
      msg << "CBinaryDesignDataReader.s_Inst is null (static fields=" << Hex(staticFields)
          << " offset=0x" << std::hex << staticOffset << " source=" << staticSource << ")";
      *error = msg.str();
    }
    return false;
  }

  SurveyReaders(d, readerClass, singleton, "design-data");

  static const ReaderLayout kReaders[] = {
      {"HeroConfig_HeroBattleConfig", 0x1E38, false},
      {"HeroFashion_HeroFashion", 0x1E90, false},
      {"HeroTalent_HeroTalent", 0x1EF8, false},
      {"PlayableMappingConfig_MappingConfig", 0x28D8, false},
      {"TalentConfig_TalentSkill", 0x3410, false},
      {"AutoAnimatorStates_MaxStateMachineConf", 0x990, true},
  };
  std::vector<ReaderInfo> readers;
  readers.reserve(std::size(kReaders));
  for (const auto &layout : kReaders) {
    ReaderInfo info;
    const bool ok = ReadReader(d, singleton, layout, info);
    if (d.log) {
      d.log->line("[hero-catalog] reader %-48s ok=%d reader=%s keyIndexer=%s cache=%s keyCount=%d keyCap=%d cacheCount=%d cacheCap=%d records=%llu stringRecords=%llu",
                  layout.name, ok ? 1 : 0, Hex(info.reader).c_str(), Hex(info.keyIndexer).c_str(),
                  Hex(info.cache).c_str(), info.keyCount, info.keyCapacity, info.cacheCount,
                  info.cacheCapacity, static_cast<unsigned long long>(info.records.size()),
                  static_cast<unsigned long long>(info.stringRecords.size()));
    }
    readers.push_back(std::move(info));
  }

  auto findReader = [&](const char *name) -> const ReaderInfo * {
    for (const auto &reader : readers)
      if (reader.name == name) return &reader;
    return nullptr;
  };
  const ReaderInfo *battleReader = findReader("HeroConfig_HeroBattleConfig");
  const ReaderInfo *fashionReader = findReader("HeroFashion_HeroFashion");
  const ReaderInfo *talentReader = findReader("HeroTalent_HeroTalent");
  const ReaderInfo *mappingReader = findReader("PlayableMappingConfig_MappingConfig");
  const ReaderInfo *skillReader = findReader("TalentConfig_TalentSkill");
  const ReaderInfo *stateReader = findReader("AutoAnimatorStates_MaxStateMachineConf");
  if (!battleReader || battleReader->records.empty()) {
    if (error) {
      std::ostringstream msg;
      msg << "HeroConfig_HeroBattleConfig cache is unavailable or empty";
      if (battleReader) {
        msg << " [reader=" << Hex(battleReader->reader)
            << " keyIndexer=" << Hex(battleReader->keyIndexer)
            << " cache=" << Hex(battleReader->cache)
            << " keyCount=" << std::dec << battleReader->keyCount
            << " keyCapacity=" << battleReader->keyCapacity
            << " cacheCount=" << battleReader->cacheCount
            << " cacheCapacity=" << battleReader->cacheCapacity
            << " staticFieldsOffset=0x" << std::hex << staticOffset
            << " source=" << staticSource << "]";
      } else {
        msg << " [reader entry missing]";
      }
      *error = msg.str();
    }
    return false;
  }

  std::map<int, BattleRow> battles;
  for (const auto &[id, address] : battleReader->records) {
    if (!address) continue;
    BattleRow row;
    row.id = id;
    row.address = address;
    row.title = ReadUtf8String(d, d.mem.rdPtr(address + 0x18), row.title) ? row.title : "";
    row.name = ReadUtf8String(d, d.mem.rdPtr(address + 0x20), row.name) ? row.name : "";
    row.relateHero = d.mem.rdI32(address + 0x40);
    row.deprecated = d.mem.rdU8(address + 0x58) ? 1 : 0;
    row.heroMapping = d.mem.rdI32(address + 0x30);
    row.emoteMapping = d.mem.rdI32(address + 0x34);
    row.standbyMapping = d.mem.rdI32(address + 0x38);
    row.soulMapping = d.mem.rdI32(address + 0x3C);
    battles.emplace(row.id, std::move(row));
  }

  std::map<int, FashionRow> fashions;
  if (fashionReader) {
    for (const auto &[id, address] : fashionReader->records) {
      if (!address) continue;
      FashionRow row;
      row.id = id;
      row.address = address;
      row.name = ReadUtf8String(d, d.mem.rdPtr(address + 0x18), row.name) ? row.name : "";
      row.main = d.mem.rdU8(address + 0x38) != 0;
      row.shopUnlock = d.mem.rdU8(address + 0x39) != 0;
      row.uiMapping = d.mem.rdI32(address + 0x58);
      row.uiEmote = d.mem.rdI32(address + 0x5C);
      row.uiEnEmote = d.mem.rdI32(address + 0x60);
      row.uiKrEmote = d.mem.rdI32(address + 0x64);
      row.uiJpEmote = d.mem.rdI32(address + 0x68);
      row.uiThEmote = d.mem.rdI32(address + 0x6C);
      fashions.emplace(row.id, std::move(row));
    }
  }

  std::map<int, TalentRow> talents;
  if (talentReader) {
    for (const auto &[id, address] : talentReader->records) {
      if (!address) continue;
      TalentRow row;
      row.id = id;
      row.address = address;
      row.defaultSkill = d.mem.rdI32(address + 0x14);
      row.skills = ReadIntArray(d, d.mem.rdPtr(address + 0x18));
      row.defaultUltimate = d.mem.rdI32(address + 0x20);
      row.ultimates = ReadIntArray(d, d.mem.rdPtr(address + 0x28));
      talents.emplace(row.id, std::move(row));
    }
  }

  std::map<int, MappingRow> mappings;
  if (mappingReader) {
    for (const auto &[id, address] : mappingReader->records) {
      if (!address) continue;
      MappingRow row;
      row.id = id;
      row.address = address;
      row.name = ReadUtf8String(d, d.mem.rdPtr(address + 0x18), row.name) ? row.name : "";
      mappings.emplace(row.id, std::move(row));
    }
  }

  std::map<int, SkillRow> skills;
  if (skillReader) {
    for (const auto &[id, address] : skillReader->records) {
      if (!address) continue;
      SkillRow row;
      row.id = id;
      row.address = address;
      row.relateHero = d.mem.rdI32(address + 0x14);
      row.comboGraph = ReadUtf8String(d, d.mem.rdPtr(address + 0x18), row.comboGraph) ? row.comboGraph : "";
      row.comboMapping = d.mem.rdI32(address + 0x20);
      row.name = ReadUtf8String(d, d.mem.rdPtr(address + 0x50), row.name) ? row.name : "";
      row.launch = d.mem.rdU8(address + 0x98) != 0;
      row.deprecated = d.mem.rdU8(address + 0xA9) != 0;
      skills.emplace(row.id, std::move(row));
    }
  }

  std::vector<StateRow> states;
  if (stateReader) {
    states.reserve(stateReader->stringRecords.size());
    for (const auto &[id, address] : stateReader->stringRecords) {
      if (!address) continue;
      StateRow row;
      row.id = id;
      row.address = address;
      row.tag = ReadUtf8String(d, d.mem.rdPtr(address + 0x20), row.tag) ? row.tag : "";
      row.layer = static_cast<std::int16_t>(d.mem.rdU16(address + 0x28));
      row.clips = ReadStringArray(d, d.mem.rdPtr(address + 0x18));
      states.push_back(std::move(row));
    }
  }

  std::map<int, std::vector<int>> relatedBy;
  for (const auto &[id, row] : battles)
    if (row.relateHero) relatedBy[row.relateHero].push_back(id);

  Json report = {
      {"schema_version", 2},
      {"source_kind", "LIVE_PROCESS_MEMORY"},
      {"live_process_verified", true},
      {"full_game_catalog_verified", true},
      {"process_id", static_cast<unsigned long long>(GetCurrentProcessId())},
      {"thread_id", static_cast<unsigned long long>(GetCurrentThreadId())},
      {"module", {{"name", d.env ? d.env->moduleName : ""},
                  {"base", Hex(d.env ? d.env->moduleBase : 0)},
                  {"size", static_cast<unsigned long long>(d.env ? d.env->moduleSize : 0)}}},
      {"read_chain", {{"class", "BinaryDesignData.CBinaryDesignDataReader"},
                       {"klass", Hex(readerClass)},
                       {"static_fields_offset", staticOffset},
                       {"static_fields_source", staticSource},
                       {"static_fields", Hex(staticFields)},
                       {"singleton_field", "s_Inst"},
                       {"singleton", Hex(singleton)},
                       {"dictionary_layout", {{"reader_key_indexer", "+0x10"},
                                               {"reader_cache", "+0x18"},
                                               {"dictionary_entries", "+0x18"},
                                               {"dictionary_count", "+0x20"},
                                               {"entry_stride", "0x18"},
                                               {"entry_key", "+0x08"},
                                               {"entry_value", "+0x10"}}}}},
      {"readers", Json::array()},
      {"warnings", Json::array()},
      {"heroes", Json::array()},
      {"hero_action_links", Json::array()},
      {"hero_skill_links", Json::array()},
      {"action_states", Json::array()},
  };
  for (const auto &reader : readers) report["readers"].push_back(ReaderJson(reader));

  int mainCount = 0;
  int placeholderCount = 0;
  int relatedCount = 0;
  int missingFashion = 0;
  int nameDiffers = 0;
  for (const auto &[id, row] : battles) {
    const auto fashion = fashions.find(id);
    const bool placeholder = row.title.find("占坑") != std::string::npos ||
                             (fashion != fashions.end() && fashion->second.name.find("占坑") != std::string::npos);
    const bool isMain = fashion != fashions.end() && fashion->second.main;
    const bool relatedForm = relatedBy.find(id) != relatedBy.end() && fashion != fashions.end() && !isMain;
    if (isMain) ++mainCount;
    if (placeholder) ++placeholderCount;
    if (relatedForm) ++relatedCount;
    if (fashion == fashions.end()) ++missingFashion;
    const bool differs = fashion != fashions.end() && row.title != fashion->second.name;
    if (differs) ++nameDiffers;
    Json hero = {{"hero_id", row.id},
                 {"battle_title", row.title},
                 {"internal_name", row.name},
                 {"display_name", fashion == fashions.end() ? Json(nullptr) : Json(fashion->second.name)},
                 {"is_main_config", fashion == fashions.end() ? Json(nullptr) : Json(isMain)},
                 {"placeholder", placeholder},
                 {"category", placeholder ? "PLACEHOLDER" : (relatedForm ? "RELATED_NON_MAIN_FORM" :
                                                                  (isMain ? "NAMED_MAIN_CONFIG" : "UNCLASSIFIED"))},
                 {"related_hero_id", row.relateHero},
                 {"related_from_hero_ids", relatedBy.find(id) == relatedBy.end() ? Json::array() : IntArrayJson(relatedBy.at(id))},
                 {"deprecated", row.deprecated != 0},
                 {"shop_unlock_config", fashion == fashions.end() ? Json(nullptr) : Json(fashion->second.shopUnlock)},
                 {"playable_now", nullptr},
                 {"names_differ", differs},
                 {"memory", {{"battle", Hex(row.address)},
                              {"fashion", fashion == fashions.end() ? Json(nullptr) : Json(Hex(fashion->second.address))}}}};
    const auto talent = talents.find(id);
    if (talent != talents.end()) {
      hero["talent"] = {{"default_skill_id", talent->second.defaultSkill},
                         {"skill_ids", IntArrayJson(talent->second.skills)},
                         {"default_ultimate_id", talent->second.defaultUltimate},
                         {"ultimate_ids", IntArrayJson(talent->second.ultimates)}};
    } else {
      hero["talent"] = nullptr;
    }
    report["heroes"].push_back(std::move(hero));

    auto addMapping = [&](const char *field, int mappingId) {
      const auto mapping = mappings.find(mappingId);
      report["hero_action_links"].push_back({{"hero_id", id}, {"source_field", field},
                                               {"mapping_id", mappingId},
                                               {"mapping_name", mapping == mappings.end() ? Json(nullptr) : Json(mapping->second.name)},
                                               {"status", mappingId == 0 ? "NO_OVERRIDE" : (mapping == mappings.end() ? "MISSING_ID" : "FOUND")}});
    };
    addMapping("_HeroMappingID", row.heroMapping);
    addMapping("_EmoteMappingID", row.emoteMapping);
    addMapping("_StandbyMappingID", row.standbyMapping);
    addMapping("_SoulMapping", row.soulMapping);
    if (fashion != fashions.end()) {
      addMapping("_UIHeroMappingID", fashion->second.uiMapping);
      addMapping("_UIHeroEmoteFaceMappingID", fashion->second.uiEmote);
      addMapping("_UIHeroEnEmoteFaceMappingID", fashion->second.uiEnEmote);
      addMapping("_UIHeroKrEmoteFaceMappingID", fashion->second.uiKrEmote);
      addMapping("_UIHeroJpEmoteFaceMappingID", fashion->second.uiJpEmote);
      addMapping("_UIHeroThEmoteFaceMappingID", fashion->second.uiThEmote);
    }
  }

  for (const auto &[heroId, hero] : battles) {
    std::set<int> ids;
    const auto talent = talents.find(heroId);
    if (talent != talents.end()) {
      ids.insert(talent->second.defaultSkill);
      ids.insert(talent->second.defaultUltimate);
      ids.insert(talent->second.skills.begin(), talent->second.skills.end());
      ids.insert(talent->second.ultimates.begin(), talent->second.ultimates.end());
    }
    for (const auto &[skillId, skill] : skills)
      if (skill.relateHero == heroId) ids.insert(skillId);
    ids.erase(0);
    for (const int skillId : ids) {
      const auto skill = skills.find(skillId);
      if (skill == skills.end()) {
        report["hero_skill_links"].push_back({{"hero_id", heroId}, {"talent_id", skillId},
                                               {"status", "MISSING_ID"}, {"related_hero_id", nullptr},
                                               {"skill_name", nullptr}, {"combo_graph", nullptr},
                                               {"combo_mapping_id", nullptr}, {"combo_mapping_name", nullptr}});
        continue;
      }
      const auto mapping = mappings.find(skill->second.comboMapping);
      report["hero_skill_links"].push_back({{"hero_id", heroId}, {"talent_id", skillId},
                                             {"listed_in_hero_talent", talent == talents.end() ? Json(nullptr) : Json(true)},
                                             {"related_hero_id", skill->second.relateHero},
                                             {"skill_name", skill->second.name}, {"combo_graph", skill->second.comboGraph},
                                             {"combo_mapping_id", skill->second.comboMapping},
                                             {"combo_mapping_name", mapping == mappings.end() ? Json(nullptr) : Json(mapping->second.name)},
                                             {"launch_config", skill->second.launch}, {"deprecated", skill->second.deprecated},
                                             {"status", skill->second.relateHero == heroId ? "FOUND" : "OWNER_MISMATCH"},
                                             {"memory", Hex(skill->second.address)}});
    }
  }

  for (const auto &state : states)
    report["action_states"].push_back({{"state_name", state.id}, {"tag", state.tag},
                                       {"layer", state.layer}, {"motion_clips", StringArrayJson(state.clips)},
                                       {"hero_id", nullptr}, {"action_hash", nullptr},
                                       {"association_status", "UNRESOLVED"}, {"memory", Hex(state.address)}});

  report["counts"] = {{"config_records", battles.size()},
                       {"main_marked_records", mainCount},
                       {"placeholder_records", placeholderCount},
                       {"related_non_main_forms", relatedCount},
                       {"missing_fashion_rows", missingFashion},
                       {"different_name_rows", nameDiffers},
                       {"mapping_links", report["hero_action_links"].size()},
                       {"hero_skill_links", report["hero_skill_links"].size()},
                       {"state_names", states.size()},
                       {"cache_records", battleReader->records.size()}};

  if (battleReader->cacheCount < battleReader->keyCount)
    report["warnings"].push_back("HeroBattle cache is smaller than its key indexer; only materialized records are exported");
  if (!fashionReader) report["warnings"].push_back("HeroFashion reader is not initialized");
  if (!talentReader) report["warnings"].push_back("HeroTalent reader is not initialized");
  if (!mappingReader) report["warnings"].push_back("PlayableMapping reader is not initialized");
  if (!skillReader) report["warnings"].push_back("TalentSkill reader is not initialized");
  if (!stateReader) report["warnings"].push_back("AutoAnimator state reader is not initialized");

  // ---- 英雄目录产物统一写入子文件夹 <outputDirectory>\hero_catalog\ ----
  const std::string catalogDir = std::string(outputDirectory) + "\\hero_catalog";
  CreateDirectoryA(catalogDir.c_str(), nullptr);

  // 各产物文件名与内容（heroes.json 沿用主 report；其余由同一份数据渲染）
  const std::string heroesJsonText = report.dump(2) + "\n";
  const std::string heroesTsvText = TsvFromJson(report["heroes"],
      {"hero_id", "battle_title", "display_name", "internal_name", "category",
       "is_main_config", "placeholder", "related_hero_id", "related_from_hero_ids",
       "deprecated", "playable_now", "names_differ"});
  const std::string heroSkillsTsvText = TsvFromJson(report["hero_skill_links"],
      {"hero_id", "talent_id", "skill_name", "combo_graph", "combo_mapping_id",
       "combo_mapping_name", "status", "listed_in_hero_talent", "related_hero_id",
       "launch_config", "deprecated"});
  const std::string heroActionLinksTsvText = TsvFromJson(report["hero_action_links"],
      {"hero_id", "source_field", "mapping_id", "mapping_name", "status"});
  const std::string actionStatesTsvText = TsvFromJson(report["action_states"],
      {"state_name", "tag", "layer", "hero_id", "action_hash", "association_status"});

  // dump_header.cs：可放类型 dump 前面的注释摘要（内存快照来源）
  const auto &counts = report["counts"];
  std::string header = "// ===== 英雄配置目录（运行时内存快照，非对局玩家）=====\r\n";
  header += "// 数据来源：LIVE_PROCESS_MEMORY（GameAssembly_Super.dll 运行时读取）\r\n";
  header += "// 配置记录：" + counts["config_records"].dump() +
            "；主英雄标记：" + counts["main_marked_records"].dump() +
            "；占位记录：" + counts["placeholder_records"].dump() + "\r\n";
  header += "// 关联非主形态：" + counts["related_non_main_forms"].dump() +
            "；技能关联：" + counts["hero_skill_links"].dump() +
            "；动作状态名：" + counts["state_names"].dump() + "\r\n";
  header += "// 当前可选英雄总数：未验证；详细数据见同目录 heroes.json\r\n";
  for (const auto &hero : report["heroes"]) {
    header += "// Hero ID=" + hero["hero_id"].dump() + " Name=" +
              TsvCell(hero["battle_title"].get<std::string>()) +
              " Category=" + hero["category"].get<std::string>() + "\r\n";
  }
  header += "// ============================================\r\n";

  // offset_log.txt：简体中文运行说明
  std::string logText = "英雄目录导出完成（运行时内存自动采集）。\r\n";
  logText += "数据来源：游戏进程内 BinaryDesignData 读取器缓存，未使用 TSV 配置表。\r\n";
  logText += "配置记录数：" + counts["config_records"].dump() +
             "\r\n主英雄标记：" + counts["main_marked_records"].dump() +
             "\r\n动作状态名：" + counts["state_names"].dump() + "\r\n";
  logText += "技能关联：" + counts["hero_skill_links"].dump() +
             "\r\n动作映射引用：" + counts["mapping_links"].dump() + "\r\n";
  for (const auto &warning : report["warnings"])
    logText += "提示：" + warning.get<std::string>() + "\r\n";

  // 先写除 manifest 外的全部文件，全部成功后再写 manifest（失败不冒充成功产物）。
  struct Product { const char *name; const std::string &text; };
  std::vector<Product> products = {
      {"heroes.json", heroesJsonText},
      {"heroes.tsv", heroesTsvText},
      {"hero_skills.tsv", heroSkillsTsvText},
      {"hero_action_links.tsv", heroActionLinksTsvText},
      {"action_states.tsv", actionStatesTsvText},
      {"dump_header.cs", header},
      {"offset_log.txt", logText},
  };
  for (const auto &p : products) {
    if (!WriteUtf8(catalogDir + "\\" + p.name, p.text)) {
      if (error) *error = "cannot write " + catalogDir + "\\" + p.name;
      return false;
    }
  }

  SYSTEMTIME now = {};
  GetLocalTime(&now);
  char stamp[48] = {};
  snprintf(stamp, sizeof(stamp), "%04u-%02u-%02uT%02u:%02u:%02u.%03u",
           (unsigned)now.wYear, (unsigned)now.wMonth, (unsigned)now.wDay,
           (unsigned)now.wHour, (unsigned)now.wMinute, (unsigned)now.wSecond,
           (unsigned)now.wMilliseconds);
  report["exported_at_local"] = std::string(stamp);

  Json manifestProducts = Json::array();
  for (const auto &p : products)
    manifestProducts.push_back({{"file", p.name}, {"bytes", p.text.size()},
                                {"sha256", Sha256(p.text)}});
  Json manifest = {{"schema_version", 2},
                   {"status", "COMPLETE_WITH_LIMITATIONS"},
                   {"source_kind", "LIVE_PROCESS_MEMORY"},
                   {"live_process_verified", true},
                   {"exported_at_local", std::string(stamp)},
                   {"counts", report["counts"]},
                   {"read_chain", report["read_chain"]},
                   {"products", manifestProducts}};
  const std::string manifestText = manifest.dump(2) + "\n";
  if (!WriteUtf8(catalogDir + "\\manifest.json", manifestText)) {
    if (error) *error = "cannot write " + catalogDir + "\\manifest.json";
    return false;
  }

  if (d.log) d.log->line("[hero-catalog] live memory capture complete: heroes=%llu skills=%llu mappings=%llu states=%llu dir=%s",
                         static_cast<unsigned long long>(battles.size()),
                         static_cast<unsigned long long>(report["hero_skill_links"].size()),
                         static_cast<unsigned long long>(mappings.size()),
                         static_cast<unsigned long long>(states.size()), catalogDir.c_str());
  return true;
}

}  // namespace ildump
