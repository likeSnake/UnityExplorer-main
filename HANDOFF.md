# 交接文档 — UnityExplorer DMA 项目

> **给接手 AI 的完整交接**：本项目当前状态、已完成工作、关键文件位置、
> 编译方法、验证步骤、待办事项与重要技术结论。
> 另一台电脑拉取本仓库后，**先读本文档 + `dump_workspace/docs/项目进度与结论记录.md`**。

---

## 0. 交接摘要（30 秒版）

- **目标**：通过注入式 dump（CE 注入自研 DLL，免 FPGA）dump 永劫无间（Unity + IL2CPP）的**完整**游戏结构（dump.cs：类/字段名+偏移/方法名+RVA/属性），**脱离对闭源注入器 Unity-Offset.dll 的依赖**。
- **本机定位（关键）**：**本机是游戏主机（target），不是 DMA 分析机**。不需要 FPGA/DMA ——
  CE 注入（`F:\DrunkenDreamDriverCE v1.4.0\`）是唯一注入通道。DMA 相关命令
  （`--self-dump-cs`/`--dma-inject`/`--dump-image`）是原分析机路径，本机不执行。
- **已完成**：
  1. ✅ 自研注入式 dump DLL（`Il2CppRuntimeDumper.dll`，机制与 Unity-Offset.dll 完全等价，已在本机编译，bin\Release\）
  2. ✅ 逆向确认 Unity-Offset.dll = 开源项目 [shalzuth/Il2CppRuntimeDumper]，源码已克隆到本地
  3. ✅ 环境完善：MSVC v143 工具链 / MemProcFS 运行时 / git 代理 / cfg 路径修正
  4. ✅ 参考 dump.cs 基准到位：`F:\gua\dump\dump2026.09.03.cs`（11525 类，Unity-Offset.dll 历史输出）
- **待办**：游戏下载完成后，**用 CE 注入自研 DLL 验证 dump.cs 完整度**（任务 1，唯一主路径）
- **环境**：Windows x64 + MSVC v143 + CE v1.4.0（游戏主机）。
  DMA 历史结论（镜像导出/self-dump/dma-inject）保留为原分析机参考，本机不再依赖。

---

## 0.5 ★ 本会话重大技术更新（2026-09-08，决定性，接手必读）

> 以下内容推翻旧 §5「任务1：CE 注入自研 DLL 验证 GetProcAddress 反射 dump」的前提。
> 详细记录见 `dump_workspace/docs/项目进度与结论记录.md` §13–§16。

### 根因定性：本构建是"双重重加固"IL2CPP（非通道问题、非时机问题）

通过多轮 DiagProbe（注入游戏验证）与磁盘字节扫描，**确认**：

1. **类型反射导出被裁剪**。用分类探针（普通体 GameAssembly.dll）GetProcAddress 实测：
   - ✅ 运行时内核导出正常：`il2cpp_get_corlib` / `il2cpp_image_get_class` /
     `image_get_class_count` / `image_get_name` / `image_get_assembly` / `init` /
     `shutdown` / `thread_attach` 等全部解析成功。
   - ❌ 类型反射导出全部为 NULL：`class_for_each` / `class_get_name` / `field_get_name` /
     `method_get_name` / `domain_get` / `domain_get_assemblies` / `class_get_fields` /
     `class_get_methods`，以及全部 `mono_*`。
   - 磁盘全量明文导出名枚举：仅 **123 个 il2cpp_***（0 个 mono_*），全部是运行时内核类
     （image/gc/thread/monitor/profiler/liveness 等），**没有任何 class/field/method 反射 getter**。
2. **`global-metadata.dat` 双份均加密**：普通体（90MB）与 Super 体（79MB）头 4 字节
   都不是魔数 `0xFAB11BAF`（普通=0x57CA7923），高熵。→ 离线 Il2CppDumper 直接读 metadata 不可行。

**结论**：本构建对 il2cpp 做了"导出裁剪 + metadata 加密"加固。**任何基于 GetProcAddress
解析 class/field/method 反射 getter 的 dump 器（含自研 DLL 的 validator、乃至旧 §5 任务 1
的 CE 注入方案）在本构建上都必然 `[FAIL]`** —— validator 要求 `class_get_fields`/
`class_get_methods` 非空，这些导出本就不存在。通道（GetProcAddress/注入）本身已被证明是好的。

> 这也解释了为什么 299KB Unity-Offset.dll 能成功 dump 11525 类：它走的是**运行时内存中
> 已解密的 Il2Cpp 结构直接偏移游走**（只用 image_get_class/get_corlib 引导），不依赖被裁剪
> 的反射导出，也不依赖（进程外的）加密 metadata。

### 已确认可行的 in-memory 游走基础（StructWalkProbe 实证）

- `image_get_class(image,i)` + `image_get_class_count(image)` **能完整列出 corlib 的
  1825 个类**（Unity 2019.4.41f2）。
- `Il2CppClass` 内存结构首字段偏移已实证：`image=+0x00`、`name=+0x10`（真实类名：
  `<Module>`/`Runtime`/`Locale`/`SR` 等）、`namespace=+0x18`（空或 `"Mono"`）。
- 字段/方法数组偏移尚未锁定（本构建布局疑似被重排，不在常规定位；+0x190 处是字符串指针
  而非数组）。需在更靠后区段继续解码。

### 下一条技术路线（已定：内存结构游走 dumper）
把自研 dumper 从"GetProcAddress 反射 API"改写为 **"内存已解密 Il2Cpp 结构直接偏移游走"**：
用 `get_corlib`/`image_get_class`/`image_get_class_count` 引导枚举镜像→类，再按
Unity 2019.4 的 `Il2CppClass`/`Il2CppImage`/`Il2CppMethodInfo`/`Il2CppFieldInfo` 结构偏移
直接读字段/方法/名字串。**首要待完成子步骤**：在注入探针中定位 fields/methods 数组的
精确偏移（参考 divinedragonfanclub/engage class.rs 布局 + +0x0F0..+0x120 计数字段区）。

**✳ 重大突破（2026-09-08）**：已用自研 `MemoryStructDumper.dll` 成功 dump 出
**Assembly-CSharp.dll 全部 39102 类**（`dump_workspace\selfdump\Assembly-CSharp_classes.cs`，
1.39MB，游戏存活、可复现）。只用 4 个已确认导出函数（get_corlib / image_get_class /
image_get_class_count / image_get_name），**完全自研，脱离反射 getter**。
已实证结构偏移：`Il2CppClass.image=+0x00, name=+0x10, namespace=+0x18, fields=+0x80`；
`FieldInfo.name=+0x00, offset(int32)=+0x18, 步长 0x20`。
**NEAC 存活阈值**：CreateRemoteThread 通道下，快速类名全量 dump 能跑完；新建文件/每类
逐字段重读会超窗被踢。**字段+方法全量落地很可能需走 CE 注入通道**（299KB 用的正是 CE）。

**辅助工具已建**：`App/Il2CppRuntimeDumper/StructWalkProbe.cpp` + `StructWalkProbe.vcxproj`
（编译 → `bin\Release\StructWalkProbe.dll`，注入后输出 `C:\walkprobe.log`）；
`App/Il2CppRuntimeDumper/MemoryStructDumper.cpp` + `MemoryStructDumper.vcxproj`
（编译 → `bin\Release\MemoryStructDumper.dll`，注入后输出 `C:\selfdump\dump.cs`）。

---

## 1. 项目背景与目标

| 项 | 值 |
| --- | --- |
| 目标游戏 | 永劫无间 Naraka Bladepoint（Unity 2019.4 + IL2CPP）|
| 硬件 | FPGA DMA（Metick/PCILeech 体系，经 MemProcFS vmm.dll 访问）|
| 本仓库角色 | Unity 外部分析器（UnityExplorer-main / er2 库）+ 自研 dump 工具链 |
| 生产主项目（只读参考） | `J:\Code\C++\dma\er_new\YJWJ_DMA_NEW`（另一台机器可能无此路径，仅参考）|
| 参考 dump 产物 | `J:\Code\C++\dma\er_new\dump\`（dump.cs / GameAssembly 镜像 / Unity-Offset.dll）|
| 最终目标 | 自主掌握完整 dump 能力，不依赖 Unity-Offset.dll |

**游戏变体**：
- 普通 `GameAssembly.dll`：裸导出 il2cpp_* API → 注入式 dump 可行
- Super `GameAssembly_Super.dll`：VMProtect 保护（.vmp0/.vmp1），il2cpp 导出被移除 → 注入式 dump 不可行（Unity-Offset.dll 同样受限）
- Unity 引擎：`UnityPlayer_LVB.dll`

---

## 2. 工作区布局（本仓库内）

```
UnityExplorer-main/
├── include/er2/unity2/headless/
│   ├── self_dump_cs.hpp        # 自研运行时结构 dump（508 类）
│   ├── dma_inject.hpp          # DMA 注入（IAT/inline hook + LoadLibraryA）
│   ├── probe_cli.hpp           # CLI 参数（已加 --self-dump-cs / --dma-inject 等）
│   └── runtime_dump_cs.hpp     # 原有 runtime dump（稀疏版）
├── App/Il2CppRuntimeDumper/    # 自研注入式 dump DLL 工程
│   ├── il2cpp_api.hpp          # il2cpp API 解析层（GetProcAddress ~80 个导出）
│   ├── runtime_dumper.cpp      # 遍历器 + dump.cs 生成（字段偏移/方法RVA/属性/参数）
│   ├── dllmain.cpp             # DllMain 注入入口 + VEH 崩溃保护
│   ├── injector.cpp            # 最小注入器（CreateRemoteThread+LoadLibraryA）
│   ├── Il2CppRuntimeDumper.vcxproj
│   └── Injector.vcxproj
├── App/ExternalResolveConsole/ # headless 主程序（含全部 --xxx 命令）
├── bin/Release/                # 编译产物
│   ├── Il2CppRuntimeDumper.dll  # 自研注入 dump DLL（311KB）
│   ├── Injector.exe             # 注入器（152KB）
│   └── vmm.dll/leechcore.dll/FTD3XX.dll  # DMA 运行时（运行目录需要）
├── dump_workspace/             # 工作区（产物/日志/文档/源码克隆）
│   ├── run/                    # DMA 运行目录（exe + DMA DLL 自包含）
│   ├── output/                 # 产物（dump.cs/json/images）
│   ├── logs/                   # 日志
│   ├── inject_test/            # 注入测试目录（cfg + DLL）
│   ├── docs/
│   │   ├── 项目进度与结论记录.md   # ★ 主进度文档（务必先读）
│   │   ├── Unity-Offset逆向报告.md # Unity-Offset.dll 逆向结论
│   │   └── 注入式dump方案.md       # 自研注入器方案
│   └── Il2CppRuntimeDumper_src/  # Unity-Offset.dll 源头源码（本地克隆）
└── dump_workspace/run/ExternalResolveConsole.exe  # headless 可执行（最新）
```

---

## 3. 关键命令速查（在 dump_workspace/run 目录运行）

```powershell
# 0. 构建 headless 主程序
& "J:\vs2026\MSBuild\Current\Bin\MSBuild.exe" App\ExternalResolveConsole.sln /p:Configuration=Release /p:Platform=x64 /m
# 输出：App\x64\Release\ExternalResolveConsole.exe → 复制到 dump_workspace\run\

# 1. DMA 连接探测（拿 PID/变体/基址）
.\ExternalResolveConsole.exe --headless --target NarakaBladepoint.exe --mode dma --scan none --limit 0 --out probe.json --log probe.log

# 2. 运行时 dump（稀疏）
.\ExternalResolveConsole.exe --headless --target NarakaBladepoint.exe --mode dma --scan msid --limit 2048 --runtime-dump-cs rt.cs --out rt.json

# 3. 镜像导出（GameAssembly + UnityPlayer）
.\ExternalResolveConsole.exe --headless --target NarakaBladepoint.exe --mode dma --scan none --limit 0 --dump-image images_dir --out img.json

# 4. 自研运行时结构 dump（508 类 / 8049 字段 / 10639 方法）
.\ExternalResolveConsole.exe --headless --target NarakaBladepoint.exe --mode dma --scan none --limit 0 --self-dump-cs self.cs --self-dump-cs-log self.log --out self.json

# 5. DMA 注入（免句柄 LoadLibrary；test-only 先验证链路）
.\ExternalResolveConsole.exe --headless --target NarakaBladepoint.exe --mode dma --scan none --limit 0 --dma-inject <dll路径> --dma-inject-pid <pid> --dma-inject-hook kernel32.dll!Sleep --dma-inject-wait-ms 25000 --out inj.json --log inj.log
# 加 --dma-inject-test 跳过 LoadLibraryA（只验证执行链路）

# 6. 注入式 dump（CE 注入自研 DLL）
# 用 CE 注入 dump_workspace\inject_test\Il2CppRuntimeDumper.dll
# 输出目录由 DLL 同目录 Il2CppRuntimeDumper.cfg 指定（当前指向 inject_test\out）
```

---

## 4. 已完成工作详情

### 4.1 DMA 运行时 dump（--runtime-dump-*）
- MSID 对象枚举 → klass → 类名/字段/方法（稀疏，字段名依赖 metadata）
- 实测：普通变体 22 类/319 字段；Super 变体 104 类/670 字段/1687 方法

### 4.2 DMA 镜像导出（--dump-image）
- DMA 整段读模块镜像落盘（有效 PE32+）
- 实测：GameAssembly_Super 440MB / GameAssembly 429MB / UnityPlayer 77MB

### 4.3 自研运行时结构 dump（--self-dump-cs）★ 主要成果
- **原理**：MSID 枚举 live 对象 → 收集唯一 Il2CppClass 指针（去重）→
  逐类读字段（名+偏移+类型）/方法（名+RVA+返回类型）→ 生成 dump.cs
- **不依赖**：注入器、global-metadata.dat 魔数扫描
- **实测**：508 类 / 8049 字段 / 10639 方法（含 RVA），类名/字段/方法全部真实
- **关键修复**：`IsCanonicalPtr` 曾误加 16 字节对齐要求导致指针全拒，
  改为仅范围检查后数据完整（`self_dump_cs.hpp` 内）

### 4.4 自研注入式 dump DLL（Il2CppRuntimeDumper）
- **机制与 Unity-Offset.dll 完全等价**（源码确认）：
  - 注入 → DllMain 触发工作线程 → GetProcAddress 解析 il2cpp API →
    assembly→image→class→field/method/property → dump.cs
- **输出**：字段偏移 `// 0x{offset}`、方法 RVA/VA、属性 get/set、TypeDefIndex、参数类型+名字
- **编译**：`msbuild App\Il2CppRuntimeDumper\Il2CppRuntimeDumper.vcxproj`
- **cfg 配置**：DLL 同目录 `Il2CppRuntimeDumper.cfg` 第一行 = 输出目录
- **健壮性**：VEH 崩溃保护（dump 异常只终止工作线程，不杀游戏）
- **尚未验证**：注入到真实游戏后的 dump.cs 完整度（游戏在下载，未实测）

### 4.5 Unity-Offset.dll 逆向（重要情报）
- **源头**：开源项目 [shalzuth/Il2CppRuntimeDumper](https://github.com/shalzuth/Il2CppRuntimeDumper)
  （DLL 内嵌源码路径证实），**源码已克隆**到 `dump_workspace/Il2CppRuntimeDumper_src/`
- **机制**（源码确认）：il2cpp API 遍历 + Mono.Cecil 重建 DummyDlls
- **局限**：只能 dump 普通变体（Super 的 il2cpp 导出被 VMP 移除）
- **对我们的意义**：验证自研实现机制 100% 对齐；可参考其输出格式

### 4.6 DMA 注入（--dma-inject）★ 原理已验证
- **原理**：DMA 写 shellcode + inline hook（IAT 或函数头）+ 游戏线程执行
  LoadLibraryA → 免句柄、免 CreateRemoteThread
- **已验证**：
  - DMA 写能力 ✅（FPGA 固件支持）
  - 代码洞/数据洞定位 ✅（手工解析 PE 节表，绕开被反作弊破坏的 API）
  - shellcode 执行 ✅（executed=true 多次实测）
  - LoadLibraryA 调用 ✅（返回过有效 HMODULE 0x18814）
  - hook 恢复 ✅
- **已知问题**：完整注入后游戏崩溃 —— 代码洞/数据洞选在 GameAssembly
  （VMP 保护区）节尾空洞，被游戏运行时覆盖 → 冲突崩溃。**工程上不稳定**，
  技术原理已证实。
- **改进方向**：代码洞改到主模块（无 VMP，但节表被 NEAC 隐藏需 DMA 直读 PE）；
  或 shellcode 内 VirtualAlloc 自申请。

---

## 5. 下一步任务（本机 = 游戏主机，按序执行）

### 任务 1（唯一主路径，游戏下载完成后立即执行）：CE 注入验证自研 DLL
> ⚠️ **2026-09-08 已证伪**：本构建类型反射导出被裁剪，GetProcAddress 反射 dump 永不成功
> （见 §0.5）。以下旧步骤只保留作历史参考，真正的下一步改为 §0.5 的"内存结构游走 dumper"。
>
**注入通道**：`F:\DrunkenDreamDriverCE v1.4.0\`（定制 CE，含 DBVM 驱动、
`NarakaBladepoint.CT`、`Unity-Offset (1).dll`）。可继续用 CE 注入通道加载自研/探针 DLL，
但 DLL 内部的解析逻辑必须换成结构游走，而非反射 getter。

**步骤**：
1. 游戏启动到**登录界面**（普通变体 GameAssembly.dll，Unity-Offset.dll 已验证此时机可 dump）
2. 确认 `dump_workspace\inject_test\Il2CppRuntimeDumper.cfg` 指向
   `F:\gua\UnityExplorer-main\dump_workspace\inject_test\out`（已改好，勿回退）
3. 用 **CE 注入** `bin\Release\Il2CppRuntimeDumper.dll`
   （CE 注入是已验证通道；`bin\Release\Injector.exe` 备用，反作弊可能拦截 CreateRemoteThread）
4. 等待 10-30 秒，检查 `dump_workspace\inject_test\out\` 下 `dump.cs` + `log.txt`
5. **对比完整度**：与基准 `F:\gua\dump\dump2026.09.03.cs` 对比类数/字段数/方法数。
   基准规模：**11525 类**（class 7486 + struct 923 + interface 233 + enum 2883）、165万行。
6. 若 dump.cs 异常/缺失：查看 log.txt（[FAIL]/[CRASH]/[OK]），
   检查 `dllmain.cpp` 的 `DumpWorkerBody` 与 `runtime_dumper.cpp` 的 `RunDump`

### 任务 2（原分析机遗留，本机不执行）
DMA 注入稳定性 / self-dump / 镜像导出 —— 属原 FPGA 分析机路径，本机无 DMA 硬件，跳过。

---

## 6. 环境与构建备忘

### 构建工具
- MSBuild：`J:\vs2026\MSBuild\Current\Bin\MSBuild.exe`（或 VS 自带）
- VS：VS2022 / VS2026 均可（PlatformToolset v143）
- 所有工程 Release|x64

### 已知编译坑（已修复，勿回退）
1. **deps/Memory/Mem.cpp**：Windows min/max 宏污染 + 中文标识符编码
   → 已加 `NOMINMAX`（framework.h）+ `/utf-8`（vcxproj）+ 条件表达式替换 std::min
2. **main.cpp 退出崩溃**（0xC0000005）：`g_ctx` 全局持有 MetickAdapter 导致
   析构期 VMMDLL_Close 崩溃 → `RunHeadless` 末尾显式 `ResetContext()+dma.reset()`
3. **self_dump_cs.hpp**：`IsCanonicalPtr` 勿加 16 字节对齐（会拒掉合法指针）
4. **Il2CppRuntimeDumper.vcxproj**：`/EHa` 已加（VEH 用，无需 __try）

### DMA 运行时依赖（exe 运行目录必须包含）
```
vmm.dll / leechcore.dll / FTD3XX.dll   # dump_workspace\run\ 已备齐
```

### 运行前置
- FPGA 硬件在线（Metick/PCILeech）
- 游戏进程运行中（DMA FixCr3 附加）
- 反作弊（NEAC）会隐藏进程（Toolhelp/Get-Process 看不到），但 DMA 可正常附加

---

## 7. 重要技术结论（勿忘记）

1. **metadata 魔数扫描（0xFAB11BAF）在普通和 Super 变体都失败** ——
   il2cpp 运行时 metadata 不以明文魔数文件存在内存；注入器靠 il2cpp API
   就地遍历（DMA 无法调 API，只能读结构）
2. **运行时 klass 结构是明文的**（DMA 可读）—— 这是 self-dump 的基础
3. **Unity-Offset.dll = shalzuth/Il2CppRuntimeDumper**（开源，源码在本地）
4. **游戏进程保护**：NEAC 隐藏进程/PEB/IAT/节表 → 依赖这些的 API 会失败，
   需用 DMA 直读 PE 手工解析
5. **DMA 注入原理可行**（executed=true + LoadLibraryA 成功），但 GameAssembly
   的 VMP 空洞不可靠 → 工程不稳定

---

## 8. 参考文档索引

| 文档 | 内容 | 位置 |
| --- | --- | --- |
| **项目进度与结论记录.md** | ★ 主进度（全部结论/卡点/步骤） | `dump_workspace/docs/` |
| **Unity-Offset逆向报告.md** | Unity-Offset.dll 逆向 + 对齐对照表 | `dump_workspace/docs/` |
| **注入式dump方案.md** | 自研注入器方案 | `dump_workspace/docs/` |
| **Il2CppRuntimeDumper_src/** | Unity-Offset.dll 源头源码（本地克隆） | `dump_workspace/` |
| README.md | er2 库原始说明 | 根目录 |
| 项目交接文档.md | 原项目（分析器↔主项目）交接 | 根目录 |

---

## 9. 与外部机器的路径差异提醒 + 本机（F: 盘）环境状态

### 9.1 本机环境现状（2026 交接时实录）

**当前仓库实际路径**：`F:\gua\UnityExplorer-main`（原为 `J:\Code\C++\dma\er_new\UnityExplorer-main`，路径已变）。

**本机缺失项（相对原始环境）**：
- ❌ **MSVC 工具链**：本机原无 VS/MSBuild（vswhere 返回空）。已通过 winget 安装
  `Microsoft.VisualStudio.2022.BuildTools` + `Microsoft.VisualStudio.Workload.VCTools`
  （v143 工具集）。**装完需验证** `cl.exe`/`MSBuild.exe` 可用后再构建。
- ❌ **DMA 运行时依赖**：`dump_workspace/run/` 为空，缺少 `vmm.dll / leechcore.dll /
  FTD3XX.dll`。运行 DMA 命令前需从 MemProcFS 分发补齐（仓库 `deps/memprocfs/` 含源码，
  或从原机器拷贝现成 DLL）。
- ❌ **参考 dump 目录** `J:\Code\C++\dma\er_new\dump\`（本机无 J: 盘）。对比完整度时
  可跳过，或用本机自研 `--self-dump-cs` 的 508 类数据作为基准。

### 9.2 本机已完成的环境准备工作

1. ✅ **`dump_workspace/inject_test/Il2CppRuntimeDumper.cfg` 已修正**：原指向不存在的
   `J:\Code\...\out`，现改为 `F:\gua\UnityExplorer-main\dump_workspace\inject_test\out`
   （注入 DLL 时将 dump.cs / log.txt 写入此目录）。
2. ✅ **源头源码已拉取**：`dump_workspace/Il2CppRuntimeDumper_src/`（24 文件，
   `Il2Cpp.cs` 44KB + `Dumper.cs` + `Program.cs` 等全部在位）。
3. ✅ **git 代理已配置**：本机 github HTTPS 直连被重置，需代理
   `http://127.0.0.1:7877`。已写入 git 全局配置（`http.proxy`/`https.proxy`）。
   若重装系统或换机，需重新配置。

### 9.4 本机环境完善完成状态（后续会话可复用）

- ✅ **MSVC v143 工具链**：`C:\Program Files (x86)\Microsoft Visual Studio\2022\BuildTools`，
  MSBuild 17.14.51 + cl v14.44。构建命令：
  ```
  & "C:\Program Files (x86)\Microsoft Visual Studio\2022\BuildTools\MSBuild\Current\Bin\MSBuild.exe" App\ExternalResolveConsole.sln /p:Configuration=Release /p:Platform=x64 /m
  ```
- ✅ **链接库**：`vmm.lib`/`leechcore.lib` 已复制到 `deps\memprocfs\includes\lib64\`
  （来自 MemProcFS v5.18 官方包，`dump_workspace\downloads\memprocfs_v5.18.zip`）。
- ✅ **运行时自包含目录** `dump_workspace\run\`：`ExternalResolveConsole.exe` +
  `vmm.dll` + `leechcore.dll` + `FTD3XX.dll`。
- ✅ **自研注入工具** `bin\Release\`：`Il2CppRuntimeDumper.dll`(310KB) + `Injector.exe`。
- ✅ **源头源码**：`dump_workspace\Il2CppRuntimeDumper_src\`（24 文件）。
- ✅ **参考 dump.cs 基准**：`F:\gua\dump\dump2026.09.03.cs`（79.6MB / 165万行 /
  **类 11525** = class 7486 + struct 923 + interface 233 + enum 2883）——
  这是 Unity-Offset.dll 的完整注入式 dump，用作自研 DLL 输出完整度的对比基准。
- ✅ **CE 注入通道**：本机 Cheet Engine 位于 `F:\DrunkenDreamDriverCE v1.4.0\`（目标
  永劫无间定制版，含 `NarakaBladepoint.CT`、`Unity-Offset (1).dll`、DBVM 驱动）。
  任务 1 用此 CE 注入自研 `Il2CppRuntimeDumper.dll`。
- ⚠️ **exe 无参运行会阻塞**：无 FPGA DMA 设备时连接初始化挂起。用 `--help` 验证可运行。

### 9.3 接手后常规路径调整清单

- `dump_workspace/docs/*.md` 内的 `J:\...` 示例路径（仅文档，可忽略或顺手改正）
- `dump_workspace/inject_test/Il2CppRuntimeDumper.cfg`（已改 F: 盘，勿回退）
- `dump_workspace/run/` 下 exe 无硬编码路径（运行目录自包含）
- **建议**：在本机重新构建所有工程（msbuild），确认编译通过后再操作
- **网络**：github 相关拉取务必先配 `git config --global http.proxy http://127.0.0.1:7877`
