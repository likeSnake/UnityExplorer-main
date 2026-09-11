// ============================================================================
// GameAssemblyStub.cpp  ->  GameAssembly.dll
// 合成「GameAssembly.dll」：在自身进程内构造一套 Unity 2019.4 布局的
// Il2CppImage / Il2CppClass / FieldInfo / MethodInfo / PropertyInfo / Il2CppType
// 以及假的（含魔数的）metadata 镜像，并导出 dumper 需要的 5 个 il2cpp_* 函数。
//
// 用途：在没有真实游戏的情况下，端到端验证 MemoryStructDumper.dll 的
//       注入通道 + 镜像发现（结构扫描）+ 布局自校准 + 全量 dump 输出。
// ============================================================================
#include <Windows.h>
#include <cstdint>
#include <cstring>
#include <cstdio>
#include <string>
#include <vector>

namespace {

uintptr_t g_base = 0;
size_t g_used = 0;
size_t g_cap = 0;
uintptr_t g_moduleBase = 0;

uintptr_t Alloc(size_t bytes, size_t align = 16) {
  g_used = (g_used + align - 1) & ~(align - 1);
  uintptr_t p = g_base + g_used;
  g_used += bytes;
  if (g_used > g_cap) return 0;
  memset((void *)p, 0, bytes);
  return p;
}
uintptr_t CStr(const char *s) {
  size_t n = strlen(s) + 1;
  uintptr_t p = Alloc(n, 1);
  if (p) memcpy((void *)p, s, n);
  return p;
}

// ---- 结构偏移（Unity 2019.4 / metadata v24.5）----
enum {
  C_IMAGE = 0x00, C_NAME = 0x10, C_NS = 0x18, C_PARENT = 0x58, C_TDH = 0x68,
  C_FIELDS = 0x80, C_PROPS = 0x90, C_METHODS = 0x98, C_IFACES = 0xA8,
  C_INSTANCE_SIZE = 0xF0, C_FLAGS = 0x108, C_TOKEN = 0x10C,
  C_MCOUNT = 0x110, C_PCOUNT = 0x112, C_FCOUNT = 0x114, C_ICOUNT = 0x11C,
  M_PTR = 0x00, M_NAME = 0x10, M_KLASS = 0x18, M_RET = 0x20, M_PARAMS = 0x28,
  M_TOKEN = 0x38, M_FLAGS = 0x3C, M_SLOT = 0x40, M_PC = 0x42,
  F_NAME = 0x00, F_TYPE = 0x08, F_PARENT = 0x10, F_OFF = 0x18,
  P_PARENT = 0x00, P_NAME = 0x08, P_GET = 0x10, P_SET = 0x18,
};

uintptr_t g_corlib = 0;
uintptr_t g_acsharp = 0;
std::vector<uintptr_t> g_corlibClasses, g_acClasses;

// metadata（假镜像，含魔数）
uintptr_t g_meta = 0;
uint32_t g_metaStringOff = 0x1000, g_metaMethodsOff = 0x2000, g_metaParamsOff = 0x3000,
         g_metaTypeDefsOff = 0x4000;
std::string g_strTab;
std::vector<std::string> g_strs;
uint32_t MetaStr(const char *s) {
  uint32_t off = (uint32_t)g_strTab.size();
  g_strs.push_back(s);
  g_strTab += s;
  g_strTab.push_back('\0');
  return off;
}
uintptr_t MethodDef(uint32_t i) { return g_meta + g_metaMethodsOff + i * 0x34; }
uintptr_t ParamDef(uint32_t i) { return g_meta + g_metaParamsOff + i * 0x0C; }
uintptr_t TypeDef(uint32_t i) { return g_meta + g_metaTypeDefsOff + i * 0x5C; }

uintptr_t MkType(uint8_t te, uintptr_t data, uint32_t attrs = 0, bool byref = false) {
  uintptr_t t = Alloc(0x10);
  *(uint64_t *)(t + 0x00) = data;
  *(uint32_t *)(t + 0x08) = (attrs & 0xFFFF) | ((uint32_t)te << 16) | (byref ? (1u << 30) : 0u);
  return t;
}

struct CB {
  uintptr_t k = 0;
  uintptr_t fields = 0, props = 0, methods = 0, ifaces = 0;
  uint32_t nf = 0, np = 0, nm = 0, ni = 0;
  uint32_t typeDefIdx = 0xFFFFFFFF;
  uint32_t mStart = 0, mCount = 0, pStart = 0;

  uintptr_t Init(const char *name, const char *ns, uintptr_t image, uint32_t flags,
                 uintptr_t parent, uint32_t instSize = 0x10) {
    k = Alloc(0x140);
    *(uintptr_t *)(k + C_IMAGE) = image;
    *(uintptr_t *)(k + C_NAME) = CStr(name);
    *(uintptr_t *)(k + C_NS) = CStr(ns ? ns : "");
    *(uintptr_t *)(k + C_PARENT) = parent;
    *(uint32_t *)(k + C_FLAGS) = flags;
    *(uint32_t *)(k + C_INSTANCE_SIZE) = instSize;
    *(uint32_t *)(k + C_TOKEN) = 0x02000001 + (uint32_t)(g_used / 0x140);
    // byval_arg 自引用（Super 无导出扫描的判定依据）
    *(uintptr_t *)(k + 0x20) = k;
    *(uint32_t *)(k + 0x28) = ((uint32_t)0x12) << 16;  // TYPE_CLASS
    return k;
  }
  void Field(const char *name, uintptr_t type, int32_t off, uint32_t fattrs) {
    uintptr_t f = Alloc(0x20);
    *(uintptr_t *)(f + F_NAME) = CStr(name);
    *(uintptr_t *)(f + F_TYPE) = type;
    *(uintptr_t *)(f + F_PARENT) = k;
    *(int32_t *)(f + F_OFF) = off;
    // 该字段专用 type 的 attrs 承载 FieldAttributes
    *(uint32_t *)(type + 0x08) = (*(uint32_t *)(type + 0x08) & 0xFFFF0000u) | (fattrs & 0xFFFF);
    if (!fields) fields = Alloc(0x20 * 32, 16);
    memcpy((void *)(fields + nf * 0x20), (void *)f, 0x20);
    ++nf;
  }
  uintptr_t Method(const char *name, uintptr_t ret, std::vector<uintptr_t> params,
                   uint16_t flags, uint16_t slot, uint32_t token) {
    uintptr_t mi = Alloc(0x48);
    static uint32_t s_rva = 0x2000;
    s_rva += 0x20;
    *(uintptr_t *)(mi + M_PTR) = g_moduleBase + s_rva;
    *(uintptr_t *)(mi + M_NAME) = CStr(name);
    *(uintptr_t *)(mi + M_KLASS) = k;
    *(uintptr_t *)(mi + M_RET) = ret;
    if (!params.empty()) {
      uintptr_t pa = Alloc(params.size() * 8, 8);
      for (size_t i = 0; i < params.size(); ++i) *(uintptr_t *)(pa + i * 8) = params[i];
      *(uintptr_t *)(mi + M_PARAMS) = pa;
    }
    *(uint32_t *)(mi + M_TOKEN) = token;
    *(uint16_t *)(mi + M_FLAGS) = flags;
    *(uint16_t *)(mi + M_SLOT) = slot;
    *(uint8_t *)(mi + M_PC) = (uint8_t)params.size();
    if (!methods) methods = Alloc(8 * 64, 8);
    *(uintptr_t *)(methods + nm * 8) = mi;
    ++nm;
    return mi;
  }
  void Prop(const char *name, uintptr_t get, uintptr_t set) {
    uintptr_t p = Alloc(0x28);
    *(uintptr_t *)(p + P_PARENT) = k;
    *(uintptr_t *)(p + P_NAME) = CStr(name);
    *(uintptr_t *)(p + P_GET) = get;
    *(uintptr_t *)(p + P_SET) = set;
    if (!props) props = Alloc(0x28 * 16, 16);
    memcpy((void *)(props + np * 0x28), (void *)p, 0x28);
    ++np;
  }
  void Iface(uintptr_t ik) {
    if (!ifaces) ifaces = Alloc(8 * 16, 8);
    *(uintptr_t *)(ifaces + ni * 8) = ik;
    ++ni;
  }
  void Commit() {
    if (nf) {
      *(uintptr_t *)(k + C_FIELDS) = fields;
      *(uint16_t *)(k + C_FCOUNT) = (uint16_t)nf;
    }
    if (np) {
      *(uintptr_t *)(k + C_PROPS) = props;
      *(uint16_t *)(k + C_PCOUNT) = (uint16_t)np;
    }
    if (nm) {
      *(uintptr_t *)(k + C_METHODS) = methods;
      *(uint16_t *)(k + C_MCOUNT) = (uint16_t)nm;
    }
    if (ni) {
      *(uintptr_t *)(k + C_IFACES) = ifaces;
      *(uint16_t *)(k + C_ICOUNT) = (uint16_t)ni;
    }
  }
};

uintptr_t MkImage(const char *name, uint32_t typeStart, uint32_t typeCount,
                  std::vector<uintptr_t> *out) {
  uintptr_t img = Alloc(0x48);
  uintptr_t asm_ = Alloc(0x60);
  *(uintptr_t *)(img + 0x00) = CStr(name);
  *(uintptr_t *)(img + 0x08) = CStr(name);
  *(uintptr_t *)(img + 0x10) = asm_;
  *(uint32_t *)(img + 0x18) = typeStart;
  *(uint32_t *)(img + 0x1C) = typeCount;
  *(uintptr_t *)(asm_ + 0x00) = img;
  if (out) out->push_back(img);
  return img;
}

void Build() {
  g_cap = 32 << 20;
  g_base = (uintptr_t)VirtualAlloc(nullptr, g_cap, MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);
  g_moduleBase = (uintptr_t)GetModuleHandleA("GameAssembly_Super.dll");
  if (!g_moduleBase) g_moduleBase = (uintptr_t)GetModuleHandleA("GameAssembly.dll");
  if (!g_moduleBase) g_moduleBase = g_base;

  std::vector<uintptr_t> images;
  g_corlib = MkImage("mscorlib.dll", 0, 0, &images);
  g_acsharp = MkImage("Assembly-CSharp.dll", 0x100, 0, &images);

  // 假 metadata 镜像
  g_meta = Alloc(0x8000, 0x1000);
  *(uint32_t *)(g_meta + 0x00) = 0xFAB11BAF;
  *(int32_t *)(g_meta + 0x04) = 24;
  *(int32_t *)(g_meta + 0x18) = (int32_t)g_metaStringOff;
  *(int32_t *)(g_meta + 0x1C) = 0x1000;
  *(int32_t *)(g_meta + 0x30) = (int32_t)g_metaMethodsOff;
  *(int32_t *)(g_meta + 0x34) = 512;
  *(int32_t *)(g_meta + 0x58) = (int32_t)g_metaParamsOff;
  *(int32_t *)(g_meta + 0x5C) = 512;
  *(int32_t *)(g_meta + 0xA0) = (int32_t)g_metaTypeDefsOff;
  *(int32_t *)(g_meta + 0xA4) = 512;

  // ---- 基础类型 ----
  CB cObject;
  cObject.Init("Object", "System", g_corlib, 0x00000001, 0);
  cObject.Commit();
  CB cString;
  cString.Init("String", "System", g_corlib, 0x00000101, cObject.k, 0x20);
  cString.Commit();
  CB cValueType;
  cValueType.Init("ValueType", "System", g_corlib, 0x00000001, cObject.k);
  cValueType.Commit();
  CB cEnum;
  cEnum.Init("Enum", "System", g_corlib, 0x00000001, cValueType.k);
  cEnum.Commit();
  CB cException;
  cException.Init("Exception", "System", g_corlib, 0x00000001, cObject.k, 0x40);
  cException.Commit();
  CB cIList;
  cIList.Init("IList", "System.Collections", g_corlib, 0x00000021, cObject.k);
  cIList.Commit();

  uintptr_t tVoid = MkType(0x01, 0);
  uintptr_t tBool = MkType(0x02, 0);
  uintptr_t tI4 = MkType(0x08, 0);
  uintptr_t tI8 = MkType(0x0A, 0);
  uintptr_t tR4 = MkType(0x0C, 0);
  uintptr_t tStr = MkType(0x0E, 0);
  uintptr_t tObj = MkType(0x1C, 0);
  uintptr_t tU1 = MkType(0x05, 0);
  uintptr_t tSzU1 = MkType(0x1D, MkType(0x05, 0));

  // mscorlib 类（8 个）
  {
    CB c;
    c.Init("DateTime", "System", g_corlib, 0x00000101, cValueType.k, 0x18);
    c.Field("ticks", MkType(0x0A, 0, 0x0001), 0x10, 0x0001);
    c.Commit();
    g_corlibClasses.push_back(c.k);
  }
  {
    CB c;
    c.Init("List`1", "System.Collections.Generic", g_corlib, 0x00000101, cObject.k, 0x30);
    c.Field("_items", tObj, 0x10, 0x0001);
    c.Field("_size", MkType(0x08, 0, 0x0001), 0x18, 0x0001);
    c.Field("_version", MkType(0x08, 0, 0x0001), 0x1C, 0x0001);
    uintptr_t g = c.Method("get_Count", tI4, {}, 0x0006, 0xFFFF, 0x06001000);
    c.Prop("Count", g, 0);
    c.Method("Add", tVoid, {tObj}, 0x0006, 0xFFFF, 0x06001001);
    c.Method(".ctor", tVoid, {}, 0x0006, 0xFFFF, 0x06001002);
    c.Commit();
    g_corlibClasses.push_back(c.k);
  }
  {
    CB c;
    c.Init("MyFlags", "System", g_corlib, 0x00000101, cEnum.k);
    uintptr_t tSelf = MkType(0x11, c.k, 0x00000101);
    c.Field("value__", MkType(0x08, 0, 0x0006), 0x10, 0x0006);
    c.Field("None", tSelf, -1, 0x0056);
    c.Field("All", tSelf, -1, 0x0056);
    c.Commit();
    g_corlibClasses.push_back(c.k);
  }
  g_corlibClasses.insert(g_corlibClasses.begin(),
                         {cObject.k, cString.k, cValueType.k, cEnum.k, cException.k, cIList.k});

  // ---- Assembly-CSharp：30 个"游戏类" ----
  for (int i = 0; i < 30; ++i) {
    char nm[64], ns[64];
    snprintf(nm, sizeof(nm), i % 3 == 0 ? "PlayerCtrl%d" : (i % 3 == 1 ? "BattleActor%d" : "UIPanel%d"), i);
    snprintf(ns, sizeof(ns), i % 2 ? "GameLogic.Battle" : "L22.Rendering");
    CB c;
    c.Init(nm, ns, g_acsharp, i % 4 == 0 ? 0x00000101 : 0x00000001, cObject.k, 0x80 + 8 * i);
    c.Field("m_instance", MkType(0x12, c.k, 0x0006), 0x10, 0x000A);           // private static
    c.Field("m_transform", tObj, 0x18, 0x0001);                                // private
    c.Field("m_health", MkType(0x0C, 0, 0x0006), 0x20, 0x0006);                // public float
    c.Field("m_name", tStr, 0x28, 0x0006);
    c.Field("s_all", tSzU1, 0x30, 0x0016);                                     // public static
    c.Field("MAX_LEVEL", MkType(0x08, 0, 0x0046), -1, 0x0046);                 // public const
    uintptr_t gHp = c.Method("get_Health", tR4, {}, 0x0006, 0xFFFF, 0x06002000 + i * 16);
    uintptr_t sHp = c.Method("set_Health", tVoid, {MkType(0x0C, 0, 0x0000)}, 0x0006, 0xFFFF,
                             0x06002001 + i * 16);
    c.Prop("Health", gHp, sHp);
    c.Method("Awake", tVoid, {}, 0x0006, 0xFFFF, 0x06002002 + i * 16);
    c.Method("Update", tVoid, {}, 0x0044, (uint16_t)(2 + i % 5), 0x06002003 + i * 16);
    c.Method("ApplyDamage", tBool,
             {MkType(0x0C, 0, 0x0000), MkType(0x0A, MkType(0x0A, 0, 0x0000), 0x0002, true)},
             0x0006, 0xFFFF, 0x06002004 + i * 16);
    c.Method("TryGetTarget", tBool,
             {MkType(0x1C, 0, 0x0002, true), MkType(0x0C, 0, 0x0000, true)}, 0x0006, 0xFFFF,
             0x06002005 + i * 16);
    if (i % 2) c.Iface(cIList.k);
    c.Commit();
    g_acClasses.push_back(c.k);
  }

  *(uint32_t *)(g_corlib + 0x1C) = (uint32_t)g_corlibClasses.size();
  *(uint32_t *)(g_acsharp + 0x1C) = (uint32_t)g_acClasses.size();
  // token = 0x02000000 | (全局 TypeDefinitionIndex + 1)
  for (size_t i = 0; i < g_corlibClasses.size(); ++i)
    *(uint32_t *)(g_corlibClasses[i] + C_TOKEN) = 0x02000000u | (uint32_t)(i + 1);
  for (size_t i = 0; i < g_acClasses.size(); ++i)
    *(uint32_t *)(g_acClasses[i] + C_TOKEN) = 0x02000000u | (uint32_t)(0x100 + i + 1);
  uintptr_t ca1 = Alloc(g_corlibClasses.size() * 8, 8);
  for (size_t i = 0; i < g_corlibClasses.size(); ++i) *(uintptr_t *)(ca1 + i * 8) = g_corlibClasses[i];
  uintptr_t ca2 = Alloc(g_acClasses.size() * 8, 8);
  for (size_t i = 0; i < g_acClasses.size(); ++i) *(uintptr_t *)(ca2 + i * 8) = g_acClasses[i];

  // 参数名 metadata（给前 6 个 Assembly-CSharp 类写 typeDef/methodDef/paramDef）
  uint32_t sP[6];
  sP[0] = MetaStr("damage");
  sP[1] = MetaStr("sourceId");
  sP[2] = MetaStr("target");
  sP[3] = MetaStr("outPos");
  sP[4] = MetaStr("value");
  sP[5] = MetaStr("deltaTime");
  // 用类名写 typeDef（dumper 用 klass->typeMetadataHandle 反查并校验类名）
  for (int i = 0; i < 6 && i < (int)g_acClasses.size(); ++i) {
    uintptr_t k = g_acClasses[i];
    uint32_t sName = MetaStr((const char *)*(uintptr_t *)(k + C_NAME));
    uint32_t sNs = MetaStr((const char *)*(uintptr_t *)(k + C_NS));
    uintptr_t td = TypeDef(i);
    *(int32_t *)(td + 0x00) = (int32_t)sName;
    *(int32_t *)(td + 0x04) = (int32_t)sNs;
    *(int32_t *)(td + 0x28) = i * 8;   // methodStart
    *(uint16_t *)(td + 0x44) = 7;      // method_count
    *(uintptr_t *)(k + C_TDH) = td;
    // 7 个方法定义（token 与 MethodInfo 对应）
    uint32_t toks[7] = {0x06002000u, 0x06002001u, 0x06002002u, 0x06002003u, 0x06002004u,
                        0x06002005u, 0x06002006u};
    for (int j = 0; j < 7; ++j) {
      uintptr_t md = MethodDef(i * 8 + j);
      *(uint32_t *)(md + 0x28) = toks[j] + (uint32_t)i * 16;
      *(int32_t *)(md + 0x0C) = (j == 4) ? 0 : (j == 5 ? 2 : 6);
      *(uint16_t *)(md + 0x32) = (uint16_t)(j == 4 ? 2 : (j == 5 ? 2 : 0));
    }
  }
  for (int i = 0; i < 6; ++i) {
    uintptr_t pd = ParamDef(i);
    *(int32_t *)(pd + 0x00) = (int32_t)sP[i];
  }
  memcpy((void *)(g_meta + g_metaStringOff), g_strTab.data(), g_strTab.size());

  char msg[256];
  snprintf(msg, sizeof(msg),
           "[stub] GameAssembly.dll built: images=%zu corlibClasses=%zu acClasses=%zu heap=0x%llx\n",
           images.size(), g_corlibClasses.size(), g_acClasses.size(),
           (unsigned long long)g_base);
  OutputDebugStringA(msg);
}

volatile LONG g_init = 0;
void EnsureInit() {
  if (InterlockedCompareExchange(&g_init, 1, 0) == 0) Build();
  else
    while (g_init != 2) Sleep(1);
  InterlockedExchange(&g_init, 2);
}

}  // namespace

#ifndef SUPER_STUB
extern "C" {

__declspec(dllexport) void *il2cpp_get_corlib(void) {
  EnsureInit();
  return (void *)g_corlib;
}

__declspec(dllexport) void *il2cpp_image_get_class(void *image, size_t index) {
  EnsureInit();
  if (image == (void *)g_corlib)
    return index < g_corlibClasses.size() ? (void *)g_corlibClasses[index] : nullptr;
  if (image == (void *)g_acsharp)
    return index < g_acClasses.size() ? (void *)g_acClasses[index] : nullptr;
  return nullptr;
}

__declspec(dllexport) size_t il2cpp_image_get_class_count(void *image) {
  EnsureInit();
  if (image == (void *)g_corlib) return g_corlibClasses.size();
  if (image == (void *)g_acsharp) return g_acClasses.size();
  return 0;
}

__declspec(dllexport) const char *il2cpp_image_get_name(void *image) {
  EnsureInit();
  if (image == (void *)g_corlib) return "mscorlib.dll";
  if (image == (void *)g_acsharp) return "Assembly-CSharp.dll";
  return "";
}

__declspec(dllexport) void *il2cpp_image_get_assembly(void *image) {
  EnsureInit();
  if (!image) return nullptr;
  return (void *)*(uintptr_t *)((uintptr_t)image + 0x10);
}

}  // extern "C"
#endif  // !SUPER_STUB

#ifdef SUPER_STUB
namespace {
DWORD WINAPI StubInitThread(LPVOID) {
  EnsureInit();
  return 0;
}
}  // namespace
#endif

BOOL APIENTRY DllMain(HMODULE, DWORD reason, LPVOID) {
#ifdef SUPER_STUB
  // 无导出变体：没有任何 API 会被调用，必须在加载时自行构建合成结构
  if (reason == DLL_PROCESS_ATTACH) {
    HANDLE h = CreateThread(nullptr, 0, StubInitThread, nullptr, 0, nullptr);
    if (h) CloseHandle(h);
  }
#else
  (void)reason;
#endif
  return TRUE;
}
