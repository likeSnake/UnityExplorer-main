#pragma once

#include <cstdint>
#include <cstdlib>
#include <iomanip>
#include <vector>
#include <sstream>
#include <string>

namespace er2 {

enum class HeadlessScanMode {
  None,
  AllMsid,
  GameObjects,
  ScriptableObjects,
};

struct HeadlessOptions {
  bool headless = false;
  bool help = false;
  std::string targetProcess = "NarakaBladepoint.exe";
  std::string runMode = "dma";
  HeadlessScanMode scanMode = HeadlessScanMode::GameObjects;
  std::string filter;
  std::uint32_t limit = 256;
  std::uintptr_t klass = 0;
  bool scanManagers = false;
  std::uint32_t managerScanMaxMb = 0;
  std::string outPath = "headless_probe.json";
  std::string logPath;
  std::string offsetHeaderPath;
  std::string runtimeDumpCsPath;
  std::uint32_t runtimeDumpLimit = 256;
  std::vector<std::uint64_t> runtimeDumpRvas;
  std::string metadataOutPath;
  bool metadataDiagnose = false;
  std::uint32_t metadataDiagnoseMaxMb = 256;
  std::uintptr_t metadataProbeVa = 0;
  std::uintptr_t metadataDiagnoseAnchor = 0;
  bool metadataDiagnoseHeuristic = false;
  std::string dumpImageDir;   // --dump-image <dir> : export GameAssembly/UnityPlayer/all-module PE images via DMA
  bool dumpImageAllModules = false;
  std::uint32_t dumpImageMaxMb = 0; // optional per-module cap for safety
  std::string selfDumpCsPath;       // --self-dump-cs <path> : runtime structure dump.cs
  std::string selfDumpCsLog;
  std::uint32_t selfDumpMaxKlass = 0;
  std::string dmaInjectDll;         // --dma-inject <dll> : DMA-based LoadLibrary injection
  std::string dmaInjectHook;        // --dma-inject-hook <module!func> : hook point
  std::uint32_t dmaInjectPid = 0;   // --dma-inject-pid <pid> : target pid
  std::uint32_t dmaInjectWaitMs = 15000;
  bool dmaInjectTest = false;       // --dma-inject-test : verify execution path only
  bool narakaBuffProbe = false;
  std::uint32_t narakaBuffMaxEntries = 1024;
  std::uint32_t narakaBuffScanEnd = 0x180;
  bool narakaRuntimePropertyProbe = false;
  std::uint32_t narakaRuntimePropertyMaxRows = 4096;
  std::uint32_t narakaRuntimePropertySamples = 1;
  std::uint32_t narakaRuntimePropertyIntervalMs = 100;
  std::string narakaRuntimePropertyCsvPath;
  bool narakaGroundProbe = false;
  bool narakaActorContainerProbe = false;
  std::uintptr_t narakaActorAddress = 0;
  std::uint32_t narakaActorMaxEntries = 4096;
  std::uint32_t narakaActorScanEnd = 0x1000;
  bool narakaActorOnlyTarka = true;
};

struct HeadlessParseResult {
  bool ok = false;
  std::string error;
  HeadlessOptions options;
};

inline std::string HeadlessToLowerAscii(std::string value) {
  for (char &ch : value) {
    if (ch >= 'A' && ch <= 'Z') {
      ch = static_cast<char>(ch - 'A' + 'a');
    }
  }
  return value;
}

inline bool HeadlessParseU32(const std::string &text, std::uint32_t &out) {
  if (text.empty()) {
    return false;
  }

  char *end = nullptr;
  const unsigned long value = std::strtoul(text.c_str(), &end, 0);
  if (!end || *end != '\0' || value > 0xFFFFFFFFul) {
    return false;
  }

  out = static_cast<std::uint32_t>(value);
  return true;
}

inline bool HeadlessParsePtr(const std::string &text, std::uintptr_t &out) {
  if (text.empty()) {
    return false;
  }

  char *end = nullptr;
  const unsigned long long value = std::strtoull(text.c_str(), &end, 0);
  if (!end || *end != '\0') {
    return false;
  }

  out = static_cast<std::uintptr_t>(value);
  return true;
}

inline bool HeadlessParseU64(const std::string &text, std::uint64_t &out) {
  if (text.empty()) {
    return false;
  }

  char *end = nullptr;
  const unsigned long long value = std::strtoull(text.c_str(), &end, 0);
  if (!end || *end != '\0') {
    return false;
  }

  out = static_cast<std::uint64_t>(value);
  return true;
}

inline bool HeadlessParseU64List(const std::string &text,
                                 std::vector<std::uint64_t> &out) {
  out.clear();
  std::size_t start = 0;
  while (start <= text.size()) {
    const std::size_t comma = text.find(',', start);
    const std::size_t end =
        comma == std::string::npos ? text.size() : comma;
    const std::string part = text.substr(start, end - start);
    if (part.empty()) {
      return false;
    }

    std::uint64_t value = 0;
    if (!HeadlessParseU64(part, value)) {
      return false;
    }
    out.push_back(value);

    if (comma == std::string::npos) {
      break;
    }
    start = comma + 1;
  }

  return !out.empty();
}

inline std::string RuntimeDumpRvaLabel(std::uint64_t rva) {
  switch (rva) {
  case 0x35E25C0ull:
    return "m_CharacterManager";
  case 0x35F4240ull:
    return "m_EntityManager";
  case 0x35E2628ull:
    return "m_CharactorSync";
  case 0x35E1250ull:
    return "m_BuffManager";
  case 0x35872C8ull:
    return "m_UserDataManager";
  case 0x359CF28ull:
    return "m_GlobalTime_BestCandidate";
  default:
    return "";
  }
}

inline bool HeadlessParseScanMode(const std::string &text,
                                  HeadlessScanMode &out) {
  const std::string mode = HeadlessToLowerAscii(text);
  if (mode == "none") {
    out = HeadlessScanMode::None;
    return true;
  }
  if (mode == "msid" || mode == "all") {
    out = HeadlessScanMode::AllMsid;
    return true;
  }
  if (mode == "gameobjects" || mode == "gameobject" || mode == "go") {
    out = HeadlessScanMode::GameObjects;
    return true;
  }
  if (mode == "scriptableobjects" || mode == "scriptableobject" ||
      mode == "so") {
    out = HeadlessScanMode::ScriptableObjects;
    return true;
  }

  return false;
}

inline const char *HeadlessScanModeName(HeadlessScanMode mode) {
  switch (mode) {
  case HeadlessScanMode::None:
    return "none";
  case HeadlessScanMode::AllMsid:
    return "msid";
  case HeadlessScanMode::GameObjects:
    return "gameobjects";
  case HeadlessScanMode::ScriptableObjects:
    return "scriptableobjects";
  default:
    return "unknown";
  }
}

inline bool HeadlessNeedsValue(const char *flag, int index, int argc,
                               HeadlessParseResult &result) {
  if (index + 1 < argc) {
    return true;
  }

  result.ok = false;
  result.error = std::string("missing value for ") + flag;
  return false;
}

inline HeadlessParseResult ParseHeadlessArgs(int argc, char **argv) {
  HeadlessParseResult result;
  result.ok = true;

  for (int i = 1; i < argc; ++i) {
    const std::string arg = argv[i] ? argv[i] : "";

    if (arg == "--headless") {
      result.options.headless = true;
    } else if (arg == "--help" || arg == "-h") {
      result.options.help = true;
    } else if (arg == "--target") {
      if (!HeadlessNeedsValue("--target", i, argc, result)) {
        return result;
      }
      result.options.targetProcess = argv[++i];
    } else if (arg == "--mode") {
      if (!HeadlessNeedsValue("--mode", i, argc, result)) {
        return result;
      }
      result.options.runMode = HeadlessToLowerAscii(argv[++i]);
    } else if (arg == "--scan") {
      if (!HeadlessNeedsValue("--scan", i, argc, result)) {
        return result;
      }
      if (!HeadlessParseScanMode(argv[++i], result.options.scanMode)) {
        result.ok = false;
        result.error = "invalid --scan value";
        return result;
      }
    } else if (arg == "--filter") {
      if (!HeadlessNeedsValue("--filter", i, argc, result)) {
        return result;
      }
      result.options.filter = argv[++i];
    } else if (arg == "--limit") {
      if (!HeadlessNeedsValue("--limit", i, argc, result)) {
        return result;
      }
      if (!HeadlessParseU32(argv[++i], result.options.limit)) {
        result.ok = false;
        result.error = "invalid --limit value";
        return result;
      }
    } else if (arg == "--klass") {
      if (!HeadlessNeedsValue("--klass", i, argc, result)) {
        return result;
      }
      if (!HeadlessParsePtr(argv[++i], result.options.klass)) {
        result.ok = false;
        result.error = "invalid --klass value";
        return result;
      }
    } else if (arg == "--scan-managers") {
      result.options.scanManagers = true;
    } else if (arg == "--manager-scan-mb") {
      if (!HeadlessNeedsValue("--manager-scan-mb", i, argc, result)) {
        return result;
      }
      if (!HeadlessParseU32(argv[++i], result.options.managerScanMaxMb)) {
        result.ok = false;
        result.error = "invalid --manager-scan-mb value";
        return result;
      }
    } else if (arg == "--out") {
      if (!HeadlessNeedsValue("--out", i, argc, result)) {
        return result;
      }
      result.options.outPath = argv[++i];
    } else if (arg == "--log") {
      if (!HeadlessNeedsValue("--log", i, argc, result)) {
        return result;
      }
      result.options.logPath = argv[++i];
    } else if (arg == "--offset-header") {
      if (!HeadlessNeedsValue("--offset-header", i, argc, result)) {
        return result;
      }
      result.options.offsetHeaderPath = argv[++i];
    } else if (arg == "--runtime-dump-cs") {
      if (!HeadlessNeedsValue("--runtime-dump-cs", i, argc, result)) {
        return result;
      }
      result.options.runtimeDumpCsPath = argv[++i];
    } else if (arg == "--runtime-dump-limit") {
      if (!HeadlessNeedsValue("--runtime-dump-limit", i, argc, result)) {
        return result;
      }
      if (!HeadlessParseU32(argv[++i], result.options.runtimeDumpLimit)) {
        result.ok = false;
        result.error = "invalid --runtime-dump-limit value";
        return result;
      }
    } else if (arg == "--runtime-dump-rvas") {
      if (!HeadlessNeedsValue("--runtime-dump-rvas", i, argc, result)) {
        return result;
      }
      if (!HeadlessParseU64List(argv[++i], result.options.runtimeDumpRvas)) {
        result.ok = false;
        result.error = "invalid --runtime-dump-rvas value";
        return result;
      }
    } else if (arg == "--metadata-out") {
      if (!HeadlessNeedsValue("--metadata-out", i, argc, result)) {
        return result;
      }
      result.options.metadataOutPath = argv[++i];
    } else if (arg == "--metadata-diagnose") {
      result.options.metadataDiagnose = true;
    } else if (arg == "--metadata-diagnose-mb") {
      if (!HeadlessNeedsValue("--metadata-diagnose-mb", i, argc, result)) {
        return result;
      }
      if (!HeadlessParseU32(argv[++i], result.options.metadataDiagnoseMaxMb)) {
        result.ok = false;
        result.error = "invalid --metadata-diagnose-mb value";
        return result;
      }
    } else if (arg == "--metadata-probe-va") {
      if (!HeadlessNeedsValue("--metadata-probe-va", i, argc, result)) {
        return result;
      }
      if (!HeadlessParsePtr(argv[++i], result.options.metadataProbeVa)) {
        result.ok = false;
        result.error = "invalid --metadata-probe-va value";
        return result;
      }
    } else if (arg == "--metadata-diagnose-anchor") {
      if (!HeadlessNeedsValue("--metadata-diagnose-anchor", i, argc, result)) {
        return result;
      }
      if (!HeadlessParsePtr(argv[++i],
                            result.options.metadataDiagnoseAnchor)) {
        result.ok = false;
        result.error = "invalid --metadata-diagnose-anchor value";
        return result;
      }
    } else if (arg == "--metadata-diagnose-heuristic") {
      result.options.metadataDiagnoseHeuristic = true;
    } else if (arg == "--dump-image") {
      if (!HeadlessNeedsValue("--dump-image", i, argc, result)) {
        return result;
      }
      result.options.dumpImageDir = argv[++i];
    } else if (arg == "--dump-image-all") {
      result.options.dumpImageAllModules = true;
    } else if (arg == "--dump-image-max-mb") {
      if (!HeadlessNeedsValue("--dump-image-max-mb", i, argc, result)) {
        return result;
      }
      if (!HeadlessParseU32(argv[++i], result.options.dumpImageMaxMb)) {
        result.ok = false;
        result.error = "invalid --dump-image-max-mb value";
        return result;
      }
    } else if (arg == "--self-dump-cs") {
      if (!HeadlessNeedsValue("--self-dump-cs", i, argc, result)) {
        return result;
      }
      result.options.selfDumpCsPath = argv[++i];
    } else if (arg == "--self-dump-cs-log") {
      if (!HeadlessNeedsValue("--self-dump-cs-log", i, argc, result)) {
        return result;
      }
      result.options.selfDumpCsLog = argv[++i];
    } else if (arg == "--self-dump-max-klass") {
      if (!HeadlessNeedsValue("--self-dump-max-klass", i, argc, result)) {
        return result;
      }
      if (!HeadlessParseU32(argv[++i], result.options.selfDumpMaxKlass)) {
        result.ok = false;
        result.error = "invalid --self-dump-max-klass value";
        return result;
      }
    } else if (arg == "--dma-inject") {
      if (!HeadlessNeedsValue("--dma-inject", i, argc, result)) {
        return result;
      }
      result.options.dmaInjectDll = argv[++i];
    } else if (arg == "--dma-inject-hook") {
      if (!HeadlessNeedsValue("--dma-inject-hook", i, argc, result)) {
        return result;
      }
      result.options.dmaInjectHook = argv[++i];
    } else if (arg == "--dma-inject-pid") {
      if (!HeadlessNeedsValue("--dma-inject-pid", i, argc, result)) {
        return result;
      }
      if (!HeadlessParseU32(argv[++i], result.options.dmaInjectPid)) {
        result.ok = false;
        result.error = "invalid --dma-inject-pid value";
        return result;
      }
    } else if (arg == "--dma-inject-wait-ms") {
      if (!HeadlessNeedsValue("--dma-inject-wait-ms", i, argc, result)) {
        return result;
      }
      if (!HeadlessParseU32(argv[++i], result.options.dmaInjectWaitMs)) {
        result.ok = false;
        result.error = "invalid --dma-inject-wait-ms value";
        return result;
      }
    } else if (arg == "--dma-inject-test") {
      result.options.dmaInjectTest = true;
    } else if (arg == "--naraka-buff-probe") {
      result.options.narakaBuffProbe = true;
    } else if (arg == "--naraka-buff-max-entries") {
      if (!HeadlessNeedsValue("--naraka-buff-max-entries", i, argc, result)) {
        return result;
      }
      if (!HeadlessParseU32(argv[++i], result.options.narakaBuffMaxEntries)) {
        result.ok = false;
        result.error = "invalid --naraka-buff-max-entries value";
        return result;
      }
    } else if (arg == "--naraka-buff-scan-end") {
      if (!HeadlessNeedsValue("--naraka-buff-scan-end", i, argc, result)) {
        return result;
      }
      if (!HeadlessParseU32(argv[++i], result.options.narakaBuffScanEnd)) {
        result.ok = false;
        result.error = "invalid --naraka-buff-scan-end value";
        return result;
      }
    } else if (arg == "--naraka-runtime-property-probe") {
      result.options.narakaRuntimePropertyProbe = true;
    } else if (arg == "--naraka-runtime-property-max-rows") {
      if (!HeadlessNeedsValue("--naraka-runtime-property-max-rows", i, argc,
                              result)) {
        return result;
      }
      if (!HeadlessParseU32(argv[++i],
                            result.options.narakaRuntimePropertyMaxRows)) {
        result.ok = false;
        result.error = "invalid --naraka-runtime-property-max-rows value";
        return result;
      }
    } else if (arg == "--naraka-runtime-property-samples") {
      if (!HeadlessNeedsValue("--naraka-runtime-property-samples", i, argc,
                              result)) {
        return result;
      }
      if (!HeadlessParseU32(argv[++i],
                            result.options.narakaRuntimePropertySamples)) {
        result.ok = false;
        result.error = "invalid --naraka-runtime-property-samples value";
        return result;
      }
    } else if (arg == "--naraka-runtime-property-interval-ms") {
      if (!HeadlessNeedsValue("--naraka-runtime-property-interval-ms", i, argc,
                              result)) {
        return result;
      }
      if (!HeadlessParseU32(
              argv[++i], result.options.narakaRuntimePropertyIntervalMs)) {
        result.ok = false;
        result.error = "invalid --naraka-runtime-property-interval-ms value";
        return result;
      }
    } else if (arg == "--naraka-runtime-property-csv") {
      if (!HeadlessNeedsValue("--naraka-runtime-property-csv", i, argc,
                              result)) {
        return result;
      }
      result.options.narakaRuntimePropertyCsvPath = argv[++i];
    } else if (arg == "--naraka-ground-probe") {
      result.options.narakaGroundProbe = true;
    } else if (arg == "--naraka-actor-container-probe") {
      result.options.narakaActorContainerProbe = true;
    } else if (arg == "--naraka-actor") {
      if (!HeadlessNeedsValue("--naraka-actor", i, argc, result)) {
        return result;
      }
      if (!HeadlessParsePtr(argv[++i], result.options.narakaActorAddress)) {
        result.ok = false;
        result.error = "invalid --naraka-actor value";
        return result;
      }
    } else if (arg == "--naraka-actor-max-entries") {
      if (!HeadlessNeedsValue("--naraka-actor-max-entries", i, argc,
                              result)) {
        return result;
      }
      if (!HeadlessParseU32(argv[++i],
                            result.options.narakaActorMaxEntries)) {
        result.ok = false;
        result.error = "invalid --naraka-actor-max-entries value";
        return result;
      }
    } else if (arg == "--naraka-actor-scan-end") {
      if (!HeadlessNeedsValue("--naraka-actor-scan-end", i, argc, result)) {
        return result;
      }
      if (!HeadlessParseU32(argv[++i], result.options.narakaActorScanEnd)) {
        result.ok = false;
        result.error = "invalid --naraka-actor-scan-end value";
        return result;
      }
    } else if (arg == "--naraka-actor-any-hero") {
      result.options.narakaActorOnlyTarka = false;
    } else {
      result.ok = false;
      result.error = std::string("unknown argument: ") + arg;
      return result;
    }
  }

  return result;
}

inline std::string HeadlessJsonEscape(const std::string &value) {
  std::ostringstream os;
  for (const unsigned char ch : value) {
    switch (ch) {
    case '"':
      os << "\\\"";
      break;
    case '\\':
      os << "\\\\";
      break;
    case '\b':
      os << "\\b";
      break;
    case '\f':
      os << "\\f";
      break;
    case '\n':
      os << "\\n";
      break;
    case '\r':
      os << "\\r";
      break;
    case '\t':
      os << "\\t";
      break;
    default:
      if (ch < 0x20 || ch >= 0x7F) {
        os << "\\u" << std::hex << std::setw(4) << std::setfill('0')
           << static_cast<int>(ch) << std::dec;
      } else {
        os << static_cast<char>(ch);
      }
      break;
    }
  }
  return os.str();
}

inline std::string HeadlessJsonString(const std::string &value) {
  return std::string("\"") + HeadlessJsonEscape(value) + "\"";
}

inline std::string HexPtr(std::uintptr_t value) {
  std::ostringstream os;
  os << "0x" << std::uppercase << std::hex
     << static_cast<unsigned long long>(value);
  return os.str();
}

} // namespace er2
