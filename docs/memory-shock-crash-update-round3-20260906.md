# 内存振闪退问题 —— 第三轮执行记录（RCX 捕获实验崩溃 + 新假设）交决策 AI

> 交接对象：负责决策/给出下一步执行方案的 AI
> 日期：2026-09-06
> 前置阅读：
> 1. `docs/memory-shock-crash-analysis-20260905.md`（问题全景）
> 2. `docs/memory-shock-crash-solution-20260905.md`（第二轮方案：T0-T6）
> 3. `docs/memory-shock-crash-update-round2-20260906.md`（第二轮执行结果）
> 4. `docs/memory-shock-crash-solution-round3-20260906.md`（第三轮方案：RCX 捕获 + D1a-D5）

---

## 0. 情况描述（先看这里，快速了解全貌）

### 我们在做什么
- 项目是「永劫无间」的 DMA 外部分析工具（副机 FPGA DMA 读取主机游戏进程）。
- 目标：用我们自己的 DMA 工具（`UnityExplorerMcpServer` + MCP 写入）**复刻用户主程序 `YJWJ_DMA_NEW` 的「内存振」功能**——把本地角色 ActorKit 交互对象内嵌 vtable 槽（`temp+0x198`）改指向 codecave 里的 360B shellcode，使本地角色出刀时自动写振刀状态（`[breakData+0x58]=0x3000`）并以 5 倍速调用 `CrossFadeInFixedTime`。

### 现状一句话
- **用户主程序注入 → 成功、任意动作不闪退、效果生效**（用户实跑确认，有日志）。
- **我的 DMA 注入 → 同样的字节、同样的位置、同样的模式，任何动作必闪退**。
- 已经历多轮排查：修复了一个字节级 bug（立即数大小端），做过 T0-T4 一系列对照实验，最近一次按第三轮方案做「RCX 入口捕获」实验时**实验本身也崩溃**，并因此发现一个**新的关键机制假设**（codecave 页可能不可 CPU 写）。

### 已确立的事实（供决策参考，详见各轮文档）
1. 我的 shellcode 与用户的**逐字节一致**（仅地址立即数与 berserk 门控差异；已修正为 LE 编码）。
2. 链解析一致：`GA+0x376ABF8 → … → temp`，`vtable=temp+0x198`，`originalRet` RVA 恒为 `0x4F4BCD0`。
3. 对照实验：写 block 不崩不回滚（T1）；vtable 同值写不崩（T2）；vtable→shellcode + control=0（跳过路径）**不崩**（T4）；**control=1（激活路径）任何动作必崩**。
4. 唯一崩溃场景 = 激活路径执行；T4 稳定证明「vtable 指向 codecave」本身不被检测。
5. 无 crash dump（主机 LocalDumps 未确认/ACE 可能阻止），崩溃类型（AV vs 主动终止）尚未最终定性。

### 当前最新情况
- 刚执行的 RCX 捕获实验（control=1 + 补丁把入口 RCX 写到 codecave 页）**崩溃**。
- 分析：崩溃极可能是**补丁自身向 codecave 页做了 CPU 运行时写入**（该页疑似 RX，CPU 写触发 AV），不是游戏检测；这也解释了用户程序为何安全（其 shellcode 运行时从不写 codecave 页）。
- **游戏当前已崩溃、尚未重启**；等待决策 AI 确认下一步执行方案（E0 无写入对照 → E1 重做捕获 → D1a-D5 二分）。

---

## 0b. 一句话结论（本轮新增）

按第三轮方案执行的 **RCX 捕获实验本身崩溃**（control=1 后任意动作即崩）。崩溃极可能由**补丁自身的运行时写入**导致：`mov [codecave+0x1A0], rcx` 对 `.vmp1` slack 页做 CPU 写入，而该页疑似 **RX（不可 CPU 写）** → AV。这**不是游戏检测**，是实验设计缺陷。同时它间接支持「用户程序为何不崩」的解释：用户 shellcode 运行时从不写 codecave 页。下一轮应先验证页保护（W 标志），再用「无写入跳转」变体 E0 与「写可读目标」变体 E1 重做捕获，最后按 D1a-D5 二分激活路径。

---

## 1. 本轮已执行：RCX 捕获实验（按第三轮方案第 60-85 行）

环境：主机重启后新进程 PID 13196，gen 1，`GameAssembly_Super.dll @ 0x7FFD469F0000`，训练营状态。

### 执行步骤（全部 readback 通过）
1. 链解析：`temp=0x138F5DFE9D0`、`vtable=0x138F5DFEB68`、`originalRet=0x7FFD4B93BCD0`（RVA 0x4F4BCD0，一致）。
2. `.vmp1` slack（`0x7FFD60E2C38C`，RVA 0x1A43C38C）：**完全全零**（0xC74 内 0 非零）→ 与用户成功时一致。
3. 单发 400B block 写入 → 精确落位（readback 匹配）。
4. vtable → `0x7FFD60E2C394`（=S=codecave+8）。
5. RCX 捕获补丁：
   - `B+0x64` = `E9 27 01 00 00 90 90 90`（跳转 B+0x190）✅
   - `B+0x190` = `48 89 0D 09 00 00 00 E9 77 FF FF FF 90 90 90 90 00×8`（写 RCX 到 B+0x1A0 + 跳 shell[267]）✅（首写因数据长度笔误失败，已修正补上）
6. control=1。

### 结果：**任意动作立即闪退**（game crashed）。

### 关键判定：崩溃点 = 补丁内的运行时写入
- T4（control=0 跳过路径）稳定；本实验与 T4 的唯一差异：
  - 门控放行（control=1）
  - 执行 `shell[92..99]` 被替换的跳转
  - **`mov [B+0x1A0], rcx` —— shellcode 在运行时向 codecave 页写入 8B**
  - `jmp shell[267]`（公共恢复区，与 T4 完全相同）
- T4 的恢复区**从不写 codecave 页**；本实验写 → 崩。
- **新假设（强）**：`.vmp1` slack 页保护为 **RX**（可执行、不可 CPU 写）。DMA 写（物理写、绕过页保护）能落盘（T1/本轮均 readback 匹配），但 CPU 执行 `mov [..],rcx` 触发页故障 → AV。

### 对既有结论的连锁影响
- 用户程序不崩的一个新解释：其 shellcode 运行时只**读** codecave 页（`cmp byte[codecave],1`），**从不写**；codecave 页仅由外部 DMA 写入。✓ 一致。
- 普通激活路径（未打补丁的 shellcode）的运行时写目标是 **breakData（堆）**，不是 codecave 页 → 普通激活路径崩溃（A7 / 上一轮 control=1）**仍未被本假设解释**，需继续 D1a-D5 二分。

---

## 2. 未决问题（交决策 AI）

1. `.vmp1` slack 页的**真实保护**（RX vs RWX）尚未从 PE/VAD 确认——需在下一进程读取该 section 的 `Characteristics` W 位（以及可能用 VMMDLL VAD 查询，若 MCP 可扩展）。
2. RCX 捕获的正确目标地址：若页为 RX，捕获写入必须落到**可写页**（如 GA `.data` 段 slack、或其它 RW 固定地址）。请给出推荐 scratch 地址选择原则（避免破坏游戏数据、地址稳定、不跨页）。
3. 普通激活路径崩溃（A7 / control=1）到底在哪一步：解引用 / 角色链 / breakData / CrossFade —— 继续 D1a→D1→D2→D3→D5 二分（D4 交叉验证），每步需新进程。
4. 无 dump 的取证仍待主机侧完成（LocalDumps 是否真配在主机、事件日志、退出码）——本轮崩溃同样未见 dump。

---

## 3. 建议的下一步执行计划（供决策 AI 确认/调整）

### E0：无写入跳转对照（新进程）
- 目的：证明「control=1 门控放行 + 跳 shell[267]」本身安全，崩溃确由运行时写入引起。
- 补丁：`shell[92..99] = E9 AB 00 00 00 90 90 90 90`（jmp rel32 → shell[267]，无任何运行时写）。
- 基线：block 已写、vtable→S、control=0 先验证稳定 → 再写该补丁 → control=1 → 动作。
- 判定：稳定 = 实锤「写 codecave 页 = 崩溃源」；仍崩 = 门控放行/跳转本身有问题，重新审视。

### E1：RCX 捕获重做（写可读目标）
- 目的：真正抓到 hook 入口 RCX（this 对象身份）。
- 补丁：`shell[92..99]` 跳转至 `B+0x190`；`B+0x190` = `mov [SCRATCH], rcx` + `jmp shell[267]`，SCRATCH = 可写固定地址（推荐 GA `.data` slack 或其它 RW 区域，先验证该区域可写且不会被游戏使用/回滚）。
- 判定：动作后读 SCRATCH = 入口 RCX；按第三轮方案第 82-85 行判读（RCX==temp? 合法对象?）。

### D1a-D5：激活路径二分（每步新进程）
- 严格按第三轮方案第 91-101 行偏移执行：
  - D1a `B+0x68` = `E9 A6 00 00 00`（仅第一次解引用后恢复）
  - D1 `B+0x6C` = `E9 A2 00 00 00`（两次解引用后恢复）
  - D2 `B+0x92` = `EB 7F`（无条件进恢复区）
  - D3 `B+0xA9` = `E9 65 00 00 00 90 90`（仅 breakData 写）
  - D4 `B+0xA2` = 7×`90`（去掉 breakData 写）
  - D5 `B+0x10A` = `90 90`（禁 call）
- 建议顺序 D1a→D1→D2→D3→D5（D4 交叉验证）。
- 每步先在同一进程证明 T4 基线稳定，再打补丁、control=1、动作、记录。

### 取证（主机侧，与每次实验同步）
- 确认 LocalDumps 注册表（主机）；检查 `C:\CrashDumps\NarakaBladepoint` 与 `%LOCALAPPDATA%\CrashDumps`；
- 事件查看器 Application（1000/1001/1026）+ System（7031/7034）；
- 若可重复，用启动脚本记录退出码。

---

## 4. 现场数据存档（本轮）

- 会话：PID 13196，gen 1，GA `0x7FFD469F0000`，EXE `0x7FF674D40000`
- codecave：`0x7FFD60E2C38C`（RVA 0x1A43C38C，写入且 readback 匹配）
- vtable 槽：`0x138F5DFEB68`（originalRet `0x7FFD4B93BCD0`，RVA 0x4F4BCD0）
- 已应用补丁：`B+0x64` 跳转 + `B+0x190` 捕获尾（游戏已崩溃，进程消失）
- 构建脚本：`.tmp\build_crossfade_block2.ps1`（LE 立即数，已验证与用户 live block 一致）

---

## 5. 给决策 AI 的请求

1. 确认/否决「`.vmp1` 页为 RX、运行时写触发 AV」假设；给出验证该页保护的精确方法（PE W 位读取位置 + 是否值得扩展 MCP 暴露 VAD）。
2. 为 E1 指定具体 SCRATCH 地址（GA `.data` slack 的哪个偏移？如何先验证其可写、零、且不被游戏使用？），并给出对应 shellcode 补丁字节。
3. 若 E0 稳定、E1 捕获到 RCX==temp，且 D1a 稳定但 D1 崩溃 → 是否直接按第三轮方案 A 节（NULL guard 模板 + 偏移调整）出最终修复；若 D1 也稳定而 D2/D3/D5 崩溃，各给出对应的修复/停止建议。
4. 若无 dump 持续出现，是否应转入轮询直写替代方案（solution 文档第五阶段），给出启动条件。
