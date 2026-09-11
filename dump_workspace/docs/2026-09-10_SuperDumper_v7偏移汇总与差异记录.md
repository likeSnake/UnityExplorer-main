# SuperDumper v7：Offset.h 顶部汇总、映射修正与验证记录

## 交付

本次按用户要求构建 DLL，由用户自行运行测试。新增偏移摘要默认启用，位置为 `dump.cs` 的 manager 摘要之后、第一行 `// Image` 之前；完整保留 `Offset::ActorModel.ActorKitInteractiveS.TargetInteractiveFID` 这种变量路径。

- DLL：`F:\gua\UnityExplorer-main\bin\Release\SuperDumper.dll`
- Release x64 / MSVC v143，大小：436736 bytes。
- SHA256：`4CD7713829B1367D33777020917ED8BB241EE07F3B816762F5C4BD8088BC0590`
- PDB：`F:\gua\UnityExplorer-main\bin\Release\SuperDumper_v7_Release.pdb`
- 自检：`153 passed / 0 failed`；原 110 项回归全部保留。
- 日志第一行：`=== SuperDumper v7 (Offset.h inventory discovery) start ===`
- 配置 `offsetsummary=1` 默认开启；设为 `0` 可关闭新摘要。
- 现有 `bin\Release\SuperDumper.cfg` 指向 `F:\gua\UnityExplorer-main\dump_workspace\inject_test\out_super`。

这次没有注入游戏，没有使用 DMA，没有改写历史 dump，没有执行游戏业务方法。153 项属于离线合成内存测试，不能视为 v7 已经实机通过。用户下次运行后可提供 `dump.cs` 顶部和 `log.txt` 的 `[offset-report]` 行分析。

## 覆盖范围及输出规则

`naraka_offset_catalog.hpp` 收录 Offset.h 中全部 **221 个数值变量声明**，包含符号初始化的变量；不包含枚举成员、局部备份变量和函数。目录按以下类别解析：

| 类别 | 数量 | 处理 |
|---|---:|---|
| 托管字段 | 149 | 精确字段名、自动属性 backing field、父类查找 |
| TypeInfo | 15 | 当前模块指针槽 RVA，并读回校验 klass；含 InitActorKitRefer |
| 方法 | 5 | MethodInfo 名称、参数数量、模块范围；CrossFade 再校验完整七参数类型 |
| IL2CPP 布局 | 9 | 本轮校准或 singleton 实例回指证据 |
| 模块基址 | 3 | 标记 RUNTIME_VA，不能当成 RVA |
| 窗口根指针 | 2 | UnityPlayer 可写页扫描和实际窗口尺寸校验，仅为候选 |
| 未恢复语义条目 | 32 | UNRESOLVED 或用户允许跳过的 SKIPPED_LEGACY |
| 配置常量、数组头、字典 Entry | 各 1 | 分别标 CONFIG_CONSTANT、ABI_LAYOUT、DERIVED/UNRESOLVED |
| vtable、hook context | 2、1 | 候选配对或明确未确认，不能仅按旧偏移认定成功 |

一次只导出选中的普通/Super/SuperIBT 模块。普通组和 `*2` 组独立；另一组未检测加载时标 `NOT_LOADED`，已加载但未选中时标 `NOT_DUMPED`。是否另一个模块加载依据 loader 模块查询，隐藏模块可能无法被该查询确认。

以下仅为格式示例，值由实际运行重新读取：

```cpp
// Offset::ActorModel.ObjectMsg = 0xD8; // status=FOUND Class: ActorModel->ObjectMessenger objectMsg [instance]
// Offset::ActorModel.RuntimeLogicLayerS.LayerWeight = 0x18; // status=FOUND Class: RuntimeLogicLayer->Single _layerWeight [instance]
// Offset::m_BuffManager = NOT_LOADED; // status=NOT_LOADED other GameAssembly variant
// Offset::Il2CppList.Items = UNRESOLVED_LAYOUT; // ... rawOffset=0x0
```

`FOUND` 是元数据匹配成功，并不证明业务读取链或 hook 行为正确。`CANDIDATE` 需要进一步验证，`ABI_LAYOUT` 和 `CONFIG_CONSTANT` 不是动态发现结果。找不到的项目不填历史值。

## 三组无注释字段的核对结果

依据 `F:\gua\myCode\Naraka\Thread.cpp` 的读取链、读取宽度、目的变量，与最新 Super dump 的类声明逐项比对。**这 24 个字段的偏移与现有 Offset.h 数字完全一致**，读取类型也相符。映射已写入 catalog 和业务 `Offset.h` 注释。

| Offset.h 结构 | 变量 | dump 实际字段 | 类型 | 偏移 |
|---|---|---|---|---|
| ActorKitInteractiveS | TargetInteractiveFID | m_targetInteractiveFID | Int32 | 0x10 |
| ActorKitInteractiveS | CanRescueActor | canRescueActor | ActorKit | 0x28 |
| ActorKitInteractiveS | CanStealActor | canStealActor | ActorKit | 0x30 |
| ActorKitInteractiveS | DyingExecuteActorFid | DyingExecuteActorFid | Int32 | 0x38 |
| ActorKitInteractiveS | InteractiveType | m_interactiveType | ActorInteractiveType | 0x58 |
| ActorKitInteractiveS | IsMainPlayer | isMainPlayer | Boolean | 0x5C |
| ActorKitInteractiveS | Status | m_status | ActorInteractiveStatus | 0x60 |
| ActorKitInteractiveS | IType | m_itype | InteractiveType | 0x64 |
| ActorKitInteractiveS | RescueFid | rescue_fid | Int32 | 0x68 |
| ActorKitInteractiveS | NowShowInteractiveType | now_showinteractiveType | ActorInteractiveType | 0x70 |
| RuntimeLogicLayerS | LayerIndex | layerIndex | Int32 | 0x10 |
| RuntimeLogicLayerS | LayerType | LayerType | ELogicLayer | 0x14 |
| RuntimeLogicLayerS | LayerWeight | _layerWeight | Single | 0x18 |
| RuntimeLogicLayerS | CurShareState | CurShareState | PlayableState | 0x48 |
| RuntimeLogicLayerS | CurStateStartTimeOffset | CurStateStartTimeOffset | Single | 0x50 |
| RuntimeLogicLayerS | CurDuration | CurDuration | Single | 0x54 |
| RuntimeLogicLayerS | CurTimeForLogicState | _curTimeForLogicState | Single | 0x60 |
| RuntimeLogicLayerS | TransitionDuration | TransitionDuration | Single | 0x74 |
| RuntimeLogicLayerS | TransitionPercent | _transitionPercent | Single | 0x78 |
| RuntimeLogicLayerS | CurIsLogicState | CurIsLogicState | Boolean | 0x7C |
| RuntimeLogicLayerS | TransitionFinish | transitionFinish | Boolean | 0x7E |
| PlayableStateS | AnimatorName | animatorName | String | 0x10 |
| PlayableStateS | NameHash | NameHash | Int32 | 0x18 |
| PlayableStateS | TagHash | TagHash | Int32 | 0x1C |

源码依据：Thread.cpp 约 548 行读取 interactive，697 行读取 RuntimeLogicLayer，774 行读取 PlayableState。Super dump 对应类起始行：9856、34429、33753。

注意 `EntityKit.actorKitInteractive` 的**声明类型是 IEntityKitInteractive 接口**，原 Offset.h 注释误写成 ActorKitInteractive，已纠正。上述布局属于 ActorKitInteractive 实现类；其他接口实现不能直接套用。当前 dump 可以证明声明和布局，不能证明某个运行时对象一定是该实现。

## 与旧注释、读取假设不一致的地方

用户明确要求冲突时以 dump 为准并汇报。本次保留 Offset.h 原数值初始化，仅补齐/纠正映射注释；新版 DLL 每次输出本次实测值。未跨变体回填业务代码。

| 项目 | 原记录 | dump 证据与本次处理 |
|---|---|---|
| TTH ActorKit 字段 | `actorKit` | 精确拼写为 `actorkit`，偏移仍 0x70；修正 catalog 与 Offset.h 注释 |
| EntityKit ActorModel 字段 | `actorModel` | 实际为 `<actorModel>k__BackingField`，偏移 0x18；自动属性解析并输出真实声明 |
| EntityKit interactive 字段 | ActorKitInteractive | 实际声明为 IEntityKitInteractive；偏移 0xD0 未变 |
| Buff.GroupId | `GroupIdEE` | 实际为 `<GroupId>k__BackingField`，Int32，0x24；修正错误字段名 |
| Buff 多个属性 | 只写属性名 | BuffInstanceIdList、BuffTidList、BuffId、BuffIId 补成实际 backing field，补充准确类型 |
| Character.AllAliveCharList、CurrentCollisionFlags、Buff 字段注释 | 缺字段类型 | 补充 Dictionary、CollisionFlags、Single、Int32 等 dump 声明 |
| 泛型 List 字段 | `_items=0x10`、`_size=0x18` | 最新 Super dump 的开放泛型 `List<T>` 两项均原样显示 0x0；属于未确认的实例布局，摘要标 UNRESOLVED_LAYOUT 并保留 rawOffset=0x0，不替换为历史值，也不宣称 0 是可用对象偏移 |
| ActorKit.FlushState | Hook.cpp 请求 ActorKit 的 8 参数方法，Offset.h 保存旧 RVA | 当前普通和 Super dump 均未找到对应类方法；按 8 参数匹配，输出 NOT_FOUND，不能替换成 ActorKitInteractableDevice 或其他类的同名方法 |
| hookContext +0x10 | 被注释为 EntityKit | ActorKitInteractableDevice 的 +0x10 实际为 PlayerEnvironmentColliderAgent agent；hook 的真实 RCX 类型未证实，输出 UNRESOLVED |
| Super static_fields | 曾以 interfaces+0x10 推断 0xC0 | 用户 v6 日志真实验证 0xB8；v7 只采用 singleton 实例 klass 回指通过的证据，冲突标 AMBIGUOUS |

泛型 List 异常影响四个变量：`Il2CppList.Items`、`Il2CppList.Size`、`BattleItemData.listItems`、`BattleItemData.listSize`。源码的静态字段 offset=0 是合法情况，与此区分处理。

不同版本/变体的方法数值差异：

| 变量 | Offset.h 原值 | 普通参考 dump | 最新 Super dump |
|---|---|---|---|
| AnimPlayableCrossFadeInFixedTimeNormal | 0x5811350 | 0x5975FF0 | 不适用 |
| AnimPlayableCrossFadeInFixedTimeSuper | 0x797D760 | 不适用 | 0x797D760 |
| LocalGenRangeSingleData | 0x111 占位 | 0x5B3DD70 | 0x5FECD50 |

`CrossFadeInFixedTime` 在 Offset.h 是符号初始化，不把它报成缺失数值；摘要返回所选变体的当前方法 RVA。`InitActorKitRefer` 按实际业务语义是当前 TTH TypeInfo RVA，不是同名方法。用户允许忽略的 `op_Subtraction`、`Call_GetVelocity` 缺可靠类/签名，标 `SKIPPED_LEGACY`。

## 定位与实现经验

1. **先精确核对已有 dump，再编写发现规则。** PowerShell 默认 Hashtable 不区分大小写，会漏掉 actorKit/actorkit 的错误；离线审计改成 Ordinal 区分大小写。类名还必须带 namespace。ActionBreakData、ActionHitSimulateData、SkillComboData 等属于 Code.Module_Battle。
2. **同名类不能取第一个。** UserData 同时是业务类和嵌套 struct；BattleItemData 有多个定义。字段解析逐个候选匹配，只有唯一声明命中才接受。UserData TypeInfo 先用 battleData 字段识别业务类，再定位槽位，避免枚举顺序改变时选中 struct。
3. **读取 API 的 false 不总等于地址坏。** MemReader::rdStr 对空字符串返回 false。初版 v7 索引把空 namespace 当成错误，新增测试发现 14 项失败；修复为允许空/null namespace、仍拒绝不可读指针后全部通过。
4. **元数据原值与可用布局分开。** 静态字段零偏移有效；开放泛型的实例零偏移不能当成对象字段。负 offset、literal、越模块方法地址、非执行页方法地址均保留对应状态。
5. **TypeInfo RVA 是模块内槽地址减基址。** 必须验证该槽仍指向预期 klass，不能把堆上的 klass 地址当成 RVA。已有 manager 的对象链验证证据用于确认 static_fields。
6. **未知旧 hook 偏移不进行数值平移。** 旧 CALL/JMP、返回块、bootstrap chain 未恢复语义时输出原因。vtable 在历史偏移处即使找到可执行指针和 MethodInfo 配对，也只标 CANDIDATE，因为目标虚方法意图未确定。
7. **窗口定位有边界且不冒充确定结果。** 扫描实际 UnityPlayer 模块可写页，验证 `slot -> +0xB8 -> +0x8 -> width@0x14,height@0x18` 与当前进程窗口客户区尺寸。预算 3 秒/25 万不同指针；唯一匹配也只是 CANDIDATE，多项 AMBIGUOUS，预算耗尽 SCAN_INCOMPLETE。这个字段无法只靠 managed dump 命名。
8. **报告不污染主 dump 统计。** 摘要调用 typeName 等函数前后保存/恢复 Stats；关闭类级 RVA 时，只要摘要开启仍会运行 TypeInfo 查找。

## 验证结果与可复现路径

离线目录审计工具：`tools\check_offset_catalog.ps1`。检查 221/221 完整覆盖，按大小写、namespace、唯一成员、backing field 和 CrossFade 参数签名核对；不会修改源 dump 或 Offset.h。

输入与报告：

- Super 输入：`dump_workspace\inject_test\out_super\dump.cs`（用户 v6 实机产物）。报告：`dump_workspace\validation\v7-offset-catalog.md`。
- 普通输入：`F:\gua\dump\dump2026.09.03.cs`（Unity-Offset.dll 参考产物）。报告：`dump_workspace\validation\v7-offset-catalog-normal.md`。

149 个字段 + 5 个方法 + 15 个 TypeInfo 共 169 条可做离线名称映射审计：

- Super：156 项可确认，5 项未解决（FlushState + 4 条 List 泛型布局），8 项属于另一变体。
- 普通参考：153 项可确认，8 项未找到（FlushState、4 条 List 字段、Buff/TTH/InitActorKitRefer 三条未附类级 RVA），8 项属于另一变体。参考文件没有 mscorlib 的 List 类段；缺少某类 RVA 不代表 v7 在运行时也找不到。
- 补正后的已有 `Class:` 注释与已匹配 dump 声明没有剩余差异。
- 总目录的其余 52 项属于模块、布局、native、常量或未恢复语义条目，必须看运行时摘要，不能以离线 156 项结果推断全部 221 项已找到。

构建命令（PowerShell）：

```powershell
& 'C:\Program Files (x86)\Microsoft Visual Studio\2022\BuildTools\MSBuild\Current\Bin\MSBuild.exe' App\Il2CppRuntimeDumper\SuperDumper.vcxproj /t:Build /p:Configuration=Release /p:Platform=x64 /v:minimal
& 'C:\Program Files (x86)\Microsoft Visual Studio\2022\BuildTools\MSBuild\Current\Bin\MSBuild.exe' App\Il2CppRuntimeDumper\DumpSelfTest.vcxproj /t:Build /p:Configuration=Release /p:Platform=x64 /v:minimal
powershell.exe -NoProfile -ExecutionPolicy Bypass -File .\tools\check_offset_catalog.ps1
```

两项目构建成功；仅原有 `/EHs` 被 `/EHa` 覆盖的 D9025 提示。自检在 `dump_workspace\validation\v7-selftest` 目录运行，日志 `selftest.log`，摘要样例 `selftest_offset_report.cs`。覆盖标准/index/handle 布局、字段继承/大小写/零偏移、同名类、方法重载/签名/地址有效性、另一变体状态、TypeInfo 槽读回、静态布局冲突、窗口多候选和完整目录输出，最终 `pass=153 fail=0`。

后续以用户新运行产生的 v7 日志和 dump 为准，优先检查 unresolved/candidate 条目的实际证据。
