#ifndef ILDUMP_STRUCTURE_INDEXES_HPP
#define ILDUMP_STRUCTURE_INDEXES_HPP

#include <windows.h>
#include <algorithm>
#include <cstdint>
#include <cstdio>
#include <initializer_list>
#include <string>
#include <unordered_map>
#include <vector>
#include "dump_quality.hpp"

namespace ildump {

__declspec(noinline) inline bool SafeCopy(void *dst, const void *src, size_t len);

// Independent files use checked writes and RAII. A failed export must never
// leave a success manifest merely because the file could be opened.
class IndexFile {
  FILE *file_ = nullptr;
  bool ok_ = true;
public:
  uint64_t rows = 0, bytes = 0, escapedBytes = 0;
  IndexFile() = default;
  IndexFile(const IndexFile &) = delete;
  IndexFile &operator=(const IndexFile &) = delete;
  ~IndexFile() { close(); }
  bool open(const std::string &path) {
    close();
    ok_ = true;
    rows = bytes = escapedBytes = 0;
    file_ = fopen(path.c_str(), "wb");
    if (!file_) return ok_ = false;
    setvbuf(file_, nullptr, _IOFBF, 256 * 1024);
    text("\xEF\xBB\xBF"); // UTF-8 BOM for Windows spreadsheet/text viewers.
    return ok_;
  }
  bool good() const { return ok_; }
  void text(const std::string &value) {
    if (!ok_ || !file_) { ok_ = false; return; }
    std::string clean;
    const std::string *output = &value;
    if (!ValidUtf8(value.data(), value.size())) {
      clean = EscapeInvalidUtf8(value.data(), value.size(), &escapedBytes);
      output = &clean;
    }
    const size_t n = fwrite(output->data(), 1, output->size(), file_);
    bytes += n;
    if (n != output->size()) ok_ = false;
  }
  static std::string escape(const std::string &s) {
    std::string out;
    out.reserve(s.size());
    for (char c : s) {
      switch (c) {
      case '\\': out += "\\\\"; break;
      case '\t': out += "\\t"; break;
      case '\r': out += "\\r"; break;
      case '\n': out += "\\n"; break;
      default: out += c; break;
      }
    }
    return out;
  }
  void row(std::initializer_list<std::string> cells, bool header = false) {
    std::string line;
    bool first = true;
    for (const auto &cell : cells) {
      if (!first) line += '\t';
      first = false;
      line += escape(cell);
    }
    text(line + "\r\n");
    if (!header && ok_) ++rows;
  }
  bool close() {
    if (file_) {
      if (fclose(file_) != 0) ok_ = false;
      file_ = nullptr;
    }
    return ok_;
  }
};

inline std::string IndexHex(uint64_t n) {
  char s[32];
  snprintf(s, sizeof(s), "0x%llX", (unsigned long long)n);
  return s;
}

struct IndexClass {
  uintptr_t klass = 0, image = 0, parent = 0;
  uint32_t typeIndex = 0, flags = 0, rawSize = 0, sizeOffset = 0;
  uint16_t rawFields = 0, rawMethods = 0, rawProps = 0, rawIfaces = 0;
  bool sizeReadable = false;
  std::string assembly, namespaze, name, kind, parentName;
};

struct IndexMethod {
  size_t owner = 0;
  uintptr_t address = 0, info = 0;
  uint32_t token = 0;
  uint16_t flags = 0, slot = 0xFFFF;
  std::string name, signature;
};

class StructureIndexes {
  IndexFile classFile_, methodFile_, baseFile_, interfaceFile_, diagnosticFile_;
  std::vector<IndexClass> classes_;
  std::vector<IndexMethod> methods_;
  uintptr_t moduleBase_ = 0;
  size_t moduleSize_ = 0;
  std::string module_, dumpPath_, variant_;
  bool active_ = false;

  bool inModule(uintptr_t p) const {
    return p && moduleBase_ && p >= moduleBase_ && p - moduleBase_ < moduleSize_;
  }
  int addressKind(uintptr_t p) const { return inModule(p) ? 0 : (p ? 1 : 2); }

public:
  std::string directory;
  uint64_t classRows = 0, methodRows = 0, interfaceRows = 0, baseRows = 0;
  uint64_t matchedSlots = 0, matchedClasses = 0, scanBytes = 0, failedScanBytes = 0;
  bool scanComplete = true;
  bool active() const { return active_; }

  bool begin(const std::string &dumpPath, const std::string &module,
             uintptr_t base, size_t size, bool superVariant) {
    dumpPath_ = dumpPath;
    module_ = module;
    moduleBase_ = base;
    moduleSize_ = size;
    variant_ = superVariant ? "Super" : "普通";
    std::string stem = dumpPath;
    const size_t slash = stem.find_last_of("/\\");
    const size_t dot = stem.rfind('.');
    if (dot != std::string::npos && (slash == std::string::npos || dot > slash)) stem.resize(dot);
    stem += "_structures";
    // Never mix new files with an earlier run, even if the timestamps collide.
    bool created = false;
    for (unsigned i = 0; i < 1000; ++i) {
      directory = stem + (i ? "_" + std::to_string(i + 1) : "");
      if (CreateDirectoryA(directory.c_str(), nullptr)) { created = true; break; }
      if (GetLastError() != ERROR_ALREADY_EXISTS) return false;
    }
    if (!created) return false;
    IndexFile pending;
    if (!pending.open(directory + "\\export_manifest.txt")) return false;
    pending.text("SuperDumper v9 结构索引\r\n状态：正在生成；此标记未更新为完成前，不应视为成功。\r\n");
    pending.text("对应 dump：" + dumpPath_ + "\r\n");
    if (!pending.close()) return false;
    if (!classFile_.open(directory + "\\class_meta.tsv") ||
        !methodFile_.open(directory + "\\rva_map.tsv") ||
        !baseFile_.open(directory + "\\all_bases.txt") ||
        !interfaceFile_.open(directory + "\\interfaces.txt") ||
        !diagnosticFile_.open(directory + "\\member_diagnostics.tsv")) return false;
    classFile_.row({"ClassId", "Assembly", "Namespace", "Name", "DumpTypeDefIndex", "ClassVA",
                    "Flags", "Kind", "ParentClassVA", "ParentName", "InstanceSize", "InstanceSizeStatus",
                    "RawInstanceSize", "RawInstanceSizeOffset", "FieldCount", "MethodCount", "PropertyCount",
                    "InterfaceCount", "RawFieldCount", "RawMethodCount", "RawPropertyCount", "RawInterfaceCount"}, true);
    methodFile_.row({"RVA", "ClassId", "Assembly", "Namespace", "Class", "Method", "Signature",
                     "MethodInfoVA", "MethodVA", "Token", "Flags", "Slot", "AddressStatus"}, true);
    baseFile_.row({"RVA", "ClassId", "Assembly", "Namespace", "Class", "ClassVA", "SlotVA",
                   "Module", "Protection", "SelectedByDump", "Status"}, true);
    interfaceFile_.row({"ClassId", "Assembly", "Namespace", "Class", "ClassVA", "ArrayIndex",
                        "InterfaceAssembly", "InterfaceNamespace", "Interface", "InterfaceClassVA", "Status"}, true);
    diagnosticFile_.row({"ClassId", "Assembly", "Namespace", "Class", "ClassVA", "Category",
      "RawCount", "Attempted", "Exported", "LimitSkipped", "Filtered", "NullArray", "Unreadable",
      "InvalidName", "InvalidUtf8", "OwnerMismatch", "InvalidType", "MissingAccessor", "NotInterface",
      "FirstIndex", "FirstAddress", "FirstName", "FirstReason", "FirstUtf8Index", "FirstUtf8Address", "FirstUtf8Name", "Status"}, true);
    active_ = true;
    return true;
  }

  void beginClass(IndexClass c) { if (active_) classes_.push_back(std::move(c)); }
  void endClass(uint64_t fields, uint64_t methods, uint64_t props, uint64_t ifaces) {
    if (!active_) return;
    const auto &c = classes_.back();
    classFile_.row({std::to_string(classes_.size()), c.assembly, c.namespaze, c.name, IndexHex(c.typeIndex),
                    IndexHex(c.klass), IndexHex(c.flags), c.kind, c.parent ? IndexHex(c.parent) : "", c.parentName,
                    "", c.sizeReadable ? "布局未验证，仅保留原始读数" : "不可读",
                    c.sizeReadable ? IndexHex(c.rawSize) : "", IndexHex(c.sizeOffset),
                    std::to_string(fields), std::to_string(methods), std::to_string(props), std::to_string(ifaces),
                    std::to_string(c.rawFields), std::to_string(c.rawMethods), std::to_string(c.rawProps),
                    std::to_string(c.rawIfaces)});
  }
  void method(IndexMethod m) {
    if (!active_) return;
    m.owner = classes_.size() - 1;
    methods_.push_back(std::move(m));
  }
  void interfaceEntry(uint32_t index, uintptr_t klass, const std::string &assembly,
                      const std::string &namespaze, const std::string &name, bool flagsMatch) {
    if (!active_) return;
    const auto &c = classes_.back();
    interfaceFile_.row({std::to_string(classes_.size()), c.assembly, c.namespaze, c.name, IndexHex(c.klass),
                        std::to_string(index), assembly, namespaze, name, IndexHex(klass),
                        flagsMatch ? "接口标志匹配" : "未确认接口标志"});
  }

  void diagnostics(const char *category, const MemberAudit &a) {
    if (!active_ || (!a.issue() && a.raw <= 4096)) return;
    const auto &c = classes_.back();
    auto n = [](uint64_t v) { return std::to_string(v); };
    diagnosticFile_.row({n(classes_.size()), c.assembly, c.namespaze, c.name, IndexHex(c.klass), category,
      n(a.raw), n(a.attempted), n(a.exported), n(a.limited), n(a.filtered),
      n(a.rejected[0]), n(a.rejected[1]), n(a.rejected[2]), n(a.rejected[3]), n(a.rejected[4]),
      n(a.rejected[5]), n(a.rejected[6]), n(a.rejected[7]),
      a.firstReason.empty() ? "" : n(a.firstIndex), a.firstAddress ? IndexHex(a.firstAddress) : "",
      a.firstName, a.firstReason, a.firstUtf8Name.empty() ? "" : n(a.firstUtf8Index),
      a.firstUtf8Address ? IndexHex(a.firstUtf8Address) : "", a.firstUtf8Name,
      a.issue() ? "存在未导出或已拒绝条目，请检查原因" : "超过旧版上限，已完整遍历"});
  }

  // One bounded scan of module data for ALL exported classes. These matches
  // are evidence of pointer values, not proof that every slot is a TypeInfo global.
  template <class D> void scanBases(D &d) {
    std::unordered_map<uintptr_t, size_t> owners;
    for (size_t i = 0; i < classes_.size(); ++i) owners.emplace(classes_[i].klass, i);
    std::vector<uint64_t> hits(classes_.size(), 0);
    if (!moduleBase_ || !moduleSize_ || moduleSize_ > UINTPTR_MAX - moduleBase_) scanComplete = false;
    const uintptr_t end = scanComplete ? moduleBase_ + moduleSize_ : moduleBase_;
    uintptr_t p = moduleBase_;
    uintptr_t data[512];
    while (scanComplete && p < end) {
      MEMORY_BASIC_INFORMATION mbi = {};
      if (!VirtualQuery((LPCVOID)p, &mbi, sizeof(mbi))) { scanComplete = false; break; }
      const uintptr_t lo = (std::max)(p, (uintptr_t)mbi.BaseAddress);
      const uintptr_t region = (uintptr_t)mbi.BaseAddress;
      if (!mbi.RegionSize || mbi.RegionSize > UINTPTR_MAX - region) { scanComplete = false; break; }
      const uintptr_t hi = (std::min)(end, region + mbi.RegionSize);
      if (hi <= p) { scanComplete = false; break; }
      const DWORD protect = mbi.Protect & 0xFFu;
      const bool readableData = protect == PAGE_READONLY || protect == PAGE_READWRITE || protect == PAGE_WRITECOPY;
      if (mbi.State == MEM_COMMIT && !(mbi.Protect & PAGE_GUARD) && readableData) {
        uintptr_t cur = (lo + 7) & ~(uintptr_t)7;
        while (cur < hi && hi - cur >= sizeof(uintptr_t)) {
          const size_t bytes = (size_t)(std::min)(hi - cur, (uintptr_t)(4096 - (cur & 4095))) & ~(size_t)7;
          if (!d.mem.probe(cur, bytes) || !SafeCopy(data, (const void *)cur, bytes)) {
            failedScanBytes += bytes; cur += bytes; continue;
          }
          scanBytes += bytes;
          for (size_t j = 0; j < bytes / 8; ++j) {
            auto owner = owners.find(data[j]);
            if (owner == owners.end()) continue;
            const size_t n = owner->second;
            const auto &c = classes_[n];
            const uintptr_t slot = cur + j * 8;
            const auto selected = d.classPointerSlots_.find(c.klass);
            const bool used = selected != d.classPointerSlots_.end() && selected->second == slot;
            baseFile_.row({IndexHex(slot - moduleBase_), std::to_string(n + 1), c.assembly, c.namespaze,
                           c.name, IndexHex(c.klass), IndexHex(slot), module_, IndexHex(mbi.Protect),
                           used ? "1" : "0", "指针值匹配，槽位用途待确认"});
            ++matchedSlots;
            if (hits[n]++ == 0) ++matchedClasses;
          }
          cur += bytes;
        }
      }
      p = hi;
    }
    if (failedScanBytes) scanComplete = false;
    for (size_t i = 0; i < classes_.size(); ++i) {
      if (hits[i]) continue;
      const auto &c = classes_[i];
      baseFile_.row({"", std::to_string(i + 1), c.assembly, c.namespaze, c.name, IndexHex(c.klass), "", module_, "", "0",
                     scanComplete ? "本次扫描范围内未命中" : "扫描不完整，未确认"});
    }
  }

  template <class D> bool finish(D &d, bool dumpComplete) {
    if (!active_) return true;
    std::stable_sort(methods_.begin(), methods_.end(), [&](const IndexMethod &a, const IndexMethod &b) {
      const int ak = addressKind(a.address), bk = addressKind(b.address);
      return ak != bk ? ak < bk : a.address < b.address;
    });
    for (const auto &m : methods_) {
      const auto &c = classes_[m.owner];
      methodFile_.row({inModule(m.address) ? IndexHex(m.address - moduleBase_) : "", std::to_string(m.owner + 1),
                       c.assembly, c.namespaze, c.name, m.name, m.signature, IndexHex(m.info), IndexHex(m.address),
                       IndexHex(m.token), IndexHex(m.flags), m.slot == 0xFFFF ? "" : std::to_string(m.slot),
                       inModule(m.address) ? "模块内地址" : (m.address ? "模块外地址，仅保留运行时VA" : "空方法指针")});
    }
    // Release the large signature buffer before scanning module pages.
    std::vector<IndexMethod>().swap(methods_);
    scanBases(d);
    const bool classesOk = classFile_.close();
    const bool methodsOk = methodFile_.close();
    const bool basesOk = baseFile_.close();
    const bool ifacesOk = interfaceFile_.close();
    const bool diagnosticsOk = diagnosticFile_.close();
    d.quality.outputEscapedBytes += classFile_.escapedBytes + methodFile_.escapedBytes +
        baseFile_.escapedBytes + interfaceFile_.escapedBytes;
    classRows = classFile_.rows; methodRows = methodFile_.rows;
    baseRows = baseFile_.rows; interfaceRows = interfaceFile_.rows;
    const bool consistent = classRows == d.st.classes && methodRows == d.st.methods && interfaceRows == d.st.ifaces;
    const bool ok = classesOk && methodsOk && basesOk && ifacesOk && diagnosticsOk && consistent && scanComplete && dumpComplete;
    IndexFile manifest;
    const std::string pendingPath = directory + "\\export_manifest.pending";
    const bool opened = manifest.open(pendingPath);
    if (opened) {
      manifest.text(std::string("SuperDumper v9 结构索引（格式版本 2）\r\n状态：") +
        (ok ? (d.quality.warnings() ? "文件已生成，存在数据质量提示" : "完成") : "未完整完成，请检查日志") + "\r\n");
      manifest.text("对应 dump：" + dumpPath_ + "\r\n模块：" + module_ + "\r\n变体：" + variant_ +
                    "\r\n模块基址：" + IndexHex(moduleBase_) + "\r\n模块大小：" + IndexHex(moduleSize_) + "\r\n");
      manifest.text("文件\t数据行数\t字节数\t写入状态\r\n");
      auto file = [&](const char *name, const IndexFile &f) {
        manifest.text(std::string(name) + "\t" + std::to_string(f.rows) + "\t" + std::to_string(f.bytes) +
                      "\t" + (f.good() ? "成功" : "失败") + "\r\n");
      };
      file("class_meta.tsv", classFile_); file("rva_map.tsv", methodFile_);
      file("all_bases.txt", baseFile_); file("interfaces.txt", interfaceFile_);
      file("member_diagnostics.tsv", diagnosticFile_);
      manifest.text("\r\n数据质量：" + std::string(d.quality.warnings() ? "存在受限、拒绝或编码异常条目；不等同于完整元数据" : "本次检查未发现成员异常；不代表全部布局已验证") +
        "\r\n异常成员组数：" + std::to_string(d.quality.groups) +
        "\r\n跳过类型数：" + std::to_string(d.quality.skippedClasses) +
        "\r\n拒绝条目数：" + std::to_string(d.quality.rejected) +
        "\r\n数量上限未遍历条目数：" + std::to_string(d.quality.limited) +
        "\r\n正文或结构索引转义非法字节数：" + std::to_string(d.quality.outputEscapedBytes) +
        "\r\n诊断文件转义原始证据字节数：" + std::to_string(diagnosticFile_.escapedBytes) + "\r\n");
      manifest.text("\r\n正文完成状态：" + std::string(dumpComplete ? "成功" : "失败") +
                    "\r\n与正文计数核对：" + (consistent ? "一致" : "不一致") +
                    "\r\n类型槽位匹配数：" + std::to_string(matchedSlots) +
                    "\r\n已匹配类型数：" + std::to_string(matchedClasses) +
                    "\r\n已扫描字节数：" + std::to_string(scanBytes) +
                    "\r\n读取失败字节数：" + std::to_string(failedScanBytes) +
                    "\r\n扫描完成状态：" + (scanComplete ? "完成" : "不完整") + "\r\n");
      manifest.text("\r\n范围与解释：\r\n"
                    "1. 索引与本次 dump 的 images/maxclasses/fields/methods/properties 配置范围一致。\r\n"
                    "2. ClassId 是本次运行的关联键，可区分同名类及重复的局部类型索引。DumpTypeDefIndex 与正文相同，不额外宣称已验证全局元数据索引。\r\n"
                    "3. 成员 Count 列记录正文实际成功导出的数量；RawCount 列保留校准位置的原始读数。\r\n"
                    "4. InstanceSize 留空；RawInstanceSize 是未验证布局的读数，不能用于对象读取或容器步长计算。\r\n"
                    "5. 方法按模块内地址、模块外地址、空指针分组排序。同一 RVA 的不同方法和重载全部保留。Slot 是 MethodInfo 的声明槽位，不是虚表字节偏移。\r\n"
                    "6. all_bases 只扫描本模块内已提交且可读的非执行数据页；保护页、不读页和执行页不在范围内。保留全部指针值匹配，未证明每个候选都是 TypeInfo 全局槽。\r\n"
                    "7. SelectedByDump=1 仅表示该槽位被原 dump 选用，不代表单例链已验证。多个候选不自动修改既有偏移摘要。\r\n"
                    "8. VA 仅适用于本次运行。TSV 使用 UTF-8 BOM；单元格内反斜杠、制表符和换行分别转义为 \\\\, \\t, \\r, \\n；非法字节按 \\xNN 保留。\r\n"
                    "9. member_diagnostics 记录异常成员组及超过旧4096上限的成员组；Attempted 是实际尝试读取数，空数组的拒绝数可大于尝试数。配置关闭的类别不作为异常。\r\n"
                    "10. 接口输出要求有效名称、类型结构和接口标志；原始数组与计数仍依赖运行时校准。成员上限0表示按原始uint16计数逐项验证。\r\n");
      manifest.text("\r\n布局线索：\r\n镜像索引模式：" + d.imageLayoutProfile_ +
                    "\r\n类标志偏移：" + IndexHex(d.lay.cFlags) + "\r\n方法计数偏移：" + IndexHex(d.lay.cMethodCount) +
                    "\r\n字段计数偏移：" + IndexHex(d.lay.cFieldCount) + "\r\n接口数组偏移：" + IndexHex(d.lay.cInterfaces) +
                    "\r\n接口计数偏移：" + IndexHex(d.lay.cIfaceCount) + "\r\n");
    }
    bool manifestOk = manifest.close() && opened;
    if (manifestOk)
      manifestOk = MoveFileExA(pendingPath.c_str(), (directory + "\\export_manifest.txt").c_str(),
                               MOVEFILE_REPLACE_EXISTING | MOVEFILE_WRITE_THROUGH) != FALSE;
    d.log->line("[索引] %s：类=%llu 方法=%llu 接口=%llu 槽位匹配=%llu；目录=%s",
                ok && manifestOk ? "完成" : "未完整完成", (unsigned long long)classRows,
                (unsigned long long)methodRows, (unsigned long long)interfaceRows,
                (unsigned long long)matchedSlots, directory.c_str());
    if (!scanComplete) d.log->line("[索引] 类型槽位扫描不完整，读取失败字节=%llu", (unsigned long long)failedScanBytes);
    std::vector<IndexClass>().swap(classes_);
    active_ = false;
    return ok && manifestOk;
  }
};

} // namespace ildump
#endif
