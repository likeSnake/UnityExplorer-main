# CharactorSync.AddDelay 时间域异常复现

- **来源文档**: `F:\gua\e595_chs\网易安全团队修复建议.md`
- **目标进程**: `NarakaBladepoint.exe`（由用户启动，PID 以实测为准）
- **任务类型**: 只读运行时验证 / 逆向证据整理
- **当前阶段**: Evidence（普通版基线已完成）
- **成功条件**: 在当前游戏版本中确认模块身份、`CharactorSync.AddDelay` 的实际定位和正常时间域样本；如用户明确配合，再进行可回退的异常复现并记录恢复过程。
- **范围**: 当前游戏进程、其明确加载模块、项目内已有工具和用户提供的文档；不启动未知驱动、不修改原始样本。

## 证据目录

- `evidence/`: 原始命令输出、模块信息和采样记录
- `artifacts/`: 派生分析结果
- `scripts/`: 任务专用只读探针（尚未创建）
- `journal.md`: 执行链路与判断记录

文档中的 RVA 和样本属于 `Assumed/Claimed`，在当前进程中重新验证前不作为事实。

## 已确认的普通版基线（2026-09-11）

- `Observed`: 当前模块为 `GameAssembly.dll`，base=`0x7FFB336B0000`，SizeOfImage=`0x1997F000`，PE timestamp=`0x6A969F1D`。
- `Observed`: `base+0x156C395` 的 5 字节为 `F2 0F 5C 40 28`，与文档目标指令完全一致。
- `Observed`: `base+0xE606490 -> klass` 的类名为 `CharactorSync`，`staticFields` 有效，`staticFields+0x8` 实例回指同一 klass。
- `Observed`: `CharactorSync` 字段 `D@+0x10`、`R@+0x68` 与 `GameBaseObject._globalTime@+0x28` 均可连续读取。
- `Observed`: 本次 10 秒界面样本中 `D≈2306.994–2306.998`、`R≈0.0060–0.0072`、`G≈194.576–204.329`，数值稳定；因未处于文档样本的同一战斗状态，不能据此判定异常或正常攻击效果。

原始基线日志：`evidence/runtime/adddelay_probe_normal_baseline_20260911.log`。
