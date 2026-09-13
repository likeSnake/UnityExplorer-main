// ============================================================================
// HeroDictProbe.cpp -> HeroDictProbe.dll
// 实证探针：注入当前进程，复用 il2cpp_dump_core.hpp 引擎解析
// CBinaryDesignDataReader 静态字段 -> s_Inst -> 6 个 hero reader -> m_Cache 字典，
// 并把每个字典对象的原始 qword 全量 hex dump 出来，锁定 Dictionary._entries /
// ._count 的真实偏移。不写 dump.cs，只写诊断日志。
// 输出：C:\herodict_probe.log
// ============================================================================
#include <Windows.h>
#include <psapi.h>
#include <cstdio>
#include <cstring>
#include <string>
#include <vector>

#include "il2cpp_dump_core.hpp"

#pragma comment(lib, "psapi.lib")

using namespace ildump;

namespace {

char g_logPath[MAX_PATH] = "C:\\herodict_probe.log";
FILE *g_f = nullptr;

void PLog(const char *fmt, ...) {
  if (!g_f) return;
  va_list ap;
  va_start(ap, fmt);
  char tmp[2048];
  vsnprintf(tmp, sizeof(tmp), fmt, ap);
  va_end(ap);
  fputs(tmp, g_f);
  fputc('\n', g_f);
  fflush(g_f);
  OutputDebugStringA(tmp);
  OutputDebugStringA("\n");
}

void LogCB(void *, const char *msg) { PLog("%s", msg); }

// ---- 复用 SuperDumper 的模块发现（简化版）----
bool SafeCopyBytes(void *dst, const void *src, size_t len) {
  __try { memcpy(dst, src, len); return true; }
  __except (EXCEPTION_EXECUTE_HANDLER) { return false; }
}

bool FindGameAssembly(uintptr_t &base, size_t &size, std::string &name) {
  const char *names[] = {"GameAssembly_Super.dll", "GameAssembly_Super_IBT.dll",
                         "GameAssembly.dll"};
  for (const char *n : names) {
    HMODULE m = GetModuleHandleA(n);
    if (!m) continue;
    MODULEINFO mi = {};
    if (GetModuleInformation(GetCurrentProcess(), m, &mi, sizeof(mi))) {
      base = (uintptr_t)m;
      size = mi.SizeOfImage;
      name = n;
      return true;
    }
  }
  HMODULE mods[1024];
  DWORD need = 0;
  if (EnumProcessModules(GetCurrentProcess(), mods, sizeof(mods), &need)) {
    DWORD cnt = need / sizeof(HMODULE);
    if (cnt > 1024) cnt = 1024;
    for (DWORD i = 0; i < cnt; ++i) {
      char path[MAX_PATH] = {};
      if (!GetModuleFileNameExA(GetCurrentProcess(), mods[i], path, MAX_PATH)) continue;
      std::string p(path);
      if (p.find("GameAssembly") == std::string::npos) continue;
      MODULEINFO mi = {};
      if (!GetModuleInformation(GetCurrentProcess(), mods[i], &mi, sizeof(mi))) continue;
      base = (uintptr_t)mods[i];
      size = mi.SizeOfImage;
      name = p;
      return true;
    }
  }
  return false;
}

std::string Hex(uintptr_t v) {
  char b[32];
  snprintf(b, sizeof(b), "0x%llX", (unsigned long long)v);
  return std::string(b);
}

// 读 IL2CPP 托管字符串（UTF-16 length@+0x10, chars@+0x14）
bool ReadMStr(Dumper &d, uintptr_t obj, std::string &out) {
  out.clear();
  if (!obj) { out = "<null>"; return true; }
  int32_t len = d.mem.rdI32(obj + 0x10);
  if (len <= 0 || len > 8192) { out = "<bad-len:" + std::to_string(len) + ">"; return false; }
  std::wstring w((size_t)len, L'\0');
  for (int32_t i = 0; i < len; ++i)
    w[(size_t)i] = (wchar_t)d.mem.rdU16(obj + 0x14 + (uintptr_t)i * 2);
  int n = WideCharToMultiByte(CP_UTF8, 0, w.data(), (int)w.size(), nullptr, 0, nullptr, nullptr);
  if (n <= 0) { out = "<conv-fail>"; return false; }
  out.resize((size_t)n);
  WideCharToMultiByte(CP_UTF8, 0, w.data(), (int)w.size(), out.data(), n, nullptr, nullptr);
  return true;
}

bool IsPtr(uintptr_t p) { return p >= 0x10000 && (p >> 48) == 0; }

// 按 namespace+name 找 klass（复用引擎 images_）
bool FindKlass(Dumper &d, const char *ns, const char *name, uintptr_t &out) {
  out = 0;
  for (const auto img : d.images_) {
    size_t cnt = d.imageCount(img);
    if (!d.env || !d.env->imageGetClass) continue;
    for (size_t i = 0; i < cnt; ++i) {
      uintptr_t k = 0;
      if (!SafeCallImageGetClass(d.env->imageGetClass, img, i, k) || !k) continue;
      char kn[256] = {}, kspace[256] = {};
      if (!d.klassName(k, kn, sizeof(kn))) continue;
      uintptr_t nsp = d.mem.rdPtr(k + d.lay.cNamespace);
      if (nsp) d.mem.rdStr(nsp, kspace, sizeof(kspace), false);
      if (!strcmp(kn, name) && !strcmp(kspace, ns)) { out = k; return true; }
    }
  }
  return false;
}

// 字段偏移按名解析（复用 engine lay）
bool FieldOff(Dumper &d, uintptr_t klass, const char *name, uint32_t &off) {
  if (!klass || !name) return false;
  uintptr_t fields = d.mem.rdPtr(klass + d.lay.cFields);
  if (!IsPtr(fields)) return false;
  uint32_t cnt = d.mem.rdU16(klass + d.lay.cFieldCount);
  for (uint32_t i = 0; i < cnt && i < 8192; ++i) {
    uintptr_t f = fields + (uintptr_t)i * d.lay.fStride;
    uintptr_t np = d.mem.rdPtr(f + d.lay.fName);
    if (!IsPtr(np)) continue;
    char fn[192] = {};
    if (!d.mem.rdStr(np, fn, sizeof(fn), true)) continue;
    if (strcmp(fn, name)) continue;
    off = (uint32_t)d.mem.rdI32(f + d.lay.fOffset);
    return true;
  }
  return false;
}

// 解析静态字段块（复刻 hero_catalog_runtime 的背引用校验，但记录更多诊断）
struct StaticResolve {
  uintptr_t klass = 0;
  uint32_t off = 0;
  uintptr_t fields = 0;
  uintptr_t instance = 0;
  uintptr_t instanceKlass = 0;
  std::string source;
  bool ok = false;
};

bool ResolveStatic(Dumper &d, uintptr_t klass, StaticResolve &r) {
  r = {};
  r.klass = klass;
  std::vector<uint32_t> cands;
  for (const auto &m : d.managerRvaResults_)
    if (m.verified && m.staticFieldsOffset)
      cands.push_back(m.staticFieldsOffset);
  cands.push_back(0xB8);
  cands.push_back(d.lay.cInterfaces + 0x10);
  cands.push_back(0xC0); cands.push_back(0xC8); cands.push_back(0xD0);
  cands.push_back(0xD8); cands.push_back(0xE0);

  uint32_t readableOff = 0; uintptr_t readableFields = 0;
  for (uint32_t c : cands) {
    uintptr_t fields = d.mem.rdPtr(klass + c);
    if (!IsPtr(fields) || !d.mem.probe(fields, 8)) {
      PLog("  static cand=0x%X -> fields=%s (skip)", c, Hex(fields).c_str());
      continue;
    }
    uintptr_t inst = d.mem.rdPtr(fields);
    uintptr_t ik = inst ? d.mem.rdPtr(inst) : 0;
    PLog("  static cand=0x%X fields=%s s_Inst=%s s_Inst->klass=%s back=%s",
         c, Hex(fields).c_str(), Hex(inst).c_str(), Hex(ik).c_str(),
         (ik == klass) ? "MATCH" : "no");
    if (IsPtr(inst) && ik == klass) {
      r.off = c; r.fields = fields; r.instance = inst; r.instanceKlass = ik;
      r.source = "back_reference"; r.ok = true;
      return true;
    }
    if (!readableOff && IsPtr(fields)) { readableOff = c; readableFields = fields; }
  }
  if (readableOff) {
    r.off = readableOff; r.fields = readableFields;
    r.instance = d.mem.rdPtr(readableFields);
    r.instanceKlass = r.instance ? d.mem.rdPtr(r.instance) : 0;
    r.source = "readable_fallback"; r.ok = true;
    return true;
  }
  return false;
}

const uint32_t kReaderOffs[6] = {0x1E38, 0x1E90, 0x1EF8, 0x28D8, 0x3410, 0x990};
const char *kReaderNames[6] = {
    "HeroConfig_HeroBattleConfig", "HeroFashion_HeroFashion",
    "HeroTalent_HeroTalent",       "PlayableMappingConfig_MappingConfig",
    "TalentConfig_TalentSkill",    "AutoAnimatorStates_MaxStateMachineConf"};

// 把一段内存 dump 成 qword 十六进制
void DumpQwords(Dumper &d, uintptr_t p, int n, const char *label) {
  std::string line = std::string("[qwords] ") + label + " @ " + Hex(p) + " :";
  for (int i = 0; i < n; ++i) {
    line += " " + Hex(d.mem.rdPtr(p + (uintptr_t)i * 8));
  }
  PLog("%s", line.c_str());
}

// 解释字典对象：尝试 entries@+0x18 老布局 与 +0x18/+0x28(现代), 并 dump 原始
void InspectDictionary(Dumper &d, uintptr_t dictObj, const char *label) {
  PLog("== dictionary %s = %s", label, Hex(dictObj).c_str());
  if (!IsPtr(dictObj)) { PLog("  (null/unreadable)"); return; }
  uintptr_t klass = d.mem.rdPtr(dictObj);
  char kn[256] = {};
  d.klassName(klass, kn, sizeof(kn));
  PLog("  klass=%s name='%s'", Hex(klass).c_str(), kn);
  DumpQwords(d, dictObj, 16, label);
  // 现代 .NET Dictionary 布局（_fastModMultiplier@+0x20, _count@+0x28）
  uintptr_t buckets = d.mem.rdPtr(dictObj + 0x10);
  uintptr_t entries = d.mem.rdPtr(dictObj + 0x18);
  uint64_t fastMod = d.mem.rdU64(dictObj + 0x20);
  int32_t cnt28 = d.mem.rdI32(dictObj + 0x28);
  int32_t cnt20 = d.mem.rdI32(dictObj + 0x20);
  PLog("  buckets=%s entries=%s fastMod=0x%llX count@0x28=%d count@0x20=%d",
       Hex(buckets).c_str(), Hex(entries).c_str(), (unsigned long long)fastMod,
       cnt28, cnt20);
  // entries 数组头部（Entry[] 的 length@+0x18 + 首个 Entry 几个字段）
  if (IsPtr(entries)) {
    DumpQwords(d, entries, 8, (std::string(label) + ".entries[]").c_str());
    int32_t arrLen = d.mem.rdI32(entries + 0x18);
    PLog("  entries.arrLen=%d", arrLen);
    if (arrLen > 0 && arrLen < 200000) {
      // Entry 步长：现代 .NET Entry 0x20 字节 (hash,next,key,value)
      // 老 .NET Entry 0x18 (hash,next,key,value 无 next?) —— 这里 dump 首条目 6 qwords
      DumpQwords(d, entries + 0x20, 6, (std::string(label) + ".entry[0]").c_str());
    }
  }
}

// 运行主逻辑
int RunProbe(Env &env) {
  Options opt;
  opt.outPath = "C:\\herodict_probe_dump.cs";  // 不实际写，run() 会尝试；我们不调用 run()
  LogCtx logc;
  logc.fn = LogCB;

  Dumper d;
  d.env = &env;
  d.opt = &opt;
  d.log = &logc;

  // 手动走 engine 的发现流程（不调用 run()，避免写大文件）
  // 复用 run() 里的前段：discoverImages -> prepareImageLayout -> buildIndexMap
  PLog("=== [1] 镜像发现 ===");
  std::vector<uintptr_t> images;
  if (env.getCorlib) {
    if (!d.discoverImages(images)) {
      PLog("[FAIL] discoverImages");
      return 1;
    }
  } else {
    d.superMode_ = true;
    if (!d.collectKlassesByScan()) { PLog("[FAIL] collectKlassesByScan"); return 1; }
    d.calibrate();
    d.buildIndexFromTokens();
    d.deriveImagesFromKlasses(images);
    if (images.empty()) { PLog("[FAIL] no images"); return 1; }
  }
  d.prepareImageLayout(images);
  if (env.getCorlib && !d.buildIndexMapFromClassTypeData(images) &&
      !d.buildIndexMapFromClassTypeHandles(images))
    d.buildIndexMapFromImages(images);
  d.images_ = images;
  if (!d.superMode_) d.calibrate();
  PLog("=== [1] images=%u ===", (unsigned)images.size());

  // 找到 CBinaryDesignDataReader
  PLog("=== [2] 找 CBinaryDesignDataReader ===");
  uintptr_t readerClass = 0;
  if (!FindKlass(d, "BinaryDesignData", "CBinaryDesignDataReader", readerClass)) {
    PLog("[FAIL] CBinaryDesignDataReader 未找到");
    return 1;
  }
  PLog("  readerClass=%s", Hex(readerClass).c_str());

  // 解析静态字段
  PLog("=== [3] 解析静态字段 ===");
  StaticResolve sr;
  if (!ResolveStatic(d, readerClass, sr)) {
    PLog("[FAIL] 静态字段未解析");
    return 1;
  }
  PLog("  静态字段 off=0x%X fields=%s s_Inst=%s source=%s",
       sr.off, Hex(sr.fields).c_str(), Hex(sr.instance).c_str(), sr.source.c_str());

  // 遍历 6 个 reader
  PLog("=== [4] 遍历 6 个 hero reader ===");
  uintptr_t singleton = sr.instance;
  if (!IsPtr(singleton)) {
    PLog("[FAIL] s_Inst 为空");
    return 1;
  }
  for (int i = 0; i < 6; ++i) {
    uint32_t fo = kReaderOffs[i];
    uintptr_t reader = d.mem.rdPtr(singleton + fo);
    PLog("--- reader[%d] %s fieldOff=0x%X reader=%s ---",
         i, kReaderNames[i], fo, Hex(reader).c_str());
    if (!IsPtr(reader)) { PLog("  (null)"); continue; }
    char rk[256] = {};
    d.klassName(d.mem.rdPtr(reader), rk, sizeof(rk));
    PLog("  reader->klass name='%s'", rk);
    DumpQwords(d, reader, 8, kReaderNames[i]);
    // 解析 reader 的三个字段偏移
    uintptr_t rklass = d.mem.rdPtr(reader);
    uint32_t kOff = 0x10, cOff = 0x18, bOff = 0x20;
    uint32_t tmp;
    if (FieldOff(d, rklass, "m_Key2Indexer", tmp)) kOff = tmp;
    if (FieldOff(d, rklass, "m_Cache", tmp)) cOff = tmp;
    if (FieldOff(d, rklass, "m_Buffer", tmp)) bOff = tmp;
    PLog("  resolved keyOff=0x%X cacheOff=0x%X bufferOff=0x%X", kOff, cOff, bOff);
    uintptr_t keyDict = d.mem.rdPtr(reader + kOff);
    uintptr_t cacheDict = d.mem.rdPtr(reader + cOff);
    InspectDictionary(d, keyDict, (std::string(kReaderNames[i]) + ".m_Key2Indexer").c_str());
    InspectDictionary(d, cacheDict, (std::string(kReaderNames[i]) + ".m_Cache").c_str());
  }

  PLog("=== probe done ===");
  return 0;
}

__declspec(noinline) int RunProbeSeh(Env *env, unsigned long *code) {
  __try { return RunProbe(*env); }
  __except (EXCEPTION_EXECUTE_HANDLER) { if (code) *code = GetExceptionCode(); return 2; }
}

DWORD WINAPI Worker(LPVOID hmod) {
  g_f = fopen(g_logPath, "w");
  PLog("=== HeroDictProbe start pid=%lu ===", GetCurrentProcessId());

  uintptr_t base = 0; size_t size = 0; std::string name;
  if (!FindGameAssembly(base, size, name)) {
    PLog("[FAIL] GameAssembly 模块未找到");
    if (g_f) fclose(g_f);
    FreeLibraryAndExitThread((HMODULE)hmod, 0);
    return 0;
  }
  PLog("[0] module=%s base=%s size=0x%llX", name.c_str(), Hex(base).c_str(),
       (unsigned long long)size);

  Env env;
  env.moduleBase = base;
  env.moduleSize = size;
  env.moduleName = name;
  env.superVariant = (name.find("Super") != std::string::npos) || (name.find("_super") != std::string::npos);
  env.executableBase = (uintptr_t)GetModuleHandleW(nullptr);

  HMODULE hmod2 = (HMODULE)base;
  env.getCorlib = (void *(*)())GetProcAddress(hmod2, "il2cpp_get_corlib");
  env.imageGetClass = (void *(*)(void *, size_t))GetProcAddress(hmod2, "il2cpp_image_get_class");
  env.imageGetClassCount = (size_t(*)(void *))GetProcAddress(hmod2, "il2cpp_image_get_class_count");
  env.imageGetName = (const char *(*)(void *))GetProcAddress(hmod2, "il2cpp_image_get_name");
  env.imageGetAssembly = (void *(*)(void *))GetProcAddress(hmod2, "il2cpp_image_get_assembly");
  const bool bootstrap = env.getCorlib && env.imageGetClass && env.imageGetClassCount && env.imageGetName;
  PLog("[0] bootstrap=%s", bootstrap ? "available" : "absent");
  if (bootstrap && env.superVariant) {
    env.scanAllMemoryForImages = true;
    env.expectMainImage = true;
  }

  unsigned long code = 0;
  int rc = RunProbeSeh(&env, &code);
  if (rc == 2) PLog("[CRASH] 0x%08lX", code);
  PLog("[done] rc=%d", rc);
  if (g_f) fclose(g_f);
  FreeLibraryAndExitThread((HMODULE)hmod, 0);
  return 0;
}

}  // namespace

BOOL APIENTRY DllMain(HMODULE hModule, DWORD reason, LPVOID) {
  if (reason == DLL_PROCESS_ATTACH) {
    DisableThreadLibraryCalls(hModule);
    HANDLE h = CreateThread(nullptr, 0, Worker, (LPVOID)hModule, 0, nullptr);
    if (h) CloseHandle(h);
  }
  return TRUE;
}