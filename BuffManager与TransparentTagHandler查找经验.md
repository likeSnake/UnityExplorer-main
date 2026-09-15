# BuffManager 与 TransparentTagHandler 查找经验

## 1. 文档目的

本文记录在 `NarakaBladepoint.exe` 的 DMA 只读环境中，查找以下两个 IL2CPP TypeInfo RVA 的可复用流程：

```cpp
// 普通 GameAssembly.dll
Offset::m_BuffManager
Offset::m_TransparentTagHandler

// Super 变体
Offset::m_BuffManager2
Offset::m_TransparentTagHandler2
```

本文中的“值”指的是 **TypeInfo slot 的 RVA**，不是动态对象地址，也不是模块基址。实际地址关系是：

```text
运行时 TypeInfo slot VA = 当前模块基址 + TypeInfo RVA
TypeInfo slot VA 处存放 klass 指针
klass + 0x10 -> 类名字符串指针
klass + 0x18 -> 命名空间字符串指针
klass + 0xB8 -> static fields 指针
```

目标是让另一个 AI 或维护者可以在游戏版本更新后，使用当前 DMA 工具重新找到并验证这两个 RVA，而不是凭旧版本偏移猜测。

---

## 2. 涉及项目和工具路径

### 2.1 运行项目

主项目：

```text
J:\Code\C++\dma\er_new\YJWJ_DMA_NEW
```

生产偏移文件：

```text
J:\Code\C++\dma\er_new\YJWJ_DMA_NEW\Naraka\Offset.h
```

主项目的模块选择逻辑：

```text
J:\Code\C++\dma\er_new\YJWJ_DMA_NEW\Naraka\main.cpp
```

主项目运行时 DMA DLL：

```text
J:\Code\C++\dma\er_new\YJWJ_DMA_NEW\Release\vmm.dll
J:\Code\C++\dma\er_new\YJWJ_DMA_NEW\Release\leechcore.dll
J:\Code\C++\dma\er_new\YJWJ_DMA_NEW\Release\FTD3XX.dll
```

Probe 输出目录：

```text
J:\Code\C++\dma\er_new\YJWJ_DMA_NEW\Release\dmaLog
```

### 2.2 本文档所在的分析项目

```text
J:\Code\C++\dma\er_new\UnityExplorer-main
```

已安装的普通 DLL 查找 skill：

```text
C:\Users\JackLong\.codex\skills\dma-normal-offset-finder\SKILL.md
```

已安装的 Super DLL 查找 skill：

```text
C:\Users\JackLong\.codex\skills\dma-super-offset-finder\SKILL.md
```

普通 DLL Probe 源码：

```text
C:\Users\JackLong\.codex\skills\dma-normal-offset-finder\scripts\dma_offset_probe_20260617.cpp
```

Super DLL Probe 源码：

```text
C:\Users\JackLong\.codex\skills\dma-super-offset-finder\scripts\dma_super_offset_probe_20260617.cpp
```

Super DLL 的补充 TransparentTagHandler 快速 Probe：

```text
C:\Users\JackLong\.codex\skills\dma-super-offset-finder\scripts\dma_super_tth_fast_20260605.cpp
```

主项目已有的历史 Probe 和日志也在：

```text
J:\Code\C++\dma\er_new\YJWJ_DMA_NEW\Release\dmaLog
```

---

## 3. 第一原则：先确认当前 DLL 变体

不能先看 `Offset.h` 再假定 DLL。必须先通过 DMA 枚举目标进程模块。

### 3.1 目标进程

目标进程名称：

```text
NarakaBladepoint.exe
```

本机 `Get-Process` 找不到进程不代表 DMA 目标不存在。当前工具连接的是 DMA 目标机，判断必须使用 `VMMDLL` 的目标进程枚举结果。

### 3.2 模块枚举

使用 `VMMDLL` 的只读调用：

```cpp
VMMDLL_Initialize(...);
VMMDLL_ProcessGetInformationAll(...);
VMMDLL_Map_GetModuleU(...);
```

模块名需要区分：

```text
GameAssembly.dll              -> 普通 DLL
GameAssembly_Super.dll        -> Super DLL
GameAssembly_Super_IBT.dll    -> Super IBT DLL
```

同时经常能看到：

```text
UnityPlayer_LVB.dll
```

`UnityPlayer_LVB.dll` 是 UnityPlayer 变体，不能把它当成 GameAssembly 使用。

### 3.3 之前的成功枚举例子

一次实际 DMA 枚举结果为：

```text
PID=4932
name=NarakaBladepoint.exe
module=GameAssembly_Super.dll
base=0x7FFCE0440000
size=0x1A43E000
module=UnityPlayer_LVB.dll
base=0x7FFCFADF0000
```

这个结果的结论是：当前必须使用 `GameAssembly_Super.dll` 对应的 TypeInfo RVA 和 Super 偏移字段，不能使用普通 `GameAssembly.dll` 的值。

### 3.4 主项目中的选择优先级

当前 `J:\Code\C++\dma\er_new\YJWJ_DMA_NEW\Naraka\main.cpp` 的逻辑依次尝试：

```text
1. GameAssembly.dll
2. GameAssembly_Super.dll
3. GameAssembly_Super_IBT.dll
```

应同时打印实际命中的模块名和基址。若多个 `GameAssembly*` 同时存在，不能只看 `Offset::GameAssembly != 0`，必须确认最终使用的变体。

---

## 4. TypeInfo 验证的通用链路

### 4.1 读取一个候选 RVA

假设模块基址为 `moduleBase`，候选 RVA 为 `candidateRva`：

```text
slot = moduleBase + candidateRva
klass = ReadPtr(slot)
```

然后读取：

```text
namePtr      = ReadPtr(klass + 0x10)
namespacePtr = ReadPtr(klass + 0x18)
staticFields = ReadPtr(klass + 0xB8)
className    = ReadCString(namePtr)
namespace    = ReadCString(namespacePtr)
```

基础判定必须满足：

```text
klass 是有效用户态地址
namePtr 是有效用户态地址
className 是预期类名
staticFields 是有效地址，或者当前对象尚未初始化时明确记录为空
```

不能因为某个候选 RVA 能读出一个非零指针就认为找到了。旧 RVA 经常能读出一个“看似像地址”的错误值，但类名会是乱码、空字符串或其他类名。

### 4.2 地址范围检查

Probe 使用用户态地址范围检查，典型条件为：

```cpp
value >= 0x10000ull && value < 0x0000800000000000ull
```

所有从远程内存读取出的 klass、字符串指针、static fields、instance、容器指针都应先做有效性检查，再继续读下一层。

### 4.3 读取策略

Probe 使用：

```cpp
VMMDLL_MemReadEx(..., VMMDLL_FLAG_NOCACHE)
```

查找阶段优先使用 `NOCACHE`，避免旧页面缓存让候选结果与当前进程状态不一致。所有 readback 都应检查：

```text
VMMDLL_MemReadEx 返回值 == true
实际读取字节数 == 请求字节数
```

---

## 5. BuffManager 的查找和确认

### 5.1 结构链

`BuffManager` 的验证链为：

```text
GameAssembly + m_BuffManager RVA
    -> BuffManager klass
    -> klass + 0xB8 = staticFields
    -> staticFields + 0x0 = BuffManager._instance
    -> _instance + 0x0 = instance klass
    -> _instance + 0x10 = _recordBuffHandlersDict
    -> dict + 0x18 = entries
    -> dict + 0x20 = count
    -> entries + 0x18 = entries length
```

其中最重要的身份判定是：

```text
klass 名称 == "BuffManager"
instanceKlass == klass
```

字典链是对候选的额外语义确认，不应单独作为唯一条件，因为游戏大厅、登录界面或换局期间字典可能为空。

### 5.2 完整候选检查

对一个候选 `rva`，建议打印至少这些字段：

```text
rva
klass
className
namespace
staticFields
instance
instanceKlass
dict
entries
count
entriesLen
score 或 pass/fail
```

推荐通过条件：

```text
className == "BuffManager"
klass != 0
staticFields 为有效地址
instance 为有效地址
instanceKlass == klass
```

如果当前生命周期允许 `instance == 0`，只能把它标记为“类定位正确、实例尚未初始化”，不能把它当作完整运行时通过。

### 5.3 普通 DLL 的高效查找方法

普通 DLL 的成功经验是优先利用类方法中的 RIP-relative 引用，而不是立即扫描整个模块。

步骤如下：

1. 从当前 dump.cs 找出 `BuffManager` 方法的 RVA，例如：

```text
BuffManager.Instance
BuffManager.Init
BuffManager.GetBuffHandler
BuffManager.FrameBeginTick
BuffManager..cctor
```

2. DMA 读取这些方法的机器码。
3. 扫描 x64 RIP-relative 指令，例如 `48 8B/8D`、`4C 8B/8D` 以及相关 RIP-relative store。
4. 根据指令末尾的 signed displacement 计算目标地址。
5. 将目标地址减去当前 GameAssembly 基址，得到候选 TypeInfo RVA。
6. 对每个候选执行第 4 节和本节的 `BuffManager` 链验证。
7. 多个方法引用同一个 TypeInfo slot 时提高可信度，最终选命中次数和结构验证均通过的 RVA。

Probe 中的计算本质是：

```text
targetVA = nextInstructionVA + signExtendedDisp32
candidateRVA = targetVA - moduleBase
```

不能把机器码中的绝对立即数直接当 RVA；必须按对应指令的 RIP-relative 位移计算。

### 5.4 普通 DLL 的历史成功结果

在 `dump2026.6.17.cs` 对应的一次验证中，成功找到：

```cpp
inline uint64_t m_BuffManager = 0xE916E88;
```

当时旧的：

```cpp
0xE647780
```

无法通过 `BuffManager` 的 class/instance 验证，因此被判定为 stale，而不是继续沿用。

注意：这个结果只对应当时的游戏版本和普通 DLL。游戏更新后必须重新验证。

---

## 6. TransparentTagHandler 的查找和确认

### 6.1 通用 TypeInfo/static 链

`TransparentTagHandler` 的第一阶段验证链为：

```text
GameAssembly + m_TransparentTagHandler RVA
    -> TransparentTagHandler klass
    -> klass + 0xB8 = staticFields
    -> staticFields + 0x0 = 静态字段区域的第一个值
    -> staticFields + 0x8 = TransparentTagHandler.s_instance
    -> s_instance + 0x0 = instance klass
```

必须首先确认：

```text
className == "TransparentTagHandler"
instanceKlass == klass
```

历史版本中 `staticFields + 0x0` 常见为 `0x41100000` 对应的 float 位模式，但这个值只能作为辅助证据，不能替代类名和 instanceKlass 判定。

### 6.2 `refCharacterMgr` 字段不能盲猜

不同版本中 `TransparentTagHandler` 实例字段布局可能变化。历史 Probe 使用过：

```text
s_instance + 0xA8 -> refCharacterMgr
```

但后来 dump 验证发现某个版本的 `+0xA8` 是 `alphaParam`，不是 CharacterManager 指针；该版本正确字段位于：

```text
s_instance + 0xD8 -> refCharacterMgr
```

所以正确做法是：

1. 先在当前 dump.cs 的 `TransparentTagHandler` 类定义中确认字段名和 offset；
2. 再按当前版本的字段 offset 读取 `refCharacterMgr`；
3. 若用于获得 CharacterManager，进一步检查其 klass、`+0x18` 本地 ActorModel 和角色容器链；
4. 不要因为旧 Probe 写的是 `+0xA8` 就直接写死到生产代码。

这也是之前 CharacterManager 读取失效的根因：把 `TransparentTagHandler + 0xA8` 错当成指针，覆盖了一个本来正确的 `CharacterManagerPtr`。

### 6.3 普通 DLL 的高效查找方法

从 dump.cs 找出以下方法的 RVA：

```text
TransparentTagHandler.Start
TransparentTagHandler.OnDestroy
TransparentTagHandler.OnMainPlayerLoadFinished
TransparentTagHandler.OnMainCameraCreated
TransparentTagHandler.UpdatePlayerTransparency
TransparentTagHandler..cctor
```

对方法机器码做与 `BuffManager` 相同的 RIP-relative 引用扫描：

```text
方法 RVA -> RIP-relative target -> candidate TypeInfo RVA
candidate -> klass -> class name
candidate -> staticFields + 0x8 -> s_instance
candidate -> instanceKlass == klass
```

多个方法命中同一个 `TransparentTagHandler` TypeInfo slot 时，优先级更高。

### 6.4 普通 DLL 的历史成功结果

一次成功验证得到：

```cpp
inline uint64_t m_TransparentTagHandler = 0xE76AD58;
```

当时旧值：

```cpp
0xE4AC7A8
```

无法通过类名和实例链验证，属于旧版本偏移。

---

## 7. Super DLL 的查找方法

Super DLL 的 TypeInfo 表更大，直接扫描整个模块会慢、DMA 请求多、误报也多。成功经验是：

```text
先确认模块变体
再验证历史候选
利用 dump/历史 all_bases 缩小类表邻域
最后做窄范围 TypeInfo name scan
```

### 7.1 当前 Super 模块选择

只接受：

```text
GameAssembly_Super.dll
GameAssembly_Super_IBT.dll
```

不能把普通 DLL 的候选直接套到 Super DLL。即使两个 DLL 的结构名称相同，TypeInfo RVA 也属于各自模块的地址空间和构建版本。

### 7.2 窄范围 TypeInfo name scan

Super Probe 的核心方法是：

1. 选择一个由 dump 或旧版 class table 确定的窄 RVA 区间；
2. 分块 DMA 读取该区间，典型 chunk 为 `0x40000`；
3. 按 8 字节对齐读取可能的 klass 指针；
4. 读取 `klass + 0x10` 的名字指针和字符串；
5. 只保留名字为 `BuffManager` 或 `TransparentTagHandler` 的 slot；
6. 对命中的 slot 重新执行完整 static/instance/容器验证。

代码中的关键逻辑相当于：

```text
for rva in [scanStart, scanEnd) step 8:
    klass = ReadPtr(moduleBase + rva)
    if !IsUserPointer(klass): continue
    namePtr = ReadPtr(klass + 0x10)
    name = ReadCString(namePtr)
    if name == "BuffManager" or name == "TransparentTagHandler":
        validate candidate
```

窄扫描的起止地址不能永久写死。它们只适用于相应版本附近的 class table，更新游戏后要根据新的 dump 或 `all_bases.txt` 重新定位。

### 7.3 Super 的历史候选为什么会误导

一次历史检查中：

```text
旧 m_TransparentTagHandler2 = 0x3704CB8
```

在新版本实际解析成了：

```text
<>c__DisplayClass0_0`2
```

而不是 `TransparentTagHandler`。这证明：

```text
“能够读到 klass” != “找到了目标类”
```

同一次版本变化中，旧的：

```text
0x36E5338
```

也没有解析为 `BuffManager`。

### 7.4 Super 的历史成功结果

在 `GameAssembly_Super.dll` 的一次 2026-06-17 DMA 验证中，窄范围扫描和结构检查得到：

```cpp
inline uint64_t m_BuffManager2 = 0x37669B0;
inline uint64_t m_TransparentTagHandler2 = 0x3786798;
```

对应的旧候选：

```cpp
0x36E5338  // 不是 BuffManager
0x3704CB8  // 解析成 <>c__DisplayClass0_0`2，不是 TransparentTagHandler
```

此结果是一次版本快照，不是永久偏移。它的价值在于说明查找方法：先用类名 scan 找到候选，再用 static/instance/容器链确认。

### 7.5 Super 找不到 TTH 时的补充方法

如果窄范围 TypeInfo name scan 没有找到 `TransparentTagHandler`：

1. 先确认 `scanStart/scanEnd` 是否来自当前版本，而不是沿用旧版本区间；
2. 检查当前 dump.cs 中 `TransparentTagHandler` 的方法和字段定义；
3. 使用：

```text
C:\Users\JackLong\.codex\skills\dma-super-offset-finder\scripts\dma_super_tth_fast_20260605.cpp
```

该 Probe 用 `TransparentTagHandler` 相关特征查找并验证：

```text
klass 名称
staticFields
staticFields + 0x8 -> s_instance
s_instance klass
```

4. 如果仍然失败，再扩大扫描区间，但必须保留限速和输出计数，避免无边界全模块读取。

---

## 8. Probe 的构建与运行

以下命令是只读 Probe 的标准构建方式。它们不修改目标进程内存。

### 8.1 Visual Studio 编译环境

VS 开发者环境脚本：

```text
J:\vs2026\VC\Auxiliary\Build\vcvars64.bat
```

### 8.2 普通 DLL Probe

```powershell
cmd /c "call J:\vs2026\VC\Auxiliary\Build\vcvars64.bat >nul && cd /d J:\Code\C++\dma\er_new\YJWJ_DMA_NEW && cl /nologo /std:c++17 /EHsc /O2 /MT /I. /I Naraka\libs C:\Users\JackLong\.codex\skills\dma-normal-offset-finder\scripts\dma_offset_probe_20260617.cpp /link /LIBPATH:Naraka\libs vmm.lib leechcore.lib FTD3XX.lib /OUT:Release\dmaLog\dma_offset_probe_20260617.exe"
```

从包含 DMA runtime DLL 的目录启动：

```powershell
Set-Location "J:\Code\C++\dma\er_new\YJWJ_DMA_NEW\Release"
& ".\dmaLog\dma_offset_probe_20260617.exe"
```

### 8.3 Super DLL Probe

```powershell
cmd /c "call J:\vs2026\VC\Auxiliary\Build\vcvars64.bat >nul && cd /d J:\Code\C++\dma\er_new\YJWJ_DMA_NEW && cl /nologo /std:c++17 /EHsc /O2 /MT /I. /I Naraka\libs C:\Users\JackLong\.codex\skills\dma-super-offset-finder\scripts\dma_super_offset_probe_20260617.cpp /link /LIBPATH:Naraka\libs vmm.lib leechcore.lib FTD3XX.lib /OUT:Release\dmaLog\dma_super_offset_probe_20260617.exe"
```

运行：

```powershell
Set-Location "J:\Code\C++\dma\er_new\YJWJ_DMA_NEW\Release"
& ".\dmaLog\dma_super_offset_probe_20260617.exe"
```

实际维护时，如果已经有当天编译好的 Probe，直接运行即可；不要因为历史源码文件名带日期就认为它一定匹配当前游戏版本。

---

## 9. 如何判断 Probe 输出是否成功

### 9.1 模块层

必须能看到：

```text
loaded GameAssembly.dll ...
```

或：

```text
loaded GameAssembly_Super.dll ...
loaded GameAssembly_Super_IBT.dll ...
```

如果这里失败，后面的 RVA 结果全部无效。

### 9.2 类身份层

BuffManager 必须看到：

```text
name="BuffManager"
```

TransparentTagHandler 必须看到：

```text
name="TransparentTagHandler"
```

出现乱码、空字符串或其他类名时，即使 `klass` 非零，也必须判定失败。

### 9.3 实例层

推荐看到：

```text
instance != 0
instanceKlass == klass
```

BuffManager 还应尽量看到有效的：

```text
dict
entries
count
entriesLen
```

TTH 还应根据当前 dump 的字段定义验证：

```text
staticFields + 0x8 -> s_instance
s_instance + current refCharacterMgr offset -> 有效 CharacterManager
```

如果处于登录界面或换局阶段，`s_instance`、`BuffManager._instance` 或 `refCharacterMgr` 暂时为零是可能的。此时应区分：

```text
TypeInfo 定位失败
对象尚未初始化
对象已销毁/换局，需要刷新链
```

不能把生命周期为空误判成 RVA 错误，也不能把旧缓存指针继续当作新一局对象。

---

## 10. 从结果同步到 YJWJ_DMA_NEW

### 10.1 写入位置

生产偏移只应写入：

```text
J:\Code\C++\dma\er_new\YJWJ_DMA_NEW\Naraka\Offset.h
```

普通 DLL 使用：

```cpp
inline uint64_t m_BuffManager = 0x...;
inline uint64_t m_TransparentTagHandler = 0x...;
```

Super/Super IBT 使用：

```cpp
inline uint64_t m_BuffManager2 = 0x...;
inline uint64_t m_TransparentTagHandler2 = 0x...;
```

不要把普通值覆盖到 `*2` 字段，也不要把 Super 值覆盖到普通字段。

### 10.2 变体切换

`Offset.h` 使用 `Offset::ApplyGameAssemblyOffsets()` 根据当前 DLL 变体选择字段。更新时要确认：

```text
当前模块名
CurrentGameAssemblyVariant
GameAssembly base
最终使用的 Offset 字段
```

主项目应在启动日志中明确打印最终模块名和基址，避免“模块是 Super、偏移却使用普通字段”的混用。

### 10.3 迁移前的最小证据记录

每次更新至少保存以下内容：

```text
验证日期
目标进程名和 DMA PID
模块名
模块基址和大小
dump.cs 绝对路径
m_BuffManager RVA
m_TransparentTagHandler RVA
klass
className
staticFields
instance/s_instance
instanceKlass
dict 或 refCharacterMgr 的验证结果
```

模块基址每次启动可能变化，不能写进 `Offset.h` 作为固定值；`Offset.h` 只保存 RVA。

---

## 11. 常见失败和排查顺序

### 11.1 把普通 DLL 和 Super DLL 混用

症状：

```text
klass 可读，但 className 错误
staticFields 为 0
instance 为 0
```

处理：重新执行模块枚举，确认模块名，再选择对应 Probe 和对应字段。

### 11.2 使用旧 TypeInfo RVA

症状：

```text
className 乱码
className 是其他类
instanceKlass != klass
```

处理：标记 stale，不能只因为地址非零就保留；重新从当前 dump 方法引用或当前 TypeInfo 表 scan。

### 11.3 只验证 className，不验证 instanceKlass

症状：在类型表中碰巧找到同名或相似对象，但静态实例链不正确。

处理：至少确认：

```text
staticFields
instance
instanceKlass == klass
```

### 11.4 登录界面对象为空

症状：TypeInfo 和 className 正确，但：

```text
BuffManager._instance == 0
TTH.s_instance == 0
refCharacterMgr == 0
```

处理：进入实际对局后再验证，或把结果标记为“类已定位、实例未初始化”。测试换局和回大厅时要刷新对象链。

### 11.5 TTH 的字段偏移沿用旧版本

症状：`s_instance` 正确，但从 `+0xA8` 读到浮点、普通数值或无意义地址。

处理：回到当前 dump.cs，查找 `refCharacterMgr` 的真实字段 offset。之前已遇到 `+0xA8` 实际为 `alphaParam`、正确链在 `+0xD8` 的版本。

### 11.6 全模块扫描耗时过长

处理顺序：

```text
先验证已知候选
再读取 dump 方法引用
再缩小 Super TypeInfo 邻域
最后才扩大范围
```

每一轮都输出 scan 范围、chunk 数、命中数量和耗时，避免误以为程序卡死。

### 11.7 DMA runtime DLL 找不到

运行 Probe 的当前目录应能找到：

```text
vmm.dll
leechcore.dll
FTD3XX.dll
```

推荐从：

```text
J:\Code\C++\dma\er_new\YJWJ_DMA_NEW\Release
```

启动，而不是从 skill 脚本目录启动。

---

## 12. 最终执行清单

每次游戏更新后按以下顺序执行：

```text
[ ] 目标进程 NarakaBladepoint.exe 已由 DMA 枚举到
[ ] 已枚举并记录 GameAssembly*.dll 的精确名称、基址、大小
[ ] 已选择 normal 或 Super/SuperIBT 对应 Probe
[ ] 已使用当前版本 dump.cs，而不是只依赖历史值
[ ] 已确认 candidate slot -> klass
[ ] 已确认 klass + 0x10 的 className
[ ] 已确认 klass + 0xB8 的 staticFields
[ ] 已确认 BuffManager._instance 或 TTH.s_instance
[ ] 已确认 instanceKlass == klass
[ ] 已按当前 dump 确认 TTH 的 refCharacterMgr offset
[ ] 已在实际对局或明确记录的生命周期状态下复测
[ ] 已分别更新 Offset.h 的普通字段或 Super 字段
[ ] 已记录验证日志和 dump 路径
[ ] 已测试大厅、进入对局、换局、返回大厅
```

只有完成这条清单，才能把结果认为是可迁移到 `YJWJ_DMA_NEW` 的生产偏移。单纯出现 `FOUND m_BuffManager=0x...` 或 `FOUND m_TransparentTagHandler=0x...`，但没有 className、staticFields、instanceKlass 和模块变体证据时，不应直接更新项目。

---

## 13. 相关文件索引

```text
J:\Code\C++\dma\er_new\YJWJ_DMA_NEW\Naraka\Offset.h
J:\Code\C++\dma\er_new\YJWJ_DMA_NEW\Naraka\main.cpp
J:\Code\C++\dma\er_new\YJWJ_DMA_NEW\Release\dmaLog
C:\Users\JackLong\.codex\skills\dma-normal-offset-finder\SKILL.md
C:\Users\JackLong\.codex\skills\dma-normal-offset-finder\scripts\dma_offset_probe_20260617.cpp
C:\Users\JackLong\.codex\skills\dma-super-offset-finder\SKILL.md
C:\Users\JackLong\.codex\skills\dma-super-offset-finder\scripts\dma_super_offset_probe_20260617.cpp
C:\Users\JackLong\.codex\skills\dma-super-offset-finder\scripts\dma_super_tth_fast_20260605.cpp
```
