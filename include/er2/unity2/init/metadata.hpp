#pragma once

#include <cstdint>
#include <filesystem>
#include <optional>
#include <vector>

#include "context.hpp"

#include "../metadata/codegen_modules.hpp"
#include "../metadata/export.hpp"
#include "../metadata/header_parser.hpp"
#include "../metadata/hint_export.hpp"
#include "../metadata/method_resolver.hpp"
#include "../metadata/pe.hpp"
#include "../metadata/registration_scanner.hpp"
#include "../metadata/scanner_pointer.hpp"

namespace er2 {

inline bool TryGetGameAssemblyModuleInfo(ModuleInfo &out) {
  out = ModuleInfo{};
  if (!IsInited()) {
    return false;
  }

  if (g_ctx.gameAssembly.base) {
    out = g_ctx.gameAssembly;
    return true;
  }

  // Try multiple names
  if (er2::GetContextModuleInfo(g_ctx.pid, L"GameAssembly.dll", out) &&
      out.base) {
    g_ctx.gameAssembly = out;
    er2::Log("[INFO] Found GameAssembly.dll at 0x" +
             detail_metadata_hint::HexU64NoPad(out.base));
    return true;
  }
  if (er2::GetContextModuleInfo(g_ctx.pid, L"GameAssembly_Super.dll", out) &&
      out.base) {
    g_ctx.gameAssembly = out;
    er2::Log("[INFO] Found GameAssembly_Super.dll at 0x" +
             detail_metadata_hint::HexU64NoPad(out.base));
    return true;
  }

  er2::Log("[WARN] GameAssembly module not found (Checked GameAssembly.dll, "
           "GameAssembly_Super.dll)");
  return false;
}

inline std::optional<ModuleInfo> TryGetGameAssemblyModuleInfo() {
  ModuleInfo out;
  if (!TryGetGameAssemblyModuleInfo(out)) {
    return std::nullopt;
  }
  return out;
}

inline bool ExportGameAssemblyMetadataByScore(std::vector<std::uint8_t> &out) {
  out.clear();

  if (!IsInited()) {
    return false;
  }

  ModuleInfo ga;
  if (!TryGetGameAssemblyModuleInfo(ga) || !ga.base) {
    er2::Log("[ERROR] ExportMetadata: Module GameAssembly failed to locate.");
    return false;
  }

  er2::Log("[INFO] ExportMetadata: Starting scan on module base: 0x" +
           detail_metadata_hint::HexU64NoPad(ga.base));
  return er2::ExportMetadataByScore(Mem(), ga.base, 0x200000u, 8192, 15.0,
                                    false, 0, 0x200000u, out);
}

inline std::optional<std::vector<std::uint8_t>>
ExportGameAssemblyMetadataByScore() {
  std::vector<std::uint8_t> out;
  if (!ExportGameAssemblyMetadataByScore(out)) {
    return std::nullopt;
  }
  return out;
}

inline bool ExportGameAssemblyMetadataHintJsonTScoreToSidecar(
    const std::filesystem::path &outDatPath) {
  if (!IsInited()) {
    return false;
  }

  ModuleInfo ga = g_ctx.unityPlayer; // Fallback
  std::wstring gaName = L"GameAssembly.dll";

  // Try to find the actual GameAssembly if it's IL2CPP
  ModuleInfo actualGa;
  if (g_ctx.gameAssembly.base) {
    ga = g_ctx.gameAssembly;
    gaName = L"GameAssembly.dll";
  } else if (er2::GetContextModuleInfo(g_ctx.pid, L"GameAssembly.dll",
                                       actualGa) &&
      actualGa.base) {
    ga = actualGa;
    gaName = L"GameAssembly.dll";
  } else if (er2::GetContextModuleInfo(g_ctx.pid, L"GameAssembly_Super.dll",
                                       actualGa) &&
             actualGa.base) {
    ga = actualGa;
    gaName = L"GameAssembly_Super.dll";
  }

  return er2::ExportMetadataHintJsonTScoreToSidecar(
      Mem(), outDatPath, ga.base, g_ctx.pid, L"", gaName.c_str());
}

} // namespace er2
