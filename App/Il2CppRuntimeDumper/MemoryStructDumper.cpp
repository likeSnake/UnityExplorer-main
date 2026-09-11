// ============================================================================
// MemoryStructDumper.cpp
// 自研注入式 IL2CPP 全量结构 dumper（内存已解密结构直接偏移游走）。
//
// 产物：<outdir>\dump.cs  —— 格式对齐 F:\gua\dump\dump2026.09.03.cs
//       <outdir>\log.txt  —— 校准/发现/统计日志
//
// 注入方式：CE 注入 / Injector.exe（DLL 名 MemoryStructDumper.dll）
// 配置：DLL 同目录 MemoryStructDumper.cfg（不存在则回退 Il2CppRuntimeDumper.cfg）
//       第一行 = 输出目录；其余行 key=value：
//         outdir=, images=子串(逗号分隔可选), fields=0/1, properties=0/1, methods=0/1,
//         instance=0/1, classrvas=0/1, metadata=0/1, outheuristic=0/1, maxclasses=N
// 只使用本构建确认导出的 4+2 个函数，其余全部结构偏移读取。
// ============================================================================
#include <Windows.h>
#include <psapi.h>
#include <shlobj.h>
#include <cstdio>
#include <cstring>
#include <string>
#include <vector>
#include <chrono>

#include "il2cpp_dump_core.hpp"

#pragma comment(lib, "psapi.lib")
#pragma comment(lib, "shell32.lib")

namespace {

using namespace ildump;

char g_dir[MAX_PATH] = "C:\\selfdump";
Options g_opt;
FILE *g_logFile = nullptr;
LogCtx g_log;
bool g_cfgLoaded = false;

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

  size_t slash = p.find_last_of("\\/");
  std::string dir = slash == std::string::npos ? std::string() : p.substr(0, slash + 1);
  std::vector<std::string> files = {
      base + ".cfg",
      dir + "Il2CppRuntimeDumper.cfg",
      dir + "MemoryStructDumper.cfg",
      std::string("Il2CppRuntimeDumper.cfg"),
      std::string("MemoryStructDumper.cfg")};

  FILE *f = nullptr;
  for (auto &c : files) {
    f = fopen(c.c_str(), "r");
    if (f) break;
  }
  if (!f) {
    // A DLL copied on its own should still leave artifacts next to itself,
    // instead of silently falling back to a machine-specific absolute path.
    if (!dir.empty()) {
      std::string fallback = dir + "out";
      strncpy_s(g_dir, fallback.c_str(), _TRUNCATE);
    }
    return false;
  }

  char line[1024];
  int lineNo = 0;
  while (fgets(line, sizeof(line), f)) {
    ++lineNo;
    std::string s = Trim(line);
    if (s.empty() || s[0] == '#' || s[0] == ';') continue;
    size_t eq = s.find('=');
    if (eq == std::string::npos) {
      if (lineNo == 1) strncpy_s(g_dir, s.c_str(), _TRUNCATE);
      continue;
    }
    std::string k = Trim(s.substr(0, eq));
    std::string v = Trim(s.substr(eq + 1));
    if (k == "outdir" || k == "out") strncpy_s(g_dir, v.c_str(), _TRUNCATE);
    else if (k == "images") g_opt.imageFilter = v;
    else if (k == "fields") g_opt.fields = (v != "0");
    else if (k == "properties") g_opt.properties = (v != "0");
    else if (k == "methods") g_opt.methods = (v != "0");
    else if (k == "instance") g_opt.printInstance = (v != "0");
    else if (k == "classrvas" || k == "class_rvas") g_opt.classRvas = (v != "0");
    else if (k == "metadata") g_opt.useMetadata = (v != "0");
    else if (k == "outheuristic") g_opt.outHeuristic = (v != "0");
    else if (k == "maxclasses") g_opt.maxClassesPerImage = (uint32_t)strtoul(v.c_str(), nullptr, 0);
  }
  fclose(f);
  g_cfgLoaded = true;
  return true;
}

bool EnsureDirectory(const char *path) {
  if (!path || !*path) return false;
  int rc = SHCreateDirectoryExA(nullptr, path, nullptr);
  return rc == ERROR_SUCCESS || rc == ERROR_ALREADY_EXISTS || GetFileAttributesA(path) != INVALID_FILE_ATTRIBUTES;
}

// 等到 GameAssembly.dll 就绪（Unity-Offset 同款策略：Sleep(1) 轮询）
HMODULE WaitForGameAssembly(int maxSeconds, LogCtx *log) {
  const int polls = maxSeconds > 0 ? maxSeconds * 1000 : 1;
  for (int i = 0; i < polls; ++i) {
    HMODULE m = GetModuleHandleA("GameAssembly.dll");
    if (m) {
      if (log && i >= 1000) log->line("[0] GameAssembly appeared after %.3fs", i / 1000.0);
      return m;
    }
    if (log && i && (i % 5000) == 0) log->line("[0] waiting GameAssembly.dll %.1fs", i / 1000.0);
    Sleep(1);
  }
  return nullptr;
}

__declspec(noinline) bool RunDumpWithSeh(Dumper *d, LogCtx *log);

DWORD WINAPI DumpWorker(LPVOID param) {
  HMODULE self = (HMODULE)param;
  // Do all CRT/container work outside DllMain's loader lock.
  LoadCfg(self);
  EnsureDirectory(g_dir);
  std::string logPath = std::string(g_dir) + "\\log.txt";
  g_logFile = fopen(logPath.c_str(), "w");
  // LogToFile owns the file write.  Keeping LogCtx::f null avoids writing
  // every line twice when the callback and the fallback stream are both set.
  g_log.f = nullptr;
  g_log.fn = LogToFile;
  g_log.line("=== MemoryStructDumper v4 (in-memory struct walk) start ===");
  g_log.line("[0] pid=%lu tid=%lu outdir=%s cfg=%s", GetCurrentProcessId(), GetCurrentThreadId(), g_dir,
             g_cfgLoaded ? "loaded" : "default");

  HMODULE ga = WaitForGameAssembly(60, &g_log);
  if (!ga) {
    g_log.line("[FAIL] GameAssembly.dll not found (60s)");
    if (g_logFile) fclose(g_logFile);
    FreeLibraryAndExitThread(self, 0);
    return 0;
  }
  MODULEINFO mi = {};
  GetModuleInformation(GetCurrentProcess(), ga, &mi, sizeof(mi));
  char gaPath[MAX_PATH] = {};
  GetModuleFileNameA(ga, gaPath, MAX_PATH);
  g_log.line("[0] GameAssembly=0x%llx size=0x%llX path=%s", (unsigned long long)(uintptr_t)ga,
             (unsigned long long)mi.SizeOfImage, gaPath[0] ? gaPath : "?");

  Env env;
  env.moduleBase = (uintptr_t)ga;
  env.moduleSize = mi.SizeOfImage;
  env.getCorlib = (void *(*)())GetProcAddress(ga, "il2cpp_get_corlib");
  env.imageGetClass = (void *(*)(void *, size_t))GetProcAddress(ga, "il2cpp_image_get_class");
  env.imageGetClassCount = (size_t(*)(void *))GetProcAddress(ga, "il2cpp_image_get_class_count");
  env.imageGetName = (const char *(*)(void *))GetProcAddress(ga, "il2cpp_image_get_name");
  env.imageGetAssembly = (void *(*)(void *))GetProcAddress(ga, "il2cpp_image_get_assembly");
  g_log.line("[0] api: corlib=%p image_get_class=%p count=%p name=%p assembly=%p", (void *)env.getCorlib,
             (void *)env.imageGetClass, (void *)env.imageGetClassCount, (void *)env.imageGetName,
             (void *)env.imageGetAssembly);
  const char *const probes[] = {"il2cpp_class_for_each", "il2cpp_class_get_fields",
                                "il2cpp_class_get_methods", "il2cpp_domain_get",
                                "il2cpp_domain_get_assemblies", "il2cpp_assembly_get_image"};
  for (const char *name : probes)
    g_log.line("[0] export %-32s = %p", name, GetProcAddress(ga, name));
  if (!env.getCorlib || !env.imageGetClass || !env.imageGetClassCount || !env.imageGetName) {
    g_log.line("[FAIL] bootstrap export missing");
    if (g_logFile) fclose(g_logFile);
    FreeLibraryAndExitThread(self, 0);
    return 0;
  }

  g_opt.outPath = std::string(g_dir) + "\\dump.cs";
  g_opt.logPath = logPath;

  Dumper d;
  d.env = &env;
  d.opt = &g_opt;
  d.log = &g_log;
  bool ok = RunDumpWithSeh(&d, &g_log);
  g_log.line("[4] result=%s", ok ? "OK" : "FAIL");
  if (!ok) g_log.line("[FAIL] dump did not complete");
  if (g_logFile) {
    fclose(g_logFile);
    g_logFile = nullptr;
  }
  FreeLibraryAndExitThread(self, 0);
  return 0;
}

// MSVC does not permit SEH in a function that contains objects requiring C++
// unwinding (std::string, Dumper, and friends).  Keep the exception boundary
// in this trivial wrapper so a malformed runtime pointer only aborts dumping.
__declspec(noinline) bool RunDumpWithSeh(Dumper *d, LogCtx *log) {
  __try {
    return d && d->run();
  } __except (EXCEPTION_EXECUTE_HANDLER) {
    if (log) log->line("[CRASH] 0x%08lX (dump aborted, game untouched)", GetExceptionCode());
    return false;
  }
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
