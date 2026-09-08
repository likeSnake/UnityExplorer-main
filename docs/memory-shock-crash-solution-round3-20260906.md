# 内存振闪退问题：第三轮收敛与修复方案

日期：2026-09-06  
依据：`docs/memory-shock-crash-analysis-20260905.md`、`docs/memory-shock-crash-solution-20260905.md`、`docs/memory-shock-crash-update-round2-20260906.md`、`.tmp/build_crossfade_block2.ps1`、`YJWJ_DMA_NEW/Naraka/Hook.cpp`

## 结论

第二轮已将故障稳定收敛到 gate 放行后的 active path：

| 已完成实验 | 结果 | 可得结论 |
|---|---|---|
| T0：连续链采样 | 通过 | 当前训练营状态下，`temp`、vtable 与 `originalRet` 稳定。 |
| T1：单次写 400B block | 通过 | `.vmp1` 候选区可写，block 不会被即时回滚。 |
| T2：vtable 同值写入 | 通过 | 单独的 8B vtable 写操作不是触发条件。 |
| T4：vtable 指向 shell，control=0 | 通过 | hook 的保存现场、恢复现场和跳回 `originalRet` 已实际执行且稳定。 |
| control=1 | 必现退出 | 问题位于 gate 后的两次对象解引用、角色匹配链、`breakData` 写入或 `CrossFadeInFixedTime` 调用。 |

因此，不再重复测试 codecave、单发/分块写、vtable 指向 shell 或对象链稳定性。当前第一优先级是用单变量实验把 active path 切开。

空闲态读取到的 `[temp+0x10] = 0x...511` 未对齐，且 `+0x18` 指向 ASCII 数据，是一个强信号；但它**不能**证明 hook 入口时 `RCX == temp`，也不能单独证明空指针。必须先记录实际入口 `RCX`，再根据分段实验定位。

无 dump 同样不能单独证明 ACE `TerminateProcess`。当前证据只说明退出发生在 active path；AV、受保护进程自行处理异常后退出、WER 配置错误和主动终止仍是并列假设。

## 偏移约定与回滚

令：

```text
B = codecave base（block 第 0 字节，8B control header）
S = B + 8（shell 第 0 字节，也是 vtable 指向的 shellEntry）
block offset = shell offset + 8
```

本轮 T1 block 的已核对边界如下：

```text
shell[92..95]   = 4C 8B 59 10                 ; mov r11,[rcx+0x10]
shell[96..99]   = 4D 8B 5B 18                 ; mov r11,[r11+0x18]
shell[100..]    = 48 B8 <Char VA LE64>
shell[138..139] = 75 7F                       ; jne shell[267]
shell[154..160] = C7 40 58 00 30 00 00         ; breakData + 0x58 = 0x3000
shell[161..167] = 4D 8B 9B C8 00 00 00         ; localActor + ActorKit
shell[258..259] = FF D0                       ; call CrossFade
shell[267..]    = 48 8B 0C 24 ...              ; 公共恢复现场区
```

每个实验都必须从新进程和已验证的 T4 基线开始：完整 block 已写入、vtable 指向 `S`、control=0、gate 保持原始 13B。写入诊断补丁后再把 `B[0]` 写为 `1`。不要把 berserk gate 改成 NOP。

存活时的收尾顺序固定为：

1. 写 `B[0]=0` 并 readback，停止 active path。
2. 等待没有输入动作的空闲帧。
3. 以 `expectedBefore=S` 把 vtable 恢复为该进程记录的 `originalRet`，并 readback。
4. 恢复本次改过的 shell 字节和诊断尾部；对 400B block 做完整 readback。

每次 write 都使用当前进程的 `generation`、本次读取到的 `expectedBefore` 和 readback。进程退出后不得把旧进程的地址、block 或 `originalRet` 用到新进程。

## 先记录实际 RCX

下面的变体只记录 hook 入口的 `RCX`，不执行两次解引用、角色链、写入或 CrossFade。它不覆盖 32B metadata，适合优先执行。

前提：确认 `B+0x190..B+0x1A7` 共 24B 仍全零；这要求候选零区至少覆盖 424B。第二轮使用的 448B `.vmp1` slack 满足长度条件，但每个新进程仍须重新读取确认。

| 地址 | 写入字节 | 写前期望 |
|---|---|---|
| `B+0x64` | `E9 27 01 00 00 90 90 90` | `4C 8B 59 10 4D 8B 5B 18` |
| `B+0x190` | `48 89 0D 09 00 00 00 E9 77 FF FF FF 90 90 90 90 00 00 00 00 00 00 00 00` | 24 个 `00` |

第一处把 `shell[92..99]` 跳转到 `B+0x190`。尾部代码的语义是：

```asm
mov [rip+0x9], rcx       ; 写入 B+0x1A0
jmp S+267               ; 进入已有公共恢复区
```

设置 control=1 后只做一次动作，再读取 `B+0x1A0` 的 LE64 值。判读规则：

| 现场结果 | 下一步 |
|---|---|
| `RCX != temp` | 原先用 `[temp+0x10]` 推断 shell 现场的前提不成立；以捕获的 `RCX` 为对象重新分析。 |
| `RCX` 非 canonical 或不满足该槽对象布局 | vtable 槽/调用约定或对象类型判断错误，停止继续修改 active path。 |
| `RCX` 合法，但 `[RCX+0x10]` 为 0 或非对象指针 | 优先执行 D1a/D1，随后修正对象契约或加入 guard。 |
| `RCX` 与 `temp` 一致且两级值都合理 | 先按 D2-D5 二分效果路径。 |

该诊断会改写 `B+0x1A0` 的数据槽，但不会破坏 metadata；按前述回滚顺序恢复入口 8B 和尾部 24B 即可。

## Active Path 二分实验

下表所有偏移均已换算为 block 地址。`E9` 目标均为已验证的 `shell[267]` 公共恢复区，所以不会跳过 stack restore。

| 实验 | 地址与写入 | 精确测试范围 | 结论 |
|---|---|---|---|
| D1a：仅第一次解引用 | `B+0x68` 写 `E9 A6 00 00 00`；期望 `4D 8B 5B 18 48` | 执行 `[RCX+0x10]` 后立即恢复 | 若退出，优先怀疑第一次解引用、入口 `RCX` 或该诊断跳转本身；须结合 RCX 记录判断。 |
| D1：两次解引用 | `B+0x6C` 写 `E9 A2 00 00 00`；期望为当前 `48 B8` 加 Char immediate 的前三个字节 | 执行两次 `r11` 解引用后立即恢复 | D1a 稳定而 D1 退出，定位为 `[r11+0x18]`。 |
| D2：角色链，无效果 | `B+0x92` 写 `EB 7F`；期望 `75 7F` | 两次解引用和 CharacterManager 链都执行，但无条件进恢复区 | D1 稳定而 D2 退出，定位为 CharacterManager 链/本地角色比较之前。 |
| D3：仅 breakData | `B+0xA9` 写 `E9 65 00 00 00 90 90`；期望 `4D 8B 9B C8 00 00 00` | 保留 `breakData+0x58=0x3000`，随后恢复 | D2 稳定而 D3 退出，定位为 breakData 链或该字段写入。 |
| D4：去掉 breakData 写 | `B+0xA2` 写 7 个 `90`；期望 `C7 40 58 00 30 00 00` | 保留 AnimPlayable 解析、参数准备和 CrossFade 调用 | 与 D3、D5 联合确认 breakData 写入是否为触发项。 |
| D5：只禁用 call | `B+0x10A` 写 `90 90`；期望 `FF D0` | 保留 breakData、AnimPlayable 解析和全部参数准备，不进入被调函数 | D5 稳定则定位为 CrossFade target、ABI 或被调函数内部路径；D5 退出而 D3 稳定则定位为 AnimPlayable/参数准备。 |

建议执行顺序是 `D1a -> D1 -> D2 -> D3 -> D5`；D4 作为 breakData 的交叉验证。每项只在一个新进程中执行一次，并先证明同一进程的 T4 基线稳定。

不能把原先建议的 `shell[92..99] = 4D 31 DB 90 90 90 90 90` 当成空指针证明。它会强制 `r11=0`，使后续比较失败，从而跳过整段 active path；它只能作为 no-op 对照，不能区分哪一个解引用、哪条角色链或哪种效果导致退出。

## 判定后的修复路径

### A. D1a 或 D1 失败：先修对象契约，再加 NULL guard

若 RCX 记录显示对象正确且只有 `[RCX+0x10]` 可能为 NULL，可在 `Hook.cpp` 的 CrossFade `ShellCode` 模板中将原 8B：

```asm
mov r11, [rcx+0x10]
mov r11, [r11+0x18]
```

替换为以下 13B 的紧凑 guard：

```asm
mov  r11, [rcx+0x10]
test r11, r11
jz   after_second_deref
mov  r11, [r11+0x18]
after_second_deref:
```

对应机器码为：

```text
4C 8B 59 10 4D 85 DB 74 04 4D 8B 5B 18
```

`jz +4` 只绕过第二次解引用；`r11=0` 随后与非零本地角色不等，原有 `jne` 会进入恢复区。该版本仍会运行 CharacterManager 链，因此只应在 D2 已证明该链稳定后采用。

该变更比原段多 5B。为了维持 `kMemoryShockCrossFadeShellcodeSize == 360` 和既有 400B block 布局，同时做以下两项等价压缩：

```text
48 C7 44 24 30 00 00 00 00   ->  31 C0 48 89 44 24 30
41 B9 00 00 00 00            ->  45 31 C9
```

两项共节省 5B。由于 local compare 前增加了 5B，而恢复区在两项压缩后仍保持 `shell[267]`，原 `75 7F` 必须改为 `75 7A`。control gate 的 `0F 85 AF 00 00 00` 目标仍是 `shell[267]`，无需改变。

不要把以上修复当成对非空垃圾指针的防护。`test r11,r11` 只能处理 NULL；若记录到的是未对齐/已释放/错误类型指针，应修正 vtable 对象契约或 hook 槽选择，而不是增加猜测性的地址范围判断。

若希望 NULL 时直接跳公共恢复区而不再读取 CharacterManager 链，`jz restore` 需要 near branch，总代码比原段多 9B。此时应让模板以标签生成跳转并重新计算所有 displacement，或允许 block 扩展；不要对 live block 的固定下标盲补。

### B. D2 失败：角色链或 local-player 判断错误

不修改 CrossFade ABI。先用记录到的 `RCX` 确定该 vtable 的 this-object 类型，再验证：

```text
[RCX+0x10] -> +0x18
GameAssembly + TransparentTagHandler -> +0xB8 -> +0x08 -> +0xD8 -> +0x18
```

只有两侧确实是同一类 ActorModel 时，才继续沿用 `cmp rax,r11`。若实际 this-object 不是 `temp`，应从当前槽的调用方反汇编恢复真实参数语义，而不是复用外部链的 `temp` 假设。

### C. D3/D4 失败：breakData 写入问题

先保存 `[breakData+0x58]` 的原值，验证 breakData 在动作窗口仍可读、可写且属于当前 local ActorKit。若 D3 单独复现退出，暂停 CrossFade 相关改动；修复对象生命周期或字段偏移后再试。不要同时改写该字段和调用动画函数。

### D. D5 失败：CrossFade 调用问题

依次核验当前进程的 `MethodInfo->methodPointer` 与 fallback `GA+0x797D760` 是否一致，保存两者前 32B、所属 section 和 RVA。随后用反汇编确认 Windows x64 ABI：`RCX/RDX/R8/R9`、shadow space、stack 参数、stack alignment 以及 XMM 参数来源。D5 稳定意味着 shell 的保存/恢复和参数准备可用，问题集中于 `call` 目标或 callee 内部；不要以更多 gate/NOP 补丁掩盖这一点。

## 主机侧无 dump 取证

以下命令必须在主机执行，并以每次实验开始时间过滤。它们仅读取现有证据；配置 LocalDumps 的写入步骤沿用上一份方案，不要在副机误配注册表。

```powershell
$since = Get-Date
$dumpKey = 'HKLM:\SOFTWARE\Microsoft\Windows\Windows Error Reporting\LocalDumps\NarakaBladepoint.exe'
Get-ItemProperty -Path $dumpKey -ErrorAction SilentlyContinue |
  Format-List DumpFolder, DumpType, DumpCount, CustomDumpFlags

Get-ChildItem 'C:\CrashDumps\NarakaBladepoint', "$env:LOCALAPPDATA\CrashDumps" `
  -ErrorAction SilentlyContinue |
  Where-Object LastWriteTime -ge $since |
  Select-Object LastWriteTime, Length, FullName

Get-WinEvent -FilterHashtable @{ LogName = 'Application'; StartTime = $since } |
  Where-Object { $_.Id -in 1000, 1001, 1026 -and $_.Message -match 'NarakaBladepoint' } |
  Select-Object TimeCreated, Id, ProviderName, Message | Format-List

Get-WinEvent -FilterHashtable @{ LogName = 'System'; StartTime = $since } |
  Where-Object { $_.Id -in 7031, 7034 } |
  Select-Object TimeCreated, Id, ProviderName, Message | Format-List
```

若需要可重复记录退出码，应由启动脚本持有该进程对象：

```powershell
$p = Start-Process -FilePath 'C:\path\to\NarakaBladepoint.exe' -PassThru
$p.WaitForExit()
'ExitCode=0x{0:X8}' -f ([uint32]$p.ExitCode)
```

ProcDump 只能作为补充现场：能附加并得到 dump 时，可用 faulting IP 与 `B..B+0x1A8`、`CrossFade` VA 对照；不能附加或没有 dump 不能单独归因于 ACE。

| 证据 | 判定 |
|---|---|
| dump/事件中为 `0xC0000005`，IP 在 shell 或其数据链 | shellcode 对象契约、指针或 ABI 错误。 |
| dump/事件中为 `0xC0000005`，IP 在 CrossFade 目标 | method pointer、参数或 callee 内部状态错误。 |
| D1-D5 某一最小片段稳定复现退出，但无 dump | 仍按该片段继续验证，不把“无 dump”升级为外部终止结论。 |
| 所有最小片段稳定，唯真实 CrossFade call 退出，且多次无 dump/无应用崩溃事件 | 外部终止或被调函数的受保护行为优先级升高；停止扩大 hook。 |

## 退出条件与替代路径

若 D1-D5 已定位出确定的 AV，应先完成相应模板修复并在两个新进程复验。若只有真实 CrossFade 调用导致无证据退出，则停止 vtable/shellcode 迭代，改为无代码注入的最小字段实验：仅轮询已验证的本地 ActorKit 生命周期，在状态边沿单独验证 `breakData+0x58` 的字段语义，不再调用 CrossFade。若单字段写同样导致退出，退回只读采样并保留本轮现场数据。

本轮验收标准是：能由 D1-D5 中恰好一个最小变量稳定指向早期解引用、角色链、breakData 或 CrossFade 调用之一；或在主机日志/dump 中获得可定位的异常 IP。未满足任一条件前，不应宣布已经确认 ACE 终止或已经确认 NULL 解引用。
