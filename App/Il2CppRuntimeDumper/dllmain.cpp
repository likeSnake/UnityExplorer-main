// ============================================================================
// dllmain.cpp - Injected IL2CPP runtime dumper entry point.
// ============================================================================
// This DLL is injected into the game process (same technique as the reference
// Unity-Offset.dll). On DLL_PROCESS_ATTACH it spawns a worker thread that:
//   1. Resolves the il2cpp_* API from GameAssembly.dll (in-process).
//   2. Walks assemblies -> images -> classes -> fields/methods/properties.
//   3. Writes a complete dump.cs (with field offsets, method RVA/VA, and
//      properties) to a configurable output directory.
//
// No exports are needed: DllMain performs the work, mirroring how the
// reference injector behaves (export directory is empty there too).
// ============================================================================

#include <Windows.h>
#include <shlwapi.h>

#include <cstdio>
#include <cstdlib>
#include <string>

#include "il2cpp_api.hpp"
#include "runtime_dumper.cpp"

// ---------------------------------------------------------------------------
// Output directory / configuration
// ---------------------------------------------------------------------------
// The reference tool writes to C:\YJDumped\dump.cs. We keep the same default
// so the user can compare directly, but allow overrides via a file next to
// the DLL: Il2CppRuntimeDumper.cfg  (first line = output directory).
// ---------------------------------------------------------------------------

namespace {

std::string g_outDir = "C:\\YJDumped";

void LoadConfigFromDllDir() {
  char dllPath[MAX_PATH] = {};
  GetModuleFileNameA(GetModuleHandleA("Il2CppRuntimeDumper.dll"), dllPath,
                     sizeof(dllPath));
  char cfgPath[MAX_PATH] = {};
  PathRemoveFileSpecA(dllPath);
  PathCombineA(cfgPath, dllPath, "Il2CppRuntimeDumper.cfg");
  FILE *f = fopen(cfgPath, "r");
  if (!f)
    return;
  char line[1024] = {};
  if (fgets(line, sizeof(line), f)) {
    std::string s(line);
    // trim whitespace/newlines
    while (!s.empty() && (s.back() == '\r' || s.back() == '\n' ||
                          s.back() == ' ' || s.back() == '\t'))
      s.pop_back();
    if (!s.empty())
      g_outDir = s;
  }
  fclose(f);
}

// The actual dump work, called from the SEH wrapper. Keeping all C++ objects
// (std::string etc) in this function lets the __try wrapper stay C-clean.
DWORD DumpWorkerBody() {
  il2::Il2CppApi api;

  constexpr int kMaxAttempts = 20;
  bool resolved = false;
  for (int attempt = 0; attempt < kMaxAttempts; ++attempt) {
    if (il2::ResolveIl2CppApi(api)) {
      resolved = true;
      break;
    }
    Sleep(2000);
  }
  if (!resolved) {
    FILE *f = fopen((g_outDir + "\\log.txt").c_str(), "a");
    if (f) {
      fprintf(f, "[FAIL] il2cpp API resolution failed after %d attempts\n",
              kMaxAttempts);
      fclose(f);
    }
    return 1;
  }

  CreateDirectoryA(g_outDir.c_str(), nullptr);

  il2::DumpOptions options;
  options.outPath = g_outDir + "\\dump.cs";
  options.includeProperties = true;
  options.includeMethods = true;
  options.includeFields = true;
  options.includePrivateMembers = false;
  options.verbose = true;

  std::string error;
  const bool ok = il2::RunDump(api, options, error);

  FILE *f = fopen((g_outDir + "\\log.txt").c_str(), "a");
  if (f) {
    fprintf(f, "[%s] %s\n", ok ? "OK" : "FAIL", error.c_str());
    fclose(f);
  }
  return ok ? 0 : 1;
}

// VEH crash containment for the dump worker. If the dump logic hits an
// unexpected structure and raises an access violation, we log it and terminate
// only our worker thread -- never the game.
static volatile LONG g_crashLogged = 0;
static DWORD g_workerThreadId = 0;
static char g_crashNote[128] = {0};

static LONG WINAPI CrashFilter(PEXCEPTION_POINTERS ep) {
  if (GetCurrentThreadId() != g_workerThreadId)
    return EXCEPTION_CONTINUE_SEARCH;
  if (InterlockedExchange(&g_crashLogged, 1))
    return EXCEPTION_CONTINUE_SEARCH;
  sprintf_s(g_crashNote, "VEH crash 0x%08X @ 0x%p", ep->ExceptionRecord->ExceptionCode,
            ep->ExceptionRecord->ExceptionAddress);
  FILE *f = fopen((g_outDir + "\\log.txt").c_str(), "a");
  if (f) {
    fprintf(f, "[CRASH] %s\n", g_crashNote);
    fclose(f);
  }
  // Do not continue execution at the faulting instruction: terminate only the
  // worker thread so the game keeps running.
  TerminateThread(GetCurrentThread(), 2);
  return EXCEPTION_CONTINUE_EXECUTION; // unreachable
}

DWORD WINAPI DumpWorker(LPVOID) {
  g_workerThreadId = GetCurrentThreadId();
  AddVectoredExceptionHandler(1, CrashFilter);
  const DWORD rc = DumpWorkerBody();
  RemoveVectoredExceptionHandler(CrashFilter);
  return rc;
}

} // namespace

BOOL APIENTRY DllMain(HMODULE hModule, DWORD reason, LPVOID) {
  if (reason == DLL_PROCESS_ATTACH) {
    DisableThreadLibraryCalls(hModule);
    LoadConfigFromDllDir();
    HANDLE h = CreateThread(nullptr, 0, DumpWorker, nullptr, 0, nullptr);
    if (h)
      CloseHandle(h);
  }
  return TRUE;
}
