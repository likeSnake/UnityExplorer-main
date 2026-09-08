#pragma once

#include "probe_cli.hpp"

#include <algorithm>
#include <cstdint>
#include <set>
#include <sstream>
#include <string>
#include <vector>

namespace er2 {

struct RuntimeDumpCsFieldRow {
  std::string name;
  std::string typeName;
  std::uint32_t offset = 0;
  std::uintptr_t type = 0;
  std::uintptr_t typeData = 0;
  std::uint32_t typeMetaBits = 0;
  std::uint32_t typeEnum = 0;
  std::string typeKind;
  bool isStatic = false;
};

struct RuntimeDumpCsMethodRow {
  std::string name;
  std::string returnTypeName;
  std::uintptr_t methodInfo = 0;
  std::uintptr_t methodPointer = 0;
  std::uint64_t methodRva = 0;
  std::uintptr_t invokerMethod = 0;
  std::uintptr_t returnType = 0;
  std::uintptr_t parameters = 0;
  std::uint32_t token = 0;
  std::uint16_t flags = 0;
  std::uint16_t iflags = 0;
  std::uint16_t slot = 0xFFFFu;
  std::uint8_t parameterCount = 0;
};

struct RuntimeDumpCsClassRow {
  std::string namespaze;
  std::string className;
  std::vector<std::string> sources;
  std::vector<std::uintptr_t> instances;
  std::uintptr_t klass = 0;
  std::uintptr_t parent = 0;
  std::uintptr_t fieldsPtr = 0;
  std::uintptr_t methodsPtr = 0;
  std::uintptr_t staticFields = 0;
  std::uint16_t fieldCount = 0;
  std::uint16_t methodCount = 0;
  std::vector<RuntimeDumpCsFieldRow> fieldsSample;
  std::vector<RuntimeDumpCsFieldRow> fields;
  std::vector<RuntimeDumpCsMethodRow> methodsSample;
  std::vector<RuntimeDumpCsMethodRow> methods;
};

struct RuntimeDumpCsBuildStats {
  std::size_t classCount = 0;
  std::size_t fieldCount = 0;
  std::size_t methodCount = 0;
};

namespace detail_runtime_dump_cs {

inline bool IsAsciiIdentifierStart(unsigned char ch) {
  return (ch >= 'A' && ch <= 'Z') || (ch >= 'a' && ch <= 'z') || ch == '_';
}

inline bool IsAsciiIdentifierPart(unsigned char ch) {
  return IsAsciiIdentifierStart(ch) || (ch >= '0' && ch <= '9');
}

inline std::string SanitizeIdentifier(const std::string &value,
                                      const std::string &fallback) {
  std::string out;
  out.reserve(value.empty() ? fallback.size() : value.size());

  for (unsigned char ch : value) {
    out.push_back(IsAsciiIdentifierPart(ch) ? static_cast<char>(ch) : '_');
  }

  if (out.empty() ||
      !IsAsciiIdentifierStart(static_cast<unsigned char>(out.front()))) {
    out = fallback.empty() ? std::string("_") : fallback;
  }

  return out;
}

inline std::string MakeUniqueIdentifier(const std::string &base,
                                        std::set<std::string> &used) {
  if (used.insert(base).second) {
    return base;
  }

  for (std::size_t i = 1;; ++i) {
    std::ostringstream candidate;
    candidate << base << "_" << i;
    if (used.insert(candidate.str()).second) {
      return candidate.str();
    }
  }
}

inline std::string SanitizeNamespace(const std::string &value) {
  std::ostringstream out;
  std::string part;
  bool wrotePart = false;

  for (std::size_t i = 0; i <= value.size(); ++i) {
    const char ch = i < value.size() ? value[i] : '.';
    if (ch != '.') {
      part.push_back(ch);
      continue;
    }

    if (!part.empty()) {
      if (wrotePart) {
        out << ".";
      }
      out << SanitizeIdentifier(part, "_");
      wrotePart = true;
      part.clear();
    }
  }

  return out.str();
}

inline std::string CSharpAliasForSystemType(const std::string &typeName) {
  if (typeName == "System.Void") {
    return "void";
  }
  if (typeName == "System.Boolean") {
    return "bool";
  }
  if (typeName == "System.Char") {
    return "char";
  }
  if (typeName == "System.SByte") {
    return "sbyte";
  }
  if (typeName == "System.Byte") {
    return "byte";
  }
  if (typeName == "System.Int16") {
    return "short";
  }
  if (typeName == "System.UInt16") {
    return "ushort";
  }
  if (typeName == "System.Int32") {
    return "int";
  }
  if (typeName == "System.UInt32") {
    return "uint";
  }
  if (typeName == "System.Int64") {
    return "long";
  }
  if (typeName == "System.UInt64") {
    return "ulong";
  }
  if (typeName == "System.Single") {
    return "float";
  }
  if (typeName == "System.Double") {
    return "double";
  }
  if (typeName == "System.String") {
    return "string";
  }
  if (typeName == "System.Object") {
    return "object";
  }
  if (typeName == "System.IntPtr") {
    return "nint";
  }
  if (typeName == "System.UIntPtr") {
    return "nuint";
  }
  return "";
}

inline bool IsSafeResolvedCSharpTypeName(const std::string &typeName) {
  if (typeName.empty() || typeName.size() > 256) {
    return false;
  }

  int genericDepth = 0;
  for (unsigned char ch : typeName) {
    const bool ok =
        IsAsciiIdentifierPart(ch) || ch == '.' || ch == '+' || ch == '`' ||
        ch == '<' || ch == '>' || ch == ',' || ch == ' ' || ch == '[' ||
        ch == ']' || ch == '*' || ch == '&';
    if (!ok) {
      return false;
    }
    if (ch == '<') {
      ++genericDepth;
    } else if (ch == '>') {
      --genericDepth;
      if (genericDepth < 0) {
        return false;
      }
    }
  }

  return genericDepth == 0;
}

inline std::string CSharpTypeForField(const RuntimeDumpCsFieldRow &field) {
  const std::string alias = CSharpAliasForSystemType(field.typeName);
  if (!alias.empty()) {
    return alias;
  }

  if (IsSafeResolvedCSharpTypeName(field.typeName)) {
    return field.typeName;
  }

  return "object";
}

inline std::string CSharpTypeNameOrObject(const std::string &typeName) {
  const std::string alias = CSharpAliasForSystemType(typeName);
  if (!alias.empty()) {
    return alias;
  }

  if (IsSafeResolvedCSharpTypeName(typeName)) {
    return typeName;
  }

  return "object";
}

inline const char *CSharpVisibilityForFlags(std::uint16_t flags) {
  switch (flags & 0x0007u) {
  case 0x0001u:
    return "private ";
  case 0x0002u:
  case 0x0003u:
    return "protected ";
  case 0x0004u:
  case 0x0005u:
    return "protected internal ";
  case 0x0006u:
    return "public ";
  default:
    return "public ";
  }
}

inline bool HasCSharpStaticFlag(std::uint16_t flags) {
  return (flags & 0x0010u) != 0;
}

inline std::string CSharpMethodPrefix(const RuntimeDumpCsMethodRow &method) {
  std::ostringstream prefix;
  prefix << CSharpVisibilityForFlags(method.flags);
  if (HasCSharpStaticFlag(method.flags)) {
    prefix << "static ";
  }
  return prefix.str();
}

inline const std::vector<RuntimeDumpCsFieldRow> &
SelectFields(const RuntimeDumpCsClassRow &klass) {
  return klass.fields.empty() ? klass.fieldsSample : klass.fields;
}

inline const std::vector<RuntimeDumpCsMethodRow> &
SelectMethods(const RuntimeDumpCsClassRow &klass) {
  return klass.methods.empty() ? klass.methodsSample : klass.methods;
}

inline void AppendStandardMethodComment(std::ostringstream &cs,
                                        const std::string &indent,
                                        const RuntimeDumpCsMethodRow &method) {
  cs << indent << "// RVA: " << HexPtr(method.methodRva)
     << " VA: " << HexPtr(method.methodPointer);
  if (method.slot != 0xFFFFu) {
    cs << " Slot: " << method.slot;
  }
  cs << "\n";
}

} // namespace detail_runtime_dump_cs

inline std::string
BuildRuntimeDumpCs(const std::vector<RuntimeDumpCsClassRow> &classes,
                   RuntimeDumpCsBuildStats *stats = nullptr) {
  RuntimeDumpCsBuildStats localStats;
  std::ostringstream cs;
  cs << "// Runtime-only dump.cs generated from live Il2CppClass data.\n";
  cs << "// global-metadata.dat was not required. Output may be partial.\n";
  cs << "// Names, fields and methods are only emitted when runtime memory "
        "provided readable evidence.\n\n";

  std::set<std::string> usedClassNames;
  for (std::size_t classIndex = 0; classIndex < classes.size();
       ++classIndex) {
    const RuntimeDumpCsClassRow &klass = classes[classIndex];
    const std::string className =
        detail_runtime_dump_cs::MakeUniqueIdentifier(
            detail_runtime_dump_cs::SanitizeIdentifier(
                klass.className,
                std::string("Class_") + std::to_string(classIndex)),
            usedClassNames);
    const std::string namespaze =
        detail_runtime_dump_cs::SanitizeNamespace(klass.namespaze);

    const std::string classIndent = "";
    const std::string memberIndent = "    ";
    for (const std::string &source : klass.sources) {
      cs << classIndent << "// Source: " << source << "\n";
    }
    cs << classIndent << "// Class: " << className << " \n";
    cs << classIndent << "// Namespace: " << namespaze << "\n";
    if (!klass.instances.empty()) {
      for (std::uintptr_t instance : klass.instances) {
        cs << classIndent << "// Instance: " << HexPtr(instance) << "\n";
      }
    } else if (klass.staticFields) {
      cs << classIndent << "// Instance: " << HexPtr(klass.staticFields)
         << " // static_fields\n";
    }
    cs << classIndent << "// Klass=" << HexPtr(klass.klass)
       << " Parent=" << HexPtr(klass.parent)
       << " Fields=" << HexPtr(klass.fieldsPtr)
       << " Methods=" << HexPtr(klass.methodsPtr)
       << " StaticFields=" << HexPtr(klass.staticFields) << "\n";
    const auto &fields = detail_runtime_dump_cs::SelectFields(klass);
    const auto &methods = detail_runtime_dump_cs::SelectMethods(klass);
    cs << classIndent << "// RuntimeRawFieldCount=" << klass.fieldCount
       << " RuntimeRawMethodCount=" << klass.methodCount << "\n";
    cs << classIndent << "// EmittedFieldCount=" << fields.size()
       << " EmittedMethodCount=" << methods.size() << "\n";
    cs << classIndent << "public class " << className << "\n";
    cs << classIndent << "{\n";

    std::size_t emittedFields = 0;
    cs << memberIndent << "// Fields\n";
    std::set<std::string> usedFieldNames;
    for (std::size_t i = 0; i < fields.size(); ++i) {
      const RuntimeDumpCsFieldRow &field = fields[i];
      const std::string fieldName =
          detail_runtime_dump_cs::MakeUniqueIdentifier(
              detail_runtime_dump_cs::SanitizeIdentifier(
                  field.name, std::string("field_") + std::to_string(i)),
              usedFieldNames);
      cs << memberIndent << (field.isStatic ? "public static " : "public ")
         << detail_runtime_dump_cs::CSharpTypeForField(field) << " "
         << fieldName << "; // offset:" << HexPtr(field.offset);
      if (field.type) {
        cs << " type=" << HexPtr(field.type);
      }
      if (field.typeData) {
        cs << " data=" << HexPtr(field.typeData);
      }
      if (field.typeMetaBits) {
        cs << " metaBits=" << HexPtr(field.typeMetaBits)
           << " typeEnum=" << HexPtr(field.typeEnum);
        if (!field.typeKind.empty()) {
          cs << " typeKind=" << field.typeKind;
        }
      }
      if (!field.typeName.empty()) {
        cs << " typeName=" << field.typeName;
      }
      cs << "\n";
      ++emittedFields;
    }

    if (emittedFields == 0) {
      cs << memberIndent << "// No readable runtime field names captured.\n";
    }

    cs << "\n" << memberIndent << "// Properties\n";
    cs << memberIndent
       << "// Runtime property table is unavailable without metadata.\n";

    std::size_t emittedMethods = 0;
    cs << "\n" << memberIndent << "// Methods:" << methods.size() << "\n";
    std::set<std::string> usedMethodNames;
    for (std::size_t i = 0; i < methods.size(); ++i) {
      const RuntimeDumpCsMethodRow &method = methods[i];
      const std::string methodName =
          detail_runtime_dump_cs::MakeUniqueIdentifier(
              detail_runtime_dump_cs::SanitizeIdentifier(
                  method.name, std::string("method_") + std::to_string(i)),
              usedMethodNames);
      detail_runtime_dump_cs::AppendStandardMethodComment(cs, memberIndent,
                                                          method);
      cs << memberIndent << detail_runtime_dump_cs::CSharpMethodPrefix(method)
         << detail_runtime_dump_cs::CSharpTypeNameOrObject(
                method.returnTypeName)
         << " " << methodName << "() { } // MethodInfo="
         << HexPtr(method.methodInfo);
      if (method.token) {
        cs << " Token=" << HexPtr(method.token);
      }
      if (method.returnType) {
        cs << " ReturnType=" << HexPtr(method.returnType);
      }
      if (method.parameters) {
        cs << " Parameters=" << HexPtr(method.parameters);
      }
      if (method.parameterCount) {
        cs << " ParameterCount="
           << static_cast<unsigned int>(method.parameterCount);
      }
      if (method.flags) {
        cs << " Flags=" << HexPtr(method.flags);
      }
      if (method.iflags) {
        cs << " IFlags=" << HexPtr(method.iflags);
      }
      if (method.invokerMethod) {
        cs << " Invoker=" << HexPtr(method.invokerMethod);
      }
      cs << "\n";
      ++emittedMethods;
    }

    if (emittedMethods == 0 && klass.methodCount > 0) {
      cs << memberIndent
         << "// Runtime method table present, but no readable method names "
            "captured.\n";
    }

    cs << classIndent << "}\n";
    cs << "\n";

    ++localStats.classCount;
    localStats.fieldCount += emittedFields;
    localStats.methodCount += emittedMethods;
  }

  if (stats) {
    *stats = localStats;
  }
  return cs.str();
}

} // namespace er2
