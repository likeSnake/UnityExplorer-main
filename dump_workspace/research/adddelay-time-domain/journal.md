# 执行日志：CharactorSync.AddDelay 时间域异常

## 2026-09-11

- `Observed`: 用户已启动 `NarakaBladepoint.exe`，CIM 查询得到 PID 18064。
- `Observed`: 来源目录 `F:\gua\e595_chs\` 当前只有修复建议文档，没有原始运行日志或复现脚本。
- `Inferred`: 需要先确认当前模块是普通版还是 Super/保护变体，再决定定位方式。
- `Assumed`: 文档中的 `0x156C395`、`F2 0F 5C 40 28` 只适用于它记录的普通版样本。
- `Observed`: 已构建 `F:\gua\UnityExplorer-main\bin\Release\AddDelayProbe.dll`，SHA-256 为 `C034D1D43F5CF19650BF32888AD4C0017C5882082B27E7EF48DDD91766C1714C`。
- `Observed`: 探针采样 40 次、间隔 250ms；只读 `CharactorSync` 的 `D@+0x10`、`R@+0x68` 和 `GameBaseObject._globalTime@+0x28`，并核对普通/ Super 候选 TypeInfo 与指令字节。
- `Observed`: 外部 `Get-Process` 能看到 PID 18064，但模块路径和模块枚举为空，符合项目记录的保护层行为。
- `Next`: 用户通过已验证 CE 通道注入探针，回传 `evidence/runtime/adddelay_probe.log`；在基线成立前不进行执行流改变。

## 普通版基线采样

- `Observed`: 当前 PID 为 2204；`GameAssembly.dll` base=`0x7FFB336B0000`、size=`0x1997F000`、timestamp=`0x6A969F1D`，与来源文档版本字段一致。
- `Observed`: `base+0x156C395` 为 `F2 0F 5C 40 28`；普通版 `CharactorSync` TypeInfo `0xE606490` 解析为 `CharactorSync`，实例 klass 回指通过。
- `Observed`: 40 次连续读取成功，`D` 稳定在 `2306.9938..2306.9978`，`R` 在 `0.005998..0.007193`，`G` 从 `194.5759106` 单调增长至 `204.3289862`。
- `Inferred`: 当前可证明“普通版目标基本块和字段链存在且可读”，尚不能证明跳过 `subsd`，因为本次没有改变执行流，也没有同场景的异常对照样本。
- `Next`: 在同一实际对局内先采集一次未干预基线；若需要异常对照，由用户明确执行其现有调试流程，探针只负责前后样本与恢复确认。

## 受控复现

- `Observed`: `AddDelayRepro.dll` 已通过 PID 2204 注入；目标指令命中 158 次，自动恢复后游戏进程仍存活。
- `Observed`: 基线 `D≈2307.00`；跳过期间 `D≈2939.20..2947.04`，与同期 `G≈632.22..640.06` 的增量对应；恢复后下一正常样本 `D=2307.003644378`。
- `Observed`: 复现前和恢复后只读探针均确认目标 RVA 字节 `F2 0F 5C 40 28`。
- `Inferred`: 当前样本完整复现文档的算术机制和恢复条件；未验证服务器端命中/伤害效果。
- `Artifacts`: `evidence/runtime/adddelay_repro_success_20260911.log`、`evidence/runtime/adddelay_probe_postrestore_20260911.log`、`2026-09-11_AddDelay复现验证.md`。
