// UnityExplorer - Unity Object Browser (for learning)
// Visual Unity memory exploration tool based on er2 library and ImGui

#include <algorithm>
#include <atomic>
#include <cctype>
#include <chrono>
#include <cmath>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <fstream>
#include <memory>
#include <mutex>
#include <optional>
#include <sstream>
#include <string>
#include <thread>
#include <vector>

// =============================================================================
// Strategy 7 Offsets
// =============================================================================
// [Offsets]
#include "GameOffsets.h"
#include <vector>

// Windows headers - Windows.h MUST come before TlHelp32.h
// Windows 头文件

// 2. 必须最先包含 Windows.h
#include <Windows.h>

// 3. 然后才是依赖 Windows.h 的头文件
#include <TlHelp32.h>
#include <d3d11.h>
#include <tchar.h>

// 4. 最后是项目自定义头文件和第三方库
#include "../../include/er2/unity2/init.hpp" // Direct include to avoid path issues
#include "MetickAdapter.hpp"                 // New Metick DMA Adapter
#include "er2/os/dma/dma_memory_accessor.hpp"
#include "er2/unity2/headless/manager_rva_scan.hpp"
#include "er2/unity2/init/module_match.hpp"

// Legacy imports (may remove later if unused)
#include "../../deps/Memory/libs/vmmdll.h" // Use unified vmmdll.h
#include "../../deps/memprocfs/includes/leechcore.h"

#include "../../deps/UnityResolve.hpp/External/Camera/UnityExternalCamera.hpp"
#include "../../deps/UnityResolve.hpp/External/Camera/UnityExternalWorldToScreen.hpp"
#include "../../deps/UnityResolve.hpp/External/GameObjectManager/Native/NativeComponent.hpp"
#include "../../deps/UnityResolve.hpp/External/GameObjectManager/Native/NativeGameObject.hpp"
#include "../../include/er2/unity2/core/reflection.hpp" // Corrected path
#include "../../include/er2/unity2/msid/msid_scan.hpp" // Added for MSID scan fix
#include "Adapters/ER2MemoryAdapter.hpp"
#include "er2/unity2/inspect/klass_header_probe.hpp"
#include "er2/unity2/inspect/native_chain_probe.hpp"
#include "er2/unity2/inspect/runtime_class_probe.hpp"
#include "er2/unity2/metadata/export.hpp"
#include "er2/unity2/metadata/il2cpp_layout_profile.hpp"
#include "er2/unity2/naraka/actor_container_probe.hpp"
#include "er2/unity2/naraka/buff_manager_probe.hpp"
#include "er2/unity2/naraka/runtime_property_probe.hpp"

// ... (existing includes)

// =============================================================================
// Class Inspector State (Redefined to ensure availability)
// =============================================================================
static bool g_showClassInspector = false;
static std::string g_inspectedClassTitle;
static std::vector<er2::unity2::core::ReflectionFieldInfo> g_inspectedFields;
static std::vector<er2::unity2::core::ReflectionMethodInfo> g_inspectedMethods;
static uintptr_t g_inspectedClassPtr = 0;

// DrawClassInspector implementation moved to bottom
// to ensure ImGui headers are available
#include "er2/unity2/metadata/scanner_direct.hpp"
#include "er2/unity2/object/managed/il2cpp_class.hpp" // Field Inspector support
#include "imgui.h"
#include "imgui_impl_dx11.h"
#include "imgui_impl_win32.h"

// =============================================================================
// DirectX 11 全局对象
// =============================================================================
static ID3D11Device *g_pd3dDevice = nullptr;
static ID3D11DeviceContext *g_pd3dDeviceContext = nullptr;
static IDXGISwapChain *g_pSwapChain = nullptr;
static bool g_SwapChainOccluded = false;
static UINT g_ResizeWidth = 0, g_ResizeHeight = 0;
static ID3D11RenderTargetView *g_mainRenderTargetView = nullptr;

// =============================================================================
// 辅助函数声明
// =============================================================================
// 辅助函数声明
bool CreateDeviceD3D(HWND hWnd);
void CleanupDeviceD3D();
void CreateRenderTarget();
void CleanupRenderTarget();
LRESULT WINAPI WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

// UI 函数前向声明
void DrawConnectionPanel();
void DrawGameObjectList();
void DrawGameObjectDetails();
void DrawCameraInfo();
void DrawMsidBrowser();
void DrawBonesPanel();
void DrawMetadataPanel();
void DrawDmaOffsetScanner();
void DrawNarakaBuffManagerProbe();
void DrawNarakaPropertyProbe();
void DrawNarakaActorContainerProbe();
void DrawW2SCalculator();
void DrawLogPanel();
void DrawProcessSelector();
void AddLog(const std::string &message);

static const er2::Il2CppLayoutProfile &ActiveIl2CppLayoutProfile();
static bool ProbeRuntimeClassFromUi(std::uintptr_t klass);
static bool ProbeNativeChainFromUi(std::uintptr_t native);

static bool ValidateMetadataBytes(const std::vector<uint8_t> &bytes,
                                  std::string &reason) {
  reason.clear();
  if (bytes.size() < 0x120) {
    reason = "metadata buffer is too small";
    return false;
  }

  const auto readU32 = [&bytes](std::size_t offset) -> std::uint32_t {
    return static_cast<std::uint32_t>(bytes[offset]) |
           (static_cast<std::uint32_t>(bytes[offset + 1]) << 8) |
           (static_cast<std::uint32_t>(bytes[offset + 2]) << 16) |
           (static_cast<std::uint32_t>(bytes[offset + 3]) << 24);
  };

  const std::uint32_t magic = readU32(0);
  if (magic != 0xFAB11BAFu) {
    char buf[128];
    sprintf_s(buf, "invalid metadata magic 0x%08X", magic);
    reason = buf;
    return false;
  }

  const std::uint32_t version = readU32(4);
  if (version < 10 || version > 100) {
    char buf[128];
    sprintf_s(buf, "invalid metadata version %u", version);
    reason = buf;
    return false;
  }

  return true;
}

// =============================================================================
// Unity 数据缓存结构
// =============================================================================
struct ComponentInfo {
  std::uintptr_t address;
  std::string typeName;
  std::string nameSpace;
};

struct CachedGameObject {
  std::uintptr_t address;
  std::string name;
  std::uintptr_t transformAddress;
  float worldPos[3];
  std::vector<ComponentInfo> components;
};

struct CachedCamera {
  std::uintptr_t address;
  std::string name;
  float fov;
  float nearClip;
  float farClip;
};

// Moved to AppState::MsidObjectEntry

struct BoneEntry {
  std::int32_t index;
  std::string name;
  std::uintptr_t transformAddress;
  float worldPos[3];
};

struct NarakaActorProbeEntry {
  std::uintptr_t actorModel = 0;
  std::uintptr_t propertyData = 0;
  std::uintptr_t runtimePropertyData = 0;
  std::uint32_t heroId = 0;
  std::uint32_t teamId = 0;
};

// 进程选择器数据结构
struct UnityProcessInfo {
  std::uint32_t pid;
  std::string processName;
  std::string windowTitle;
  bool isIl2Cpp;
};

// 运行模式枚举
enum class RunMode { WinAPI = 0, DMA = 1 };

// =============================================================================
// 应用程序状态
// =============================================================================
struct AppState {
  // 连接状态
  std::atomic<bool> isConnected{false};
  std::atomic<bool> isConnecting{false};
  std::string statusMessage;

  std::string runtimeType;
  RunMode runMode = RunMode::WinAPI;
  int selectedRunMode = 0;   // 0=WinAPI, 1=DMA
  bool isNarakaMode = false; // [New] Exclusive scan mode for Naraka

  // Module Info
  std::uintptr_t gameAssemblyBase = 0;
  std::uint32_t gameAssemblySize = 0;

  // [New] Manual Camera Address for Decoupled Logic
  std::uintptr_t manualCameraAddress = 0;

  struct MsidObjectEntry {
    std::uint32_t instanceId;
    std::string fullTypeName;
    std::string namespaze; // [New] Namespace (IL2CPP: Klass + 0x18)
    std::string className; // [New] Class Name Only
    std::uintptr_t nativePtr;
    std::string name; // [New] Captured Object Name
  };

  // 缓存数据
  std::vector<CachedGameObject> gameObjects;
  std::vector<CachedCamera> cameras;
  std::vector<MsidObjectEntry> msidObjects;
  std::vector<BoneEntry> bones;
  std::vector<NarakaActorProbeEntry> narakaActorProbeEntries;
  std::mutex dataMutex;

  // UI 状态
  bool showDemoWindow = false;
  bool showGameObjectList = true;
  bool showCameraInfo = true;
  bool showConnectionPanel = true;
  bool showMsidBrowser = false;
  bool showBonesPanel = false;
  bool showMetadataPanel = false;
  bool showDmaOffsetScanner = true;
  bool showNarakaBuffManagerProbe = false;
  bool showNarakaPropertyProbe = false;
  bool showNarakaActorContainerProbe = false;
  bool showW2SCalculator = false;
  bool showLogPanel = true; // 默认打开以便调试
  bool autoRefresh = false;
  float refreshInterval = 1.0f;
  float lastRefreshTime = 0.0f;

  // Manual Module Selector
  bool showModuleSelector = false;
  bool isScanningModules = false;
  struct ModuleEntry {
    std::string name;
    std::uintptr_t base;
    std::uint32_t size;
  };
  std::vector<ModuleEntry> moduleList;

  // 搜索过滤
  char searchFilter[256] = "";
  char msidTypeFilter[256] = "";
  int selectedGameObjectIndex = -1;

  // 统计信息
  int totalGameObjects = 0;
  int filteredGameObjects = 0;
  int totalMsidObjects = 0;

  // W2S 计算器数据
  float w2sWorldPos[3] = {0, 0, 0};
  float w2sScreenX = 0.0f;
  float w2sScreenY = 0.0f;
  bool w2sValid = false;

  // Log 输出
  std::vector<std::string> logMessages;
  std::mutex logMutex;

  // 进程选择器状态
  // 进程选择器状态
  std::vector<UnityProcessInfo> unityProcesses;
  bool showProcessSelector = false;
  int selectedProcessIndex = -1;

  // 扫描状态
  std::atomic<bool> isScanning{false}; // UI scanning process

  // ========== DMA Offset Scanner State ==========
  std::atomic<bool> isScanningManagerRvas{false};
  std::uint32_t managerRvaScanMaxMb = 96;
  er2::ManagerRvaScanResult managerRvaScanResult;
  std::string managerRvaScanStatus = "Not scanned";

  // ========== Naraka RuntimeProperty Probe State ==========
  char narakaProbeActorAddress[32] = "";
  char narakaProbeFilterDataId[64] = "";
  int narakaProbeMaxRows = 4096;
  int narakaProbeSelectedActor = -1;
  bool narakaProbeOnlyChanged = false;
  bool narakaProbeOnlyTarka = true;
  bool narakaProbeAutoPoll = false;
  float narakaProbePollInterval = 0.10f;
  float narakaProbeLastPollTime = 0.0f;
  er2::NarakaRuntimePropertySnapshot narakaProbeBaseline;
  er2::NarakaRuntimePropertySnapshot narakaProbeCurrent;
  std::vector<er2::NarakaRuntimePropertyDiffRow> narakaProbeDiff;
  bool narakaProbeHasBaseline = false;
  bool narakaProbeHasCurrent = false;
  std::string narakaProbeStatus = "Idle";

  // ========== Naraka BuffManager Probe State ==========
  char narakaBuffProbeFilter[64] = "";
  int narakaBuffProbeMaxEntries = 1024;
  int narakaBuffProbeScanEnd = 0x800;
  bool narakaBuffProbeOnlyChanged = true;
  bool narakaBuffProbeAutoPoll = false;
  float narakaBuffProbePollInterval = 0.10f;
  float narakaBuffProbeLastPollTime = 0.0f;
  er2::NarakaBuffManagerSnapshot narakaBuffProbeBaseline;
  er2::NarakaBuffManagerSnapshot narakaBuffProbeCurrent;
  std::vector<er2::NarakaBuffEntryDiff> narakaBuffProbeDiff;
  bool narakaBuffProbeHasBaseline = false;
  bool narakaBuffProbeHasCurrent = false;
  std::string narakaBuffProbeStatus = "Idle";

  // ========== Naraka Actor Container Probe State ==========
  char narakaActorContainerFilter[64] = "";
  int narakaActorContainerMaxEntries = 4096;
  int narakaActorContainerScanEnd = 0x1000;
  bool narakaActorContainerOnlyChanged = true;
  bool narakaActorContainerAutoPoll = false;
  float narakaActorContainerPollInterval = 0.10f;
  float narakaActorContainerLastPollTime = 0.0f;
  er2::NarakaActorContainerSnapshot narakaActorContainerBaseline;
  er2::NarakaActorContainerSnapshot narakaActorContainerCurrent;
  std::vector<er2::NarakaActorContainerDiff> narakaActorContainerDiff;
  std::vector<er2::NarakaActorContainerEntryDiff> narakaActorContainerEntryDiff;
  bool narakaActorContainerHasBaseline = false;
  bool narakaActorContainerHasCurrent = false;
  std::string narakaActorContainerStatus = "Idle";

  // 模块扫描
  bool useCustomModuleNames = false;
  char customUnityPlayerName[128] = "UnityPlayer.dll";
  char customGameAssemblyName[128] = "GameAssembly.dll";
  char gameDirectoryPath[512] = "";
  // 游戏目录路径（用于扫描 DLL）
  char targetProcessName[128] = "NarakaBladepoint.exe";

  // Strategy 7 Fallback Flag
  bool useFallbackStrategy7 = false;
  std::vector<CachedGameObject>
      fallbackObjects; // Manual object list for Strategy 7

  // ========== Field Inspector State ==========
  bool showFieldInspector = false;
  std::uintptr_t inspectedNativePtr = 0;                // Native Object 地址
  std::uintptr_t inspectedManagedPtr = 0;               // Managed Object 地址
  std::uintptr_t inspectedKlass = 0;                    // Il2CppClass 地址
  std::string inspectedClassName;                       // 类名
  std::vector<er2::Il2CppFieldExtract> inspectedFields; // 字段列表

  // ========== Runtime Class Probe State ==========
  std::uintptr_t runtimeProbeKlass = 0;
  er2::RuntimeClassProbeResult runtimeProbeResult;
  bool runtimeProbeHasResult = false;
  er2::KlassHeaderProbeResult klassHeaderProbeResult;
  bool klassHeaderProbeHasResult = false;

  // ========== Native Chain Probe State ==========
  std::uintptr_t nativeChainProbeNative = 0;
  er2::NativeChainProbeResult nativeChainProbeResult;
  bool nativeChainProbeHasResult = false;
};

static AppState g_appState;
static std::shared_ptr<MetickAdapter> g_dmaAdapter = nullptr;
static ER2MemoryAdapter *g_UnityResolveAdapter = nullptr;

static const er2::Il2CppLayoutProfile &ActiveIl2CppLayoutProfile() {
  return er2::kIl2CppUnity2019_4_Lts24_5_Profile;
}

static void LogRuntimeClassProbeResult(
    std::uintptr_t klass, const er2::RuntimeClassProbeResult &result) {
  char buf[512];
  if (!result.valid) {
    sprintf_s(buf, "[RuntimeProbe] Klass 0x%llX failed: %s",
              (unsigned long long)klass, result.failureReason.c_str());
    AddLog(buf);
    return;
  }

  sprintf_s(buf,
            "[RuntimeProbe] Klass 0x%llX => %s fields=%u methods=%u "
            "parent=0x%llX",
            (unsigned long long)klass, result.fullName.c_str(),
            result.fieldCount, result.methodCount,
            (unsigned long long)result.parent);
  AddLog(buf);
}

static void LogKlassHeaderProbeResult(
    std::uintptr_t klass, const er2::KlassHeaderProbeResult &result) {
  char buf[512];
  if (!result.valid) {
    sprintf_s(buf, "[KlassProbe] Klass 0x%llX failed: %s",
              (unsigned long long)klass, result.failureReason.c_str());
    AddLog(buf);
    return;
  }

  if (result.bestNamespace.empty()) {
    sprintf_s(buf, "[KlassProbe] Klass 0x%llX bestName +0x%X -> \"%s\"",
              (unsigned long long)klass, result.bestNameOffset,
              result.bestName.c_str());
  } else {
    sprintf_s(
        buf,
        "[KlassProbe] Klass 0x%llX bestName +0x%X -> \"%s\", namespace +0x%X "
        "-> \"%s\"",
        (unsigned long long)klass, result.bestNameOffset,
        result.bestName.c_str(), result.bestNamespaceOffset,
        result.bestNamespace.c_str());
  }
  AddLog(buf);

  int emitted = 0;
  for (const auto &entry : result.entries) {
    if (!entry.readable || entry.value.empty()) {
      continue;
    }

    sprintf_s(buf, "[KlassProbe]   +0x%X ptr=0x%llX score=%d \"%s\"",
              entry.offset, (unsigned long long)entry.pointer, entry.score,
              entry.value.c_str());
    AddLog(buf);

    ++emitted;
    if (emitted >= 16) {
      AddLog("[KlassProbe]   ... more readable entries omitted");
      break;
    }
  }
}

static bool ProbeKlassHeaderFromUi(std::uintptr_t klass) {
  if (!er2::g_ctx.memory) {
    g_appState.klassHeaderProbeResult = er2::KlassHeaderProbeResult{};
    g_appState.klassHeaderProbeResult.failureReason =
        "memory accessor is null";
    g_appState.klassHeaderProbeHasResult = true;
    AddLog("[KlassProbe] Memory accessor is null");
    return false;
  }

  g_appState.runtimeProbeKlass = klass;
  const bool ok = er2::ProbeKlassHeaderStrings(
      er2::Mem(), klass, g_appState.klassHeaderProbeResult);
  g_appState.klassHeaderProbeHasResult = true;
  LogKlassHeaderProbeResult(klass, g_appState.klassHeaderProbeResult);
  return ok;
}

static bool ProbeRuntimeClassFromUi(std::uintptr_t klass) {
  if (!er2::g_ctx.memory) {
    g_appState.runtimeProbeResult = er2::RuntimeClassProbeResult{};
    g_appState.runtimeProbeResult.failureReason = "memory accessor is null";
    g_appState.runtimeProbeHasResult = true;
    AddLog("[RuntimeProbe] Memory accessor is null");
    return false;
  }

  g_appState.runtimeProbeKlass = klass;
  const bool ok = er2::ProbeRuntimeClass(
      er2::Mem(), klass, ActiveIl2CppLayoutProfile(),
      g_appState.runtimeProbeResult);
  g_appState.runtimeProbeHasResult = true;
  LogRuntimeClassProbeResult(klass, g_appState.runtimeProbeResult);
  if (!ok) {
    (void)ProbeKlassHeaderFromUi(klass);
  }
  return ok;
}

static void LogNativeChainProbeResult(
    std::uintptr_t native, const er2::NativeChainProbeResult &result) {
  char buf[512];
  if (!result.valid) {
    sprintf_s(buf, "[NativeProbe] Native 0x%llX failed: %s",
              (unsigned long long)native, result.failureReason.c_str());
    AddLog(buf);
  } else {
    const auto &best = result.best();
    sprintf_s(buf,
              "[NativeProbe] Native 0x%llX best managed +0x%X -> 0x%llX, "
              "klass=0x%llX, type=%s",
              (unsigned long long)native, best.managedOffset,
              (unsigned long long)best.managed,
              (unsigned long long)best.klass,
              best.runtimeClass.fullName.c_str());
    AddLog(buf);
  }

  for (const auto &c : result.candidates) {
    sprintf_s(buf,
              "[NativeProbe]   +0x%X managed=0x%llX back=%s klass=0x%llX "
              "score=%d %s",
              c.managedOffset, (unsigned long long)c.managed,
              c.hasNativeBackPointer ? "yes" : "no",
              (unsigned long long)c.klass, c.score,
              c.valid ? c.runtimeClass.fullName.c_str()
                      : c.failureReason.c_str());
    AddLog(buf);
  }
}

static bool ProbeNativeChainFromUi(std::uintptr_t native) {
  if (!er2::g_ctx.memory) {
    g_appState.nativeChainProbeResult = er2::NativeChainProbeResult{};
    g_appState.nativeChainProbeResult.failureReason = "memory accessor is null";
    g_appState.nativeChainProbeHasResult = true;
    AddLog("[NativeProbe] Memory accessor is null");
    return false;
  }

  g_appState.nativeChainProbeNative = native;
  const bool ok = er2::ProbeNativeObjectChain(
      er2::Mem(), native, ActiveIl2CppLayoutProfile(),
      g_appState.nativeChainProbeResult);
  g_appState.nativeChainProbeHasResult = true;
  LogNativeChainProbeResult(native, g_appState.nativeChainProbeResult);
  if (!ok && g_appState.nativeChainProbeResult.bestIndex >= 0) {
    const auto &best = g_appState.nativeChainProbeResult.best();
    if (best.klassReadable) {
      (void)ProbeKlassHeaderFromUi(best.klass);
    }
  }
  return ok;
}

// =============================================================================
// 辅助函数
// =============================================================================

// 启用调试权限 (绕过反调试保护)
bool EnableDebugPrivilege() {
  HANDLE hToken;
  if (!OpenProcessToken(GetCurrentProcess(),
                        TOKEN_ADJUST_PRIVILEGES | TOKEN_QUERY, &hToken)) {
    return false;
  }

  LUID luid;
  if (!LookupPrivilegeValueW(NULL, SE_DEBUG_NAME, &luid)) {
    CloseHandle(hToken);
    return false;
  }

  TOKEN_PRIVILEGES tp;
  tp.PrivilegeCount = 1;
  tp.Privileges[0].Luid = luid;
  tp.Privileges[0].Attributes = SE_PRIVILEGE_ENABLED;

  bool success =
      AdjustTokenPrivileges(hToken, FALSE, &tp, sizeof(tp), nullptr, nullptr);
  CloseHandle(hToken);

  return success && (GetLastError() == ERROR_SUCCESS);
}
void AddLog(const std::string &message) {
  std::lock_guard<std::mutex> lock(g_appState.logMutex);
  g_appState.logMessages.push_back(message);
  if (g_appState.logMessages.size() > 1000) {
    g_appState.logMessages.erase(g_appState.logMessages.begin());
  }
}

// Copy text to Windows clipboard
void CopyToClipboard(const std::string &text) {
  if (OpenClipboard(nullptr)) {
    EmptyClipboard();

    HGLOBAL hMem = GlobalAlloc(GMEM_MOVEABLE, text.size() + 1);
    if (hMem) {
      memcpy(GlobalLock(hMem), text.c_str(), text.size() + 1);
      GlobalUnlock(hMem);
      SetClipboardData(CF_TEXT, hMem);
    }

    CloseClipboard();
  }
}

// 宽字符转多字节字符串
std::string WideToUtf8(const std::wstring &wstr) {
  if (wstr.empty())
    return "";
  int size = WideCharToMultiByte(CP_UTF8, 0, wstr.c_str(), -1, nullptr, 0,
                                 nullptr, nullptr);
  if (size <= 0)
    return "";
  std::string result(size - 1, '\0');
  WideCharToMultiByte(CP_UTF8, 0, wstr.c_str(), -1, &result[0], size, nullptr,
                      nullptr);
  return result;
}

// 获取进程窗口标题
std::string GetProcessWindowTitle(std::uint32_t pid) {
  struct EnumData {
    std::uint32_t targetPid;
    std::wstring title;
  } data = {pid, L""};

  EnumWindows(
      [](HWND hwnd, LPARAM lParam) -> BOOL {
        EnumData *pData = reinterpret_cast<EnumData *>(lParam);
        DWORD windowPid = 0;
        GetWindowThreadProcessId(hwnd, &windowPid);
        if (windowPid == pData->targetPid && IsWindowVisible(hwnd)) {
          wchar_t title[256] = {};
          GetWindowTextW(hwnd, title, 256);
          if (wcslen(title) > 0) {
            pData->title = title;
            return FALSE; // 找到后停止枚举
          }
        }
        return TRUE;
      },
      reinterpret_cast<LPARAM>(&data));

  return WideToUtf8(data.title);
}

// 枚举所有系统进程（不过滤，让用户自行选择）
std::vector<UnityProcessInfo> EnumerateAllProcesses() {
  std::vector<UnityProcessInfo> result;

  HANDLE snapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
  if (snapshot == INVALID_HANDLE_VALUE) {
    return result;
  }

  PROCESSENTRY32W entry;
  entry.dwSize = sizeof(entry);

  if (Process32FirstW(snapshot, &entry)) {
    do {
      std::uint32_t pid = static_cast<std::uint32_t>(entry.th32ProcessID);

      // 跳过系统进程 (PID 0 和 4)
      if (pid == 0 || pid == 4) {
        continue;
      }

      // 尝试获取进程信息（需要权限）
      HANDLE hProc = OpenProcess(PROCESS_QUERY_LIMITED_INFORMATION, FALSE, pid);
      if (!hProc) {
        continue; // 无权限访问，跳过
      }
      CloseHandle(hProc);

      UnityProcessInfo info;
      info.pid = pid;
      info.processName = WideToUtf8(entry.szExeFile);
      info.windowTitle = GetProcessWindowTitle(pid);

      // 检测是否包含 Unity 模块（用于标记，不强制过滤）
      er2::ModuleInfo up;
      bool hasUnityPlayer =
          er2::GetRemoteModuleInfo(pid, L"UnityPlayer.dll", up) && up.base != 0;

      er2::ModuleInfo ga;
      bool hasGameAssembly =
          er2::GetRemoteModuleInfo(pid, L"GameAssembly.dll", ga) &&
          ga.base != 0;

      if (hasUnityPlayer) {
        info.isIl2Cpp = hasGameAssembly;
      } else {
        info.isIl2Cpp = false;
      }

      // 添加标记：是否是已知 Unity 进程
      // 使用 windowTitle 字段前缀标记（临时方案）
      if (hasUnityPlayer) {
        info.windowTitle = "[Unity] " + info.windowTitle;
      }

      result.push_back(info);
    } while (Process32NextW(snapshot, &entry));
  }

  CloseHandle(snapshot);
  return result;
}

// 枚举指定进程加载的所有模块
void ListProcessModules(std::uint32_t pid) {
  AddLog("========================================");
  char header[128];
  sprintf_s(header, "[INFO] Listing modules for PID: %u", pid);
  AddLog(header);
  AddLog("----------------------------------------");

  HANDLE snapshot =
      CreateToolhelp32Snapshot(TH32CS_SNAPMODULE | TH32CS_SNAPMODULE32, pid);
  if (snapshot == INVALID_HANDLE_VALUE) {
    AddLog("[ERROR] Failed to create module snapshot (access denied?)");
    return;
  }

  MODULEENTRY32W me;
  me.dwSize = sizeof(me);

  int count = 0;
  if (Module32FirstW(snapshot, &me)) {
    do {
      std::string moduleName = WideToUtf8(me.szModule);
      char buf[512];
      sprintf_s(buf, "  [%d] %s (base: 0x%llX, size: 0x%X)", count++,
                moduleName.c_str(),
                reinterpret_cast<unsigned long long>(me.modBaseAddr),
                me.modBaseSize);
      AddLog(buf);

      // 高亮 Unity 相关模块
      std::string lower = moduleName;
      for (auto &c : lower)
        c = static_cast<char>(tolower(c));
      if (lower.find("unity") != std::string::npos ||
          lower.find("gameassembly") != std::string::npos ||
          lower.find("mono") != std::string::npos ||
          lower.find("il2cpp") != std::string::npos) {
        AddLog("       ^ [UNITY-RELATED MODULE]");
      }
    } while (Module32NextW(snapshot, &me));
  }

  CloseHandle(snapshot);

  sprintf_s(header, "[INFO] Total: %d modules loaded", count);
  AddLog(header);
  AddLog("========================================");
}

// 扫描游戏目录的 DLL 文件（绕过进程保护）
void ScanGameDirectory(const std::string &gamePath) {
  AddLog("========================================");
  AddLog("[INFO] Scanning game directory for Unity modules...");
  AddLog("----------------------------------------");

  std::string searchPath = gamePath;
  if (!searchPath.empty() && searchPath.back() != '\\' &&
      searchPath.back() != '/') {
    searchPath += "\\";
  }
  searchPath += "*.dll";

  WIN32_FIND_DATAA findData;
  HANDLE hFind = FindFirstFileA(searchPath.c_str(), &findData);

  if (hFind == INVALID_HANDLE_VALUE) {
    AddLog("[ERROR] Failed to scan directory - check path");
    return;
  }

  std::vector<std::string> unityModules;
  std::vector<std::string> allDlls;

  do {
    std::string fileName = findData.cFileName;
    allDlls.push_back(fileName);

    // 检查是否是 Unity 相关模块
    std::string lower = fileName;
    for (auto &c : lower)
      c = static_cast<char>(tolower(c));

    bool isUnityRelated = false;
    std::string category;

    // 检测 UnityPlayer 变体
    if (lower.find("unityplayer") != std::string::npos ||
        lower.find("unity_player") != std::string::npos ||
        lower.find("player") != std::string::npos &&
            lower.find("unity") != std::string::npos) {
      isUnityRelated = true;
      category = "UnityPlayer";
    }
    // 检测 GameAssembly 变体
    else if (lower.find("gameassembly") != std::string::npos ||
             lower.find("assembly") != std::string::npos) {
      isUnityRelated = true;
      category = "IL2CPP";
    }
    // 检测 Mono
    else if (lower.find("mono") != std::string::npos) {
      isUnityRelated = true;
      category = "Mono";
    }
    // 检测 IL2CPP
    else if (lower.find("il2cpp") != std::string::npos) {
      isUnityRelated = true;
      category = "IL2CPP";
    }

    if (isUnityRelated) {
      char buf[512];
      LARGE_INTEGER fileSize;
      fileSize.LowPart = findData.nFileSizeLow;
      fileSize.HighPart = findData.nFileSizeHigh;

      sprintf_s(buf, "  [%s] %s (Size: %.2f MB)", category.c_str(),
                fileName.c_str(),
                static_cast<double>(fileSize.QuadPart) / (1024.0 * 1024.0));
      AddLog(buf);
      unityModules.push_back(fileName);
    }
  } while (FindNextFileA(hFind, &findData));

  FindClose(hFind);

  AddLog("----------------------------------------");

  if (unityModules.empty()) {
    AddLog("[WARN] No Unity-related DLLs found");
    AddLog("[HINT] This may not be a Unity game directory");
  } else {
    char buf[256];
    sprintf_s(buf, "[INFO] Found %zu Unity-related DLLs:", unityModules.size());
    AddLog(buf);
    for (const auto &m : unityModules) {
      AddLog("  - " + m);
    }
    AddLog("");
    AddLog("[HINT] Try these module names in the custom fields:");

    // 智能建议
    for (const auto &m : unityModules) {
      std::string lower = m;
      for (auto &c : lower)
        c = static_cast<char>(tolower(c));
      if (lower.find("player") != std::string::npos) {
        AddLog("  UnityPlayer DLL: " + m);
      }
      if (lower.find("assembly") != std::string::npos) {
        AddLog("  GameAssembly DLL: " + m);
      }
    }
  }

  char summary[256];
  sprintf_s(summary, "[INFO] Total DLLs in directory: %zu", allDlls.size());
  AddLog(summary);
  AddLog("========================================");
}

// 自定义模块名初始化（支持魔改游戏）
bool InitSettingsWithCustomModules(std::uint32_t pid,
                                   const wchar_t *unityPlayerName,
                                   const wchar_t *gameAssemblyName) {
  er2::ResetContext();
  er2::g_ctx.pid = pid;

  // [Fix] DMA Mode Support
  if (g_appState.runMode == RunMode::DMA) {
    if (!g_dmaAdapter) {
      AddLog("[ERROR] DMA adapter not initialized");
      return false;
    }

    std::string sUnityPlayer = WideToUtf8(unityPlayerName);
    std::string sGameAssembly = WideToUtf8(gameAssemblyName);

    // 查找 UnityPlayer
    auto modules = g_dmaAdapter->FindModulesByKeyword(sUnityPlayer);
    bool foundPlayer = false;
    for (const auto &mod : modules) {
      // 简单匹配：只要包含名字即可，或者精确匹配
      // FindModulesByKeyword 已经是模糊匹配
      er2::g_ctx.unityPlayer.base = mod.baseAddress;
      er2::g_ctx.unityPlayer.size = mod.imageSize;
      foundPlayer = true;
      char logBuf[256];
      sprintf_s(logBuf, "[INFO] [DMA] Found '%s' at 0x%llX (Size: 0x%X)",
                mod.name.c_str(),
                static_cast<unsigned long long>(mod.baseAddress),
                mod.imageSize);
      AddLog(logBuf);
      break; // 取第一个匹配
    }

    if (!foundPlayer) {
      char logBuf[256];
      sprintf_s(logBuf, "[ERROR] [DMA] Module '%s' not found",
                sUnityPlayer.c_str());
      AddLog(logBuf);
      return false;
    }

    // 查找 GameAssembly (检测运行时)
    auto gaModules = g_dmaAdapter->FindModulesByKeyword(sGameAssembly);
    if (!gaModules.empty()) {
      er2::g_ctx.runtime = er2::ManagedBackend::Il2Cpp;
      er2::g_ctx.gameAssembly.base = gaModules.front().baseAddress;
      er2::g_ctx.gameAssembly.size = gaModules.front().imageSize;
      char logBuf[256];
      sprintf_s(logBuf, "[INFO] [DMA] Found '%s' - IL2CPP runtime detected",
                sGameAssembly.c_str());
      AddLog(logBuf);
    } else {
      er2::g_ctx.runtime = er2::ManagedBackend::Mono;
      AddLog("[INFO] [DMA] GameAssembly not found - assuming Mono runtime");
    }

    er2::g_ctx.unityPlayerRange.base = er2::g_ctx.unityPlayer.base;
    er2::g_ctx.unityPlayerRange.size = er2::g_ctx.unityPlayer.size;

    // [Fix] Maintain memory accessor in context
    er2::g_ctx.memory = g_dmaAdapter;

    return true;
  }

  // [Legacy] WinAPI Logic
  er2::g_ctx.process = er2::OpenProcessForRead(pid);
  if (!er2::g_ctx.process) {
    AddLog("[ERROR] Failed to open process (WinAPI)");
    er2::ResetContext();
    return false;
  }

  // [Fix] Setup Memory Accessor for WinAPI
  er2::g_ctx.memory =
      std::make_shared<er2::WinApiMemoryAccessor>(er2::g_ctx.process);

  // 使用自定义模块名查找 UnityPlayer
  if (!er2::GetRemoteModuleInfo(pid, unityPlayerName, er2::g_ctx.unityPlayer) ||
      !er2::g_ctx.unityPlayer.base) {
    char logBuf[256];
    sprintf_s(logBuf, "[ERROR] Module '%ls' not found in process",
              unityPlayerName);
    AddLog(logBuf);
    er2::ResetContext();
    return false;
  }

  char logBuf[256];
  sprintf_s(logBuf, "[INFO] Found '%ls' at 0x%llX", unityPlayerName,
            static_cast<unsigned long long>(er2::g_ctx.unityPlayer.base));
  AddLog(logBuf);

  // 使用自定义模块名检测运行时类型
  er2::ModuleInfo ga;
  bool isIl2Cpp =
      er2::GetRemoteModuleInfo(pid, gameAssemblyName, ga) && ga.base != 0;
  er2::g_ctx.runtime =
      isIl2Cpp ? er2::ManagedBackend::Il2Cpp : er2::ManagedBackend::Mono;

  if (isIl2Cpp) {
    sprintf_s(logBuf, "[INFO] Found '%ls' - IL2CPP runtime detected",
              gameAssemblyName);
    AddLog(logBuf);
  } else {
    AddLog("[INFO] GameAssembly not found - assuming Mono runtime");
  }

  er2::g_ctx.unityPlayerRange.base = er2::g_ctx.unityPlayer.base;
  er2::g_ctx.unityPlayerRange.size = er2::g_ctx.unityPlayer.size;

  return true;
}

// 手动按 PID 初始化连接（支持自定义模块名）
bool ManualInitByPid(std::uint32_t pid) {
  AddLog("[DEBUG] ManualInitByPid called");

  // 确定使用的模块名
  std::wstring unityPlayerName;
  std::wstring gameAssemblyName;

  if (g_appState.useCustomModuleNames) {
    // 使用自定义模块名 - 修复转换逻辑
    int len = MultiByteToWideChar(CP_ACP, 0, g_appState.customUnityPlayerName,
                                  -1, nullptr, 0);
    if (len > 0) {
      unityPlayerName.resize(len - 1); // 不包含 null 终止符
      MultiByteToWideChar(CP_ACP, 0, g_appState.customUnityPlayerName, -1,
                          &unityPlayerName[0], len);
    }

    len = MultiByteToWideChar(CP_ACP, 0, g_appState.customGameAssemblyName, -1,
                              nullptr, 0);
    if (len > 0) {
      gameAssemblyName.resize(len - 1); // 不包含 null 终止符
      MultiByteToWideChar(CP_ACP, 0, g_appState.customGameAssemblyName, -1,
                          &gameAssemblyName[0], len);
    }

    char logBuf[256];
    sprintf_s(logBuf, "[INFO] Using custom modules: %s, %s",
              g_appState.customUnityPlayerName,
              g_appState.customGameAssemblyName);
    AddLog(logBuf);
  } else {
    // 使用默认模块名
    unityPlayerName = L"UnityPlayer.dll";
    gameAssemblyName = L"GameAssembly.dll";
    AddLog("[INFO] Using default module names");
  }

  // 第一步：初始化设置
  AddLog("[DEBUG] Step 1: Calling InitSettingsWithCustomModules...");
  if (!InitSettingsWithCustomModules(pid, unityPlayerName.c_str(),
                                     gameAssemblyName.c_str())) {
    AddLog("[ERROR] InitSettingsWithCustomModules failed");
    return false;
  }
  AddLog("[DEBUG] Step 1: InitSettingsWithCustomModules succeeded");

  // 第二步：扫描 GOM 偏移
  AddLog("[DEBUG] Step 2: Scanning GOM slot...");

  // [Fix] Dynamically select correct accessor based on RunMode
  er2::IMemoryAccessor *pMem = nullptr;
  std::unique_ptr<er2::WinApiMemoryAccessor> winApiAccessor = nullptr;

  if (g_appState.runMode == RunMode::DMA) {
    if (!g_dmaAdapter) {
      AddLog("[ERROR] DMA mode selected but g_dmaAdapter is null");
      return false;
    }
    pMem = g_dmaAdapter.get();
    AddLog("[DEBUG] Using DMA Memory Accessor");
  } else {
    // [Fix] Ensure we use the accessor already created or create one
    if (!er2::g_ctx.memory) {
      er2::g_ctx.memory =
          std::make_shared<er2::WinApiMemoryAccessor>(er2::g_ctx.process);
    }
    pMem = er2::g_ctx.memory.get();
    AddLog("[DEBUG] Using Context Memory Accessor");
  }

  std::uint64_t gomSlotRva = 0;

  char logBuf[256];
  sprintf_s(logBuf, "[DEBUG] UnityPlayer base: 0x%llX, size: 0x%X",
            static_cast<unsigned long long>(er2::g_ctx.unityPlayer.base),
            er2::g_ctx.unityPlayer.size);
  AddLog(logBuf);

  if (!er2::FindGomGlobalSlotRvaByScan(*pMem, er2::g_ctx.unityPlayer.base,
                                       er2::g_ctx.gomOff, gomSlotRva)) {
    AddLog("[ERROR] GOM scan failed - this is the critical failure point");
    AddLog("[HINT] Possible reasons:");
    AddLog("  1. The game uses a heavily modified Unity engine");
    AddLog("  2. The GameObjectManager structure has been changed");
    AddLog("  3. Anti-cheat/obfuscation is preventing memory access");
    er2::ResetContext();
    return false;
  }

  er2::g_ctx.gomGlobalSlotRva = gomSlotRva;
  er2::g_ctx.gomGlobalSlotVa =
      er2::g_ctx.unityPlayer.base + static_cast<std::uintptr_t>(gomSlotRva);

  sprintf_s(logBuf, "[DEBUG] GOM slot found at RVA: 0x%llX, VA: 0x%llX",
            gomSlotRva,
            static_cast<unsigned long long>(er2::g_ctx.gomGlobalSlotVa));
  AddLog(logBuf);
  AddLog("[SUCCESS] GOM slot found successfully");

  // 第三步：MSID 扫描（可选）
  AddLog("[DEBUG] Step 3: Scanning MSID slot (optional)...");
  std::uintptr_t msIdSlotVa = 0;
  if (er2::FindMsIdToPointerSlotVaByScan(*pMem, er2::g_ctx.unityPlayer,
                                         er2::g_ctx.gomOff, msIdSlotVa,
                                         nullptr)) {
    er2::g_ctx.msIdToPointerSlotVa = msIdSlotVa;
    er2::g_ctx.msIdToPointerSlotRva =
        static_cast<std::uint64_t>(msIdSlotVa - er2::g_ctx.unityPlayer.base);
    sprintf_s(logBuf, "[DEBUG] MSID slot found at VA: 0x%llX",
              static_cast<unsigned long long>(msIdSlotVa));
    AddLog(logBuf);
  } else {
    AddLog("[WARN] MSID slot not found (some features may not work)");
  }

  // UnityResolve Integration
  std::shared_ptr<er2::IMemoryAccessor> currentAccessor = nullptr;
  if (g_appState.runMode == RunMode::DMA) {
    if (g_dmaAdapter)
      currentAccessor = g_dmaAdapter;
  } else {
    currentAccessor = er2::g_ctx.memory;
  }

  if (currentAccessor) {
    if (!g_UnityResolveAdapter) {
      g_UnityResolveAdapter = new ER2MemoryAdapter(currentAccessor);
      UnityExternal::SetGlobalMemoryAccessor(g_UnityResolveAdapter);
      AddLog("[INFO] UnityResolve adapter initialized");
    } else {
      g_UnityResolveAdapter->UpdateAccessor(currentAccessor);
      AddLog("[INFO] UnityResolve adapter updated");
    }
  } else {
    AddLog("[WARN] Failed to init UnityResolve adapter: No memory accessor");
  }

  AddLog("[SUCCESS] Connection initialized successfully");
  return true;
}

// 自定义启发式 GOM 扫描器 (带详细日志)
std::uint64_t FindGomRuleBased(const er2::IMemoryAccessor &mem,
                               std::uintptr_t base,
                               const er2::GomOffsets &off) {
  AddLog("[DEBUG] Starting Rule-Based GOM Scan...");

  // 1. 读取 PE 头获取区段信息
  IMAGE_DOS_HEADER dosHeader{};
  if (!mem.Read(base, &dosHeader, sizeof(dosHeader))) {
    AddLog("[ERROR] Failed to read DOS header");
    return 0;
  }

  IMAGE_NT_HEADERS64 ntHeaders{};
  if (!mem.Read(base + dosHeader.e_lfanew, &ntHeaders, sizeof(ntHeaders))) {
    AddLog("[ERROR] Failed to read NT headers");
    return 0;
  }

  std::uintptr_t secBase = base + dosHeader.e_lfanew + sizeof(DWORD) +
                           sizeof(IMAGE_FILE_HEADER) +
                           ntHeaders.FileHeader.SizeOfOptionalHeader;

  std::vector<IMAGE_SECTION_HEADER> sections;
  for (WORD i = 0; i < ntHeaders.FileHeader.NumberOfSections; i++) {
    IMAGE_SECTION_HEADER sec{};
    if (mem.Read(secBase + i * sizeof(IMAGE_SECTION_HEADER), &sec,
                 sizeof(sec))) {
      sections.push_back(sec);
    }
  }

  char logBuf[512];
  sprintf_s(logBuf, "[DEBUG] Scanned %zu sections", sections.size());
  AddLog(logBuf);

  // 2. 遍历区段
  for (const auto &sec : sections) {
    char name[9] = {};
    memcpy(name, sec.Name, 8);
    std::string sName = name;

    // 过滤: 允许 .data, .rdata, .vmp*, _RDATA, .bss
    bool wanted = false;
    std::string lower = sName;
    for (auto &c : lower)
      c = tolower(c);

    if (lower.find(".data") != std::string::npos ||
        lower.find(".rdata") != std::string::npos ||
        lower.find(".vmp") != std::string::npos ||
        lower.find("_rdata") != std::string::npos ||
        lower.find(".bss") != std::string::npos) {
      wanted = true;
    }

    // 可选：允许所有可读区段，只要不是 .text (代码段通常不放 GOM 指针)
    if (lower.find(".text") != std::string::npos)
      wanted = false;

    if (!wanted) {
      // Un-comment to see skipped sections
      // sprintf_s(logBuf, "[DEBUG] Skipping section: %s", sName.c_str());
      // AddLog(logBuf);
      continue;
    }

    sprintf_s(logBuf, "[DEBUG] Scanning section: %s (RVA: 0x%X, Size: 0x%X)",
              sName.c_str(), sec.VirtualAddress, sec.Misc.VirtualSize);
    AddLog(logBuf);

    // 3. 扫描逻辑
    std::uintptr_t startVa = base + sec.VirtualAddress;
    std::uint32_t size = sec.Misc.VirtualSize;
    // Cap size if needed
    if (size > 50 * 1024 * 1024)
      size = 50 * 1024 * 1024; // Limit to 50MB per section

    const size_t CHUNK_SIZE = 0x10000; // 64KB chunks
    std::vector<uint8_t> buffer(CHUNK_SIZE);

    for (size_t offset = 0; offset < size; offset += CHUNK_SIZE) {
      size_t readSize = (std::min)(CHUNK_SIZE, (size_t)(size - offset));
      if (!mem.Read(startVa + offset, buffer.data(), readSize)) {
        continue;
      }

      // Iterate pointers
      for (size_t i = 0; i < readSize; i += 8) {
        uintptr_t ptrCandidate =
            *reinterpret_cast<uintptr_t *>(buffer.data() + i);

        // Basic sanity check
        if (ptrCandidate < 0x10000 || (ptrCandidate & 7) != 0)
          continue;

        // 调用 er2 的验证函数
        auto res = er2::CheckGameObjectManagerCandidateBlindScan(
            mem, ptrCandidate, off);
        if (res.ok) {
          uint64_t foundRva = (std::uint64_t)(sec.VirtualAddress + offset + i);
          sprintf_s(logBuf,
                    "[SUCCESS] Found Candidate! RVA: 0x%llX, Manager: 0x%llX, "
                    "Score: %d",
                    (unsigned long long)foundRva,
                    (unsigned long long)ptrCandidate, res.score);
          AddLog(logBuf);
          return foundRva;
        }
      }
    }
  }

  AddLog("[INFO] Rule-Based Scan finished with no result.");
  return 0;
}
// 策略三：数据流扫描 (MainCamera 锚点回溯)
std::uint64_t ScanGomByDataPattern(const er2::IMemoryAccessor &mem,
                                   std::uintptr_t base) {
  AddLog("[DEBUG] Starting Data Pattern GOM Scan (MainCamera Anchor)...");

  // 1. 读取区段信息 (复用代码，实际应该封装)
  IMAGE_DOS_HEADER dosHeader{};
  if (!mem.Read(base, &dosHeader, sizeof(dosHeader)))
    return 0;
  IMAGE_NT_HEADERS64 ntHeaders{};
  if (!mem.Read(base + dosHeader.e_lfanew, &ntHeaders, sizeof(ntHeaders)))
    return 0;

  std::uintptr_t secBase = base + dosHeader.e_lfanew + sizeof(DWORD) +
                           sizeof(IMAGE_FILE_HEADER) +
                           ntHeaders.FileHeader.SizeOfOptionalHeader;

  std::vector<IMAGE_SECTION_HEADER> sections;
  for (WORD i = 0; i < ntHeaders.FileHeader.NumberOfSections; i++) {
    IMAGE_SECTION_HEADER sec{};
    if (mem.Read(secBase + i * sizeof(IMAGE_SECTION_HEADER), &sec,
                 sizeof(sec))) {
      sections.push_back(sec);
    }
  }

  // 2. 扫描数据段 (.data, .bss, .vmp, _RDATA)
  for (const auto &sec : sections) {
    char name[9] = {};
    memcpy(name, sec.Name, 8);
    std::string sName = name;
    std::string lower = sName;
    for (auto &c : lower)
      c = tolower(c);

    bool wanted = false;
    if (lower.find(".data") != std::string::npos ||
        lower.find(".bss") != std::string::npos ||
        lower.find(".vmp") != std::string::npos ||
        lower.find("_rdata") != std::string::npos) {
      wanted = true;
    }
    // UnityPlayer often has GOM in .data or .bss (if dynamic)
    if (!wanted)
      continue;

    AddLog((std::string("[DEBUG] Pattern Scanning section: ") + sName).c_str());

    std::uintptr_t startVa = base + sec.VirtualAddress;
    std::uint32_t size = sec.Misc.VirtualSize;
    if (size > 100 * 1024 * 1024)
      size = 100 * 1024 * 1024; // Cap at 100MB

    const size_t CHUNK_SIZE = 0x10000;
    std::vector<uint8_t> buffer(CHUNK_SIZE);

    for (size_t offset = 0; offset < size; offset += CHUNK_SIZE) {
      size_t readSize = (std::min)(CHUNK_SIZE, (size_t)(size - offset));
      if (!mem.Read(startVa + offset, buffer.data(), readSize))
        continue;

      for (size_t i = 0; i < readSize; i += 8) {
        std::uintptr_t candidatePtr =
            *reinterpret_cast<std::uintptr_t *>(buffer.data() + i);

        // 2.1 指针有效性检查
        if (candidatePtr < 0x100000 || (candidatePtr & 7) != 0)
          continue;

        // 2.2 Offset-Agnostic Check (Strategy 4)
        // Read first 0x200 bytes of the candidate object
        std::vector<uint8_t> objBuf(0x200);
        if (!mem.Read(candidatePtr, objBuf.data(), 0x200))
          continue;

        bool foundMainCameraName = false;
        int foundNameOffset = -1;

        // Iterate potential pointers within the object
        for (int j = 0; j < 0x200; j += 8) {
          std::uintptr_t potentialNamePtr =
              *reinterpret_cast<std::uintptr_t *>(objBuf.data() + j);

          // Basic pointer structure check
          if (potentialNamePtr < 0x10000 || (potentialNamePtr & 7) != 0)
            continue;

          // Try to read string
          char nameBuf[64] = {};
          if (mem.Read(potentialNamePtr, nameBuf, 63)) {
            // Ensure null termination
            nameBuf[63] = '\0';
            if (strstr(nameBuf, "MainCamera")) {
              foundMainCameraName = true;
              foundNameOffset = j;
              break;
            }
          }
        }

        if (foundMainCameraName) {
          // Found MainCamera!
          char log[256];
          sprintf_s(log,
                    "[SUCCESS] Found MainCamera at 0x%llX (NameOffset: 0x%X)",
                    (unsigned long long)candidatePtr, foundNameOffset);
          AddLog(log);

          // 2.4 回溯 GOM
          // 当前地址是 LastTaggedObject[5]
          // GOM 可能就在附近。
          // 按照用户提示：TaggedObjectCache_Base = Addr - (5 * 8)
          std::uintptr_t ptrSlotAddr = startVa + offset + i;
          std::uintptr_t taggedBaseSlot = ptrSlotAddr - (5 * 8);

          // 验证 LastTaggedObject[0] (should be valid or 0)
          std::uintptr_t tag0Ptr = 0;
          mem.Read(taggedBaseSlot, &tag0Ptr, sizeof(tag0Ptr));

          sprintf_s(
              log,
              "[INFO] Calculated TaggedBaseSlot: 0x%llX (Tag0 Ptr: 0x%llX)",
              (unsigned long long)taggedBaseSlot, (unsigned long long)tag0Ptr);
          AddLog(log);

          // 返回 TaggedBaseSlot 的 RVA
          return taggedBaseSlot - base;
        }
      }
    }
  }

  return 0;
}

// 策略五：反客为主 (String -> Pointer -> Object)
std::uint64_t ScanGomByStringReverse(const er2::IMemoryAccessor &mem,
                                     std::uintptr_t base) {
  AddLog("[DEBUG] Starting Reverse String GOM Scan (Strategy 5)...");

  // 1. 获取所有区段
  IMAGE_DOS_HEADER dosHeader{};
  if (!mem.Read(base, &dosHeader, sizeof(dosHeader)))
    return 0;

  IMAGE_NT_HEADERS64 ntHeaders{};
  if (!mem.Read(base + dosHeader.e_lfanew, &ntHeaders, sizeof(ntHeaders)))
    return 0;

  std::uintptr_t secBase = base + dosHeader.e_lfanew + sizeof(DWORD) +
                           sizeof(IMAGE_FILE_HEADER) +
                           ntHeaders.FileHeader.SizeOfOptionalHeader;

  std::vector<IMAGE_SECTION_HEADER> sections;
  for (WORD i = 0; i < ntHeaders.FileHeader.NumberOfSections; i++) {
    IMAGE_SECTION_HEADER sec{};
    if (mem.Read(secBase + i * sizeof(IMAGE_SECTION_HEADER), &sec,
                 sizeof(sec))) {
      sections.push_back(sec);
    }
  }

  std::vector<std::uintptr_t> stringCandidates;

  // 2. 第一步：搜索 "MainCamera" 字符串地址
  AddLog("[DEBUG] Step 1: Scanning for 'MainCamera' string...");
  const char *targetStr = "MainCamera";
  const size_t targetLen = strlen(targetStr); // 10
  const size_t CHUNK_SIZE = 0x10000;
  std::vector<uint8_t> buffer(CHUNK_SIZE);

  for (const auto &sec : sections) {
    // 字符串通常在 .rdata 或 .data，但也可能在 .vmp
    // 扫描所有可读区段
    std::uintptr_t startVa = base + sec.VirtualAddress;
    std::uint32_t size = sec.Misc.VirtualSize;
    if (size > 100 * 1024 * 1024)
      size = 100 * 1024 * 1024;

    for (size_t offset = 0; offset < size; offset += CHUNK_SIZE) {
      size_t readSize = (std::min)(CHUNK_SIZE, (size_t)(size - offset));
      if (!mem.Read(startVa + offset, buffer.data(), readSize))
        continue;

      // 暴力搜索字符串
      for (size_t i = 0; i < readSize - targetLen; ++i) {
        if (memcmp(buffer.data() + i, targetStr, targetLen) == 0) {
          // 检查结尾是否为 \0 (更严格)
          if (i + targetLen < readSize && buffer[i + targetLen] == '\0') {
            std::uintptr_t strAddr = startVa + offset + i;
            stringCandidates.push_back(strAddr);
            // log limited
            if (stringCandidates.size() <= 5) {
              char log[256];
              sprintf_s(log, "  Found string at 0x%llX",
                        (unsigned long long)strAddr);
              AddLog(log);
            }
          }
        }
      }
    }
  }

  if (stringCandidates.empty()) {
    AddLog("[WARN] 'MainCamera' string not found in module memory.");
    return 0; // 失败
  }

  // 3. 第二步：搜索指向这些字符串的指针
  AddLog("[DEBUG] Step 2: Scanning for pointers to strings...");
  std::vector<std::pair<std::uintptr_t, std::uintptr_t>>
      pointerCandidates; // <PointerAddr, StrAddr>

  for (const auto &sec : sections) {
    // String Pointer 通常在堆或 .data (GameObject 结构体内)
    // 我们只扫 .data/.bss/.vmp
    char name[9] = {};
    memcpy(name, sec.Name, 8);
    std::string lower = name;
    for (auto &c : lower)
      c = tolower(c);

    bool wanted = (lower.find(".data") != std::string::npos ||
                   lower.find(".bss") != std::string::npos ||
                   lower.find(".vmp") != std::string::npos ||
                   lower.find("_rdata") != std::string::npos);
    if (!wanted)
      continue;

    std::uintptr_t startVa = base + sec.VirtualAddress;
    std::uint32_t size = sec.Misc.VirtualSize;
    if (size > 50 * 1024 * 1024)
      size = 50 * 1024 * 1024;

    for (size_t offset = 0; offset < size; offset += CHUNK_SIZE) {
      size_t readSize = (std::min)(CHUNK_SIZE, (size_t)(size - offset));
      if (!mem.Read(startVa + offset, buffer.data(), readSize))
        continue;

      for (size_t i = 0; i < readSize; i += 8) {
        std::uintptr_t val =
            *reinterpret_cast<std::uintptr_t *>(buffer.data() + i);

        // 检查 val 是否等于任何 stringCandidate
        for (auto strAddr : stringCandidates) {
          if (val == strAddr) {
            std::uintptr_t ptrAddr = startVa + offset + i;
            pointerCandidates.push_back({ptrAddr, strAddr});
            if (pointerCandidates.size() <= 5) {
              char log[256];
              sprintf_s(log, "  Found pointer at 0x%llX -> String 0x%llX",
                        (unsigned long long)ptrAddr,
                        (unsigned long long)strAddr);
              AddLog(log);
            }
          }
        }
      }
    }
  }

  if (pointerCandidates.empty()) {
    AddLog("[WARN] No pointers to 'MainCamera' string found.");
    return 0;
  }

  // 4. 第三步：以 PointerAddr 为中心，回溯寻找 Tag (5)
  AddLog("[DEBUG] Step 3: Backtracking for Tag (5)...");

  for (auto cand : pointerCandidates) {
    std::uintptr_t ptrAddr = cand.first; // 这个是 GameObject->NamePtr 的地址

    // 假设 Tag 在 NamePtr 前面 0x0 ~ 0x100 字节内
    // 读取 ptrAddr 前面的内存
    const int SEARCH_RANGE = 0x100;
    std::vector<uint8_t> backBuf(SEARCH_RANGE);
    std::uintptr_t searchStart = ptrAddr - SEARCH_RANGE;

    if (!mem.Read(searchStart, backBuf.data(), SEARCH_RANGE))
      continue;

    // 遍历寻找值为 5 的 int32/int16
    // 我们从后往前找 (靠近 NamePtr)
    for (int i = SEARCH_RANGE - 4; i >= 0; i -= 4) {
      // Check int32
      // Tag is usually uint16, but accessing as 32 is often safe or check 16
      // Unity Tag is unt16, usually aligned 4
      uint32_t val = *reinterpret_cast<uint32_t *>(backBuf.data() + i);
      if ((val & 0xFFFF) == 5) { // MainCamera Tag
        // Found Tag!
        int tagOffsetFromBack = i;
        std::uintptr_t tagAddr = searchStart + i;

        // NameOffset = ptrAddr - GameObjectBase
        // TagOffset = tagAddr - GameObjectBase
        // 仅仅依靠相对距离：
        int delta =
            (int)(ptrAddr -
                  tagAddr); // NameAddr - TagAddr (e.g. 0x60 - 0x54 = 0xC)

        char log[256];
        sprintf_s(log, "[INFO] Found Tag 5 at 0x%llX (Delta Name-Tag: 0x%X)",
                  (unsigned long long)tagAddr, delta);
        AddLog(log);

        // 假设 TagOffset 是 0x54 (标准) 或者 0x60
        std::vector<std::uintptr_t> potentialBases;
        potentialBases.push_back(tagAddr - 0x54);
        potentialBases.push_back(tagAddr - 0x60);
        potentialBases.push_back(ptrAddr - 0x60);

        // 5. 第四步：搜索 GOM (指向 GameObjectBase 的指针)
        for (auto goBase : potentialBases) {
          // 让我们只搜 .data section，因为 GOM 肯定在全局区
          for (const auto &sec : sections) {
            // Filter again
            char name[9] = {};
            memcpy(name, sec.Name, 8);
            std::string lower = name;
            for (auto &c : lower)
              c = tolower(c);
            if (lower.find(".data") == std::string::npos &&
                lower.find(".bss") == std::string::npos &&
                lower.find(".vmp") == std::string::npos &&
                lower.find("_rdata") == std::string::npos)
              continue;

            std::uintptr_t sVa = base + sec.VirtualAddress;
            std::uint32_t sSz = sec.Misc.VirtualSize;
            if (sSz > 50 * 1024 * 1024)
              sSz = 50 * 1024 * 1024;

            std::vector<uint8_t> sBuf(CHUNK_SIZE);
            for (size_t off = 0; off < sSz; off += CHUNK_SIZE) {
              size_t rSz = (std::min)(CHUNK_SIZE, (size_t)(sSz - off));
              if (!mem.Read(sVa + off, sBuf.data(), rSz))
                continue;

              for (size_t k = 0; k < rSz; k += 8) {
                std::uintptr_t v =
                    *reinterpret_cast<std::uintptr_t *>(sBuf.data() + k);
                if (v == goBase) {
                  // 找到了指向 GO Base 的指针！
                  std::uintptr_t gomSlot = sVa + off + k;

                  // GOM array 里的 index 5
                  std::uintptr_t gomBase = gomSlot - (5 * 8);

                  sprintf_s(log, "[SUCCESS] Strategy 5 found GOM! RVA: 0x%llX",
                            (unsigned long long)(gomBase - base));
                  AddLog(log);
                  return gomBase - base;
                }
              }
            }
          }
        }
      }
    }
  }

  return 0;
}

// =============================================================================
// Strategy 6: Doubly Linked List Heap Scan (DMA Optimized)
// =============================================================================

// Helper to validate user-mode pointers
bool IsValidPointer(std::uintptr_t ptr) {
  return (ptr >= 0x100000 && ptr < 0x7FFFFFFFFFFF) && ((ptr & 7) == 0);
}

static std::uintptr_t ParseHexAddressOrZero(const char *text) {
  if (!text || !*text) {
    return 0;
  }
  char *end = nullptr;
  const unsigned long long value = std::strtoull(text, &end, 0);
  if (end == text) {
    return 0;
  }
  return static_cast<std::uintptr_t>(value);
}

static std::uintptr_t GetSelectedNarakaProbeActor() {
  const std::uintptr_t manual =
      ParseHexAddressOrZero(g_appState.narakaProbeActorAddress);
  if (er2::IsCanonicalUserPtr(manual)) {
    return manual;
  }

  std::lock_guard<std::mutex> lock(g_appState.dataMutex);
  if (g_appState.narakaProbeSelectedActor >= 0 &&
      g_appState.narakaProbeSelectedActor <
          static_cast<int>(g_appState.narakaActorProbeEntries.size())) {
    return g_appState
        .narakaActorProbeEntries[g_appState.narakaProbeSelectedActor]
        .actorModel;
  }
  return 0;
}

static void RefreshNarakaProbeActors() {
  if (!er2::g_ctx.memory) {
    g_appState.narakaProbeStatus = "No memory accessor";
    AddLog("[NarakaProbe] Connect DMA before refreshing actors");
    return;
  }
  if (g_appState.gameAssemblyBase == 0) {
    g_appState.narakaProbeStatus = "GameAssembly not selected";
    AddLog("[NarakaProbe] GameAssembly base is 0");
    return;
  }

  const er2::IMemoryAccessor &mem = *er2::g_ctx.memory;
  std::uintptr_t characterManagerClass = 0;
  std::uintptr_t characterManagerStatic = 0;
  std::uintptr_t characterManager = 0;
  const std::uint64_t characterManagerRva =
      er2::DefaultNarakaManagerRvas().characterManager;
  const std::uintptr_t characterManagerSlot =
      g_appState.gameAssemblyBase + characterManagerRva;
  if (!er2::ResolveNarakaCharacterManagerInstance(
          mem, g_appState.gameAssemblyBase, er2::DefaultNarakaManagerRvas(),
          {}, characterManagerClass, characterManagerStatic,
          characterManager)) {
    char buf[192];
    sprintf_s(buf,
              "CharacterManager read failed slot=0x%llX rva=0x%llX "
              "class=0x%llX static=0x%llX instance=0x%llX",
              (unsigned long long)characterManagerSlot,
              (unsigned long long)characterManagerRva,
              (unsigned long long)characterManagerClass,
              (unsigned long long)characterManagerStatic,
              (unsigned long long)characterManager);
    g_appState.narakaProbeStatus = buf;
    AddLog(std::string("[NarakaProbe] ") + buf);
    return;
  }

  std::vector<std::uintptr_t> actors;
  std::string error;
  if (!er2::EnumerateNarakaActorModelsFromCharacterManager(
          mem, characterManager, {}, 256, actors, &error)) {
    g_appState.narakaProbeStatus = error;
    AddLog("[NarakaProbe] " + error);
    return;
  }

  std::vector<NarakaActorProbeEntry> entries;
  entries.reserve(actors.size());
  for (const std::uintptr_t actor : actors) {
    er2::NarakaActorRuntimePropertyCandidate candidate;
    if (!er2::ReadNarakaActorRuntimePropertyCandidate(
            mem, actor, er2::DefaultNarakaRuntimePropertyOptions(),
            candidate)) {
      continue;
    }

    NarakaActorProbeEntry entry;
    entry.actorModel = candidate.actorModel;
    entry.propertyData = candidate.propertyData;
    entry.runtimePropertyData = candidate.runtimePropertyData;
    entry.heroId = candidate.heroId;
    entry.teamId = candidate.teamId;
    entries.push_back(entry);
  }

  {
    std::lock_guard<std::mutex> lock(g_appState.dataMutex);
    g_appState.narakaActorProbeEntries = std::move(entries);
    if (g_appState.narakaProbeSelectedActor >=
        static_cast<int>(g_appState.narakaActorProbeEntries.size())) {
      g_appState.narakaProbeSelectedActor = -1;
    }
    if (g_appState.narakaProbeSelectedActor < 0) {
      for (int i = 0;
           i < static_cast<int>(g_appState.narakaActorProbeEntries.size());
           ++i) {
        if (g_appState.narakaActorProbeEntries[i].heroId == 1000006) {
          g_appState.narakaProbeSelectedActor = i;
          break;
        }
      }
    }
  }

  char buf[160];
  sprintf_s(buf, "[NarakaProbe] Refreshed %zu actor candidates",
            actors.size());
  g_appState.narakaProbeStatus = buf;
  AddLog(buf);
}

static bool CaptureNarakaProbeSnapshot(
    bool baseline, er2::NarakaRuntimePropertySnapshot &target) {
  if (!er2::g_ctx.memory) {
    g_appState.narakaProbeStatus = "No memory accessor";
    return false;
  }

  const std::uintptr_t actor = GetSelectedNarakaProbeActor();
  if (!er2::IsCanonicalUserPtr(actor)) {
    g_appState.narakaProbeStatus = "Select or enter an ActorModel";
    return false;
  }

  const int maxRows = std::max(1, g_appState.narakaProbeMaxRows);
  er2::NarakaRuntimePropertySnapshot snapshot;
  if (!er2::CaptureNarakaRuntimeProperties(*er2::g_ctx.memory, actor,
                                           static_cast<std::size_t>(maxRows),
                                           snapshot)) {
    g_appState.narakaProbeStatus = snapshot.error;
    AddLog("[NarakaProbe] Capture failed: " + snapshot.error);
    return false;
  }

  target = std::move(snapshot);
  if (baseline) {
    g_appState.narakaProbeHasBaseline = true;
    g_appState.narakaProbeDiff.clear();
  } else {
    g_appState.narakaProbeHasCurrent = true;
    if (g_appState.narakaProbeHasBaseline) {
      g_appState.narakaProbeDiff = er2::DiffNarakaRuntimePropertySnapshots(
          g_appState.narakaProbeBaseline, g_appState.narakaProbeCurrent);
    } else {
      er2::NarakaRuntimePropertySnapshot empty;
      g_appState.narakaProbeDiff =
          er2::DiffNarakaRuntimePropertySnapshots(empty,
                                                  g_appState.narakaProbeCurrent);
    }
  }

  char buf[192];
  sprintf_s(buf,
            "[NarakaProbe] Captured %s actor=0x%llX hero=%u rows=%zu",
            baseline ? "baseline" : "current", (unsigned long long)actor,
            target.heroId, target.rows.size());
  g_appState.narakaProbeStatus = buf;
  AddLog(buf);
  return true;
}

static std::string MakeNarakaProbeCsv() {
  std::ostringstream out;
  out << "data_id,index,value_address,baseline_int,current_int,int_delta,"
         "baseline_float,current_float,float_delta,changed\n";
  for (const auto &row : g_appState.narakaProbeDiff) {
    out << row.dataId << ',' << row.index << ",0x" << std::hex
        << static_cast<unsigned long long>(row.valueAddress) << std::dec << ','
        << row.baselineInt << ',' << row.currentInt << ',' << row.intDelta
        << ',' << row.baselineFloat << ',' << row.currentFloat << ','
        << row.floatDelta << ',' << (row.changed ? "true" : "false") << '\n';
  }
  return out.str();
}

static bool ExportNarakaProbeCsv() {
  if (g_appState.narakaProbeDiff.empty()) {
    g_appState.narakaProbeStatus = "No diff rows to export";
    return false;
  }

  CreateDirectoryA(".tmp", nullptr);
  CreateDirectoryA(".tmp\\runtime_property_probe", nullptr);
  SYSTEMTIME st{};
  GetLocalTime(&st);
  char path[MAX_PATH];
  sprintf_s(path,
            ".tmp\\runtime_property_probe\\runtime_property_%04u%02u%02u_"
            "%02u%02u%02u.csv",
            st.wYear, st.wMonth, st.wDay, st.wHour, st.wMinute, st.wSecond);

  std::ofstream file(path, std::ios::binary);
  if (!file) {
    g_appState.narakaProbeStatus = "Failed to open export path";
    return false;
  }
  file << MakeNarakaProbeCsv();
  g_appState.narakaProbeStatus = std::string("Exported ") + path;
  AddLog("[NarakaProbe] Exported CSV: " + std::string(path));
  return true;
}

static bool CaptureNarakaActorContainerSnapshotUi(
    bool baseline, er2::NarakaActorContainerSnapshot &target) {
  if (!er2::g_ctx.memory) {
    g_appState.narakaActorContainerStatus = "No memory accessor";
    return false;
  }

  const std::uintptr_t actor = GetSelectedNarakaProbeActor();
  if (!er2::IsCanonicalUserPtr(actor)) {
    g_appState.narakaActorContainerStatus =
        "Select or enter an ActorModel in Naraka Runtime Property Probe";
    return false;
  }

  const int maxEntries =
      std::max(1, g_appState.narakaActorContainerMaxEntries);
  er2::NarakaActorContainerOptions options =
      er2::DefaultNarakaActorContainerOptions();
  options.offsets.scanEnd = static_cast<std::uint32_t>(
      std::max(0x20, g_appState.narakaActorContainerScanEnd));

  er2::NarakaActorContainerSnapshot snapshot;
  if (!er2::CaptureNarakaActorContainerSnapshot(
          *er2::g_ctx.memory, actor, options,
          static_cast<std::size_t>(maxEntries), snapshot)) {
    g_appState.narakaActorContainerStatus = snapshot.error;
    AddLog("[NarakaActorContainer] Capture failed: " + snapshot.error);
    return false;
  }

  target = std::move(snapshot);
  if (baseline) {
    g_appState.narakaActorContainerHasBaseline = true;
    g_appState.narakaActorContainerDiff.clear();
    g_appState.narakaActorContainerEntryDiff.clear();
  } else {
    g_appState.narakaActorContainerHasCurrent = true;
    if (g_appState.narakaActorContainerHasBaseline) {
      g_appState.narakaActorContainerDiff =
          er2::DiffNarakaActorContainers(
              g_appState.narakaActorContainerBaseline,
              g_appState.narakaActorContainerCurrent);
      g_appState.narakaActorContainerEntryDiff =
          er2::DiffNarakaActorContainerEntries(
              g_appState.narakaActorContainerBaseline,
              g_appState.narakaActorContainerCurrent);
    } else {
      er2::NarakaActorContainerSnapshot empty;
      g_appState.narakaActorContainerDiff =
          er2::DiffNarakaActorContainers(empty,
                                         g_appState.narakaActorContainerCurrent);
      g_appState.narakaActorContainerEntryDiff =
          er2::DiffNarakaActorContainerEntries(
              empty, g_appState.narakaActorContainerCurrent);
    }
  }

  char buf[240];
  sprintf_s(buf,
            "[NarakaActorContainer] Captured %s actor=0x%llX hero=%u "
            "property=0x%llX runtime=0x%llX containers=%zu entries=%zu",
            baseline ? "baseline" : "current",
            (unsigned long long)target.actorModel, target.heroId,
            (unsigned long long)target.propertyData,
            (unsigned long long)target.runtimePropertyData,
            target.containers.size(), target.entries.size());
  g_appState.narakaActorContainerStatus = buf;
  AddLog(buf);
  return true;
}

static std::string MakeNarakaActorContainerCsv() {
  std::ostringstream out;
  out << "state,owner_kind,owner_offset,container_kind,container,entry_va,key,"
         "hash,next,raw_value,value_ptr,value_klass,baseline_int0,"
         "current_int0,baseline_int1,current_int1,baseline_int2,"
         "current_int2,baseline_int3,current_int3,uint0,float0\n";
  for (const auto &row : g_appState.narakaActorContainerEntryDiff) {
    out << (row.added ? "added" : (row.changed ? "changed" : "same")) << ','
        << er2::NarakaActorContainerOwnerKindName(row.current.ownerKind)
        << ",0x" << std::hex
        << static_cast<unsigned long long>(row.current.ownerOffset) << ','
        << er2::NarakaActorContainerKindName(row.current.containerKind)
        << ",0x" << static_cast<unsigned long long>(row.current.container)
        << ",0x" << static_cast<unsigned long long>(row.current.entryAddress)
        << std::dec << ',' << row.current.key << ',' << row.current.hash << ','
        << row.current.next << ",0x" << std::hex
        << static_cast<unsigned long long>(row.current.rawValue) << ",0x"
        << static_cast<unsigned long long>(row.current.valuePtr) << ",0x"
        << static_cast<unsigned long long>(row.current.valueKlass) << std::dec
        << ',' << row.baselineInt0 << ',' << row.current.int0 << ','
        << row.baselineInt1 << ',' << row.current.int1 << ','
        << row.baselineInt2 << ',' << row.current.int2 << ','
        << row.baselineInt3 << ',' << row.current.int3 << ','
        << row.current.uint0 << ',' << row.current.float0 << '\n';
  }
  return out.str();
}

static bool ExportNarakaActorContainerCsv() {
  if (g_appState.narakaActorContainerEntryDiff.empty()) {
    g_appState.narakaActorContainerStatus = "No diff rows to export";
    return false;
  }

  CreateDirectoryA(".tmp", nullptr);
  CreateDirectoryA(".tmp\\actor_container_probe", nullptr);
  SYSTEMTIME st{};
  GetLocalTime(&st);
  char path[MAX_PATH];
  sprintf_s(path,
            ".tmp\\actor_container_probe\\actor_container_%04u%02u%02u_"
            "%02u%02u%02u.csv",
            st.wYear, st.wMonth, st.wDay, st.wHour, st.wMinute, st.wSecond);

  std::ofstream file(path, std::ios::binary);
  if (!file) {
    g_appState.narakaActorContainerStatus = "Failed to open export path";
    return false;
  }
  file << MakeNarakaActorContainerCsv();
  g_appState.narakaActorContainerStatus = std::string("Exported ") + path;
  AddLog("[NarakaActorContainer] Exported CSV: " + std::string(path));
  return true;
}

static std::string HexFromPtr(std::uintptr_t value) {
  char buf[32];
  sprintf_s(buf, "0x%llX", static_cast<unsigned long long>(value));
  return buf;
}

static bool CaptureNarakaBuffProbeSnapshot(
    bool baseline, er2::NarakaBuffManagerSnapshot &target) {
  if (!er2::g_ctx.memory) {
    g_appState.narakaBuffProbeStatus = "No memory accessor";
    return false;
  }
  if (g_appState.gameAssemblyBase == 0) {
    g_appState.narakaBuffProbeStatus = "GameAssembly base is 0";
    return false;
  }

  const int maxEntries = std::max(1, g_appState.narakaBuffProbeMaxEntries);
  er2::NarakaBuffManagerOptions options =
      er2::DefaultNarakaBuffManagerOptions();
  options.offsets.scanEnd =
      static_cast<std::uint32_t>(std::max(0x20, g_appState.narakaBuffProbeScanEnd));
  er2::NarakaBuffManagerSnapshot snapshot;
  if (!er2::CaptureNarakaBuffManagerSnapshot(
          *er2::g_ctx.memory, g_appState.gameAssemblyBase, options,
          static_cast<std::size_t>(maxEntries), snapshot)) {
    g_appState.narakaBuffProbeStatus = snapshot.error;
    AddLog("[NarakaBuffProbe] Capture failed: " + snapshot.error);
    return false;
  }

  target = std::move(snapshot);
  if (baseline) {
    g_appState.narakaBuffProbeHasBaseline = true;
    g_appState.narakaBuffProbeDiff.clear();
  } else {
    g_appState.narakaBuffProbeHasCurrent = true;
    if (g_appState.narakaBuffProbeHasBaseline) {
      g_appState.narakaBuffProbeDiff = er2::DiffNarakaBuffManagerSnapshots(
          g_appState.narakaBuffProbeBaseline,
          g_appState.narakaBuffProbeCurrent);
    } else {
      er2::NarakaBuffManagerSnapshot empty;
      g_appState.narakaBuffProbeDiff = er2::DiffNarakaBuffManagerSnapshots(
          empty, g_appState.narakaBuffProbeCurrent);
    }
  }

  char buf[240];
  sprintf_s(buf,
            "[NarakaBuffProbe] Captured %s class=0x%llX static=0x%llX "
            "instance=0x%llX containers=%zu entries=%zu",
            baseline ? "baseline" : "current",
            (unsigned long long)target.buffManagerClass,
            (unsigned long long)target.staticFields,
            (unsigned long long)target.instance, target.containers.size(),
            target.entries.size());
  g_appState.narakaBuffProbeStatus = buf;
  AddLog(buf);
  return true;
}

static std::string MakeNarakaBuffProbeCsv() {
  std::ostringstream out;
  out << "state,owner_kind,owner_offset,container_kind,container,entry_va,key,"
         "value_ptr,value_klass,baseline_int0,current_int0,baseline_int1,"
         "current_int1,baseline_int2,current_int2,baseline_int3,current_int3,"
         "uint0,float0\n";
  for (const auto &row : g_appState.narakaBuffProbeDiff) {
    const auto containerIt = std::find_if(
        g_appState.narakaBuffProbeCurrent.containers.begin(),
        g_appState.narakaBuffProbeCurrent.containers.end(),
        [&row](const er2::NarakaBuffContainerCandidate &container) {
          return container.container == row.current.container;
        });
    const char *ownerKind =
        containerIt != g_appState.narakaBuffProbeCurrent.containers.end()
            ? er2::NarakaBuffContainerOwnerKindName(containerIt->ownerKind)
            : "";
    const std::uint32_t ownerOffset =
        containerIt != g_appState.narakaBuffProbeCurrent.containers.end()
            ? containerIt->ownerOffset
            : 0;

    out << (row.added ? "added" : (row.changed ? "changed" : "same")) << ','
        << ownerKind << ",0x" << std::hex << ownerOffset << std::dec << ','
        << er2::NarakaBuffContainerKindName(row.current.containerKind) << ",0x"
        << std::hex << static_cast<unsigned long long>(row.current.container)
        << ",0x" << static_cast<unsigned long long>(row.current.entryAddress)
        << std::dec << ',' << row.current.key << ",0x" << std::hex
        << static_cast<unsigned long long>(row.current.valuePtr) << ",0x"
        << static_cast<unsigned long long>(row.current.valueKlass) << std::dec
        << ',' << row.baselineInt0 << ',' << row.current.int0 << ','
        << row.baselineInt1 << ',' << row.current.int1 << ','
        << row.baselineInt2 << ',' << row.current.int2 << ','
        << row.baselineInt3 << ',' << row.current.int3 << ','
        << row.current.uint0 << ',' << row.current.float0 << '\n';
  }
  return out.str();
}

static bool ExportNarakaBuffProbeCsv() {
  if (g_appState.narakaBuffProbeDiff.empty()) {
    g_appState.narakaBuffProbeStatus = "No diff rows to export";
    return false;
  }

  CreateDirectoryA(".tmp", nullptr);
  CreateDirectoryA(".tmp\\buff_manager_probe", nullptr);
  SYSTEMTIME st{};
  GetLocalTime(&st);
  char path[MAX_PATH];
  sprintf_s(path,
            ".tmp\\buff_manager_probe\\buff_manager_%04u%02u%02u_"
            "%02u%02u%02u.csv",
            st.wYear, st.wMonth, st.wDay, st.wHour, st.wMinute, st.wSecond);

  std::ofstream file(path, std::ios::binary);
  if (!file) {
    g_appState.narakaBuffProbeStatus = "Failed to open export path";
    return false;
  }
  file << MakeNarakaBuffProbeCsv();
  g_appState.narakaBuffProbeStatus = std::string("Exported ") + path;
  AddLog("[NarakaBuffProbe] Exported CSV: " + std::string(path));
  return true;
}

std::uint64_t ScanGomByDoublyLinkedList(const er2::IMemoryAccessor &mem,
                                        std::uintptr_t base) {
  AddLog("[DEBUG] Starting Doubly Linked List GOM Scan (Strategy 6)...");

  // Get VMMM Handle and PID
  // Assuming 'mem' is DMAMemoryAccessor and we can cast it or get global
  // context But strictly, we should assume 'dma' is available. We'll access the
  // global g_appState or er2 context if possible, but better to dynamic cast or
  // rely on global er2::g_ctx.memory if initialized.

  auto dmaAccessor =
      std::dynamic_pointer_cast<er2::DMAMemoryAccessor>(er2::g_ctx.memory);
  if (!dmaAccessor) {
    AddLog("[ERROR] Strategy 6 requires DMA Accessor.");
    return 0;
  }

  VMM_HANDLE hVMM = dmaAccessor->GetHandle();
  DWORD pid = dmaAccessor->GetPID();

  // Get LeechCore Handle for Scatter Read
  ULONG64 hLC_val = 0;
  if (!VMMDLL_ConfigGet(hVMM, VMMDLL_OPT_CORE_LEECHCORE_HANDLE, &hLC_val)) {
    AddLog("[ERROR] Failed to get LeechCore handle.");
    return 0;
  }
  HANDLE hLC = (HANDLE)hLC_val;

  // 1. Map VAD (Virtual Address Descriptor) to find Heap/Private RW memory
  AddLog("[INFO] Mapping memory regions (VAD)...");
  PVMMDLL_MAP_VAD pVadMap = nullptr;
  if (!VMMDLL_Map_GetVadU(hVMM, pid, FALSE, &pVadMap)) {
    AddLog("[ERROR] Failed to get VAD map.");
    return 0;
  }

  std::vector<std::pair<std::uint64_t, std::uint32_t>> validRegions;

  // Collect valid Heap-like regions
  // VAD flags usually indicate protection. We want private RW.
  for (DWORD i = 0; i < pVadMap->cMap; i++) {
    auto &entry = pVadMap->pMap[i];
    // Filter generic constraints for a "Heap"
    // Private Memory (not mapped file), RW protection
    if (entry.Protection != 4) // WinAPI PAGE_READWRITE = 4
      continue;
    if (entry.fPrivateMemory == 0)
      continue;

    // Size Filter: > 1MB (optimization)
    std::uint64_t size = entry.vaEnd - entry.vaStart + 1;
    if (size < 1 * 1024 * 1024)
      continue;

    // Address sanity
    if (entry.vaStart < 0x100000 || entry.vaStart > 0x7FFFFFFFFFFF)
      continue;

    validRegions.push_back({entry.vaStart, (std::uint32_t)size});
  }

  char log[256];
  sprintf_s(log, "[INFO] Found %zu large private RW regions.",
            validRegions.size());
  AddLog(log);

  // Free VAD Map
  VMMDLL_MemFree(pVadMap);

  // 2. Scan Regions
  const size_t CHUNK_SIZE = 0x10000; // 64KB
  std::vector<uint8_t> buffer(CHUNK_SIZE);

  struct CandidateNode {
    std::uintptr_t addr;
    std::uintptr_t next;
    std::uintptr_t prev;
  };

  // We process regions in chunks
  for (const auto &region : validRegions) {
    std::uintptr_t start = region.first;
    std::uint32_t size = region.second;

    // Limit scan size per region to avoid freezing?
    // User said "Full Memory / Heap Scan". We'll try to be somewhat exhaustive
    // but responsive.
    if (size > 64 * 1024 * 1024)
      size = 64 * 1024 * 1024; // Cap at 64MB per region

    for (std::uint32_t offset = 0; offset < size; offset += CHUNK_SIZE) {
      std::uint32_t readSize =
          (std::min)((std::uint32_t)CHUNK_SIZE, size - offset);
      if (!mem.Read(start + offset, buffer.data(), readSize))
        continue;

      std::vector<CandidateNode> candidates;

      // Pass 1: Local Pointer Validation
      for (size_t i = 0; i < readSize; i += 8) {
        // Assume standard layout: [Next, Prev, Object]
        // Or [Prev, Next, Object]? User said "Node->Next->Prev == Node".
        // Commonly in C#: LinkedList<T> is [Next, Prev, Value].
        // GOM Node is: Next(8), Prev(8), Object(8).

        std::uintptr_t ptr = start + offset + i;
        std::uintptr_t next =
            *reinterpret_cast<std::uintptr_t *>(buffer.data() + i);

        if (!IsValidPointer(next))
          continue;

        // We can read Prev locally
        if (i + 16 <= readSize) {
          std::uintptr_t prev =
              *reinterpret_cast<std::uintptr_t *>(buffer.data() + i + 8);
          if (!IsValidPointer(prev))
            continue;
          candidates.push_back({ptr, next, prev});
        } else {
          // End of chunk, treat carefully or skip last 8 bytes
        }
      }

      if (candidates.empty())
        continue;

      // Pass 2: Scatter Verify (Next->Prev == Node)
      // We need to read 'next + 8' (PrevOffset)
      PPMEM_SCATTER ppMEMs = nullptr;
      if (!LcAllocScatter1((DWORD)candidates.size(), &ppMEMs)) {
        continue;
      }

      for (size_t i = 0; i < candidates.size(); i++) {
        ppMEMs[i]->qwA = candidates[i].next + 8; // Read Next->Prev
      }

      LcReadScatter(hLC, (DWORD)candidates.size(), ppMEMs);

      std::vector<CandidateNode> confirmedLoopNodes;
      for (size_t i = 0; i < candidates.size(); i++) {
        if (ppMEMs[i]->f && ppMEMs[i]->qwA != MEM_SCATTER_ADDR_INVALID) {
          std::uintptr_t nextPrev =
              *reinterpret_cast<std::uintptr_t *>(ppMEMs[i]->pb);
          if (nextPrev == candidates[i].addr) {
            confirmedLoopNodes.push_back(candidates[i]);
          }
        }
      }
      LcMemFree(ppMEMs);

      if (confirmedLoopNodes.empty())
        continue;

      // Pass 3: Payload Verification
      // candidates[i].addr + 16 is Object pointer.
      // We need to verify Object is valid, and Object content.

      // Sub-Pass 3.1: Read Object Pointer
      // Note: In Candidates we didn't store Object pointer, only Next/Prev.
      // We can read Object from local buffer if we still have it, OR scatter
      // read it. We have local buffer 'buffer', we can recalculate offset.

      std::vector<std::pair<CandidateNode, std::uintptr_t>> nodesWithObjs;

      for (auto &node : confirmedLoopNodes) {
        // Calculate offset in current buffer
        size_t bufOff = node.addr - (start + offset);
        if (bufOff + 24 <= readSize) {
          std::uintptr_t obj =
              *reinterpret_cast<std::uintptr_t *>(buffer.data() + bufOff + 16);
          if (IsValidPointer(obj)) {
            nodesWithObjs.push_back({node, obj});
          }
        }
      }

      if (nodesWithObjs.empty())
        continue;

      // Sub-Pass 3.2: Scatter Read Object fields
      // Need to read: Obj+0x30 (Pool), Obj+0x54 (Tag & Layer), Obj+0x60
      // (NamePtr) We can read one page (0x100) from Obj

      if (!LcAllocScatter1((DWORD)nodesWithObjs.size(), &ppMEMs))
        continue;

      for (size_t i = 0; i < nodesWithObjs.size(); i++) {
        ppMEMs[i]->qwA = nodesWithObjs[i].second; // Read Object base
        // We only get 4096 bytes page, that covers 0x30, 0x54, 0x60 easily.
      }

      LcReadScatter(hLC, (DWORD)nodesWithObjs.size(), ppMEMs);

      for (size_t i = 0; i < nodesWithObjs.size(); i++) {
        if (ppMEMs[i]->f) {
          uint8_t *pObj = ppMEMs[i]->pb;

          // Verify 1: ComponentPool (0x30)
          std::uintptr_t pool =
              *reinterpret_cast<std::uintptr_t *>(pObj + 0x30);
          // if (!IsValidPointer(pool)) continue; // Allow null? Usually not.

          // Verify 2: Tag (0x54) - uint16_t or uint32_t? User said int32
          // 0-30000 Unity Tag is usually 16 bit but aligned. Let's read 16.
          uint32_t tag = *reinterpret_cast<uint32_t *>(pObj + 0x54);
          if (tag > 30000)
            continue;

          // Verify 3: Layer (0x50) 0-31
          uint32_t layer = *reinterpret_cast<uint32_t *>(pObj + 0x50);
          if (layer > 31)
            continue;

          // Verify 4: Name Ptr (0x60)
          std::uintptr_t namePtr =
              *reinterpret_cast<std::uintptr_t *>(pObj + 0x60);
          if (!IsValidPointer(namePtr))
            continue;

          // Found a very strong candidate!
          // Read Name string to be 100% sure
          char nameBuf[128];
          if (mem.Read(namePtr, nameBuf, 127)) {
            nameBuf[127] = 0;
            std::string name(nameBuf);
            // Valid names check
            bool nameValid = false;
            static const char *kCommonNames[] = {"MainCamera", "Camera",
                                                 "Player", "Environment",
                                                 "Directional Light"};
            for (auto cn : kCommonNames) {
              if (strstr(nameBuf, cn)) {
                nameValid = true;
                break;
              }
            }

            if (nameValid) {
              sprintf_s(log, "[SUCCESS] Found GOM Node! Object: %s (0x%llX)",
                        nameBuf, (unsigned long long)nodesWithObjs[i].second);
              AddLog(log);

              // Backtrack to Head
              // Follow Prev until we hit the same node or 0
              std::uintptr_t curr = nodesWithObjs[i].first.addr;
              std::uintptr_t cursor = nodesWithObjs[i].first.prev;
              int hops = 0;
              while (IsValidPointer(cursor) && cursor != curr &&
                     hops < 100000) {
                // Check if cursor is Head? Head usually points back to last
                // node in Prev. Wait, GOM Head is a structure containing the
                // list head. In Unity, GOM has 'g_GameObjectManager'. It
                // contains a list. The 'Head' node is usually a sentinel or the
                // manager itself. User says: "Until Prev == 0 or Prev == Self".

                std::uintptr_t prevOfCursor = 0;
                if (!mem.Read(cursor + 8, &prevOfCursor, sizeof(prevOfCursor)))
                  break;

                if (cursor == prevOfCursor) {
                  // Self loop found (Sentinel)
                  // This 'cursor' is likely the GOM Head Node inside GOM.
                  // GOM Address = cursor (roughly, maybe offset)
                  AddLog("[INFO] Found Loop Head via backtracking.");
                  // The GOM head node is usually at offset into GOM.
                  // Assuming cursor is the 'GOM' or 'GOM+Offset'.
                  // Standard: GOM->LastTaggedObject...
                  // The user didn't specify offset from Head Node to GOM Base.
                  // But usually Head Node IS inside GOM.
                  // We return 'cursor - base' (RVA).

                  // However, Unity GOM structure:
                  // class GameObjectManager {
                  //    ...
                  //    List<GameObject> activeObjects; // 0x???
                  // }
                  // activeObjects is a sentinel node.
                  // We return the RVA of this sentinel? Or GOM base?
                  // Function expects GOM Global Slot RVA.
                  // Let's assume cursor IS the GOM pointer (or close to it).
                  // Let's return cursor - base.
                  LcMemFree(ppMEMs);
                  return cursor - base;
                }

                cursor = prevOfCursor;
                hops++;
              }

              if (cursor == 0 || cursor == curr) {
                // Hit 0 or looped back to start
                AddLog("[INFO] Backtracked to 0 or Self.");
                // If 0, then the last valid node was the head? No, 0 means not
                // circular. If looped back to start, then the start node was
                // part of the ring. We need the sentinel. For now, let's return
                // the node we found, maybe the user can verify? Actually, let's
                // return 'curr', and let logic handle it? But we need GOM Base.
                // Let's just return 'curr - base' and hope
                // 'FindGomGlobalSlotRva' logic handles it? No, 'ScanGom' is
                // supposed to return the GOM registry address. If we found a
                // Node, we are close. Let's assume the 'Head' detection
                // (Prev==Self) is the key.
              }
            }
          }
        }
      }
      LcMemFree(ppMEMs);
    }
  }

  return 0;
}

// Global override for CharMgr offset (Auto-detected)
static uint64_t g_activeCharMgrOffset = Offset::m_CharacterManager;

static const char *ManagerRvaKindUiName(er2::ManagerRvaKind kind) {
  switch (kind) {
  case er2::ManagerRvaKind::CharacterManager:
    return "CharacterManager";
  case er2::ManagerRvaKind::EntityManager:
    return "EntityManager";
  case er2::ManagerRvaKind::GlobalTime:
    return "GlobalTime";
  case er2::ManagerRvaKind::CharactorSync:
    return "CharactorSync";
  case er2::ManagerRvaKind::UserDataManager:
    return "UserDataManager";
  default:
    return "Unknown";
  }
}

static uint64_t *RuntimeOffsetForManagerKind(er2::ManagerRvaKind kind) {
  switch (kind) {
  case er2::ManagerRvaKind::CharacterManager:
    return &Offset::m_CharacterManager;
  case er2::ManagerRvaKind::EntityManager:
    return &Offset::m_EntityManager;
  case er2::ManagerRvaKind::GlobalTime:
    return &Offset::m_GlobalTime;
  case er2::ManagerRvaKind::CharactorSync:
    return &Offset::m_CharactorSync;
  case er2::ManagerRvaKind::UserDataManager:
    return &Offset::m_UserDataManager;
  default:
    return nullptr;
  }
}

static void ApplyConfirmedManagerRvaOffsets() {
  const er2::ManagerRvaKind kinds[] = {
      er2::ManagerRvaKind::CharacterManager,
      er2::ManagerRvaKind::EntityManager,
      er2::ManagerRvaKind::GlobalTime,
      er2::ManagerRvaKind::CharactorSync,
      er2::ManagerRvaKind::UserDataManager,
  };

  int applied = 0;
  std::lock_guard<std::mutex> lock(g_appState.dataMutex);
  for (const er2::ManagerRvaKind kind : kinds) {
    const er2::ManagerRvaCandidate *best =
        er2::FindBestConfirmedManagerRvaCandidate(
            g_appState.managerRvaScanResult, kind);
    uint64_t *target = RuntimeOffsetForManagerKind(kind);
    if (!best || !target) {
      continue;
    }

    *target = best->rva;
    if (kind == er2::ManagerRvaKind::CharacterManager) {
      g_activeCharMgrOffset = best->rva;
    }
    ++applied;
  }

  char buf[160];
  sprintf_s(buf, "[OffsetScan] Applied %d confirmed manager RVA(s)", applied);
  AddLog(buf);
}

static void StartManagerRvaScanFromUi() {
  if (g_appState.isScanningManagerRvas) {
    AddLog("[WARN] Manager RVA scan is already running");
    return;
  }

  if (!g_appState.isConnected || !er2::g_ctx.memory) {
    AddLog("[ERROR] Connect DMA before running the manager RVA scan");
    return;
  }

  if (g_appState.gameAssemblyBase == 0 || g_appState.gameAssemblySize == 0) {
    AddLog("[ERROR] GameAssembly module is not selected");
    return;
  }

  const std::uintptr_t moduleBase = g_appState.gameAssemblyBase;
  const std::uint32_t moduleSize = g_appState.gameAssemblySize;
  const std::uint32_t maxMb = g_appState.managerRvaScanMaxMb;
  std::string moduleName = g_appState.customGameAssemblyName;

  g_appState.isScanningManagerRvas = true;
  {
    std::lock_guard<std::mutex> lock(g_appState.dataMutex);
    g_appState.managerRvaScanStatus = "Scanning current GameAssembly...";
    g_appState.managerRvaScanResult = er2::ManagerRvaScanResult{};
  }

  std::thread([moduleBase, moduleSize, maxMb, moduleName]() {
    AddLog("========================================");
    AddLog("[OffsetScan] Starting live GameAssembly manager RVA scan...");

    char startBuf[256];
    sprintf_s(startBuf, "[OffsetScan] Module: %s @ 0x%llX size=0x%X",
              moduleName.empty() ? "(unknown)" : moduleName.c_str(),
              (unsigned long long)moduleBase, moduleSize);
    AddLog(startBuf);

    er2::ManagerRvaScanOptions options;
    if (maxMb != 0) {
      options.maxSectionBytes = maxMb * 1024u * 1024u;
    }

    er2::ManagerRvaScanResult result;
    const auto started = std::chrono::steady_clock::now();
    const bool ok = er2::ScanGameAssemblyManagerRvas(
        er2::Mem(), moduleBase, moduleSize, options, result);

    if (ok) {
      for (er2::ManagerRvaCandidate &candidate : result.candidates) {
        if (candidate.kind != er2::ManagerRvaKind::GlobalTime ||
            !er2::IsCanonicalUserPtr(candidate.resolvedPtr)) {
          continue;
        }

        double first = 0.0;
        double second = 0.0;
        if (!er2::ReadValue(er2::Mem(), candidate.resolvedPtr + 0x28,
                            first)) {
          continue;
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(120));
        if (!er2::ReadValue(er2::Mem(), candidate.resolvedPtr + 0x28,
                            second)) {
          continue;
        }
        if (!std::isfinite(first) || !std::isfinite(second)) {
          continue;
        }

        const double delta = second - first;
        candidate.value = second;
        candidate.observedDelta = delta;
        if (delta > 0.0 && delta < 5.0) {
          candidate.score += 40;
          candidate.confirmed = true;
          candidate.evidence += "; delta=" + std::to_string(delta);
        }
      }
      er2::SortManagerRvaCandidates(result);
    }

    const auto finished = std::chrono::steady_clock::now();
    const double seconds =
        std::chrono::duration<double>(finished - started).count();

    std::string status;
    if (ok) {
      char doneBuf[256];
      sprintf_s(doneBuf,
                "Scan complete: %zu candidates, %.1f MB scanned, %.2fs",
                result.candidates.size(),
                static_cast<double>(result.bytesScanned) /
                    (1024.0 * 1024.0),
                seconds);
      status = doneBuf;
      AddLog(std::string("[OffsetScan] ") + status);

      const er2::ManagerRvaKind kinds[] = {
          er2::ManagerRvaKind::CharacterManager,
          er2::ManagerRvaKind::EntityManager,
          er2::ManagerRvaKind::GlobalTime,
          er2::ManagerRvaKind::CharactorSync,
          er2::ManagerRvaKind::UserDataManager,
      };
      for (const er2::ManagerRvaKind kind : kinds) {
        const er2::ManagerRvaCandidate *best =
            er2::FindBestManagerRvaCandidate(result, kind);
        if (!best) {
          continue;
        }

        char line[512];
        sprintf_s(line,
                  "[OffsetScan] Best %-18s rva=0x%llX score=%u confirmed=%s "
                  "%s",
                  ManagerRvaKindUiName(kind),
                  static_cast<unsigned long long>(best->rva), best->score,
                  best->confirmed ? "yes" : "no", best->evidence.c_str());
        AddLog(line);
      }
    } else {
      status = "Scan failed: " + result.error;
      AddLog("[ERROR] [OffsetScan] " + status);
    }

    {
      std::lock_guard<std::mutex> lock(g_appState.dataMutex);
      g_appState.managerRvaScanResult = std::move(result);
      g_appState.managerRvaScanStatus = status;
    }
    g_appState.isScanningManagerRvas = false;
    AddLog("========================================");
  }).detach();
}

// =============================================================================
// Strategy 7: Anchor-Based Discovery (For Naraka / VMP)
// =============================================================================
// =============================================================================
// Naraka Mode: Exclusive Data Scanning
// =============================================================================
void ScanNarakaData() {
  AddLog("[INFO] Starting Naraka Mode Scan...");

  if (g_appState.gameAssemblyBase == 0) {
    AddLog("[ERROR] GameAssembly base is 0. Please initialize DMA first.");
    return;
  }

  const er2::IMemoryAccessor &mem = *er2::g_ctx.memory;
  uintptr_t charMgrPtr = 0;

  // 1. Read CharacterManager
  mem.Read(g_appState.gameAssemblyBase + Offset::m_CharacterManager,
           &charMgrPtr, sizeof(charMgrPtr));

  if (!charMgrPtr) {
    AddLog(
        "[ERROR] CharacterManager is NULL. Check GameOffsets or Game State.");
    return;
  }

  char logBuf[256];
  sprintf_s(logBuf, "[INFO] CharacterManager: 0x%llX",
            (unsigned long long)charMgrPtr);
  AddLog(logBuf);

  // [DEBUG] Dump CharacterManager first 0x100 bytes
  {
    uint64_t buf[16] = {0};
    mem.Read(charMgrPtr, buf, sizeof(buf));
    AddLog("[DEBUG] CharacterManager Memory Dump (0x00-0x80):");
    for (int i = 0; i < 4; i++) {
      sprintf_s(logBuf, "+%02X: %llX %llX %llX %llX", i * 32, buf[i * 4],
                buf[i * 4 + 1], buf[i * 4 + 2], buf[i * 4 + 3]);
      AddLog(logBuf);
    }
  }

  // 2. Read AllAliveCharList (0x58)
  uintptr_t listPtr = 0;
  mem.Read(charMgrPtr + Offset::Character.AllAliveCharList, &listPtr,
           sizeof(listPtr));

  if (!listPtr) {
    AddLog("[ERROR] AllAliveCharList is NULL.");
    return;
  }

  // [DEBUG] Dump List Memory
  {
    uint64_t buf[8] = {0};
    mem.Read(listPtr, buf, sizeof(buf));
    AddLog("[DEBUG] List Memory Dump:");
    sprintf_s(logBuf, "0x00: %llX %llX (Class/Monitor?)", buf[0], buf[1]);
    AddLog(logBuf);
    sprintf_s(logBuf, "0x10: %llX (Items Array?)", buf[2]);
    AddLog(logBuf);
    sprintf_s(logBuf, "0x18: %llX (Size?)", buf[3]);
    AddLog(logBuf);
  }

  // 3. Dictionary Iteration (fid2AliveCharacter)
  uintptr_t entriesArray = 0;
  int32_t count = 0;

  // Dictionary Layout: +0x18 = Entries, +0x20 = Count
  mem.Read(listPtr + 0x18, &entriesArray, sizeof(entriesArray));
  mem.Read(listPtr + 0x20, &count, sizeof(count));

  sprintf_s(logBuf, "[INFO] Dictionary: 0x%llX, Entries: 0x%llX, Count: %d",
            (unsigned long long)listPtr, (unsigned long long)entriesArray,
            count);
  AddLog(logBuf);

  if (count <= 0 || count > 1000) {
    AddLog("[WARN] Dictionary count is invalid or empty.");
    return;
  }

  if (entriesArray < 0x10000) {
    AddLog("[WARN] Entries pointer invalid.");
    return;
  }

  std::vector<CachedGameObject> newList;

  const uint32_t ENTRY_SIZE = 0x18; // 24 bytes
  const uint32_t VAL_OFFSET = 0x10; // Value offset in Entry

  for (int32_t i = 0; i < count; i++) {
    uintptr_t entryAddr = entriesArray + 0x20 + (i * ENTRY_SIZE);

    uintptr_t managedObj = 0;
    mem.Read(entryAddr + VAL_OFFSET, &managedObj, sizeof(managedObj));

    if (managedObj < 0x10000)
      continue;

    // --- Naraka Specific Data Reading ---
    CachedGameObject cached;
    cached.address = managedObj;

    // 1. HeroID (ProPerty.Heroid 0x108)
    uint32_t heroId = 0;

    // Check PropertyData struct pointer first
    uintptr_t propertyDataPtr = 0;
    mem.Read(managedObj + Offset::ActorModel.PropertyData, &propertyDataPtr,
             sizeof(propertyDataPtr));

    uint32_t teamId = 0;
    uint32_t weaponId = 0;
    uintptr_t namePtr = 0;

    if (propertyDataPtr > 0x10000) {
      mem.Read(propertyDataPtr + Offset::ActorModel.ProPerty.Heroid, &heroId,
               sizeof(heroId));
      mem.Read(propertyDataPtr + Offset::ActorModel.ProPerty.Group, &teamId,
               sizeof(teamId));
      mem.Read(propertyDataPtr + Offset::ActorModel.ProPerty.WeaponId,
               &weaponId, sizeof(weaponId));
      mem.Read(propertyDataPtr + Offset::ActorModel.ProPerty.PlayerName,
               &namePtr, sizeof(namePtr));
    }

    // Fallback/Legacy read (if struct not used or offset 0x348 is wrong)
    if (heroId == 0) {
      mem.Read(managedObj + Offset::ActorModel.ProPerty.Heroid, &heroId,
               sizeof(heroId));
    }

    // 2. TeamID (If not read above)
    if (teamId == 0)
      mem.Read(managedObj + Offset::ActorModel.ProPerty.Group, &teamId,
               sizeof(teamId));

    // 3. PlayerName (If not read above)
    if (namePtr == 0)
      mem.Read(managedObj + Offset::ActorModel.ProPerty.PlayerName, &namePtr,
               sizeof(namePtr));

    std::string name = "Bot/Unknown";
    if (namePtr > 0x10000) {
      int32_t len = 0;
      mem.Read(namePtr + 0x10, &len, sizeof(len));
      if (len > 0 && len < 64) {
        std::vector<char16_t> buf(len + 1);
        mem.Read(namePtr + 0x14, buf.data(), len * 2);
        buf[len] = 0;
        std::string s;
        for (auto c : buf)
          s += (char)c;
        name = s;
      }
    }

    // 6. Transform
    // Transform is likely at `managedObj + Offset::ActorModel.Transform`
    // (0x118)
    uintptr_t transformPtr = 0;
    mem.Read(managedObj + Offset::ActorModel.Transform, &transformPtr,
             sizeof(transformPtr));

    cached.name = "[" + std::to_string(teamId) + "] " + name +
                  " (Hero:" + std::to_string(heroId) + ")";
    cached.transformAddress = transformPtr;

    if (transformPtr > 0x10000) {
      auto posOpt = er2::GetTransformWorldPosition(transformPtr);
      if (posOpt.has_value()) {
        cached.worldPos[0] = posOpt->x;
        cached.worldPos[1] = posOpt->y;
        cached.worldPos[2] = posOpt->z;
      }
    }

    // Store raw pointer for deeper inspection
    ComponentInfo modelInfo;
    modelInfo.address = managedObj;
    modelInfo.typeName = "ActorModel";
    modelInfo.nameSpace = "Naraka";
    cached.components.push_back(modelInfo);

    newList.push_back(cached);
  }

  {
    std::lock_guard<std::mutex> lock(g_appState.dataMutex);
    g_appState.gameObjects = std::move(newList);
    g_appState.totalGameObjects =
        static_cast<int>(g_appState.gameObjects.size());
  }
  sprintf_s(logBuf, "[SUCCESS] Naraka Scan Complete. Found %d actors.",
            g_appState.totalGameObjects);
  AddLog(logBuf);
}

// =============================================================================
// End Strategy 7
// =============================================================================

// =============================================================================
// Strategy 8: Custom Dump (Direct Offset Read)
// =============================================================================
// Helper to dump hex
void HexDumpLog(const char *label, uintptr_t addr, const void *data,
                size_t size) {
  const uint8_t *p = (const uint8_t *)data;
  char buf[256];
  char ascii[17];
  ascii[16] = 0;

  AddLog(std::string("[DEBUG] HexDump: ") + label);
  for (size_t i = 0; i < size; i += 16) {
    int pos = 0;
    pos += sprintf_s(buf + pos, 256 - pos, "  +%02X: ", (uint32_t)i);

    for (size_t j = 0; j < 16; j++) {
      if (i + j < size) {
        pos += sprintf_s(buf + pos, 256 - pos, "%02X ", p[i + j]);
        unsigned char c = p[i + j];
        ascii[j] = (c >= 32 && c < 127) ? c : '.';
      } else {
        pos += sprintf_s(buf + pos, 256 - pos, "   ");
        ascii[j] = ' ';
      }
    }
    AddLog(std::string(buf) + " | " + ascii);
  }
}

// [New] Strategy: Scan Cameras via MSID (Decoupled from GOM)
void ScanCamerasFromMsid() {
  if (!er2::IsInited()) {
    AddLog("[ERROR] ER2 Not Inited.");
    return;
  }
  AddLog("[INFO] Scanning Cameras via MSID (No GOM dependency)...");
  std::vector<CachedCamera> newCameras;

  // Ensure MSID slot is valid
  if (er2::g_ctx.msIdToPointerSlotVa == 0) {
    AddLog("[WARN] MSID Slot VA is 0. Trying to refresh context...");
    // AutoInit might be needed or re-scanning context
  }

  er2::EnumerateOptions opt;
  opt.onlyGameObject = false; // Camera is a Component
  opt.onlyScriptableObject = false;

  int candidateCount = 0;
  er2::UnityPlayerRange range;
  range.base = er2::g_ctx.unityPlayer.base;
  range.size = er2::g_ctx.unityPlayer.size;

  er2::EnumerateMsIdToPointerObjects(
      er2::Runtime(), *er2::g_ctx.memory, er2::g_ctx.msIdToPointerSlotVa,
      er2::g_ctx.off, range, opt, [&](const er2::ObjectInfo &info) {
        if (info.typeFullName.find("UnityEngine.Camera") != std::string::npos) {
          candidateCount++;
          CachedCamera cam;
          cam.address = info.native;
          // Use captured name or fallback
          cam.name = info.objectName.empty()
                         ? ("Camera_MSID_" + std::to_string(info.instanceId))
                         : info.objectName;

          // Verify by reading matrix? Optional.
          // For now, accept it.
          newCameras.push_back(cam);
        }
      });

  AddLog("[INFO] MSID Scan Complete. Found " + std::to_string(candidateCount) +
         " cameras.");

  std::lock_guard<std::mutex> lock(g_appState.dataMutex);
  if (!newCameras.empty()) {
    g_appState.cameras = newCameras;
    // Auto-select first if none selected
    if (g_appState.manualCameraAddress == 0) {
      g_appState.manualCameraAddress = newCameras[0].address;
    }
  }
}

void ScanGomByCustomOffsets() {
  AddLog("[INFO] Starting Strategy 8 (Custom Dump)...");

  if (Offset::GameAssembly == 0) {
    if (g_appState.gameAssemblyBase != 0)
      Offset::GameAssembly = g_appState.gameAssemblyBase;
    else {
      AddLog("[ERROR] GameAssembly base not set! Run InitDma first.");
      return;
    }
  }

  const er2::IMemoryAccessor &mem = *er2::g_ctx.memory;
  uintptr_t charMgrPtr = 0;

  // Read Character Manager
  uintptr_t mgrAddr = Offset::GameAssembly + Offset::m_CharacterManager;
  if (!mem.Read(mgrAddr, &charMgrPtr, sizeof(charMgrPtr))) {
    AddLog("[ERROR] Failed to read CharacterManager pointer.");
    return;
  }

  if (!charMgrPtr) {
    AddLog("[ERROR] CharacterManager pointer is NULL.");
    return;
  }

  char logBuf[256];
  sprintf_s(logBuf, "[INFO] CharacterManager: 0x%llX",
            (unsigned long long)charMgrPtr);
  AddLog(logBuf);

  // DEBUG: Dump CharacterManager memory (Check +0x58 offset)
  std::vector<uint8_t> mgrDebug(0x80);
  if (mem.Read(charMgrPtr, mgrDebug.data(), 0x80)) {
    HexDumpLog("CharacterManager (+0x00 to +0x80)", charMgrPtr, mgrDebug.data(),
               0x80);
  }

  // Read List Pointer
  uintptr_t listPtr = 0;
  // Use Offset::Character.AllAliveCharList (0x58)
  mem.Read(charMgrPtr + Offset::Character.AllAliveCharList, &listPtr,
           sizeof(listPtr));

  if (!listPtr) {
    AddLog("[ERROR] List Pointer is NULL.");
    return;
  }

  // [Fix] List Object layout verified by user:
  // +0x20 _items (Array Pointer)
  // +0x28 _size  (Int32)

  uintptr_t itemsArray = 0;
  mem.Read(listPtr + 0x20, &itemsArray, sizeof(itemsArray));

  int32_t size = 0;
  mem.Read(listPtr + 0x28, &size, sizeof(size));

  sprintf_s(logBuf, "[INFO] List: 0x%llX, Items: 0x%llX, Size: %d",
            (unsigned long long)listPtr, (unsigned long long)itemsArray, size);
  AddLog(logBuf);

  if (size <= 0) {
    AddLog("[INFO] List is empty.");
    return;
  }

  if (size > 100)
    size = 100;

  for (int32_t i = 0; i < size; i++) {
    uintptr_t managedObj = 0;
    // items array -> elements at +0x20 + index*8
    mem.Read(itemsArray + 0x20 + (i * 8), &managedObj, sizeof(managedObj));

    if (!managedObj)
      continue;

    // managedObj is the C# Character object

    // Try verify Native Object presence (optional debug)
    uintptr_t nativeObj = 0;
    mem.Read(managedObj + 0x10, &nativeObj, sizeof(nativeObj));

    // Dump Property Data using Offset::ActorModel.ProPerty from ManagedObject
    // Assuming ActorModel fields are directly inside Character, or inherited.

    // Read Property.PlayerName (0x1F8) string
    uintptr_t namePtr = 0;
    mem.Read(managedObj + Offset::ActorModel.ProPerty.PlayerName, &namePtr,
             sizeof(namePtr));

    // Read Property.Heroid (0x108)
    uint32_t heroId = 0;
    mem.Read(managedObj + Offset::ActorModel.ProPerty.Heroid, &heroId,
             sizeof(heroId));

    // Read Property.WeaponId (0x128)
    uint32_t weaponId = 0;
    mem.Read(managedObj + Offset::ActorModel.ProPerty.WeaponId, &weaponId,
             sizeof(weaponId));

    std::string name = "Unknown";
    if (namePtr) {
      uint32_t len = 0;
      mem.Read(namePtr + 0x10, &len, sizeof(len));
      if (len > 0 && len < 64) {
        wchar_t wname[64];
        mem.Read(namePtr + 0x14, &wname, len * 2);
        wname[len] = 0;
        char mbName[128];
        WideCharToMultiByte(CP_UTF8, 0, wname, -1, mbName, sizeof(mbName),
                            nullptr, nullptr);
        name = mbName;
      }
    }

    sprintf_s(logBuf,
              "  [%d] Managed: 0x%llX | Native: 0x%llX | ID: %u | Name: %s", i,
              (unsigned long long)managedObj, (unsigned long long)nativeObj,
              heroId, name.c_str());
    AddLog(logBuf);
  }

  AddLog("[INFO] Strategy 8 Dump Complete.");
}

bool InitDmaConnection() {
  er2::ResetContext();
  AddLog("[INFO] Initializing DMA connection (Metick + FixCR3)...");

  // 1. Initialize Adapter & Metick Library
  g_dmaAdapter = std::make_shared<MetickAdapter>();

  std::string targetProc = g_appState.targetProcessName;
  if (targetProc.empty())
    targetProc = "NarakaBladepoint.exe";

  AddLog("[INFO] Initializing Metick DMA for: " + targetProc);
  if (!g_dmaAdapter->Initialize(targetProc)) {
    const std::string detail = g_dmaAdapter->GetLastError();
    AddLog(detail.empty()
               ? "[ERROR] Failed to initialize DMA adapter."
               : "[ERROR] Failed to initialize DMA adapter: " + detail);
    return false;
  }

  // 2. Fix CR3 (Critical)
  AddLog("[INFO] Attempting to fix CR3 (DTB)...");
  if (!g_dmaAdapter->FixCr3()) {
    AddLog("[ERROR] FixCr3 failed! Memory reads will likely fail.");
    // We continue but warn heavily
  } else {
    AddLog("[SUCCESS] CR3 Fixed!");
  }

  // 3. Setup Context
  er2::g_ctx.pid =
      mem::Get_Process_Id(targetProc.c_str()); // Using mem namespace
  er2::g_ctx.memory = g_dmaAdapter;            // Set polymorphic accessor
  const er2::IMemoryAccessor &accessor = *er2::g_ctx.memory;

  char logBuf[512];
  sprintf_s(logBuf, "[INFO] Attached to PID: %u", er2::g_ctx.pid);
  AddLog(logBuf);

  // 4. Enumerate Modules (Using Adapter EnumerateModules)
  AddLog("[INFO] Enumerating loaded modules...");
  // Use the standard adapter function which returns vector<DmaModuleInfo>
  auto dmaModules = g_dmaAdapter->EnumerateModules();

  std::vector<er2::UnityModuleCandidate> allModules;

  for (const auto &mod : dmaModules) {
    er2::UnityModuleCandidate m;
    m.name = mod.name;
    m.base = static_cast<std::uintptr_t>(mod.baseAddress);
    m.size = mod.imageSize;
    allModules.push_back(m);
  }

  sprintf_s(logBuf, "[INFO] Found %zu modules in process", allModules.size());
  AddLog(logBuf);

  const er2::UnityRuntimeModules runtimeModules =
      er2::FindUnityRuntimeModules(allModules);

  if (runtimeModules.gameAssembly.base != 0) {
    g_appState.gameAssemblyBase = runtimeModules.gameAssembly.base;
    g_appState.gameAssemblySize = runtimeModules.gameAssembly.size;
    strncpy_s(g_appState.customGameAssemblyName,
              sizeof(g_appState.customGameAssemblyName),
              runtimeModules.gameAssembly.name.c_str(), _TRUNCATE);
    Offset::GameAssembly = runtimeModules.gameAssembly.base;
    er2::g_ctx.gameAssembly.base = runtimeModules.gameAssembly.base;
    er2::g_ctx.gameAssembly.size = runtimeModules.gameAssembly.size;

    sprintf_s(logBuf, "[INFO] Detected GameAssembly module: %s (Base: 0x%llX)",
              runtimeModules.gameAssembly.name.c_str(),
              (unsigned long long)runtimeModules.gameAssembly.base);
    AddLog(logBuf);
  } else {
    g_appState.gameAssemblyBase = 0;
    g_appState.gameAssemblySize = 0;
    er2::g_ctx.gameAssembly = er2::ModuleInfo{};
    AddLog("[INFO] GameAssembly*.dll not found - assuming Mono runtime");
  }

  // 5. Get UnityPlayer base address
  if (runtimeModules.unityPlayer.base == 0) {
    AddLog("[ERROR] UnityPlayer*.dll module not found!");
    return false;
  }

  er2::g_ctx.unityPlayer.base = runtimeModules.unityPlayer.base;
  er2::g_ctx.unityPlayer.size = runtimeModules.unityPlayer.size;
  // [FIX] Initialize UnityPlayer Range for EnumerateObjects validation
  er2::g_ctx.unityPlayerRange.base = er2::g_ctx.unityPlayer.base;
  er2::g_ctx.unityPlayerRange.size = er2::g_ctx.unityPlayer.size;

  Offset::UnityPlayer = runtimeModules.unityPlayer.base;

  sprintf_s(logBuf, "[INFO] UnityPlayer module: %s @ 0x%llX",
            runtimeModules.unityPlayer.name.c_str(),
            (unsigned long long)runtimeModules.unityPlayer.base);
  AddLog(logBuf);

  // Set Runtime based on GameAssembly presence
  er2::g_ctx.runtime = (runtimeModules.gameAssembly.base != 0)
                           ? er2::ManagedBackend::Il2Cpp
                           : er2::ManagedBackend::Mono;
  if (er2::g_ctx.runtime == er2::ManagedBackend::Il2Cpp) {
    const er2::Il2CppLayoutProfile *profile =
        er2::FindIl2CppLayoutProfileForUnity2019_4("2019.4.41");
    if (profile) {
      sprintf_s(logBuf,
                "[INFO] IL2CPP layout profile: %s (%s, source: %s)",
                profile->id, profile->unityVersionRange, profile->source);
      AddLog(logBuf);
    }
  }

  // 6. Pre-scan diagnostics
  AddLog("[INFO] Verifying PE header access via DMA...");
  IMAGE_DOS_HEADER dosHeader{};
  if (!accessor.Read(runtimeModules.unityPlayer.base, &dosHeader,
                     sizeof(dosHeader))) {
    AddLog("[ERROR] Failed to read DOS header - DMA read failed!");
    return false;
  }
  if (dosHeader.e_magic != IMAGE_DOS_SIGNATURE) {
    AddLog("[ERROR] Invalid DOS signature");
    return false;
  }
  AddLog("[INFO] DOS Header OK.");

  // 7. Scan GOM (Skipped, user must click buttons)
  // We used to scan here automatically, but now we respect user preference to
  // manual button clicks. We only set context pointers.
  AddLog("[INFO] DMA Initialized. Please select a SCAN STRATEGY below.");

  // Update Context - Reset GOM just in case
  er2::g_ctx.gomGlobalSlotRva = 0;
  er2::g_ctx.gomGlobalSlotVa = 0;

  // 8. Initialize UnityResolve Adapter (DMA Mode)
  AddLog("[INFO] Initializing UnityResolve adapter (DMA mode)...");
  if (g_dmaAdapter) {
    if (!g_UnityResolveAdapter) {
      g_UnityResolveAdapter = new ER2MemoryAdapter(g_dmaAdapter);
      UnityExternal::SetGlobalMemoryAccessor(g_UnityResolveAdapter);
      AddLog("[SUCCESS] UnityResolve adapter initialized (DMA)");
    } else {
      g_UnityResolveAdapter->UpdateAccessor(g_dmaAdapter);
      AddLog("[SUCCESS] UnityResolve adapter updated (DMA)");
    }
  } else {
    AddLog(
        "[WARN] DMA adapter is null, cannot initialize UnityResolve adapter");
  }

  return true;
}

// 读取组件类型信息
bool GetComponentTypeInfo(std::uintptr_t nativeComponent,
                          ComponentInfo &outInfo) {
  outInfo = ComponentInfo{};
  outInfo.address = nativeComponent;

  // 获取 Managed 组件
  std::uintptr_t managedComp = 0;
  if (!er2::GetNativeComponentManaged(er2::Mem(), nativeComponent,
                                      er2::g_ctx.gomOff, managedComp) ||
      !managedComp) {
    return false;
  }

  // 读取类型信息
  er2::TypeInfo ti;
  if (!er2::ReadManagedObjectTypeInfo(er2::g_ctx.runtime, er2::Mem(),
                                      managedComp, er2::g_ctx.off, ti)) {
    return false;
  }

  outInfo.typeName = ti.name;
  outInfo.nameSpace = ti.namespaze;
  return true;
}

// =============================================================================
// Unity 数据获取函数
// =============================================================================
void RefreshGameObjects() {
  AddLog("[INFO] Start refreshing GameObject list...");

  // [New] Naraka Mode Exclusive Path
  if (g_appState.isNarakaMode) {
    ScanNarakaData();
    return;
  }

  auto gameObjectsOpt = er2::EnumerateGameObjects();
  if (!gameObjectsOpt.has_value()) {
    if (g_appState.useFallbackStrategy7) {
      // Fallback: Read from CharacterManager directly
      // We know standard GOM failed. Use the manual list.
      AddLog("[INFO] Standard GOM failed. Using Strategy 7 Fallback (Hardcoded "
             "Characters)...");

      std::vector<CachedGameObject> newList;
      const er2::IMemoryAccessor &mem = er2::Mem();

      // Re-read Manager (copied logic for safety)
      std::uintptr_t charMgr = 0;
      mem.Read(g_appState.gameAssemblyBase + g_activeCharMgrOffset, &charMgr,
               sizeof(charMgr));
      if (charMgr > 0x10000) {
        std::uintptr_t charListPtr = 0;
        mem.Read(charMgr + Offset::Character.AllAliveCharList, &charListPtr,
                 sizeof(charListPtr));

        if (charListPtr > 0x10000) {
          std::uintptr_t itemsArray = 0;
          if (!mem.Read(charListPtr + 0x18, &itemsArray,
                        sizeof(itemsArray))) { // Standard list
            mem.Read(charListPtr + Offset::Character.ArrayStart, &itemsArray,
                     sizeof(itemsArray)); // Fallback offset
          }

          uint32_t size = 0;
          mem.Read(
              charListPtr + 0x18, &size,
              sizeof(size)); // Usually size is at 0x18 for List, items at 0x10?
          // Wait, Standard Unity List:
          // 0x10: _items (Array)
          // 0x18: _size (int)
          // Array: 0x20: Elements

          // My offset reading above used 0x18 for items, which might be wrong
          // for standard List. Let's Correct: List<T>: 0x10: _items 0x18:
          // _size

          // If previous code failed, we can try robust read here.
          std::uintptr_t realItems = 0;
          uint32_t listSize = 0;

          // 1. Read List Size (0x18 standard)
          mem.Read(charListPtr + 0x18, &listSize, sizeof(listSize));

          // Debug List Header (Help diagnose if Size is elsewhere)
          {
            uint64_t v10 = 0, v18 = 0, v20 = 0, v30 = 0;
            mem.Read(charListPtr + 0x10, &v10, 8);
            mem.Read(charListPtr + 0x18, &v18,
                     8); // Size is 32bit, but read 64 to see surrounding
            mem.Read(charListPtr + 0x20, &v20, 8);
            mem.Read(charListPtr + 0x30, &v30, 8);
            char buf[256];
            sprintf_s(buf,
                      "[DEBUG] ListRaw: +10=%llX +18=%llX +20=%llX +30=%llX "
                      "(Size=%d)",
                      v10, v18, v20, v30, listSize);
            AddLog(buf);
          }

          if (listSize > 10000)
            listSize = 10000; // Cap for safety

          // 2. Find Items Array by verifying Length
          // We look for a pointer P such that P->Length (at P+0x18) >=
          // listSize
          auto CheckPtr = [&](std::uintptr_t ptr) -> bool {
            if (ptr < 0x10000 || (ptr & 7) != 0)
              return false;
            uint32_t arrLen = 0;
            if (mem.Read(ptr + 0x18, &arrLen, sizeof(arrLen))) {
              // Heuristic: Array Cap must be >= List Size
              // And reasonable upper bound
              if (arrLen >= listSize && arrLen < 50000)
                return true;

              // Debug failed checks if they look like pointers
              // char buf[128];
              // sprintf_s(buf, "CheckPtr %llX failed: Len=%d vs Size=%d",
              // ptr, arrLen, listSize); AddLog(buf);
            }
            return false;
          };

          const uint32_t offsets[] = {0x10, 0x30, 0x18, 0x20, 0x28, 0x40};
          for (auto off : offsets) {
            std::uintptr_t ptr = 0;
            if (mem.Read(charListPtr + off, &ptr, 8)) {
              if (CheckPtr(ptr)) {
                realItems = ptr;
                char buf[128];
                sprintf_s(buf,
                          "[DEBUG] Verified ItemsArray at List+0x%X -> 0x%llX",
                          off, (unsigned long long)ptr);
                AddLog(buf);
                break;
              }
            }
          }

          if (realItems == 0 && listSize > 0) {
            AddLog("[ERROR] Failed to find valid ItemsArray matching List "
                   "Size.");
            return;
          }

          // Use the newly found listSize for the loop
          size = listSize;

          if (realItems > 0x10000) {
            // [DEBUG] Check what realItems is.
            // Hypothesis 1: realItems is C# Array (Items at +0x20)
            // Hypothesis 2: realItems IS the first Player (Inline 0x30)

            // Log raw read of Index 0 at +0x20
            std::uintptr_t testIdx0 = 0;
            mem.Read(realItems + 0x20, &testIdx0, sizeof(testIdx0));
            {
              char dbg[128];
              sprintf_s(dbg, "[DEBUG] Array[0] (Ptr+0x20): 0x%llX",
                        (unsigned long long)testIdx0);
              AddLog(dbg);
            }

            // Test Hypothesis 2: realItems IS the managed object
            uint32_t testHeroId = 0;
            mem.Read(realItems + 0x108, &testHeroId, sizeof(testHeroId));
            if (testHeroId > 0 && testHeroId < 100000) {
              AddLog("[DEBUG] Hypothesis: realItems IS the Player Object! "
                     "(Found valid-ish HeroID)");
            }

            // Loop
            const uint32_t OFF_HEROID = 0x108;
            const uint32_t OFF_NAME = 0x1F8;
            const uint32_t OFF_TRANS = 0x118;

            for (uint32_t i = 0; i < size; i++) {
              std::uintptr_t managedObj = 0;
              // Assume Hypothesis 1 for now (Standard items array)
              mem.Read(realItems + 0x20 + (i * 8), &managedObj,
                       sizeof(managedObj));

              if (i < 3) {
                char dbg[128];
                sprintf_s(dbg, "[DEBUG] List[%d]: 0x%llX", i,
                          (unsigned long long)managedObj);
                AddLog(dbg);
              }

              if (managedObj > 0x10000) {
                // Direct Read Mode (Bypassing NativeObject check)
                CachedGameObject cached;
                cached.address = managedObj; // Use Managed Address as ID

                // Read Hero ID
                uint32_t heroId = 0;
                mem.Read(managedObj + OFF_HEROID, &heroId, sizeof(heroId));

                // Read Name (System.String)
                std::uintptr_t namePtr = 0;
                mem.Read(managedObj + OFF_NAME, &namePtr, sizeof(namePtr));

                std::string entityName = "Unknown";
                if (namePtr > 0x10000) {
                  // Unity String: 0x10=Length, 0x14=Chars
                  int32_t len = 0;
                  if (mem.Read(namePtr + 0x10, &len, sizeof(len)) && len > 0 &&
                      len < 128) {
                    // Read UTF-16 Chars
                    std::vector<char16_t> nameBuf(len + 1);
                    if (mem.Read(namePtr + 0x14, nameBuf.data(), len * 2)) {
                      nameBuf[len] = 0;
                      // Simple conversion to UTF-8/ASCII for ImGui
                      std::string s;
                      for (auto c : nameBuf) {
                        if (c == 0)
                          break;
                        s += (char)c;
                      }
                      entityName = s;
                    }
                  }
                }

                cached.name = "[" + std::to_string(heroId) + "] " + entityName;

                // Read Transform & Pos
                std::uintptr_t transformPtr = 0;
                mem.Read(managedObj + OFF_TRANS, &transformPtr,
                         sizeof(transformPtr));

                cached.transformAddress = transformPtr;
                if (cached.transformAddress > 0x10000) {
                  // Assume it's a standard Unity Transform component
                  auto posOpt =
                      er2::GetTransformWorldPosition(cached.transformAddress);
                  if (posOpt.has_value()) {
                    cached.worldPos[0] = posOpt->x;
                    cached.worldPos[1] = posOpt->y;
                    cached.worldPos[2] = posOpt->z;
                  }
                } else {
                  cached.worldPos[0] = cached.worldPos[1] = cached.worldPos[2] =
                      0.0f;
                }

                newList.push_back(cached);
              }
            }
          }
        }
      }

      std::lock_guard<std::mutex> lock(g_appState.dataMutex);
      g_appState.gameObjects = std::move(newList);
      g_appState.totalGameObjects =
          static_cast<int>(g_appState.gameObjects.size());
      return;
    }

    AddLog("[ERROR] Failed to enumerate GameObjects");
    return;
  }

  std::vector<CachedGameObject> newList;
  for (const auto &entry : gameObjectsOpt.value()) {
    CachedGameObject cached;
    cached.address = entry.nativeObject;

    // 读取GameObject名称
    std::string goName;
    if (er2::ReadGameObjectName(er2::Mem(), entry.nativeObject, er2::g_ctx.off,
                                goName)) {
      cached.name = goName;
    } else {
      cached.name = "Unknown";
    }

    // 获取 Transform 组件
    cached.transformAddress = er2::GetTransformComponent(entry.nativeObject);

    // 获取世界坐标
    if (cached.transformAddress != 0) {
      auto posOpt = er2::GetTransformWorldPosition(cached.transformAddress);
      if (posOpt.has_value()) {
        cached.worldPos[0] = posOpt->x;
        cached.worldPos[1] = posOpt->y;
        cached.worldPos[2] = posOpt->z;
      } else {
        cached.worldPos[0] = cached.worldPos[1] = cached.worldPos[2] = 0.0f;
      }
    } else {
      cached.worldPos[0] = cached.worldPos[1] = cached.worldPos[2] = 0.0f;
    }

    // 获取组件列表及类型信息
    auto components = er2::GetAllComponents(entry.nativeObject);
    for (auto compAddr : components) {
      ComponentInfo compInfo;
      if (GetComponentTypeInfo(compAddr, compInfo)) {
        cached.components.push_back(compInfo);
      } else {
        // 如果无法读取类型，仍然保存地址
        compInfo.address = compAddr;
        compInfo.typeName = "Unknown";
        compInfo.nameSpace = "";
        cached.components.push_back(compInfo);
      }
    }

    newList.push_back(cached);
  }

  std::lock_guard<std::mutex> lock(g_appState.dataMutex);
  g_appState.gameObjects = std::move(newList);
  g_appState.totalGameObjects = static_cast<int>(g_appState.gameObjects.size());

  char buf[256];
  sprintf_s(buf, "[INFO] Refresh complete, total %d GameObjects",
            g_appState.totalGameObjects);
  AddLog(buf);
}

void RefreshCameras() {
  std::vector<CachedCamera> newList;
  bool usingManual = false;

  // 1. Priority: Manual/Decoupled Address
  if (g_appState.manualCameraAddress != 0) {
    CachedCamera cached;
    cached.address = g_appState.manualCameraAddress;
    cached.name = "Manual/Decoupled Camera";
    newList.push_back(cached);
    usingManual = true;
  } else {
    // 2. Priority: MSID Scanned List (if populated directly by
    // ScanCamerasFromMsid) Check if we already have cameras from a recent MSID
    // scan
    std::lock_guard<std::mutex> lock(g_appState.dataMutex);
    if (!g_appState.cameras.empty()) {
      // If we have cameras and they look like MSID ones (check name?), keep
      // them? Actually, ScanCamerasFromMsid writes directly to
      // g_appState.cameras. So RefreshCameras logic might overwrite it if we
      // are not careful. Let's make RefreshCameras ONLY do the GOM fallback if
      // the list is empty or requested.

      // But RefreshCameras is called by "Scan GOM" buttons.
      // If we wanted to keep MSID cameras, we should perhaps merge or skip GOM
      // scan if we have data.
    }
  }

  // 3. Fallback: Standard GOM Scan (only if not using manual and list is
  // empty/cleared)
  if (!usingManual) {
    auto mainCam = er2::FindMainCamera();
    if (mainCam != 0) {
      CachedCamera cached;
      cached.address = mainCam;
      cached.name = "Main Camera (GOM)";
      newList.push_back(cached);
    }
  }

  std::lock_guard<std::mutex> lock(g_appState.dataMutex);
  if (!newList.empty()) {
    g_appState.cameras = std::move(newList);
  }
}

void RefreshMsidObjects() {
  AddLog("[INFO] Start scanning MSID global objects...");

  // [Fix] Ensure MSID slot is known. If not, try to scan it now.
  if (er2::g_ctx.msIdToPointerSlotVa == 0) {
    AddLog("[WARN] MSID Slot not initialized. Attempting to scan...");
    if (er2::g_ctx.unityPlayer.base == 0) {
      AddLog("[ERROR] UnityPlayer not initialized. Cannot scan.");
      return;
    }

    std::uintptr_t slotVa = 0;
    if (er2::FindMsIdToPointerSlotVaByScan(*er2::g_ctx.memory,
                                           er2::g_ctx.unityPlayer,
                                           er2::g_ctx.gomOff, slotVa)) {
      er2::g_ctx.msIdToPointerSlotVa = slotVa;
      er2::g_ctx.msIdToPointerSlotRva = slotVa - er2::g_ctx.unityPlayer.base;
      char buf[128];
      sprintf_s(buf, "[SUCCESS] MSID Slot found at 0x%llX",
                (unsigned long long)slotVa);
      AddLog(buf);
    } else {
      AddLog("[ERROR] MSID Slot scan failed. Feature unavailable.");
      return;
    }
  }

  std::string filterStr = g_appState.msidTypeFilter;
  std::string filterLower;
  filterLower.reserve(filterStr.size());
  for (char c : filterStr) {
    filterLower.push_back(static_cast<char>(
        (c >= 'A' && c <= 'Z') ? (c - 'A' + 'a') : c));
  }

  std::vector<AppState::MsidObjectEntry> newList;
  int enumeratedCount = 0;
  int filteredOutCount = 0;

  // 使用 EnumerateMsIdToPointerObjects 枚举所有对象
  er2::EnumerateOptions opt;
  opt.onlyGameObject = false;
  opt.onlyScriptableObject = false;

  er2::UnityPlayerRange range;
  range.base = er2::g_ctx.unityPlayer.base;
  range.size = er2::g_ctx.unityPlayer.size;

  er2::EnumerateMsIdToPointerObjects(
      er2::Runtime(), *er2::g_ctx.memory, er2::g_ctx.msIdToPointerSlotVa,
      er2::g_ctx.off, range, opt, [&](const er2::ObjectInfo &info) {
        enumeratedCount++;
        // 应用类型过滤
        if (!filterLower.empty()) {
          if (!er2::MatchFilterLower(info.typeFullName, filterLower) &&
              !er2::MatchFilterLower(info.typeClassName, filterLower) &&
              !er2::MatchFilterLower(info.objectName, filterLower)) {
            filteredOutCount++;
            return;
          }
        }

        AppState::MsidObjectEntry entry;
        entry.nativePtr = info.native;
        entry.instanceId = info.instanceId;
        entry.fullTypeName = info.typeFullName;
        entry.namespaze = info.typeNamespace; // [New] Capture Namespace
        entry.className = info.typeClassName;
        entry.name = info.objectName;

        newList.push_back(entry);
      });

  std::lock_guard<std::mutex> lock(g_appState.dataMutex);
  g_appState.msidObjects = std::move(newList);
  g_appState.totalMsidObjects = static_cast<int>(g_appState.msidObjects.size());

  char buf[256];
  sprintf_s(buf,
            "[INFO] MSID scan complete, enumerated %d, filtered %d, total %d "
            "objects",
            enumeratedCount, filteredOutCount, g_appState.totalMsidObjects);
  AddLog(buf);
}

void RefreshBones() {
  std::lock_guard<std::mutex> lock(g_appState.dataMutex);

  if (g_appState.selectedGameObjectIndex < 0 ||
      g_appState.selectedGameObjectIndex >=
          static_cast<int>(g_appState.gameObjects.size())) {
    AddLog("[WARN] Please select a GameObject first");
    return;
  }

  const auto &selectedGO =
      g_appState.gameObjects[g_appState.selectedGameObjectIndex];
  std::string logMsg = "[INFO] Start scanning bones: " + selectedGO.name;
  AddLog(logMsg);

  auto bonesData = er2::GetBoneTransformAll(selectedGO.address);

  std::vector<BoneEntry> newList;
  for (const auto &bone : bonesData) {
    BoneEntry entry;
    entry.index = bone.index;
    entry.name = bone.boneName;
    entry.transformAddress = bone.transform;

    // 读取骨骼世界坐标
    auto posOpt = er2::GetTransformWorldPosition(bone.transform);
    if (posOpt.has_value()) {
      entry.worldPos[0] = posOpt->x;
      entry.worldPos[1] = posOpt->y;
      entry.worldPos[2] = posOpt->z;
    } else {
      entry.worldPos[0] = entry.worldPos[1] = entry.worldPos[2] = 0.0f;
    }

    newList.push_back(entry);
  }

  g_appState.bones = std::move(newList);
  char buf[256];
  sprintf_s(buf, "[INFO] Bone scan complete, total %d bones",
            static_cast<int>(g_appState.bones.size()));
  AddLog(buf);
}

void ConnectToUnity() {
  g_appState.isConnecting = true;
  g_appState.statusMessage = "Searching for Unity process...";
  AddLog("[INFO] Searching for Unity process...");

  if (er2::AutoInit()) {
    g_appState.isConnected = true;
    g_appState.runtimeType =
        (er2::Runtime() == er2::ManagedBackend::Il2Cpp) ? "IL2CPP" : "Mono";

    char buf[256];
    sprintf_s(buf, "Connected to Unity process (PID: %lu)",
              static_cast<unsigned long>(er2::Pid()));
    g_appState.statusMessage = buf;
    std::string logMsg = std::string("[INFO] ") + buf;
    AddLog(logMsg);

    // 初始刷新数据
    RefreshGameObjects();
    RefreshCameras();

    // Initialize UnityResolve Adapter (WinAPI Mode)
    AddLog("[INFO] Initializing UnityResolve adapter (WinAPI mode)...");
    if (er2::g_ctx.memory) {
      if (!g_UnityResolveAdapter) {
        g_UnityResolveAdapter = new ER2MemoryAdapter(er2::g_ctx.memory);
        UnityExternal::SetGlobalMemoryAccessor(g_UnityResolveAdapter);
        AddLog("[SUCCESS] UnityResolve adapter initialized (WinAPI)");
      } else {
        g_UnityResolveAdapter->UpdateAccessor(er2::g_ctx.memory);
        AddLog("[SUCCESS] UnityResolve adapter updated (WinAPI)");
      }
    } else {
      AddLog("[WARN] er2::g_ctx.memory is null, cannot initialize UnityResolve "
             "adapter");
    }
  } else {
    g_appState.isConnected = false;
    g_appState.statusMessage = "Unity process not found";
    AddLog("[ERROR] Unity process not found");
  }

  g_appState.isConnecting = false;
}

// =============================================================================
// ImGui 界面绘制函数
// =============================================================================
void DrawConnectionPanel() {
  if (!g_appState.showConnectionPanel)
    return;

  ImGui::Begin("Connection Panel", &g_appState.showConnectionPanel,
               ImGuiWindowFlags_AlwaysAutoResize);

  // Status Indicator
  if (g_appState.isConnected) {
    ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.0f, 1.0f, 0.0f, 1.0f));
    ImGui::Bullet();
    ImGui::PopStyleColor();
    ImGui::SameLine();
    ImGui::Text("Connected");
  } else if (g_appState.isConnecting) {
    ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 1.0f, 0.0f, 1.0f));
    ImGui::Bullet();
    ImGui::PopStyleColor();
    ImGui::SameLine();
    ImGui::Text("Connecting...");
  } else {
    ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 0.0f, 0.0f, 1.0f));
    ImGui::Bullet();
    ImGui::PopStyleColor();
    ImGui::SameLine();
    ImGui::Text("Disconnected");
  }

  ImGui::Separator();

  // Status Info
  ImGui::Text("Status: %s", g_appState.statusMessage.c_str());
  if (g_appState.isConnected) {
    ImGui::Text("Runtime: %s", g_appState.runtimeType.c_str());
    ImGui::Text("Object Count: %d", g_appState.totalGameObjects);
  }

  ImGui::Separator();

  // Actions
  if (g_appState.isConnecting) {
    ImGui::BeginDisabled();
    ImGui::Button("Connecting...", ImVec2(120, 0));
    ImGui::EndDisabled();
  } else if (g_appState.isConnected) {
    if (ImGui::Button("Refresh Data", ImVec2(120, 0))) {
      std::thread([]() {
        RefreshGameObjects();
        RefreshCameras();
      }).detach();
    }

    // Auto Refresh
    ImGui::Separator();
    ImGui::Checkbox("Auto Refresh", &g_appState.autoRefresh);
    if (g_appState.autoRefresh) {
      ImGui::SameLine();
      ImGui::SetNextItemWidth(100);
      ImGui::SliderFloat("Interval(s)", &g_appState.refreshInterval, 0.5f,
                         5.0f);
    }
  } else {
    // Mode Selection
    ImGui::Text("Mode:");
    ImGui::SameLine();
    ImGui::RadioButton("WinAPI", &g_appState.selectedRunMode, 0);
    ImGui::SameLine();
    ImGui::RadioButton("DMA", &g_appState.selectedRunMode, 1);
    g_appState.runMode = static_cast<RunMode>(g_appState.selectedRunMode);

    ImGui::Separator();

    if (g_appState.runMode == RunMode::DMA) {
      // DMA Initialization
      ImGui::InputText("Target Process", g_appState.targetProcessName, 128);
      ImGui::SameLine();
      if (ImGui::Button("[HolyKnight]")) {
        strcpy_s(g_appState.targetProcessName, "HolyKnightRicca.exe");
      }
      if (ImGui::Button("Initialize DMA", ImVec2(160, 0))) {
        std::thread([]() {
          g_appState.isConnecting = true;
          g_appState.statusMessage = "Initializing DMA...";
          if (InitDmaConnection()) {
            g_appState.isConnected = true;
            g_appState.statusMessage = "DMA Connected";
            // No auto-refresh here, user must click GOM scan buttons
          } else {
            g_appState.isConnected = false;
            g_appState.statusMessage = "DMA Failed";
          }
          g_appState.isConnecting = false;
        }).detach();
      }
    } else {
      // WinAPI Connect
      if (ImGui::Button("Connect Unity", ImVec2(120, 0))) {
        std::thread(ConnectToUnity).detach();
      }
      ImGui::SameLine();
      if (ImGui::Button("Manual Select", ImVec2(110, 0))) {
        g_appState.showProcessSelector = true;
        g_appState.selectedProcessIndex = -1;
        if (!g_appState.isScanning) {
          g_appState.isScanning = true;
          std::thread([]() {
            AddLog("[INFO] Scanning system processes...");
            auto procs = EnumerateAllProcesses();
            {
              std::lock_guard<std::mutex> lock(g_appState.dataMutex);
              g_appState.unityProcesses = std::move(procs);
            }
            char buf[128];
            sprintf_s(buf, "[INFO] Found %zu processes",
                      g_appState.unityProcesses.size());
            AddLog(buf);
            g_appState.isScanning = false;
          }).detach();
        }
      }
    }
  } // End if(connected) else

  // [New] Debug & Manual Config Section (Always Visible)
  ImGui::Separator();
  if (ImGui::CollapsingHeader("Debug Config", ImGuiTreeNodeFlags_DefaultOpen)) {
    ImGui::Checkbox("Use Custom Module Names",
                    &g_appState.useCustomModuleNames);
    if (g_appState.useCustomModuleNames) {
      ImGui::InputText("UnityPlayer Name", g_appState.customUnityPlayerName,
                       128);
      ImGui::InputText("GameAssembly Name", g_appState.customGameAssemblyName,
                       128);
    }

    if (ImGui::Button("Disconnect", ImVec2(120, 0))) {
      g_appState.isConnected = false;
      g_appState.isConnecting = false;
      g_appState.statusMessage = "Disconnected";
      // Reset pointers
      er2::ResetContext();
      g_dmaAdapter.reset(); // Release adapter
      AddLog("[INFO] Disconnected.");
    }

    // New DMA Features Control
    if (g_appState.runMode == RunMode::DMA && g_appState.isConnected) {
      ImGui::SameLine();
      if (ImGui::Button("Fix CR3 (DTB)", ImVec2(120, 0))) {
        std::thread([]() {
          if (g_dmaAdapter) {
            AddLog("[INFO] Attempting to Fix CR3...");
            if (g_dmaAdapter->FixCr3()) {
              AddLog("[SUCCESS] Fix CR3 Success!");
            } else {
              AddLog("[ERROR] Fix CR3 Failed.");
            }
          }
        }).detach();
      }
    }

    if (g_appState.isConnected) {
      ImGui::Separator();
      if (ImGui::CollapsingHeader("Runtime Class Probe",
                                  ImGuiTreeNodeFlags_DefaultOpen)) {
        const auto &profile = ActiveIl2CppLayoutProfile();
        ImGui::Text("Profile: %s", profile.id);
        ImGui::TextDisabled("%s", profile.source);
        ImGui::InputScalar("Klass Address", ImGuiDataType_U64,
                           &g_appState.runtimeProbeKlass, nullptr, nullptr,
                           "%llX",
                           ImGuiInputTextFlags_CharsHexadecimal);

        if (g_appState.inspectedKlass != 0) {
          if (ImGui::Button("Use Inspected Klass", ImVec2(170, 0))) {
            g_appState.runtimeProbeKlass = g_appState.inspectedKlass;
          }
          ImGui::SameLine();
        }

        if (ImGui::Button("Probe Runtime Class", ImVec2(170, 0))) {
          ProbeRuntimeClassFromUi(g_appState.runtimeProbeKlass);
        }
        ImGui::SameLine();
        if (ImGui::Button("Probe Klass Header", ImVec2(170, 0))) {
          ProbeKlassHeaderFromUi(g_appState.runtimeProbeKlass);
        }

        if (g_appState.runtimeProbeHasResult) {
          const auto &r = g_appState.runtimeProbeResult;
          if (r.valid) {
            ImGui::TextColored(ImVec4(0.4f, 1.0f, 0.4f, 1.0f),
                               "Valid runtime class");
            ImGui::Text("Full Name: %s", r.fullName.c_str());
            ImGui::Text("Namespace: %s", r.namespaze.c_str());
            ImGui::Text("Class Name: %s", r.className.c_str());
            ImGui::Text("Parent: 0x%llX", (unsigned long long)r.parent);
            ImGui::Text("Fields: 0x%llX", (unsigned long long)r.fields);
            ImGui::Text("Methods: 0x%llX", (unsigned long long)r.methods);
            ImGui::Text("Static Fields: 0x%llX",
                        (unsigned long long)r.staticFields);
            ImGui::Text("Field Count: %u", r.fieldCount);
            ImGui::Text("Method Count: %u", r.methodCount);
          } else {
            ImGui::TextColored(ImVec4(1.0f, 0.35f, 0.25f, 1.0f),
                               "Probe failed: %s",
                               r.failureReason.c_str());
          }
        }

        if (g_appState.klassHeaderProbeHasResult) {
          const auto &r = g_appState.klassHeaderProbeResult;
          ImGui::SeparatorText("Klass Header Strings");
          if (r.valid) {
            ImGui::TextColored(ImVec4(0.4f, 1.0f, 0.4f, 1.0f),
                               "Best Name: +0x%X -> %s", r.bestNameOffset,
                               r.bestName.c_str());
            if (!r.bestNamespace.empty()) {
              ImGui::Text("Namespace: +0x%X -> %s",
                          r.bestNamespaceOffset, r.bestNamespace.c_str());
            }
          } else {
            ImGui::TextColored(ImVec4(1.0f, 0.35f, 0.25f, 1.0f),
                               "Header probe failed: %s",
                               r.failureReason.c_str());
          }

          if (ImGui::BeginTable("KlassHeaderProbeEntries", 4,
                                ImGuiTableFlags_Borders |
                                    ImGuiTableFlags_RowBg |
                                    ImGuiTableFlags_Resizable)) {
            ImGui::TableSetupColumn("Offset",
                                    ImGuiTableColumnFlags_WidthFixed, 55.0f);
            ImGui::TableSetupColumn("Pointer",
                                    ImGuiTableColumnFlags_WidthFixed, 105.0f);
            ImGui::TableSetupColumn("Score",
                                    ImGuiTableColumnFlags_WidthFixed, 45.0f);
            ImGui::TableSetupColumn("Value",
                                    ImGuiTableColumnFlags_WidthStretch);
            ImGui::TableHeadersRow();

            for (const auto &entry : r.entries) {
              if (!entry.readable || entry.value.empty()) {
                continue;
              }

              ImGui::TableNextRow();
              ImGui::TableNextColumn();
              ImGui::Text("+0x%X", entry.offset);
              ImGui::TableNextColumn();
              ImGui::Text("0x%llX", (unsigned long long)entry.pointer);
              ImGui::TableNextColumn();
              ImGui::Text("%d", entry.score);
              ImGui::TableNextColumn();
              ImGui::Text("%s", entry.value.c_str());
            }
            ImGui::EndTable();
          }
        }

        ImGui::Separator();
        ImGui::InputScalar("Native Address", ImGuiDataType_U64,
                           &g_appState.nativeChainProbeNative, nullptr,
                           nullptr, "%llX",
                           ImGuiInputTextFlags_CharsHexadecimal);
        if (g_appState.inspectedNativePtr != 0) {
          if (ImGui::Button("Use Inspected Native", ImVec2(170, 0))) {
            g_appState.nativeChainProbeNative = g_appState.inspectedNativePtr;
          }
          ImGui::SameLine();
        }
        if (ImGui::Button("Probe Native Chain", ImVec2(170, 0))) {
          ProbeNativeChainFromUi(g_appState.nativeChainProbeNative);
        }

        if (g_appState.nativeChainProbeHasResult) {
          const auto &r = g_appState.nativeChainProbeResult;
          if (r.valid) {
            const auto &best = r.best();
            ImGui::TextColored(ImVec4(0.4f, 1.0f, 0.4f, 1.0f),
                               "Best managed offset: +0x%X",
                               best.managedOffset);
            ImGui::Text("Managed: 0x%llX",
                        (unsigned long long)best.managed);
            ImGui::Text("Klass: 0x%llX", (unsigned long long)best.klass);
            ImGui::Text("Runtime Type: %s",
                        best.runtimeClass.fullName.c_str());
            if (ImGui::Button("Use Native Best Klass", ImVec2(170, 0))) {
              g_appState.runtimeProbeKlass = best.klass;
            }
            ImGui::SameLine();
            if (ImGui::Button("Probe Best Klass Header", ImVec2(190, 0))) {
              ProbeKlassHeaderFromUi(best.klass);
            }
          } else {
            ImGui::TextColored(ImVec4(1.0f, 0.35f, 0.25f, 1.0f),
                               "Native probe failed: %s",
                               r.failureReason.c_str());
            if (r.bestIndex >= 0) {
              const auto &best = r.best();
              if (best.klassReadable) {
                if (ImGui::Button("Probe Best Klass Header",
                                  ImVec2(190, 0))) {
                  g_appState.runtimeProbeKlass = best.klass;
                  ProbeKlassHeaderFromUi(best.klass);
                }
              }
            }
          }

          if (ImGui::BeginTable("NativeProbeCandidates", 6,
                                ImGuiTableFlags_Borders |
                                    ImGuiTableFlags_RowBg |
                                    ImGuiTableFlags_Resizable)) {
            ImGui::TableSetupColumn("Offset",
                                    ImGuiTableColumnFlags_WidthFixed, 55.0f);
            ImGui::TableSetupColumn("Managed",
                                    ImGuiTableColumnFlags_WidthFixed, 105.0f);
            ImGui::TableSetupColumn("Back",
                                    ImGuiTableColumnFlags_WidthFixed, 40.0f);
            ImGui::TableSetupColumn("Klass",
                                    ImGuiTableColumnFlags_WidthFixed, 105.0f);
            ImGui::TableSetupColumn("Score",
                                    ImGuiTableColumnFlags_WidthFixed, 45.0f);
            ImGui::TableSetupColumn("Result",
                                    ImGuiTableColumnFlags_WidthStretch);
            ImGui::TableHeadersRow();

            for (const auto &c : r.candidates) {
              ImGui::TableNextRow();
              ImGui::TableNextColumn();
              ImGui::Text("+0x%X", c.managedOffset);
              ImGui::TableNextColumn();
              ImGui::Text("0x%llX", (unsigned long long)c.managed);
              ImGui::TableNextColumn();
              ImGui::Text("%s", c.hasNativeBackPointer ? "yes" : "no");
              ImGui::TableNextColumn();
              ImGui::Text("0x%llX", (unsigned long long)c.klass);
              ImGui::TableNextColumn();
              ImGui::Text("%d", c.score);
              ImGui::TableNextColumn();
              if (c.valid) {
                ImGui::Text("%s", c.runtimeClass.fullName.c_str());
              } else {
                ImGui::TextDisabled("%s", c.failureReason.c_str());
              }
            }
            ImGui::EndTable();
          }
        }
      }

      ImGui::Separator();
      ImGui::Text("Scan Strategies:");

      if (ImGui::Button("DEBUG: Scan All GOM Candidates", ImVec2(240, 0))) {
        std::thread([]() {
          if (!er2::g_ctx.unityPlayer.base) {
            AddLog("[ERROR] UnityPlayer not found");
            return;
          }
          AddLog("========================================");
          AddLog("[DEBUG] Starting Standard GOM Scan...");
          AddLog("----------------------------------------");

          std::uint64_t rva = 0;
          // 直接使用 er2 库的标准扫描函数，兼容 WinAPI 和 DMA
          if (er2::FindGomGlobalSlotRvaByScan(*er2::g_ctx.memory,
                                              er2::g_ctx.unityPlayer.base,
                                              er2::g_ctx.gomOff, rva)) {
            er2::g_ctx.gomGlobalSlotRva = rva;
            er2::g_ctx.gomGlobalSlotVa = er2::g_ctx.unityPlayer.base + rva;

            char buf[128];
            sprintf_s(buf, "[SUCCESS] GOM Found! RVA: 0x%llX", rva);
            AddLog(buf);

            // 立即刷新对象列表
            RefreshGameObjects();
            RefreshCameras();
          } else {
            AddLog("[WARN] Standard GOM Scan failed. Ensure offsets are "
                   "correct for this Unity version.");
          }
          AddLog("========================================");
        }).detach();
      }

      if (ImGui::Button("1. Scan GOM (Standard)", ImVec2(200, 0))) {
        std::thread([]() {
          if (!er2::g_ctx.unityPlayer.base)
            return;

          if (g_appState.runMode == RunMode::DMA) {
            AddLog("[INFO] Starting Standard GOM Scan (DMA Mode)...");
            if (er2::g_ctx.memory == g_dmaAdapter) {
              AddLog("[DEBUG] Verified: Using DMA Accessor");
            } else {
              AddLog("[WARN] DMA Mode active but Context Memory != "
                     "g_dmaAdapter!");
            }
          } else {
            AddLog("[INFO] Starting Standard GOM Scan (WinAPI Mode)...");
          }

          std::uint64_t rva = 0;
          if (er2::FindGomGlobalSlotRvaByScan(*er2::g_ctx.memory,
                                              er2::g_ctx.unityPlayer.base,
                                              er2::g_ctx.gomOff, rva)) {
            er2::g_ctx.gomGlobalSlotRva = rva;
            er2::g_ctx.gomGlobalSlotVa = er2::g_ctx.unityPlayer.base + rva;
            char buf[128];
            sprintf_s(buf, "[SUCCESS] GOM Found! RVA: 0x%llX", rva);
            AddLog(buf);
            RefreshGameObjects();
            RefreshCameras();
          } else {
            AddLog("[WARN] Standard GOM Scan failed.");
          }
        }).detach();
      }

      if (ImGui::Button("2. Scan GOM (Rule Based)", ImVec2(200, 0))) {
        std::thread([]() {
          if (!er2::g_ctx.unityPlayer.base)
            return;
          std::uint64_t rva =
              FindGomRuleBased(*er2::g_ctx.memory, er2::g_ctx.unityPlayer.base,
                               er2::g_ctx.gomOff);
          if (rva) {
            er2::g_ctx.gomGlobalSlotRva = rva;
            er2::g_ctx.gomGlobalSlotVa = er2::g_ctx.unityPlayer.base + rva;
            char buf[128];
            sprintf_s(buf, "[SUCCESS] GOM Found! RVA: 0x%llX", rva);
            AddLog(buf);
            RefreshGameObjects();
          }
        }).detach();
      }

      if (ImGui::Button("3. Scan GOM (Data Pattern)", ImVec2(200, 0))) {
        std::thread([]() {
          if (!er2::g_ctx.unityPlayer.base)
            return;
          std::uint64_t rva = ScanGomByDataPattern(*er2::g_ctx.memory,
                                                   er2::g_ctx.unityPlayer.base);
          if (rva) {
            er2::g_ctx.gomGlobalSlotRva = rva;
            er2::g_ctx.gomGlobalSlotVa = er2::g_ctx.unityPlayer.base + rva;
            char buf[128];
            sprintf_s(buf, "[SUCCESS] GOM Found! RVA: 0x%llX", rva);
            AddLog(buf);
            RefreshGameObjects();
          }
        }).detach();
      }

      if (ImGui::Button("4. Scan GOM (Linked List)", ImVec2(200, 0))) {
        std::thread([]() {
          if (!er2::g_ctx.unityPlayer.base)
            return;
          std::uint64_t rva = ScanGomByDoublyLinkedList(
              *er2::g_ctx.memory, er2::g_ctx.unityPlayer.base);
          if (rva) {
            er2::g_ctx.gomGlobalSlotRva = rva;
            er2::g_ctx.gomGlobalSlotVa = er2::g_ctx.unityPlayer.base + rva;
            char buf[128];
            sprintf_s(buf, "[SUCCESS] GOM Found! RVA: 0x%llX", rva);
            AddLog(buf);
            RefreshGameObjects();
          }
        }).detach();
      }

      if (ImGui::Button("7. Naraka Mode (Exclusive)", ImVec2(200, 0))) {
        std::thread([]() {
          if (!er2::g_ctx.unityPlayer.base)
            return;

          g_appState.isNarakaMode = true;
          // Set GameAssembly Base (Usually same as UnityPlayer in Naraka
          // context or separate?) MetickAdapter sets gameAssemblyBase
          // separately. Ensure it is set. deps/Naraka/Memory.cpp searches for
          // "GameAssembly.dll". main.cpp InitDmaConnection sets
          // g_appState.gameAssemblyBase.

          ScanNarakaData();
        }).detach();
      }

      // Placeholder - I am not replacing yet, just thinking.
      // I will use grep to find the definition first.
    } else {
      ImGui::TextColored(ImVec4(1, 1, 0, 1),
                         "Connect to enable Scan Strategies.");
    }
  }

  // Auto Refresh
  if (g_appState.isConnected) {
    ImGui::Separator();
    ImGui::Checkbox("Auto Refresh", &g_appState.autoRefresh);
    if (g_appState.autoRefresh) {
      ImGui::SameLine();
      ImGui::SetNextItemWidth(100);
      ImGui::SliderFloat("Interval(s)", &g_appState.refreshInterval, 0.5f,
                         5.0f);
    }
  }

  ImGui::End();
}

// 进程选择器面板
void DrawProcessSelector() {
  if (!g_appState.showProcessSelector)
    return;

  ImGui::SetNextWindowSize(ImVec2(600, 400), ImGuiCond_FirstUseEver);
  if (ImGui::Begin("Select Unity Process", &g_appState.showProcessSelector)) {
    // 刷新按钮
    if (g_appState.isScanning) {
      ImGui::BeginDisabled();
      ImGui::Button("Scanning...", ImVec2(100, 0));
      ImGui::EndDisabled();
    } else {
      if (ImGui::Button("Refresh", ImVec2(100, 0))) {
        g_appState.isScanning = true;
        std::thread([]() {
          AddLog("[INFO] Rescanning all processes...");
          auto processes = EnumerateAllProcesses();
          {
            std::lock_guard<std::mutex> lock(g_appState.dataMutex);
            g_appState.unityProcesses = std::move(processes);
            g_appState.selectedProcessIndex = -1;
          }
          char buf[128];
          sprintf_s(buf, "[INFO] Found %zu processes",
                    g_appState.unityProcesses.size());
          AddLog(buf);
          g_appState.isScanning = false;
        }).detach();
      }
    }

    ImGui::SameLine();
    ImGui::Text("Found: %zu processes", g_appState.unityProcesses.size());

    ImGui::Separator();

    // 自定义模块名配置（支持魔改游戏）
    ImGui::Checkbox(
        "Use Custom Module Names (for modded games)",
        reinterpret_cast<bool *>(
            &g_appState.useCustomModuleNames)); // ImGui expects bool*
    if (g_appState.useCustomModuleNames) {
      ImGui::Indent();
      ImGui::SetNextItemWidth(200);
      ImGui::InputText("UnityPlayer DLL", g_appState.customUnityPlayerName,
                       sizeof(g_appState.customUnityPlayerName));
      ImGui::SameLine();
      ImGui::TextDisabled("(e.g. UnityPlayer_LVB.dll)");

      ImGui::SetNextItemWidth(200);
      ImGui::InputText("GameAssembly DLL", g_appState.customGameAssemblyName,
                       sizeof(g_appState.customGameAssemblyName));
      ImGui::SameLine();
      ImGui::TextDisabled("(e.g. GameAssembly_Super.dll)");
      ImGui::Unindent();

      // 游戏目录扫描功能（绕过进程保护）
      ImGui::Spacing();
      ImGui::TextColored(ImVec4(1.0f, 0.8f, 0.0f, 1.0f),
                         "Can't list modules? Scan game directory instead:");
      ImGui::SetNextItemWidth(350);
      ImGui::InputText("Game Directory", g_appState.gameDirectoryPath,
                       sizeof(g_appState.gameDirectoryPath));
      ImGui::SameLine();
      if (ImGui::Button("Scan DLLs")) {
        if (strlen(g_appState.gameDirectoryPath) > 0) {
          ScanGameDirectory(g_appState.gameDirectoryPath);
        } else {
          AddLog("[ERROR] Please enter the game directory path first");
        }
      }
      ImGui::TextDisabled(
          "(Copy path from File Explorer, e.g. C:\\Games\\MyGame)");
    }

    ImGui::Separator();

    // 进程列表表格
    std::lock_guard<std::mutex> lock(g_appState.dataMutex);

    if (ImGui::BeginTable("ProcessTable", 4,
                          ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg |
                              ImGuiTableFlags_ScrollY |
                              ImGuiTableFlags_Resizable,
                          ImVec2(0, -40))) {
      ImGui::TableSetupColumn("PID", ImGuiTableColumnFlags_WidthFixed, 60.0f);
      ImGui::TableSetupColumn("Process Name", ImGuiTableColumnFlags_WidthFixed,
                              150.0f);
      ImGui::TableSetupColumn("Window Title",
                              ImGuiTableColumnFlags_WidthStretch);
      ImGui::TableSetupColumn("Runtime", ImGuiTableColumnFlags_WidthFixed,
                              70.0f);
      ImGui::TableHeadersRow();

      for (size_t i = 0; i < g_appState.unityProcesses.size(); i++) {
        const auto &proc = g_appState.unityProcesses[i];
        ImGui::TableNextRow();

        bool isSelected =
            (g_appState.selectedProcessIndex == static_cast<int>(i));

        ImGui::TableNextColumn();
        char label[32];
        sprintf_s(label, "%u##proc%zu", proc.pid, i);
        if (ImGui::Selectable(label, isSelected,
                              ImGuiSelectableFlags_SpanAllColumns)) {
          g_appState.selectedProcessIndex = static_cast<int>(i);
        }

        ImGui::TableNextColumn();
        ImGui::Text("%s", proc.processName.c_str());

        ImGui::TableNextColumn();
        if (proc.windowTitle.empty()) {
          ImGui::TextDisabled("(no window)");
        } else {
          ImGui::Text("%s", proc.windowTitle.c_str());
        }

        ImGui::TableNextColumn();
        if (proc.isIl2Cpp) {
          ImGui::TextColored(ImVec4(0.2f, 0.8f, 0.2f, 1.0f), "IL2CPP");
        } else {
          ImGui::TextColored(ImVec4(0.8f, 0.6f, 0.2f, 1.0f), "Mono");
        }
      }

      ImGui::EndTable();
    }

    // 底部按钮
    ImGui::Separator();

    bool canConnect = g_appState.selectedProcessIndex >= 0 &&
                      g_appState.selectedProcessIndex <
                          static_cast<int>(g_appState.unityProcesses.size());

    if (!canConnect) {
      ImGui::BeginDisabled();
    }

    if (ImGui::Button("Connect Selected", ImVec2(140, 0))) {
      const auto &selectedProc =
          g_appState.unityProcesses[g_appState.selectedProcessIndex];
      std::uint32_t targetPid = selectedProc.pid;

      g_appState.isConnecting = true;
      g_appState.statusMessage = "Connecting to selected process...";

      std::thread([targetPid]() {
        char logBuf[256];
        sprintf_s(logBuf, "[INFO] Attempting to connect to PID: %u", targetPid);
        AddLog(logBuf);

        if (ManualInitByPid(targetPid)) {
          g_appState.isConnected = true;
          g_appState.runtimeType =
              (er2::Runtime() == er2::ManagedBackend::Il2Cpp) ? "IL2CPP"
                                                              : "Mono";

          sprintf_s(logBuf, "Connected to Unity process (PID: %u)", targetPid);
          // Status update is thread-safe enough for our usage (std::string
          // assignment)
          g_appState.statusMessage = logBuf;
          AddLog("[SUCCESS] " + std::string(logBuf));

          if (er2::g_ctx.unityPlayer.base != 0) {
            AddLog("[INFO] Use 'Module Info' or 'Object List' to inspect.");
          }

          // Trigger refresh from thread (safe if Refresh functions manage
          // their own locks)
          RefreshGameObjects();
          RefreshCameras();

          // Close selector on success
          g_appState.showProcessSelector = false;
        } else {
          g_appState.isConnected = false;
          g_appState.statusMessage = "Failed to connect to selected process";
          AddLog("[ERROR] Connection failed - see log for details");
        }

        g_appState.isConnecting = false;
      }).detach();
    }

    if (!canConnect) {
      ImGui::EndDisabled();
    }

    ImGui::SameLine();
    if (ImGui::Button("Cancel", ImVec2(80, 0))) {
      g_appState.showProcessSelector = false;
    }

    // 添加 List Modules 按钮帮助调试
    ImGui::SameLine();
    if (!canConnect) {
      ImGui::BeginDisabled();
    }
    if (ImGui::Button("List Modules", ImVec2(100, 0))) {
      const auto &selectedProc =
          g_appState.unityProcesses[g_appState.selectedProcessIndex];
      ListProcessModules(selectedProc.pid);
    }
    if (!canConnect) {
      ImGui::EndDisabled();
    }
    ImGui::SameLine();
    ImGui::TextDisabled("(View in Log Output)");

    // Manual Scan Strategies (Moved outside thread!)
    ImGui::Separator();
    ImGui::Text("Scan Strategies (Manual Override)");

    if (ImGui::Button("Standard GOM")) {
      std::thread([] {
        InitDmaConnection(); // Re-runs full init including scan
      }).detach();
    }
    ImGui::SameLine();
    if (ImGui::Button("Naraka Mode")) {
      std::thread([] {
        g_appState.isNarakaMode = true;
        ScanNarakaData();
      }).detach();
    }
    ImGui::SameLine();
    if (ImGui::Button("Strategy 8 (Dump)")) {
      std::thread(ScanGomByCustomOffsets).detach();
    }
    ImGui::TextDisabled("Note: Main logic runs in background.");
  }
  ImGui::End();
}

void DrawGameObjectList() {
  if (!g_appState.showGameObjectList)
    return;

  ImGui::Begin("GameObject List", &g_appState.showGameObjectList);

  // 搜索框
  ImGui::SetNextItemWidth(-1);
  ImGui::InputTextWithHint("##Search", "Search object name...",
                           g_appState.searchFilter,
                           sizeof(g_appState.searchFilter));

  ImGui::Separator();

  // 对象列表
  std::lock_guard<std::mutex> lock(g_appState.dataMutex);

  std::string filterStr = g_appState.searchFilter;
  g_appState.filteredGameObjects = 0;

  if (ImGui::BeginChild("GameObjectScroll", ImVec2(0, 0), true)) {
    for (size_t i = 0; i < g_appState.gameObjects.size(); i++) {
      const auto &obj = g_appState.gameObjects[i];

      // 应用搜索过滤
      if (!filterStr.empty()) {
        if (obj.name.find(filterStr) == std::string::npos) {
          continue;
        }
      }

      g_appState.filteredGameObjects++;

      // 显示对象
      bool selected =
          (g_appState.selectedGameObjectIndex == static_cast<int>(i));
      char label[512];
      sprintf_s(label, "%s##%zu", obj.name.c_str(), i);

      if (ImGui::Selectable(label, selected)) {
        g_appState.selectedGameObjectIndex = static_cast<int>(i);
      }

      // 悬停提示
      if (ImGui::IsItemHovered()) {
        ImGui::BeginTooltip();
        ImGui::Text("Address: 0x%llX", (unsigned long long)obj.address);
        ImGui::Text("Name: %s", obj.name.c_str());
        ImGui::Text("Position: (%.2f, %.2f, %.2f)", obj.worldPos[0],
                    obj.worldPos[1], obj.worldPos[2]);
        ImGui::EndTooltip();
      }
    }
  }
  ImGui::EndChild();

  ImGui::End();
}

void DrawGameObjectDetails() {
  if (g_appState.selectedGameObjectIndex < 0 ||
      g_appState.selectedGameObjectIndex >=
          static_cast<int>(g_appState.gameObjects.size())) {
    return;
  }

  ImGui::Begin("Object Details");

  std::lock_guard<std::mutex> lock(g_appState.dataMutex);
  const auto &obj = g_appState.gameObjects[g_appState.selectedGameObjectIndex];

  ImGui::Text("Name: %s", obj.name.c_str());
  ImGui::Separator();

  // 基本信息
  if (ImGui::CollapsingHeader("Basic Info", ImGuiTreeNodeFlags_DefaultOpen)) {
    ImGui::Text("Native Address: 0x%llX", (unsigned long long)obj.address);
    ImGui::Text("Transform Address: 0x%llX",
                (unsigned long long)obj.transformAddress);
  }

  // Transform 信息
  if (ImGui::CollapsingHeader("Transform", ImGuiTreeNodeFlags_DefaultOpen)) {
    ImGui::Text("World Position:");
    ImGui::Text("  X: %.4f", obj.worldPos[0]);
    ImGui::Text("  Y: %.4f", obj.worldPos[1]);
    ImGui::Text("  Z: %.4f", obj.worldPos[2]);
  }

  // 组件列表
  if (ImGui::CollapsingHeader("Components", ImGuiTreeNodeFlags_DefaultOpen)) {
    if (obj.components.empty()) {
      ImGui::TextDisabled("No components or failed to read");
    } else {
      // 使用表格显示组件
      if (ImGui::BeginTable("ComponentsTable", 3,
                            ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg)) {
        ImGui::TableSetupColumn("#", ImGuiTableColumnFlags_WidthFixed, 30.0f);
        ImGui::TableSetupColumn("Type", ImGuiTableColumnFlags_WidthStretch);
        ImGui::TableSetupColumn("Address", ImGuiTableColumnFlags_WidthFixed,
                                120.0f);
        ImGui::TableHeadersRow();

        for (size_t i = 0; i < obj.components.size(); i++) {
          const auto &comp = obj.components[i];
          ImGui::TableNextRow();

          ImGui::TableNextColumn();
          ImGui::Text("%zu", i);

          ImGui::TableNextColumn();
          if (!comp.nameSpace.empty()) {
            ImGui::Text("%s.%s", comp.nameSpace.c_str(), comp.typeName.c_str());
          } else {
            ImGui::Text("%s", comp.typeName.c_str());
          }

          ImGui::TableNextColumn();
          ImGui::Text("0x%llX", (unsigned long long)comp.address);
        }

        ImGui::EndTable();
      }
    }
  }

  ImGui::End();
}

void DrawCameraInfo() {
  if (!g_appState.showCameraInfo)
    return;

  ImGui::Begin("Camera Info", &g_appState.showCameraInfo);

  std::lock_guard<std::mutex> lock(g_appState.dataMutex);

  // [New] Manual Camera Control
  if (ImGui::CollapsingHeader("Camera Source Config",
                              ImGuiTreeNodeFlags_DefaultOpen)) {
    // Manual Address Input (Direct Binding)
    ImGui::InputScalar("Manual Address (Hex)", ImGuiDataType_U64,
                       &g_appState.manualCameraAddress, NULL, NULL, "%llX",
                       ImGuiInputTextFlags_CharsHexadecimal);

    if (ImGui::Button("Scan Cameras via MSID", ImVec2(200, 0))) {
      std::thread(ScanCamerasFromMsid).detach();
    }
    ImGui::SameLine();
    ImGui::TextDisabled("(Decoupled from GOM)");
  }
  ImGui::Separator();

  if (g_appState.cameras.empty()) {
    ImGui::TextDisabled("No camera found");
  } else {
    for (const auto &cam : g_appState.cameras) {
      if (ImGui::CollapsingHeader(cam.name.c_str(),
                                  ImGuiTreeNodeFlags_DefaultOpen)) {
        ImGui::Text("Address: 0x%llX", (unsigned long long)cam.address);

        // 获取相机矩阵
        // [DMA Optimization] Force NoCache for Matrix Read to ensure
        // real-time update
        if (g_appState.runMode == RunMode::DMA && g_dmaAdapter) {
          g_dmaAdapter->SetForceNoCache(true);
        }

        auto matrixOpt = er2::GetCameraMatrix(cam.address);

        // [DMA Optimization] Restore Cache
        if (g_appState.runMode == RunMode::DMA && g_dmaAdapter) {
          g_dmaAdapter->SetForceNoCache(false);
        }

        if (matrixOpt.has_value()) {
          ImGui::Text("ViewProj Matrix: Retrieved");

          if (ImGui::TreeNode("Matrix Data")) {
            const auto &m = matrixOpt.value();
            for (int row = 0; row < 4; row++) {
              ImGui::Text("[%d] %.4f, %.4f, %.4f, %.4f", row, m[0][row],
                          m[1][row], m[2][row], m[3][row]);
            }
            ImGui::TreePop();
          }
        } else {
          ImGui::TextDisabled("Failed to get matrix");
        }
      }
    }
  }

  ImGui::End();
}

// =============================================================================
// 新增功能面板
// =============================================================================

// UnityResolve Inspector State
static bool g_showURInspector = false;
static std::uintptr_t g_urInspectedNativePtr = 0;
static std::string g_urInspectedTypeName; // 类型名称 (GameObject/Transform/etc)
static std::string g_urInspectedName;
static std::vector<int32_t> g_urComponentTypes;
static int g_urInspectedTag = 0;
static int g_urInspectedCount = 0;
static bool g_urIsGameObject = false; // 是否为 GameObject 类型

void DrawClassInspector() {
  if (!g_showClassInspector)
    return;

  ImGui::SetNextWindowSize(ImVec2(600, 500), ImGuiCond_FirstUseEver);
  ImGui::Begin("Class Inspector", &g_showClassInspector);

  ImGui::Text("Class: %s", g_inspectedClassTitle.c_str());
  ImGui::TextDisabled("Address: 0x%llX",
                      (unsigned long long)g_inspectedClassPtr);
  ImGui::Separator();

  if (ImGui::BeginTabBar("ClassTabs")) {
    if (ImGui::BeginTabItem("Fields")) {
      ImGui::Text("Count: %zu", g_inspectedFields.size());

      if (ImGui::BeginTable("FieldsTable", 3,
                            ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg |
                                ImGuiTableFlags_Resizable)) {
        ImGui::TableSetupColumn("Offset", ImGuiTableColumnFlags_WidthFixed,
                                60.0f);
        ImGui::TableSetupColumn("Name");
        ImGui::TableSetupColumn("Type Info (Ptr)");
        ImGui::TableHeadersRow();

        for (const auto &field : g_inspectedFields) {
          ImGui::TableNextRow();
          ImGui::TableNextColumn();
          ImGui::Text("0x%X", field.offset);
          ImGui::TableNextColumn();
          ImGui::Text("%s", field.name.c_str());
          ImGui::TableNextColumn();
          ImGui::Text("0x%llX", (unsigned long long)field.typePtr);
        }
        ImGui::EndTable();
      }
      ImGui::EndTabItem();
    }

    if (ImGui::BeginTabItem("Methods")) {
      ImGui::Text("Count: %zu", g_inspectedMethods.size());
      if (ImGui::BeginTable("MethodsTable", 2,
                            ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg |
                                ImGuiTableFlags_Resizable)) {
        ImGui::TableSetupColumn("Address", ImGuiTableColumnFlags_WidthFixed,
                                120.0f);
        ImGui::TableSetupColumn("Name");
        ImGui::TableHeadersRow();

        for (const auto &method : g_inspectedMethods) {
          ImGui::TableNextRow();
          ImGui::TableNextColumn();
          ImGui::Text("0x%llX", (unsigned long long)method.methodPtr);
          ImGui::TableNextColumn();
          ImGui::Text("%s", method.name.c_str());
        }
        ImGui::EndTable();
      }
      ImGui::EndTabItem();
    }
    ImGui::EndTabBar();
  }

  ImGui::End();
}

void DrawUnityResolveInspector() {
  if (!g_showURInspector)
    return;
  ImGui::Begin("UnityResolve Inspector", &g_showURInspector);

  if (g_urInspectedNativePtr == 0) {
    ImGui::Text("No object selected.");
    ImGui::End();
    return;
  }

  // 检查内存适配器状态
  if (!UnityExternal::GetGlobalMemoryAccessor()) {
    ImGui::TextColored(ImVec4(1.0f, 0.2f, 0.2f, 1.0f),
                       "[ERROR] Memory Accessor not initialized!");
    ImGui::Text("Please reconnect to the game process.");
    ImGui::End();
    return;
  }

  ImGui::Text("Native Address: 0x%llX",
              (unsigned long long)g_urInspectedNativePtr);
  ImGui::Text("Type: %s", g_urInspectedTypeName.c_str());

  ImGui::Separator();

  if (g_urIsGameObject) {
    // GameObject 模式 - 使用 NativeGameObject 偏移量
    UnityExternal::NativeGameObject go(g_urInspectedNativePtr);

    if (ImGui::Button("Refresh (GameObject)")) {
      char logBuf[256];
      sprintf_s(logBuf, "[DEBUG] Refreshing GameObject @ 0x%llX",
                (unsigned long long)g_urInspectedNativePtr);
      AddLog(logBuf);

      // 逐步测试每个读取操作
      uintptr_t managed = 0;
      if (go.GetManaged(managed)) {
        sprintf_s(logBuf, "  [OK] Managed: 0x%llX",
                  (unsigned long long)managed);
        AddLog(logBuf);
      } else {
        AddLog("  [FAIL] GetManaged failed!");
      }

      uintptr_t pool = 0;
      if (go.GetComponentPool(pool)) {
        sprintf_s(logBuf, "  [OK] ComponentPool: 0x%llX",
                  (unsigned long long)pool);
        AddLog(logBuf);
      } else {
        AddLog("  [FAIL] GetComponentPool failed!");
      }

      int32_t count = 0;
      if (go.GetComponentCount(count)) {
        sprintf_s(logBuf, "  [OK] ComponentCount: %d", count);
        AddLog(logBuf);
        g_urInspectedCount = count;
      } else {
        AddLog("  [FAIL] GetComponentCount failed!");
        g_urInspectedCount = 0;
      }

      int32_t tag = 0;
      if (go.GetTag(tag)) {
        sprintf_s(logBuf, "  [OK] Tag: %d", tag);
        AddLog(logBuf);
        g_urInspectedTag = tag;
      } else {
        AddLog("  [FAIL] GetTag failed!");
        g_urInspectedTag = 0;
      }

      std::string name = go.GetName();
      if (!name.empty()) {
        AddLog(std::string("  [OK] Name: ") + name);
        g_urInspectedName = name;
      } else {
        AddLog("  [FAIL] GetName returned empty!");
        g_urInspectedName = "";

        // 额外调试：显示 name 指针读取详情
        uintptr_t namePtr = 0;
        if (UnityExternal::ReadPtrGlobal(g_urInspectedNativePtr + 0x60u,
                                         namePtr)) {
          sprintf_s(logBuf, "  [DEBUG] NamePtr @ +0x60: 0x%llX",
                    (unsigned long long)namePtr);
          AddLog(logBuf);
        } else {
          AddLog("  [DEBUG] Failed to read NamePtr @ +0x60");
        }
      }

      if (go.GetComponentTypeIds(g_urComponentTypes)) {
        sprintf_s(logBuf, "  [OK] Component TypeIds: %zu items",
                  g_urComponentTypes.size());
        AddLog(logBuf);
      } else {
        AddLog("  [FAIL] GetComponentTypeIds failed!");
        g_urComponentTypes.clear();
      }
    }
    ImGui::SameLine();
    ImGui::Text("Name: %s", g_urInspectedName.c_str());

    ImGui::Text("Tag ID: %d", g_urInspectedTag);
    ImGui::Text("Components: %d", g_urInspectedCount);

    if (ImGui::CollapsingHeader("Component List")) {
      for (size_t i = 0; i < g_urComponentTypes.size(); ++i) {
        uintptr_t compPtr = 0;
        if (go.GetComponent((int)i, compPtr)) {
          // 获取组件类型名称
          std::string typeName = "Unknown";
          ComponentInfo info;
          if (GetComponentTypeInfo(compPtr, info)) {
            typeName = info.typeName;
            if (!info.nameSpace.empty()) {
              typeName = info.nameSpace + "." + typeName;
            }
          }

          ImGui::Text("[%zu] %s", i, typeName.c_str());
          ImGui::SameLine();

          char btnLabel[64];
          sprintf_s(btnLabel, "Inspect##%zu", i);
          if (ImGui::SmallButton(btnLabel)) {
            g_inspectedClassTitle = typeName;
            g_inspectedFields.clear();
            g_inspectedMethods.clear();
            g_inspectedClassPtr = 0;

            // Read Managed Object -> Klass
            uintptr_t managedObj = 0;
            // +0x28 is standard unity_object_managed_ptr
            if (UnityExternal::ReadPtrGlobal(compPtr + 0x28, managedObj) &&
                managedObj) {
              // Read Klass at +0x0 (Il2Cpp)
              if (UnityExternal::ReadPtrGlobal(managedObj + 0x0,
                                               g_inspectedClassPtr) &&
                  g_inspectedClassPtr) {
                if (er2::g_ctx.memory) {
                  er2::unity2::core::GetClassFields(*er2::g_ctx.memory,
                                                    g_inspectedClassPtr,
                                                    g_inspectedFields);
                  er2::unity2::core::GetClassMethods(*er2::g_ctx.memory,
                                                     g_inspectedClassPtr,
                                                     g_inspectedMethods);
                  g_showClassInspector = true;
                }
              } else {
                AddLog("[WARN] Failed to read Klass ptr from managed object");
              }
            } else {
              AddLog("[WARN] Failed to read Managed object from component");
            }
          }
          ImGui::SameLine();
          ImGui::TextDisabled("(ID: %d)", g_urComponentTypes[i]);
          ImGui::TextDisabled("    Native Addr: 0x%llX",
                              (unsigned long long)compPtr);
        } else {
          ImGui::Text("[%zu] ID: %d (Failed to read ptr)", i,
                      g_urComponentTypes[i]);
        }
      }
    }
  } else {
    // Component 模式 - 使用 NativeComponent 读取所属 GameObject
    UnityExternal::NativeComponent comp(g_urInspectedNativePtr);

    ImGui::TextColored(ImVec4(1.0f, 0.8f, 0.2f, 1.0f),
                       "[This is a Component, not GameObject]");

    if (ImGui::Button("Find Owner GameObject")) {
      uintptr_t ownerGo = 0;
      if (comp.GetGameObject(ownerGo) && ownerGo != 0) {
        // 切换到 GameObject 模式
        g_urInspectedNativePtr = ownerGo;
        g_urIsGameObject = true;
        g_urInspectedTypeName = "GameObject (from Component)";

        UnityExternal::NativeGameObject go(ownerGo);
        g_urInspectedName = go.GetName();
        go.GetTag(g_urInspectedTag);
        go.GetComponentCount(g_urInspectedCount);
        go.GetComponentTypeIds(g_urComponentTypes);

        AddLog("[INFO] Navigated to owner GameObject: 0x" +
               std::to_string(ownerGo));
      } else {
        AddLog("[WARN] Failed to get owner GameObject");
      }
    }

    // 显示 Managed 指针信息
    uintptr_t managed = 0;
    if (comp.GetManaged(managed)) {
      ImGui::Text("Managed Ptr: 0x%llX", (unsigned long long)managed);
    }

    uintptr_t goPtr = 0;
    if (comp.GetGameObject(goPtr)) {
      ImGui::Text("Owner GO (Native): 0x%llX", (unsigned long long)goPtr);
    }
  }

  ImGui::End();
}

void DrawMsidBrowser() {
  if (!g_appState.showMsidBrowser)
    return;

  ImGui::Begin("MSID Global Object Browser", &g_appState.showMsidBrowser);

  // 统计信息
  ImGui::Text("Total Objects: %d", er2::MsIdCount());
  ImGui::Text("Current List: %d", g_appState.totalMsidObjects);
  ImGui::Separator();

  // 类型过滤器
  ImGui::SetNextItemWidth(300);
  ImGui::InputTextWithHint(
      "##MsidFilter", "Type filter (e.g. Camera, Transform)",
      g_appState.msidTypeFilter, sizeof(g_appState.msidTypeFilter));
  ImGui::SameLine();
  if (ImGui::Button("Scan Objects (ER2 Strict)", ImVec2(180, 0))) {
    std::thread(RefreshMsidObjects).detach();
  }
  ImGui::SameLine();
  ImGui::TextDisabled("(Use for WinAPI/DMA comparison)");
  ImGui::SameLine();
  if (ImGui::Button("Clear", ImVec2(60, 0))) {
    std::lock_guard<std::mutex> lock(g_appState.dataMutex);
    g_appState.msidObjects.clear();
    g_appState.totalMsidObjects = 0;
  }

  // 一键筛选 GameObject
  if (ImGui::Button("Filter: GameObject Only", ImVec2(180, 0))) {
    strcpy_s(g_appState.msidTypeFilter, "GameObject");
    std::thread(RefreshMsidObjects).detach();
    AddLog("[INFO] Filtering for GameObject type...");
  }
  ImGui::SameLine();
  if (ImGui::Button("Filter: Transform", ImVec2(150, 0))) {
    strcpy_s(g_appState.msidTypeFilter, "Transform");
    std::thread(RefreshMsidObjects).detach();
    AddLog("[INFO] Filtering for Transform type...");
  }
  ImGui::SameLine();
  if (ImGui::Button("Filter: Camera", ImVec2(120, 0))) {
    strcpy_s(g_appState.msidTypeFilter, "Camera");
    std::thread(RefreshMsidObjects).detach();
    AddLog("[INFO] Filtering for Camera type...");
  }

  ImGui::Separator();

  // 对象列表表格
  std::lock_guard<std::mutex> lock(g_appState.dataMutex);

  if (ImGui::BeginTable("MsidTable", 9,
                        ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg |
                            ImGuiTableFlags_ScrollY)) {
    ImGui::TableSetupColumn("ID", ImGuiTableColumnFlags_WidthFixed, 60.0f);
    ImGui::TableSetupColumn("Namespace", ImGuiTableColumnFlags_WidthFixed,
                            120.0f);
    ImGui::TableSetupColumn("Type", ImGuiTableColumnFlags_WidthFixed, 150.0f);
    ImGui::TableSetupColumn("Name",
                            ImGuiTableColumnFlags_WidthStretch); // Name
    ImGui::TableSetupColumn("Address", ImGuiTableColumnFlags_WidthFixed,
                            100.0f);
    ImGui::TableSetupColumn("Cp", ImGuiTableColumnFlags_WidthFixed, 30.0f);
    ImGui::TableSetupColumn("Inspect", ImGuiTableColumnFlags_WidthFixed, 50.0f);
    ImGui::TableSetupColumn("Probe", ImGuiTableColumnFlags_WidthFixed, 45.0f);
    ImGui::TableSetupColumn("UR", ImGuiTableColumnFlags_WidthFixed, 40.0f);
    ImGui::TableHeadersRow();

    ImGuiListClipper clipper;
    clipper.Begin(static_cast<int>(g_appState.msidObjects.size()));
    while (clipper.Step()) {
      for (int i = clipper.DisplayStart; i < clipper.DisplayEnd; i++) {
        const auto &obj = g_appState.msidObjects[i];
        ImGui::TableNextRow();
        ImGui::TableNextColumn();
        ImGui::Text("%u", obj.instanceId);
        ImGui::TableNextColumn();
        ImGui::TextDisabled("%s", obj.namespaze.c_str()); // Namespace (grayed)
        ImGui::TableNextColumn();
        ImGui::Text("%s", obj.className.c_str());
        ImGui::TableNextColumn();
        ImGui::Text("%s", obj.name.c_str()); // Display Name
        ImGui::TableNextColumn();
        ImGui::Text("0x%llX", (unsigned long long)obj.nativePtr);
        ImGui::TableNextColumn();
        char buttonId[64];
        sprintf_s(buttonId, "##cp%u", obj.instanceId);
        if (ImGui::SmallButton(buttonId)) {
          char addrBuf[32];
          sprintf_s(addrBuf, "0x%llX", (unsigned long long)obj.nativePtr);
          CopyToClipboard(addrBuf);
          AddLog("[INFO] Copied: " + std::string(addrBuf));
        }
        // Inspect Button
        ImGui::TableNextColumn();
        char inspectId[64];
        sprintf_s(inspectId, "Ins##%u", obj.instanceId);
        if (ImGui::SmallButton(inspectId)) {
          g_appState.nativeChainProbeNative = obj.nativePtr;
          // 读取 Managed Object 和 Klass
          std::uintptr_t managedPtr = 0;
          if (er2::ReadPtr(er2::Mem(),
                           obj.nativePtr +
                               er2::g_ctx.off.unity_object_managed_ptr,
                           managedPtr) &&
              er2::IsCanonicalUserPtr(managedPtr)) {
            std::uintptr_t klass = 0;
            if (er2::ReadPtr(er2::Mem(), managedPtr, klass) &&
                er2::IsCanonicalUserPtr(klass)) {
              g_appState.inspectedNativePtr = obj.nativePtr;
              g_appState.inspectedManagedPtr = managedPtr;
              g_appState.inspectedKlass = klass;
              g_appState.runtimeProbeKlass = klass;
              g_appState.inspectedClassName = obj.fullTypeName;
              g_appState.inspectedFields.clear();

              // 遍历字段
              er2::EnumerateClassFields(er2::Mem(), klass, er2::g_ctx.off,
                                        g_appState.inspectedFields);
              g_appState.showFieldInspector = true;

              char logBuf[256];
              sprintf_s(logBuf, "[INFO] Inspecting class '%s' with %zu fields",
                        obj.fullTypeName.c_str(),
                        g_appState.inspectedFields.size());
              AddLog(logBuf);
            } else {
              AddLog("[ERROR] Failed to read klass pointer");
            }
          } else {
            AddLog("[ERROR] Failed to read managed pointer");
            ProbeNativeChainFromUi(obj.nativePtr);
          }
        }

        // Native Chain Probe
        ImGui::TableNextColumn();
        char probeBtnId[64];
        sprintf_s(probeBtnId, "Prb##%u", obj.instanceId);
        if (ImGui::SmallButton(probeBtnId)) {
          ProbeNativeChainFromUi(obj.nativePtr);
        }

        // UR Inspect
        ImGui::TableNextColumn();
        char urBtnId[64];
        sprintf_s(urBtnId, "UR##%u", obj.instanceId);
        if (ImGui::SmallButton(urBtnId)) {
          g_urInspectedNativePtr = obj.nativePtr;
          g_urInspectedTypeName = obj.fullTypeName;

          // 检测是否为 GameObject 类型
          g_urIsGameObject = (obj.className == "GameObject");

          if (g_urIsGameObject) {
            // GameObject 模式 - 直接读取
            UnityExternal::NativeGameObject go(obj.nativePtr);
            g_urInspectedName = go.GetName();
            go.GetTag(g_urInspectedTag);
            go.GetComponentCount(g_urInspectedCount);
            go.GetComponentTypeIds(g_urComponentTypes);
          } else {
            // Component 模式 - 重置数据
            g_urInspectedName = "";
            g_urInspectedTag = 0;
            g_urInspectedCount = 0;
            g_urComponentTypes.clear();
          }
          g_showURInspector = true;
        }
      }
    }
    ImGui::EndTable();
  }

  ImGui::End();
}

// ========== Field Inspector Window ==========
void DrawFieldInspector() {
  if (!g_appState.showFieldInspector)
    return;

  ImGui::SetNextWindowSize(ImVec2(600, 400), ImGuiCond_FirstUseEver);
  if (ImGui::Begin("Field Inspector", &g_appState.showFieldInspector)) {
    // Header
    ImGui::Text("Class: %s", g_appState.inspectedClassName.c_str());
    ImGui::Text("Klass Address: 0x%llX",
                (unsigned long long)g_appState.inspectedKlass);
    ImGui::Text("Managed Object: 0x%llX",
                (unsigned long long)g_appState.inspectedManagedPtr);
    ImGui::Text("Native Object: 0x%llX",
                (unsigned long long)g_appState.inspectedNativePtr);
    ImGui::Separator();

    // Get static fields pointer
    std::uintptr_t staticFieldsPtr = 0;
    er2::GetStaticFieldsPtr(er2::Mem(), g_appState.inspectedKlass,
                            er2::g_ctx.off, staticFieldsPtr);

    ImGui::Text("Static Fields Base: 0x%llX",
                (unsigned long long)staticFieldsPtr);
    ImGui::Text("Total Fields: %zu", g_appState.inspectedFields.size());

    const auto &profile = ActiveIl2CppLayoutProfile();
    ImGui::TextDisabled("Runtime Profile: %s", profile.id);
    if (ImGui::Button("Probe This Klass", ImVec2(150, 0))) {
      ProbeRuntimeClassFromUi(g_appState.inspectedKlass);
    }
    if (g_appState.runtimeProbeHasResult &&
        g_appState.runtimeProbeResult.klass == g_appState.inspectedKlass) {
      const auto &runtimeProbe = g_appState.runtimeProbeResult;
      if (runtimeProbe.valid) {
        ImGui::TextColored(ImVec4(0.4f, 1.0f, 0.4f, 1.0f),
                           "Runtime Name: %s",
                           runtimeProbe.fullName.c_str());
        ImGui::Text("Runtime Counts: fields=%u methods=%u",
                    runtimeProbe.fieldCount, runtimeProbe.methodCount);
        ImGui::Text("Profile Fields Ptr: 0x%llX",
                    (unsigned long long)runtimeProbe.fields);
        ImGui::Text("Profile Methods Ptr: 0x%llX",
                    (unsigned long long)runtimeProbe.methods);
        ImGui::Text("Profile Static Fields: 0x%llX",
                    (unsigned long long)runtimeProbe.staticFields);
      } else {
        ImGui::TextDisabled("Runtime Profile Probe: %s",
                            runtimeProbe.failureReason.c_str());
      }
    }

    // ========== Diagnostics Panel ==========
    if (ImGui::CollapsingHeader("Offset Diagnostics (Advanced)")) {
      ImGui::Indent();

      uintptr_t k = g_appState.inspectedKlass;
      // 1. Raw Value Checks
      ImGui::TextDisabled("Checking Klass (0x%llX) Raw Values:",
                          (unsigned long long)k);

      // Helper to read pointers
      auto ReadPtrVal = [&](uint32_t offset) -> uintptr_t {
        uintptr_t val = 0;
        er2::ReadPtr(er2::Mem(), k + offset, val);
        return val;
      };
      // Helper to read uint32
      auto ReadU32 = [&](uint32_t offset) -> uint32_t {
        uint32_t val = 0;
        er2::Mem().Read(k + offset, &val, sizeof(val));
        return val;
      };
      // Helper to read uint16
      auto ReadU16 = [&](uint32_t offset) -> uint16_t {
        uint16_t val = 0;
        er2::Mem().Read(k + offset, &val, sizeof(val));
        return val;
      };

      // Fields Ptr Candidate (Default 0x80)
      ImGui::Text("  +0x80 (Ptr): 0x%llX",
                  (unsigned long long)ReadPtrVal(0x80));

      // Static Fields Ptr Candidate (Default 0xB8)
      ImGui::Text("  +0xB8 (Ptr): 0x%llX",
                  (unsigned long long)ReadPtrVal(0xB8));

      // Field Count Candidates (Default 0x114)
      ImGui::Separator();
      ImGui::Text("Field Count Candidates:");
      ImGui::Text("  +0x110 (U16): %u  (U32): %u", ReadU16(0x110),
                  ReadU32(0x110));
      ImGui::Text("  +0x114 (U16): %u  (U32): %u <== Default", ReadU16(0x114),
                  ReadU32(0x114));
      ImGui::Text("  +0x118 (U16): %u  (U32): %u", ReadU16(0x118),
                  ReadU32(0x118));
      ImGui::Text("  +0x11C (U16): %u  (U32): %u", ReadU16(0x11C),
                  ReadU32(0x11C));
      ImGui::Text("  +0x120 (U16): %u  (U32): %u", ReadU16(0x120),
                  ReadU32(0x120));

      ImGui::Separator();
      ImGui::TextColored(ImVec4(1.0f, 1.0f, 0.0f, 1.0f),
                         "Heuristics Analysis (0x100-0x140):");
      bool found = false;
      for (uint32_t off = 0x100; off <= 0x140; off += 2) {
        uint16_t v = ReadU16(off);
        if (v > 0 && v < 2000) {
          ImGui::BulletText("Offset +0x%X: Value = %u", off, v);
          found = true;
        }
      }
      if (!found)
        ImGui::TextDisabled("No valid field count candidates found.");

      ImGui::Separator();
      if (ImGui::Button("Dump Klass Header (256 bytes) to Log")) {
        std::vector<uint8_t> buffer(256);
        if (er2::Mem().Read(k, buffer.data(), buffer.size())) {
          AddLog("=== Klass Header Dump (0x" + std::to_string(k) + ") ===");
          char lineBuf[256];
          for (size_t i = 0; i < buffer.size(); i += 16) {
            std::string hexStr;
            for (size_t j = 0; j < 16 && i + j < buffer.size(); j++) {
              char byteBuf[8];
              sprintf_s(byteBuf, "%02X ", buffer[i + j]);
              hexStr += byteBuf;
            }
            sprintf_s(lineBuf, "+0x%03X: %s", (uint32_t)i, hexStr.c_str());
            AddLog(lineBuf);
          }
          AddLog("==========================================");
        } else {
          AddLog("[ERROR] Failed to read Klass memory");
        }
      }

      ImGui::Separator();
      if (ImGui::Button("Dump First FieldInfo (32 bytes)")) {
        uintptr_t fieldsPtr = 0;
        if (er2::ReadPtr(er2::Mem(), k + er2::g_ctx.off.il2cppclass_fields,
                         fieldsPtr) &&
            fieldsPtr != 0) {
          std::vector<uint8_t> buffer(32);
          if (er2::Mem().Read(fieldsPtr, buffer.data(), buffer.size())) {
            AddLog("=== First FieldInfo Dump (Offset 0x80 -> " +
                   std::to_string(fieldsPtr) + ") ===");
            char lineBuf[256];
            // Line 1: 0x00 - 0x10
            sprintf_s(lineBuf, "+0x00: %016llX  (Name Ptr?)",
                      *(uint64_t *)&buffer[0]);
            AddLog(lineBuf);
            sprintf_s(lineBuf, "+0x08: %016llX  (Type Ptr?)",
                      *(uint64_t *)&buffer[8]);
            AddLog(lineBuf);
            sprintf_s(lineBuf, "+0x10: %016llX  (Parent Ptr?)",
                      *(uint64_t *)&buffer[16]);
            AddLog(lineBuf);
            sprintf_s(lineBuf, "+0x18: %08X          (Offset?)",
                      *(uint32_t *)&buffer[24]);
            AddLog(lineBuf);
            AddLog("==========================================");
          } else {
            AddLog("[ERROR] Failed to read FieldInfo memory");
          }
        } else {
          AddLog("[ERROR] Failed to read fields pointer (metadata invalid?)");
        }
      }
      ImGui::Unindent();
    }
    ImGui::Separator();

    // Fields Table
    if (ImGui::BeginTable("FieldsTable", 5,
                          ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg |
                              ImGuiTableFlags_ScrollY |
                              ImGuiTableFlags_Resizable)) {
      ImGui::TableSetupColumn("Name", ImGuiTableColumnFlags_WidthStretch);
      ImGui::TableSetupColumn("Offset", ImGuiTableColumnFlags_WidthFixed,
                              80.0f);
      ImGui::TableSetupColumn("Static", ImGuiTableColumnFlags_WidthFixed,
                              50.0f);
      ImGui::TableSetupColumn("Value (Hex)", ImGuiTableColumnFlags_WidthFixed,
                              120.0f);
      ImGui::TableSetupColumn("Copy", ImGuiTableColumnFlags_WidthFixed, 40.0f);
      ImGui::TableHeadersRow();

      for (const auto &field : g_appState.inspectedFields) {
        ImGui::TableNextRow();

        // Name
        ImGui::TableNextColumn();
        if (field.isStatic) {
          ImGui::TextColored(ImVec4(0.5f, 1.0f, 0.5f, 1.0f), "%s",
                             field.name.c_str());
        } else {
          ImGui::Text("%s", field.name.c_str());
        }

        // Offset
        ImGui::TableNextColumn();
        ImGui::Text("0x%X", field.offset);

        // Static
        ImGui::TableNextColumn();
        ImGui::Text("%s", field.isStatic ? "Yes" : "No");

        // Value
        ImGui::TableNextColumn();
        if (field.isStatic && staticFieldsPtr != 0) {
          // Read static field value (assume pointer-sized for simplicity)
          std::uintptr_t value = 0;
          if (er2::Mem().Read(staticFieldsPtr + field.offset, &value,
                              sizeof(value))) {
            ImGui::Text("0x%llX", (unsigned long long)value);
          } else {
            ImGui::TextDisabled("(read error)");
          }
        } else if (!field.isStatic && g_appState.inspectedManagedPtr != 0) {
          // Read instance field value
          std::uintptr_t value = 0;
          if (er2::Mem().Read(g_appState.inspectedManagedPtr + field.offset,
                              &value, sizeof(value))) {
            ImGui::Text("0x%llX", (unsigned long long)value);
          } else {
            ImGui::TextDisabled("(read error)");
          }
        } else {
          ImGui::TextDisabled("-");
        }

        // Copy Button
        ImGui::TableNextColumn();
        char copyId[64];
        sprintf_s(copyId, "##cpf%u", field.offset);
        if (ImGui::SmallButton(copyId)) {
          char buf[256];
          sprintf_s(buf, "%s: offset=0x%X, static=%s", field.name.c_str(),
                    field.offset, field.isStatic ? "true" : "false");
          CopyToClipboard(buf);
          AddLog("[INFO] Copied field info");
        }
      }
      ImGui::EndTable();
    }

    ImGui::Separator();
    if (ImGui::Button("Refresh Fields")) {
      g_appState.inspectedFields.clear();
      er2::EnumerateClassFields(er2::Mem(), g_appState.inspectedKlass,
                                er2::g_ctx.off, g_appState.inspectedFields);
      AddLog("[INFO] Fields refreshed");
    }
    ImGui::SameLine();
    if (ImGui::Button("Close")) {
      g_appState.showFieldInspector = false;
    }
  }
  ImGui::End();
}

void DrawBonesPanel() {
  if (!g_appState.showBonesPanel)
    return;

  ImGui::Begin("Bones Browser", &g_appState.showBonesPanel);

  if (!g_appState.isConnected) {
    ImGui::TextDisabled("Please connect to Unity process first");
    ImGui::End();
    return;
  }

  if (ImGui::Button("Scan Bones", ImVec2(120, 0))) {
    std::thread(RefreshBones).detach();
  }
  ImGui::SameLine();
  ImGui::TextDisabled("(Select a GameObject in the list first)");

  ImGui::Separator();

  std::lock_guard<std::mutex> lock(g_appState.dataMutex);
  ImGui::Text("Total Bones: %zu", g_appState.bones.size());
  ImGui::Separator();

  if (ImGui::BeginTable("BonesTable", 6,
                        ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg |
                            ImGuiTableFlags_ScrollY)) {
    ImGui::TableSetupColumn("Index", ImGuiTableColumnFlags_WidthFixed, 50.0f);
    ImGui::TableSetupColumn("Name", ImGuiTableColumnFlags_WidthStretch);
    ImGui::TableSetupColumn("X", ImGuiTableColumnFlags_WidthFixed, 80.0f);
    ImGui::TableSetupColumn("Y", ImGuiTableColumnFlags_WidthFixed, 80.0f);
    ImGui::TableSetupColumn("Z", ImGuiTableColumnFlags_WidthFixed, 80.0f);
    ImGui::TableSetupColumn("Copy", ImGuiTableColumnFlags_WidthFixed, 50.0f);
    ImGui::TableHeadersRow();

    for (const auto &bone : g_appState.bones) {
      ImGui::TableNextRow();
      ImGui::TableNextColumn();
      ImGui::Text("%d", bone.index);
      ImGui::TableNextColumn();
      ImGui::Text("%s", bone.name.c_str());
      ImGui::TableNextColumn();
      ImGui::Text("%.2f", bone.worldPos[0]);
      ImGui::TableNextColumn();
      ImGui::Text("%.2f", bone.worldPos[1]);
      ImGui::TableNextColumn();
      ImGui::Text("%.2f", bone.worldPos[2]);
      ImGui::TableNextColumn();
      char buttonId[64];
      sprintf_s(buttonId, "##cpb%d", bone.index);
      if (ImGui::SmallButton(buttonId)) {
        char buf[128];
        sprintf_s(buf, "0x%llX (%.2f, %.2f, %.2f)",
                  (unsigned long long)bone.transformAddress, bone.worldPos[0],
                  bone.worldPos[1], bone.worldPos[2]);
        CopyToClipboard(buf);
        AddLog("[INFO] Copied bone info");
      }
    }

    ImGui::EndTable();
  }

  ImGui::End();
}

void DrawDmaModuleSelector() {
  if (!g_appState.showModuleSelector)
    return;

  ImGui::SetNextWindowSize(ImVec2(600, 400), ImGuiCond_FirstUseEver);
  if (ImGui::Begin("Select GameAssembly Module",
                   &g_appState.showModuleSelector)) {

    if (g_appState.isScanningModules) {
      ImGui::BeginDisabled();
      ImGui::Button("Scanning...");
      ImGui::EndDisabled();
    } else {
      if (ImGui::Button("Refresh Module List")) {
        g_appState.isScanningModules = true;
        std::thread([]() {
          // Scan logic using dma
          if (g_appState.runMode != RunMode::DMA || !er2::g_ctx.memory) {
            AddLog("[ERROR] DMA not initialized or not in DMA mode");
            g_appState.isScanningModules = false;
            return;
          }

          // Cast to DMA accessor
          auto dma = std::static_pointer_cast<er2::DMAMemoryAccessor>(
              er2::g_ctx.memory);
          if (!dma) {
            AddLog("[ERROR] Failed to cast memory accessor");
            g_appState.isScanningModules = false;
            return;
          }

          auto mods = dma->EnumerateModules();
          std::vector<AppState::ModuleEntry> newList;
          for (const auto &m : mods) {
            newList.push_back({m.name, m.baseAddress, m.imageSize});
          }

          std::lock_guard<std::mutex> lock(g_appState.dataMutex);
          g_appState.moduleList = std::move(newList);
          char buf[128];
          sprintf_s(buf, "[INFO] Scanned %zu modules",
                    g_appState.moduleList.size());
          AddLog(buf);
          g_appState.isScanningModules = false;
        }).detach();
      }
    }

    // Filter
    static char filterBuf[128] = "GameAssembly";
    ImGui::SameLine();
    ImGui::InputText("Filter", filterBuf, sizeof(filterBuf));

    ImGui::Separator();

    // List
    std::lock_guard<std::mutex> lock(g_appState.dataMutex);
    if (ImGui::BeginTable("Modules", 4,
                          ImGuiTableFlags_Borders | ImGuiTableFlags_ScrollY |
                              ImGuiTableFlags_Resizable)) {
      ImGui::TableSetupColumn("Action", ImGuiTableColumnFlags_WidthFixed, 80);
      ImGui::TableSetupColumn("Name");
      ImGui::TableSetupColumn("Base", ImGuiTableColumnFlags_WidthFixed, 140);
      ImGui::TableSetupColumn("Size", ImGuiTableColumnFlags_WidthFixed, 100);
      ImGui::TableHeadersRow();

      std::string filterLower = filterBuf;
      for (auto &c : filterLower)
        c = tolower(c);

      for (const auto &mod : g_appState.moduleList) {
        // Filter logic
        if (!filterLower.empty()) {
          std::string modNameLower = mod.name;
          for (auto &c : modNameLower)
            c = tolower(c);
          if (modNameLower.find(filterLower) == std::string::npos)
            continue;
        }

        ImGui::TableNextRow();
        ImGui::TableNextColumn();
        char btnId[64];
        sprintf_s(btnId, "Select##%llX", (unsigned long long)mod.base);
        if (ImGui::Button(btnId)) {
          g_appState.gameAssemblyBase = mod.base;
          g_appState.gameAssemblySize = mod.size;
          strncpy_s(g_appState.customGameAssemblyName,
                    sizeof(g_appState.customGameAssemblyName), mod.name.c_str(),
                    sizeof(g_appState.customGameAssemblyName) - 1);

          char logMsg[256];
          sprintf_s(logMsg,
                    "[INFO] Manually selected GameAssembly: %s (0x%llX)",
                    mod.name.c_str(), (unsigned long long)mod.base);
          AddLog(logMsg);
          g_appState.showModuleSelector = false;
        }

        ImGui::TableNextColumn();
        ImGui::Text("%s", mod.name.c_str());

        ImGui::TableNextColumn();
        ImGui::Text("0x%llX", (unsigned long long)mod.base);

        ImGui::TableNextColumn();
        ImGui::Text("0x%X", mod.size);
      }
      ImGui::EndTable();
    }
  }
  ImGui::End();
}

void DrawDmaOffsetScanner() {
  if (!g_appState.showDmaOffsetScanner)
    return;

  ImGui::SetNextWindowSize(ImVec2(980, 560), ImGuiCond_FirstUseEver);
  if (!ImGui::Begin("DMA Offset Scanner", &g_appState.showDmaOffsetScanner)) {
    ImGui::End();
    return;
  }

  if (!g_appState.isConnected) {
    ImGui::TextDisabled("Connect DMA before scanning runtime offsets.");
    ImGui::End();
    return;
  }

  ImGui::Text("Target Module");
  ImGui::SameLine();
  if (g_appState.gameAssemblyBase != 0) {
    ImGui::TextColored(ImVec4(0.35f, 1.0f, 0.45f, 1.0f),
                       "%s @ 0x%llX size=0x%X",
                       g_appState.customGameAssemblyName,
                       (unsigned long long)g_appState.gameAssemblyBase,
                       g_appState.gameAssemblySize);
  } else {
    ImGui::TextColored(ImVec4(1.0f, 0.35f, 0.25f, 1.0f), "Not selected");
  }

  if (ImGui::Button("Select Module")) {
    g_appState.showModuleSelector = true;
  }
  ImGui::SameLine();
  if (ImGui::Button("Fix CR3")) {
    std::thread([]() {
      if (!g_dmaAdapter) {
        AddLog("[ERROR] DMA adapter is not initialized");
        return;
      }

      AddLog("[INFO] Attempting to Fix CR3...");
      if (g_dmaAdapter->FixCr3()) {
        AddLog("[SUCCESS] Fix CR3 Success!");
      } else {
        AddLog("[ERROR] Fix CR3 Failed.");
      }
    }).detach();
  }

  ImGui::Separator();

  int scanMb = static_cast<int>(g_appState.managerRvaScanMaxMb);
  if (ImGui::InputInt("Max bytes per section (MB, 0 = full)",
                      &scanMb)) {
    if (scanMb < 0) {
      scanMb = 0;
    }
    if (scanMb > 2048) {
      scanMb = 2048;
    }
    g_appState.managerRvaScanMaxMb = static_cast<std::uint32_t>(scanMb);
  }

  if (g_appState.isScanningManagerRvas) {
    ImGui::BeginDisabled();
    ImGui::Button("Scanning...", ImVec2(180, 0));
    ImGui::EndDisabled();
    ImGui::SameLine();
    ImGui::ProgressBar(-1.0f * static_cast<float>(ImGui::GetTime()) * 0.35f,
                       ImVec2(240, 0));
  } else {
    if (ImGui::Button("Scan Current GameAssembly", ImVec2(220, 0))) {
      StartManagerRvaScanFromUi();
    }
  }

  ImGui::SameLine();
  if (ImGui::Button("Apply Confirmed RVAs", ImVec2(180, 0))) {
    ApplyConfirmedManagerRvaOffsets();
  }

  std::string statusCopy;
  er2::ManagerRvaScanResult resultCopy;
  {
    std::lock_guard<std::mutex> lock(g_appState.dataMutex);
    statusCopy = g_appState.managerRvaScanStatus;
    resultCopy = g_appState.managerRvaScanResult;
  }

  ImGui::TextWrapped("Status: %s", statusCopy.c_str());
  ImGui::Text("Current Runtime Offsets: Character=0x%llX Entity=0x%llX "
              "Sync=0x%llX UserData=0x%llX GlobalTime=0x%llX",
              (unsigned long long)Offset::m_CharacterManager,
              (unsigned long long)Offset::m_EntityManager,
              (unsigned long long)Offset::m_CharactorSync,
              (unsigned long long)Offset::m_UserDataManager,
              (unsigned long long)Offset::m_GlobalTime);

  ImGui::Separator();

  const er2::ManagerRvaKind kinds[] = {
      er2::ManagerRvaKind::CharacterManager,
      er2::ManagerRvaKind::EntityManager,
      er2::ManagerRvaKind::GlobalTime,
      er2::ManagerRvaKind::CharactorSync,
      er2::ManagerRvaKind::UserDataManager,
  };

  if (ImGui::BeginTable("ManagerRvaSummary", 8,
                        ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg |
                            ImGuiTableFlags_Resizable)) {
    ImGui::TableSetupColumn("Kind", ImGuiTableColumnFlags_WidthFixed, 130.0f);
    ImGui::TableSetupColumn("Confirmed RVA",
                            ImGuiTableColumnFlags_WidthFixed, 105.0f);
    ImGui::TableSetupColumn("Best RVA", ImGuiTableColumnFlags_WidthFixed,
                            95.0f);
    ImGui::TableSetupColumn("Score", ImGuiTableColumnFlags_WidthFixed, 55.0f);
    ImGui::TableSetupColumn("OK", ImGuiTableColumnFlags_WidthFixed, 40.0f);
    ImGui::TableSetupColumn("Resolved", ImGuiTableColumnFlags_WidthFixed,
                            120.0f);
    ImGui::TableSetupColumn("Count/Value", ImGuiTableColumnFlags_WidthFixed,
                            95.0f);
    ImGui::TableSetupColumn("Evidence", ImGuiTableColumnFlags_WidthStretch);
    ImGui::TableHeadersRow();

    for (const er2::ManagerRvaKind kind : kinds) {
      const er2::ManagerRvaCandidate *confirmed =
          er2::FindBestConfirmedManagerRvaCandidate(resultCopy, kind);
      const er2::ManagerRvaCandidate *best =
          er2::FindBestManagerRvaCandidate(resultCopy, kind);

      ImGui::TableNextRow();
      ImGui::TableNextColumn();
      ImGui::Text("%s", ManagerRvaKindUiName(kind));

      ImGui::TableNextColumn();
      if (confirmed) {
        ImGui::TextColored(ImVec4(0.35f, 1.0f, 0.45f, 1.0f), "0x%llX",
                           (unsigned long long)confirmed->rva);
      } else {
        ImGui::TextDisabled("0x0");
      }

      ImGui::TableNextColumn();
      if (best) {
        ImGui::Text("0x%llX", (unsigned long long)best->rva);
      } else {
        ImGui::TextDisabled("-");
      }

      ImGui::TableNextColumn();
      ImGui::Text("%u", best ? best->score : 0);

      ImGui::TableNextColumn();
      ImGui::Text("%s", best && best->confirmed ? "yes" : "no");

      ImGui::TableNextColumn();
      if (best) {
        ImGui::Text("0x%llX", (unsigned long long)best->resolvedPtr);
      } else {
        ImGui::TextDisabled("-");
      }

      ImGui::TableNextColumn();
      if (best && best->count != 0) {
        ImGui::Text("%u", best->count);
      } else if (best && std::isfinite(best->value) && best->value != 0.0) {
        ImGui::Text("%.6f", best->value);
      } else {
        ImGui::TextDisabled("-");
      }

      ImGui::TableNextColumn();
      ImGui::TextWrapped("%s", best ? best->evidence.c_str() : "");
    }
    ImGui::EndTable();
  }

  if (ImGui::CollapsingHeader("All Candidates")) {
    if (ImGui::BeginTable("ManagerRvaCandidates", 9,
                          ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg |
                              ImGuiTableFlags_Resizable |
                              ImGuiTableFlags_ScrollY,
                          ImVec2(0, 230))) {
      ImGui::TableSetupColumn("Kind", ImGuiTableColumnFlags_WidthFixed,
                              120.0f);
      ImGui::TableSetupColumn("RVA", ImGuiTableColumnFlags_WidthFixed, 90.0f);
      ImGui::TableSetupColumn("Score", ImGuiTableColumnFlags_WidthFixed,
                              50.0f);
      ImGui::TableSetupColumn("OK", ImGuiTableColumnFlags_WidthFixed, 35.0f);
      ImGui::TableSetupColumn("Section", ImGuiTableColumnFlags_WidthFixed,
                              65.0f);
      ImGui::TableSetupColumn("Slot VA", ImGuiTableColumnFlags_WidthFixed,
                              115.0f);
      ImGui::TableSetupColumn("Resolved", ImGuiTableColumnFlags_WidthFixed,
                              115.0f);
      ImGui::TableSetupColumn("Chain", ImGuiTableColumnFlags_WidthFixed,
                              220.0f);
      ImGui::TableSetupColumn("Evidence", ImGuiTableColumnFlags_WidthStretch);
      ImGui::TableHeadersRow();

      for (const er2::ManagerRvaCandidate &candidate :
           resultCopy.candidates) {
        ImGui::TableNextRow();
        ImGui::TableNextColumn();
        ImGui::Text("%s", ManagerRvaKindUiName(candidate.kind));
        ImGui::TableNextColumn();
        ImGui::Text("0x%llX", (unsigned long long)candidate.rva);
        ImGui::TableNextColumn();
        ImGui::Text("%u", candidate.score);
        ImGui::TableNextColumn();
        ImGui::Text("%s", candidate.confirmed ? "yes" : "no");
        ImGui::TableNextColumn();
        ImGui::Text("%s", candidate.section.c_str());
        ImGui::TableNextColumn();
        ImGui::Text("0x%llX", (unsigned long long)candidate.slotVa);
        ImGui::TableNextColumn();
        ImGui::Text("0x%llX", (unsigned long long)candidate.resolvedPtr);
        ImGui::TableNextColumn();
        ImGui::TextWrapped("%s", candidate.chain.c_str());
        ImGui::TableNextColumn();
        ImGui::TextWrapped("%s", candidate.evidence.c_str());
      }
      ImGui::EndTable();
    }
  }

  DrawDmaModuleSelector();
  ImGui::End();
}

void DrawNarakaBuffManagerProbe() {
  if (!g_appState.showNarakaBuffManagerProbe) {
    return;
  }

  ImGui::SetNextWindowSize(ImVec2(1180, 650), ImGuiCond_FirstUseEver);
  if (!ImGui::Begin("Naraka BuffManager Probe",
                    &g_appState.showNarakaBuffManagerProbe)) {
    ImGui::End();
    return;
  }

  if (!g_appState.isConnected || !er2::g_ctx.memory) {
    ImGui::TextDisabled("Connect DMA before probing BuffManager.");
    ImGui::End();
    return;
  }

  const auto &rvas = er2::DefaultNarakaManagerRvas();
  ImGui::Text("Target");
  ImGui::SameLine();
  ImGui::Text("GameAssembly=0x%llX BuffManagerRVA=0x%llX Slot=0x%llX",
              (unsigned long long)g_appState.gameAssemblyBase,
              (unsigned long long)rvas.buffManager,
              (unsigned long long)(g_appState.gameAssemblyBase +
                                   static_cast<std::uintptr_t>(
                                       rvas.buffManager)));
  ImGui::TextWrapped("Status: %s",
                     g_appState.narakaBuffProbeStatus.c_str());
  ImGui::Separator();

  ImGui::SetNextItemWidth(130);
  ImGui::InputInt("Max Entries", &g_appState.narakaBuffProbeMaxEntries);
  if (g_appState.narakaBuffProbeMaxEntries < 1) {
    g_appState.narakaBuffProbeMaxEntries = 1;
  }
  if (g_appState.narakaBuffProbeMaxEntries > 8192) {
    g_appState.narakaBuffProbeMaxEntries = 8192;
  }
  ImGui::SameLine();
  ImGui::SetNextItemWidth(110);
  ImGui::InputInt("Scan End", &g_appState.narakaBuffProbeScanEnd, 0x80, 0x200,
                  ImGuiInputTextFlags_CharsHexadecimal);
  if (g_appState.narakaBuffProbeScanEnd < 0x20) {
    g_appState.narakaBuffProbeScanEnd = 0x20;
  }
  if (g_appState.narakaBuffProbeScanEnd > 0x4000) {
    g_appState.narakaBuffProbeScanEnd = 0x4000;
  }
  ImGui::SameLine();
  if (ImGui::Button("Capture Baseline")) {
    CaptureNarakaBuffProbeSnapshot(true,
                                   g_appState.narakaBuffProbeBaseline);
  }
  ImGui::SameLine();
  if (ImGui::Button("Capture Current")) {
    CaptureNarakaBuffProbeSnapshot(false,
                                   g_appState.narakaBuffProbeCurrent);
  }
  ImGui::SameLine();
  ImGui::Checkbox("Auto Poll", &g_appState.narakaBuffProbeAutoPoll);
  ImGui::SameLine();
  ImGui::SetNextItemWidth(120);
  ImGui::SliderFloat("Poll(s)", &g_appState.narakaBuffProbePollInterval, 0.05f,
                     1.0f, "%.2f");
  ImGui::SameLine();
  if (ImGui::Button("Export CSV")) {
    ExportNarakaBuffProbeCsv();
  }

  ImGui::Checkbox("Only Changed", &g_appState.narakaBuffProbeOnlyChanged);
  ImGui::SameLine();
  ImGui::SetNextItemWidth(240);
  ImGui::InputTextWithHint("Filter", "key / value / container hex",
                           g_appState.narakaBuffProbeFilter,
                           sizeof(g_appState.narakaBuffProbeFilter));

  ImGui::Text(
      "Baseline: %s containers=%zu entries=%zu | Current: %s containers=%zu "
      "entries=%zu | Diff rows=%zu",
      g_appState.narakaBuffProbeHasBaseline ? "yes" : "no",
      g_appState.narakaBuffProbeBaseline.containers.size(),
      g_appState.narakaBuffProbeBaseline.entries.size(),
      g_appState.narakaBuffProbeHasCurrent ? "yes" : "no",
      g_appState.narakaBuffProbeCurrent.containers.size(),
      g_appState.narakaBuffProbeCurrent.entries.size(),
      g_appState.narakaBuffProbeDiff.size());
  ImGui::Text("Class=0x%llX StaticFields=0x%llX Instance=0x%llX",
              (unsigned long long)g_appState.narakaBuffProbeCurrent
                  .buffManagerClass,
              (unsigned long long)g_appState.narakaBuffProbeCurrent
                  .staticFields,
              (unsigned long long)g_appState.narakaBuffProbeCurrent.instance);

  if (ImGui::CollapsingHeader("Container Candidates",
                              ImGuiTreeNodeFlags_DefaultOpen)) {
    if (ImGui::BeginTable("NarakaBuffContainerTable", 8,
                          ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg |
                              ImGuiTableFlags_ScrollY |
                              ImGuiTableFlags_Resizable,
                          ImVec2(0, 165))) {
      ImGui::TableSetupColumn("Owner", ImGuiTableColumnFlags_WidthFixed, 90);
      ImGui::TableSetupColumn("Off", ImGuiTableColumnFlags_WidthFixed, 55);
      ImGui::TableSetupColumn("Kind", ImGuiTableColumnFlags_WidthFixed, 85);
      ImGui::TableSetupColumn("Owner VA", ImGuiTableColumnFlags_WidthFixed,
                              135);
      ImGui::TableSetupColumn("Container", ImGuiTableColumnFlags_WidthFixed,
                              135);
      ImGui::TableSetupColumn("Array", ImGuiTableColumnFlags_WidthFixed, 135);
      ImGui::TableSetupColumn("Count", ImGuiTableColumnFlags_WidthFixed, 65);
      ImGui::TableSetupColumn("Evidence", ImGuiTableColumnFlags_WidthStretch);
      ImGui::TableHeadersRow();

      for (const auto &container :
           g_appState.narakaBuffProbeCurrent.containers) {
        ImGui::TableNextRow();
        ImGui::TableNextColumn();
        ImGui::Text("%s",
                    er2::NarakaBuffContainerOwnerKindName(
                        container.ownerKind));
        ImGui::TableNextColumn();
        ImGui::Text("0x%X", container.ownerOffset);
        ImGui::TableNextColumn();
        ImGui::Text("%s",
                    er2::NarakaBuffContainerKindName(container.kind));
        ImGui::TableNextColumn();
        ImGui::Text("0x%llX", (unsigned long long)container.owner);
        ImGui::TableNextColumn();
        ImGui::Text("0x%llX", (unsigned long long)container.container);
        ImGui::TableNextColumn();
        ImGui::Text("0x%llX", (unsigned long long)container.backingArray);
        ImGui::TableNextColumn();
        ImGui::Text("%d", container.count);
        ImGui::TableNextColumn();
        ImGui::TextWrapped("%s", container.evidence.c_str());
      }
      ImGui::EndTable();
    }
  }

  ImGui::Separator();
  const std::string filter = g_appState.narakaBuffProbeFilter;
  if (ImGui::BeginTable("NarakaBuffDiffTable", 14,
                        ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg |
                            ImGuiTableFlags_ScrollY |
                            ImGuiTableFlags_Resizable,
                        ImVec2(0, 305))) {
    ImGui::TableSetupColumn("State", ImGuiTableColumnFlags_WidthFixed, 72);
    ImGui::TableSetupColumn("Kind", ImGuiTableColumnFlags_WidthFixed, 82);
    ImGui::TableSetupColumn("Container", ImGuiTableColumnFlags_WidthFixed,
                            135);
    ImGui::TableSetupColumn("Entry VA", ImGuiTableColumnFlags_WidthFixed, 135);
    ImGui::TableSetupColumn("Key", ImGuiTableColumnFlags_WidthFixed, 86);
    ImGui::TableSetupColumn("Value Ptr", ImGuiTableColumnFlags_WidthFixed,
                            135);
    ImGui::TableSetupColumn("Klass", ImGuiTableColumnFlags_WidthFixed, 135);
    ImGui::TableSetupColumn("Base0", ImGuiTableColumnFlags_WidthFixed, 70);
    ImGui::TableSetupColumn("Now0", ImGuiTableColumnFlags_WidthFixed, 70);
    ImGui::TableSetupColumn("Base1", ImGuiTableColumnFlags_WidthFixed, 70);
    ImGui::TableSetupColumn("Now1", ImGuiTableColumnFlags_WidthFixed, 70);
    ImGui::TableSetupColumn("Base2", ImGuiTableColumnFlags_WidthFixed, 70);
    ImGui::TableSetupColumn("Now2", ImGuiTableColumnFlags_WidthFixed, 70);
    ImGui::TableSetupColumn("Now3", ImGuiTableColumnFlags_WidthFixed, 70);
    ImGui::TableHeadersRow();

    for (const auto &row : g_appState.narakaBuffProbeDiff) {
      if (g_appState.narakaBuffProbeOnlyChanged && !row.changed) {
        continue;
      }

      if (!filter.empty()) {
        const std::string keyText = std::to_string(row.current.key);
        const std::string valueText = HexFromPtr(row.current.valuePtr);
        const std::string containerText = HexFromPtr(row.current.container);
        if (keyText.find(filter) == std::string::npos &&
            valueText.find(filter) == std::string::npos &&
            containerText.find(filter) == std::string::npos) {
          continue;
        }
      }

      ImGui::TableNextRow();
      ImGui::TableNextColumn();
      if (row.added) {
        ImGui::TextColored(ImVec4(0.4f, 1.0f, 0.55f, 1.0f), "added");
      } else if (row.changed) {
        ImGui::TextColored(ImVec4(1.0f, 0.85f, 0.2f, 1.0f), "changed");
      } else {
        ImGui::TextDisabled("same");
      }
      ImGui::TableNextColumn();
      ImGui::Text("%s",
                  er2::NarakaBuffContainerKindName(
                      row.current.containerKind));
      ImGui::TableNextColumn();
      ImGui::Text("0x%llX", (unsigned long long)row.current.container);
      ImGui::TableNextColumn();
      ImGui::Text("0x%llX", (unsigned long long)row.current.entryAddress);
      ImGui::TableNextColumn();
      ImGui::Text("%d", row.current.key);
      ImGui::TableNextColumn();
      ImGui::Text("0x%llX", (unsigned long long)row.current.valuePtr);
      ImGui::TableNextColumn();
      ImGui::Text("0x%llX", (unsigned long long)row.current.valueKlass);
      ImGui::TableNextColumn();
      ImGui::Text("%d", row.baselineInt0);
      ImGui::TableNextColumn();
      ImGui::Text("%d", row.current.int0);
      ImGui::TableNextColumn();
      ImGui::Text("%d", row.baselineInt1);
      ImGui::TableNextColumn();
      ImGui::Text("%d", row.current.int1);
      ImGui::TableNextColumn();
      ImGui::Text("%d", row.baselineInt2);
      ImGui::TableNextColumn();
      ImGui::Text("%d", row.current.int2);
      ImGui::TableNextColumn();
      ImGui::Text("%d", row.current.int3);
    }
    ImGui::EndTable();
  }

  ImGui::End();
}

void DrawNarakaActorContainerProbe() {
  if (!g_appState.showNarakaActorContainerProbe) {
    return;
  }

  ImGui::SetNextWindowSize(ImVec2(1220, 690), ImGuiCond_FirstUseEver);
  if (!ImGui::Begin("Naraka Actor Container Probe",
                    &g_appState.showNarakaActorContainerProbe)) {
    ImGui::End();
    return;
  }

  if (!g_appState.isConnected || !er2::g_ctx.memory) {
    ImGui::TextDisabled("Connect DMA before probing Actor containers.");
    ImGui::End();
    return;
  }

  const std::uintptr_t selectedActor = GetSelectedNarakaProbeActor();
  ImGui::Text("Selected ActorModel=0x%llX", (unsigned long long)selectedActor);
  ImGui::SameLine();
  if (ImGui::Button("Refresh Actors")) {
    RefreshNarakaProbeActors();
  }
  ImGui::TextWrapped("Status: %s",
                     g_appState.narakaActorContainerStatus.c_str());
  ImGui::Separator();

  ImGui::SetNextItemWidth(130);
  ImGui::InputInt("Max Entries", &g_appState.narakaActorContainerMaxEntries);
  if (g_appState.narakaActorContainerMaxEntries < 1) {
    g_appState.narakaActorContainerMaxEntries = 1;
  }
  if (g_appState.narakaActorContainerMaxEntries > 32768) {
    g_appState.narakaActorContainerMaxEntries = 32768;
  }
  ImGui::SameLine();
  ImGui::SetNextItemWidth(110);
  ImGui::InputInt("Scan End", &g_appState.narakaActorContainerScanEnd, 0x100,
                  0x400, ImGuiInputTextFlags_CharsHexadecimal);
  if (g_appState.narakaActorContainerScanEnd < 0x20) {
    g_appState.narakaActorContainerScanEnd = 0x20;
  }
  if (g_appState.narakaActorContainerScanEnd > 0x8000) {
    g_appState.narakaActorContainerScanEnd = 0x8000;
  }
  ImGui::SameLine();
  if (ImGui::Button("Capture Baseline")) {
    CaptureNarakaActorContainerSnapshotUi(
        true, g_appState.narakaActorContainerBaseline);
  }
  ImGui::SameLine();
  if (ImGui::Button("Capture Current")) {
    CaptureNarakaActorContainerSnapshotUi(
        false, g_appState.narakaActorContainerCurrent);
  }
  ImGui::SameLine();
  ImGui::Checkbox("Auto Poll", &g_appState.narakaActorContainerAutoPoll);
  ImGui::SameLine();
  ImGui::SetNextItemWidth(120);
  ImGui::SliderFloat("Poll(s)", &g_appState.narakaActorContainerPollInterval,
                     0.05f, 1.0f, "%.2f");
  ImGui::SameLine();
  if (ImGui::Button("Export CSV")) {
    ExportNarakaActorContainerCsv();
  }

  ImGui::Checkbox("Only Changed", &g_appState.narakaActorContainerOnlyChanged);
  ImGui::SameLine();
  ImGui::SetNextItemWidth(260);
  ImGui::InputTextWithHint("Filter", "owner / key / value / klass hex",
                           g_appState.narakaActorContainerFilter,
                           sizeof(g_appState.narakaActorContainerFilter));

  ImGui::Text(
      "Baseline: %s containers=%zu entries=%zu | Current: %s containers=%zu "
      "entries=%zu | Container diffs=%zu | Entry diffs=%zu",
      g_appState.narakaActorContainerHasBaseline ? "yes" : "no",
      g_appState.narakaActorContainerBaseline.containers.size(),
      g_appState.narakaActorContainerBaseline.entries.size(),
      g_appState.narakaActorContainerHasCurrent ? "yes" : "no",
      g_appState.narakaActorContainerCurrent.containers.size(),
      g_appState.narakaActorContainerCurrent.entries.size(),
      g_appState.narakaActorContainerDiff.size(),
      g_appState.narakaActorContainerEntryDiff.size());
  ImGui::Text("Hero=%u Team=%u PropertyData=0x%llX RuntimePropertyData=0x%llX",
              g_appState.narakaActorContainerCurrent.heroId,
              g_appState.narakaActorContainerCurrent.teamId,
              (unsigned long long)
                  g_appState.narakaActorContainerCurrent.propertyData,
              (unsigned long long)g_appState.narakaActorContainerCurrent
                  .runtimePropertyData);

  if (ImGui::CollapsingHeader("Container Count Diff",
                              ImGuiTreeNodeFlags_DefaultOpen)) {
    if (ImGui::BeginTable("NarakaActorContainerTable", 10,
                          ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg |
                              ImGuiTableFlags_ScrollY |
                              ImGuiTableFlags_Resizable,
                          ImVec2(0, 185))) {
      ImGui::TableSetupColumn("State", ImGuiTableColumnFlags_WidthFixed, 70);
      ImGui::TableSetupColumn("Owner", ImGuiTableColumnFlags_WidthFixed, 115);
      ImGui::TableSetupColumn("Off", ImGuiTableColumnFlags_WidthFixed, 55);
      ImGui::TableSetupColumn("Kind", ImGuiTableColumnFlags_WidthFixed, 85);
      ImGui::TableSetupColumn("Owner VA", ImGuiTableColumnFlags_WidthFixed,
                              135);
      ImGui::TableSetupColumn("Container", ImGuiTableColumnFlags_WidthFixed,
                              135);
      ImGui::TableSetupColumn("Array", ImGuiTableColumnFlags_WidthFixed, 135);
      ImGui::TableSetupColumn("Base", ImGuiTableColumnFlags_WidthFixed, 58);
      ImGui::TableSetupColumn("Now", ImGuiTableColumnFlags_WidthFixed, 58);
      ImGui::TableSetupColumn("Evidence", ImGuiTableColumnFlags_WidthStretch);
      ImGui::TableHeadersRow();

      for (const auto &row : g_appState.narakaActorContainerDiff) {
        if (g_appState.narakaActorContainerOnlyChanged && !row.changed) {
          continue;
        }
        ImGui::TableNextRow();
        ImGui::TableNextColumn();
        if (row.added) {
          ImGui::TextColored(ImVec4(0.4f, 1.0f, 0.55f, 1.0f), "added");
        } else if (row.changed) {
          ImGui::TextColored(ImVec4(1.0f, 0.85f, 0.2f, 1.0f), "changed");
        } else {
          ImGui::TextDisabled("same");
        }
        ImGui::TableNextColumn();
        ImGui::Text("%s", er2::NarakaActorContainerOwnerKindName(
                              row.current.ownerKind));
        ImGui::TableNextColumn();
        ImGui::Text("0x%X", row.current.ownerOffset);
        ImGui::TableNextColumn();
        ImGui::Text("%s",
                    er2::NarakaActorContainerKindName(row.current.kind));
        ImGui::TableNextColumn();
        ImGui::Text("0x%llX", (unsigned long long)row.current.owner);
        ImGui::TableNextColumn();
        ImGui::Text("0x%llX", (unsigned long long)row.current.container);
        ImGui::TableNextColumn();
        ImGui::Text("0x%llX", (unsigned long long)row.current.backingArray);
        ImGui::TableNextColumn();
        ImGui::Text("%d", row.baselineCount);
        ImGui::TableNextColumn();
        ImGui::Text("%d", row.current.count);
        ImGui::TableNextColumn();
        ImGui::TextWrapped("%s", row.current.evidence.c_str());
      }
      ImGui::EndTable();
    }
  }

  ImGui::Separator();
  const std::string filter = g_appState.narakaActorContainerFilter;
  if (ImGui::BeginTable("NarakaActorEntryDiffTable", 17,
                        ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg |
                            ImGuiTableFlags_ScrollY |
                            ImGuiTableFlags_Resizable,
                        ImVec2(0, 330))) {
    ImGui::TableSetupColumn("State", ImGuiTableColumnFlags_WidthFixed, 72);
    ImGui::TableSetupColumn("Owner", ImGuiTableColumnFlags_WidthFixed, 110);
    ImGui::TableSetupColumn("Off", ImGuiTableColumnFlags_WidthFixed, 55);
    ImGui::TableSetupColumn("Kind", ImGuiTableColumnFlags_WidthFixed, 82);
    ImGui::TableSetupColumn("Container", ImGuiTableColumnFlags_WidthFixed,
                            135);
    ImGui::TableSetupColumn("Key", ImGuiTableColumnFlags_WidthFixed, 86);
    ImGui::TableSetupColumn("Raw", ImGuiTableColumnFlags_WidthFixed, 135);
    ImGui::TableSetupColumn("Value", ImGuiTableColumnFlags_WidthFixed, 135);
    ImGui::TableSetupColumn("Klass", ImGuiTableColumnFlags_WidthFixed, 135);
    ImGui::TableSetupColumn("Base0", ImGuiTableColumnFlags_WidthFixed, 70);
    ImGui::TableSetupColumn("Now0", ImGuiTableColumnFlags_WidthFixed, 70);
    ImGui::TableSetupColumn("Base1", ImGuiTableColumnFlags_WidthFixed, 70);
    ImGui::TableSetupColumn("Now1", ImGuiTableColumnFlags_WidthFixed, 70);
    ImGui::TableSetupColumn("Base2", ImGuiTableColumnFlags_WidthFixed, 70);
    ImGui::TableSetupColumn("Now2", ImGuiTableColumnFlags_WidthFixed, 70);
    ImGui::TableSetupColumn("Base3", ImGuiTableColumnFlags_WidthFixed, 70);
    ImGui::TableSetupColumn("Now3", ImGuiTableColumnFlags_WidthFixed, 70);
    ImGui::TableHeadersRow();

    for (const auto &row : g_appState.narakaActorContainerEntryDiff) {
      if (g_appState.narakaActorContainerOnlyChanged && !row.changed) {
        continue;
      }

      if (!filter.empty()) {
        const std::string ownerText =
            er2::NarakaActorContainerOwnerKindName(row.current.ownerKind);
        const std::string keyText = std::to_string(row.current.key);
        const std::string rawText = HexFromPtr(row.current.rawValue);
        const std::string valueText = HexFromPtr(row.current.valuePtr);
        const std::string klassText = HexFromPtr(row.current.valueKlass);
        if (ownerText.find(filter) == std::string::npos &&
            keyText.find(filter) == std::string::npos &&
            rawText.find(filter) == std::string::npos &&
            valueText.find(filter) == std::string::npos &&
            klassText.find(filter) == std::string::npos) {
          continue;
        }
      }

      ImGui::TableNextRow();
      ImGui::TableNextColumn();
      if (row.added) {
        ImGui::TextColored(ImVec4(0.4f, 1.0f, 0.55f, 1.0f), "added");
      } else if (row.changed) {
        ImGui::TextColored(ImVec4(1.0f, 0.85f, 0.2f, 1.0f), "changed");
      } else {
        ImGui::TextDisabled("same");
      }
      ImGui::TableNextColumn();
      ImGui::Text("%s", er2::NarakaActorContainerOwnerKindName(
                            row.current.ownerKind));
      ImGui::TableNextColumn();
      ImGui::Text("0x%X", row.current.ownerOffset);
      ImGui::TableNextColumn();
      ImGui::Text("%s", er2::NarakaActorContainerKindName(
                            row.current.containerKind));
      ImGui::TableNextColumn();
      ImGui::Text("0x%llX", (unsigned long long)row.current.container);
      ImGui::TableNextColumn();
      ImGui::Text("%d", row.current.key);
      ImGui::TableNextColumn();
      ImGui::Text("0x%llX", (unsigned long long)row.current.rawValue);
      ImGui::TableNextColumn();
      ImGui::Text("0x%llX", (unsigned long long)row.current.valuePtr);
      ImGui::TableNextColumn();
      ImGui::Text("0x%llX", (unsigned long long)row.current.valueKlass);
      ImGui::TableNextColumn();
      ImGui::Text("%d", row.baselineInt0);
      ImGui::TableNextColumn();
      ImGui::Text("%d", row.current.int0);
      ImGui::TableNextColumn();
      ImGui::Text("%d", row.baselineInt1);
      ImGui::TableNextColumn();
      ImGui::Text("%d", row.current.int1);
      ImGui::TableNextColumn();
      ImGui::Text("%d", row.baselineInt2);
      ImGui::TableNextColumn();
      ImGui::Text("%d", row.current.int2);
      ImGui::TableNextColumn();
      ImGui::Text("%d", row.baselineInt3);
      ImGui::TableNextColumn();
      ImGui::Text("%d", row.current.int3);
    }
    ImGui::EndTable();
  }

  ImGui::End();
}

void DrawNarakaPropertyProbe() {
  if (!g_appState.showNarakaPropertyProbe) {
    return;
  }

  ImGui::SetNextWindowSize(ImVec2(1100, 620), ImGuiCond_FirstUseEver);
  if (!ImGui::Begin("Naraka Runtime Property Probe",
                    &g_appState.showNarakaPropertyProbe)) {
    ImGui::End();
    return;
  }

  if (!g_appState.isConnected || !er2::g_ctx.memory) {
    ImGui::TextDisabled("Connect DMA before probing runtime properties.");
    ImGui::End();
    return;
  }

  ImGui::Text("Target");
  ImGui::SameLine();
  ImGui::Text("GameAssembly=0x%llX CharacterRVA=0x%llX",
              (unsigned long long)g_appState.gameAssemblyBase,
              (unsigned long long)er2::DefaultNarakaManagerRvas()
                  .characterManager);

  if (ImGui::Button("Refresh Actors")) {
    RefreshNarakaProbeActors();
  }
  ImGui::SameLine();
  ImGui::Checkbox("Only HeroID 1000006", &g_appState.narakaProbeOnlyTarka);
  ImGui::SameLine();
  ImGui::SetNextItemWidth(210);
  ImGui::InputTextWithHint("Manual ActorModel", "0x...",
                           g_appState.narakaProbeActorAddress,
                           sizeof(g_appState.narakaProbeActorAddress));

  ImGui::TextWrapped("Status: %s", g_appState.narakaProbeStatus.c_str());
  ImGui::Separator();

  if (ImGui::BeginChild("NarakaProbeActors", ImVec2(0, 160), true)) {
    std::lock_guard<std::mutex> lock(g_appState.dataMutex);
    if (ImGui::BeginTable("NarakaProbeActorTable", 6,
                          ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg |
                              ImGuiTableFlags_ScrollY |
                              ImGuiTableFlags_Resizable)) {
      ImGui::TableSetupColumn("Use", ImGuiTableColumnFlags_WidthFixed, 48);
      ImGui::TableSetupColumn("ActorModel", ImGuiTableColumnFlags_WidthFixed,
                              145);
      ImGui::TableSetupColumn("Hero", ImGuiTableColumnFlags_WidthFixed, 70);
      ImGui::TableSetupColumn("Team", ImGuiTableColumnFlags_WidthFixed, 55);
      ImGui::TableSetupColumn("PropertyData", ImGuiTableColumnFlags_WidthFixed,
                              145);
      ImGui::TableSetupColumn("RuntimePropertyData",
                              ImGuiTableColumnFlags_WidthStretch);
      ImGui::TableHeadersRow();

      for (int i = 0;
           i < static_cast<int>(g_appState.narakaActorProbeEntries.size());
           ++i) {
        const auto &entry = g_appState.narakaActorProbeEntries[i];
        if (g_appState.narakaProbeOnlyTarka && entry.heroId != 1000006) {
          continue;
        }

        ImGui::TableNextRow();
        ImGui::TableNextColumn();
        char selectId[32];
        sprintf_s(selectId, "Use##naraka_actor_%d", i);
        if (ImGui::SmallButton(selectId)) {
          g_appState.narakaProbeSelectedActor = i;
          sprintf_s(g_appState.narakaProbeActorAddress,
                    sizeof(g_appState.narakaProbeActorAddress), "0x%llX",
                    (unsigned long long)entry.actorModel);
        }
        ImGui::TableNextColumn();
        if (i == g_appState.narakaProbeSelectedActor) {
          ImGui::TextColored(ImVec4(0.35f, 1.0f, 0.45f, 1.0f), "0x%llX",
                             (unsigned long long)entry.actorModel);
        } else {
          ImGui::Text("0x%llX", (unsigned long long)entry.actorModel);
        }
        ImGui::TableNextColumn();
        ImGui::Text("%u", entry.heroId);
        ImGui::TableNextColumn();
        ImGui::Text("%u", entry.teamId);
        ImGui::TableNextColumn();
        ImGui::Text("0x%llX", (unsigned long long)entry.propertyData);
        ImGui::TableNextColumn();
        ImGui::Text("0x%llX", (unsigned long long)entry.runtimePropertyData);
      }
      ImGui::EndTable();
    }
  }
  ImGui::EndChild();

  ImGui::SetNextItemWidth(120);
  ImGui::InputInt("Max Rows", &g_appState.narakaProbeMaxRows);
  if (g_appState.narakaProbeMaxRows < 1) {
    g_appState.narakaProbeMaxRows = 1;
  }
  if (g_appState.narakaProbeMaxRows > 4096) {
    g_appState.narakaProbeMaxRows = 4096;
  }
  ImGui::SameLine();
  if (ImGui::Button("Capture Baseline")) {
    CaptureNarakaProbeSnapshot(true, g_appState.narakaProbeBaseline);
  }
  ImGui::SameLine();
  if (ImGui::Button("Capture Current")) {
    CaptureNarakaProbeSnapshot(false, g_appState.narakaProbeCurrent);
  }
  ImGui::SameLine();
  ImGui::Checkbox("Auto Poll", &g_appState.narakaProbeAutoPoll);
  ImGui::SameLine();
  ImGui::SetNextItemWidth(120);
  ImGui::SliderFloat("Poll(s)", &g_appState.narakaProbePollInterval, 0.05f,
                     1.0f, "%.2f");
  ImGui::SameLine();
  if (ImGui::Button("Export CSV")) {
    ExportNarakaProbeCsv();
  }

  ImGui::Checkbox("Only Changed", &g_appState.narakaProbeOnlyChanged);
  ImGui::SameLine();
  ImGui::SetNextItemWidth(180);
  ImGui::InputTextWithHint("DataID filter", "e.g. 11002 or 999",
                           g_appState.narakaProbeFilterDataId,
                           sizeof(g_appState.narakaProbeFilterDataId));

  ImGui::Text("Baseline: %s rows=%zu | Current: %s rows=%zu | Diff rows=%zu",
              g_appState.narakaProbeHasBaseline ? "yes" : "no",
              g_appState.narakaProbeBaseline.rows.size(),
              g_appState.narakaProbeHasCurrent ? "yes" : "no",
              g_appState.narakaProbeCurrent.rows.size(),
              g_appState.narakaProbeDiff.size());

  ImGui::Separator();
  const std::string filter = g_appState.narakaProbeFilterDataId;
  if (ImGui::BeginTable("NarakaProbeDiffTable", 10,
                        ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg |
                            ImGuiTableFlags_ScrollY |
                            ImGuiTableFlags_Resizable,
                        ImVec2(0, 310))) {
    ImGui::TableSetupColumn("DataID", ImGuiTableColumnFlags_WidthFixed, 80);
    ImGui::TableSetupColumn("Index", ImGuiTableColumnFlags_WidthFixed, 60);
    ImGui::TableSetupColumn("Value VA", ImGuiTableColumnFlags_WidthFixed, 135);
    ImGui::TableSetupColumn("Base Int", ImGuiTableColumnFlags_WidthFixed, 90);
    ImGui::TableSetupColumn("Now Int", ImGuiTableColumnFlags_WidthFixed, 90);
    ImGui::TableSetupColumn("Delta", ImGuiTableColumnFlags_WidthFixed, 80);
    ImGui::TableSetupColumn("Base F", ImGuiTableColumnFlags_WidthFixed, 90);
    ImGui::TableSetupColumn("Now F", ImGuiTableColumnFlags_WidthFixed, 90);
    ImGui::TableSetupColumn("dF", ImGuiTableColumnFlags_WidthFixed, 80);
    ImGui::TableSetupColumn("Changed", ImGuiTableColumnFlags_WidthFixed, 70);
    ImGui::TableHeadersRow();

    for (const auto &row : g_appState.narakaProbeDiff) {
      if (g_appState.narakaProbeOnlyChanged && !row.changed) {
        continue;
      }
      if (!filter.empty() &&
          std::to_string(row.dataId).find(filter) == std::string::npos) {
        continue;
      }

      ImGui::TableNextRow();
      ImGui::TableNextColumn();
      if (row.changed) {
        ImGui::TextColored(ImVec4(1.0f, 0.85f, 0.2f, 1.0f), "%d",
                           row.dataId);
      } else {
        ImGui::Text("%d", row.dataId);
      }
      ImGui::TableNextColumn();
      ImGui::Text("%d", row.index);
      ImGui::TableNextColumn();
      ImGui::Text("0x%llX", (unsigned long long)row.valueAddress);
      ImGui::TableNextColumn();
      ImGui::Text("%d", row.baselineInt);
      ImGui::TableNextColumn();
      ImGui::Text("%d", row.currentInt);
      ImGui::TableNextColumn();
      ImGui::Text("%d", row.intDelta);
      ImGui::TableNextColumn();
      ImGui::Text("%.3f", row.baselineFloat);
      ImGui::TableNextColumn();
      ImGui::Text("%.3f", row.currentFloat);
      ImGui::TableNextColumn();
      ImGui::Text("%.3f", row.floatDelta);
      ImGui::TableNextColumn();
      ImGui::Text("%s", row.changed ? "yes" : "no");
    }
    ImGui::EndTable();
  }

  ImGui::End();
}

void DrawMetadataPanel() {
  if (!g_appState.showMetadataPanel)
    return;

  ImGui::Begin("Metadata Export", &g_appState.showMetadataPanel,
               ImGuiWindowFlags_AlwaysAutoResize);

  if (!g_appState.isConnected) {
    ImGui::TextDisabled("Please connect to Unity process first");
    ImGui::End();
    return;
  }

  ImGui::Text("Export IL2CPP Metadata");
  ImGui::Separator();

  // Manual Module Selection Info
  ImGui::Text("Target Module: ");
  ImGui::SameLine();
  if (g_appState.gameAssemblyBase != 0) {
    ImGui::TextColored(ImVec4(0, 1, 0, 1), "0x%llX (Size: 0x%X)",
                       (unsigned long long)g_appState.gameAssemblyBase,
                       g_appState.gameAssemblySize);
    if (g_appState.customGameAssemblyName[0]) {
      ImGui::SameLine();
      ImGui::TextDisabled("(%s)", g_appState.customGameAssemblyName);
    }
  } else {
    ImGui::TextColored(ImVec4(1, 0, 0, 1), "Not Detected");
  }

  if (ImGui::Button("Manual Select (DMA Scan)")) {
    g_appState.showModuleSelector = true;
  }

  // Call the selector window function (it handles its own window)
  DrawDmaModuleSelector();

  ImGui::Separator();
  if (ImGui::CollapsingHeader("Debug Context")) {
    ImGui::Text("PID: %u", er2::Pid());
    ImGui::Text("UnityPlayer: 0x%llX",
                (unsigned long long)er2::UnityPlayerBase());
    ImGui::Text("Accessor Inited: %s", er2::IsInited() ? "Yes" : "No");

    uintptr_t gaBase = 0;
    if (g_appState.gameAssemblyBase != 0) {
      gaBase = g_appState.gameAssemblyBase;
    }
    ImGui::Text("Active GA Base: 0x%llX", (unsigned long long)gaBase);
    ImGui::Text("Custom Name Used: %s",
                g_appState.useCustomModuleNames ? "Yes" : "No");
    if (g_appState.useCustomModuleNames) {
      ImGui::Text("Custom Name: %s", g_appState.customGameAssemblyName);
    }
  }

  // [DMA Diagnostic] Test if DMA can read memory at all
  ImGui::Separator();
  if (ImGui::CollapsingHeader("DMA Diagnostics")) {
    ImGui::TextColored(ImVec4(1.0f, 0.8f, 0.0f, 1.0f),
                       "Use these to diagnose DMA read issues:");
    ImGui::Spacing();

    if (ImGui::Button("Test: Read PE Header (GameAssembly)", ImVec2(280, 30))) {
      std::thread([]() {
        if (g_appState.gameAssemblyBase == 0) {
          AddLog("[ERROR] GameAssembly base not set. Select module first.");
          return;
        }

        AddLog("========================================");
        AddLog("[DMA Test] Testing PE Header read...");
        char buf[256];
        sprintf_s(buf, "[DMA Test] Target Address: 0x%llX",
                  (unsigned long long)g_appState.gameAssemblyBase);
        AddLog(buf);

        // Try to read DOS Header (first 64 bytes - should always be in memory)
        uint8_t dosHeader[64] = {0};
        bool readOk = er2::Mem().Read(g_appState.gameAssemblyBase, dosHeader,
                                      sizeof(dosHeader));

        if (readOk) {
          // Check for MZ signature
          if (dosHeader[0] == 'M' && dosHeader[1] == 'Z') {
            AddLog(
                "[DMA Test] SUCCESS: DOS Header readable (MZ signature found)");

            // Read e_lfanew (offset to PE header)
            uint32_t e_lfanew = *reinterpret_cast<uint32_t *>(&dosHeader[0x3C]);
            sprintf_s(buf, "[DMA Test] e_lfanew = 0x%X", e_lfanew);
            AddLog(buf);

            // Try reading NT Header signature
            uint32_t peSignature = 0;
            if (er2::Mem().Read(g_appState.gameAssemblyBase + e_lfanew,
                                &peSignature, 4)) {
              if (peSignature == 0x00004550) { // "PE\0\0"
                AddLog("[DMA Test] SUCCESS: NT Header readable (PE signature "
                       "found)");
                AddLog("[SUCCESS] DMA connection is HEALTHY for this module!");
              } else {
                sprintf_s(
                    buf,
                    "[DMA Test] WARNING: Got signature 0x%X (expected 0x4550)",
                    peSignature);
                AddLog(buf);
              }
            } else {
              AddLog("[DMA Test] FAILED: Cannot read NT Header");
            }
          } else {
            sprintf_s(buf,
                      "[DMA Test] WARNING: DOS Header invalid. Got: 0x%02X%02X "
                      "(expected 'MZ')",
                      dosHeader[0], dosHeader[1]);
            AddLog(buf);
            AddLog("[DMA Test] This indicates garbage data or wrong address.");
          }
        } else {
          AddLog("[DMA Test] FAILED: Cannot read DOS Header (64 bytes) at base "
                 "address");
          AddLog("[CRITICAL] DMA cannot read GameAssembly.dll module memory!");
          AddLog("[HINT] Try: 1. Fix CR3 (DTB) button  2. Reconnect  3. Check "
                 "if game uses anti-DMA protection");
        }

        AddLog("========================================");
      }).detach();
    }

    if (ImGui::Button("Test: Read Heap Address (MetaBase)", ImVec2(280, 30))) {
      std::thread([]() {
        AddLog("========================================");
        AddLog("[DMA Test] Finding and testing Metadata address...");

        if (g_appState.gameAssemblyBase == 0) {
          AddLog("[ERROR] GameAssembly base not set.");
          return;
        }

        // Use strict pointer scan only; direct heuristic can hit module-local
        // false positives in packed/modded GameAssembly builds.
        er2::FoundMetadata found =
            er2::FindMetadataPointerStrict(er2::Mem(),
                                           g_appState.gameAssemblyBase,
                                           0x200000u, 8192, 15.0, 0);

        if (found.metaBase == 0) {
          AddLog("[DMA Test] Strict pointer scan returned no valid metadata.");
          AddLog(
              "[CRITICAL] Could not find a 0xFAB11BAF metadata blob pointer in "
              "module data.");
          AddLog("[HINT] For this CTF build, metadata may be encrypted, "
                 "relocated, or reconstructed only after custom loader logic.");
          return;
        }

        char buf[256];
        sprintf_s(buf, "[DMA Test] Scan returned MetaBase: 0x%llX",
                  (unsigned long long)found.metaBase);
        AddLog(buf);

        // Check if metaBase is within module range (this would be suspicious)
        if (found.metaBase >= g_appState.gameAssemblyBase &&
            found.metaBase <
                g_appState.gameAssemblyBase + g_appState.gameAssemblySize) {
          AddLog("[WARNING] MetaBase is INSIDE module range - validate "
                 "carefully.");
          AddLog("[WARNING] True global-metadata should be in heap memory, not "
                 "inside the DLL.");
        }

        // Try reading Magic number at metaBase
        uint32_t magic = 0;
        if (er2::Mem().Read(found.metaBase, &magic, 4)) {
          sprintf_s(buf, "[DMA Test] Magic at MetaBase: 0x%08X", magic);
          AddLog(buf);

          if (magic == 0xFAB11BAF) {
            AddLog("[SUCCESS] Valid Metadata Magic found! (0xFAB11BAF)");
          } else {
            AddLog("[WARNING] Invalid Magic. Expected 0xFAB11BAF.");
            AddLog("[HINT] The address is wrong or memory is corrupted.");
          }
        } else {
          AddLog("[DMA Test] FAILED: Cannot read memory at MetaBase address");
          AddLog("[CRITICAL] If MetaBase is in heap, DMA might not have access "
                 "to heap pages.");
        }

        AddLog("========================================");
      }).detach();
    }
  }

  ImGui::Separator();

  static bool isExporting = false;
  static std::string exportStatus = "";

  if (!isExporting) {
    if (ImGui::Button("Export global-metadata.dat", ImVec2(200, 40))) {
      isExporting = true;
      std::thread([]() {
        AddLog("[INFO] Starting metadata export...");

        std::vector<uint8_t> outBytes;
        bool ok = false;

        if (g_appState.gameAssemblyBase != 0) {
          ok = er2::ExportMetadataByScore(
              er2::Mem(), g_appState.gameAssemblyBase,
              0x200000u, // chunkSize
              8192,      // maxPages
              15.0,      // maxSeconds
              false,     // strictVersion (if requiredVersion=0)
              0,         // requiredVersion
              0x200000u, // headerScanSize
              outBytes);
        } else {
          // Fallback to wrapper if base not detected manually (unlikely if
          // connected)
          auto opt = er2::ExportGameAssemblyMetadataByScore();
          if (opt.has_value()) {
            outBytes = opt.value();
            ok = true;
          }
        }

        if (ok) {
          std::string validateReason;
          if (!ValidateMetadataBytes(outBytes, validateReason)) {
            ok = false;
            exportStatus = "Export rejected: " + validateReason;
            AddLog("[ERROR] Metadata export rejected: " + validateReason);
          }
        }

        if (ok) {
          // 保存到文件
          std::ofstream file("global-metadata.dat", std::ios::binary);
          if (file.is_open()) {
            file.write(reinterpret_cast<const char *>(outBytes.data()),
                       outBytes.size());
            file.close();
            char buf[512];
            sprintf_s(buf, "Export success: global-metadata.dat (%zu bytes)",
                      outBytes.size());
            exportStatus = buf;
            std::string logMsg = std::string("[SUCCESS] ") + buf;
            AddLog(logMsg);
          } else {
            exportStatus = "Failed to save file";
            AddLog("[ERROR] Failed to save file");
          }
        } else {
          exportStatus = "Export failed (Metadata Header not found)";
          AddLog("[ERROR] Metadata export failed. Ensure GameAssembly is "
                 "loaded and accessible.");
        }

        isExporting = false;
      }).detach();
    }

    // Hint.json export button
    if (ImGui::Button("Export with Hint.json", ImVec2(200, 40))) {
      isExporting = true;
      std::thread([]() {
        AddLog("[INFO] Starting metadata + hint.json export...");

        std::vector<uint8_t> outBytes;
        bool ok = false;

        // 1. Export Metadata
        if (g_appState.gameAssemblyBase != 0) {
          ok = er2::ExportMetadataByScore(
              er2::Mem(), g_appState.gameAssemblyBase, 0x200000u, 8192, 15.0,
              false, 0, 0x200000u, outBytes);
        } else {
          auto opt = er2::ExportGameAssemblyMetadataByScore();
          if (opt.has_value()) {
            outBytes = opt.value();
            ok = true;
          }
        }

        if (ok) {
          std::string validateReason;
          if (!ValidateMetadataBytes(outBytes, validateReason)) {
            ok = false;
            exportStatus = "Export rejected: " + validateReason;
            AddLog("[ERROR] Metadata export rejected: " + validateReason);
          }
        }

        if (ok) {
          std::string metadataPath = "global-metadata.dat";
          std::ofstream file(metadataPath, std::ios::binary);
          if (file.is_open()) {
            file.write(reinterpret_cast<const char *>(outBytes.data()),
                       outBytes.size());
            file.close();

            // 2. Export Hint (Need module base)
            bool hintOk = false;

            // [Fix] Use actual module name instead of hardcoded
            // GameAssembly.dll
            std::wstring gaName = L"GameAssembly.dll";
            if (g_appState.useCustomModuleNames &&
                g_appState.customGameAssemblyName[0]) {
              int len = MultiByteToWideChar(
                  CP_ACP, 0, g_appState.customGameAssemblyName, -1, nullptr, 0);
              if (len > 0) {
                gaName.resize(len - 1);
                MultiByteToWideChar(CP_ACP, 0,
                                    g_appState.customGameAssemblyName, -1,
                                    &gaName[0], len);
              }
            }

            if (g_appState.gameAssemblyBase != 0) {
              // Manual call with correct base
              hintOk = er2::ExportMetadataHintJsonTScoreToSidecar(
                  er2::Mem(), metadataPath, g_appState.gameAssemblyBase,
                  er2::g_ctx.pid,
                  L"",             // modulePath
                  gaName.c_str()); // moduleName
            } else {
              // Fallback
              hintOk = er2::ExportGameAssemblyMetadataHintJsonTScoreToSidecar(
                  metadataPath);
            }

            if (hintOk) {
              char buf[512];
              sprintf_s(buf, "Export success: metadata (%zu bytes) + hint.json",
                        outBytes.size());
              exportStatus = buf;
              AddLog("[SUCCESS] " + std::string(buf));
            } else {
              exportStatus = "Metadata OK, but hint.json failed";
              AddLog("[WARN] Metadata exported, but hint.json generation "
                     "failed");
            }
          } else {
            exportStatus = "Failed to save file";
            AddLog("[ERROR] Failed to save file");
          }
        } else {
          exportStatus = "Export failed";
          AddLog("[ERROR] Metadata export failed");
        }
        isExporting = false;
      }).detach();
    }
    ImGui::SameLine();
    ImGui::TextDisabled("(More accurate)");

    // DumpSDK2 - 使用 er2 内置 API 导出 C# SDK
    ImGui::Spacing();
    if (ImGui::Button("Export C# SDK (dump.cs)", ImVec2(200, 40))) {
      isExporting = true;
      std::thread([]() {
        AddLog("[INFO] Starting SDK export (dump.cs + generic.json)...");

        er2::DumpSdk6Paths paths;
        bool ok = false;
        try {
          if (g_appState.gameAssemblyBase != 0) {
            // [Fix] Pass custom module name if available
            std::string gaName = "GameAssembly.dll";
            if (g_appState.useCustomModuleNames &&
                g_appState.customGameAssemblyName[0]) {
              gaName = g_appState.customGameAssemblyName;
            }

            ok = er2::DumpSdk6Dump(er2::Mem(), g_appState.gameAssemblyBase,
                                   g_appState.gameAssemblySize, gaName, paths);
          } else {
            ok = er2::DumpSdk6Dump(paths);
          }
        } catch (const std::exception &e) {
          AddLog(std::string("[CRITICAL] Exception in SDK Export: ") +
                 e.what());
          ok = false;
        } catch (...) {
          AddLog("[CRITICAL] Unknown exception crash in SDK Export");
          ok = false;
        }

        if (ok) {
          char buf[768];
          sprintf_s(buf,
                    "SDK export success!\n  Output: %s\n  dump.cs: %s\n  "
                    "generic.json: %s",
                    paths.outDir.c_str(), paths.dumpCsPath.c_str(),
                    paths.genericJsonPath.c_str());
          exportStatus = buf;
          AddLog("[SUCCESS] " + std::string(buf));
        } else {
          exportStatus =
              "SDK export failed - ensure Unity IL2CPP process is connected";
          AddLog("[ERROR] SDK export failed - check connection and runtime "
                 "type (IL2CPP required)");
        }
        isExporting = false;
      }).detach();
    }
    ImGui::SameLine();
    ImGui::TextDisabled("(dump.cs + generic.json)");
  } else {
    ImGui::BeginDisabled();
    ImGui::Button("Exporting...", ImVec2(200, 40));
    ImGui::EndDisabled();

    // Progress display
    ImGui::Spacing();
    ImGui::ProgressBar(-1.0f * (float)ImGui::GetTime() * 0.4f, ImVec2(-1, 0));
    ImGui::Text("Please wait...");
  }

  if (!exportStatus.empty()) {
    ImGui::Separator();
    ImGui::TextWrapped("%s", exportStatus.c_str());
  }

  ImGui::End();
}

void DrawW2SCalculator() {
  if (!g_appState.showW2SCalculator)
    return;

  ImGui::Begin("World To Screen (W2S)", &g_appState.showW2SCalculator);

  if (!g_appState.isConnected) {
    ImGui::TextDisabled("Please connect to Unity process first");
    ImGui::End();
    return;
  }

  ImGui::Text("Input World Position:");
  ImGui::InputFloat3("World Position", g_appState.w2sWorldPos);

  static float screenWidth = 1920.0f;
  static float screenHeight = 1080.0f;
  ImGui::InputFloat("Screen Width", &screenWidth);
  ImGui::InputFloat("Screen Height", &screenHeight);

  if (ImGui::Button("Calculate", ImVec2(120, 0))) {
    auto cam = er2::FindMainCamera();
    if (cam != 0) {
      auto matrixOpt = er2::GetCameraMatrix(cam);
      if (matrixOpt.has_value()) {
        glm::vec3 worldPos(g_appState.w2sWorldPos[0], g_appState.w2sWorldPos[1],
                           g_appState.w2sWorldPos[2]);

        er2::ScreenRect screenRect;
        screenRect.x = 0.0f;
        screenRect.y = 0.0f;
        screenRect.width = screenWidth;
        screenRect.height = screenHeight;

        auto result =
            er2::WorldToScreenPoint(matrixOpt.value(), screenRect, worldPos);

        if (result.visible) {
          g_appState.w2sScreenX = result.x;
          g_appState.w2sScreenY = result.y;
          g_appState.w2sValid = true;
          AddLog("[INFO] W2S calculation success");
        } else {
          g_appState.w2sValid = false;
          AddLog("[WARN] W2S calculation failed (may be out of view)");
        }
      } else {
        AddLog("[ERROR] Failed to get camera matrix");
      }
    } else {
      AddLog("[ERROR] Main camera not found");
    }
  }

  ImGui::Separator();
  ImGui::Text("UnityResolve W2S Verification");

  if (g_appState.cameras.empty()) {
    ImGui::TextDisabled("No cameras found via MSID. Run Scan Cameras first.");
  } else {
    static int selectedCamIdx = 0;
    if (selectedCamIdx >= g_appState.cameras.size())
      selectedCamIdx = 0;

    const char *previewName =
        g_appState.cameras.empty()
            ? "None"
            : g_appState.cameras[selectedCamIdx].name.c_str();
    if (ImGui::BeginCombo("Select Camera (UR)", previewName)) {
      for (int i = 0; i < g_appState.cameras.size(); ++i) {
        bool isSelected = (selectedCamIdx == i);
        if (ImGui::Selectable(g_appState.cameras[i].name.c_str(), isSelected)) {
          selectedCamIdx = i;
        }
        if (isSelected)
          ImGui::SetItemDefaultFocus();
      }
      ImGui::EndCombo();
    }

    if (ImGui::Button("Calculate (UnityResolve)")) {
      if (!g_appState.cameras.empty()) {
        uintptr_t camAddr = g_appState.cameras[selectedCamIdx].address;
        glm::mat4 matrix;
        if (UnityExternal::Camera_GetMatrix(camAddr, matrix)) {
          UnityExternal::ScreenRect screen = {0, 0, screenWidth, screenHeight};
          glm::vec3 worldPos(g_appState.w2sWorldPos[0],
                             g_appState.w2sWorldPos[1],
                             g_appState.w2sWorldPos[2]);

          auto res =
              UnityExternal::WorldToScreenPoint(matrix, screen, worldPos);

          if (res.visible) {
            AddLog("[INFO] [UR] W2S Success: " + std::to_string(res.x) + ", " +
                   std::to_string(res.y));
            g_appState.w2sScreenX = res.x;
            g_appState.w2sScreenY = res.y;
            g_appState.w2sValid = true;
          } else {
            AddLog("[WARN] [UR] W2S Invisible");
          }
        } else {
          AddLog("[ERROR] [UR] Failed to read matrix");
        }
      }
    }
  }

  ImGui::Separator();

  if (g_appState.w2sValid) {
    ImGui::Text("Screen Position:");
    ImGui::Text("  X: %.2f", g_appState.w2sScreenX);
    ImGui::Text("  Y: %.2f", g_appState.w2sScreenY);
    ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.0f, 1.0f, 0.0f, 1.0f));
    ImGui::Text("Valid Result");
    ImGui::PopStyleColor();
  } else {
    ImGui::TextDisabled("Click 'Calculate' to see result");
  }

  ImGui::End();
}

void DrawLogPanel() {
  if (!g_appState.showLogPanel)
    return;

  ImGui::Begin("Log Output", &g_appState.showLogPanel);

  if (ImGui::Button("Clear")) {
    std::lock_guard<std::mutex> lock(g_appState.logMutex);
    g_appState.logMessages.clear();
  }

  ImGui::SameLine();

  if (ImGui::Button("Copy All")) {
    std::lock_guard<std::mutex> lock(g_appState.logMutex);
    std::string allLogs;
    for (const auto &msg : g_appState.logMessages) {
      allLogs += msg + "\n";
    }
    CopyToClipboard(allLogs);
  }

  ImGui::SameLine();
  {
    std::lock_guard<std::mutex> lock(g_appState.logMutex);
    ImGui::TextDisabled("(%zu messages)", g_appState.logMessages.size());
  }

  ImGui::Separator();

  if (ImGui::BeginChild("LogScroll", ImVec2(0, 0), true)) {
    std::lock_guard<std::mutex> lock(g_appState.logMutex);

    for (const auto &msg : g_appState.logMessages) {
      // 根据类型设置颜色
      if (msg.find("[ERROR]") != std::string::npos) {
        ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 0.3f, 0.3f, 1.0f));
      } else if (msg.find("[WARN]") != std::string::npos) {
        ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 1.0f, 0.0f, 1.0f));
      } else if (msg.find("[SUCCESS]") != std::string::npos) {
        ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.0f, 1.0f, 0.0f, 1.0f));
      } else {
        ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.8f, 0.8f, 0.8f, 1.0f));
      }

      ImGui::TextWrapped("%s", msg.c_str());
      ImGui::PopStyleColor();
    }

    // 自动滚动到底部
    if (ImGui::GetScrollY() >= ImGui::GetScrollMaxY())
      ImGui::SetScrollHereY(1.0f);
  }
  ImGui::EndChild();

  ImGui::End();
}

void DrawMainMenuBar() {
  if (ImGui::BeginMainMenuBar()) {
    if (ImGui::BeginMenu("Windows")) {
      ImGui::MenuItem("Connection Panel", nullptr,
                      &g_appState.showConnectionPanel);
      ImGui::MenuItem("GameObject List", nullptr,
                      &g_appState.showGameObjectList);
      ImGui::MenuItem("Camera Info", nullptr, &g_appState.showCameraInfo);
      ImGui::Separator();
      ImGui::MenuItem("MSID Browser", nullptr, &g_appState.showMsidBrowser);
      ImGui::MenuItem("Bones Browser", nullptr, &g_appState.showBonesPanel);
      ImGui::MenuItem("Metadata Export", nullptr,
                      &g_appState.showMetadataPanel);
      ImGui::MenuItem("DMA Offset Scanner", nullptr,
                      &g_appState.showDmaOffsetScanner);
      ImGui::MenuItem("Naraka BuffManager Probe", nullptr,
                      &g_appState.showNarakaBuffManagerProbe);
      ImGui::MenuItem("Naraka Property Probe", nullptr,
                      &g_appState.showNarakaPropertyProbe);
      ImGui::MenuItem("Naraka Actor Container Probe", nullptr,
                      &g_appState.showNarakaActorContainerProbe);
      ImGui::MenuItem("W2S Calculator", nullptr, &g_appState.showW2SCalculator);
      ImGui::MenuItem("Log Output", nullptr, &g_appState.showLogPanel);
      ImGui::Separator();
      ImGui::MenuItem("ImGui Demo", nullptr, &g_appState.showDemoWindow);
      ImGui::EndMenu();
    }

    if (ImGui::BeginMenu("Help")) {
      if (ImGui::MenuItem("About")) {
        // 可以添加关于对话框
      }
      ImGui::EndMenu();
    }

    // 右侧状态信息
    ImGui::SameLine(ImGui::GetWindowWidth() - 200);
    if (g_appState.isConnected) {
      ImGui::TextColored(ImVec4(0.0f, 1.0f, 0.0f, 1.0f), "o Connected");
    } else {
      ImGui::TextColored(ImVec4(1.0f, 0.0f, 0.0f, 1.0f), "o Disconnected");
    }

    ImGui::EndMainMenuBar();
  }
}

// =============================================================================
// 主程序入口
// =============================================================================
// 兼容 Windows 子系统 (解决 LNK2019)
int main(int, char **);
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance,
                   LPSTR lpCmdLine, int nCmdShow) {
  return main(0, nullptr);
}

int main(int, char **) {
  // 尝试启用调试权限 (需要管理员权限才能成功)
  if (EnableDebugPrivilege()) {
    // 权限提升成功 - 稍后会在日志中显示
  }

  // DPI 感知
  ImGui_ImplWin32_EnableDpiAwareness();
  float main_scale = ImGui_ImplWin32_GetDpiScaleForMonitor(
      ::MonitorFromPoint(POINT{0, 0}, MONITOR_DEFAULTTOPRIMARY));

  // 创建窗口
  WNDCLASSEXW wc = {sizeof(wc),
                    CS_CLASSDC,
                    WndProc,
                    0L,
                    0L,
                    GetModuleHandle(nullptr),
                    nullptr,
                    nullptr,
                    nullptr,
                    nullptr,
                    L"UnityExplorer",
                    nullptr};
  ::RegisterClassExW(&wc);

  HWND hwnd = ::CreateWindowW(
      wc.lpszClassName, L"Unity Explorer - Object Browser (Learning Tool)",
      WS_OVERLAPPEDWINDOW, 100, 100, (int)(1400 * main_scale),
      (int)(900 * main_scale), nullptr, nullptr, wc.hInstance, nullptr);

  // 初始化 Direct3D
  if (!CreateDeviceD3D(hwnd)) {
    CleanupDeviceD3D();
    ::UnregisterClassW(wc.lpszClassName, wc.hInstance);
    return 1;
  }

  // 显示窗口
  ::ShowWindow(hwnd, SW_SHOWDEFAULT);
  ::UpdateWindow(hwnd);

  // 设置 ImGui 上下文
  IMGUI_CHECKVERSION();
  ImGui::CreateContext();
  ImGuiIO &io = ImGui::GetIO();
  (void)io;
  io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;

  // 设置深色主题
  ImGui::StyleColorsDark();

  // 调整样式
  ImGuiStyle &style = ImGui::GetStyle();
  style.ScaleAllSizes(main_scale);
  style.WindowRounding = 5.0f;
  style.FrameRounding = 3.0f;
  style.Colors[ImGuiCol_WindowBg] = ImVec4(0.1f, 0.1f, 0.12f, 0.95f);

  // 设置字体（使用系统中文字体）
  io.Fonts->AddFontFromFileTTF("c:\\Windows\\Fonts\\msyh.ttc",
                               16.0f * main_scale, nullptr,
                               io.Fonts->GetGlyphRangesChineseFull());

  // 初始化后端
  ImGui_ImplWin32_Init(hwnd);
  ImGui_ImplDX11_Init(g_pd3dDevice, g_pd3dDeviceContext);

  // 背景颜色
  ImVec4 clear_color = ImVec4(0.06f, 0.06f, 0.08f, 1.00f);

  // [Log Hook] Connect er2 library logs to UI
  er2::g_logCallback = [](const std::string &msg) {
    std::lock_guard<std::mutex> lock(g_appState.logMutex);
    g_appState.logMessages.push_back(msg);
  };

  AddLog("[INFO] Unity Explorer started");

  // 主循环
  bool done = false;
  while (!done) {
    // 消息处理
    MSG msg;
    while (::PeekMessage(&msg, nullptr, 0U, 0U, PM_REMOVE)) {
      ::TranslateMessage(&msg);
      ::DispatchMessage(&msg);
      if (msg.message == WM_QUIT)
        done = true;
    }
    if (done)
      break;

    // 处理窗口遮挡
    if (g_SwapChainOccluded &&
        g_pSwapChain->Present(0, DXGI_PRESENT_TEST) == DXGI_STATUS_OCCLUDED) {
      ::Sleep(10);
      continue;
    }
    g_SwapChainOccluded = false;

    // 处理窗口大小变化
    if (g_ResizeWidth != 0 && g_ResizeHeight != 0) {
      CleanupRenderTarget();
      g_pSwapChain->ResizeBuffers(0, g_ResizeWidth, g_ResizeHeight,
                                  DXGI_FORMAT_UNKNOWN, 0);
      g_ResizeWidth = g_ResizeHeight = 0;
      CreateRenderTarget();
    }

    // 自动刷新
    if (g_appState.isConnected && g_appState.autoRefresh) {
      float currentTime = static_cast<float>(ImGui::GetTime());
      if (currentTime - g_appState.lastRefreshTime >=
          g_appState.refreshInterval) {
        g_appState.lastRefreshTime = currentTime;
        std::thread([]() {
          RefreshGameObjects();
          RefreshCameras();
        }).detach();
      }
    }
    if (g_appState.isConnected && g_appState.narakaProbeAutoPoll &&
        g_appState.showNarakaPropertyProbe) {
      float currentTime = static_cast<float>(ImGui::GetTime());
      if (currentTime - g_appState.narakaProbeLastPollTime >=
          g_appState.narakaProbePollInterval) {
        g_appState.narakaProbeLastPollTime = currentTime;
        CaptureNarakaProbeSnapshot(false, g_appState.narakaProbeCurrent);
      }
    }
    if (g_appState.isConnected && g_appState.narakaBuffProbeAutoPoll &&
        g_appState.showNarakaBuffManagerProbe) {
      float currentTime = static_cast<float>(ImGui::GetTime());
      if (currentTime - g_appState.narakaBuffProbeLastPollTime >=
          g_appState.narakaBuffProbePollInterval) {
        g_appState.narakaBuffProbeLastPollTime = currentTime;
        CaptureNarakaBuffProbeSnapshot(false,
                                       g_appState.narakaBuffProbeCurrent);
      }
    }
    if (g_appState.isConnected && g_appState.narakaActorContainerAutoPoll &&
        g_appState.showNarakaActorContainerProbe) {
      float currentTime = static_cast<float>(ImGui::GetTime());
      if (currentTime - g_appState.narakaActorContainerLastPollTime >=
          g_appState.narakaActorContainerPollInterval) {
        g_appState.narakaActorContainerLastPollTime = currentTime;
        CaptureNarakaActorContainerSnapshotUi(
            false, g_appState.narakaActorContainerCurrent);
      }
    }

    // 开始 ImGui 帧
    ImGui_ImplDX11_NewFrame();
    ImGui_ImplWin32_NewFrame();
    ImGui::NewFrame();

    // 绘制 UI
    DrawMainMenuBar();
    DrawConnectionPanel();
    DrawGameObjectList();
    DrawGameObjectDetails();
    DrawCameraInfo();

    // 新增面板
    DrawMsidBrowser();
    DrawUnityResolveInspector();
    DrawClassInspector(); // New Class Inspector
    DrawBonesPanel();
    DrawFieldInspector(); // Field Inspector for class field analysis
    DrawMetadataPanel();
    DrawDmaOffsetScanner();
    DrawNarakaBuffManagerProbe();
    DrawNarakaPropertyProbe();
    DrawNarakaActorContainerProbe();
    DrawW2SCalculator();
    DrawLogPanel();
    DrawProcessSelector();

    // Demo 窗口
    if (g_appState.showDemoWindow) {
      ImGui::ShowDemoWindow(&g_appState.showDemoWindow);
    }

    // 渲染
    ImGui::Render();
    const float clear_color_with_alpha[4] = {
        clear_color.x * clear_color.w, clear_color.y * clear_color.w,
        clear_color.z * clear_color.w, clear_color.w};
    g_pd3dDeviceContext->OMSetRenderTargets(1, &g_mainRenderTargetView,
                                            nullptr);
    g_pd3dDeviceContext->ClearRenderTargetView(g_mainRenderTargetView,
                                               clear_color_with_alpha);
    ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());

    // 呈现
    HRESULT hr = g_pSwapChain->Present(1, 0);
    g_SwapChainOccluded = (hr == DXGI_STATUS_OCCLUDED);
  }

  // 清理
  ImGui_ImplDX11_Shutdown();
  ImGui_ImplWin32_Shutdown();
  ImGui::DestroyContext();

  CleanupDeviceD3D();
  ::DestroyWindow(hwnd);
  ::UnregisterClassW(wc.lpszClassName, wc.hInstance);

  return 0;
}

// =============================================================================
// DirectX 11 辅助函数
// =============================================================================
bool CreateDeviceD3D(HWND hWnd) {
  DXGI_SWAP_CHAIN_DESC sd;
  ZeroMemory(&sd, sizeof(sd));
  sd.BufferCount = 2;
  sd.BufferDesc.Width = 0;
  sd.BufferDesc.Height = 0;
  sd.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
  sd.BufferDesc.RefreshRate.Numerator = 60;
  sd.BufferDesc.RefreshRate.Denominator = 1;
  sd.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;
  sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
  sd.OutputWindow = hWnd;
  sd.SampleDesc.Count = 1;
  sd.SampleDesc.Quality = 0;
  sd.Windowed = TRUE;
  sd.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;

  UINT createDeviceFlags = 0;
  D3D_FEATURE_LEVEL featureLevel;
  const D3D_FEATURE_LEVEL featureLevelArray[2] = {D3D_FEATURE_LEVEL_11_0,
                                                  D3D_FEATURE_LEVEL_10_0};

  HRESULT res = D3D11CreateDeviceAndSwapChain(
      nullptr, D3D_DRIVER_TYPE_HARDWARE, nullptr, createDeviceFlags,
      featureLevelArray, 2, D3D11_SDK_VERSION, &sd, &g_pSwapChain,
      &g_pd3dDevice, &featureLevel, &g_pd3dDeviceContext);

  if (res == DXGI_ERROR_UNSUPPORTED) {
    res = D3D11CreateDeviceAndSwapChain(
        nullptr, D3D_DRIVER_TYPE_WARP, nullptr, createDeviceFlags,
        featureLevelArray, 2, D3D11_SDK_VERSION, &sd, &g_pSwapChain,
        &g_pd3dDevice, &featureLevel, &g_pd3dDeviceContext);
  }

  if (res != S_OK)
    return false;

  CreateRenderTarget();
  return true;
}

void CleanupDeviceD3D() {
  CleanupRenderTarget();
  if (g_pSwapChain) {
    g_pSwapChain->Release();
    g_pSwapChain = nullptr;
  }
  if (g_pd3dDeviceContext) {
    g_pd3dDeviceContext->Release();
    g_pd3dDeviceContext = nullptr;
  }
  if (g_pd3dDevice) {
    g_pd3dDevice->Release();
    g_pd3dDevice = nullptr;
  }
}

void CreateRenderTarget() {
  ID3D11Texture2D *pBackBuffer;
  g_pSwapChain->GetBuffer(0, IID_PPV_ARGS(&pBackBuffer));
  g_pd3dDevice->CreateRenderTargetView(pBackBuffer, nullptr,
                                       &g_mainRenderTargetView);
  pBackBuffer->Release();
}

void CleanupRenderTarget() {
  if (g_mainRenderTargetView) {
    g_mainRenderTargetView->Release();
    g_mainRenderTargetView = nullptr;
  }
}

// Forward declare message handler from imgui_impl_win32.cpp
extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd,
                                                             UINT msg,
                                                             WPARAM wParam,
                                                             LPARAM lParam);

LRESULT WINAPI WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) {
  if (ImGui_ImplWin32_WndProcHandler(hWnd, msg, wParam, lParam))
    return true;

  switch (msg) {
  case WM_SIZE:
    if (wParam == SIZE_MINIMIZED)
      return 0;
    g_ResizeWidth = (UINT)LOWORD(lParam);
    g_ResizeHeight = (UINT)HIWORD(lParam);
    return 0;
  case WM_SYSCOMMAND:
    if ((wParam & 0xfff0) == SC_KEYMENU)
      return 0;
    break;
  case WM_DESTROY:
    ::PostQuitMessage(0);
    return 0;
  }
  return ::DefWindowProcW(hWnd, msg, wParam, lParam);
}
