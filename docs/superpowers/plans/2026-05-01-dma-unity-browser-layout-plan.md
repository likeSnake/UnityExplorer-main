# DMA Unity Browser Layout Implementation Plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** Build a DMA-first Unity memory browser that can inspect Unity/IL2CPP objects and fields accurately across Unity versions while keeping WinAPI as a local-debug fallback.

**Architecture:** Treat the second PC as the only analysis/runtime host: it owns MemProcFS/Metick, module enumeration, metadata export, layout detection, object browsing, and ImGui rendering. The target PC only runs the Unity game. Runtime data should flow through one session object: DMA attach -> module map -> Unity/IL2CPP metadata snapshot -> versioned layout profile -> object/field inspector.

**Tech Stack:** C++20/MSVC, er2 header-only core, MemProcFS/Metick DMA accessor, ImGui DX11, UnityHeaders-derived IL2CPP layout profiles, existing metadataRegistration/fieldOffsets parser.

---

## Operating Model

The right model for your two-PC setup is:

- Host PC: runs Unity game only.
- Analysis PC: runs `UnityExplorer.exe`, owns DMA hardware, reads target process memory through MemProcFS/Metick, and never depends on `FindWindow`, local Unity window classes, or local WinAPI process handles.
- WinAPI mode remains useful for quick same-machine debugging, smoke tests, and comparing parser behavior, but it must not be the default mental model for the browser.

This means `ExternalResolveConsole.exe` can stay a WinAPI smoke/demo program, while `App/UnityExplorer` becomes the real DMA browser.

## File Structure

- Modify: `App/UnityExplorer/main.cpp`
  - Keep UI and existing workflows, but route initialization and Inspector calls through DMA-first services.
  - Eventually split large logic out of this file after behavior is stable.

- Modify: `App/UnityExplorer/MetickAdapter.hpp`
  - Add small diagnostics and read policy knobs only if needed.
  - Avoid putting Unity parsing logic here.

- Create: `include/er2/unity2/init/dma_session.hpp`
  - DMA-oriented context setup helper.
  - Stores process name, pid, module map, UnityPlayer, GameAssembly, backend runtime, read health, and last error.

- Create: `include/er2/unity2/metadata/unity_version.hpp`
  - Extract Unity version from safe sources: module strings, asset/globalgamemanagers path if available later, metadata hints, or user override.
  - Keep detection optional because some games strip strings.

- Create: `include/er2/unity2/metadata/il2cpp_layout_profile.hpp`
  - Compact layout profile derived from Il2CppInspectorRedux UnityHeaders.
  - Contains offsets/sizes for `Il2CppClass`, `FieldInfo`, `MethodInfo`, `Il2CppType`, `Il2CppMetadataRegistration`.

- Create: `include/er2/unity2/metadata/il2cpp_layout_resolver.hpp`
  - Chooses layout profile by metadata version + Unity version range.
  - Validates chosen profile against runtime anchors before exposing it.

- Create: `include/er2/unity2/inspect/field_model.hpp`
  - Neutral field model for UI: type name, field name, offset, static flag, source, confidence, readable value preview.

- Create: `include/er2/unity2/inspect/class_inspector.hpp`
  - Inspector API that prefers metadata dynamic offsets and uses layout profile only for runtime-only details.

- Create: `tests/dma_layout_profile_smoke/`
  - Offline/WinAPI-compatible tests for profile selection and field model behavior.
  - No real DMA hardware required for pure resolver tests.

## Phase 1: Make DMA Session the First-Class Init Path

### Task 1: Add DMA Session Model

**Files:**
- Create: `include/er2/unity2/init/dma_session.hpp`
- Modify: `App/UnityExplorer/main.cpp`

- [ ] Define `DmaUnitySession` with `pid`, `processName`, `unityPlayer`, `gameAssembly`, `runtime`, `moduleCount`, `canReadUnityPlayerPe`, `canReadGameAssemblyPe`, `lastError`.

- [ ] Add a helper that converts `MetickAdapter::EnumerateModules()` into `er2::ModuleInfo` entries.

- [ ] Move the module-detection part of `InitDmaConnection()` into a reusable helper while preserving current behavior.

- [ ] Ensure DMA init does not call `er2::AutoInit()` and does not rely on `FindUnityWndClassPids()`.

- [ ] Verification:
  - Build `App/UnityExplorer.sln`.
  - On analysis PC, open UnityExplorer, select DMA, initialize target process.
  - Expected log sequence: target PID -> module count -> UnityPlayer base -> GameAssembly base if IL2CPP -> PE header read OK.

### Task 2: Add Read Health Diagnostics

**Files:**
- Modify: `include/er2/unity2/init/dma_session.hpp`
- Modify: `App/UnityExplorer/main.cpp`

- [ ] Add `ProbeModulePeHeader(mem, module)` helper.

- [ ] Add `ProbePointerRead(mem, address)` helper for known anchors.

- [ ] Show a compact session panel in UnityExplorer:
  - process name
  - pid
  - runtime
  - UnityPlayer base/size
  - GameAssembly base/size
  - PE read status
  - metadata status

- [ ] Verification:
  - Bad target name reports "process not found".
  - Good target with stale CR3 reports module found but PE read failed.
  - Good CR3 reports PE read OK.

## Phase 2: Add UnityHeaders-Derived Layout Profiles

### Task 3: Extract Compact Profile Table

**Files:**
- Create: `include/er2/unity2/metadata/il2cpp_layout_profile.hpp`

- [ ] Do not vendor every UnityHeaders `.h` file into runtime include path.

- [ ] Manually extract the minimum profile fields needed first:
  - metadata version range
  - Unity version range label
  - `Il2CppMetadataRegistration.typesCount/types/fieldOffsetsCount/fieldOffsets/typeDefinitionsSizes`
  - `Il2CppClass.name/namespaze/parent/fields/methods/static_fields/field_count/method_count`
  - `FieldInfo.name/type/parent/offset`
  - `MethodInfo.name/klass/return_type/parameters`
  - `Il2CppType.attrs` if used

- [ ] Start with profiles for:
  - metadata `24.3/24.4/24.5` for Unity 2019.3/2019.4/2020.1
  - metadata `27.1/27.2` for Unity 2020.2/2020.3/2021.1
  - metadata `29/29.1` for Unity 2021.2/2021.3
  - metadata `31` for Unity 2022.3.33+
  - metadata `35/38/39` for Unity 6000.x

- [ ] Add source comments pointing to the corresponding UnityHeaders filename, not copied giant structs.

- [ ] Verification:
  - Compile-only test that profile table has unique metadata/version ranges.
  - Confirm no old fixed offset is silently used when profile selection fails.

### Task 4: Implement Profile Resolver

**Files:**
- Create: `include/er2/unity2/metadata/unity_version.hpp`
- Create: `include/er2/unity2/metadata/il2cpp_layout_resolver.hpp`
- Modify: `include/er2/unity2/init/field_offset.hpp`

- [ ] Parse metadata version from existing `MetadataHeaderFields.version`.

- [ ] Add optional Unity version string detection.

- [ ] Resolve profile by:
  - exact metadata version first
  - Unity version range if available
  - newest compatible profile if Unity version is unknown

- [ ] Add runtime validation:
  - `Il2CppClass.name` points to a readable C string for known class pointers.
  - `Il2CppMetadataRegistration.fieldOffsets` points to readable offset arrays.
  - `UnityEngine.Object::m_CachedPtr` offset can be resolved dynamically.

- [ ] Expose result as `{profile, confidence, reason}`.

- [ ] Verification:
  - Resolver unit test for each supported metadata version.
  - Runtime log shows selected profile and reason.

## Phase 3: Make Field Inspector Metadata-First

### Task 5: Add Neutral Field Model

**Files:**
- Create: `include/er2/unity2/inspect/field_model.hpp`
- Create: `include/er2/unity2/inspect/class_inspector.hpp`
- Modify: `App/UnityExplorer/main.cpp`

- [ ] Define `FieldSource`:
  - `MetadataFieldOffsets`
  - `RuntimeFieldInfoProfile`
  - `FallbackFixedOffsets`
  - `Unknown`

- [ ] Define `FieldConfidence`:
  - `High`
  - `Medium`
  - `Low`
  - `Invalid`

- [ ] Build field list from metadata first:
  - type definition -> field definition name/type token
  - `metadataRegistration.fieldOffsets[typeIndex][fieldSlot]`
  - parent traversal for inherited fields

- [ ] Use runtime `FieldInfo` only for extra data that metadata cannot provide cheaply.

- [ ] Mark old `EnumerateClassFields()` path as fallback/experimental in the UI.

- [ ] Verification:
  - `UnityEngine.Object::m_CachedPtr` resolves through `TryGetFieldOffset`.
  - UI shows source and confidence for each field.

### Task 6: Read Field Values Safely Over DMA

**Files:**
- Modify: `include/er2/unity2/inspect/field_model.hpp`
- Modify: `include/er2/unity2/inspect/class_inspector.hpp`
- Modify: `App/UnityExplorer/main.cpp`

- [ ] Add value preview readers:
  - pointer
  - bool/int/float
  - Unity string
  - object reference pointer

- [ ] Use bounded reads and canonical pointer checks.

- [ ] Use scatter reads for field previews when inspecting many fields.

- [ ] Do not auto-write memory from Inspector views.

- [ ] Verification:
  - Inspect one known GameObject/component.
  - Bad/unreadable field displays unreadable, not garbage.
  - DMA no-cache mode can be toggled for a stale value.

## Phase 4: Object Browser Pipeline

### Task 7: Stabilize Object Sources

**Files:**
- Modify: `App/UnityExplorer/main.cpp`
- Modify: `include/er2/unity2/msid/enumerate_objects.hpp`
- Modify: `include/er2/unity2/gom/gom_scan.hpp`

- [ ] Treat GOM scan, MSID scan, and `FindObjectsOfTypeAll` as separate object sources.

- [ ] Store browser nodes as `{native, managed, typeName, name, source, readStatus}`.

- [ ] Add source filters in UI.

- [ ] Verification:
  - GOM source works without MSID.
  - MSID source failure does not kill GOM browsing.
  - Object rows show which source found them.

### Task 8: Add Snapshot/Refresh Policy

**Files:**
- Modify: `App/UnityExplorer/main.cpp`
- Create: `include/er2/unity2/inspect/object_snapshot.hpp`

- [ ] Add manual snapshot refresh button.

- [ ] Add optional timed refresh with a conservative interval.

- [ ] Keep selected object stable by address, but revalidate before reading fields.

- [ ] Verification:
  - Refresh does not freeze UI for normal object counts.
  - Deleted/unreadable objects are marked stale.

## Phase 5: Validation and Delivery

### Task 9: Build and Smoke Tests

**Files:**
- Modify: `tests/winapi_smoke/main.cpp`
- Create: `tests/dma_layout_profile_smoke/main.cpp`
- Modify or create corresponding `.vcxproj` files as needed.

- [ ] Build:
  - `msbuild App\UnityExplorer.sln /p:Configuration=Release /p:Platform=x64`
  - `msbuild tests\winapi_smoke\Er2WinApiSmoke.vcxproj /p:Configuration=Release /p:Platform=x64`

- [ ] Add offline resolver tests that do not require the game.

- [ ] Add a manual DMA checklist:
  - target process visible
  - CR3 fixed
  - UnityPlayer PE readable
  - GameAssembly PE readable for IL2CPP
  - metadata exported
  - metadataRegistration found
  - fieldOffsets found
  - `UnityEngine.Object::m_CachedPtr` resolved

### Task 10: Documentation

**Files:**
- Create: `docs/dma-unity-browser.md`
- Modify: `README.md`

- [ ] Document the two-PC workflow.

- [ ] Document the difference between:
  - `ExternalResolveConsole.exe`: local WinAPI smoke/demo
  - `UnityExplorer.exe`: DMA browser

- [ ] Document where UnityHeaders contributes:
  - versioned IL2CPP runtime struct profiles
  - profile validation
  - fallback runtime reflection

- [ ] Document what UnityHeaders does not solve:
  - missing CR3
  - unreadable DMA pages
  - stripped/obfuscated metadata
  - game-specific encrypted metadata
  - native UnityEngine object layout quirks outside IL2CPP metadata

## Recommended Execution Order

1. Phase 1 first, because DMA session correctness decides everything else.
2. Phase 3 next, because dynamic metadata field offsets already exist and will immediately improve Inspector accuracy.
3. Phase 2 after that, to replace old fixed runtime offsets with version profiles.
4. Phase 4 once the Inspector can trust its field data.
5. Phase 5 continuously, not only at the end.

## Acceptance Criteria

- In DMA mode, the browser never calls local WinAPI auto process discovery.
- A target process on the host PC can be attached by name from the analysis PC.
- UnityPlayer and GameAssembly module bases are resolved through DMA module enumeration.
- Metadata export and `metadataRegistration.fieldOffsets` work in DMA mode.
- Field Inspector shows field source/confidence.
- Old fixed `Il2CppClass` offsets are only fallback and are visibly labeled as such.
- The UI can inspect at least one known object/component without crashing when fields are unreadable.
