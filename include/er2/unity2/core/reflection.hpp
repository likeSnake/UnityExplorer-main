#pragma once

#include "../../mem/memory_accessor.hpp"
#include "../../mem/memory_read.hpp" // For ReadCString
#include "../init.hpp"               // For g_ctx
#include "offsets.hpp"
#include <string>
#include <vector>

namespace er2::unity2::core {

struct ReflectionFieldInfo {
  std::string name;
  int32_t offset;
  uintptr_t typePtr; // Il2CppType*
};

struct ReflectionMethodInfo {
  std::string name;
  uintptr_t methodPtr;
};

// 获取类的字段列表
inline bool GetClassFields(const IMemoryAccessor &mem, uintptr_t klass,
                           std::vector<ReflectionFieldInfo> &outFields) {
  outFields.clear();
  if (!klass)
    return false;

  // 1. 获取字段数量
  uint16_t fieldCount = 0;
  if (!ReadValue(mem, klass + g_ctx.off.il2cppclass_field_count, fieldCount)) {
    return false;
  }

  if (fieldCount == 0)
    return true; // 无字段

  // 2. 获取字段数组起始地址
  uintptr_t fieldsPtr = 0;
  if (!ReadValue(mem, klass + g_ctx.off.il2cppclass_fields, fieldsPtr)) {
    return false;
  }
  if (!fieldsPtr)
    return true;

  // 3. 遍历字段
  for (int i = 0; i < fieldCount; ++i) {
    uintptr_t fieldAddr = fieldsPtr + (i * g_ctx.off.il2cpp_field_info_size);

    ReflectionFieldInfo info;
    // 读取 Offset
    if (!ReadValue(mem, fieldAddr + g_ctx.off.il2cpp_field_info_offset,
                   info.offset)) {
      continue;
    }

    // 读取 Name 指针
    uintptr_t namePtr = 0;
    if (ReadValue(mem, fieldAddr + g_ctx.off.il2cpp_field_info_name, namePtr)) {
      ReadCString(mem, namePtr, info.name);
    }

    // 读取 Type 指针
    ReadValue(mem, fieldAddr + g_ctx.off.il2cpp_field_info_type, info.typePtr);

    outFields.push_back(info);
  }

  return true;
}

// 获取类的方法列表 (Experimental)
inline bool GetClassMethods(const IMemoryAccessor &mem, uintptr_t klass,
                            std::vector<ReflectionMethodInfo> &outMethods) {
  outMethods.clear();
  if (!klass)
    return false;

  // 1. 获取方法数量
  uint16_t methodCount = 0;
  if (!ReadValue(mem, klass + g_ctx.off.il2cppclass_method_count,
                 methodCount)) {
    return false;
  }

  if (methodCount > 2000) {

  // Sanity check: 方法数量通常不会非常巨大，如果读出 60000+ 可能是偏移错误
  if (methodCount > 2000) {
    // Log warning?
    return false;
  }
  if (methodCount == 0)
    return true;

  // 2. 获取方法数组指针 (MethodInfo**)
  uintptr_t methodsPtr = 0;
  if (!ReadValue(mem, klass + g_ctx.off.il2cppclass_methods, methodsPtr)) {
    return false;
  }
  if (!methodsPtr)
    return true;

  // 3. 遍历方法指针数组
  for (int i = 0; i < methodCount; ++i) {
    uintptr_t methodPtr = 0;
    // 读取 MethodInfo*
    if (!ReadValue(mem, methodsPtr + (i * sizeof(uintptr_t)), methodPtr)) {
      continue;
    }
    if (!methodPtr)
      continue;

    ReflectionMethodInfo info;
    info.methodPtr = methodPtr;

    // 读取 Name
    uintptr_t namePtr = 0;
    if (ReadValue(mem, methodPtr + g_ctx.off.il2cpp_method_info_name,
                  namePtr)) {
      ReadCString(mem, namePtr, info.name);
    }

    if (!info.name.empty()) {
      outMethods.push_back(info);
    }
  }

  return true;
}

} // namespace er2::unity2::core
