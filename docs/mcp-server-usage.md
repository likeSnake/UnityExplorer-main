# UnityExplorer MCP 常驻服务使用说明

本文说明 `UnityExplorerMcpServer.exe` 的实际运行方式。服务是一个常驻的
Windows x64 进程：它直接通过 `MetickAdapter -> VMMDLL/MemProcFS -> FPGA DMA`
访问目标进程，不是启动一次 CLI 后解析日志。日志只用于诊断，AI 的读写结果
通过 MCP 的 JSON-RPC 响应返回。

本项目通常运行在 DMA 副机，`NarakaBladepoint.exe` 位于另一台主机。目标 PID、
模块路径和内存内容均由 FPGA/VMMDLL 从远端采样得到；副机上的
`Get-Process`/`tasklist` 看不到目标是正常现象，不能据此判断 `target_not_found`。
是否找到目标只以 `unity_session_connect` 的 DMA 返回结果为准。

## 0. 构建与运行文件

要求 Windows x64、Visual Studio 2022/MSVC v143，以及可用的 MemProcFS/VMMDLL
和 FPGA 运行环境。打开 `App/UnityExplorerMcpServer/UnityExplorerMcpServer.sln`，
选择 `Release` + `x64` 后生成。项目输出目录为：

```text
App\\x64\\Release\\UnityExplorerMcpServer.exe
```

构建目标会把 `vmm.dll`、`leechcore.dll`、`FTD3XX.dll` 复制到输出目录；启动时让
这三个 DLL 与 EXE 位于同一目录。项目 manifest 要求管理员权限，UAC 提示被取消
或使用非 elevated shell 时，DMA 初始化可能失败。

## 1. 启动服务

建议先启动 HTTP 服务，再由 MCP client 调用 `unity_session_connect`。这样目标
游戏尚未启动时服务仍保持运行并处于 `idle`；目标在已连接后退出或重启时，服务进程
不会自动退出，但当前 session 不会自动重连，须重新执行 connect（必要时先
disconnect）。当前版本没有后台 health monitor：目标退出、PID 复用或模块变化会在
下一次 `connect`、`modules` 或 `read` 时报告错误；服务不会仅因目标异常就自行变成
`faulted`。

在包含 `UnityExplorerMcpServer.exe` 和三个运行库 DLL 的目录执行：

```powershell
cd J:\Code\C++\dma\er_new\UnityExplorer-main\App\x64\Release
.\UnityExplorerMcpServer.exe `
  --transport http `
  --bind 127.0.0.1 `
  --port 19002 `
  --token "replace-with-a-long-random-token"
```

默认 endpoint 是 `http://127.0.0.1:18765/mcp`；上例固定为
`http://127.0.0.1:19002/mcp`。`--token` 也可以省略并通过
`UNITY_EXPLORER_MCP_TOKEN` 环境变量提供；若两者都没有，服务会生成一次性
token 并写到 stderr。该 token 在本次服务进程生命周期内有效，不放在 URL 中。

浏览器型 client 需要为每个来源显式加入精确 allowlist，例如：

```powershell
--allow-origin http://127.0.0.1:3000
```

没有 `Origin` 的本机后台请求默认放行；带有 `Origin` 时必须与某个 allowlist
值完全一致（scheme、host、port 均参与比较）。

服务默认只监听 loopback。不要把 `--bind` 改成 LAN 地址来替代认证和 TLS。

如果希望 HTTP 监听前自动尝试连接目标，必须同时提供 `--target` 和
`--auto-connect`：

```powershell
.\UnityExplorerMcpServer.exe --transport http `
  --target NarakaBladepoint.exe --auto-connect
```

自动连接使用默认 `timeoutMs=30000`；失败时写 stderr，但服务仍会继续监听。默认
不使用这两个参数更适合目标会重启的场景：先让服务处于 `idle`，再通过
`unity_session_connect` 连接。`--auto-connect` 不能单独使用，`--target` 仅是
启动时的目标名，MCP connect 仍需提供 `targetProcess`。

Release 输出目录应同时包含：

```text
UnityExplorerMcpServer.exe
vmm.dll
leechcore.dll
FTD3XX.dll
```

## 2. MCP 请求

每个 HTTP 请求都应发送：

```text
Authorization: Bearer <token>
Accept: application/json
Content-Type: application/json
```

现代协议版本为 `2026-07-28`。服务接受不带
`MCP-Protocol-Version` 的标准请求；如果 client 发送该 header，值必须与服务
版本一致。HTTP 只接受 `POST /mcp`，请求体是 JSON-RPC 2.0；也支持批量请求和
`text/event-stream` 响应。

请求体应使用 `Content-Type: application/json`，`Accept` 至少包含
`application/json` 或 `text/event-stream`。缺失或错误 Bearer token 返回 `401`；
带有 `Origin` 但不在精确 allowlist 返回 `403`；路径不是 `/mcp` 返回 `404`，其它
HTTP method 返回 `405`，请求体超过限制返回 `413`。默认允许没有 `Origin` 的 native
client，但仍必须通过 Bearer authentication。

当前 transport 每个 TCP 连接只处理一条请求并返回后关闭；“常驻”指服务进程和
DMA session 的生命周期，不是 HTTP keep-alive。`Mcp-Method`、`Mcp-Name` 是可选的
legacy mirrored headers，若发送必须与 body 中每一条 JSON-RPC 请求一致，通常可
省略。常见 HTTP 错误为 `401`（token）、`403`（Origin）、`404`（路径）、`405`
（方法）、`415`（Accept）、`413`（请求体过大）和 `503`（并发 worker/资源已满）。

PowerShell 示例（把 token 替换为启动服务时的值）：

```powershell
$token = "replace-with-a-long-random-token"
$headers = @{
  Authorization = "Bearer $token"
  Accept = "application/json"
  "Content-Type" = "application/json"
}
$body = '{"jsonrpc":"2.0","id":1,"method":"server/discover","params":{}}'
Invoke-RestMethod -Uri http://127.0.0.1:19002/mcp `
  -Method Post -Headers $headers -Body $body
```

`tools/call` 的业务失败仍是正常 JSON-RPC result，但其中 `isError` 为
`true`，并带稳定的 `error.code`。解析 client 返回的
`result.structuredContent` 即可获得机器可读数据；`result.content[0].text`
是等价的 JSON 文本。

## 3. 典型调用顺序

### 3.1 发现工具和检查状态

推荐先发送 `server/discover`，确认 protocol version 和 `tools` capability：

```json
{"jsonrpc":"2.0","id":1,"method":"server/discover","params":{}}
```

兼容旧 client 的 `initialize` 和健康检查 `ping` 也受支持；
`notifications/initialized` 是 notification，不会返回 response。

```json
{"jsonrpc":"2.0","id":2,"method":"tools/list","params":{}}
```

只读默认暴露以下工具：

```text
unity_session_status
unity_session_connect
unity_session_disconnect
unity_memory_read
unity_memory_read_ptr
unity_memory_read_string
unity_pointer_chain_resolve
unity_modules_list
unity_session_modules   (unity_modules_list 的兼容别名)
```

`unity_session_status` 不需要目标进程已连接：

```json
{"jsonrpc":"2.0","id":3,"method":"tools/call","params":{"name":"unity_session_status","arguments":{}}}
```

重点字段包括 `state`（`idle`、`connecting`、`ready`、`draining` 或 `faulted`）、
`generation`、`pid`、`targetProcess`、`runtime`、`modules`、`lastError`、
`writeEnabled`、`writePolicy`、`writeRanges`、`pendingJobs` 和 `activeJob`。

### 3.1.1 AI 友好的只读辅助工具

以下三个工具专门为 AI/脚本调用设计，把常见的指针解引用与字符串读取
封装成一步完成，返回结果已经过字节序/编码处理，调用方无需自己反转
hex 或转换 ASCII：

```json
{
  "jsonrpc":"2.0","id":31,
  "method":"tools/call",
  "params":{"name":"unity_memory_read_ptr","arguments":{
    "expectedGeneration":1,"address":"0x7FFE08570000"
  }}
}
```

- `unity_memory_read_ptr`：读取 8 字节并按小端解释为指针。返回
  `valueHex`（如 `0x0000020C414C9C30`）、`valueDec`、`raw`（原始 16 位
  hex）、`isCanonical`、`isUserPointer`。省去手工字节反转。
- `unity_memory_read_string`：读取 NUL 结尾字符串，默认 `encoding=ascii`
  （字节串到第一个 `\0` 为止）；`encoding=utf16` 时按 UTF-16LE 解码为
  UTF-8。可用 `maxBytes` 限制（默认 256，上限 4096）。返回 `text`、
  `actual`、`truncated`。
- `unity_pointer_chain_resolve`：在单个 executor job 内逐级解引用指针链。
  参数 `base` + `offsets` 数组（如
  `{"base":"0x7FFE08570000+0x376ABF8","offsets":[0xB8,0x8,0x70,0x90,0x0]}`
  的语义为：`base` 起，每级读 8 字节指针，地址为当前值+下一偏移）。
  返回每一跳 `{index,offset,address,valueHex,valueDec,isCanonical,
  isUserPointer}`，以及 `finalValueHex/finalValueDec`；命中空指针返回
  `null_pointer`，读取失败返回 `read_failed` 并带已完成的跳步，便于定位。
  该工具替代多次 `unity_memory_read` 手工拼接指针链。

### 3.2 连接一次并复用 DMA session

```json
{
  "jsonrpc":"2.0",
  "id":4,
  "method":"tools/call",
  "params":{
    "name":"unity_session_connect",
    "arguments":{
      "targetProcess":"NarakaBladepoint.exe",
      "mode":"dma",
      "timeoutMs":30000
    }
  }
}
```

连接成功后保存返回的 `session.generation`。服务只初始化一次 VMM/CR3，并在
后续 read/modules 调用中复用同一个 session。若进程加载多个
`UnityPlayer*.dll` 或 `GameAssembly*.dll`，必须通过 `unityPlayerName` 或
`gameAssemblyName` 显式选择，否则返回 `module_ambiguous`。

### 3.3 读取模块和内存

```json
{
  "jsonrpc":"2.0",
  "id":5,
  "method":"tools/call",
  "params":{
    "name":"unity_modules_list",
    "arguments":{"expectedGeneration":1}
  }
}
```

```json
{
  "jsonrpc":"2.0",
  "id":6,
  "method":"tools/call",
  "params":{
    "name":"unity_memory_read",
    "arguments":{
      "expectedGeneration":1,
      "address":"0x7FFE08570000",
      "length":64,
      "nocache":true
    }
  }
}
```

地址可用十进制或 `0x` 十六进制字符串。响应中的 `data` 是小写 hex 字符串，
`actual` 表示实际读取字节数。当前单次 read 上限为 64 KiB，并拒绝零地址、
溢出及非 canonical user address。

### 3.4 断开和重连

```json
{
  "jsonrpc":"2.0",
  "id":7,
  "method":"tools/call",
  "params":{
    "name":"unity_session_disconnect",
    "arguments":{"expectedGeneration":1}
  }
}
```

每次 disconnect 或 replacement connect 都会推进 `generation`。任何带旧
`expectedGeneration` 的地址型请求都返回 `stale_generation`，不能继续访问旧
目标地址。当前没有后台 health monitor：目标退出后 status 可能暂时仍显示
`ready`，通常要到下一次 read/modules/write 或 connect 才会发现 DMA/进程已失效。
目标游戏重启后应重新调用 connect，并使用新返回的 generation。

## 4. 写入策略

默认构建和默认启动均为只读，`tools/list` 中不会出现
`unity_memory_write`。只有指定 `--enable-writes` 才会注册该 tool；若没有任何
`--write-range`，策略仍为 `deny_all`，实际写入会被拒绝。要真正允许写入，服务
启动参数和每次请求的保护条件都必须满足：

- 指定 `--enable-writes`；
- 至少指定一个 `--write-range BASE:LENGTH`。

命令行中的 `--write-range` 没有 `--enable-writes` 时会直接被参数校验拒绝。

```powershell
.\UnityExplorerMcpServer.exe `
  --transport http --bind 127.0.0.1 --port 19002 `
  --token "replace-with-a-long-random-token" `
  --enable-writes `
  --write-range 0x000001F000000000:0x1000
```

每次写入还必须提供：

- 当前 `expectedGeneration`；
- `address`、偶数长度 hex `data` 和同长度 `expectedBefore`；
- 地址完整落在单个配置 allowlist，且不能跨页；allowlist 是半开区间
  `[BASE, BASE+LENGTH)`，请求不能拼接相邻区间；
- `data` 非空且最多 4096 bytes（4 KiB，与单页一致，足以一次写入 CrossFade
  内存振 shellcode 块），单次请求不能跨 4 KiB page；地址和末地址必须是
  low canonical user address；
- 可选 `mask` 也必须与 `data` 同长度，比较 `expectedBefore` 时只检查 mask 为 1
  的位；
- 服务先 direct read 比较 `expectedBefore`，写入后强制 readback（即使请求中的
  `verifyReadback` 为 false，当前 session policy 仍要求校验）；
- 比较失败、越界、旧 generation、readback 不一致都会在底层 write 前或紧邻
  写入处返回明确错误。

示例：

```json
{
  "jsonrpc":"2.0",
  "id":8,
  "method":"tools/call",
  "params":{
    "name":"unity_memory_write",
    "arguments":{
      "expectedGeneration":1,
      "address":"0x000001F000000100",
      "data":"01000000",
      "expectedBefore":"00000000",
      "verifyReadback":true
    }
  }
}
```

没有明确的 typed action 和经过验证的 allowlist 时，不建议启用 raw write。

## 5. stdio 与 HTTP 的区别

`--transport stdio` 保留用于单客户端协议测试：client 启动子进程，通过 stdin
逐行发送 JSON-RPC、从 stdout 逐行读取响应；日志只写 stderr。它不适合“先让
服务常驻、之后让多个 AI 连接”的场景。

stdio transport 不执行 HTTP 层的 `Bearer` 或 `Origin` 检查；调用方必须自行保护
子进程的 stdin/stdout 管道。它在 stdin 遇到 EOF 时正常返回并结束服务进程，输入
中的空行默认忽略，notification（例如 `notifications/initialized`）不产生输出。
因此 stdio 的“常驻”只在父 client 持续保持管道时成立。

`--transport http` 是推荐的常驻模式：服务独立运行，多个 MCP client 可以先后
连接；HTTP 线程只负责协议和鉴权，所有 DMA/`er2` 操作进入同一个 bounded
`SessionExecutor` 串行队列，因此不会并发污染全局 VMM/Scatter 状态。

## 6. 生命周期与并发

服务启动时 session 默认是 `idle`，不会因为没有目标进程而退出。一次成功的
`connect` 建立并复用一个 VMM/CR3/DMA session；`disconnect`、replacement
`connect` 和进程重启后的重新连接都会推进 `generation`。当前没有后台 health
monitor，目标退出、PID 复用或模块改变要等下一次 `read`、`modules` 或 `connect`
才会被发现。

所有会触碰 `AnalysisSession`、VMMDLL 或 `er2` 全局 context 的操作都提交到一个
`SessionExecutor` worker 串行执行。executor 的待处理队列上限为 64；队列满时
返回业务错误 `busy`。HTTP transport 最多同时保留 32 个 request worker，超过
上限的连接直接返回 HTTP `503`/`busy`。`unity_session_status` 中的
`pendingJobs` 是待处理数量，`activeJob` 表示唯一的执行中任务；二者不会让 DMA
操作并行化。

收到 `Ctrl+C`（`SIGINT`）或 `SIGTERM` 后，HTTP transport 先停止监听并等待已有
worker 返回，再由 service 停止 executor、等待活动 DMA job，最后按顺序断开
`AnalysisSession`、释放 `MetickAdapter` 和 VMM/FPGA 句柄。停止阶段不应再发送新
请求；已在途请求完成后服务才退出。

## 7. 安全边界

- 当前命令行服务只接受 IPv4 loopback `127.0.0.1`；HTTP transport 默认拒绝非
  loopback bind。不要把它当作远程服务，也不要用端口转发绕过这一边界。
- HTTP 没有 TLS。Bearer token 是粗粒度的进程级凭据，所有通过认证的 client
  共享同一组 `read`/`session.admin` 权限（启用写入时还共享 `memory.write`），
  当前没有 per-client scope 隔离。`Origin` allowlist 只限制带 Origin 的浏览器
  请求，不替代 token；没有 Origin 的 native client 仍必须带 token。
- 默认写入关闭。启用写入必须同时使用 `--enable-writes` 和至少一个窄范围的
  `--write-range`；建议只允许实验所需页面并单独保存 token，避免把 token 放入
  shell 历史或公开日志。
- DMA 读写拥有目标进程级权限。服务日志用于诊断，不是权限审计；需要审计时应
  在 MCP client 或外层代理记录请求和响应。

## 8. 常见错误

| code | 处理 |
| --- | --- |
| `not_connected` | 先调用 connect，或检查 status |
| `target_not_found` | 确认进程名；服务会继续运行等待下一次 connect |
| `dma_init_failed` | 检查 VMM/VMMDLL、MemProcFS、FPGA DLL 和管理员权限；失败后可重试 connect |
| `cr3_failed` | 目标已找到但 DTB/CR3 校验失败；确认目标已完全启动后再连接 |
| `module_not_found` | 目标缺少 UnityPlayer 或请求的 GameAssembly；检查模块名和位数 |
| `module_ambiguous` | 指定精确的 UnityPlayer/GameAssembly 名称 |
| `stale_generation` | 重新 status/connect，使用最新 generation |
| `busy` | 等待当前请求完成后重试 |
| `invalid_argument` | 检查数字、地址 canonical、长度上限、hex 数据和跨页/溢出条件 |
| `read_failed` | DMA/VMMDLL 读取失败；确认目标仍在运行后重新 connect |
| `permission_denied` | 检查 tool scope 或写入启动策略 |
| `compare_failed` | `expectedBefore`（按 `mask`）与目标内存不一致；重新 read 后再决定是否写入 |
| `write_failed` | 后端写入失败；不要自动重复写入，先确认目标状态 |
| `readback_failed` | 写入后的强制 readback 失败；重新检查 DMA session |
| `readback_mismatch` | readback 与请求数据不同；视为写入结果不确定，重新读取确认 |
| `401` | Bearer token 缺失或不匹配 |
| `403` | `Origin` 不在 `--allow-origin` 精确列表 |
| `404` | URL 必须是配置的 `/mcp` endpoint |
| `405` | 只支持 `POST /mcp` |
| `415` | `Accept` 必须包含 `application/json` 或 `text/event-stream` |
| `413` | 请求体超过 4 MiB 默认上限 |
| `503` | HTTP worker 达到 32 个并发上限；稍后重试 |

停止服务时先停止接收请求，再等待当前 DMA job 完成并释放 VMM/FPGA 句柄；不要
关闭 `NarakaBladepoint.exe` 来解决服务端口或 DMA 占用问题。

## 9. 当前能力边界

本版本已经提供持久 session、模块枚举、通用 memory read 和受策略保护的可选
raw write。GOM、MSID、Transform、Camera、Metadata 及 Naraka 专用 Probe 仍在
现有 GUI/Headless 前端中，尚未注册为 MCP tools；后续应复用同一
`AnalysisSession/SessionExecutor`，不能再创建第二个 DMA owner。
