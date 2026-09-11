// ============================================================================
// injector.cpp - Minimal DLL injector for Il2CppRuntimeDumper.dll
// ============================================================================
// Usage:
//   Injector.exe <target.exe> <dll-path>
// Example:
//   Injector.exe NarakaBladepoint.exe C:\...\Il2CppRuntimeDumper.dll
//
// Technique: OpenProcess + VirtualAllocEx + WriteProcessMemory + CreateRemoteThread
// (LoadLibraryA). This mirrors how the reference injector loads Unity-Offset.dll.
// Requires admin (game anti-cheat may block it; for local CTF fixture testing).
// ============================================================================

#include <Windows.h>
#include <TlHelp32.h>
#include <cstdio>
#include <cstring>
#include <cstdlib>
#include <string>

namespace {

DWORD FindProcessId(const char *name) {
  DWORD pid = 0;
  HANDLE snap = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
  if (snap == INVALID_HANDLE_VALUE)
    return 0;
  PROCESSENTRY32W pe;
  pe.dwSize = sizeof(pe);
  // convert name to wide
  wchar_t wname[64] = {};
  MultiByteToWideChar(CP_ACP, 0, name, -1, wname, 64);
  if (Process32FirstW(snap, &pe)) {
    do {
      if (_wcsicmp(pe.szExeFile, wname) == 0) {
        pid = pe.th32ProcessID;
        break;
      }
    } while (Process32NextW(snap, &pe));
  }
  CloseHandle(snap);
  return pid;
}

bool Inject(DWORD pid, const std::string &dllPath) {
  HANDLE hProc = OpenProcess(
      PROCESS_QUERY_INFORMATION | PROCESS_CREATE_THREAD | PROCESS_VM_OPERATION |
          PROCESS_VM_WRITE | PROCESS_VM_READ,
      FALSE, pid);
  if (!hProc) {
    printf("[-] OpenProcess failed: %lu\n", GetLastError());
    return false;
  }

  size_t len = dllPath.size() + 1;
  void *remote = VirtualAllocEx(hProc, nullptr, len, MEM_COMMIT | MEM_RESERVE,
                                PAGE_READWRITE);
  if (!remote) {
    printf("[-] VirtualAllocEx failed: %lu\n", GetLastError());
    CloseHandle(hProc);
    return false;
  }

  if (!WriteProcessMemory(hProc, remote, dllPath.c_str(), len, nullptr)) {
    printf("[-] WriteProcessMemory failed: %lu\n", GetLastError());
    VirtualFreeEx(hProc, remote, 0, MEM_RELEASE);
    CloseHandle(hProc);
    return false;
  }

  HMODULE k32 = GetModuleHandleA("kernel32.dll");
  FARPROC loadLib = GetProcAddress(k32, "LoadLibraryA");
  HANDLE hThread = CreateRemoteThread(hProc, nullptr, 0,
                                      reinterpret_cast<LPTHREAD_START_ROUTINE>(
                                          loadLib),
                                      remote, 0, nullptr);
  if (!hThread) {
    printf("[-] CreateRemoteThread failed: %lu\n", GetLastError());
    VirtualFreeEx(hProc, remote, 0, MEM_RELEASE);
    CloseHandle(hProc);
    return false;
  }

  printf("[+] Injected. Waiting for LoadLibraryA...\n");
  WaitForSingleObject(hThread, 10000);
  DWORD exitCode = 0;
  GetExitCodeThread(hThread, &exitCode);
  printf("[+] LoadLibraryA returned %p\n", (void *)exitCode);

  CloseHandle(hThread);
  VirtualFreeEx(hProc, remote, 0, MEM_RELEASE);
  CloseHandle(hProc);
  return exitCode != 0;
}

} // namespace

int main(int argc, char **argv) {
  if (argc < 3) {
    printf("Usage: Injector.exe <process-name|pid> <dll-path>\n");
    printf("Example: Injector.exe NarakaBladepoint.exe C:\\dumps\\Il2CppRuntimeDumper.dll\n");
    printf("         Injector.exe 18064 C:\\dumps\\AddDelayProbe.dll\n");
    return 1;
  }

  char *end = nullptr;
  unsigned long parsed = std::strtoul(argv[1], &end, 10);
  const DWORD pid = (end && *end == '\0' && parsed > 0 && parsed <= 0xFFFFFFFFul)
                        ? static_cast<DWORD>(parsed)
                        : FindProcessId(argv[1]);
  if (!pid) {
    printf("[-] Process '%s' not found\n", argv[1]);
    return 1;
  }
  printf("[+] Target %s pid=%lu\n", argv[1], pid);

  const bool ok = Inject(pid, argv[2]);
  printf("[%s] injection %s\n", ok ? "+" : "-", ok ? "OK" : "FAILED");
  return ok ? 0 : 1;
}
