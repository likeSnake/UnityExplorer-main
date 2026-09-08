#pragma once

#include "../../core/types.hpp"
#include "../../mem/memory_read.hpp"
#include "../inspect/klass_header_probe.hpp"
#include "../inspect/runtime_class_probe.hpp"
#include "../metadata/il2cpp_layout_profile.hpp"

#include <cstdint>
#include <sstream>
#include <string>
#include <vector>

namespace er2 {

struct RuntimeIl2CppTypeDescription {
  std::string name;
  std::uintptr_t typePtr = 0;
  std::uintptr_t data = 0;
  std::uint32_t metaBits = 0;
  std::uint32_t typeEnum = 0;
};

inline bool DescribeRuntimeIl2CppTypeInternal(
    const IMemoryAccessor &mem, const Il2CppLayoutProfile &profile,
    std::uintptr_t typePtr, RuntimeIl2CppTypeDescription &out, int depth);

inline std::string RuntimePrimitiveIl2CppTypeName(std::uint32_t typeEnum) {
  switch (typeEnum) {
  case 0x01:
    return "System.Void";
  case 0x02:
    return "System.Boolean";
  case 0x03:
    return "System.Char";
  case 0x04:
    return "System.SByte";
  case 0x05:
    return "System.Byte";
  case 0x06:
    return "System.Int16";
  case 0x07:
    return "System.UInt16";
  case 0x08:
    return "System.Int32";
  case 0x09:
    return "System.UInt32";
  case 0x0A:
    return "System.Int64";
  case 0x0B:
    return "System.UInt64";
  case 0x0C:
    return "System.Single";
  case 0x0D:
    return "System.Double";
  case 0x0E:
    return "System.String";
  case 0x18:
    return "System.IntPtr";
  case 0x19:
    return "System.UIntPtr";
  case 0x1C:
    return "System.Object";
  default:
    return "";
  }
}

inline std::string RuntimeIl2CppTypeKindName(std::uint32_t typeEnum) {
  switch (typeEnum) {
  case 0x01:
    return "VOID";
  case 0x02:
    return "BOOLEAN";
  case 0x03:
    return "CHAR";
  case 0x04:
    return "I1";
  case 0x05:
    return "U1";
  case 0x06:
    return "I2";
  case 0x07:
    return "U2";
  case 0x08:
    return "I4";
  case 0x09:
    return "U4";
  case 0x0A:
    return "I8";
  case 0x0B:
    return "U8";
  case 0x0C:
    return "R4";
  case 0x0D:
    return "R8";
  case 0x0E:
    return "STRING";
  case 0x0F:
    return "PTR";
  case 0x10:
    return "BYREF";
  case 0x11:
    return "VALUETYPE";
  case 0x12:
    return "CLASS";
  case 0x13:
    return "VAR";
  case 0x14:
    return "ARRAY";
  case 0x15:
    return "GENERICINST";
  case 0x16:
    return "TYPEDBYREF";
  case 0x18:
    return "I";
  case 0x19:
    return "U";
  case 0x1B:
    return "FNPTR";
  case 0x1C:
    return "OBJECT";
  case 0x1D:
    return "SZARRAY";
  case 0x1E:
    return "MVAR";
  case 0x1F:
    return "CMOD_REQD";
  case 0x20:
    return "CMOD_OPT";
  case 0x21:
    return "INTERNAL";
  case 0x40:
    return "MODIFIER";
  case 0x41:
    return "SENTINEL";
  case 0x45:
    return "PINNED";
  case 0x55:
    return "ENUM";
  default:
    return "";
  }
}

inline std::string BuildRuntimeResolvedClassName(
    const RuntimeClassProbeResult &runtime) {
  if (!runtime.fullName.empty()) {
    return runtime.fullName;
  }
  return BuildRuntimeClassFullName(runtime.namespaze, runtime.className);
}

inline bool IsPrintableRuntimeTypeName(const std::string &value) {
  if (value.empty() || value.size() > 256) {
    return false;
  }

  for (unsigned char ch : value) {
    if (ch < 0x20 || ch > 0x7E) {
      return false;
    }
  }

  return true;
}

inline bool TryResolveRuntimeClassTypeName(
    const IMemoryAccessor &mem, const Il2CppLayoutProfile &profile,
    std::uintptr_t klass, std::string &outName) {
  outName.clear();
  if (!IsCanonicalUserPtr(klass)) {
    return false;
  }

  RuntimeClassProbeResult runtime;
  if (ProbeRuntimeClass(mem, klass, profile, runtime)) {
    const std::string name = BuildRuntimeResolvedClassName(runtime);
    if (IsPrintableRuntimeTypeName(name)) {
      outName = name;
      return true;
    }
  }

  KlassHeaderProbeResult header;
  if (!ProbeKlassHeaderStrings(mem, klass, header) ||
      !IsPrintableRuntimeTypeName(header.bestName)) {
    return false;
  }

  if (!header.bestNamespace.empty() &&
      IsPrintableRuntimeTypeName(header.bestNamespace)) {
    outName = BuildRuntimeClassFullName(header.bestNamespace, header.bestName);
  } else {
    outName = header.bestName;
  }
  return !outName.empty();
}

inline bool TryReadRuntimeGenericInstArgs(
    const IMemoryAccessor &mem, const Il2CppLayoutProfile &profile,
    std::uintptr_t inst, int depth, std::vector<std::string> &args) {
  args.clear();
  if (!IsCanonicalUserPtr(inst)) {
    return false;
  }

  std::uint32_t argc = 0;
  std::uintptr_t argv = 0;
  if (!ReadValue(mem, inst + 0x00u, argc) ||
      !ReadPtr(mem, inst + 0x08u, argv) || !IsCanonicalUserPtr(argv) ||
      argc == 0 || argc >= 32) {
    return false;
  }

  args.reserve(argc);
  for (std::uint32_t i = 0; i < argc; ++i) {
    std::uintptr_t argType = 0;
    RuntimeIl2CppTypeDescription arg;
    if (ReadPtr(mem, argv + static_cast<std::uintptr_t>(i) * 8u, argType) &&
        DescribeRuntimeIl2CppTypeInternal(mem, profile, argType, arg,
                                          depth + 1) &&
        IsPrintableRuntimeTypeName(arg.name)) {
      args.push_back(arg.name);
    }
  }

  return !args.empty();
}

inline std::string BuildRuntimeGenericTypeName(
    const std::string &baseName, const std::vector<std::string> &args) {
  if (baseName.empty()) {
    return "";
  }

  if (args.empty()) {
    return baseName;
  }

  std::ostringstream ss;
  ss << baseName << "<";
  for (std::size_t i = 0; i < args.size(); ++i) {
    if (i != 0) {
      ss << ", ";
    }
    ss << args[i];
  }
  ss << ">";
  return ss.str();
}

inline bool SampleRuntimePointerWords(const IMemoryAccessor &mem,
                                      std::uintptr_t address,
                                      std::vector<std::uintptr_t> &out,
                                      std::size_t count = 8) {
  out.clear();
  if (!IsCanonicalUserPtr(address) || count == 0 || count > 16) {
    return false;
  }

  out.reserve(count);
  for (std::size_t i = 0; i < count; ++i) {
    std::uintptr_t word = 0;
    if (!ReadPtr(mem, address + static_cast<std::uintptr_t>(i) * 8u, word)) {
      break;
    }
    out.push_back(word);
  }

  return !out.empty();
}

inline bool DescribeRuntimeIl2CppTypeInternal(
    const IMemoryAccessor &mem, const Il2CppLayoutProfile &profile,
    std::uintptr_t typePtr, RuntimeIl2CppTypeDescription &out, int depth) {
  out = RuntimeIl2CppTypeDescription{};
  out.typePtr = typePtr;

  if (!IsCanonicalUserPtr(typePtr) || depth > 8) {
    return false;
  }

  (void)ReadPtr(mem, typePtr, out.data);
  if (!ReadValue(mem, typePtr + 0x08u, out.metaBits)) {
    return false;
  }

  out.typeEnum = (out.metaBits >> 16) & 0xFFu;
  out.name = RuntimePrimitiveIl2CppTypeName(out.typeEnum);
  if (!out.name.empty()) {
    return true;
  }

  RuntimeIl2CppTypeDescription snapshot = out;

  if (out.typeEnum == 0x11u || out.typeEnum == 0x12u) {
    if (!IsCanonicalUserPtr(out.data)) {
      return false;
    }

    if (!TryResolveRuntimeClassTypeName(mem, profile, out.data, out.name)) {
      out = snapshot;
      return false;
    }
    return !out.name.empty();
  }

  if (out.typeEnum == 0x1Du || out.typeEnum == 0x0Fu ||
      out.typeEnum == 0x10u) {
    RuntimeIl2CppTypeDescription elem;
    if (!DescribeRuntimeIl2CppTypeInternal(mem, profile, out.data, elem,
                                           depth + 1)) {
      out = snapshot;
      return false;
    }

    out.name = elem.name;
    if (out.typeEnum == 0x1Du) {
      out.name += "[]";
    } else if (out.typeEnum == 0x0Fu) {
      out.name += "*";
    } else {
      out.name += "&";
    }
    return true;
  }

  if (out.typeEnum == 0x14u) {
    if (!IsCanonicalUserPtr(out.data)) {
      return false;
    }

    std::uintptr_t elemType = 0;
    std::uint8_t rank = 0;
    if (!ReadPtr(mem, out.data + 0x00u, elemType) ||
        !ReadValue(mem, out.data + 0x08u, rank)) {
      return false;
    }

    RuntimeIl2CppTypeDescription elem;
    if (!DescribeRuntimeIl2CppTypeInternal(mem, profile, elemType, elem,
                                           depth + 1)) {
      out = snapshot;
      return false;
    }

    if (rank <= 1) {
      out.name = elem.name + "[]";
    } else {
      out.name = elem.name + "[" + std::string(rank - 1, ',') + "]";
    }
    return true;
  }

  if (out.typeEnum == 0x15u) {
    if (!IsCanonicalUserPtr(out.data)) {
      return false;
    }

    std::uintptr_t baseType = 0;
    std::uintptr_t inst = 0;
    (void)ReadPtr(mem, out.data + 0x00u, baseType);
    (void)ReadPtr(mem, out.data + 0x08u, inst);

    RuntimeIl2CppTypeDescription base;
    std::vector<std::string> args;
    if (DescribeRuntimeIl2CppTypeInternal(mem, profile, baseType, base,
                                          depth + 1)) {
      (void)TryReadRuntimeGenericInstArgs(mem, profile, inst, depth, args);
      out.name = BuildRuntimeGenericTypeName(base.name, args);
      return !out.name.empty();
    }

    std::uintptr_t genericClassTypeDefinition = 0;
    std::uintptr_t genericClassInst = 0;
    (void)ReadPtr(mem, out.data + 0x00u, genericClassTypeDefinition);
    (void)ReadPtr(mem, out.data + 0x08u, genericClassInst);

    std::string baseName;
    if (TryResolveRuntimeClassTypeName(mem, profile,
                                       genericClassTypeDefinition,
                                       baseName)) {
      (void)TryReadRuntimeGenericInstArgs(mem, profile, genericClassInst,
                                          depth, args);
      out.name = BuildRuntimeGenericTypeName(baseName, args);
      return !out.name.empty();
    }

    std::uintptr_t cachedClass = 0;
    (void)ReadPtr(mem, out.data + 0x18u, cachedClass);
    if (TryResolveRuntimeClassTypeName(mem, profile, cachedClass, baseName)) {
      (void)TryReadRuntimeGenericInstArgs(mem, profile, genericClassInst,
                                          depth, args);
      out.name = BuildRuntimeGenericTypeName(baseName, args);
      return !out.name.empty();
    }

    out = snapshot;
    return false;
  }

  return false;
}

inline bool DescribeRuntimeIl2CppType(
    const IMemoryAccessor &mem, const Il2CppLayoutProfile &profile,
    std::uintptr_t typePtr, RuntimeIl2CppTypeDescription &out) {
  return DescribeRuntimeIl2CppTypeInternal(mem, profile, typePtr, out, 0);
}

} // namespace er2
