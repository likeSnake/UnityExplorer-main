#pragma once
// ============================================================================
// dma_inject.hpp - DMA-based code injection (no process handle, no remote
//                  thread, no VirtualAllocEx).
// ============================================================================
// Implements the PCILeech "UMD" technique over the DMA memory accessor:
//   1. Locate an IAT thunk in the target's main executable for a frequently
//      called import (e.g. kernel32!Sleep, user32!GetMessageW).
//   2. Locate a r-x "code cave" (unused tail of an executable section) and a
//      rw- "data cave" (unused tail of a writable section) in the same module.
//   3. Write shellcode into the code cave and its config (function addresses +
//      payload) into the data cave.
//   4. Rewrite the IAT thunk to point at the shellcode.
//   5. When the target next calls the hooked import, the shellcode runs: it
//      calls LoadLibraryA(dllPath) to load our dumper DLL, then jumps back to
//      the original function.
//   6. Restore the IAT thunk and clean the caves.
//
// Everything is done through VMMDLL_MemWrite (DMA write) - no OS API call
// into the target, which is the anti-cheat-relevant advantage of DMA.
//
// Prerequisite: the FPGA firmware must support DMA writes (PCILeech default
// firmware does; verify with a probe write before relying on injection).
// ============================================================================

#include <Windows.h>
#include <psapi.h>

#ifndef NOMINMAX
#define NOMINMAX
#endif

#include <cstdint>
#include <cstring>
#include <sstream>
#include <string>
#include <vector>

#include "../../os/dma/dma_memory_accessor.hpp"
#include "../metadata/pe.hpp"
#include "../../../../deps/Memory/libs/vmmdll.h"

namespace er2 {

// ---------------------------------------------------------------------------
// Options / result
// ---------------------------------------------------------------------------

struct DmaInjectOptions {
  std::uint32_t pid = 0;
  std::string dllPath;           // absolute path of DLL to load
  std::string hookModule;        // import module, e.g. "kernel32.dll"
  std::string hookFunction;      // import function, e.g. "Sleep"
  std::uint32_t waitMs = 15000;  // how long to wait for the shellcode to run
  std::uint32_t pollMs = 25;     // poll interval while waiting
  bool testOnly = false;         // skip LoadLibraryA, only verify execution path
};

struct DmaInjectResult {
  bool ok = false;
  std::string error;
  std::uintptr_t iatThunk = 0;
  std::uintptr_t iatFunction = 0;
  std::uintptr_t codeCave = 0;
  std::uintptr_t dataCave = 0;
  std::uintptr_t loadLibraryAddr = 0;
  bool directHook = false; // true = inline hook at iatFunction address
  bool executed = false;   // shellcode actually ran (status was set)
  std::uintptr_t loadLibraryResult = 0; // HMODULE returned by LoadLibraryA (0 if failed/testOnly)
  bool restored = false;   // IAT restored after execution
  std::string moduleName;  // target module used for the hook
};

// ---------------------------------------------------------------------------
// Win64 shellcode: calls LoadLibraryA(dllPath), sets status, then jumps to the
// original function. Two variants:
//   - IAT mode: original untouched -> jmp [data+0x10].
//   - Inline mode: function head was overwritten; restore saved original bytes
//     (data+0x30, 16 bytes) back to iatFunction (data+0x18), then jmp.
// Unified data-cave layout (must match the injector's build step):
//   +0x00 QWORD dllPathPtr
//   +0x08 QWORD LoadLibraryA
//   +0x10 QWORD jmp target (original function)
//   +0x18 QWORD restore target (original function, inline only)
//   +0x20 QWORD status (0 = not run, 1 = ran)
//   +0x28 QWORD (reserved)
//   +0x30 BYTE[16] original bytes (inline restore)
//   +0x40 CHAR[] dll path
// ---------------------------------------------------------------------------
inline std::vector<std::uint8_t> BuildWin64LoadLibraryShellcode(
    std::uintptr_t dataCave, bool inlineMode) {
  std::vector<std::uint8_t> sc(0x60, 0x00);
  auto putQ = [&](std::size_t off, std::uint64_t v) {
    std::memcpy(sc.data() + off, &v, 8);
  };
  // mov rbx, dataCave          (rbx is non-volatile; survives the call)
  sc[0x00] = 0x48; sc[0x01] = 0xBB;
  putQ(0x02, dataCave);
  // mov rcx, [rbx]             ; rcx = dllPathPtr
  sc[0x0A] = 0x48; sc[0x0B] = 0x8B; sc[0x0C] = 0x0B;
  // test qword ptr [rbx+0x28], 1  ; if testOnly, skip LoadLibraryA
  sc[0x0D] = 0x48; sc[0x0E] = 0x83; sc[0x0F] = 0x7B;
  sc[0x10] = 0x28; sc[0x11] = 0x00;
  sc[0x12] = 0x74; sc[0x13] = 0x08;   // jz +8 (skip call)
  // call qword ptr [rbx+8]      ; LoadLibraryA(path)
  sc[0x14] = 0xFF; sc[0x15] = 0x53; sc[0x16] = 0x08;
  // mov [rbx+0x20], rax         ; status = LoadLibraryA result (HMODULE)
  sc[0x17] = 0x48; sc[0x18] = 0x89; sc[0x19] = 0x43;
  sc[0x1A] = 0x20;
  sc[0x1B] = 0xEB; sc[0x1C] = 0x08;  // jmp +8 (skip testOnly status write)
  // testOnly path: mov qword ptr [rbx+0x20], 1
  sc[0x1D] = 0x48; sc[0x1E] = 0xC7; sc[0x1F] = 0x43;
  sc[0x20] = 0x20; sc[0x21] = 0x01; sc[0x22] = 0x00;
  sc[0x23] = 0x00; sc[0x24] = 0x00;
  if (!inlineMode) {
    // mov rcx, [rbx+0x10]; jmp rcx
    sc[0x25] = 0x48; sc[0x26] = 0x8B; sc[0x27] = 0x4B; sc[0x28] = 0x10;
    sc[0x29] = 0xFF; sc[0x2A] = 0xE1;
    return sc;
  }
  // inline mode: restore original bytes then jump back.
  // mov rdx, [rbx+0x18]        ; rdx = restore target (iatFunction)
  sc[0x25] = 0x48; sc[0x26] = 0x8B; sc[0x27] = 0x53; sc[0x28] = 0x18;
  // lea r8, [rbx+0x30]         ; r8 = source (orig bytes)
  sc[0x29] = 0x4C; sc[0x2A] = 0x8D; sc[0x2B] = 0x43; sc[0x2C] = 0x30;
  // mov rax, [r8]; mov [rdx], rax
  sc[0x2D] = 0x49; sc[0x2E] = 0x8B; sc[0x2F] = 0x00;
  sc[0x30] = 0x48; sc[0x31] = 0x89; sc[0x32] = 0x02;
  // mov rax, [r8+8]; mov [rdx+8], rax
  sc[0x33] = 0x49; sc[0x34] = 0x8B; sc[0x35] = 0x40; sc[0x36] = 0x08;
  sc[0x37] = 0x48; sc[0x38] = 0x89; sc[0x39] = 0x42; sc[0x3A] = 0x08;
  // mov rcx, [rbx+0x10]; jmp rcx   ; jump to original (now restored)
  sc[0x3B] = 0x48; sc[0x3C] = 0x8B; sc[0x3D] = 0x4B; sc[0x3E] = 0x10;
  sc[0x3F] = 0xFF; sc[0x40] = 0xE1;
  return sc;
}

// ---------------------------------------------------------------------------
// Data cave layout:
//   +0x00 QWORD dllPathPtr
//   +0x08 QWORD LoadLibraryA addr
//   +0x10 QWORD jmp target (original function)
//   +0x18 QWORD restore target (original function, inline only)
//   +0x20 QWORD status (0 = not run, 1 = ran)
//   +0x28 QWORD testOnly flag (1 = skip LoadLibraryA)
//   +0x30 BYTE[16] original bytes (inline restore)
//   +0x40 CHAR[] dll path
// ---------------------------------------------------------------------------
inline constexpr std::size_t kDmaInjectDataHeader = 0x20;

// ---------------------------------------------------------------------------
// Find an IAT thunk for module!function in the target's main executable.
// ---------------------------------------------------------------------------
inline bool DmaFindIatThunk(const IMemoryAccessor &mem, std::uint32_t pid,
                            const std::string &moduleName,
                            const std::string &hookModule,
                            const std::string &hookFunction,
                            std::uintptr_t &outThunk,
                            std::uintptr_t &outFunction) {
  outThunk = 0;
  outFunction = 0;
  const auto *dma = dynamic_cast<const DMAMemoryAccessor *>(&mem);
  if (!dma)
    return false;
  VMM_HANDLE h = dma->GetHandle();
  if (!h)
    return false;

  VMMDLL_WIN_THUNKINFO_IAT thunk = {};
  if (!VMMDLL_WinGetThunkInfoIATU(h, pid, moduleName.c_str(),
                                  hookModule.c_str(), hookFunction.c_str(),
                                  &thunk)) {
    return false;
  }
  if (!thunk.fValid || thunk.f32)
    return false;
  outThunk = static_cast<std::uintptr_t>(thunk.vaThunk);
  outFunction = static_cast<std::uintptr_t>(thunk.vaFunction);
  return outThunk != 0 && outFunction != 0;
}

// ---------------------------------------------------------------------------
// Find code cave (r-x) and data cave (rw-) in a module, using our own PE
// section parser (ReadModuleSections) instead of VMMDLL_ProcessGetSectionsU
// which anti-cheat can break. moduleBase is the module's base address.
// ---------------------------------------------------------------------------
inline bool DmaFindCavesAtModule(const IMemoryAccessor &mem,
                                 std::uintptr_t moduleBase,
                                 std::uint32_t moduleSize,
                                 std::size_t codeNeed, std::size_t dataNeed,
                                 std::uintptr_t &outCodeCave,
                                 std::uintptr_t &outDataCave) {
  outCodeCave = 0;
  outDataCave = 0;
  if (!moduleBase || moduleSize == 0)
    return false;

  std::vector<ModuleSection> sections;
  std::uint32_t sizeOfImage = 0;
  if (!ReadModuleSections(mem, moduleBase, sizeOfImage, sections))
    return false;
  if (sizeOfImage && moduleSize > sizeOfImage)
    moduleSize = sizeOfImage;

  const std::uintptr_t moduleEnd = moduleBase + moduleSize;
  for (const auto &sec : sections) {
    const std::uint32_t vaddr = sec.rva;
    const std::uint32_t vsize = sec.size;
    if (!vsize)
      continue;
    const std::uint32_t rounded = (vsize + 0xFFFu) & ~0xFFFu;
    if (rounded < 0x1000)
      continue;
    const std::size_t slack = rounded - vsize;

    // Need to know characteristics; ReadModuleSections doesn't give them, so
    // infer from common names + read a page to test accessibility.
    const bool maybeExec = sec.name == ".text" || sec.name == "il2cpp" ||
                           sec.name == ".vmp0" || sec.name == ".vmp1" ||
                           sec.name == ".voltbl";
    const bool maybeWrite = sec.name == ".data" || sec.name == ".fptable" ||
                            sec.name == ".tls" || sec.name == "_RDATA";

    if (!outCodeCave && maybeExec && slack >= codeNeed) {
      const std::uintptr_t cave = moduleBase + vaddr + rounded - codeNeed;
      BYTE page[0x1000] = {0};
      if (mem.Read(cave & ~0xFFFull, page, sizeof(page)))
        outCodeCave = cave;
    }
    if (!outDataCave && maybeWrite && slack >= dataNeed) {
      const std::uintptr_t cave = moduleBase + vaddr + rounded - dataNeed;
      BYTE page[0x1000] = {0};
      if (mem.Read(cave & ~0xFFFull, page, sizeof(page)))
        outDataCave = cave;
    }
    if (outCodeCave && outDataCave)
      break;
  }
  return outCodeCave != 0 && outDataCave != 0;
}

// ---------------------------------------------------------------------------
// Main DMA injection routine.
// ---------------------------------------------------------------------------
inline bool RunDmaInject(const IMemoryAccessor &mem, DmaInjectOptions &options,
                         DmaInjectResult &result) {
  result = DmaInjectResult{};
  const auto *dma = dynamic_cast<const DMAMemoryAccessor *>(&mem);
  if (!dma) {
    result.error = "memory accessor is not a DMA accessor";
    return false;
  }
  VMM_HANDLE h = dma->GetHandle();
  if (!h) {
    result.error = "VMM handle unavailable";
    return false;
  }
  if (!options.pid) {
    result.error = "no pid specified";
    return false;
  }
  if (options.dllPath.empty()) {
    result.error = "no dll path specified";
    return false;
  }

  // Default hook: kernel32!Sleep (called extremely often).
  if (options.hookModule.empty())
    options.hookModule = "kernel32.dll";
  if (options.hookFunction.empty())
    options.hookFunction = "Sleep";

  // 1. Target module = main executable of the process.
  VMMDLL_PROCESS_INFORMATION pi = {};
  pi.magic = VMMDLL_PROCESS_INFORMATION_MAGIC;
  pi.wVersion = VMMDLL_PROCESS_INFORMATION_VERSION;
  SIZE_T cb = sizeof(pi);
  if (!VMMDLL_ProcessGetInformation(h, options.pid, &pi, &cb) ||
      !pi.szName[0]) {
    result.error = "failed to get process information";
    return false;
  }
  result.moduleName = pi.szName;
  const std::string modName = result.moduleName;

  // 2. IAT thunk. First try the IAT lookup (works on unprotected targets).
  //    On protected targets (IAT hidden) fall back to a direct inline hook:
  //    resolve the hookFunction as a real function address (e.g. Sleep) via
  //    VMMDLL_ProcessGetProcAddressU, or accept an explicit hex address.
  {
    std::string hookModuleCandidates[] = {modName, "UnityPlayer.dll",
                                          "GameAssembly.dll",
                                          "UnityPlayer_LVB.dll"};
    bool found = false;
    for (const auto &m : hookModuleCandidates) {
      if (m.empty())
        continue;
      if (DmaFindIatThunk(mem, options.pid, m, options.hookModule,
                          options.hookFunction, result.iatThunk,
                          result.iatFunction)) {
        result.moduleName = m;
        found = true;
        break;
      }
    }
    if (!found) {
      // Fallback 1: hookFunction is an absolute hex address.
      std::uintptr_t directTarget = 0;
      try {
        directTarget = static_cast<std::uintptr_t>(
            std::stoull(options.hookFunction, nullptr, 0));
      } catch (...) {
        directTarget = 0;
      }
      // Fallback 2: hookFunction is a function name -> resolve its real
      // address (kernel32!Sleep etc). This is the anti-cheat-robust path.
      if (!directTarget) {
        directTarget = static_cast<std::uintptr_t>(
            VMMDLL_ProcessGetProcAddressU(h, options.pid,
                                          options.hookModule.c_str(),
                                          options.hookFunction.c_str()));
      }
      if (directTarget && directTarget > 0x10000) {
        result.iatThunk = directTarget; // used as hook target address
        result.iatFunction = directTarget;
        result.directHook = true;
        result.moduleName = modName;
      } else {
        result.error = "could not find IAT thunk for " + options.hookModule +
                       "!" + options.hookFunction +
                       " and could not resolve a direct hook address "
                       "(anti-cheat may hide PEB/IAT)";
        return false;
      }
    }
  }

  // 3. Caves: use GameAssembly (known base) with our own PE parser; fall back
  //    to UnityPlayer. Never rely on VMMDLL_ProcessGetSectionsU (anti-cheat).
  const std::size_t codeNeed = 0x20;
  const std::size_t dataNeed =
      kDmaInjectDataHeader + options.dllPath.size() + 1 + 0x20;
  std::uintptr_t caveModuleBase = 0;
  std::uint32_t caveModuleSize = 0;
  std::string caveModuleName;
  if (g_ctx.gameAssembly.base && g_ctx.gameAssembly.size) {
    caveModuleBase = g_ctx.gameAssembly.base;
    caveModuleSize = g_ctx.gameAssembly.size;
    caveModuleName = "GameAssembly";
  } else if (g_ctx.unityPlayer.base && g_ctx.unityPlayer.size) {
    caveModuleBase = g_ctx.unityPlayer.base;
    caveModuleSize = g_ctx.unityPlayer.size;
    caveModuleName = "UnityPlayer";
  }
  if (!caveModuleBase ||
      !DmaFindCavesAtModule(mem, caveModuleBase, caveModuleSize, codeNeed,
                            dataNeed, result.codeCave, result.dataCave)) {
    result.error = "could not find code/data caves in " + caveModuleName;
    return false;
  }

  // 4. LoadLibraryA address.
  result.loadLibraryAddr = static_cast<std::uintptr_t>(
      VMMDLL_ProcessGetProcAddressU(h, options.pid, "kernel32.dll",
                                    "LoadLibraryA"));
  if (!result.loadLibraryAddr) {
    result.error = "could not resolve LoadLibraryA";
    return false;
  }

  // 5. Build shellcode + data cave contents.
  const std::vector<std::uint8_t> sc =
      BuildWin64LoadLibraryShellcode(result.dataCave, result.directHook);

  std::vector<std::uint8_t> data(dataNeed, 0);
  const std::uintptr_t dllPathPtr = result.dataCave + kDmaInjectDataHeader + 0x20;
  std::memcpy(data.data() + 0x00, &dllPathPtr, 8);
  std::memcpy(data.data() + 0x08, &result.loadLibraryAddr, 8);
  std::memcpy(data.data() + 0x10, &result.iatFunction, 8);   // jmp target
  std::memcpy(data.data() + 0x18, &result.iatFunction, 8);   // restore target
  // +0x20 status = 0 (not run yet)
  std::uint64_t testOnlyVal = options.testOnly ? 1 : 0;
  std::memcpy(data.data() + 0x28, &testOnlyVal, 8);
  std::memcpy(data.data() + kDmaInjectDataHeader + 0x20,
              options.dllPath.c_str(), options.dllPath.size() + 1);

  // 6. Write: data cave, code cave, then arm the hook.
  if (!VMMDLL_MemWrite(h, options.pid, result.dataCave, data.data(),
                       static_cast<DWORD>(data.size()))) {
    result.error = "DMA write of data cave failed (firmware write support?)";
    return false;
  }
  if (!VMMDLL_MemWrite(h, options.pid, result.codeCave,
                       const_cast<BYTE *>(sc.data()),
                       static_cast<DWORD>(sc.size()))) {
    result.error = "DMA write of code cave failed";
    return false;
  }

  if (result.directHook) {
    // ---- Inline hook at a function address (not an IAT slot) ----
    // Save the original 16 bytes, then overwrite the function head with:
    //   FF 25 00 00 00 00 <codeCave>   ; jmp qword ptr [rip+0] ; target
    // The code cave shellcode (which begins with mov rax,dataCave) must be
    // preceded by a 12-byte trampoline that does the same job. Simplest
    // robust approach: prepend a 6-byte 'jmp rel32' from the function head
    // into a second cave region that holds an absolute jmp to the shellcode.
    std::uint8_t orig[16] = {0};
    DWORD bytesRead = 0;
    if (!VMMDLL_MemReadEx(h, options.pid, result.iatFunction, orig, 16,
                          &bytesRead, 0) ||
        bytesRead != 16) {
      result.error = "failed to read original bytes for inline hook";
      return false;
    }
    // Save original bytes into the data cave (header + 0x30, 16 bytes).
    std::memcpy(data.data() + 0x30, orig, 16);
    if (!VMMDLL_MemWrite(h, options.pid, result.dataCave, data.data(),
                         static_cast<DWORD>(data.size()))) {
      result.error = "DMA write of data cave (with orig bytes) failed";
      return false;
    }

    // Trampoline: mov rax, codeCave ; jmp rax  (12 bytes)
    std::uint8_t tramp[16] = {0};
    tramp[0] = 0x48; tramp[1] = 0xB8;             // mov rax, imm64
    std::memcpy(tramp + 2, &result.codeCave, 8);
    tramp[10] = 0xFF; tramp[11] = 0xE0;           // jmp rax
    if (!VMMDLL_MemWrite(h, options.pid, result.iatFunction, tramp, 12)) {
      result.error = "DMA write of inline hook failed";
      return false;
    }
    // Wait for status (dataCave + 0x20), then restore original bytes.
    {
      const std::uint64_t deadline = GetTickCount64() + options.waitMs;
      std::uint64_t status = 0;
      while (GetTickCount64() < deadline) {
        if (!VMMDLL_MemReadEx(h, options.pid, result.dataCave + 0x20,
                              reinterpret_cast<PBYTE>(&status), 8, nullptr,
                              VMMDLL_FLAG_NOCACHE))
          break;
        if (status != 0) {
          result.executed = true;
          result.loadLibraryResult = static_cast<std::uintptr_t>(status);
          break;
        }
        Sleep(options.pollMs);
      }
    }
    // Restore original function head.
    (void)VMMDLL_MemWrite(h, options.pid, result.iatFunction, orig, 16);
    Sleep(10);
    std::vector<std::uint8_t> zero(sc.size(), 0);
    (void)VMMDLL_MemWrite(h, options.pid, result.codeCave, zero.data(),
                          static_cast<DWORD>(zero.size()));
    std::vector<std::uint8_t> zdata(data.size(), 0);
    (void)VMMDLL_MemWrite(h, options.pid, result.dataCave, zdata.data(),
                          static_cast<DWORD>(zdata.size()));
    result.restored = true;
  } else {
    // ---- IAT thunk redirect ----
    std::uint64_t newThunk = result.codeCave;
    if (!VMMDLL_MemWrite(h, options.pid, result.iatThunk,
                         reinterpret_cast<PBYTE>(&newThunk), 8)) {
      result.error = "DMA write of IAT thunk failed";
      return false;
    }
    // Wait for execution (poll status at dataCave+0x20).
    {
      const std::uint64_t deadline = GetTickCount64() + options.waitMs;
      std::uint64_t status = 0;
      while (GetTickCount64() < deadline) {
        if (!VMMDLL_MemReadEx(h, options.pid, result.dataCave + 0x20,
                              reinterpret_cast<PBYTE>(&status), 8, nullptr,
                              VMMDLL_FLAG_NOCACHE))
          break;
        if (status != 0) {
          result.executed = true;
          result.loadLibraryResult = static_cast<std::uintptr_t>(status);
          break;
        }
        Sleep(options.pollMs);
      }
    }
    // Restore IAT thunk, then clear caves.
    std::uint64_t orig = result.iatFunction;
    (void)VMMDLL_MemWrite(h, options.pid, result.iatThunk,
                          reinterpret_cast<PBYTE>(&orig), 8);
    Sleep(10);
    std::vector<std::uint8_t> zero(sc.size(), 0);
    (void)VMMDLL_MemWrite(h, options.pid, result.codeCave, zero.data(),
                          static_cast<DWORD>(zero.size()));
    std::vector<std::uint8_t> zdata(data.size(), 0);
    (void)VMMDLL_MemWrite(h, options.pid, result.dataCave, zdata.data(),
                          static_cast<DWORD>(zdata.size()));
    result.restored = true;
  }

  result.ok = true;
  return true;
}

} // namespace er2
