# SuperDumper 默认时间戳目录构建记录

日期：2026-09-12

## 修改内容

`App/Il2CppRuntimeDumper/SuperDumper.cpp` 的默认输出根目录由 `C:\\selfdump` 改为 `C:\\YJDumped`。
每次注入运行开始时生成精确到毫秒的时间戳目录：

```text
C:\\YJDumped\\YYYYMMDD_HHMMSS_mmm\\
```

本次运行的 `dump_*.cs`、`log.txt`、`*_structures` 和其他附加产物都会写入该目录。

如果 DLL 同目录存在 `SuperDumper.cfg` 并明确设置第一行目录或 `outdir/out=...`，该目录作为根目录，仍会在其下创建时间戳子目录。`bin\\Release` 中的两个 SuperDumper 配置已统一改为 `C:\\YJDumped`，避免旧的 `out_super` 路径覆盖默认设置。

## 构建结果

构建命令：

```powershell
MSBuild.exe App\\Il2CppRuntimeDumper\\SuperDumper.vcxproj /p:Configuration=Release /p:Platform=x64 /m
```

首次修改后曾因旧 PDB 被占用而未更新 DLL；随后关闭 Release 调试信息并执行完整重建，构建成功。产物：

```text
F:\\gua\\UnityExplorer-main\\bin\\Release\\SuperDumper.dll
```

当前 Release DLL 不再生成或依赖 PDB，便于复制到其他机器注入使用。

## 验证要点

日志新增 `outputMode`、`root` 和 `timestamp` 行，用于确认时间戳目录及其根路径；配置文件存在时会显示 `configured-timestamp-folder`，但仍位于 `C:\\YJDumped`（除非用户另行修改配置根目录）。
