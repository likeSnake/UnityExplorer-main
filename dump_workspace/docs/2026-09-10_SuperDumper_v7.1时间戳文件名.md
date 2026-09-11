# SuperDumper v7.1：dump 文件名增加时间戳

按用户要求，输出文件名从固定 `dump.cs` 改为：

```text
dump_YYYYMMDD_HHMMSS_mmm.cs
示例：dump_20260910_160530_123.cs
```

使用运行开始时的本机时间，精确到毫秒。输出目录仍取 SuperDumper.cfg。日志文件仍为 log.txt，并新增 `[0] dumpFile=完整路径 (timestamp=local run start)`，便于定位本轮产物。

仅修改 SuperDumper.cpp 的输出路径生成和版本日志，以及项目 PDB 名称。dump 正文和 v7 偏移摘要协议保持兼容；使用 YJWJOffset_new.py 提取时，`--dump` 指向实际带时间戳的文件。

交付：

- DLL：`F:\gua\UnityExplorer-main\bin\Release\SuperDumper.dll`
- 日志版本：`SuperDumper v7.1 (timestamped dump files)`
- 大小：437760 bytes。
- SHA256：`2A9071A711C1CA06A5D999C7CBDD7FFCB75F27D51F18D3C3CB539CF757FCA576`
- PDB：`bin\Release\SuperDumper_v7_1_Release.pdb`

验证：MSVC v143 / Release x64 构建成功；检查输出路径仅由时间戳名称生成，没有残留固定 dump.cs 赋值。只有原有 /EHs 被 /EHa 覆盖提示。本次未注入游戏，后续由用户运行验证；未改动已有 dump 文件。
