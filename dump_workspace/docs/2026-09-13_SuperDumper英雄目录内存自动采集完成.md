# SuperDumper 英雄目录内存自动采集完成（2026-09-13）

## 结论

目标「完全不依赖 TSV，直接从游戏内存自动获取英雄目录，只需一个 SuperDumper.dll 即可导出」已达成。

- `SuperDumper.dll` 在完成 IL2CPP 结构 dump（`dump_*.cs` + `_structures/`）后，自动再走一遍英雄配置读取链路，输出 `hero_catalog.json`。
- 不再需要 `HeroCatalog.dll` / `HeroCatalogHost.exe` / 任何 TSV 配置表。
- 数据来源：运行时已物化的 `BinaryDesignData` 读取器缓存（`m_Cache` 字典），与历史文件版结果一致且更全。

## 本次唯一修复：Dictionary 内存布局

实测（游戏 PID 17952，`GameAssembly_Super.dll` 基址 `0x7FFBFCF40000`）发现，
本构建的 `Dictionary<,>` 用的是**带 `_fastModMultiplier` 的现代 .NET 布局**：

| 字段 | 旧假设（经典布局） | 本构建实测 |
| --- | ---: | ---: |
| `_buckets` | +0x10 | +0x10 |
| `_entries` | +0x18 | +0x18 |
| `_fastModMultiplier` (UInt64) | — | **+0x20** |
| `_count` (Int32) | **+0x20** | **+0x28** |
| `Entry` 步长 | 0x18 (key@+0x08, value@+0x10) | 0x18 ✔（不变） |

原 `ReadDictionaryView` 把 `_count` 当 +0x20 读，读到的是 `_fastModMultiplier`
的低 32 位（垃圾，被 `count-out-of-range` 拒掉），导致 `cacheCount=0`、
「HeroConfig cache is unavailable or empty」。

修复：`App/Il2CppRuntimeDumper/hero_catalog_runtime.cpp::ReadDictionaryView`
不再依赖对泛型实例化类不可靠的 `FieldOffsetByName("_count")`，改为以
`_entries` 数组长度（capacity）为锚，在 `+0x20` 与 `+0x28` 两个候选里选
「0 ≤ count ≤ capacity」的合理值（现代布局 `+0x28` 胜出，`+0x20` 是
`_fastModMultiplier` 低位、远超 capacity 被排除）。

## 实测结果（20260913_042703_369）

```
[4] result=OK_WITH_WARNINGS
[hero-catalog] live memory capture complete:
   heroes=37 skills=322 mappings=4281 states=11624
```

- HeroBattleConfig cache=37（英雄战斗表）
- HeroFashion cache=37（外观表）
- HeroTalent cache=37（天赋表）
- PlayableMappingConfig cache=4281（映射表，运行时 > 历史文件版 370）
- TalentSkill cache=326（技能表）
- AutoAnimatorStates cache=22954 / 去重后 stringRecords=11624

`hero_catalog.json` 抽查：英雄中文名（土御门胡桃/宁红夜/迦南/天海/妖刀姬…）、
关联形态（伏魔金刚→大佛，RELATED_NON_MAIN_FORM）、中文技能名（庇护/庇护·增援/
净天地…）、连招图名（hero_onmyoji_ns_1_combo_graph…）全部正确读出。

## 读取链路（已验证）

```
CBinaryDesignDataReader 静态字段块 @ klass+0xB8（s_Inst 背引用校验通过）
  └─ s_Inst (单例)
      └─ 6 个 reader 字段（偏移固定：HeroBattle=0x1E38 / HeroFashion=0x1E90 /
         HeroTalent=0x1EF8 / PlayableMapping=0x28D8 / TalentSkill=0x3410 /
         AutoAnimator=0x990）
          └─ reader.m_Cache @ +0x18（Dictionary<,>，m_Key2Indexer/m_Buffer 惰性未物化）
              └─ Dictionary._entries @ +0x18（Entry[]，len=cache 容量）
              └─ Dictionary._count @ +0x28
                  └─ Entry[key@+0x08, value@+0x10, 步长 0x18] → 配置行对象
```

行对象字段偏移与当前 dump.cs 完全一致（_HeroTitle@0x18 / _HeroName@0x20 /
_RelateHeroID@0x40 / _Deprecated@0x58 / _HeroMappingID@0x30 …）。

## 注意事项

### ★ 验证/注入前必读：重复注入 DLL 会失败，需先重启游戏

- **重复注入同一 DLL 会导致第二次 DllMain/worker 与已加载模块冲突而卡住**。
  每次注入验证前必须先重启游戏进程到**登录界面**（英雄配置已物化的时机），
  再做一次干净首次注入。
- 判断是否已到登录界面的标志：窗口标题为 `Naraka`、工作集约 3.5GB、
  `GameAssembly_Super.dll` 已加载、进程正常响应。

### 自行重启游戏的命令（已核实启动入口）

游戏安装目录：`F:\yjwj_2025-12-28-11-00\Naraka\`。

启动链：顶层 `LauncherGame.exe`（游戏自带启动器，只进到启动器界面）；
**`program\StartGame.exe` 才是可直接启动游戏到登录界面的入口**（无命令行参数）。

```powershell
# 1) 结束当前游戏进程（含反作弊和启动入口，避免残留占用）
Stop-Process -Name NarakaBladepoint -Force -ErrorAction SilentlyContinue
Stop-Process -Name StartGame -Force -ErrorAction SilentlyContinue

# 2) 稍候，重新直接用 StartGame.exe 启动游戏到登录界面
Start-Sleep -Seconds 3
Start-Process -FilePath 'F:\yjwj_2025-12-28-11-00\Naraka\program\StartGame.exe' `
  -WorkingDirectory 'F:\yjwj_2025-12-28-11-00\Naraka\program'

# 3) 等待游戏进入登录界面（轮询 NarakaBladepoint.exe 出现且工作集达标）
#    随后即可用 Injector.exe <pid> <SuperDumper.dll> 做干净首次注入
```

相关可执行文件（`...\Naraka\program\`）：
- `NarakaBladepoint.exe`（4677632 B）— 被注入的目标游戏进程
- `StartGame.exe`（4174328 B）— 真正拉起游戏的入口（可直接启动，无参数）
- `NeacClient.exe` — NEAC 反作弊客户端
- `LauncherGame.exe`（顶层）— 游戏自带启动器，仅进启动器界面

> 注意：`NarakaBladepoint.exe` 的命令行被反作弊隐藏（CIM 查询为空），
> 属正常现象，不要据此判断进程未正常启动。

### 其它
- `PlayableMapping` 运行时 4281 条 > 历史文件版 370 条，属不同加载覆盖度，
  不能把历史表数据当当前全集（与 report.md 结论一致）。
- 探针 `HeroDictProbe.*`（`App/Il2CppRuntimeDumper/HeroDictProbe.cpp/.vcxproj`，
  输出 `C:\herodict_probe.log`）是诊断用独立 DLL，不是正式产物，可保留用于后续
  字典布局回归验证。