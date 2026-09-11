// ============================================================================
// StructWalkProbe v25 - 决定性实验：
//   (1) 检测 kernel32!GetProcAddress 是否被 inline hook（前 16 字节）
//   (2) 直接解析 GameAssembly.dll 内存导出表（绕过 GetProcAddress），
//       列出全部 il2cpp_* 导出名；对关键反射 getter 同时输出
//       "导出表 RVA" 与 "GetProcAddress 结果" 以对比判定是否为 hook 所致
//   (3) 系统扫描 metadata 区，收集全部不同 image（名字 + 类数）——为全量 dump 铺路
// 输出 C:\walkprobe.log。编译 StructWalkProbe.vcxproj Release|x64。
// ============================================================================
#include <Windows.h>
#include <psapi.h>
#include <cstdio>
#include <cstdint>
#include <cstring>

namespace {
char g_logPath[MAX_PATH] = {};
void SLog(const char *fmt, ...) {
  FILE *f = fopen(g_logPath, "a");
  if (!f) return;
  va_list ap; va_start(ap, fmt);
  vfprintf(f, fmt, ap);
  va_end(ap);
  fprintf(f, "\n");
  fclose(f);
}
inline bool IsReadable(const void *p, size_t len) {
  if (!p) return false;
  MEMORY_BASIC_INFORMATION mbi = {};
  if (VirtualQuery(p, &mbi, sizeof(mbi)) == 0) return false;
  DWORD prot = mbi.Protect;
  if (prot == 0 || (prot & 0x100)) return false;
  if (prot & (PAGE_GUARD | PAGE_NOCACHE)) return false;
  return true;
}

static const char *const kKey[] = {
    "il2cpp_class_get_name", "il2cpp_class_get_namespace", "il2cpp_class_get_fields",
    "il2cpp_class_get_methods", "il2cpp_class_for_each",
    "il2cpp_class_get_method_from_name", "il2cpp_class_get_field_from_name",
    "il2cpp_domain_get", "il2cpp_domain_get_assemblies", "il2cpp_assembly_get_image",
    "il2cpp_field_get_name", "il2cpp_field_get_offset", "il2cpp_method_get_name",
    "il2cpp_get_corlib", "il2cpp_image_get_class", "il2cpp_image_get_class_count",
    "il2cpp_class_get_properties", "il2cpp_property_get_name",
    "il2cpp_class_get_parent", "il2cpp_class_get_type", "il2cpp_type_get_name",
    "il2cpp_class_is_enum", "il2cpp_class_is_valuetype", "il2cpp_class_get_flags"};
static const int kKeyN = sizeof(kKey) / sizeof(kKey[0]);

uintptr_t g_imgs[1024];
char g_imgn[1024][64];
int g_nimg = 0;

DWORD WINAPI WalkWorker(LPVOID) {
  __try {
  SLog("=== v25 start ===");
  HMODULE ga = GetModuleHandleA("GameAssembly.dll");
  if (!ga) { SLog("[!] no GA"); return 0; }

  {
    const unsigned char *gpa = (const unsigned char *)(void *)GetProcAddress;
    char hex[80] = {}; int no = 0;
    for (int i = 0; i < 16; ++i) no += sprintf_s(hex + no, sizeof(hex) - no, "%02X ", gpa[i]);
    SLog("[1] kernel32!GetProcAddress first16: %s", hex);
  }

  __try {
    const unsigned char *base = (const unsigned char *)ga;
    unsigned long e_lfanew = *(const unsigned long *)(base + 0x3C);
    const unsigned char *nt = base + e_lfanew;
    const unsigned char *opt = nt + 0x18;
    unsigned short magic = *(const unsigned short *)opt;
    unsigned long expRva = (magic == 0x20B) ? *(const unsigned long *)(opt + 0x70)
                                            : *(const unsigned long *)(opt + 0x60);
    SLog("[2] PE magic=0x%04X exportDirRVA=0x%08lX", magic, expRva);
    const unsigned char *exp = base + expRva;
    unsigned long nNames = *(const unsigned long *)(exp + 24);
    unsigned long nFuncs = *(const unsigned long *)(exp + 20);
    unsigned long addrNames = *(const unsigned long *)(exp + 32);
    unsigned long addrOrds = *(const unsigned long *)(exp + 36);
    unsigned long addrFuncs = *(const unsigned long *)(exp + 28);
    SLog("[2] NumberOfNames=%lu NumberOfFunctions=%lu", nNames, nFuncs);

    const unsigned long *names = (const unsigned long *)(base + addrNames);
    const unsigned short *ords = (const unsigned short *)(base + addrOrds);
    const unsigned long *funcs = (const unsigned long *)(base + addrFuncs);

    SLog("[3] key names: exportRVA vs GetProcAddress");
    for (int k = 0; k < kKeyN; ++k) {
      unsigned long rva = 0; bool found = false;
      for (unsigned long i = 0; i < nNames && i < 20000; ++i) {
        const char *nm = (const char *)(base + names[i]);
        if (!IsReadable(nm, 4)) continue;
        if (!strcmp(nm, kKey[k])) { rva = funcs[ords[i]]; found = true; break; }
      }
      void *g = GetProcAddress(ga, kKey[k]);
      SLog("[3]  %-42s exportRVA=%s0x%08lX  GPA=0x%p",
           kKey[k], found ? "" : "MISS ", rva, g);
    }

    int nIl2 = 0;
    for (unsigned long i = 0; i < nNames && i < 20000; ++i) {
      const char *nm = (const char *)(base + names[i]);
      if (!IsReadable(nm, 4)) continue;
      if (nm[0] == 'i' && nm[1] == 'l' && nm[2] == '2' && nm[3] == 'c') {
        ++nIl2;
        SLog("[4]  %s -> 0x%X", nm, funcs[ords[i]]);
      }
    }
    SLog("[4] total il2cpp_* in export table = %d", nIl2);
  } __except (EXCEPTION_EXECUTE_HANDLER) {
    SLog("[2] export-table parse CRASH 0x%08lX", GetExceptionCode());
  }

  auto get_corlib = (void *(*)(void))GetProcAddress(ga, "il2cpp_get_corlib");
  auto image_get_name = (const char *(*)(const void *))GetProcAddress(ga, "il2cpp_image_get_name");
  auto image_get_count = (size_t(*)(const void *))GetProcAddress(ga, "il2cpp_image_get_class_count");
  if (get_corlib && image_get_name && image_get_count) {
    void *corlib = get_corlib();
    uintptr_t base = (uintptr_t)corlib;
    uintptr_t lo = (base & ~0xFFFULL) - 0x40000;
    uintptr_t hi = (base & ~0xFFFULL) + 0x40000;
    for (uintptr_t p = lo; p < hi && g_nimg < 1024; p += 8) {
      if (!IsReadable((void *)p, 8)) continue;
      uintptr_t v = *(uintptr_t *)p;
      for (int mode = 0; mode < 2 && g_nimg < 1024; ++mode) {
        uintptr_t img = 0;
        if (mode == 0) img = v;
        else { if (!v || !IsReadable((void *)v, 0x20)) continue; img = *(uintptr_t *)v; }
        if (!img || !IsReadable((void *)img, 1)) continue;
        const char *nm = image_get_name((void *)img);
        if (!nm || !*nm) continue;
        if (!strstr(nm, ".dll") && !strstr(nm, ".exe")) continue;
        bool dup = false;
        for (int q = 0; q < g_nimg; ++q) if (!strcmp(g_imgn[q], nm)) { dup = true; break; }
        if (dup) continue;
        strcpy_s(g_imgn[g_nimg], nm);
        g_imgs[g_nimg] = img;
        ++g_nimg;
      }
    }
    SLog("[5] collected %d distinct images:", g_nimg);
    for (int q = 0; q < g_nimg; ++q)
      SLog("[5]  %-48s img=0x%llX classes=%llu", g_imgn[q],
           (unsigned long long)g_imgs[q], (unsigned long long)image_get_count((void *)g_imgs[q]));
  }

  SLog("=== v25 done ===");
  } __except (EXCEPTION_EXECUTE_HANDLER) {
    SLog("[CRASH] 0x%08lX", GetExceptionCode());
  }
  FreeLibraryAndExitThread(GetModuleHandleA("StructWalkProbe.dll"), 0);
  return 0;
}
} // namespace

BOOL APIENTRY DllMain(HMODULE hModule, DWORD reason, LPVOID) {
  if (reason == DLL_PROCESS_ATTACH) {
    DisableThreadLibraryCalls(hModule);
    strcpy_s(g_logPath, "C:\\walkprobe.log");
    DeleteFileA("C:\\walkprobe.log");
    HANDLE h = CreateThread(nullptr, 0, WalkWorker, nullptr, 0, nullptr);
    if (h) CloseHandle(h);
  }
  return TRUE;
}