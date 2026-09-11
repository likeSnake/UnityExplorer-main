# Super 模块 Dump 修正工作日志

> 日期：2026-09-09  
> 对象：`GameAssembly_Super.dll` / `GameAssembly_Super_IBT.dll` 变体  
> 修正产物：`F:\gua\UnityExplorer-main\bin\Release\SuperDumper.dll`

## 1. 问题现象

检查 `dump_workspace\inject_test\out_super\` 后，原产物实际为：

```text
dump.cs  = 784,120 bytes（约 765 KiB）
log.txt  = 2,868 bytes
```

虽然文件不是字面上的几百字节，但内容远未达到完整 dump 要求。原日志统计为：

```text
images  = 7
classes = 2251
fields  = 5275
methods = 2
props   = 2190
```

发现的镜像只有 `netstandard.dll`、`UnityEngine.dll`、`System.dll`、`System.Core.dll` 等框架
程序集，没有 `Assembly-CSharp.dll`。因此这是“部分框架结构被正确输出”，不是完整 dump。

## 2. 根因

原 `SuperDumper.cpp` 的路径选择逻辑是：

```text
只要 4 个 bootstrap 导出存在
    -> 进入普通体 API 引导
否则
    -> 进入 Super 纯内存扫描
```

但实际 Super 构建并不一定把所有 `il2cpp_*` 导出都清零。当前日志显示：

```text
module = GameAssembly_Super.dll
bootstrap exports = available
```

这些残留导出只能访问少量已注册的框架镜像，不能代表完整的类型表已经可以通过导出遍历。
于是 dumper 误走普通体分支，得到 7 个镜像和 2251 个类，随后因为流程本身没有完整度门槛而写出
`result=OK`。这就是本次“文件能生成但效果很差”的直接原因。

## 3. 修正内容

### 3.1 按模块变体选择路径

在 `SuperDumper.cpp` 中增加模块身份判断：

- `GameAssembly.dll` 且 bootstrap 完整：使用普通体导出引导；
- `GameAssembly_Super.dll`、`GameAssembly_Super_IBT.dll` 或名称包含对应 Super 后缀：
  强制使用纯内存扫描；
- Super 变体即使残留 `il2cpp_get_corlib`、`il2cpp_image_get_class` 等导出，也只把它们写入诊断日志，
  不再用它们决定遍历路径。

纯内存路径执行：

```text
扫描可读进程区域
    -> 通过 Il2CppClass 名称、image、byval_arg 自引用进行结构校验
    -> 收集全部 Il2CppClass*
    -> 由 token 建立 TypeDefinitionIndex 表
    -> 由 klass->image 去重得到完整镜像集合
    -> 复用布局校准、字段/方法/属性读取和 dump 输出
```

### 3.2 增加不完整结果提示

Super 模式完成后会额外检查：

- 是否发现 `Assembly-CSharp.dll`；
- 类数是否低于 10000；
- 方法数是否低于 10000。

异常时在日志中写入 `[WARN]`，避免“小规模框架 dump”被误判为完整成功。

### 3.3 构建文件调整

由于旧 PDB 文件在链接时被占用，工程的 Release PDB 名称改为：

```text
SuperDumper_v2_Release.pdb
```

这只是构建产物路径调整，不影响 DLL 的运行接口和注入方式。

## 4. 验证结果

### 4.1 构建

`SuperDumper.vcxproj` 已成功完成 Release|x64 重建：

```text
F:\gua\UnityExplorer-main\bin\Release\SuperDumper.dll
```

### 4.2 离线回归

重新运行 `DumpSelfTest.exe`，结果仍为：

```text
pass=59 fail=0
```

覆盖标准布局、+8 变形布局、索引型 `Il2CppType.data`、方法尾部变化以及无导出 Super 引导路径。

### 4.3 尚未执行的部分

本次没有再次注入真实游戏，也没有替用户运行 Super 变体。真实 Super 结果需要用户自行注入新 DLL
后检查：

```text
dump_workspace\inject_test\out_super\log.txt
dump_workspace\inject_test\out_super\dump.cs
```

重点应看到：

```text
[0] variant=Super ...
[0] Super module detected: forcing pure memory scan
[0] super mode: no bootstrap exports -> pure memory scan
[scan] klasses found = ...
[scan] images derived from klasses = ...
[2] dumping 'Assembly-CSharp.dll' ...
[3] DONE classes=... fields=... methods=... props=...
[4] result=OK
```

如果仍然只有少量类，优先回传完整 `log.txt`，尤其是 `[scan] klasses found`、`[cal]` 和
`[scan] images derived` 行；不要只回传 dump 文件大小。

## 5. 修改文件

```text
App/Il2CppRuntimeDumper/SuperDumper.cpp
App/Il2CppRuntimeDumper/SuperDumper.vcxproj
App/Il2CppRuntimeDumper/il2cpp_dump_core.hpp
bin/Release/SuperDumper.dll
```

`il2cpp_dump_core.hpp` 只增加了 Super 输出完整度警告，不改变已经通过 59/59 自测的结构读取逻辑。

## 6. 第二轮实测与最终修正（18:05）

### 6.1 第一轮方案被真实日志否定

用户清空旧输出并注入第一轮 DLL 后，`out_super` 只生成了 `log.txt`，没有 `dump.cs`：

```text
variant=Super bootstrap exports: available
Super module detected: forcing pure memory scan
super mode: no bootstrap exports -> pure memory scan
[scan] regions=2634 bytes=0x135E58000
[scan] klasses found = 0
[FAIL] no Il2CppClass found by scan
[4] result=FAIL
```

这证明第一轮“只要模块名是 Super 就丢弃全部兼容导出”的决策不正确。真实 Super 类结构不满足
合成测试使用的 `byval_arg.data == klass` 自引用条件，因此纯 `Il2CppClass` 扫描把所有真实类都
过滤掉。合成测试虽然通过，但只证明算法适用于合成布局，不能证明真实保护体仍保留同一自引用。

同时，日志已经证明当前 Super 模块仍保留下列可调用入口：

```text
il2cpp_get_corlib
il2cpp_image_get_class
il2cpp_image_get_class_count
il2cpp_image_get_name
il2cpp_image_get_assembly
```

所以最终路线改为：保留这些导出作为可靠的名称、类数量与逐类枚举入口，只把“镜像发现”扩大到
全部可读、私有、可写内存区域。纯类扫描仅作为导出确实不存在时的最后回退，不再是 Super 的
默认路径。

### 6.2 镜像发现不再依赖普通体回指

第二轮扫描同时收集两类候选：

1. 内存中保存的 `Il2CppImage*` 指针；
2. 直接位于扫描区域中的镜像结构，即当前 qword 指向 `.dll/.exe` 名称时，把 qword 地址本身作为候选。

Super 模式的初筛不再强制 `image->assembly->image == image`，因为保护体可能重排 assembly/image
关系。每个候选最后必须依次通过以下交叉验证：

```text
image_get_name(candidate) 返回合法程序集名
image_get_class_count(candidate) 返回 1..400000
image_get_class(candidate, 0) 返回可读 Il2CppClass
firstClass->image == candidate
firstClass->name 是合法类名
```

所有探测性导出调用都增加了独立 SEH 边界。单个假候选即使触发访问异常，也只会被拒绝，不会让
整次 dump 在找到真实镜像表之前中止。

### 6.3 自适应 `Il2CppImage` 布局

旧实现把 `+0x18/+0x1C` 固定解释成 `typeStart/typeCount`。Super 旧产物的数量关系提示保护体可能
把类数量放在 `+0x18`，继续硬编码会造成错误排序、错误 TypeDefIndex，甚至把其它字段误当成类数。

新实现遵循两个原则：

- 类数量始终优先采用 `image_get_class_count` 的返回值，不再用较大的 raw dword 覆盖 API 真值；
- 在 `+0x18..+0x44` 候选 dword 中，按相邻镜像 `next.start == current.start + API count`
  的连续关系评分，自动选择 `typeStart` 字段。

已支持并记录以下 profile：

```text
count@0x1C,start@0x18              标准布局
count@0x18,start@0x1C              Super 交换布局
count@...,start@其它候选偏移       其它重排布局
synthesized-address-order          无可用 start 字段时按镜像地址顺序累计
```

布局、原始 `raw18/raw1C`、API count、最终 start 都会写入日志，下一次真实运行无需再根据文件大小猜测。

### 6.4 扫描性能与诊断

全内存扫描仍按 32 KiB 块顺序读取，但去重集合只记录通过 ASCII 名称前缀初筛的指针，不再记录
进程中的每个随机指针，避免 5 GB 进程产生巨大的哈希表。新增日志字段包括：

```text
[scan] full private image scan regions=... bytes=... qwords=... tested=... candidates=... elapsedMs=...
[scan] image validation structural=... apiAccepted=... reject(struct=... name=... count=... class=...)
[img] layout=... countMatch18=... countMatch1C=... exact=... compatible=...
img=... name='...' start=... count=... raw18=... raw1C=...
```

### 6.5 离线验证

`DumpSelfTest.cpp` 新增 Phase D，实际走完整混合分支：

```text
Super 兼容导出
-> 全私有内存镜像扫描
-> 多个结构候选
-> API + firstClass->image 交叉验证
-> count@0x18/start@0x1C 自动识别
-> 类、字段、属性、方法完整输出
```

最终结果：

```text
pass=68 fail=0
```

其中 Phase D 实际得到 11 个结构候选、API 验证后仅保留 1 个真实镜像，并正确恢复
`count@0x18,start@0x1C`。这同时验证了候选过滤和镜像头自适应，而不是只测试直接传入的镜像列表。

### 6.6 当前交付状态与下一次真实验收

当前交付 DLL：

```text
F:\gua\UnityExplorer-main\bin\Release\SuperDumper.dll
```

本轮只完成构建和离线验证，没有替用户注入真实目标。下一次用户自行运行后，完整成功日志应包含：

```text
=== SuperDumper v2 (hybrid image discovery) start ===
Super module has usable compatibility exports
[scan] full private image scan ...
[scan] image validation ... apiAccepted=接近完整镜像数
[img] layout=...
[1] images = 接近完整镜像数
[2] dumping 'Assembly-CSharp.dll' ...
[3] DONE classes=数万 fields=... methods=...
[4] result=OK
```

若仍失败，完整 `log.txt` 已足够区分“没有结构候选、API 验证拒绝、镜像布局无法确定、
Assembly-CSharp 缺失、类布局校准失败”五类问题。

## 7. 第三轮实测：27 MB 伪完整结果（18:40）

### 7.1 为什么 27 MB 仍然不正常

用户实测已生成 `dump.cs`，文件为 `28,378,800` 字节（约 27.1 MB），但大小不能作为完整性判据。日志中的结构化计数为：

```text
images=123
classes=56495
fields=269855
methods=23
props=80991
```

镜像和类已基本枚举完整，但 56,495 个类只输出 23 个方法明显不成立。参考 dump 中有约 41.8 万条有效 RVA；同时旧产物还存在大量 `Class` / `Type0` 退化名称。因此这次结果属于“类框架完整、方法和类型语义缺失”的伪完整 dump。

### 7.2 从日志定位出的两个真实布局差异

第一处是新版 `MethodInfo` 头部新增了一个指针槽。旧版解释为：

```text
name=0x10 klass=0x18 return=0x20 parameters=0x28
```

实测 Super 对应：

```text
name=0x18 klass=0x20 return=0x28 parameters=0x30
token=0x48 flags=0x4C slot=0x50 parameterCount=0x52
```

旧校准器只尝试旧头部，所以无法验证 methods 数组，最终把几乎所有方法丢弃。

旧日志还提供了一个重要反证：在 `cMethods` 保持默认 `0x98` 时，方法尾部探测得到
`score=219 tested=219`，说明 `0x98` 确实能稳定取到真实 `MethodInfo*`；同时旧 dump 从
`0x90` 读出的属性名称连续且语义正确，只是返回类型因 accessor 头部错位而统一退化为
`Type0`。因此实机更可信的数组位置仍是 `properties=0x90`、`methods=0x98`，不能仅凭
class 尾部整体后移就推测为 `0x98/0xA0`。

第二处是镜像头中没有可用的连续 `typeStart`。`+0x18` 是 API 返回的 class count，`+0x1C` 多数为零。旧实现按镜像地址累加 class count，虽然能写出全部类，却会给类和 `Il2CppType.data` 建立错误的全局索引关系，造成类型名大量退化。

### 7.3 修复方式

核心 dumper 现在会：

- 同时探测三代 `MethodInfo` 头部布局，并将识别出的 name、klass、return、parameters 偏移用于方法数组和属性访问器校验；
- 仅在 methods 数组已验证后校准方法尾部和参数数组，避免用错误数组污染后续判断；
- 探测类内嵌 `Il2CppType` 的 `0x20/0x28/0x30/0x38` 候选位置；
- 当内嵌 `data` 是小整数且类型为 CLASS/VALUETYPE/ENUM 时，直接据此重建 `TypeDefinitionIndex -> Il2CppClass*` 表；
- 以每个镜像中的最小类索引恢复镜像顺序和起始 TypeDefinitionIndex；
- API 输出类时使用重建后的真实类索引，而不是错误的镜像地址累加值；
- Super 输出方法数低于 10,000 时返回失败并保留警告，避免再次把伪完整文件报告为成功。

### 7.4 新增回归场景与结果

`DumpSelfTest.cpp` 新增 Phase E，完整模拟本次实测特征：

```text
Il2CppClass: properties=0x90 methods=0x98 interfaces=0xB0
counts: method=0x120 property=0x122 field=0x124 interface=0x128
MethodInfo: name=0x18 klass=0x20 return=0x28 parameters=0x30
MethodInfo tail: token=0x48 flags=0x4C slot=0x50 parameterCount=0x52
image: class count only, no usable typeStart
klass byval_arg.data: global TypeDefinitionIndex
```

自动校准实际命中了上述布局，并恢复全部 15 个合成方法、正确 RVA、参数签名、基类、接口及索引型字段类型。全部离线回归结果：

```text
pass=87 fail=0
```

正式 DLL 的新日志首行为：

```text
=== SuperDumper v3 (layout and type-index recovery) start ===
```

下一轮实测的关键成功标志是 `[tbl] index map built from klass byval`、methods 校准得分不再为零、`method_count` 命中有效偏移，以及最终 methods 数量恢复到合理量级。

### 7.5 构建交付

Release x64 已成功构建：

```text
DLL: F:\gua\UnityExplorer-main\bin\Release\SuperDumper.dll
size: 307200 bytes
SHA256: 24A7C6B83B277F6049337B8B4B38D1B08EB0EC00F7D96E77A45F6F9C918AC941
PDB: F:\gua\UnityExplorer-main\bin\Release\SuperDumper_v3_Release.pdb
```

旧 `SuperDumper_v2_Release.pdb` 被进程占用，首次 Rebuild 因无法覆盖该 PDB 而失败；将本轮 PDB 改为 v3 文件名后链接成功。DLL 名称没有变化。

## 8. 第四轮实测：方法恢复，但类型仍不完整（19:10）

### 8.1 v3 实测结论

v3 成功生成：

```text
dump.cs: 73,722,369 bytes（70.3 MiB）
images=123
classes=56495
fields=269855
methods=497361
props=80991
RVA non-empty=496235
result=OK
```

方法布局修复已被真实目标验证。校准稳定命中：

```text
methods=0x98
MethodInfo name/klass/ret/params=0x18/0x20/0x28/0x30
MethodInfo token/flags/slot/paramCount=0x48/0x4C/0x50/0x52
method_count=0x120
```

方法数从 23 恢复到 497,361，且 496,235 条带非零 RVA，这一部分已经成功。

### 8.2 为什么 v3 仍不能判为最终成功

同口径质量脚本结果：

```text
Super v3: Class 类型占位 260281，Type0 0
参考产物: Class 类型占位 6，Type0 0
```

`Class` 类型占位仍有 260,281 行，说明类类型、值类型和大量方法参数仍未解析到真实类名。日志也直接显示：

```text
[tbl] klass byval index probe off=0x28 hits=15 unique=1 samples=512
[tbl] index map built from images
[cal] typedata ptr=0 idx=0 -> mode=pointer
```

索引探测没有通过，代码回退到了镜像地址顺序；这也导致 `Assembly-CSharp.dll` 被错误放到 TypeDefIndex 0，而参考布局中它位于全部框架程序集之后。因此 v3 是“方法完整、类型语义仍不完整”，不能仅依据 70 MB 和 `result=OK` 判为最终成功。

参数名仍是另一项独立差距：v3 为“合成 235,101 / 真实 0”，参考产物为“合成 20,018 / 真实 189,459”。新版 MethodInfo 的参数数组是紧凑 `Il2CppType*[]`，本身不保存名字；后续需要利用 method metadata handle 恢复参数定义表。本轮 v4 先解决 class/type handle 映射，并保留相应日志供下一轮确认。

### 8.3 v4 修复：metadata type handle

当前特征符合新版 libil2cpp 的 `Il2CppMetadataTypeHandle` 表示：`Il2CppType.data` 是一个指向 metadata 类型定义的句柄，既不是 `Il2CppClass*`，也不是小整数 TypeDefinitionIndex。

v4 新增以下处理：

- 从每个 API 枚举类的内嵌 `byval_arg` 探测指针型 metadata handle；
- 排除可直接解释为 `Il2CppClass*` 的普通指针，要求 handle 可读、对齐且样本唯一率足够高；
- 建立 `metadata type handle -> Il2CppClass*` 反向表，用于字段、返回值、参数和属性类型解析；
- 对所有 handle 按地址排序，并用每个镜像内 API 类顺序进行连续性验证；只有至少 75% 镜像和类严格连续时才据此恢复全局 TypeDefinitionIndex；
- 若全局顺序验证不通过，仍保留 handle 到 class 的名称映射，但不采用未经验证的索引顺序；
- 新增 `typeFallbacks` 输出统计，Super 占位数超过 1,000 时写入警告并返回失败，避免假成功。

新增 Phase F 模拟 metadata handle 表示，验证 handle 反向映射、索引排序、类型名、方法签名和零占位。全部离线回归：

```text
pass=99 fail=0
```

### 8.4 v4 构建交付

```text
DLL: F:\gua\UnityExplorer-main\bin\Release\SuperDumper.dll
size: 318464 bytes
SHA256: 8B935A494BA9ED9400951CB8F8BD980CF5DCADFA6F9FF64FBEAF0A2A40B77223
PDB: F:\gua\UnityExplorer-main\bin\Release\SuperDumper_v4_Release.pdb
```

新日志首行：

```text
=== SuperDumper v4 (metadata type-handle recovery) start ===
```

真实成功时应看到 `type-handle map built`、`typedata ... mode=handle`，并且最终 `typeFallbacks` 接近零。

### 8.5 v4 真实运行验收（2026-09-10）

用户使用 v4 重新注入并生成了完整产物：

```text
dump.cs: 76,884,360 bytes（73.3 MiB），1,927,568 行
SHA256: 9AC8B9ED25F7EA95E664C21568623070AEA751DA59260CAE0C9EC61530C8BF3D
log.txt SHA256: 974C0CF5C8CC3D238A31081DEBD2EF05CEC72C7923C22C67011BE25C2E260766
images=123
classes=56495
fields=269855
methods=497361
props=80991
RVA non-empty=496235
RVA empty=1126
result=OK
```

日志命中了 v4 预期路径：

```text
[tbl] klass type-handle probe off=0x20 hits=512 unique=512 tdhMatch=512 samples=512
[tbl] type-handle map built entries=56478 records=56478 expected=56496
[cal] typedata ... -> mode=handle
[3] ... typeFallbacks=0
```

类型恢复目标已经成功。v3 中 260,281 处 `Class` 占位已被真实类型替换；例如末尾结构现在能够输出：

```csharp
public Vector4 position;
public Matrix4x4 worldToShadowMatrix;
```

质量脚本报告的 5 个 `Class` 不是类型退化，而是名为 `Class` 的合法枚举成员、字段或属性，因此实际类型占位为零。方法数量也严格满足 `496235 + 1126 = 497361`，文件末尾类定义正常闭合，没有发现截断、读取失败或无效类型标记。

56496 个 API 类槽位中输出了 56495 个。唯一缺口为当前合成索引 `0xD13E`，位于 `Ionic.Zip.Unity.dll` 的最后一个类槽；该槽的类名不可读或未通过名称校验，因此 `writeClass` 按设计跳过。缺失率约为 0.0018%，不影响主体 dump 的可用性。

本次结果可判定为“v4 核心修复成功，dump 已可用于主要分析”，但还不能判定为与参考 `Unity-Offset.dll` 完全等价，仍有两项明确差距：

- type handle 共恢复 56478 个，较 56496 个类少 18 个；122/123 个镜像顺序严格通过，唯一未通过的镜像符合 `mscorlib.dll` 特殊基础类型缺少普通 handle 的特征。当前因此采用 `map-only`，并回退到运行时镜像地址顺序，导致 `Assembly-CSharp.dll` 为 Image 0 / TypeDefIndex 0，而参考产物中为 Image 123 / TypeDefIndex 17113。
- 参数类型已经恢复，但参数名称仍全部为合成的 `p0`、`p1` 等。日志为 `runtimeParamNames=0`、`synthParams=352775`；参考产物则包含大量真实参数名。恢复真实参数名需要继续定位 method metadata handle 和 metadata 参数定义表。

结论：v4 已解决此前导致 Super dump 基本不可用的类型解析问题，73.3 MiB 的文件规模正常且结构完整；后续若追求参考产物级一致性，应继续修复含少量缺失 handle 时的镜像排序，并单独恢复真实参数名。

### 8.6 v5 类级 RVA 输出（2026-09-10）

用户指出参考普通版 dump 在 `GameBaseObject` 类头部还包含：

```text
// RVA: 0xe644808
// RVA: 241453064
```

这里的两个数值相等（十六进制与十进制），表示 **GameAssembly 内保存 `Il2CppClass*` 的全局 TypeInfo 指针槽 RVA**，不是 `// Instance: 0x...` 中的堆上 `Il2CppClass*` 地址。参考文件来自普通 `GameAssembly.dll`，因此其 `0xe644808` 不能直接复制到 `GameAssembly_Super.dll`；Super 必须按自己的模块基址和内存布局重新解析。

v5 在通用 dump 核心中加入了类级槽位扫描：

- 先从当前运行的类型表/API 枚举中定位 `GameBaseObject`、`CharacterManager`、`UserData`、`CharactorSync`、`AcSDKManager`、`GameEntity`；
- 在当前 `GameAssembly` 模块映像范围内扫描指向这些 `Il2CppClass*` 的 qword；优先检查可写数据页，未命中再检查非执行只读页；
- 对每个类选择最小合法槽地址，计算 `slotVA - moduleBase`；
- 类头部按参考格式输出十六进制和十进制两行；日志记录候选数、槽地址、RVA 和未命中原因；
- 增加 `classrvas=0/1` 配置开关，默认开启。离线测试提供 `overrideClassPointerSlots`，避免依赖真实进程地址。

新增输出示例：

```text
// Class: GameBaseObject
// Namespace:
// Instance: 0x...
// RVA: 0x...
// RVA: <decimal>
public sealed class GameBaseObject ...
```

验证结果：`DumpSelfTest.exe` 全部通过，`pass=102 fail=0`；覆盖标准布局、Super 变形布局、无导出扫描、metadata handle 以及类级 RVA 双格式输出。

v5 Release 构建：

```text
DLL: F:\gua\UnityExplorer-main\bin\Release\SuperDumper.dll
size: 323584 bytes
SHA256: D0BE7E88686487C18B698F56A60430A719A7620579898B9307B8CDEDB96C0C13
PDB: F:\gua\UnityExplorer-main\bin\Release\SuperDumper_v5_Release.pdb
```

实机运行后应重点查看 `log.txt` 中的 `[class-rva]` 行：`resolved` 表示找到槽位的目标类数量，`slot=... rva=... candidates=...` 表示最终选择及扫描命中数；`slot-not-found` 只说明该类的全局指针槽不在当前扫描范围内，不会影响其余类的正常 dump。

### 8.7 v6 BuffManager / TransparentTagHandler 顶部偏移摘要（2026-09-10）

本次目标是在注入进程内直接查找以下 TypeInfo 指针槽 RVA，并把结果放在 `dump.cs` 第一行开始的位置：

```text
普通 GameAssembly.dll:
m_BuffManager
m_TransparentTagHandler

Super / Super IBT:
m_BuffManager2
m_TransparentTagHandler2
```

实现复用了 v5 的当前模块 TypeInfo 槽扫描，没有引入 DMA。扫描目标增加 `BuffManager` 与 `TransparentTagHandler`，先从当前运行时类表按“精确类名 + 空命名空间”找到 `Il2CppClass*`，再在本次加载的 GameAssembly 模块映像内寻找值等于该 klass 的 qword，并计算 `slot - moduleBase`。扫描仍优先可写页，只读页仅补充可写页未命中的类，避免回退结果覆盖更可信的全局数据槽。

在类和槽定位后增加了静态实例链验证：

```text
BuffManager:
klass -> static_fields -> +0x0 _instance -> instance klass

TransparentTagHandler:
klass -> static_fields -> +0x8 s_instance -> instance klass
```

`static_fields` 不再固定写死为 `klass+0xB8`。首选位置由运行时校准后的 `cInterfaces+0x10` 推导：标准布局为 `0xB8`，本次已知 Super 新布局为 `0xC0`；同时保留 `0xB8/0xC0/0xC8/0xD0` 兼容候选，并以 `instanceKlass == klass` 作为最终验证条件。

顶部输出状态含义：

```text
VERIFIED   类、TypeInfo 槽、static_fields、实例回指全部通过
CLASS_ONLY 类和 TypeInfo 槽已定位，但实例尚未初始化或实例链当前不可确认
NOT_FOUND  类未找到或模块内 TypeInfo 槽未找到，顶部同时写明 reason
```

Super 输出格式示例：

```text
// ===== Runtime TypeInfo RVA Discovery =====
// Module: GameAssembly_Super.dll
// Module Base: 0x...
// m_BuffManager2 = 0x...; // decimal: ... status=VERIFIED
//   klass=0x... slot=0x... candidates=... staticFieldsOffset=0xC0 staticFields=0x... instanceOffset=0x0 instance=0x... instanceKlass=0x...
// m_TransparentTagHandler2 = 0x...; // decimal: ... status=CLASS_ONLY
//   klass=0x... slot=0x... candidates=... staticFieldsOffset=0xC0 staticFields=0x... instanceOffset=0x8 instance=0x0 instanceKlass=0x0
// ==========================================

// Image 0: ...
```

日志同步增加 `[manager-rva]` 证据行，包含 klass、slot、RVA、候选数、static fields 偏移、static fields、instance、instance klass 和状态。这样即使运行时对象为空，也能区分“TypeInfo 已定位”和“类型定位失败”。

离线测试扩充为 13 个合成类，并覆盖：普通字段名、Super 的 `*2` 字段名、标准 `static_fields=0xB8`、Super `static_fields=0xC0`、`VERIFIED`、`CLASS_ONLY`、`NOT_FOUND`，以及摘要确实位于第一条 `// Image` 之前。回归结果：

```text
pass=110 fail=0
```

v6 Release 构建：

```text
DLL: F:\gua\UnityExplorer-main\bin\Release\SuperDumper.dll
size: 329216 bytes
SHA256: 5C9974216B205D10FE4EAEBB9461B634B5FFED5226BC81763EE43C64CB296F1D
PDB: F:\gua\UnityExplorer-main\bin\Release\SuperDumper_v6_Release.pdb
PDB SHA256: 195EE2B3B6661AFDC70FFFE5F95A05BBB8BDEAA1D5AC3CD611AD1B5690DB3BA8
```

新日志首行：

```text
=== SuperDumper v6 (manager TypeInfo RVA discovery) start ===
```

本节只确认离线结构和 Release 构建通过；真实 RVA 必须由 v6 注入当前目标进程后按本次模块基址重新产生，不能沿用经验文档中的历史普通版或 Super 版数值。

## 2026-09-10 v7：完整 Offset.h 汇总与映射核对

v7 已构建，新增 221 项 Offset.h 目录摘要和 `[offset-report]` 日志，位于 dump.cs 的 Image 列表之前。三组 ActorKitInteractiveS / RuntimeLogicLayerS / PlayableStateS 的 24 个字段已补齐精确映射，数字与现有 Super dump 一致。对其他不符项以 dump 为准，并记录泛型 List 零偏移、旧 FlushState 缺失、TTH 字段拼写、接口声明和普通 CrossFade RVA 差异。

自检 `pass=153 fail=0`；DLL `436736 bytes`，SHA256 `4CD7713829B1367D33777020917ED8BB241EE07F3B816762F5C4BD8088BC0590`。v7 尚待用户实机运行。

完整过程、24 项映射表、差异和限制见 [v7 偏移汇总与差异记录](2026-09-10_SuperDumper_v7偏移汇总与差异记录.md)。特别更正：上文 v6 的 `static_fields=0xC0` 是合成场景；用户真实 Super 结果验证为 `0xB8`，v7 使用回指证据而非 interfaces+0x10 猜测。
