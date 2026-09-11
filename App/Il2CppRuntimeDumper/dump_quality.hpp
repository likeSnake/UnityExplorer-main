#ifndef ILDUMP_QUALITY_HPP
#define ILDUMP_QUALITY_HPP
#include <cstdint>
#include <cstring>
#include <string>
namespace ildump {
// Validate complete Unicode scalar values, including overlong/surrogate checks.
inline size_t Utf8Sequence(const char *p, size_t n) {
  if (!n) return 0;
  const auto *s = reinterpret_cast<const unsigned char *>(p);
  const unsigned c = s[0];
  if (c < 0x80) return 1;
  const size_t len = c >= 0xC2 && c <= 0xDF ? 2 :
      (c >= 0xE0 && c <= 0xEF ? 3 : (c >= 0xF0 && c <= 0xF4 ? 4 : 0));
  if (!len || n < len) return 0;
  for (size_t i = 1; i < len; ++i) if ((s[i] & 0xC0) != 0x80) return 0;
  if ((c == 0xE0 && s[1] < 0xA0) || (c == 0xED && s[1] >= 0xA0) ||
      (c == 0xF0 && s[1] < 0x90) || (c == 0xF4 && s[1] >= 0x90)) return 0;
  return len;
}
inline bool ValidUtf8(const char *s, size_t n) {
  for (size_t i = 0; i < n;) {
    const size_t len = Utf8Sequence(s + i, n - i);
    if (!len) return false;
    i += len;
  }
  return true;
}
inline bool ValidUtf8(const char *s) { return s && ValidUtf8(s, strlen(s)); }
inline std::string EscapeInvalidUtf8(const char *s, size_t n, uint64_t *count = nullptr) {
  std::string out;
  out.reserve(n);
  const char hex[] = "0123456789ABCDEF";
  for (size_t i = 0; i < n;) {
    const size_t len = Utf8Sequence(s + i, n - i);
    if (len) { out.append(s + i, len); i += len; }
    else {
      const auto b = static_cast<unsigned char>(s[i++]);
      out += "\\x"; out += hex[b >> 4]; out += hex[b & 15];
      if (count) ++*count;
    }
  }
  return out;
}
enum class MemberIssue : unsigned {
  NullArray, Unreadable, InvalidName, InvalidUtf8, OwnerMismatch,
  InvalidType, MissingAccessor, NotInterface, Count
};
inline const char *MemberIssueName(MemberIssue issue) {
  static const char *names[] = {"成员数组为空", "内存不可读或空成员指针", "名称无效或未完整读取",
    "名称含非法UTF-8", "成员所属类型不匹配", "类型结构无效", "属性没有可用访问器", "目标不是有效接口"};
  return names[static_cast<unsigned>(issue)];
}
struct MemberAudit {
  uint32_t raw = 0, attempted = 0, exported = 0, limited = 0, filtered = 0;
  uint32_t rejected[static_cast<unsigned>(MemberIssue::Count)] = {};
  uint32_t firstIndex = 0;
  uintptr_t firstAddress = 0;
  std::string firstName, firstReason;
  uint32_t firstUtf8Index = 0;
  uintptr_t firstUtf8Address = 0;
  std::string firstUtf8Name;
  uint32_t rejectedCount() const { uint32_t n = 0; for (auto v : rejected) n += v; return n; }
  bool issue() const { return limited || rejectedCount(); }
  void reject(MemberIssue why, uint32_t index, uintptr_t address, const char *name = "", uint32_t count = 1) {
    rejected[static_cast<unsigned>(why)] += count;
    if (why == MemberIssue::InvalidUtf8 && firstUtf8Name.empty()) {
      firstUtf8Name = name; firstUtf8Index = index; firstUtf8Address = address;
    }
    if (firstReason.empty()) {
      firstIndex = index; firstAddress = address; firstName = name;
      firstReason = MemberIssueName(why);
    }
  }
};
struct QualityStats {
  uint64_t groups = 0, rejected = 0, limited = 0, outputEscapedBytes = 0, skippedClasses = 0;
  bool warnings() const { return groups || outputEscapedBytes || skippedClasses; }
};
} // namespace ildump
#endif
