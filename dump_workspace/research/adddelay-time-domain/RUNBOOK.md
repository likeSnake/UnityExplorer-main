# AddDelay 只读基线采样

## 产物

- DLL：`F:\gua\UnityExplorer-main\bin\Release\AddDelayProbe.dll`
- 配置：`F:\gua\UnityExplorer-main\bin\Release\AddDelayProbe.cfg`
- 日志：`F:\gua\UnityExplorer-main\dump_workspace\research\adddelay-time-domain\evidence\runtime\adddelay_probe.log`

## 操作

1. 保持 `NarakaBladepoint.exe` 在当前可重复观察的界面；先不要改变游戏内存或设置断点。
2. 在定制 CE 中选择 PID `18064`（若游戏重启，以当前 PID 为准）。
3. 使用 CE 的 DLL 注入功能加载 `AddDelayProbe.dll`。
4. 等待约 12 秒，确认日志文件生成并复制回本项目 `evidence/runtime/`。
5. 若游戏重启，旧日志不要覆盖，先改名为带时间戳的副本再重新采样。

## 判读门槛

- `[MODULE]` 必须显示实际 `GameAssembly.dll` 或 `GameAssembly_Super.dll`，并记录 base/size/timestamp。
- `[SYNC]` 必须出现 `name='CharactorSync' status=CLASS_OK`；仅有非零 klass 不算成功。
- `instanceKlass` 应为 `same=YES`；登录界面对象尚未初始化时，明确记录 `instance=0`。
- `[CODE]` 的普通样本目标应为 `F2 0F 5C 40 28` 才能与文档指令建立对应关系。Super 版本不能套用普通 RVA。
- `[SAMPLE]` 至少应有连续有效的 `D/R/G`。只看单个数值不能证明异常或恢复。

该 DLL 不写目标进程、不开启硬件断点、不调用游戏方法。异常复现必须在这份基线日志确认模块和对象链之后单独记录，避免把旧版本 RVA 或未初始化对象误判成机制复现。

## 当前持续复现实例

`AddDelayRepro.dll` 已改为持续模式并注入当前 PID `2204`。它会持续为游戏线程设置目标执行断点，直到检测到：

```text
F:\gua\UnityExplorer-main\dump_workspace\research\adddelay-time-domain\evidence\runtime\adddelay_repro.stop
```

收到关闭指令后，由维护者创建该空文件；DLL 会清除断点、移除 VEH、采样恢复 2 秒并自动卸载。持续运行期间 `adddelay_repro.log` 由 DLL 独占写入，读取失败属于预期现象。
