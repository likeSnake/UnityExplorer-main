# Headless DMA Offset Workflow

## Current Goal

Build a local, read-only, DMA-first Unity/IL2CPP probe that can be driven from
the command line. The immediate CTF goal is to produce an `Offset.h`-like
runtime report for the current target process without requiring manual ImGui
clicks.

## Target Observed

- Process: `NarakaBladepoint.exe`
- DMA: Metick/MemProcFS through local FPGA DMA hardware
- Unity module: `UnityPlayer_LVB.dll`
- IL2CPP module: `GameAssembly_Super.dll`
- Unity version target: Unity 2019.4.41 candidate
- Layout profile currently selected:
  `unity-2019.4-lts-24.5`

## Headless Entry

Build:

```powershell
msbuild App\ExternalResolveConsole.sln /p:Configuration=Release /p:Platform=x64
```

Run:

```powershell
App\x64\Release\ExternalResolveConsole.exe --headless --target NarakaBladepoint.exe --mode dma --scan gameobjects --limit 32 --out bin\Release\headless_probe.json --log bin\Release\headless_probe.log
```

Run with live manager/global RVA discovery:

```powershell
App\x64\Release\ExternalResolveConsole.exe --headless --target NarakaBladepoint.exe --mode dma --scan gameobjects --limit 64 --scan-managers --manager-scan-mb 96 --out bin\Release\headless_probe.json --log bin\Release\headless_probe.log --offset-header bin\Release\OffsetRuntime.h
```

## Runtime-only dump.cs notes

When `global-metadata.dat` cannot be found through DMA, use runtime class
evidence instead of metadata export:

```powershell
App\x64\Release\ExternalResolveConsole.exe --headless --target NarakaBladepoint.exe --mode dma --scan none --limit 0 --runtime-dump-rvas 0x35E25C0,0x35F4240,0x35E2628,0x35872C8,0x359CF28 --runtime-dump-cs .tmp\runtime_dump.cs --runtime-dump-limit 64 --out .tmp\runtime_dump.json --log .tmp\runtime_dump.log
```

Latest DMA runtime dump evidence on `NarakaBladepoint.exe`:

- `UnityPlayer_LVB.dll` was loaded at `0x7FF9CA1C0000`.
- `GameAssembly_Super.dll` was loaded at `0x7FF9B0A40000`.
- MSID slot was found at `0x7FF9CD5711D0`, count `232652`.
- MSID object enumeration accepted `8882` objects and stored `2048` samples.
- Runtime dump built `122` `Il2CppClass` reports.
- Conservative `dump.cs` export emitted `12` classes, `177` fields, and `1`
  method from live DMA evidence.
- Output now uses standard dump-like sections:
  `// Class`, `// Namespace`, `// Instance`, `// Fields`, `// Properties`,
  `// Methods`, and method `// RVA / VA / Slot` comments when verified.
- Primitive/string fields resolve to C# aliases such as `bool`, `int`, `float`,
  and `string`.
- Method export is intentionally conservative: method rows are emitted only
  when `MethodInfo` has sane token/flag/parameter evidence and pointers stay
  inside expected ranges. This avoids treating type tables or code bytes as
  method names.
- Source/type names from MSID are retained as evidence comments, but weak names
  such as `UnityEngine.UI` are not promoted to class names when they would
  collapse many different `klass` values into one misleading name.

Latest verified command:

```powershell
App\x64\Release\ExternalResolveConsole.exe --headless --target NarakaBladepoint.exe --mode dma --scan msid --limit 2048 --scan-managers --manager-scan-mb 96 --runtime-dump-rvas 0x35E25C0,0x35F4240,0x35E2628,0x35872C8,0x359CF28 --runtime-dump-cs .tmp\runtime_dump_dma_final_20260505_230625.cs --runtime-dump-limit 512 --out .tmp\runtime_dump_dma_final_20260505_230625.json --log .tmp\runtime_dump_dma_final_20260505_230625.log
```

Current limitation:

- `global-metadata.dat` is still not required and was not used.
- Full Il2CppDumper-equivalent method signatures/properties are not possible
  from this runtime-only path yet. Recovering them requires a dedicated
  `Il2CppClass`/`MethodInfo` layout auto-prober or a recovered metadata/code
  registration path.
- The manager section scan can fail independently when DMA cannot read PE
  sections on that attempt; runtime dump still proceeds from MSID and explicit
  RVA seeds.

Known-good run result:

- PID: `8608`
- UnityPlayer: `UnityPlayer_LVB.dll @ 0x7FFE23A70000`
- GameAssembly: `GameAssembly_Super.dll @ 0x7FFE09D80000`
- MSID slot: `0x7FFE26E211D0`
- MSID slot RVA: `0x33B11D0`
- MSID count: `242129`
- GameObjects matched: `343`
- Stored object samples: `32`

## Implemented Files

- `App/ExternalResolveConsole/main.cpp`
  - Adds `--headless` mode.
  - Initializes Metick DMA and FixCR3.
  - Enumerates loaded modules.
  - Matches `UnityPlayer*.dll` and `GameAssembly*.dll`.
  - Scans MSID slot.
  - Enumerates GameObject/ScriptableObject/all MSID rows.
  - Emits JSON and optional log output.

- `include/er2/unity2/headless/probe_cli.hpp`
  - CLI parser and headless JSON helpers.

- `include/er2/unity2/headless/manager_rva_scan.hpp`
  - Header-only live `GameAssembly` manager RVA scanner.
  - Uses old Naraka code only as chain-shape reference, not as an RVA source.
  - Scores candidates by current memory readability and downstream sanity
    checks.

- `tests/headless_cli_smoke/*`
  - Smoke test for argument parsing and JSON string formatting.

- `tests/manager_rva_scan_smoke/*`
  - Synthetic PE/memory smoke test proving the scanner discovers the live slot
    RVA instead of reusing legacy constants.

## Important Finding

The MSID/GameObject chain works, but `Il2CppClass` field layout is not fully
trusted yet for the current protected/mutated build.

Current GameObject klass:

- `klass = 0x256DCD03FF0`
- Runtime class name read fails.
- Header probe currently reports `UnityEngine` as best name, which is likely a
namespace-only false positive.
- `fieldsPtr` from the selected profile appears suspicious for GameObject.

This means current reliable exports are:

- module bases and RVAs,
- MSID slot and count,
- native/managed/klass object samples,
- old-offset candidate pointer validation.

The next layer is to add a runtime `Il2CppClass` layout auto-prober before
trusting field names/offsets for game-specific classes.

## Offset.h-like Export Strategy

The old `Offset.h` mixes three categories:

1. Runtime module bases:
   - `UnityPlayer`
   - `GameAssembly`
   - `NarakaBladepoint`

2. Module RVAs:
   - `m_CharacterManager`
   - `m_UserDataManager`
   - `m_EntityManager`
   - `m_GlobalTime`
   - `m_BuffManager`

3. Instance field offsets:
   - `Character.AllAliveCharList`
   - `ActorModel.Transform`
   - `ActorModel.PropertyData`
   - `ActorModel.ProPerty.PlayerName`

The safe first export should emit:

- confirmed current module bases,
- confirmed MSID slot RVA,
- GameObject sample addresses,
- current fixed Unity native offsets used by the browser,
- legacy RVA candidates with live pointer-read validation,
- placeholders for manager RVAs that are not yet proven.

Do not mark legacy manager RVAs as “confirmed” unless the headless probe proves
the pointer and downstream structure are sane.

## Current Live RVA Discovery Notes

The old `Offset.h` files are reference material only. Do not copy their RVAs.
The scanner reads current `GameAssembly_Super.dll` sections and emits live
candidates under `Offset::CurrentDiscovered`.

Generated header policy:

- `Offset::CurrentDiscovered::<name>` is non-zero only when the candidate is
  confirmed by current-process memory validation.
- `Offset::CurrentDiscovered::<name>_BestCandidate` keeps the highest-scored
  candidate even when it is not confirmed, for manual follow-up.

Latest observed live candidates:

- `m_CharacterManager`: `0x35E25C0`, confirmed by
  `aliveCharacters count=3`.
- `m_CharactorSync`: `0x35E2628`, confirmed by `shrtt ~= 0.0167` and proximity
  to `m_CharacterManager`.
- `m_EntityManager`: candidate family around `0x35DFB60` through
  `0x35EA5E0`, confirmed by entity list counts around 598-600. Prefer the JSON
  evidence and entity type samples over a single hardcoded value.
- `m_UserDataManager`: candidates include `0x35CE498`, `0x3ABBBF0`, and
  `0x3ABBC10`; both `+0xB8` and `+0xC0` static-field style chains are scanned.
- `m_GlobalTime`: not confirmed yet. The scanner may find pointer-shaped
  candidates, but the header keeps the confirmed RVA as `0` until a short
  two-read probe observes a positive time delta.

## Verification Commands

```powershell
msbuild tests\headless_cli_smoke\HeadlessCliSmoke.vcxproj /p:Configuration=Release /p:Platform=x64
tests\headless_cli_smoke\bin\x64\Release\HeadlessCliSmoke.exe

msbuild tests\manager_rva_scan_smoke\ManagerRvaScanSmoke.vcxproj /p:Configuration=Release /p:Platform=x64
tests\manager_rva_scan_smoke\bin\x64\Release\ManagerRvaScanSmoke.exe

msbuild App\ExternalResolveConsole.sln /p:Configuration=Release /p:Platform=x64
App\x64\Release\ExternalResolveConsole.exe --help
App\x64\Release\ExternalResolveConsole.exe --headless --target NarakaBladepoint.exe --mode dma --scan gameobjects --limit 32 --out bin\Release\headless_probe.json --log bin\Release\headless_probe.log
App\x64\Release\ExternalResolveConsole.exe --headless --target NarakaBladepoint.exe --mode dma --scan gameobjects --limit 64 --scan-managers --manager-scan-mb 96 --out bin\Release\headless_probe.json --log bin\Release\headless_probe.log --offset-header bin\Release\OffsetRuntime.h
```
