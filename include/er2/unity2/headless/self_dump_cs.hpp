#pragma once

// ============================================================================
// SelfDumpCs - self-contained IL2CPP runtime structure dump (DMA, no injector)
// ============================================================================
// Generates a complete dump.cs directly from live runtime structures read
// through the DMA memory accessor. Does NOT depend on:
//   - Unity-Offset.dll / any injector
//   - global-metadata.dat magic scan (0xFAB11BAF)
//
// Strategy:
//   1. Find the Il2Cpp TypeInfo table (byvalIndex -> Il2CppClass*) in the
//      GameAssembly .data section using runtime-known anchor classes
//      (System.Object / System.String / UnityEngine.Object / UnityEngine.Transform).
//   2. Iterate every byval slot; for each non-null klass pointer, read:
//        - class name / namespace        (klass.name@0x10, klass.namespace@0x18)
//        - field list                    (klass.fields@0x80, fieldCount@0x11C)
//        - method list                   (klass.methods@0x98, methodCount@0x118)
//   3. For each field: name, type (via runtime Il2CppType resolver), offset.
//   4. For each method: name, return type, parameter count, RVA, token.
//   5. Emit dump.cs in the standard Il2CppDumper-like layout:
//        // Namespace / public class X // TypeDefIndex
//        // Fields / // Properties / // Methods (with RVA/VA comments)
//
// Layout profile: unity-2019.4-lts-24.5 (Il2CppLayoutProfile) - current target.
// ============================================================================

#include <Windows.h>

#include <algorithm>
#include <cstdint>
#include <cstring>
#include <fstream>
#include <map>
#include <set>
#include <sstream>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <vector>

#include "../../mem/memory_read.hpp"
#include "../inspect/runtime_class_probe.hpp"
#include "../metadata/il2cpp_layout_profile.hpp"
#include "../msid/enumerate_objects.hpp"
#include "../msid/msid_scan.hpp"
#include "probe_cli.hpp"
#include "runtime_type_resolver.hpp"

namespace er2 {

// ---------------------------------------------------------------------------
// Options / results
// ---------------------------------------------------------------------------

struct SelfDumpCsOptions {
  std::string outPath;      // output dump.cs path
  std::string logPath;      // optional log path
  std::uint32_t maxKlass = 0; // 0 = unlimited (iterate whole TypeInfo table)
  std::uint32_t maxFieldsPerKlass = 0x2000;
  std::uint32_t maxMethodsPerKlass = 0x8000;
  bool includeEmpty = false; // emit classes with no fields/methods too
};

struct SelfDumpCsStats {
  std::size_t tableScanBytes = 0;
  std::uintptr_t typeInfoTable = 0;
  std::uint32_t typeInfoCount = 0;
  std::size_t klassResolved = 0;
  std::size_t fieldRows = 0;
  std::size_t methodRows = 0;
  std::size_t methodRva = 0;
  std::string error;
};

namespace detail_self_dump {

// ---------------------------------------------------------------------------
// Small helpers
// ---------------------------------------------------------------------------

inline bool IsCanonicalPtr(std::uintptr_t p) {
  return p >= 0x10000 && p <= 0x7FFFFFFFFFFFull;
}

inline bool ReadRuntimeCStringSafe(const IMemoryAccessor &mem,
                                   std::uintptr_t address, std::string &out,
                                   std::size_t maxLen = 256) {
  out.clear();
  if (!IsCanonicalPtr(address) || maxLen == 0)
    return false;
  out.reserve(maxLen < 64 ? maxLen : 64);
  for (std::size_t i = 0; i < maxLen; ++i) {
    char ch = '\0';
    if (!mem.Read(address + i, &ch, sizeof(ch)))
      return false;
    if (ch == '\0')
      return true;
    out.push_back(ch);
  }
  return false;
}

inline bool IsPrintableName(const std::string &s) {
  if (s.empty() || s.size() > 256)
    return false;
  for (unsigned char c : s) {
    if (c < 0x20 || c > 0x7E)
      return false;
  }
  return true;
}

inline bool IsReadableMethodName(const std::string &name) {
  if (name.empty())
    return true;
  if (name.size() > 192 || !IsPrintableName(name))
    return false;
  for (unsigned char ch : name) {
    const bool alpha = (ch >= 'A' && ch <= 'Z') || (ch >= 'a' && ch <= 'z');
    const bool digit = ch >= '0' && ch <= '9';
    const bool ok = alpha || digit || ch == '_' || ch == '.' || ch == '<' ||
                    ch == '>' || ch == '`' || ch == '+' || ch == '-';
    if (!ok)
      return false;
  }
  return true;
}

inline bool IsRuntimeAddressInsideGameAssembly(std::uintptr_t address) {
  return g_ctx.gameAssembly.base && g_ctx.gameAssembly.size &&
         address >= g_ctx.gameAssembly.base &&
         address < g_ctx.gameAssembly.base + g_ctx.gameAssembly.size;
}

inline bool IsLikelyMethodToken(std::uint32_t token) {
  return token == 0 || (token & 0xFF000000u) == 0x06000000u;
}

// ---------------------------------------------------------------------------
// Klass collection from live objects (no metadata, no TypeInfo table)
// ---------------------------------------------------------------------------
// The most reliable runtime-only way to enumerate classes is to walk the MSID
// object table: every live UnityEngine.Object has a managed header whose first
// pointer is its Il2CppClass. Collecting unique klass pointers covers every
// class that has at least one live instance, which for game-structure analysis
// is the valuable set (managers, actors, UI, config assets, etc).

struct CollectedKlass {
  std::uintptr_t klass = 0;
  std::string fullName;
  std::string ns;
  std::string name;
};

inline bool ResolveKlassFullName(const IMemoryAccessor &mem,
                                 const Il2CppLayoutProfile &profile,
                                 std::uintptr_t klass,
                                 std::string &outFullName) {
  outFullName.clear();
  if (!IsCanonicalPtr(klass))
    return false;
  std::uintptr_t namePtr = 0;
  std::uintptr_t nsPtr = 0;
  if (!ReadPtr(mem, klass + profile.klass.name, namePtr) ||
      !IsCanonicalPtr(namePtr))
    return false;
  std::string name;
  if (!ReadRuntimeCStringSafe(mem, namePtr, name) || name.empty() ||
      !IsPrintableName(name))
    return false;
  std::string ns;
  if (ReadPtr(mem, klass + profile.klass.namespaze, nsPtr) &&
      IsCanonicalPtr(nsPtr)) {
    (void)ReadRuntimeCStringSafe(mem, nsPtr, ns);
  }
  if (ns.empty())
    outFullName = name;
  else
    outFullName = ns + "." + name;
  return !outFullName.empty();
}

inline bool CollectKlassesFromObjects(const IMemoryAccessor &mem,
                                      const Il2CppLayoutProfile &profile,
                                      std::vector<CollectedKlass> &out,
                                      std::uintptr_t maxObjects) {
  out.clear();
  std::unordered_map<std::uintptr_t, std::string> seen;

  const Offsets &off = g_ctx.off;

  std::uintptr_t msidSlot = g_ctx.msIdToPointerSlotVa;
  if (!msidSlot) {
    std::uint32_t score = 0;
    if (!FindMsIdToPointerSlotVaByScan(mem, g_ctx.unityPlayer, g_ctx.gomOff,
                                       msidSlot, &score))
      return false;
    g_ctx.msIdToPointerSlotVa = msidSlot;
  }

  EnumerateOptions opt;
  opt.onlyGameObject = false;
  opt.onlyScriptableObject = false;

  std::uintptr_t seenObjects = 0;

  const bool enumOk = EnumerateMsIdToPointerObjects(
      g_ctx.runtime, mem, msidSlot, off, g_ctx.unityPlayerRange, opt,
      [&](const ObjectInfo &info) {
        if (maxObjects && ++seenObjects > maxObjects)
          return;
        if (!IsCanonicalPtr(info.native))
          return;
        std::uintptr_t managed = 0;
        if (!ReadPtr(mem, info.native + off.unity_object_managed_ptr, managed) ||
            !IsCanonicalPtr(managed))
          return;
        std::uintptr_t klass = 0;
        if (!ReadPtr(mem, managed, klass) || !IsCanonicalPtr(klass))
          return;
        if (seen.find(klass) != seen.end())
          return;

        std::string full;
        if (!ResolveKlassFullName(mem, profile, klass, full))
          return;
        seen[klass] = full;

        CollectedKlass ck;
        ck.klass = klass;
        ck.fullName = full;
        const std::size_t dot = full.rfind('.');
        if (dot != std::string::npos) {
          ck.ns = full.substr(0, dot);
          ck.name = full.substr(dot + 1);
        } else {
          ck.name = full;
        }
        out.push_back(ck);
      });

  return enumOk && !out.empty();
}

} // namespace detail_self_dump

// ---------------------------------------------------------------------------
// Public: run the self-contained dump
// ---------------------------------------------------------------------------

inline bool RunSelfDumpCs(const IMemoryAccessor &mem, SelfDumpCsOptions &options,
                          SelfDumpCsStats &stats) {
  stats = SelfDumpCsStats{};
  if (!g_ctx.gameAssembly.base || !g_ctx.gameAssembly.size) {
    stats.error = "GameAssembly module info missing";
    return false;
  }

  const Il2CppLayoutProfile *profile =
      FindIl2CppLayoutProfileForMetadataVersion(24);
  if (!profile)
    profile = &kIl2CppUnity2019_4_Lts24_5_Profile;

  std::ofstream logFile;
  if (!options.logPath.empty())
    logFile.open(options.logPath, std::ios::binary | std::ios::trunc);
  const auto log = [&](const std::string &m) {
    er2::Log(m);
    if (logFile.is_open())
      logFile << m << std::endl;
  };

  // ---- Step 1: collect unique klass pointers from live MSID objects ----
  log("[SelfDump] Collecting unique runtime classes from live objects...");
  std::vector<detail_self_dump::CollectedKlass> klasses;
  const std::uint32_t maxKlass =
      options.maxKlass ? options.maxKlass : 200000;
  if (!detail_self_dump::CollectKlassesFromObjects(mem, *profile, klasses,
                                                   maxKlass) ||
      klasses.empty()) {
    stats.error = "Failed to collect runtime classes from objects";
    log("[SelfDump] " + stats.error);
    return false;
  }
  log("[SelfDump]   collected " + std::to_string(klasses.size()) +
      " unique classes");
  for (std::size_t i = 0; i < klasses.size() && i < 12; ++i)
    log("[SelfDump]     " + klasses[i].fullName + " klass=0x" +
        HexPtr(klasses[i].klass));

  // ---- Step 2: iterate klasses and build dump.cs ----
  std::ofstream csFile(options.outPath,
                       std::ios::binary | std::ios::out | std::ios::trunc);
  if (!csFile.good()) {
    stats.error = "Failed to open output dump.cs: " + options.outPath;
    return false;
  }

  csFile << "// Runtime self-dump.cs generated by er2 SelfDumpCs (DMA).\n";
  csFile << "// No injector, no global-metadata.dat scan. Live structures.\n";
  csFile << "// Classes collected from live MSID objects: "
         << klasses.size() << "\n\n";

  std::set<std::string> usedClassNames;
  std::size_t fieldRows = 0;
  std::size_t methodRows = 0;
  std::size_t methodRva = 0;

  for (const auto &ck : klasses) {
    const std::uintptr_t klass = ck.klass;

    // klass header
    std::uintptr_t namePtr = 0, nsPtr = 0, fieldsPtr = 0, methodsPtr = 0;
    std::uint16_t fieldCount = 0, methodCount = 0;
    if (!ReadPtr(mem, klass + profile->klass.name, namePtr) ||
        !detail_self_dump::IsCanonicalPtr(namePtr))
      continue;
    std::string name = ck.name, ns = ck.ns;
    if (ReadPtr(mem, klass + profile->klass.namespaze, nsPtr) &&
        detail_self_dump::IsCanonicalPtr(nsPtr)) {
      (void)detail_self_dump::ReadRuntimeCStringSafe(mem, nsPtr, ns);
    }
    (void)ReadPtr(mem, klass + profile->klass.fields, fieldsPtr);
    (void)ReadPtr(mem, klass + profile->klass.methods, methodsPtr);
    (void)ReadValue(mem, klass + profile->klass.fieldCount, fieldCount);
    (void)ReadValue(mem, klass + profile->klass.methodCount, methodCount);

    // unique class name
    std::string uniqueName = name;
    if (!usedClassNames.insert(uniqueName).second) {
      for (std::uint32_t k = 1;; ++k) {
        std::string cand = name + "_" + std::to_string(k);
        if (usedClassNames.insert(cand).second) {
          uniqueName = cand;
          break;
        }
      }
    }

    csFile << "// Class: " << uniqueName << "\n";
    csFile << "// Namespace: " << ns << "\n";
    csFile << "// Klass=0x" << std::hex << std::uppercase << klass
           << " Fields=0x" << fieldsPtr << " Methods=0x" << methodsPtr
           << " StaticFields=0x"
           << [&]() {
                std::uintptr_t sf = 0;
                (void)ReadPtr(mem, klass + profile->klass.staticFields, sf);
                return sf;
              }()
           << std::dec << " FieldCount=" << fieldCount
           << " MethodCount=" << methodCount << "\n";
    csFile << "public class " << uniqueName << "\n{\n";
    csFile << "    // Fields\n";

    // fields (with runtime plausibility validation like the browser path)
    std::uint16_t fmax =
        std::min<std::uint16_t>(fieldCount,
                                static_cast<std::uint16_t>(
                                    std::min<std::uint32_t>(
                                        options.maxFieldsPerKlass, 0xFFFFu)));
    if (fieldsPtr && detail_self_dump::IsCanonicalPtr(fieldsPtr)) {
      for (std::uint16_t fi = 0; fi < fmax; ++fi) {
        const std::uintptr_t finfo =
            fieldsPtr +
            static_cast<std::uintptr_t>(fi) * profile->fieldInfo.size;
        std::uintptr_t fnamePtr = 0;
        std::string fname;
        const bool nameRead =
            ReadPtr(mem, finfo + profile->fieldInfo.name, fnamePtr) &&
            detail_self_dump::IsCanonicalPtr(fnamePtr) &&
            detail_self_dump::ReadRuntimeCStringSafe(mem, fnamePtr, fname);
        if (!nameRead || !detail_self_dump::IsPrintableName(fname))
          continue;
        std::int32_t rawOff = -1;
        (void)ReadValue(mem, finfo + profile->fieldInfo.offset, rawOff);
        if (rawOff < 0 || rawOff >= 0x10000)
          continue;
        std::uintptr_t ftypePtr = 0;
        if (!ReadPtr(mem, finfo + profile->fieldInfo.type, ftypePtr) ||
            !detail_self_dump::IsCanonicalPtr(ftypePtr))
          continue;
        std::string ftype = "object";
        RuntimeIl2CppTypeDescription desc;
        if (DescribeRuntimeIl2CppType(mem, *profile, ftypePtr, desc) &&
            !desc.name.empty() && desc.name.size() < 200) {
          ftype = desc.name;
        }
        const std::uint32_t offU = static_cast<std::uint32_t>(rawOff);
        csFile << "    public " << ftype << " " << fname << "; // offset:0x"
               << std::hex << std::uppercase << offU << std::dec << "\n";
        ++fieldRows;
      }
    }

    csFile << "\n    // Methods\n";

    // methods (with runtime plausibility validation like the browser path)
    std::uint16_t mmax =
        std::min<std::uint16_t>(methodCount,
                                static_cast<std::uint16_t>(
                                    std::min<std::uint32_t>(
                                        options.maxMethodsPerKlass, 0xFFFFu)));
    if (methodsPtr && detail_self_dump::IsCanonicalPtr(methodsPtr)) {
      for (std::uint16_t mi = 0; mi < mmax; ++mi) {
        std::uintptr_t minfo = 0;
        if (!ReadPtr(mem,
                     methodsPtr + static_cast<std::uintptr_t>(mi) *
                                      sizeof(std::uintptr_t),
                     minfo) ||
            !detail_self_dump::IsCanonicalPtr(minfo))
          continue;
        std::uintptr_t mnamePtr = 0;
        std::string mname;
        if (!ReadPtr(mem, minfo + profile->methodInfo.name, mnamePtr) ||
            !detail_self_dump::IsCanonicalPtr(mnamePtr) ||
            !detail_self_dump::ReadRuntimeCStringSafe(mem, mnamePtr, mname))
          continue;
        if (!detail_self_dump::IsReadableMethodName(mname))
          continue;
        std::uintptr_t methodPtr = 0;
        std::uintptr_t retTypePtr = 0;
        std::uint32_t token = 0;
        (void)ReadPtr(mem, minfo + profile->methodInfo.methodPointer,
                      methodPtr);
        (void)ReadPtr(mem, minfo + profile->methodInfo.returnType, retTypePtr);
        (void)ReadValue(mem, minfo + profile->methodInfo.token, token);
        // Validation: method name readable and, when methodPtr is present, it
        // must live inside GameAssembly. Other fields (invoker/ret/params/token
        // /slot) are informational only and must not reject a valid method.
        if (methodPtr != 0 &&
            !detail_self_dump::IsRuntimeAddressInsideGameAssembly(methodPtr))
          continue;
        std::uint64_t rva = 0;
        if (detail_self_dump::IsRuntimeAddressInsideGameAssembly(methodPtr)) {
          rva = static_cast<std::uint64_t>(methodPtr - g_ctx.gameAssembly.base);
          ++methodRva;
        }
        std::string retName = "void";
        if (retTypePtr && detail_self_dump::IsCanonicalPtr(retTypePtr)) {
          RuntimeIl2CppTypeDescription desc;
          if (DescribeRuntimeIl2CppType(mem, *profile, retTypePtr, desc) &&
              !desc.name.empty() && desc.name.size() < 200) {
            retName = desc.name;
          }
        }
        csFile << "    // RVA: 0x" << std::hex << std::uppercase << rva
               << " VA: 0x" << methodPtr << std::dec;
        if (token && (token & 0xFF000000u) == 0x06000000u)
          csFile << " Token: 0x" << std::hex << std::uppercase << token
                 << std::dec;
        csFile << "\n";
        csFile << "    public " << retName << " " << mname << "() { }\n";
        ++methodRows;
      }
    }

    csFile << "}\n\n";
  }

  csFile.flush();
  csFile.close();

  stats.klassResolved = klasses.size();
  stats.fieldRows = fieldRows;
  stats.methodRows = methodRows;
  stats.methodRva = methodRva;

  log("[SelfDump] done: klass=" + std::to_string(klasses.size()) +
      " fields=" + std::to_string(fieldRows) +
      " methods=" + std::to_string(methodRows) +
      " methodsWithRva=" + std::to_string(methodRva) +
      " -> " + options.outPath);

  if (logFile.is_open())
    logFile.close();
  return true;
}

} // namespace er2
