#pragma once

#include <cstdint>
#include <filesystem>
#include <string>
#include <unordered_map>
#include <vector>

#include "../../os/win/win_memory_accessor.hpp"
#include "../../os/win/win_module.hpp"
#include "../../os/win/win_process.hpp"

#include "../metadata.hpp"
#include "../metadata/metadata_images.hpp"

#include "path.hpp"
#include "sdk_common.hpp"
#include "sdk_dump_cs.hpp"
#include "sdk_generic_json.hpp"
#include "sdk_metadata_helpers.hpp"
#include "sdk_registration.hpp"
#include "sdk_strings.hpp"

namespace er2 {

struct DumpSdk6Paths {
  std::string outDir;
  std::string dumpCsPath;
  std::string genericJsonPath;
};

inline bool DumpSdk6DumpWithAccessor(const IMemoryAccessor &mem,
                                     std::uint32_t pid,
                                     std::uintptr_t gameAssemblyBase,
                                     std::uint32_t gameAssemblySize,
                                     const std::string &gameAssemblyName,
                                     DumpSdk6Paths &outPaths) {
  outPaths = DumpSdk6Paths{};

  try {
    if (gameAssemblyBase == 0) {
      return false;
    }

    std::wstring moduleNameW;
    for (char c : gameAssemblyName)
      moduleNameW += (wchar_t)c;
    if (moduleNameW.empty())
      moduleNameW = L"GameAssembly.dll";

    MetadataHint hint;
    if (!BuildMetadataHintTScore(mem, gameAssemblyBase, pid, L"",
                                 moduleNameW.c_str(), hint)) {
      return false;
    }

    // Override module size if provided
    if (gameAssemblySize != 0)
      hint.moduleSize = gameAssemblySize;

    std::vector<std::uint8_t> metaBytes;
    if (!ExportMetadataByScore(mem, gameAssemblyBase, 0x200000u, 8192, 15.0,
                               false, 0, 0x200000u, metaBytes)) {
      return false;
    }

    MetadataHeaderFields header;
    if (!ReadMetadataHeaderFieldsFromBytes(metaBytes, header)) {
      return false;
    }

    // [Fix] Sanity Checks for Metadata Header to prevent bad_alloc
    // Header Version Check (Basic) - Unity 2017+ usually uses >= 19
    if (header.version < 19 || header.version > 120) {
      char buf[128];
      std::snprintf(buf, sizeof(buf), "[ERROR] Invalid Metadata Version: %d",
                    header.version);
      er2::Log(buf);
      return false;
    }

    // TypeDefinition Sanity Check
    // Calculate count manually to check before subsequent allocations
    if (header.typeDefinitionsSize % sizeof(DumpSdk6TypeDefRaw) != 0) {
      er2::Log("[ERROR] TypeDefinitionSize not aligned");
      return false;
    }
    std::uint32_t typeCount =
        header.typeDefinitionsSize / sizeof(DumpSdk6TypeDefRaw);
    if (typeCount > 10000000u) { // Cap at 10 million types
      char buf[128];
      std::snprintf(buf, sizeof(buf),
                    "[ERROR] TypeDefinitionCount too large: %u", typeCount);
      er2::Log(buf);
      return false;
    }

    // Bounds check for key offsets
    if (header.typeDefinitionsOffset + header.typeDefinitionsSize >
        metaBytes.size()) {
      er2::Log("[ERROR] TypeDefinitions out of bounds");
      return false;
    }

    std::vector<MetadataImageInfo> images;
    (void)ReadImagesFromBytes(metaBytes, header, images);

    std::vector<std::string> typeToImage;
    (void)BuildTypeDefIndexToImageNameFromBytes(metaBytes, typeToImage);

    std::unordered_map<std::uint32_t, std::string> typeMap;
    std::vector<std::string> typeFullName;
    (void)BuildTypeFullNameAndByvalMapFromBytes(metaBytes, header, typeFullName,
                                                typeMap);

    std::vector<DumpSdk6GenericParamInfo> genericParams =
        BuildGenericParamInfoFromBytes(metaBytes, header);

    std::uintptr_t typesPtr = 0;
    std::uint32_t typesCount = 0;
    std::uintptr_t fieldOffsetsPtr = 0;
    (void)DumpSdk6GetMetadataRegistrationTypes(mem, hint.metadataRegistration,
                                               header.version, typesPtr,
                                               typesCount, fieldOffsetsPtr);

    const std::string outDir = JoinPathA(GetExeDirA(), "DumpSDK");
    std::error_code ec;
    std::filesystem::create_directories(std::filesystem::path(outDir), ec);

    const std::string dumpCsPath = JoinPathA(outDir, "dump.cs");
    const std::string genericJsonPath = JoinPathA(outDir, "generic.json");

    if (!DumpSdk6WriteGenericJsonFile(genericJsonPath, header, metaBytes,
                                      genericParams, typeFullName)) {
      return false;
    }

    if (!DumpSdk6WriteDumpCsFile(dumpCsPath, mem, hint, header, metaBytes,
                                 images, typeToImage, typeFullName, typeMap,
                                 genericParams, typesPtr, typesCount,
                                 fieldOffsetsPtr)) {
      return false;
    }

    outPaths.outDir = outDir;
    outPaths.dumpCsPath = dumpCsPath;
    outPaths.genericJsonPath = genericJsonPath;

    return true;

  } catch (const std::bad_alloc &e) {
    char buf[256];
    std::snprintf(buf, sizeof(buf),
                  "[CRITICAL] Exception in SDK Export: bad_alloc (%s). "
                  "Metadata Header likely corrupt.",
                  e.what());
    er2::Log(buf);
    return false;
  } catch (const std::exception &e) {
    char buf[256];
    std::snprintf(buf, sizeof(buf), "[CRITICAL] Exception in SDK Export: %s",
                  e.what());
    er2::Log(buf);
    return false;
  }
}

inline bool DumpSdk6DumpByPid(std::uint32_t pid, DumpSdk6Paths &outPaths) {
  HANDLE hProc = OpenProcessForRead(pid);
  if (!hProc) {
    return false;
  }

  WinApiMemoryAccessor mem(hProc);

  ModuleInfo gameAssembly;
  if (!GetRemoteModuleInfo(pid, L"GameAssembly.dll", gameAssembly) ||
      !gameAssembly.base || gameAssembly.size == 0) {
    CloseHandle(hProc);
    return false;
  }

  bool result =
      DumpSdk6DumpWithAccessor(mem, pid, gameAssembly.base, gameAssembly.size,
                               "GameAssembly.dll", outPaths);

  CloseHandle(hProc);
  return result;
}

} // namespace er2
