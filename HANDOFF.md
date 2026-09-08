# 交接文档 — UnityExplorer DMA 项目

> **给接手 AI 的完整交接**：本项目当前状态、已完成工作、关键文件位置、
> 编译方法、验证步骤、待办事项与重要技术结论。
> 另一台电脑拉取本仓库后，**先读本文档 + `dump_workspace/docs/项目进度与结论记录.md`**。

---

## 0. 交接摘要（30 秒版）

- **目标**：通过 DMA（FPGA，免句柄免注入）dump 永劫无间（Unity + IL2CPP）的**完整**游戏结构（dump.cs：类/字段名+偏移/方法名+RVA/属性），**脱离对闭源注入器 Unity-Offset.dll 的依赖**。
- **已完成**：
  1. ✅ DMA 镜像导出（`--dump-image`，GameAssembly 440MB 有效 PE）
  2. ✅ 自研运行时结构 dump（`--self-dump-cs`：**508 类 / 8049 字段 / 10639 方法**）
  3. ✅ 自研注入式 dump DLL（`Il2CppRuntimeDumper.dll`，机制与 Unity-Offset.dll 完全等价，已编译）
  4. ✅ 逆向确认 Unity-Offset.dll = 开源项目 [shalzuth/Il2CppRuntimeDumper]，源码已克隆到本地
  5. ✅ DMA 注入原理验证（`--dma-inject`：executed=true，LoadLibraryA 成功加载过 DLL）
- **待办**：游戏下载完成后，**用 CE 注入自研 DLL 验证 dump.cs 完整度**（首选路径）；DMA 注入受 VMP 保护模块空洞限制不稳定（原理已验证）。
- **环境**：Windows x64 + Visual Studio 2022/2026 + MemProcFS(vmm.dll) + FPGA DMA（Metick/PCILeech）。

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

## 5. 下一步任务（接手后按序执行）

### 任务 1（最高优先）：游戏下载完成后，CE 注入验证自研 DLL
1. 游戏启动到**登录界面**（普通变体，Unity-Offset.dll 已验证此时机可 dump）
2. 确认 `dump_workspace\inject_test\Il2CppRuntimeDumper.cfg` 指向期望输出目录
3. 用 **CE 注入** `dump_workspace\inject_test\Il2CppRuntimeDumper.dll`
   （用户有 CE 注入器；也可用 `bin\Release\Injector.exe`，但反作弊可能拦截
   CreateRemoteThread —— 用户 CE 注入是已验证通道）
4. 等待 10-30 秒，检查输出目录 `dump.cs` + `log.txt`
5. **对比**：与 Unity-Offset.dll 的输出（`J:\Code\C++\dma\er_new\dump\` 下
   历史 dump.cs）对比类数/字段数/方法数，验证完整度
6. 若 dump.cs 异常/缺失：查看 log.txt（[FAIL]/[CRASH]/[OK]），
   检查 `dllmain.cpp` 的 `DumpWorkerBody` 与 `runtime_dumper.cpp` 的 `RunDump`

### 任务 2（可选攻坚）：DMA 注入稳定性
- 代码洞改到主模块（NarakaBladepoint.exe）：
  用 DMA 直读主模块 PE（`ReadModuleSections` 已验证可读 GameAssembly，
  主模块同样方式）找无 VMP 的 r-x/rw- 空洞
- 或 shellcode 内先 VirtualAlloc 再跳转（需两步 shellcode）

### 任务 3（可选）：dump.cs 输出格式升级
- 若需 Il2CppDumper 兼容格式：参考 `Il2CppRuntimeDumper_src/Dumper.cs`
  的 Mono.Cecil DummyDll 重建方式

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

## 9. 与外部机器的路径差异提醒

- 本仓库路径原为 `J:\Code\C++\dma\er_new\UnityExplorer-main`，
  拉到新机器后**路径会变**。以下位置硬编码了旧路径，接手后需处理：
  - `dump_workspace/docs/*.md` 内的示例路径（仅文档，可忽略）
  - `dump_workspace/inject_test/Il2CppRuntimeDumper.cfg`（输出目录，需改）
  - `dump_workspace/run/` 下 exe 无硬编码路径（运行目录自包含）
  - 参考 dump 目录 `J:\Code\C++\dma\er_new\dump\` 可能不存在于新机器
    （可跳过对比，或从原机器拷贝）
- **建议**：在新机器重新构建所有工程（msbuild），确认编译通过后再操作
