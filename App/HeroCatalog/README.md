# HeroCatalog：英雄目录文件导出 DLL

输入已有的 UTF-8 TSV 配置表，导出英雄 ID、名称、分类、技能名、连招图和动作映射。该 DLL 由独立 `HeroCatalogHost.exe` 加载并显式调用，不在 `DllMain` 做导出，不连接或注入游戏进程，也不是新版 `SuperDumper.dll`。

本轮的数据发现、验证结果与剩余工作见 [研究报告](../../dump_workspace/research/hero-catalog/report.md)。

## 使用

从 `F:\gua\UnityExplorer-main` 执行：

```powershell
& '.\bin\Release\HeroCatalogHost.exe' `
  '.\bin\Release\HeroCatalog.dll' `
  'F:\gua\dump\dump_all\super\config' `
  '.\dump_workspace\research\hero-catalog\artifacts\super_reference'
```

程序返回 `OUTPUT=...`，每次创建独立的带时间戳目录，保留旧产物。退出码为 0 表示文件导出完成，不代表当前游戏、当前开放英雄或动作全集已验证。

只需要必选主表即可运行。存在的可选表必须有正确格式；缺表会记录为 `MISSING`，只有表头的空表记录为 `EMPTY`。输入是原参考工具的原始 TSV：一行一条记录、Tab 分隔，不支持带引号的跨行 CSV；不会将文件中的 `\n` 自动还原为换行。

| 输入文件 | 用途 |
| --- | --- |
| `HeroConfig_HeroBattleConfig.tsv`（必选） | 英雄 ID、战斗名称、内部名称、关联形态、映射 ID |
| `HeroFashion_HeroFashion.tsv` | 显示名称、主英雄标记、界面动画映射 |
| `HeroTalent_HeroTalent.tsv` | 默认及备选天赋技能 ID |
| `TalentConfig_TalentSkill.tsv` | 所属英雄、中文技能名、连招图、连招映射 |
| `PlayableMappingConfig_MappingConfig.tsv` | 映射 ID 对应的配置组名称 |
| `AutoAnimatorStates_MaxStateMachineConf.tsv` | 独立动作状态名称、标签、层号 |

导出文件：

- `heroes.json`：全部目录、来源、计数和未验证项。
- `heroes.tsv`：英雄清单，保留两个来源的名称。
- `hero_skills.tsv`：中文技能名、连招图与映射关系；`listed_in_hero_talent` 区分当前天赋表列出的条目与其他关联技能。
- `hero_action_links.tsv`：英雄字段引用的动作映射组，保留来源字段。
- `action_states.tsv`：独立状态目录，`hero_id` 与 `action_hash` 未确认时为 `null`。
- `dump_header.cs`：可放在类型 dump 前面的注释摘要。
- `offset_log.txt`：简体中文运行说明。
- `manifest.json`：源文件与产物的 SHA-256、大小、计数；所有七个产物成功写入后才生成。

JSON 使用 UTF-8；TSV、注释摘要和中文日志含 UTF-8 BOM，方便 Windows 工具读取。TSV 输出对 Tab、换行和反斜杠进行转义。

## 独立运行时信息目录（推荐）

详细英雄、技能和动作资料不写入 `dump.cs`。使用独立打包脚本，在 dump 旁边创建带时间戳的 `<dump名>_runtime_info_*` 文件夹；原 dump 的字节内容和文件名保持不变。

```powershell
powershell.exe -NoProfile -ExecutionPolicy Bypass -File `
  '.\App\HeroCatalog\Export-HeroCatalogBundle.ps1' `
  -CatalogDirectory '<HeroCatalogHost 的 OUTPUT 目录>' `
  -SourceDump 'F:\gua\dump\dump_all\super\dump.cs'
```

未指定 `-OutputRoot` 时，脚本会自动在源 `dump.cs` 所在目录创建资料目录；也可以显式指定其他目录保存历史 Bundle。

目录内包含 `heroes.json`、`heroes.tsv`、`hero_skills.tsv`、`hero_action_links.tsv`、`action_states.tsv`、`offset_log.txt`、`README.md` 和 `bundle_manifest.json`。清单记录源 dump 的 SHA-256，并明确 `dump_modified=false`。

这是后续接入 `SuperDumper.dll` 的输出约定：

```text
dump_YYYYMMDD_HHMMSS_mmm.cs
dump_YYYYMMDD_HHMMSS_mmm_runtime_info/
  heroes.json
  hero_skills.tsv
  hero_action_links.tsv
  action_states.tsv
  bundle_manifest.json
```

## 旧的组合脚本（仅历史兼容）

旧脚本会生成带顶部注释的新 dump 副本，原 dump 不变。它保留用于读取旧产物，但不再是推荐流程；详细数据现在应使用上一节的独立目录。脚本要求输入 dump 与配置目录属于同一导出目录，并验证摘要哈希；目录相同不等于版本已得到独立确认。

```powershell
powershell.exe -NoProfile -ExecutionPolicy Bypass -File `
  '.\App\HeroCatalog\Compose-HeroCatalogDump.ps1' `
  -CatalogDirectory '<上一步 OUTPUT 目录>' `
  -SourceDump 'F:\gua\dump\dump_all\super\dump.cs' `
  -OutputPath '<上一步 OUTPUT 目录>\dump_with_hero_catalog.cs' `
  -Legacy
```

组合文件旁生成独立的 `.manifest.json`。若组合过程中失败且没有该清单，输出应视为不完整；重新运行使用新输出名。

## 构建和验证

Visual Studio 2022 / MSVC v143，Release x64，C++17，静态 C++ 运行库。JSON 复用仓库中的 nlohmann/json 3.11.3（MIT），摘要使用 Windows BCrypt。

```powershell
$buildTool = 'C:\Program Files (x86)\Microsoft Visual Studio\2022\BuildTools\MSBuild\Current\Bin\MSBuild.exe'
& $buildTool '.\App\HeroCatalog\HeroCatalog.vcxproj' /p:Configuration=Release /p:Platform=x64 /nologo
& $buildTool '.\App\HeroCatalog\HeroCatalogHost.vcxproj' /p:Configuration=Release /p:Platform=x64 /nologo
& '.\bin\Release\HeroCatalogHost.exe' '.\bin\Release\HeroCatalog.dll' --self-test '.\dump_workspace\research\hero-catalog\validation'
```

自检实际加载 DLL，调用导出接口，检查生成文件和错误路径，不访问游戏。每次测试保留独立的输入夹具与 `test_result.json`。

`Verify-HeroCatalog.ps1` 可独立核对源文件、产物 SHA-256、记录数量与 ID 唯一性；存在组合 dump 时还验证正文与原文件一致（仅去除重复 BOM）。

`Verify-HeroCatalogBundle.ps1` 专门核对独立目录，确认源 dump 存在、大小和 SHA-256 一致，且每个详细文件与 `bundle_manifest.json` 一致。

## API

接口见 `HeroCatalog.h`：`HeroCatalogVersion()` 返回 1；`ExportHeroCatalogW()` 接收配置目录、输出根目录及至少 32768 个 `wchar_t` 的结果缓冲区。成功时返回新目录，失败时返回错误消息。接口不跨 DLL 边界传递 STL 对象，不抛出 C++ 异常。导出时不得同时修改输入文件。

不把主英雄标记、商店解锁配置或技能 `_Launch` 推断成当前可选状态；不把参考表 `_KEY` 当作动作 Hash；不将配置组名、中文技能名或连招图名冒充具体动画片段名。
