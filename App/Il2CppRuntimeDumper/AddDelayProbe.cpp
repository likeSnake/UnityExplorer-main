// AddDelayProbe.dll
// 进程内只读诊断探针：确认 GameAssembly 变体、AddDelay 指令字节、
// CharactorSync 静态实例以及 G/D/R 时间域样本。
// 不写目标进程内存，不调用游戏方法，不设置断点。

#include <Windows.h>
#include <Psapi.h>
#include <cstdio>
#include <cstdarg>
#include <cstdint>
#include <cstring>
#include <string>
#include <vector>

#pragma comment(lib, "psapi.lib")

namespace {

FILE *g_log = nullptr;
std::string g_out;

void Log(const char *fmt, ...) {
  if (!g_log) return;
  va_list ap;
  va_start(ap, fmt);
  vfprintf(g_log, fmt, ap);
  va_end(ap);
  fputc('\n', g_log);
  fflush(g_log);
}

bool ReadBytes(const void *src, void *dst, size_t n) {
  __try {
    memcpy(dst, src, n);
    return true;
  } __except (EXCEPTION_EXECUTE_HANDLER) {
    memset(dst, 0, n);
    return false;
  }
}

template <class T> bool ReadValue(uintptr_t address, T &out) {
  return ReadBytes(reinterpret_cast<const void *>(address), &out, sizeof(T));
}

bool UserPtr(uintptr_t p) {
  return p >= 0x10000ull && p < 0x0000800000000000ull && (p & 7) == 0;
}

bool ReadCString(uintptr_t p, char *out, size_t cap) {
  // char* 不要求 8 字节对齐；UserPtr 只用于 qword/对象指针。
  if (!out || cap == 0 || p < 0x10000ull || p >= 0x0000800000000000ull) return false;
  out[0] = 0;
  for (size_t i = 0; i + 1 < cap; ++i) {
    char c = 0;
    if (!ReadValue(p + i, c)) return false;
    out[i] = c;
    if (c == 0) return true;
    if (static_cast<unsigned char>(c) < 0x20 && c != '\t') return false;
  }
  out[cap - 1] = 0;
  return false;
}

void Hex(const uint8_t *p, size_t n, char *out, size_t cap) {
  size_t at = 0;
  for (size_t i = 0; i < n && at + 4 < cap; ++i)
    at += static_cast<size_t>(snprintf(out + at, cap - at, "%02X%s", p[i], i + 1 == n ? "" : " "));
  if (cap) out[cap - 1] = 0;
}

struct Module {
  HMODULE handle = nullptr;
  uintptr_t base = 0;
  uint32_t size = 0;
  char name[MAX_PATH] = {};
  uint32_t timestamp = 0;
};

bool ReadPeInfo(HMODULE h, uint32_t &timestamp, uint32_t &sizeImage) {
  uint8_t hdr[0x400] = {};
  if (!ReadBytes(reinterpret_cast<const void *>(h), hdr, sizeof(hdr)) || hdr[0] != 'M' || hdr[1] != 'Z') return false;
  uint32_t lfanew = 0;
  memcpy(&lfanew, hdr + 0x3C, sizeof(lfanew));
  if (lfanew < 0x40 || lfanew > 0x1000) return false;
  uint8_t nt[0x200] = {};
  if (!ReadBytes(reinterpret_cast<const void *>(reinterpret_cast<uintptr_t>(h) + lfanew), nt, sizeof(nt)) ||
      nt[0] != 'P' || nt[1] != 'E' || nt[2] != 0 || nt[3] != 0) return false;
  memcpy(&timestamp, nt + 8, sizeof(timestamp));
  memcpy(&sizeImage, nt + 0x50, sizeof(sizeImage));
  return true;
}

bool FindModule(Module &out) {
  const char *names[] = {"GameAssembly_Super.dll", "GameAssembly_Super_IBT.dll", "GameAssembly.dll"};
  for (const char *name : names) {
    HMODULE h = GetModuleHandleA(name);
    if (!h) continue;
    MODULEINFO mi = {};
    if (!GetModuleInformation(GetCurrentProcess(), h, &mi, sizeof(mi))) continue;
    out.handle = h;
    out.base = reinterpret_cast<uintptr_t>(h);
    out.size = mi.SizeOfImage;
    strncpy_s(out.name, name, _TRUNCATE);
    ReadPeInfo(h, out.timestamp, out.size);
    return true;
  }
  HMODULE mods[2048] = {};
  DWORD needed = 0;
  if (!EnumProcessModulesEx(GetCurrentProcess(), mods, sizeof(mods), &needed, LIST_MODULES_ALL)) return false;
  DWORD count = needed / sizeof(HMODULE);
  if (count > 2048) count = 2048;
  for (DWORD i = 0; i < count; ++i) {
    char path[MAX_PATH] = {};
    if (!GetModuleBaseNameA(GetCurrentProcess(), mods[i], path, MAX_PATH)) continue;
    if (_strnicmp(path, "GameAssembly", 12) != 0) continue;
    MODULEINFO mi = {};
    if (!GetModuleInformation(GetCurrentProcess(), mods[i], &mi, sizeof(mi))) continue;
    out.handle = mods[i];
    out.base = reinterpret_cast<uintptr_t>(mods[i]);
    out.size = mi.SizeOfImage;
    strncpy_s(out.name, path, _TRUNCATE);
    ReadPeInfo(mods[i], out.timestamp, out.size);
    return true;
  }
  return false;
}

bool IsSuper(const Module &m) {
  return strstr(m.name, "_Super") != nullptr || strstr(m.name, "_super") != nullptr;
}

void LogCode(const Module &m, uint32_t rva, const char *label, size_t n) {
  if (static_cast<uint64_t>(rva) + n > m.size) {
    Log("[CODE] %s rva=0x%X 越过镜像边界 size=0x%X", label, rva, m.size);
    return;
  }
  uint8_t bytes[64] = {};
  if (!ReadBytes(reinterpret_cast<const void *>(m.base + rva), bytes, n)) {
    Log("[CODE] %s rva=0x%X 读取失败", label, rva);
    return;
  }
  char text[256] = {};
  Hex(bytes, n, text, sizeof(text));
  Log("[CODE] %s RVA=0x%X VA=0x%llX bytes=%s", label, rva,
      static_cast<unsigned long long>(m.base + rva), text);
}

struct Candidate {
  uint32_t rva;
  const char *label;
};

struct SyncState {
  uintptr_t slot = 0;
  uintptr_t klass = 0;
  uintptr_t statics = 0;
  uintptr_t instance = 0;
  char className[128] = {};
  bool valid = false;
};

SyncState ResolveSync(const Module &m, const Candidate &c) {
  SyncState s;
  s.slot = m.base + c.rva;
  if (!ReadValue(s.slot, s.klass) || !UserPtr(s.klass)) {
    Log("[SYNC] candidate=%s rva=0x%X klass=INVALID", c.label, c.rva);
    return s;
  }
  uintptr_t namePtr = 0;
  ReadValue(s.klass + 0x10, namePtr);
  ReadCString(namePtr, s.className, sizeof(s.className));
  ReadValue(s.klass + 0xB8, s.statics);
  if (UserPtr(s.statics)) ReadValue(s.statics + 0x8, s.instance);
  s.valid = strcmp(s.className, "CharactorSync") == 0;
  Log("[SYNC] candidate=%s rva=0x%X klass=0x%llX name='%s' staticFields=0x%llX instance=0x%llX status=%s",
      c.label, c.rva, static_cast<unsigned long long>(s.klass), s.className,
      static_cast<unsigned long long>(s.statics), static_cast<unsigned long long>(s.instance),
      s.valid ? "CLASS_OK" : "CLASS_MISMATCH");
  if (s.valid && UserPtr(s.instance)) {
    uintptr_t instanceKlass = 0;
    ReadValue(s.instance, instanceKlass);
    Log("[SYNC] instanceKlass=0x%llX same=%s fields D@+0x10 double R@+0x68 float",
        static_cast<unsigned long long>(instanceKlass), instanceKlass == s.klass ? "YES" : "NO");
  }
  return s;
}

bool ResolveGameBase(const Module &m, const Candidate &c, uintptr_t &statics) {
  uintptr_t klass = 0, namePtr = 0;
  if (!ReadValue(m.base + c.rva, klass) || !UserPtr(klass) || !ReadValue(klass + 0x10, namePtr)) return false;
  char name[128] = {};
  ReadCString(namePtr, name, sizeof(name));
  if (strcmp(name, "GameBaseObject") != 0) return false;
  if (!ReadValue(klass + 0xB8, statics) || !UserPtr(statics)) return false;
  Log("[BASE] candidate=%s rva=0x%X klass=0x%llX staticFields=0x%llX name='%s'",
      c.label, c.rva, static_cast<unsigned long long>(klass), static_cast<unsigned long long>(statics), name);
  return true;
}

void Sample(const SyncState &sync, uintptr_t gameStatics, uint32_t seq) {
  if (!sync.valid || !UserPtr(sync.instance)) {
    Log("[SAMPLE] #%u unavailable (CharactorSync instance not initialized)", seq);
    return;
  }
  double d = 0.0;
  float r = 0.0f;
  double g = 0.0;
  if (!ReadValue(sync.instance + 0x10, d) || !ReadValue(sync.instance + 0x68, r)) {
    Log("[SAMPLE] #%u instance=0x%llX D/R read failed", seq, static_cast<unsigned long long>(sync.instance));
    return;
  }
  bool haveG = UserPtr(gameStatics) && ReadValue(gameStatics + 0x28, g);
  const double reconstructed = haveG ? g + d : 0.0;
  Log("[SAMPLE] #%u instance=0x%llX D(localToGlobalTimestampDiff)=%.9f R(shrtt)=%.6f G(globalTime)=%.9f G+D=%.9f gStatus=%s",
      seq, static_cast<unsigned long long>(sync.instance), d, static_cast<double>(r), g,
      reconstructed, haveG ? "OK" : "UNAVAILABLE");
}

void LoadOutput(HMODULE self) {
  char path[MAX_PATH] = {};
  GetModuleFileNameA(self, path, MAX_PATH);
  std::string p(path);
  size_t slash = p.find_last_of("\\/");
  std::string dir = slash == std::string::npos ? "." : p.substr(0, slash);
  std::string cfg = p.substr(0, p.rfind('.')) + ".cfg";
  FILE *f = nullptr;
  if (fopen_s(&f, cfg.c_str(), "rb") == 0 && f) {
    char line[MAX_PATH] = {};
    if (fgets(line, sizeof(line), f)) {
      size_t a = strspn(line, " \t\r\n");
      size_t b = strlen(line);
      while (b > a && (line[b - 1] == '\r' || line[b - 1] == '\n' || line[b - 1] == ' ' || line[b - 1] == '\t')) --b;
      if (b > a) dir.assign(line + a, b - a);
    }
    fclose(f);
  }
  CreateDirectoryA(dir.c_str(), nullptr);
  g_out = dir + "\\adddelay_probe.log";
  fopen_s(&g_log, g_out.c_str(), "wb");
}

DWORD WINAPI Worker(LPVOID arg) {
  LoadOutput(static_cast<HMODULE>(arg));
  Log("=== AddDelayProbe 只读诊断开始 ===");
  Log("pid=%lu tid=%lu", GetCurrentProcessId(), GetCurrentThreadId());
  Module mod;
  if (!FindModule(mod)) {
    Log("[FAIL] 未找到 GameAssembly 模块");
    if (g_log) fclose(g_log);
    FreeLibraryAndExitThread(static_cast<HMODULE>(arg), 0);
    return 0;
  }
  Log("[MODULE] name=%s base=0x%llX size=0x%X timestamp=0x%08X variant=%s",
      mod.name, static_cast<unsigned long long>(mod.base), mod.size, mod.timestamp,
      IsSuper(mod) ? "Super" : "Normal");
  Log("[MODULE] 文档样本 BuildID=25090506 timestamp=0x6A969F1D SizeOfImage=0x1997F000");

  // 普通版文档目标；Super 目标仅作当前 dump 的代码位置参考。
  LogCode(mod, 0x156C250, "Normal AddDelay entry", 32);
  LogCode(mod, 0x156C395, "Normal target +0x145", 16);
  LogCode(mod, 0xD0287B0, "Super AddDelay (2026-09-10 dump)", 32);

  const Candidate syncCandidates[] = {
      {0x0E606490, "normal dump2026.09.03"},
      {0x0E60D3F0, "normal dump_all snapshot"},
      {0x0374C368, "super dump2026.09.10"},
  };
  const Candidate baseCandidates[] = {
      {0x0E644808, "normal dump2026.09.03"},
      {0x0E647780, "normal dump_all snapshot"},
      {0x03755D18, "super dump2026.09.10"},
  };
  SyncState sync;
  for (const Candidate &c : syncCandidates) {
    SyncState found = ResolveSync(mod, c);
    if (found.valid && !sync.valid) sync = found;
  }
  uintptr_t gameStatics = 0;
  for (const Candidate &c : baseCandidates) {
    if (ResolveGameBase(mod, c, gameStatics)) break;
  }
  Log("[LAYOUT] CharactorSync D offset=0x10 (double), shrtt offset=0x68 (float); GameBaseObject _globalTime static offset=0x28 (double)");
  for (uint32_t i = 0; i < 40; ++i) {
    Sample(sync, gameStatics, i);
    Sleep(250);
  }
  Log("=== AddDelayProbe 只读诊断结束；输出=%s ===", g_out.c_str());
  if (g_log) fclose(g_log);
  FreeLibraryAndExitThread(static_cast<HMODULE>(arg), 0);
  return 0;
}

} // namespace

BOOL APIENTRY DllMain(HMODULE hModule, DWORD reason, LPVOID) {
  if (reason == DLL_PROCESS_ATTACH) {
    DisableThreadLibraryCalls(hModule);
    HANDLE h = CreateThread(nullptr, 0, Worker, hModule, 0, nullptr);
    if (h) CloseHandle(h);
  }
  return TRUE;
}
