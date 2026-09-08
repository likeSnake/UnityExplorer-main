# MCP 服务 AI 使用体验改进清单

> 用途：记录 AI/脚本通过 MCP 调用时遇到的"读写不友好"问题，供后续统一修改
> 完善。每条记录：现象 → 影响 → 建议改法。改动时同步更新
> `docs/mcp-server-usage.md` 与 `docs/mcp-server-next-agent-handoff.md`。

## 已修复（2026-09-04 轮）

| # | 问题 | 影响 | 改法 | 状态 |
| --- | --- | --- | --- | --- |
| 1 | `unity_memory_read` 返回的 `data` 是内存原始字节序（小端 hex），AI 必须自己反转字节序才能得到指针值，推链时极易出错 | 指针链解析繁琐、易错；本次曾因小端反转错误把 klass 指针推偏 | 新增 `unity_memory_read_ptr`（自动小端解释，返回 valueHex/valueDec/isCanonical/isUserPointer） | ✅ 已上线 |
| 2 | hex → 字符串（类名/动作名）需要 AI 手工 ASCII 解码 | 每读一个名字都要写转换代码 | 新增 `unity_memory_read_string`（ascii/utf16 自动解码到 NUL） | ✅ 已上线 |
| 3 | 多级指针链要逐级手工调用 read 并拼接地址 | vtable 链解析要十几步、易断 | 新增 `unity_pointer_chain_resolve`（一次调用按 base+offsets 逐级解引用，返回每跳地址/值） | ✅ 已上线 |
| 4 | 单次写上限 256B，CrossFade 内存振 shellcode 块（约 400B）放不下 | 无法整块注入 | `AnalysisSessionOptions::maxWriteBytes` 256 → 4096（4 KiB 单页） | ✅ 已上线 |

## 待改进（后续统一处理）

| # | 问题 | 影响 | 建议改法 | 状态 |
| --- | --- | --- | --- | --- |
| 5 | `unity_session_status` 总是返回完整模块列表（182 个），响应体积大、刷屏 | AI 只想看关键字段时输出被淹没；多次调用浪费带宽 | 增加可选参数 `includeModules:false`（默认 true 保持兼容），或新增 `unity_session_status_summary` | 待办 |
| 6 | `address` 参数 schema 只接受 string，传数字（PowerShell 变量序列化）会被拒 | AI 用脚本变量拼请求时容易把数字直接序列化导致 400 | 服务端 schema 允许 `address` 同时为 string 或 integer；`ParseUnsigned` 已支持两种 | 待办 |
| 7 | `tools/call` 的错误在 `structuredContent.error.code`，成功在 `structuredContent.data`，两层结构不一致 | 解析代码要分两支处理 | 统一 success/failure 的字段布局（或文档给出明确分支规则） | 待办 |
| 8 | PowerShell 环境函数名易撞系统别名（`R`=Invoke-History、`r` 等） | 脚本里短函数名会被解析成历史命令，报错难查 | 在 usage.md 提供"safe 命名规范"（如 `ReadMem`/`GetPtr` 前缀），或提供官方示例脚本 | 待办 |
| 9 | 指针链解析失败时缺少"已完成跳步"信息（旧行为） | 定位断点要重跑 | 已在新工具 `unity_pointer_chain_resolve` 中返回 `steps`（含失败跳步）；`unity_memory_read` 保持原样 | ✅ 已由 #3 覆盖 |
| 10 | 模块基址/RVA 计算要 AI 手工做算术 | 每次都要 `GA + 0xRVA` | 可选新增 `unity_module_rva_to_va {moduleName, rva}` 只读工具 | 待办 |
| 11 | 写操作前置条件多（expectedBefore/mask/readback），错误码细碎 | 首次接触者容易在参数上卡住 | usage.md 增加"写操作最小示例 + 常见错误对照表"（已有部分），补充 PowerShell 完整示例 | 待办 |

## 记录规范

- 新增条目：写清**现象 → 影响 → 建议改法**三要素，方便后续 AI 直接实现。
- 修改代码时：同步更新 usage.md（API 文档）和 handoff（交接事实），不要只改行为不改文档。
- 上线验证：每个工具改动后跑四个 smoke tests（AnalysisSessionSmoke /
  McpProtocolSmoke / HttpTransportSmoke / ModuleMatchSmoke），并记录退出码。
