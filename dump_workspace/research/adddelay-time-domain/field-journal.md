# Field Journal：AddDelay 时间域异常

## 目标

验证来源文档描述的普通版 `CharactorSync.AddDelay` 单条指令跳过是否会使 `localToGlobalTimestampDiff` 失去 `GameBaseObject._globalTime` 校正，并确认自动恢复条件。

## 执行链路

1. 只读探针确认 PID、模块身份、TypeInfo、字段地址和目标机器码。
2. 在训练营实际对局中记录 1 秒正常样本。
3. 临时为当前游戏线程设置 `DR0/DR7` 执行断点；VEH 仅在目标 RIP 处将其前移 5 字节。
4. 持续 8 秒采样并统计命中次数。
5. 清除所有临时断点，继续采样恢复过程。
6. 重新注入只读探针确认目标代码字节和正常 D 值。

## 证据

- 普通版 timestamp/SizeOfImage 与来源文档一致。
- 目标字节 `F2 0F 5C 40 28` 前后保持一致。
- 命中 158 次；D 从约 2307 增至约 2939–2947，增量约等于 G。
- 清除后下一正常样本恢复到约 2307；进程保持存活。

## 可复用经验

- 外部 Toolhelp 进程名枚举会被保护层隐藏时，已知 PID 仍可作为 `OpenProcess` 入口。
- 字符串指针不需要 8 字节对齐；对象/qword 指针才需要对齐检查。
- TypeInfo/RVA 必须先按模块 timestamp、SizeOfImage 和目标字节确认，不能跨 Normal/Super 复用。
- 恢复断点只恢复执行流；旧 D 要等下一次正常 `AddDelay` 才会覆盖。

## 未验证项

未采集服务器端命中判定、HP/护甲变化或网络包接受结果；本记录只覆盖客户端时间计算原语。

## 持续模式恢复记录（2026-09-11）

- 用户发出恢复指令后创建停止标志：`evidence/runtime/adddelay_repro.stop`（0 字节）。
- 日志出现停止处理并清除执行断点：`hits=6054`。
- 清除断点后的连续样本中，D 从 `4090.2917` 回落至 `2307.0184–2307.0199`，与原始正常基线一致。
- PID `2204` 的 `NarakaBladepoint` 仍存活；模块枚举确认 `AddDelayRepro.dll` 已卸载。
- 本次归档：`evidence/runtime/adddelay_repro_persistent_restore_20260911_110636.log`。
- 归档 SHA-256：`FE100FEB88DEDE3092DA3511D38674EC2CA61536AA4BB642519510EF3C2BBFA4`。
