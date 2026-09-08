#pragma once

#include <cstdint>
#include <string>
#include <vector>

#include "../../../core/types.hpp"
#include "../../../mem/memory_read.hpp"
#include "../../inspect/klass_header_probe.hpp"
#include "../../core/offsets.hpp"

namespace er2 {

inline bool ReadIl2CppClassName(const IMemoryAccessor &mem,
                                std::uintptr_t klass, const Offsets &off,
                                std::string &outNamespace,
                                std::string &outClassName) {
  outNamespace.clear();
  outClassName.clear();

  if (!IsCanonicalUserPtr(klass)) {
    return false;
  }

  std::uintptr_t namePtr = 0;
  if (!ReadPtr(mem, klass + off.il2cppclass_name_ptr, namePtr)) {
    return false;
  }

  if (!IsCanonicalUserPtr(namePtr)) {
    return false;
  }

  if (!ReadCString(mem, namePtr, outClassName)) {
    return false;
  }

  std::uintptr_t nsPtr = 0;
  if (ReadPtr(mem, klass + off.il2cppclass_namespace_ptr, nsPtr) &&
      IsCanonicalUserPtr(nsPtr)) {
    ReadCString(mem, nsPtr, outNamespace);
  }

  return true;
}

inline bool ReadIl2CppClassNameAdaptive(const IMemoryAccessor &mem,
                                        std::uintptr_t klass,
                                        const Offsets &off,
                                        std::string &outNamespace,
                                        std::string &outClassName) {
  outNamespace.clear();
  outClassName.clear();

  std::string ns;
  std::string cn;
  if (ReadIl2CppClassName(mem, klass, off, ns, cn) && !cn.empty()) {
    outNamespace = std::move(ns);
    outClassName = std::move(cn);
    return true;
  }

  KlassHeaderProbeResult headerProbe;
  if (!ProbeKlassHeaderStrings(mem, klass, headerProbe)) {
    return false;
  }

  if (headerProbe.bestName.empty()) {
    return false;
  }

  outNamespace = headerProbe.bestNamespace;
  outClassName = headerProbe.bestName;
  return true;
}

inline bool IsClassOrParent(const IMemoryAccessor &mem, std::uintptr_t klass,
                            const Offsets &off, const char *targetNs,
                            const char *targetName, int maxDepth = 10) {
  if (!IsCanonicalUserPtr(klass) || !targetName) {
    return false;
  }

  std::uintptr_t current = klass;
  for (int i = 0; i < maxDepth && IsCanonicalUserPtr(current); ++i) {
    std::string ns;
    std::string cn;
    if (!ReadIl2CppClassNameAdaptive(mem, current, off, ns, cn)) {
      break;
    }

    bool nsMatch =
        (targetNs == nullptr || targetNs[0] == '\0' || ns == targetNs);
    bool cnMatch = (cn == targetName);

    if (nsMatch && cnMatch) {
      return true;
    }

    std::uintptr_t parent = 0;
    if (!ReadPtr(mem, current + off.il2cppclass_parent, parent)) {
      break;
    }

    current = parent;
  }

  return false;
}

// ========== Il2CppClass Field Analysis ==========

// IL2CPP field attribute constant
constexpr std::uint16_t FIELD_ATTRIBUTE_STATIC = 0x0010;

/// <summary>
/// Extracted field information from Il2CppClass
/// </summary>
struct Il2CppFieldExtract {
  std::string name;           // 字段名称
  std::uint32_t offset = 0;   // 字段偏移量
  bool isStatic = false;      // 是否为静态字段
  std::uintptr_t typePtr = 0; // Il2CppType* 指针 (可用于进一步分析类型)
};

/// <summary>
/// Enumerate all fields of an Il2CppClass
/// </summary>
/// <param name="mem">Memory accessor</param>
/// <param name="klass">Il2CppClass address</param>
/// <param name="off">Offsets configuration</param>
/// <param name="outFields">Output vector of field information</param>
/// <returns>true if successful</returns>
inline bool EnumerateClassFields(const IMemoryAccessor &mem,
                                 std::uintptr_t klass, const Offsets &off,
                                 std::vector<Il2CppFieldExtract> &outFields) {
  outFields.clear();

  if (!IsCanonicalUserPtr(klass)) {
    return false;
  }

  // Read fields pointer
  std::uintptr_t fieldsPtr = 0;
  if (!ReadPtr(mem, klass + off.il2cppclass_fields, fieldsPtr)) {
    return false;
  }

  if (!IsCanonicalUserPtr(fieldsPtr)) {
    return false;
  }

  // Read field count
  std::uint16_t fieldCount = 0;
  if (!mem.Read(klass + off.il2cppclass_field_count, &fieldCount,
                sizeof(fieldCount))) {
    return false;
  }

  // Sanity check
  if (fieldCount == 0 || fieldCount > 1000) {
    return fieldCount == 0; // 0 fields is valid
  }

  outFields.reserve(fieldCount);

  for (std::uint16_t i = 0; i < fieldCount; ++i) {
    std::uintptr_t fieldInfoAddr = fieldsPtr + (static_cast<std::uintptr_t>(i) *
                                                off.il2cpp_field_info_size);

    Il2CppFieldExtract field;

    // Read field name
    std::uintptr_t namePtr = 0;
    if (ReadPtr(mem, fieldInfoAddr + off.il2cpp_field_info_name, namePtr) &&
        IsCanonicalUserPtr(namePtr)) {
      ReadCString(mem, namePtr, field.name);
    }

    // Read field offset
    std::int32_t fieldOffset = 0;
    if (mem.Read(fieldInfoAddr + off.il2cpp_field_info_offset, &fieldOffset,
                 sizeof(fieldOffset))) {
      field.offset = static_cast<std::uint32_t>(fieldOffset);
    }

    // Read type pointer
    if (ReadPtr(mem, fieldInfoAddr + off.il2cpp_field_info_type,
                field.typePtr) &&
        IsCanonicalUserPtr(field.typePtr)) {
      // Read attrs to determine if static
      std::uint16_t attrs = 0;
      if (mem.Read(field.typePtr + off.il2cpp_type_attrs, &attrs,
                   sizeof(attrs))) {
        field.isStatic = (attrs & FIELD_ATTRIBUTE_STATIC) != 0;
      }
    }

    outFields.push_back(field);
  }

  return true;
}

/// <summary>
/// Get static fields data pointer from Il2CppClass
/// </summary>
inline bool GetStaticFieldsPtr(const IMemoryAccessor &mem, std::uintptr_t klass,
                               const Offsets &off,
                               std::uintptr_t &outStaticFieldsPtr) {
  outStaticFieldsPtr = 0;

  if (!IsCanonicalUserPtr(klass)) {
    return false;
  }

  if (!ReadPtr(mem, klass + off.il2cppclass_static_fields,
               outStaticFieldsPtr)) {
    return false;
  }

  return IsCanonicalUserPtr(outStaticFieldsPtr);
}

/// <summary>
/// Read a static field value from Il2CppClass
/// </summary>
/// <typeparam name="T">Value type to read</typeparam>
/// <param name="mem">Memory accessor</param>
/// <param name="klass">Il2CppClass address</param>
/// <param name="off">Offsets configuration</param>
/// <param name="fieldOffset">Field offset within static data region</param>
/// <param name="outValue">Output value</param>
/// <returns>true if successful</returns>
template <typename T>
inline bool ReadStaticFieldValue(const IMemoryAccessor &mem,
                                 std::uintptr_t klass, const Offsets &off,
                                 std::uint32_t fieldOffset, T &outValue) {
  std::uintptr_t staticFieldsPtr = 0;
  if (!GetStaticFieldsPtr(mem, klass, off, staticFieldsPtr)) {
    return false;
  }

  return mem.Read(staticFieldsPtr + fieldOffset, &outValue, sizeof(T));
}

/// <summary>
/// Read a static field pointer from Il2CppClass
/// </summary>
inline bool ReadStaticFieldPtr(const IMemoryAccessor &mem, std::uintptr_t klass,
                               const Offsets &off, std::uint32_t fieldOffset,
                               std::uintptr_t &outPtr) {
  return ReadStaticFieldValue(mem, klass, off, fieldOffset, outPtr);
}

} // namespace er2
