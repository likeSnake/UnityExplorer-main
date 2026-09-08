#pragma once

#include <cstdint>

namespace er2 {

/// <summary>
/// Unity Object and IL2CPP class offsets for memory reading.
/// Values may need calibration based on Unity version.
/// </summary>
struct Offsets {
  std::uint32_t ms_id_to_pointer_rva = 0;

  std::uint32_t game_object_name_ptr = 0x60;

  std::uint32_t scriptable_object_name_ptr = 0x38;

  std::uint32_t unity_object_instance_id = 0x08;

  std::uint32_t unity_object_managed_ptr = 0x28;

  // Il2CppManagedObjectHeader.klass = +0x00
  std::uint32_t managed_object_klass = 0x00;

  std::uint32_t il2cppclass_name_ptr = 0x10;

  std::uint32_t il2cppclass_namespace_ptr = 0x18;

  std::uint32_t il2cppclass_parent = 0x58;

  // Mono Offsets
  std::uint32_t mono_class_name = 0x48;
  std::uint32_t mono_class_namespace = 0x50;
  std::uint32_t mono_class_parent = 0x30;

  // ========== Il2CppClass Field Analysis Offsets这是我自己添加的，可能弃用
  // ========== Il2CppClass -> FieldInfo* fields (指向字段数组的指针)
  std::uint32_t il2cppclass_fields = 0x80;
  // Il2CppClass -> void* static_fields (指向静态数据区域的指针)
  std::uint32_t il2cppclass_static_fields = 0xB8;
  // Il2CppClass -> uint16_t field_count (字段数量)
  std::uint32_t il2cppclass_field_count = 0x11C;
  // Il2CppClass -> Il2CppType* byval_arg (用于获取类型信息)
  std::uint32_t il2cppclass_byval_arg = 0x40;

  // ========== Il2CppFieldInfo Offsets ==========
  // sizeof(Il2CppFieldInfo) - 字段信息结构体大小
  std::uint32_t il2cpp_field_info_size = 0x20;
  // FieldInfo -> const char* name
  std::uint32_t il2cpp_field_info_name = 0x10;
  // FieldInfo -> Il2CppType* type
  std::uint32_t il2cpp_field_info_type = 0x08;
  // FieldInfo -> Il2CppClass* parent
  std::uint32_t il2cpp_field_info_parent = 0x10;
  // FieldInfo -> int32_t offset (字段在对象/静态区域中的偏移)
  std::uint32_t il2cpp_field_info_offset = 0x18;

  // ========== Il2CppType Offsets ==========
  // Il2CppType -> attrs (包含 FIELD_ATTRIBUTE_STATIC 等标志)
  std::uint32_t il2cpp_type_attrs = 0x06; // uint16_t, 位于 type->data 之后

  // ========== Il2CppClass Method Analysis Offsets (Estimated) ==========
  // Il2CppClass -> MethodInfo* methods (指针数组)
  std::uint32_t il2cppclass_methods = 0x98;
  // Il2CppClass -> uint16_t method_count
  std::uint32_t il2cppclass_method_count = 0x118;

  // ========== Il2CppMethodInfo Offsets ==========
  // MethodInfo -> const char* name
  std::uint32_t il2cpp_method_info_name = 0x10;
  // MethodInfo -> Il2CppClass* parent
  std::uint32_t il2cpp_method_info_class = 0x18;
  // MethodInfo -> Il2CppType* return_type
  std::uint32_t il2cpp_method_info_return_type = 0x20;
  // MethodInfo -> ParameterInfo* parameters
  std::uint32_t il2cpp_method_info_parameters = 0x28; // Not fully verified
};

} // namespace er2
