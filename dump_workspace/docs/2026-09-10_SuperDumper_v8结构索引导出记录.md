# SuperDumper v8：第一阶段结构索引导出

日期：2026-09-10。

## 本次完成范围

根据 dump_all 分析后的计划，完成第一阶段：在现有时间戳 dump.cs 外，增加类型元数据、方法地址、全部导出类的指针槽位候选、接口关系四种索引，以及简体中文产物清单。

Release x64 DLL 已构建；离线自检 222 项通过、0 项失败。尚未注入目标进程，实机验证由用户进行。配置表、枚举常量值、完整虚函数表和模块内存镜像属于后续阶段，本次未实现。

## 交付

- DLL：`F:\gua\UnityExplorer-main\bin\Release\SuperDumper.dll`
- 版本标记：`SuperDumper v8 (结构索引导出)`。
- 大小：504320 字节。
- SHA256：`3E2AB17AC59893875138497FE3952342F4D621DF05081236A4F5851FED6BBCAB`。
- PDB：`bin\Release\SuperDumper_v8_Release.pdb`。
- 原 v7.1 DLL 备份：`dump_workspace\validation\v8-structures-selftest\SuperDumper_v7_1_before.dll`。
- 备份 SHA256：`2A9071A711C1CA06A5D999C7CBDD7FFCB75F27D51F18D3C3CB539CF757FCA576`。

没有改动已有的实机 dump、偏移头文件或用户重命名后的 Python 提取脚本。

## 运行后的文件

输出目录仍从原 cfg 读取。示例：

```text
输出目录/
  log.txt
  dump_20260910_173000_123.cs
  dump_20260910_173000_123_structures/
    class_meta.tsv
    rva_map.tsv
    all_bases.txt
    interfaces.txt
    export_manifest.txt
```

索引目录从本次 dump 文件名派生。同名索引目录已存在时，自动使用 `_2`、`_3` 等后缀，避免覆盖或混入前一轮文件。

SuperDumper 默认开启。需要关闭时，可在已有 cfg 中加入：

```ini
structureindexes=0
```

共享核心的默认选项仍为关闭；本次由 SuperDumper 入口开启，不会自动改变其他使用共享核心的工具的产物种类。

## 文件含义与可靠性

| 文件 | 主要信息 | 解释 |
|---|---|---|
| class_meta.tsv | ClassId、程序集、命名空间、类名、正文类型索引、ClassVA、Flags、类型种类、父类、成员计数 | Count 列来自实际成功写入正文的成员数，RawCount 列记录校准位置的原始计数 |
| rva_map.tsv | RVA、所属类、方法名、完整显示签名、MethodInfoVA、方法 VA、Token、Flags、Slot、地址状态 | 模块内地址按 RVA 排序；重载和共享地址全部保留；模块外地址和空指针不伪造 RVA |
| all_bases.txt | 指针槽 RVA/VA、对应类、所属模块、页面保护、是否被原正文选用、候选状态 | 扩展到本次导出的全部类，保留同一类多个匹配；未命中也有状态行 |
| interfaces.txt | 类与接口的逐条关系、原接口数组下标、接口所属程序集与命名空间、接口 ClassVA、状态 | 与正文接受的接口条目一致，并额外标注接口标志是否匹配 |
| export_manifest.txt | 中文完成状态、模块与变体、对应 dump、文件行数和字节数、扫描结果、解释与布局线索 | 最终清单只有在写入和关闭成功后才替换初始的“正在生成”标记 |

四个索引均为制表符分隔的 UTF-8 BOM 文本；all_bases.txt 和 interfaces.txt 也可按 TSV 读取。机器字段名保持固定英文标识，状态说明使用简体中文。单元格中的反斜杠、制表符和换行有明确转义规则。

ClassId 是本轮产物内的唯一关联键，可区分同名类、不同程序集的类以及局部类型索引重复的情况。它不是跨版本稳定 ID。DumpTypeDefIndex 使用正文已有的索引，不额外宣称该索引已被全局元数据验证。

## 刻意保留的未确认信息

### 实例大小

当前核心的 cInstanceSize 仍是默认位置，尚未具备独立的实例大小布局校准。新类型表的 `InstanceSize` 因此留空，`InstanceSizeStatus` 标为“布局未验证，仅保留原始读数”或“不可读”。

`RawInstanceSize` 和 `RawInstanceSizeOffset` 只保留现场线索，不能直接用于对象大小、数组元素大小或泛型容器步长计算。这样避免重复参考 class_meta.tsv 中“输出一个数值就当成有效结果”的问题。

### 类型指针槽位

本次核对源码确认，原 discoverClassPointerSlots 主要扫描少量指定目标，并不覆盖所有类。因此新增独立扫描，针对本次实际写出正文的类，在目标模块内已提交、可读的非执行数据页查找对应指针值。

扫描保留所有匹配，并标注“指针值匹配，槽位用途待确认”。单个匹配只证明该地址保存着类指针，不证明它一定是预期 TypeInfo 全局变量。

`SelectedByDump=1` 只表示原正文使用过该槽位，不等于已验证单例链。本次全量候选不会反向覆盖原来的经理类发现结果或顶部偏移摘要。

不可读页、保护页和执行页不在扫描范围内；扫描覆盖的是该模块可读的非执行数据页，不承诺扫描整个进程。读取失败会计数并导致索引阶段返回未完整完成，未命中项不会填零冒充成功。

### 方法与接口

模块外的方法指针仅保留运行时 VA，RVA 列为空。空方法指针也有独立状态，均保留在方法表中，便于与正文方法总数核对。

MethodInfo 中的 Slot 是声明槽号，不是虚函数表字节偏移。本次还没有生成完整 vtables.txt。

接口关系复用正文的解析和过滤结果；额外的状态列记录接口标志验证结果，不把未确认标志的条目称为完全验证。

## 同时修正的变体识别问题

此前入口实际使用：

```cpp
IsSuperVariant(mod) || ModuleHasVmpSections(mod.base)
```

参考普通 GameAssembly.dll 本身也存在 .vmp 节。这意味着即使模块名明确是 GameAssembly.dll，仍可能被错误标为 Super，继而影响入口路线、变体偏移摘要和新增清单中的模块身份。

本次修改为：明确的普通 GameAssembly.dll 保持普通变体身份；Super 名称按 Super 处理；不明确的发现结果才允许用节区特征辅助判断。先前分析中“已经优先按模块身份判断”的描述不够完整，本次以入口实际代码为准纠正。

## 写入与资源处理

- 新索引使用独立缓冲写入器，检查短写及 fclose 失败。
- 初始 export_manifest.txt 标记“正在生成”。最终清单先完整写入 export_manifest.pending，关闭成功后再替换；失败时保留未完成标记。
- 主 dump 的 Writer 补充短写和关闭失败检测，防止截断后误报成功。
- 方法签名在正文生成时收集，用于方法索引；排序写出后立即释放签名集合，避免一直占用内存。
- Dumper 在卸载 DLL 前离开作用域，释放索引和游走容器、关闭文件。

新增索引会增加排序内存与文件 I/O；当前只做了离线正确性验证，实际约 50 万方法时的耗时和内存峰值仍需用户实机日志确认。

## 验证

使用 Visual Studio 2022 BuildTools / MSVC v143，Release x64。

```powershell
& 'C:\Program Files (x86)\Microsoft Visual Studio\2022\BuildTools\MSBuild\Current\Bin\MSBuild.exe' `
  'App\Il2CppRuntimeDumper\DumpSelfTest.vcxproj' /p:Configuration=Release /p:Platform=x64 /m /nologo /verbosity:minimal

& 'C:\Program Files (x86)\Microsoft Visual Studio\2022\BuildTools\MSBuild\Current\Bin\MSBuild.exe' `
  'App\Il2CppRuntimeDumper\SuperDumper.vcxproj' /p:Configuration=Release /p:Platform=x64 /m /nologo /verbosity:minimal
```

构建成功；只有原有的 D9025：/EHs 被 /EHa 覆盖提示。

离线测试输出目录：`dump_workspace\validation\v8-structures-selftest`。

- 总结果：`pass=222 fail=0`。
- 覆盖标准、变形、Super 扫描、Super 兼容 API、新布局、metadata type handle 六组现有场景。
- 新增检查：索引列对齐；实际成员计数；类、方法、接口与指针槽的关联；RVA 排序；未确认实例大小标记。
- 专门验证同名类、重载、共享 RVA、模块外指针、同一类多个槽位。
- 同一份合成内存分别关闭和开启索引，生成的 dump 正文逐字节相同。
- 验证方法/字段开关与 maxclasses 限制生效；目录冲突时已有索引不覆盖。
- 锁定初始清单，模拟最终替换失败：正确返回失败，原清单仍为“正在生成”。测试日志内对应的“未完整完成”是故意构造的失败场景。
- 独立复核 9 个已完成输出目录，文件编码、TSV 列数、真实行数和字节数全部与清单一致。
- `git diff --check` 通过。

本次没有运行偏移提取脚本的旧测试入口，因为用户已将脚本重命名为 YJWJOffset_改良.py，而旧测试固定引用 YJWJOffset_new.py。本轮未修改提取逻辑；与其相关的保护来自上述“索引开关前后正文逐字节一致”检查。

## 实机检查方式

使用新 SuperDumper.dll，沿用原 cfg 和运行方式。运行后查看 log.txt 中的 v8 标记及 `[索引]` 信息，并找到与新时间戳 dump 对应的 `_structures` 目录。

重点检查 export_manifest.txt：状态应为“完成”、正文完成状态应为“成功”、与正文计数核对应为“一致”。保留新 dump.cs、log.txt 和对应索引目录，以便后续核对真实产物。

## 修改文件

- `App/Il2CppRuntimeDumper/structure_indexes.hpp`：新增独立导出器。
- `App/Il2CppRuntimeDumper/il2cpp_dump_core.hpp`：收集正文解析结果、输出索引、检测主文件写入失败。
- `App/Il2CppRuntimeDumper/SuperDumper.cpp`：默认开启索引、增加 cfg 开关、修正普通模块身份、更新版本并释放资源。
- `App/Il2CppRuntimeDumper/SuperDumper.vcxproj`：登记新头文件及 v8 PDB。
- `App/Il2CppRuntimeDumper/DumpSelfTest.cpp`、`DumpSelfTest.vcxproj`：新增输出一致性和失败场景检查。
