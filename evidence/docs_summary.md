# AddDelay 时间域异常：历史文档与实现摘要

日期：2026-09-11。此摘要只记录本次读到的文档与源码；没有连接 DMA，也没有操作当前主机进程。文档中的历史 `Observed` 不等于当前主机已经验证。

## 来源

- `dump_workspace/research/adddelay-time-domain/2026-09-11_AddDelay复现验证.md`
- `dump_workspace/research/adddelay-time-domain/field-journal.md`
- `dump_workspace/research/adddelay-time-domain/journal.md`
- `dump_workspace/research/adddelay-time-domain/README.md`
- `dump_workspace/research/adddelay-time-domain/RUNBOOK.md`
- `App/Il2CppRuntimeDumper/AddDelayRepro.cpp`
- `App/Il2CppRuntimeDumper/AddDelayProbe.cpp`

工作区根路径：`J:\Code\C++\dma\er_new\UnityExplorer-main`。

## 要复现的效果

文档目标是跳过普通版 `CharactorSync.AddDelay` 中一次 `_globalTime` 减法，使 `localToGlobalTimestampDiff`（下称 D）相对正常基线多出约当前 `_globalTime`（下称 G）。原方法注入进程内 DLL，在目标指令设置 DR0/DR7 硬件执行断点；VEH 收到目标地址的 `EXCEPTION_SINGLE_STEP` 后将 RIP 前移 5 字节，保持 `.text` 原字节不变。

这证明的是客户端时间校正计算路径和 D 的变化；原文明确未证明服务器命中、伤害、HP/护甲或网络包接受结果。

## 历史样本定位

| 项目 | 值 | 来源等级 |
|---|---|---|
| 模块 | `GameAssembly.dll`（Normal） | 历史文档记录 |
| 历史 PID / base | `2204` / `0x7FFB336B0000` | 不可复用到当前进程 |
| PE timestamp / SizeOfImage | `0x6A969F1D` / `0x1997F000` | 当前主机待复核 |
| AddDelay entry RVA | `0x156C250` | Probe 源码 |
| 目标指令 RVA | `0x156C395`（entry + `0x145`） | 文档及源码 |
| 目标字节 | `F2 0F 5C 40 28` | 文档记录为前后保持一致 |
| 解码 | `subsd xmm0, qword ptr [rax+0x28]` | 按字节推导 |
| CharactorSync TypeInfo RVA | `0xE606490` | 历史普通版成功样本 |
| GameBaseObject TypeInfo RVA | `0xE644808` | Repro / Probe 源码 |

Probe 中另有 Normal 快照候选 `CharactorSync=0xE60D3F0`、`GameBaseObject=0xE647780`；Super 候选 `CharactorSync=0x374C368`、`GameBaseObject=0x3755D18`、AddDelay entry=`0xD0287B0`。这些只是源码候选，不是本次实测地址，也不能把 Normal 的目标指令 RVA 套用到 Super。

## 对象链和类型

1. `klass = *(moduleBase + TypeInfoRva)`。
2. `className = *(klass + 0x10)`；按 C 字符串核对 `CharactorSync` / `GameBaseObject`。
3. `staticFields = *(klass + 0xB8)`。
4. `syncInstance = *(syncStaticFields + 0x8)`；必须验证 `*(syncInstance) == syncKlass`。
5. `D = *(double*)(syncInstance + 0x10)`：`localToGlobalTimestampDiff`。
6. `R = *(float*)(syncInstance + 0x68)`：`shrtt`。
7. `G = *(double*)(gameBaseObjectStaticFields + 0x28)`：`_globalTime`（静态字段，不是 GameBaseObject 实例字段）。

对象 / qword 指针按 8 字节对齐检查；类名 `char*` 不应强制 8 字节对齐。基址、PID、对象指针必须从主机当前 DMA 视图读取，副机本地 WinAPI 枚举不能替代主机进程定位。

## 历史时序与成功判据

- 初始只读 Probe：40 次，间隔 250 ms；D=`2306.9938..2306.9978`，R=`0.005998..0.007193`，G=`194.5759106..204.3289862`。
- 有限时复现记录：正常采样约 1 秒，执行断点生效 8 秒，命中 158 次，清除后继续采样恢复。
- 历史异常：D 从约 `2307.00` 增为 `2939.20..2947.04`，与同期 G=`632.22..640.06` 相符，即 `D_abnormal - D_baseline ≈ G`。
- 恢复：停止干预后，下一次正常 AddDelay 写入使 D 回到 `2307.003644378` 附近；旧 D 不会因清理动作立即恢复。
- 前后机器码都应为 `F2 0F 5C 40 28`，对象链与进程持续有效，必须有连续样本，不能用单次字段回读宣布复现。
- 持续模式恢复记录：停止后 hits=`6054`，D 从 `4090.2917` 回落到 `2307.0184–2307.0199`，DLL 已卸载。

当前 `AddDelayRepro.cpp` 是持续模式：每 100 ms 为线程重设断点并采样，检测 `adddelay_repro.stop` 后清除断点，继续采样 20 次（2 秒），移除 VEH 并卸载。源码中的 `kArmMs=8000` 已不参与控制，因此不能依据旧注释认为现有 DLL 会在 8 秒后自动停止。

## 当前材料缺口与实现约束

- 文档目录现有 5 个 Markdown 文件，未携带其引用的 `evidence/runtime/*.log` 原始日志；历史数值目前只能追溯到文档记录。
- README 写 `Evidence` / 只读验证，journal 和最终复现记录显示后续已复现并恢复；以具体后续记录解释旧 README 状态。
- 原始实现依赖主机进程内 DLL、VEH 和线程上下文 API。DMA 读写本身不等于已具备远程执行 VEH / 改 CPU 调试寄存器的能力；应先检查 DMA 项目既有注入与执行机制再确定迁移路径。
- Repro 源码只记录目标字节和 PE 身份，没有在设置断点前严格比较目标 5 字节、timestamp、SizeOfImage；迁移时不能沿用此弱门槛。
- Repro `SetBp` 直接占用 DR0 并 OR `Dr7` 的 bit 0，未保存原 DR0/DR7，也未完整设置 DR0 对应条件位；清理置零 DR0 并清 bit 0，不能视为恢复原调试寄存器状态。
- Repro 未检查 VEH 安装成功、未验证实例 klass 回指、忽略 GameBaseObject 解析失败，且持有实例指针不刷新；当前工程必须明确处理初始化失败、场景切换、进程退出和回滚。

## 文档 SHA-256

| 文件 | SHA-256 |
|---|---|
| `2026-09-11_AddDelay复现验证.md` | `8D1F11DACF6360F43B834F4A1D69C85F1A6D53E12FB98AC0242117BB1298B8CF` |
| `field-journal.md` | `F17262FE6A13E845E4A4405C8BE76195EA65CA81C081C00C4D34C087EBA5A839` |
| `journal.md` | `D1F6894874E4C9063B9D9B7821A4896B40FDE7C91A00E6D7A671B81AA71206CB` |
| `README.md` | `C906DA6D12CC3903CA8C44E0C73A1FB7F8EF69D21031911FAF7E0238AAA099BA` |
| `RUNBOOK.md` | `51FFF8F79DFA1D9B9AF94FE5BC2DCCBB32EE70CE528F8645176CA6FE1F6A5342` |
