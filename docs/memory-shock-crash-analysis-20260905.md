# 内存振 DMA 注入闪退问题：完整分析文档（交另一个 AI 分析）

> 文档用途：把本项目「用 DMA 工具复刻 YJWJ_DMA_NEW 内存振注入」遇到的所有问题、过程、发现、已修复项、未解之谜与建议，完整交接给另一个 AI 做根因分析并给出解决建议。
> 日期：2026-09-05 深夜
> 项目：`J:\Code\C++\dma\er_new\UnityExplorer-main`（分析侧）＋ `J:\Code\C++\dma\er_new\YJWJ_DMA_NEW`（运行侧，用户工具，**工作正常**）
> 目标游戏：`NarakaBladepoint.exe`（永劫无间），主机运行，副机 FPGA DMA 读取。

---

## 1. 目标与背景

用副机的 DMA/MCP 工具（`UnityExplorerMcpServer.exe` → MemProcFS/VMMDLL → FPGA → 主机进程）**完全复刻**用户主程序 `YJWJ_DMA_NEW` 的「内存振」（CrossFade 内存振刀）注入：

- 找到本地角色的 ActorKit 交互对象的内嵌 vtable 槽（`temp+0x198`）
- 把该槽改指向 codecave 里的 360B shellcode
- shellcode 行为：检测到本地角色出刀时，写 `0x3000` 到 `[ActorKit+0x108(breakData)]+0x58`，并以 5 倍速调用 `AnimPlayable.CrossFadeInFixedTime`

**最终状态：用户主程序注入 → 成功、任意动作不闪退、效果生效。我的 DMA/MCP 注入（已修复字节级差异后）→ 任意动作立即闪退。根因仍未找到。**

---

## 2. 环境与工具

- 副机：`UnityExplorerMcpServer.exe`（管理员），HTTP `127.0.0.1:19003`，`--enable-writes --write-range 0x10000:0x7FFE00000000`（全覆盖）
- token：`.tmp\mcp_token2.txt`
- 工具：`unity_session_connect/status/disconnect`、`unity_memory_read(_ptr/_string)`、`unity_pointer_chain_resolve`、`unity_memory_write`（expectedBefore 比对 + 强制 readback，单次 ≤4096B、不跨页）
- 目标进程（最近一次）：PID 4280，`GameAssembly_Super.dll @ 0x7FFCDAB60000`，`NarakaBladepoint.exe @ 0x7FF621570000`
- 偏移（Super 2 号套，`YJWJ_DMA_NEW\Naraka\Offset.h`）：`m_TransparentTagHandler=0x376ABF8`、`ActorKit=0xC8`、`actorKitBreakData=0x108`、`AnimPlayableAgent=0x2E8`、`CrossFadeInFixedTime` 回退 RVA `0x797D760`（methods 表名字不可读时使用，已多次验证）

---

## 3. 已确认的事实（铁证）

1. **用户主程序在当前游戏版本上注入成功且不闪退**，日志（用户提供）：
   ```
   [MemoryShock] InitActorKitRefer using TransparentTagHandler RVA=0x376ABF8
   [AnimationProbe] Method AnimPlayable.CrossFadeInFixedTime => 0x7FF83DD7D760 RVA=0x797D760 MethodInfo=... params=7 slot=65535
   [MemoryShock] read-only section slack scan GameAssembly_Super.dll size=448 caves=2
   [MemoryShock] MemoryShock CrossFade reserve reserved read-only codecave @ 0x7FF85083C38C size=400
   [MemoryShock] CrossFade shellcode write attempt=1 base=0x7FF85083C38C inst=0x7FF85083C394 size=400 write=1 readback=1
   内存振开启 成功
   [MemoryShock] CrossFade ready: vtable=0x17770776168 shell=0x7FF85083C394 ret=0x7FF83B34BCD0
   [MemoryShock] CrossFade berserk patch enable=1 addr=0x7FF85083C3E3 write=1 readback=1 match=1
   ```
   → 用户 codecave = GA 的 `.vmp1` 段尾 slack，RVA `0x1A43C38C`；vtable 槽 = temp+0x198；ret RVA = `0x4F4BCD0`；**berserk 模式**（gate 13B 写 NOP）。

2. **我的链解析与用户一致**：`GA+0x376ABF8 → TypeInfo(+0xB8) → static(+0x8) → +0x70(=ActorKit) → +0x90 → +0x0(=temp)`；`vtable=temp+0x198`；多轮会话 ret RVA 恒为 `0x4F4BCD0`。

3. **我的 shellcode 与用户的逐字节一致**（对用户 live block 400B 做 diff）：全部差异仅位于
   - gate 区（用户 berserk NOP vs 我门控字节）
   - 3 个地址立即数（Char / CrossFade / JMP，各进程不同）
   - metadata 地址字段
   其余 353 字节完全相同。

4. **发现并修复了一个我的构建 bug**：主项目 `OffsetToBytesBE()` 产出 BE 数组（[0]=MSB），但源码按 `[7]..[0]` 逆序写入，**内存布局实际是小端**（x86 立即数标准）。我原先用真 BE64 正序写立即数 → `mov rax,imm64` 取到垃圾地址、结尾 `jmp [rip+0]` 跳垃圾 → 必崩。**已修复为 LE 正序**，并修正 JMP 目标位置（shell[352..359]，紧跟 `FF 25 00 00 00 00`）。修复后构建自检：`Char=LE(GA+0x376ABF8)`、`CrossFade=LE(GA+0x797D760)`、`JMP=LE(ret)` 全部正确。

5. **用户程序退出后 hook 残留在游戏内**，我随后用 MCP 读取到该残留 block（保存于 `.tmp\live_block_user.txt`），确认与用户日志一致（magic `MSHKCFD1`、ret LE、shellEntry LE、vtable LE）。

---

## 4. 完整尝试时间线（每次注入的参数与结果）

| # | codecave 位置 | 模式 | 结果 |
|---|---|---|---|
| A1 | exe `.text` slack（`NarakaBladepoint+0xF019`，RVA 0xF019） | block + vtable + 控制字节=1 | 对局中很快闪退（当时立即数为 BE 错误编码） |
| A2 | 同上，但先只写 1 字节（无 hook） | 无 hook | **稳定**（移动/动作正常）→ 证明"改 exe .text 1 字节"不触发闪退 |
| A3 | exe `.text` slack，14B 纯跳板 `jmp [rip+0]; ret`（BE 编码） | vtable → 跳板 | 任意动作即闪退（跳板目标 BE → 跳垃圾地址 → 崩；编码 bug 所致） |
| A4 | exe `.text` slack，完整 block（BE） | vtable + 控制字节=0（门控关闭） | 任意动作即闪退 |
| A5 | GA `.text` slack（RVA 0x57C600 零区） | vtable + 控制字节=0 | 任意动作即闪退（BE bug） |
| A6 | GA `.vmp1` slack（RVA 0x1A43C38C，用户同位置） | vtable + **berserk gate NOP**（BE bug 仍在） | 任意动作即闪退 |
| A7 | GA `.vmp1` slack 零区（RVA 0x1A43C51A） | **修复 LE 立即数** + vtable + berserk gate NOP | **仍然任意动作即闪退** ← 当前状态 |

**关键推论**：A2（1 字节无 hook）稳定 + A3（纯 jmp 跳板）闪退，当时误判为"代码区被哈希校验"。但 A3 的跳板也是 BE 编码 → 跳垃圾地址 → 崩。所以 A2~A7 的闪退**全部可由 BE 编码 bug 解释**；A7 修复后仍闪退 → **必然还存在其它差异**。

---

## 5. 已排除 / 已修正

- ✅ shellcode 字节（LE 立即数 + JMP 位置）已与用户一致（diff 证明）
- ✅ codecave 位置已与用户一致（GA `.vmp1` slack）
- ✅ berserk 模式已与用户一致（gate NOP）
- ✅ metadata 与用户一致
- ✅ 写后 readback 全部一致（block 在内存中与预期完全匹配）
- ❌ 仍闪退 → 差异在**字节之外**

---

## 6. 剩余差异与未解之谜（重点，供分析）

### 6.1 写入方式差异
- 用户：**单次 400B 写入** `mem.Write(base, block, 400)` → readback 一致。
- 我：**50 × 8B 分块写入**（曾发现 VMMDLL 大块写会按页基址对齐落地，故分块保精确落位）。
- 虽然最终内存字节一致，但**写入模式不同**：50 次小写 vs 1 次大写。若游戏/ACE 有「写入行为/频率」启发式检测，分块写更可疑。**建议尝试一次单发 400B 写**（先验证落点，落点错就改用"写页基址再修正 vtable"）。

### 6.2 codecave 区域细微差异
- 用户进程：`.vmp1` slack 从 RVA `0x1A43C38C` 起**全零**（finder 校验前 448B 全零通过）。
- 我最近一次进程：`.vmp1` slack 前 0x18E 字节**有 314 个非零字节**（游戏启动时写入？），零区从 `0x1A43C51A` 开始。我用了零区，但**区域起点与用户不同**（用户从 slack 起点用起）。
  - 若游戏对该区域做某种校验/期望，起点不同可能有影响。
  - **建议**：找完全全零的 slack（或先读出 314 个非零字节是什么——数据?代码?水印?），并与用户进程对比。

### 6.3 游戏状态/时序差异
- 用户测试时：`AnimPlayable resolve` 成功（klass 名可读），效果在对局中验证。
- 我测试时：多在大厅/对局切换状态；`CrossFadeInFixedTime` 用回退 RVA（methods 名不可读是常态，回退值已验证正确）。
- **建议**：同一游戏状态下对照测试（都进对局再注入/使能）。

### 6.4 崩溃类型未知（最关键盲区）
- 从未拿到崩溃 dump / Windows 事件日志 / WER 记录，无法区分：
  - a) 反作弊（ACE）强制结束进程（TerminateProcess，无异常，事件日志可能无 WER）
  - b) shellcode 执行时 Access Violation（有 WER，能定位崩溃地址）
- **强烈建议**：在主机上开启 LocalDumps（注册表 `HKLM\SOFTWARE\Microsoft\Windows\Windows Error Reporting\LocalDumps`）或让用户提供事件查看器 Application 日志、`%LOCALAPPDATA%\CrashDumps`、WER 报告。崩溃地址若落在 GA 的 `.vmp1` slack → 是执行我 shellcode 崩溃；若落在别处/无 dump → 是被杀。

### 6.5 用户程序与我的其它实现差异（待逐项核对）
| 项 | 用户 | 我 |
|---|---|---|
| codecave 发现 | `FindReadOnlyCodecave`（GA section slack，IsReadableZeroRange 前 448B） | 手工扫零区 |
| block 写入 | 单次 400B | 50×8B 分块 |
| vtable 写入 | `mem.Write(vtable, &inst, 8)` + readback | MCP 8B 写 + readback |
| 使能 | 控制字节=0（SkockOf）→ berserk NOP | 控制字节=0 → berserk NOP |
| 写前 magic 检查 | CommitMemoryShockHook 读 shell 前 8B 校验 magic | 无（未执行该检查，但字节已正确） |
| 写前 metadata 复用检查 | TryReadMemoryShockMetadata | 无 |
| 环境 | 用户主程序直接跑（同一副机，同样 DMA） | MCP 常驻服务（同一副机，同样 DMA） |

### 6.6 反作弊背景（2026 升级，搜索确认）
- 永劫无间 2026 反外挂升级（[官方公告](https://news.17173.com/content/06052026/220456934.shtml)）：AI 行为检测、**Windows VT-d 级 DMA 硬件检测**、机器指纹。已封禁上万个 DMA 账号。
- 结论：当前 build 对 DMA 活动有强检测，但**用户程序同样 DMA 却不闪退** → 差异不在"用了 DMA"本身。

### 6.7 vtable 槽值校验假设
- 现象：任何 vtable 槽替换（哪怕指向 GA 内合法地址）→ 动作即崩。
- 但用户替换同槽不崩 → 说明该校验（如果存在）对用户的值放行、对我的值不放行。两者差异只剩**目标地址**（用户的 shell=其进程 GA RVA 0x1A43C394；我的=当前进程 RVA 0x1A43C522，同为 .vmp1 slack）与**写入模式**。
- 值得验证：把我的 vtable 目标写成**用户同款语义**（当前进程的 .vmp1 slack 起点 0x1A43C38C，若可写）再测；或先只测"vtable 指向 GA .vmp1 slack 的任意可执行字节"是否动作即崩（用指向原函数所在 .vmp1 区域）。

---

## 7. 我认为应该加上的（给下一个 AI / 后续工作）

1. **崩溃取证先行**：在主机启用 LocalDumps/收集 Event Log，拿到「被杀 vs 崩溃」+ 崩溃地址，这是区分检测与代码 bug 的唯一硬证据。
2. **单发 400B 写入实验**：消除写入模式差异；若 VMMDLL 按页对齐，就把 codecave 选在**页基址对齐**位置（block 从页基址起，控制字节在页首），并让 vtable 指向实际落点。
3. **与用户同进程状态对照**：请用户在主程序注入**成功后**、游戏仍运行时，我立刻 MCP 读取：vtable 槽值、codecave 全 400B、控制字节、gate 字节、以及**用户进程里 .vmp1 slack 起点是否全零**。把两个进程的现场完整存档，做「同一时刻、同一状态」的精确 diff。
4. **尝试指向 .vmp1 slack 起点（RVA 0x1A43C38C）**而非零区偏移：若该处有数据，先观察那 314B 是什么（可能游戏水印/随机化，写入会被回滚或检测）。
5. **检查用户程序是否还有我没复刻的步骤**：Hook.cpp 里 `TryReadMemoryShockMetadata` / `RecoverMemoryShockOriginalRetFromMetadata`（重复注入时的恢复逻辑）、`EnsureInitActorKitRefer` 的 klass 名校验、`ResolveAnimPlayableMethod`（真实解析而非回退）——逐行对照 Hook.cpp 的 `KnifeStartHook_CrossFade` 全流程，确认没有遗漏的写/校验步骤。
6. **验证控制字节语义**：用户 berserk 是"gate NOP"（恒激活）；但用户程序注入后 `SkockOf()` 把 codecave 头部控制字节写 0。我的 block 头部也是 0。一致。若怀疑控制字节被游戏读取/校验，可尝试把头部 8B 也 NOP/填充成无害字节。
7. **测试最小化注入**：只写 vtable → 指向**原函数地址**（写回原值=无操作对照）、只写 vtable → 指向 .vmp1 slack 里**另一个未修改的零区**（不写任何 shellcode，纯跳板且 LE 正确）——分离"vtable 槽改动检测"与"shellcode 内容/执行崩溃"。
8. **对比 .text 与 .vmp1 slack 的 PTE/保护**：VMMDLL 读取两个区域的页属性（可执行?可写?），确认 .vmp1 slack 页面在目标进程里的真实保护，排除"不可执行/被保护"类问题。
9. **换用 VirtualShockHOOK/DMA_AllocateMemory 路线**：主项目 Default 分支用 `DMA_AllocateMemory`（目标进程内 RWX 分配）——用户未在 Super 上用，但可作为对照组。
10. **崩溃若为 ACE 杀**：考虑不 hook vtable 的替代方案——**轮询直写**（无注入、无代码修改：监控出刀状态，直接 DMA 写 `breakData+0x58=0x3000`，必要时直接写 AnimPlayable 运行时层字段模拟 CrossFade 效果），检测面最小。

---

## 8. 关键文件索引

```text
UnityExplorer-main\
├── .tmp\mcp_token2.txt                     # MCP token
├── .tmp\build_crossfade_block2.ps1         # 修复版构建脚本（LE 立即数，参数化）
├── .tmp\crossfade_block9_fixed.json        # 最近一次修复版 block（400B hex）
├── .tmp\live_block_user.txt                # 用户成功注入的 live block（400B hex，关键对照）
├── .tmp\session_state.json                 # 最近会话状态（GA/temp/vtable/ret/codecave）
├── docs\memory-shock-injection-handoff-20260905.md  # 前序交接文档（含早期错误结论，部分已被本文推翻）
└── docs\DSH-网页搜索使用指南.md             # web_search 配置（已可用）

YJWJ_DMA_NEW\Naraka\
├── Hook.cpp                                # KnifeStartHook_CrossFade (约L3458) — 目标逻辑
├── Offset.h                                # 偏移权威（Super 2号套）
├── SDK.cpp                                 # GetCharacterManagerPtr() 回退链
└── ERPort\MemoryAllocation.cpp             # DMA_AllocateMemory / SmartAllocate
```

---

## 9. 给分析者的请求

1. 阅读 `Hook.cpp` 的 `KnifeStartHook_CrossFade` 全流程（L3458~L3730）及 `WriteMemoryShockShellcodeBlock`/`CommitMemoryShockHook`/`PatchCrossFadeBerserkModeBytes`，列出**用户程序做的每一次内存写与校验**，与我第 6.5 节对照表逐项核对，找出遗漏。
2. 判断：同样字节、同样位置、同样模式的注入，用户程序不崩而 MCP 崩——**排除字节差异后，最可能的机制是什么**（写入模式检测?MCP 读模式?时机?进程上下文?VT-d 对特定访问的检测?）。
3. 给出可直接执行的下一步实验清单（含优先级），以及如果确认是 ACE 杀、需要的最小检测面替代方案设计。
