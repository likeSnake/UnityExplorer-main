# 内存振 DMA 注入闪退解决方案

日期：2026-09-05  
适用对象：`UnityExplorerMcpServer`、`YJWJ_DMA_NEW` 的 CrossFade 内存振实验  
依据：`docs/memory-shock-crash-analysis-20260905.md`、`YJWJ_DMA_NEW/Naraka/Hook.cpp`

## 结论先行

当前证据不足以证明闪退是 ACE 完整性校验。原分析中的 A1-A6 使用了错误的 BE immediate；A7 修正后仍闪退，但没有 crash dump、Windows Event Log 或进程退出码，因此“被 ACE 杀”和“shellcode 执行 AV”仍未区分。`.vmp1` tail slack 是一个可测试的候选位置，不能作为已经验证的绕过结论。

最可能的三个工程问题按优先级排列如下：

1. **目标对象竞态**：MCP 通过多次 HTTP 请求解析链、写 block、写 vtable。`temp` 或 `temp+0x198` 可能在请求之间被 Unity 换代，最终改到已经失效的对象。
2. **提交瞬间的指针撕裂或时序窗口**：目标线程可能在 vtable 8 字节写入过程中调用该槽；DMA 写入不应假定具备目标 CPU 视角下的原子性。
3. **执行环境不等价**：当前页的 `PAGE_EXECUTE` 属性、VMProtect 段布局、CrossFade method pointer 或调用 ABI 可能与用户程序成功时不同。写后 readback 只能证明存储字节正确，不能证明 CPU 可以执行或目标参数有效。

ACE 主动终止是第四种必须保留的假设。只有在取证后，才应决定继续修正 hook 还是停止代码注入并改为轮询读写。

## 已确认的事实与边界

`Hook.cpp` 的 CrossFade 路径在 `KnifeStartHook_CrossFade()` 中执行以下步骤：

1. `EnsureInitActorKitRefer()` 校验 `TransparentTagHandler` TypeInfo。
2. 解析 `GA + RVA -> +0xB8 -> +0x08 -> +0x70 -> +0x90 -> +0x00`，得到 `temp`。
3. 读取 `vtable = temp + 0x198` 和 `originalRet = [vtable]`。
4. 尝试从旧 shellcode 尾部读取 `MSHKCFD1` metadata，并在命中时接管旧 hook；否则恢复旧 hook 的 original return。
5. 解析 `CrossFadeInFixedTime`，失败时使用 Super fallback RVA `0x797D760`。
6. 生成 360B shellcode，并附加 8B control byte、32B metadata，总 block 为 400B。
7. `WriteMemoryShockShellcodeBlock()` 用一次 `mem.Write(base, block, 400)`，随后完整 readback。
8. `CommitMemoryShockHook()` 先校验 shell magic，再写 `[vtable] = shellEntry`，readback 后调用 `SkockOf()`。

MCP 的 `AnalysisSession::Write()` 还会在 backend write 前强制读取 `expectedBefore`，并拒绝跨 4KiB 写入；成功 readback 表明 MCP 的逻辑层和 DMA 读回路径看到了预期字节。它不证明目标线程在整个提交窗口内没有运行，也不证明目标页可执行。

因此，以下判断目前只能标记为 `[待验证]`：

- ACE 是否因为写入地址、写入次数或 vtable 值而终止进程；
- `.vmp1` slack 是否在当前进程中具有可执行保护；
- MCP 的 50 次 8B 写入是否真的比主程序的一次 400B 写入改变了目标可见行为；
- `temp`、vtable 和 `originalRet` 是否在 MCP 的多个请求之间保持稳定。

## 第一阶段：先取得崩溃证据

在任何新的写实验前，给目标进程配置用户态 dump。建议使用专用目录，避免把 dump 写入游戏安装目录：

```powershell
$dumpDir = 'C:\CrashDumps\NarakaBladepoint'
New-Item -ItemType Directory -Force -Path $dumpDir | Out-Null
New-Item -Path 'HKLM:\SOFTWARE\Microsoft\Windows\Windows Error Reporting\LocalDumps\NarakaBladepoint.exe' -Force | Out-Null
New-ItemProperty -Path 'HKLM:\SOFTWARE\Microsoft\Windows\Windows Error Reporting\LocalDumps\NarakaBladepoint.exe' -Name DumpFolder -PropertyType ExpandString -Value $dumpDir -Force | Out-Null
New-ItemProperty -Path 'HKLM:\SOFTWARE\Microsoft\Windows\Windows Error Reporting\LocalDumps\NarakaBladepoint.exe' -Name DumpType -PropertyType DWord -Value 2 -Force | Out-Null
```

同时记录：

- 事件查看器 `Windows Logs -> Application` 中的 Event ID 1000/1001；
- `NarakaBladepoint.exe` 的退出码和退出时间；
- `%LOCALAPPDATA%\CrashDumps` 及上述目录是否生成 dump；
- MCP 日志中的最后一个 read/write 请求和目标 `generation`。

判定规则：

| 结果 | 含义 | 下一步 |
|---|---|---|
| dump 的 ExceptionCode 为 `0xC0000005`，Faulting IP 在 shellEntry | shellcode/执行环境问题 | 进入第二阶段，先不换注入位置 |
| dump 的 Faulting IP 在 CrossFade method 或目标对象地址附近 | method pointer、ABI 或对象生命周期问题 | 做 method/prologue 和对象稳定性实验 |
| 无 dump，进程有明确主动退出/事件记录 | 外部终止或反作弊响应的可能性高 | 停止增加 hook 复杂度，进入第五阶段 |
| 无 dump、无事件、退出码异常 | 仍不能定性 | 用 ProcDump/WER 复现一次，并保存完整时间线 |

每次实验必须使用新的游戏进程和新的 `generation`。禁止把上一次进程的 GA、heap、codecave 或 vtable 地址带入下一次实验。

## 第二阶段：把 MCP 提交变成一个可验证的事务

### 2.1 解析链稳定性

在写入前连续读取 3 次，每次间隔 10-20ms，保存以下快照：

```text
generation
GA base
TransparentTagHandler slot
TypeInfo / static / ActorKit / InteractableDevice / temp
vtable address
current vtable target
CrossFade address
codecave base and shellEntry
```

只有在三次 `temp`、vtable 地址和 current target 全部一致时，才允许提交。vtable 地址必须满足 8B 对齐；`current target` 必须是 GA code pointer，且不能已经指向本次 shellEntry。

写入 block 后，必须在同一个 worker/job 中立即重新读取 vtable 槽和 `temp` 链。若任一值改变，不能继续写 vtable，先放弃该候选并重新解析。

### 2.2 提交顺序

推荐顺序如下：

1. 读取并保存原始 vtable target、原始 400B codecave 内容和目标链快照。
2. 将完整 block 写入同一 4KiB 页内，单次 backend write；MCP 侧不要再拆成 50 次 8B，除非单发实验明确证明 backend 会错误对齐。
3. 对 block 做一次 no-cache 完整 readback，并校验 magic、metadata、3 个 LE64 地址和所有跳转表项。
4. 再次解析链，确认 vtable 地址和 current target 仍与步骤 1 一致。
5. 最后执行一次 8B vtable 写入并立即 readback。
6. control byte 保持 `0`，先不要同时 patch 13B berserk gate。待普通门控版本稳定后，再单独测试 berserk。

如果 MCP backend 的单次 400B 写入在物理层确实发生页基址偏移，应修正 backend 的写入实现或选择页内对齐的 block；不要用多次小写掩盖地址转换错误。`AnalysisSession` 已经禁止跨页，因此该问题应在 backend 日志中直接验证，而不是靠目标进程结果推断。

### 2.3 回滚顺序

正常关闭时先把 vtable 恢复为保存的 originalRet，readback 成功后再清除 codecave。若发现目标链已经换代，不要把旧 originalRet 写入新对象；先重新解析并确认该槽当前值仍是本次 shellEntry。所有回滚操作都带 `expectedBefore`，避免覆盖第三方或游戏自身更新的指针。

## 第三阶段：最小化定位实验

每次只改变一个变量。每项都要在相同大厅/对局状态下重复两次，并记录 dump 结果。

### T0：只读基线

不写任何地址，连续采样 vtable 链、`originalRet`、CrossFade address 和 codecave 候选。目标是确认对象是否会在 100ms 内变化。若链本身不稳定，先解决时序，不能进入 hook 实验。

### T1：写 block，不改 vtable

把 400B 写入候选 codecave，但不让任何 vtable 指向它。观察游戏 30-60 秒并检查该区域是否被回滚。若此项就导致退出，问题在写入检测、页保护或区域选择，不在 shellcode 执行。

### T2：vtable 原值对照

把 vtable 槽写回它当前已经存在的 `originalRet`，确认“写 vtable 这一动作本身”是否导致退出。若退出，优先怀疑写入时序、目标对象失效或外部检测。

### T3：合法代码地址对照

在确认页面可执行后，使 vtable 暂时指向一个已知的、不会被调用的 GA code pointer；不要指向零区。该实验只用于判断 vtable 值校验，不能让目标函数真正执行未知地址。出现退出时立即回滚。

### T4：门控 shellcode

使用与用户 live block 完全一致的 gate bytes（control byte 为 `0`），只在 vtable 稳定后提交。不要启用 berserk。若触发 AV，dump 中 faulting IP 和寄存器是判断参数/ABI 的关键。

### T5：method pointer 对照

优先使用当前进程 `AnimPlayable` TypeInfo 的 `MethodInfo->methodPointer`。fallback `GA+0x797D760` 只有在读取该 RVA 的 prologue、所在 section 和方法参数数目均通过校验后才使用。保存两者的 RVA、前 32B bytes 和来源标签，确保 MCP 与用户程序实际调用的是同一个函数。

### T6：codecave 位置对照

在同一进程内依次测试：

- `.vmp1` tail slack；
- 目标进程通过 `DMA_AllocateMemory` 得到的 RWX 区域；
- 仅用于 T1 的普通 `.text` slack。

对每个候选同时记录 PE section characteristics、section end、页边界和 VAD protection。只有 `.vmp1` 和 RWX allocation 都通过执行权限检查后，才比较它们的稳定性。`.text` slack 的失败不能单独证明完整性校验。

## 第四阶段：MCP 服务应增加的工程能力

### 4.1 增加原子 hook job

不要让上层 AI 通过 3-5 个独立 `unity_memory_write` 请求拼装 hook。新增一个服务端 job，输入只包含当前 `generation`、目标链描述和 block bytes，由同一个 executor 完成：

```text
resolve -> stability check -> write block -> readback -> re-resolve -> write vtable -> readback
```

job 返回所有地址、before/after bytes、每步耗时和失败阶段。这样可以区分“HTTP 请求之间对象换代”和“单个 backend write 失败”。

### 4.2 增加执行前验证

在允许 vtable 写入前验证：

- `shellEntry` 与 `base+8` 的关系正确；
- block magic 和 metadata 的 little-endian 解码正确；
- `originalRet` 属于当前 GA code range；
- CrossFade method pointer 属于当前 GA code range；
- shellcode 所在 VAD/section 具备 execute 权限；
- block 不跨页，且 vtable 地址 8B 对齐；
- 三次目标链快照一致。

### 4.3 增加现场日志

每次 job 写入 JSON 现场文件，至少包含：

```text
timestamp, pid, generation, module bases, section/VAD info,
temp, vtable, originalRet, shellEntry, method pointer,
block sha256, before/readback sha256, write count, rollback result
```

日志中不要保存 MCP bearer token。现场文件必须与 crash dump 使用同一时间戳关联。

## 第五阶段：确认是外部终止时的替代方案

若 T1/T2 在不执行 shellcode 的情况下也会导致进程终止，或者多次复现始终无 AV dump 而只有主动退出，应停止继续扩大代码注入面。对于当前研究目标，采用无代码修改的轮询方案：

1. 只读解析本地角色和 `ActorKit` 链；
2. 以 50-100ms 周期读取出刀/动作状态和 `breakData` 有效性；
3. 在状态边沿触发时，仅写 `[ActorKit + 0x108] + 0x58 = 0x3000`；
4. 每次写入带 expected-before，并在状态结束或对象换代时恢复原值；
5. 暂时不调用 `CrossFadeInFixedTime`，先验证 breakData 写入本身是否满足实验目标。

该模式仍需验证字段语义和对象生命周期，但不会引入远程执行指针、vtable 替换、shellcode 页和调用 ABI 问题。若连单字段写入也触发终止，则问题是写入策略或目标检测，应退回只读采样。

## 实施优先级与停止条件

建议执行顺序：

1. 配置 LocalDumps，完成一次无写入基线。
2. 实现 T0，确认链稳定性。
3. 在 MCP 中实现单 job 提交、block 单发写入和完整现场日志。
4. 依次执行 T1、T2、T4；每步出现退出立即回滚并保存证据。
5. 仅在 T4 有 AV dump 时执行 T5/T6；仅在无 dump 主动退出时进入第五阶段。

以下任一条件成立时停止继续尝试 vtable hook：

- 同一 build、同一状态下 T2 重复触发无 dump 退出；
- block 未执行时目标区域被立即回滚；
- VAD 明确不可执行且不存在可用 RWX 对照；
- 目标链在提交窗口持续换代，无法建立稳定快照。

## 最终验收标准

解决方案只有同时满足以下条件才算完成：

- 至少两次新进程复现，使用新的 `generation` 和现场文件；
- block、vtable、control gate 的 before/after/readback 可追溯；
- 能明确区分 AV、主动终止、写入回滚和对象竞态；
- 正常关闭可恢复 originalRet，且不会把旧地址写入新对象；
- 若 hook 仍不可行，轮询方案在相同状态下完成目标字段验证，或明确证明写入也会被终止。

在获得这些证据前，不应把“选择 `.vmp1` slack + 分块写入”称为已验证配方；它只能作为 T6 的一个实验变量。
