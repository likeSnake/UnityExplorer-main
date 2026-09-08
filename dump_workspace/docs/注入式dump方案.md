# 自研注入式 IL2CPP Dump 方案（Il2CppRuntimeDumper）

## 目标

实现与 Unity-Offset.dll 同机制的**注入式 IL2CPP 运行时 dump 器**，完全自研源码，
可脱离对 Unity-Offset.dll 二进制的依赖，产出完整 dump.cs（含字段偏移、方法 RVA/VA、属性）。

## 机制（与 Unity-Offset.dll 完全一致）

```
注入 DLL 到游戏进程
  -> DllMain 触发 DumpWorker 线程
  -> GetModuleHandle/EnumProcessModulesEx 定位 GameAssembly*.dll
  -> GetProcAddress 解析全部 il2cpp_* 导出（约 80 个 API）
  -> il2cpp_domain_get_assemblies 遍历程序集
  -> il2cpp_assembly_get_image 取镜像
  -> il2cpp_image_get_class_count / il2cpp_image_get_class 遍历类
  -> il2cpp_class_get_fields / il2cpp_class_get_methods / il2cpp_class_get_properties
  -> 字段：il2cpp_field_get_name / _get_offset / _get_type
  -> 方法：il2cpp_method_get_name / _get_return_type / _get_param_count / _get_pointer(RVA)
  -> 属性：il2cpp_property_get_name / get_/set_ 方法
  -> 写出 dump.cs（标准 Il2CppDumper 风格：TypeDefIndex / 字段偏移 / RVA / VA）
```

**为什么能比 DMA 完整**：注入后在进程内直接调用 il2cpp 运行时 API，
metadata 由运行时持有和遍历，无需全局魔数扫描（DMA 在 Super/普通都扫不到 0xFAB11BAF）。

**为什么 Super 变体不行**（与 Unity-Offset.dll 相同的限制）：
Super 的 GameAssembly 被 VMProtect 保护，il2cpp_* 导出被移除/加密，
GetProcAddress 无法解析 → dump 失败。**普通 GameAssembly.dll 裸导出全部 il2cpp API，可 dump。**

## 产物

| 文件 | 说明 |
| --- | --- |
| `App/Il2CppRuntimeDumper/il2cpp_api.hpp` | il2cpp API 解析层（进程内 GetProcAddress） |
| `App/Il2CppRuntimeDumper/runtime_dumper.cpp` | 类型系统遍历 + dump.cs 生成 |
| `App/Il2CppRuntimeDumper/dllmain.cpp` | 注入入口（DllMain + 重试等待域就绪） |
| `App/Il2CppRuntimeDumper/injector.cpp` | 最小注入器（CreateRemoteThread+LoadLibraryA） |
| `bin/Release/Il2CppRuntimeDumper.dll` | 编译产物（298KB，无导出，注入式） |
| `bin/Release/Injector.exe` | 注入器 |

## 用法

```powershell
# 1. 启动游戏（普通变体）
# 2. 注入（配置输出目录见 Il2CppRuntimeDumper.cfg，默认 C:\YJDumped）
Injector.exe NarakaBladepoint.exe <path>\Il2CppRuntimeDumper.dll
# 3. 等待几秒，检查输出目录 dump.cs + log.txt
```

配置文件 `Il2CppRuntimeDumper.cfg`（放在 DLL 同目录，第一行 = 输出目录）。

## 输出格式（对齐 Il2CppDumper 风格）

```csharp
// Image 0: Assembly-CSharp.dll (42893 classes)

// Namespace: 
public class <Module> // TypeDefIndex: 0
{
    // Fields
    public int m_Value; // 0x10

    // Properties
    public int Value { get; set; }

    // Methods:
    // RVA: 0x3C1010 VA: 0x7FFD12901010
    public void SetValue(int arg0);
}
```

## 待验证

- [ ] 普通变体注入测试（需要游戏运行 + 注入器可注入，注意反作弊）
- [ ] 与 Unity-Offset.dll 输出 dump.cs 对比（类数/字段数/方法数）
- [ ] 方法参数类型/名称完整性验证
- [ ] 若反作弊拦截 CreateRemoteThread，需用用户的注入通道

## 关键差异表

| 维度 | Unity-Offset.dll | 自研 Il2CppRuntimeDumper |
| --- | --- | --- |
| 源码 | 无（闭源） | ✅ 全自研 |
| 机制 | il2cpp API 注入遍历 | ✅ 相同 |
| 输出目录 | C:\YJDumped\dump.cs | 可配置（cfg） |
| 字段偏移 | ✅ | ✅ |
| 方法 RVA/VA | ✅ | ✅ |
| 属性 | ✅ | ✅ |
| 类型别名 | T-857777416 等（部分原始） | C# 别名（int/string/bool）|
| 普通变体 | ✅ | ✅ 待实测 |
| Super 变体 | ❌ | ❌（相同限制）|
