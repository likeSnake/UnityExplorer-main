// ============================================================================
// runtime_dumper.cpp - In-process IL2CPP type-system dumper.
// ============================================================================
// Walks every assembly -> image -> class in the target process through the
// resolved il2cpp API and emits a standard Il2CppDumper-style dump.cs:
//
//   // Image 0: Assembly-CSharp.dll (42893 classes)
//   // Namespace: Foo
//   public class Bar : Base // TypeDefIndex: 42
//   {
//       // Fields
//       public int m_Value; // 0x10
//       // Properties
//       public int Value { get; set; }
//       // Methods
//       // RVA: 0x123456 Offset: 0x123456 VA: 0x7FF...
//       public void SetValue(int arg0);
//   }
//
// Fields carry their instance offset (via il2cpp_field_get_offset), methods
// carry RVA/VA (method pointer relative to GameAssembly base), and property
// accessors are resolved through get_/set_ methods.
// ============================================================================

#include <Windows.h>
#include <psapi.h>

#ifndef NOMINMAX
#define NOMINMAX
#endif

#include <algorithm>
#include <cstdint>
#include <cstdio>
#include <cstring>
#include <fstream>
#include <map>
#include <set>
#include <sstream>
#include <string>
#include <vector>

#include "il2cpp_api.hpp"

namespace il2 {

// ---------------------------------------------------------------------------
// Type name helper
// ---------------------------------------------------------------------------

inline std::string TypeName(const Il2CppApi &api, Il2CppType *type) {
  if (!type)
    return "void";
  if (api.type_get_name)
    if (const char *n = api.type_get_name(type))
      return n;
  if (api.type_get_class_or_element_class) {
    if (Il2CppClass *k = api.type_get_class_or_element_class(type))
      if (api.class_get_name)
        if (const char *n = api.class_get_name(k))
          return n;
  }
  return "void";
}

inline bool IsStaticField(std::uint32_t flags) {
  // FIELD_ATTRIBUTE_STATIC = 0x0010
  return (flags & 0x0010u) != 0;
}

inline bool IsPublicField(std::uint32_t flags) {
  // FIELD_ATTRIBUTE_FIELD_ACCESS_MASK = 0x0007 ; PUBLIC = 0x0006
  return (flags & 0x0007u) == 0x0006u;
}

inline std::string AccessModifier(std::uint32_t flags, bool isField) {
  const std::uint32_t mask = isField ? 0x0007u : 0x0007u;
  switch (flags & mask) {
  case 0x0001u:
    return "private";
  case 0x0002u:
  case 0x0003u:
    return "protected";
  case 0x0004u:
  case 0x0005u:
    return "protected internal";
  case 0x0006u:
    return "public";
  default:
    return "public";
  }
}

// ---------------------------------------------------------------------------
// C# type aliasing for primitive names returned by il2cpp.
// ---------------------------------------------------------------------------
inline std::string CsAlias(const std::string &n) {
  if (n == "System.Void")
    return "void";
  if (n == "System.Boolean")
    return "bool";
  if (n == "System.Char")
    return "char";
  if (n == "System.SByte")
    return "sbyte";
  if (n == "System.Byte")
    return "byte";
  if (n == "System.Int16")
    return "short";
  if (n == "System.UInt16")
    return "ushort";
  if (n == "System.Int32")
    return "int";
  if (n == "System.UInt32")
    return "uint";
  if (n == "System.Int64")
    return "long";
  if (n == "System.UInt64")
    return "ulong";
  if (n == "System.Single")
    return "float";
  if (n == "System.Double")
    return "double";
  if (n == "System.String")
    return "string";
  if (n == "System.Object")
    return "object";
  if (n == "System.IntPtr")
    return "nint";
  if (n == "System.UIntPtr")
    return "nuint";
  return n;
}

// ---------------------------------------------------------------------------
// Dump options
// ---------------------------------------------------------------------------
struct DumpOptions {
  std::string outPath;          // default: C:\YJDumped\dump.cs (like reference)
  bool includeProperties = true;
  bool includeMethods = true;
  bool includeFields = true;
  bool includePrivateMembers = false; // reference omits private by default
  size_t maxClassesPerImage = 0;      // 0 = unlimited
  bool verbose = false;
};

// ---------------------------------------------------------------------------
// Main dump routine
// ---------------------------------------------------------------------------
inline bool RunDump(const Il2CppApi &api, const DumpOptions &options,
                    std::string &error) {
  error.clear();

  if (!api.domain_get_assemblies || !api.assembly_get_image) {
    error = "core il2cpp symbols missing (protected build?)";
    return false;
  }

  Il2CppDomain *domain = nullptr;
  if (api.domain_get)
    domain = api.domain_get();
  if (!domain) {
    error = "il2cpp_domain_get returned null";
    return false;
  }

  // Collect assemblies.
  std::vector<Il2CppAssembly *> assemblies;
  if (api.domain_get_assemblies_count) {
    const size_t count = api.domain_get_assemblies_count(domain);
    if (count > 0 && count < 10000) {
      if (api.domain_get_assemblies) {
        Il2CppAssembly **arr = api.domain_get_assemblies(domain, nullptr);
        if (arr) {
          for (size_t i = 0; i < count; ++i)
            assemblies.push_back(arr[i]);
        }
      }
    }
  }
  if (assemblies.empty() && api.domain_get_assemblies) {
    size_t count = 0;
    Il2CppAssembly **arr = api.domain_get_assemblies(domain, &count);
    if (arr) {
      for (size_t i = 0; i < count; ++i)
        assemblies.push_back(arr[i]);
    }
  }
  if (assemblies.empty()) {
    error = "no assemblies enumerated";
    return false;
  }

  // Open output.
  std::ofstream ofs(options.outPath,
                    std::ios::binary | std::ios::out | std::ios::trunc);
  if (!ofs.good()) {
    error = "failed to open output file: " + options.outPath;
    return false;
  }

  ofs << "// In-process IL2CPP runtime dump (il2cpp API walker).\n";
  ofs << "// GameAssembly: " << api.moduleName << " @ 0x" << std::hex
      << api.baseAddress << std::dec << "\n";
  ofs << "// Assemblies: " << assemblies.size() << "\n\n";

  size_t totalClasses = 0;
  size_t totalFields = 0;
  size_t totalMethods = 0;
  size_t totalProperties = 0;
  size_t imageIndex = 0;

  std::set<std::string> usedClassNames;

  for (Il2CppAssembly *assembly : assemblies) {
    if (!assembly)
      continue;
    Il2CppImage *image = api.assembly_get_image(assembly);
    if (!image)
      continue;

    std::string imageName = "<unknown>";
    if (api.image_get_name)
      if (const char *n = api.image_get_name(image))
        imageName = n;

    size_t classCount = 0;
    if (api.image_get_class_count)
      classCount = api.image_get_class_count(image);
    if (classCount == 0 || classCount > 10000000u) {
      ofs << "// Image " << imageIndex << ": " << imageName
          << " (0 classes - skipped)\n\n";
      ++imageIndex;
      continue;
    }

    ofs << "// Image " << imageIndex << ": " << imageName << " ("
        << classCount << " classes)\n\n";
    ++imageIndex;

    const size_t classLimit =
        options.maxClassesPerImage ? std::min(classCount, options.maxClassesPerImage)
                                   : classCount;

    for (size_t ci = 0; ci < classLimit; ++ci) {
      Il2CppClass *klass = api.image_get_class(image, ci);
      if (!klass)
        continue;

      std::string name = "<unknown>";
      if (api.class_get_name)
        if (const char *n = api.class_get_name(klass))
          name = n;
      if (name.empty() || name == "<Module>")
        continue; // skip module initializer pseudo-class

      std::string ns;
      if (api.class_get_namespace)
        if (const char *n = api.class_get_namespace(klass))
          ns = n;

      // Parent / base class
      std::string parentName;
      if (api.class_get_parent)
        if (Il2CppClass *parent = api.class_get_parent(klass))
          if (api.class_get_name)
            if (const char *pn = api.class_get_name(parent))
              parentName = pn;

      // kind
      bool isEnum = false, isValueType = false, isInterface = false,
           isAbstract = false;
      if (api.class_is_enum)
        isEnum = api.class_is_enum(klass) != 0;
      if (api.class_is_valuetype)
        isValueType = api.class_is_valuetype(klass) != 0;
      if (api.class_is_interface)
        isInterface = api.class_is_interface(klass) != 0;
      if (api.class_is_abstract)
        isAbstract = api.class_is_abstract(klass) != 0;

      std::string kind = "class";
      if (isInterface)
        kind = "interface";
      else if (isEnum)
        kind = "enum";
      else if (isValueType)
        kind = "struct";

      // TypeDefIndex (token & 0x00FFFFFF, standard)
      int typeDefIndex = -1;
      if (api.class_get_type_token)
        typeDefIndex = api.class_get_type_token(klass) & 0x00FFFFFF;

      // Unique display name within this dump.
      std::string display = name;
      if (!usedClassNames.insert(display).second) {
        for (size_t k = 1;; ++k) {
          std::string cand = name + "_" + std::to_string(k);
          if (usedClassNames.insert(cand).second) {
            display = cand;
            break;
          }
        }
      }

      ofs << "// Namespace: " << ns << "\n";
      ofs << "public " << kind << " " << display;
      if (!parentName.empty() && parentName != "System.Object" &&
          parentName != "System.ValueType" && parentName != "System.Enum")
        ofs << " : " << parentName;
      if (typeDefIndex >= 0)
        ofs << " // TypeDefIndex: " << typeDefIndex;
      ofs << "\n{\n";

      // ---- Fields ----
      if (options.includeFields) {
        ofs << "\t// Fields\n";
        if (api.class_get_fields && api.field_get_name && api.field_get_offset) {
          void *iter = nullptr;
          size_t fieldCount = 0;
          while (FieldInfo *field = api.class_get_fields(klass, &iter)) {
            const char *fname = api.field_get_name(field);
            if (!fname || !fname[0])
              continue;
            std::uint32_t flags = 0;
            if (api.field_get_flags)
              flags = api.field_get_flags(field);
            if (!options.includePrivateMembers &&
                (flags & 0x0007u) != 0x0006u)
              continue; // private/protected omitted like reference output
            std::string ftype = "object";
            if (api.field_get_type)
              ftype = CsAlias(TypeName(api, api.field_get_type(field)));
            const size_t offset = api.field_get_offset(field);
            ofs << "\tpublic " << ftype << " " << fname << "; // 0x" << std::hex
                << offset << std::dec;
            if (IsStaticField(flags))
              ofs << " [static]";
            ofs << "\n";
            ++fieldCount;
            ++totalFields;
          }
          (void)fieldCount;
        }
      }

      // ---- Properties ----
      if (options.includeProperties && api.class_get_properties &&
          api.property_get_name) {
        ofs << "\n\t// Properties\n";
        void *iter = nullptr;
        while (PropertyInfo *prop = api.class_get_properties(klass, &iter)) {
          const char *pname = api.property_get_name(prop);
          if (!pname || !pname[0])
            continue;
          MethodInfo *getter = nullptr, *setter = nullptr;
          if (api.property_get_get_method)
            getter = api.property_get_get_method(prop);
          if (api.property_get_set_method)
            setter = api.property_get_set_method(prop);
          std::string ptype = "object";
          if (getter && api.method_get_return_type)
            ptype = CsAlias(TypeName(api, api.method_get_return_type(getter)));
          else if (setter && api.method_get_param_count &&
                   api.method_get_param &&
                   api.method_get_param_count(setter) >= 1)
            ptype = CsAlias(TypeName(api, api.method_get_param(setter, 0)));
          ofs << "\t" << ptype << " " << pname << " { ";
          if (getter)
            ofs << "get; ";
          if (setter)
            ofs << "set; ";
          ofs << "}\n";
          ++totalProperties;
        }
      }

      // ---- Methods ----
      if (options.includeMethods && api.class_get_methods &&
          api.method_get_name) {
        ofs << "\n\t// Methods:" << "\n";
        void *iter = nullptr;
        while (MethodInfo *method = api.class_get_methods(klass, &iter)) {
          const char *mname = api.method_get_name(method);
          if (!mname || !mname[0])
            continue;

          // skip property accessors when properties are shown separately? keep them.
          std::uint32_t flags = 0;
          if (api.method_get_flags)
            flags = api.method_get_flags(method);
          if (!options.includePrivateMembers &&
              (flags & 0x0007u) != 0x0006u)
            continue;

          std::string ret = "void";
          if (api.method_get_return_type)
            ret = CsAlias(TypeName(api, api.method_get_return_type(method)));

          std::uint32_t paramCount = 0;
          if (api.method_get_param_count)
            paramCount = api.method_get_param_count(method);
          if (paramCount > 64)
            paramCount = 64;

          std::string paramsStr;
          for (std::uint32_t pi = 0; pi < paramCount; ++pi) {
            if (pi)
              paramsStr += ", ";
            std::string ptype = "object";
            if (api.method_get_param)
              ptype = CsAlias(TypeName(api, api.method_get_param(method, pi)));
            std::string pname = "arg" + std::to_string(pi);
            if (api.method_get_param_name)
              if (const char *pn = api.method_get_param_name(method, pi))
                if (pn[0])
                  pname = pn;
            paramsStr += ptype + " " + pname;
          }

          // method pointer -> RVA/VA
          void *methodPtr = nullptr;
          if (api.method_get_pointer)
            methodPtr = api.method_get_pointer(method);
          std::uintptr_t va =
              reinterpret_cast<std::uintptr_t>(methodPtr);
          std::uint64_t rva = 0;
          if (va && api.baseAddress && va >= api.baseAddress)
            rva = va - api.baseAddress;

          ofs << "\t// RVA: 0x" << std::hex << rva << " VA: 0x" << va
              << std::dec;
          std::uint32_t token = 0;
          if (api.method_get_token)
            token = api.method_get_token(method);
          if (token)
            ofs << " Token: 0x" << std::hex << token << std::dec;
          ofs << "\n";
          ofs << "\tpublic " << ret << " " << mname << "(" << paramsStr
              << ");\n";
          ++totalMethods;
        }
      }

      ofs << "}\n\n";
      ++totalClasses;
    }
  }

  ofs.flush();
  ofs.close();

  if (options.verbose) {
    std::string summary =
        "[Dump] classes=" + std::to_string(totalClasses) +
        " fields=" + std::to_string(totalFields) +
        " methods=" + std::to_string(totalMethods) +
        " properties=" + std::to_string(totalProperties) +
        " -> " + options.outPath;
    OutputDebugStringA(summary.c_str());
    FILE *f = fopen((options.outPath + ".log").c_str(), "a");
    if (f) {
      fprintf(f, "%s\n", summary.c_str());
      fclose(f);
    }
  }
  return true;
}

} // namespace il2
