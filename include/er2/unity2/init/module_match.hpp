#pragma once

#include <cctype>
#include <cstdint>
#include <string>
#include <vector>

namespace er2 {

struct UnityModuleCandidate {
  std::string name;
  std::uintptr_t base = 0;
  std::uint32_t size = 0;
};

struct UnityRuntimeModules {
  UnityModuleCandidate unityPlayer;
  UnityModuleCandidate gameAssembly;
};

namespace detail_module_match {

inline std::string ToLowerAscii(std::string value) {
  for (char &ch : value) {
    ch =
        static_cast<char>(std::tolower(static_cast<unsigned char>(ch)));
  }
  return value;
}

inline bool StartsWith(std::string value, std::string prefix) {
  value = ToLowerAscii(std::move(value));
  prefix = ToLowerAscii(std::move(prefix));
  return value.rfind(prefix, 0) == 0;
}

inline bool EndsWith(std::string value, std::string suffix) {
  value = ToLowerAscii(std::move(value));
  suffix = ToLowerAscii(std::move(suffix));
  if (value.size() < suffix.size()) {
    return false;
  }

  return value.compare(value.size() - suffix.size(), suffix.size(), suffix) ==
         0;
}

} // namespace detail_module_match

inline bool IsUnityPlayerModuleName(const std::string &name) {
  return detail_module_match::StartsWith(name, "UnityPlayer") &&
         detail_module_match::EndsWith(name, ".dll");
}

inline bool IsGameAssemblyModuleName(const std::string &name) {
  return detail_module_match::StartsWith(name, "GameAssembly") &&
         detail_module_match::EndsWith(name, ".dll");
}

inline UnityRuntimeModules
FindUnityRuntimeModules(const std::vector<UnityModuleCandidate> &modules) {
  UnityRuntimeModules found;

  for (const UnityModuleCandidate &module : modules) {
    if (found.unityPlayer.base == 0 && IsUnityPlayerModuleName(module.name)) {
      found.unityPlayer = module;
    }

    if (found.gameAssembly.base == 0 &&
        IsGameAssemblyModuleName(module.name)) {
      found.gameAssembly = module;
    }

    if (found.unityPlayer.base != 0 && found.gameAssembly.base != 0) {
      break;
    }
  }

  return found;
}

} // namespace er2
