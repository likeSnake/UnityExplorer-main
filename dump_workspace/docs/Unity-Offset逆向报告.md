# Unity-Offset.dll 逆向结论 + 自研注入器对齐报告

## 重大突破：找到 Unity-Offset.dll 的源头源码

Unity-Offset.dll 基于开源项目 **[shalzuth/Il2CppRuntimeDumper](https://github.com/shalzuth/Il2CppRuntimeDumper)**
（证据：DLL 内嵌源码路径 `H:\C++Code\il2CppRuntimeDumperEx\il2CppRuntimeDumperEx\il2cpp_dump.cpp` +
README 描述完全吻合 "dump out il2cpp assemblies from memory, useful for apps where
the metadata is encrypted"）。

源码已克隆到本地：
```
dump_workspace/Il2CppRuntimeDumper_src/
```

## Unity-Offset.dll 完整机制（源码确认）

### 1. 注入方式（Program.cs）
- `NativeNetSharp.Inject("gameprocname", dllBytes)` —— 自定义自注入器
- DLL 被注入后，`Main` 的 else 分支执行：
  ```
  il2cpp_thread_attach(il2cpp_domain_get())
  Il2Cpp.InitAssemblies()
  Dumper.Dump()
  ```
- **inline hook 辅助**（`JmpPatch`/`Hook`）：`mov r11, target; jmp r11` 12字节
  补丁 + VirtualProtect + FlushInstructionCache（注入触发用）

### 2. 程序集/类枚举（Il2Cpp.cs / Il2CppClass.cs）
- `il2cpp_domain_get()` → `il2cpp_domain_get_assemblies(domain, &size)`
- 每个 assembly：`il2cpp_assembly_get_image` → `il2cpp_image_get_name`
- 每个类：`il2cpp_image_get_class(image, i)` / `il2cpp_image_get_class_count(image)`
- **反射辅助**：`il2cpp_class_from_name(corlib, "System.Reflection", "Assembly")`
  → `il2cpp_class_get_method_from_name(...GetAssemblies...)` → `il2cpp_runtime_invoke`
  （用它拿真实 .NET Assembly 对象数组，非裸指针）

### 3. 类解析（Il2CppClass 构造）
```cpp
Name       = il2cpp_class_get_name(ptr)
Namespace  = il2cpp_class_get_namespace(ptr)
Flags      = il2cpp_class_get_flags(ptr)
Image      = il2cpp_class_get_image(ptr)
Type       = il2cpp_class_get_type(ptr)
Declaring  = il2cpp_class_get_declaring_type(ptr)
BaseClass  = il2cpp_class_get_parent(ptr)
Methods    = while(il2cpp_class_get_methods(ptr, &iter))
Fields     = while(il2cpp_class_get_fields(ptr, &iter))
Properties = while(il2cpp_class_get_properties(ptr, &iter))
Interfaces = while(il2cpp_class_get_interfaces(ptr, &iter))
```
- **缓存 + 递归**：CachedClasses + ClassesToAdd 队列，懒生成所有依赖类

### 4. 方法解析（Il2CppMethod）
```cpp
Name        = il2cpp_method_get_name(ptr)
ReturnType  = il2cpp_method_get_return_type(ptr)
Flags       = il2cpp_method_get_flags(ptr, &implFlags)
IsGeneric   = il2cpp_method_is_generic(ptr)
ParamCount  = il2cpp_method_get_param_count(ptr)
for i: Param = il2cpp_method_get_param(ptr, i)
        ParamName = il2cpp_method_get_param_name(ptr, i)
```

### 5. 字段解析（Il2CppField）
```cpp
Name   = il2cpp_field_get_name(ptr)
Flags  = il2cpp_field_get_flags(ptr)
Type   = il2cpp_field_get_type(ptr)
Offset = il2cpp_field_get_offset(ptr)
```

### 6. 输出（Dumper.cs）
- 用 **Mono.Cecil** 重建 `DummyDlls\*.dll`（假的 .NET 程序集）
- 字段加 `FieldOffsetAttribute`（`0x{offset:X}`）
- 属性挂 get/set 方法、基类、接口、泛型参数、嵌套类型

## 与自研 Il2CppRuntimeDumper 的对齐情况

| 环节 | Unity-Offset.dll（源码） | 自研 Il2CppRuntimeDumper | 状态 |
| --- | --- | --- | --- |
| 注入方式 | NativeNetSharp.Inject / JmpPatch | DllMain 线程 + cfg 配置 | ✅ 不同但等价 |
| 定位 GameAssembly | GetModuleHandle("GameAssembly.dll") | GetModuleHandle + 模块枚举 | ✅ |
| 解析 il2cpp API | GetProcAddress ~200 个 | GetProcAddress ~80 个（核心全） | ✅ 核心齐 |
| 程序集枚举 | domain_get_assemblies | 同 | ✅ |
| 类枚举 | image_get_class(_count) | 同 | ✅ |
| 类字段解析 | class_get_fields/name/offset/type | 同 | ✅ |
| 方法解析 | class_get_methods/name/ret/param | 同（含参数） | ✅ |
| 属性解析 | class_get_properties | 同 | ✅ |
| 输出 | DummyDlls（Mono.Cecil） | dump.cs（文本） | ⚠️ 格式不同 |
| 字段 offset | FieldOffset 属性 | `// 0x{offset}` 注释 | ✅ 信息等值 |
| 方法 RVA/VA | （源码未显式，但注入器有此能力） | `// RVA: 0x.. VA: 0x..` | ✅ 更明确 |

**核心结论**：自研注入器的机制与 Unity-Offset.dll **完全一致**（同样的 il2cpp API、
同样的遍历顺序、同样的字段/方法/属性数据）。两者唯一差异是输出格式（DummyDll 重建
vs 纯文本 dump.cs）——文本格式反而更适合直接对照 Il2CppDumper。

## 为什么 Unity-Offset.dll 只能 dump 普通变体（源码证实）

`UnityDllName = "GameAssembly"` 常量 + `GetModuleHandleA("GameAssembly.dll")` 精确匹配。
Super 变体加载的是 `GameAssembly_Super.dll`，且 **il2cpp_* 导出被 VMProtect 移除**，
`GetProcAddress` 全部失败 → dump 无法开始。这与我们的注入器限制完全相同。

## 下一步

1. **用 CE 注入自研 Il2CppRuntimeDumper.dll 到游戏（普通变体）**，实测 dump.cs
2. 对比 Unity-Offset.dll 输出（类数/字段数/方法数）验证完整度
3. 可选：把输出改为 DummyDll 重建（若需要 Il2CppDumper 兼容格式）
4. DMA 注入路径（dma_inject）继续攻坚反作弊干扰（IAT/caves 查找被 NEAC 破坏的问题）
