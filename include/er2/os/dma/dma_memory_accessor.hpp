#pragma once

// ============================================================================
// DMA Memory Accessor - MemProcFS Based External Memory Reader
// ============================================================================
// Uses MemProcFS (vmmdll.h) to access process memory via DMA hardware.
// Requires PCILeech FPGA or similar DMA device.
//
// Runtime dependencies (must be in exe directory):
//   - vmm.dll
//   - leechcore.dll
//   - ftd3xx.dll
// ============================================================================

#include <Windows.h>
#include <cstddef>
#include <cstdint>
#include <functional>
#include <optional>
#include <string>
#include <vector>

// #include <winternl.h> // CONFLICT: Removed to avoid clash with
// DMALibrary/nt/structs.h

#ifndef _NTSTATUS_DEFINED
using NTSTATUS = long;
#define _NTSTATUS_DEFINED
#endif

#include "../../mem/memory_accessor.hpp"

// MemProcFS headers
// MemProcFS headers
// Point to the new Memory library vmmdll.h to ensure compatibility with Mem.cpp
#include "../../../../deps/Memory/libs/vmmdll.h"

#pragma comment(lib, "vmm.lib")

namespace er2 {

/// <summary>
/// DMA Memory Accessor, using MemProcFS library to read target process memory
/// via FPGA DMA hardware.
/// </summary>
class DMAMemoryAccessor : public IMemoryAccessor {
public:
  /// <summary>
  /// Constructor: initializes MemProcFS and connects to DMA device.
  /// </summary>
  /// <param name="deviceArg">Device argument, default "fpga" (DMA
  /// hardware)</param>
  explicit DMAMemoryAccessor(const char *deviceArg = "fpga") {
    LPCSTR args[] = {"", "-device", deviceArg, "-memmap", "auto"};
    m_hVMM = VMMDLL_Initialize(5, args);
    m_bInitialized = (m_hVMM != nullptr);
    m_ownsVmmHandle = m_bInitialized;
  }

  /// <summary>
  /// Constructor from existing handle (for MetickAdapter)
  /// </summary>
  explicit DMAMemoryAccessor(VMM_HANDLE hVMM)
      : m_hVMM(hVMM), m_bInitialized(hVMM != nullptr),
        m_ownsVmmHandle(false) {}

  ~DMAMemoryAccessor() {
    if (m_hVMM && m_ownsVmmHandle) {
      VMMDLL_Close(m_hVMM);
    }
    m_hVMM = nullptr;
    m_ownsVmmHandle = false;
  }

  // Disable Copy
  DMAMemoryAccessor(const DMAMemoryAccessor &) = delete;
  DMAMemoryAccessor &operator=(const DMAMemoryAccessor &) = delete;

  // Allow Move
  DMAMemoryAccessor(DMAMemoryAccessor &&other) noexcept
      : m_hVMM(other.m_hVMM), m_dwPID(other.m_dwPID),
        m_bInitialized(other.m_bInitialized),
        m_ownsVmmHandle(other.m_ownsVmmHandle) {
    other.m_hVMM = nullptr;
    other.m_dwPID = 0;
    other.m_bInitialized = false;
    other.m_ownsVmmHandle = false;
  }

  DMAMemoryAccessor &operator=(DMAMemoryAccessor &&other) noexcept {
    if (this != &other) {
      if (m_hVMM && m_ownsVmmHandle)
        VMMDLL_Close(m_hVMM);
      m_hVMM = other.m_hVMM;
      m_dwPID = other.m_dwPID;
      m_bInitialized = other.m_bInitialized;
      m_ownsVmmHandle = other.m_ownsVmmHandle;
      other.m_hVMM = nullptr;
      other.m_dwPID = 0;
      other.m_bInitialized = false;
      other.m_ownsVmmHandle = false;
    }
    return *this;
  }

  /// <summary>
  /// Checks if DMA is successfully initialized.
  /// </summary>
  bool IsInitialized() const { return m_bInitialized; }

  /// <summary>
  /// Gets VMM handle.
  /// </summary>
  VMM_HANDLE GetHandle() const { return m_hVMM; }

  /// <summary>
  /// Mark the handle as externally owned. The accessor will stop closing it
  /// in its destructor; the owner must call its own shutdown routine.
  /// </summary>
  void ReleaseHandleOwnership() noexcept { m_ownsVmmHandle = false; }

  /// <summary>
  /// Gets current attached process PID.
  /// </summary>
  DWORD GetPID() const { return m_dwPID; }

  // ========================================================================
  // Process Operations
  // ========================================================================

  /// <summary>
  /// Attaches to target process.
  /// </summary>
  /// <param name="processName">Process Name (e.g.
  /// "NarakaBladepoint.exe")</param> <returns>True if successful</returns>
  bool Attach(const char *processName) {
    if (!m_hVMM)
      return false;
    return VMMDLL_PidGetFromName(m_hVMM, const_cast<LPSTR>(processName),
                                 &m_dwPID);
  }

  /// <summary>
  /// Gets module base address.
  /// </summary>
  /// <param name="moduleName">Module Name (e.g. "UnityPlayer.dll")</param>
  /// <returns>Module base address, or 0 if failed</returns>
  std::uintptr_t GetModuleBase(const char *moduleName) const {
    if (!m_hVMM || m_dwPID == 0)
      return 0;
    return static_cast<std::uintptr_t>(VMMDLL_ProcessGetModuleBaseU(
        m_hVMM, m_dwPID, const_cast<LPSTR>(moduleName)));
  }

  /// <summary>
  /// Module Info Struct
  /// </summary>
  struct DmaModuleInfo {
    std::string name;
    std::string fullName;
    std::uintptr_t baseAddress;
    std::uint32_t imageSize;
    bool isWow64;
  };

  /// <summary>
  /// Enumerates all loaded modules in target process (for diagnostics and
  /// manual search)
  /// </summary>
  /// <returns>List of module info</returns>
  std::vector<DmaModuleInfo> EnumerateModules() const {
    std::vector<DmaModuleInfo> result;
    if (!m_hVMM || m_dwPID == 0)
      return result;

    PVMMDLL_MAP_MODULE pModuleMap = nullptr;
    if (!VMMDLL_Map_GetModuleU(m_hVMM, m_dwPID, &pModuleMap, 0)) {
      return result;
    }

    if (pModuleMap) {
      for (DWORD i = 0; i < pModuleMap->cMap; i++) {
        const auto &entry = pModuleMap->pMap[i];
        DmaModuleInfo info;
        info.name = entry.uszText ? entry.uszText : "";
        info.fullName = entry.uszFullName ? entry.uszFullName : "";
        info.baseAddress = static_cast<std::uintptr_t>(entry.vaBase);
        info.imageSize = entry.cbImageSize;
        info.isWow64 = entry.fWoW64;
        result.push_back(info);
      }
      VMMDLL_MemFree(pModuleMap);
    }
    return result;
  }

  /// <summary>
  /// Fuzzy search for module containing keyword (case-insensitive)
  /// </summary>
  /// <param name="keyword">Search keyword (e.g. "unity", "player")</param>
  /// <returns>List of matching module info</returns>
  std::vector<DmaModuleInfo>
  FindModulesByKeyword(const std::string &keyword) const {
    std::vector<DmaModuleInfo> result;
    auto allModules = EnumerateModules();

    std::string lowerKeyword = keyword;
    for (auto &c : lowerKeyword)
      c = static_cast<char>(tolower(c));

    for (const auto &mod : allModules) {
      std::string lowerName = mod.name;
      for (auto &c : lowerName)
        c = static_cast<char>(tolower(c));

      if (lowerName.find(lowerKeyword) != std::string::npos) {
        result.push_back(mod);
      }
    }
    return result;
  }

  // ========================================================================
  // IMemoryAccessor Interface Implementation
  // ========================================================================

  /// <summary>
  /// Reads memory (IMemoryAccessor implementation).
  /// Uses VMMDLL_FLAG_NOCACHE to ensure data freshness.
  /// </summary>
  bool Read(std::uintptr_t address, void *buffer,
            std::size_t size) const override {
    if (!m_hVMM || m_dwPID == 0 || !buffer || size == 0)
      return false;

    DWORD bytesRead = 0;
    BOOL success =
        VMMDLL_MemReadEx(m_hVMM, m_dwPID, static_cast<ULONG64>(address),
                         static_cast<PBYTE>(buffer), static_cast<DWORD>(size),
                         &bytesRead, VMMDLL_FLAG_NOCACHE);

    return success && (bytesRead == static_cast<DWORD>(size));
  }

  /// <summary>
  /// Writes memory (IMemoryAccessor implementation).
  /// </summary>
  bool Write(std::uintptr_t address, const void *buffer,
             std::size_t size) const override {
    if (!m_hVMM || m_dwPID == 0 || !buffer || size == 0)
      return false;

    return VMMDLL_MemWrite(m_hVMM, m_dwPID, static_cast<ULONG64>(address),
                           const_cast<PBYTE>(static_cast<const BYTE *>(buffer)),
                           static_cast<DWORD>(size));
  }

  // ========================================================================
  // Generic Read Templates
  // ========================================================================

  /// <summary>
  /// Generic read function.
  /// </summary>
  /// <summary>
  /// Generic read function.
  /// </summary>
  template <typename T> bool ReadValue(std::uintptr_t address, T &out) const {
    return this->Read(address, &out, sizeof(T));
  }

  /// <summary>
  /// Generic read function, returning optional.
  /// </summary>
  template <typename T>
  std::optional<T> ReadValue(std::uintptr_t address) const {
    T value{};
    if (this->ReadValue(address, value))
      return value;
    return std::nullopt;
  }

  /// <summary>
  /// Reads pointer value.
  /// </summary>
  std::optional<std::uintptr_t> ReadPtr(std::uintptr_t address) const {
    return this->ReadValue<std::uintptr_t>(address);
  }

  /// <summary>
  /// Reads string (fixed length).
  /// </summary>
  std::string ReadString(std::uintptr_t address,
                         std::size_t maxLength = 256) const {
    std::string buffer(maxLength, '\0');
    // Cast to void* to allow writing (since C++17 string::data() is non-const
    // but compiler might be strict)
    if (!this->Read(address, (void *)buffer.data(), maxLength))
      return {};

    // Find null terminator
    size_t len = buffer.find('\0');
    if (len != std::string::npos)
      buffer.resize(len);

    return buffer;
  }

  // ========================================================================
  // Log Callback
  // ========================================================================

  using LogCallback = std::function<void(const std::string &)>;

  /// <summary>
  /// Set log callback function
  /// </summary>
  void SetLogCallback(LogCallback cb) { m_logCallback = cb; }

  // ========================================================================
  // Performance Optimization Interface (Scatter Read)
  // ========================================================================

  // Performance Optimization Interface (Scatter Read)
  // ========================================================================

  // ========================================================================
  // Performance Optimization Interface (Scatter Read)
  // ========================================================================

  struct ScatterRequest {
    std::uintptr_t address;
    void *buffer;
    std::size_t size;
  };

  mutable std::vector<ScatterRequest> m_scatterRequests;
  mutable std::vector<MEM_SCATTER> m_memScatters;
  mutable std::vector<PMEM_SCATTER> m_memScatterPtrs;

  virtual void PrepareScatterRead(std::uintptr_t address, void *buffer,
                                  std::size_t size) const override {
    if (!address || !buffer || size == 0)
      return;
    m_scatterRequests.push_back({address, buffer, size});
  }

  virtual bool ExecuteScatterRead() const override {
    if (m_scatterRequests.empty())
      return true;

    size_t count = m_scatterRequests.size();
    m_memScatters.resize(count);
    m_memScatterPtrs.resize(count);

    for (size_t i = 0; i < count; ++i) {
      auto &req = m_scatterRequests[i];
      auto &sc = m_memScatters[i];

      sc.version = MEM_SCATTER_VERSION;
      sc.f = FALSE;
      sc.qwA = req.address;
      sc.pb = static_cast<PBYTE>(req.buffer);
      sc.cb = static_cast<DWORD>(req.size);

      m_memScatterPtrs[i] = &sc;
    }

    // Call MemProcFS Scatter Read
    DWORD dwFlags = VMMDLL_FLAG_NOCACHE; // Ensure freshness
    DWORD dwRead =
        VMMDLL_MemReadScatter(m_hVMM, m_dwPID, m_memScatterPtrs.data(),
                              static_cast<DWORD>(count), dwFlags);

    // Clear requests regardless of result to prepare for next batch
    m_scatterRequests.clear();
    m_memScatters.clear();
    m_memScatterPtrs.clear();

    return dwRead == count;
  }

protected:
  void Log(const std::string &msg) const {
    if (m_logCallback)
      m_logCallback(msg);
  }

  VMM_HANDLE m_hVMM = nullptr;
  DWORD m_dwPID = 0;
  bool m_bInitialized = false;
  bool m_ownsVmmHandle = false;
  LogCallback m_logCallback;
};

} // namespace er2
