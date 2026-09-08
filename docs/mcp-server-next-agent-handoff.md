# UnityExplorer MCP 后续对接交接单

> 交接对象：下一位负责真实 DMA 联调、MCP 验收和后续扩展的 AI
>
> 工作区：`J:\Code\C++\dma\er_new\UnityExplorer-main`
>
> 目标：让 `UnityExplorerMcpServer.exe` 作为唯一 DMA owner 持续运行，之后由
> MCP client 对同一个 `AnalysisSession` 进行结构化读写调试。

## 交接时现场快照

以下是本次交接前最后一次只读检查的结果；下一位 AI 必须重新核实，不要把它们当
作永久状态：

- `NarakaBladepoint.exe`、`UnityExplorerMcpServer.exe`、MemProcFS/vmm 相关进程均
  未在进程列表中发现。
- TCP `127.0.0.1:18765` 和 `127.0.0.1:19002` 均未监听。
- 一次管理员启动尝试曾返回 PID `24912`，随后复查该 PID 已不存在；因此不能
  假设服务仍在运行，也不能只凭这个 PID 结束进程。
- 当前交互 shell 不是 Administrator；服务 EXE 的 manifest 要求
  `requireAdministrator`，必须由 elevated shell/VS 启动。
- Release EXE 最后已确认存在于 `App\x64\Release\UnityExplorerMcpServer.exe`；
  其依赖 DLL 应与 EXE 同目录。
- 本项目运行在 DMA 副机；目标 `NarakaBladepoint.exe` 在另一台主机。副机本地
  `Get-Process` 看不到目标并不表示 DMA 目标不存在，必须以 VMMDLL 的远端 PID
  查询和内存读结果判断。

## 本次真实联调记录（2026-09-04）

以下结果已在本轮交接中实际执行，作为后续回归基线；游戏重启或 DMA 重置后仍须
重新获取 generation 和模块基址：

- 服务以管理员权限运行，HTTP endpoint 为 `http://127.0.0.1:19002/mcp`，监听
  owner PID `25476`（本项目 `UnityExplorerMcpServer.exe`）。服务保持 running。
- 通过远端 DMA 成功连接 `NarakaBladepoint.exe`，VMMDLL 返回远端 PID `10344`，
  runtime=`il2cpp`，模块数 `182`，当前 generation=`3`。
- `GameAssembly_Super.dll`：基址 `0x7FFE08570000`，imageSize `440655872`；
  `UnityPlayer_LVB.dll`：基址 `0x7FFE22F20000`，imageSize `77533184`。
- 当前 generation 的两个模块基址各读取 64 bytes（`actual=64`），均以 `4d5a`
  开头（PE `MZ`）。
- 先 disconnect（generation `1 -> 2`）再 reconnect（generation `2 -> 3`）成功；
  使用旧 generation `2` 的 read 返回 `stale_generation`，服务和 DMA session 未崩溃。
- HTTP 已验证：错误 token=`401`、拒绝 Origin=`403`、错误路径=`404`、非 POST=`405`、
  不支持 Accept=`415`、body 超限=`413`；所有错误请求后端口仍保持监听。
- 默认只读策略已通过 MCP 验证：`status.writeEnabled=false`、`writePolicy=disabled`，
  `unity_memory_write` 未注册，直接调用返回 JSON-RPC `-32601 Unknown tool`。
- stdio elevated 子进程发送 discover/tools/list/status 后关闭 stdin，stdout 仅含
  JSON 行，stderr 仅含 ready 日志，退出码=`0`。
- 四个现有 smoke binary 本轮退出码均为 `0`；Release x64 服务重新构建成功。

## 接续稳定性复检记录（2026-09-04，同一服务实例）

本轮复检由下一位 AI 直接经 MCP 调用验证，服务进程 PID `25476`
（`UnityExplorerMcpServer.exe`）全程未重启，仍为唯一监听 `127.0.0.1:19002`
的实例，重复结果与上一节完全一致：

- 服务实例：`Get-CimInstance` 仅返回 `UnityExplorerMcpServer.exe` PID `25476`
  一条；`Get-NetTCPConnection` 中 `127.0.0.1:19002` 的 `OwningProcess=25476`，
  无第二实例、无端口冲突。
- 远端 session：`unity_session_status` 返回 `state=ready`、`pid=10344`、
  `targetProcess=NarakaBladepoint.exe`、`runtime=il2cpp`、`generation=3`、
  `modules.Count=182`、`pendingJobs=0`、`activeJob=false`、`lastError` 空。
- 只读策略未暴露/未执行写入：`tools/list` 仅注册 6 个工具
  （unity_memory_read、unity_modules_list、unity_session_connect、
  unity_session_disconnect、unity_session_modules、unity_session_status），
  `unity_memory_write` 未注册；直接调用 `unity_memory_write` 返回 JSON-RPC
  `-32601 Unknown tool`，未触发任何底层写。session 中 `writeEnabled=false`、
  `writePolicy=disabled`、`writeRanges=[]`（空）。
- 只读功能仍可用（gen 3）：`unity_memory_read {expectedGeneration:3,
  address:0x7FFE08570000, length:64, nocache:true}` 返回 `actual=64`，
  `data` 以 `4d5a`（PE `MZ`）开头，与上节模块基址一致。
- 复检完成后服务继续运行，未 disconnect、未推进 generation，仍可供后续 AI
  通过 MCP 调用。

结论：单一实例监听、远端 generation 仍为 3、默认只读策略不暴露也不执行
`unity_memory_write`，三项稳定性检查全部通过，服务保持常驻。

## 服务升级 + AI 友好只读工具记录（2026-09-04，服务已重启）

### 为什么升级

此前通过 MCP 做指针链/字符串解析时，AI 需要自己反转小端字节序、把 hex
转 ASCII、逐级手工拼接指针地址，操作繁琐且易错（曾因小端反转错误推偏
地址）。为提升后续 DMA 联调效率，直接改良了服务：

- 新增 3 个只读工具（`App/UnityExplorerMcpServer/main.cpp`）：
  - `unity_memory_read_ptr`：读 8 字节自动小端解释，直接返回
    `valueHex`/`valueDec`/`isCanonical`/`isUserPointer`；
  - `unity_memory_read_string`：读 NUL 结尾字符串，支持 `ascii`/`utf16`
    编码自动解码，返回文本；
  - `unity_pointer_chain_resolve`：单次调用按 `base + offsets[]` 逐级解
    引用指针链，每跳返回地址/值，空指针与失败带已完成跳步。
- `AnalysisSessionOptions::maxWriteBytes` 由 256 提升到 4096（4 KiB 单页），
  使 CrossFade 内存振 shellcode 块（约 400B）可单次写入、不跨页。
  `--enable-writes` 未启用，`unity_memory_write` 仍不注册。

### 升级操作与现场快照

- 原 PID `25476` 已停；新服务以管理员运行，监听 owner PID `204`，
  endpoint `http://127.0.0.1:19002/mcp`，Bearer token 不变
  （`codex-local-20260904-19002`）。重启脚本记录见
  `App\x64\Release\restart_mcp_server.log`；新二进制已复制到
  `App\x64\Release\UnityExplorerMcpServer.exe`（staging 目录可删）。
- 重新 connect 后：`state=ready`、`pid=10344`、`runtime=il2cpp`、
  `modules=182`、`generation=1`（新进程从 1 重新计数）。`GameAssembly_Super.dll`
  基址仍为 `0x7FFE08570000`（游戏未重启，地址空间不变）。
- `tools/list` 现返回 9 个工具：unity_session_status、unity_session_connect、
  unity_session_disconnect、unity_memory_read、unity_memory_read_ptr、
  unity_memory_read_string、unity_pointer_chain_resolve、unity_modules_list、
  unity_session_modules。
- 新工具实测（gen 1）：
  - `unity_memory_read_ptr @0x7FFE0BCDABF8` →
    `valueHex=0x20C414C9C30`（与手工解析一致）；
  - `unity_pointer_chain_resolve {base:0x7FFE0BCDABF8,
    offsets:[0,0xB8,0x8,0x70,0x90,0x0]}` → 6 跳全部成功，最终
    `0x20DE67BC440`；`vtableSlot=0x20DE67BC5D8` →
    `unity_memory_read_ptr` 得 `valueHex=0x7FFE0D4BBCD0`（GA 代码区，
    即 `vtbale_ret_address` 候选）；
  - `unity_memory_read_string @0x20DD4B613EC` → `AnimPlayable`。
- 四个 smoke tests 均退出码 0。

注意：generation 已从 3 变 1；后续任何带旧 generation 的请求都会返回
`stale_generation`，必须先重新 status/connect。

当前不要把上述 PID、地址或 generation 当作永久值；它们只对应本轮远端游戏实例。

## 直接交给下一位 AI 的提示词

```text
你正在接手 UnityExplorer-main 的 MCP 常驻服务改造。先阅读：

1. J:\Code\C++\dma\er_new\UnityExplorer-main\项目交接文档.md
2. J:\Code\C++\dma\er_new\UnityExplorer-main\docs\mcp-server-usage.md
3. J:\Code\C++\dma\er_new\UnityExplorer-main\docs\mcp-persistent-server-refactor-plan.md
4. 本文件 docs\mcp-server-next-agent-handoff.md

然后再执行命令或修改代码。工作区不是 Git repository；保留已有用户改动，
不要用 git reset/checkout 或删除无关文件。

当前任务优先级：
1. 验证/清理由本项目启动的 MCP 服务（只处理已核实的服务 PID，不要关闭
   NarakaBladepoint.exe，也不要结束未知 DMA/系统进程）。
2. 以管理员权限启动最新 Release x64 MCP 服务，确认进程、端口和日志。
3. 用独立 stdio 子进程验证 EOF 正常退出且没有 Shutdown 死锁。
4. 用 HTTP 验证 discover、tools/list、status、鉴权错误、Origin 错误和
   generation 失效行为。
5. 目标游戏实际运行后，再连接 NarakaBladepoint.exe，枚举模块并读取模块
   PE/DOS 头；保存本次返回的 generation，任何旧地址或旧 generation 都不可复用。
6. 只在证据充分时修改代码；每次修改后运行相关 smoke tests 并记录结果。

重要事实：
- 这是常驻进程，不是启动 CLI 后解析日志。HTTP endpoint 默认
  http://127.0.0.1:18765/mcp；推荐端口 19002。服务默认 loopback、Bearer token、
  只读；HTTP 每个 TCP 连接处理一条请求后关闭，但进程和 DMA session 持续存在。
- EXE manifest 要求 requireAdministrator。非 elevated shell 启动失败不代表代码失败，
  不要为了测试而移除 manifest；应使用 elevated PowerShell/VS 或记录权限阻塞。
- 目标退出后服务不会自动退出，也没有后台 health monitor；status 可能短暂仍为
  ready，下一次 read/modules/connect 才会发现目标失效。不要声称已实现自动重连。
- 当前已注册工具：unity_session_status、unity_session_connect、
  unity_session_disconnect、unity_memory_read、unity_memory_read_ptr、
  unity_memory_read_string、unity_pointer_chain_resolve、unity_modules_list，
  以及兼容别名 unity_session_modules。只有 --enable-writes 才注册
  unity_memory_write，且还必须配置 --write-range；写入最多 4096 bytes
  （4 KiB 单页，足够单次写入 CrossFade shellcode 块）、禁止跨 4 KiB page、
  要求 expectedBefore
  和强制 readback。
- 所有 DMA/VMMDLL/er2 操作必须经过同一个 SessionExecutor；禁止在 HTTP handler 或
  新线程中直接操作 er2::g_ctx、DmaData 或 VMMDLL。
- 当前 GOM/MSID/Transform/Camera/Metadata/Naraka Probe 尚未注册为 MCP tools；不要
  在交接报告中把它们写成已可通过 MCP 调用。

历史联调数据仅供比对，不能直接使用：上一次曾见到 PID 10344、generation 1、
GameAssembly_Super.dll 基址 0x7FFE08570000、UnityPlayer_LVB.dll 基址
0x7FFE22F20000；目标重启后这些值均视为过期，必须重新 connect/modules。
此前为服务启动尝试返回过 PID 24912；接手时先用进程路径和命令行确认它是否仍是
本项目服务，不能仅凭 PID 结束进程。

每个阶段都要报告：实际命令、退出码/HTTP 状态、关键 JSON 字段、目标 PID、
generation、模块名/基址，以及未完成项。若目标进程不在列表中，明确记录
target_not_found 并继续完成不依赖目标的协议测试，不要反复重启或关闭游戏。
```

## 已完成的实现基线

### 服务与协议

- `App/UnityExplorerMcpServer/main.cpp`：服务入口、参数解析、DMA backend、
  `ServiceState` 和工具注册。
- `App/UnityExplorerMcpServer/http_transport.cpp`：loopback Streamable HTTP，
  Bearer token、Origin allowlist、请求限制、JSON/SSE 响应和 worker 生命周期。
- `App/UnityExplorerMcpServer/stdio_transport.cpp`：逐行 JSON-RPC；日志只走
  stderr，stdin EOF 返回 0。
- `App/UnityExplorerMcpServer/mcp_server.cpp` 与
  `include/mcp/mcp_server.hpp`：JSON-RPC、schema 校验、scope 检查和工具分发。
- `include/analysis/analysis_session.hpp`：唯一 session、generation、模块快照、
  read/write policy、canonical address 和 stale-generation 检查。
- `include/analysis/session_executor.hpp`：有界队列（64）和单 worker 串行执行。

### 构建产物

当前已生成的 Release x64 文件：

```text
J:\Code\C++\dma\er_new\UnityExplorer-main\App\x64\Release\UnityExplorerMcpServer.exe
J:\Code\C++\dma\er_new\UnityExplorer-main\App\x64\Release\vmm.dll
J:\Code\C++\dma\er_new\UnityExplorer-main\App\x64\Release\leechcore.dll
J:\Code\C++\dma\er_new\UnityExplorer-main\App\x64\Release\FTD3XX.dll
```

解决方案：`App\UnityExplorerMcpServer\UnityExplorerMcpServer.sln`。可用
`J:\vs2026\MSBuild\Current\Bin\MSBuild.exe` 构建 `Release|x64`。构建日志中
已有成功记录 `UnityExplorerMcpServer.vcxproj -> App\x64\Release`；旧的 C++
兼容性 warning（MemProcFS 头文件零长度数组等）不是本次 MCP 改造引入的错误，
但新改动不能增加 error。

### 已通过的 smoke tests

以下现有二进制最近一次均返回退出码 0：

```text
tests\analysis_session_smoke\bin\x64\Release\AnalysisSessionSmoke.exe
tests\mcp_protocol_smoke\bin\x64\Release\McpProtocolSmoke.exe
tests\http_transport_smoke\bin\x64\Release\HttpTransportSmoke.exe
tests\module_match_smoke\bin\x64\Release\ModuleMatchSmoke.exe
```

这些测试覆盖 session generation、协议分发、HTTP 认证/Origin/Accept/路径、
模块匹配等；接手后若源码变更应重新构建并重跑，不能只引用旧结果。

## 真实联调步骤

### 1. 先确认占用者

只查询，不要先杀进程：

```powershell
Get-CimInstance Win32_Process |
  Where-Object { $_.Name -match 'UnityExplorerMcpServer|MemProcFS|NarakaBladepoint|dma' } |
  Select-Object ProcessId,Name,ExecutablePath,CommandLine
Get-NetTCPConnection -State Listen |
  Where-Object { $_.LocalPort -in @(18765,19002) } |
  Select-Object LocalAddress,LocalPort,OwningProcess
```

若发现占用端口的进程，必须核对 `ExecutablePath` 是本工作区的
`UnityExplorerMcpServer.exe` 后才可停止它。不要按名称批量结束 `NarakaBladepoint`、
`vmm` 或未知服务。服务停止后再次确认端口释放。

### 2. 启动 HTTP 服务

在 elevated PowerShell 中执行（token 换成一次性随机值，不要写入公开日志）：

```powershell
Set-Location 'J:\Code\C++\dma\er_new\UnityExplorer-main\App\x64\Release'
.\UnityExplorerMcpServer.exe `
  --transport http `
  --bind 127.0.0.1 `
  --port 19002 `
  --token 'replace-with-random-token'
```

若目标已经运行且希望启动即连接，可额外使用
`--target NarakaBladepoint.exe --auto-connect`；自动连接失败时服务仍应继续监听。
没有 `--auto-connect` 时，服务启动不打开 DMA，初始 session 应为 `idle`。

验证：

```powershell
Get-Process UnityExplorerMcpServer
Get-NetTCPConnection -State Listen -LocalPort 19002
```

### 3. HTTP 协议验收

每次请求使用 `POST http://127.0.0.1:19002/mcp`，并带：

```text
Authorization: Bearer <token>
Accept: application/json
Content-Type: application/json
```

依次调用：

```json
{"jsonrpc":"2.0","id":1,"method":"server/discover","params":{}}
{"jsonrpc":"2.0","id":2,"method":"tools/list","params":{}}
{"jsonrpc":"2.0","id":3,"method":"tools/call","params":{"name":"unity_session_status","arguments":{}}}
```

至少验证这些负向结果：错误 token=`401`、不在 allowlist 的 Origin=`403`、错误
路径=`404`、非 POST=`405`、不支持的 Accept=`415`、过大 body=`413`。服务进程和
监听端口在这些请求后都必须仍然存活。

注意：当前 transport 源码主要校验 `Accept`，调用方应始终发送
`Content-Type: application/json`。如果验收要求“缺失/错误 Content-Type 必须返回
415”，先在 `http_transport.cpp` 增加明确校验，再同步更新
`tests/http_transport_smoke/main.cpp` 和本目录文档；不要只改文字不改行为。

### 4. 连接真实目标并读取

目标进程名通常为 `NarakaBladepoint.exe`。先确认它存在，再调用：

```json
{
  "jsonrpc":"2.0",
  "id":10,
  "method":"tools/call",
  "params":{"name":"unity_session_connect","arguments":{
    "targetProcess":"NarakaBladepoint.exe",
    "mode":"dma",
    "timeoutMs":30000
  }}
}
```

成功后记录返回的 `session.generation`、`pid`、`runtime` 和完整模块列表。若出现
`module_ambiguous`，根据错误中列出的候选精确填写 `unityPlayerName` 或
`gameAssemblyName`，不要猜测 DLL 名称。然后用同一 generation 调用
`unity_modules_list`，再读取选定模块基址的 2 bytes/64 bytes，确认返回数据以
`4d5a`（PE `MZ`）开头。若返回 `target_not_found`、`cr3_failed` 或 `read_failed`，
先保留 `lastError` 和日志，确认游戏已完全启动后再重试。

### 5. generation 与关闭验收

- 保存当前 generation；带旧 generation 的 `read/modules/write` 必须返回
  `stale_generation`，且不能触发底层 DMA read/write。
- 调用 `unity_session_disconnect` 后确认状态回到 `idle`、PID/module 清空、
  generation 单调递增。
- 重新 connect 后必须使用新的 generation；不要把上一局的地址、PID 或模块基址
  复制到新请求。
- 发送 `Ctrl+C`/`SIGTERM` 后确认 HTTP worker 先结束，executor drain 完成，最后
  释放 `AnalysisSession`、`MetickAdapter`、VMM/FPGA handle；退出不应卡死。

## stdio 子进程验收

stdio 只适合单客户端协议测试，不提供 HTTP Bearer/Origin 认证。因 EXE 要求
管理员权限，优先在 elevated shell 启动；若无法自动处理 UAC，记录环境限制，
不要删除 manifest。向 stdin 逐行发送以下 JSON：

```text
{"jsonrpc":"2.0","id":1,"method":"server/discover","params":{}}
{"jsonrpc":"2.0","id":2,"method":"tools/list","params":{}}
{"jsonrpc":"2.0","id":3,"method":"tools/call","params":{"name":"unity_session_status","arguments":{}}}
```

关闭 stdin（EOF）后应收到对应响应并以退出码 0 结束；stdout 只能有 JSON 行，
日志只能在 stderr。重点观察是否再次出现 `ServiceState::Shutdown()` 锁重入或
executor join 死锁。

## 写入验收边界

默认只读。除非用户明确要求并给出经过验证的地址范围，不要启动
`--enable-writes`。启用时必须同时指定窄范围 `--write-range BASE:LENGTH`，并在
请求中提供：

- 当前 `expectedGeneration`；
- 偶数长度 hex `data`（最多 4096 bytes，即 4 KiB 单页；足以单次写入
  CrossFade 内存振 shellcode 块约 400B）；
- 同长度 `expectedBefore`；
- 可选同长度 `mask`；
- 服务端强制 readback，`verifyReadback=false` 也不能绕过策略。

必须验证 disabled、deny-all、越界、跨页、stale generation、before mismatch 和
readback mismatch 都在底层 `VMMDLL_MemWrite` 前或紧邻位置失败。不要用真实游戏
关键状态做首次写入；优先使用用户明确提供的、可恢复的测试页，并记录 before/after。

## 不要误判的已知边界

1. `AnalysisSession` 的 `generation` 是本项目 DMA 业务代次，不是 MCP 旧版
   `Mcp-Session-Id`；现代协议请求可直接使用 `server/discover`。
2. HTTP 连接短连接不表示 DMA session 短连接；session 在服务进程内复用。
3. 没有 health monitor/auto-reconnect；目标退出后不能仅凭 status=ready 断言内存
   仍可读。
4. `GameAssembly.dll`、`GameAssembly_Super.dll`、`GameAssembly_Super_IBT.dll` 的
   RVA 不可互换；历史 `Offset.h` 或旧 dump 只能作为候选，必须通过当前模块和
   TypeInfo/readback 重新验证。
5. UnityExplorer-main 与 `YJWJ_DMA_NEW` 是两个工程；不要自动把 MCP 读到的值写回
   `YJWJ_DMA_NEW\Naraka\Offset.h`，除非完成交接文档规定的多层证据验证。

## 完成标准与交付格式

下一位 AI 在结束前应给出：

```text
[环境]
- elevated: yes/no
- 目标进程: name/pid 或 not running
- MCP PID、endpoint、监听端口

[协议]
- discover/tools/list/status 结果
- 401/403/404/405/415/413（已验证项）
- stdio EOF 退出码

[DMA]
- connect code、pid、runtime、generation
- UnityPlayer/GameAssembly 实际名称与基址
- MZ read 结果
- disconnect/reconnect/stale_generation 结果

[代码与测试]
- 修改文件和原因
- 运行的构建命令、smoke tests、退出码
- 未完成项和明确阻塞原因
```

配套操作说明见 [mcp-server-usage.md](J:\Code\C++\dma\er_new\UnityExplorer-main\docs\mcp-server-usage.md)。
