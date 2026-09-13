# 英雄目录与动作信息：分析、实现与验证记录

日期：2026-09-12。

## 1. 完成范围

目标已纠正为游戏的英雄种类目录，包括英雄 ID、名称和关联动作资料，不是对局里有多少个玩家。

本轮构建了 `F:\gua\UnityExplorer-main\bin\Release\HeroCatalog.dll`，通过独立 `HeroCatalogHost.exe` 加载，读取已有配置表并验证输出。普通与 Super 历史样本均生成英雄目录及带顶部摘要的 dump 副本。

**这不是已经接入游戏自动采集的 SuperDumper 新版。** 原 `SuperDumper.cpp`、`SuperDumper.dll` 及原始 dump 未改动。当前进程的普通 WinAPI 模块枚举返回拒绝访问，未通过其他手段绕过，也没有把本轮 DLL 注入游戏。后续运行时数据采集和当前可选英雄数量仍未完成，不能将本报告中的历史表数据当作当前游戏现场。

## 2. 本次查找过程及有效案例

最贴近本次问题的既有案例是 `dump_workspace/docs/2026-09-10_dump_all产物分析与扩展可行性.md` 中的 BinaryDesignData 配置表导出分析。该案例已经指出：类声明只能提供字段位置，配置行需要读取实际数据；只看缓存会受加载阶段影响。

`docs/经验文档/BuffManager与TransparentTagHandler查找经验.md` 提供的可复用原则是按当前变体验证类型和实例身份，不套历史指针。但本次的英雄目录入口是配置系统，不应继续沿 CharacterManager 的对局玩家集合处理。

具体证据链：

1. 旧 SDK 的 `HeroType` 和 `GetHeroType` 是人工维护的枚举与映射，不能用于验证完整英雄数量。
2. 当前已有 Super dump 中 `HeroIdDef` 是空声明，直接数该类型的成员无法得到目录。
3. 找到 `BinaryDesignData.CBinaryDesignDataReader`，其中包含 `HeroConfig_HeroBattleConfig`、`HeroFashion_HeroFashion` 等 Reader。
4. 在历史 `dump_all/{normal,super}/config` 中确认这些表确实含有数据，逐行检查主键、类型、名称及关联字段。
5. 英雄战斗表关联外观表，分开保留主英雄标记、占位名称及关联形态。再用英雄天赋表的技能 ID 和技能详情表的所属英雄进行交叉验证。
6. 动作映射只通过明确 ID 关联；独立状态表没有确认的英雄归属，不用名字相似性强行归类。

只读访问记录在 [read_access_20260912_01.json](evidence/read_access_20260912_01.json)：进程 PID 25208、启动时间 05:22:47，05:33:35 的模块枚举返回 `MODULE_ENUM_FAILED / win32_error=5`。它只证明该时刻进程存在及访问失败，不证明当前模块版本。

## 3. 数据入口及语义

| 数据 | 配置来源 | 解释 |
| --- | --- | --- |
| 英雄 ID、内部名称、战斗名称 | `HeroConfig_HeroBattleConfig._ID/_HeroName/_HeroTitle` | `_HeroName` 是内部标识，中文名称来自 `_HeroTitle` |
| 显示名称与主英雄标记 | `HeroFashion_HeroFashion._HeroName/_IsMainHero` | 不等于服务器当前开放名单 |
| 关联形态 | `HeroConfig_HeroBattleConfig._RelateHeroID` | 结合被引用记录的非主英雄标记识别关联形态，保留原始关系 |
| 默认和备选技能 ID | `HeroTalent_HeroTalent` | 默认技能、默认奥义及两个备选列表分别保留 |
| 中文技能名、连招图 | `TalentConfig_TalentSkill._TalentName/_ComboGraph` | 与 `_RelateHero` 校验所属英雄 |
| 技能连招映射 | `TalentConfig_TalentSkill._ComboMappingID` | 与下表 ID 连接，仍是配置组层级 |
| 英雄/UI/待机/表情映射 | 英雄表、外观表的 MappingID 字段 | 每条关系记录来源表与来源字段 |
| 映射组名 | `PlayableMappingConfig_MappingConfig._MappingName` | 不直接当成某个动画或动作名 |
| 独立状态名、标签、层号 | `AutoAnimatorStates_MaxStateMachineConf._ID/_TagStr/_LayerIdx` | 不填未知英雄 ID 和动作 Hash |

2026-09-11 既有 Super dump 的定位参考（仅适用于该文件）：

- `HeroConfig_HeroBattleConfig`：第 1677857 行，命名空间为 `LX.Game`。
- `HeroFashion_HeroFashion`：第 1684213 行。
- `HeroTalent_HeroTalent`：第 1685579 行。
- `TalentConfig_TalentSkill`：第 1735058 行。
- `CBinaryDesignDataReader`：第 1755735 行，命名空间为 `BinaryDesignData`。
- 主 Reader 的英雄战斗表字段：第 1756709 行；外观表字段：第 1756720 行。
- `HeroConfig_HeroBattleConfig_BinaryReader`：第 1773535 行，包含键索引、缓存、字节缓冲区及 padding 字段。

这些位置说明类结构已经存在；本轮未将字段偏移写死到新 DLL，更未把旧对象 VA 用于当前进程。

## 4. 实际输出统计

| 指标 | 普通参考 | Super 参考 |
| --- | ---: | ---: |
| 英雄战斗表记录 | 37 | 37 |
| `_IsMainHero=1` 记录 | 33 | 33 |
| 非占位主英雄记录 | 28 | 28 |
| 占位记录 | 5 | 5 |
| 有明确关联的非主形态 | 4 | 4 |
| 英雄与外观表缺失关联 | 0 | 0 |
| 英雄/UI/待机/表情映射引用 | 370 | 370 |
| 其中非零且解析成功 | 294 | 294 |
| 其中零值（不覆盖默认映射） | 76 | 76 |
| 非零映射解析失败 | 0 | 0 |
| 英雄—技能详情关联 | 321 | 321 |
| 技能所属英雄或详情校验失败 | 0 | 0 |
| 非零技能映射解析失败 | 0 | 0 |
| 独立状态名 | 19057 | 9672 |

不能据此回答“现在能选 28 个英雄”。33 条主英雄记录里含 5 条占位记录；剩下的 28 条仍包含藤鬼这类需进一步解释的角色。开放名单可能受版本、玩法和服务器配置影响，因此 `current_playable_hero_count` 和每个英雄的 `playable_now` 都输出 `null`。

4 个关联形态为天海→伏魔金刚、岳山→神将、沈妙→铁卫、万钧→雷主。它们保留在完整配置清单中，不额外算成独立可选英雄。

两份状态目录交集为 9672 条，Super 独有为 0，普通独有为 9385。差异可能来自加载程度、采集覆盖或版本，本轮没有足够证据定因。新工具逐源保留目录，不把合并后的并集宣称为当前游戏全集。

实际关联示例：英雄 `1000001`（土御门胡桃）→技能 `3470000`（庇护）→连招图 `hero_onmyoji_ns_1_combo_graph`→映射组 `hero_onmyoji_ns_1_combo_player`。这说明中文技能名和资源组可以通过配置主键取到，无须依赖对局中该英雄出现。

## 5. 发现的不一致及处理

1. `deps/Naraka/SDK.cpp` 把 `1000028` 写成“彩戏师”，参考表中显示“蓝梦”；`1000026` 的旧手工名称是“张启灵”，参考表为“张起灵”。本轮以表格保存名称，未改动旧业务代码。
2. `1000008` 在战斗表是“伏魔金刚”，外观表是“大佛”；`1000012` 分别为“神将”和“将军俑”。这是来源之间的名称差异，两个名字都保存，`names_differ=true`。
3. 同一状态 `attack_1000` 在普通与 Super 参考文件中的 `_KEY` 分别为 `1153489536`、`1085103104`。相同状态名的这个键不稳定，不能作为已验证动作 Hash。新输出只保留 `_ID` 状态名，Hash 保持未知。
4. 参考配置中的部分 `_MotionClipsList`、`_SkillList` 等数组含疑似对象引用被拆分后的整数。新工具不将这些字段当成真实动作 ID 或动画名称，优先使用类型清楚的标量和英雄天赋表的整数 ID 列表。
5. `_Launch` 是技能配置标记，不是英雄开放验证。新工具原样保存为 `launch_config`，不推算 `playable_now`。

## 6. 实现与产物

源码在 `App/HeroCatalog/`，使用 MSVC v143 / C++17 / x64，复用仓库 nlohmann/json 3.11.3（MIT）。支持中文路径、UTF-8 校验、重复 ID 拒绝、主键一致性检查、缺表/空表区分、整数边界检查、显式关联状态、时间戳独立目录和源文件/产物 SHA-256。

详细资料现在采用独立 `*_runtime_info_*` 目录。`dump.cs` 保持原样，目录内保存英雄、技能、动作状态、来源和验证清单；旧的 `Compose-HeroCatalogDump.ps1` 只保留用于兼容此前已经生成的 `dump_with_hero_catalog.cs`。

接口通过独立宿主加载 DLL 执行。本次产物：

- `bin/Release/HeroCatalog.dll`，SHA-256 `72D9EF5CB401C504673A43859AA57959DADD2AF21C908CB7A296504253B1B363`。
- `bin/Release/HeroCatalogHost.exe`，SHA-256 `663F29B7FB67C60425F845BFD7F7E3B394BD4BF7FFEA278EDF416AFE24DDB76B`。

最终 Super 目录：[catalog_20260912_170137_210_16784_0](artifacts/super_reference/catalog_20260912_170137_210_16784_0/)。其中：

- [英雄清单](artifacts/super_reference/catalog_20260912_170137_210_16784_0/heroes.tsv)
- [中文技能与连招映射](artifacts/super_reference/catalog_20260912_170137_210_16784_0/hero_skills.tsv)
- [动作状态目录](artifacts/super_reference/catalog_20260912_170137_210_16784_0/action_states.tsv)
- [带英雄顶部摘要的 dump 副本](artifacts/super_reference/catalog_20260912_170137_210_16784_0/dump_with_hero_catalog.cs)

推荐的独立目录：[dump_runtime_info_20260912_171249_129_0](bundles/dump_runtime_info_20260912_171249_129_0/)。该目录包含同样的详细结构和 `bundle_manifest.json`，清单明确 `dump_modified=false`；源 dump SHA-256 为 `46F160246DBBAF1CCEADD867799E6AE330E873B45EBDA1D104371A178E080E2C`。

普通版对应目录：[dump_runtime_info_20260912_171550_031_0](bundles/dump_runtime_info_20260912_171550_031_0/)，源 dump SHA-256 为 `020702AB8B85AB7202DE59275FD874BC34D39E2E27775016BA1FFB9DE6F20F47`。两个独立 Bundle 均通过 12 项校验。

最终普通目录：[catalog_20260912_170137_308_15176_0](artifacts/normal_reference/catalog_20260912_170137_308_15176_0/)。早一轮 16:56 的输出保留为开发过程记录，缺少后续补齐的技能详情，应使用这里列出的最终目录。

文件组合脚本仍能验证来源目录和摘要哈希，但不再推荐把摘要放进 dump。新的独立打包脚本只复制详细文件，写入 `dump_modified=false` 和源 dump SHA-256；不会触碰 `Class:` 正文，也不会覆盖原 dump。

## 7. 验证结果

- 两个 Release x64 工程构建成功，无构建警告。
- 实际加载 DLL 的自检：29 项通过、0 失败。包含重复 ID、主键不符、乱码、列数错、缺表/空表、非法布尔值、数组类型/溢出、技能所属英雄冲突、缺失映射和避免输出覆盖。
- 普通和 Super 原始 TSV 实际导出均成功。
- 独立产物验证：34 项通过、0 失败。包含源文件/产物 SHA-256、行数、ID 唯一性、组合文件哈希和原始正文一致性。
- [加载测试记录](validation/selftest_8424_1438750/test_result.json)
- [独立产物验证记录](validation/artifact_verification_20260912.json)

以上验证的对象是文件解析与目录导出，不能替代运行时完整性测试。

## 8. 后续接入要求与未完成项

运行时接入前需要获得当前构建、当前加载阶段的配置数据。已有结构层能帮助定位 Reader，但它不自动证明缓存覆盖全部表记录。应分别记录键索引有效数、缓存有效数、成功导出数、未物化记录、读取失败数，不能用数组容量当配置行数。

建议将已完成的文件导出逻辑与数据采集解耦：采集阶段提供同结构的表记录及版本/来源信息，目录构建阶段负责主键校验、分类、关联和文件生成。当前 DLL 仅实现文件输入接口，没有实现游戏进程内采集接口。

还缺少三项实证：当前可选英雄的正式筛选规则；映射组到具体动画片段/状态的完整资源关系；当前运行时完整表覆盖。遇到未加载记录时保留未知，不通过强行调用游戏业务方法来补齐。本轮没有实现 DMA 注入、调试绕过或游戏状态修改。

用户已明确允许继续构建和加载测试，不需要重新索要同一授权。本报告保留完成范围与技术边界，避免后续维护者把“独立宿主已加载 DLL”理解成“DLL 已在游戏内验证”。
