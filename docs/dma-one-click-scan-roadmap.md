# DMA One-Click Offset Scan Roadmap

## Purpose

This document records the current successful DMA workflow and the next
implementation plan for turning the probe into a one-click scanner.

The goal is not to copy old offsets. The scanner must discover current runtime
RVAs from the target process through DMA, validate them with current memory
evidence, and export an `Offset.h`-like report that can be reused by the
browser and by external research code.

## Current Successful Workflow

### 1. Use DMA as the primary backend

The probe starts from the local DMA backend and fixes CR3/DTB before doing any
Unity analysis. This is important because the target process may not be usable
through normal WinAPI reads, and the user wants the secondary PC to drive the
browser/debugger without manual interaction on the host PC.

Current known-good command:

```powershell
App\x64\Release\ExternalResolveConsole.exe --headless --target NarakaBladepoint.exe --mode dma --scan gameobjects --limit 64 --scan-managers --manager-scan-mb 96 --out bin\Release\headless_probe.json --log bin\Release\headless_probe.log --offset-header bin\Release\OffsetRuntime.h
```

### 2. Detect the real loaded modules dynamically

The target build may load renamed or protected module variants, for example:

- `GameAssembly_Super.dll`
- `GameAssembly_Super_IBT.dll`
- `GameAssembly.dll`
- `UnityPlayer_LVB.dll`
- `UnityPlayer_LVB_IBT.dll`
- `UnityPlayer.dll`

The working rule is:

- first loaded module whose name starts with `UnityPlayer` and ends with
  `.dll` is treated as the Unity engine module;
- first loaded module whose name starts with `GameAssembly` and ends with
  `.dll` is treated as the IL2CPP logic module.

Latest observed runtime:

```cpp
UnityPlayer_LVB.dll   @ 0x7FFE23A70000
GameAssembly_Super.dll @ 0x7FFE09D80000
```

### 3. Use Unity 2019.4 LTS layout as a starting profile

The current target reports Unity `2019.4.41` from `UnityPlayer.dll`
properties. The active profile is based on the closest available
Il2CppInspectorRedux UnityHeaders profile:

```text
unity-2019.4-lts-24.5
source: 24.5-2019.4.21-2019.4.24.h
```

This is useful for native Unity structure shape, but it is not enough to trust
all IL2CPP class fields in a protected or modified build.

### 4. Do not block on metadata

The metadata pointer scan did not find a clean `0xFAB11BAF` metadata blob in
the expected way. This can happen when metadata is encrypted, relocated,
reconstructed late, or hidden by custom loader logic.

The current successful path therefore avoids making metadata mandatory:

- MSID scan is used to enumerate Unity native objects.
- GameObject filtering is based on runtime object shape.
- Manager/global RVAs are discovered by scanning `GameAssembly` sections for
  pointer slots and validating downstream chains.

### 5. Scan current module data, then validate chain shape

The scanner walks current `GameAssembly` memory and treats every candidate as
untrusted until it passes downstream checks.

The common shape is:

```text
GameAssembly .data/.rdata slot
  -> TypeInfo / Klass-like pointer
  -> static_fields
  -> manager pointer / list / dictionary / scalar
```

This is why many manager RVAs look close to each other and why a generic
scanner can find valid pointers before it knows the exact semantic name.

### 6. Export evidence, not guesses

The generated header uses two levels:

- `Offset::CurrentDiscovered::<name>` is non-zero only when current-process
  memory validation confirms the candidate.
- `Offset::CurrentDiscovered::<name>_BestCandidate` records the highest-scored
  candidate for manual follow-up when confirmation is still weak.

Latest strong matches:

```cpp
namespace Offset::CurrentDiscovered {
inline constexpr std::uint64_t m_CharacterManager = 0x35E25C0ull;
inline constexpr std::uint64_t m_CharactorSync    = 0x35E2628ull;
inline constexpr std::uint64_t m_EntityManager    = 0x35DFB60ull;
inline constexpr std::uint64_t m_UserDataManager  = 0x35872C8ull;
}
```

## Cross-Check With Manual Findings

The user's manually discovered current RVAs, normalized to ASCII labels for
terminal-safe documentation:

```cpp
WorldBaseRva         = 0x35E25C0
BuildingSkillBaseRva = 0x36071C0
ItemBaseRva          = 0x35EBBA8
WorldTimeBaseRva     = 0x35EBAF0
PingBaseRva          = 0x35E2628
BuffManagerRva       = 0x35E1250
WindowSizeBaseRva    = 0x35F95C0
```

Current scanner comparison:

| RVA | Manual meaning | Scanner result | Current interpretation |
| --- | --- | --- | --- |
| `0x35E25C0` | world base | confirmed as `m_CharacterManager` | strong match |
| `0x35E2628` | ping base | confirmed as `m_CharactorSync` | strong match |
| `0x35EBBA8` | item base | confirmed as CharacterManager-like, count `37` | likely valid but misclassified by generic manager validator |
| `0x35EBAF0` | world time base | GlobalTime candidate, value about `14608.7`, not delta-confirmed | likely valid; needs static time validator |
| `0x36071C0` | building skill base | not covered yet | needs dedicated validator |
| `0x35E1250` | BuffManager | not covered yet | needs dedicated validator |
| `0x35F95C0` | window size | not covered yet | needs dedicated validator |

The important lesson is that the generic scanner is already finding the same
region and several exact RVAs. The next improvement is semantic validation, not
more blind scanning.

## Why This Worked

The successful path combined four ideas:

1. Runtime module discovery instead of hardcoded module names.
2. DMA memory reads as the source of truth.
3. Metadata-independent scanning when protected metadata is not available.
4. Conservative export policy: confirmed values are separated from candidates.

This makes the workflow useful for a protected CTF build where Unity and
GameAssembly module names are randomized or wrapped, metadata is not reliable,
and class names may be unreadable.

## One-Click Scan Target Design

The future one-click mode should run a fixed pipeline and emit a complete
runtime report:

```text
init DMA
  -> fix CR3
  -> attach target process
  -> enumerate modules
  -> detect UnityPlayer/GameAssembly
  -> select Unity layout profile
  -> verify PE access
  -> scan MSID slot
  -> enumerate object samples
  -> scan manager/static slots
  -> run semantic validators
  -> export JSON/log/header
  -> print confidence summary
```

Recommended CLI:

```powershell
App\x64\Release\ExternalResolveConsole.exe --headless --target NarakaBladepoint.exe --mode dma --one-click-offset-scan --out bin\Release\headless_probe.json --log bin\Release\headless_probe.log --offset-header bin\Release\OffsetRuntime.h
```

The current `--scan-managers` mode can become an internal stage of
`--one-click-offset-scan`.

## Next Implementation Plan

### Phase 1: Candidate validation mode

Add a headless option that accepts exact RVAs and prints validation evidence:

```powershell
--validate-rvas 0x35E25C0,0x36071C0,0x35EBBA8,0x35EBAF0,0x35E2628,0x35E1250,0x35F95C0
```

Output should include:

- slot VA;
- first pointer value;
- static fields pointer;
- candidate chain kind;
- readable downstream fields;
- count/value samples;
- confidence score;
- reason for failure if rejected.

This gives a fast feedback loop for manual discoveries and prevents repeatedly
asking the user to click UI items.

### Phase 2: Dedicated semantic validators

Add separate validators for the known target categories:

- Character/world manager:
  - alive character list count;
  - list/array shape;
  - sample actor pointers.
- Item/entity manager:
  - item/entity list or dictionary count;
  - sample entry pointers;
  - optional type/name/object shape when readable.
- Global/world time:
  - scalar float/double plausibility;
  - stable non-zero value;
  - optional multi-sample delta when scene is not paused.
- CharactorSync/ping:
  - small positive float;
  - proximity to CharacterManager family;
  - repeated-read stability.
- Building skill / UserData skill:
  - battle data pointer;
  - skill data pointer;
  - dictionary/list shape.
- BuffManager:
  - manager singleton chain;
  - buff dictionary/list count;
  - sample buff entry shape.
- Window size:
  - width/height integer pair or vector shape;
  - values must be plausible screen dimensions.

Each validator should return structured evidence instead of only `true/false`.

### Phase 3: Semantic classification

Replace generic labels like `CharacterManager-like` with a ranked
classification result:

```json
{
  "rva": "0x35EBBA8",
  "bestKind": "ItemManager",
  "confidence": 0.82,
  "alternatives": [
    { "kind": "CharacterManagerLike", "confidence": 0.55 }
  ],
  "evidence": {
    "count": 37,
    "chain": "slot->klass->static_fields->list"
  }
}
```

This prevents valid item/global/buff bases from being exported under the wrong
name just because their static-field chain shape resembles another manager.

### Phase 4: One-click export contract

Generate a stable header namespace:

```cpp
namespace Offset::CurrentDiscovered {
inline constexpr std::uint64_t WorldBase = 0x35E25C0ull;
inline constexpr std::uint64_t BuildingSkillBase = 0x36071C0ull;
inline constexpr std::uint64_t ItemBase = 0x35EBBA8ull;
inline constexpr std::uint64_t WorldTimeBase = 0x35EBAF0ull;
inline constexpr std::uint64_t PingBase = 0x35E2628ull;
inline constexpr std::uint64_t BuffManager = 0x35E1250ull;
inline constexpr std::uint64_t WindowSizeBase = 0x35F95C0ull;
}
```

Only emit non-zero values when the semantic validator confirms the current
process evidence. Otherwise emit:

```cpp
inline constexpr std::uint64_t ItemBase = 0x0ull;
inline constexpr std::uint64_t ItemBase_BestCandidate = 0x35EBBA8ull;
```

### Phase 5: Regression and smoke tests

Add synthetic memory tests for each validator:

- exact RVA should be found from a fake PE image;
- bad pointer should be rejected with a useful reason;
- wrong semantic shape should not be exported under the wrong name;
- best candidate should remain available when confirmation is weak.

The existing `manager_rva_scan_smoke` test should be expanded or split into
small validator-specific tests.

## Acceptance Criteria

One-click scan is considered successful when:

1. A single command attaches through DMA and completes without ImGui clicks.
2. The output header contains confirmed current RVAs for at least:
   - world base;
   - item base;
   - world time base;
   - ping base.
3. Unconfirmed categories are exported as zero plus best-candidate evidence.
4. JSON contains enough evidence to explain every accepted/rejected RVA.
5. The scanner does not use old RVAs as source data.
6. Synthetic smoke tests pass for all new validator categories.

## Risk Notes

- Protected builds may move or reconstruct static fields after login/scene
  load. The scanner should report target state and timestamps.
- Time values may not change during a short probe if the scene is paused,
  cached, or DMA reads are stale. Global time needs both static plausibility and
  optional delta confirmation.
- Metadata and class names are useful when available, but the one-click path
  must still work when they are unreadable.
- Similar manager chains can produce false semantic labels. Prefer multiple
  independent evidence points over a single count or pointer shape.
