// ============================================================================
// diag_probe.cpp - 诊断 DLL：注入游戏后输出 il2cpp API 解析状态
// 用于排查 Il2CppRuntimeDumper 的 [FAIL] API resolution failed
// 编译：MSBuild DiagProbe.vcxproj (Release|x64)
// ============================================================================
#include <Windows.h>
#include <psapi.h>
#include <cstdio>

namespace {
char g_diagPath[MAX_PATH] = {};
void DiagLog(const char *fmt, ...) {
  FILE *f = fopen(g_diagPath, "a");
  if (!f)
    return;
  va_list ap;
  va_start(ap, fmt);
  vfprintf(f, fmt, ap);
  va_end(ap);
  fprintf(f, "\n");
  fclose(f);
}

DWORD WINAPI DiagWorker(LPVOID) {
  DiagLog("=== diag worker start ===");

  // 1) GetModuleHandle 直接查找
  HMODULE hGA = GetModuleHandleA("GameAssembly.dll");
  DiagLog("[1] GetModuleHandleA(GameAssembly.dll) = 0x%p", (void *)hGA);

  // 2) 遍历所有 GameAssembly* 模块
  HMODULE modules[2048];
  DWORD needed = 0;
  if (EnumProcessModulesEx(GetCurrentProcess(), modules, sizeof(modules), &needed,
                           LIST_MODULES_ALL)) {
    const DWORD count = needed / sizeof(HMODULE);
    DiagLog("[2] EnumProcessModulesEx total=%u", count);
    int found = 0;
    for (DWORD i = 0; i < count; ++i) {
      char name[MAX_PATH] = {};
      if (GetModuleBaseNameA(GetCurrentProcess(), modules[i], name, sizeof(name))) {
        if (_strnicmp(name, "GameAssembly", 12) == 0) {
          DiagLog("[2]   GameAssembly module: '%s' @ 0x%p", name, (void *)modules[i]);
          found++;
        }
      }
    }
    if (!found)
      DiagLog("[2]   no GameAssembly* module found via EnumProcessModulesEx");
  } else {
    DiagLog("[2] EnumProcessModulesEx FAILED err=%u", GetLastError());
  }

  // 3) 在 hGA 上解析核心导出（若 hGA 有效）
  HMODULE target = hGA;
  if (!target) {
    // fallback: 用普通变体名直接 GetModuleHandle 试变体名
    const char *variants[] = {"GameAssembly_Super.dll", "GameAssembly_Super1.dll",
                              "GameAssembly_Super_IBT.dll", "GameAssembly_Super_IBT1.dll"};
    for (const char *v : variants) {
      HMODULE h = GetModuleHandleA(v);
      DiagLog("[3] GetModuleHandleA(%s) = 0x%p", v, (void *)h);
      if (h && !target)
        target = h;
    }
  }

  if (target) {
    // [A] CONFIRMED-exported (plaintext in on-disk GameAssembly export name
//     table / runtime core): if ANY of these resolve in our channel, then
//     the channel works and the dump failure is the validator requiring
//     stripped type-layout exports.
const char *syms[] = {
    "il2cpp_get_corlib",       "il2cpp_image_get_class",
    "il2cpp_image_get_class_count", "il2cpp_image_get_name",
    "il2cpp_image_get_assembly", "il2cpp_init", "il2cpp_shutdown",
    "il2cpp_thread_attach",
    // 299KB reference DLL critical names (type-layout enumeration)
    "il2cpp_class_for_each",   "il2cpp_class_get_method_from_name",
    "il2cpp_class_get_field_from_name", "il2cpp_class_get_name",
    "il2cpp_field_get_name",   "il2cpp_method_get_name",
    // our validator core names (stripped hypothesis)
    "il2cpp_domain_get",       "il2cpp_domain_get_assemblies",
    "il2cpp_class_get_fields", "il2cpp_class_get_methods",
};
    for (const char *s : syms) {
      FARPROC p = GetProcAddress(target, s);
      DiagLog("[4] GetProcAddress(%s) = 0x%p", s, (void *)p);
    }
    // mono fallback 检查
    const char *monoSyms[] = {"mono_domain_get", "mono_domain_get_assemblies",
                              "mono_class_get_name", "mono_class_get_fields",
                              "mono_field_get_offset", "mono_method_get_name",
                              "mono_class_get_methods", "mono_assembly_get_image",
                              "mono_image_get_name"};
    for (const char *s : monoSyms) {
      FARPROC p = GetProcAddress(target, s);
      DiagLog("[4m] GetProcAddress(%s) = 0x%p", s, (void *)p);
    }
  } else {
    DiagLog("[3] no GameAssembly module handle obtained at all");
  }

  // 4.5) 手工解析 GameAssembly.dll 的 PE 导出表（绕过 GetProcAddress）
  {
    const unsigned char *base = (const unsigned char *)hGA;
    if (base && base[0] == 'M' && base[1] == 'Z') {
      const unsigned char *pe = base + *(const unsigned long *)(base + 0x3C);
      if (pe[0] == 'P' && pe[1] == 'E') {
        const unsigned long opt = pe + 0x18 - (const unsigned char *)0; // placeholder
        (void)opt;
        // Optional header offset = 0x18 from PE signature
        const unsigned char *optHdr = pe + 0x18;
        const unsigned short magic = *(const unsigned short *)optHdr;
        DiagLog("[PE] magic=0x%04X (%s)", magic,
                magic == 0x20B ? "PE32+" : (magic == 0x10B ? "PE32" : "?"));
        // Export directory RVA in data directory index 0
        const unsigned long exportRva =
            magic == 0x20B ? *(const unsigned long *)(optHdr + 0x70)
                           : *(const unsigned long *)(optHdr + 0x60);
        DiagLog("[PE] export dir RVA = 0x%08lX", exportRva);
        if (exportRva) {
          // find the section containing exportRva
          const unsigned short numSections =
              *(const unsigned short *)(pe + 6);
          const unsigned char *secTable =
              optHdr + (magic == 0x20B ? 0xF0 : 0xE0);
          unsigned long rvaToOff = 0;
          for (unsigned short si = 0; si < numSections; ++si) {
            const unsigned char *sec = secTable + si * 40;
            const unsigned long vaddr = *(const unsigned long *)(sec + 12);
            const unsigned long vsize = *(const unsigned long *)(sec + 8);
            const unsigned long rawPtr = *(const unsigned long *)(sec + 20);
            if (exportRva >= vaddr && exportRva < vaddr + vsize) {
              rvaToOff = rawPtr - vaddr;
              break;
            }
          }
          if (rvaToOff) {
            const unsigned char *exp =
                base + rvaToOff + exportRva;
            const unsigned long nameRva = *(const unsigned long *)(exp + 32);
            unsigned long nameOff = nameRva;
            // resolve name RVA to offset
            unsigned long nOff = 0;
            const unsigned char *sec2 = secTable;
            for (unsigned short si = 0; si < numSections; ++si, sec2 += 40) {
              const unsigned long vaddr = *(const unsigned long *)(sec2 + 12);
              const unsigned long vsize = *(const unsigned long *)(sec2 + 8);
              const unsigned long rawPtr = *(const unsigned long *)(sec2 + 20);
              if (nameRva >= vaddr && nameRva < vaddr + vsize) {
                nOff = rawPtr - vaddr;
                break;
              }
            }
            if (nOff)
              nameOff = nameRva + nOff;
            DiagLog("[PE] export DLL name: %s", (const char *)(base + nameOff));
            const unsigned long numFuncs =
                *(const unsigned long *)(exp + 20);
            DiagLog("[PE] number of exports = %lu", numFuncs);
            // 检查 il2cpp_domain_get 是否在名字表里
            const unsigned long namesRva = *(const unsigned long *)(exp + 24);
            // resolve names RVA
            unsigned long namesOff = 0;
            const unsigned char *sec3 = secTable;
            for (unsigned short si = 0; si < numSections; ++si, sec3 += 40) {
              const unsigned long vaddr = *(const unsigned long *)(sec3 + 12);
              const unsigned long vsize = *(const unsigned long *)(sec3 + 8);
              const unsigned long rawPtr = *(const unsigned long *)(sec3 + 20);
              if (namesRva >= vaddr && namesRva < vaddr + vsize) {
                namesOff = rawPtr - vaddr;
                break;
              }
            }
            if (namesOff) {
              int foundIl2Cpp = 0;
              for (unsigned long i = 0; i < numFuncs; ++i) {
                const unsigned long nameEntryRva =
                    *(const unsigned long *)(base + namesOff + namesRva + i * 4);
                unsigned long entryOff = 0;
                const unsigned char *sec4 = secTable;
                for (unsigned short si = 0; si < numSections; ++si, sec4 += 40) {
                  const unsigned long vaddr = *(const unsigned long *)(sec4 + 12);
                  const unsigned long vsize = *(const unsigned long *)(sec4 + 8);
                  const unsigned long rawPtr = *(const unsigned long *)(sec4 + 20);
                  if (nameEntryRva >= vaddr && nameEntryRva < vaddr + vsize) {
                    entryOff = rawPtr - vaddr;
                    break;
                  }
                }
                const char *symName = (const char *)(base + entryOff + nameEntryRva);
                if (symName && strncmp(symName, "il2cpp_", 7) == 0) {
                  foundIl2Cpp++;
                }
              }
              DiagLog("[PE] il2cpp_* exports found in table: %d", foundIl2Cpp);
            }
          } else {
            DiagLog("[PE] export RVA not mapped to file offset");
          }
        }
      } else {
        DiagLog("[PE] invalid PE signature");
      }
    } else {
      DiagLog("[PE] not MZ header");
    }
  }

  // 4.6) 内存中读取 EAT，dump 前 40 个导出函数的前 16 字节（识别 VMP 桩 vs 真实代码）
#ifdef _MSC_VER
    __try {
#endif
  {
    // 从 PE 头读 EAT（进程内直接读模块内存）
    const unsigned char *base = (const unsigned char *)hGA;
    const unsigned char *pe = base + *(const unsigned long *)(base + 0x3C);
    const unsigned char *optHdr = pe + 0x18;
    const unsigned short magic = *(const unsigned short *)optHdr;
    const unsigned long exportRva =
        magic == 0x20B ? *(const unsigned long *)(optHdr + 0x70)
                       : *(const unsigned long *)(optHdr + 0x60);
    // EAT = export dir + 28 (AddressOfFunctions)
    const unsigned long funcsRva = *(const unsigned long *)(base + exportRva + 28);
    const unsigned long numFuncs = *(const unsigned long *)(base + exportRva + 20);
    DiagLog("[EAT] exportRva=0x%08lX numFuncs=%lu funcsRva=0x%08lX", exportRva, numFuncs,
            funcsRva);
    const unsigned char *eat = base + funcsRva;
    for (unsigned long i = 0; i < 40 && i < numFuncs; ++i) {
      const unsigned long funcRva = *(const unsigned long *)(eat + i * 4);
      const unsigned char *fn = base + funcRva;
      // 读 16 字节
      char hex[64] = {};
      for (int b = 0; b < 16; ++b)
        sprintf_s(hex + b * 3, 3, "%02X ", fn[b]);
      DiagLog("[EAT] [%2lu] RVA=0x%08lX bytes=%s", i, funcRva, hex);
    }
    // 内存中完整导出名字表枚举（SEH 保护下逐条读，明确真实可用导出集）
    const unsigned long namesRva = *(const unsigned long *)(base + exportRva + 32);
    const unsigned char *namesArr = base + namesRva;
    const unsigned long numNames = *(const unsigned long *)(base + exportRva + 24);
    DiagLog("[EAT] namesRva=0x%08lX NumberOfNames=%lu", namesRva, numNames);
    unsigned long nIl2 = 0, nMono = 0;
    for (unsigned long i = 0; i < numNames && i < 8000; ++i) {
      const unsigned long strRva = *(const unsigned long *)(namesArr + i * 4);
      const char *s = (const char *)(base + strRva);
      if (s[0] >= 'a' && s[0] <= 'z') {
        // 只打印 il2cpp_* 与全部小写导出名，便于统计
        const bool isIl2 = (s[0] == 'i' && s[1] == 'l' && s[2] == '2' && s[3] == 'c');
        const bool isMono = (s[0] == 'm' && s[1] == 'o' && s[2] == 'n' && s[3] == 'o');
        if (isIl2) {
          nIl2++;
          DiagLog("[EATNAME] %s", s);
        } else if (isMono) {
          nMono++;
          DiagLog("[EATNAME] %s", s);
        }
      }
    }
    DiagLog("[EAT] total il2cpp_* exports = %lu, mono_* = %lu", nIl2, nMono);
  }
#ifdef _MSC_VER
    } __except (EXCEPTION_EXECUTE_HANDLER) {
      DiagLog("[EAT] crash/access-violation while reading export table "
              "(code=0x%08lX) - exports are VMP-encrypted or unreadable in memory",
              GetExceptionCode());
    }
#endif
  HMODULE modules2[2048];
  DWORD needed2 = 0;
  if (EnumProcessModulesEx(GetCurrentProcess(), modules2, sizeof(modules2), &needed2,
                           LIST_MODULES_ALL)) {
    const DWORD count2 = needed2 / sizeof(HMODULE);
    DiagLog("[5] full module list (%u):", count2);
    for (DWORD i = 0; i < count2 && i < 60; ++i) {
      char name[MAX_PATH] = {};
      if (GetModuleBaseNameA(GetCurrentProcess(), modules2[i], name, sizeof(name)))
        DiagLog("[5]   %s @ 0x%p", name, (void *)modules2[i]);
    }
  }

  DiagLog("=== diag worker done ===");
  FreeLibraryAndExitThread(GetModuleHandleA("DiagProbe.dll"), 0);
  return 0;
}
} // namespace

BOOL APIENTRY DllMain(HMODULE hModule, DWORD reason, LPVOID) {
  if (reason == DLL_PROCESS_ATTACH) {
    DisableThreadLibraryCalls(hModule);
    // 输出到 C:\diag.log（避免路径处理复杂性）
    strcpy_s(g_diagPath, "C:\\diag.log");
    HANDLE h = CreateThread(nullptr, 0, DiagWorker, nullptr, 0, nullptr);
    if (h)
      CloseHandle(h);
  }
  return TRUE;
}
