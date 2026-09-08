#pragma once
// ============================================================================
// il2cpp_api.hpp - In-process IL2CPP API resolution for injected dumper.
// ============================================================================
// Resolves the il2cpp_* exports from GameAssembly.dll via GetProcAddress after
// the DLL is injected into the game process. This is the same mechanism used
// by the reference injector (Unity-Offset.dll / il2CppRuntimeDumperEx): the
// plain GameAssembly.dll exports the full il2cpp runtime API, so an in-process
// dumper can walk the whole managed type system through the official API.
//
// Layout note: all resolve calls happen inside the target process; every
// pointer here is a real function pointer in the game's address space.
// ============================================================================

#include <Windows.h>
#include <psapi.h>
#include <cstdint>
#include <cstdio>
#include <cstring>
#include <string>

namespace il2 {

// ---------------------------------------------------------------------------
// Forward declarations of the IL2CPP opaque types we need to name.
// ---------------------------------------------------------------------------
struct Il2CppDomain;
struct Il2CppAssembly;
struct Il2CppImage;
struct Il2CppClass;
struct Il2CppType;
struct FieldInfo;
struct MethodInfo;
struct PropertyInfo;
struct Il2CppString;
struct Il2CppObject;

// ---------------------------------------------------------------------------
// The resolved function table.
// ---------------------------------------------------------------------------
struct Il2CppApi {
  HMODULE gameAssembly = nullptr;
  std::uintptr_t baseAddress = 0;
  std::string moduleName;

  // Domain / assembly / image
  Il2CppDomain *(*domain_get)(void) = nullptr;
  Il2CppAssembly **(*domain_get_assemblies)(Il2CppDomain *, size_t *) = nullptr;
  size_t (*domain_get_assemblies_count)(Il2CppDomain *) = nullptr;
  const char *(*assembly_get_name)(const Il2CppAssembly *) = nullptr;
  Il2CppImage *(*assembly_get_image)(const Il2CppAssembly *) = nullptr;
  const char *(*image_get_name)(const Il2CppImage *) = nullptr;
  Il2CppClass *(*image_get_class)(const Il2CppImage *, size_t) = nullptr;
  size_t (*image_get_class_count)(const Il2CppImage *) = nullptr;
  Il2CppClass *(*image_class_from_name)(const Il2CppImage *, const char *,
                                        const char *) = nullptr;
  const char *(*image_get_filename)(const Il2CppImage *) = nullptr;

  // Class
  const char *(*class_get_name)(const Il2CppClass *) = nullptr;
  const char *(*class_get_namespace)(const Il2CppClass *) = nullptr;
  Il2CppClass *(*class_get_parent)(const Il2CppClass *) = nullptr;
  Il2CppType *(*class_get_type)(const Il2CppClass *) = nullptr;
  Il2CppClass *(*class_from_type)(const Il2CppType *) = nullptr;
  int (*class_get_type_token)(const Il2CppClass *) = nullptr;
  std::uint32_t (*class_get_flags)(const Il2CppClass *) = nullptr;
  int (*class_is_valuetype)(const Il2CppClass *) = nullptr;
  int (*class_is_enum)(const Il2CppClass *) = nullptr;
  int (*class_is_abstract)(const Il2CppClass *) = nullptr;
  int (*class_is_interface)(const Il2CppClass *) = nullptr;
  int (*class_is_generic)(const Il2CppClass *) = nullptr;
  int (*class_is_inflated)(const Il2CppClass *) = nullptr;
  const Il2CppType *(*class_enum_basetype)(const Il2CppClass *) = nullptr;
  size_t (*class_num_fields)(const Il2CppClass *) = nullptr;
  size_t (*class_num_methods)(const Il2CppClass *) = nullptr;
  size_t (*class_num_properties)(const Il2CppClass *) = nullptr;
  const char *(*class_get_assemblyname)(const Il2CppClass *) = nullptr;
  Il2CppImage *(*class_get_image)(const Il2CppClass *) = nullptr;

  // Fields
  FieldInfo *(*class_get_fields)(const Il2CppClass *, void **) = nullptr;
  const char *(*field_get_name)(const FieldInfo *) = nullptr;
  Il2CppType *(*field_get_type)(const FieldInfo *) = nullptr;
  size_t (*field_get_offset)(const FieldInfo *) = nullptr;
  std::uint32_t (*field_get_flags)(const FieldInfo *) = nullptr;
  Il2CppClass *(*field_get_parent)(const FieldInfo *) = nullptr;
  int (*field_get_token)(const FieldInfo *) = nullptr;
  int (*field_is_literal)(const FieldInfo *) = nullptr;
  int (*field_is_static)(const FieldInfo *) = nullptr;

  // Methods
  MethodInfo *(*class_get_methods)(const Il2CppClass *, void **) = nullptr;
  const char *(*method_get_name)(const MethodInfo *) = nullptr;
  Il2CppClass *(*method_get_class)(const MethodInfo *) = nullptr;
  Il2CppClass *(*method_get_declaring_type)(const MethodInfo *) = nullptr;
  Il2CppType *(*method_get_return_type)(const MethodInfo *) = nullptr;
  std::uint32_t (*method_get_param_count)(const MethodInfo *) = nullptr;
  const char *(*method_get_param_name)(const MethodInfo *, std::uint32_t) = nullptr;
  Il2CppType *(*method_get_param)(const MethodInfo *, std::uint32_t) = nullptr;
  std::uint32_t (*method_get_token)(const MethodInfo *) = nullptr;
  std::uint32_t (*method_get_flags)(const MethodInfo *) = nullptr;
  std::uint32_t (*method_get_iflags)(const MethodInfo *) = nullptr;
  int (*method_is_static)(const MethodInfo *) = nullptr;
  int (*method_is_instance)(const MethodInfo *) = nullptr;
  int (*method_is_generic)(const MethodInfo *) = nullptr;
  int (*method_is_inflated)(const MethodInfo *) = nullptr;
  void *(*method_get_pointer)(const MethodInfo *) = nullptr;
  void *(*method_get_invoker)(const MethodInfo *) = nullptr;

  // Properties
  PropertyInfo *(*class_get_properties)(const Il2CppClass *, void **) = nullptr;
  const char *(*property_get_name)(const PropertyInfo *) = nullptr;
  MethodInfo *(*property_get_get_method)(const PropertyInfo *) = nullptr;
  MethodInfo *(*property_get_set_method)(const PropertyInfo *) = nullptr;
  Il2CppClass *(*property_get_parent)(const PropertyInfo *) = nullptr;
  std::uint32_t (*property_get_flags)(const PropertyInfo *) = nullptr;
  int (*property_get_token)(const PropertyInfo *) = nullptr;

  // Types
  const char *(*type_get_name)(const Il2CppType *) = nullptr;
  const char *(*type_get_assembly_qualified_name)(const Il2CppType *) = nullptr;
  Il2CppClass *(*type_get_class_or_element_class)(const Il2CppType *) = nullptr;
  std::uint32_t (*type_get_attrs)(const Il2CppType *) = nullptr;
  int (*type_is_byref)(const Il2CppType *) = nullptr;
  int (*type_is_pointer)(const Il2CppType *) = nullptr;
  int (*type_is_static)(const Il2CppType *) = nullptr;

  // Runtime (minimal, for bootstrapping)
  Il2CppDomain *(*domain_assembly_open)(Il2CppDomain *, const char *) = nullptr;
  Il2CppDomain *(*get_corlib)(void) = nullptr;
  const Il2CppAssembly *(*assembly_load)(const char *) = nullptr;
};

// ---------------------------------------------------------------------------
// Resolution helper.
// ---------------------------------------------------------------------------
inline bool ResolveSymbol(const Il2CppApi &api, const char *name,
                          void *&out) {
  out = nullptr;
  if (!api.gameAssembly || !name)
    return false;
  out = reinterpret_cast<void *>(
      GetProcAddress(api.gameAssembly, name));
  return out != nullptr;
}

#define IL2_RESOLVE(api, member, name)                                        \
  do {                                                                         \
    void *__p = nullptr;                                                       \
    if (ResolveSymbol(api, name, __p)) {                                       \
      *reinterpret_cast<void **>(&(api).member) = __p;                         \
    }                                                                          \
  } while (0)

// ---------------------------------------------------------------------------
// Resolve the whole table from GameAssembly.dll.
// Returns true if the core symbols (domain_get_assemblies, class_get_fields,
// method_get_name, field_get_offset) are all present.
// ---------------------------------------------------------------------------
inline bool ResolveIl2CppApi(Il2CppApi &api) {
  std::memset(&api, 0, sizeof(api));

  // Find GameAssembly*.dll in the current process.
  api.moduleName = "GameAssembly.dll";
  api.gameAssembly = GetModuleHandleA("GameAssembly.dll");
  if (!api.gameAssembly) {
    // Variant names like GameAssembly_Super.dll / GameAssembly_Super_IBT.dll
    api.gameAssembly = GetModuleHandleA("GameAssembly_Super.dll");
    if (api.gameAssembly)
      api.moduleName = "GameAssembly_Super.dll";
    else {
      api.gameAssembly = GetModuleHandleA("GameAssembly_Super_IBT.dll");
      if (api.gameAssembly)
        api.moduleName = "GameAssembly_Super_IBT.dll";
    }
  }
  if (!api.gameAssembly) {
    // Fall back to enumerating loaded modules for any GameAssembly*.dll
    HMODULE modules[1024];
    DWORD needed = 0;
    if (EnumProcessModulesEx(GetCurrentProcess(), modules, sizeof(modules),
                             &needed, LIST_MODULES_ALL)) {
      const DWORD count = needed / sizeof(HMODULE);
      for (DWORD i = 0; i < count; ++i) {
        char name[MAX_PATH] = {};
        if (GetModuleBaseNameA(GetCurrentProcess(), modules[i], name,
                               sizeof(name)) &&
            _strnicmp(name, "GameAssembly", 12) == 0) {
          api.gameAssembly = modules[i];
          api.moduleName = name;
          break;
        }
      }
    }
  }
  if (!api.gameAssembly)
    return false;

  // base address
  MODULEINFO mi = {};
  if (GetModuleInformation(GetCurrentProcess(), api.gameAssembly, &mi,
                           sizeof(mi)))
    api.baseAddress = reinterpret_cast<std::uintptr_t>(mi.lpBaseOfDll);

  // Resolve every symbol we care about.
  IL2_RESOLVE(api, domain_get, "il2cpp_domain_get");
  IL2_RESOLVE(api, domain_get_assemblies, "il2cpp_domain_get_assemblies");
  IL2_RESOLVE(api, domain_get_assemblies_count,
              "il2cpp_domain_get_assemblies_count");
  IL2_RESOLVE(api, assembly_get_name, "il2cpp_assembly_get_name");
  IL2_RESOLVE(api, assembly_get_image, "il2cpp_assembly_get_image");
  IL2_RESOLVE(api, image_get_name, "il2cpp_image_get_name");
  IL2_RESOLVE(api, image_get_class, "il2cpp_image_get_class");
  IL2_RESOLVE(api, image_get_class_count, "il2cpp_image_get_class_count");
  IL2_RESOLVE(api, image_class_from_name, "il2cpp_image_class_from_name");
  IL2_RESOLVE(api, image_get_filename, "il2cpp_image_get_filename");
  IL2_RESOLVE(api, class_get_name, "il2cpp_class_get_name");
  IL2_RESOLVE(api, class_get_namespace, "il2cpp_class_get_namespace");
  IL2_RESOLVE(api, class_get_parent, "il2cpp_class_get_parent");
  IL2_RESOLVE(api, class_get_type, "il2cpp_class_get_type");
  IL2_RESOLVE(api, class_from_type, "il2cpp_class_from_type");
  IL2_RESOLVE(api, class_get_type_token, "il2cpp_class_get_type_token");
  IL2_RESOLVE(api, class_get_flags, "il2cpp_class_get_flags");
  IL2_RESOLVE(api, class_is_valuetype, "il2cpp_class_is_valuetype");
  IL2_RESOLVE(api, class_is_enum, "il2cpp_class_is_enum");
  IL2_RESOLVE(api, class_is_abstract, "il2cpp_class_is_abstract");
  IL2_RESOLVE(api, class_is_interface, "il2cpp_class_is_interface");
  IL2_RESOLVE(api, class_is_generic, "il2cpp_class_is_generic");
  IL2_RESOLVE(api, class_is_inflated, "il2cpp_class_is_inflated");
  IL2_RESOLVE(api, class_enum_basetype, "il2cpp_class_enum_basetype");
  IL2_RESOLVE(api, class_num_fields, "il2cpp_class_num_fields");
  IL2_RESOLVE(api, class_num_methods, "il2cpp_class_num_methods");
  IL2_RESOLVE(api, class_num_properties, "il2cpp_class_num_properties");
  IL2_RESOLVE(api, class_get_assemblyname, "il2cpp_class_get_assemblyname");
  IL2_RESOLVE(api, class_get_image, "il2cpp_class_get_image");
  IL2_RESOLVE(api, class_get_fields, "il2cpp_class_get_fields");
  IL2_RESOLVE(api, field_get_name, "il2cpp_field_get_name");
  IL2_RESOLVE(api, field_get_type, "il2cpp_field_get_type");
  IL2_RESOLVE(api, field_get_offset, "il2cpp_field_get_offset");
  IL2_RESOLVE(api, field_get_flags, "il2cpp_field_get_flags");
  IL2_RESOLVE(api, field_get_parent, "il2cpp_field_get_parent");
  IL2_RESOLVE(api, field_get_token, "il2cpp_field_get_token");
  IL2_RESOLVE(api, field_is_literal, "il2cpp_field_is_literal");
  IL2_RESOLVE(api, field_is_static, "il2cpp_field_is_static");
  IL2_RESOLVE(api, class_get_methods, "il2cpp_class_get_methods");
  IL2_RESOLVE(api, method_get_name, "il2cpp_method_get_name");
  IL2_RESOLVE(api, method_get_class, "il2cpp_method_get_class");
  IL2_RESOLVE(api, method_get_declaring_type, "il2cpp_method_get_declaring_type");
  IL2_RESOLVE(api, method_get_return_type, "il2cpp_method_get_return_type");
  IL2_RESOLVE(api, method_get_param_count, "il2cpp_method_get_param_count");
  IL2_RESOLVE(api, method_get_param_name, "il2cpp_method_get_param_name");
  IL2_RESOLVE(api, method_get_param, "il2cpp_method_get_param");
  IL2_RESOLVE(api, method_get_token, "il2cpp_method_get_token");
  IL2_RESOLVE(api, method_get_flags, "il2cpp_method_get_flags");
  IL2_RESOLVE(api, method_get_iflags, "il2cpp_method_get_iflags");
  IL2_RESOLVE(api, method_is_static, "il2cpp_method_is_static");
  IL2_RESOLVE(api, method_is_instance, "il2cpp_method_is_instance");
  IL2_RESOLVE(api, method_is_generic, "il2cpp_method_is_generic");
  IL2_RESOLVE(api, method_is_inflated, "il2cpp_method_is_inflated");
  IL2_RESOLVE(api, method_get_pointer, "il2cpp_method_get_pointer");
  IL2_RESOLVE(api, method_get_invoker, "il2cpp_method_get_invoker");
  IL2_RESOLVE(api, class_get_properties, "il2cpp_class_get_properties");
  IL2_RESOLVE(api, property_get_name, "il2cpp_property_get_name");
  IL2_RESOLVE(api, property_get_get_method, "il2cpp_property_get_get_method");
  IL2_RESOLVE(api, property_get_set_method, "il2cpp_property_get_set_method");
  IL2_RESOLVE(api, property_get_parent, "il2cpp_property_get_parent");
  IL2_RESOLVE(api, property_get_flags, "il2cpp_property_get_flags");
  IL2_RESOLVE(api, property_get_token, "il2cpp_property_get_token");
  IL2_RESOLVE(api, type_get_name, "il2cpp_type_get_name");
  IL2_RESOLVE(api, type_get_assembly_qualified_name,
              "il2cpp_type_get_assembly_qualified_name");
  IL2_RESOLVE(api, type_get_class_or_element_class,
              "il2cpp_type_get_class_or_element_class");
  IL2_RESOLVE(api, type_get_attrs, "il2cpp_type_get_attrs");
  IL2_RESOLVE(api, type_is_byref, "il2cpp_type_is_byref");
  IL2_RESOLVE(api, type_is_pointer, "il2cpp_type_is_pointer");
  IL2_RESOLVE(api, type_is_static, "il2cpp_type_is_static");
  IL2_RESOLVE(api, domain_assembly_open, "il2cpp_domain_assembly_open");
  IL2_RESOLVE(api, get_corlib, "il2cpp_get_corlib");
  IL2_RESOLVE(api, assembly_load, "il2cpp_assembly_load");

  // Core symbols must be present for the dump to work.
  return api.domain_get_assemblies != nullptr &&
         api.assembly_get_image != nullptr &&
         api.image_get_class_count != nullptr &&
         api.image_get_class != nullptr && api.class_get_name != nullptr &&
         api.class_get_fields != nullptr && api.field_get_name != nullptr &&
         api.field_get_offset != nullptr && api.class_get_methods != nullptr &&
         api.method_get_name != nullptr;
}

} // namespace il2
