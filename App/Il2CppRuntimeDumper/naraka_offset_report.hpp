// Read-only offset report. Included after Dumper is defined.
#pragma once
#include "naraka_offset_catalog.hpp"

namespace ildump {
struct OffsetResult {
  bool hasValue = false;
  uint64_t value = 0;
  std::string status = "NOT_FOUND";
  std::string evidence;
  uintptr_t owner = 0, type = 0, method = 0;
  uint32_t attrs = 0;
};

class OffsetReporter {
public:
  Dumper &d;
  std::unordered_map<std::string, std::vector<uintptr_t>> classes;
  bool windowScanned = false;
  OffsetResult windowResult;

  explicit OffsetReporter(Dumper &dumper) : d(dumper) {}

  static std::string hex(uint64_t value) {
    char buf[32];
    snprintf(buf, sizeof(buf), "0x%llX", (unsigned long long)value);
    return buf;
  }
  static OffsetResult failure(const char *status, const std::string &why) {
    OffsetResult r;
    r.status = status;
    r.evidence = why;
    return r;
  }
  static OffsetResult value(uint64_t v, const char *status, const std::string &why) {
    OffsetResult r = failure(status, why);
    r.hasValue = true;
    r.value = v;
    return r;
  }
  static std::string key(const std::string &name, const std::string &ns) {
    return ns + "\n" + name;
  }

  void buildIndex() {
    std::unordered_set<uintptr_t> seen;
    auto add = [&](uintptr_t k) {
      if (!k || !seen.insert(k).second) return;
      char name[512] = {}, ns[512] = {};
      if (!d.klassName(k, name, sizeof(name))) return;
      uintptr_t nsp = 0;
      if (!d.mem.rd(k + d.lay.cNamespace, nsp)) return;
      // MemReader::rdStr deliberately reports an empty string as false.
      // Empty/null namespace is valid for most Assembly-CSharp business classes.
      if (nsp) {
        char first = 0;
        if (!d.mem.rd(nsp, first) ||
            (first && !d.mem.rdStr(nsp, ns, sizeof(ns), false))) return;
      }
      classes[key(name, ns)].push_back(k);
    };
    // The API is authoritative even when the inferred global index map has gaps.
    if (d.env->imageGetClass) {
      for (uintptr_t img : d.images_) {
        for (size_t i = 0, n = d.imageCount(img); i < n; ++i) {
          uintptr_t k = 0;
          if (SafeCallImageGetClass(d.env->imageGetClass, img, i, k)) add(k);
        }
      }
    }
    for (uintptr_t k : d.typeTable_) add(k);
    for (uintptr_t k : d.klasses_) add(k);
  }

  OffsetResult findClass(const char *name, const char *ns = "") {
    const auto it = classes.find(key(name, ns));
    if (it == classes.end()) return failure("NOT_FOUND", std::string("class-not-found: ") + ns + "." + name);
    if (it->second.size() != 1)
      return failure("AMBIGUOUS", std::string("multiple classes: ") + ns + "." + name);
    OffsetResult r = value(it->second[0], "FOUND", std::string("Class: ") + name);
    r.owner = it->second[0];
    return r;
  }

  std::string typeLabel(uintptr_t type) {
    char name[512] = {};
    d.typeName(type, name, sizeof(name), 0);
    return name;
  }

  OffsetResult field(uintptr_t klass, const char *name) {
    if (!klass) return failure("NOT_FOUND", "class-not-found");
    std::unordered_set<uintptr_t> visited;
    for (unsigned depth = 0; klass && depth < 32 && visited.insert(klass).second; ++depth) {
      uint16_t count = 0;
      if (!d.mem.rd(klass + d.lay.cFieldCount, count)) return failure("UNREADABLE", "field count unreadable");
      if (count > 4096) return failure("UNREADABLE", "invalid field count");
      // Prefer an exact field over its possible auto-property backing field.
      for (int mode = 0; mode < 2; ++mode) {
        const std::string wanted = mode ? std::string("<") + name + ">k__BackingField" : name;
        if (mode && name[0] == '<') break;
        OffsetResult result;
        unsigned hits = 0;
        for (uint32_t i = 0; i < count; ++i) {
          char found[512] = {};
          uintptr_t type = 0;
          int32_t off = 0;
          uint32_t attrs = 0;
          if (!d.readField(klass, i, found, sizeof(found), type, off, attrs) || wanted != found) continue;
          uintptr_t fi = d.mem.rdPtr(klass + d.lay.cFields) + (uintptr_t)i * d.lay.fStride;
          uint32_t bits = 0;
          if (!d.mem.rd(fi + d.lay.fOffset, off) ||
              !d.mem.rd(type + d.lay.tBits, bits))
            return failure("UNREADABLE", wanted + " metadata read failed");
          attrs = bits & 0xFFFF;
          char ownerName[512] = {};
          d.klassName(klass, ownerName, sizeof(ownerName));
          const std::string evidence = std::string("Class: ") + ownerName + "->" +
              typeLabel(type) + " " + found + ((attrs & FA_STATIC) ? " [static]" : " [instance]");
          if (attrs & FA_LITERAL) return failure("NOT_FIELD_OFFSET", evidence + " is a literal");
          if (off < 0) return failure("SPECIAL_STORAGE", evidence + " offset=" + std::to_string(off));
          if (!(attrs & FA_STATIC) && strchr(ownerName, '`') && off < 16)
            return failure("UNRESOLVED_LAYOUT", evidence + " rawOffset=" + hex(off) +
                           "; generic definition has no confirmed instance layout");
          result = value((uint32_t)off, "FOUND", evidence);
          result.owner = klass;
          result.type = type;
          result.attrs = attrs;
          ++hits;
        }
        if (hits > 1) return failure("AMBIGUOUS", wanted + " has multiple matching fields");
        if (hits == 1) return result;
      }
      klass = d.mem.rdPtr(klass + d.lay.cParent);
    }
    return failure("NOT_FOUND", std::string("field-not-found: ") + name);
  }

  OffsetResult field(const char *klass, const char *ns, const char *name) {
    const auto it = classes.find(key(klass, ns));
    if (it == classes.end()) return failure("NOT_FOUND", std::string("class-not-found: ") + ns + "." + klass);
    OffsetResult result = failure("NOT_FOUND", std::string("field-not-found: ") + klass + "." + name);
    unsigned hits = 0;
    for (uintptr_t k : it->second) {
      OffsetResult candidate = field(k, name);
      if (candidate.hasValue || candidate.status != "NOT_FOUND") { result = candidate; ++hits; }
    }
    if (hits > 1) return failure("AMBIGUOUS", std::string("multiple declaring classes match: ") + klass + "." + name);
    return result;
  }

  bool inModule(uintptr_t p) const {
    return p >= d.env->moduleBase && p - d.env->moduleBase < d.env->moduleSize;
  }
  bool executable(uintptr_t p) {
    MEMORY_BASIC_INFORMATION mbi = {};
    if (!inModule(p) || !VirtualQuery((void *)p, &mbi, sizeof(mbi)) ||
        mbi.State != MEM_COMMIT || (mbi.Protect & (PAGE_GUARD | PAGE_NOACCESS))) return false;
    const DWORD pr = mbi.Protect & 0xFF;
    return pr == PAGE_EXECUTE || pr == PAGE_EXECUTE_READ ||
           pr == PAGE_EXECUTE_READWRITE || pr == PAGE_EXECUTE_WRITECOPY;
  }

  OffsetResult method(const char *klass, const char *ns, const char *name, int params) {
    OffsetResult c = findClass(klass, ns);
    if (!c.hasValue) return c;
    uintptr_t k = c.owner;
    std::unordered_set<uintptr_t> visited;
    for (unsigned depth = 0; k && depth < 32 && visited.insert(k).second; ++depth) {
      unsigned hits = 0;
      OffsetResult found;
      uint16_t count = 0;
      if (!d.mem.rd(k + d.lay.cMethodCount, count)) return failure("UNREADABLE", "method count unreadable");
      if (count > 4096) return failure("UNREADABLE", "invalid method count");
      for (uint32_t i = 0; i < count; ++i) {
        uintptr_t mi = 0;
        char mn[512] = {};
        uint8_t pc = 0;
        if (!d.readMethod(k, i, mi, mn, sizeof(mn)) || strcmp(mn, name) ||
            !d.mem.rd(mi + d.lay.mParamCount, pc) || (params >= 0 && pc != params))
          continue;
        // The known CrossFade entry has seven specific parameter types.
        // This excludes UnityEngine.Animator overloads and future same-count overloads.
        if (!strcmp(name, "CrossFadeInFixedTime")) {
          static const char *expected[] = {"Int32", "Single", "Int32", "Single", "Single",
                                           "TransitionSourceType", "Boolean"};
          uintptr_t pa = d.mem.rdPtr(mi + d.lay.mParams);
          bool match = pc == 7;
          for (uint32_t j = 0; match && j < pc; ++j) {
            uintptr_t tp = 0;
            const char *unused = nullptr;
            match = d.readMethodParam(pa, j, tp, &unused) && typeLabel(tp) == expected[j];
          }
          if (!match) continue;
        }
        ++hits;
        uintptr_t ptr = 0;
        if (!d.mem.rd(mi + d.lay.mPtr, ptr) || !ptr)
          found = failure("NOT_FOUND", std::string(name) + " methodPointer unavailable");
        else if (!inModule(ptr))
          found = failure("OUTSIDE_MODULE", std::string(name) + " VA=" + hex(ptr));
        else
          found = value(ptr - d.env->moduleBase, executable(ptr) ? "FOUND" : "NON_EXECUTABLE",
                        std::string("Class: ") + klass + "->" + name +
                        " params=" + std::to_string(pc) + " MethodInfo=" + hex(mi) +
                        " VA=" + hex(ptr) + " slot=" + hex(d.mem.rdU16(mi + d.lay.mSlot)));
        found.method = mi;
        found.owner = k;
      }
      if (hits > 1) return failure("AMBIGUOUS", std::string(name) + " has multiple matching overloads");
      if (hits == 1) return found;
      k = d.mem.rdPtr(k + d.lay.cParent);
    }
    return failure("NOT_FOUND", std::string("method/signature-not-found: ") + klass + "." + name);
  }

  OffsetResult typeInfo(const OffsetSpec &spec) {
    const auto candidates = classes.find(key(spec.klass, spec.namespaze));
    if (candidates == classes.end()) return failure("NOT_FOUND", std::string("class-not-found: ") + spec.klass);
    OffsetResult result = failure("NOT_FOUND", std::string("TypeInfo slot-not-found: ") + spec.klass);
    unsigned hits = 0;
    for (uintptr_t k : candidates->second) {
      // UserData also names an unrelated nested struct. Its business fields
      // identify the singleton consumed by the client before looking at slots.
      if (spec.member[0] && !field(k, spec.member).hasValue) continue;
      const auto slot = d.classPointerSlots_.find(k);
      if (slot == d.classPointerSlots_.end()) continue;
      uintptr_t current = 0;
      if (!inModule(slot->second) || !d.mem.rd(slot->second, current) || current != k) {
        if (!hits) result = failure("UNREADABLE", "TypeInfo slot readback changed or failed");
        continue;
      }
      result = value(slot->second - d.env->moduleBase, "FOUND",
                     std::string("Class: ") + spec.klass + " [TypeInfo RVA] slot=" +
                     hex(slot->second) + " klass=" + hex(k));
      ++hits;
    }
    if (hits > 1) return failure("AMBIGUOUS", "multiple same-name classes have live TypeInfo slots");
    return result;
  }

  OffsetResult staticFieldsLayout() {
    // cInterfaces+0x10 is only a hypothesis. Real v6 Super logs verified +0xB8
    // even with interfaces at +0xB0, so require singleton back-reference evidence.
    uint32_t off = 0;
    unsigned verified = 0;
    for (const auto &r : d.managerRvaResults_) {
      if (!r.verified) continue;
      if (verified && off != r.staticFieldsOffset)
        return failure("AMBIGUOUS", "manager static_fields layout validations disagree");
      off = r.staticFieldsOffset;
      ++verified;
    }
    if (verified) return value(off, "VERIFIED", "singleton instance klass back-reference");
    return failure("UNRESOLVED", "static_fields layout lacks a live singleton back-reference");
  }

  uintptr_t klassForType(uintptr_t type) {
    const uint32_t te = (d.mem.rdU32(type + d.lay.tBits) >> 16) & 0xFF;
    uintptr_t data = d.mem.rdPtr(type + d.lay.tData);
    if (te == T_GENERICINST) return d.mem.rdPtr(data + 0x18);
    if (te != T_CLASS && te != T_VALUETYPE) return 0;
    if (d.typeDataIsHandle_) {
      auto it = d.typeHandleToKlass_.find(data);
      return it == d.typeHandleToKlass_.end() ? 0 : it->second;
    }
    if (d.typeDataIsIndex_) return data < d.typeTable_.size() ? d.typeTable_[data] : 0;
    return data;
  }

  OffsetResult dictionaryEntry(const OffsetSpec &spec) {
    OffsetResult dict = field(spec.klass, spec.namespaze, spec.member);
    if (!dict.hasValue) return dict;
    uintptr_t dk = klassForType(dict.type);
    OffsetResult entries = field(dk, "_entries");
    if (!entries.hasValue) entries = field(dk, "entries");
    if (!entries.hasValue) return failure("UNRESOLVED", "Dictionary entries type not recovered");
    uint32_t te = (d.mem.rdU32(entries.type + d.lay.tBits) >> 16) & 0xFF;
    if (te != T_SZARRAY) return failure("UNRESOLVED", "Dictionary entries is not SZARRAY");
    uintptr_t ek = klassForType(d.mem.rdPtr(entries.type + d.lay.tData));
    OffsetResult val = field(ek, "value");
    char parent[128] = {};
    d.klassName(d.mem.rdPtr(ek + d.lay.cParent), parent, sizeof(parent));
    if (!val.hasValue || val.value < 16 || strcmp(parent, "ValueType"))
      return failure("UNRESOLVED", "Dictionary.Entry unboxed value layout unconfirmed");
    return value(32 + val.value - 16, "DERIVED",
                 "Il2CppArray header(0x20) + unboxed Entry.value; " + val.evidence);
  }

  OffsetResult window() {
    if (windowScanned) return windowResult;
    windowScanned = true;
    if (!d.env->unityPlayerBase || !d.env->unityPlayerSize)
      return windowResult = failure("NOT_LOADED", "UnityPlayer module unavailable");
    if (!d.env->windowWidth || !d.env->windowHeight)
      return windowResult = failure("UNRESOLVED", "no visible game client window for dimensions check");
    const uintptr_t base = d.env->unityPlayerBase, end = base + d.env->unityPlayerSize;
    if (end <= base) return windowResult = failure("UNREADABLE", "invalid UnityPlayer range");
    std::unordered_map<uintptr_t, bool> checked;
    std::vector<uintptr_t> matches;
    size_t hitCount = 0;
    const ULONGLONG started = GetTickCount64();
    bool limited = false;
    // Validate only the pointer chain actually consumed by SDK.cpp. Dimensions
    // alone are insufficient to claim a uniquely identified native singleton.
    for (uintptr_t p = base; p < end;) {
      MEMORY_BASIC_INFORMATION mbi = {};
      if (!VirtualQuery((void *)p, &mbi, sizeof(mbi))) { limited = true; break; }
      uintptr_t lo = (std::max)(p, (uintptr_t)mbi.BaseAddress);
      uintptr_t hi = (std::min)(end, (uintptr_t)mbi.BaseAddress + mbi.RegionSize);
      if (hi <= p) break;
      if (mbi.State == MEM_COMMIT && !(mbi.Protect & (PAGE_GUARD | PAGE_NOACCESS)) &&
          Dumper::isWritableProtect(mbi.Protect)) {
        for (uintptr_t cur = (lo + 7) & ~uintptr_t(7); cur + 8 <= hi;) {
          if (GetTickCount64() - started > 3000) { limited = true; break; }
          uintptr_t words[512];
          size_t count = (std::min)(size_t(512), size_t((hi - cur) / 8));
          if (SafeCopy(words, (void *)cur, count * 8)) {
            for (size_t i = 0; i < count; ++i) {
              uintptr_t k = words[i];
              if (k < 0x10000 || k >= 0x0000800000000000ull || (k & 7)) continue;
              auto it = checked.find(k);
              bool ok = false;
              if (it != checked.end()) ok = it->second;
              else {
                if (checked.size() >= 250000) { limited = true; break; }
                uintptr_t statics = 0, obj = 0;
                int width = 0, height = 0;
                ok = d.mem.rd(k + 0xB8, statics) && statics &&
                     d.mem.rd(statics + 8, obj) && obj &&
                     d.mem.rd(obj + 0x14, width) && d.mem.rd(obj + 0x18, height) &&
                     width == d.env->windowWidth && height == d.env->windowHeight;
                checked.emplace(k, ok);
              }
              if (ok) { ++hitCount; if (matches.size() < 16) matches.push_back(cur + i * 8 - base); }
            }
          }
          cur += count * 8;
          if (limited) break;
        }
      }
      if (limited) break;
      p = hi;
    }
    std::string evidence = "module=" + d.env->unityPlayerName + " client=" +
        std::to_string(d.env->windowWidth) + "x" + std::to_string(d.env->windowHeight) +
        " chain=slot->+0xB8->+0x8->width@0x14,height@0x18 candidates=" + std::to_string(hitCount);
    for (uintptr_t rva : matches) evidence += " " + hex(rva);
    if (limited) return windowResult = failure("SCAN_INCOMPLETE", evidence);
    if (!hitCount) return windowResult = failure("NOT_FOUND", evidence);
    if (hitCount != 1) return windowResult = failure("AMBIGUOUS", evidence);
    return windowResult = value(matches[0], "CANDIDATE", evidence + "; native identity unconfirmed");
  }

  OffsetResult vtable(const OffsetSpec &spec) {
    OffsetResult c = findClass(spec.klass, spec.namespaze);
    if (!c.hasValue) return c;
    uintptr_t target = d.mem.rdPtr(c.owner + spec.arg);
    if (!executable(target))
      return failure("UNRESOLVED", "historical probe +" + hex(spec.arg) +
                     " does not point to module executable code; " + spec.note);
    // Confirm a function/MethodInfo pair instead of treating any executable
    // pointer as a virtual entry. Still a candidate: hook method intent is unknown.
    for (int delta : {8, -8}) {
      uintptr_t mi = d.mem.rdPtr(c.owner + spec.arg + delta);
      if (!mi || d.mem.rdPtr(mi + d.lay.mPtr) != target ||
          d.mem.rdPtr(mi + d.lay.mKlass) != c.owner) continue;
      char name[256] = {};
      if (!d.mem.rdStr(d.mem.rdPtr(mi + d.lay.mName), name, sizeof(name), true)) continue;
      return value(spec.arg, "CANDIDATE", std::string("Class: ") + spec.klass +
                   "->" + name + " targetRVA=" + hex(target - d.env->moduleBase) +
                   " MethodInfo=" + hex(mi) + "; intended hook method must be confirmed");
    }
    return failure("UNRESOLVED", "historical executable pointer has no verified MethodInfo pair");
  }

  OffsetResult resolve(const OffsetSpec &spec) {
    const int active = d.env->superVariant ? 2 : 1;
    if (spec.variant && spec.variant != active)
      return failure(d.env->otherVariantLoaded ? "NOT_DUMPED" : "NOT_LOADED",
                     "other GameAssembly variant; this dump covers only " + d.env->moduleName);
    switch (spec.kind) {
    case OffsetKind::Field: return field(spec.klass, spec.namespaze, spec.member);
    case OffsetKind::TypeInfo: return typeInfo(spec);
    case OffsetKind::Method: return method(spec.klass, spec.namespaze, spec.member, spec.arg);
    case OffsetKind::Window: return window();
    case OffsetKind::DictionaryEntry: return dictionaryEntry(spec);
    case OffsetKind::Vtable: return vtable(spec);
    case OffsetKind::Context:
      return failure("UNRESOLVED",
          "actual hook RCX type unconfirmed; ActorKitInteractableDevice.agent is PlayerEnvironmentColliderAgent, not EntityKit");
    case OffsetKind::Module: {
      uintptr_t base = 0;
      if (!strcmp(spec.member, "GameAssembly")) base = d.env->moduleBase;
      if (!strcmp(spec.member, "UnityPlayer")) base = d.env->unityPlayerBase;
      if (!strcmp(spec.member, "NarakaBladepoint")) base = d.env->executableBase;
      return base ? value(base, "RUNTIME_VA", "module base VA; changes on restart")
                  : failure("NOT_LOADED", "module base unavailable");
    }
    case OffsetKind::Layout: {
      if (!strcmp(spec.member, "staticFields")) return staticFieldsLayout();
      if (!d.lay.calibrated) return failure("UNRESOLVED", "runtime layout calibration failed");
      uint32_t off = 0;
      if (!strcmp(spec.member, "cName")) off = d.lay.cName;
      else if (!strcmp(spec.member, "cParent")) off = d.lay.cParent;
      else if (!strcmp(spec.member, "cMethods")) off = d.lay.cMethods;
      else if (!strcmp(spec.member, "cMethodCount")) off = d.lay.cMethodCount;
      else if (!strcmp(spec.member, "mPtr")) off = d.lay.mPtr;
      else if (!strcmp(spec.member, "mName")) off = d.lay.mName;
      else if (!strcmp(spec.member, "mSlot")) off = d.lay.mSlot;
      else if (!strcmp(spec.member, "mParamCount")) off = d.lay.mParamCount;
      else return failure("UNRESOLVED", "unknown layout member");
      return value(off, "CALIBRATED", std::string("runtime layout: ") + spec.member);
    }
    case OffsetKind::ArrayHeader: return value(4 * sizeof(uintptr_t), "ABI_LAYOUT", spec.note);
    case OffsetKind::Constant: return value(spec.arg, "CONFIG_CONSTANT", spec.note);
    default:
      if (!strcmp(spec.key, "op_Subtraction") || !strcmp(spec.key, "Call_GetVelocity"))
        return failure("SKIPPED_LEGACY", "unused old entry; user allowed skipping; " + std::string(spec.note));
      return failure("UNRESOLVED", spec.note);
    }
  }

  void write(Writer &w) {
    buildIndex();
    w.puts("// ===== Offset.h Discovery Summary (v7) =====\r\n");
    w.putf("// Active module: %s base=%s variant=%s\r\n", d.env->moduleName.c_str(),
           hex(d.env->moduleBase).c_str(), d.env->superVariant ? "Super/SuperIBT" : "Normal");
    w.putf("// Other variant: %s\r\n", d.env->otherVariantLoaded
        ? "NOT_DUMPED (loaded but not selected)" : "NOT_LOADED (no values available in this run)");
    w.puts("// Keys preserve Offset.h paths. RVA, instance/static field offset and runtime VA are labelled separately.\r\n");
    w.puts("// FOUND/CALIBRATED/VERIFIED/DERIVED = resolved; CANDIDATE requires validation; ABI_LAYOUT/CONFIG_CONSTANT are not discoveries.\r\n");
    w.puts("// UNRESOLVED_LAYOUT preserves raw metadata in evidence; generic instance offsets below the object header are not usable offsets.\r\n");
    w.puts("// 未加载的另一变体不填值；未找到/待确认/已跳过旧项均明确标注，不沿用历史偏移。\r\n");
    size_t resolved = 0, unavailable = 0, candidate = 0, constants = 0, unresolved = 0;
    for (const auto &spec : kOffsetSpecs) {
      OffsetResult r = resolve(spec);
      if (r.status == "FOUND" || r.status == "VERIFIED" || r.status == "CALIBRATED" ||
          r.status == "DERIVED" || r.status == "RUNTIME_VA") ++resolved;
      else if (r.status == "NOT_LOADED" || r.status == "NOT_DUMPED") ++unavailable;
      else if (r.status == "CANDIDATE") ++candidate;
      else if (r.status == "ABI_LAYOUT" || r.status == "CONFIG_CONSTANT") ++constants;
      else ++unresolved;
      const std::string v = r.hasValue ? hex(r.value) : r.status;
      w.putf("// Offset::%s = %s; // status=%s %s\r\n",
             spec.key, v.c_str(), r.status.c_str(), r.evidence.c_str());
      d.log->line("[offset-report] %s=%s status=%s %s",
                  spec.key, v.c_str(), r.status.c_str(), r.evidence.c_str());
    }
    w.putf("// Inventory: total=%llu resolved=%llu other/unavailable=%llu candidates=%llu constants=%llu unresolved/skipped=%llu\r\n",
           (unsigned long long)(sizeof(kOffsetSpecs) / sizeof(kOffsetSpecs[0])),
           (unsigned long long)resolved, (unsigned long long)unavailable,
           (unsigned long long)candidate, (unsigned long long)constants, (unsigned long long)unresolved);
    w.puts("// ==========================================\r\n\r\n");
    d.log->line("[offset-report] inventory=%llu resolved=%llu unavailable=%llu candidates=%llu constants=%llu unresolved=%llu",
                (unsigned long long)(sizeof(kOffsetSpecs) / sizeof(kOffsetSpecs[0])),
                (unsigned long long)resolved, (unsigned long long)unavailable,
                (unsigned long long)candidate, (unsigned long long)constants, (unsigned long long)unresolved);
  }
};

inline void Dumper::writeOffsetSummary(Writer &w) {
  const Stats savedStats = st;
  OffsetReporter reporter(*this);
  reporter.write(w);
  // Metadata inspection for the report must not inflate the dump's statistics.
  st = savedStats;
}
} // namespace ildump
