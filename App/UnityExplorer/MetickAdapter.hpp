#pragma once

// Include implementation interface from er2
#include "../../include/er2/os/dma/dma_memory_accessor.hpp"
#include <chrono>
#include <string>
#include <vector>

// Depend on deps/Memory
#include "../../deps/Memory/Mem.h"

// [Fix Header Conflicts]
#ifndef _NTSTATUS_DEFINED
using NTSTATUS = long;
#define _NTSTATUS_DEFINED
#endif

// Disable warnings from external libs
#pragma warning(push)
#pragma warning(disable : 4005) // Macro redefinition
#pragma warning(disable : 4430) // Missing type specifier

// We don't include Memory.h from DMALibrary anymore, we use deps/Memory/Mem.h
// which uses namespaces mem and DmaData.

#pragma warning(pop)

class MetickAdapter : public er2::DMAMemoryAccessor {
public:
  // Initialize with null handle first
  explicit MetickAdapter() : er2::DMAMemoryAccessor((VMM_HANDLE) nullptr) {}

  virtual ~MetickAdapter() {
    if (m_hScatter) {
      VMMDLL_Scatter_CloseHandle(m_hScatter);
      m_hScatter = nullptr;
    }

    // MetickAdapter is the sole owner of the process-wide MemProcFS session.
    // Shut it down before the base destructor runs, then detach the base view
    // so DMAMemoryAccessor cannot close the same VMM handle a second time.
    if (m_ownsGlobalDma) {
      mem::Shutdown();
      m_ownsGlobalDma = false;
    }
    this->ReleaseHandleOwnership();
    this->m_hVMM = nullptr;
    this->m_dwPID = 0;
    this->m_bInitialized = false;
  }

  const std::string &GetLastError() const { return m_lastError; }

  // Initialize the library
  // processName: e.g. "UnityPlayer.dll" or "NarakaBladepoint.exe"
  bool Initialize(const std::string &processName) {
    m_lastError.clear();

    // 1. Set the target process name for FixCr3/GetModule
    mem::SetTargetProcessName(processName);

    // 2. Initialize DMA (FPGA)
    if (!mem::DMA_Init()) {
      m_lastError =
          "mem::DMA_Init failed (VMMDLL_Initialize or FPGA setup failed)";
      return false;
    }
    m_ownsGlobalDma = true;

    // 3. Get Process ID
    int pid = mem::Get_Process_Id(processName.c_str());
    if (pid == 0) {
      // Maybe wait and retry? For now return false.
      m_lastError = "target process not found through MemProcFS: " + processName;
      mem::Shutdown();
      m_ownsGlobalDma = false;
      return false;
    }

    // 4. Update Global PID (Critical for Mem library functions)
    DmaData::PID = pid;

    // 5. Sync to base class
    this->m_hVMM = DmaData::vHandle;
    this->m_dwPID = static_cast<DWORD>(pid);
    this->m_bInitialized = (this->m_hVMM != nullptr);
    if (!this->m_bInitialized) {
      m_lastError = "DMA handle is null after process lookup";
      mem::Shutdown();
      m_ownsGlobalDma = false;
      return false;
    }

    return true;
  }

  // Fix CR3 (Critical Feature)
  bool FixCr3(std::uint32_t timeoutMs = 30000) {
    // Use FixCr32 as it seems more generic (PFN scan) than FixCr3 (File based)
    // Line 150 Mem.cpp
    return mem::FixCr32(timeoutMs);
  }

  // ========================================================================
  // Cache Control (Optimization)
  // ========================================================================
private:
  mutable bool m_forceNoCache = false;
  std::string m_lastError;
  bool m_ownsGlobalDma = false;

public:
  void SetForceNoCache(bool enabled) { m_forceNoCache = enabled; }

  // Core Read Interface - Optimized for Page Fault / Cache issues
  virtual bool Read(std::uintptr_t address, void *buffer,
                    std::size_t size) const override {
    if (!m_hVMM || m_dwPID == 0)
      return false;

    DWORD read = 0;

    // Determine flags based on forceNoCache
    DWORD flags = m_forceNoCache ? VMMDLL_FLAG_NOCACHE : 0;

    // 1. Try with selected flags
    if (VMMDLL_MemReadEx(m_hVMM, m_dwPID, (ULONG64)address, (PBYTE)buffer,
                         (DWORD)size, &read, flags) &&
        read == (DWORD)size) {
      return true;
    }

    // 2. Retry with NOCACHE (Fallback for stale/invalid pages) if not already
    // forced
    if (!m_forceNoCache) {
      if (VMMDLL_MemReadEx(m_hVMM, m_dwPID, (ULONG64)address, (PBYTE)buffer,
                           (DWORD)size, &read, VMMDLL_FLAG_NOCACHE) &&
          read == (DWORD)size) {
        return true;
      }
    }

    return false;
  }

  // Core Write Interface
  virtual bool Write(std::uintptr_t address, const void *buffer,
                     std::size_t size) const override {
    if (!m_hVMM || m_dwPID == 0)
      return false;
    return VMMDLL_MemWrite(m_hVMM, m_dwPID, (ULONG64)address, (PBYTE)buffer,
                           (DWORD)size);
  }

  // ========================================================================
  // Scatter Read Implementation
  // ========================================================================

private:
  mutable VMMDLL_SCATTER_HANDLE m_hScatter = nullptr;
  struct ScatterRequest {
    std::uintptr_t addr;
    void *buffer;
    std::size_t size;
  };
  mutable std::vector<ScatterRequest> m_scatterRequests;

public:
  virtual void PrepareScatterRead(std::uintptr_t address, void *buffer,
                                  std::size_t size) const override {
    if (!m_hScatter) {
      // Use ZEROPAD_ON_FAIL, but DO NOT force NOCACHE.
      // Allow VMMDLL to decide or use cache if available (Performance
      // Optimization)
      m_hScatter = VMMDLL_Scatter_Initialize(m_hVMM, m_dwPID,
                                             VMMDLL_FLAG_ZEROPAD_ON_FAIL);
    }

    // Check if handle creation failed
    if (!m_hScatter)
      return;

    VMMDLL_Scatter_Prepare(m_hScatter, (ULONG64)address, (DWORD)size);
    m_scatterRequests.push_back({address, buffer, size});
  }

  virtual bool ExecuteScatterRead() const override {
    if (!m_hScatter || m_scatterRequests.empty())
      return false;

    VMMDLL_Scatter_ExecuteRead(m_hScatter);

    // Retrieve results
    for (const auto &req : m_scatterRequests) {
      DWORD bytesRead = 0;
      VMMDLL_Scatter_Read(m_hScatter, (ULONG64)req.addr, (DWORD)req.size,
                          (PBYTE)req.buffer, &bytesRead);
    }

    // Cleanup for next batch
    VMMDLL_Scatter_Clear(m_hScatter, m_dwPID, VMMDLL_FLAG_ZEROPAD_ON_FAIL);
    m_scatterRequests.clear();
    return true;
  }

  // Accessor for Keyboard Input (New Feature)
  c_keys &GetInputManager() {
    return key; // Global 'key' variable from Mem.h
  }

  // Helper helper
  VMM_HANDLE GetVmmHandle() const { return DmaData::vHandle; }
};
