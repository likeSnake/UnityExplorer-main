// ============================================================================
// DumpSelfTest.cpp
// 离线自检：在进程内构造「Unity 2019.4 布局的 IL2CPP 结构」（Il2CppImage /
// Il2CppClass / FieldInfo / MethodInfo / PropertyInfo / Il2CppType / 假 metadata），
// 用与注入 DLL 完全相同的引擎 il2cpp_dump_core.hpp 生成 dump.cs，再逐行断言。
//
// 两套场景：
//   Phase A 标准布局（2019.4 教科书偏移）
//   Phase B 变形布局 —— 复现实测加固构建：
//     · Il2CppClass 尾部 +8 位移（flags 0x110 / token 0x114 / counts 0x118,0x11A,0x11C,0x124）
//     · MethodInfo 双指针（token 0x40 / flags 0x44 / slot 0x48 / paramCount 0x4A）
//     · Il2CppType.data 为 TypeDefinitionIndex（索引型）
//     验证校准器能否自动识别、以及类型名/修饰符/接口是否仍然正确。
//
// 编译：DumpSelfTest.vcxproj (Console, x64)
// ============================================================================
#include <Windows.h>
#include <cstdio>
#include <cstring>
#include <string>
#include <vector>

#include "il2cpp_dump_core.hpp"

using namespace ildump;

// ---------------------------------------------------------------------------
// 偏移配置
// ---------------------------------------------------------------------------
struct Offsets {
  uint32_t cImage, cName, cNS, cParent, cTDH, cFields, cProps, cMethods, cIfaces;
  uint32_t cFlags, cToken, cMCount, cPCount, cFCount, cICount, cInstSize;
  uint32_t mPtr, mName, mKlass, mRet, mParams, mToken, mFlags, mSlot, mPC, mSize;
  uint32_t fName, fType, fOffset, fStride;
  uint32_t pParent, pName, pGet, pSet, pStride;
  bool typeDataIndex;
  bool embeddedTypeDataIndex;
  bool typeDataHandle;
  uint32_t klassSize;
};

static const Offsets kStd = {
    0x00, 0x10, 0x18, 0x58, 0x68, 0x80, 0x90, 0x98, 0xA8,
    0x108, 0x10C, 0x110, 0x112, 0x114, 0x11C, 0xF0,
    0x00, 0x10, 0x18, 0x20, 0x28, 0x38, 0x3C, 0x40, 0x42, 0x48,
    0x00, 0x08, 0x18, 0x20,
    0x00, 0x08, 0x10, 0x18, 0x28,
    false, false, false, 0x140};

static const Offsets kDeform = {
    0x00, 0x10, 0x18, 0x58, 0x68, 0x80, 0x90, 0x98, 0xA8,
    0x110, 0x114, 0x118, 0x11A, 0x11C, 0x124, 0xF8,
    0x00, 0x10, 0x18, 0x20, 0x28, 0x40, 0x44, 0x48, 0x4A, 0x58,
    0x00, 0x08, 0x18, 0x20,
    0x00, 0x08, 0x10, 0x18, 0x28,
    true, false, false, 0x160};

// Index-based protected variant combined with the newer class/method layout.
static const Offsets kSuperIndex = {
    0x00, 0x10, 0x18, 0x58, 0x68, 0x80, 0x90, 0x98, 0xB0,
    0x118, 0x11C, 0x120, 0x122, 0x124, 0x128, 0x100,
    0x00, 0x18, 0x20, 0x28, 0x30, 0x48, 0x4C, 0x50, 0x52, 0x60,
    0x00, 0x08, 0x18, 0x20,
    0x00, 0x08, 0x10, 0x18, 0x28,
    true, true, false, 0x180};

// The real Super v3 log shows pointer-sized, non-class type data. Model the
// newer Il2CppMetadataTypeHandle representation separately.
static const Offsets kSuperHandle = {
    0x00, 0x10, 0x18, 0x58, 0x68, 0x80, 0x90, 0x98, 0xB0,
    0x118, 0x11C, 0x120, 0x122, 0x124, 0x128, 0x100,
    0x00, 0x18, 0x20, 0x28, 0x30, 0x48, 0x4C, 0x50, 0x52, 0x60,
    0x00, 0x08, 0x18, 0x20,
    0x00, 0x08, 0x10, 0x18, 0x28,
    false, false, true, 0x180};

static Offsets g_off = kStd;

// ---------------------------------------------------------------------------
// 合成内存布局分配器
// ---------------------------------------------------------------------------
static uintptr_t g_base = 0;
static size_t g_used = 0;
static size_t g_cap = 0;
// 方法指针必须落在真实可读内存里：把堆的 0x2000 处当作"模块基址"
static uintptr_t g_moduleBase = 0;
static uintptr_t g_img = 0;
static std::vector<uintptr_t> g_classes;
struct PendingClassType {
  uintptr_t type;
  uintptr_t klass;
};
static std::vector<PendingClassType> g_pendingClassTypes;
static std::unordered_map<uintptr_t, uintptr_t> g_classTypeHandles;

void HeapInit(size_t cap) {
  if (g_base) {  // 释放上一场景，避免全内存扫描扫到历史合成的 klass
    VirtualFree((void *)g_base, 0, MEM_RELEASE);
    g_base = 0;
  }
  g_base = (uintptr_t)VirtualAlloc(nullptr, cap, MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);
  g_used = 0;
  g_cap = cap;
  g_pendingClassTypes.clear();
  g_classTypeHandles.clear();
}
uintptr_t Alloc(size_t bytes, size_t align = 16) {
  g_used = (g_used + align - 1) & ~(align - 1);
  uintptr_t p = g_base + g_used;
  g_used += bytes;
  if (g_used > g_cap) {
    printf("FATAL: synthetic heap overflow\n");
    exit(2);
  }
  memset((void *)p, 0, bytes);
  return p;
}
uintptr_t CStr(const char *s) {
  size_t n = strlen(s) + 1;
  uintptr_t p = Alloc(n, 1);
  memcpy((void *)p, s, n);
  return p;
}

// ---------------------------------------------------------------------------
// 类型构造
// ---------------------------------------------------------------------------
uintptr_t MkType(uint8_t te, uintptr_t data, uint32_t attrs = 0, bool byref = false) {
  uintptr_t t = Alloc(0x10);
  *(uint64_t *)(t + 0x00) = data;
  *(uint32_t *)(t + 0x08) = (attrs & 0xFFFF) | ((uint32_t)te << 16) | (byref ? (1u << 30) : 0u);
  return t;
}
// 类类型：data 在标准场景为 klass 指针，在变形场景为 TypeDefinitionIndex
uintptr_t MkClassType(uint8_t te, uintptr_t klassPtr, uint32_t typeIndex, uint32_t attrs) {
  uintptr_t type = MkType(te, g_off.typeDataIndex ? (uintptr_t)typeIndex : klassPtr, attrs);
  if (g_off.typeDataHandle) g_pendingClassTypes.push_back({type, klassPtr});
  return type;
}

struct ClassBuilder {
  uintptr_t k = 0;
  std::vector<uintptr_t> fields, props, methods, ifaces;

  ClassBuilder(const char *name, const char *ns, uintptr_t image, uint32_t flags,
               uintptr_t parent = 0) {
    k = Alloc(g_off.klassSize);
    *(uintptr_t *)(k + g_off.cImage) = image;
    *(uintptr_t *)(k + g_off.cName) = CStr(name);
    *(uintptr_t *)(k + g_off.cNS) = ns && *ns ? CStr(ns) : CStr("");
    *(uintptr_t *)(k + g_off.cParent) = parent;
    *(uint32_t *)(k + g_off.cFlags) = flags;
    *(uint32_t *)(k + g_off.cToken) = 0x02000001;
    *(uint32_t *)(k + g_off.cInstSize) = 0x10;
  }
  void SetTypeDefHandle(uintptr_t h) { *(uintptr_t *)(k + g_off.cTDH) = h; }
  void AddField(const char *name, uintptr_t type, int32_t off, uint32_t fattrs) {
    uintptr_t f = Alloc(g_off.fStride);
    *(uintptr_t *)(f + g_off.fName) = CStr(name);
    *(uintptr_t *)(f + g_off.fType) = type;
    *(uintptr_t *)(f + 0x10) = k;
    *(int32_t *)(f + g_off.fOffset) = off;
    *(uint32_t *)(f + 0x1C) = 0x04000001;
    // FieldInfo 的 type->attrs 承载 FieldAttributes
    uint32_t bits = *(uint32_t *)(type + 0x08);
    *(uint32_t *)(type + 0x08) = (bits & 0xFFFF0000u) | (fattrs & 0xFFFF);
    fields.push_back(f);
  }
  void AddProp(const char *name, uintptr_t getter, uintptr_t setter) {
    uintptr_t p = Alloc(g_off.pStride);
    *(uintptr_t *)(p + g_off.pParent) = k;
    *(uintptr_t *)(p + g_off.pName) = CStr(name);
    *(uintptr_t *)(p + g_off.pGet) = getter;
    *(uintptr_t *)(p + g_off.pSet) = setter;
    props.push_back(p);
  }
  uintptr_t AddMethod(const char *name, uintptr_t ret, uintptr_t rvaOrZero,
                      std::vector<uintptr_t> params, uint16_t flags, uint16_t slot,
                      uint32_t token) {
    uintptr_t mi = Alloc(g_off.mSize);
    *(uintptr_t *)(mi + g_off.mPtr) = rvaOrZero ? (g_moduleBase + rvaOrZero) : 0;
    *(uintptr_t *)(mi + g_off.mName) = CStr(name);
    *(uintptr_t *)(mi + g_off.mKlass) = k;
    *(uintptr_t *)(mi + g_off.mRet) = ret;
    uintptr_t pa = 0;
    if (!params.empty()) {
      pa = Alloc(params.size() * 8, 8);
      for (size_t i = 0; i < params.size(); ++i) *(uintptr_t *)(pa + i * 8) = params[i];
    }
    *(uintptr_t *)(mi + g_off.mParams) = pa;
    *(uint32_t *)(mi + g_off.mToken) = token;
    *(uint16_t *)(mi + g_off.mFlags) = flags;
    *(uint16_t *)(mi + g_off.mSlot) = slot;
    *(uint8_t *)(mi + g_off.mPC) = (uint8_t)params.size();
    methods.push_back(mi);
    return mi;
  }
  void AddIface(uintptr_t ik) { ifaces.push_back(ik); }

  void Commit() {
    if (!fields.empty()) {
      uintptr_t arr = Alloc(fields.size() * g_off.fStride, 16);
      for (size_t i = 0; i < fields.size(); ++i)
        memcpy((void *)(arr + i * g_off.fStride), (void *)fields[i], g_off.fStride);
      *(uintptr_t *)(k + g_off.cFields) = arr;
      *(uint16_t *)(k + g_off.cFCount) = (uint16_t)fields.size();
    }
    if (!props.empty()) {
      uintptr_t arr = Alloc(props.size() * g_off.pStride, 16);
      for (size_t i = 0; i < props.size(); ++i)
        memcpy((void *)(arr + i * g_off.pStride), (void *)props[i], g_off.pStride);
      *(uintptr_t *)(k + g_off.cProps) = arr;
      *(uint16_t *)(k + g_off.cPCount) = (uint16_t)props.size();
    }
    if (!methods.empty()) {
      uintptr_t arr = Alloc(methods.size() * 8, 8);
      for (size_t i = 0; i < methods.size(); ++i) *(uintptr_t *)(arr + i * 8) = methods[i];
      *(uintptr_t *)(k + g_off.cMethods) = arr;
      *(uint16_t *)(k + g_off.cMCount) = (uint16_t)methods.size();
    }
    if (!ifaces.empty()) {
      uintptr_t arr = Alloc(ifaces.size() * 8, 8);
      for (size_t i = 0; i < ifaces.size(); ++i) *(uintptr_t *)(arr + i * 8) = ifaces[i];
      *(uintptr_t *)(k + g_off.cIfaces) = arr;
      *(uint16_t *)(k + g_off.cICount) = (uint16_t)ifaces.size();
    }
  }
};

// ---------------------------------------------------------------------------
// 假 metadata（参数名）
// ---------------------------------------------------------------------------
struct FakeMeta {
  uintptr_t base = 0;
  uint32_t stringOff = 0x1000, methodsOff = 0x2000, paramsOff = 0x3000, typeDefsOff = 0x4000;
  std::vector<std::string> strings;
  std::string blob;

  uint32_t AddStr(const char *s) {
    uint32_t off = (uint32_t)blob.size();
    strings.push_back(s);
    blob += s;
    blob.push_back('\0');
    return off;
  }
  void Init() {
    base = Alloc(0x8000, 0x1000);
    *(uint32_t *)(base + 0x00) = 0xFAB11BAF;
    *(int32_t *)(base + 0x04) = 24;
    *(int32_t *)(base + 0x18) = (int32_t)stringOff;
    *(int32_t *)(base + 0x1C) = 0x1000;
    *(int32_t *)(base + 0x30) = (int32_t)methodsOff;
    *(int32_t *)(base + 0x34) = 256;
    *(int32_t *)(base + 0x58) = (int32_t)paramsOff;
    *(int32_t *)(base + 0x5C) = 256;
    *(int32_t *)(base + 0xA0) = (int32_t)typeDefsOff;
    *(int32_t *)(base + 0xA4) = 256;
  }
  void FlushStrings() { memcpy((void *)(base + stringOff), blob.data(), blob.size()); }
  uintptr_t MethodDef(uint32_t i) { return base + methodsOff + i * 0x34; }
  uintptr_t ParamDef(uint32_t i) { return base + paramsOff + i * 0x0C; }
  uintptr_t TypeDef(uint32_t i) { return base + typeDefsOff + i * 0x5C; }
};

// ---------------------------------------------------------------------------
// 断言
// ---------------------------------------------------------------------------
static int g_fail = 0, g_pass = 0;

void Check(bool cond, const char *what, const std::string &extra = "") {
  if (cond) {
    ++g_pass;
    printf("  [ OK ] %s\n", what);
  } else {
    ++g_fail;
    printf("  [FAIL] %s %s\n", what, extra.c_str());
  }
}
void CheckContains(const std::string &hay, const std::string &needle, const char *what) {
  bool ok = hay.find(needle) != std::string::npos;
  Check(ok, what, ok ? "" : ("missing: <<" + needle + ">>"));
}
void LogPrint(void *, const char *msg) { printf("      | %s\n", msg); }

std::string ReadFile(const char *path) {
  std::string s;
  FILE *f = fopen(path, "rb");
  if (!f) return s;
  fseek(f, 0, SEEK_END);
  long n = ftell(f);
  fseek(f, 0, SEEK_SET);
  s.resize((size_t)n);
  if (n) fread(&s[0], 1, (size_t)n, f);
  fclose(f);
  return s;
}

std::string RvaPrefix(uintptr_t moduleBase, uint64_t rva, int slot) {
  char b[192];
  if (slot >= 0)
    snprintf(b, sizeof(b), "\t// RVA: 0x%llx VA: 0x%llx Slot: %d\r\n", (unsigned long long)rva,
             (unsigned long long)(moduleBase + rva), slot);
  else
    snprintf(b, sizeof(b), "\t// RVA: 0x%llx VA: 0x%llx\r\n", (unsigned long long)rva,
             (unsigned long long)(moduleBase + rva));
  return b;
}

std::string ManagerRvaLine(const char *fieldName, uintptr_t moduleBase,
                           uintptr_t slot, const char *status) {
  char b[192];
  const uint64_t rva = (uint64_t)(slot - moduleBase);
  snprintf(b, sizeof(b), "// %s = 0x%llX; // decimal: %llu status=%s\r\n",
           fieldName, (unsigned long long)rva, (unsigned long long)rva, status);
  return b;
}

// ---------------------------------------------------------------------------
// 场景构建（两套布局共用）
// ---------------------------------------------------------------------------
struct Scene {
  Env env;
  std::vector<uintptr_t> classes;
  uintptr_t img = 0;
  uintptr_t moduleBase = 0;
  uintptr_t buffManagerKlass = 0;
  uintptr_t transparentTagHandlerKlass = 0;
  uintptr_t buffManagerSlot = 0;
  uintptr_t transparentTagHandlerSlot = 0;
  uintptr_t buffManagerStaticFields = 0;
  uintptr_t transparentTagHandlerStaticFields = 0;
};

Scene BuildScene(bool withSerializable) {
  Scene sc;
  HeapInit(48 << 20);
  g_moduleBase = g_base + 0x2000;  // 可读的"模块基址"，用于 RVA 计算
  sc.moduleBase = g_moduleBase;

  uintptr_t imgName = CStr("Test.dll");
  uintptr_t asm_ = Alloc(0x60);
  uintptr_t img = Alloc(0x48);
  g_img = img;
  sc.img = img;
  *(uintptr_t *)(img + 0x00) = imgName;
  *(uintptr_t *)(img + 0x08) = CStr("Test");
  *(uintptr_t *)(img + 0x10) = asm_;
  *(uint32_t *)(img + 0x18) = 0x100;  // typeStart
  *(uintptr_t *)(asm_ + 0x00) = img;

  // 索引约定：classes[i] 的 TypeDefinitionIndex = 0x100 + i
  ClassBuilder cObject("Object", "System", img, 0x00000001);
  cObject.Commit();
  ClassBuilder cBase("BaseClass", "Test.Game", img, 0x00000001, cObject.k);
  cBase.Commit();
  ClassBuilder cIFoo("IFoo", "Test.Game", img, 0x00000021);
  cIFoo.Commit();
  ClassBuilder cIBar("IBar", "Test.Game", img, 0x00000021);
  cIBar.Commit();
  ClassBuilder cValueType("ValueType", "System", img, 0x00000001, cObject.k);
  cValueType.Commit();
  ClassBuilder cEnum("Enum", "System", img, 0x00000001, cValueType.k);
  cEnum.Commit();

  uintptr_t tVoid = MkType(T_VOID, 0);
  uintptr_t tI4 = MkType(T_I4, 0);
  uintptr_t tString = MkType(T_STRING, 0);
  uintptr_t tU1 = MkType(T_U1, 0);
  uintptr_t tR4elem = MkType(T_R4, 0);
  uintptr_t tI4elem = MkType(T_I4, 0);

  // 假 metadata
  FakeMeta fm;
  fm.Init();
  uint32_t sSample = fm.AddStr("Sample");
  uint32_t sNs = fm.AddStr("Test.Game");
  uint32_t sP0 = fm.AddStr("a");
  uint32_t sP1 = fm.AddStr("b");
  uint32_t sP2 = fm.AddStr("c");
  uint32_t sValue = fm.AddStr("value");
  fm.FlushStrings();

  // Sample：public sealed（+可选 Serializable），基类 + 2 接口
  ClassBuilder cSample("Sample", "Test.Game", img,
                       0x00000101 | (withSerializable ? 0x00002000 : 0), cBase.k);
  cSample.AddIface(cIFoo.k);
  cSample.AddIface(cIBar.k);
  cSample.AddField("m_count", MkType(T_I4, 0, 0x0006), 0x10, 0x0006);
  cSample.AddField("s_flag", MkType(T_BOOLEAN, 0, 0x0011), 0x00, 0x0011 | 0x0020);
  cSample.AddField("Name", MkType(T_STRING, 0, 0x0046), -1, 0x0046);
  uintptr_t mGet = cSample.AddMethod("get_Count", tI4, 0x1010, {}, 0x0006, 0xFFFF, 0x06000001);
  uintptr_t mSet = cSample.AddMethod("set_Count", tVoid, 0x1020, {MkType(T_I4, 0, 0x0000)}, 0x0006,
                                     0xFFFF, 0x06000002);
  cSample.AddProp("Count", mGet, mSet);
  cSample.AddMethod(".ctor", tVoid, 0x1000, {}, 0x0006, 0xFFFF, 0x06000003);
  cSample.AddMethod("DoWork", tVoid, 0x1030,
                    {MkType(T_I4, 0, 0x0000), MkType(T_R4, tR4elem, 0x0000, true),
                     MkType(T_I4, tI4elem, 0x0002, true)},
                    0x0006, 0xFFFF, 0x06000004);
  cSample.AddMethod("Virtual", tVoid, 0x1040, {}, 0x0146, 4, 0x06000005);
  cSample.AddMethod("NoPointer", tVoid, 0, {}, 0x0006, 0xFFFF, 0x06000006);
  cSample.Commit();

  // 枚举（父类 Enum -> kind=enum；成员用索引型类类型）
  ClassBuilder cEnumMy("MyEnum", "Test.Game", img, 0x00000101, cEnum.k);
  uintptr_t tEnum = MkClassType(T_VALUETYPE, cEnumMy.k, 0x102, 0x00000101);
  cEnumMy.AddField("value__", MkType(T_I4, 0, 0x0006), 0x10, 0x0006);
  cEnumMy.AddField("ValueA", tEnum, -1, 0x0056);
  cEnumMy.Commit();
  *(uint32_t *)(cEnumMy.k + g_off.cInstSize) = 4;  // 枚举实例大小 = 4

  // 结构体 + 接口
  ClassBuilder cStruct("MyStruct", "Test.Game", img, 0x00000101, cValueType.k);
  cStruct.AddIface(cIFoo.k);
  cStruct.AddIface(cIBar.k);
  cStruct.AddField("x", MkType(T_I4, 0, 0x0006), 0x10, 0x0006);
  cStruct.Commit();

  // 数组 / 泛型 / 索引型类类型字段
  ClassBuilder cArr("Arrays", "Test.Game", img, 0x00000001);
  uintptr_t tSz = MkType(T_SZARRAY, MkType(T_U1, 0, 0x0000), 0x0006);
  cArr.AddField("data", tSz, 0x18, 0x0006);
  uintptr_t tArr2 = MkType(T_ARRAY, [&]() {
    uintptr_t at = Alloc(0x20);
    *(uintptr_t *)(at + 0x00) = MkType(T_I4, 0, 0x0000);
    *(uint8_t *)(at + 0x08) = 2;
    return at;
  }(), 0x0006);
  cArr.AddField("grid", tArr2, 0x20, 0x0006);
  uintptr_t tGen = MkType(T_GENERICINST, [&]() {
    uintptr_t gc = Alloc(0x20);
    *(uintptr_t *)(gc + 0x18) = cBase.k;  // cached_class
    return gc;
  }(), 0x0006);
  cArr.AddField("boxed", tGen, 0x28, 0x0006);
  // 索引型 data：0x106 = cBase 在类表中的 TypeDefinitionIndex
  cArr.AddField("indexed", MkClassType(T_CLASS, cBase.k, 0x106, 0x0006), 0x30, 0x0006);
  cArr.Commit();

  // 额外类：让 字段/方法/属性 计数样本值各不相同，避免计数字段校准歧义
  ClassBuilder cE1("Extra1", "Test.Game", img, 0x00000001);
  cE1.AddField("f1", MkType(T_I4, 0, 0x0006), 0x10, 0x0006);
  cE1.AddMethod("M1", tVoid, 0x1100, {}, 0x0006, 0xFFFF, 0x06001001);
  cE1.AddMethod("M2", tVoid, 0x1110, {}, 0x0006, 0xFFFF, 0x06001002);
  cE1.AddProp("P1", cE1.methods[0], 0);
  cE1.Commit();

  ClassBuilder cE2("Extra2", "Test.Game", img, 0x00000001);
  cE2.AddField("f1", MkType(T_I4, 0, 0x0006), 0x10, 0x0006);
  cE2.AddField("f2", MkType(T_I4, 0, 0x0006), 0x14, 0x0006);
  cE2.AddMethod("M1", tVoid, 0x1200, {}, 0x0006, 0xFFFF, 0x06002001);
  cE2.AddMethod("M2", tVoid, 0x1210, {}, 0x0006, 0xFFFF, 0x06002002);
  cE2.AddMethod("M3", tVoid, 0x1220, {}, 0x0006, 0xFFFF, 0x06002003);
  cE2.AddProp("P1", cE2.methods[0], 0);
  cE2.AddProp("P2", cE2.methods[1], 0);
  cE2.Commit();

  ClassBuilder cE3("Extra3", "Test.Game", img, 0x00000001);
  cE3.AddField("f1", MkType(T_I4, 0, 0x0006), 0x10, 0x0006);
  cE3.AddField("f2", MkType(T_I4, 0, 0x0006), 0x14, 0x0006);
  cE3.AddField("f3", MkType(T_I4, 0, 0x0006), 0x18, 0x0006);
  cE3.AddMethod("M1", tVoid, 0x1300, {}, 0x0006, 0xFFFF, 0x06003001);
  cE3.AddMethod("M2", tVoid, 0x1310, {}, 0x0006, 0xFFFF, 0x06003002);
  cE3.AddMethod("M3", tVoid, 0x1320, {}, 0x0006, 0xFFFF, 0x06003003);
  cE3.AddMethod("M4", tVoid, 0x1330, {}, 0x0006, 0xFFFF, 0x06003004);
  cE3.AddProp("P1", cE3.methods[0], 0);
  cE3.AddProp("P2", cE3.methods[1], 0);
  cE3.AddProp("P3", cE3.methods[2], 0);
  cE3.Commit();

  // 空类
  ClassBuilder cModule("<Module>", "", img, 0x00000000);
  cModule.Commit();

  // 两个运行时偏移目标。static_fields 随当前 Il2CppClass 布局移动，
  // singleton 字段分别位于 +0x0 和 +0x8，实例首指针回指自身 klass。
  ClassBuilder cBuffManager("BuffManager", "", img, 0x00000001);
  cBuffManager.Commit();
  ClassBuilder cTransparentTagHandler("TransparentTagHandler", "", img, 0x00000001);
  cTransparentTagHandler.Commit();
  const uint32_t staticFieldsOffset = g_off.cIfaces + 0x10;
  uintptr_t buffStaticFields = Alloc(0x20);
  uintptr_t buffInstance = Alloc(0x20);
  *(uintptr_t *)buffInstance = cBuffManager.k;
  *(uintptr_t *)(buffStaticFields + 0x0) = buffInstance;
  *(uintptr_t *)(cBuffManager.k + staticFieldsOffset) = buffStaticFields;
  uintptr_t transparentStaticFields = Alloc(0x20);
  uintptr_t transparentInstance = Alloc(0x20);
  *(uintptr_t *)transparentInstance = cTransparentTagHandler.k;
  *(uintptr_t *)(transparentStaticFields + 0x8) = transparentInstance;
  *(uintptr_t *)(cTransparentTagHandler.k + staticFieldsOffset) = transparentStaticFields;
  sc.buffManagerKlass = cBuffManager.k;
  sc.transparentTagHandlerKlass = cTransparentTagHandler.k;
  sc.buffManagerStaticFields = buffStaticFields;
  sc.transparentTagHandlerStaticFields = transparentStaticFields;

  // 类表（顺序即 TypeDefinitionIndex 顺序）
  sc.classes = {cModule.k, cSample.k, cEnumMy.k, cStruct.k,
                cArr.k,    cObject.k, cBase.k,  cIFoo.k,
                cE1.k,     cE2.k,     cE3.k,    cBuffManager.k,
                cTransparentTagHandler.k};
  if (g_off.typeDataHandle) {
    // Allocate handles in global TypeDefinitionIndex order. Their address
    // order therefore models a contiguous metadata type-definition table.
    for (uintptr_t k : sc.classes) g_classTypeHandles[k] = Alloc(0x20, 16);
    for (const PendingClassType &pending : g_pendingClassTypes)
      *(uintptr_t *)(pending.type + 0x00) = g_classTypeHandles[pending.klass];
  }

  // byval_arg can point back to klass, store a TypeDefinitionIndex, or carry
  // a metadata type handle. All forms keep a unique TypeDef token.
  for (size_t i = 0; i < sc.classes.size(); ++i) {
    uintptr_t k = sc.classes[i];
    uintptr_t byvalData = k;
    if (g_off.embeddedTypeDataIndex) byvalData = (uintptr_t)(0x100 + i);
    if (g_off.typeDataHandle) byvalData = g_classTypeHandles[k];
    *(uintptr_t *)(k + 0x20) = byvalData;
    *(uint32_t *)(k + 0x28) = ((uint32_t)T_CLASS) << 16;   // byval_arg.bits (type=CLASS)
    if (g_off.typeDataHandle) *(uintptr_t *)(k + g_off.cTDH) = byvalData;
    *(uint32_t *)(k + g_off.cToken) = 0x02000000u | (uint32_t)(0x100 + i + 1);
  }
  g_classes = sc.classes;
  *(uint32_t *)(img + 0x1C) = (uint32_t)sc.classes.size();
  uintptr_t classArr = Alloc(sc.classes.size() * 8, 8);
  for (size_t i = 0; i < sc.classes.size(); ++i)
    *(uintptr_t *)(classArr + i * 8) = sc.classes[i];

  // Class-level RVA output is driven by the module global slot containing a
  // klass pointer. Use an explicit synthetic slot so every layout phase can
  // assert the exact two-line Unity-Offset-compatible rendering.
  uintptr_t sampleClassSlot = Alloc(8, 8);
  *(uintptr_t *)sampleClassSlot = cSample.k;
  sc.buffManagerSlot = Alloc(8, 8);
  *(uintptr_t *)sc.buffManagerSlot = cBuffManager.k;
  sc.transparentTagHandlerSlot = Alloc(8, 8);
  *(uintptr_t *)sc.transparentTagHandlerSlot = cTransparentTagHandler.k;

  // Sample 的 typeDef + methodDef + paramDef（参数名）
  if (!g_off.typeDataHandle) cSample.SetTypeDefHandle(fm.TypeDef(0));
  {
    uintptr_t td = fm.TypeDef(0);
    *(int32_t *)(td + 0x00) = (int32_t)sSample;
    *(int32_t *)(td + 0x04) = (int32_t)sNs;
    *(int32_t *)(td + 0x28) = 0;
    *(uint16_t *)(td + 0x44) = 6;
  }
  struct MD {
    uint32_t token, pStart, pCount;
  };
  MD mds[6] = {{0x06000003, 0, 0}, {0x06000001, 0, 0}, {0x06000002, 3, 1},
               {0x06000004, 0, 3}, {0x06000005, 0, 0}, {0x06000006, 0, 0}};
  uint32_t paramNames[4] = {sP0, sP1, sP2, sValue};
  for (int i = 0; i < 6; ++i) {
    uintptr_t md = fm.MethodDef(i);
    *(int32_t *)(md + 0x0C) = (int32_t)mds[i].pStart;
    *(uint32_t *)(md + 0x28) = mds[i].token;
    *(uint16_t *)(md + 0x32) = (uint16_t)mds[i].pCount;
  }
  for (int i = 0; i < 4; ++i) {
    uintptr_t pd = fm.ParamDef(i);
    *(int32_t *)(pd + 0x00) = (int32_t)paramNames[i];
  }

  sc.env.moduleBase = g_moduleBase;
  sc.env.moduleSize = g_cap - 0x2000; // Do not scan outside the synthetic module allocation.
  sc.env.moduleName = "GameAssembly.dll";
  sc.env.overrideImages = {img};
  sc.env.overrideClassPointerSlots = {
      {cSample.k, sampleClassSlot},
      {cBuffManager.k, sc.buffManagerSlot},
      {cTransparentTagHandler.k, sc.transparentTagHandlerSlot},
  };
  sc.env.getCorlib = []() -> void * { return (void *)g_img; };
  sc.env.imageGetClassCount = [](void *) -> size_t { return g_classes.size(); };
  sc.env.imageGetClass = [](void *, size_t i) -> void * {
    return i < g_classes.size() ? (void *)g_classes[i] : nullptr;
  };
  sc.env.imageGetName = [](void *p) -> const char * {
    return p == (void *)g_img ? "Test.dll" : "";
  };
  return sc;
}

// v7 regression fixtures deliberately differ from the historical Offset.h.
// They exercise metadata evidence, ambiguity, and rejection of unusable values.
void TestOffsetReport() {
  printf("\n########## Phase G: Offset inventory discovery ##########\n");
  g_off = kStd;
  HeapInit(2 << 20);
  g_moduleBase = Alloc(0x20000);
  DWORD oldProtect = 0;
  Check(VirtualProtect((void *)(g_moduleBase + 0x10000), 0x1000,
                       PAGE_EXECUTE_READ, &oldProtect) != 0, "G: synthetic executable page");
  uintptr_t img = Alloc(0x48);
  *(uintptr_t *)img = CStr("Offsets.dll");
  ClassBuilder base("EntityKit", "", img, 1);
  base.AddField("<actorModel>k__BackingField", MkType(T_OBJECT, 0), 0x28, FA_PRIVATE);
  base.Commit();
  ClassBuilder actor("ActorModel", "", img, 1, base.k);
  actor.AddField("objectMsg", MkType(T_OBJECT, 0), 0xE8, FA_PUBLIC);
  actor.AddField("<objectMsg>k__BackingField", MkType(T_OBJECT, 0), 0xF8, FA_PRIVATE);
  actor.AddField("_staticZero", MkType(T_I4, 0), 0, FA_STATIC);
  actor.AddField("constant", MkType(T_I4, 0), 0, FA_LITERAL | FA_STATIC);
  actor.AddField("tls", MkType(T_I4, 0), -1, FA_STATIC);
  actor.Commit();
  ClassBuilder tth("TransparentTagHandler", "", img, 1);
  tth.AddField("actorkit", MkType(T_OBJECT, 0), 0x78, FA_PRIVATE);
  tth.Commit();
  ClassBuilder userDecoy("UserData", "", img, 1);
  userDecoy.AddField("success", MkType(T_BOOLEAN, 0), 0x10, FA_PUBLIC);
  userDecoy.Commit();
  ClassBuilder user("UserData", "", img, 1);
  user.AddField("battleData", MkType(T_OBJECT, 0), 0x90, FA_PUBLIC);
  user.Commit();
  ClassBuilder otherNs("ActorModel", "Other", img, 1);
  otherNs.AddField("objectMsg", MkType(T_OBJECT, 0), 0x48, FA_PUBLIC);
  otherNs.Commit();
  ClassBuilder generic("List`1", "System.Collections.Generic", img, 1);
  generic.AddField("_items", MkType(T_OBJECT, 0), 0, FA_PUBLIC);
  generic.Commit();
  ClassBuilder transition("TransitionSourceType", "", img, 1);
  transition.Commit();
  auto crossParams = [&]() -> std::vector<uintptr_t> {
    return {MkType(T_I4, 0), MkType(T_R4, 0), MkType(T_I4, 0), MkType(T_R4, 0),
            MkType(T_R4, 0), MkType(T_VALUETYPE, transition.k), MkType(T_BOOLEAN, 0)};
  };
  ClassBuilder anim("AnimPlayable", "", img, 1);
  auto wrongParams = crossParams();
  wrongParams[5] = MkType(T_I4, 0);
  uintptr_t cross = anim.AddMethod("CrossFadeInFixedTime", MkType(T_VOID, 0), 0x10020,
                                 crossParams(), 6, 0xFFFF, 0x06000001);
  anim.AddMethod("CrossFadeInFixedTime", MkType(T_VOID, 0), 0x10040,
                 wrongParams, 6, 0xFFFF, 0x06000002);
  anim.AddMethod("Overload", MkType(T_VOID, 0), 0x10060, {MkType(T_I4, 0)}, 6, 0, 0x06000003);
  anim.AddMethod("Overload", MkType(T_VOID, 0), 0x10080, {MkType(T_R4, 0)}, 6, 0, 0x06000004);
  anim.AddMethod("Null", MkType(T_VOID, 0), 0, {}, 6, 0, 0x06000005);
  anim.AddMethod("Outside", MkType(T_VOID, 0), 0x30000, {}, 6, 0, 0x06000006);
  anim.AddMethod("Data", MkType(T_VOID, 0), 0x200, {}, 6, 0, 0x06000007);
  anim.Commit();

  Env env;
  env.moduleBase = g_moduleBase;
  env.moduleSize = 0x20000;
  env.moduleName = "GameAssembly.dll";
  Options opt;
  LogCtx log;
  Dumper d;
  d.env = &env; d.opt = &opt; d.log = &log; d.lay.calibrated = true;
  d.klasses_ = {base.k, actor.k, tth.k, userDecoy.k, user.k, otherNs.k, generic.k, anim.k};
  d.typeTable_ = d.klasses_; // Index must deduplicate pointers from both sources.
  OffsetReporter r(d);
  r.buildIndex();
  auto get = [&](const char *key) -> OffsetResult {
    for (const auto &s : kOffsetSpecs) if (!strcmp(key, s.key)) return r.resolve(s);
    return OffsetReporter::failure("TEST_MISSING_KEY", key);
  };
  Check(get("ActorModel.ObjectMsg").value == 0xE8, "G: live offset replaces historical 0xD8 and beats backing alias");
  Check(r.field("ActorModel", "", "actorModel").value == 0x28,
        "G: inherited auto-property backing field");
  Check(r.field("ActorModel", "Other", "objectMsg").value == 0x48, "G: namespace isolation");
  Check(r.classes[r.key("ActorModel", "")].size() == 1, "G: deduplicate identical klass pointers");
  auto zero = r.field(actor.k, "_staticZero");
  Check(zero.hasValue && zero.value == 0 && zero.status == "FOUND", "G: zero static field offset is valid");
  Check(r.field(actor.k, "constant").status == "NOT_FIELD_OFFSET", "G: literal is not an offset");
  Check(r.field(actor.k, "tls").status == "SPECIAL_STORAGE", "G: negative offset is not unsigned RVA");
  Check(get("Il2CppList.Items").status == "UNRESOLVED_LAYOUT", "G: generic zero instance offset not usable");
  Check(get("MemoryShockTransparentTagHandlerActorKitOffset").value == 0x78, "G: TTH exact private field name");
  Check(r.field(tth.k, "actorKit").status == "NOT_FOUND", "G: field spelling is case sensitive");
  Check(get("ActorModel.battleData").value == 0x90, "G: same-name class resolved by unique member");
  r.classes[r.key("UserData", "")].push_back(user.k);
  Check(get("ActorModel.battleData").status == "AMBIGUOUS", "G: multiple declaring candidates are rejected");
  r.classes[r.key("UserData", "")].pop_back();
  *(uintptr_t *)(base.k + g_off.cParent) = actor.k;
  Check(r.field(actor.k, "missing").status == "NOT_FOUND", "G: parent cycle terminates");
  *(uintptr_t *)(base.k + g_off.cParent) = 0;
  Check(r.field(uintptr_t(0x1234), "missing").status == "UNREADABLE", "G: failed metadata reads not zero");
  auto method = get("CrossFadeInFixedTime");
  Check(method.status == "FOUND" && method.value == 0x10020, "G: CrossFade seven-parameter signature disambiguates");
  Check(r.method("AnimPlayable", "", "Overload", 1).status == "AMBIGUOUS", "G: same-count overload rejected");
  Check(r.method("AnimPlayable", "", "Null", 0).status == "NOT_FOUND", "G: null methodPointer rejected");
  Check(r.method("AnimPlayable", "", "Outside", 0).status == "OUTSIDE_MODULE", "G: method outside selected module rejected");
  Check(r.method("AnimPlayable", "", "Data", 0).status == "NON_EXECUTABLE", "G: method in data page flagged");
  Check(get("m_TransparentTagHandler2").status == "NOT_LOADED", "G: normal run does not invent Super RVA");
  env.superVariant = true;
  env.moduleName = "GameAssembly_Super.dll";
  Check(get("m_TransparentTagHandler").status == "NOT_LOADED", "G: Super run does not invent normal RVA");
  env.otherVariantLoaded = true;
  Check(get("m_TransparentTagHandler").status == "NOT_DUMPED", "G: loaded unselected variant labelled");
  env.otherVariantLoaded = false;
  uintptr_t slot = g_moduleBase + 0x100;
  *(uintptr_t *)slot = tth.k;
  d.classPointerSlots_[tth.k] = slot;
  Check(get("m_TransparentTagHandler2").value == 0x100, "G: TypeInfo is slot RVA not klass VA");
  *(uintptr_t *)slot = actor.k;
  Check(get("m_TransparentTagHandler2").status == "UNREADABLE", "G: changed slot readback rejected");
  *(uintptr_t *)slot = tth.k;
  // Also exercise the production named-target scanner with the decoy first.
  opt.classRvas = false;
  *(uintptr_t *)(g_moduleBase + 0x180) = user.k;
  *(uintptr_t *)(g_moduleBase + 0x188) = userDecoy.k;
  d.discoverClassPointerSlots({});
  Check(d.classPointerSlots_.count(user.k) && !d.classPointerSlots_.count(userDecoy.k),
        "G: summary-only mode retains business UserData slot");
  Check(get("m_UserDataManager2").value == 0x180, "G: TypeInfo business member identity");
  Check(r.staticFieldsLayout().status == "UNRESOLVED", "G: no unverified static_fields layout guess");
  ManagerRvaResult evidence;
  evidence.verified = true; evidence.staticFieldsOffset = 0xB8;
  d.lay.cInterfaces = 0xB0;
  d.managerRvaResults_ = {evidence};
  Check(r.staticFieldsLayout().value == 0xB8, "G: real B8 evidence wins over interfaces+10=C0");
  evidence.staticFieldsOffset = 0xC0;
  d.managerRvaResults_.push_back(evidence);
  Check(r.staticFieldsLayout().status == "AMBIGUOUS", "G: contradictory layout evidence rejected");
  d.managerRvaResults_.pop_back();
  Check(get("Call_GetVelocity").status == "SKIPPED_LEGACY" &&
        get("op_Subtraction").status == "SKIPPED_LEGACY", "G: optional obsolete methods explicitly skipped");
  Check(get("MemoryShockHookContextEntityKitOffset").status == "UNRESOLVED",
        "G: unknown hook context cannot be inferred from same byte offset");
  // A matching vtable pair is only a candidate; it does not identify hook intent.
  ClassBuilder interactable("ActorKitInteractableDevice", "", img, 1);
  uintptr_t vk = Alloc(0x200);
  memcpy((void *)vk, (void *)interactable.k, g_off.klassSize);
  uintptr_t mi = Alloc(g_off.mSize);
  memcpy((void *)mi, (void *)cross, g_off.mSize);
  *(uintptr_t *)(mi + g_off.mKlass) = vk;
  *(uintptr_t *)(vk + 0x198) = g_moduleBase + 0x10020;
  *(uintptr_t *)(vk + 0x1A0) = mi;
  r.classes[r.key("ActorKitInteractableDevice", "")] = {vk};
  Check(get("MemoryShockInteractableVtableSlotOffset").status == "CANDIDATE", "G: vtable match never claims hook semantics");
  Check(get("Base_WindowBase2").status == "NOT_LOADED", "G: missing native module explicit");
  env.unityPlayerBase = g_moduleBase;
  env.unityPlayerSize = 0x20000;
  env.unityPlayerName = "UnityPlayer_LVB.dll";
  env.windowWidth = 1280; env.windowHeight = 720;
  uintptr_t native = Alloc(0xD0), statics = Alloc(0x20), dims = Alloc(0x20);
  *(uintptr_t *)(native + 0xB8) = statics;
  *(uintptr_t *)(statics + 8) = dims;
  *(int *)(dims + 0x14) = 1280; *(int *)(dims + 0x18) = 720;
  *(uintptr_t *)(g_moduleBase + 0x1F0) = native;
  r.windowScanned = false;
  auto window = get("Base_WindowBase2");
  Check(window.status == "CANDIDATE" && window.value == 0x1F0, "G: native window chain yields only a candidate");
  *(uintptr_t *)(g_moduleBase + 0x1F8) = native;
  r.windowScanned = false;
  Check(get("Base_WindowBase2").status == "AMBIGUOUS", "G: duplicate native slots rejected");
  *(int *)(dims + 0x14) = 640;
  r.windowScanned = false;
  Check(get("Base_WindowBase2").status == "NOT_FOUND", "G: wrong client dimensions rejected");

  d.st.classes = 17; d.st.classTypeFallbacks = 3;
  const Stats before = d.st;
  const char *reportPath = "selftest_offset_report.cs";
  Writer writer;
  Check(writer.open(reportPath), "G: open report fixture");
  d.writeOffsetSummary(writer);
  writer.close();
  const std::string report = ReadFile(reportPath);
  Check(!memcmp(&before, &d.st, sizeof(Stats)), "G: report preserves main dump statistics");
  size_t count = 0, pos = 0;
  while ((pos = report.find("// Offset::", pos)) != std::string::npos) { ++count; pos += 11; }
  Check(count == 221, "G: all 221 inventory entries rendered once");
  CheckContains(report, "Offset::ActorModel.ObjectMsg = 0xE8; // status=FOUND", "G: final report uses live field offset");
  CheckContains(report, "Offset::Il2CppList.Items = UNRESOLVED_LAYOUT;", "G: unsafe generic offset withheld in final report");
}

// ---------------------------------------------------------------------------
struct IndexTable {
  std::vector<std::string> header;
  std::vector<std::vector<std::string>> rows;
  bool valid = true;
  size_t column(const char *name) const {
    auto it = std::find(header.begin(), header.end(), name);
    return (size_t)(it - header.begin());
  }
};

IndexTable ReadIndexTable(const std::string &path) {
  IndexTable table;
  std::string content = ReadFile(path.c_str());
  if (content.compare(0, 3, "\xEF\xBB\xBF") == 0) content.erase(0, 3);
  size_t pos = 0;
  while (pos < content.size()) {
    size_t end = content.find('\n', pos);
    if (end == std::string::npos) end = content.size();
    std::string line = content.substr(pos, end - pos);
    if (!line.empty() && line.back() == '\r') line.pop_back();
    std::vector<std::string> cells;
    size_t cell = 0;
    for (;;) {
      size_t tab = line.find('\t', cell);
      if (tab == std::string::npos) { cells.push_back(line.substr(cell)); break; }
      cells.push_back(line.substr(cell, tab - cell));
      cell = tab + 1;
    }
    if (table.header.empty()) table.header = cells;
    else {
      if (cells.size() != table.header.size()) table.valid = false;
      table.rows.push_back(std::move(cells));
    }
    pos = end + 1;
  }
  if (table.header.empty()) table.valid = false;
  return table;
}

void CheckStructureIndexes(Dumper &d, const char *phase) {
  const std::string dir = d.indexes_.directory;
  IndexTable classes = ReadIndexTable(dir + "\\class_meta.tsv");
  IndexTable methods = ReadIndexTable(dir + "\\rva_map.tsv");
  IndexTable ifaces = ReadIndexTable(dir + "\\interfaces.txt");
  IndexTable bases = ReadIndexTable(dir + "\\all_bases.txt");
  const std::string label = std::string(phase) + ": ";
  Check(classes.valid && methods.valid && ifaces.valid && bases.valid,
        (label + "索引列对齐，包括空 RVA 单元格").c_str());
  if (!classes.valid || !methods.valid || !ifaces.valid || !bases.valid) return;
  Check(classes.rows.size() == d.st.classes && methods.rows.size() == d.st.methods && ifaces.rows.size() == d.st.ifaces,
        (label + "独立文件行数与正文统计一致").c_str());
  std::unordered_map<std::string, std::vector<uint64_t>> counts;
  uint64_t sumFields = 0, sumMethods = 0, sumProps = 0, sumIfaces = 0;
  bool sizesUnverified = true;
  for (const auto &row : classes.rows) {
    counts[row[classes.column("ClassId")]] = {0, 0};
    sumFields += strtoull(row[classes.column("FieldCount")].c_str(), nullptr, 10);
    sumMethods += strtoull(row[classes.column("MethodCount")].c_str(), nullptr, 10);
    sumProps += strtoull(row[classes.column("PropertyCount")].c_str(), nullptr, 10);
    sumIfaces += strtoull(row[classes.column("InterfaceCount")].c_str(), nullptr, 10);
    if (!row[classes.column("InstanceSize")].empty() || row[classes.column("InstanceSizeStatus")].empty()) sizesUnverified = false;
  }
  bool joins = counts.size() == classes.rows.size();
  bool sorted = true;
  bool noRvaSeen = false;
  uint64_t lastRva = 0;
  for (const auto &row : methods.rows) {
    auto it = counts.find(row[methods.column("ClassId")]);
    if (it == counts.end()) joins = false;
    else ++it->second[0];
    const auto &rva = row[methods.column("RVA")];
    if (rva.empty()) noRvaSeen = true;
    else {
      const uint64_t current = strtoull(rva.c_str(), nullptr, 0);
      if (noRvaSeen || current < lastRva) sorted = false;
      lastRva = current;
    }
  }
  for (const auto &row : ifaces.rows) {
    auto it = counts.find(row[ifaces.column("ClassId")]);
    if (it == counts.end()) joins = false;
    else ++it->second[1];
  }
  for (const auto &row : classes.rows) {
    const auto &n = counts[row[classes.column("ClassId")]];
    if (n[0] != strtoull(row[classes.column("MethodCount")].c_str(), nullptr, 10) ||
        n[1] != strtoull(row[classes.column("InterfaceCount")].c_str(), nullptr, 10)) joins = false;
  }
  for (const auto &row : bases.rows) if (!counts.count(row[bases.column("ClassId")])) joins = false;
  Check(joins, (label + "ClassId 唯一且方法/接口/槽位正确关联").c_str());
  Check(sumFields == d.st.fields && sumMethods == d.st.methods && sumProps == d.st.props && sumIfaces == d.st.ifaces,
        (label + "类表记录实际成功导出的成员数量").c_str());
  Check(sorted, (label + "方法索引按 RVA 排序，空/外部地址在后").c_str());
  Check(sizesUnverified, (label + "不把未校准的实例大小当成已确认值").c_str());
  CheckContains(ReadFile((dir + "\\export_manifest.txt").c_str()), "状态：完成\r\n",
                (label + "中文清单在全部写入成功后提交").c_str());
}

void TestStructureIndexEdges() {
  printf("\n########## Phase H: 结构索引边界与正文一致性 ##########\n");
  g_off = kStd;
  Scene scene = BuildScene(false);
  uintptr_t tVoid = MkType(T_VOID, 0);
  uintptr_t tInt = MkType(T_I4, 0), tFloat = MkType(T_R4, 0);
  for (int i = 0; i < 2; ++i) {
    ClassBuilder duplicate("Collision", "Tests", scene.img, TA_PUBLIC);
    duplicate.AddMethod("Overload", tVoid, 0x1100, {i ? tFloat : tInt}, 0x0006, 0xFFFF, 0x06001000 + i);
    duplicate.AddMethod("Overload", tVoid, 0x1100, {}, 0x0006, 0xFFFF, 0x06001010 + i);
    uintptr_t outside = duplicate.AddMethod("Outside", tVoid, 1, {}, 0x0006, 0xFFFF, 0x06001020 + i);
    *(uintptr_t *)(outside + g_off.mPtr) = scene.moduleBase + scene.env.moduleSize + 0x1000;
    duplicate.Commit();
    *(uintptr_t *)(duplicate.k + 0x20) = duplicate.k;
    *(uint32_t *)(duplicate.k + 0x28) = (uint32_t)T_CLASS << 16;
    *(uint32_t *)(duplicate.k + g_off.cToken) = 0x02000000u | (uint32_t)(0x101 + scene.classes.size());
    scene.classes.push_back(duplicate.k);
  }
  g_classes = scene.classes;
  *(uint32_t *)(scene.img + 0x1C) = (uint32_t)scene.classes.size();
  const uintptr_t secondSlot = Alloc(8, 8);
  *(uintptr_t *)secondSlot = scene.buffManagerKlass;
  LogCtx log; log.fn = LogPrint;
  Options original; original.outPath = "selftest_indexes_before.cs";
  Dumper baseline; baseline.env = &scene.env; baseline.opt = &original; baseline.log = &log;
  Check(baseline.run(), "H: 正文基线生成成功");
  Options opt = original; opt.outPath = "selftest_indexes.cs"; opt.structureIndexes = true;
  Dumper d; d.env = &scene.env; d.opt = &opt; d.log = &log;
  Check(d.run(), "H: 带索引导出成功");
  Check(ReadFile(original.outPath.c_str()) == ReadFile(opt.outPath.c_str()), "H: 开启索引后 dump 正文逐字节不变");
  CheckStructureIndexes(d, "H");
  const auto methods = ReadIndexTable(d.indexes_.directory + "\\rva_map.tsv");
  size_t shared = 0, external = 0;
  std::unordered_set<std::string> classIds, signatures;
  for (const auto &r : methods.rows) {
    if (r[methods.column("Method")] == "Overload") {
      ++shared; classIds.insert(r[methods.column("ClassId")]); signatures.insert(r[methods.column("Signature")]);
    }
    if (r[methods.column("Method")] == "Outside" && r[methods.column("RVA")].empty() &&
        r[methods.column("AddressStatus")] == "模块外地址，仅保留运行时VA") ++external;
  }
  Check(shared == 4 && classIds.size() == 2 && signatures.size() == 3,
        "H: 同名类、重载和共享 RVA 均保留，不按名字或地址去重");
  Check(external == 2, "H: 模块外指针不伪装成目标模块 RVA");
  const auto bases = ReadIndexTable(d.indexes_.directory + "\\all_bases.txt");
  bool firstFound = false, secondFound = false;
  for (const auto &r : bases.rows) {
    if (r[bases.column("SlotVA")] == IndexHex(scene.buffManagerSlot)) firstFound = true;
    if (r[bases.column("SlotVA")] == IndexHex(secondSlot)) secondFound = true;
  }
  Check(firstFound && secondFound, "H: 同一类的多个真实槽位都已导出");

  Options limited = opt; limited.outPath = "selftest_indexes_limited.cs";
  limited.methods = false; limited.fields = false; limited.maxClassesPerImage = 2;
  Dumper partial; partial.env = &scene.env; partial.opt = &limited; partial.log = &log;
  Check(partial.run(), "H: 配置限制下索引仍正常完成");
  CheckStructureIndexes(partial, "H-limited");
  Check(partial.indexes_.methodRows == 0 && partial.indexes_.classRows == 2, "H: 索引遵守方法开关和类数量上限");

  Dumper collide; collide.env = &scene.env; collide.opt = &limited; collide.log = &log;
  const std::string previousManifest = ReadFile((partial.indexes_.directory + "\\export_manifest.txt").c_str());
  Check(collide.run() && collide.indexes_.directory != partial.indexes_.directory &&
        previousManifest == ReadFile((partial.indexes_.directory + "\\export_manifest.txt").c_str()),
        "H: 同名目录自动避让，已有索引不覆盖");

  StructureIndexes locked;
  Check(locked.begin("selftest_indexes_locked.cs", "Test.dll", scene.moduleBase, 4096, false), "H: 创建清单提交失败测试");
  const std::string manifestPath = locked.directory + "\\export_manifest.txt";
  HANDLE handle = CreateFileA(manifestPath.c_str(), GENERIC_READ, FILE_SHARE_READ, nullptr, OPEN_EXISTING, 0, nullptr);
  Check(handle != INVALID_HANDLE_VALUE, "H: 锁定旧清单以模拟最终提交失败");
  Dumper empty; empty.log = &log;
  Check(!locked.finish(empty, true), "H: 清单无法提交时明确返回失败");
  CheckContains(ReadFile(manifestPath.c_str()), "状态：正在生成", "H: 提交失败不遗留完成标记");
  if (handle != INVALID_HANDLE_VALUE) CloseHandle(handle);
}

void TestQualityRepairs() {
  printf("\n########## Phase I: v9 实机问题回归 ##########\n");
  Check(NameLooksValid("中文接口", false) && NameLooksValid("I接口", false), "I: 保留合法中文名称");
  const char *invalid[] = {"NP\x83", "E\xFCx", "\xC0\xAF", "\xED\xA0\x80", "\xF4\x90\x80\x80", "\xE4\xB8"};
  bool rejected = true;
  for (const char *name : invalid) rejected = rejected && !ValidUtf8(name) && !NameLooksValid(name, false);
  Check(rejected && ValidUtf8("接口\xF0\x9F\x98\x80"), "I: 拒绝非法、过长、代理和截断UTF-8，保留合法四字节字符");
  Writer textWriter;
  Check(textWriter.open("selftest_utf8.cs"), "I: 创建编码回归文件");
  const std::string longName(6000, 'N');
  textWriter.putf("%s中文\n", longName.c_str());
  textWriter.puts("NP\x83");
  Check(textWriter.close(), "I: 编码回归文件完整关闭");
  const std::string text = ReadFile("selftest_utf8.cs");
  Check(text == longName + "中文\nNP\\x83" && textWriter.escapedBytes == 1 && ValidUtf8(text.c_str()),
        "I: 长格式串不截断，非法字节可追溯转义，输出严格UTF-8");

  g_off = kStd;
  Scene scene = BuildScene(false);
  const uintptr_t tInt = MkType(T_I4, 0);
  ClassBuilder large("LargeGeneratedType", "Tests", scene.img, TA_PUBLIC);
  for (unsigned i = 0; i < 32760; ++i) large.AddField(("Field" + std::to_string(i)).c_str(), tInt, 0x10 + i * 4, 6);
  uintptr_t getter = 0;
  for (unsigned i = 0; i < 26374; ++i) {
    const uintptr_t method = large.AddMethod(("Method" + std::to_string(i)).c_str(), tInt, 0x1100, {}, 6, 0xFFFF, 0x06000001 + i);
    if (!i) getter = method;
  }
  for (unsigned i = 0; i < 5340; ++i) large.AddProp(("Property" + std::to_string(i)).c_str(), getter, 0);
  large.Commit();
  ClassBuilder iface("I合法接口", "Tests", scene.img, TA_PUBLIC | TA_INTERFACE | TA_ABSTRACT);
  ClassBuilder iface2("ISecond", "Tests", scene.img, TA_PUBLIC | TA_INTERFACE | TA_ABSTRACT);
  ClassBuilder ordinary("DisplayClass", "Tests", scene.img, TA_PUBLIC);
  ClassBuilder badName("NP\x83", "Tests", scene.img, TA_PUBLIC | TA_INTERFACE | TA_ABSTRACT);
  ClassBuilder owner("MixedInterfaces", "Tests", scene.img, TA_PUBLIC);
  owner.AddIface(iface.k); owner.AddIface(ordinary.k); owner.AddIface(badName.k);
  owner.AddIface(iface.k); owner.AddIface(0); owner.AddIface(iface2.k); owner.Commit();
  ClassBuilder empty("UninitializedMembers", "Tests", scene.img, TA_PUBLIC);
  *(uint16_t *)(empty.k + g_off.cFCount) = 32760;
  ClassBuilder badMembers("MixedMembers", "Tests", scene.img, TA_PUBLIC);
  badMembers.AddField("Good", tInt, 0x10, 6);
  badMembers.AddField("WrongOwner", tInt, 0x14, 6);
  *(uintptr_t *)(badMembers.fields.back() + 0x10) = ordinary.k;
  badMembers.AddField("NP\x83", tInt, 0x18, 6);
  badMembers.AddField("BadType", tInt, 0x1C, 6);
  *(uintptr_t *)(badMembers.fields.back() + g_off.fType) = 0;
  badMembers.AddField("GoodTail", tInt, 0x20, 6);
  badMembers.AddProp("NoAccessor", 0, 0);
  badMembers.Commit();

  Options opt; opt.outPath = "selftest_quality.cs"; opt.structureIndexes = true; opt.useMetadata = false;
  LogCtx log; log.fn = LogPrint;
  Dumper d; d.env = &scene.env; d.opt = &opt; d.log = &log;
  Check(d.indexes_.begin(opt.outPath, "Test.dll", scene.moduleBase, scene.env.moduleSize, false), "I: 创建质量诊断索引");
  Writer w; Check(w.open(opt.outPath.c_str()), "I: 创建质量回归正文");
  d.writeClass(w, large.k, 0);
  Check(d.st.fields == 32760 && d.st.methods == 26374 && d.st.props == 5340, "I: 大型生成类完整越过4096上限");
  d.writeClass(w, owner.k, 1);
  d.writeClass(w, empty.k, 2);
  d.writeClass(w, badMembers.k, 3);
  const bool closed = w.close();
  d.quality.outputEscapedBytes += w.escapedBytes;
  Check(closed && d.indexes_.finish(d, closed), "I: 含拒绝条目的文件仍完整落盘");
  const std::string body = ReadFile(opt.outPath.c_str());
  CheckContains(body, "Field32759;", "I: 最后一个大型字段存在");
  CheckContains(body, "Method26373()", "I: 最后一个大型方法存在");
  CheckContains(body, "Property5339 { get; }", "I: 最后一个大型属性存在");
  CheckContains(body, "MixedInterfaces : I合法接口, ISecond", "I: 仅导出真实接口，并保留无效项之后的合法接口");
  Check(body.find("DisplayClass") == std::string::npos && body.find("NP") == std::string::npos &&
        body.find("WrongOwner;") == std::string::npos && body.find("BadType;") == std::string::npos,
        "I: 非接口、非法名称和错属成员不污染正文");
  CheckContains(body, "GoodTail;", "I: 中间坏成员不导致后续有效成员丢失");
  const auto diagnostic = ReadIndexTable(d.indexes_.directory + "\\member_diagnostics.tsv");
  bool largeField = false, largeMethod = false, largeProp = false, missing = false, rejectedIface = false, badField = false;
  for (const auto &r : diagnostic.rows) {
    auto cell = [&](const char *key) -> const std::string & { return r[diagnostic.column(key)]; };
    if (cell("Class") == "LargeGeneratedType") {
      largeField |= cell("Category") == "字段" && cell("Exported") == "32760";
      largeMethod |= cell("Category") == "方法" && cell("Exported") == "26374";
      largeProp |= cell("Category") == "属性" && cell("Exported") == "5340";
    }
    if (cell("Class") == "UninitializedMembers") missing = cell("NullArray") == "32760" && cell("Attempted") == "0";
    if (cell("Class") == "MixedInterfaces") rejectedIface = cell("Exported") == "2" && cell("NotInterface") == "1" &&
        cell("InvalidUtf8") == "1" && cell("Unreadable") == "1" && cell("Filtered") == "1";
    if (cell("Class") == "MixedMembers" && cell("Category") == "字段") badField = cell("Exported") == "2" &&
        cell("OwnerMismatch") == "1" && cell("InvalidUtf8") == "1" && cell("InvalidType") == "1";
  }
  Check(diagnostic.valid && largeField && largeMethod && largeProp, "I: 大型类型诊断计数与全部输出一致");
  Check(missing && rejectedIface && badField, "I: 空数组、错误接口和错误字段按原因精确计数");
  CheckContains(ReadFile((d.indexes_.directory + "\\member_diagnostics.tsv").c_str()), "NP\\x83",
                "I: 即使首个错误不是编码错误，也保留非法UTF-8的原始字节证据");
  CheckContains(ReadFile((d.indexes_.directory + "\\export_manifest.txt").c_str()), "状态：文件已生成，存在数据质量提示", "I: 质量异常不能标记为无条件完成");
  const auto interfaceTable = ReadIndexTable(d.indexes_.directory + "\\interfaces.txt");
  Check(interfaceTable.valid && interfaceTable.rows.size() == 2 && d.quality.warnings(), "I: 接口索引仅保留两条有效关系");
  bool allUtf8 = true;
  for (const char *file : {"class_meta.tsv", "rva_map.tsv", "interfaces.txt", "all_bases.txt", "member_diagnostics.tsv", "export_manifest.txt"}) {
    const auto data = ReadFile((d.indexes_.directory + "\\" + file).c_str());
    allUtf8 = allUtf8 && !data.empty() && ValidUtf8(data.data(), data.size());
  }
  Check(allUtf8 && ValidUtf8(body.c_str()), "I: 含真实坏字节的场景下所有产物均为严格UTF-8");

  Options limited = opt; limited.maxFields = limited.maxMethods = limited.maxProps = 4096;
  Dumper cap; cap.env = &scene.env; cap.opt = &limited; cap.log = &log;
  Writer cw; cw.open("selftest_quality_limited.cs"); cap.writeClass(cw, large.k, 0); cw.close();
  Check(cap.st.fields == 4096 && cap.st.methods == 4096 && cap.st.props == 4096 &&
        cap.quality.limited == (32760 + 26374 + 5340 - 3 * 4096), "I: 显式上限仍生效且逐项报告未遍历数量");

  // Reproduce a nested-type count that was mistaken for interface_count.
  Dumper refine; refine.env = &scene.env; refine.opt = &opt; refine.log = &log;
  refine.lay.cIfaceCount = 0x118; refine.lay.scoreIfaces = 12;
  for (unsigned i = 0; i < 12; ++i) {
    ClassBuilder sample(("InterfaceCountProbe" + std::to_string(i)).c_str(), "Tests", scene.img, TA_PUBLIC);
    const unsigned count = 1 + i % 2;
    const uintptr_t arr = Alloc((count + 1) * 8, 8);
    *(uintptr_t *)arr = iface.k;
    if (count == 2) *(uintptr_t *)(arr + 8) = iface2.k;
    *(uintptr_t *)(sample.k + g_off.cIfaces) = arr;
    *(uint16_t *)(sample.k + 0x118) = (uint16_t)(6 + i % 5);
    *(uint16_t *)(sample.k + g_off.cICount) = (uint16_t)count;
    refine.typeTable_.push_back(sample.k);
  }
  refine.refineInterfaceCount();
  Check(refine.lay.cIfaceCount == g_off.cICount, "I: 扩展结构证据纠正错误的嵌套类型计数偏移");
  refine.typeTable_.resize(4); refine.lay.cIfaceCount = 0x118;
  refine.refineInterfaceCount();
  Check(refine.lay.cIfaceCount == 0x118, "I: 证据不足时不臆测新布局");

  uintptr_t pages = (uintptr_t)VirtualAlloc(nullptr, 8192, MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);
  Check(pages != 0, "I: 分配跨保护区读取场景");
  if (pages) {
    memcpy((void *)(pages + 4093), "AB中文", sizeof("AB中文"));
    DWORD old = 0;
    Check(VirtualProtect((void *)(pages + 4096), 4096, PAGE_READONLY, &old) != 0, "I: 建立相邻可读的不同保护区");
    MemReader reader; char name[64] = {};
    Check(reader.probe(pages, 8) && reader.rdStr(pages + 4093, name, sizeof(name)) && !strcmp(name, "AB中文"),
          "I: 已缓存区域尾部跨入另一可读页的名称不再丢失");
    Check(VirtualProtect((void *)(pages + 4096), 4096, PAGE_NOACCESS, &old) != 0, "I: 建立不可读边界");
    MemReader guarded;
    Check(!guarded.probe(pages + 4093, 12) && !guarded.rdStr(pages + 4093, name, sizeof(name)), "I: 不可读边界拒绝不完整字符串");
    VirtualFree((void *)pages, 0, MEM_RELEASE);
  }
  MemReader names; char shortName[4] = {};
  Check(!names.rdStr(CStr("NoTerminatorWithinCap"), shortName, sizeof(shortName)), "I: 缓冲区内未找到终止符不能当成完整名称");
  char unset[8] = "old";
  Check(!d.klassName(0, unset, sizeof(unset)) && unset[0] == 0, "I: 空类型读取不泄漏未初始化名称");
}

int main() {
  printf("=== DumpSelfTest: 合成 IL2CPP 结构 -> dump.cs 格式校验 ===\n");

  // ===================== Phase A：标准布局 =====================
  printf("\n########## Phase A: 标准 2019.4 布局 ##########\n");
  g_off = kStd;
  Scene a = BuildScene(false);
  Options optA;
  optA.outPath = "selftest_dump.cs";
  optA.structureIndexes = true;
  optA.useMetadata = true;
  Dumper dA;
  dA.env = &a.env;
  dA.opt = &optA;
  LogCtx logA;
  logA.fn = LogPrint;
  dA.log = &logA;

  bool okA = dA.run();
  Check(okA, "A: engine run() 成功");
  printf("  stats: classes=%llu fields=%llu methods=%llu props=%llu ifaces=%llu "
         "metaParams=%llu synthParams=%llu\n",
         (unsigned long long)dA.st.classes, (unsigned long long)dA.st.fields,
         (unsigned long long)dA.st.methods, (unsigned long long)dA.st.props,
         (unsigned long long)dA.st.ifaces, (unsigned long long)dA.st.metaParams,
         (unsigned long long)dA.st.synthParams);
  std::string dumpA = ReadFile(optA.outPath.c_str());
  Check(!dumpA.empty(), "A: dump.cs 已生成");
  Check(dA.lay.cFlags == 0x108, "A: flags 校准 = 0x108");
  Check(dA.lay.cMethodCount == 0x110, "A: method_count 校准 = 0x110");
  Check(dA.lay.cFieldCount == 0x114, "A: field_count 校准 = 0x114");
  Check(dA.lay.mToken == 0x38, "A: MethodInfo token 校准 = 0x38");
  Check(dA.typeDataIsIndex_ == false, "A: data 语义 = 指针");
  CheckContains(dumpA, "// Image 0: Test.dll - 256\r\n", "A: 镜像头部行");
  CheckContains(dumpA, "// Class: <Module> \r\n// Namespace: \r\n", "A: <Module> 头部（尾随空格）");
  Check(dA.st.classRvas == 3, "A: 类级 RVA 输出数量正确");
  CheckContains(dumpA, "// RVA: 0x", "A: 类级 RVA 十六进制格式");
  CheckContains(dumpA, "// RVA: ", "A: 类级 RVA 十进制格式");
  Check(dumpA.find("// ===== Runtime TypeInfo RVA Discovery =====\r\n") == 0,
        "A: manager RVA 摘要位于文件第一行");
  CheckContains(dumpA,
                ManagerRvaLine("m_BuffManager", a.moduleBase, a.buffManagerSlot, "VERIFIED"),
                "A: BuffManager RVA 十六进制/十进制及实例链验证");
  CheckContains(dumpA,
                ManagerRvaLine("m_TransparentTagHandler", a.moduleBase,
                               a.transparentTagHandlerSlot, "VERIFIED"),
                "A: TransparentTagHandler RVA 十六进制/十进制及实例链验证");
  Check(dumpA.find("m_TransparentTagHandler") < dumpA.find("// Image 0:"),
        "A: 两个结果均位于 Image 列表之前");
  CheckContains(dumpA, "internal class <Module> // TypeDefIndex: 100\r\n{}\r\n", "A: 空类 {} 形式");
  CheckContains(dumpA, "public sealed class Sample : BaseClass, IFoo, IBar // TypeDefIndex: 101\r\n",
                "A: 类声明（基类+接口）");
  CheckContains(dumpA, "\tpublic Int32 m_count; // offset:0x10\r\n", "A: 实例字段+偏移");
  CheckContains(dumpA, "\tprivate static readonly Boolean s_flag; // offset:0x0\r\n", "A: 静态只读字段");
  CheckContains(dumpA, "\tpublic const String Name; // size:0x8\r\n", "A: const 字段 size");
  CheckContains(dumpA, "\tpublic Int32 Count { get; set; }\r\n", "A: 属性 get/set");
  CheckContains(dumpA, RvaPrefix(a.moduleBase, 0x1030, -1) +
                           "\tpublic Void DoWork(Int32 a, ref Single b, out Int32 c) { }\r\n",
                "A: 参数名/ref/out");
  CheckContains(dumpA, RvaPrefix(a.moduleBase, 0x1040, 4) +
                           "\tpublic virtual Void Virtual() { }\r\n",
                "A: virtual + Slot");
  CheckContains(dumpA, "\t// RVA: 0x VA: 0x0\r\n\tpublic Void NoPointer() { }\r\n",
                "A: 空方法指针 RVA 格式");
  CheckContains(dumpA, "public enum MyEnum // TypeDefIndex:", "A: enum 判定");
  CheckContains(dumpA, "\tpublic const MyEnum ValueA; // size:0x4\r\n", "A: 枚举 const size:0x4");
  CheckContains(dumpA, "public struct MyStruct : IFoo, IBar // TypeDefIndex:", "A: struct + 接口");
  CheckContains(dumpA, "public interface IFoo // TypeDefIndex:", "A: interface 判定");
  CheckContains(dumpA, "\tpublic Byte[] data; // offset:0x18\r\n", "A: SZARRAY 类型名");
  CheckContains(dumpA, "\tpublic Int32[,] grid; // offset:0x20\r\n", "A: 多维数组类型名");
  CheckContains(dumpA, "\tpublic BaseClass boxed; // offset:0x28\r\n", "A: GENERICINST 名称");
  Check(dA.st.classes == a.classes.size(), "A: 类计数正确");
  CheckStructureIndexes(dA, "A");

  // ===================== Phase B：变形布局 =====================
  printf("\n########## Phase B: 变形布局（+8 位移 / MethodInfo 双指针 / 索引型 data）##########\n");
  g_off = kDeform;
  Scene b = BuildScene(true);
  Options optB;
  optB.outPath = "selftest_dump_deform.cs";
  optB.structureIndexes = true;
  optB.useMetadata = true;
  Dumper dB;
  dB.env = &b.env;
  dB.opt = &optB;
  LogCtx logB;
  logB.fn = LogPrint;
  dB.log = &logB;

  bool okB = dB.run();
  Check(okB, "B: engine run() 成功");
  printf("  stats: classes=%llu fields=%llu methods=%llu props=%llu ifaces=%llu\n",
         (unsigned long long)dB.st.classes, (unsigned long long)dB.st.fields,
         (unsigned long long)dB.st.methods, (unsigned long long)dB.st.props,
         (unsigned long long)dB.st.ifaces);
  std::string dumpB = ReadFile(optB.outPath.c_str());
  Check(!dumpB.empty(), "B: dump.cs 已生成");

  // 校准必须自动识别变形偏移
  Check(dB.lay.cFlags == 0x110, "B: flags 自动校准 = 0x110（+8 位移）",
        "got 0x" + std::to_string(dB.lay.cFlags));
  Check(dB.lay.cToken == 0x114, "B: token 自动校准 = 0x114");
  Check(dB.lay.cMethodCount == 0x118, "B: method_count 自动校准 = 0x118");
  Check(dB.lay.cPropCount == 0x11A, "B: property_count 自动校准 = 0x11A");
  Check(dB.lay.cFieldCount == 0x11C, "B: field_count 自动校准 = 0x11C");
  Check(dB.lay.cIfaceCount == 0x124, "B: iface_count 自动校准 = 0x124");
  Check(dB.lay.scoreIfaces >= 2, "B: 接口数组校准通过");
  Check(dB.lay.mToken == 0x40 && dB.lay.mFlags == 0x44 && dB.lay.mSlot == 0x48 &&
            dB.lay.mParamCount == 0x4A,
        "B: MethodInfo 尾部自动校准 = 0x40/0x44/0x48/0x4A");
  Check(dB.typeDataIsIndex_ == true, "B: data 语义识别为 index");

  // 输出必须仍然正确
  CheckContains(dumpB, "public sealed class Sample : BaseClass, IFoo, IBar // TypeDefIndex: 101\r\n",
                "B: 类修饰符正确（不再全部 internal）");
  CheckContains(dumpB, "[Serializable]\r\n", "B: [Serializable] 正确输出");
  CheckContains(dumpB, "\tpublic Int32 m_count; // offset:0x10\r\n", "B: 字段偏移正确");
  CheckContains(dumpB, "\tpublic BaseClass indexed; // offset:0x30\r\n",
                "B: 索引型 data 解析出类名（修复 Class 退化）");
  CheckContains(dumpB, "\tpublic const MyEnum ValueA; // size:0x4\r\n", "B: 索引型枚举 const size");
  CheckContains(dumpB, "public enum MyEnum // TypeDefIndex:", "B: enum 判定");
  CheckContains(dumpB, "public struct MyStruct : IFoo, IBar // TypeDefIndex:", "B: struct + 接口（计数已校准）");
  CheckContains(dumpB, "\tpublic Int32 Count { get; set; }\r\n", "B: 属性正确");
  CheckContains(dumpB, RvaPrefix(b.moduleBase, 0x1030, -1) +
                           "\tpublic Void DoWork(Int32 a, ref Single b, out Int32 c) { }\r\n",
                "B: 方法尾部校准后签名正确");
  Check(dB.st.classes == b.classes.size(), "B: 类计数正确");

  // ===================== Phase C：Super 无导出引导 =====================
  printf("\n########## Phase C: Super 无导出引导（全内存扫描）##########\n");
  g_off = kDeform;  // 用变形布局模拟真实 Super 构建
  Scene c = BuildScene(false);
  c.env.moduleName = "GameAssembly_Super.dll";
  c.env.superVariant = true;
  c.env.getCorlib = nullptr;
  c.env.imageGetClass = nullptr;
  c.env.imageGetClassCount = nullptr;
  c.env.imageGetName = nullptr;
  c.env.overrideImages.clear();

  Options optC;
  optC.outPath = "selftest_dump_super.cs";
  optC.structureIndexes = true;
  optC.useMetadata = true;
  Dumper dC;
  dC.env = &c.env;
  dC.opt = &optC;
  LogCtx logC;
  logC.fn = LogPrint;
  dC.log = &logC;

  bool okC = dC.run();
  Check(okC, "C: Super 路径 run() 成功");
  Check(dC.superMode_, "C: 进入 super（无导出）模式");
  Check(dC.klasses_.size() >= c.classes.size(), "C: 全内存扫描收集到全部类",
        "got " + std::to_string(dC.klasses_.size()));
  Check(!dC.typeTable_.empty(), "C: token 索引表已建立");
  std::string dumpC = ReadFile(optC.outPath.c_str());
  Check(!dumpC.empty(), "C: dump.cs 已生成");
  CheckContains(dumpC, "// Image 0: Test.dll - 256\r\n", "C: 镜像由 klass->image 推导正确");
  CheckContains(dumpC, "public sealed class Sample : BaseClass, IFoo, IBar // TypeDefIndex: 101\r\n",
                "C: 类修饰符正确");
  CheckContains(dumpC, "\tpublic Int32 m_count; // offset:0x10\r\n", "C: 字段偏移正确");
  CheckContains(dumpC, "\tpublic BaseClass indexed; // offset:0x30\r\n", "C: 索引型 data 解析");
  CheckContains(dumpC, "\t// Methods:6\r\n", "C: 方法列表完整");
  CheckContains(dumpC, "\tpublic Int32 Count { get; set; }\r\n", "C: 属性完整");
  Check(dC.st.classes == c.classes.size(), "C: 类计数正确",
        "got " + std::to_string(dC.st.classes) + " want " + std::to_string(c.classes.size()));

  // ===================== Phase D：Super 兼容导出 + 交换镜像头 =====================
  printf("\n########## Phase D: Super 兼容导出（全镜像扫描 + 交换 start/count）##########\n");
  g_off = kDeform;
  Scene d = BuildScene(false);
  d.env.moduleName = "GameAssembly_Super.dll";
  d.env.superVariant = true;
  d.env.overrideClassPointerSlots.erase(
      std::remove_if(d.env.overrideClassPointerSlots.begin(),
                     d.env.overrideClassPointerSlots.end(),
                     [&](const std::pair<uintptr_t, uintptr_t> &entry) {
                       return entry.first == d.transparentTagHandlerKlass;
                     }),
      d.env.overrideClassPointerSlots.end());
  // 模拟实机日志所揭示的变体：API count 与 +0x18 一致，typeStart 在 +0x1C。
  *(uint32_t *)(d.img + 0x18) = (uint32_t)d.classes.size();
  *(uint32_t *)(d.img + 0x1C) = 0x100;
  d.env.overrideImages.clear();
  d.env.scanAllMemoryForImages = true;

  Options optD;
  optD.outPath = "selftest_dump_super_compat.cs";
  optD.structureIndexes = true;
  optD.useMetadata = true;
  Dumper dD;
  dD.env = &d.env;
  dD.opt = &optD;
  LogCtx logD;
  logD.fn = LogPrint;
  dD.log = &logD;

  bool okD = dD.run();
  Check(okD, "D: Super 兼容导出路径 run() 成功");
  Check(!dD.superMode_, "D: 保留兼容 API 枚举路径");
  Check(dD.images_.size() == 1 && dD.images_[0] == d.img,
        "D: 全内存扫描经 API 交叉验证只保留真实镜像");
  Check(dD.imageLayoutProfile_ == "count@0x18,start@0x1C",
        "D: 自动识别交换后的镜像头布局", dD.imageLayoutProfile_);
  Check(dD.imageCount(d.img) == d.classes.size(), "D: 类数量始终采用兼容 API 真值");
  Check(dD.imageStart(d.img) == 0x100, "D: typeStart 从 +0x1C 恢复");
  std::string dumpD = ReadFile(optD.outPath.c_str());
  CheckContains(dumpD, "// Image 0: Test.dll - 256\r\n", "D: 镜像头 TypeDefIndex 正确");
  CheckContains(dumpD, "public sealed class Sample : BaseClass, IFoo, IBar // TypeDefIndex: 101\r\n",
                "D: 交换镜像头后类内容仍完整");
  CheckContains(dumpD,
                "// m_TransparentTagHandler2 = NOT_FOUND; // reason=slot-not-found",
                "D: 槽未命中时在顶部明确输出 NOT_FOUND");
  Check(dD.st.classes == d.classes.size(), "D: 交换镜像头后类计数正确");

  // ===================== Phase E：实测 Super 新版结构 =====================
  printf("\n########## Phase E: Super 新版结构（无 typeStart + byval 索引 + 新 MethodInfo）##########\n");
  g_off = kSuperIndex;
  Scene e = BuildScene(true);
  e.env.moduleName = "GameAssembly_Super.dll";
  e.env.superVariant = true;
  // 实机镜像头只有 API class count，没有可用的连续 typeStart。
  *(uint32_t *)(e.img + 0x18) = (uint32_t)e.classes.size();
  *(uint32_t *)(e.img + 0x1C) = 0;

  Options optE;
  optE.outPath = "selftest_dump_super_new.cs";
  optE.structureIndexes = true;
  optE.useMetadata = true;
  Dumper dE;
  dE.env = &e.env;
  dE.opt = &optE;
  LogCtx logE;
  logE.fn = LogPrint;
  dE.log = &logE;

  bool okE = dE.run();
  Check(okE, "E: Super 新版结构 run() 成功");
  Check(dE.imageLayoutProfile_.find("klass-byval-index@0x20") == 0,
        "E: 从 klass->byval_arg 重建全局类型索引", dE.imageLayoutProfile_);
  Check(dE.imageStart(e.img) == 0x100, "E: 无 typeStart 时恢复镜像起始索引");
  Check(dE.classIndex(e.classes[1], 0) == 0x101,
        "E: API 类顺序映射到真实 TypeDefinitionIndex");
  Check(dE.lay.cProperties == 0x90, "E: properties 数组校准 = 0x90");
  Check(dE.lay.cMethods == 0x98, "E: methods 数组校准 = 0x98");
  Check(dE.lay.cInterfaces == 0xB0, "E: interfaces 数组校准 = 0xB0");
  Check(dE.lay.cMethodCount == 0x120, "E: method_count 校准 = 0x120");
  Check(dE.lay.cPropCount == 0x122, "E: property_count 校准 = 0x122");
  Check(dE.lay.cFieldCount == 0x124, "E: field_count 校准 = 0x124");
  Check(dE.lay.cIfaceCount == 0x128, "E: iface_count 校准 = 0x128");
  Check(dE.lay.mName == 0x18 && dE.lay.mKlass == 0x20 && dE.lay.mRet == 0x28 &&
            dE.lay.mParams == 0x30,
        "E: 新 MethodInfo 头自动校准");
  Check(dE.lay.mToken == 0x48 && dE.lay.mFlags == 0x4C && dE.lay.mSlot == 0x50 &&
            dE.lay.mParamCount == 0x52,
        "E: 新 MethodInfo 尾自动校准");
  Check(dE.typeDataIsIndex_, "E: 字段类型 data 识别为 index");
  Check(dE.st.methods == 15, "E: 方法没有因头部错位而丢失",
        "got " + std::to_string(dE.st.methods));
  std::string dumpE = ReadFile(optE.outPath.c_str());
  CheckContains(dumpE, "// Image 0: Test.dll - 256\r\n", "E: 镜像头索引正确");
  CheckContains(dumpE, "public sealed class Sample : BaseClass, IFoo, IBar // TypeDefIndex: 101\r\n",
                "E: 类、基类和接口完整");
  CheckContains(dumpE, "\tpublic BaseClass indexed; // offset:0x30\r\n",
                "E: 索引类型恢复为真实类名");
  CheckContains(dumpE, RvaPrefix(e.moduleBase, 0x1030, -1) +
                           "\tpublic Void DoWork(Int32 a, ref Single b, out Int32 c) { }\r\n",
                "E: 新 MethodInfo 方法签名和 RVA 完整");
  CheckContains(dumpE,
                ManagerRvaLine("m_BuffManager2", e.moduleBase, e.buffManagerSlot, "VERIFIED"),
                "E: Super 变体使用 m_BuffManager2 名称");
  CheckContains(dumpE, "staticFieldsOffset=0xC0",
                "E: Super 新布局按 cInterfaces+0x10 定位 static_fields");

  // ===================== Phase F：实测 metadata type handle =====================
  printf("\n########## Phase F: Super metadata type handle（类型反向映射 + 索引排序）##########\n");
  g_off = kSuperHandle;
  Scene f = BuildScene(true);
  f.env.moduleName = "GameAssembly_Super_IBT.dll";
  f.env.superVariant = true;
  *(uintptr_t *)(f.transparentTagHandlerStaticFields + 0x8) = 0;
  *(uint32_t *)(f.img + 0x18) = (uint32_t)f.classes.size();
  *(uint32_t *)(f.img + 0x1C) = 0;

  Options optF;
  optF.outPath = "selftest_dump_super_handle.cs";
  optF.structureIndexes = true;
  optF.useMetadata = false;
  Dumper dF;
  dF.env = &f.env;
  dF.opt = &optF;
  LogCtx logF;
  logF.fn = LogPrint;
  dF.log = &logF;

  bool okF = dF.run();
  Check(okF, "F: metadata type handle run() 成功");
  Check(dF.imageLayoutProfile_.find("klass-type-handle@0x20") == 0,
        "F: 从 klass->byval_arg handle 重建索引", dF.imageLayoutProfile_);
  Check(dF.typeHandleToKlass_.size() == f.classes.size(),
        "F: metadata handle 反向表覆盖全部类");
  Check(dF.imageStart(f.img) == 0, "F: 由 handle 地址顺序恢复镜像起始索引");
  Check(dF.classIndex(f.classes[1], 99) == 1,
        "F: 由 handle 地址顺序恢复 TypeDefinitionIndex");
  Check(dF.typeDataIsHandle_, "F: 字段类型 data 识别为 handle");
  Check(dF.st.classTypeFallbacks == 0, "F: Class 类型占位完全消除",
        "got " + std::to_string(dF.st.classTypeFallbacks));
  Check(dF.st.methods == 15, "F: handle 模式下方法保持完整");
  std::string dumpF = ReadFile(optF.outPath.c_str());
  CheckContains(dumpF, "// Image 0: Test.dll - 0\r\n", "F: handle 顺序镜像头正确");
  CheckContains(dumpF, "public sealed class Sample : BaseClass, IFoo, IBar // TypeDefIndex: 1\r\n",
                "F: handle 模式类、基类和接口完整");
  CheckContains(dumpF, "\tpublic BaseClass indexed; // offset:0x30\r\n",
                "F: metadata handle 解析为真实类名");
  CheckContains(dumpF, RvaPrefix(f.moduleBase, 0x1030, -1) +
                           "\tpublic Void DoWork(Int32 p0, ref Single p1, out Int32 p2) { }\r\n",
                "F: handle 模式方法类型和 RVA 完整");
  CheckContains(dumpF,
                ManagerRvaLine("m_TransparentTagHandler2", f.moduleBase,
                               f.transparentTagHandlerSlot, "CLASS_ONLY"),
                "F: 实例未初始化时保留 RVA 并标记 CLASS_ONLY");

  Check(dumpF.find("Offset.h Discovery Summary") < dumpF.find("// Image 0:"),
        "F: v7 summary appears before image list in a full dump");
  TestOffsetReport();
  CheckStructureIndexes(dB, "B");
  CheckStructureIndexes(dC, "C");
  CheckStructureIndexes(dD, "D");
  CheckStructureIndexes(dE, "E");
  CheckStructureIndexes(dF, "F");
  TestStructureIndexEdges();
  TestQualityRepairs();
  printf("\n=== 结果: pass=%d fail=%d ===\n", g_pass, g_fail);
  return g_fail ? 1 : 0;
}
