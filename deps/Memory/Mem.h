#pragma once

#include "MemIO.h"
#include "pch.h"
#include <chrono> // Fix missing system_clock
#include <cstdint>
#include <shared_mutex>
#include <unordered_map>


namespace DmaData {
inline VMM_HANDLE vHandle;

inline HWND Hwnd;
inline VMMDLL_SCATTER_HANDLE hS;
inline VMMDLL_SCATTER_HANDLE hS1;

inline HANDLE hProcess;
inline int PID;
inline bool 是CR3;
inline std::string TargetProcessName =
    "UnityPlayer.dll"; // Default, will be overwritten
} // namespace DmaData
extern bool 主机鼠标控制中;
enum class e_registry_type {
  none = REG_NONE,
  sz = REG_SZ,
  expand_sz = REG_EXPAND_SZ,
  binary = REG_BINARY,
  dword = REG_DWORD,
  dword_little_endian = REG_DWORD_LITTLE_ENDIAN,
  dword_big_endian = REG_DWORD_BIG_ENDIAN,
  link = REG_LINK,
  multi_sz = REG_MULTI_SZ,
  resource_list = REG_RESOURCE_LIST,
  full_resource_descriptor = REG_FULL_RESOURCE_DESCRIPTOR,
  resource_requirements_list = REG_RESOURCE_REQUIREMENTS_LIST,
  qword = REG_QWORD,
  qword_little_endian = REG_QWORD_LITTLE_ENDIAN
};

class c_registry {
private:
public:
  c_registry() {}

  ~c_registry() {}

  std::string QueryValue(const char *path, e_registry_type type);
};

class c_keys {
private:
  uint64_t gafAsyncKeyStateExport = 0;
  uint64_t gptCursorAsyncExport = 0;
  uint8_t state_bitmap[64]{};
  uint8_t previous_state_bitmap[256 / 8]{};
  uint64_t win32kbase = 0;

  long mouse_positionx = 0;
  long mouse_positiony = 0;

  int win_logon_pid = 0;

  c_registry registry;
  std::chrono::time_point<std::chrono::system_clock> start =
      std::chrono::system_clock::now();

public:
  c_keys() = default;

  ~c_keys() = default;

  bool InitKeyboard();

  void UpdateKeys();
  void UpdateMousePosition();
  bool IsKeyDown(uint32_t virtual_key_code);
};
inline c_keys key;

namespace mem {
inline bool isCr3 = false;
inline bool NewisCr3 = false;
inline uint64_t Baseaddr = 0;
VMM_HANDLE DMA_Initialize();
bool SetFPGA();
bool DMA_Init();
// Idempotently release the process-wide DMA/VMM state. The caller must ensure
// no memory operation is in flight before invoking this function.
void Shutdown();
void SetTargetProcessName(const std::string &name);
int Get_Process_Id(const char *process_name);
bool Read(uintptr_t address, void *buffer, size_t size);
uint64_t Get_Moduleaddr(VMM_HANDLE Hvmm, int PID, const char *module_name);
// Attempt to validate/repair the target process directory table base (DTB).
// The timeout covers plugin initialization, progress polling, and candidate
// DTB/PFN probing so a failed hardware/plugin state cannot block callers
// indefinitely.
bool FixCr3(std::uint32_t timeoutMs = 30000);
bool FixCr32(std::uint32_t timeoutMs = 30000);
void ReadTArry(uint64_t Adr, int Count, uint64_t *actors);
uint64_t FindSignature(const char *signature, uint64_t range_start,
                       uint64_t range_end, int PID);
void scatterClear(VMMDLL_SCATTER_HANDLE h);
void scatterExec(VMMDLL_SCATTER_HANDLE h);
VMMDLL_SCATTER_HANDLE scatterInit();
// VMMDLL_SCATTER_HANDLE CreateScatterHandle();
// void AddScatterReadRequest(VMMDLL_SCATTER_HANDLE handle, uint64_t address,
// void* buffer, size_t size); void ExecuteReadScatter(VMMDLL_SCATTER_HANDLE
// handle, int pid = 0);
void scatterDestroy(VMMDLL_SCATTER_HANDLE h);
void scatterPrep(VMMDLL_SCATTER_HANDLE h, uint64_t address, int size);
void scatterRead(VMMDLL_SCATTER_HANDLE hS, QWORD va, DWORD cb, PBYTE pb,
                 PDWORD pcbRead);
uint64_t FindAddr(uint64_t addr);
void RefreshConfigSet();
void RefreshConfigSetALL();
static std::unordered_map<uint64_t, uint64_t> valist;

std::string RegistryQueryValue(std::string path, DWORD type);
std::vector<int> GetPidList(std::string name);
bool InitRemoteKeyDetection();
bool IsValidPointer(uint64_t Ptr);

template <typename T> T RAM(int PID, uint64_t addr) {
  T Novalue = {};
  VMMDLL_MemReadEx(DmaData::vHandle, PID, addr, (PBYTE)&Novalue, sizeof(T), 0,
                   VMMDLL_FLAG_NOCACHE);
  return Novalue;
}

template <typename T> T Read(void *address) {
  T buffer{};
  memset(&buffer, 0, sizeof(T));
  Read(reinterpret_cast<uint64_t>(address), reinterpret_cast<void *>(&buffer),
       sizeof(T));
  return buffer;
}

template <typename T> T Read(uint64_t address) {
  return Read<T>(reinterpret_cast<void *>(address));
}

template <typename T> T ReadEx(uint64_t addr) {
  T Novalue = {};

  if (isCr3) {
    uint64_t tmpaddr = FindAddr(addr);
    VMMDLL_MemRead(DmaData::vHandle, -1, tmpaddr, (PBYTE)&Novalue, sizeof(T));
    return Novalue;
  }

  VMMDLL_MemRead(DmaData::vHandle, DmaData::PID, addr, (PBYTE)&Novalue,
                 sizeof(T));
  return Novalue;
}

template <typename T> T ReadExpid(uint64_t addr, int pid) {
  T Novalue = {};
  VMMDLL_MemRead(DmaData::vHandle, pid, addr, (PBYTE)&Novalue, sizeof(T));
  return Novalue;
}

bool Readdata(uint64_t addr, void *buffer, _In_ DWORD cb);

template <typename TV>
static TV scatterRead(VMMDLL_SCATTER_HANDLE h, uint64_t address, size_t size) {
  TV buffer{};
  DWORD bytesRead = 0;
  return VMMDLL_Scatter_Read(h, address, size, (PBYTE)&buffer, &bytesRead)
             ? buffer
             : TV();
};

} // namespace mem
