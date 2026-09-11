// ============================================================================
// SyntheticUnityHost.cpp
// 无游戏环境下的端到端验证宿主：加载同目录的合成 GameAssembly_Super.dll
// （不含任何 il2cpp 导出），然后驻留等待注入 SuperDumper.dll。
// ============================================================================
#include <Windows.h>
#include <cstdio>

int main(int argc, char **argv) {
  int seconds = (argc > 1) ? atoi(argv[1]) : 180;
  HMODULE h = LoadLibraryA("GameAssembly_Super.dll");
  if (!h) {
    printf("[-] LoadLibraryA(GameAssembly_Super.dll) failed: %lu\n", GetLastError());
    return 1;
  }
  printf("[+] GameAssembly_Super.dll loaded at %p\n", (void *)h);
  printf("[+] pid=%lu  (inject with: Injector.exe SyntheticUnityHost.exe <path>\\SuperDumper.dll)\n",
         GetCurrentProcessId());
  printf("[+] staying alive for %d s...\n", seconds);
  fflush(stdout);
  for (int i = 0; i < seconds * 10; ++i) Sleep(100);
  printf("[+] host exiting\n");
  return 0;
}
