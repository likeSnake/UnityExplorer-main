# SuperDumper v7 实机输出检查

## 结论

用户运行后的 v7 dump 正常完成，新增顶部汇总已生效；主 dump 的数量指标与上一轮 v6 完全一致，没有发现数量回退。**这证明本轮导出和摘要生成成功，不代表 221 个变量全部已找到或业务对象链全部已验证。** 本次检查没有修改 DLL、业务代码或用户产物。

检查对象（文件时间 2026-09-10 15:18:37）：

- `F:\gua\UnityExplorer-main\dump_workspace\inject_test\out_super\dump.cs`
- `F:\gua\UnityExplorer-main\dump_workspace\inject_test\out_super\log.txt`
- 模块：`GameAssembly_Super.dll`，基址 `0x7FFE8A020000`，大小 `0x1A43E000`。
- 日志确认 `SuperDumper v7`，末行 `[4] result=OK`；没有 `[FAIL]` / `[CRASH]`。
- dump SHA256：`D15E6621EA5D525528F89C201A7E7D57F1E0C8835D4EC9DE04DB0F69DFCFB861`
- log SHA256：`9415653015F019BFEF7571BDA0A5246EFC86D7502308CCA016251A1D449F86C1`

## 主体完整度检查

| 指标 | 当前 v7 | 上轮 v6 |
|---|---:|---:|
| dump 字节数 | 76915609 | 76885309 |
| 导出类 | 56495 | 56495 |
| 字段 | 269855 | 269855 |
| 方法 | 497361 | 497361 |
| 属性 | 80991 | 80991 |
| 接口条目 | 6952 | 6952 |
| 跳过类 | 1 | 1 |
| 空方法指针 | 1126 | 1126 |
| 类型名 fallback | 0 | 0 |

v6 对照来自 `dump_workspace\inject_test\out_super_old4\log.txt`。本轮文件约 73.35 MiB（76.92 MB），1,927,304 行，123 个 Image，比 v6 增加 30300 字节。直接扫描正文的类、字段、属性计数与日志一致；方法有 496235 行非空地址和 1126 行空地址，共 497361，空地址保留原有格式 `// RVA: 0x VA: 0x0`。

枚举总数 56496，输出 56495，另 1 类在名字读取/合法性检查处跳过，与 v6 一致；日志没有记录该类身份，不能进一步断言它是否重要。元数据参数名仍未找到，参数名使用 p0/p1 等替代。这些是现有输出限制，不是本次摘要新增导致的回退。

## 顶部摘要与正文一致性

摘要包含 221 行、221 个不同变量路径，没有漏行或重复路径；第 238 行输出总统计，第 241 行开始 Image 列表。

| 状态分组 | 数量 | 含义 |
|---|---:|---|
| 已解析 | 169 | FOUND 156 + CALIBRATED 8 + VERIFIED 1 + DERIVED 1 + RUNTIME_VA 3 |
| 另一变体未加载 | 10 | 普通组、普通 CrossFade、普通 vtable 项 |
| 候选 | 1 | interactable vtable 字节偏移，语义未完全确认 |
| 常量 | 2 | 配置占位常量和数组 ABI 头部，不属于发现结果 |
| 未确认/未找到/跳过 | 39 | UNRESOLVED 31 + NOT_FOUND 2 + UNRESOLVED_LAYOUT 4 + SKIPPED_LEGACY 2 |

将摘要的全部 **145 条 FOUND 字段变量** 与实际正文按类名、真实字段名、字段类型、静态/实例属性、数值逐条交叉核对，共 141 个不同字段（部分变量复用同一字段），**141/141 匹配**。方法及 TypeInfo 通过目录离线审计核对，报告在 `dump_workspace\validation\v7-offset-catalog-runtime-20260910.md`。

用户特别指定的三组映射全部 FOUND，24/24 的偏移与 Offset.h 相同：ActorKitInteractiveS 10 条、RuntimeLogicLayerS 11 条、PlayableStateS 3 条。

## 本轮关键结果

| 项目 | 结果 | 证据/限制 |
|---|---|---|
| m_BuffManager2 | 0x374AE20 | VERIFIED，实例 klass 回指与目标类相同 |
| m_TransparentTagHandler2 | 0x376ABF8 | TypeInfo 槽已找到；manager 检查为 CLASS_ONLY，s_instance=0 |
| InitActorKitRefer | 0x376ABF8 | 当前 TTH TypeInfo RVA |
| Il2CppClassStaticFieldsOffset | 0xB8 | 经 singleton 实例回指验证 |
| CrossFadeInFixedTime / Super 回退项 | 0x797D760 | AnimPlayable 的七参数签名，模块内地址 |
| Character.ArrayStart | 0x30 | 从实际 Dictionary Entry.value 布局推导成功 |
| LocalGenRangeSingleData | 0x5FECD50 | RangeAttackEmitter 的 14 参数方法；原 0x111 是旧占位 |
| m_GlobalTime2 | 0x3755D18 | GameBaseObject TypeInfo |
| m_CharacterManager2 | 0x374C2B8 | CharacterManager TypeInfo |
| m_UserDataManager2 | 0x3772EA8 | 具有业务字段的 UserData TypeInfo |
| m_CharactorSync2 | 0x374C368 | MobileNetwork.CharactorSync TypeInfo |
| m_EntityManager2 | 0x3755DD0 | GameLogic.Common.GameEntity TypeInfo |

TTH 的顶部两处状态并不矛盾：目录 FOUND 指 TypeInfo 槽已找到；manager CLASS_ONLY 指实例链未验证。本次实例为零，不能据此认定对象链已可读，也不能据此否定 TypeInfo RVA。

## 仍未解决或需要区分的项目

1. `Base_WindowBase2` 为 NOT_FOUND：UnityPlayer_LVB.dll 可写页内，指定链与 1920×1080 客户区尺寸匹配的候选为 0。该结果只说明当前查找规则未命中，不证明窗口根指针不存在。没有回填旧 0x3764380。
2. `FlushState` 为 NOT_FOUND：业务规则要求 `ActorKit.FlushState`，当前未找到。
3. `MemoryShockInteractableVtableSlotOffset=0x198` 为 CANDIDATE：相邻 MethodInfo 配对指向 **ActorKitInteractableDevice.FlushState**，目标 RVA `0x4F4BCD0`。这不是 ActorKit 类的方法，也不能替代第 2 项或证明旧 hook 的上下文语义正确。
4. `Il2CppList.Items/Size` 与 `BattleItemData.listItems/listSize` 四项为 UNRESOLVED_LAYOUT。开放泛型 List 的正文原始 offset 为 0；保留 rawOffset=0，不将它用作实例读取偏移，也不回填旧 0x10/0x18。
5. 31 个 UNRESOLVED 包含 MatrixAddr、未证明类型的 hook context、旧 native CALL/JMP 和指针链。当前没有额外证据恢复这些条目。
6. `op_Subtraction`、`Call_GetVelocity` 按用户允许标 SKIPPED_LEGACY。

与 Offset.h 的本轮数值差异仅在旧占位 LocalGenRangeSingleData 和上述四条泛型原始布局；其他已确认字段值一致，已有 Class 注释没有新差异。这里的“值一致”不能替代运行时业务对象类型和链路验证。

## 检查方式

- 读取本轮日志首尾、校准结果、汇总和异常标记。
- 对照 v6 输出计数，直接流式扫描本轮完整 dump 的 Image/类/字段/方法/属性条目。
- 按精确大小写逐条比较 145 条字段摘要与正文，不只依赖摘要自己报告成功。
- 运行 `tools\check_offset_catalog.ps1 -Report .\dump_workspace\validation\v7-offset-catalog-runtime-20260910.md`，保留此前离线报告。
- 本轮只生成检查报告，没有进行功能修复、重新构建或再次运行目标。
