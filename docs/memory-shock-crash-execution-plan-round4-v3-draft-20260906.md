# 内存振闪退问题：第四轮执行计划与操作 AI 约束（round4-v3 修订草案）

日期：2026-09-06
文档角色：项目经理执行文档
适用对象：负责实际 MCP/DMA 实验的操作 AI
前置记录：`docs/memory-shock-crash-update-round3-20260906.md`
草案状态：**DRAFT**——本草案含 F1–F10 修订，待项目经理评审后定稿；定稿前执行以已发布版本为准。

## 修订记录（F1–F10）

| 编号 | 严重度 | 修订位置 | 修订内容摘要 |
|---|---|---|---|
| F1 | 高 | 强制操作协议 §5 不可变实验账本 | 定义 `planRecordSha256` 规范化哈希范围（仅覆盖计划记录正文、不含 hash 行本身），解决自引用矛盾 |
| F2 | 高 | Gate G1 判定 | 明确 E0 非 PASS（CRASH/EXIT_UNKNOWN）⇒ 所有写实验（含 E1）阻塞，仅允许 G0 级只读采集与主机取证 |
| F3 | 中 | Gate G4 表格 D2 | 表意与 `EB 7F` 字节语义对齐（无条件跳过中段链）；标注待 PM 确认 |
| F4 | 中 | Gate G3 判定 | 补充"合法 RCX"判据谓词（非零、已映射可读、与 temp/vtable 可区分） |
| F5 | 中 | Gate G1 执行顺序 | 固定动作脚本执行 3 次、每次后观察 30s，与 G0 稳定性口径一致 |
| F6 | 低 | 强制操作协议 §5 计划记录字段 | 必填字段补充 control 的 address/length/before/data |
| F7 | 低 | 强制操作协议 §4 日志与证据包 | 新增"交接协议"小节：回执/交付载体、交付清单、双方 hash 核对 |
| F8 | 低 | 授权与回执协议 | 补充 approvalId 生命周期：绑定 planVersion，版本变更即全部失效 |
| F9 | 低 | Gate G4 二分判定规则 | 定义 dump 不可得时次级证据组合（退出码+事件 ID+时间戳对齐）的采信条件 |
| F10 | 低 | Gate G0 / Gate G5 | 补充 360B 压缩方案与 T4 block（448B / B..B+0x1A7）尺寸换算说明 |

修订标记约定：`【v3新增】` 为新插入内容；`【v3修订】` 为对原文的替换/修改，修改处均保留上下文以便核对。

---

## 文档控制与当前门禁

```text
planVersion: round4-v3-draft（定稿后改为 round4-v3）
planOwner: 项目经理 AI
operatorRole: 操作 AI（仅执行已批准步骤）
currentGate: CLOSED
allowedWhileClosed: 只读采集、主机证据归档、实验日志补全
forbiddenWhileClosed: 任何目标内存写入、patch 替换、scratch 选址、下一实验启动
```

【v3新增 F8】approvalId 生命周期规则：每个 approvalId 绑定签发时的 `planVersion`；若 `planVersion` 变更（含本草案定稿为 round4-v3），所有未回传最终状态的 approvalId 自动失效，必须由项目经理重新签发。历史账本不因版本变更而改写。

`currentGate=CLOSED` 的原因是：主机取证尚未完整、codecave 的 CPU 写保护未确认、没有项目经理批准且可回滚的 scratch。只有本文件中对应 gate 的所有证据齐全后，项目经理才会把下一实验 ID 标记为 `APPROVED`。

第三轮记录中"codecave 页为 RX、捕获写入导致 AV"只能标记为**待验证假设**。它解释了 RCX 捕获实验，但尚无 VAD/PTE 或 faulting IP 证据；禁止使用"实锤""必然""已确认根因"描述该假设。

## 角色与权限边界

| 角色 | 允许事项 | 禁止事项 |
|---|---|---|
| 项目经理 AI（唯一批准人） | 编写计划、批准实验 ID、评审证据、开关 gate、定义判定与停止条件 | 执行 MCP/DMA 写入、临时调试目标进程。 |
| 操作 AI | 严格执行当前 `APPROVED` 的实验 ID、采集证据、回滚、追加实验账本 | 修改地址/字节/顺序、选择 scratch、合并变量、根据猜测启动下一实验。 |
| 主机证据收集者 | 采集 Event Log、LocalDumps、退出码、dump 与时间戳 | 用缺失日志推断不存在异常。 |

每次异常、崩溃、`STOPPED` 或 `INCONCLUSIVE` 后，操作 AI 只能完成证据采集与交接；必须等待项目经理给出新的 `APPROVED` 实验 ID，不能自行重试或推进。

## 授权与回执协议

`currentGate=CLOSED` 时，没有任何隐式、历史或口头授权。操作 AI 只有收到项目经理给出的、完整匹配下列结构的单条批准记录后，才能开始一个实验；未收到记录等同于 `DENIED`：

```text
approvalId: R4-<experimentId>-<utc>
planVersion: round4-v3（草案定稿后生效；定稿前以已发布版本为准）
decision: APPROVED
experimentId: <唯一允许的 E0 / E1 / D1a ...>
preconditionEvidence: <已审计证据目录及 hash>
allowedWrites: <地址、长度、expectedBefore、data、rollback 的已审计摘要>
actionScript: <固定动作脚本和观察时长>
stopOn: <本计划的停止条件引用>
```

【v3新增 F8】生命周期与失效规则：
- approvalId 绑定签发时的 `planVersion`；`planVersion` 变更 ⇒ 所有未完成 approvalId 自动失效，需重新签发；
- 任何"重新解释地址、改字节、补做验证或重复实验"的情况均使该批准失效（原文已有），补充：该失效**不可通过口头延续**，必须由项目经理重新签发新的 `approvalId`。

操作 AI 的首条回执只能是 `STARTED` 或 `STOPPED`，并附账本路径和已核对的 `approvalId`。实验结束后只能回传 `COMPLETE`、`CRASH`、`EXIT_UNKNOWN`、`STOPPED` 或 `INCONCLUSIVE`，以及证据目录和账本 hash。任何需要重新解释地址、改字节、补做验证或重复实验的情况，均使该批准失效，必须由项目经理重新签发新的 `approvalId`。

【v3新增 F7】回执与交接载体：操作 AI 的首条回执与最终状态回传，通过账本文件与证据目录交付，并在回执行消息中引用核对 `approvalId` 与账本 `planRecordSha256`；项目经理以该引用为准收单（交接细则见"强制操作协议 §4 日志与证据包"）。

## 项目经理结论

当前只接受以下事实：

1. T0、T1、T2、T4 通过；`control=1` 是唯一已知的退出触发条件。
2. RCX 捕获实验本身失败，但不能据此认定 `.vmp1` 页为 RX。捕获补丁向 codecave 页执行了 CPU 写入，实验设计同时引入了一个新的写保护变量。
3. 现有资料没有 crash dump、faulting IP、主机事件日志或可靠退出码。因此不得写"已确认 ACE"，也不得写"已确认 NULL 解引用"。
4. 下一轮必须先验证实验工具自身没有引入新的运行时写入，再按 active path 做单变量二分。

操作 AI 的任务是执行和记录，不负责临时改变实验设计。任何地址、字节、顺序或停止条件与本文件不一致时，先暂停并回报，不得自行猜测。

## 强制操作协议

### 1. 实验隔离

- 每个实验使用全新的游戏进程和新的 `generation`。
- 进程重启后重新读取 GA、EXE、codecave、`temp`、vtable、`originalRet`、CrossFade VA；禁止复用旧进程地址。
- 每个进程只执行一个实验编号。不得在同一进程连续尝试 E0、E1、D1a 等多个变量。
- 每次实验先建立 T4 基线：block 完整、vtable 指向 `S=B+8`、control=0、原始 gate 未改写；基线不稳定即停止该进程。
- 实验结束时按"control=0 -> 恢复 vtable -> 恢复 block/补丁"顺序回滚。进程已退出时不得向该进程继续写入。

### 2. 写入纪律

每一次读写都必须在日志中记录：

```text
timestamp_utc, experiment_id, pid, generation,
GA/EXE base, codecave B, shell S, temp, vtable,
originalRet, CrossFade VA, address, length,
expectedBefore, data, backend result, readback, elapsed_ms
```

【v3新增 F6】上述记录项中的 `control` 写入同样必须记录，且 control 的 address/length/expectedBefore/data 同时进入实验账本计划记录（见 §5 字段清单）。

- 所有写入使用当前 `generation` 和 `expectedBefore`。
- 写入前先读取并保存原字节；readback 不匹配立即停止，不允许自动重试覆盖。
- 任何补丁都必须在同一 4 KiB 页内，并记录 shell offset 与 block offset 的换算。
- 禁止修改 berserk gate；本轮只使用原始 13B gate 和 `control=1`。
- 禁止向 RX/未知保护页做运行时记录。DMA 写入成功不等于目标 CPU 可写。

### 3. 不确定性停止规则

出现以下任一情况，操作 AI 必须停止当前实验，只提交日志和问题，不得自行选择替代方案：

- `generation`、PID、GA、codecave、temp 或 vtable 在基线采样中不一致；
- expected-before 与实际值不一致；
- patch 长度、目标位移、block offset 无法由文档直接核算；
- readback 失败、部分匹配、跨页或地址保护未知；
- 游戏退出但没有确认退出时间、实验阶段和最后一次写入；
- 需要选择新的 scratch 地址、新的 fallback RVA 或新的 shellcode 模板；
- 需要把两个实验合并到同一进程，或需要改变本文件的顺序。

暂停报告必须包含：`STOP` 标记、实验编号、最后成功步骤、最后一次读写、已知/未知项、等待项目经理决定的问题。

### 4. 日志与证据包

每个实验完成后立即生成独立目录：

```text
.tmp/memory-shock-round4/<experiment_id>/<timestamp>/
```

至少保存：

```text
manifest.json             # 进程、generation、所有地址和实验参数
timeline.jsonl             # 每一条读写和动作事件，按时间排序
before_after.json          # patch 前后字节、完整 block hash
protection.txt             # section/VAD 保护证据；无法取得则写 UNKNOWN
host-telemetry.txt         # 主机事件、LocalDumps、退出码结果
result.md                  # 只写观察事实、判定和未决问题
```

日志不得保存 MCP bearer token。时间统一使用 UTC ISO-8601，同时保留本地时间用于对照事件查看器。实验结果只允许使用 `PASS`、`CRASH`、`EXIT_UNKNOWN`、`STOPPED`、`INCONCLUSIVE`。

【v3新增 F7】交接协议（本节新增）：

- 载体：证据目录 `.tmp/memory-shock-round4/<experiment_id>/<timestamp>/` 与账本文件 `docs/experiment-log/<experiment_id>-<pid>-<utc>.md`；
- 操作 AI 交付：上述六件证据文件 + 账本 `planRecordSha256`；
- 主机证据收集者交付：LocalDumps 键值、CrashDumps 目录新增文件清单、Event ID 1000/1001/1026 与 System 7031/7034、进程退出码与退出时间、ProcDump 附加结果（如可用），附 SHA-256 校验清单；
- 核对规则：项目经理评审时以账本 hash 与证据包 manifest 交叉核对；任何不一致（缺失、hash 不符、时间戳错位）⇒ 该实验结果标记 `INCONCLUSIVE` 并停止。主机证据收集者不得以缺失日志推断不存在异常（与角色表禁止事项一致）。

### 5. 不可变实验账本

开始任何已批准实验前，操作 AI 必须先创建：

```text
docs/experiment-log/<experiment_id>-<pid>-<utc>.md
```

实验账本分为两段，第一段"计划记录"写完后不得改写；第二段只允许追加"结果记录"。计划记录完成后立即计算并写入 `planRecordSha256`；结果记录必须引用该 hash。发现计划记录有误时，保留原账本，并新建 `<experiment_id>-amendment-<n>.md` 和新的批准记录，禁止覆盖旧内容。不得用结论反向修改计划、预期字节或动作脚本。

【v3新增 F1】`planRecordSha256` 规范化定义（本节新增，解决自引用矛盾）：

- 哈希范围：计划记录正文中从 `experimentId` 行起、至 hash 字段行之前的所有必填字段行（UTF-8 字节序列，行尾统一 LF）；
- 计算时机：计划记录正文写完、hash 行写入前，立即计算 SHA-256；
- 写入方式：`planRecordSha256: <hex>` 作为计划段**最后一行一次性追加**，追加后整段锁定，禁止任何改写；
- 自引用处理：hash 值不包含 hash 行本身，不存在循环依赖；任何修订必须新建 `<experiment_id>-amendment-<n>.md` 并重新签发批准，禁止覆盖旧内容。

计划记录的必填字段（【v3修订 F6】control 已补入）：

```text
experimentId, planVersion, approvalId, planRecordSha256, operator,
startUtc, pid, processStartUtc, generation,
GA/EXE base, module build identifier,
game state, fixed action script, B/S/temp/vtable/originalRet/CrossFade,
control 的 address/length/before/data,     ← F6 新增
唯一变量, 逐项 precondition, 所有 write 的 address/length/before/data,
rollback bytes, success/failure/unknown definition, next gate.
```

结果记录的必填字段：

```text
每次 read/write 的 UTC、before/data/readback、readback hash、耗时、
链三次快照、control/vtable 历史、动作实际时间、
进程状态、退出码、dump/Event Log 原始路径、
回滚尝试与 readback、观察事实、支持的假设、未决问题、最终状态。
```

结论必须分层书写：`观察事实`、`支持的假设`、`已验证结论`。单个实验不允许产出"已证实根因"。

## Gate G0：先决取证（E-Prep）

E-Prep 不改目标进程内存，只做主机和模块信息采集。操作 AI 必须在任何 E0/E1/D 实验前完成：

1. 在主机确认 LocalDumps 注册表实际键值和 dump 目录；副机不得代替主机写注册表。
2. 记录实验开始 UTC 时间、目标 PID、退出监控方式。
3. 读取 `.vmp1` 所属 PE section 的 `Characteristics`，至少记录 `R`、`W`、`X` 位；若无法读取 VAD，明确写 `VAD=UNKNOWN`。
4. 验证 `B` 前 448B 为零或记录非零范围；验证 `B` 到 `B+0x1A7` 不跨页。
5. 保存 T4 block 原始 hash 和 vtable 原始值。

【v3新增 F10】尺寸换算说明（本节新增）：T4 block 区间为 `B..B+0x1A7`（424B），位于 codecave 内；"B 前 448B"指 codecave 基址前的对齐保留区，用于确认无越界复用。G5 所述"固定 360B 压缩方案"指 shell 区间的固定尺寸上限（360B），其与 448B/424B 的具体换算关系**待项目经理确认后在本段固化**；在反汇编、重新计算相对跳转与完整 block hash 通过前，不得采用该方案。

PE section 的 W 位只能说明镜像 section 的声明属性；它不能替代 VAD/PTE 证据。若 section 与 VAD 结论冲突，状态为 `INCONCLUSIVE` 并暂停。G0 本身不写目标内存，因此不得虚构 readback；其稳定判定为：所有只读样本、hash 和地址换算相互一致，固定动作脚本连续执行 3 次且每次后观察 30 秒进程存活，无新增关键主机事件。任何一项缺失均不得打开 G1。

## Gate G1：E0 无运行时写入对照

### 目的

证明"control=1 放行后直接进入公共恢复区"是否安全，同时避免 RCX 捕获实验向 codecave 页写入。

### 正确补丁

原始位置：`shell[92..99]`，即 `B+0x64..B+0x6B`。目标：`shell[267]`。E9 的下一条指令地址为 `shell[97]`，位移为 `267-97=170=0xAA`。

```text
写入：E9 AA 00 00 00 90 90 90
长度：8B
写前：4C 8B 59 10 4D 8B 5B 18
```

`E9 AB 00 00 00` 加 4 个 NOP 是错误方案：长度为 9B，会覆盖 Char immediate 的第一个字节，禁止使用。

### 执行顺序（【v3修订 F5】动作次数与 G0 口径一致）

1. 新进程完成 E-Prep，解析并记录全部地址。
2. 写入完整 T4 block，readback 400B。
3. vtable 写为 `S`，readback；control 保持 0，完成一次动作确认 T4 基线。
4. 写 E0 8B 补丁，readback；control 写 1，执行固定动作脚本 **3 次，每次动作后观察 30 秒**。
5. 记录进程是否存活，并采集主机事件和退出码。
6. 按回滚协议收尾。

### 判定（【v3修订 F2】E0 非 PASS 的阻塞规则）

- `PASS`：无运行时写入的放行/恢复路径稳定。该结果只支持"捕获实验新增写入是变量"的假设，不能证明 codecave 页为 RX，也不能证明前一次是 AV。可申请进入 G2。
- `CRASH`：仍退出。停止 D1a-D5，先取得 faulting IP 或确认 gate/跳转语义；不能归因于 codecave 页写保护。**【v3新增 F2】** E0 非 PASS 时，**所有写实验（含 E1/G2/G3）不得启动**，仅允许 G0 级只读采集与主机取证，直至取得 faulting IP 或项目经理重新签发批准。
- `EXIT_UNKNOWN`：进程消失但无退出码/事件。保留现场，先完成主机取证，再由项目经理决定；在项目经理决定前不得启动 E1/D1a-D5。

## Gate G2：S0 scratch 资格验证

G2 是 E1 的硬前置。操作 AI 不得搜索或试写"GA `.data` slack"、其他模块 slack、heap 空洞或未知地址来寻找 scratch。候选地址只能由目标模块/测试环境所有者提供，并且下列证据全部存档后，项目经理才可批准其作为 E1 的 `SCRATCH`：

```text
1. 所有权与生命周期：该地址由诊断环境专用，且不会被游戏对象、vtable、metadata、容器或堆管理使用；
2. 映射属性：当前进程对应 VAD=RW 或 RWX，PE section 与 VAD 不冲突；
3. 边界：8B 对齐、不跨页，完整可回滚范围已定义；
4. 稳定性：写前连续 3 次、间隔 50ms 的完整快照一致；
5. 回滚：原始 8B 已保存，恢复字节、expected-before 与 readback 条件明确；
6. 影响分析：记录该页所在模块/区域、相邻数据，以及为何写入不会改变游戏状态。
```

任一证据缺失时，G2=`BLOCKED`，E1 和 D1a-D5 全部不允许执行。此时操作 AI 只能继续只读采集或提交候选的所有权资料，不能自行选址。

## Gate G3：E1 写入可验证 scratch 的 RCX 捕获

### scratch 选择门槛

操作 AI 不得自行选择地址。候选 scratch 必须已经通过 G2，且 `SCRATCH` 以 approval ID 写入实验账本。以下条件只用于复核，不构成自行批准授权：

```text
属于当前 GA 的已确认 RW section 或 VAD=RW；
至少 8B 可写，且不跨页；
当前读取为 0，连续 3 次 50ms 采样不变化；
不属于 vtable、对象字段、metadata、堆管理结构或正在使用的数组；
写入后可 readback，并在回滚时恢复原值。
```

若没有满足条件的地址，E1 状态为 `STOPPED`，不得把 codecave、`.text` slack、模块 `.data` slack 或未知页当 scratch。

### 捕获片段的批准要求

当前没有通过 G2 的 scratch，因此 E1 没有可执行 patch 字节，状态固定为 `STOPPED`。此前将 `B+0x1A0` 作为捕获目标的模板已经撤销：它位于 codecave 页，既未证明属于诊断存储，也未证明 CPU 可写，不能作为任何 E1 的默认或候选 scratch。

G2 通过后，项目经理必须以新的 E1 补充文档单独批准捕获片段；操作 AI 不得自行把地址填入历史模板。补充文档必须同时给出：

```text
SCRATCH VA、所有权证明、VAD/PTE 证据和完整回滚字节；
经汇编器生成的完整 patch bytes、每条指令 offset 和 branch displacement；
被片段改写的寄存器/flags 与公共恢复路径的保存、恢复证明；
每一处 write 的 expectedBefore、readback 和停止判定；
该片段为何不会再向 codecave 或其他未知 CPU 页做运行时写入。
```

仅有"地址可读写""全零"或 PE section 的 `W` 位均不足以批准 E1。

### 判定

【v3新增 F4】"合法 RCX"判据谓词（供本段第 1 条使用）：

```text
1. RCX 非零；
2. RCX 指向当前进程已映射且可读的内存范围；
3. RCX 与已知地址（temp、vtable、B/S 区域）可区分；
4. 不满足任一条件时，状态为 INCONCLUSIVE，不得将任意非零值视为合法捕获。
```

- 读取到合法 RCX（满足上述全部谓词）：记录 RCX、`[RCX+0x10]`、`[RCX+0x10]+0x18` 的只读值，进入 D1a。
- RCX 与外部 `temp` 不同：以后续 hook 现场 RCX 为准，暂停旧链假设。
- RCX 捕获仍导致退出：停止，优先检查 scratch 的 VAD/PTE 和 faulting IP。
- scratch 写入后被游戏改写：标记 `INCONCLUSIVE`，更换经批准的 RW scratch，不得覆盖游戏数据。

## Gate G4：D1a-D5 active path 二分

每项均需新进程和 T4 基线。每个补丁只写表中长度，写前期望必须匹配；目标均为已有公共恢复区 `shell[267]`。

| 编号 | block 地址 | 写入 | 原始期望 | 测试范围 |
|---|---|---|---|---|
| D1a | `B+0x68` | `E9 A6 00 00 00` | `4D 8B 5B 18 48` | 保留第一次 `[RCX+0x10]`，跳过第二次解引用及后续。 |
| D1 | `B+0x6C` | `E9 A2 00 00 00` | 当前 `48 B8` 加 immediate 前 3B | 执行两次 r11 解引用后恢复。 |
| D2 | `B+0x92` | `EB 7F` | `75 7F` | 【v3修订 F3】无条件跳过 CharacterManager/breakData 中段链（`B+0x94..B+0x112`），直接恢复。⚠️ 待 PM 确认：若预期语义为"执行链后恢复"，当前字节不成立，需重新设计。 |
| D3 | `B+0xA9` | `E9 65 00 00 00 90 90` | `4D 8B 9B C8 00 00 00` | 保留 breakData 写入，随后恢复。 |
| D4 | `B+0xA2` | `90 90 90 90 90 90 90` | `C7 40 58 00 30 00 00` | 禁用 breakData 写，保留 CrossFade 准备。 |
| D5 | `B+0x10A` | `90 90` | `FF D0` | 禁用实际 call，保留参数准备。 |

推荐顺序：`G0 -> G1/E0 -> G2/S0 -> G3/E1 -> G4/D1a -> D1 -> D2 -> D3 -> D5 -> D4`。若 E1 尚未得到合法 RCX，G4 不得打开；不能把 D1a 的结果解释为特定对象解引用根因。

### 二分判定规则

- D1a `CRASH`：早期入口/第一次解引用或 D1a 跳转语义问题，结合 E1 RCX 判定；不得直接写"NULL"。
- D1a `PASS`、D1 `CRASH`：优先定位第二次 `[r11+0x18]` 解引用。
- D1 `PASS`、D2 `CRASH`：定位 CharacterManager/local-player 链或比较前字段。
- D2 `PASS`、D3 `CRASH`：定位 breakData 链或字段写入。
- D3 `PASS`、D5 `CRASH`：D5 已跳过实际 call，优先检查 `AnimPlayable` 链、参数准备和 D5 patch 语义；先获得 dump/IP，不得归因于 callee。
- D3 `PASS`、D5 `PASS`：实际 `call rax` 是主要候选，但仍需 D4 作独立交叉验证和主机证据，不能直接宣布 ABI 或 callee 已确认。
- D4 与 D5 是平行的交叉验证，不能作为严格的前后二分：D4 同时改变 breakData 写入状态并保留 call，D5 保留 breakData 写入并移除 call。两项结果矛盾、或任一项不能由 faulting IP 支持时，结论为 `INCONCLUSIVE`，gate 关闭。
- D5 `CRASH`：先检查 D5 patch 是否准确覆盖 `FF D0`，然后获取 dump/IP；无证据时状态为 `INCONCLUSIVE`。

【v3新增 F9】次级证据组合（本节新增）：当 D5（或任一实验）`CRASH` 且无法取得 faulting IP 时，允许以次级证据组合作为降级证据：

```text
1. 进程退出码；
2. Application Event ID 1000/1001/1026（或 System 7031/7034）；
3. 时间戳与实验最后一次写入对齐。
```

次级证据仍不足以区分 AV / 主动退出 / 写保护 fault 时，状态为 `INCONCLUSIVE`，gate 关闭。dump 不可得本身不构成"无异常"证据，也不免除主机取证职责。

任一结果与表格不一致时，停止并提交 `STOP` 报告；不允许为了"确认假设"再添加 NOP、改 gate 或扩大 patch 范围。两个独立新进程对同一最小变体的结果矛盾时，gate 自动关闭，结论为 `INCONCLUSIVE`。

## Gate G5：源码修复闸门

操作 AI 只负责提交定位证据，不得直接把诊断 patch 固化到源码。只有满足以下全部条件，才创建源码修复任务：

1. E1 捕获到合法 RCX，且 scratch 写入保护已验证；
2. D1a-D5 或 crash dump 将故障缩小到单一指令/调用边界；
3. 新进程至少重复两次得到相同边界；
4. 主机证据能区分 AV、主动退出、写保护 fault 和未知退出；
5. 项目经理批准新的 shellcode 长度、branch displacement、metadata offset 和回滚策略。

若确认 NULL，源码模板应使用带标签的 assembler/layout builder 生成：

```asm
mov  r11, [rcx+0x10]
test r11, r11
jz   restore
mov  r11, [r11+0x18]
```

固定 360B 的压缩方案只能在反汇编、重新计算相对跳转和完整 block hash 通过后采用。live block 上的固定下标补丁不视为最终修复。【v3新增 F10】"固定 360B"与 T4 block 尺寸的关系见 G0 段尺寸换算说明；数值待 PM 确认后固化。

## 主机取证职责

操作 AI 每个实验结束后必须请求/收集主机侧：

```text
LocalDumps 键值；CrashDumps 目录新增文件；
Application Event ID 1000/1001/1026；
System Event ID 7031/7034；进程退出码和退出时间；
若可用，ProcDump 的附加结果与 faulting IP。
```

没有这些数据时，结果只能写 `EXIT_UNKNOWN` 或 `INCONCLUSIVE`。ProcDump 无法附加本身不是 ACE 证据。

## 里程碑、停止条件和交付物

### M1：证据链准备完成

交付 E-Prep 目录、保护属性、原始 block hash、主机 dump 配置和基线日志。

### M2：运行时写入假设隔离

E0 得到 `PASS` 或有可定位 faulting IP。E0 未完成不得进入 E1/D1a。（【v3修订 F2】"E0 未完成"定义为 E0 未回传 `PASS`；CRASH/EXIT_UNKNOWN 同样阻塞 E1 与全部 D 实验。）

### M3：入口对象确认

E1 得到合法 RCX 和两级只读现场，或明确证明 scratch/保护条件不可满足。

### M4：active path 定位

D1a-D5 中一个边界在两个新进程重复得到相同结果，并附完整证据包。

### M5：修复验收

源码修复在两个新进程中通过：T4 稳定、control=1 不退出、目标效果符合预期、正常回滚成功。若无法满足，转入只读/轮询方案评估，不继续扩大注入面。

以下情况立即终止本轮 hook 研究并上报：

- E0、E1 或任一 D 实验出现无法解释的运行时 CPU 写入；
- 连续两次实验无 dump、无事件、无退出码且结果不一致；
- G2 的 scratch 候选 VAD/PTE 保护始终未知；
- 需要绕过或关闭保护机制才能继续；
- 真实 CrossFade call 是唯一退出点且多次无证据主动终止。

最终状态必须是 `FIXED`、`ROLLBACK_TO_READ_ONLY` 或 `INCONCLUSIVE` 三者之一，禁止用"基本解决""疑似 ACE""应该是 NULL"等非审计结论收尾。
