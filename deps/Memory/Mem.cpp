#define _CRT_SECURE_NO_WARNINGS
#include "Mem.h"
#include <algorithm>
#include <limits>
#include <thread>

using namespace std;
int win_logon_pid = 0;
uint64_t win32kbase = 0;
uint64_t gafAsyncKeyStateExport = 0;
uint8_t state_bitmap[64]{};
uint8_t previous_state_bitmap[256 / 8]{};
bool 主机鼠标控制中 = false;
std::shared_mutex mutexDec;

// 初始化DMA
void mem::SetTargetProcessName(const std::string &name) {
  DmaData::TargetProcessName = name;
}

VMM_HANDLE mem::DMA_Initialize() {
  // Create a vector of command-line arguments
  vector<LPCSTR> args = {"-norefresh", "-device", "FPGA"};
  // Initialize the VMM library with these arguments and get the handle
  return VMMDLL_Initialize(static_cast<DWORD>(args.size()), args.data());
}

unsigned char abort2[4] = {0x10, 0x00, 0x10, 0x00};
bool mem::SetFPGA() {
  ULONG64 qwID = 0, qwVersionMajor = 0, qwVersionMinor = 0;
  if (!VMMDLL_ConfigGet(DmaData::vHandle, LC_OPT_FPGA_FPGA_ID, &qwID) &&
      VMMDLL_ConfigGet(DmaData::vHandle, LC_OPT_FPGA_VERSION_MAJOR,
                       &qwVersionMajor) &&
      VMMDLL_ConfigGet(DmaData::vHandle, LC_OPT_FPGA_VERSION_MINOR,
                       &qwVersionMinor)) {
    return false;
  }

  if ((qwVersionMajor >= 4) &&
      ((qwVersionMajor >= 5) || (qwVersionMinor >= 7))) {
    HANDLE handle;
    LC_CONFIG config;

    // 使用传统的成员赋值和 strncpy 复制字符串
    // Use strncpy_s for safety
    config.dwVersion = LC_CONFIG_VERSION;
    strncpy_s(config.szDevice, "existing", _TRUNCATE);

    handle = LcCreate(&config);
    if (!handle) {
      return false;
    }

    LcCommand(handle, LC_CMD_FPGA_CFGREGPCIE_MARKWR | 0x002, 4,
              reinterpret_cast<PBYTE>(&abort2), NULL, NULL);
    LcClose(handle);
  }

  return true;
}

bool mem::DMA_Init() {

  // Keep one VMM owner per process. Repeated connect attempts reuse the
  // existing handle instead of leaking/overwriting a live session.
  if (DmaData::vHandle) {
    MemIO::Init(DmaData::vHandle);
    return true;
  }

  vector<LPCSTR> args = {"-norefresh", "-device", "fpga://algo=0"};

  DmaData::vHandle = VMMDLL_Initialize(args.size(), args.data());
  if (!DmaData::vHandle) {
    return false;
  }

  ULONG64 FPGA_ID = 0, DEVICE_ID = 0;

  VMMDLL_ConfigGet(DmaData::vHandle, LC_OPT_FPGA_FPGA_ID, &FPGA_ID);
  VMMDLL_ConfigGet(DmaData::vHandle, LC_OPT_FPGA_DEVICE_ID, &DEVICE_ID);
  if (!mem::SetFPGA()) {
    LOG("[!] Could not set FPGA!\n");
    VMM_HANDLE failedHandle = DmaData::vHandle;
    DmaData::vHandle = nullptr;
    MemIO::Shutdown();
    VMMDLL_Close(failedHandle);
    return false;
  }
  MemIO::Init(DmaData::vHandle);
  return true;
}

void mem::Shutdown() {
  // The caller must stop the executor/worker threads before invoking this
  // function. Detach globals before closing so late failure paths cannot use
  // a stale handle. VMMDLL_Close is issued exactly once for the owned handle.
  VMMDLL_SCATTER_HANDLE scatter0 = DmaData::hS;
  VMMDLL_SCATTER_HANDLE scatter1 = DmaData::hS1;
  DmaData::hS = nullptr;
  DmaData::hS1 = nullptr;
  if (scatter0)
    VMMDLL_Scatter_CloseHandle(scatter0);
  if (scatter1 && scatter1 != scatter0)
    VMMDLL_Scatter_CloseHandle(scatter1);

  VMM_HANDLE handle = DmaData::vHandle;
  DmaData::vHandle = nullptr;
  DmaData::PID = 0;
  DmaData::hProcess = nullptr;
  DmaData::是CR3 = false;
  isCr3 = false;
  NewisCr3 = false;
  Baseaddr = 0;

  {
    std::unique_lock<std::shared_mutex> cacheLock(mutexDec);
    valist.clear();
  }

  MemIO::Shutdown();
  if (handle)
    VMMDLL_Close(handle);
}

// DMA获取进程ID
int mem::Get_Process_Id(const char *process_name) {
  DWORD buffer;
  if (VMMDLL_PidGetFromName(DmaData::vHandle, (LPSTR)process_name, &buffer))
    return buffer;
  return 0;
}

bool mem::Read(uintptr_t address, void *buffer, size_t size) {
  if (!DmaData::vHandle || DmaData::PID == 0 || !buffer || size == 0)
    return false;
  DWORD read_size = 0;

  if (!VMMDLL_MemReadEx(DmaData::vHandle, DmaData::PID, address,
                        static_cast<PBYTE>(buffer), size, &read_size,
                        VMMDLL_FLAG_NOCACHE)) {
    return false;
  }

  return (read_size == size);
}

// DMA获取模块基址
uint64_t mem::Get_Moduleaddr(VMM_HANDLE Hvmm, int PID,
                             const char *module_name) {
  return VMMDLL_ProcessGetModuleBaseU(Hvmm, PID, (LPSTR)module_name);
}

std::string c_registry::QueryValue(const char *path, e_registry_type type) {
  if (!DmaData::vHandle)
    return "";

  BYTE buffer[0x128];
  DWORD _type = static_cast<DWORD>(type);
  DWORD size = sizeof(buffer);

  if (!VMMDLL_WinReg_QueryValueExU(DmaData::vHandle, const_cast<LPSTR>(path),
                                   &_type, buffer, &size)) {
    LOG("[!] failed QueryValueExU call\n");
    return "";
  }
  // TODO: implement something nicer & better than this.
  if (type == e_registry_type::dword) {
    DWORD dwordValue = *reinterpret_cast<DWORD *>(buffer);
    return std::to_string(dwordValue);
  }
  std::wstring wstr = std::wstring(reinterpret_cast<wchar_t *>(buffer));
  return std::string(wstr.begin(), wstr.end());
}

uint64_t cbSize = 0x80000;
// callback for VfsFileListU
VOID cbAddFile(_Inout_ HANDLE h, _In_ LPCSTR uszName, _In_ ULONG64 cb,
               _In_opt_ PVMMDLL_VFS_FILELIST_EXINFO pExInfo) {
  if (strcmp(uszName, "dtb.txt") == 0)
    cbSize = cb;
}

struct Info {
  uint32_t index;
  uint32_t process_id;
  uint64_t dtb;
  uint64_t kernelAddr;
  std::string name;
};

// --- Helper Functions and Structs for new FixCr32 ---
static uint64_t cr3_cbSize = 0x80000;
// callback for VfsFileListU
static VOID cr3_cbAddFile(_Inout_ HANDLE h, _In_ LPCSTR uszName,
                          _In_ ULONG64 cb,
                          _In_opt_ PVMMDLL_VFS_FILELIST_EXINFO pExInfo) {
  if (strcmp(uszName, "dtb.txt") == 0)
    cr3_cbSize = cb;
}

struct Cr3_Info {
  uint32_t index;
  uint32_t process_id;
  uint64_t dtb;
  uint64_t kernelAddr;
  std::string name;
};

bool mem::FixCr32(std::uint32_t timeoutMs) {
  const auto deadline = std::chrono::steady_clock::now() +
                        std::chrono::milliseconds(timeoutMs);
  PVMMDLL_MAP_MODULEENTRY module_entry = NULL;

  // 1. Try default first
  if (VMMDLL_Map_GetModuleFromNameU(
          DmaData::vHandle, DmaData::PID,
          const_cast<LPSTR>(DmaData::TargetProcessName.c_str()), &module_entry,
          NULL)) {
    VMMDLL_MemFree(module_entry);
    return true;
  }

  // A zero timeout still permits the cheap direct lookup above, but must not
  // enter plugin/PFN work that could otherwise wait forever.
  if (std::chrono::steady_clock::now() >= deadline) {
    std::fprintf(stderr, "[-] FixCr32 timed out before plugin initialization\n");
    return false;
  }

  // 2. Initialize Plugins
  if (!VMMDLL_InitializePlugins(DmaData::vHandle)) {
    std::fprintf(stderr, "[-] Failed VMMDLL_InitializePlugins call\n");
    return false;
  }

  // 3. Wait for progress, bounded by the caller supplied deadline.
  {
    const auto now = std::chrono::steady_clock::now();
    const auto initialWait =
        deadline < now + std::chrono::milliseconds(500)
            ? deadline
            : now + std::chrono::milliseconds(500);
    std::this_thread::sleep_until(initialWait);
  }
  bool progressReady = false;
  while (std::chrono::steady_clock::now() < deadline) {
    BYTE bytes[4] = {0};
    DWORD i = 0;
    auto nt = VMMDLL_VfsReadW(
        DmaData::vHandle,
        const_cast<LPWSTR>(L"\\misc\\procinfo\\progress_percent.txt"), bytes, 3,
        &i, 0);

    // VMMDLL returns a byte count; explicitly terminate at the bytes actually
    // returned before passing the buffer to atoi.
    i = std::min<DWORD>(i, static_cast<DWORD>(sizeof(bytes) - 1));
    bytes[i] = 0;

    if (nt == VMMDLL_STATUS_SUCCESS &&
        atoi(reinterpret_cast<LPSTR>(bytes)) == 100) {
      progressReady = true;
      break;
    }

    const auto now = std::chrono::steady_clock::now();
    if (now >= deadline)
      break;
    std::this_thread::sleep_until(
        deadline < now + std::chrono::milliseconds(100)
            ? deadline
            : now + std::chrono::milliseconds(100));
  }
  if (!progressReady) {
    std::fprintf(stderr, "[-] FixCr32 timed out waiting for procinfo progress\n");
    return false;
  }

  // 4. Get file list to find dtb.txt size
  cr3_cbSize = 0x80000;
  VMMDLL_VFS_FILELIST2 VfsFileList;
  VfsFileList.dwVersion = VMMDLL_VFS_FILELIST_VERSION;
  VfsFileList.h = 0;
  VfsFileList.pfnAddDirectory = 0;
  VfsFileList.pfnAddFile = cr3_cbAddFile;

  if (!VMMDLL_VfsListU(DmaData::vHandle,
                       const_cast<LPSTR>("\\misc\\procinfo\\"), &VfsFileList))
    return false;

  // 5. Read dtb.txt.  Keep one byte for an explicit terminator and initialize
  // the storage so a short VFS read can never expose uninitialized text.
  const size_t buffer_size = std::max<size_t>(cr3_cbSize, 1);
  std::unique_ptr<BYTE[]> bytes(new BYTE[buffer_size]{});
  DWORD j = 0;
  const DWORD read_capacity =
      buffer_size - 1 > static_cast<size_t>(UINT32_MAX)
          ? UINT32_MAX
          : static_cast<DWORD>(buffer_size - 1);

  if (VMMDLL_VfsReadW(DmaData::vHandle,
                      const_cast<LPWSTR>(L"\\misc\\procinfo\\dtb.txt"),
                      bytes.get(), read_capacity, &j, 0) !=
      VMMDLL_STATUS_SUCCESS) {
    return false;
  }

  const size_t actual_size = std::min<size_t>(j, read_capacity);
  bytes[actual_size] = 0;

  // 6. Parse and try DTBs
  std::vector<uint64_t> possible_dtbs = {};
  std::string lines(reinterpret_cast<char *>(bytes.get()), actual_size);
  std::istringstream iss(lines);
  std::string line = "";

  while (std::getline(iss, line)) {
    Cr3_Info info = {};
    std::istringstream info_ss(line);
    if (info_ss >> std::hex >> info.index >> std::dec >> info.process_id >>
        std::hex >> info.dtb >> info.kernelAddr >> info.name) {

      if (info.process_id == 0) // Potential match
        possible_dtbs.push_back(info.dtb);

      if (DmaData::TargetProcessName.find(info.name) != std::string::npos)
        possible_dtbs.push_back(info.dtb);
    }
  }

  // 7. Try Patched DTBs
  for (size_t i = 0; i < possible_dtbs.size(); i++) {
    if (std::chrono::steady_clock::now() >= deadline) {
      std::fprintf(stderr, "[-] FixCr32 timed out while probing DTB candidates\n");
      return false;
    }
    auto dtb = possible_dtbs[i];
    VMMDLL_ConfigSet(DmaData::vHandle, VMMDLL_OPT_PROCESS_DTB | DmaData::PID,
                     dtb);

    if (VMMDLL_Map_GetModuleFromNameU(
            DmaData::vHandle, DmaData::PID,
            const_cast<LPSTR>(DmaData::TargetProcessName.c_str()),
            &module_entry, NULL)) {

      VMMDLL_MemFree(module_entry);
      MemIO::Clear();
      std::fprintf(stderr, "[INFO] Patched DTB: 0x%llX\n",
                   (unsigned long long)dtb);
      MemIO::SetProcessId(DmaData::PID, dtb);
      return true;
    }
  }

  if (std::chrono::steady_clock::now() >= deadline) {
    std::fprintf(stderr, "[-] FixCr32 timed out before PFN fallback\n");
    return false;
  }

  // 8. Try PFNMap (Legacy Fallback)
  // Re-added as fallback if plugin method fails
  {
    PVMMDLL_MAP_PFN pfnMap = nullptr;
    DWORD progress[64] = {0};
    if (VMMDLL_Map_GetPfnEx(DmaData::vHandle, &pfnMap, true, progress)) {
      bool isPfnMapValid =
          pfnMap && ((uintptr_t)pfnMap < 0x7FFFFFFFFFFFFFFFull);
      if (isPfnMapValid) {
        for (int i = 0; i < pfnMap->cMap; i++) {
          if (std::chrono::steady_clock::now() >= deadline) {
            std::fprintf(stderr,
                         "[-] FixCr32 timed out while scanning PFN map\n");
            break;
          }
          if (pfnMap->pMap[i].AddressInfo.dwPid == 0) {
            uint64_t dtb_full = (uint64_t)pfnMap->pMap[i].dwPfn << 12;
            VMMDLL_ConfigSet(DmaData::vHandle,
                             VMMDLL_OPT_PROCESS_DTB | DmaData::PID, dtb_full);
            if (VMMDLL_Map_GetModuleFromNameU(
                    DmaData::vHandle, DmaData::PID,
                    const_cast<LPSTR>(DmaData::TargetProcessName.c_str()),
                    &module_entry, NULL)) {
              VMMDLL_MemFree(module_entry);
              VMMDLL_MemFree(pfnMap);
              MemIO::Clear();
              std::fprintf(stderr, "[INFO] Patched DTB via PFN: 0x%llX\n",
                           (unsigned long long)dtb_full);
              MemIO::SetProcessId(DmaData::PID, dtb_full);
              return true;
            }
          }
        }
        VMMDLL_MemFree(pfnMap);
      }
    }
  }

  return false;
}

bool mem::FixCr3(std::uint32_t timeoutMs) {
  const auto deadline = std::chrono::steady_clock::now() +
                        std::chrono::milliseconds(timeoutMs);
  PVMMDLL_MAP_MODULEENTRY module_entry = NULL;
  bool result; // = VMMDLL_Map_GetModuleFromNameU(DmaData::vmm_handle,
               // DmaData::PID,
               // const_cast<LPSTR>("DeltaForceClient-Win64-Shipping.exe"),
               // &module_entry, NULL);
  // if (result)
  // return true; //Doesn't need to be patched lol

  if (std::chrono::steady_clock::now() >= deadline) {
    LOG("[-] FixCr3 timed out before plugin initialization\n");
    return false;
  }

  if (!VMMDLL_InitializePlugins(DmaData::vHandle)) {
    LOG("[-] Failed VMMDLL_InitializePlugins call\n");
    return false;
  }
  {
    const auto now = std::chrono::steady_clock::now();
    const auto initialWait =
        deadline < now + std::chrono::milliseconds(500)
            ? deadline
            : now + std::chrono::milliseconds(500);
    std::this_thread::sleep_until(initialWait);
  }
  // LOG("[+] Plugin\n");
  /*

  */
  bool progressReady = false;
  while (std::chrono::steady_clock::now() < deadline) {
    BYTE bytes[4] = {0};
    DWORD i = 0;

    // auto nt = VMMDLL_VfsReadU(DmaData::vmm_handle,
    // const_cast<LPSTR>("\\misc\\procinfo\\progress_percent.txt"), bytes, 3,
    // &i, 0);
    auto nt = VMMDLL_VfsReadW(
        DmaData::vHandle,
        const_cast<LPWSTR>(L"\\misc\\procinfo\\progress_percent.txt"), bytes, 3,
        &i, 0);
    i = std::min<DWORD>(i, static_cast<DWORD>(sizeof(bytes) - 1));
    bytes[i] = 0;
    if (nt == VMMDLL_STATUS_SUCCESS &&
        atoi(reinterpret_cast<LPSTR>(bytes)) == 100) {
      progressReady = true;
      break;
    }
    std::fprintf(stderr, "查找进度:%d(如果持续10多秒都是0，则重启游戏机)\n",
                 atoi(reinterpret_cast<LPSTR>(bytes)));
    const auto now = std::chrono::steady_clock::now();
    if (now >= deadline)
      break;
    std::this_thread::sleep_until(
        deadline < now + std::chrono::milliseconds(1000)
            ? deadline
            : now + std::chrono::milliseconds(1000));
  }
  if (!progressReady) {
    LOG("[-] FixCr3 timed out waiting for procinfo progress\n");
    return false;
  }

  // printf("跑1\n");

  // have to sleep a little or we try reading the file before the plugin
  // initializes fully

  // printf("跑2\n");

  // LOG("[+] progress\n");

  cbSize = 0x80000;
  VMMDLL_VFS_FILELIST2 VfsFileList;
  VfsFileList.dwVersion = VMMDLL_VFS_FILELIST_VERSION;
  VfsFileList.h = 0;
  VfsFileList.pfnAddDirectory = 0;
  VfsFileList.pfnAddFile = cbAddFile; // dumb af callback who made this system

  result = VMMDLL_VfsListU(
      DmaData::vHandle, const_cast<LPSTR>("\\misc\\procinfo\\"), &VfsFileList);
  if (!result)
    return false;
  // printf("跑3\n");
  // Read the data from the txt and parse it.  Allocate initialized storage,
  // reserve one byte for a terminator, and use the actual VFS byte count when
  // constructing the string.
  const size_t buffer_size = std::max<size_t>(cbSize, 1);
  std::unique_ptr<BYTE[]> bytes(new BYTE[buffer_size]{});
  DWORD j = 0;
  const DWORD read_capacity =
      buffer_size - 1 > static_cast<size_t>(UINT32_MAX)
          ? UINT32_MAX
          : static_cast<DWORD>(buffer_size - 1);
  auto nt = VMMDLL_VfsReadW(DmaData::vHandle,
                            const_cast<LPWSTR>(L"\\misc\\procinfo\\dtb.txt"),
                            bytes.get(), read_capacity, &j, 0);
  if (nt != VMMDLL_STATUS_SUCCESS)
    return false;
  const size_t actual_size = std::min<size_t>(j, read_capacity);
  bytes[actual_size] = 0;
  // printf("跑4\n");
  std::vector<uint64_t> possible_dtbs = {};
  std::string lines(reinterpret_cast<char *>(bytes.get()), actual_size);
  std::istringstream iss(lines);
  std::string line = "";

  while (std::getline(iss, line)) {
    Info info = {};

    std::istringstream info_ss(line);
    if (info_ss >> std::hex >> info.index >> std::dec >> info.process_id >>
        std::hex >> info.dtb >> info.kernelAddr >> info.name) {
      // printf("1..\n");
      if (info.process_id ==
          0) // parts that lack a name or have a NULL pid are suspects
        possible_dtbs.push_back(info.dtb);
      // Simple subset check for safety
      if (!info.name.empty() &&
          DmaData::TargetProcessName.find(info.name) != std::string::npos)
        possible_dtbs.push_back(info.dtb);
    }
  }
  // printf("跑5\n");
  // loop over possible dtbs and set the config to use it til we find the
  // correct one
  for (size_t i = 0; i < possible_dtbs.size(); i++) {
    if (std::chrono::steady_clock::now() >= deadline) {
      LOG("[-] FixCr3 timed out while probing DTB candidates\n");
      return false;
    }
    auto dtb = possible_dtbs[i];
    VMMDLL_ConfigSet(DmaData::vHandle, VMMDLL_OPT_PROCESS_DTB | DmaData::PID,
                     dtb);
    // VMMDLL_ConfigSet(DmaData::vmm_handle, VMMDLL_OPT_PROCESS— | DmaData::PID,
    // dtb);
    result = VMMDLL_Map_GetModuleFromNameU(
        DmaData::vHandle, DmaData::PID,
        const_cast<LPSTR>(DmaData::TargetProcessName.c_str()), &module_entry,
        NULL);
    if (result) {
      MemIO::SetProcessId(DmaData::PID, dtb);
      LOG("[+] Patched DTB \n");
      // printf("\ndtb %llx,%d,%d\n", dtb, possible_dtbs.size(),
    // i);//157f37000，7bd037000，7bf937000
      VMMDLL_MemFree(module_entry);
      module_entry = NULL;
      return true;
    }
  }
  // printf("跑6\n");
  // LOG("[-] Failed to patch module\n");
  return false;
}

void mem::ReadTArry(uint64_t Adr, int Count, uint64_t *actors) {

  VMMDLL_MemReadEx(DmaData::vHandle, DmaData::PID, (uint64_t)Adr, (PBYTE)actors,
                   Count * sizeof(uint64_t), 0, VMMDLL_FLAG_NOCACHE);
}

static const char *hexdigits =
    "\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000"
    "\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000"
    "\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000"
    "\000\001\002\003\004\005\006\007\010\011\000\000\000\000\000\000"
    "\000\012\013\014\015\016\017\000\000\000\000\000\000\000\000\000"
    "\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000"
    "\000\012\013\014\015\016\017\000\000\000\000\000\000\000\000\000"
    "\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000"
    "\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000"
    "\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000"
    "\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000"
    "\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000"
    "\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000"
    "\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000"
    "\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000"
    "\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000";

static uint8_t GetByte(const char *hex) {
  return static_cast<uint8_t>((hexdigits[hex[0]] << 4) | (hexdigits[hex[1]]));
}

uint64_t mem::FindSignature(const char *signature, uint64_t range_start,
                            uint64_t range_end, int PID) {
  if (!signature || signature[0] == '\0' || range_start >= range_end)
    return 0;

  if (PID == 0)
    PID = DmaData::PID;

  std::vector<uint8_t> buffer(range_end - range_start);
  if (!VMMDLL_MemReadEx(DmaData::vHandle, PID, range_start, buffer.data(),
                        buffer.size(), 0, VMMDLL_FLAG_NOCACHE))
    return 0;

  const char *pat = signature;
  uint64_t first_match = 0;
  for (uint64_t i = range_start; i < range_end; i++) {
    if (*pat == '?' || buffer[i - range_start] == GetByte(pat)) {
      if (!first_match)
        first_match = i;

      if (!pat[2])
        break;

      pat += (*pat == '?') ? 2 : 3;
    } else {
      pat = signature;
      first_match = 0;
    }
  }

  return first_match;
}

bool c_keys::InitKeyboard() {
  std::string win = registry.QueryValue(
      "HKLM\\SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion\\CurrentBuild",
      e_registry_type::sz);
  int Winver = 0;
  if (!win.empty())
    Winver = std::stoi(win);
  else
    return false;
  std::string ubr = registry.QueryValue(
      "HKLM\\SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion\\UBR",
      e_registry_type::dword);
  int Ubr = 0;
  if (!ubr.empty())
    Ubr = std::stoi(ubr);
  else
    return false;
  this->win_logon_pid = mem::Get_Process_Id("winlogon.exe");
  if (Winver > 22000) {
    auto pids = mem::GetPidList("csrss.exe");
    for (size_t i = 0; i < pids.size(); i++) {
      auto pid = pids[i];

      PVMMDLL_MAP_MODULEENTRY win32k_module_info;
      if (!VMMDLL_Map_GetModuleFromNameW(
              DmaData::vHandle, pid, const_cast<LPWSTR>(L"win32ksgd.sys"),
              &win32k_module_info, VMMDLL_MODULE_FLAG_NORMAL)) {
        if (!VMMDLL_Map_GetModuleFromNameW(
                DmaData::vHandle, pid, const_cast<LPWSTR>(L"win32k.sys"),
                &win32k_module_info, VMMDLL_MODULE_FLAG_NORMAL)) {
          LOG("failed to get module win32k info\n");
          return false;
        }
      }
      uintptr_t win32k_base = win32k_module_info->vaBase;
      size_t win32k_size = win32k_module_info->cbImageSize;
      // win32ksgd
      auto g_session_ptr =
          mem::FindSignature("48 8B 05 ? ? ? ? 48 8B 04 C8", win32k_base,
                             win32k_base + win32k_size, pid);
      if (!g_session_ptr) {
        // win32k
        g_session_ptr =
            mem::FindSignature("48 8B 05 ? ? ? ? FF C9", win32k_base,
                               win32k_base + win32k_size, pid);
        if (!g_session_ptr) {
          LOG("failed to find g_session_global_slots\n");
          return false;
        }
      }
      int relative = mem::ReadExpid<int>(g_session_ptr + 3, pid);
      uintptr_t g_session_global_slots = g_session_ptr + 7 + relative;
      uintptr_t user_session_state = 0;
      for (int i = 0; i < 4; i++) {
        user_session_state = mem::ReadExpid<uintptr_t>(
            mem::ReadExpid<uintptr_t>(
                mem::ReadExpid<uintptr_t>(g_session_global_slots, pid) + 8 * i,
                pid),
            pid);
        if (user_session_state > 0x7FFFFFFFFFFF)
          break;
      }

      PVMMDLL_MAP_MODULEENTRY win32kbase_module_info;
      if (!VMMDLL_Map_GetModuleFromNameW(
              DmaData::vHandle, pid, const_cast<LPWSTR>(L"win32kbase.sys"),
              &win32kbase_module_info, VMMDLL_MODULE_FLAG_NORMAL)) {
        LOG("failed to get module win32kbase info\n");
        return false;
      }
      uintptr_t win32kbase_base = win32kbase_module_info->vaBase;
      size_t win32kbase_size = win32kbase_module_info->cbImageSize;

      // Unsure if this sig will work on all versions. (sig is from
      // PostUpdateKeyStateEvent function. seems to exist in both older version
      // and the new version of win32kbase that I have checked)
      uintptr_t ptr = mem::FindSignature(
          "48 8D 90 ? ? ? ? E8 ? ? ? ? 0F 57 C0", win32kbase_base,
          win32kbase_base + win32kbase_size, pid);
      uint32_t session_offset = 0x0;
      if (ptr) {
        session_offset = mem::ReadExpid<uint32_t>(ptr + 3, pid);
        gafAsyncKeyStateExport = user_session_state + session_offset;

      } else {
        LOG("failed to find offset for gafAyncKeyStateExport\n");
        return false;
      }

      if (gafAsyncKeyStateExport > 0x7FFFFFFFFFFF)
        break;
    }
    if (gafAsyncKeyStateExport > 0x7FFFFFFFFFFF)
      return true;
    return false;
  } else {
    PVMMDLL_MAP_EAT eat_map = NULL;
    PVMMDLL_MAP_EATENTRY eat_map_entry;
    bool result =
        VMMDLL_Map_GetEATU(DmaData::vHandle,
                           mem::Get_Process_Id("winlogon.exe") |
                               VMMDLL_PID_PROCESS_WITH_KERNELMEMORY,
                           const_cast<LPSTR>("win32kbase.sys"), &eat_map);
    if (!result)
      return false;

    if (eat_map->dwVersion != VMMDLL_MAP_EAT_VERSION) {
      VMMDLL_MemFree(eat_map);
      eat_map_entry = NULL;
      return false;
    }

    for (int i = 0; i < eat_map->cMap; i++) {
      eat_map_entry = eat_map->pMap + i;
      if (strcmp(eat_map_entry->uszFunction, "gafAsyncKeyState") == 0) {
        gafAsyncKeyStateExport = eat_map_entry->vaFunction;

        break;
      }
    }

    VMMDLL_MemFree(eat_map);
    eat_map = NULL;
    if (gafAsyncKeyStateExport < 0x7FFFFFFFFFFF) {
      PVMMDLL_MAP_MODULEENTRY module_info;
      auto result = VMMDLL_Map_GetModuleFromNameW(
          DmaData::vHandle,
          mem::Get_Process_Id("winlogon.exe") |
              VMMDLL_PID_PROCESS_WITH_KERNELMEMORY,
          static_cast<LPCWSTR>(L"win32kbase.sys"), &module_info,
          VMMDLL_MODULE_FLAG_NORMAL);
      if (!result) {
        LOG("failed to get module info\n");
        return false;
      }

      char str[32];
      if (!VMMDLL_PdbLoad(DmaData::vHandle,
                          mem::Get_Process_Id("winlogon.exe") |
                              VMMDLL_PID_PROCESS_WITH_KERNELMEMORY,
                          module_info->vaBase, str)) {
        LOG("failed to load pdb\n");
        return false;
      }

      uintptr_t gafAsyncKeyState;
      if (!VMMDLL_PdbSymbolAddress(DmaData::vHandle, str,
                                   const_cast<LPSTR>("gafAsyncKeyState"),
                                   &gafAsyncKeyState)) {
        LOG("failed to find gafAsyncKeyState\n");
        return false;
      }
      LOG("found gafAsyncKeyState at: 0x%p\n", gafAsyncKeyState);
    }
    if (gafAsyncKeyStateExport > 0x7FFFFFFFFFFF)
      return true;
    return false;
  }
}

void c_keys::UpdateMousePosition() {
  if (gptCursorAsyncExport < 0x7FFFFFFFFFFF)
    return;

  struct {
    LONG x;
    LONG y;
  } pt = {0};

  if (!VMMDLL_MemReadEx(DmaData::vHandle,
                        this->win_logon_pid |
                            VMMDLL_PID_PROCESS_WITH_KERNELMEMORY,
                        gptCursorAsyncExport, reinterpret_cast<PBYTE>(&pt),
                        sizeof(pt), NULL, VMMDLL_FLAG_NOCACHE)) {
    LOG("读取失败了\n");
    return;
  }
  // printf("mouse %d %d\n", pt.x, pt.y);
  mouse_positionx = pt.x;
  mouse_positiony = pt.y;
}

void c_keys::UpdateKeys() {
  uint8_t previous_key_state_bitmap[64] = {0};
  memcpy(previous_key_state_bitmap, state_bitmap, 64);

  VMMDLL_MemReadEx(DmaData::vHandle,
                   this->win_logon_pid | VMMDLL_PID_PROCESS_WITH_KERNELMEMORY,
                   gafAsyncKeyStateExport,
                   reinterpret_cast<PBYTE>(&state_bitmap), 64, NULL,
                   VMMDLL_FLAG_NOCACHE);
  for (int vk = 0; vk < 256; ++vk)
    if ((state_bitmap[(vk * 2 / 8)] & 1 << vk % 4 * 2) &&
        !(previous_key_state_bitmap[(vk * 2 / 8)] & 1 << vk % 4 * 2))
      previous_state_bitmap[vk / 8] |= 1 << vk % 8;
}

bool c_keys::IsKeyDown(uint32_t virtual_key_code) {
  if (gafAsyncKeyStateExport < 0x7FFFFFFFFFFF)
    return false;
  if (std::chrono::system_clock::now() - start >
      std::chrono::milliseconds(100)) {
    UpdateKeys();
    start = std::chrono::system_clock::now();
  }
  return state_bitmap[(virtual_key_code * 2 / 8)] &
         1 << virtual_key_code % 4 * 2;
}

void mem::scatterClear(VMMDLL_SCATTER_HANDLE h) {
  VMMDLL_Scatter_Clear(h, DmaData::PID,
                       VMMDLL_FLAG_NOCACHE | VMMDLL_FLAG_ZEROPAD_ON_FAIL);
}

bool mem::IsValidPointer(uint64_t Ptr) {
  return !(bool)(Ptr < 0x1000000 || Ptr > 0x7FFFFFF00000 ||
                 Ptr % sizeof(uint64_t));
}

void mem::scatterExec(VMMDLL_SCATTER_HANDLE h) {
  VMMDLL_Scatter_ExecuteRead(h);
}

VMMDLL_SCATTER_HANDLE mem::scatterInit() {
  return VMMDLL_Scatter_Initialize(DmaData::vHandle, DmaData::PID,
                                   VMMDLL_FLAG_NOCACHE |
                                       VMMDLL_FLAG_ZEROPAD_ON_FAIL);
}

void mem::scatterDestroy(VMMDLL_SCATTER_HANDLE h) {
  VMMDLL_Scatter_CloseHandle(h);
}

void mem::scatterPrep(VMMDLL_SCATTER_HANDLE h, uint64_t address, int size) {
  VMMDLL_Scatter_Prepare(h, address, size);
}

void mem::scatterRead(VMMDLL_SCATTER_HANDLE hS, QWORD addr, DWORD size,
                      PBYTE pb, PDWORD pcbRead) {
  VMMDLL_Scatter_Read(hS, addr, size, pb, pcbRead);
}

void mem::RefreshConfigSet() {
  VMMDLL_ConfigSet(DmaData::vHandle, VMMDLL_OPT_REFRESH_FREQ_MEM_PARTIAL, 1);
  if (!isCr3) {
    VMMDLL_ConfigSet(DmaData::vHandle, VMMDLL_OPT_REFRESH_FREQ_TLB_PARTIAL, 1);
  }
}
void mem::RefreshConfigSetALL() {
  VMMDLL_ConfigSet(DmaData::vHandle, VMMDLL_OPT_REFRESH_ALL, 1);
}

std::string mem::RegistryQueryValue(std::string path, DWORD type) {
  BYTE buffer[0x128];
  DWORD _type = (DWORD)type;
  DWORD size = sizeof(buffer);

  if (!VMMDLL_WinReg_QueryValueExU(DmaData::vHandle, (LPSTR)path.c_str(),
                                   &_type, buffer, &size)) {
    return std::string();
  }

  std::wstring wstr = std::wstring((wchar_t *)buffer);
  return std::string(wstr.begin(), wstr.end());
}

std::vector<int> mem::GetPidList(std::string name) {
  PVMMDLL_PROCESS_INFORMATION process_info = NULL;
  DWORD total_processes = 0;
  std::vector<int> list = {};

  if (!VMMDLL_ProcessGetInformationAll(DmaData::vHandle, &process_info,
                                       &total_processes)) {
    return list;
  }

  for (size_t i = 0; i < total_processes; i++) {
    auto process = process_info[i];
    if (strstr(process.szNameLong, name.c_str()))
      list.push_back(process.dwPID);
  }

  return list;
}

bool mem::InitRemoteKeyDetection() {
  std::string win = RegistryQueryValue(
      "HKLM\\SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion\\CurrentBuild",
      REG_SZ);
  int Winver = 0;
  if (win.empty()) {
    return false;
  }
  Winver = stoi(win);
  win_logon_pid = Get_Process_Id("winlogon.exe");
  if (Winver > 22000) {
    auto pids = GetPidList("csrss.exe");
    for (size_t i = 0; i < pids.size(); i++) {
      auto pid = pids[i];
      uintptr_t tmp = VMMDLL_ProcessGetModuleBaseU(DmaData::vHandle, pid,
                                                   (LPSTR) "win32ksgd.sys");
      uintptr_t g_session_global_slots = tmp + 0x3110;
      uintptr_t user_session_state = RAM<uintptr_t>(
          pid,
          RAM<uintptr_t>(pid, RAM<uintptr_t>(pid, g_session_global_slots)));
      gafAsyncKeyStateExport = user_session_state + 0x3690;
      if (gafAsyncKeyStateExport > 0x7FFFFFFFFFFF)
        break;
    }
    if (gafAsyncKeyStateExport > 0x7FFFFFFFFFFF)
      return true;
    return false;
  } else {
    PVMMDLL_MAP_EAT eat_map = NULL;
    PVMMDLL_MAP_EATENTRY eat_map_entry;
    bool result = VMMDLL_Map_GetEATU(DmaData::vHandle,
                                     Get_Process_Id("winlogon.exe") |
                                         VMMDLL_PID_PROCESS_WITH_KERNELMEMORY,
                                     (LPSTR) "win32kbase.sys", &eat_map);
    if (!result)
      return false;

    if (eat_map->dwVersion != VMMDLL_MAP_EAT_VERSION) {
      VMMDLL_MemFree(eat_map);
      eat_map_entry = NULL;
      return false;
    }

    for (int i = 0; i < eat_map->cMap; i++) {
      eat_map_entry = eat_map->pMap + i;
      if (strcmp(eat_map_entry->uszFunction, "gafAsyncKeyState") == 0) {
        gafAsyncKeyStateExport = eat_map_entry->vaFunction;
        break;
      }
    }

    VMMDLL_MemFree(eat_map);
    eat_map = NULL;
    if (gafAsyncKeyStateExport > 0x7FFFFFFFFFFF)
      return true;
    return false;
  }
}

uint64_t mem::FindAddr(uint64_t addr) {
  // 第一步：读缓存（共享锁）
  {
    std::shared_lock lock(mutexDec); // RAII锁 (C++17)
    if (auto it = valist.find(addr); it != valist.end()) {
      return it->second; // 命中缓存直接返回
    }
  }

  // 第二步：物理转换（无锁）
  ULONG64 physicalAddr = 0;
  if (!VMMDLL_MemVirt2Phys(DmaData::vHandle, DmaData::PID, addr,
                           &physicalAddr)) {
    return 0; // 转换失败
  }

  // 第三步：写入缓存（独占锁+双重检查）
  {
    std::unique_lock lock(mutexDec);

    // 双重检查：可能其他线程已写入
    if (auto it = valist.find(addr); it != valist.end()) {
      return it->second;
    }

    auto [it, inserted] = valist.emplace(addr, physicalAddr);
    return it->second;
  }

  return physicalAddr;
}

bool mem::Readdata(uint64_t addr, void *buffer, _In_ DWORD cb) {
  return VMMDLL_MemRead(DmaData::vHandle, DmaData::PID, addr, (PBYTE)buffer,
                        cb);
}
