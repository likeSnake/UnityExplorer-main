// ============================================================================
// il2cpp_dump_core.hpp
// 自研 in-memory IL2CPP 结构游走 dumper 引擎（不依赖被裁剪的反射 getter）。
//
// 目标：复刻 F:\gua\dump\dump2026.09.03.cs（Unity-Offset.dll 历史产物）的
//       输出格式与完整度：
//         // Image <idx>: <name> - <typeStart>
//         // Class: <name>
//         // Namespace: <ns>
//         // Instance: 0x<klass>
//         // RVA: 0x<class pointer slot rva>
//         // RVA: <class pointer slot rva in decimal>
//         [Serializable]
//         <mods><kind> <name> : <base>, <iface...> // TypeDefIndex: <hex>
//         {
//         	// Fields
//         	<mods><type> <name>; // offset:0x..   (literal: // size:0x..)
//         	// Properties
//         	<mods><type> <name> { get; set; }
//         	// Methods:N
//         	// RVA: 0x.. VA: 0x.. Slot: N
//         	<mods><ret> <name>(<type> <pname>, ...) { }
//         }
//
// 依赖：只使用本构建确认导出的 4 个函数
//   il2cpp_get_corlib / il2cpp_image_get_class(_count) / il2cpp_image_get_name
// 其余全部为内存结构直接偏移读取，偏移由运行时自校准（Calibrate）得出。
// ============================================================================
#ifndef ILDUMP_CORE_HPP
#define ILDUMP_CORE_HPP

#include <windows.h>
#include <cstdint>
#include <cstdio>
#include <cstdarg>
#include <cstring>
#include <cstdlib>
#include <string>
#include <vector>
#include <unordered_map>
#include <unordered_set>
#include <algorithm>
#include <utility>
#include "structure_indexes.hpp"

namespace ildump {

// ---------------------------------------------------------------------------
// 基础工具
// ---------------------------------------------------------------------------
typedef void (*LogFn)(void *ud, const char *msg);

struct LogCtx {
  LogFn fn = nullptr;
  void *ud = nullptr;
  FILE *f = nullptr;
  void line(const char *fmt, ...) {
    char tmp[2048];
    va_list ap;
    va_start(ap, fmt);
    vsnprintf(tmp, sizeof(tmp), fmt, ap);
    va_end(ap);
    const std::string clean = EscapeInvalidUtf8(tmp, strlen(tmp));
    if (fn) fn(ud, clean.c_str());
    if (f) {
      fputs(clean.c_str(), f);
      fputc('\n', f);
      fflush(f);
    }
  }
};

// 无 C++ 对象的裸拷贝，用 SEH 兜底（__try 不能出现在有析构对象的函数里）
__declspec(noinline) inline bool SafeCopy(void *dst, const void *src, size_t len) {
  __try {
    memcpy(dst, src, len);
    return true;
  } __except (EXCEPTION_EXECUTE_HANDLER) {
    return false;
  }
}

// Export calls used during discovery also need a narrow SEH boundary. A
// structural false positive must reject only that candidate, not abort the
// entire dump before the real image table is reached.
__declspec(noinline) inline bool SafeCallGetCorlib(void *(*fn)(), uintptr_t &out) {
  out = 0;
  if (!fn) return false;
  __try {
    out = (uintptr_t)fn();
    return true;
  } __except (EXCEPTION_EXECUTE_HANDLER) {
    out = 0;
    return false;
  }
}

__declspec(noinline) inline bool SafeCallImageGetName(const char *(*fn)(void *), uintptr_t image,
                                                       uintptr_t &out) {
  out = 0;
  if (!fn || !image) return false;
  __try {
    out = (uintptr_t)fn((void *)image);
    return true;
  } __except (EXCEPTION_EXECUTE_HANDLER) {
    out = 0;
    return false;
  }
}

__declspec(noinline) inline bool SafeCallImageGetClassCount(size_t (*fn)(void *), uintptr_t image,
                                                             size_t &out) {
  out = 0;
  if (!fn || !image) return false;
  __try {
    out = fn((void *)image);
    return true;
  } __except (EXCEPTION_EXECUTE_HANDLER) {
    out = 0;
    return false;
  }
}

__declspec(noinline) inline bool SafeCallImageGetClass(void *(*fn)(void *, size_t), uintptr_t image,
                                                        size_t index, uintptr_t &out) {
  out = 0;
  if (!fn || !image) return false;
  __try {
    out = (uintptr_t)fn((void *)image, index);
    return true;
  } __except (EXCEPTION_EXECUTE_HANDLER) {
    out = 0;
    return false;
  }
}

// ---------------------------------------------------------------------------
// 内存读取器（区域缓存 + 安全读）
// ---------------------------------------------------------------------------
class MemReader {
public:
  struct Reg {
    uintptr_t lo, hi;
  };
  std::vector<Reg> regs_;
  uint64_t vqCount_ = 0;

  void reset() {
    regs_.clear();
    vqCount_ = 0;
  }

  void addRegion(uintptr_t lo, uintptr_t hi) {
    if (hi <= lo) return;
    size_t i = 0;
    while (i < regs_.size() && regs_[i].hi < lo) ++i;
    // 与前一个合并
    if (i > 0 && regs_[i - 1].hi >= lo) {
      if (hi > regs_[i - 1].hi) regs_[i - 1].hi = hi;
      while (i < regs_.size() && regs_[i].lo <= regs_[i - 1].hi) {
        if (regs_[i].hi > regs_[i - 1].hi) regs_[i - 1].hi = regs_[i].hi;
        regs_.erase(regs_.begin() + (long)i);
      }
      return;
    }
    if (i < regs_.size() && regs_[i].lo <= hi) {
      if (lo < regs_[i].lo) regs_[i].lo = lo;
      if (hi > regs_[i].hi) regs_[i].hi = hi;
      return;
    }
    regs_.insert(regs_.begin() + (long)i, Reg{lo, hi});
  }

  bool cached(uintptr_t p, size_t len) const {
    if (p < 0x10000 || (len && p + len < p)) return false;
    size_t lo = 0, hi = regs_.size();
    while (lo < hi) {
      size_t mid = (lo + hi) >> 1;
      if (regs_[mid].hi <= p) lo = mid + 1;
      else hi = mid;
    }
    return lo < regs_.size() && p >= regs_[lo].lo && p + len <= regs_[lo].hi;
  }

  bool probe(uintptr_t p, size_t len) {
    if (p < 0x10000) return false;
    if (len == 0) return true;
    if (p + len < p) return false;
    const uintptr_t end = p + len;
    uintptr_t cursor = p;
    while (cursor < end) {
      size_t lo = 0, hi = regs_.size();
      while (lo < hi) {
        const size_t mid = (lo + hi) >> 1;
        if (regs_[mid].hi <= cursor) lo = mid + 1;
        else hi = mid;
      }
      if (lo < regs_.size() && cursor >= regs_[lo].lo) {
        cursor = std::min(end, regs_[lo].hi);
        continue;
      }
      // A record/string can cross adjacent regions with different protection.
      // Query only the previously unknown part, then retain the cached prefix.
      MEMORY_BASIC_INFORMATION mbi = {};
      ++vqCount_;
      if (!VirtualQuery((LPCVOID)cursor, &mbi, sizeof(mbi)) || mbi.State != MEM_COMMIT ||
          (mbi.Protect & (PAGE_GUARD | PAGE_NOACCESS))) return false;
      const uintptr_t rlo = (uintptr_t)mbi.BaseAddress;
      const uintptr_t rhi = rlo + mbi.RegionSize;
      if (rhi <= cursor || rlo > cursor) return false;
      addRegion(rlo, rhi);
      cursor = std::min(end, rhi);
    }
    return true;
  }

  template <class T>
  bool rd(uintptr_t p, T &out) {
    if (!probe(p, sizeof(T))) return false;
    return SafeCopy(&out, (const void *)p, sizeof(T));
  }
  uintptr_t rdPtr(uintptr_t p) {
    uintptr_t v = 0;
    return rd(p, v) ? v : (uintptr_t)0;
  }
  uint64_t rdU64(uintptr_t p) {
    uint64_t v = 0;
    return rd(p, v) ? v : 0ull;
  }
  uint32_t rdU32(uintptr_t p) {
    uint32_t v = 0;
    return rd(p, v) ? v : 0u;
  }
  uint16_t rdU16(uintptr_t p) {
    uint16_t v = 0;
    return rd(p, v) ? v : (uint16_t)0;
  }
  uint8_t rdU8(uintptr_t p) {
    uint8_t v = 0;
    return rd(p, v) ? v : (uint8_t)0;
  }
  int32_t rdI32(uintptr_t p) {
    int32_t v = 0;
    return rd(p, v) ? v : 0;
  }

  // 读 C 串；asciiOnly=true 时拒绝 >=0x80 的字节（校准用严格模式）
  bool rdStr(uintptr_t p, char *out, size_t cap, bool asciiOnly = false) {
    if (!out || cap < 2) return false;
    out[0] = 0;
    if (!p) return false;
    size_t n = 0;
    size_t window = 128;  // 失败后永久缩小，避免每个字节都重试大窗口
    while (n + 1 < cap) {
      size_t chunk = cap - 1 - n;
      if (chunk > window) chunk = window;
      if (!probe(p + n, chunk)) {
        if (window > 1) {
          window = 1;
          continue;
        }
        break;
      }
      char tmp[128];
      if (!SafeCopy(tmp, (const void *)(p + n), chunk)) break;
      bool stop = false;
      for (size_t i = 0; i < chunk; ++i) {
        unsigned char c = (unsigned char)tmp[i];
        if (c == 0) {
          out[n] = 0;
          return n > 0;
        }
        if (c < 0x20 || c == 0x7F) {
          out[0] = 0;
          return false;
        }
        if (asciiOnly && c > 0x7E) {
          out[0] = 0;
          return false;
        }
        out[n++] = (char)c;
        if (n + 1 >= cap) {
          stop = true;
          break;
        }
      }
      if (stop) break;
    }
    out[n] = 0;
    return false; // A partial or unterminated name is not a complete C string.
  }
};

// 名字合法性（用于校准/结构验证）
inline bool NameLooksValid(const char *s, bool strict) {
  if (!s || !*s) return false;
  size_t n = strlen(s);
  if (n == 0 || n > 512) return false;
  if (!ValidUtf8(s, n)) return false;
  unsigned char c0 = (unsigned char)s[0];
  if (c0 == 0x80 || (c0 > 0x80 && c0 < 0xC0)) return false;  // UTF-8 续字节开头
  if (strstr(s, ".dll") || strstr(s, ".exe") || strchr(s, '\\') || strchr(s, '/') || strchr(s, ':'))
    return false;
  int alnum = 0;
  for (size_t i = 0; i < n; ++i) {
    unsigned char c = (unsigned char)s[i];
    if ((c >= '0' && c <= '9') || (c >= 'A' && c <= 'Z') || (c >= 'a' && c <= 'z') || c == '_') ++alnum;
    else if (c == '<' || c == '>' || c == '.' || c == '`' || c == '$' || c == '@' || c == '-' ||
             c == '[' || c == ']' || c == ',' || c == '+' || c == '|' || c == '&' || c == '*' ||
             c == '=' || c == '(' || c == ')' || c == ' ' || c == '\'' || c == '?' || c == '!')
      ;
    else if (c >= 0x80) {
      if (strict) return false;
      ++alnum;
    } else
      return false;
  }
  return alnum > 0;
}

// ---------------------------------------------------------------------------
// IL2CPP 类型枚举
// ---------------------------------------------------------------------------
enum Il2CppTypeEnum : uint8_t {
  T_END = 0x00,
  T_VOID = 0x01,
  T_BOOLEAN = 0x02,
  T_CHAR = 0x03,
  T_I1 = 0x04,
  T_U1 = 0x05,
  T_I2 = 0x06,
  T_U2 = 0x07,
  T_I4 = 0x08,
  T_U4 = 0x09,
  T_I8 = 0x0A,
  T_U8 = 0x0B,
  T_R4 = 0x0C,
  T_R8 = 0x0D,
  T_STRING = 0x0E,
  T_PTR = 0x0F,
  T_BYREF = 0x10,
  T_VALUETYPE = 0x11,
  T_CLASS = 0x12,
  T_VAR = 0x13,
  T_ARRAY = 0x14,
  T_GENERICINST = 0x15,
  T_TYPEDBYREF = 0x16,
  T_I = 0x18,
  T_U = 0x19,
  T_FNPTR = 0x1B,
  T_OBJECT = 0x1C,
  T_SZARRAY = 0x1D,
  T_MVAR = 0x1E,
  T_CMOD_REQD = 0x1F,
  T_CMOD_OPT = 0x20,
  T_INTERNAL = 0x21,
  T_MODIFIER = 0x40,
  T_SENTINEL = 0x41,
  T_PINNED = 0x45,
  T_ENUM = 0x55,
};

// .NET 标志位
enum {
  // TypeAttributes
  TA_VISIBILITY_MASK = 0x00000007,
  TA_NOT_PUBLIC = 0x00000000,
  TA_PUBLIC = 0x00000001,
  TA_NESTED_PUBLIC = 0x00000002,
  TA_NESTED_PRIVATE = 0x00000003,
  TA_NESTED_FAMILY = 0x00000004,
  TA_NESTED_ASSEMBLY = 0x00000005,
  TA_NESTED_FAM_AND_ASSEM = 0x00000006,
  TA_NESTED_FAM_OR_ASSEM = 0x00000007,
  TA_INTERFACE = 0x00000020,
  TA_ABSTRACT = 0x00000080,
  TA_SEALED = 0x00000100,
  TA_SERIALIZABLE = 0x00002000,
  // FieldAttributes
  FA_FIELD_ACCESS_MASK = 0x0007,
  FA_PRIVATE = 0x0001,
  FA_FAM_AND_ASSEM = 0x0002,
  FA_ASSEMBLY = 0x0003,
  FA_FAMILY = 0x0004,
  FA_FAM_OR_ASSEM = 0x0005,
  FA_PUBLIC = 0x0006,
  FA_STATIC = 0x0010,
  FA_INIT_ONLY = 0x0020,
  FA_LITERAL = 0x0040,
  // MethodAttributes
  MA_MEMBER_ACCESS_MASK = 0x0007,
  MA_PRIVATE = 0x0001,
  MA_FAM_AND_ASSEM = 0x0002,
  MA_ASSEMBLY = 0x0003,
  MA_FAMILY = 0x0004,
  MA_FAM_OR_ASSEM = 0x0005,
  MA_PUBLIC = 0x0006,
  MA_STATIC = 0x0010,
  MA_FINAL = 0x0020,
  MA_VIRTUAL = 0x0040,
  MA_HIDE_BY_SIG = 0x0080,
  MA_NEW_SLOT = 0x0100,
  MA_ABSTRACT = 0x0400,
  MA_SPECIAL_NAME = 0x0800,
  MA_PINVOKE_IMPL = 0x2000,
  // ParamAttributes
  PA_OUT = 0x0002,
};

// ---------------------------------------------------------------------------
// 结构布局（默认值 = Unity 2019.4 / metadata v24.5 标准布局）
// ---------------------------------------------------------------------------
struct Layout {
  // Il2CppClass
  uint32_t cImage = 0x00;
  uint32_t cName = 0x10;
  uint32_t cNamespace = 0x18;
  uint32_t cParent = 0x58;
  uint32_t cTypeDefHandle = 0x68;
  uint32_t cFields = 0x80;
  uint32_t cEvents = 0x88;
  uint32_t cProperties = 0x90;
  uint32_t cMethods = 0x98;
  uint32_t cNestedTypes = 0xA0;
  uint32_t cInterfaces = 0xA8;
  uint32_t cInstanceSize = 0xF0;
  uint32_t cFlags = 0x108;
  uint32_t cToken = 0x10C;
  uint32_t cMethodCount = 0x110;
  uint32_t cPropCount = 0x112;
  uint32_t cFieldCount = 0x114;
  uint32_t cEventCount = 0x116;
  uint32_t cNestedCount = 0x118;
  uint32_t cIfaceCount = 0x11C;
  // FieldInfo
  uint32_t fName = 0x00;
  uint32_t fType = 0x08;
  uint32_t fParent = 0x10;
  uint32_t fOffset = 0x18;
  uint32_t fStride = 0x20;
  // MethodInfo
  uint32_t mPtr = 0x00;
  uint32_t mName = 0x10;
  uint32_t mKlass = 0x18;
  uint32_t mRet = 0x20;
  uint32_t mParams = 0x28;
  uint32_t mToken = 0x38;
  uint32_t mFlags = 0x3C;
  uint32_t mIFlags = 0x3E;
  uint32_t mSlot = 0x40;
  uint32_t mParamCount = 0x42;
  // PropertyInfo
  uint32_t pParent = 0x00;
  uint32_t pName = 0x08;
  uint32_t pGet = 0x10;
  uint32_t pSet = 0x18;
  uint32_t pAttrs = 0x20;
  uint32_t pStride = 0x28;
  // Il2CppType
  uint32_t tData = 0x00;
  uint32_t tBits = 0x08;
  // 校准质量
  int scoreFields = 0, scoreMethods = 0, scoreProps = 0, scoreIfaces = 0;
  int scoreFlags = 0, scoreIfaceCount = 0, scoreParamMode = 0;
  bool calibrated = false;
};

// ---------------------------------------------------------------------------
// 元数据（可选）：仅用于方法参数名（结构体里没有参数名）
// ---------------------------------------------------------------------------
struct Metadata {
  MemReader *mem = nullptr;
  uintptr_t base = 0;
  int32_t stringOff = 0;
  int32_t methodsOff = 0;
  int32_t paramsOff = 0;
  int32_t typeDefsOff = 0;
  int32_t typeDefsCount = 0;
  int32_t methodDefSize = 0x34;
  int32_t paramDefSize = 0x0C;
  int32_t typeDefSize = 0x5C;
  bool ok = false;

  // 用 klass 的 typeMetadataHandle + 类名验证并定位 metadata 基址
  bool locate(uintptr_t typeDefHandle, const char *klassName) {
    if (!mem || !typeDefHandle || !klassName) return false;
    int32_t nameIdx = mem->rdI32(typeDefHandle + 0x00);
    int32_t nsIdx = mem->rdI32(typeDefHandle + 0x04);
    if (nameIdx < 0 || nameIdx > 0x8000000) return false;
    // 从 typeDefHandle 向前以 4KB 步长找 metadata 魔数
    uintptr_t start = typeDefHandle & ~(uintptr_t)0xFFF;
    // A protected build may leave no plaintext metadata magic at all.  Keep
    // this probe bounded: an unbounded 1 GiB page walk for every class can
    // consume the injection window before the actual dump starts.
    for (uintptr_t back = 0; back <= 0x04000000ull; back += 0x1000) {
      uintptr_t cand = start - back;
      if (cand < 0x10000) break;
      uint32_t magic = mem->rdU32(cand);
      if (magic != 0xFAB11BAF) continue;
      int32_t ver = mem->rdI32(cand + 0x04);
      if (ver < 19 || ver > 31) continue;
      int32_t sOff = mem->rdI32(cand + 0x18);
      int32_t sCnt = mem->rdI32(cand + 0x1C);
      int32_t mOff = mem->rdI32(cand + 0x30);
      int32_t pOff = mem->rdI32(cand + 0x58);
      int32_t tdOff = mem->rdI32(cand + 0xA0);
      int32_t tdCnt = mem->rdI32(cand + 0xA4);
      if (sOff <= 0 || sCnt <= 0 || mOff <= 0 || pOff <= 0 || tdOff <= 0) continue;
      if ((uintptr_t)tdOff > (typeDefHandle - cand)) continue;  // typeDef 必须在镜像内
      char nm[512];
      if (!mem->rdStr(cand + sOff + nameIdx, nm, sizeof(nm), true)) continue;
      if (strcmp(nm, klassName) != 0) continue;
      base = cand;
      stringOff = sOff;
      methodsOff = mOff;
      paramsOff = pOff;
      typeDefsOff = tdOff;
      typeDefsCount = tdCnt;
      ok = true;
      return true;
    }
    // 第二遍：目标构建可能把 metadata 头部的魔数改写了（实测普通体头 4 字节非
    // 0xFAB11BAF）。此时不再依赖魔数，只用「stringOff + nameIndex 必须精确等于
    // 类名」这一条强约束做结构化定位，误判概率可忽略。
    for (uintptr_t back = 0; back <= 0x04000000ull; back += 0x1000) {
      uintptr_t cand = start - back;
      if (cand < 0x10000) break;
      int32_t sOff = mem->rdI32(cand + 0x18);
      int32_t sCnt = mem->rdI32(cand + 0x1C);
      int32_t mOff = mem->rdI32(cand + 0x30);
      int32_t pOff = mem->rdI32(cand + 0x58);
      int32_t tdOff = mem->rdI32(cand + 0xA0);
      int32_t tdCnt = mem->rdI32(cand + 0xA4);
      if (sOff <= 0x40 || sOff > 0x10000000 || sCnt <= 0) continue;
      if (mOff <= 0x40 || pOff <= 0x40 || tdOff <= 0x40) continue;
      if ((uintptr_t)tdOff > (typeDefHandle - cand)) continue;
      char nm[512];
      if (!mem->rdStr(cand + sOff + nameIdx, nm, sizeof(nm), true)) continue;
      if (strcmp(nm, klassName) != 0) continue;
      base = cand;
      stringOff = sOff;
      methodsOff = mOff;
      paramsOff = pOff;
      typeDefsOff = tdOff;
      typeDefsCount = tdCnt;
      ok = true;
      return true;
    }
    return false;
  }

  // 取方法参数名：methodDef 指针 + 序号
  bool paramName(uintptr_t methodDef, int idx, char *out, size_t cap) {
    out[0] = 0;
    if (!ok || !methodDef) return false;
    int32_t pStart = mem->rdI32(methodDef + 0x0C);
    int32_t pCount = mem->rdU16(methodDef + 0x32);
    if (idx < 0 || idx >= pCount) return false;
    uintptr_t pd = base + (uintptr_t)paramsOff + (uintptr_t)(pStart + idx) * (uintptr_t)paramDefSize;
    int32_t nIdx = mem->rdI32(pd + 0x00);
    if (nIdx < 0) return false;
    return mem->rdStr(base + (uintptr_t)stringOff + (uintptr_t)nIdx, out, cap, false);
  }

  // 在类的 method 定义区间内按 token 匹配 methodDef
  uintptr_t findMethodDef(uintptr_t typeDef, uint32_t token) {
    if (!ok || !typeDef) return 0;
    int32_t mStart = mem->rdI32(typeDef + 0x28);
    int32_t mCount = mem->rdU16(typeDef + 0x44);
    if (mCount <= 0 || mCount > 20000) return 0;
    for (int32_t i = 0; i < mCount; ++i) {
      uintptr_t md = base + (uintptr_t)methodsOff + (uintptr_t)(mStart + i) * (uintptr_t)methodDefSize;
      uint32_t t = mem->rdU32(md + 0x28);
      if (t == token) return md;
    }
    return 0;
  }
};

// ---------------------------------------------------------------------------
// 输出 Writer（大缓冲）
// ---------------------------------------------------------------------------
class Writer {
public:
  FILE *f_ = nullptr;
  std::vector<char> buf_;
  size_t len_ = 0;
  uint64_t written_ = 0;
  bool failed_ = false;
  uint64_t escapedBytes = 0;

  Writer() = default;
  Writer(const Writer &) = delete;
  Writer &operator=(const Writer &) = delete;
  ~Writer() { close(); }

  bool open(const char *path, size_t cap = 8u << 20) {
    close();
    written_ = 0;
    failed_ = false;
    escapedBytes = 0;
    f_ = fopen(path, "wb");
    if (!f_) return false;
    buf_.assign(cap, 0);
    len_ = 0;
    return true;
  }
  void putRaw(const char *s, size_t n) {
    if (n == 0) return;
    if (!f_ || failed_) { failed_ = true; return; }
    if (n > buf_.size()) {
      flush();
      const size_t count = fwrite(s, 1, n, f_);
      written_ += count;
      if (count != n) failed_ = true;
      return;
    }
    if (len_ + n > buf_.size()) flush();
    memcpy(buf_.data() + len_, s, n);
    len_ += n;
  }
  void put(const char *s, size_t n) {
    if (ValidUtf8(s, n)) putRaw(s, n);
    else {
      const std::string clean = EscapeInvalidUtf8(s, n, &escapedBytes);
      putRaw(clean.data(), clean.size());
    }
  }
  void puts(const char *s) { put(s, strlen(s)); }
  void putc_(char c) { put(&c, 1); }
  void putf(const char *fmt, ...) {
    char tmp[4096];
    va_list ap;
    va_start(ap, fmt);
    int n = vsnprintf(tmp, sizeof(tmp), fmt, ap);
    va_end(ap);
    if (n <= 0) return;
    if ((size_t)n < sizeof(tmp)) put(tmp, (size_t)n);
    else {
      std::vector<char> full((size_t)n + 1);
      va_start(ap, fmt);
      vsnprintf(full.data(), full.size(), fmt, ap);
      va_end(ap);
      put(full.data(), (size_t)n);
    }
  }
  void flush() {
    if (len_ && f_) {
      const size_t count = fwrite(buf_.data(), 1, len_, f_);
      written_ += count;
      if (count != len_) failed_ = true;
    }
    len_ = 0;
  }
  bool close() {
    flush();
    if (f_) {
      if (fclose(f_) != 0) failed_ = true;
      f_ = nullptr;
    }
    return !failed_;
  }
};

// ---------------------------------------------------------------------------
// 引擎
// ---------------------------------------------------------------------------
struct Options {
  bool fields = true;
  bool properties = true;
  bool methods = true;
  bool printInstance = true;   // // Instance: 0x<klass>
  bool classRvas = true;       // Unity-Offset-compatible TypeInfo pointer-slot RVA
  bool offsetSummary = true;   // Complete Offset.h inventory before the image list
  bool structureIndexes = false; // SuperDumper enables companion indexes by default.
  bool useMetadata = true;     // 参数名（找到 metadata 时）
  bool outHeuristic = true;    // 参数 attrs & Out -> "out "，否则 byref -> "ref "
  uint32_t maxClassesPerImage = 0;  // 0 = 不限
  // Zero uses the complete uint16 count. Each member is independently checked.
  uint32_t maxFields = 0;
  uint32_t maxMethods = 0;
  uint32_t maxProps = 0;
  uint32_t maxIfaces = 512;
  std::string imageFilter;     // 空 = 全部；否则按子串匹配
  std::string outPath;
  std::string logPath;
};

struct Env {
  uintptr_t moduleBase = 0;
  size_t moduleSize = 0;
  std::string moduleName;
  bool superVariant = false;
  uintptr_t unityPlayerBase = 0;
  size_t unityPlayerSize = 0;
  std::string unityPlayerName;
  uintptr_t executableBase = 0;
  bool otherVariantLoaded = false;
  int windowWidth = 0, windowHeight = 0;
  void *(*getCorlib)() = nullptr;
  void *(*imageGetClass)(void *, size_t) = nullptr;
  size_t (*imageGetClassCount)(void *) = nullptr;
  const char *(*imageGetName)(void *) = nullptr;
  void *(*imageGetAssembly)(void *) = nullptr;
  // Hardened/Super builds may expose usable image APIs while keeping the
  // image registry in a different allocation from corlib.  In that case the
  // normal allocation-local discovery sees only framework images; request a
  // full scan of writable private regions for image/assembly structures.
  bool scanAllMemoryForImages = false;
  bool expectMainImage = false;
  // 测试用：直接给定镜像列表（非空则跳过自动发现）
  std::vector<uintptr_t> overrideImages;
  // 测试用：klass -> 模块内 TypeInfo 指针槽 VA。
  std::vector<std::pair<uintptr_t, uintptr_t>> overrideClassPointerSlots;
};

struct Stats {
  uint64_t images = 0, classes = 0, fields = 0, methods = 0, props = 0, ifaces = 0;
  uint64_t skippedClasses = 0, badNames = 0, metaParams = 0, synthParams = 0;
  uint64_t runtimeParamNames = 0, parameterInfoLayout = 0;
  uint64_t rvaZero = 0, classTypeFallbacks = 0, classRvas = 0;
};

struct ManagerRvaResult {
  std::string className;
  uintptr_t klass = 0;
  uintptr_t slot = 0;
  uint64_t rva = 0;
  uint64_t candidateCount = 0;
  uint32_t staticFieldsOffset = 0;
  uintptr_t staticFields = 0;
  uint32_t instanceFieldOffset = 0;
  uintptr_t instance = 0;
  uintptr_t instanceKlass = 0;
  bool foundClass = false;
  bool foundSlot = false;
  bool verified = false;
};

class Dumper {
public:
  MemReader mem;
  Metadata meta;
  Layout lay;
  Env *env = nullptr;
  Options *opt = nullptr;
  LogCtx *log = nullptr;
  Stats st;
  QualityStats quality;
  MemberAudit *memberAudit_ = nullptr; // Set only while writing a class category.
  std::vector<uintptr_t> images_;
  // TypeDefinitionIndex -> Il2CppClass*（普通体由镜像枚举建立；Super 由类型表扫描建立）
  std::vector<uintptr_t> typeTable_;
  // Il2CppType.data 语义：true = 小整数按 TypeDefinitionIndex 解释
  bool typeDataIsIndex_ = false;
  bool typeDataIsHandle_ = false;
  int typeDataPtrHits_ = 0, typeDataIdxHits_ = 0, typeDataHandleHits_ = 0;
  // Newer libil2cpp builds store Il2CppMetadataTypeHandle in Il2CppType.data.
  std::unordered_map<uintptr_t, uintptr_t> typeHandleToKlass_;
  // 参数数组布局：0 = 紧凑 Il2CppType*[]，1 = ParameterInfo(type@+0x10)，2 = ParameterInfo(type@+0x08)
  int paramMode_ = 0;
  // Super：全内存扫描得到的 Il2CppClass* 集合
  std::vector<uintptr_t> klasses_;
  bool superMode_ = false;
  // Image headers are not identical in every protected build. Cache the
  // authoritative API count and the calibrated global TypeDefinition start
  // instead of reading +0x18/+0x1C throughout the writer.
  std::unordered_map<uintptr_t, size_t> imageCounts_;
  std::unordered_map<uintptr_t, uint32_t> imageStarts_;
  std::unordered_map<uintptr_t, uint32_t> classIndices_;
  // Il2CppClass* -> GameAssembly 内保存该指针的 TypeInfo 全局槽 VA。
  std::unordered_map<uintptr_t, uintptr_t> classPointerSlots_;
  // Dedicated results rendered at the top of dump.cs for direct offset use.
  std::vector<ManagerRvaResult> managerRvaResults_;
  std::string imageLayoutProfile_;
  StructureIndexes indexes_;

  std::string indexAssembly(uintptr_t k) {
    char name[256] = {};
    uintptr_t img = mem.rdPtr(k + lay.cImage);
    if (!mem.rdStr(mem.rdPtr(img), name, sizeof(name), false) || !ValidUtf8(name) || !imageNameLooksValid(name)) return "";
    return name;
  }

  std::string indexNamespace(uintptr_t k) {
    char ns[512] = {};
    mem.rdStr(mem.rdPtr(k + lay.cNamespace), ns, sizeof(ns), false);
    return ns;
  }

  // ---------------- 类型名 ----------------
  bool klassName(uintptr_t k, char *out, size_t cap) {
    if (out && cap) out[0] = 0;
    if (!k || !out || cap < 2) return false;
    uintptr_t np = mem.rdPtr(k + lay.cName);
    return mem.rdStr(np, out, cap, false);
  }

  bool isKlassPtr(uintptr_t k) {
    if (!k) return false;
    uintptr_t np = mem.rdPtr(k + lay.cName);
    return np && mem.probe(np, 1);
  }

  size_t imageCount(uintptr_t img) {
    auto it = imageCounts_.find(img);
    if (it != imageCounts_.end()) return it->second;
    size_t count = 0;
    if (env && env->imageGetClassCount)
      SafeCallImageGetClassCount(env->imageGetClassCount, img, count);
    if (count == 0 || count > 400000) count = (size_t)mem.rdU32(img + 0x1C);
    if ((count == 0 || count > 400000) && env && env->scanAllMemoryForImages)
      count = (size_t)mem.rdU32(img + 0x18);
    if (count <= 400000) imageCounts_[img] = count;
    return count;
  }

  uint32_t imageStart(uintptr_t img) const {
    auto it = imageStarts_.find(img);
    return it != imageStarts_.end() ? it->second : 0;
  }

  uint32_t classIndex(uintptr_t klass, uint32_t fallback) const {
    auto it = classIndices_.find(klass);
    return it != classIndices_.end() ? it->second : fallback;
  }

  void prepareImageLayout(std::vector<uintptr_t> &images) {
    imageStarts_.clear();
    imageLayoutProfile_.clear();
    if (images.empty()) return;

    size_t countAt18 = 0, countAt1C = 0;
    for (uintptr_t img : images) {
      size_t count = imageCount(img);
      if (count && mem.rdU32(img + 0x18) == count) ++countAt18;
      if (count && mem.rdU32(img + 0x1C) == count) ++countAt1C;
    }

    // Find the dword whose values form the strongest non-overlapping chain
    // when paired with the API-reported class counts. This identifies the
    // real typeStart even when a protector swaps or moves header fields.
    const uint32_t offsets[] = {0x18, 0x1C, 0x20, 0x24, 0x28, 0x2C,
                                0x30, 0x34, 0x38, 0x3C, 0x40, 0x44};
    const bool countClearlyAt18 = countAt18 > countAt1C && countAt18 * 2 >= images.size();
    const bool countClearlyAt1C = countAt1C >= countAt18 && countAt1C * 2 >= images.size();
    int bestScore = -1000000, bestExact = 0, bestCompatible = 0;
    uint32_t bestOffset = 0;
    for (uint32_t off : offsets) {
      if ((countClearlyAt18 && off == 0x18) || (countClearlyAt1C && off == 0x1C)) continue;
      struct StartItem {
        uint32_t start;
        size_t count;
      };
      std::vector<StartItem> items;
      bool valid = true;
      bool anyNonZero = false;
      for (uintptr_t img : images) {
        uint32_t start = mem.rdU32(img + off);
        size_t count = imageCount(img);
        if (start > 4000000 || count == 0 || count > 400000 ||
            (uint64_t)start + count > 4000000) {
          valid = false;
          break;
        }
        if (start) anyNonZero = true;
        items.push_back({start, count});
      }
      if (!valid || (!anyNonZero && images.size() > 1)) continue;
      std::sort(items.begin(), items.end(), [](const StartItem &a, const StartItem &b) {
        return a.start < b.start;
      });
      int exact = 0, compatible = 0, duplicates = 0;
      for (size_t i = 1; i < items.size(); ++i) {
        uint64_t expected = (uint64_t)items[i - 1].start + items[i - 1].count;
        if (items[i].start == items[i - 1].start) ++duplicates;
        if (items[i].start == expected) ++exact;
        if (items[i].start >= expected) ++compatible;
      }
      int score = exact * 20 + compatible * 3 - duplicates * 30;
      if (items.size() > 1 && items.front().start == 0) score += 5;
      if ((!countClearlyAt18 && off == 0x18) || (countClearlyAt18 && off == 0x1C)) score += 2;
      if (score > bestScore) {
        bestScore = score;
        bestExact = exact;
        bestCompatible = compatible;
        bestOffset = off;
      }
    }

    const bool chainIsUseful = images.size() <= 2 || bestExact > 0 ||
                               bestCompatible * 2 >= (int)images.size();
    if (bestOffset && chainIsUseful) {
      for (uintptr_t img : images) imageStarts_[img] = mem.rdU32(img + bestOffset);
      std::sort(images.begin(), images.end(), [&](uintptr_t a, uintptr_t b) {
        uint32_t sa = imageStarts_[a], sb = imageStarts_[b];
        return sa == sb ? a < b : sa < sb;
      });
      char profile[96];
      snprintf(profile, sizeof(profile), "count@%s,start@0x%02X",
               countClearlyAt18 ? "0x18" : (countClearlyAt1C ? "0x1C" : "API"), bestOffset);
      imageLayoutProfile_ = profile;
      log->line("[img] layout=%s countMatch18=%llu countMatch1C=%llu exact=%d compatible=%d",
                imageLayoutProfile_.c_str(), (unsigned long long)countAt18,
                (unsigned long long)countAt1C, bestExact, bestCompatible);
      return;
    }

    // Last-resort profile for headers where no global start survives. Image
    // objects are allocated as a contiguous registry in this Unity line, so
    // address order retains registration order and cumulative counts recover
    // stable TypeDefinitionIndex values.
    std::sort(images.begin(), images.end());
    uint64_t next = 0;
    for (uintptr_t img : images) {
      imageStarts_[img] = (uint32_t)next;
      next += imageCount(img);
      if (next > 4000000) break;
    }
    imageLayoutProfile_ = "synthesized-address-order";
    log->line("[img] layout=%s countMatch18=%llu countMatch1C=%llu",
              imageLayoutProfile_.c_str(), (unsigned long long)countAt18,
              (unsigned long long)countAt1C);
  }

  // Protected/newer builds may remove typeStart from Il2CppImage while the
  // embedded klass->byval_arg still carries the global TypeDefinitionIndex.
  // Recover both the type table and image order from that authoritative index.
  bool buildIndexMapFromClassTypeData(std::vector<uintptr_t> &images) {
    if (!env || !env->imageGetClass || !env->imageGetClassCount || images.empty()) return false;
    const uint32_t typeOffsets[] = {0x20, 0x28, 0x30, 0x38};
    std::vector<uintptr_t> samples;
    for (uintptr_t img : images) {
      size_t count = imageCount(img);
      if (!count || count > 400000) continue;
      size_t want = 512 - samples.size();
      if (!want) break;
      size_t step = count > want ? count / want : 1;
      for (size_t i = 0; i < count && samples.size() < 512; i += step) {
        uintptr_t k = (uintptr_t)env->imageGetClass((void *)img, i);
        if (k) samples.push_back(k);
      }
    }
    if (samples.size() < 8) return false;

    uint32_t bestOffset = 0;
    size_t bestHits = 0, bestUnique = 0;
    for (uint32_t off : typeOffsets) {
      size_t hits = 0;
      std::unordered_set<uint32_t> unique;
      for (uintptr_t k : samples) {
        uint32_t bits = mem.rdU32(k + off + lay.tBits);
        uint32_t te = (bits >> 16) & 0xFF;
        uintptr_t data = mem.rdPtr(k + off + lay.tData);
        if (te != T_CLASS && te != T_VALUETYPE && te != T_ENUM) continue;
        if (data >= 4000000) continue;
        ++hits;
        unique.insert((uint32_t)data);
      }
      if (hits > bestHits || (hits == bestHits && unique.size() > bestUnique)) {
        bestOffset = off;
        bestHits = hits;
        bestUnique = unique.size();
      }
    }
    log->line("[tbl] klass byval index probe off=0x%X hits=%llu unique=%llu samples=%llu",
              bestOffset, (unsigned long long)bestHits, (unsigned long long)bestUnique,
              (unsigned long long)samples.size());
    if (!bestOffset || bestHits * 2 < samples.size() || bestUnique * 4 < bestHits * 3)
      return false;

    std::vector<std::pair<uint32_t, uintptr_t>> indexed;
    std::unordered_map<uintptr_t, uint32_t> starts;
    size_t expected = 0, invalid = 0;
    uint32_t maxIndex = 0;
    for (uintptr_t img : images) {
      size_t count = imageCount(img);
      expected += count;
      uint32_t minIndex = 0xFFFFFFFFu;
      for (size_t i = 0; i < count; ++i) {
        uintptr_t k = (uintptr_t)env->imageGetClass((void *)img, i);
        if (!k) {
          ++invalid;
          continue;
        }
        uint32_t bits = mem.rdU32(k + bestOffset + lay.tBits);
        uint32_t te = (bits >> 16) & 0xFF;
        uintptr_t data = mem.rdPtr(k + bestOffset + lay.tData);
        if ((te != T_CLASS && te != T_VALUETYPE && te != T_ENUM) || data >= 4000000) {
          ++invalid;
          continue;
        }
        uint32_t idx = (uint32_t)data;
        indexed.push_back({idx, k});
        if (idx < minIndex) minIndex = idx;
        if (idx > maxIndex) maxIndex = idx;
      }
      if (minIndex != 0xFFFFFFFFu) starts[img] = minIndex;
    }
    if (indexed.size() * 4 < expected * 3 || maxIndex >= 4000000) {
      log->line("[tbl] klass byval index rejected indexed=%llu expected=%llu invalid=%llu max=%u",
                (unsigned long long)indexed.size(), (unsigned long long)expected,
                (unsigned long long)invalid, maxIndex);
      return false;
    }

    std::vector<uintptr_t> table((size_t)maxIndex + 1, 0);
    std::unordered_map<uintptr_t, uint32_t> reverse;
    reverse.reserve(indexed.size());
    size_t filled = 0, collisions = 0;
    for (const auto &entry : indexed) {
      if (table[entry.first] && table[entry.first] != entry.second) {
        ++collisions;
        continue;
      }
      if (!table[entry.first]) ++filled;
      table[entry.first] = entry.second;
      reverse[entry.second] = entry.first;
    }
    if (collisions * 100 > indexed.size()) {
      log->line("[tbl] klass byval index rejected collisions=%llu indexed=%llu",
                (unsigned long long)collisions, (unsigned long long)indexed.size());
      return false;
    }

    typeTable_.swap(table);
    classIndices_.swap(reverse);
    for (const auto &entry : starts) imageStarts_[entry.first] = entry.second;
    std::sort(images.begin(), images.end(), [&](uintptr_t a, uintptr_t b) {
      uint32_t sa = imageStart(a), sb = imageStart(b);
      return sa == sb ? a < b : sa < sb;
    });
    char profile[128];
    snprintf(profile, sizeof(profile), "klass-byval-index@0x%02X (%s)", bestOffset,
             imageLayoutProfile_.c_str());
    imageLayoutProfile_ = profile;
    log->line("[tbl] index map built from klass byval: size=%llu filled=%llu collisions=%llu images=%llu",
              (unsigned long long)typeTable_.size(), (unsigned long long)filled,
              (unsigned long long)collisions, (unsigned long long)starts.size());
    return true;
  }

  // Newer Unity versions keep a metadata type handle in the embedded
  // klass->byval_arg and in CLASS/VALUETYPE Il2CppType.data.  The handle is
  // not an Il2CppClass pointer, but every image API still gives us the class
  // owning it.  Build the reverse map and, only when handle address order is
  // consistent with every image's local class order, recover global indices.
  bool buildIndexMapFromClassTypeHandles(std::vector<uintptr_t> &images) {
    if (!env || !env->imageGetClass || !env->imageGetClassCount || images.empty()) return false;
    const uint32_t typeOffsets[] = {0x20, 0x28, 0x30, 0x38};
    std::vector<uintptr_t> samples;
    for (uintptr_t img : images) {
      size_t count = imageCount(img);
      if (!count || count > 400000) continue;
      size_t want = 512 - samples.size();
      if (!want) break;
      size_t step = count > want ? count / want : 1;
      for (size_t i = 0; i < count && samples.size() < 512; i += step) {
        uintptr_t k = (uintptr_t)env->imageGetClass((void *)img, i);
        if (k) samples.push_back(k);
      }
    }
    if (samples.size() < 8) return false;

    uint32_t bestOffset = 0;
    size_t bestHits = 0, bestUnique = 0, bestTdhMatches = 0;
    int bestScore = -1;
    for (uint32_t off : typeOffsets) {
      size_t hits = 0, tdhMatches = 0;
      std::unordered_set<uintptr_t> unique;
      for (uintptr_t k : samples) {
        uint32_t bits = mem.rdU32(k + off + lay.tBits);
        uint32_t te = (bits >> 16) & 0xFF;
        uintptr_t data = mem.rdPtr(k + off + lay.tData);
        if (te != T_CLASS && te != T_VALUETYPE && te != T_ENUM) continue;
        if (data < 0x10000 || (data & 3) || !mem.probe(data, 1)) continue;
        if (isKlassPtr(data)) continue;
        ++hits;
        unique.insert(data);
        if (mem.rdPtr(k + lay.cTypeDefHandle) == data) ++tdhMatches;
      }
      int score = (int)(hits * 2 + tdhMatches);
      if (score > bestScore || (score == bestScore && unique.size() > bestUnique)) {
        bestScore = score;
        bestOffset = off;
        bestHits = hits;
        bestUnique = unique.size();
        bestTdhMatches = tdhMatches;
      }
    }
    log->line("[tbl] klass type-handle probe off=0x%X hits=%llu unique=%llu tdhMatch=%llu samples=%llu",
              bestOffset, (unsigned long long)bestHits, (unsigned long long)bestUnique,
              (unsigned long long)bestTdhMatches, (unsigned long long)samples.size());
    if (!bestOffset || bestHits * 2 < samples.size() || bestUnique * 4 < bestHits * 3)
      return false;

    struct HandleClass {
      uintptr_t handle;
      uintptr_t klass;
      uintptr_t image;
      size_t localIndex;
    };
    std::vector<HandleClass> records;
    std::unordered_map<uintptr_t, uintptr_t> handles;
    size_t expected = 0, invalid = 0, collisions = 0;
    for (uintptr_t img : images) {
      size_t count = imageCount(img);
      expected += count;
      for (size_t i = 0; i < count; ++i) {
        uintptr_t k = (uintptr_t)env->imageGetClass((void *)img, i);
        if (!k) {
          ++invalid;
          continue;
        }
        uint32_t bits = mem.rdU32(k + bestOffset + lay.tBits);
        uint32_t te = (bits >> 16) & 0xFF;
        uintptr_t handle = mem.rdPtr(k + bestOffset + lay.tData);
        if ((te != T_CLASS && te != T_VALUETYPE && te != T_ENUM) || handle < 0x10000 ||
            (handle & 3) || !mem.probe(handle, 1) || isKlassPtr(handle)) {
          ++invalid;
          continue;
        }
        auto inserted = handles.emplace(handle, k);
        if (!inserted.second && inserted.first->second != k) {
          ++collisions;
          continue;
        }
        uintptr_t tdh = mem.rdPtr(k + lay.cTypeDefHandle);
        if (tdh >= 0x10000 && !(tdh & 3) && mem.probe(tdh, 1)) {
          auto tdhInserted = handles.emplace(tdh, k);
          if (!tdhInserted.second && tdhInserted.first->second != k) ++collisions;
        }
        records.push_back({handle, k, img, i});
      }
    }
    if (records.size() * 4 < expected * 3 || collisions * 100 > records.size()) {
      log->line("[tbl] type-handle map rejected records=%llu expected=%llu invalid=%llu collisions=%llu",
                (unsigned long long)records.size(), (unsigned long long)expected,
                (unsigned long long)invalid, (unsigned long long)collisions);
      return false;
    }
    typeHandleToKlass_.swap(handles);
    log->line("[tbl] type-handle map built entries=%llu records=%llu expected=%llu",
              (unsigned long long)typeHandleToKlass_.size(), (unsigned long long)records.size(),
              (unsigned long long)expected);

    std::sort(records.begin(), records.end(), [](const HandleClass &a, const HandleClass &b) {
      return a.handle == b.handle ? a.klass < b.klass : a.handle < b.handle;
    });
    std::unordered_map<uintptr_t, uint32_t> ranks;
    ranks.reserve(records.size());
    for (size_t i = 0; i < records.size(); ++i) ranks[records[i].klass] = (uint32_t)i;

    size_t exactImages = 0, exactClasses = 0;
    std::unordered_map<uintptr_t, uint32_t> starts;
    for (uintptr_t img : images) {
      size_t count = imageCount(img);
      uint32_t minRank = 0xFFFFFFFFu, maxRank = 0;
      bool complete = count != 0;
      std::vector<uint32_t> localRanks;
      localRanks.reserve(count);
      for (size_t i = 0; i < count; ++i) {
        uintptr_t k = (uintptr_t)env->imageGetClass((void *)img, i);
        auto it = ranks.find(k);
        if (it == ranks.end()) {
          complete = false;
          break;
        }
        localRanks.push_back(it->second);
        if (it->second < minRank) minRank = it->second;
        if (it->second > maxRank) maxRank = it->second;
      }
      bool exact = complete && localRanks.size() == count &&
                   (uint64_t)maxRank - minRank + 1 == count;
      if (exact) {
        for (size_t i = 0; i < localRanks.size(); ++i) {
          if (localRanks[i] != minRank + i) {
            exact = false;
            break;
          }
        }
      }
      if (exact) {
        starts[img] = minRank;
        ++exactImages;
        exactClasses += count;
      }
    }
    const bool orderAccepted = exactImages * 4 >= images.size() * 3 &&
                               exactClasses * 4 >= records.size() * 3 &&
                               records.size() == expected;
    log->line("[tbl] type-handle order exactImages=%llu/%llu exactClasses=%llu/%llu -> %s",
              (unsigned long long)exactImages, (unsigned long long)images.size(),
              (unsigned long long)exactClasses, (unsigned long long)records.size(),
              orderAccepted ? "accepted" : "map-only");
    if (!orderAccepted) return false;

    typeTable_.assign(records.size(), 0);
    classIndices_.clear();
    classIndices_.reserve(records.size());
    for (size_t i = 0; i < records.size(); ++i) {
      typeTable_[i] = records[i].klass;
      classIndices_[records[i].klass] = (uint32_t)i;
    }
    for (const auto &entry : starts) imageStarts_[entry.first] = entry.second;
    std::sort(images.begin(), images.end(), [&](uintptr_t a, uintptr_t b) {
      uint32_t sa = imageStart(a), sb = imageStart(b);
      return sa == sb ? a < b : sa < sb;
    });
    char profile[128];
    snprintf(profile, sizeof(profile), "klass-type-handle@0x%02X (%s)", bestOffset,
             imageLayoutProfile_.c_str());
    imageLayoutProfile_ = profile;
    log->line("[tbl] index map built from type-handle order: size=%llu images=%llu",
              (unsigned long long)typeTable_.size(), (unsigned long long)starts.size());
    return true;
  }

  // Il2CppType.data 双语义解析：
  //   指针形态（已解析）= Il2CppClass*；索引形态（未解析）= TypeDefinitionIndex。
  // 实测加固构建里大量 T_CLASS/T_VALUETYPE 的 data 仍是索引，直接用指针读会退化成
  // "Class"/"Type0"（旧版 219707 行）。这里两种形态都试。
  uintptr_t resolveKlass(uintptr_t data, char *nmOut, size_t cap) {
    if (nmOut && cap) nmOut[0] = 0;
    if (!data) return 0;
    auto tryPtr = [&](uintptr_t k) -> uintptr_t {
      if (!k) return 0;
      if (nmOut) return klassName(k, nmOut, cap) ? k : (uintptr_t)0;
      return isKlassPtr(k) ? k : (uintptr_t)0;
    };
    auto hit = typeHandleToKlass_.find(data);
    if (hit != typeHandleToKlass_.end()) {
      uintptr_t k = tryPtr(hit->second);
      if (k) return k;
    }
    if (typeDataIsIndex_ && data < typeTable_.size()) {
      uintptr_t k = tryPtr(typeTable_[(size_t)data]);
      if (k) return k;
    }
    if (data >= 0x10000) {
      uintptr_t k = tryPtr(data);
      if (k) return k;
    }
    if (!typeDataIsIndex_ && data < typeTable_.size()) {
      uintptr_t k = tryPtr(typeTable_[(size_t)data]);
      if (k) return k;
    }
    return 0;
  }

  // 普通体：镜像枚举本身就带 TypeDefinitionIndex 顺序，直接建立索引表
  void buildIndexMapFromImages(const std::vector<uintptr_t> &images) {
    typeTable_.clear();
    if (!env || !env->imageGetClass || !env->imageGetClassCount) return;
    size_t maxIdx = 0;
    for (uintptr_t img : images) {
      uint32_t ts = imageStart(img);
      size_t cnt = imageCount(img);
      if (ts + cnt > maxIdx) maxIdx = ts + cnt;
    }
    if (maxIdx == 0 || maxIdx > 4000000) return;
    typeTable_.assign(maxIdx, 0);
    size_t filled = 0;
    for (uintptr_t img : images) {
      uint32_t ts = imageStart(img);
      size_t cnt = imageCount(img);
      if (cnt > 400000) cnt = 0;
      for (size_t i = 0; i < cnt; ++i) {
        uintptr_t k = (uintptr_t)env->imageGetClass((void *)img, i);
        if (!k) continue;
        if (ts + i < typeTable_.size()) {
          typeTable_[ts + i] = k;
          ++filled;
        }
      }
    }
    log->line("[tbl] index map built from images: size=%llu filled=%llu",
              (unsigned long long)typeTable_.size(), (unsigned long long)filled);
  }

  // 统计 T_CLASS/T_VALUETYPE 的 data 更像指针还是更像索引
  void detectTypeDataMode(const std::vector<uintptr_t> &samples) {
    typeDataPtrHits_ = 0;
    typeDataIdxHits_ = 0;
    typeDataHandleHits_ = 0;
    typeDataIsHandle_ = false;
    if (typeTable_.empty() && typeHandleToKlass_.empty()) return;
    for (uintptr_t k : samples) {
      uintptr_t farr = mem.rdPtr(k + lay.cFields);
      uint32_t nf = mem.rdU16(k + lay.cFieldCount);
      if (nf > 32) nf = 32;
      for (uint32_t i = 0; i < nf; ++i) {
        uintptr_t fi = farr + (uintptr_t)i * lay.fStride;
        uintptr_t tp = mem.rdPtr(fi + lay.fType);
        if (!looksLikeType(tp)) continue;
        uint32_t bits = mem.rdU32(tp + lay.tBits);
        uint32_t te = (bits >> 16) & 0xFF;
        if (te != T_CLASS && te != T_VALUETYPE && te != T_ENUM) continue;
        uintptr_t d = mem.rdPtr(tp + lay.tData);
        if (!d) continue;
        if (typeHandleToKlass_.find(d) != typeHandleToKlass_.end()) {
          ++typeDataHandleHits_;
        } else if (d >= 0x10000) {
          char nm[256];
          if (klassName(d, nm, sizeof(nm))) ++typeDataPtrHits_;
        } else if (d < typeTable_.size() && typeTable_[(size_t)d]) {
          ++typeDataIdxHits_;
        }
      }
    }
    // 指针形态能读到合法类名；索引形态只能通过索引表命中
    typeDataIsHandle_ = typeDataHandleHits_ > typeDataPtrHits_ &&
                        typeDataHandleHits_ > typeDataIdxHits_;
    typeDataIsIndex_ = !typeDataIsHandle_ && typeDataIdxHits_ > typeDataPtrHits_;
    log->line("[cal] typedata ptr=%d idx=%d handle=%d -> mode=%s", typeDataPtrHits_,
              typeDataIdxHits_, typeDataHandleHits_,
              typeDataIsHandle_ ? "handle" : (typeDataIsIndex_ ? "index" : "pointer"));
  }

  bool typeName(uintptr_t tp, char *out, size_t cap, int depth) {
    if (!out || cap < 2) return false;
    out[0] = 0;
    if (!tp || depth > 6) {
      snprintf(out, cap, "?");
      return false;
    }
    uintptr_t data = mem.rdPtr(tp + lay.tData);
    uint32_t bits = mem.rdU32(tp + lay.tBits);
    uint32_t te = (bits >> 16) & 0xFF;
    switch (te) {
      case T_VOID: snprintf(out, cap, "Void"); return true;
      case T_BOOLEAN: snprintf(out, cap, "Boolean"); return true;
      case T_CHAR: snprintf(out, cap, "Char"); return true;
      case T_I1: snprintf(out, cap, "SByte"); return true;
      case T_U1: snprintf(out, cap, "Byte"); return true;
      case T_I2: snprintf(out, cap, "Int16"); return true;
      case T_U2: snprintf(out, cap, "UInt16"); return true;
      case T_I4: snprintf(out, cap, "Int32"); return true;
      case T_U4: snprintf(out, cap, "UInt32"); return true;
      case T_I8: snprintf(out, cap, "Int64"); return true;
      case T_U8: snprintf(out, cap, "UInt64"); return true;
      case T_R4: snprintf(out, cap, "Single"); return true;
      case T_R8: snprintf(out, cap, "Double"); return true;
      case T_STRING: snprintf(out, cap, "String"); return true;
      case T_OBJECT: snprintf(out, cap, "Object"); return true;
      case T_I: snprintf(out, cap, "IntPtr"); return true;
      case T_U: snprintf(out, cap, "UIntPtr"); return true;
      case T_TYPEDBYREF: snprintf(out, cap, "TypedReference"); return true;
      case T_FNPTR: snprintf(out, cap, "FnPtr"); return true;
      case T_CLASS:
      case T_VALUETYPE:
      case T_ENUM: {
        char nm[512];
        if (resolveKlass(data, nm, sizeof(nm))) {
          snprintf(out, cap, "%s", nm);
          return true;
        }
        ++st.classTypeFallbacks;
        snprintf(out, cap, "Class");
        return false;
      }
      case T_SZARRAY: {
        char inner[400];
        typeName(data, inner, sizeof(inner), depth + 1);
        snprintf(out, cap, "%s[]", inner);
        return true;
      }
      case T_ARRAY: {
        // Il2CppArrayType { Il2CppType* etype; u8 rank; u8 numsizes; u8 numlobounds; ... }
        char inner[400];
        uintptr_t etype = mem.rdPtr(data + 0x00);
        uint8_t rank = mem.rdU8(data + 0x08);
        typeName(etype, inner, sizeof(inner), depth + 1);
        if (rank <= 1) snprintf(out, cap, "%s[]", inner);
        else {
          char commas[32];
          size_t n = 0;
          for (int i = 1; i < (int)rank && n + 1 < sizeof(commas); ++i) commas[n++] = ',';
          commas[n] = 0;
          snprintf(out, cap, "%s[%s]", inner, commas);
        }
        return true;
      }
      case T_PTR: {
        char inner[400];
        typeName(data, inner, sizeof(inner), depth + 1);
        snprintf(out, cap, "%s*", inner);
        return true;
      }
      case T_BYREF: {
        char inner[400];
        typeName(data, inner, sizeof(inner), depth + 1);
        snprintf(out, cap, "%s&", inner);
        return true;
      }
      case T_GENERICINST: {
        // Il2CppGenericClass { type/idx, Il2CppGenericContext, Il2CppClass* cached_class }
        uintptr_t cached = mem.rdPtr(data + 0x18);
        char nm[512];
        if (cached && resolveKlass(cached, nm, sizeof(nm))) {
          snprintf(out, cap, "%s", nm);
          return true;
        }
        uintptr_t tdef = mem.rdPtr(data + 0x00);
        if (tdef) {
          uintptr_t kk = mem.rdPtr(tdef + lay.tData);
          if (kk && resolveKlass(kk, nm, sizeof(nm))) {
            snprintf(out, cap, "%s", nm);
            return true;
          }
        }
        snprintf(out, cap, "GenericClass");
        return false;
      }
      case T_VAR:
      case T_MVAR: {
        char nm[512];
        uintptr_t kk = mem.rdPtr(tp + lay.tData);
        if (kk && resolveKlass(kk, nm, sizeof(nm))) {
          snprintf(out, cap, "%s", nm);
          return true;
        }
        snprintf(out, cap, "T");
        return true;
      }
      default:
        break;
    }
    snprintf(out, cap, "Type%u", te);
    return false;
  }

  uint32_t typeSize(uintptr_t tp) {
    if (!tp) return 8;
    uintptr_t data = mem.rdPtr(tp + lay.tData);
    uint32_t bits = mem.rdU32(tp + lay.tBits);
    uint32_t te = (bits >> 16) & 0xFF;
    switch (te) {
      case T_BOOLEAN:
      case T_I1:
      case T_U1: return 1;
      case T_CHAR:
      case T_I2:
      case T_U2: return 2;
      case T_I4:
      case T_U4:
      case T_R4: return 4;
      case T_I8:
      case T_U8:
      case T_R8: return 8;
      case T_VALUETYPE:
      case T_ENUM: {
        uintptr_t kk = resolveKlass(data, nullptr, 0);
        uint32_t is = kk ? mem.rdU32(kk + lay.cInstanceSize) : 0u;
        if (is > 0 && is < 0x10000) return is;
        return 4;
      }
      default: return 8;
    }
  }

  // ---------------- 修饰符 ----------------
  static const char *visibilityOf(uint32_t access, uint32_t mask) {
    uint32_t v = access & mask;
    switch (v) {
      case 1: return "private ";
      case 2: return "internal ";
      case 3: return "internal ";
      case 4: return "protected ";
      case 5: return "protected ";
      case 6: return "public ";
      case 7: return "protected ";
      default: return "internal ";
    }
  }
  static const char *classVisibility(uint32_t flags) {
    switch (flags & TA_VISIBILITY_MASK) {
      case TA_PUBLIC:
      case TA_NESTED_PUBLIC: return "public ";
      case TA_NESTED_PRIVATE: return "private ";
      case TA_NESTED_FAMILY: return "protected ";
      case TA_NOT_PUBLIC:
      case TA_NESTED_ASSEMBLY: return "internal ";
      default: return "internal ";
    }
  }

  void classMods(uint32_t flags, const char *kind, char *out, size_t cap) {
    const char *vis = classVisibility(flags);
    // C# 不允许 enum/struct/interface 带 sealed/abstract/static 修饰
    if (kind && strcmp(kind, "class") != 0) {
      snprintf(out, cap, "%s", vis);
      return;
    }
    const char *kw = "";
    bool isAbstract = (flags & TA_ABSTRACT) != 0;
    bool isSealed = (flags & TA_SEALED) != 0;
    if (isAbstract && isSealed) kw = "static ";
    else if (isSealed) kw = "sealed ";
    else if (isAbstract) kw = "abstract ";
    snprintf(out, cap, "%s%s", vis, kw);
  }

  void fieldMods(uint32_t fattr, char *out, size_t cap) {
    const char *vis = visibilityOf(fattr, FA_FIELD_ACCESS_MASK);
    if (fattr & FA_LITERAL) {  // const 不写 static/readonly
      snprintf(out, cap, "%sconst ", vis);
      return;
    }
    const char *st = (fattr & FA_STATIC) ? "static " : "";
    const char *ro = (fattr & FA_INIT_ONLY) ? "readonly " : "";
    snprintf(out, cap, "%s%s%s", vis, st, ro);
  }

  void methodMods(uint16_t mflags, char *out, size_t cap) {
    const char *vis = visibilityOf(mflags, MA_MEMBER_ACCESS_MASK);
    const char *st = (mflags & MA_STATIC) ? "static " : "";
    const char *sealed_ = (mflags & MA_FINAL) ? "sealed " : "";
    const char *vk = "";
    if (mflags & MA_ABSTRACT)
      vk = ((mflags & MA_VIRTUAL) && !(mflags & MA_NEW_SLOT)) ? "abstract override " : "abstract ";
    else if (mflags & MA_VIRTUAL)
      vk = (mflags & MA_NEW_SLOT) ? "virtual " : "override ";
    const char *ex = (mflags & MA_PINVOKE_IMPL) ? "extern " : "";
    snprintf(out, cap, "%s%s%s%s%s", vis, st, sealed_, vk, ex);
  }

  // ---------------- 校准 ----------------
  struct Candidate {
    uint32_t off;
    int score;
  };

  int sampleClasses(std::vector<uintptr_t> &out, int want) {
    out.clear();
    if (!env) return 0;
    if (!env->imageGetClass) {
      // Super：从扫描结果 / token 索引表取样
      const std::vector<uintptr_t> &src = typeTable_.empty() ? klasses_ : typeTable_;
      size_t total = src.size();
      if (total == 0) return 0;
      size_t step = total > (size_t)want ? total / (size_t)want : 1;
      for (size_t i = 0; i < total && (int)out.size() < want; i += step) {
        uintptr_t k = src[i];
        if (!k) continue;
        char nm[256];
        if (!klassName(k, nm, sizeof(nm))) continue;
        if (!NameLooksValid(nm, true)) continue;
        out.push_back(k);
      }
      return (int)out.size();
    }
    std::vector<uintptr_t> images = images_;
    if (images.empty()) images = env->overrideImages;
    if (images.empty() && env->getCorlib) {
      uintptr_t c = 0;
      if (SafeCallGetCorlib(env->getCorlib, c) && c) images.push_back(c);
    }
    for (uintptr_t img : images) {
      if (!img || !env->imageGetClassCount || !env->imageGetClass) continue;
      size_t cnt = imageCount(img);
      if (cnt == 0 || cnt > 400000) continue;
      size_t step = cnt > (size_t)want ? cnt / (size_t)want : 1;
      if (step == 0) step = 1;
      for (size_t i = 0; i < cnt && (int)out.size() < want; i += step) {
        void *k = env->imageGetClass((void *)img, i);
        if (!k) continue;
        char nm[256];
        if (!klassName((uintptr_t)k, nm, sizeof(nm))) continue;
        if (!NameLooksValid(nm, true)) continue;
        out.push_back((uintptr_t)k);
      }
      if ((int)out.size() >= want) break;
    }
    return (int)out.size();
  }

  bool looksLikeType(uintptr_t tp) {
    if (!tp || !mem.probe(tp, 0x10)) return false;
    uint32_t bits = mem.rdU32(tp + lay.tBits);
    uint32_t te = (bits >> 16) & 0xFF;
    if (te == 0 || te > 0x60) return false;
    switch (te) {
      case T_VOID:
      case T_BOOLEAN:
      case T_CHAR:
      case T_I1:
      case T_U1:
      case T_I2:
      case T_U2:
      case T_I4:
      case T_U4:
      case T_I8:
      case T_U8:
      case T_R4:
      case T_R8:
      case T_STRING:
      case T_OBJECT:
      case T_I:
      case T_U:
      case T_CLASS:
      case T_VALUETYPE:
      case T_SZARRAY:
      case T_ARRAY:
      case T_GENERICINST:
      case T_VAR:
      case T_MVAR:
      case T_PTR:
      case T_BYREF:
      case T_ENUM:
      case T_FNPTR:
      case T_TYPEDBYREF:
        return true;
      default:
        return false;
    }
  }

  bool testFieldsArray(uintptr_t base, uint32_t stride, uintptr_t klass) {
    // 单元素即可判定：FieldInfo.parent 必须回指本类（强约束）
    uintptr_t fi = base;
    if (!mem.probe(fi, 0x20)) return false;
    char nm[256];
    if (!mem.rdStr(mem.rdPtr(fi + lay.fName), nm, sizeof(nm), true)) return false;
    if (!NameLooksValid(nm, true)) return false;
    uintptr_t tp = mem.rdPtr(fi + lay.fType);
    if (!looksLikeType(tp)) return false;
    if (mem.rdPtr(fi + lay.fParent) != klass) return false;
    int32_t off = mem.rdI32(fi + lay.fOffset);
    if (off < -0x1000 || off > 0x100000) return false;
    return true;
  }

  bool testMethodsArray(uintptr_t base, uintptr_t klass, uint32_t nameOff,
                        uint32_t klassOff, uint32_t retOff) {
    for (int i = 0; i < 2; ++i) {
      uintptr_t mi = mem.rdPtr(base + (uintptr_t)i * 8);
      if (!mi || !mem.probe(mi, 0x58)) return false;
      char nm[256];
      if (!mem.rdStr(mem.rdPtr(mi + nameOff), nm, sizeof(nm), true)) return false;
      if (!NameLooksValid(nm, true)) return false;
      if (mem.rdPtr(mi + klassOff) != klass) return false;
      uintptr_t ret = mem.rdPtr(mi + retOff);
      if (!looksLikeType(ret)) return false;
      uintptr_t ptr = mem.rdPtr(mi + lay.mPtr);
      if (ptr) {
        if (ptr < 0x10000) return false;
        if (!mem.probe(ptr, 4)) return false;
      }
    }
    return true;
  }

  void calibrateMethodTail(const std::vector<uintptr_t> &samples) {
    struct Tail {
      uint32_t token, flags, iflags, slot, paramCount;
    };
    const Tail candidates[] = {
        {0x38, 0x3C, 0x3E, 0x40, 0x42},
        {0x40, 0x44, 0x46, 0x48, 0x4A},
        {0x48, 0x4C, 0x4E, 0x50, 0x52}};
    int bestScore = -1;
    Tail best = candidates[0];
    int tested = 0;
    for (const Tail &c : candidates) {
      int score = 0;
      int nonzeroTokens = 0;
      for (uintptr_t k : samples) {
        uintptr_t arr = mem.rdPtr(k + lay.cMethods);
        if (!arr) continue;
        uintptr_t mi = mem.rdPtr(arr);
        if (!mi || !mem.probe(mi, 0x58)) continue;
        ++tested;
        uint32_t token = mem.rdU32(mi + c.token);
        uint16_t flags = mem.rdU16(mi + c.flags);
        uint16_t iflags = mem.rdU16(mi + c.iflags);
        uint16_t slot = mem.rdU16(mi + c.slot);
        uint8_t pc = mem.rdU8(mi + c.paramCount);
        bool tokenOk = token == 0 || ((token & 0xFF000000u) == 0x06000000u);
        bool flagsOk = (flags & 0x8000u) == 0;
        bool iflagsOk = (iflags & 0xFFF0u) == 0;
        bool slotOk = slot == 0xFFFFu || slot < 0x8000u;
        bool pcOk = pc <= 64;
        if (token != 0) ++nonzeroTokens;
        if (tokenOk && flagsOk && iflagsOk && slotOk && pcOk) score += 2;
      }
      // A real MethodInfo table normally carries non-zero 0x06xxxxxx tokens;
      // this tie-break prevents an all-zero padding tail from winning.
      score += nonzeroTokens;
      if (score > bestScore) {
        bestScore = score;
        best = c;
      }
    }
    if (bestScore >= 0 && tested > 0) {
      lay.mToken = best.token;
      lay.mFlags = best.flags;
      lay.mIFlags = best.iflags;
      lay.mSlot = best.slot;
      lay.mParamCount = best.paramCount;
    }
    log->line("[cal] method_tail token=0x%X flags=0x%X iflags=0x%X slot=0x%X params=0x%X score=%d tested=%d",
              lay.mToken, lay.mFlags, lay.mIFlags, lay.mSlot, lay.mParamCount, bestScore, tested);
  }

  // TypeAttributes 主判据 = token 形态（0x02xxxxxx 只会出现在真正的 token 字段）
  void calibrateFlagsToken(const std::vector<uintptr_t> &samples) {
    const uint32_t cand[] = {0x100, 0x104, 0x108, 0x10C, 0x110, 0x114, 0x118, 0x11C,
                             0x120, 0x124, 0x128, 0x12C, 0x130, 0x134, 0x138, 0x140};
    int best = 0;
    uint32_t bestFlags = lay.cFlags;
    for (uint32_t fl : cand) {
      int sc = 0, tokHits = 0;
      for (uintptr_t k : samples) {
        uint32_t f = mem.rdU32(k + fl);
        uint32_t t = mem.rdU32(k + fl + 4);
        if ((t & 0xFF000000u) != 0x02000000u) continue;
        if ((t & 0x00FFFFFFu) == 0) continue;
        ++tokHits;
        if (f == 0) continue;
        if (f & 0xFF300000u) continue;  // 超出 TypeAttributes 已知位
        ++sc;
      }
      sc += tokHits;
      if (sc > best) {
        best = sc;
        bestFlags = fl;
      }
    }
    if (best > 0 && best * 2 >= (int)samples.size()) {
      lay.cFlags = bestFlags;
      lay.cToken = bestFlags + 4;
      lay.scoreFlags = best;
    }
    log->line("[cal] flags off=0x%X token off=0x%X score=%d/%d", lay.cFlags, lay.cToken, best,
              (int)samples.size());
  }

  // 参数数组布局：一次性全局判定，取代"逐参数猜测"
  bool paramAt(uintptr_t mi, uint32_t idx, int mode, uintptr_t &type, uintptr_t &namePtr) {
    type = 0;
    namePtr = 0;
    uintptr_t params = mem.rdPtr(mi + lay.mParams);
    if (!params) return false;
    if (mode == 0) {
      uintptr_t tp = mem.rdPtr(params + (uintptr_t)idx * 8);
      if (!looksLikeType(tp)) return false;
      type = tp;
      return true;
    }
    uintptr_t pi = params + (uintptr_t)idx * 0x18;
    uintptr_t np = mem.rdPtr(pi + 0x00);
    uintptr_t tp = mem.rdPtr(pi + (mode == 1 ? 0x10u : 0x08u));
    if (!looksLikeType(tp)) return false;
    type = tp;
    namePtr = np;
    return true;
  }

  void calibrateParamMode(const std::vector<uintptr_t> &samples) {
    int score[3] = {0, 0, 0};
    int tested = 0;
    for (uintptr_t k : samples) {
      uintptr_t arr = mem.rdPtr(k + lay.cMethods);
      uint32_t nm = mem.rdU16(k + lay.cMethodCount);
      if (nm > 4) nm = 4;
      for (uint32_t i = 0; i < nm; ++i) {
        uintptr_t mi = mem.rdPtr(arr + (uintptr_t)i * 8);
        if (!mi) continue;
        uint8_t pc = mem.rdU8(mi + lay.mParamCount);
        if (pc == 0 || pc > 16) continue;
        ++tested;
        for (int mode = 0; mode < 3; ++mode) {
          bool allOk = true;
          int nameOk = 0;
          for (uint32_t j = 0; j < pc; ++j) {
            uintptr_t tp = 0, np = 0;
            if (!paramAt(mi, j, mode, tp, np)) {
              allOk = false;
              break;
            }
            if (mode != 0 && np) {
              char tmp[256];
              if (mem.rdStr(np, tmp, sizeof(tmp), false) && NameLooksValid(tmp, false)) ++nameOk;
            }
          }
          if (!allOk) continue;
          score[mode] += 2;
          if (mode != 0) {
            // ParameterInfo 形态必然带名字；一个名字都读不到说明猜错了布局
            if (nameOk == 0) score[mode] -= 1;
            else score[mode] += nameOk;
          }
        }
      }
    }
    int best = 0;
    for (int i = 1; i < 3; ++i)
      if (score[i] > score[best]) best = i;
    paramMode_ = best;
    lay.scoreParamMode = score[best];
    log->line("[cal] parammode compact=%d pi10=%d pi08=%d -> mode=%d tested=%d", score[0], score[1],
              score[2], paramMode_, tested);
  }

  bool testPropsArray(uintptr_t base, uint32_t stride, uintptr_t klass) {
    // 单元素即可判定：PropertyInfo.parent 必须回指本类，且 get/set 至少一个有效
    uintptr_t pi = base;
    if (!mem.probe(pi, 0x28)) return false;
    char nm[256];
    if (!mem.rdStr(mem.rdPtr(pi + lay.pName), nm, sizeof(nm), true)) return false;
    if (!NameLooksValid(nm, true)) return false;
    if (mem.rdPtr(pi + lay.pParent) != klass) return false;
    uintptr_t g = mem.rdPtr(pi + lay.pGet);
    uintptr_t s = mem.rdPtr(pi + lay.pSet);
    if (!g && !s) return false;
    for (int j = 0; j < 2; ++j) {
      uintptr_t acc = j ? s : g;
      if (!acc) continue;
      if (!mem.probe(acc, 0x48)) return false;
      char an[256];
      if (!mem.rdStr(mem.rdPtr(acc + lay.mName), an, sizeof(an), true)) return false;
      if (!NameLooksValid(an, true)) return false;
    }
    return true;
  }

  // implementedInterfaces 的元素必须是「带 Interface 标志的合法 klass」
  bool looksLikeInterface(uintptr_t kk) {
    if (!kk || (kk & 7)) return false;
    char nm[256];
    if (!klassName(kk, nm, sizeof(nm))) return false;
    if (!NameLooksValid(nm, false)) return false;
    uint32_t fl = 0;
    if (!mem.rd(kk + lay.cFlags, fl)) return false;
    if (!(fl & TA_INTERFACE)) return false;
    if (fl & ~(uint32_t)0x001FFFFF) return false;
    if (indexAssembly(kk).empty()) return false;
    return true;
  }

  bool testIfaceArray(uintptr_t base, uint32_t stride) {
    // Only a locator hint: counts are independently scored below and every
    // declared element is validated again at output time.
    return looksLikeInterface(mem.rdPtr(base));
  }

  // 计数：用探测出的数组长度反推 u16 计数字段
  uint32_t detectArrayLen(uintptr_t base, uint32_t stride, int kind, uintptr_t klass) {
    uint32_t n = 0;
    for (uint32_t i = 0; i < 8192; ++i) {
      if (kind == 0) {  // fields
        uintptr_t fi = base + (uintptr_t)i * stride;
        if (!mem.probe(fi, 0x20)) break;
        char nm[256];
        if (!mem.rdStr(mem.rdPtr(fi + lay.fName), nm, sizeof(nm), true)) break;
        if (!NameLooksValid(nm, true)) break;
        if (!looksLikeType(mem.rdPtr(fi + lay.fType))) break;
        if (klass && mem.rdPtr(fi + lay.fParent) != klass) break;
      } else if (kind == 1) {  // methods (pointer array)
        uintptr_t mi = mem.rdPtr(base + (uintptr_t)i * 8);
        if (!mi || !mem.probe(mi, 0x48)) break;
        char nm[256];
        if (!mem.rdStr(mem.rdPtr(mi + lay.mName), nm, sizeof(nm), true)) break;
        if (!NameLooksValid(nm, true)) break;
        if (klass && mem.rdPtr(mi + lay.mKlass) != klass) break;
      } else if (kind == 2) {  // properties
        uintptr_t pi = base + (uintptr_t)i * stride;
        if (!mem.probe(pi, 0x28)) break;
        char nm[256];
        if (!mem.rdStr(mem.rdPtr(pi + lay.pName), nm, sizeof(nm), true)) break;
        if (!NameLooksValid(nm, true)) break;
        if (klass && mem.rdPtr(pi + lay.pParent) != klass) break;
      } else {  // interfaces
        uintptr_t kk = mem.rdPtr(base + (uintptr_t)i * stride);
        if (!looksLikeInterface(kk)) break;
      }
      ++n;
    }
    return n;
  }

  void calibrate() {
    std::vector<uintptr_t> samples;
    int ns = sampleClasses(samples, 96);
    log->line("[cal] sample classes = %d", ns);
    if (ns < 2) {
      lay.calibrated = false;
      return;
    }

    // Unity 2019 normally keeps these arrays at 0x80/0x90/0x98.  Hardened
    // game builds have been observed to move auxiliary pointers farther into
    // Il2CppClass, so include the complete post-header range as candidates.
    const uint32_t arrCand[] = {
        0x70, 0x78, 0x80, 0x88, 0x90, 0x98, 0xA0, 0xA8, 0xB0, 0xB8, 0xC0,
        0xC8, 0xD0, 0xD8, 0xE0, 0xE8, 0xF0, 0xF8, 0x100, 0x108, 0x110,
        0x118, 0x120, 0x128, 0x130, 0x138, 0x140, 0x148, 0x150, 0x158,
        0x160, 0x168, 0x170, 0x178, 0x180, 0x188, 0x190, 0x198, 0x1A0,
        0x1A8, 0x1B0, 0x1B8, 0x1C0, 0x1C8, 0x1D0, 0x1D8, 0x1E0, 0x1E8,
        0x1F0, 0x1F8, 0x200, 0x208, 0x210, 0x218, 0x220, 0x228, 0x230};
    const uint32_t strideCandF[] = {0x20, 0x28, 0x18};
    const uint32_t strideCandP[] = {0x28, 0x20, 0x30};

    // ---- fields ----
    {
      int best = 0;
      uint32_t bestOff = lay.cFields, bestStride = lay.fStride;
      for (uint32_t o : arrCand) {
        for (uint32_t s : strideCandF) {
          int sc = 0;
          for (uintptr_t k : samples) {
            uintptr_t p = mem.rdPtr(k + o);
            if (!p || !mem.probe(p, 0x20)) continue;
            if (testFieldsArray(p, s, k)) ++sc;
          }
          if (sc > best) {
            best = sc;
            bestOff = o;
            bestStride = s;
          }
        }
      }
      if (best >= 2) {
        lay.cFields = bestOff;
        lay.fStride = bestStride;
        lay.scoreFields = best;
      }
      log->line("[cal] fields off=0x%X stride=0x%X score=%d/%d", lay.cFields, lay.fStride, best, ns);
    }
    // ---- methods ----
    {
      struct MethodHead {
        uint32_t name, klass, ret, params;
      };
      const MethodHead heads[] = {
          {0x10, 0x18, 0x20, 0x28},  // Unity 2019: ptr, invoker, name
          {0x18, 0x20, 0x28, 0x30},  // newer: ptr, virtual ptr, invoker, name
          {0x20, 0x28, 0x30, 0x38}};
      int best = 0;
      uint32_t bestOff = lay.cMethods;
      MethodHead bestHead = heads[0];
      for (uint32_t o : arrCand) {
        for (const MethodHead &head : heads) {
          int sc = 0;
          for (uintptr_t k : samples) {
            uintptr_t p = mem.rdPtr(k + o);
            if (!p || !mem.probe(p, 8)) continue;
            if (testMethodsArray(p, k, head.name, head.klass, head.ret)) ++sc;
          }
          if (sc > best) {
            best = sc;
            bestOff = o;
            bestHead = head;
          }
        }
      }
      if (best >= 2) {
        lay.cMethods = bestOff;
        lay.mName = bestHead.name;
        lay.mKlass = bestHead.klass;
        lay.mRet = bestHead.ret;
        lay.mParams = bestHead.params;
        lay.scoreMethods = best;
      }
      log->line("[cal] methods off=0x%X head(name=0x%X klass=0x%X ret=0x%X params=0x%X) score=%d/%d",
                lay.cMethods, lay.mName, lay.mKlass, lay.mRet, lay.mParams, best, ns);
      // 方法尾部独立评分：即使数组命中率低也要校准（否则 token/flags/slot 错位）
      if (lay.scoreMethods >= 2) calibrateMethodTail(samples);
      else log->line("[cal] method_tail skipped (method array not validated)");
    }
    // ---- properties ----
    {
      int best = 0;
      uint32_t bestOff = lay.cProperties, bestStride = lay.pStride;
      for (uint32_t o : arrCand) {
        for (uint32_t s : strideCandP) {
          int sc = 0;
          for (uintptr_t k : samples) {
            uintptr_t p = mem.rdPtr(k + o);
            if (!p || !mem.probe(p, 0x28)) continue;
            if (testPropsArray(p, s, k)) ++sc;
          }
          if (sc > best) {
            best = sc;
            bestOff = o;
            bestStride = s;
          }
        }
      }
      if (best >= 2) {
        lay.cProperties = bestOff;
        lay.pStride = bestStride;
        lay.scoreProps = best;
      }
      log->line("[cal] props off=0x%X stride=0x%X score=%d/%d", lay.cProperties, lay.pStride, best, ns);
    }
    // ---- flags / token（必须早于 interfaces：接口判定依赖 TA_INTERFACE）----
    calibrateFlagsToken(samples);
    // ---- interfaces ----
    {
      int best = 0;
      uint32_t bestOff = lay.cInterfaces;
      for (uint32_t o : arrCand) {
        int sc = 0;
        for (uintptr_t k : samples) {
          uintptr_t p = mem.rdPtr(k + o);
          if (!p || !mem.probe(p, 16)) continue;
          if (testIfaceArray(p, 8)) ++sc;
        }
        if (sc > best) {
          best = sc;
          bestOff = o;
        }
      }
      if (best >= 2) {
        lay.cInterfaces = bestOff;
        lay.scoreIfaces = best;
      }
      log->line("[cal] ifaces off=0x%X score=%d/%d", lay.cInterfaces, best, ns);
    }

    // ---- 计数字段（u16）----
    struct CountCand {
      uint32_t off;
      int score;
    };
    struct CountProbe {
      uint32_t *outOff;
      int kind;
      uint32_t stride;
      uint32_t baseOff;
      const char *name;
      uint32_t baseDefault = 0;
      int lens = 0;
      std::vector<CountCand> cand;
    };
    CountProbe cps[4] = {
        {&lay.cFieldCount, 0, lay.fStride, lay.cFields, "field_count", 0x114, 0, {}},
        {&lay.cMethodCount, 1, 8, lay.cMethods, "method_count", 0x110, 0, {}},
        {&lay.cPropCount, 2, lay.pStride, lay.cProperties, "property_count", 0x112, 0, {}},
        {&lay.cIfaceCount, 3, 8, lay.cInterfaces, "iface_count", 0x11C, 0, {}},
    };
    for (CountProbe &cp : cps) {
      if (cp.kind == 3 && lay.scoreIfaces < 2) {
        log->line("[cal] %s skipped (interface array not validated, keep 0x%X)", cp.name,
                  *cp.outOff);
        continue;
      }
      std::vector<std::pair<uintptr_t, uint32_t>> lens;
      for (uintptr_t k : samples) {
        uintptr_t p = mem.rdPtr(k + cp.baseOff);
        if (!p) continue;
        uint32_t n = detectArrayLen(p, cp.stride, cp.kind, k);
        if (n == 0) continue;
        lens.push_back({k, n});
        if (lens.size() >= 120) break;
      }
      cp.lens = (int)lens.size();
      cp.cand.clear();
      if (lens.empty()) {
        log->line("[cal] %s off=0x%X (no samples, keep default)", cp.name, *cp.outOff);
        continue;
      }
      for (uint32_t o = 0xC8; o <= 0x220; o += 2) {
        int sc = 0;
        for (auto &pr : lens) {
          if (mem.rdU16(pr.first + o) == (uint16_t)pr.second) ++sc;
        }
        if (sc > 0) cp.cand.push_back({o, sc});
      }
      std::sort(cp.cand.begin(), cp.cand.end(), [&](const CountCand &a, const CountCand &b) {
        if (a.score != b.score) return a.score > b.score;
        // 同分时取最接近标准布局位置的候选（避免 flags 低 16 位等巧合值抢位）
        uint32_t da = (a.off > cp.baseDefault) ? a.off - cp.baseDefault : cp.baseDefault - a.off;
        uint32_t db = (b.off > cp.baseDefault) ? b.off - cp.baseDefault : cp.baseDefault - b.off;
        return da < db;
      });
    }
    // 四个计数字段互斥：按得分从高到低分配，已被占用的偏移不再复用
    // （旧版把 iface_count 误判成 field_count 的偏移就是这样产生的）
    {
      std::vector<int> order = {0, 1, 2, 3};
      std::sort(order.begin(), order.end(), [&](int a, int b) {
        int sa = cps[a].cand.empty() ? 0 : cps[a].cand[0].score;
        int sb = cps[b].cand.empty() ? 0 : cps[b].cand[0].score;
        return sa > sb;
      });
      std::vector<uint32_t> used;
      for (int idx : order) {
        CountProbe &cp = cps[idx];
        uint32_t chosen = *cp.outOff;
        int chosenScore = 0;
        for (const CountCand &c : cp.cand) {
          bool taken = false;
          for (uint32_t u : used)
            if (u == c.off) {
              taken = true;
              break;
            }
          if (taken) continue;
          chosen = c.off;
          chosenScore = c.score;
          break;
        }
        if (chosenScore >= 2) {
          *cp.outOff = chosen;
          used.push_back(chosen);
        }
        log->line("[cal] %s off=0x%X score=%d/%d", cp.name, *cp.outOff, chosenScore, cp.lens);
      }
    }

    refineInterfaceCount();

    // ---- 参数数组布局（依赖已经校准的方法数组、方法尾部和 method_count）----
    if (lay.scoreMethods >= 2) calibrateParamMode(samples);
    else log->line("[cal] parammode skipped (method array not validated)");

    // ---- parent 偏移校验 ----
    {
      int ok = 0, total = 0;
      for (uintptr_t k : samples) {
        uintptr_t p = mem.rdPtr(k + lay.cParent);
        ++total;
        if (!p) {
          ++ok;
          continue;
        }
        char nm[256];
        if (klassName(p, nm, sizeof(nm)) && NameLooksValid(nm, true)) ++ok;
      }
      log->line("[cal] parent off=0x%X ok=%d/%d", lay.cParent, ok, total);
      if (total >= 4 && ok * 2 < total) {
        // 尝试其它候选
        for (uint32_t o = 0x40; o <= 0x70; o += 8) {
          if (o == lay.cParent) continue;
          int ok2 = 0;
          for (uintptr_t k : samples) {
            uintptr_t p = mem.rdPtr(k + o);
            if (!p) continue;
            char nm[256];
            if (klassName(p, nm, sizeof(nm)) && NameLooksValid(nm, true)) ++ok2;
          }
          if (ok2 > ok) {
            ok = ok2;
            lay.cParent = o;
          }
        }
        log->line("[cal] parent -> 0x%X ok=%d/%d", lay.cParent, ok, total);
      }
    }

    lay.calibrated = (lay.scoreFields + lay.scoreMethods + lay.scoreProps) > 0;
    log->line("[cal] layout image=0x%X name=0x%X ns=0x%X parent=0x%X fields=0x%X/%X props=0x%X/%X methods=0x%X",
              lay.cImage, lay.cName, lay.cNamespace, lay.cParent, lay.cFields, lay.fStride,
              lay.cProperties, lay.pStride, lay.cMethods);
    log->line("[cal] counts method=0x%X prop=0x%X field=0x%X iface=0x%X", lay.cMethodCount,
              lay.cPropCount, lay.cFieldCount, lay.cIfaceCount);
    log->line("[cal] calibrated=%d", lay.calibrated ? 1 : 0);
  }

  // A nested-type count can accidentally match interface counts in a small
  // sample. Recheck with distributed classes; never pick a fixed Super offset.
  void refineInterfaceCount() {
    if (lay.scoreIfaces < 2) return;
    std::vector<uintptr_t> samples;
    const auto &source = typeTable_.empty() ? klasses_ : typeTable_;
    if (!source.empty()) {
      const size_t step = std::max<size_t>(1, source.size() / 2048);
      for (size_t i = 0; i < source.size() && samples.size() < 2048; i += step)
        if (source[i]) samples.push_back(source[i]);
    } else sampleClasses(samples, 512);
    std::vector<std::pair<uintptr_t, uint32_t>> lengths;
    for (uintptr_t k : samples) {
      const uintptr_t array = mem.rdPtr(k + lay.cInterfaces);
      if (!array) continue;
      uint32_t n = 0;
      while (n < 512 && looksLikeInterface(mem.rdPtr(array + (uintptr_t)n * 8))) ++n;
      if (n && n < 512) lengths.push_back({k, n});
    }
    if (lengths.size() < 8) return;
    struct Score { uint32_t offset; int score = 0, exact = 0; };
    auto score = [&](uint32_t off) {
      Score s{off};
      for (const auto &p : lengths) {
        uint16_t n = 0;
        if (!mem.rd(p.first + off, n)) { s.score -= 4; continue; }
        if (n == p.second) { s.score += 4; ++s.exact; }
        else s.score -= n > p.second ? 4 : 1;
      }
      return s;
    };
    const Score previous = score(lay.cIfaceCount);
    Score best = previous;
    for (uint32_t off = 0xC8; off <= 0x220; off += 2) {
      if (off == lay.cFieldCount || off == lay.cMethodCount || off == lay.cPropCount ||
          off == lay.cFlags || off == lay.cFlags + 2 || off == lay.cToken || off == lay.cToken + 2) continue;
      const Score candidate = score(off);
      if (candidate.score > best.score) best = candidate;
    }
    const bool replace = best.offset != previous.offset && best.exact >= 8 &&
        best.exact >= previous.exact + 4 && best.score - previous.score >= std::max<int>(8, (int)lengths.size() / 16);
    log->line("[接口校准] 扩展样本=%u 原偏移=0x%X 匹配=%d 得分=%d 候选=0x%X 匹配=%d 得分=%d 结果=%s",
      (unsigned)lengths.size(), previous.offset, previous.exact, previous.score,
      best.offset, best.exact, best.score, replace ? "证据改善，更新接口计数" : "保留原偏移");
    if (replace) lay.cIfaceCount = best.offset;
  }

  // ---------------- 镜像发现 ----------------
  static bool imageNameLooksValid(const char *name) {
    if (!name || !*name) return false;
    size_t n = strlen(name);
    if (n < 5 || n >= 128) return false;
    return (n >= 4 && _stricmp(name + n - 4, ".dll") == 0) ||
           (n >= 4 && _stricmp(name + n - 4, ".exe") == 0);
  }

  bool looksLikeAsciiPrefix(uintptr_t p) {
    if (!mem.cached(p, 4)) return false;
    uint32_t word = mem.rdU32(p);
    for (int i = 0; i < 4; ++i) {
      uint8_t c = (uint8_t)(word >> (i * 8));
      if (c < 0x20 || c > 0x7E) return false;
    }
    return true;
  }

  bool readImageHeaderName(uintptr_t p, char *out, size_t cap) {
    if (!p || (p & 7) || !mem.probe(p, 8)) return false;
    if (!mem.rdStr(mem.rdPtr(p), out, cap, true)) return false;
    return imageNameLooksValid(out);
  }

  bool looksLikeImage(uintptr_t p) {
    if (!p || (p & 7)) return false;
    if (!mem.probe(p, 0x48)) return false;
    char nm[128];
    if (!readImageHeaderName(p, nm, sizeof(nm))) return false;
    uint32_t typeCount = mem.rdU32(p + 0x1C);
    if (typeCount == 0 || typeCount > 400000) {
      // Some protected builds keep the class count at +0x18 and repurpose
      // the following word.  When image APIs are available, accept that
      // alternate header shape and let the API count be the final authority.
      uint32_t altCount = mem.rdU32(p + 0x18);
      if (!env || !env->scanAllMemoryForImages || altCount == 0 || altCount > 400000) return false;
    }
    // In Super mode, the compatibility APIs perform the authoritative final
    // validation. Do not require the normal assembly back-pointer or assume
    // that +0x18 is typeStart here.
    if (env && env->scanAllMemoryForImages && env->imageGetClassCount && env->imageGetClass)
      return true;
    if (mem.rdU32(p + 0x18) > 4000000) return false;
    uintptr_t asm_ = mem.rdPtr(p + 0x10);
    if (!asm_ || !mem.probe(asm_, 8)) return false;
    if (mem.rdPtr(asm_ + 0x00) != p) return false;
    return true;
  }

  void appendImageCandidate(uintptr_t p, std::vector<uintptr_t> &out) {
    if (!looksLikeImage(p)) return;
    for (uintptr_t e : out)
      if (e == p) return;
    if (out.size() < 8192) out.push_back(p);
  }

  void scanRangeForImages(uintptr_t lo, uintptr_t hi, std::vector<uintptr_t> &out) {
    if (hi <= lo) return;
    if (hi - lo > (512ull << 20)) hi = lo + (512ull << 20);
    uintptr_t p = (lo + 7) & ~(uintptr_t)7;
    uintptr_t qwords[2048];
    while (p + 8 <= hi && out.size() < 4096) {
      size_t n = (size_t)((hi - p) / 8);
      if (n > 2048) n = 2048;
      size_t bytes = n * 8;
      if (!mem.probe(p, bytes)) {
        if (!mem.probe(p, 8)) {
          p += 0x1000;
          continue;
        }
        n = 1;
        bytes = 8;
      }
      if (!SafeCopy(qwords, (const void *)p, bytes)) {
        p += bytes;
        continue;
      }
      for (size_t i = 0; i < n; ++i) {
        uintptr_t v = qwords[i];
        if (v >= 0x10000 && !(v & 7)) appendImageCandidate(v, out);
        // Also recognize an Il2CppImage stored directly in the scanned range:
        // its first qword is the image-name pointer, so the qword address is
        // the candidate. This covers protected registries that do not retain
        // a separate array of Il2CppImage* values.
        if (v >= 0x10000 && looksLikeAsciiPrefix(v)) {
          char directName[128];
          if (mem.rdStr(v, directName, sizeof(directName), true) && imageNameLooksValid(directName))
            appendImageCandidate(p + i * 8, out);
        }
      }
      p += bytes;
    }
  }

  void addAllocationRange(uintptr_t addr, std::vector<uintptr_t> &out) {
    MEMORY_BASIC_INFORMATION mbi;
    if (VirtualQuery((LPCVOID)addr, &mbi, sizeof(mbi)) == 0) return;
    if (mbi.State != MEM_COMMIT) return;
    uintptr_t lo = (uintptr_t)mbi.AllocationBase ? (uintptr_t)mbi.AllocationBase : (uintptr_t)mbi.BaseAddress;
    uintptr_t hi = (uintptr_t)mbi.BaseAddress + mbi.RegionSize;
    scanRangeForImages(lo, hi, out);
  }

  static bool isWritableProtect(DWORD protect) {
    const DWORD p = protect & 0xFFu;
    return p == PAGE_READWRITE || p == PAGE_WRITECOPY || p == PAGE_EXECUTE_READWRITE ||
           p == PAGE_EXECUTE_WRITECOPY;
  }

  void scanRangeForImagesFast(uintptr_t lo, uintptr_t hi, std::vector<uintptr_t> &out,
                              std::unordered_set<uintptr_t> &tested, uint64_t &qwordsScanned) {
    if (hi <= lo) return;
    uintptr_t p = (lo + 7) & ~(uintptr_t)7;
    uintptr_t qwords[4096];
    while (p + 8 <= hi) {
      size_t n = (size_t)((hi - p) / 8);
      if (n > 4096) n = 4096;
      size_t bytes = n * 8;
      if (!SafeCopy(qwords, (const void *)p, bytes)) {
        p += bytes;
        continue;
      }
      qwordsScanned += n;
      for (size_t i = 0; i < n; ++i) {
        uintptr_t v = qwords[i];
        if (v >= 0x10000 && !(v & 7) && mem.cached(v, 0x48)) {
          // Delay insertion into the dedupe set until the cheap pointer/name
          // tests pass; a 5 GB process can contain millions of unrelated
          // unique pointers.
          uintptr_t namep = mem.rdPtr(v);
          if (looksLikeAsciiPrefix(namep) && tested.insert(v).second)
            appendImageCandidate(v, out);
        }
        if (v >= 0x10000 && looksLikeAsciiPrefix(v)) {
          char directName[128];
          uintptr_t direct = p + i * 8;
          if (mem.rdStr(v, directName, sizeof(directName), true) &&
              imageNameLooksValid(directName) && tested.insert(direct).second)
            appendImageCandidate(direct, out);
        }
      }
      p += bytes;
    }
  }

  void scanAllPrivateMemoryForImages(std::vector<uintptr_t> &out) {
    std::vector<std::pair<uintptr_t, uintptr_t>> ranges;
    uintptr_t p = 0x10000;
    MEMORY_BASIC_INFORMATION mbi;
    size_t totalBytes = 0;
    size_t regionCount = 0;
    while (VirtualQuery((LPCVOID)p, &mbi, sizeof(mbi))) {
      uintptr_t lo = (uintptr_t)mbi.BaseAddress;
      uintptr_t hi = lo + mbi.RegionSize;
      if (mbi.State == MEM_COMMIT && !(mbi.Protect & (PAGE_GUARD | PAGE_NOACCESS)) && hi > lo) {
        // Cache every readable region so candidate-pointer checks do not
        // issue a VirtualQuery for each random qword.  Scan writable private
        // heaps where Il2CppImage/Il2CppAssembly allocations normally live;
        // module .data is already covered by the module-local scan above.
        mem.addRegion(lo, hi);
        ++regionCount;
        if (mbi.Type == MEM_PRIVATE && isWritableProtect(mbi.Protect)) {
          ranges.push_back({lo, hi});
          totalBytes += (size_t)(hi - lo);
        }
      }
      if (hi <= p) break;
      p = hi;
      if (regionCount > 200000) break;
    }
    std::unordered_set<uintptr_t> tested;
    tested.reserve(1 << 18);
    uint64_t qwordsScanned = 0;
    ULONGLONG started = GetTickCount64();
    for (auto &r : ranges) scanRangeForImagesFast(r.first, r.second, out, tested, qwordsScanned);
    ULONGLONG elapsed = GetTickCount64() - started;
    log->line("[scan] full private image scan regions=%llu bytes=0x%llX qwords=%llu tested=%llu candidates=%llu elapsedMs=%llu",
              (unsigned long long)ranges.size(), (unsigned long long)totalBytes,
              (unsigned long long)qwordsScanned, (unsigned long long)tested.size(),
              (unsigned long long)out.size(), (unsigned long long)elapsed);
  }

  // Unity-Offset's class-level RVA is the address of a GameAssembly global
  // slot whose qword value is the Il2CppClass*, not the heap address of the
  // Il2CppClass itself. Resolve the named targets, then scan this run's module
  // image for their live values. BuffManager and TransparentTagHandler also
  // receive a static singleton-chain validation result for the dump header.
  void discoverClassPointerSlots(const std::vector<uintptr_t> &images) {
    classPointerSlots_.clear();
    managerRvaResults_.clear();
    managerRvaResults_.resize(2);
    managerRvaResults_[0].className = "BuffManager";
    managerRvaResults_[0].instanceFieldOffset = 0x0;
    managerRvaResults_[1].className = "TransparentTagHandler";
    managerRvaResults_[1].instanceFieldOffset = 0x8;
    if (!opt || (!opt->classRvas && !opt->offsetSummary) || !env ||
        !env->moduleBase || !env->moduleSize) return;

    struct NamedTarget {
      const char *name;
      const char *namespaze;
      uintptr_t klass;
      uint64_t hits;
      std::vector<uintptr_t> slots;
      int managerIndex;
    };
    NamedTarget targets[] = {
        {"GameBaseObject", "", 0, 0, {}, -1},
        {"CharacterManager", "", 0, 0, {}, -1},
        {"UserData", "", 0, 0, {}, -1},
        {"CharactorSync", "MobileNetwork", 0, 0, {}, -1},
        {"AcSDKManager", "AntiCheatSDK", 0, 0, {}, -1},
        {"GameEntity", "GameLogic.Common", 0, 0, {}, -1},
        {"BuffManager", "", 0, 0, {}, 0},
        {"TransparentTagHandler", "", 0, 0, {}, 1},
    };
    const size_t targetCount = sizeof(targets) / sizeof(targets[0]);

    auto considerKlass = [&](uintptr_t k) {
      if (!k) return;
      char name[256] = {};
      char namespaze[256] = {};
      if (!klassName(k, name, sizeof(name))) return;
      mem.rdStr(mem.rdPtr(k + lay.cNamespace), namespaze, sizeof(namespaze), false);
      for (size_t i = 0; i < targetCount; ++i) {
        if (!targets[i].klass && strcmp(name, targets[i].name) == 0 &&
            strcmp(namespaze, targets[i].namespaze) == 0) {
          // A nested struct is also named UserData in this build. Select the
          // business class by its actual field, regardless of enumeration order.
          if (!strcmp(name, "UserData")) {
            bool hasBattleData = false;
            const uint32_t count = mem.rdU16(k + lay.cFieldCount);
            for (uint32_t f = 0; f < count && f < 4096; ++f) {
              char fn[256] = {};
              uintptr_t type = 0;
              int32_t off = 0;
              uint32_t attrs = 0;
              if (readField(k, f, fn, sizeof(fn), type, off, attrs) && !strcmp(fn, "battleData")) {
                hasBattleData = true;
                break;
              }
            }
            if (!hasBattleData) continue;
          }
          targets[i].klass = k;
          return;
        }
      }
    };

    for (uintptr_t k : typeTable_) considerKlass(k);
    for (uintptr_t k : klasses_) considerKlass(k);

    // A partial index table must not hide a target that the compatibility API
    // can still enumerate.
    bool missingClass = false;
    for (size_t i = 0; i < targetCount; ++i)
      if (!targets[i].klass) missingClass = true;
    if (missingClass && env->imageGetClass) {
      for (uintptr_t img : images) {
        size_t count = imageCount(img);
        for (size_t i = 0; i < count; ++i) {
          uintptr_t k = 0;
          if (SafeCallImageGetClass(env->imageGetClass, img, i, k)) considerKlass(k);
        }
      }
    }

    const bool usingOverrides = !env->overrideClassPointerSlots.empty();
    if (usingOverrides) {
      for (const auto &entry : env->overrideClassPointerSlots) {
        if (!entry.first || entry.second < env->moduleBase ||
            entry.second - env->moduleBase >= env->moduleSize)
          continue;
        classPointerSlots_[entry.first] = entry.second;
        for (size_t i = 0; i < targetCount; ++i) {
          if (targets[i].klass != entry.first) continue;
          ++targets[i].hits;
          targets[i].slots.push_back(entry.second);
          break;
        }
      }
      log->line("[class-rva] override slots=%llu",
                (unsigned long long)classPointerSlots_.size());
    }

    std::unordered_map<uintptr_t, size_t> targetByKlass;
    for (size_t i = 0; i < targetCount; ++i)
      if (targets[i].klass) targetByKlass[targets[i].klass] = i;
    if (targetByKlass.empty() && !usingOverrides) {
      log->line("[class-rva] no named target classes found");
      return;
    }

    auto isExecutableProtect = [](DWORD protect) {
      const DWORD p = protect & 0xFFu;
      return p == PAGE_EXECUTE || p == PAGE_EXECUTE_READ ||
             p == PAGE_EXECUTE_READWRITE || p == PAGE_EXECUTE_WRITECOPY;
    };
    uint64_t writableQwords = 0, readonlyQwords = 0;
    const uintptr_t moduleEnd = env->moduleBase + env->moduleSize;

    // Pass 1 scans writable image data, which is where initialized TypeInfo
    // globals normally live. Pass 2 checks non-executable read-only data only
    // for targets that were not found in pass 1.
    auto scanPass = [&](bool writablePass) {
      uintptr_t p = env->moduleBase;
      MEMORY_BASIC_INFORMATION mbi = {};
      uintptr_t qwords[4096];
      while (p < moduleEnd && VirtualQuery((LPCVOID)p, &mbi, sizeof(mbi))) {
        uintptr_t lo = (uintptr_t)mbi.BaseAddress;
        uintptr_t hi = lo + mbi.RegionSize;
        if (lo < env->moduleBase) lo = env->moduleBase;
        if (hi > moduleEnd) hi = moduleEnd;
        const bool committed = mbi.State == MEM_COMMIT &&
                               !(mbi.Protect & (PAGE_GUARD | PAGE_NOACCESS));
        const bool writable = isWritableProtect(mbi.Protect);
        const bool wantedProtection = writablePass ? writable
                                                   : (!writable && !isExecutableProtect(mbi.Protect));
        if (committed && wantedProtection && hi > lo) {
          uintptr_t cur = (lo + 7) & ~(uintptr_t)7;
          while (cur + 8 <= hi) {
            size_t count = (size_t)((hi - cur) / 8);
            if (count > 4096) count = 4096;
            const size_t bytes = count * 8;
            if (SafeCopy(qwords, (const void *)cur, bytes)) {
              if (writablePass) writableQwords += count;
              else readonlyQwords += count;
              for (size_t i = 0; i < count; ++i) {
                auto it = targetByKlass.find(qwords[i]);
                if (it == targetByKlass.end()) continue;
                NamedTarget &target = targets[it->second];
                if (!writablePass &&
                    classPointerSlots_.find(target.klass) != classPointerSlots_.end())
                  continue;
                ++target.hits;
                uintptr_t slot = cur + i * 8;
                auto old = classPointerSlots_.find(target.klass);
                if (old == classPointerSlots_.end() || slot < old->second)
                  classPointerSlots_[target.klass] = slot;
                if (target.slots.size() < 16) target.slots.push_back(slot);
              }
            }
            cur += bytes;
          }
        }
        if (hi <= p) break;
        p = hi;
      }
    };

    ULONGLONG elapsed = 0;
    if (!usingOverrides) {
      ULONGLONG started = GetTickCount64();
      scanPass(true);
      bool unresolved = false;
      for (const auto &entry : targetByKlass)
        if (classPointerSlots_.find(entry.first) == classPointerSlots_.end()) unresolved = true;
      if (unresolved) scanPass(false);
      elapsed = GetTickCount64() - started;
    }

    log->line("[class-rva] scan classes=%llu resolved=%llu writableQwords=%llu readonlyQwords=%llu elapsedMs=%llu",
              (unsigned long long)targetByKlass.size(),
              (unsigned long long)classPointerSlots_.size(),
              (unsigned long long)writableQwords,
              (unsigned long long)readonlyQwords,
              (unsigned long long)elapsed);
    for (size_t i = 0; i < targetCount; ++i) {
      if (!targets[i].klass) {
        log->line("[class-rva] %s class-not-found", targets[i].name);
        continue;
      }
      auto it = classPointerSlots_.find(targets[i].klass);
      if (it == classPointerSlots_.end()) {
        log->line("[class-rva] %s klass=0x%llx slot-not-found",
                  targets[i].name, (unsigned long long)targets[i].klass);
        continue;
      }
      log->line("[class-rva] %s klass=0x%llx slot=0x%llx rva=0x%llx candidates=%llu",
                targets[i].name, (unsigned long long)targets[i].klass,
                (unsigned long long)it->second,
                (unsigned long long)(it->second - env->moduleBase),
                (unsigned long long)targets[i].hits);
      if (!targets[i].slots.empty()) {
        char candidates[512] = {};
        size_t used = 0;
        for (uintptr_t slot : targets[i].slots) {
          const int n = snprintf(candidates + used, sizeof(candidates) - used,
                                 "%s0x%llx", used ? "," : "",
                                 (unsigned long long)(slot - env->moduleBase));
          if (n <= 0 || (size_t)n >= sizeof(candidates) - used) break;
          used += (size_t)n;
        }
        log->line("[class-rva] %s candidate-rvas=%s", targets[i].name, candidates);
      }
    }

    auto validateManager = [&](NamedTarget &target) {
      if (target.managerIndex < 0) return;
      ManagerRvaResult &result = managerRvaResults_[(size_t)target.managerIndex];
      result.klass = target.klass;
      result.foundClass = target.klass != 0;
      result.candidateCount = target.hits;
      result.staticFieldsOffset = lay.cInterfaces + 0x10;
      if (!target.klass) {
        log->line("[manager-rva] %s status=NOT_FOUND reason=class-not-found", target.name);
        return;
      }
      auto slot = classPointerSlots_.find(target.klass);
      if (slot == classPointerSlots_.end()) {
        log->line("[manager-rva] %s klass=0x%llx status=NOT_FOUND reason=slot-not-found",
                  target.name, (unsigned long long)target.klass);
        return;
      }
      result.slot = slot->second;
      result.rva = (uint64_t)(result.slot - env->moduleBase);
      result.foundSlot = true;

      std::vector<uint32_t> staticFieldOffsets;
      const uint32_t candidates[] = {lay.cInterfaces + 0x10, 0xB8, 0xC0, 0xC8, 0xD0};
      for (uint32_t off : candidates) {
        if (std::find(staticFieldOffsets.begin(), staticFieldOffsets.end(), off) ==
            staticFieldOffsets.end())
          staticFieldOffsets.push_back(off);
      }
      for (uint32_t off : staticFieldOffsets) {
        uintptr_t staticFields = mem.rdPtr(target.klass + off);
        if (!staticFields ||
            !mem.probe(staticFields, (size_t)result.instanceFieldOffset + sizeof(uintptr_t)))
          continue;
        uintptr_t instance = mem.rdPtr(staticFields + result.instanceFieldOffset);
        uintptr_t instanceKlass = 0;
        if (instance && mem.probe(instance, sizeof(uintptr_t)))
          instanceKlass = mem.rdPtr(instance);
        if (!result.staticFields) {
          result.staticFieldsOffset = off;
          result.staticFields = staticFields;
          result.instance = instance;
          result.instanceKlass = instanceKlass;
        }
        if (instance && instanceKlass == target.klass) {
          result.staticFieldsOffset = off;
          result.staticFields = staticFields;
          result.instance = instance;
          result.instanceKlass = instanceKlass;
          result.verified = true;
          break;
        }
      }
      log->line("[manager-rva] %s klass=0x%llx slot=0x%llx rva=0x%llx candidates=%llu staticFieldsOff=0x%X staticFields=0x%llx instanceOff=0x%X instance=0x%llx instanceKlass=0x%llx status=%s",
                target.name, (unsigned long long)result.klass,
                (unsigned long long)result.slot, (unsigned long long)result.rva,
                (unsigned long long)result.candidateCount, result.staticFieldsOffset,
                (unsigned long long)result.staticFields, result.instanceFieldOffset,
                (unsigned long long)result.instance,
                (unsigned long long)result.instanceKlass,
                result.verified ? "VERIFIED" : "CLASS_ONLY");
    };
    for (size_t i = 0; i < targetCount; ++i) validateManager(targets[i]);
  }

  void writeManagerRvaSummary(Writer &w) {
    if (!opt || !opt->classRvas || !env) return;
    const char *moduleName = env->moduleName.empty()
                                 ? (env->superVariant ? "GameAssembly_Super.dll" : "GameAssembly.dll")
                                 : env->moduleName.c_str();
    const char *suffix = env->superVariant ? "2" : "";
    w.puts("// ===== Runtime TypeInfo RVA Discovery =====\r\n");
    w.putf("// Module: %s\r\n", moduleName);
    w.putf("// Module Base: 0x%llX\r\n", (unsigned long long)env->moduleBase);
    for (const ManagerRvaResult &result : managerRvaResults_) {
      if (!result.foundSlot) {
        w.putf("// m_%s%s = NOT_FOUND; // reason=%s klass=0x%llX\r\n",
               result.className.c_str(), suffix,
               result.foundClass ? "slot-not-found" : "class-not-found",
               (unsigned long long)result.klass);
        continue;
      }
      w.putf("// m_%s%s = 0x%llX; // decimal: %llu status=%s\r\n",
             result.className.c_str(), suffix, (unsigned long long)result.rva,
             (unsigned long long)result.rva,
             result.verified ? "VERIFIED" : "CLASS_ONLY");
      w.putf("//   klass=0x%llX slot=0x%llX candidates=%llu staticFieldsOffset=0x%X staticFields=0x%llX instanceOffset=0x%X instance=0x%llX instanceKlass=0x%llX\r\n",
             (unsigned long long)result.klass, (unsigned long long)result.slot,
             (unsigned long long)result.candidateCount, result.staticFieldsOffset,
             (unsigned long long)result.staticFields, result.instanceFieldOffset,
             (unsigned long long)result.instance,
             (unsigned long long)result.instanceKlass);
    }
    w.puts("// ==========================================\r\n\r\n");
  }

  void writeOffsetSummary(Writer &w);

  // ---------------- Super：无导出引导 ----------------
  // 目标构建（GameAssembly_Super.dll）裁掉了全部 il2cpp 导出，因此不能用
  // get_corlib/image_get_class 引导。这里改为纯内存发现：
  //   ① 扫描全部可读区域，用「结构自校验」收集 Il2CppClass*
  //   ② 用 klass->token (0x02000000|row) 还原全局 TypeDefinitionIndex
  //   ③ 由 klass->image 去重推导镜像表
  // 自校验条件（不依赖任何导出）：
  //   name@+0x10 合法标识符 + byval_arg.data==klass 且 type∈{CLASS,VALUETYPE}
  //   + image 合法（名字以 .dll/.exe 结尾）
  bool looksLikeKlassStruct(uintptr_t k, char *nmOut, size_t cap) {
    if (!k || (k & 7)) return false;
    if (!mem.probe(k, 0x120)) return false;
    char nm[256];
    if (!klassName(k, nm, sizeof(nm))) return false;
    if (!NameLooksValid(nm, true)) return false;
    bool selfRef = false;
    const uint32_t cand[] = {0x20, 0x28, 0x30, 0x38};
    for (uint32_t off : cand) {
      uint32_t bits = mem.rdU32(k + off + lay.tBits);
      uint32_t te = (bits >> 16) & 0xFF;
      if ((te == T_CLASS || te == T_VALUETYPE) && mem.rdPtr(k + off + lay.tData) == k) {
        selfRef = true;
        break;
      }
    }
    if (!selfRef) return false;
    uintptr_t img = mem.rdPtr(k + lay.cImage);
    if (!img) return false;
    char inm[128];
    if (!mem.rdStr(mem.rdPtr(img + 0x00), inm, sizeof(inm), true)) return false;
    if (!strstr(inm, ".dll") && !strstr(inm, ".exe")) return false;
    if (nmOut && cap) snprintf(nmOut, cap, "%s", nm);
    return true;
  }

  // 1MB 粒度块集合：快速排除指向未提交区域的 qword
  struct BlockSet {
    std::vector<uint64_t> blocks;
    void build(const std::vector<std::pair<uintptr_t, uintptr_t>> &regions) {
      blocks.clear();
      for (auto &r : regions) {
        for (uint64_t b = r.first >> 20; b <= (r.second >> 20); ++b) blocks.push_back(b);
      }
      std::sort(blocks.begin(), blocks.end());
      blocks.erase(std::unique(blocks.begin(), blocks.end()), blocks.end());
    }
    bool has(uintptr_t v) const {
      return std::binary_search(blocks.begin(), blocks.end(), (uint64_t)(v >> 20));
    }
  };

  bool collectKlassesByScan() {
    std::vector<std::pair<uintptr_t, uintptr_t>> regions;
    uintptr_t p = 0x10000;
    MEMORY_BASIC_INFORMATION mbi;
    size_t totalBytes = 0;
    while (VirtualQuery((LPCVOID)p, &mbi, sizeof(mbi))) {
      uintptr_t lo = (uintptr_t)mbi.BaseAddress;
      uintptr_t hi = lo + mbi.RegionSize;
      if (mbi.State == MEM_COMMIT && !(mbi.Protect & (PAGE_GUARD | PAGE_NOACCESS)) && hi > lo) {
        mem.addRegion(lo, hi);
        regions.push_back({lo, hi});
        totalBytes += (size_t)(hi - lo);
      }
      if (hi <= p) break;
      p = hi;
      if (regions.size() > 200000) break;
    }
    BlockSet bs;
    bs.build(regions);
    log->line("[scan] regions=%llu bytes=0x%llX", (unsigned long long)regions.size(),
              (unsigned long long)totalBytes);

    std::unordered_set<uintptr_t> found;
    found.reserve(1 << 16);
    uintptr_t buf[1024];
    for (auto &r : regions) {
      uintptr_t q = (r.first + 7) & ~(uintptr_t)7;
      uintptr_t hi = r.second;
      while (q + 8 <= hi) {
        size_t n = (size_t)((hi - q) / 8);
        if (n > 1024) n = 1024;
        size_t bytes = n * 8;
        if (!mem.probe(q, bytes)) {
          if (!mem.probe(q, 8)) {
            q += 0x1000;
            continue;
          }
          n = 1;
          bytes = 8;
        }
        if (!SafeCopy(buf, (const void *)q, bytes)) {
          q += bytes;
          continue;
        }
        for (size_t i = 0; i < n; ++i) {
          uintptr_t v = buf[i];
          if (v < 0x10000 || (v & 7)) continue;
          if (!bs.has(v)) continue;
          if (found.count(v)) continue;  // 已确认，避免对每个实例重复校验
          if (looksLikeKlassStruct(v, nullptr, 0)) found.insert(v);
        }
        q += bytes;
      }
    }
    klasses_.assign(found.begin(), found.end());
    std::sort(klasses_.begin(), klasses_.end());
    log->line("[scan] klasses found = %llu", (unsigned long long)klasses_.size());
    return !klasses_.empty();
  }

  // 用 token 还原全局 TypeDefinitionIndex；同时按 image 边界交叉验证
  // 注意：klass->token = 0x02000000 | TypeDef 表行号（1-based），
  //      而全局 TypeDefinitionIndex 是 0-based，故 idx = row - 1。
  void buildIndexFromTokens() {
    typeTable_.clear();
    if (klasses_.empty()) return;
    size_t maxIdx = 0;
    for (uintptr_t k : klasses_) {
      uint32_t tok = mem.rdU32(k + lay.cToken);
      if ((tok & 0xFF000000u) != 0x02000000u) continue;
      uint32_t row = tok & 0x00FFFFFFu;
      if (row == 0) continue;
      if (row > maxIdx) maxIdx = row;
    }
    if (maxIdx == 0 || maxIdx > 4000000) {
      log->line("[tbl] token indexing unavailable -> unordered class set");
      return;
    }
    typeTable_.assign(maxIdx, 0);  // idx = row-1
    size_t filled = 0;
    for (uintptr_t k : klasses_) {
      uint32_t tok = mem.rdU32(k + lay.cToken);
      if ((tok & 0xFF000000u) != 0x02000000u) continue;
      uint32_t row = tok & 0x00FFFFFFu;
      if (row == 0 || (size_t)row > typeTable_.size()) continue;
      size_t idx = (size_t)row - 1;
      if (!typeTable_[idx]) {
        typeTable_[idx] = k;
        ++filled;
      }
    }
    log->line("[tbl] index from tokens: size=%llu filled=%llu",
              (unsigned long long)typeTable_.size(), (unsigned long long)filled);
  }

  void deriveImagesFromKlasses(std::vector<uintptr_t> &out) {
    out.clear();
    for (uintptr_t k : klasses_) {
      uintptr_t img = mem.rdPtr(k + lay.cImage);
      if (!img) continue;
      bool dup = false;
      for (uintptr_t e : out)
        if (e == img) {
          dup = true;
          break;
        }
      if (!dup) out.push_back(img);
    }
    std::sort(out.begin(), out.end(), [&](uintptr_t a, uintptr_t b) {
      return mem.rdU32(a + 0x18) < mem.rdU32(b + 0x18);
    });
    log->line("[scan] images derived from klasses = %llu", (unsigned long long)out.size());
  }

  bool discoverImages(std::vector<uintptr_t> &out) {
    out.clear();
    if (!env) return false;
    if (!env->overrideImages.empty()) {
      out = env->overrideImages;
      return !out.empty();
    }
    if (!env->getCorlib) return false;
    uintptr_t corlib = 0;
    if (!SafeCallGetCorlib(env->getCorlib, corlib) || !corlib) return false;
    out.push_back(corlib);

    // 1) corlib 所在分配块
    addAllocationRange(corlib, out);
    // 2) 每个已发现镜像所在分配块（BFS，最多 64 轮）
    for (size_t i = 0; i < out.size() && i < 64; ++i) addAllocationRange(out[i], out);
    // 2.5) Super/hardened builds may split the image registry over multiple
    // private allocations.  The ordinary corlib-local BFS then returns only
    // a handful of framework images.  Scan every readable region when the
    // caller explicitly requests the broader registry search.
    if (env->scanAllMemoryForImages) scanAllPrivateMemoryForImages(out);
    // 3) 模块镜像（.data 里可能有镜像数组/结构）
    if (env->moduleBase && env->moduleSize) {
      uintptr_t p = env->moduleBase;
      uintptr_t end = env->moduleBase + env->moduleSize;
      while (p < end) {
        MEMORY_BASIC_INFORMATION mbi;
        if (VirtualQuery((LPCVOID)p, &mbi, sizeof(mbi)) == 0) break;
        if (mbi.State == MEM_COMMIT && !(mbi.Protect & (PAGE_GUARD | PAGE_NOACCESS))) {
          uintptr_t rlo = (uintptr_t)mbi.BaseAddress;
          uintptr_t rhi = rlo + mbi.RegionSize;
          if (rhi > end) rhi = end;
          scanRangeForImages(rlo, rhi, out);
        }
        p = (uintptr_t)mbi.BaseAddress + mbi.RegionSize;
      }
    }
    // 4) 结构校验 + 名称/类数（此刻调用 il2cpp 导出是安全的）
    std::vector<uintptr_t> good;
    std::vector<std::string> names;
    const size_t structuralCandidates = out.size();
    size_t rejectedStructure = 0, rejectedName = 0, rejectedCount = 0, rejectedClass = 0;
    for (uintptr_t img : out) {
      if (!looksLikeImage(img)) {
        ++rejectedStructure;
        continue;
      }
      uintptr_t namePtr = 0;
      if (!SafeCallImageGetName(env->imageGetName, img, namePtr) || !namePtr) {
        ++rejectedName;
        continue;
      }
      char nmBuf[128];
      if (!mem.rdStr(namePtr, nmBuf, sizeof(nmBuf), true) || !imageNameLooksValid(nmBuf)) {
        ++rejectedName;
        continue;
      }
      size_t cnt = 0;
      if (!SafeCallImageGetClassCount(env->imageGetClassCount, img, cnt) || cnt == 0 ||
          cnt > 400000) {
        ++rejectedCount;
        continue;
      }
      // API name/count alone can accept a lookalike header because these
      // getters are shallow. Cross-check one returned class and its image
      // backlink before allowing the candidate into the dump.
      uintptr_t firstClass = 0;
      char firstName[256];
      if (!SafeCallImageGetClass(env->imageGetClass, img, 0, firstClass) || !firstClass ||
          mem.rdPtr(firstClass + lay.cImage) != img ||
          !klassName(firstClass, firstName, sizeof(firstName)) ||
          !NameLooksValid(firstName, true)) {
        ++rejectedClass;
        continue;
      }
      bool dup = false;
      for (auto &s : names)
        if (s == nmBuf) {
          dup = true;
          break;
        }
      if (dup) continue;
      names.push_back(nmBuf);
      good.push_back(img);
      imageCounts_[img] = cnt;
    }
    out.swap(good);
    log->line("[scan] image validation structural=%llu apiAccepted=%llu reject(struct=%llu name=%llu count=%llu class=%llu)",
              (unsigned long long)structuralCandidates, (unsigned long long)out.size(),
              (unsigned long long)rejectedStructure, (unsigned long long)rejectedName,
              (unsigned long long)rejectedCount, (unsigned long long)rejectedClass);
    return !out.empty();
  }

  // ---------------- 成员读取 ----------------
  bool rejectMember(MemberIssue why, uint32_t idx, uintptr_t address, const char *name = "") {
    if (memberAudit_) memberAudit_->reject(why, idx, address, name);
    return false;
  }
  bool memberOwnerMatches(uintptr_t owner, uintptr_t klass) {
    if (owner == klass) return true;
    // Inflated generic members can retain their type definition's owner.
    const uintptr_t handle = mem.rdPtr(klass + lay.cTypeDefHandle);
    return owner && handle && mem.rdPtr(owner + lay.cTypeDefHandle) == handle;
  }
  bool memberName(uintptr_t pointer, char *name, size_t cap, uint32_t idx, uintptr_t record) {
    name[0] = 0;
    if (!pointer || !mem.probe(pointer, 1)) return rejectMember(MemberIssue::Unreadable, idx, record);
    if (!mem.rdStr(pointer, name, cap, false)) return rejectMember(MemberIssue::InvalidName, idx, record, name);
    if (!ValidUtf8(name)) return rejectMember(MemberIssue::InvalidUtf8, idx, record, name);
    if (!NameLooksValid(name, false)) return rejectMember(MemberIssue::InvalidName, idx, record, name);
    return true;
  }
  uint32_t prepareMembers(uintptr_t klass, uint32_t arrayOffset, uint32_t raw, uint32_t limit,
                          bool enabled, MemberAudit &a) {
    if (!enabled) return 0;
    a.raw = raw;
    const uint32_t count = limit ? std::min(raw, limit) : raw;
    a.limited = raw - count;
    if (!count) return 0;
    uintptr_t array = 0;
    if (!mem.rd(klass + arrayOffset, array)) {
      a.reject(MemberIssue::Unreadable, 0, klass + arrayOffset, "", count);
      return 0;
    }
    if (!array) {
      a.reject(MemberIssue::NullArray, 0, klass + arrayOffset, "", count);
      return 0;
    }
    if ((array & 7) || !mem.probe(array, 8)) {
      a.reject(MemberIssue::Unreadable, 0, array, "", count);
      return 0;
    }
    return count;
  }
  void finishMembers(uintptr_t klass, const char *name, const char *category, MemberAudit &a, uint64_t exported) {
    memberAudit_ = nullptr;
    a.exported = static_cast<uint32_t>(exported);
    indexes_.diagnostics(category, a);
    if (!a.issue()) return;
    ++quality.groups;
    quality.rejected += a.rejectedCount();
    quality.limited += a.limited;
    // The TSV retains every affected category; keep the main log bounded.
    if (quality.groups <= 40)
      log->line("[质量] 类=%s VA=0x%llX 类别=%s 原始=%u 尝试=%u 导出=%u 上限跳过=%u 拒绝=%u 首因=%s 首项=%u 地址=0x%llX 名称=%s",
        name, (unsigned long long)klass, category, a.raw, a.attempted, a.exported, a.limited,
        a.rejectedCount(), a.firstReason.empty() ? "配置数量上限" : a.firstReason.c_str(),
        a.firstIndex, (unsigned long long)a.firstAddress, a.firstName.c_str());
  }

  bool readField(uintptr_t klass, uint32_t idx, char *fname, size_t fcap, uintptr_t &ftype,
                 int32_t &foff, uint32_t &fattr) {
    uintptr_t arr = mem.rdPtr(klass + lay.cFields);
    if (!arr) return rejectMember(MemberIssue::NullArray, idx, klass + lay.cFields);
    uintptr_t fi = arr + (uintptr_t)idx * lay.fStride;
    if (!mem.probe(fi, lay.fStride)) return rejectMember(MemberIssue::Unreadable, idx, fi);
    if (!memberName(mem.rdPtr(fi + lay.fName), fname, fcap, idx, fi)) return false;
    if (!memberOwnerMatches(mem.rdPtr(fi + lay.fParent), klass))
      return rejectMember(MemberIssue::OwnerMismatch, idx, fi, fname);
    ftype = mem.rdPtr(fi + lay.fType);
    if (!looksLikeType(ftype)) return rejectMember(MemberIssue::InvalidType, idx, fi, fname);
    foff = mem.rdI32(fi + lay.fOffset);
    fattr = mem.rdU32(ftype + lay.tBits) & 0xFFFF;  // Il2CppType.attrs = FieldAttributes
    return true;
  }

  bool readMethod(uintptr_t klass, uint32_t idx, uintptr_t &mi, char *mname, size_t mcap) {
    uintptr_t arr = mem.rdPtr(klass + lay.cMethods);
    if (!arr) return rejectMember(MemberIssue::NullArray, idx, klass + lay.cMethods);
    mi = mem.rdPtr(arr + (uintptr_t)idx * 8);
    if (!mi || !mem.probe(mi, std::max(lay.mParamCount + 1, lay.mKlass + 8)))
      return rejectMember(MemberIssue::Unreadable, idx, mi ? mi : arr + (uintptr_t)idx * 8);
    if (!memberName(mem.rdPtr(mi + lay.mName), mname, mcap, idx, mi)) return false;
    if (!memberOwnerMatches(mem.rdPtr(mi + lay.mKlass), klass))
      return rejectMember(MemberIssue::OwnerMismatch, idx, mi, mname);
    if (!looksLikeType(mem.rdPtr(mi + lay.mRet))) return rejectMember(MemberIssue::InvalidType, idx, mi, mname);
    return true;
  }

  // MethodInfo::parameters is ParameterInfo[] in stock IL2CPP (name at +0,
  // type at +8, 0x18-byte stride).  A few stripped builds expose an older
  // compact Il2CppType*[] representation instead.  Accept both layouts and
  // let the plausibility checks decide per method.
  bool readMethodParam(uintptr_t params, uint32_t index, uintptr_t &type,
                       const char **runtimeName) {
    type = 0;
    if (runtimeName) *runtimeName = nullptr;
    if (!params) return false;

    // The compact form is unambiguous when its first word points at a valid
    // Il2CppType.  Check it before probing ParameterInfo so a later compact
    // element is not mistaken for the +0x10 type word of an Info record.
    uintptr_t compact = mem.rdPtr(params + (uintptr_t)index * 8);
    if (looksLikeType(compact)) {
      type = compact;
      return true;
    }

    const uintptr_t pi = params + (uintptr_t)index * 0x18;
    uintptr_t namep = mem.rdPtr(pi + 0x00);
    // Current Unity ParameterInfo keeps Il2CppType* at +0x10; older
    // runtimes used +0x08, so retain that layout as a fallback.
    uintptr_t typed = mem.rdPtr(pi + 0x10);
    if (!looksLikeType(typed)) typed = mem.rdPtr(pi + 0x08);
    if (looksLikeType(typed)) {
      type = typed;
    } else {
      return false;
    }
    if (runtimeName && namep) {
      char tmp[256];
      if (mem.rdStr(namep, tmp, sizeof(tmp), false) && NameLooksValid(tmp, false)) {
        // The returned pointer is only used as an opaque marker; the caller
        // reads it once more into its own output buffer.
        *runtimeName = reinterpret_cast<const char *>(namep);
      }
    }
    return true;
  }

  bool readProperty(uintptr_t klass, uint32_t idx, uintptr_t &pi, char *pname, size_t pcap) {
    uintptr_t arr = mem.rdPtr(klass + lay.cProperties);
    if (!arr) return rejectMember(MemberIssue::NullArray, idx, klass + lay.cProperties);
    pi = arr + (uintptr_t)idx * lay.pStride;
    if (!mem.probe(pi, lay.pStride)) return rejectMember(MemberIssue::Unreadable, idx, pi);
    if (!memberName(mem.rdPtr(pi + lay.pName), pname, pcap, idx, pi)) return false;
    if (!memberOwnerMatches(mem.rdPtr(pi + lay.pParent), klass))
      return rejectMember(MemberIssue::OwnerMismatch, idx, pi, pname);
    return true;
  }

  // ---------------- 输出 ----------------
  void writeField(Writer &w, uintptr_t klass, uint32_t idx) {
    char nm[512];
    uintptr_t tp = 0;
    int32_t off = 0;
    uint32_t attr = 0;
    if (!readField(klass, idx, nm, sizeof(nm), tp, off, attr)) return;
    char tn[512];
    typeName(tp, tn, sizeof(tn), 0);
    char mods[64];
    fieldMods(attr, mods, sizeof(mods));
    if (attr & FA_LITERAL) {
      w.putf("\t%s%s %s; // size:0x%X\r\n", mods, tn, nm, typeSize(tp));
    } else {
      w.putf("\t%s%s %s; // offset:0x%X\r\n", mods, tn, nm, (uint32_t)off);
    }
    ++st.fields;
  }

  void writeMethod(Writer &w, uintptr_t klass, uint32_t idx, uintptr_t typeDefHandle) {
    uintptr_t mi = 0;
    char nm[512];
    if (!readMethod(klass, idx, mi, nm, sizeof(nm))) return;

    uintptr_t ptr = mem.rdPtr(mi + lay.mPtr);
    uintptr_t ret = mem.rdPtr(mi + lay.mRet);
    uint16_t flags = mem.rdU16(mi + lay.mFlags);
    uint16_t slot = mem.rdU16(mi + lay.mSlot);
    uint8_t pc = mem.rdU8(mi + lay.mParamCount);
    if (pc > 64) pc = 64;
    uint32_t token = mem.rdU32(mi + lay.mToken);

    if (ptr) {
      uintptr_t base = env->moduleBase;
      uintptr_t rva = (base && ptr >= base) ? (ptr - base) : ptr;
      if (slot != 0xFFFF)
        w.putf("\t// RVA: 0x%llx VA: 0x%llx Slot: %u\r\n", (unsigned long long)rva,
               (unsigned long long)ptr, (unsigned)slot);
      else
        w.putf("\t// RVA: 0x%llx VA: 0x%llx\r\n", (unsigned long long)rva, (unsigned long long)ptr);
    } else {
      ++st.rvaZero;
      if (slot != 0xFFFF)
        w.putf("\t// RVA: 0x VA: 0x0 Slot: %u\r\n", (unsigned)slot);
      else
        w.puts("\t// RVA: 0x VA: 0x0\r\n");
    }

    char mods[96];
    methodMods(flags, mods, sizeof(mods));
    char rn[512];
    typeName(ret, rn, sizeof(rn), 0);
    std::string signature = std::string(mods) + rn + " " + nm + "(";

    uintptr_t mdef = 0;
    if (opt->useMetadata && meta.ok && typeDefHandle) mdef = meta.findMethodDef(typeDefHandle, token);

    for (uint32_t i = 0; i < pc; ++i) {
      if (i) signature += ", ";
      uintptr_t ptp = 0, namePtr = 0;
      bool okParam = paramAt(mi, i, paramMode_, ptp, namePtr);
      uint32_t pbits = okParam ? mem.rdU32(ptp + lay.tBits) : 0u;
      bool byref = (pbits & 0x40000000u) != 0;
      uint16_t pattr = (uint16_t)(pbits & 0xFFFF);
      char pn[512];
      bool haveName = false;
      if (mdef) {
        char tmp[512];
        if (meta.paramName(mdef, (int)i, tmp, sizeof(tmp)) && NameLooksValid(tmp, false)) {
          snprintf(pn, sizeof(pn), "%s", tmp);
          haveName = true;
          ++st.metaParams;
        }
      }
      if (!haveName && namePtr) {
        if (mem.rdStr(namePtr, pn, sizeof(pn), false) && NameLooksValid(pn, false)) {
          haveName = true;
          ++st.runtimeParamNames;
        }
      }
      if (!haveName) {
        snprintf(pn, sizeof(pn), "p%u", i);
        ++st.synthParams;
      }
      char tn[512];
      uintptr_t realTp = ptp;
      if (byref && ptp) realTp = mem.rdPtr(ptp + lay.tData);  // BYREF -> element type
      if (!okParam) {
        snprintf(tn, sizeof(tn), "?");
      } else {
        typeName(realTp, tn, sizeof(tn), 0);
      }
      if (byref && opt->outHeuristic && (pattr & PA_OUT)) signature += "out ";
      else if (byref) signature += "ref ";
      signature += tn;
      signature += " ";
      signature += pn;
    }
    signature += ")";
    w.puts("\t");
    w.puts(signature.c_str());
    w.puts(" { }\r\n");
    if (indexes_.active()) {
      IndexMethod entry;
      entry.address = ptr; entry.info = mi; entry.token = token;
      entry.flags = flags; entry.slot = slot; entry.name = nm;
      entry.signature = std::move(signature);
      indexes_.method(std::move(entry));
    }
    ++st.methods;
  }

  void writeProperty(Writer &w, uintptr_t klass, uint32_t idx) {
    uintptr_t pi = 0;
    char nm[512];
    if (!readProperty(klass, idx, pi, nm, sizeof(nm))) return;
    uintptr_t get = mem.rdPtr(pi + lay.pGet);
    uintptr_t set = mem.rdPtr(pi + lay.pSet);
    uintptr_t acc = get ? get : set;
    if (!acc || !mem.probe(acc, lay.mParamCount + 1)) {
      rejectMember(MemberIssue::MissingAccessor, idx, pi, nm);
      return;
    }
    uint16_t flags = mem.rdU16(acc + lay.mFlags);
    uintptr_t rt = 0;
    if (get) {
      rt = mem.rdPtr(get + lay.mRet);
    } else {
      uint8_t pc = mem.rdU8(set + lay.mParamCount);
      if (pc >= 1) {
        uintptr_t ptp = 0, np = 0;
        if (paramAt(set, (uint32_t)(pc - 1), paramMode_, ptp, np)) {
          uint32_t bits = mem.rdU32(ptp + lay.tBits);
          rt = ((bits & 0x40000000u) != 0) ? mem.rdPtr(ptp + lay.tData) : ptp;
        }
      }
    }
    char tn[512];
    typeName(rt, tn, sizeof(tn), 0);
    char mods[96];
    methodMods(flags, mods, sizeof(mods));
    if (get && set) w.putf("\t%s%s %s { get; set; }\r\n", mods, tn, nm);
    else if (get) w.putf("\t%s%s %s { get; }\r\n", mods, tn, nm);
    else w.putf("\t%s%s %s { set; }\r\n", mods, tn, nm);
    ++st.props;
  }

  void writeClass(Writer &w, uintptr_t k, uint32_t typeDefIndex) {
    char nm[512];
    if (!klassName(k, nm, sizeof(nm)) || !NameLooksValid(nm, false)) {
      ++st.skippedClasses;
      ++quality.skippedClasses;
      if (quality.skippedClasses <= 16)
        log->line("[质量] 跳过名称无效或不可读的类型：VA=0x%llX 名称=%s", (unsigned long long)k, nm);
      return;
    }
    char ns[512];
    ns[0] = 0;
    mem.rdStr(mem.rdPtr(k + lay.cNamespace), ns, sizeof(ns), false);

    uint32_t flags = mem.rdU32(k + lay.cFlags);
    uintptr_t parent = mem.rdPtr(k + lay.cParent);
    char parentName[512];
    parentName[0] = 0;
    bool haveParent = parent && klassName(parent, parentName, sizeof(parentName));

    const char *kind = "class";
    if (flags & TA_INTERFACE) kind = "interface";
    else if (haveParent && strcmp(parentName, "Enum") == 0) kind = "enum";
    else if (haveParent && strcmp(parentName, "ValueType") == 0) kind = "struct";

    const Stats before = st;
    if (indexes_.active()) {
      IndexClass entry;
      entry.klass = k; entry.image = mem.rdPtr(k + lay.cImage);
      entry.parent = parent; entry.typeIndex = typeDefIndex; entry.flags = flags;
      entry.assembly = indexAssembly(k); entry.namespaze = ns; entry.name = nm; entry.kind = kind;
      if (haveParent) {
        const std::string parentNs = indexNamespace(parent);
        entry.parentName = parentNs.empty() ? parentName : parentNs + "." + parentName;
      }
      entry.sizeOffset = lay.cInstanceSize;
      entry.sizeReadable = mem.rd(k + lay.cInstanceSize, entry.rawSize);
      entry.rawFields = mem.rdU16(k + lay.cFieldCount);
      entry.rawMethods = mem.rdU16(k + lay.cMethodCount);
      entry.rawProps = mem.rdU16(k + lay.cPropCount);
      entry.rawIfaces = mem.rdU16(k + lay.cIfaceCount);
      indexes_.beginClass(std::move(entry));
    }

    w.putf("// Class: %s \r\n", nm);
    w.putf("// Namespace: %s\r\n", ns);
    if (opt->printInstance) w.putf("// Instance: 0x%llx\r\n", (unsigned long long)k);
    auto classSlot = classPointerSlots_.find(k);
    if (opt->classRvas && classSlot != classPointerSlots_.end() &&
        env && classSlot->second >= env->moduleBase &&
        classSlot->second - env->moduleBase < env->moduleSize) {
      const uint64_t rva = (uint64_t)(classSlot->second - env->moduleBase);
      w.putf("// RVA: 0x%llx\r\n", (unsigned long long)rva);
      w.putf("// RVA: %llu\r\n", (unsigned long long)rva);
      ++st.classRvas;
    }
    if (flags & TA_SERIALIZABLE) w.puts("[Serializable]\r\n");

    char mods[64];
    classMods(flags, kind, mods, sizeof(mods));
    w.putf("%s%s %s", mods, kind, nm);

    // 基类 + 接口
    bool anyBase = false;
    if (strcmp(kind, "interface") != 0 && strcmp(kind, "enum") != 0) {
      if (haveParent && strcmp(parentName, "Object") != 0 && strcmp(parentName, "ValueType") != 0 &&
          strcmp(parentName, "Enum") != 0 && parentName[0]) {
        w.putf(" : %s", parentName);
        anyBase = true;
      }
    }
    if (strcmp(kind, "enum") != 0) {
      uintptr_t ifArr = mem.rdPtr(k + lay.cInterfaces);
      MemberAudit audit;
      const uint32_t nIf = prepareMembers(k, lay.cInterfaces, mem.rdU16(k + lay.cIfaceCount), opt->maxIfaces, true, audit);
      std::unordered_set<uintptr_t> accepted;
      memberAudit_ = &audit;
      for (uint32_t i = 0; i < nIf; ++i) {
          ++audit.attempted;
          const uintptr_t address = ifArr + (uintptr_t)i * 8;
          uintptr_t ik = 0;
          if (!mem.rd(address, ik) || !ik) { rejectMember(MemberIssue::Unreadable, i, address); continue; }
          char in[512] = {};
          if (!memberName(mem.rdPtr(ik + lay.cName), in, sizeof(in), i, ik)) continue;
          if (!looksLikeInterface(ik)) { rejectMember(MemberIssue::NotInterface, i, ik, in); continue; }
          if (ik == k || ik == parent || !accepted.insert(ik).second) { ++audit.filtered; continue; }
          w.putf(anyBase ? ", %s" : " : %s", in);
          anyBase = true;
          ++st.ifaces;
          if (indexes_.active())
            indexes_.interfaceEntry(i, ik, indexAssembly(ik), indexNamespace(ik), in, true);
      }
      finishMembers(k, nm, "接口", audit, st.ifaces - before.ifaces);
    }
    w.putf(" // TypeDefIndex: %x\r\n", typeDefIndex);

    MemberAudit fieldAudit, methodAudit, propAudit;
    const uint32_t nFields = prepareMembers(k, lay.cFields, mem.rdU16(k + lay.cFieldCount), opt->maxFields, opt->fields, fieldAudit);
    const uint32_t nMeth = prepareMembers(k, lay.cMethods, mem.rdU16(k + lay.cMethodCount), opt->maxMethods, opt->methods, methodAudit);
    const uint32_t nProp = prepareMembers(k, lay.cProperties, mem.rdU16(k + lay.cPropCount), opt->maxProps, opt->properties, propAudit);

    if (nFields == 0 && nProp == 0 && nMeth == 0) {
      w.puts("{}\r\n\r\n");
      finishMembers(k, nm, "字段", fieldAudit, 0);
      finishMembers(k, nm, "方法", methodAudit, 0);
      finishMembers(k, nm, "属性", propAudit, 0);
      ++st.classes;
      indexes_.endClass(0, 0, 0, st.ifaces - before.ifaces);
      return;
    }
    w.puts("{\r\n");
    bool wrote = false;
    if (nFields) {
      w.puts("\t// Fields\r\n");
      memberAudit_ = &fieldAudit;
      for (uint32_t i = 0; i < nFields; ++i) { ++fieldAudit.attempted; writeField(w, k, i); }
      wrote = true;
    }
    if (nProp) {
      if (wrote) w.puts("\r\n");
      w.puts("\t// Properties\r\n");
      memberAudit_ = &propAudit;
      for (uint32_t i = 0; i < nProp; ++i) { ++propAudit.attempted; writeProperty(w, k, i); }
      wrote = true;
    }
    if (nMeth) {
      if (wrote) w.puts("\r\n");
      w.putf("\t// Methods:%u\r\n", nMeth);
      uintptr_t tdh = mem.rdPtr(k + lay.cTypeDefHandle);
      memberAudit_ = &methodAudit;
      for (uint32_t i = 0; i < nMeth; ++i) { ++methodAudit.attempted; writeMethod(w, k, i, tdh); }
    }
    finishMembers(k, nm, "字段", fieldAudit, st.fields - before.fields);
    finishMembers(k, nm, "方法", methodAudit, st.methods - before.methods);
    finishMembers(k, nm, "属性", propAudit, st.props - before.props);
    w.puts("}\r\n\r\n");
    ++st.classes;
    indexes_.endClass(st.fields - before.fields, st.methods - before.methods,
                      st.props - before.props, st.ifaces - before.ifaces);
  }

  // ---------------- 主流程 ----------------
  bool run() {
    if (!env || !opt || !log) return false;
    quality = {};
    memberAudit_ = nullptr;
    meta.mem = &mem;
    imageCounts_.clear();
    imageStarts_.clear();
    classIndices_.clear();
    classPointerSlots_.clear();
    managerRvaResults_.clear();
    typeHandleToKlass_.clear();
    imageLayoutProfile_.clear();

    std::vector<uintptr_t> images;
    if (env->getCorlib) {
      if (!discoverImages(images)) {
        log->line("[FAIL] no images discovered");
        return false;
      }
    } else {
      // ---- Super：无导出引导 ----
      superMode_ = true;
      log->line("[0] super mode: no bootstrap exports -> pure memory scan");
      if (!collectKlassesByScan()) {
        log->line("[FAIL] no Il2CppClass found by scan");
        return false;
      }
      // 先校准（token 偏移未知时无法建索引），再按 token 建索引表
      calibrate();
      buildIndexFromTokens();
      deriveImagesFromKlasses(images);
      if (images.empty()) {
        log->line("[FAIL] no images derived from klasses");
        return false;
      }
    }
    prepareImageLayout(images);
    if (env->getCorlib && !buildIndexMapFromClassTypeData(images) &&
        !buildIndexMapFromClassTypeHandles(images))
      buildIndexMapFromImages(images);
    images_ = images;
    log->line("[1] images = %u", (unsigned)images.size());
    for (uintptr_t img : images) {
      char nm[128];
      mem.rdStr(mem.rdPtr(img + 0x00), nm, sizeof(nm), false);
      size_t cnt = imageCount(img);
      log->line("    img=0x%llx name='%s' start=%u count=%llu raw18=%u raw1C=%u",
                (unsigned long long)img, nm, imageStart(img), (unsigned long long)cnt,
                mem.rdU32(img + 0x18), mem.rdU32(img + 0x1C));
    }

    if (!superMode_) calibrate();

    // Il2CppType.data 语义（依赖已校准的 fields/field_count）
    {
      std::vector<uintptr_t> samples;
      sampleClasses(samples, 96);
      detectTypeDataMode(samples);
    }

    discoverClassPointerSlots(images);

    // 元数据（参数名）：两种模式都用取样类去找
    if (opt->useMetadata) {
      std::vector<uintptr_t> msamples;
      sampleClasses(msamples, 96);
      for (uintptr_t k : msamples) {
        char nm[512];
        if (!klassName(k, nm, sizeof(nm))) continue;
        uintptr_t tdh = mem.rdPtr(k + lay.cTypeDefHandle);
        if (!tdh) continue;
        if (meta.locate(tdh, nm)) {
          log->line("[meta] base=0x%llx stringOff=0x%X methodsOff=0x%X paramsOff=0x%X typeDefsOff=0x%X",
                    (unsigned long long)meta.base, meta.stringOff, meta.methodsOff, meta.paramsOff,
                    meta.typeDefsOff);
          break;
        }
      }
      if (!meta.ok) log->line("[meta] not found -> runtime/synthetic param names");
    }

    Writer w;
    if (!w.open(opt->outPath.c_str())) {
      log->line("[FAIL] cannot open %s", opt->outPath.c_str());
      return false;
    }
    if (opt->structureIndexes) {
      if (!indexes_.begin(opt->outPath, env->moduleName, env->moduleBase, env->moduleSize, env->superVariant)) {
        log->line("[索引] 无法创建结构索引文件；目录=%s", indexes_.directory.c_str());
        return false;
      }
      log->line("[索引] 开始生成结构索引；目录=%s", indexes_.directory.c_str());
    }
    writeManagerRvaSummary(w);
    if (opt->offsetSummary) writeOffsetSummary(w);
    // 头部：全部镜像
    for (size_t ii = 0; ii < images.size(); ++ii) {
      uintptr_t img = images[ii];
      char nm[128];
      mem.rdStr(mem.rdPtr(img + 0x00), nm, sizeof(nm), false);
      if (!opt->imageFilter.empty() && !strstr(nm, opt->imageFilter.c_str())) continue;
      w.putf("// Image %u: %s - %u\r\n", (unsigned)ii, nm, imageStart(img));
      ++st.images;
    }
    w.puts("\r\n");

    for (uintptr_t img : images) {
      char inm[128];
      mem.rdStr(mem.rdPtr(img + 0x00), inm, sizeof(inm), false);
      if (!opt->imageFilter.empty() && !strstr(inm, opt->imageFilter.c_str())) continue;
      uint32_t ts = imageStart(img);
      size_t cnt = imageCount(img);
      if (opt->maxClassesPerImage && cnt > opt->maxClassesPerImage) cnt = opt->maxClassesPerImage;
      log->line("[2] dumping '%s' classes=%llu startIdx=0x%X", inm, (unsigned long long)cnt, ts);

      if (env->imageGetClass) {
        uint32_t typeDefIndex = ts;
        for (size_t i = 0; i < cnt; ++i) {
          uintptr_t k = (uintptr_t)env->imageGetClass((void *)img, i);
          if (!k) {
            ++typeDefIndex;
            continue;
          }
          writeClass(w, k, classIndex(k, typeDefIndex));
          ++typeDefIndex;
          if ((i & 0x1FFF) == 0) w.flush();
        }
      } else if (!typeTable_.empty()) {
        for (uint32_t i = 0; i < cnt; ++i) {
          if (opt->maxClassesPerImage && i >= opt->maxClassesPerImage) break;
          size_t idx = (size_t)ts + i;
          uintptr_t k = idx < typeTable_.size() ? typeTable_[idx] : 0;
          if (k) writeClass(w, k, ts + i);
          if ((i & 0x1FFF) == 0) w.flush();
        }
      } else {
        uint32_t n = 0;
        for (uintptr_t k : klasses_) {
          if (mem.rdPtr(k + lay.cImage) != img) continue;
          writeClass(w, k, ts + n);
          ++n;
          if (opt->maxClassesPerImage && n >= opt->maxClassesPerImage) break;
          if ((n & 0x1FFF) == 0) w.flush();
        }
      }
      w.flush();
      log->line("    -> classes=%llu fields=%llu methods=%llu props=%llu", (unsigned long long)st.classes,
                (unsigned long long)st.fields, (unsigned long long)st.methods,
                (unsigned long long)st.props);
    }
    bool dumpComplete = w.close();
    quality.outputEscapedBytes += w.escapedBytes;
    if (!dumpComplete) log->line("[FAIL] dump 文件写入或关闭失败，可能存在截断");
    log->line("[3] DONE classes=%llu fields=%llu methods=%llu props=%llu ifaces=%llu skipped=%llu bytes=%llu",
              (unsigned long long)st.classes, (unsigned long long)st.fields, (unsigned long long)st.methods,
              (unsigned long long)st.props, (unsigned long long)st.ifaces,
              (unsigned long long)st.skippedClasses, (unsigned long long)w.written_);
    log->line("[3] metaParams=%llu runtimeParamNames=%llu synthParams=%llu rvaZero=%llu vq=%llu",
              (unsigned long long)st.metaParams, (unsigned long long)st.runtimeParamNames,
              (unsigned long long)st.synthParams, (unsigned long long)st.rvaZero,
              (unsigned long long)mem.vqCount_);
    log->line("[3] classRvas=%llu/%llu (class TypeInfo pointer slots)",
              (unsigned long long)st.classRvas,
              (unsigned long long)st.classes);
    log->line("[3] parammode=%d typedata=%s flags=0x%X token=0x%X ifaces=0x%X(%d) tbl=%llu handles=%llu typeFallbacks=%llu",
              paramMode_, typeDataIsHandle_ ? "handle" : (typeDataIsIndex_ ? "index" : "pointer"),
              lay.cFlags, lay.cToken, lay.cInterfaces, lay.scoreIfaces,
              (unsigned long long)typeTable_.size(),
              (unsigned long long)typeHandleToKlass_.size(),
              (unsigned long long)st.classTypeFallbacks);
    if (superMode_ || env->expectMainImage) {
      // A Super run that discovers only the framework images is a false
      // positive: the engine can still serialize a small, syntactically valid
      // dump while completely missing Assembly-CSharp.  Make this visible to
      // the caller and the log instead of reporting a misleading success.
      bool hasMainImage = false;
      for (uintptr_t img : images) {
        char nm[128] = {};
        if (!mem.rdStr(mem.rdPtr(img + 0x00), nm, sizeof(nm), true)) continue;
        if (_stricmp(nm, "Assembly-CSharp.dll") == 0) {
          hasMainImage = true;
          break;
        }
      }
      if (!hasMainImage) log->line("[WARN] Super scan did not discover Assembly-CSharp.dll; dump is incomplete");
      if (st.classes < 10000) log->line("[WARN] Super class count is unexpectedly low: %llu",
                                       (unsigned long long)st.classes);
      if (st.methods < 10000) log->line("[WARN] Super method count is unexpectedly low: %llu",
                                        (unsigned long long)st.methods);
      if (st.classTypeFallbacks > 1000)
        log->line("[WARN] Super class-type fallback count is unexpectedly high: %llu",
                  (unsigned long long)st.classTypeFallbacks);
      if (env->expectMainImage && (!hasMainImage || st.methods < 10000 || st.classTypeFallbacks > 1000))
        dumpComplete = false;
    }
    const bool indexesComplete = indexes_.finish(*this, dumpComplete);
    log->line("[质量] 汇总：异常成员组=%llu 拒绝条目=%llu 数量上限跳过=%llu 正文和索引转义字节=%llu；%s",
      (unsigned long long)quality.groups, (unsigned long long)quality.rejected,
      (unsigned long long)quality.limited, (unsigned long long)quality.outputEscapedBytes,
      quality.warnings() ? (opt->structureIndexes ? "存在质量提示；详细原因见 member_diagnostics.tsv，不应视为完整元数据" :
          "存在质量提示；完整成员诊断需启用structureindexes，不应视为完整元数据") : "本次成员检查未发现异常");
    return dumpComplete && indexesComplete;
  }
};

}  // namespace ildump

#include "naraka_offset_report.hpp"

#endif  // ILDUMP_CORE_HPP
