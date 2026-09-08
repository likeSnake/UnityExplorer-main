#pragma once

#include <cstdint>
#include <string>

namespace er2 {

struct Il2CppClassLayout {
  std::uint32_t name = 0;
  std::uint32_t namespaze = 0;
  std::uint32_t parent = 0;
  std::uint32_t byvalArg = 0;
  std::uint32_t fields = 0;
  std::uint32_t methods = 0;
  std::uint32_t staticFields = 0;
  std::uint32_t methodCount = 0;
  std::uint32_t fieldCount = 0;
};

struct Il2CppFieldInfoLayout {
  std::uint32_t name = 0;
  std::uint32_t type = 0;
  std::uint32_t parent = 0;
  std::uint32_t offset = 0;
  std::uint32_t token = 0;
  std::uint32_t size = 0;
};

struct Il2CppMethodInfoLayout {
  std::uint32_t methodPointer = 0;
  std::uint32_t invokerMethod = 0;
  std::uint32_t name = 0;
  std::uint32_t klass = 0;
  std::uint32_t returnType = 0;
  std::uint32_t parameters = 0;
  std::uint32_t token = 0;
  std::uint32_t flags = 0;
  std::uint32_t iflags = 0;
  std::uint32_t slot = 0;
  std::uint32_t parameterCount = 0;
};

struct Il2CppMetadataRegistrationLayout {
  std::uint32_t genericClassesCount = 0;
  std::uint32_t genericClasses = 0;
  std::uint32_t genericInstsCount = 0;
  std::uint32_t genericInsts = 0;
  std::uint32_t genericMethodTableCount = 0;
  std::uint32_t genericMethodTable = 0;
  std::uint32_t typesCount = 0;
  std::uint32_t types = 0;
  std::uint32_t methodSpecsCount = 0;
  std::uint32_t methodSpecs = 0;
  std::uint32_t fieldOffsetsCount = 0;
  std::uint32_t fieldOffsets = 0;
  std::uint32_t typeDefinitionsSizesCount = 0;
  std::uint32_t typeDefinitionsSizes = 0;
};

struct Il2CppLayoutProfile {
  const char *id = "";
  const char *unityVersionRange = "";
  const char *source = "";
  std::uint32_t metadataVersionMajor = 0;
  std::uint32_t metadataVersionMinor = 0;
  Il2CppClassLayout klass;
  Il2CppFieldInfoLayout fieldInfo;
  Il2CppMethodInfoLayout methodInfo;
  Il2CppMetadataRegistrationLayout metadataRegistration;
};

inline constexpr Il2CppLayoutProfile kIl2CppUnity2019_4_Lts24_5_Profile{
    "unity-2019.4-lts-24.5",
    "2019.4.21-2019.4.x candidate",
    "Il2CppInspectorRedux UnityHeaders/24.5-2019.4.21-2019.4.24.h",
    24,
    5,
    {/* name */ 0x10,
     /* namespaze */ 0x18,
     /* parent */ 0x58,
     /* byvalArg */ 0x20,
     /* fields */ 0x80,
     /* methods */ 0x98,
     /* staticFields */ 0xB8,
     /* methodCount */ 0x118,
     /* fieldCount */ 0x11C},
    {/* name */ 0x00,
     /* type */ 0x08,
     /* parent */ 0x10,
     /* offset */ 0x18,
     /* token */ 0x1C,
     /* size */ 0x20},
    {/* methodPointer */ 0x00,
     /* invokerMethod */ 0x08,
     /* name */ 0x10,
     /* klass */ 0x18,
     /* returnType */ 0x20,
     /* parameters */ 0x28,
     /* token */ 0x48,
     /* flags */ 0x4C,
     /* iflags */ 0x4E,
     /* slot */ 0x50,
     /* parameterCount */ 0x52},
    {/* genericClassesCount */ 0x00,
     /* genericClasses */ 0x08,
     /* genericInstsCount */ 0x10,
     /* genericInsts */ 0x18,
     /* genericMethodTableCount */ 0x20,
     /* genericMethodTable */ 0x28,
     /* typesCount */ 0x30,
     /* types */ 0x38,
     /* methodSpecsCount */ 0x40,
     /* methodSpecs */ 0x48,
     /* fieldOffsetsCount */ 0x50,
     /* fieldOffsets */ 0x58,
     /* typeDefinitionsSizesCount */ 0x60,
     /* typeDefinitionsSizes */ 0x68}};

inline bool IsUnity2019_4VersionString(const std::string &unityVersion) {
  return unityVersion.rfind("2019.4.", 0) == 0;
}

inline const Il2CppLayoutProfile *
FindIl2CppLayoutProfileForUnity2019_4(const std::string &unityVersion) {
  if (!IsUnity2019_4VersionString(unityVersion)) {
    return nullptr;
  }

  return &kIl2CppUnity2019_4_Lts24_5_Profile;
}

inline const Il2CppLayoutProfile *
FindIl2CppLayoutProfileForMetadataVersion(std::uint32_t metadataVersion) {
  if (metadataVersion == 24) {
    return &kIl2CppUnity2019_4_Lts24_5_Profile;
  }

  return nullptr;
}

} // namespace er2
