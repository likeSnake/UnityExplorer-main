// ============================================================================
// SuperDumper.cpp  ->  SuperDumper.dll
// Super 变体（GameAssembly_Super.dll / GameAssembly_Super_IBT.dll，VMProtect 加固、
// il2cpp 导出全部被裁掉）的注入式 dump 入口。
//
// 与 MemoryStructDumper.dll 的差别只在「引导」：
//   · 普通体：GetModuleHandleA("GameAssembly.dll") + 4 个 bootstrap 导出
//   · Super ：模块发现（PEB/枚举/PE 头扫描） + 全内存 Il2CppClass 扫描
//              + klass->token 还原 TypeDefinitionIndex + klass->image 推导镜像
// 结构游走、布局校准、输出格式完全复用 il2cpp_dump_core.hpp。
//
// 按模块身份判断变体；Super 即使保留 bootstrap 导出，也使用扩大的镜像发现范围。
// 配置：DLL 同目录 SuperDumper.cfg（回退 MemoryStructDumper.cfg / Il2CppRuntimeDumper.cfg）
//   第一行 = 输出目录；其余 key=value：outdir/images/fields/properties/methods/
//   instance/classrvas/offsetsummary/structureindexes/metadata/outheuristic/maxclasses/
//   maxfields/maxmethods/maxprops/maxifaces（0=按原始计数逐项验证）
// 输出：默认写入 C:\\YJDumped\\YYYYMMDD_HHMMSS_mmm\\
//       目录中的 dump_YYYYMMDD_HHMMSS_mmm.cs、log.txt 及 _structures 产物。
//       如果 SuperDumper.cfg 明确指定了 outdir，则把它作为根目录，仍创建时间戳子目录。
// ============================================================================
#include <Windows.h>
#include <psapi.h>
#include <TlHelp32.h>
#include <cstdio>
#include <cstring>
#include <fstream>
#include <iomanip>
#include <sstream>
#include <string>
#include <unordered_set>
#include <vector>

#include "il2cpp_dump_core.hpp"
#include "hero_catalog_runtime.hpp"

#pragma comment(lib, "psapi.lib")

namespace {

using namespace ildump;

char g_dir[MAX_PATH] = "C:\\YJDumped";
bool g_outDirConfigured = false;
Options g_opt;
FILE *g_logFile = nullptr;
LogCtx g_log;
std::vector<std::string> g_cfgWarnings;
std::string g_runtimeCatalogMode = "auto";

void LogToFile(void *ud, const char *msg) {
  (void)ud;
  if (g_logFile) {
    fputs(msg, g_logFile);
    fputc('\n', g_logFile);
    fflush(g_logFile);
  }
  OutputDebugStringA(msg);
  OutputDebugStringA("\n");
}

std::string Trim(const std::string &s) {
  size_t a = s.find_first_not_of(" \t\r\n");
  if (a == std::string::npos) return "";
  size_t b = s.find_last_not_of(" \t\r\n");
  return s.substr(a, b - a + 1);
}

bool LoadCfg(HMODULE hModule) {
  char dllPath[MAX_PATH] = {};
  GetModuleFileNameA(hModule, dllPath, MAX_PATH);
  std::string p(dllPath);
  size_t dot = p.rfind('.');
  std::string base = (dot == std::string::npos) ? p : p.substr(0, dot);
  size_t slash = base.find_last_of("\\/");
  std::string dir = (slash == std::string::npos) ? "" : base.substr(0, slash + 1);

  std::vector<std::string> files = {base + ".cfg",
                                    dir + "MemoryStructDumper.cfg",
                                    dir + "Il2CppRuntimeDumper.cfg"};
  FILE *f = nullptr;
  for (auto &c : files) {
    f = fopen(c.c_str(), "r");
    if (f) break;
  }
  if (!f) return false;

  char line[1024];
  int lineNo = 0;
  while (fgets(line, sizeof(line), f)) {
    ++lineNo;
    std::string s = Trim(line);
    if (s.empty() || s[0] == '#' || s[0] == ';') continue;
    size_t eq = s.find('=');
    if (eq == std::string::npos) {
      if (lineNo == 1) {
        strncpy_s(g_dir, s.c_str(), _TRUNCATE);
        g_outDirConfigured = true;
      }
      continue;
    }
    std::string k = Trim(s.substr(0, eq));
    std::string v = Trim(s.substr(eq + 1));
    if (k == "outdir" || k == "out") {
      strncpy_s(g_dir, v.c_str(), _TRUNCATE);
      g_outDirConfigured = true;
    }
    else if (k == "images") g_opt.imageFilter = v;
    else if (k == "fields") g_opt.fields = (v != "0");
    else if (k == "properties") g_opt.properties = (v != "0");
    else if (k == "methods") g_opt.methods = (v != "0");
    else if (k == "instance") g_opt.printInstance = (v != "0");
    else if (k == "classrvas" || k == "class_rvas") g_opt.classRvas = (v != "0");
    else if (k == "offsetsummary") g_opt.offsetSummary = (v != "0");
    else if (k == "structureindexes") g_opt.structureIndexes = (v != "0");
    else if (k == "metadata") g_opt.useMetadata = (v != "0");
    else if (k == "outheuristic") g_opt.outHeuristic = (v != "0");
    else if (k == "runtimecatalog" || k == "runtime_catalog") g_runtimeCatalogMode = v;
    else if (k == "maxclasses") g_opt.maxClassesPerImage = (uint32_t)strtoul(v.c_str(), nullptr, 0);
    else if (k == "maxfields" || k == "maxmethods" || k == "maxprops" || k == "maxifaces") {
      char *end = nullptr;
      const unsigned long n = strtoul(v.c_str(), &end, 0);
      if (v.empty() || v[0] == '-' || !end || *end || n > 65535)
        g_cfgWarnings.push_back("忽略无效成员上限：" + k + "=" + v + "；允许 0 至 65535，0 表示按原始计数遍历");
      else if (k == "maxfields") g_opt.maxFields = (uint32_t)n;
      else if (k == "maxmethods") g_opt.maxMethods = (uint32_t)n;
      else if (k == "maxprops") g_opt.maxProps = (uint32_t)n;
      else g_opt.maxIfaces = (uint32_t)n;
    }
  }
  fclose(f);
  return true;
}

void EnsureDir(const char *dir) {
  std::string s(dir);
  for (size_t i = 0; i < s.size(); ++i) {
    if (s[i] == '\\' || s[i] == '/') {
      if (i == 0) continue;
      std::string sub = s.substr(0, i);
      CreateDirectoryA(sub.c_str(), nullptr);
    }
  }
  CreateDirectoryA(dir, nullptr);
}

// ---------------------------------------------------------------------------
// 模块发现（无导出依赖）
// ---------------------------------------------------------------------------
bool ModuleLooksLikeGameAssembly(uintptr_t base) {
  if (!base) return false;
  const unsigned char *p = (const unsigned char *)base;
  unsigned char hdr[0x400];
  if (!SafeCopy(hdr, p, sizeof(hdr))) return false;
  if (hdr[0] != 'M' || hdr[1] != 'Z') return false;
  uint32_t lfanew = *(const uint32_t *)(hdr + 0x3C);
  if (lfanew < 0x40 || lfanew > 0x1000) return false;
  unsigned char nt[0x108];
  if (!SafeCopy(nt, p + lfanew, sizeof(nt))) return false;
  if (nt[0] != 'P' || nt[1] != 'E' || nt[2] != 0 || nt[3] != 0) return false;
  uint16_t machine = *(const uint16_t *)(nt + 4);
  if (machine != 0x8664) return false;
  uint16_t nsec = *(const uint16_t *)(nt + 6);
  uint32_t sizeOfImage = *(const uint32_t *)(nt + 0x50);
  if (nsec == 0 || nsec > 96) return false;
  if (sizeOfImage < (10u << 20) || sizeOfImage > (3u << 30)) return false;
  // 节名里出现 .vmp0/.vmp1 或导出表含 il2cpp_get_corlib 即认为命中
  unsigned char sec[40];
  for (uint16_t i = 0; i < nsec; ++i) {
    if (!SafeCopy(sec, p + lfanew + 0x18 + *(const uint16_t *)(nt + 0x14) + (size_t)i * 40,
                  sizeof(sec)))
      break;
    if (!memcmp(sec, ".vmp", 4)) return true;
  }
  // 导出表探测（dataDirectory[0] 在 PE32+ 的 nt+0x18+0x70+0x10 = nt+0x98）
  uint32_t expRva = *(const uint32_t *)(nt + 0x98);
  if (expRva) {
    unsigned char expDir[40];
    if (SafeCopy(expDir, p + expRva, sizeof(expDir))) {
      uint32_t nNames = *(const uint32_t *)(expDir + 24);
      uint32_t addrNames = *(const uint32_t *)(expDir + 32);
      if (nNames > 0 && nNames < 20000 && addrNames) {
        unsigned char nameRva[4];
        if (SafeCopy(nameRva, p + addrNames, 4)) {
          uint32_t rva0 = *(const uint32_t *)nameRva;
          char buf[64];
          if (SafeCopy(buf, p + rva0, sizeof(buf)) && !memcmp(buf, "il2cpp_get_corlib", 17))
            return true;
        }
      }
    }
  }
  return false;
}

bool ModuleHasVmpSections(uintptr_t base) {
  if (!base) return false;
  unsigned char hdr[0x400] = {};
  if (!SafeCopy(hdr, (const void *)base, sizeof(hdr)) || hdr[0] != 'M' || hdr[1] != 'Z') return false;
  uint32_t lfanew = *(const uint32_t *)(hdr + 0x3C);
  if (lfanew < 0x40 || lfanew > 0x1000) return false;
  unsigned char nt[0x108] = {};
  if (!SafeCopy(nt, (const void *)(base + lfanew), sizeof(nt)) || nt[0] != 'P' || nt[1] != 'E' ||
      nt[2] != 0 || nt[3] != 0)
    return false;
  uint16_t nsec = *(const uint16_t *)(nt + 6);
  uint16_t optSize = *(const uint16_t *)(nt + 20);
  if (nsec == 0 || nsec > 96 || optSize < 0x70) return false;
  const uintptr_t secBase = base + lfanew + 0x18 + optSize;
  unsigned char sec[40] = {};
  for (uint16_t i = 0; i < nsec; ++i) {
    if (!SafeCopy(sec, (const void *)(secBase + (size_t)i * 40), sizeof(sec))) break;
    if (!memcmp(sec, ".vmp", 4)) return true;
  }
  return false;
}

struct ModuleInfo {
  uintptr_t base = 0;
  size_t size = 0;
  std::string name;
  std::string how;
};

// A Super build may still expose a small set of il2cpp_* entry points after
// VMProtect has initialized them.  Those exports are not a complete class
// enumeration API, so treating their presence as proof of a normal build
// silently limits the dump to the framework images.  Decide the route from
// the module identity first; only an actual GameAssembly.dll may use the
// bootstrap path.
bool IsSuperVariant(const ModuleInfo &mod) {
  std::string s = mod.name;
  for (char &c : s) {
    if (c >= 'A' && c <= 'Z') c = (char)(c - 'A' + 'a');
  }
  return s.find("gameassembly_super") != std::string::npos ||
         s.find("_super_ibt") != std::string::npos;
}

bool FindGameAssemblyModule(LogCtx &log, ModuleInfo &out, int waitSeconds) {
  static const char *kNames[] = {"GameAssembly_Super.dll", "GameAssembly_Super_IBT.dll",
                                 "GameAssembly.dll"};
  // ① GetModuleHandle 精确名（含等待）
  for (int tick = 0; tick < waitSeconds * 100; ++tick) {
    for (const char *n : kNames) {
      HMODULE m = GetModuleHandleA(n);
      if (!m) continue;
      MODULEINFO mi = {};
      if (GetModuleInformation(GetCurrentProcess(), m, &mi, sizeof(mi))) {
        out.base = (uintptr_t)m;
        out.size = mi.SizeOfImage;
        out.name = n;
        out.how = "GetModuleHandleA";
        return true;
      }
    }
    Sleep(10);
  }
  // ② 枚举进程模块（名字含 GameAssembly）
  HMODULE mods[1024];
  DWORD need = 0;
  if (EnumProcessModules(GetCurrentProcess(), mods, sizeof(mods), &need)) {
    DWORD cnt = need / sizeof(HMODULE);
    if (cnt > 1024) cnt = 1024;
    for (DWORD i = 0; i < cnt; ++i) {
      char path[MAX_PATH] = {};
      if (!GetModuleFileNameExA(GetCurrentProcess(), mods[i], path, MAX_PATH)) continue;
      std::string p(path);
      if (p.find("GameAssembly") == std::string::npos) continue;
      MODULEINFO mi = {};
      if (!GetModuleInformation(GetCurrentProcess(), mods[i], &mi, sizeof(mi))) continue;
      out.base = (uintptr_t)mods[i];
      out.size = mi.SizeOfImage;
      out.name = p;
      out.how = "EnumProcessModules";
      return true;
    }
  }
  // ③ PE 头扫描（模块被反作弊从 loader 列表隐藏时的兜底）
  uintptr_t p = 0x10000;
  MEMORY_BASIC_INFORMATION mbi;
  while (VirtualQuery((LPCVOID)p, &mbi, sizeof(mbi))) {
    uintptr_t lo = (uintptr_t)mbi.BaseAddress;
    uintptr_t hi = lo + mbi.RegionSize;
    if (mbi.State == MEM_COMMIT && mbi.Type == MEM_IMAGE &&
        !(mbi.Protect & (PAGE_GUARD | PAGE_NOACCESS))) {
      if (ModuleLooksLikeGameAssembly(lo)) {
        unsigned char nt[0x108];
        uint32_t lfanew = 0;
        if (SafeCopy(&lfanew, (const void *)(lo + 0x3C), 4) &&
            SafeCopy(nt, (const void *)(lo + lfanew), sizeof(nt))) {
          out.base = lo;
          out.size = *(const uint32_t *)(nt + 0x50);
          out.name = ModuleHasVmpSections(lo) ? "GameAssembly_Super.dll (PE scan)"
                                              : "GameAssembly.dll (PE scan)";
          out.how = "PE header scan";
          return true;
        }
      }
    }
    if (hi <= p) break;
    p = hi;
  }
  log.line("[FAIL] GameAssembly module not found");
  return false;
}

// SEH 边界：放在无 C++ 局部对象的薄函数里（MSVC C2712）
__declspec(noinline) int RunDumpWithSeh(Dumper *d, unsigned long *crashCode) {
  __try {
    return d->run() ? 0 : 1;
  } __except (EXCEPTION_EXECUTE_HANDLER) {
    if (crashCode) *crashCode = GetExceptionCode();
    return 2;
  }
}

BOOL CALLBACK FindDumpWindow(HWND hwnd, LPARAM param) {
  auto *env = reinterpret_cast<Env *>(param);
  DWORD pid = 0;
  GetWindowThreadProcessId(hwnd, &pid);
  RECT rect = {};
  if (pid == GetCurrentProcessId() && IsWindowVisible(hwnd) &&
      !GetWindow(hwnd, GW_OWNER) && GetClientRect(hwnd, &rect)) {
    int width = rect.right - rect.left, height = rect.bottom - rect.top;
    if (width >= 320 && height >= 200 && width <= 32768 && height <= 32768 &&
        (int64_t)width * height > (int64_t)env->windowWidth * env->windowHeight) {
      env->windowWidth = width;
      env->windowHeight = height;
    }
  }
  return TRUE;
}

DWORD WINAPI DumpWorker(LPVOID param) {
  HMODULE self = (HMODULE)param;
  // Keep all file-system and C++ container work out of DllMain/loader lock.
  g_opt.structureIndexes = true;
  const bool cfgLoaded = LoadCfg(self);
  SYSTEMTIME started = {};
  GetLocalTime(&started);
  char timestamp[32] = {};
  snprintf(timestamp, sizeof(timestamp), "%04u%02u%02u_%02u%02u%02u_%03u",
           (unsigned)started.wYear, (unsigned)started.wMonth,
           (unsigned)started.wDay, (unsigned)started.wHour,
           (unsigned)started.wMinute, (unsigned)started.wSecond,
           (unsigned)started.wMilliseconds);
  const std::string outputRoot = g_dir;
  const std::string timestampDir = outputRoot + "\\" + timestamp;
  strncpy_s(g_dir, timestampDir.c_str(), _TRUNCATE);
  EnsureDir(g_dir);
  char dumpFileName[64] = {};
  snprintf(dumpFileName, sizeof(dumpFileName), "dump_%s.cs", timestamp);
  g_opt.outPath = std::string(g_dir) + "\\" + dumpFileName;
  std::string logPath = std::string(g_dir) + "\\log.txt";
  g_logFile = fopen(logPath.c_str(), "w");
  g_log.f = nullptr;  // 只走 fn（fn 内部写文件），避免重复写
  g_log.fn = LogToFile;
  g_log.line("=== SuperDumper v9 (接口校验、完整成员遍历与质量诊断) start ===");
  for (const auto &message : g_cfgWarnings) g_log.line("[配置] %s", message.c_str());
  g_log.line("[配置] 成员上限：字段=%u 方法=%u 属性=%u 接口=%u；0 表示使用原始 uint16 计数并逐项验证",
    g_opt.maxFields, g_opt.maxMethods, g_opt.maxProps, g_opt.maxIfaces);
  g_log.line("[0] pid=%lu tid=%lu outdir=%s cfg=%s", GetCurrentProcessId(), GetCurrentThreadId(),
             g_dir, cfgLoaded ? "loaded" : "default");
  g_log.line("[0] dumpFile=%s (timestamp=local run start)", g_opt.outPath.c_str());
  g_log.line("[0] outputMode=%s root=%s timestamp=%s",
             g_outDirConfigured ? "configured-timestamp-folder"
                                : "default-timestamp-folder",
             outputRoot.c_str(), timestamp);

  ModuleInfo mod;
  if (!FindGameAssemblyModule(g_log, mod, 30)) {
    if (g_logFile) fclose(g_logFile);
    FreeLibraryAndExitThread(self, 0);
    return 0;
  }
  g_log.line("[0] module=%s base=0x%llx size=0x%llX how=%s", mod.name.c_str(),
             (unsigned long long)mod.base, (unsigned long long)mod.size, mod.how.c_str());

  Env env;
  env.moduleBase = mod.base;
  env.moduleSize = mod.size;
  // A known normal GameAssembly.dll may itself contain .vmp sections.
  const bool superVariant = IsSuperVariant(mod) ||
      (_stricmp(mod.name.c_str(), "GameAssembly.dll") != 0 && ModuleHasVmpSections(mod.base));
  env.moduleName = mod.name;
  env.superVariant = superVariant;
  env.executableBase = (uintptr_t)GetModuleHandleW(nullptr);
  env.otherVariantLoaded = superVariant ? GetModuleHandleA("GameAssembly.dll") != nullptr
      : (GetModuleHandleA("GameAssembly_Super.dll") != nullptr ||
         GetModuleHandleA("GameAssembly_Super_IBT.dll") != nullptr);
  const char *players[] = {superVariant ? "UnityPlayer_LVB.dll" : "UnityPlayer.dll",
                           superVariant ? "UnityPlayer.dll" : "UnityPlayer_LVB.dll"};
  for (const char *name : players) {
    HMODULE player = GetModuleHandleA(name);
    MODULEINFO info = {};
    if (player && GetModuleInformation(GetCurrentProcess(), player, &info, sizeof(info))) {
      env.unityPlayerBase = (uintptr_t)player;
      env.unityPlayerSize = info.SizeOfImage;
      env.unityPlayerName = name;
      break;
    }
  }
  EnumWindows(FindDumpWindow, (LPARAM)&env);
  // Some Super builds retain a small but usable image API surface. Keep those
  // functions as the class-enumeration authority, while broadening image
  // discovery to every writable private allocation. This avoids both the
  // incomplete corlib-local result and the over-strict pure class scan.
  HMODULE hmod = (HMODULE)mod.base;
  auto probeCorlib = (void *(*)())GetProcAddress(hmod, "il2cpp_get_corlib");
  auto probeImageGetClass =
      (void *(*)(void *, size_t))GetProcAddress(hmod, "il2cpp_image_get_class");
  auto probeImageGetClassCount =
      (size_t(*)(void *))GetProcAddress(hmod, "il2cpp_image_get_class_count");
  auto probeImageGetName =
      (const char *(*)(void *))GetProcAddress(hmod, "il2cpp_image_get_name");
  auto probeImageGetAssembly =
      (void *(*)(void *))GetProcAddress(hmod, "il2cpp_image_get_assembly");
  const bool bootstrap = probeCorlib && probeImageGetClass && probeImageGetClassCount && probeImageGetName;
  g_log.line("[0] variant=%s bootstrap exports: %s (corlib=%p image_get_class=%p count=%p name=%p assembly=%p)",
             superVariant ? "Super" : "normal", bootstrap ? "available" : "absent",
             (void *)probeCorlib, (void *)probeImageGetClass, (void *)probeImageGetClassCount,
             (void *)probeImageGetName, (void *)probeImageGetAssembly);
  if (bootstrap) {
    env.getCorlib = probeCorlib;
    env.imageGetClass = probeImageGetClass;
    env.imageGetClassCount = probeImageGetClassCount;
    env.imageGetName = probeImageGetName;
    env.imageGetAssembly = probeImageGetAssembly;
    if (superVariant) {
      env.scanAllMemoryForImages = true;
      env.expectMainImage = true;
      g_log.line("[0] Super module has usable compatibility exports: using full private image scan + API class enumeration");
    }
  } else {
    // A genuinely export-free build still uses the older pure-memory fallback.
    env.getCorlib = nullptr;
    env.imageGetClass = nullptr;
    env.imageGetClassCount = nullptr;
    env.imageGetName = nullptr;
    env.imageGetAssembly = nullptr;
    if (superVariant) g_log.line("[0] Super module detected: forcing pure memory scan");
  }

  g_opt.logPath = logPath;

  unsigned long crashCode = 0;
  int rc = 1;
  bool qualityWarnings = false;
  {
    Dumper d;
    d.env = &env;
    d.opt = &g_opt;
    d.log = &g_log;
    rc = RunDumpWithSeh(&d, &crashCode);
    if (rc == 0 && g_runtimeCatalogMode != "off" && g_runtimeCatalogMode != "0" &&
        g_runtimeCatalogMode != "false" && g_runtimeCatalogMode != "disabled") {
      std::string catalogError;
      if (!CaptureHeroCatalogRuntime(d, g_dir, g_runtimeCatalogMode, &catalogError)) {
        g_log.line("[hero-catalog] live memory capture unavailable: %s", catalogError.c_str());
        if (g_runtimeCatalogMode == "required") rc = 1;
      }
    }
    qualityWarnings = d.quality.warnings();
  } // Release collectors and close files before FreeLibraryAndExitThread.
  if (rc == 2) g_log.line("[CRASH] 0x%08lX (dump aborted, target untouched)", crashCode);
  bool ok = (rc == 0);
  g_log.line("[4] result=%s", ok ? (qualityWarnings ? "OK_WITH_WARNINGS" : "OK") : "FAIL");
  if (g_logFile) {
    fclose(g_logFile);
    g_logFile = nullptr;
  }
  FreeLibraryAndExitThread(self, 0);
  return 0;
}

}  // namespace

BOOL APIENTRY DllMain(HMODULE hModule, DWORD reason, LPVOID) {
  if (reason == DLL_PROCESS_ATTACH) {
    DisableThreadLibraryCalls(hModule);
    HANDLE h = CreateThread(nullptr, 0, DumpWorker, (LPVOID)hModule, 0, nullptr);
    if (h) CloseHandle(h);
  }
  return TRUE;
}
