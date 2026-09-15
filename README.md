<div align="center">

# ExternalResolve202x

**Unity 2020-2023 运行时内存结构算法还原库（er2 / header-only）**

*跨进程读取 | Header-Only | IL2CPP/MONO*

![C++](https://img.shields.io/badge/C%2B%2B-17-blue?style=flat-square)
![Platform](https://img.shields.io/badge/Platform-Windows%20x64-lightgrey?style=flat-square)
![Header Only](https://img.shields.io/badge/Header--Only-Yes-green?style=flat-square)

</div>

---

> [!NOTE]
> **版本兼容性说明**  
> 本项目面向 Unity 2020~2023（Windows x64）。对比 Unity2020 更早版本在部分结构偏移与布局上存在差异。  
> 支持 Mono 与 IL2CPP 两种后端（AutoInit 自动识别）。   
> Unity6版本请移步至仓库:https://github.com/zushinzackery2-ship-it/Unity6-eXternalrEsolve   

> [!IMPORTANT]
> **代码重构说明**  
> 当前项目包含'**相当大量**'的AI重构代码，可能存在大量维护性问题。  
> 功能实现集中在 `include/er2`，入口侧尽量保持薄封装。

## 访问器

- **最小内存访问抽象**：以 `IMemoryAccessor` 为核心，算法层只依赖读内存接口。
- **Windows 适配**：默认提供 WinAPI 实现（`ReadProcessMemory`）适配器。
- **Header-only**：纯头文件库。

## 功能概览

| 功能 | 说明 |
|:-----|:-----|
| **AutoInit + 上下文管理** | 自动发现 Unity 进程、定位 UnityPlayer/GameAssembly (IL2CPP) 或 Mono 模块、缓存关键 offset/全局槽，并暴露 `g_ctx + Mem()` 读写入口 |
| **跨进程内存访问** | 以 `IMemoryAccessor` 为核心，提供 WinAPI 适配，所有算法都只依赖统一的读内存接口 |
| **GOM 扫描与遍历** | 盲扫 GameObjectManager、校验桶结构、枚举 GameObject/组件，支持按 tag/名称/组件类型快速搜索 |
| **MSID 全局注册表** | 枚举 `UnityEngine.Object` 实例，筛选 GameObject/ScriptableObject，读取名称、InstanceID、托管类型等信息 |
| **Transform / Camera / W2S** | 解析 Transform 层级得到世界坐标；读取相机视图投影矩阵并完成世界坐标到屏幕坐标转换 |
| **Bones** | 遍历 Transform 子树获取骨骼索引、名称与世界坐标 |
| **IL2CPP Metadata + Hint 导出** | 自动扫描 metadata header、导出 `global-metadata.dat`，并可生成 `*.hint.json` |
| **DumpSDK2 工具链** | 结合 metadata/hint 结果生成 C# API 描述与泛型结构信息，辅助离线分析/SDK 导出 |
| **模块化 Header-only 设计** | `er2/unity2/*` 下分门别类的子模块（gom/msid/object/camera/transform/metadata 等），可按需引用 |

---

## 核心 API 列表

| 模块 | 代表 API | 说明 |
|:----|:---------|:-----|
| **上下文 / AutoInit** | `AutoInit()` / `IsInited()` | 自动发现 Unity 进程、刷新 `g_ctx` |
|  | `ReadPtr(addr)` / `ReadValue<T>(addr)` | 基于 `g_ctx + Mem()` 的统一读内存封装 |
| **GOM** | `GomManager()` / `GomBucketsPtr()` / `FindGameObjectThroughTag(tag)` | 访问 GameObjectManager、遍历桶并按 tag 搜索 |
|  | `EnumerateGameObjects()` / `GetGameObjectByName(name)` | 列举 GameObject，或按名称精确查找 |
| **MSID** | `MsIdToPointerSlotVa()` / `MsIdCount()` | 读取 ms_id_to_pointer set 元数据 |
|  | `FindObjectsOfTypeAll(ns, name)` | 枚举或按命名空间+类型名查找 `UnityEngine.Object` 实例 |
| **对象/名称** | `ReadGameObjectName(nativeGo)` | 读取 Native/Managed 对象名称 |
| **Transform / Camera / W2S** | `GetTransformWorldPosition(transformPtr)` | 解析层级状态，输出世界坐标 |
|  | `FindMainCamera()` / `GetCameraMatrix(nativeCamera)` | 找主相机、读取视图投影矩阵 |
| **Metadata / Hint** | `ExportGameAssemblyMetadataByScore()` | 一次性导出 metadata bytes 与 hint json |
| **DumpSDK2** | `DumpSdk` 相关接口 | 生成 C# API 与泛型结构描述 |

---

## AutoInit 后可直接调用的 API

调用 `er2::AutoInit()` 成功后，以下 API 可直接使用，无需手动传递 mem/offsets 等参数。

| 分类 | API | 说明 |
|:-----|:----|:-----|
| **上下文** | `Pid()` | 返回目标进程 PID |
|  | `IsInited()` | 是否已初始化 |
|  | `Runtime()` | 返回 Backend 类型 (IL2CPP/Mono) |
|  | `UnityPlayerBase()` | 返回 UnityPlayer.dll 基址 |
|  | `GomGlobalSlotVa()` | 返回 GOM 全局槽虚拟地址 |
|  | `MsIdToPointerSlotVa()` | 返回 MSID 槽虚拟地址 |
|  | `Mem()` | 返回内存访问器 `WinApiMemoryAccessor` |
|  | `Off()` | 返回核心偏移量结构 `Offsets` |
|  | `GomOff()` | 返回 GOM 偏移量结构 `GomOffsets` |
|  | `CamOff()` | 返回相机偏移量结构 `CameraOffsets` |
|  | `TransformOff()` | 返回 Transform 偏移量结构 `TransformOffsets` |
|  | `ReadPtr(addr)` | 读取指针，返回 `optional<uintptr_t>` |
|  | `ReadValue<T>(addr)` | 读取任意类型值，返回 `optional<T>` |
| **GameObject** | `EnumerateGameObjects()` | 枚举所有 GameObject，返回 `optional<vector<GameObjectEntry>>` |
|  | `FindGameObjectThroughTag(tag)` | 按 Tag 查找第一个 GameObject，返回 `uintptr_t` |
|  | `GetGameObjectByName(name)` | 按名称查找所有同名 GameObject，返回 `vector<uintptr_t>` |
| **组件** | `GetAllComponents(go)` | 获取 GameObject 的所有组件，返回 `vector<uintptr_t>` |
|  | `GetComponentThroughTypeId(go, typeId)` | 按 TypeId 获取组件 |
|  | `GetComponentThroughTypeName(go, typeName)` | 按类型名获取组件 |
|  | `GetTransformComponent(go)` | 获取 Transform 组件 |
|  | `GetCameraComponent(go)` | 获取 Camera 组件 |
| **Transform** | `GetTransformWorldPosition(transform)` | 获取 Transform 世界坐标 |
|  | `GetBoneTransformAll(rootGo)` | 获取骨骼列表 |
| **相机 / W2S** | `FindMainCamera()` | 查找主相机 |
|  | `GetCameraMatrix(cam)` | 获取相机 ViewProj 矩阵 |
|  | `WorldToScreenPoint(viewProj, screen, pos)` | 世界坐标转屏幕坐标 |
| **MSID** | `FindObjectsOfTypeAll(className)` | 按类名查找所有实例，返回 `vector<FindObjectsOfTypeAllResult>` |
| **Metadata** | `ExportGameAssemblyMetadataByScore()` | 导出 metadata 字节 |

> **返回值说明**：返回 `optional<T>` 的函数成功时有值，失败时为空；返回 `bool` + out 参数的函数成功返回 true。

---

## 编译要求

- **C++ 标准**：C++17
- **编译器**：MSVC（Visual Studio 2022）
- **平台**：Windows x64
- **链接方式**：静态运行时（/MT）
- **第三方库**：需要 [GLM](https://github.com/g-truc/glm)（用于矩阵运算），请自行下载并配置 include 路径

---

<details>
<summary><strong>目录结构</strong></summary>

```
External-oldVersion4Unity202x/
├── Resolve202x.hpp             # 统一入口头文件
├── include/
│   └── er2/
│       ├── core/               # 基础类型定义
│       ├── mem/                # 内存访问抽象
│       ├── os/win/             # Windows 平台实现
│       └── unity2/
│           ├── camera/         # 相机与 W2S
│           ├── core/           # 偏移定义
│           ├── dumpsdk/        # SDK 导出
│           ├── gom/            # GameObjectManager
│           ├── init/           # AutoInit 封装层
│           ├── metadata/       # IL2CPP 元数据
│           ├── msid/           # InstanceID 映射
│           ├── object/         # Native/Managed 对象
│           ├── transform/      # Transform 世界坐标
│           └── util/           # 工具函数
├── Analysis/
│   ├── Algorithms/             # 算法说明文档
│   └── Structures/             # 结构说明文档
├── tools/                      # 示例工具
└── docs/                       # 补充文档
```

</details>

---

## 快速开始

```cpp
#include "Resolve202x.hpp"
#include <cstdio>

int main()
{
    // 自动发现 Unity 进程并初始化上下文
    if (!er2::AutoInit())
    {
        return 1;
    }

    // 输出环境信息
    printf("Runtime: %s\n", er2::Runtime() == er2::ManagedBackend::Il2Cpp ? "IL2CPP" : "Mono");

    // 查找主相机
    const std::uintptr_t cam = er2::FindMainCamera();
    if (cam)
    {
        // 获取相机矩阵
        auto viewProjOpt = er2::GetCameraMatrix(cam);
        if (viewProjOpt.has_value())
        {
            // ...
        }
    }

    // 按名称查找游戏对象
    auto controller = er2::GetGameObjectByName("Controller");
    if (controller) 
    {
        printf("Controller: 0x%llX\n", (unsigned long long)controller);
    }

    return 0;
}
```

---

## AutoInit 与 init/* 封装

- `er2::AutoInit()` 会自动定位 Unity 进程并填充全局上下文 `g_ctx`
- `include/er2/unity2/init/*` 提供基于 `g_ctx + Mem()` 的薄封装，尽量减少手动传参

---

<div align="center">

**Platform:** Windows x64 | **License:** MIT

</div>

---

## MCP 常驻服务（HTTP + JSON-RPC 持续读写 DMA）

本项目除了 GUI 分析器和 Headless CLI 之外，还提供一个**常驻 MCP 服务** `UnityExplorerMcpServer.exe`：它直接通过 `MetickAdapter -> VMMDLL/MemProcFS -> FPGA DMA` 访问 `NarakaBladepoint.exe`，AI / 脚本以 HTTP `POST /mcp` 的 JSON-RPC 2.0 方式持续读写，无需每次启动 CLI 再解析日志。完整细节见 `docs/mcp-server-usage.md`，下面是可直接照做的速查。

### 定位与前提

- 服务是一个常驻 Windows x64 进程，运行在 DMA 副机；`NarakaBladepoint.exe` 在另一台主机，由 FPGA/VMMDLL 远端采样。副机上 `Get-Process`/`tasklist` 看不到目标是正常现象，是否找到目标只以 `unity_session_connect` 的返回为准。
- 输出目录必须同时包含：
  ```text
  UnityExplorerMcpServer.exe
  vmm.dll
  leechcore.dll
  FTD3XX.dll
  ```
  当前已就位：`App\x64\Release\`。

### 1. 启动服务（HTTP，常驻）

```powershell
cd J:\Code\C++\dma\er_new\UnityExplorer-main\App\x64\Release
.\UnityExplorerMcpServer.exe `
  --transport http `
  --bind 127.0.0.1 `
  --port 19002 `
  --token "replace-with-a-long-random-token"
```

- endpoint：`http://127.0.0.1:19002/mcp`（默认端口为 `18765`）。
- `--token` 也可用环境变量 `UNITY_EXPLORER_MCP_TOKEN` 提供；两者都不给时，服务生成一次性 token 写到 stderr。
- 默认只监听 loopback（`127.0.0.1`），不要改 `--bind` 到 LAN。
- 希望 HTTP 监听前自动连接目标，加 `--target NarakaBladepoint.exe --auto-connect`（`--auto-connect` 不能单独使用）。不勾选时服务启动后处于 `idle`，由 MCP client 调 `unity_session_connect` 建立 session。

### 2. 请求约定

每个 HTTP 请求带：

```text
Authorization: Bearer <token>
Accept: application/json
Content-Type: application/json
```

只接受 `POST /mcp`，请求体是 JSON-RPC 2.0。PowerShell 示例：

```powershell
$token = "replace-with-a-long-random-token"
$headers = @{
  Authorization = "Bearer $token"
  Accept = "application/json"
  "Content-Type" = "application/json"
}
$body = '{"jsonrpc":"2.0","id":1,"method":"server/discover","params":{}}'
Invoke-RestMethod -Uri http://127.0.0.1:19002/mcp -Method Post -Headers $headers -Body $body
```

`tools/call` 的业务失败仍是正常 JSON-RPC result，但 `isError=true` 且带 `error.code`；读取 `result.structuredContent`（或 `result.content[0].text`）得到机器可读数据。

### 3. 典型调用顺序

先发现工具 / 看状态：

```json
{"jsonrpc":"2.0","id":1,"method":"server/discover","params":{}}
{"jsonrpc":"2.0","id":2,"method":"tools/list","params":{}}
{"jsonrpc":"2.0","id":3,"method":"tools/call","params":{"name":"unity_session_status","arguments":{}}}
```

默认只读工具：

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

连接一次并复用 DMA session（保存返回的 `session.generation`）：

```json
{"jsonrpc":"2.0","id":4,"method":"tools/call","params":{
  "name":"unity_session_connect",
  "arguments":{"targetProcess":"NarakaBladepoint.exe","mode":"dma","timeoutMs":30000}
}}
```

读模块 / 读内存 / 读指针 / 走指针链：

```json
{"jsonrpc":"2.0","id":5,"method":"tools/call","params":{"name":"unity_modules_list","arguments":{"expectedGeneration":1}}}
{"jsonrpc":"2.0","id":6,"method":"tools/call","params":{"name":"unity_memory_read","arguments":{"expectedGeneration":1,"address":"0x7FFE08570000","length":64,"nocache":true}}}
{"jsonrpc":"2.0","id":7,"method":"tools/call","params":{"name":"unity_memory_read_ptr","arguments":{"expectedGeneration":1,"address":"0x7FFE08570000"}}}
{"jsonrpc":"2.0","id":8,"method":"tools/call","params":{"name":"unity_pointer_chain_resolve","arguments":{"base":"0x7FFE08570000+0x376ABF8","offsets":[0xB8,0x8,0x70,0x90,0x0]}}}
```

- `unity_memory_read_ptr`：读 8 字节按小端解释为指针，返回 `valueHex/valueDec/isCanonical/isUserPointer`，省去手工字节反转。
- `unity_memory_read_string`：读 NUL 结尾字符串，`encoding=ascii`（默认）或 `utf16`，`maxBytes` 上限 4096。
- `unity_pointer_chain_resolve`：一次 job 内逐级解引用链，`base` 起每级读 8 字节指针、地址 = 当前值 + 下一偏移；返回每一跳和 `finalValueHex/finalValueDec`，命中空指针返回 `null_pointer`，失败返回 `read_failed`。

断开 / 重连：

```json
{"jsonrpc":"2.0","id":9,"method":"tools/call","params":{"name":"unity_session_disconnect","arguments":{"expectedGeneration":1}}}
```

每次 disconnect 或 replacement connect 都会推进 `generation`，带旧 `expectedGeneration` 的请求返回 `stale_generation`。目标重启后要重新 `connect` 并用新 generation。

### 4. 写入策略（默认只读）

默认构建与默认启动均为只读，`tools/list` 不出现 `unity_memory_write`。要真正启用写入，服务启动参数和每笔请求都必须同时满足：

```powershell
.\UnityExplorerMcpServer.exe `
  --transport http --bind 127.0.0.1 --port 19002 `
  --token "replace-with-a-long-random-token" `
  --enable-writes `
  --write-range 0x000001F000000000:0x1000
```

- `--write-range BASE:LENGTH` 至少一个，是半开区间 `[BASE, BASE+LENGTH)`；没有 `--enable-writes` 时 `--write-range` 会被参数校验拒绝。
- 每笔写入还要带 `expectedGeneration`、`address`、偶数长度 hex `data` 和同长度 `expectedBefore`；`data` 最多 4096 bytes 且不能跨 4 KiB page。
- 服务先 compare `expectedBefore`，写入后强制 readback；compare 失败 / 越界 / 旧 generation / readback 不一致都会报错。

写入示例（`data`/`expectedBefore` 各 4 字节）：

```json
{"jsonrpc":"2.0","id":10,"method":"tools/call","params":{
  "name":"unity_memory_write",
  "arguments":{
    "expectedGeneration":1,
    "address":"0x000001F000000100",
    "data":"01000000",
    "expectedBefore":"00000000",
    "verifyReadback":true
  }
}}
```

没有明确的 typed action 和经过验证的 allowlist 时，不建议启用 raw write。

### 5. 生命周期与并发

- 一次成功 `connect` 建立并复用同一个 VMM/CR3/DMA session；`disconnect`、replacement `connect`、进程重启后重连都会推进 `generation`。
- 会触碰 VMMDLL/`er2` 全局 context 的操作都进同一个 `SessionExecutor` 串行队列（上限 64）；HTTP 最多 32 个并发 worker。DMA 操作不会并行化。
- 收到 `Ctrl+C`/`SIGINT` 后：先停 HTTP 监听 → 等 worker → 停 executor → 断开 session → 释放 MetickAdapter/VMM/FPGA。

### 6. 常见错误码

| code | 处理 |
| --- | --- |
| `not_connected` | 先 `unity_session_connect` |
| `target_not_found` | 确认进程名；服务继续运行等待下次 connect |
| `dma_init_failed` | 查 VMMDLL/MemProcFS/FPGA DLL、管理员权限 |
| `module_ambiguous` | 显式指定 `unityPlayerName`/`gameAssemblyName` |
| `stale_generation` | 重新 status/connect 用最新 generation |
| `permission_denied` | 写工具或用启动策略不匹配 |
| `compare_failed` | `expectedBefore` 不一致，重新 read 后决定 |
| `readback_mismatch` | 写入结果不确定，重新读取确认 |
| `401/403/404/405/413/503` | token / Origin / 路径 / 方法 / 体积 / worker 占用 |

### 7. 当前能力边界

本版本已提供持久 session、模块枚举、通用 memory read 和受策略保护的可选 raw write。GOM、MSID、Transform、Camera、Metadata 及 Naraka 专用 Probe（`--naraka-ground-probe` 等）仍在 GUI/Headless 前端（`ExternalResolveConsole.exe`）里，尚未注册为 MCP tools。需要这些能力时，先走 Headless CLI，后续再复用 `AnalysisSession/SessionExecutor` 接入 MCP，不要另建第二个 DMA owner。

> 与 `YJWJ_DMA_NEW` 的关系：本服务是**分析侧**，用于只读发现与窄范围写验证；得到验证的 RVA/字段偏移/指针链再迁入 `YJWJ_DMA_NEW\Naraka\Offset.h` + `SDK.cpp`/`Thread.cpp`（详见本仓库 `项目交接文档.md`）。



