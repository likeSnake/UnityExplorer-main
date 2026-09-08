#include "er2/unity2/init/module_match.hpp"

#include <cstdio>
#include <vector>

namespace {

bool Expect(bool condition, const char *message) {
  if (condition) {
    return true;
  }

  std::printf("[module_match] FAIL: %s\n", message);
  return false;
}

} // namespace

int main() {
  bool ok = true;

  ok &= Expect(er2::IsUnityPlayerModuleName("UnityPlayer.dll"),
               "UnityPlayer.dll should match");
  ok &= Expect(er2::IsUnityPlayerModuleName("UnityPlayer_LVB_IBT.dll"),
               "UnityPlayer_LVB_IBT.dll should match");
  ok &= Expect(!er2::IsUnityPlayerModuleName("SomeUnityPlayer.dll"),
               "SomeUnityPlayer.dll should not match prefix rule");
  ok &= Expect(!er2::IsUnityPlayerModuleName("UnityPlayer_LVB.exe"),
               "UnityPlayer_LVB.exe should not match dll suffix rule");

  ok &= Expect(er2::IsGameAssemblyModuleName("GameAssembly.dll"),
               "GameAssembly.dll should match");
  ok &= Expect(er2::IsGameAssemblyModuleName("GameAssembly_Super_IBT.dll"),
               "GameAssembly_Super_IBT.dll should match");
  ok &= Expect(!er2::IsGameAssemblyModuleName("Super_GameAssembly.dll"),
               "Super_GameAssembly.dll should not match prefix rule");

  std::vector<er2::UnityModuleCandidate> modules = {
      {"kernel32.dll", 0x1000, 0x100},
      {"UnityPlayer_LVB.dll", 0x2000, 0x200},
      {"UnityPlayer.dll", 0x3000, 0x300},
      {"GameAssembly_Super_IBT.dll", 0x4000, 0x400},
      {"GameAssembly.dll", 0x5000, 0x500},
  };

  const er2::UnityRuntimeModules found = er2::FindUnityRuntimeModules(modules);
  ok &= Expect(found.unityPlayer.base == 0x2000,
               "should pick first UnityPlayer*.dll in module order");
  ok &= Expect(found.gameAssembly.base == 0x4000,
               "should pick first GameAssembly*.dll in module order");

  if (!ok) {
    return 1;
  }

  std::printf("[module_match] PASS\n");
  return 0;
}
