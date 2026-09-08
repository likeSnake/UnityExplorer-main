#pragma once

#include "../../core/types.hpp"
#include "../../mem/memory_read.hpp"
#include "../metadata/il2cpp_layout_profile.hpp"

#include <cstdint>
#include <string>

namespace er2 {

struct RuntimeClassProbeResult {
  bool valid = false;
  std::uintptr_t klass = 0;
  std::string className;
  std::string namespaze;
  std::string fullName;
  std::uintptr_t parent = 0;
  std::uintptr_t fields = 0;
  std::uintptr_t methods = 0;
  std::uintptr_t staticFields = 0;
  std::uint16_t fieldCount = 0;
  std::uint16_t methodCount = 0;
  std::string failureReason;
};

inline std::string BuildRuntimeClassFullName(const std::string &namespaze,
                                             const std::string &className) {
  if (namespaze.empty()) {
    return className;
  }

  if (className.empty()) {
    return namespaze;
  }

  return namespaze + "." + className;
}

inline bool ReadOptionalRuntimePtr(const IMemoryAccessor &mem,
                                   std::uintptr_t address,
                                   std::uintptr_t &out) {
  out = 0;
  if (!ReadPtr(mem, address, out)) {
    return false;
  }

  return out == 0 || IsCanonicalUserPtr(out);
}

inline bool ReadRuntimeCString(const IMemoryAccessor &mem,
                               std::uintptr_t address, std::string &out,
                               std::size_t maxLen = 256) {
  out.clear();
  if (!IsCanonicalUserPtr(address) || maxLen == 0) {
    return false;
  }

  out.reserve(maxLen < 64 ? maxLen : 64);
  for (std::size_t i = 0; i < maxLen; ++i) {
    char ch = '\0';
    if (!mem.Read(address + i, &ch, sizeof(ch))) {
      return false;
    }

    if (ch == '\0') {
      return true;
    }

    out.push_back(ch);
  }

  return false;
}

inline bool ProbeRuntimeClass(const IMemoryAccessor &mem, std::uintptr_t klass,
                              const Il2CppLayoutProfile &profile,
                              RuntimeClassProbeResult &out) {
  out = RuntimeClassProbeResult{};
  out.klass = klass;

  if (!IsCanonicalUserPtr(klass)) {
    out.failureReason = "klass is not a canonical user pointer";
    return false;
  }

  std::uintptr_t namePtr = 0;
  if (!ReadPtr(mem, klass + profile.klass.name, namePtr) ||
      !IsCanonicalUserPtr(namePtr)) {
    out.failureReason = "class name pointer is unreadable or invalid";
    return false;
  }

  if (!ReadRuntimeCString(mem, namePtr, out.className, 256) ||
      out.className.empty()) {
    out.failureReason = "class name string is unreadable or empty";
    return false;
  }

  std::uintptr_t namespacePtr = 0;
  if (ReadPtr(mem, klass + profile.klass.namespaze, namespacePtr) &&
      IsCanonicalUserPtr(namespacePtr)) {
    (void)ReadRuntimeCString(mem, namespacePtr, out.namespaze, 256);
  }

  if (!ReadOptionalRuntimePtr(mem, klass + profile.klass.parent, out.parent)) {
    out.failureReason = "parent pointer is unreadable or invalid";
    return false;
  }

  if (!ReadOptionalRuntimePtr(mem, klass + profile.klass.fields, out.fields)) {
    out.failureReason = "fields pointer is unreadable or invalid";
    return false;
  }

  if (!ReadOptionalRuntimePtr(mem, klass + profile.klass.methods,
                              out.methods)) {
    out.failureReason = "methods pointer is unreadable or invalid";
    return false;
  }

  if (!ReadOptionalRuntimePtr(mem, klass + profile.klass.staticFields,
                              out.staticFields)) {
    out.failureReason = "static fields pointer is unreadable or invalid";
    return false;
  }

  if (!ReadValue(mem, klass + profile.klass.fieldCount, out.fieldCount)) {
    out.failureReason = "field count is unreadable";
    return false;
  }

  if (!ReadValue(mem, klass + profile.klass.methodCount, out.methodCount)) {
    out.failureReason = "method count is unreadable";
    return false;
  }

  if (out.fieldCount > 4096) {
    out.failureReason = "field count is outside sane bounds";
    return false;
  }

  if (out.methodCount > 16384) {
    out.failureReason = "method count is outside sane bounds";
    return false;
  }

  out.fullName = BuildRuntimeClassFullName(out.namespaze, out.className);
  out.valid = true;
  return true;
}

} // namespace er2
