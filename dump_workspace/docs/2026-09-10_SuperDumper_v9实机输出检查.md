# SuperDumper v9 实机输出检查

日期：2026-09-10。用户已运行 v9，本轮检查真实产物并追查诊断证据，没有修改 Dumper 源码、重建 DLL 或操作目标进程。

## 结论

v9 的成员上限修复和编码保护已在实机生效，正文及结构索引对应一致。大型类型原先在 4096 项停止的问题已消除，文件没有写入截断，非法字节没有再污染正常产物。

但仍不能标为完整成功。新诊断揭示了此前遗漏的两个程序问题：合法 Protobuf 成员名称被过严规则过滤；接口解析仍强烈呈现数组布局或步长不匹配。IDMAP 等类在当前校准位置仍读到空字段数组。日志 `OK_WITH_WARNINGS` 正确反映了这些限制。

## 检查对象

根目录：`F:\gua\UnityExplorer-main\dump_workspace\inject_test\out_super`。

- 正文：`dump_20260910_190110_294.cs`。
- 日志：`log.txt`，版本为 `SuperDumper v9 (接口校验、完整成员遍历与质量诊断)`。
- 结构目录：`dump_20260910_190110_294_structures`。
- 模块：`GameAssembly_Super.dll`，基址 `0x7FF93E0A0000`，大小 `0x1A43E000`，PID 10356。
- 运行约 19:01:10 至 19:01:29；镜像扫描日志耗时 15672 ms，共 123 个程序集镜像。
- 正文 SHA256：`339FE94D2B3D4876CAE7ACD004071D57642F5E28CD4591CDBEA60174AEFD3309`。

当前输出目录已没有上轮 v8 文件。因此下列 v8 对照使用上轮已保存检查报告中的计数，本轮没有进行 v8 与 v9 的逐字节新旧比较。

## 已通过的实机检查

### 导出数量

| 内容 | v8 已记录数量 | 本次 v9 | 变化 |
| --- | ---: | ---: | ---: |
| 正文字节数 | 76915609 | 83000398 | +6084789 |
| 类型 | 56495 | 56495 | 0 |
| 字段 | 269855 | 297979 | +28124 |
| 方法 | 497361 | 532497 | +35136 |
| 属性 | 80991 | 82235 | +1244 |
| 接口关系 | 6952 | 6913 | -39 |

正文为 1987394 行，约 83.0 MB。不能单凭大小判定完整性，也不能将所有数量增长都归因于单一修复，因为两次运行的类初始化状态可能不同。

以下大型类型已超过 4096，实际导出数量与本轮对应 RawCount 相等：

| 类型 | 类别 | 实际导出 |
| --- | --- | ---: |
| MessageFmtDefine | 字段 | 4171 |
| ActDef | 字段 | 24027 |
| ProtoFactoryPb | 字段 | 6689 |
| LXAllShaderPropertyID | 字段 | 8491 |
| ObjectTranslator | 方法 | 26374 |
| ILFixDynamicMethodWrapper（Assembly-CSharp） | 方法 | 15171 |
| ReaderProtoGameConfigArchive | 方法 | 5360 |
| VivoxCoreInstancePINVOKE | 方法 | 4615 |
| ReaderProtoGameConfigArchive | 属性 | 5340 |

日志和所有诊断行的数量上限跳过数均为 0。上述四类方法增加总数恰为 35136，与总方法数增量相同；属性增量 1244 也与大型属性组恢复数相同。

### 文件与正文一致性

| 文件 | 数据行数 | 字节数 |
| --- | ---: | ---: |
| class_meta.tsv | 56495 | 10617188 |
| rva_map.tsv | 532497 | 102737994 |
| interfaces.txt | 6913 | 1097394 |
| all_bases.txt | 56562 | 8497348 |
| member_diagnostics.tsv | 34568 | 7409607 |
| export_manifest.txt | 产物说明 | 2859 |

全量核对通过：

- 正文和全部结构文件严格 UTF-8 解码成功。正常正文与索引的非法字节转义数为 0；诊断文件保留了 25 个原始证据字节的转义。
- 清单实际行数、字节数与文件一致；TSV 列数正确。
- ClassId、类型 VA 唯一，类型顺序、名称、命名空间、类型索引与正文一致。
- 每类实际字段、方法、属性数与正文一致；全部 532497 个方法按所属类、VA、Slot、完整签名逐项匹配，没有缺失或额外索引项。
- 方法排序与 RVA 算术正确：531367 个模块内方法指针，1130 个空指针，模块外为 0；不同非空 RVA 为 367731 个，8309 个 RVA 被多个方法共享。
- 接口条目与正文已输出的接口声明一致；6913 条均标为“接口标志匹配”。能在类型表中直接关联的 1042 条均为 interface，未再关联到普通 class、enum 或 struct。
- 其余 5871 条接口目标未作为独立类型定义列入 class_meta，可能包括实例化泛型；不能仅凭未关联就判错。
- 指针槽位关联、对齐、范围和 RVA 正确。36738 个匹配覆盖 36671 个类型，19824 个类型未命中，65 个类型有多个候选；正文选用的 8 个槽位均正确对应。
- 诊断行的 RawCount 与导出、过滤、拒绝和数量限制计数相符；类关联正确。

以上只证明文件生成与内部对应关系。接口数组读取方法是否正确、原始计数是否代表完整可用成员，仍须依据后面的语义证据判断。

可复查脚本：`dump_workspace\validation\v9-real-output-check\audit.py`。

机器检查结果：`dump_workspace\validation\v9-real-output-check\verification.json`，`consistency_errors` 为空。

## 仍需修复：名称规则误过滤 Protobuf 成员

`member_diagnostics.tsv:2415` 记录 OpeningTimeResponse：

```text
方法：RawCount=19，Attempted=19，Exported=16，InvalidName=3
首个被拒绝名称：pb::Google.Protobuf.IMessage.get_Descriptor
```

下一行对应属性：

```text
属性：RawCount=4，Attempted=4，Exported=3，InvalidName=1
首个被拒绝名称：pb::Google.Protobuf.IMessage.Descriptor
```

这是可辨认的生成代码接口实现名称，不是乱码。源码 `il2cpp_dump_core.hpp:302` 中的 `NameLooksValid()` 一律拒绝冒号 `:`，因此这些名称会被过滤。

全量统计：5693 个类均出现上述方法首样本和属性首样本，对应 InvalidName 拒绝计数共 17079 个方法、5693 个属性。诊断只保留每组首个一般错误，不能据此列出全部被拒绝方法的精确名字；但冒号过滤造成有效成员缺失已有直接证据。

日志还显示一个类型被跳过：

```text
<PrivateImplementationDetails>{0D533E08-1E7F-4C0B-8CE7-7E465A4986C6}
```

该名称属于可辨认的编译器生成类型形式，当前名称白名单也不允许花括号。这个跳过并不等于目标没有该类型。

这些名称规则在 v8 中已经存在，本次是新增诊断暴露了具体原因，不能将它们说成 v9 新增的回退。后续应将校准用的保守名称启发式与已验证结构的输出名称校验分开，在保持编码、长度和可读性校验的同时保留合法生成名称。

## 仍需修复：接口数组布局或步长不匹配

本轮日志仍使用接口数组偏移 `0xB0`、接口计数偏移 `0x128`。关键新证据如下：

- 全部 6913 条有效接口记录的 ArrayIndex 都是偶数，奇数下标有效项为 0。
- 诊断中的 439 个接口组以不可读条目为首因；其中很多首个失败索引都是 1，对应所谓类地址却是 `0x4`、`0x11`、`0x6`、`0x12` 等小整数。
- ActorModel 的原始接口数为 14，程序尝试 14 个 8 字节位置，只输出 7 条，位于下标 0、2、4、6、8、10、12。其第一个失败“类地址”是 `0x4`（诊断第 649 行）。

这强烈指向代码正在将类似 `Il2CppRuntimeInterfaceOffsetPair { interfaceType, offset }` 的记录数组按纯 `Il2CppClass*[]` 读取：一个位置是接口指针，下一个位置是偏移整数。当前源码 `writeClass()` 明确使用 `ifArr + i * 8`，没有校准接口元素步长。

因此 v9 的逐项校验消除了输出污染，却没有解决上游数组识别问题，并可能漏掉数组后半段。不能因为剩余记录都具有接口标志，就认定接口导出完整。

本轮扩展计数校准日志为：500 个样本，原 `0x128` 匹配 261 次、得分 760，候选 `0xE0` 匹配 262 次、得分 810。候选只多一个精确匹配，未满足更新条件，因此保留原偏移。这证明“只复核计数”不足以解决当前数组结构问题；不能据此将 `0xE0` 当作正确计数位置。

下一步需要同时核对接口数组位置、8/16 字节元素结构、计数来源和完整范围。当前只有产物及有限证据，尚不能把具体替代偏移或所有类型的真实接口数量定为最终结论。

## 空数组与缺失统计

异常成员组 34559 个；诊断总行数 34568，其中额外 9 行是超过旧上限且完整遍历成功的大型成员组。拒绝计数共 335831，上限跳过为 0。

| 类别 | 空数组对应原始条目 | 名称无效 | 其他拒绝 |
| --- | ---: | ---: | ---: |
| 字段 | 252677 | 0 | 0 |
| 方法 | 45872 | 17079 | 0 |
| 属性 | 5958 | 5693 | 0 |
| 接口 | 7555 | 81 | 771 不可读、25 非法 UTF-8、120 非接口 |

这里的“拒绝条目”是对原始读数和当前遍历结果的统计，不等于已经确认存在 335831 个有效成员等待恢复。特别是接口布局仍有疑点，其原始计数不能直接作为真实缺失量。

Assembly-CSharp 中 IDMAP0–IDMAP3 的 RawFieldCount 各为 32760，IDMAP4 为 18661；五者 Attempted 和 Exported 均为 0，NullArray 等于原始数。诊断第 27192 行给出 IDMAP0 的字段数组槽地址 `0x1C5BB9B5D50`，相对其 ClassVA 为 `+0x80`，说明当前读取位置的数组指针确实为空。

其他程序集还有同名 IDMAP0：Assembly-CSharp-firstpass 原始字段数 2034，CommonUtils 为 11，Localization 为 3，均记录为空数组。不能仅按类名将它们合并。

空指针原因可能是尚未初始化、局部布局差异或其他运行时机制，本轮没有读取现场内存来判定，也没有通过修改现场来强制初始化。

## 用户关心的偏移及类型

ActorKit、ActorKitInteractive、PlayableState、RuntimeLogicLayer、TransparentTagHandler、GameBaseObject、BuffManager 的本轮字段、方法、属性实际输出数均与对应 RawCount 相等。这不包含前述接口数量问题。

顶部偏移摘要仍为 221 项，状态分布与 v8 检查记录相同：FOUND=156、UNRESOLVED=31、NOT_LOADED=10、CALIBRATED=8、UNRESOLVED_LAYOUT=4、RUNTIME_VA=3、NOT_FOUND=2、SKIPPED_LEGACY=2，其余 CONFIG_CONSTANT、VERIFIED、CANDIDATE、DERIVED、ABI_LAYOUT 各 1。

- BuffManager2 类槽 RVA=`0x374AE20`，VERIFIED，实例回指确认。
- TransparentTagHandler2 类槽 RVA=`0x376ABF8`，CLASS_ONLY，实例仍为 0。
- static_fields=`0xB8`。

合成参数名、未验证实例大小、候选指针槽用途和未解决偏移项等既有边界继续保留。当前产物可用于已输出成员与偏移分析，但不应作为“全部元数据完整无误”的最终基准。
