# MCP 只读验证报告（2026-09-05）

> 副机经 `UnityExplorerMcpServer.exe`（MCP Streamable HTTP, 127.0.0.1:19002）→ MemProcFS/FPGA DMA → 主机 `NarakaBladepoint.exe`。
> 全程只读，未启用写入。目标进程实时运行中。

## 0. 会话快照

- 目标进程：`NarakaBladepoint.exe`（远端 PID 9436）
- Runtime：`il2cpp`
- 模块数：182
- Generation：1
- `NarakaBladepoint.exe`     @ `0x7FF774EC0000`
- `GameAssembly_Super.dll`   @ `0x7FFABC2C0000`  ← **Super 变体，使用 Offset.h 2 号偏移套**
- `UnityPlayer_LVB.dll`      @ `0x7FFAD6700000`

## 1. Manager 链验证（Super 偏移套，全部只读）

| Manager | RVA (2号套) | 链 | 结果 |
| --- | --- | --- | --- |
| CharacterManager 主链 | `0x374C2B8` | slot→TypeInfo→static+0x8 | ❌ `0x300000003`（tag 值，主链失效，与 09-03 记录一致） |
| **TransparentTagHandler 回退** | `0x376ABF8` | slot→TypeInfo→static+0x8→+0xD8 | ✅ `0x152CA8D1850`（有效 CharacterManager） |
| CharactorSync | `0x374C368` | slot→TypeInfo→static+0x8 | ✅ `0x152B9E66C80` |
| BuffManager | `0x374AE20` | slot→TypeInfo→static+0x0 | ✅ `0x15358E5EE00`（recordBuffHandlersDict=0x1535F7CEA20） |
| UserDataManager | `0x3772EA8` | slot→TypeInfo→static+**0x0** | ✅ `0x15364937800`（static+0x8 为 0，需用 +0x0） |
| EntityManager | `0x3755DD0` | slot→TypeInfo→static+0x8 | ✅ `0x152BB28A700` |
| GlobalTime | `0x3755D18` | slot→TypeInfo→static+0x8 | ✅ 实例 `0x151EBE212D0`（但 `_globalTime@+0x28` 读出非合理 double，偏移待校准） |
| Base_WindowBase2 | `0x3764380` | slot→TypeInfo(0x151ECB93528)，static 不在 +0xB8 | ⚠️ 未解析出宽高，需专用验证器 |

## 2. 角色列表（CharacterManager +0x50 → +0x18 → arr；arr+0x18=count，+0x30+i*0x18=actor）

- 存活角色数：**3**
- actor[0] `0x152E80BA540`（本地）：名字 `tspro`，Heroid `1000003`，Group `1`，WeaponId `3200330`，WeaponType `102`，visible `1`
- actor[1] `0x152E8449540`：名字 `颜家俊战神`，Heroid `1000007`，Group `99`，visible `1`
- actor[2] `0x153C489D000`：名字 `岁月沧海`，Heroid `1000007`，Group `99`，visible `0`

名字均为 IL2CPP 托管字符串（对象+0x10 长度、+0x14 起 UTF-16 字符）读取。

## 3. 其他字段

- CharactorSync.shrtt(+0x68) = `0.006921171` → ping ≈ 13.8 ms
- Local ActorModel：PropertyData(+0x358)=0x1535E55C540、RuntimePropertyData(+0xa8)=0x1541B1DB380、ActorWeapon(+0xf8)=0x1534EAFAD10、ActorKit(+0xc8)=0x152BB513A80、cullingVisible(+0x6c)=1
- PropertyData：Heroid(+0x108)=1000003、Group(+0x118)=1、WeaponId(+0x128)=3200330、RobotId(+0x130)=0、PlayerName(+0x200)="tspro"

## 4. 与 Offset.h 对照结论

1. 当前加载 `GameAssembly_Super.dll`，2 号偏移套全部命中，DMA 读取链有效。
2. CharacterManager 主链仍失效（`static+0x8 = 0x300000003`），必须走 TransparentTagHandler 回退链（与 `YJWJ_DMA_NEW\Naraka\SDK.cpp` `GetCharacterManagerPtr()` 现有实现一致）。
3. **UserDataManager2 的 static 实例偏移是 +0x0 而非 +0x8**：`SDK.cpp` 若以 `static+0x8` 读取 UserDataManager 需要复核（本项目只读验证，未改主项目代码）。
4. GlobalTime `_globalTime` 字段偏移 +0x28 在当前 build 读出非合理 double，需校准；WindowSize 类静态字段不在 +0xB8，需专用验证器。

## 5. 操作记录

- 启动：`UnityExplorerMcpServer.exe --transport http --bind 127.0.0.1 --port 19002 --token codex-session-*`（管理员）
- MCP 调用：unity_session_connect / unity_session_status / unity_modules_list / unity_memory_read / unity_memory_read_ptr / unity_memory_read_string / unity_pointer_chain_resolve
- 全部为只读；服务保持运行，未 disconnect。
