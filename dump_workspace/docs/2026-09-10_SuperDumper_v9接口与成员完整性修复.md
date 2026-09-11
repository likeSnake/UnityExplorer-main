# SuperDumper v9：接口与成员完整性修复记录

日期：2026-09-10。

## 交付结果

针对 v8 实机检查中发现的接口污染、非法 UTF-8 和每类 4096 个成员截断问题，完成代码修改并构建 Release x64 DLL。离线自检结果为 **253 项通过、0 项失败**。本轮没有注入或操作游戏，实机验证由用户执行。

- DLL：`F:\gua\UnityExplorer-main\bin\Release\SuperDumper.dll`。
- 日志版本：`SuperDumper v9 (接口校验、完整成员遍历与质量诊断)`。
- DLL 大小：529920 字节。
- DLL SHA256：`349D56D9DF41ADE860AB65D6676C450368AC772BB89117D404DC03C3E47534A3`。
- PDB：`F:\gua\UnityExplorer-main\bin\Release\SuperDumper_v9_Release.pdb`。
- 原 v8 DLL 备份：`dump_workspace\validation\v9-quality-selftest\SuperDumper_v8_before.dll`。
- 备份 SHA256：`3E2AB17AC59893875138497FE3952342F4D621DF05081236A4F5851FED6BBCAB`。

没有改写旧实机 dump、旧索引、用户 Offset.h、偏移提取 Python 脚本或现有 cfg。

## 原因与处理

### 接口混入普通类型

v8 校准阶段检查了接口标志，但实际输出阶段没有对每个元素执行相同约束。因此普通闭包类、枚举、结构体，以及解释错误的指针都可能进入正文的接口声明。上轮确认有 35 条未确认关系，其中 20 条可以直接对应到非接口类型。

v9 在输出每条接口关系前检查名称、完整 UTF-8、类型地址对齐、可读标志、有效程序集名称及 `TA_INTERFACE`。不合格项不进入正文和 interfaces.txt，转入成员诊断统计。重复关系按接口 VA 去重，避免把不同命名空间或程序集中的同名接口误合并。中间出现错误条目时，仍检查原始计数范围内后续条目，保留后续合法接口。

同时增加接口计数的扩展校准：分散抽取最多 2048 个类型，依据有效接口数组前缀核对候选计数偏移。候选必须有足够匹配、明确优于当前偏移才允许更新，并在日志中记录原偏移、候选、匹配数与得分。证据不足时保持原偏移；没有硬编码普通体或 Super 的新计数位置。

此机制用于纠正“嵌套类型数量恰好与接口数量相近”等误判。真实 Super 是否因此改变原来的 `0x128`，必须看下一次 `[接口校准]` 日志，不能提前认定具体新偏移。

### 非法 UTF-8 进入文本

此前名称检查放行高位字节，且写入器只处理制表符、换行和反斜杠，没有验证完整 UTF-8。

新增共享编码校验，拒绝孤立续字节、过长序列、代理码点、超出 U+10FFFF 的编码和被截断的序列；保留中文等合法 Unicode 名称。正常输出中的非法名称会被拒绝，文本写入器还提供最后一道保护：异常字节以 `\xNN` 形式保留，不直接写入非法编码，也不静默丢弃。

成员诊断单独保留每组第一个编码错误的索引、地址和原始名称。因此即使该组第一个错误是“非接口”或“所属类错误”，后面遇到的非法名称仍有可追溯证据。日志同样经过编码处理。

还修正了两个相关边界：

- 读取 C 字符串必须真正遇到终止符，不能把读到一半或达到缓冲区容量的名称当作完整名称。
- 正文格式化字符串超过原来的 4096 字节临时缓冲时，按所需长度分配，不再静默截断。

### 每类成员被固定截为 4096

原先 maxFields、maxMethods、maxProps 默认均为 4096，超过后直接缩减循环次数。v9 将这三项默认设为 0，表示使用完整的原始 uint16 计数，最多 65535 项。

遍历仍逐条验证成员记录：数组与记录可读性、名称、所属类型、字段或返回类型结构、属性访问器等。泛型成员允许所属类型共享同一个非零类型定义句柄。遇到坏成员会记录原因并继续检查后续成员，不会因为中间一个坏项而丢失有效尾部。

新增可选 cfg 项：

```ini
maxfields=0
maxmethods=0
maxprops=0
maxifaces=512
```

默认即可使用，无须修改现有 cfg。0 表示按原始计数逐项验证，允许显式指定 1–65535 的上限。接口默认仍保留 512 的保护上限；被上限排除的条目会明确计数。无效配置值会被忽略并给出简体中文提示。

### 可读内存区域边界被误判

检查发现原 MemReader::probe 在请求跨出已缓存区域时直接失败，即使下一段也是可读内存。这会影响跨保护区边界的记录或名称。

现在只查询尚未缓存的后续范围，逐段确认可读并保留缓存。后续区域不可读时仍拒绝读取，实际拷贝继续有 SEH 保护。离线测试覆盖相邻 PAGE_READWRITE/PAGE_READONLY 区域和 PAGE_NOACCESS 边界。

## 新增诊断及完成状态

时间戳正文命名和普通/Super 识别方式保持原工作流程。每次 `_structures` 目录现在包含：

```text
class_meta.tsv
rva_map.tsv
all_bases.txt
interfaces.txt
member_diagnostics.tsv
export_manifest.txt
```

`member_diagnostics.tsv` 记录所有异常成员组，以及超过旧 4096 上限的成员组。一组对应“某个类的字段/方法/属性/接口”之一，状态和原因使用简体中文。主要字段：

| 字段 | 含义 |
| --- | --- |
| ClassId / ClassVA / Class / Category | 与本轮类型表关联及成员类别 |
| RawCount / Attempted / Exported | 原始数量、真正尝试读取的数量、实际输出数量 |
| LimitSkipped / Filtered | 上限未遍历数，以及重复、自身或父类关系过滤数 |
| NullArray / Unreadable | 空成员数组、不可读记录或空成员指针 |
| InvalidName / InvalidUtf8 | 无效或不完整名称、非法 UTF-8 名称 |
| OwnerMismatch / InvalidType | 所属类型不匹配、类型结构校验失败 |
| MissingAccessor / NotInterface | 无可用属性访问器、目标不是有效接口 |
| FirstIndex / FirstAddress / FirstName / FirstReason | 本组第一个拒绝原因和证据 |
| FirstUtf8Index / FirstUtf8Address / FirstUtf8Name | 本组第一个编码错误的独立证据 |

空数组可导致 Attempted=0、NullArray=RawCount。这表示没有可遍历的数组，不能解释为已经逐项读过。配置关闭的成员类别不作为异常。

主日志保留前 40 个异常成员组的详情及总量，避免大量缺失类让日志失控；完整分组在 TSV 中。名称无效而跳过的类另有日志和清单计数。

清单格式版本更新为 2，保留原四个索引的字段结构，并增加诊断文件的行数和字节数。完成状态区分：

- `完成` / `result=OK`：文件写入成功，且本次检查没有发现成员质量异常；仍不代表所有布局都已验证。
- `文件已生成，存在数据质量提示` / `result=OK_WITH_WARNINGS`：文件可使用，但存在拒绝、受限、编码或跳过类型等信息，须看诊断。
- `未完整完成` / `result=FAIL`：写入、关闭、清单提交、扫描或其他必要阶段失败。

## 验证记录

使用 Visual Studio 2022 BuildTools、MSVC v143、Release x64 构建 DumpSelfTest 和 SuperDumper，构建成功。仅有原来已存在的 D9025：/EHs 被 /EHa 覆盖提示。

最终离线自检日志：`dump_workspace\validation\v9-quality-selftest\final\selftest.log`，结果 `pass=253 fail=0`。

- 原 222 项回归全部通过，覆盖标准、变形、Super 扫描、兼容 API、新布局和类型句柄等场景。
- 新增 31 项检查；大型合成类型完整输出 32760 个字段、26374 个方法、5340 个属性，末尾成员均存在。
- 混入普通类、非法名称、重复项和空指针的接口数组，仅输出两条真正接口；错误之后的有效接口被保留。
- 错属字段、非法类型、空数组、缺少属性访问器均按原因准确统计；坏字段之后的有效尾部仍存在。
- 显式恢复 4096 上限时，实际输出受限，未遍历数量准确计入质量状态。
- 构造错误的嵌套类型计数偏移，扩展证据能纠正；只有四个样本时保持旧值。
- 长文本、真实非法字节形式 `NP\x83`、合法中文、跨内存区域读取和不完整字符串均有回归覆盖。
- 含质量问题时，清单不再显示无条件“完成”。原有文件写入与清单提交失败用例继续通过。

另用独立 Python 检查最终 10 个已完成输出目录的 60 个结构文件：全部严格 UTF-8 解码成功；TSV 列数、ClassId 关联、清单实际行数和字节数一致；诊断中原始数与导出、过滤、拒绝、上限数相符。索引开启和关闭的合成正文逐字节一致。故意锁定清单的失败目录保持“正在生成”，按预期处理。

`git diff --check` 通过。

## 实机复核与当前边界

运行上述新版 DLL，沿用现有方式和 cfg。确认 log.txt 首行为 v9，并保留本次时间戳 dump、log.txt 和整个同名 `_structures` 目录。

重点看大型类是否超过 4096、interfaces.txt 是否仅有有效接口、严格 UTF-8 读取是否成功，以及 `member_diagnostics.tsv` 中 IDMAP0–IDMAP4 和其他缺失类的具体原因。接口计数是否调整以实际 `[接口校准]` 证据为准。

本次修复了已确认的程序过滤、编码、遍历上限和区域边界问题。对于校准位置读到空成员数组的类，当前会记录该位置的空指针证据；究竟是未初始化、局部布局不符还是其他原因，仍需进一步证据。不能从旧 dump 凭空恢复成员，也没有调用内部类初始化函数来改变现场。IDMAP 等实际缺失能恢复多少，仍需下一轮实机输出确认；不能预先承诺原始计数全部变为有效成员。

实例大小布局、合成参数名、未解决的偏移摘要项等仍按既有状态标注，本轮没有将这些未知信息改为已验证。

## 修改文件

- `App/Il2CppRuntimeDumper/dump_quality.hpp`：共享 UTF-8 校验、转义及成员诊断数据结构。
- `App/Il2CppRuntimeDumper/il2cpp_dump_core.hpp`：接口验证和计数复核、成员遍历及诊断、字符串与内存边界修正。
- `App/Il2CppRuntimeDumper/structure_indexes.hpp`：诊断导出、编码保护和带数据质量状态的清单。
- `App/Il2CppRuntimeDumper/SuperDumper.cpp`：v9 版本、成员上限配置、OK_WITH_WARNINGS。
- `App/Il2CppRuntimeDumper/SuperDumper.vcxproj`、`DumpSelfTest.vcxproj`：登记新头文件及 v9 PDB。
- `App/Il2CppRuntimeDumper/DumpSelfTest.cpp`：真实问题对应的回归场景。
