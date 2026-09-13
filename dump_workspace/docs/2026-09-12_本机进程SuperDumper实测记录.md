# 2026-09-12 本机进程 SuperDumper 实测记录

## 实测环境

- 目标进程：`NarakaBladepoint.exe`
- PID：`16064`
- 运行方式：本机注入，不使用 DMA
- 注入器：`F:\\gua\\UnityExplorer-main\\bin\\Release\\Injector.exe`
- DLL：`F:\\gua\\UnityExplorer-main\\bin\\Release\\SuperDumper.dll`
- 输出目录：`F:\\gua\\UnityExplorer-main\\dump_workspace\\inject_test\\out_super`
- 实测时间：2026-09-12 17:35（Asia/Shanghai）

## 注入过程

第一次使用相对 DLL 路径时，远程 `LoadLibraryA` 返回 0。原因是路径相对于目标进程的工作目录解析，目标进程无法找到 DLL。

改用绝对路径后注入成功：

```powershell
.\\bin\\Release\\Injector.exe 16064 F:\\gua\\UnityExplorer-main\\bin\\Release\\SuperDumper.dll
```

注入器结果：`LoadLibraryA returned 0000000054380000`，`injection OK`。

## 产物验证

新产物：

- [dump_20260912_173518_039.cs](../inject_test/out_super/dump_20260912_173518_039.cs)
- [结构索引目录](../inject_test/out_super/dump_20260912_173518_039_structures/)
- [本轮日志](../inject_test/out_super/log.txt)

日志确认：

- 模块：`GameAssembly_Super.dll`
- Super 变体路径已启用
- 类：`56495`
- 字段：`462900`
- 方法：`538872`
- 属性：`82786`
- 接口：`9946`
- dump 大小：`96954465` 字节
- 结果：`OK_WITH_WARNINGS`

结构索引目录中的 `export_manifest.txt` 与正文计数一致，生成了 `class_meta.tsv`、`rva_map.tsv`、`all_bases.txt`、`interfaces.txt` 和 `member_diagnostics.tsv`。

## 英雄相关证据

当前实机 dump 中已经出现英雄配置相关运行时类型和字段，例如：

- `HeroConfig_HeroBattleConfig`
- `HeroFashion_HeroFashion`
- `PlayableMappingConfig_MappingConfig`
- `TalentConfig_TalentSkill`
- `HeroBattleStatus`
- `BasePlayableMappingList`
- `ReplacePlayableMappingList`

这证明当前运行时结构可被 SuperDumper 导出。英雄数量、中文名称和技能配置记录仍需要配置表或运行时 Reader 缓存数据作为数据源，不能仅凭类声明数量推导。

## 当前限制

本轮未启动 DMA，也未使用 DMA 探针；之前 DMA 命令失败是因为当前机器没有可用 DMA 适配器。SuperDumper 本机注入和 Super 变体结构导出已经成功。

日志标记 `OK_WITH_WARNINGS` 表示部分成员数组被运行时布局校验拒绝，详细原因位于 `member_diagnostics.tsv`；不影响本轮正文和结构索引文件生成，但不应把所有成员都视为等同于完整元数据验证结果。

## 重启后的第二轮实测

重启游戏后使用新的 PID `2008`，只加载一次正式 `SuperDumper.dll`。本轮扫描耗时约 `30078 ms`，随后正常完成：

- [第二轮 dump](../inject_test/out_super/dump_20260912_180540_218.cs)，大小 `95863057` 字节
- [第二轮结构目录](../inject_test/out_super/dump_20260912_180540_218_structures/)
- 模块：`GameAssembly_Super.dll`
- 类/字段/方法/属性/接口：`56495 / 451760 / 534338 / 82522 / 9837`
- 结果：`OK_WITH_WARNINGS`

本轮独立英雄资料目录为 [dump_20260912_180540_218_runtime_info_20260912_184632_996_0](../inject_test/out_super/dump_20260912_180540_218_runtime_info_20260912_184632_996_0/)，与对应 dump 位于同一级目录。目录包含英雄、技能、动作状态、映射关系、顶部摘要、简体中文日志和 `bundle_manifest.json`。Bundle 校验结果为 `12 passed, 0 failed`，清单记录 `dump_modified=false`，原始 dump 未被修改。

本轮英雄资料统计：配置记录 `37`、主英雄标记 `33`、非占位主英雄 `28`、占位记录 `5`、关联形态 `4`、技能关联 `321`、动作状态名 `9672`。
