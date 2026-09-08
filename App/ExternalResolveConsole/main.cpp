#ifndef NOMINMAX
#define NOMINMAX
#endif

#include "Resolve202x.hpp"

#include "../UnityExplorer/MetickAdapter.hpp"

#include "er2/unity2/headless/probe_cli.hpp"
#include "er2/unity2/headless/runtime_dump_cs.hpp"
#include "er2/unity2/headless/runtime_type_resolver.hpp"
#include "er2/unity2/headless/manager_rva_scan.hpp"
#include "er2/unity2/headless/self_dump_cs.hpp"
#include "er2/unity2/headless/dma_inject.hpp"
#include "er2/unity2/inspect/klass_header_probe.hpp"
#include "er2/unity2/inspect/native_chain_probe.hpp"
#include "er2/unity2/inspect/runtime_class_probe.hpp"
#include "er2/unity2/metadata/export.hpp"
#include "er2/unity2/metadata/metadata_header_fields.hpp"
#include "er2/unity2/metadata/il2cpp_layout_profile.hpp"
#include "er2/unity2/msid/enumerate_objects.hpp"
#include "er2/unity2/msid/msid_scan.hpp"
#include "er2/unity2/naraka/actor_container_probe.hpp"
#include "er2/unity2/naraka/buff_manager_probe.hpp"
#include "er2/unity2/object/managed/il2cpp_class.hpp"
#include "er2/unity2/object/managed/managed_object.hpp"
#include "er2/unity2/init/module_match.hpp"

#include <Windows.h>

#include <algorithm>
#include <chrono>
#include <cmath>
#include <cstdint>
#include <cstdio>
#include <fstream>
#include <map>
#include <memory>
#include <optional>
#include <set>
#include <sstream>
#include <string>
#include <thread>
#include <unordered_map>
#include <vector>

namespace {

struct HeadlessModuleState {
  std::string unityPlayerName;
  std::string gameAssemblyName;
  std::size_t moduleCount = 0;
  bool unityDosOk = false;
  bool gameAssemblyDosOk = false;
};

struct HeadlessObjectRow {
  er2::ObjectInfo info;
  std::uintptr_t managed = 0;
  std::uintptr_t klass = 0;
};

struct HeadlessPointerWordsReport {
  std::uintptr_t address = 0;
  std::vector<std::uintptr_t> words;
};

struct HeadlessFieldReport {
  std::string name;
  std::string typeName;
  std::uintptr_t type = 0;
  std::uintptr_t typeData = 0;
  std::uint32_t typeMetaBits = 0;
  std::uint32_t typeEnum = 0;
  std::string typeKind;
  std::vector<std::uintptr_t> typeDataWords;
  std::vector<HeadlessPointerWordsReport> typeDataWordSamples;
  std::uint32_t offset = 0;
  bool isStatic = false;
};

struct HeadlessMethodReport {
  std::string name;
  std::string returnTypeName;
  std::uintptr_t methodInfo = 0;
  std::uintptr_t methodPointer = 0;
  std::uint64_t methodRva = 0;
  std::uintptr_t invokerMethod = 0;
  std::uintptr_t returnType = 0;
  std::uintptr_t parameters = 0;
  std::uint32_t token = 0;
  std::uint16_t flags = 0;
  std::uint16_t iflags = 0;
  std::uint16_t slot = 0xFFFFu;
  std::uint8_t parameterCount = 0;
};

struct HeadlessKlassReport {
  std::uintptr_t klass = 0;
  std::vector<std::string> sources;
  std::vector<std::uintptr_t> instances;
  bool runtimeValid = false;
  er2::RuntimeClassProbeResult runtime;
  er2::KlassHeaderProbeResult header;
  std::uintptr_t parent = 0;
  std::uintptr_t fields = 0;
  std::uintptr_t methods = 0;
  std::uintptr_t staticFields = 0;
  std::uint16_t fieldCount = 0;
  std::uint16_t methodCount = 0;
  std::vector<HeadlessFieldReport> fieldsSample;
  std::vector<HeadlessMethodReport> methodsSample;
};

struct LegacyRvaCandidate {
  const char *name = "";
  std::uint64_t normalRva = 0;
  std::uint64_t superRva = 0;
  const char *comment = "";
};

struct LegacyRvaValidation {
  LegacyRvaCandidate candidate;
  std::uint64_t selectedRva = 0;
  std::uintptr_t address = 0;
  bool readable = false;
  std::uintptr_t pointerValue = 0;
  bool pointerCanonical = false;
};

struct HeadlessMetadataExportReport {
  bool requested = false;
  bool ok = false;
  std::string path;
  std::string error;
  std::size_t bytes = 0;
  std::uint32_t magic = 0;
  std::uint32_t version = 0;
  er2::MetadataHeaderFields header;
};

struct HeadlessMetadataProbeReport {
  std::uintptr_t address = 0;
  bool readOk = false;
  bool headerOk = false;
  std::uint32_t magic = 0;
  std::uint32_t version = 0;
  int score = 0;
  std::uint32_t maxEnd = 0;
  er2::MetadataHeaderFields header;
};

struct HeadlessMetadataRawCandidate {
  std::uintptr_t address = 0;
  std::string regionText;
  bool privateMemory = false;
  bool image = false;
  std::uint32_t protection = 0;
  std::uint32_t version = 0;
  int score = 0;
  std::uint32_t maxEnd = 0;
  er2::MetadataHeaderFields header;
};

struct HeadlessMetadataDiagnosticsReport {
  bool requested = false;
  bool ok = false;
  std::string error;
  std::uint32_t vadCount = 0;
  std::uint32_t regionsScanned = 0;
  std::uint64_t bytesScanned = 0;
  std::uint32_t readFailures = 0;
  std::uint32_t magicHits = 0;
  std::uint32_t heuristicHits = 0;
  std::uintptr_t anchor = 0;
  std::uint32_t anchorVadHits = 0;
  HeadlessMetadataProbeReport probe;
  std::vector<HeadlessMetadataRawCandidate> candidates;
};

struct HeadlessRuntimeDumpReport {
  bool requested = false;
  bool ok = false;
  std::string path;
  std::string error;
  std::size_t classCount = 0;
  std::size_t fieldCount = 0;
  std::size_t methodCount = 0;
};

struct HeadlessNarakaBuffProbeReport {
  bool requested = false;
  bool ok = false;
  std::string error;
  er2::NarakaBuffManagerSnapshot snapshot;
};

struct HeadlessNarakaRuntimePropertyProbeReport {
  bool requested = false;
  bool ok = false;
  std::string error;
  std::uintptr_t actorModel = 0;
  std::string csvPath;
  std::vector<er2::NarakaRuntimePropertySnapshot> snapshots;
  std::vector<er2::NarakaRuntimePropertyDiffRow> diff;
};

struct HeadlessNarakaGroundProbeReport {
  bool requested = false;
  bool ok = false;
  std::string error;
  std::uintptr_t characterManager = 0;
  std::uintptr_t actorModel = 0;
  std::uintptr_t actorKit = 0;
  std::uintptr_t actorPhysics = 0;
  std::uintptr_t advanceGroundStatus = 0;
  std::uintptr_t alignToGroundData = 0;
  std::int32_t currentCollisionFlags = 0;
  bool isGroundedByCollisionFlags = false;
  bool advanceNearGround = false;
  float advanceGroundDistance = 0.0f;
  bool alignNearGround = false;
  float alignGroundDistance = 0.0f;
  float alignRootGroundDistance = 0.0f;
};

struct HeadlessNarakaActorContainerProbeReport {
  bool requested = false;
  bool ok = false;
  std::string error;
  er2::NarakaActorContainerSnapshot snapshot;
};

struct HeadlessModuleImageReport {
  bool requested = false;
  bool ok = false;
  std::string error;
  std::string gameAssemblyName;
  std::string gameAssemblyPath;
  std::uint64_t gameAssemblyBytes = 0;
  std::string unityPlayerPath;
  std::uint64_t unityPlayerBytes = 0;
  std::size_t moduleCount = 0;
};

struct HeadlessSelfDumpReport {
  bool requested = false;
  bool ok = false;
  std::string error;
  std::string path;
  std::uintptr_t typeInfoTable = 0;
  std::uint32_t typeInfoCount = 0;
  std::size_t klassResolved = 0;
  std::size_t fieldRows = 0;
  std::size_t methodRows = 0;
  std::size_t methodRva = 0;
};

struct HeadlessDmaInjectReport {
  bool requested = false;
  bool ok = false;
  std::string error;
  std::string dllPath;
  std::string moduleName;
  std::string hookPoint;
  std::uintptr_t iatThunk = 0;
  std::uintptr_t iatFunction = 0;
  std::uintptr_t codeCave = 0;
  std::uintptr_t dataCave = 0;
  std::uintptr_t loadLibraryAddr = 0;
  std::uintptr_t loadLibraryResult = 0;
  bool executed = false;
  bool restored = false;
};

struct HeadlessRunResult {
  bool ok = false;
  std::string error;
  er2::HeadlessOptions options;
  HeadlessModuleState modules;
  std::uintptr_t msidSlot = 0;
  std::uint32_t msidScanScore = 0;
  std::uint32_t msidCount = 0;
  std::size_t matchedObjectCount = 0;
  std::vector<HeadlessObjectRow> objects;
  std::vector<HeadlessKlassReport> klassReports;
  std::vector<LegacyRvaValidation> legacyCandidates;
  HeadlessMetadataExportReport metadataExport;
  HeadlessMetadataDiagnosticsReport metadataDiagnostics;
  HeadlessRuntimeDumpReport runtimeDump;
  HeadlessNarakaBuffProbeReport narakaBuffProbe;
  HeadlessNarakaRuntimePropertyProbeReport narakaRuntimePropertyProbe;
  HeadlessNarakaGroundProbeReport narakaGroundProbe;
  HeadlessNarakaActorContainerProbeReport narakaActorContainerProbe;
  er2::ManagerRvaScanResult managerScan;
  HeadlessModuleImageReport moduleImages;
  HeadlessSelfDumpReport selfDump;
  HeadlessDmaInjectReport dmaInject;
  std::vector<std::string> logs;
};

// Updated from YJWJ_DMA_NEW/Naraka/Offset.h (verified production offsets)
const LegacyRvaCandidate kLegacyRvaCandidates[] = {
    {"Base_WindowBase", 0x2795D38ull, 0x3676AE0ull, "window size base"},
    {"m_GlobalTime", 0xe142ac8ull, 0x3668A90ull, "Class: GameBaseObject"},
    {"m_CharacterManager", 0xe1406f0ull, 0x365F150ull,
     "Class: CharacterManager"},
    {"m_UserDataManager", 0xe142348ull, 0x36849E8ull, "Class: UserData"},
    {"m_CharactorSync", 0xe106e10ull, 0x365F1F8ull, "Class: CharactorSync"},
    {"m_EntityManager", 0xe141380ull, 0x3668B48ull, "Class: GameEntity"},
    {"m_BuffManager", 0xE13FC68ull, 0x365DD80ull,
     "BuffManager TypeInfo/class candidate"},
    {"m_TransparentTagHandler", 0xDFABA70ull, 0x367D078ull,
     "Class: TransparentTagHandler"},
};

void PrintUsage() {
  std::puts("Unity headless DMA probe");
  std::puts("");
  std::puts("Usage:");
  std::puts("  ExternalResolveConsole.exe --headless --target NarakaBladepoint.exe "
            "--scan gameobjects --limit 256 --out headless_probe.json");
  std::puts("");
  std::puts("Options:");
  std::puts("  --headless             Run one probe and exit");
  std::puts("  --target <exe>         Target process name");
  std::puts("  --mode <dma>           Access mode. First version supports dma");
  std::puts("  --scan <mode>          none | gameobjects | scriptableobjects | msid");
  std::puts("  --filter <text>        Case-insensitive object/type filter");
  std::puts("  --limit <n>            Max object rows stored in JSON");
  std::puts("  --klass <address>      Extra Il2CppClass* to inspect");
  std::puts("  --scan-managers        Scan current GameAssembly for manager RVAs");
  std::puts("  --manager-scan-mb <n>  Optional max bytes per section for manager scan");
  std::puts("  --out <path>           JSON output path");
  std::puts("  --log <path>           Optional plain text log output path");
  std::puts("  --offset-header <path> Optional Offset.h-like C++ header output");
  std::puts("  --runtime-dump-cs <path> Optional runtime-only dump.cs output");
  std::puts("  --runtime-dump-limit <n> Max runtime classes/methods sampled");
  std::puts("  --runtime-dump-rvas <csv> Extra GameAssembly RVAs whose slots hold TypeInfo");
  std::puts("  --metadata-out <path>  Optional DMA global-metadata.dat export");
  std::puts("  --metadata-diagnose    Scan VAD memory for metadata magic");
  std::puts("  --metadata-diagnose-mb <n> Max VAD bytes for metadata diagnostics");
  std::puts("  --metadata-probe-va <address> Probe one metadata candidate VA");
  std::puts("  --metadata-diagnose-anchor <address> Scan only VADs near this VA");
  std::puts("  --metadata-diagnose-heuristic Include header-shape candidates without magic");
  std::puts("  --dump-image <dir>          DMA-dump GameAssembly/UnityPlayer PE images to dir");
  std::puts("  --dump-image-all            Also dump all loaded modules to <dir>/modules/");
  std::puts("  --dump-image-max-mb <n>     Optional per-module size cap for --dump-image-all");
  std::puts("  --self-dump-cs <path>       Self-contained runtime dump.cs (no injector, no metadata scan)");
  std::puts("  --self-dump-cs-log <path>   Optional log for SelfDumpCs");
  std::puts("  --self-dump-max-klass <n>   Optional klass iteration cap for SelfDumpCs");
  std::puts("  --dma-inject <dll>          DMA-based LoadLibrary injection (no handle/thread)");
  std::puts("  --dma-inject-hook <m!f>     Hook point, default kernel32.dll!Sleep");
  std::puts("  --dma-inject-pid <pid>      Target pid (default: attached pid)");
  std::puts("  --dma-inject-wait-ms <n>    Max wait for shellcode execution, default 15000");
  std::puts("  --dma-inject-test           Verify execution path only (skip LoadLibraryA)");
  std::puts("  --naraka-buff-probe  Probe Naraka BuffManager TypeInfo/static fields");
  std::puts("  --naraka-buff-max-entries <n> Max BuffManager entries sampled");
  std::puts("  --naraka-buff-scan-end <hex> Last owner field offset scanned");
  std::puts("  --naraka-runtime-property-probe Probe selected/local Actor RuntimePropertyData");
  std::puts("  --naraka-runtime-property-max-rows <n> Max RuntimeProperty rows sampled");
  std::puts("  --naraka-runtime-property-samples <n> Number of RuntimeProperty samples");
  std::puts("  --naraka-runtime-property-interval-ms <n> Delay between samples");
  std::puts("  --naraka-runtime-property-csv <path> Optional full RuntimeProperty CSV output");
  std::puts("  --naraka-ground-probe Read local ActorKit ground/collision state");
  std::puts("  --naraka-actor-container-probe Scan selected/local ActorModel containers");
  std::puts("  --naraka-actor <hex> Manual ActorModel address");
  std::puts("  --naraka-actor-max-entries <n> Max Actor container entries sampled");
  std::puts("  --naraka-actor-scan-end <hex> Last Actor owner field offset scanned");
  std::puts("  --naraka-actor-any-hero Do not prefer HeroID 1000006 when auto-picking");
}

void AddRunLog(HeadlessRunResult &run, const std::string &message) {
  run.logs.push_back(message);
  std::printf("%s\n", message.c_str());
}

bool ReadDosHeaderOk(const er2::IMemoryAccessor &mem, std::uintptr_t base) {
  if (!base) {
    return false;
  }

  IMAGE_DOS_HEADER dos{};
  return mem.Read(base, &dos, sizeof(dos)) && dos.e_magic == IMAGE_DOS_SIGNATURE;
}

std::string ObjectKindName(er2::ObjectKind kind) {
  switch (kind) {
  case er2::ObjectKind::GameObject:
    return "GameObject";
  case er2::ObjectKind::ScriptableObject:
    return "ScriptableObject";
  case er2::ObjectKind::Other:
    return "Other";
  default:
    return "Unknown";
  }
}

bool TryReadManagedAndKlassFromNative(
    const er2::IMemoryAccessor &mem, std::uintptr_t native,
    const er2::Il2CppLayoutProfile *profile, std::uintptr_t &managed,
    std::uintptr_t &klass) {
  managed = 0;
  klass = 0;

  if (!profile) {
    return false;
  }

  std::uintptr_t directManaged = 0;
  std::uintptr_t directKlass = 0;
  if (er2::ReadPtr(mem, native + er2::g_ctx.off.unity_object_managed_ptr,
                   directManaged) &&
      er2::IsCanonicalUserPtr(directManaged) &&
      er2::ReadPtr(mem, directManaged, directKlass) &&
      er2::IsCanonicalUserPtr(directKlass)) {
    er2::RuntimeClassProbeResult runtime;
    if (er2::ProbeRuntimeClass(mem, directKlass, *profile, runtime)) {
      managed = directManaged;
      klass = directKlass;
      return true;
    }
  }

  er2::NativeChainProbeResult chain;
  (void)er2::ProbeNativeObjectChain(mem, native, *profile, chain);
  if (chain.bestIndex < 0 ||
      chain.bestIndex >= static_cast<int>(chain.candidates.size())) {
    return false;
  }

  const er2::NativeChainProbeCandidate &best = chain.best();
  if (!er2::IsCanonicalUserPtr(best.managed) ||
      !er2::IsCanonicalUserPtr(best.klass)) {
    return false;
  }

  managed = best.managed;
  klass = best.klass;
  return true;
}

bool TryResolveRuntimeIl2CppTypeName(const er2::IMemoryAccessor &mem,
                                     const er2::Il2CppLayoutProfile &profile,
                                     std::uintptr_t typePtr,
                                     std::uintptr_t &typeData,
                                     std::uint32_t &typeMetaBits,
                                     std::uint32_t &typeEnum,
                                     std::string &typeName) {
  typeData = 0;
  typeMetaBits = 0;
  typeEnum = 0;
  typeName.clear();

  er2::RuntimeIl2CppTypeDescription desc;
  const bool described =
      er2::DescribeRuntimeIl2CppType(mem, profile, typePtr, desc);
  typeData = desc.data;
  typeMetaBits = desc.metaBits;
  typeEnum = desc.typeEnum;
  typeName = desc.name;
  return described && !typeName.empty();
}

void ReadFieldSample(const er2::IMemoryAccessor &mem,
                     const er2::Il2CppLayoutProfile &profile,
                     const HeadlessKlassReport &klass,
                     std::vector<HeadlessFieldReport> &out,
                     std::uint16_t maxFields = 64) {
  out.clear();

  if (!er2::IsCanonicalUserPtr(klass.fields) || klass.fieldCount == 0 ||
      klass.fieldCount > 4096) {
    return;
  }

  const std::uint16_t count =
      klass.fieldCount < maxFields ? klass.fieldCount : maxFields;
  out.reserve(count);

  for (std::uint16_t i = 0; i < count; ++i) {
    const std::uintptr_t fieldInfo =
        klass.fields +
        static_cast<std::uintptr_t>(i) * profile.fieldInfo.size;

    HeadlessFieldReport field;
    std::uintptr_t namePtr = 0;
    if (er2::ReadPtr(mem, fieldInfo + profile.fieldInfo.name, namePtr) &&
        er2::IsCanonicalUserPtr(namePtr)) {
      (void)er2::ReadRuntimeCString(mem, namePtr, field.name, 192);
    }

    std::int32_t rawOffset = 0;
    if (er2::ReadValue(mem, fieldInfo + profile.fieldInfo.offset,
                       rawOffset) &&
        rawOffset >= 0) {
      field.offset = static_cast<std::uint32_t>(rawOffset);
    }

    std::uintptr_t typePtr = 0;
    if (er2::ReadPtr(mem, fieldInfo + profile.fieldInfo.type, typePtr) &&
        er2::IsCanonicalUserPtr(typePtr)) {
      field.type = typePtr;
      std::uint16_t attrs = 0;
      if (er2::ReadValue(mem, typePtr + er2::g_ctx.off.il2cpp_type_attrs,
                         attrs)) {
        field.isStatic = (attrs & er2::FIELD_ATTRIBUTE_STATIC) != 0;
      }
      (void)TryResolveRuntimeIl2CppTypeName(
          mem, profile, typePtr, field.typeData, field.typeMetaBits,
          field.typeEnum, field.typeName);
      field.typeKind = er2::RuntimeIl2CppTypeKindName(field.typeEnum);
      if (field.typeEnum == 0x15u &&
          er2::IsCanonicalUserPtr(field.typeData)) {
        (void)er2::SampleRuntimePointerWords(mem, field.typeData,
                                             field.typeDataWords);
        for (std::uintptr_t word : field.typeDataWords) {
          if (!er2::IsCanonicalUserPtr(word)) {
            continue;
          }

          HeadlessPointerWordsReport nested;
          nested.address = word;
          if (er2::SampleRuntimePointerWords(mem, word, nested.words, 6)) {
            field.typeDataWordSamples.push_back(std::move(nested));
          }
        }
      }
    }

    out.push_back(std::move(field));
  }
}

bool IsReadableRuntimeName(const std::string &name) {
  if (name.empty()) {
    return true;
  }

  for (unsigned char ch : name) {
    if (ch < 0x20 || ch > 0x7E) {
      return false;
    }
  }
  return true;
}

bool IsIdentifierLikeRuntimeName(const std::string &name,
                                 bool allowDots = false) {
  if (name.empty() || name.size() > 192 || !IsReadableRuntimeName(name)) {
    return false;
  }

  const unsigned char first = static_cast<unsigned char>(name.front());
  if ((first >= '0' && first <= '9') || name.front() == '.') {
    return false;
  }

  for (unsigned char ch : name) {
    const bool alpha = (ch >= 'A' && ch <= 'Z') || (ch >= 'a' && ch <= 'z');
    const bool digit = ch >= '0' && ch <= '9';
    const bool ok = alpha || digit || ch == '_' || ch == '`' || ch == '+' ||
                    (allowDots && ch == '.');
    if (!ok) {
      return false;
    }
  }

  return true;
}

bool IsReadableRuntimeMethodName(const std::string &name) {
  if (name.empty()) {
    return true;
  }

  if (name.size() > 192 || !IsReadableRuntimeName(name)) {
    return false;
  }

  for (unsigned char ch : name) {
    const bool alpha = (ch >= 'A' && ch <= 'Z') || (ch >= 'a' && ch <= 'z');
    const bool digit = ch >= '0' && ch <= '9';
    const bool ok = alpha || digit || ch == '_' || ch == '.' || ch == '<' ||
                    ch == '>' || ch == '`' || ch == '+' || ch == '-';
    if (!ok) {
      return false;
    }
  }

  return true;
}

bool IsPlausibleRuntimeField(const HeadlessFieldReport &field) {
  return er2::IsCanonicalUserPtr(field.type) && field.offset < 0x10000 &&
         IsReadableRuntimeName(field.name);
}

std::size_t CountTrustedRuntimeFieldPrefix(
    const std::vector<HeadlessFieldReport> &fields) {
  if (fields.empty()) {
    return 0;
  }

  std::size_t trusted = 0;
  for (const HeadlessFieldReport &field : fields) {
    if (!IsPlausibleRuntimeField(field)) {
      break;
    }

    ++trusted;
  }

  return trusted;
}

bool IsRuntimeAddressInsideGameAssembly(std::uintptr_t address) {
  return er2::g_ctx.gameAssembly.base && er2::g_ctx.gameAssembly.size &&
         address >= er2::g_ctx.gameAssembly.base &&
         address < er2::g_ctx.gameAssembly.base + er2::g_ctx.gameAssembly.size;
}

bool IsLikelyRuntimeMethodToken(std::uint32_t token) {
  return token == 0 || (token & 0xFF000000u) == 0x06000000u;
}

bool IsPlausibleRuntimeMethod(const HeadlessMethodReport &method) {
  if (!er2::IsCanonicalUserPtr(method.methodInfo)) {
    return false;
  }

  if (method.methodPointer != 0 &&
      !IsRuntimeAddressInsideGameAssembly(method.methodPointer)) {
    return false;
  }

  if (method.invokerMethod != 0 &&
      !er2::IsCanonicalUserPtr(method.invokerMethod)) {
    return false;
  }

  if (method.returnType != 0 && !er2::IsCanonicalUserPtr(method.returnType)) {
    return false;
  }

  if (method.parameters != 0 && !er2::IsCanonicalUserPtr(method.parameters)) {
    return false;
  }

  if (!IsLikelyRuntimeMethodToken(method.token)) {
    return false;
  }

  if (method.parameterCount > 32) {
    return false;
  }

  if (method.slot != 0xFFFFu && method.slot > 4096u) {
    return false;
  }

  if (method.flags > 0x3FFFu || method.iflags > 0x3FFFu) {
    return false;
  }

  if (method.methodPointer == 0 && method.token == 0 &&
      method.name.empty()) {
    return false;
  }

  return true;
}

struct RuntimeDumpExportDecision {
  bool exportable = false;
  std::size_t trustedFieldPrefix = 0;
  std::string reason;
};

RuntimeDumpExportDecision
ExplainRuntimeDumpExportDecision(const HeadlessKlassReport &klass) {
  RuntimeDumpExportDecision decision;
  decision.trustedFieldPrefix =
      CountTrustedRuntimeFieldPrefix(klass.fieldsSample);

  const bool hasRuntimeName =
      klass.runtimeValid && !klass.runtime.className.empty() &&
      IsIdentifierLikeRuntimeName(klass.runtime.className) &&
      (klass.runtime.namespaze.empty() ||
       IsIdentifierLikeRuntimeName(klass.runtime.namespaze, true));
  const bool hasHeaderName =
      klass.header.valid && !klass.header.bestName.empty() &&
      IsIdentifierLikeRuntimeName(klass.header.bestName) &&
      (klass.header.bestNamespace.empty() ||
       IsIdentifierLikeRuntimeName(klass.header.bestNamespace, true));
  const bool hasSourceEvidence = !klass.sources.empty();
  const bool hasInstanceEvidence = !klass.instances.empty();
  const bool hasMethodEvidence =
      er2::IsCanonicalUserPtr(klass.methods) && klass.methodCount > 0 &&
      klass.methodCount <= 4096 && !klass.methodsSample.empty();
  const bool hasFieldEvidence =
      er2::IsCanonicalUserPtr(klass.fields) && klass.fieldCount > 0 &&
      klass.fieldCount <= 4096 && !klass.fieldsSample.empty() &&
      decision.trustedFieldPrefix > 0;

  if ((hasRuntimeName || hasHeaderName) &&
      (hasFieldEvidence || hasMethodEvidence)) {
    decision.exportable = true;
    decision.reason = "named class with runtime member evidence";
    return decision;
  }

  if (klass.runtimeValid && !klass.runtime.className.empty() &&
      (!IsIdentifierLikeRuntimeName(klass.runtime.className) ||
       (!klass.runtime.namespaze.empty() &&
        !IsIdentifierLikeRuntimeName(klass.runtime.namespaze, true)))) {
    decision.reason = "runtime class name is not printable";
    return decision;
  }

  if (klass.fieldCount > 4096) {
    decision.reason = "raw field count exceeds safety limit";
    return decision;
  }

  if (klass.methodCount > 4096) {
    decision.reason = "raw method count exceeds safety limit";
    return decision;
  }

  if (hasFieldEvidence && decision.trustedFieldPrefix >= 4) {
    decision.exportable = true;
    decision.reason = "trusted runtime field table prefix";
    return decision;
  }

  if ((hasRuntimeName || hasHeaderName || hasSourceEvidence ||
       hasInstanceEvidence) &&
      (hasFieldEvidence || hasMethodEvidence)) {
    decision.exportable = true;
    decision.reason = "partial DMA runtime evidence";
    return decision;
  }

  if (!er2::IsCanonicalUserPtr(klass.fields) && klass.fieldCount != 0) {
    decision.reason = "fields pointer is not canonical";
    return decision;
  }

  if (klass.fieldCount == 0 && klass.methodCount == 0) {
    decision.reason = "no runtime fields or methods";
    return decision;
  }

  if (!hasFieldEvidence && !hasMethodEvidence) {
    decision.reason = "no readable runtime member sample";
    return decision;
  }

  std::ostringstream reason;
  reason << "weak anonymous member evidence, trusted field prefix="
         << decision.trustedFieldPrefix;
  decision.reason = reason.str();
  return decision;
}

void ReadMethodSample(const er2::IMemoryAccessor &mem,
                      const er2::Il2CppLayoutProfile &profile,
                      const HeadlessKlassReport &klass,
                      std::vector<HeadlessMethodReport> &out,
                      std::uint16_t maxMethods = 64) {
  out.clear();

  if (!er2::IsCanonicalUserPtr(klass.methods) || klass.methodCount == 0 ||
      klass.methodCount > 4096) {
    return;
  }

  const std::uint16_t count =
      klass.methodCount < maxMethods ? klass.methodCount : maxMethods;
  out.reserve(count);

  for (std::uint16_t i = 0; i < count; ++i) {
    std::uintptr_t methodInfo = 0;
    if (!er2::ReadPtr(mem,
                      klass.methods +
                          static_cast<std::uintptr_t>(i) *
                              sizeof(std::uintptr_t),
                      methodInfo) ||
        !er2::IsCanonicalUserPtr(methodInfo)) {
      continue;
    }

    HeadlessMethodReport method;
    method.methodInfo = methodInfo;
    (void)er2::ReadPtr(mem, methodInfo + profile.methodInfo.methodPointer,
                       method.methodPointer);
    (void)er2::ReadPtr(mem, methodInfo + profile.methodInfo.invokerMethod,
                       method.invokerMethod);
    (void)er2::ReadPtr(mem, methodInfo + profile.methodInfo.returnType,
                       method.returnType);
    (void)er2::ReadPtr(mem, methodInfo + profile.methodInfo.parameters,
                       method.parameters);
    (void)er2::ReadValue(mem, methodInfo + profile.methodInfo.token,
                         method.token);
    (void)er2::ReadValue(mem, methodInfo + profile.methodInfo.flags,
                         method.flags);
    (void)er2::ReadValue(mem, methodInfo + profile.methodInfo.iflags,
                         method.iflags);
    (void)er2::ReadValue(mem, methodInfo + profile.methodInfo.slot,
                         method.slot);
    (void)er2::ReadValue(mem, methodInfo + profile.methodInfo.parameterCount,
                         method.parameterCount);
    if (IsRuntimeAddressInsideGameAssembly(method.methodPointer)) {
      method.methodRva = static_cast<std::uint64_t>(
          method.methodPointer - er2::g_ctx.gameAssembly.base);
    }
    std::uintptr_t returnTypeData = 0;
    std::uint32_t returnTypeMetaBits = 0;
    std::uint32_t returnTypeEnum = 0;
    (void)TryResolveRuntimeIl2CppTypeName(
        mem, profile, method.returnType, returnTypeData, returnTypeMetaBits,
        returnTypeEnum, method.returnTypeName);
    if (method.returnTypeName.empty()) {
      method.returnTypeName = "System.Void";
    }

    std::uintptr_t namePtr = 0;
    if (er2::ReadPtr(mem, methodInfo + profile.methodInfo.name, namePtr) &&
        er2::IsCanonicalUserPtr(namePtr)) {
      (void)er2::ReadRuntimeCString(mem, namePtr, method.name, 192);
    }

    if (!IsReadableRuntimeMethodName(method.name)) {
      method.name.clear();
    }

    if (!IsPlausibleRuntimeMethod(method)) {
      continue;
    }

    out.push_back(std::move(method));
  }
}

HeadlessKlassReport BuildKlassReport(const er2::IMemoryAccessor &mem,
                                      std::uintptr_t klass,
                                      const er2::Il2CppLayoutProfile &profile,
                                      std::vector<std::string> sources = {},
                                      std::vector<std::uintptr_t> instances = {}) {
  HeadlessKlassReport report;
  report.klass = klass;
  report.sources = std::move(sources);
  report.instances = std::move(instances);

  report.runtimeValid = er2::ProbeRuntimeClass(mem, klass, profile,
                                               report.runtime);
  (void)er2::ProbeKlassHeaderStrings(mem, klass, report.header);

  (void)er2::ReadPtr(mem, klass + profile.klass.parent, report.parent);
  (void)er2::ReadPtr(mem, klass + profile.klass.fields, report.fields);
  (void)er2::ReadPtr(mem, klass + profile.klass.methods, report.methods);
  (void)er2::ReadPtr(mem, klass + profile.klass.staticFields,
                     report.staticFields);
  (void)er2::ReadValue(mem, klass + profile.klass.fieldCount,
                       report.fieldCount);
  (void)er2::ReadValue(mem, klass + profile.klass.methodCount,
                       report.methodCount);

  ReadFieldSample(mem, profile, report, report.fieldsSample);
  ReadMethodSample(mem, profile, report, report.methodsSample);
  return report;
}

void AppendFieldReportJson(std::ostringstream &json,
                           const HeadlessFieldReport &field,
                           const std::string &indent) {
  json << indent << "{";
  json << "\"name\":" << er2::HeadlessJsonString(field.name) << ",";
  json << "\"offset\":"
       << er2::HeadlessJsonString(er2::HexPtr(field.offset)) << ",";
  json << "\"type\":" << er2::HeadlessJsonString(er2::HexPtr(field.type))
       << ",";
  json << "\"typeData\":"
       << er2::HeadlessJsonString(er2::HexPtr(field.typeData)) << ",";
  json << "\"typeMetaBits\":"
       << er2::HeadlessJsonString(er2::HexPtr(field.typeMetaBits)) << ",";
  json << "\"typeEnum\":"
       << er2::HeadlessJsonString(er2::HexPtr(field.typeEnum)) << ",";
  json << "\"typeKind\":" << er2::HeadlessJsonString(field.typeKind)
       << ",";
  json << "\"typeName\":" << er2::HeadlessJsonString(field.typeName)
       << ",";
  json << "\"typeDataWords\":[";
  for (std::size_t i = 0; i < field.typeDataWords.size(); ++i) {
    if (i != 0) {
      json << ",";
    }
    json << er2::HeadlessJsonString(er2::HexPtr(field.typeDataWords[i]));
  }
  json << "],";
  json << "\"typeDataWordSamples\":[";
  for (std::size_t i = 0; i < field.typeDataWordSamples.size(); ++i) {
    const HeadlessPointerWordsReport &sample = field.typeDataWordSamples[i];
    if (i != 0) {
      json << ",";
    }
    json << "{\"address\":"
         << er2::HeadlessJsonString(er2::HexPtr(sample.address))
         << ",\"words\":[";
    for (std::size_t wordIndex = 0; wordIndex < sample.words.size();
         ++wordIndex) {
      if (wordIndex != 0) {
        json << ",";
      }
      json << er2::HeadlessJsonString(er2::HexPtr(sample.words[wordIndex]));
    }
    json << "]}";
  }
  json << "],";
  json << "\"static\":" << (field.isStatic ? "true" : "false");
  json << "}";
}

void AppendMethodReportJson(std::ostringstream &json,
                            const HeadlessMethodReport &method,
                            const std::string &indent) {
  json << indent << "{";
  json << "\"name\":" << er2::HeadlessJsonString(method.name) << ",";
  json << "\"methodInfo\":"
       << er2::HeadlessJsonString(er2::HexPtr(method.methodInfo)) << ",";
  json << "\"methodPointer\":"
       << er2::HeadlessJsonString(er2::HexPtr(method.methodPointer)) << ",";
  json << "\"methodRva\":"
       << er2::HeadlessJsonString(er2::HexPtr(method.methodRva)) << ",";
  json << "\"invokerMethod\":"
       << er2::HeadlessJsonString(er2::HexPtr(method.invokerMethod)) << ",";
  json << "\"returnTypeName\":"
       << er2::HeadlessJsonString(method.returnTypeName) << ",";
  json << "\"returnType\":"
       << er2::HeadlessJsonString(er2::HexPtr(method.returnType)) << ",";
  json << "\"parameters\":"
       << er2::HeadlessJsonString(er2::HexPtr(method.parameters)) << ",";
  json << "\"token\":" << er2::HeadlessJsonString(er2::HexPtr(method.token))
       << ",";
  json << "\"flags\":" << er2::HeadlessJsonString(er2::HexPtr(method.flags))
       << ",";
  json << "\"iflags\":"
       << er2::HeadlessJsonString(er2::HexPtr(method.iflags)) << ",";
  json << "\"slot\":";
  if (method.slot == 0xFFFFu) {
    json << "null";
  } else {
    json << method.slot;
  }
  json << ",\"parameterCount\":"
       << static_cast<unsigned int>(method.parameterCount);
  json << "}";
}

void AppendKlassReportJson(std::ostringstream &json,
                           const HeadlessKlassReport &klass,
                           const std::string &indent) {
  json << indent << "{\n";
  json << indent << "  \"klass\":"
       << er2::HeadlessJsonString(er2::HexPtr(klass.klass)) << ",\n";
  json << indent << "  \"sources\":[";
  for (std::size_t i = 0; i < klass.sources.size(); ++i) {
    if (i != 0) {
      json << ",";
    }
    json << er2::HeadlessJsonString(klass.sources[i]);
  }
  json << "],\n";
  json << indent << "  \"instances\":[";
  for (std::size_t i = 0; i < klass.instances.size(); ++i) {
    if (i != 0) {
      json << ",";
    }
    json << er2::HeadlessJsonString(er2::HexPtr(klass.instances[i]));
  }
  json << "],\n";
  json << indent << "  \"runtimeValid\":"
       << (klass.runtimeValid ? "true" : "false") << ",\n";
  json << indent << "  \"fullName\":"
       << er2::HeadlessJsonString(klass.runtime.fullName) << ",\n";
  json << indent << "  \"className\":"
       << er2::HeadlessJsonString(klass.runtime.className) << ",\n";
  json << indent << "  \"namespace\":"
       << er2::HeadlessJsonString(klass.runtime.namespaze) << ",\n";
  json << indent << "  \"runtimeFailure\":"
       << er2::HeadlessJsonString(klass.runtime.failureReason) << ",\n";
  json << indent << "  \"headerBestName\":"
       << er2::HeadlessJsonString(klass.header.bestName) << ",\n";
  json << indent << "  \"headerBestNamespace\":"
       << er2::HeadlessJsonString(klass.header.bestNamespace) << ",\n";
  json << indent << "  \"headerFailure\":"
       << er2::HeadlessJsonString(klass.header.failureReason) << ",\n";
  json << indent << "  \"parent\":"
       << er2::HeadlessJsonString(er2::HexPtr(klass.parent)) << ",\n";
  json << indent << "  \"fieldsPtr\":"
       << er2::HeadlessJsonString(er2::HexPtr(klass.fields)) << ",\n";
  json << indent << "  \"methodsPtr\":"
       << er2::HeadlessJsonString(er2::HexPtr(klass.methods)) << ",\n";
  json << indent << "  \"staticFields\":"
       << er2::HeadlessJsonString(er2::HexPtr(klass.staticFields)) << ",\n";
  json << indent << "  \"fieldCount\":" << klass.fieldCount << ",\n";
  json << indent << "  \"methodCount\":" << klass.methodCount << ",\n";
  const RuntimeDumpExportDecision dumpDecision =
      ExplainRuntimeDumpExportDecision(klass);
  json << indent << "  \"trustedFieldPrefix\":"
       << dumpDecision.trustedFieldPrefix << ",\n";
  json << indent << "  \"dumpExportable\":"
       << (dumpDecision.exportable ? "true" : "false") << ",\n";
  json << indent << "  \"dumpExportReason\":"
       << er2::HeadlessJsonString(dumpDecision.reason) << ",\n";
  json << indent << "  \"fieldsSample\":[\n";
  for (std::size_t i = 0; i < klass.fieldsSample.size(); ++i) {
    AppendFieldReportJson(json, klass.fieldsSample[i], indent + "    ");
    if (i + 1 < klass.fieldsSample.size()) {
      json << ",";
    }
    json << "\n";
  }
  json << indent << "  ],\n";
  json << indent << "  \"methodsSample\":[\n";
  for (std::size_t i = 0; i < klass.methodsSample.size(); ++i) {
    AppendMethodReportJson(json, klass.methodsSample[i], indent + "    ");
    if (i + 1 < klass.methodsSample.size()) {
      json << ",";
    }
    json << "\n";
  }
  json << indent << "  ]\n";
  json << indent << "}";
}

void AppendManagerCandidateJson(std::ostringstream &json,
                                const er2::ManagerRvaCandidate &candidate,
                                const std::string &indent) {
  json << indent << "{\n";
  json << indent << "  \"name\":"
       << er2::HeadlessJsonString(candidate.name) << ",\n";
  json << indent << "  \"section\":"
       << er2::HeadlessJsonString(candidate.section) << ",\n";
  json << indent << "  \"rva\":"
       << er2::HeadlessJsonString(er2::HexPtr(candidate.rva)) << ",\n";
  json << indent << "  \"slotVa\":"
       << er2::HeadlessJsonString(er2::HexPtr(candidate.slotVa)) << ",\n";
  json << indent << "  \"slotPointer\":"
       << er2::HeadlessJsonString(er2::HexPtr(candidate.slotPointer))
       << ",\n";
  json << indent << "  \"staticFields\":"
       << er2::HeadlessJsonString(er2::HexPtr(candidate.staticFields))
       << ",\n";
  json << indent << "  \"resolvedPtr\":"
       << er2::HeadlessJsonString(er2::HexPtr(candidate.resolvedPtr))
       << ",\n";
  json << indent << "  \"auxPtr\":"
       << er2::HeadlessJsonString(er2::HexPtr(candidate.auxPtr)) << ",\n";
  json << indent << "  \"count\":" << candidate.count << ",\n";
  json << indent << "  \"value\":" << candidate.value << ",\n";
  json << indent << "  \"observedDelta\":" << candidate.observedDelta
       << ",\n";
  json << indent << "  \"score\":" << candidate.score << ",\n";
  json << indent << "  \"confirmed\":"
       << (candidate.confirmed ? "true" : "false") << ",\n";
  json << indent << "  \"chain\":"
       << er2::HeadlessJsonString(candidate.chain) << ",\n";
  json << indent << "  \"evidence\":"
       << er2::HeadlessJsonString(candidate.evidence) << "\n";
  json << indent << "}";
}

void AppendNarakaBuffContainerJson(
    std::ostringstream &json,
    const er2::NarakaBuffContainerCandidate &container,
    const std::string &indent) {
  json << indent << "{\n";
  json << indent << "  \"ownerKind\":"
       << er2::HeadlessJsonString(
              er2::NarakaBuffContainerOwnerKindName(container.ownerKind))
       << ",\n";
  json << indent << "  \"owner\":"
       << er2::HeadlessJsonString(er2::HexPtr(container.owner)) << ",\n";
  json << indent << "  \"ownerOffset\":"
       << er2::HeadlessJsonString(er2::HexPtr(container.ownerOffset))
       << ",\n";
  json << indent << "  \"kind\":"
       << er2::HeadlessJsonString(
              er2::NarakaBuffContainerKindName(container.kind))
       << ",\n";
  json << indent << "  \"container\":"
       << er2::HeadlessJsonString(er2::HexPtr(container.container)) << ",\n";
  json << indent << "  \"backingArray\":"
       << er2::HeadlessJsonString(er2::HexPtr(container.backingArray))
       << ",\n";
  json << indent << "  \"count\":" << container.count << ",\n";
  json << indent << "  \"evidence\":"
       << er2::HeadlessJsonString(container.evidence) << "\n";
  json << indent << "}";
}

void AppendNarakaBuffEntryJson(std::ostringstream &json,
                               const er2::NarakaBuffEntrySample &entry,
                               const std::string &indent) {
  json << indent << "{\n";
  json << indent << "  \"containerKind\":"
       << er2::HeadlessJsonString(
              er2::NarakaBuffContainerKindName(entry.containerKind))
       << ",\n";
  json << indent << "  \"container\":"
       << er2::HeadlessJsonString(er2::HexPtr(entry.container)) << ",\n";
  json << indent << "  \"entryAddress\":"
       << er2::HeadlessJsonString(er2::HexPtr(entry.entryAddress)) << ",\n";
  json << indent << "  \"key\":" << entry.key << ",\n";
  json << indent << "  \"valuePtr\":"
       << er2::HeadlessJsonString(er2::HexPtr(entry.valuePtr)) << ",\n";
  json << indent << "  \"valueKlass\":"
       << er2::HeadlessJsonString(er2::HexPtr(entry.valueKlass)) << ",\n";
  json << indent << "  \"int0\":" << entry.int0 << ",\n";
  json << indent << "  \"int1\":" << entry.int1 << ",\n";
  json << indent << "  \"int2\":" << entry.int2 << ",\n";
  json << indent << "  \"int3\":" << entry.int3 << ",\n";
  json << indent << "  \"uint0\":" << entry.uint0 << ",\n";
  json << indent << "  \"float0\":" << entry.float0 << "\n";
  json << indent << "}";
}

void AppendNarakaLocalBuffEntryJson(
    std::ostringstream &json, const er2::NarakaLocalBuffEntry &entry,
    const std::string &indent) {
  json << indent << "{\n";
  json << indent << "  \"entryAddress\":"
       << er2::HeadlessJsonString(er2::HexPtr(entry.entryAddress)) << ",\n";
  json << indent << "  \"key\":" << entry.key << ",\n";
  json << indent << "  \"buff\":"
       << er2::HeadlessJsonString(er2::HexPtr(entry.buff)) << ",\n";
  json << indent << "  \"klass\":"
       << er2::HeadlessJsonString(er2::HexPtr(entry.klass)) << ",\n";
  json << indent << "  \"buffId\":" << entry.buffId << ",\n";
  json << indent << "  \"groupId\":" << entry.groupId << ",\n";
  json << indent << "  \"instanceId\":" << entry.instanceId << ",\n";
  json << indent << "  \"ownerFid\":" << entry.ownerFid << ",\n";
  json << indent << "  \"casterFid\":" << entry.casterFid << ",\n";
  json << indent << "  \"leftTime\":" << entry.leftTime << "\n";
  json << indent << "}";
}

void AppendNarakaLocalBuffSnapshotJson(
    std::ostringstream &json, const er2::NarakaLocalBuffSnapshot &local,
    const std::string &indent) {
  json << indent << "{\n";
  json << indent << "  \"error\":"
       << er2::HeadlessJsonString(local.error) << ",\n";
  json << indent << "  \"targetBuffId\":" << local.targetBuffId << ",\n";
  json << indent << "  \"hasTargetBuff\":"
       << (local.hasTargetBuff ? "true" : "false") << ",\n";
  json << indent << "  \"characterManager\":"
       << er2::HeadlessJsonString(er2::HexPtr(local.characterManager))
       << ",\n";
  json << indent << "  \"actorModel\":"
       << er2::HeadlessJsonString(er2::HexPtr(local.actorModel)) << ",\n";
  json << indent << "  \"propertyData\":"
       << er2::HeadlessJsonString(er2::HexPtr(local.propertyData)) << ",\n";
  json << indent << "  \"fid\":" << local.fid << ",\n";
  json << indent << "  \"heroId\":" << local.heroId << ",\n";
  json << indent << "  \"buffHandlersDict\":"
       << er2::HeadlessJsonString(er2::HexPtr(local.buffHandlersDict))
       << ",\n";
  json << indent << "  \"buffHandlersEntries\":"
       << er2::HeadlessJsonString(er2::HexPtr(local.buffHandlersEntries))
       << ",\n";
  json << indent << "  \"buffHandlersCount\":" << local.buffHandlersCount
       << ",\n";
  json << indent << "  \"buffHandlersArrayLength\":"
       << local.buffHandlersArrayLength << ",\n";
  json << indent << "  \"selectedHandlerKey\":"
       << local.selectedHandlerKey << ",\n";
  json << indent << "  \"buffHandler\":"
       << er2::HeadlessJsonString(er2::HexPtr(local.buffHandler)) << ",\n";
  json << indent << "  \"buffTidList\":"
       << er2::HeadlessJsonString(er2::HexPtr(local.buffTidList)) << ",\n";
  json << indent << "  \"buffTidItems\":"
       << er2::HeadlessJsonString(er2::HexPtr(local.buffTidItems)) << ",\n";
  json << indent << "  \"buffTidCount\":" << local.buffTidCount << ",\n";
  json << indent << "  \"buffTidValues\":[";
  for (std::size_t i = 0; i < local.buffTidValues.size(); ++i) {
    if (i != 0) {
      json << ",";
    }
    json << local.buffTidValues[i];
  }
  json << "],\n";
  json << indent << "  \"buffInstanceIdList\":"
       << er2::HeadlessJsonString(er2::HexPtr(local.buffInstanceIdList))
       << ",\n";
  json << indent << "  \"buffDict\":"
       << er2::HeadlessJsonString(er2::HexPtr(local.buffDict)) << ",\n";
  json << indent << "  \"buffDictEntries\":"
       << er2::HeadlessJsonString(er2::HexPtr(local.buffDictEntries))
       << ",\n";
  json << indent << "  \"buffDictCount\":" << local.buffDictCount << ",\n";
  json << indent << "  \"buffDictArrayLength\":" << local.buffDictArrayLength
       << ",\n";
  json << indent << "  \"buffEntries\":[\n";
  for (std::size_t i = 0; i < local.buffEntries.size(); ++i) {
    AppendNarakaLocalBuffEntryJson(json, local.buffEntries[i], indent + "    ");
    if (i + 1 < local.buffEntries.size()) {
      json << ",";
    }
    json << "\n";
  }
  json << indent << "  ]\n";
  json << indent << "}";
}

void AppendNarakaActorContainerJson(
    std::ostringstream &json,
    const er2::NarakaActorContainerCandidate &container,
    const std::string &indent) {
  json << indent << "{\n";
  json << indent << "  \"ownerKind\":"
       << er2::HeadlessJsonString(
              er2::NarakaActorContainerOwnerKindName(container.ownerKind))
       << ",\n";
  json << indent << "  \"owner\":"
       << er2::HeadlessJsonString(er2::HexPtr(container.owner)) << ",\n";
  json << indent << "  \"ownerOffset\":"
       << er2::HeadlessJsonString(er2::HexPtr(container.ownerOffset))
       << ",\n";
  json << indent << "  \"kind\":"
       << er2::HeadlessJsonString(
              er2::NarakaActorContainerKindName(container.kind))
       << ",\n";
  json << indent << "  \"container\":"
       << er2::HeadlessJsonString(er2::HexPtr(container.container)) << ",\n";
  json << indent << "  \"backingArray\":"
       << er2::HeadlessJsonString(er2::HexPtr(container.backingArray))
       << ",\n";
  json << indent << "  \"count\":" << container.count << ",\n";
  json << indent << "  \"evidence\":"
       << er2::HeadlessJsonString(container.evidence) << "\n";
  json << indent << "}";
}

void AppendNarakaActorContainerEntryJson(
    std::ostringstream &json, const er2::NarakaActorContainerEntry &entry,
    const std::string &indent) {
  json << indent << "{\n";
  json << indent << "  \"ownerKind\":"
       << er2::HeadlessJsonString(
              er2::NarakaActorContainerOwnerKindName(entry.ownerKind))
       << ",\n";
  json << indent << "  \"ownerOffset\":"
       << er2::HeadlessJsonString(er2::HexPtr(entry.ownerOffset)) << ",\n";
  json << indent << "  \"containerKind\":"
       << er2::HeadlessJsonString(
              er2::NarakaActorContainerKindName(entry.containerKind))
       << ",\n";
  json << indent << "  \"container\":"
       << er2::HeadlessJsonString(er2::HexPtr(entry.container)) << ",\n";
  json << indent << "  \"entryAddress\":"
       << er2::HeadlessJsonString(er2::HexPtr(entry.entryAddress)) << ",\n";
  json << indent << "  \"key\":" << entry.key << ",\n";
  json << indent << "  \"hash\":" << entry.hash << ",\n";
  json << indent << "  \"next\":" << entry.next << ",\n";
  json << indent << "  \"rawValue\":"
       << er2::HeadlessJsonString(er2::HexPtr(entry.rawValue)) << ",\n";
  json << indent << "  \"valuePtr\":"
       << er2::HeadlessJsonString(er2::HexPtr(entry.valuePtr)) << ",\n";
  json << indent << "  \"valueKlass\":"
       << er2::HeadlessJsonString(er2::HexPtr(entry.valueKlass)) << ",\n";
  json << indent << "  \"int0\":" << entry.int0 << ",\n";
  json << indent << "  \"int1\":" << entry.int1 << ",\n";
  json << indent << "  \"int2\":" << entry.int2 << ",\n";
  json << indent << "  \"int3\":" << entry.int3 << ",\n";
  json << indent << "  \"uint0\":" << entry.uint0 << ",\n";
  json << indent << "  \"float0\":" << entry.float0 << "\n";
  json << indent << "}";
}

void AppendNarakaRuntimePropertyDiffJson(
    std::ostringstream &json, const er2::NarakaRuntimePropertyDiffRow &row,
    const std::string &indent) {
  json << indent << "{\n";
  json << indent << "  \"dataId\":" << row.dataId << ",\n";
  json << indent << "  \"index\":" << row.index << ",\n";
  json << indent << "  \"valueAddress\":"
       << er2::HeadlessJsonString(er2::HexPtr(row.valueAddress)) << ",\n";
  json << indent << "  \"hadBaseline\":"
       << (row.hadBaseline ? "true" : "false") << ",\n";
  json << indent << "  \"changed\":" << (row.changed ? "true" : "false")
       << ",\n";
  json << indent << "  \"baselineInt\":" << row.baselineInt << ",\n";
  json << indent << "  \"currentInt\":" << row.currentInt << ",\n";
  json << indent << "  \"intDelta\":" << row.intDelta << ",\n";
  json << indent << "  \"baselineFloat\":" << row.baselineFloat << ",\n";
  json << indent << "  \"currentFloat\":" << row.currentFloat << ",\n";
  json << indent << "  \"floatDelta\":" << row.floatDelta << "\n";
  json << indent << "}";
}

void AppendNarakaGroundProbeJson(
    std::ostringstream &json, const HeadlessNarakaGroundProbeReport &ground,
    const std::string &indent) {
  json << indent << "{\n";
  json << indent << "  \"requested\":"
       << (ground.requested ? "true" : "false") << ",\n";
  json << indent << "  \"ok\":" << (ground.ok ? "true" : "false")
       << ",\n";
  json << indent << "  \"error\":"
       << er2::HeadlessJsonString(ground.error) << ",\n";
  json << indent << "  \"characterManager\":"
       << er2::HeadlessJsonString(er2::HexPtr(ground.characterManager))
       << ",\n";
  json << indent << "  \"actorModel\":"
       << er2::HeadlessJsonString(er2::HexPtr(ground.actorModel)) << ",\n";
  json << indent << "  \"actorKit\":"
       << er2::HeadlessJsonString(er2::HexPtr(ground.actorKit)) << ",\n";
  json << indent << "  \"actorPhysics\":"
       << er2::HeadlessJsonString(er2::HexPtr(ground.actorPhysics))
       << ",\n";
  json << indent << "  \"advanceGroundStatus\":"
       << er2::HeadlessJsonString(er2::HexPtr(ground.advanceGroundStatus))
       << ",\n";
  json << indent << "  \"alignToGroundData\":"
       << er2::HeadlessJsonString(er2::HexPtr(ground.alignToGroundData))
       << ",\n";
  json << indent << "  \"currentCollisionFlags\":"
       << ground.currentCollisionFlags << ",\n";
  json << indent << "  \"collisionFlagsBelowMask\":4,\n";
  json << indent << "  \"isGroundedByCollisionFlags\":"
       << (ground.isGroundedByCollisionFlags ? "true" : "false") << ",\n";
  json << indent << "  \"advanceNearGround\":"
       << (ground.advanceNearGround ? "true" : "false") << ",\n";
  json << indent << "  \"advanceGroundDistance\":"
       << ground.advanceGroundDistance << ",\n";
  json << indent << "  \"alignNearGround\":"
       << (ground.alignNearGround ? "true" : "false") << ",\n";
  json << indent << "  \"alignGroundDistance\":"
       << ground.alignGroundDistance << ",\n";
  json << indent << "  \"alignRootGroundDistance\":"
       << ground.alignRootGroundDistance << "\n";
  json << indent << "}";
}

void AppendMetadataHeaderFieldsJson(std::ostringstream &json,
                                    const er2::MetadataHeaderFields &header,
                                    const std::string &indent) {
  json << indent << "{\n";
  json << indent << "  \"version\":" << header.version << ",\n";
  json << indent << "  \"stringOffset\":"
       << er2::HeadlessJsonString(er2::HexPtr(header.stringOffset)) << ",\n";
  json << indent << "  \"stringBytes\":" << header.stringSize << ",\n";
  json << indent << "  \"methodsOffset\":"
       << er2::HeadlessJsonString(er2::HexPtr(header.methodsOffset)) << ",\n";
  json << indent << "  \"methodsBytes\":" << header.methodsSize << ",\n";
  json << indent << "  \"fieldsOffset\":"
       << er2::HeadlessJsonString(er2::HexPtr(header.fieldsOffset)) << ",\n";
  json << indent << "  \"fieldsBytes\":" << header.fieldsSize << ",\n";
  json << indent << "  \"typeDefinitionsOffset\":"
       << er2::HeadlessJsonString(er2::HexPtr(header.typeDefinitionsOffset))
       << ",\n";
  json << indent << "  \"typeDefinitionsBytes\":"
       << header.typeDefinitionsSize << ",\n";
  json << indent << "  \"imagesOffset\":"
       << er2::HeadlessJsonString(er2::HexPtr(header.imagesOffset)) << ",\n";
  json << indent << "  \"imagesBytes\":" << header.imagesSize << "\n";
  json << indent << "}";
}

void AppendMetadataProbeJson(std::ostringstream &json,
                             const HeadlessMetadataProbeReport &probe,
                             const std::string &indent) {
  json << indent << "{\n";
  json << indent << "  \"address\":"
       << er2::HeadlessJsonString(er2::HexPtr(probe.address)) << ",\n";
  json << indent << "  \"readOk\":" << (probe.readOk ? "true" : "false")
       << ",\n";
  json << indent << "  \"headerOk\":"
       << (probe.headerOk ? "true" : "false") << ",\n";
  json << indent << "  \"magic\":"
       << er2::HeadlessJsonString(er2::HexPtr(probe.magic)) << ",\n";
  json << indent << "  \"version\":" << probe.version << ",\n";
  json << indent << "  \"score\":" << probe.score << ",\n";
  json << indent << "  \"maxEnd\":" << probe.maxEnd << ",\n";
  json << indent << "  \"header\":";
  AppendMetadataHeaderFieldsJson(json, probe.header, indent + "  ");
  json << "\n" << indent << "}";
}

void AppendMetadataRawCandidateJson(
    std::ostringstream &json, const HeadlessMetadataRawCandidate &candidate,
    const std::string &indent) {
  json << indent << "{\n";
  json << indent << "  \"address\":"
       << er2::HeadlessJsonString(er2::HexPtr(candidate.address)) << ",\n";
  json << indent << "  \"regionText\":"
       << er2::HeadlessJsonString(candidate.regionText) << ",\n";
  json << indent << "  \"privateMemory\":"
       << (candidate.privateMemory ? "true" : "false") << ",\n";
  json << indent << "  \"image\":" << (candidate.image ? "true" : "false")
       << ",\n";
  json << indent << "  \"protection\":" << candidate.protection << ",\n";
  json << indent << "  \"version\":" << candidate.version << ",\n";
  json << indent << "  \"score\":" << candidate.score << ",\n";
  json << indent << "  \"maxEnd\":" << candidate.maxEnd << ",\n";
  json << indent << "  \"header\":";
  AppendMetadataHeaderFieldsJson(json, candidate.header, indent + "  ");
  json << "\n" << indent << "}";
}

std::string BuildRunJson(const HeadlessRunResult &run) {
  std::ostringstream json;
  json << "{\n";
  json << "  \"ok\":" << (run.ok ? "true" : "false") << ",\n";
  json << "  \"error\":" << er2::HeadlessJsonString(run.error) << ",\n";
  json << "  \"targetProcess\":"
       << er2::HeadlessJsonString(run.options.targetProcess) << ",\n";
  json << "  \"runMode\":" << er2::HeadlessJsonString(run.options.runMode)
       << ",\n";
  json << "  \"pid\":" << er2::g_ctx.pid << ",\n";
  json << "  \"runtime\":"
       << er2::HeadlessJsonString(er2::g_ctx.runtime ==
                                           er2::ManagedBackend::Il2Cpp
                                       ? "IL2CPP"
                                       : "Mono")
       << ",\n";
  json << "  \"scanMode\":"
       << er2::HeadlessJsonString(
              er2::HeadlessScanModeName(run.options.scanMode))
       << ",\n";
  json << "  \"filter\":" << er2::HeadlessJsonString(run.options.filter)
       << ",\n";
  json << "  \"limit\":" << run.options.limit << ",\n";
  json << "  \"modules\":{\n";
  json << "    \"count\":" << run.modules.moduleCount << ",\n";
  json << "    \"unityPlayer\":{\n";
  json << "      \"name\":"
       << er2::HeadlessJsonString(run.modules.unityPlayerName)
       << ",\n";
  json << "      \"base\":"
       << er2::HeadlessJsonString(er2::HexPtr(er2::g_ctx.unityPlayer.base))
       << ",\n";
  json << "      \"size\":" << er2::g_ctx.unityPlayer.size << ",\n";
  json << "      \"dosHeaderOk\":"
       << (run.modules.unityDosOk ? "true" : "false") << "\n";
  json << "    },\n";
  json << "    \"gameAssembly\":{\n";
  json << "      \"name\":"
       << er2::HeadlessJsonString(run.modules.gameAssemblyName)
       << ",\n";
  json << "      \"base\":"
       << er2::HeadlessJsonString(er2::HexPtr(er2::g_ctx.gameAssembly.base))
       << ",\n";
  json << "      \"size\":" << er2::g_ctx.gameAssembly.size << ",\n";
  json << "      \"dosHeaderOk\":"
       << (run.modules.gameAssemblyDosOk ? "true" : "false") << "\n";
  json << "    }\n";
  json << "  },\n";
  json << "  \"layoutProfile\":{\n";
  const er2::Il2CppLayoutProfile *profile =
      er2::FindIl2CppLayoutProfileForUnity2019_4("2019.4.41");
  json << "    \"id\":" << er2::HeadlessJsonString(profile ? profile->id : "")
       << ",\n";
  json << "    \"source\":"
       << er2::HeadlessJsonString(profile ? profile->source : "") << "\n";
  json << "  },\n";
  json << "  \"msid\":{\n";
  json << "    \"slot\":"
       << er2::HeadlessJsonString(er2::HexPtr(run.msidSlot))
       << ",\n";
  json << "    \"slotRva\":"
       << er2::HeadlessJsonString(
              run.msidSlot && er2::g_ctx.unityPlayer.base
                  ? er2::HexPtr(run.msidSlot - er2::g_ctx.unityPlayer.base)
                  : "0x0")
       << ",\n";
  json << "    \"scanScore\":" << run.msidScanScore << ",\n";
  json << "    \"count\":" << run.msidCount << "\n";
  json << "  },\n";
  json << "  \"matchedObjectCount\":" << run.matchedObjectCount << ",\n";
  json << "  \"objects\":[\n";
  for (std::size_t i = 0; i < run.objects.size(); ++i) {
    const HeadlessObjectRow &row = run.objects[i];
    json << "    {\n";
    json << "      \"native\":"
         << er2::HeadlessJsonString(er2::HexPtr(row.info.native)) << ",\n";
    json << "      \"managed\":"
         << er2::HeadlessJsonString(er2::HexPtr(row.managed))
         << ",\n";
    json << "      \"klass\":"
         << er2::HeadlessJsonString(er2::HexPtr(row.klass))
         << ",\n";
    json << "      \"instanceId\":" << row.info.instanceId << ",\n";
    json << "      \"name\":" << er2::HeadlessJsonString(row.info.objectName)
         << ",\n";
    json << "      \"kind\":"
         << er2::HeadlessJsonString(ObjectKindName(row.info.kind))
         << ",\n";
    json << "      \"typeFullName\":"
         << er2::HeadlessJsonString(row.info.typeFullName) << ",\n";
    json << "      \"typeNamespace\":"
         << er2::HeadlessJsonString(row.info.typeNamespace) << ",\n";
    json << "      \"typeClassName\":"
         << er2::HeadlessJsonString(row.info.typeClassName) << "\n";
    json << "    }";
    if (i + 1 < run.objects.size()) {
      json << ",";
    }
    json << "\n";
  }
  json << "  ],\n";
  json << "  \"klassReports\":[\n";
  for (std::size_t i = 0; i < run.klassReports.size(); ++i) {
    AppendKlassReportJson(json, run.klassReports[i], "    ");
    if (i + 1 < run.klassReports.size()) {
      json << ",";
    }
    json << "\n";
  }
  json << "  ],\n";
  json << "  \"metadataExport\":{\n";
  json << "    \"requested\":"
       << (run.metadataExport.requested ? "true" : "false") << ",\n";
  json << "    \"ok\":" << (run.metadataExport.ok ? "true" : "false")
       << ",\n";
  json << "    \"path\":"
       << er2::HeadlessJsonString(run.metadataExport.path) << ",\n";
  json << "    \"error\":"
       << er2::HeadlessJsonString(run.metadataExport.error) << ",\n";
  json << "    \"bytes\":" << run.metadataExport.bytes << ",\n";
  json << "    \"magic\":"
       << er2::HeadlessJsonString(er2::HexPtr(run.metadataExport.magic))
       << ",\n";
  json << "    \"version\":" << run.metadataExport.version << ",\n";
  json << "    \"stringBytes\":" << run.metadataExport.header.stringSize
       << ",\n";
  json << "    \"methodsBytes\":" << run.metadataExport.header.methodsSize
       << ",\n";
  json << "    \"fieldsBytes\":" << run.metadataExport.header.fieldsSize
       << ",\n";
  json << "    \"typeDefinitionsBytes\":"
       << run.metadataExport.header.typeDefinitionsSize << ",\n";
  json << "    \"imagesBytes\":" << run.metadataExport.header.imagesSize
       << "\n";
  json << "  },\n";
  json << "  \"metadataDiagnostics\":{\n";
  json << "    \"requested\":"
       << (run.metadataDiagnostics.requested ? "true" : "false") << ",\n";
  json << "    \"ok\":" << (run.metadataDiagnostics.ok ? "true" : "false")
       << ",\n";
  json << "    \"error\":"
       << er2::HeadlessJsonString(run.metadataDiagnostics.error) << ",\n";
  json << "    \"vadCount\":" << run.metadataDiagnostics.vadCount << ",\n";
  json << "    \"regionsScanned\":"
       << run.metadataDiagnostics.regionsScanned << ",\n";
  json << "    \"bytesScanned\":" << run.metadataDiagnostics.bytesScanned
       << ",\n";
  json << "    \"readFailures\":" << run.metadataDiagnostics.readFailures
       << ",\n";
  json << "    \"magicHits\":" << run.metadataDiagnostics.magicHits
       << ",\n";
  json << "    \"heuristicHits\":" << run.metadataDiagnostics.heuristicHits
       << ",\n";
  json << "    \"anchor\":"
       << er2::HeadlessJsonString(er2::HexPtr(run.metadataDiagnostics.anchor))
       << ",\n";
  json << "    \"anchorVadHits\":" << run.metadataDiagnostics.anchorVadHits
       << ",\n";
  json << "    \"probe\":";
  AppendMetadataProbeJson(json, run.metadataDiagnostics.probe, "    ");
  json << ",\n";
  json << "    \"candidates\":[\n";
  for (std::size_t i = 0; i < run.metadataDiagnostics.candidates.size();
       ++i) {
    AppendMetadataRawCandidateJson(json,
                                   run.metadataDiagnostics.candidates[i],
                                   "      ");
    if (i + 1 < run.metadataDiagnostics.candidates.size()) {
      json << ",";
    }
    json << "\n";
  }
  json << "    ]\n";
  json << "  },\n";
  json << "  \"runtimeDump\":{\n";
  json << "    \"requested\":"
       << (run.runtimeDump.requested ? "true" : "false") << ",\n";
  json << "    \"ok\":" << (run.runtimeDump.ok ? "true" : "false")
       << ",\n";
  json << "    \"path\":" << er2::HeadlessJsonString(run.runtimeDump.path)
       << ",\n";
  json << "    \"error\":"
       << er2::HeadlessJsonString(run.runtimeDump.error) << ",\n";
  json << "    \"classCount\":" << run.runtimeDump.classCount << ",\n";
  json << "    \"fieldCount\":" << run.runtimeDump.fieldCount << ",\n";
  json << "    \"methodCount\":" << run.runtimeDump.methodCount << "\n";
  json << "  },\n";
  json << "  \"narakaBuffProbe\":{\n";
  json << "    \"requested\":"
       << (run.narakaBuffProbe.requested ? "true" : "false") << ",\n";
  json << "    \"ok\":" << (run.narakaBuffProbe.ok ? "true" : "false")
       << ",\n";
  json << "    \"error\":"
       << er2::HeadlessJsonString(run.narakaBuffProbe.error) << ",\n";
  json << "    \"buffManagerRva\":"
       << er2::HeadlessJsonString(
              er2::HexPtr(er2::DefaultNarakaManagerRvas().buffManager))
       << ",\n";
  json << "    \"buffManagerSlot\":"
       << er2::HeadlessJsonString(er2::HexPtr(
              er2::g_ctx.gameAssembly.base
                  ? er2::g_ctx.gameAssembly.base +
                        static_cast<std::uintptr_t>(
                            er2::DefaultNarakaManagerRvas().buffManager)
                  : 0))
       << ",\n";
  json << "    \"buffManagerClass\":"
       << er2::HeadlessJsonString(
              er2::HexPtr(run.narakaBuffProbe.snapshot.buffManagerClass))
       << ",\n";
  json << "    \"staticFields\":"
       << er2::HeadlessJsonString(
              er2::HexPtr(run.narakaBuffProbe.snapshot.staticFields))
       << ",\n";
  json << "    \"instance\":"
       << er2::HeadlessJsonString(
              er2::HexPtr(run.narakaBuffProbe.snapshot.instance))
       << ",\n";
  json << "    \"containerCount\":"
       << run.narakaBuffProbe.snapshot.containers.size() << ",\n";
  json << "    \"entryCount\":"
       << run.narakaBuffProbe.snapshot.entries.size() << ",\n";
  json << "    \"containers\":[\n";
  for (std::size_t i = 0;
       i < run.narakaBuffProbe.snapshot.containers.size(); ++i) {
    AppendNarakaBuffContainerJson(
        json, run.narakaBuffProbe.snapshot.containers[i], "      ");
    if (i + 1 < run.narakaBuffProbe.snapshot.containers.size()) {
      json << ",";
    }
    json << "\n";
  }
  json << "    ],\n";
  json << "    \"entries\":[\n";
  for (std::size_t i = 0; i < run.narakaBuffProbe.snapshot.entries.size();
       ++i) {
    AppendNarakaBuffEntryJson(json, run.narakaBuffProbe.snapshot.entries[i],
                              "      ");
    if (i + 1 < run.narakaBuffProbe.snapshot.entries.size()) {
      json << ",";
    }
    json << "\n";
  }
  json << "    ]\n";
  json << "    ,\"localBuffs\":";
  AppendNarakaLocalBuffSnapshotJson(json, run.narakaBuffProbe.snapshot.local,
                                    "    ");
  json << "\n";
  json << "  },\n";
  json << "  \"narakaRuntimePropertyProbe\":{\n";
  json << "    \"requested\":"
       << (run.narakaRuntimePropertyProbe.requested ? "true" : "false")
       << ",\n";
  json << "    \"ok\":"
       << (run.narakaRuntimePropertyProbe.ok ? "true" : "false") << ",\n";
  json << "    \"error\":"
       << er2::HeadlessJsonString(run.narakaRuntimePropertyProbe.error)
       << ",\n";
  json << "    \"actorModel\":"
       << er2::HeadlessJsonString(
              er2::HexPtr(run.narakaRuntimePropertyProbe.actorModel))
       << ",\n";
  json << "    \"csvPath\":"
       << er2::HeadlessJsonString(run.narakaRuntimePropertyProbe.csvPath)
       << ",\n";
  json << "    \"sampleCount\":"
       << run.narakaRuntimePropertyProbe.snapshots.size() << ",\n";
  const er2::NarakaRuntimePropertySnapshot *lastRuntimeProperty = nullptr;
  if (!run.narakaRuntimePropertyProbe.snapshots.empty()) {
    lastRuntimeProperty = &run.narakaRuntimePropertyProbe.snapshots.back();
  }
  json << "    \"actorPropertyData\":"
       << er2::HeadlessJsonString(er2::HexPtr(
              lastRuntimeProperty ? lastRuntimeProperty->propertyData : 0))
       << ",\n";
  json << "    \"runtimePropertyData\":"
       << er2::HeadlessJsonString(er2::HexPtr(
              lastRuntimeProperty ? lastRuntimeProperty->runtimePropertyData
                                  : 0))
       << ",\n";
  json << "    \"propertyRoot\":"
       << er2::HeadlessJsonString(er2::HexPtr(
              lastRuntimeProperty ? lastRuntimeProperty->propertyRoot : 0))
       << ",\n";
  json << "    \"idEntries\":"
       << er2::HeadlessJsonString(er2::HexPtr(
              lastRuntimeProperty ? lastRuntimeProperty->idEntries : 0))
       << ",\n";
  json << "    \"valuesArray\":"
       << er2::HeadlessJsonString(er2::HexPtr(
              lastRuntimeProperty ? lastRuntimeProperty->valuesArray : 0))
       << ",\n";
  json << "    \"heroId\":"
       << (lastRuntimeProperty ? lastRuntimeProperty->heroId : 0) << ",\n";
  json << "    \"teamId\":"
       << (lastRuntimeProperty ? lastRuntimeProperty->teamId : 0) << ",\n";
  json << "    \"idCount\":"
       << (lastRuntimeProperty ? lastRuntimeProperty->idCount : 0) << ",\n";
  json << "    \"rowCount\":"
       << (lastRuntimeProperty ? lastRuntimeProperty->rows.size() : 0)
       << ",\n";
  const std::size_t runtimePropertyChangedCount =
      static_cast<std::size_t>(std::count_if(
          run.narakaRuntimePropertyProbe.diff.begin(),
          run.narakaRuntimePropertyProbe.diff.end(),
          [](const er2::NarakaRuntimePropertyDiffRow &row) {
            return row.changed;
          }));
  json << "    \"diffCount\":"
       << run.narakaRuntimePropertyProbe.diff.size() << ",\n";
  json << "    \"changedCount\":" << runtimePropertyChangedCount << ",\n";
  json << "    \"changed\":[\n";
  bool wroteRuntimePropertyDiff = false;
  for (const auto &row : run.narakaRuntimePropertyProbe.diff) {
    if (!row.changed) {
      continue;
    }
    if (wroteRuntimePropertyDiff) {
      json << ",\n";
    }
    AppendNarakaRuntimePropertyDiffJson(json, row, "      ");
    wroteRuntimePropertyDiff = true;
  }
  if (wroteRuntimePropertyDiff) {
    json << "\n";
  }
  json << "    ]\n";
  json << "  },\n";
  json << "  \"narakaGroundProbe\":";
  AppendNarakaGroundProbeJson(json, run.narakaGroundProbe, "  ");
  json << ",\n";
  json << "  \"narakaActorContainerProbe\":{\n";
  json << "    \"requested\":"
       << (run.narakaActorContainerProbe.requested ? "true" : "false")
       << ",\n";
  json << "    \"ok\":"
       << (run.narakaActorContainerProbe.ok ? "true" : "false") << ",\n";
  json << "    \"error\":"
       << er2::HeadlessJsonString(run.narakaActorContainerProbe.error)
       << ",\n";
  json << "    \"actorModel\":"
       << er2::HeadlessJsonString(er2::HexPtr(
              run.narakaActorContainerProbe.snapshot.actorModel))
       << ",\n";
  json << "    \"propertyData\":"
       << er2::HeadlessJsonString(er2::HexPtr(
              run.narakaActorContainerProbe.snapshot.propertyData))
       << ",\n";
  json << "    \"runtimePropertyData\":"
       << er2::HeadlessJsonString(er2::HexPtr(
              run.narakaActorContainerProbe.snapshot.runtimePropertyData))
       << ",\n";
  json << "    \"heroId\":"
       << run.narakaActorContainerProbe.snapshot.heroId << ",\n";
  json << "    \"teamId\":"
       << run.narakaActorContainerProbe.snapshot.teamId << ",\n";
  json << "    \"containerCount\":"
       << run.narakaActorContainerProbe.snapshot.containers.size() << ",\n";
  json << "    \"entryCount\":"
       << run.narakaActorContainerProbe.snapshot.entries.size() << ",\n";
  json << "    \"containers\":[\n";
  for (std::size_t i = 0;
       i < run.narakaActorContainerProbe.snapshot.containers.size(); ++i) {
    AppendNarakaActorContainerJson(
        json, run.narakaActorContainerProbe.snapshot.containers[i], "      ");
    if (i + 1 < run.narakaActorContainerProbe.snapshot.containers.size()) {
      json << ",";
    }
    json << "\n";
  }
  json << "    ],\n";
  json << "    \"entries\":[\n";
  for (std::size_t i = 0;
       i < run.narakaActorContainerProbe.snapshot.entries.size(); ++i) {
    AppendNarakaActorContainerEntryJson(
        json, run.narakaActorContainerProbe.snapshot.entries[i], "      ");
    if (i + 1 < run.narakaActorContainerProbe.snapshot.entries.size()) {
      json << ",";
    }
    json << "\n";
  }
  json << "    ]\n";
  json << "  },\n";
  json << "  \"managerScan\":{\n";
  json << "    \"enabled\":" << (run.options.scanManagers ? "true" : "false")
       << ",\n";
  json << "    \"ok\":" << (run.managerScan.ok ? "true" : "false") << ",\n";
  json << "    \"error\":"
       << er2::HeadlessJsonString(run.managerScan.error) << ",\n";
  json << "    \"imageSize\":" << run.managerScan.imageSize << ",\n";
  json << "    \"sectionsVisited\":" << run.managerScan.sectionsVisited
       << ",\n";
  json << "    \"bytesScanned\":" << run.managerScan.bytesScanned << ",\n";
  json << "    \"pointerSlots\":" << run.managerScan.pointerSlots << ",\n";
  json << "    \"candidates\":[\n";
  for (std::size_t i = 0; i < run.managerScan.candidates.size(); ++i) {
    AppendManagerCandidateJson(json, run.managerScan.candidates[i], "      ");
    if (i + 1 < run.managerScan.candidates.size()) {
      json << ",";
    }
    json << "\n";
  }
  json << "    ]\n";
  json << "  },\n";
  json << "  \"moduleImages\":{\n";
  json << "    \"requested\":"
       << (run.moduleImages.requested ? "true" : "false") << ",\n";
  json << "    \"ok\":" << (run.moduleImages.ok ? "true" : "false") << ",\n";
  json << "    \"error\":"
       << er2::HeadlessJsonString(run.moduleImages.error) << ",\n";
  json << "    \"gameAssemblyName\":"
       << er2::HeadlessJsonString(run.moduleImages.gameAssemblyName) << ",\n";
  json << "    \"gameAssemblyPath\":"
       << er2::HeadlessJsonString(run.moduleImages.gameAssemblyPath) << ",\n";
  json << "    \"gameAssemblyBytes\":" << run.moduleImages.gameAssemblyBytes
       << ",\n";
  json << "    \"unityPlayerPath\":"
       << er2::HeadlessJsonString(run.moduleImages.unityPlayerPath) << ",\n";
  json << "    \"unityPlayerBytes\":" << run.moduleImages.unityPlayerBytes
       << ",\n";
  json << "    \"moduleCount\":" << run.moduleImages.moduleCount << "\n";
  json << "  },\n";
  json << "  \"selfDump\":{\n";
  json << "    \"requested\":"
       << (run.selfDump.requested ? "true" : "false") << ",\n";
  json << "    \"ok\":" << (run.selfDump.ok ? "true" : "false") << ",\n";
  json << "    \"error\":" << er2::HeadlessJsonString(run.selfDump.error)
       << ",\n";
  json << "    \"path\":"
       << er2::HeadlessJsonString(run.selfDump.path) << ",\n";
  json << "    \"typeInfoTable\":"
       << er2::HeadlessJsonString(er2::HexPtr(run.selfDump.typeInfoTable))
       << ",\n";
  json << "    \"typeInfoCount\":" << run.selfDump.typeInfoCount << ",\n";
  json << "    \"klassResolved\":" << run.selfDump.klassResolved << ",\n";
  json << "    \"fieldRows\":" << run.selfDump.fieldRows << ",\n";
  json << "    \"methodRows\":" << run.selfDump.methodRows << ",\n";
  json << "    \"methodRva\":" << run.selfDump.methodRva << "\n";
  json << "  },\n";
  json << "  \"dmaInject\":{\n";
  json << "    \"requested\":"
       << (run.dmaInject.requested ? "true" : "false") << ",\n";
  json << "    \"ok\":" << (run.dmaInject.ok ? "true" : "false") << ",\n";
  json << "    \"error\":" << er2::HeadlessJsonString(run.dmaInject.error)
       << ",\n";
  json << "    \"dllPath\":"
       << er2::HeadlessJsonString(run.dmaInject.dllPath) << ",\n";
  json << "    \"moduleName\":"
       << er2::HeadlessJsonString(run.dmaInject.moduleName) << ",\n";
  json << "    \"hookPoint\":"
       << er2::HeadlessJsonString(run.dmaInject.hookPoint) << ",\n";
  json << "    \"iatThunk\":"
       << er2::HeadlessJsonString(er2::HexPtr(run.dmaInject.iatThunk)) << ",\n";
  json << "    \"iatFunction\":"
       << er2::HeadlessJsonString(er2::HexPtr(run.dmaInject.iatFunction))
       << ",\n";
  json << "    \"codeCave\":"
       << er2::HeadlessJsonString(er2::HexPtr(run.dmaInject.codeCave)) << ",\n";
  json << "    \"dataCave\":"
       << er2::HeadlessJsonString(er2::HexPtr(run.dmaInject.dataCave)) << ",\n";
  json << "    \"loadLibraryAddr\":"
       << er2::HeadlessJsonString(er2::HexPtr(run.dmaInject.loadLibraryAddr))
       << ",\n";
  json << "    \"loadLibraryResult\":"
       << er2::HeadlessJsonString(er2::HexPtr(run.dmaInject.loadLibraryResult))
       << ",\n";
  json << "    \"executed\":"
       << (run.dmaInject.executed ? "true" : "false") << ",\n";
  json << "    \"restored\":"
       << (run.dmaInject.restored ? "true" : "false") << "\n";
  json << "  },\n";
  json << "  \"logs\":[\n";
  for (std::size_t i = 0; i < run.logs.size(); ++i) {
    json << "    " << er2::HeadlessJsonString(run.logs[i]);
    if (i + 1 < run.logs.size()) {
      json << ",";
    }
    json << "\n";
  }
  json << "  ]\n";
  json << "}\n";
  return json.str();
}

std::string JoinPath(const std::string &dir, const std::string &file) {
  if (dir.empty())
    return file;
  if (file.empty())
    return dir;
  const char last = dir.back();
  return (last == '\\' || last == '/') ? dir + file : dir + "\\" + file;
}

bool WriteTextFile(const std::string &path, const std::string &content) {
  std::ofstream file(path, std::ios::binary);
  if (!file) {
    return false;
  }
  file.write(content.data(), static_cast<std::streamsize>(content.size()));
  return static_cast<bool>(file);
}

bool WriteBinaryFile(const std::string &path,
                     const std::vector<std::uint8_t> &content) {
  std::ofstream file(path, std::ios::binary);
  if (!file) {
    return false;
  }

  if (!content.empty()) {
    file.write(reinterpret_cast<const char *>(content.data()),
               static_cast<std::streamsize>(content.size()));
  }
  return static_cast<bool>(file);
}

std::string FormatBoolLiteral(bool value) { return value ? "true" : "false"; }

std::string BuildNarakaRuntimePropertyCsv(
    const std::vector<er2::NarakaRuntimePropertySnapshot> &snapshots) {
  std::ostringstream csv;
  csv << "sample,actor_model,hero_id,team_id,property_data,"
         "runtime_property_data,property_root,id_entries,values_array,"
         "id_count,data_id,index,value_address,int_value,uint_value,"
         "float_value,value_read_ok\n";
  for (std::size_t sample = 0; sample < snapshots.size(); ++sample) {
    const auto &snapshot = snapshots[sample];
    for (const auto &row : snapshot.rows) {
      csv << sample << ",0x" << std::hex
          << static_cast<unsigned long long>(snapshot.actorModel) << std::dec
          << ',' << snapshot.heroId << ',' << snapshot.teamId << ",0x"
          << std::hex << static_cast<unsigned long long>(snapshot.propertyData)
          << ",0x"
          << static_cast<unsigned long long>(snapshot.runtimePropertyData)
          << ",0x" << static_cast<unsigned long long>(snapshot.propertyRoot)
          << ",0x" << static_cast<unsigned long long>(snapshot.idEntries)
          << ",0x" << static_cast<unsigned long long>(snapshot.valuesArray)
          << std::dec << ',' << snapshot.idCount << ',' << row.dataId << ','
          << row.index << ",0x" << std::hex
          << static_cast<unsigned long long>(row.valueAddress) << std::dec
          << ',' << row.intValue << ',' << row.uintValue << ','
          << row.floatValue << ',' << (row.valueReadOk ? "true" : "false")
          << '\n';
    }
  }
  return csv.str();
}

std::string VadTextAnsi(const VMMDLL_MAP_VADENTRY &entry) {
  return entry.uszText ? entry.uszText : "";
}

bool IsReadableVadProtection(std::uint32_t protection) {
  switch (protection) {
  case PAGE_READONLY:
  case PAGE_READWRITE:
  case PAGE_WRITECOPY:
  case PAGE_EXECUTE_READ:
  case PAGE_EXECUTE_READWRITE:
  case PAGE_EXECUTE_WRITECOPY:
    return true;
  default:
    return false;
  }
}

HeadlessMetadataProbeReport ProbeMetadataHeaderAtVa(
    const er2::IMemoryAccessor &mem, std::uintptr_t address) {
  HeadlessMetadataProbeReport report;
  report.address = address;
  if (!address) {
    return report;
  }

  std::vector<std::uint8_t> header(0x1000);
  report.readOk = mem.Read(address, header.data(), header.size());
  if (!report.readOk) {
    header.resize(0x120);
    report.readOk = mem.Read(address, header.data(), header.size());
  }
  if (!report.readOk) {
    return report;
  }

  report.magic = er2::ReadU32LE(header.data());
  report.version = er2::ReadU32LE(header.data() + 4);
  const er2::MetadataScoreResult score =
      er2::ScoreMetadataHeader(header.data(), header.size(), 0, true, 0);
  report.score = score.score;
  report.maxEnd = score.maxEnd;
  report.headerOk =
      er2::ReadMetadataHeaderFieldsFromMemory(mem, address, report.header);
  return report;
}

void AddMetadataRawCandidate(HeadlessRunResult &run,
                             const VMMDLL_MAP_VADENTRY &vad,
                             std::uintptr_t address,
                             const std::uint8_t *header,
                             std::size_t headerSize) {
  HeadlessMetadataRawCandidate candidate;
  candidate.address = address;
  candidate.regionText = VadTextAnsi(vad);
  candidate.privateMemory = vad.fPrivateMemory != 0;
  candidate.image = vad.fImage != 0;
  candidate.protection = vad.Protection;
  candidate.version = headerSize >= 8 ? er2::ReadU32LE(header + 4) : 0;
  const er2::MetadataScoreResult score =
      er2::ScoreMetadataHeader(header, headerSize, 0, true, 0);
  candidate.score = score.score;
  candidate.maxEnd = score.maxEnd;
  (void)er2::ReadMetadataHeaderFieldsFromMemory(er2::Mem(), address,
                                                candidate.header);
  run.metadataDiagnostics.candidates.push_back(std::move(candidate));
}

void RunMetadataDiagnosticsIfRequested(HeadlessRunResult &run,
                                       const MetickAdapter &dma) {
  if (!run.options.metadataDiagnose && !run.options.metadataProbeVa) {
    return;
  }

  run.metadataDiagnostics.requested = true;

  if (run.options.metadataProbeVa) {
    run.metadataDiagnostics.probe =
        ProbeMetadataHeaderAtVa(er2::Mem(), run.options.metadataProbeVa);
    std::ostringstream ss;
    ss << "[MetadataDiag] Probe VA " << er2::HexPtr(run.options.metadataProbeVa)
       << " read=" << (run.metadataDiagnostics.probe.readOk ? "yes" : "no")
       << " magic=" << er2::HexPtr(run.metadataDiagnostics.probe.magic)
       << " version=" << run.metadataDiagnostics.probe.version
       << " score=" << run.metadataDiagnostics.probe.score;
    AddRunLog(run, ss.str());
  }

  if (!run.options.metadataDiagnose) {
    run.metadataDiagnostics.ok = run.metadataDiagnostics.probe.readOk;
    return;
  }

  VMM_HANDLE hVMM = dma.GetVmmHandle();
  const DWORD pid = dma.GetPID();
  if (!hVMM || pid == 0) {
    run.metadataDiagnostics.error = "DMA VMM handle or PID is not available";
    AddRunLog(run, "[ERROR] Metadata diagnostics failed: " +
                       run.metadataDiagnostics.error);
    return;
  }

  PVMMDLL_MAP_VAD vadMap = nullptr;
  if (!VMMDLL_Map_GetVadU(hVMM, pid, TRUE, &vadMap) || !vadMap) {
    run.metadataDiagnostics.error = "VMMDLL_Map_GetVadU failed";
    AddRunLog(run, "[ERROR] Metadata diagnostics failed: " +
                       run.metadataDiagnostics.error);
    return;
  }

  run.metadataDiagnostics.vadCount = vadMap->cMap;
  run.metadataDiagnostics.anchor = run.options.metadataDiagnoseAnchor;
  const std::uint64_t maxBytes =
      static_cast<std::uint64_t>(run.options.metadataDiagnoseMaxMb) * 1024ull *
      1024ull;
  constexpr std::size_t kChunkSize = 0x10000;
  constexpr std::size_t kOverlap = 0x120;
  const std::uint32_t maxCandidates = 32;
  std::vector<std::uint8_t> chunk(kChunkSize + kOverlap);

  AddRunLog(run, "[MetadataDiag] Scanning VAD memory for FAB11BAF magic...");
  if (run.options.metadataDiagnoseAnchor) {
    AddRunLog(run, "[MetadataDiag] Anchor window center: " +
                       er2::HexPtr(run.options.metadataDiagnoseAnchor));
  }

  for (DWORD i = 0; i < vadMap->cMap; ++i) {
    const VMMDLL_MAP_VADENTRY &vad = vadMap->pMap[i];
    if (run.metadataDiagnostics.bytesScanned >= maxBytes) {
      break;
    }
    if (vad.vaEnd < vad.vaStart || vad.vaStart < 0x10000 ||
        vad.vaStart > 0x00007FFFFFFFFFFFull) {
      continue;
    }
    if (!IsReadableVadProtection(vad.Protection)) {
      continue;
    }
    if (!vad.fPrivateMemory && !vad.fImage) {
      continue;
    }
    if (run.options.metadataDiagnoseAnchor) {
      constexpr std::uint64_t kAnchorWindow = 256ull * 1024ull * 1024ull;
      const std::uint64_t anchor =
          static_cast<std::uint64_t>(run.options.metadataDiagnoseAnchor);
      const bool containsAnchor = anchor >= vad.vaStart && anchor <= vad.vaEnd;
      const bool nearAnchor =
          vad.vaStart <= anchor + kAnchorWindow &&
          vad.vaEnd + kAnchorWindow >= anchor;
      if (!nearAnchor) {
        continue;
      }
      if (containsAnchor) {
        ++run.metadataDiagnostics.anchorVadHits;
      }
    }

    std::uint64_t regionSize = vad.vaEnd - vad.vaStart + 1;
    if (regionSize < 0x1000) {
      continue;
    }
    const std::uint64_t left = maxBytes - run.metadataDiagnostics.bytesScanned;
    if (regionSize > left) {
      regionSize = left;
    }

    ++run.metadataDiagnostics.regionsScanned;
    std::uint64_t offset = 0;
    while (offset < regionSize &&
           run.metadataDiagnostics.bytesScanned < maxBytes) {
      const std::size_t toRead = static_cast<std::size_t>(
          (std::min<std::uint64_t>)(kChunkSize + kOverlap,
                                    regionSize - offset));
      const std::uintptr_t address =
          static_cast<std::uintptr_t>(vad.vaStart + offset);
      if (!er2::Mem().Read(address, chunk.data(), toRead)) {
        ++run.metadataDiagnostics.readFailures;
        offset += kChunkSize;
        continue;
      }

      run.metadataDiagnostics.bytesScanned += toRead;
      const std::size_t limit = toRead >= 0x120 ? toRead - 0x120 : 0;
      for (std::size_t pos = 0; pos <= limit; pos += 4) {
        const bool magicHit =
            er2::ReadU32LE(chunk.data() + pos) == 0xFAB11BAFu;
        er2::MetadataScoreResult score{0, 0};
        if (!magicHit && run.options.metadataDiagnoseHeuristic) {
          score = er2::ScoreMetadataHeader(chunk.data(), toRead, pos, true, 0);
        }

        if (!magicHit && score.score <= 0) {
          continue;
        }

        if (magicHit) {
          ++run.metadataDiagnostics.magicHits;
        } else {
          ++run.metadataDiagnostics.heuristicHits;
        }
        if (run.metadataDiagnostics.candidates.size() < maxCandidates) {
          AddMetadataRawCandidate(run, vad, address + pos, chunk.data() + pos,
                                  toRead - pos);
        }
      }

      offset += kChunkSize;
    }
  }

  VMMDLL_MemFree(vadMap);

  std::sort(run.metadataDiagnostics.candidates.begin(),
            run.metadataDiagnostics.candidates.end(),
            [](const HeadlessMetadataRawCandidate &a,
               const HeadlessMetadataRawCandidate &b) {
              if (a.score != b.score) {
                return a.score > b.score;
              }
              return a.address < b.address;
            });
  run.metadataDiagnostics.ok = true;

  std::ostringstream ss;
  ss << "[MetadataDiag] VAD scan done: vadCount="
     << run.metadataDiagnostics.vadCount
     << " regions=" << run.metadataDiagnostics.regionsScanned
     << " bytes=" << run.metadataDiagnostics.bytesScanned
     << " readFailures=" << run.metadataDiagnostics.readFailures
     << " magicHits=" << run.metadataDiagnostics.magicHits
     << " heuristicHits=" << run.metadataDiagnostics.heuristicHits
     << " anchorVadHits=" << run.metadataDiagnostics.anchorVadHits
     << " candidates=" << run.metadataDiagnostics.candidates.size();
  AddRunLog(run, ss.str());

  if (!run.metadataDiagnostics.candidates.empty()) {
    const HeadlessMetadataRawCandidate &best =
        run.metadataDiagnostics.candidates.front();
    std::ostringstream bestLine;
    bestLine << "[MetadataDiag] Best raw candidate " << er2::HexPtr(best.address)
             << " score=" << best.score << " version=" << best.version
             << " maxEnd=" << best.maxEnd
             << " private=" << (best.privateMemory ? "yes" : "no")
             << " image=" << (best.image ? "yes" : "no");
    AddRunLog(run, bestLine.str());
  }
}

void ValidateLegacyRvaCandidates(HeadlessRunResult &run) {
  run.legacyCandidates.clear();

  if (!er2::g_ctx.gameAssembly.base) {
    return;
  }

  const bool isSuper =
      run.modules.gameAssemblyName.find("Super") != std::string::npos;
  for (const LegacyRvaCandidate &candidate : kLegacyRvaCandidates) {
    LegacyRvaValidation validation;
    validation.candidate = candidate;
    validation.selectedRva = isSuper && candidate.superRva != 0
                                 ? candidate.superRva
                                 : candidate.normalRva;
    validation.address =
        er2::g_ctx.gameAssembly.base +
        static_cast<std::uintptr_t>(validation.selectedRva);

    validation.readable =
        er2::ReadPtr(er2::Mem(), validation.address, validation.pointerValue);
    validation.pointerCanonical =
        er2::IsCanonicalUserPtr(validation.pointerValue);
    run.legacyCandidates.push_back(validation);
  }
}

void ScanManagerRvasIfRequested(HeadlessRunResult &run) {
  if (!run.options.scanManagers) {
    return;
  }

  if (!er2::g_ctx.gameAssembly.base || !er2::g_ctx.gameAssembly.size) {
    AddRunLog(run,
              "[ERROR] Manager RVA scan skipped: GameAssembly not available");
    return;
  }

  er2::ManagerRvaScanOptions options;
  if (run.options.managerScanMaxMb != 0) {
    options.maxSectionBytes = run.options.managerScanMaxMb * 1024u * 1024u;
  }

  AddRunLog(run,
            "[INFO] Scanning current GameAssembly for manager/global RVAs...");
  bool scanOk = false;
  std::string lastError;
  for (int attempt = 1; attempt <= 3; ++attempt) {
    scanOk = er2::ScanGameAssemblyManagerRvas(
        er2::Mem(), er2::g_ctx.gameAssembly.base, er2::g_ctx.gameAssembly.size,
        options, run.managerScan);
    if (scanOk) {
      break;
    }

    lastError = run.managerScan.error;
    if (attempt < 3 &&
        lastError == "failed to read GameAssembly PE sections") {
      std::ostringstream retry;
      retry << "[WARN] Manager RVA scan section read failed, retry "
            << (attempt + 1) << "/3";
      AddRunLog(run, retry.str());
      std::this_thread::sleep_for(std::chrono::milliseconds(150));
      continue;
    }
    break;
  }

  if (!scanOk) {
    if (!lastError.empty()) {
      run.managerScan.error = lastError;
    }
    AddRunLog(run, "[ERROR] Manager RVA scan failed: " + run.managerScan.error);
    return;
  }

  for (er2::ManagerRvaCandidate &candidate : run.managerScan.candidates) {
    if (candidate.kind != er2::ManagerRvaKind::GlobalTime ||
        !er2::IsCanonicalUserPtr(candidate.resolvedPtr)) {
      continue;
    }

    double first = 0.0;
    double second = 0.0;
    if (!er2::ReadValue(er2::Mem(), candidate.resolvedPtr + 0x28, first)) {
      continue;
    }
    std::this_thread::sleep_for(std::chrono::milliseconds(120));
    if (!er2::ReadValue(er2::Mem(), candidate.resolvedPtr + 0x28, second)) {
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
  er2::SortManagerRvaCandidates(run.managerScan);

  std::ostringstream ss;
  ss << "[INFO] Manager RVA scan complete: candidates="
     << run.managerScan.candidates.size()
     << ", pointerSlots=" << run.managerScan.pointerSlots
     << ", bytesScanned=" << run.managerScan.bytesScanned;
  AddRunLog(run, ss.str());

  const er2::ManagerRvaKind kinds[] = {
      er2::ManagerRvaKind::CharacterManager,
      er2::ManagerRvaKind::EntityManager,
      er2::ManagerRvaKind::GlobalTime,
      er2::ManagerRvaKind::CharactorSync,
      er2::ManagerRvaKind::UserDataManager,
  };
  for (const er2::ManagerRvaKind kind : kinds) {
    const er2::ManagerRvaCandidate *best =
        er2::FindBestManagerRvaCandidate(run.managerScan, kind);
    if (!best) {
      continue;
    }
    std::ostringstream line;
    line << "[FOUND] " << best->name << " rva=" << er2::HexPtr(best->rva)
         << " score=" << best->score << " " << best->evidence;
    AddRunLog(run, line.str());
  }
}

// DMA module image dump. Reads the target module's memory image (base .. base+
// SizeOfImage) through the DMA accessor and writes it to outPath. Pages that
// DMA cannot read (paged out / guarded) are zero-filled so the whole image is
// preserved as a file usable as an offline PE image. Returns file size written,
// or 0 on total failure.
std::uint64_t DumpModuleImageViaDma(const er2::IMemoryAccessor &mem,
                                    std::uintptr_t base, std::uint32_t size,
                                    const std::string &outPath,
                                    std::string &error) {
  error.clear();
  if (!base || size == 0) {
    error = "Invalid module base/size";
    return 0;
  }
  if (size > 0x80000000u) { // 2 GiB sanity cap
    error = "Module image size exceeds safe cap";
    return 0;
  }

  std::ofstream file(outPath, std::ios::binary | std::ios::trunc);
  if (!file) {
    error = "Failed to open output file: " + outPath;
    return 0;
  }

  constexpr std::uint32_t kChunk = 0x100000; // 1 MiB per DMA read
  std::vector<std::uint8_t> buf(kChunk);
  std::uint32_t remaining = size;
  std::uint32_t offset = 0;
  std::uint64_t written = 0;
  std::uint32_t readFailures = 0;

  while (remaining > 0) {
    const std::size_t toRead =
        remaining > kChunk ? kChunk : static_cast<std::size_t>(remaining);
    if (!mem.Read(base + static_cast<std::uintptr_t>(offset), buf.data(),
                  toRead)) {
      ++readFailures;
      std::memset(buf.data(), 0, toRead); // fault-tolerant zero-fill
    }
    file.write(reinterpret_cast<const char *>(buf.data()),
               static_cast<std::streamsize>(toRead));
    offset += static_cast<std::uint32_t>(toRead);
    remaining -= static_cast<std::uint32_t>(toRead);
    written += static_cast<std::uint64_t>(toRead);
  }

  file.flush();
  const bool good = static_cast<bool>(file);
  file.close();
  if (!good) {
    error = "File write failed partway through";
    return 0;
  }
  return written;
}

void DumpModuleImagesIfRequested(HeadlessRunResult &run,
                                 const std::shared_ptr<MetickAdapter> &dma) {
  run.moduleImages.requested = !run.options.dumpImageDir.empty();
  if (!run.moduleImages.requested) {
    return;
  }
  if (!dma || !er2::g_ctx.memory) {
    run.moduleImages.error = "DMA memory accessor is not available";
    AddRunLog(run, "[ERROR] Module image dump skipped: " +
                       run.moduleImages.error);
    return;
  }

  std::error_code ec;
  std::filesystem::create_directories(
      std::filesystem::path(run.options.dumpImageDir), ec);

  // 1. GameAssembly* image (primary Il2Cpp logic module)
  if (er2::g_ctx.gameAssembly.base && er2::g_ctx.gameAssembly.size) {
    std::string gaName =
        run.modules.gameAssemblyName.empty() ? "GameAssembly.dll"
                                             : run.modules.gameAssemblyName;
    run.moduleImages.gameAssemblyName = gaName;
    // Strip .dll, append _dump.dll to align with reference naming.
    std::string baseName = gaName;
    const std::string dll = ".dll";
    if (baseName.size() >= dll.size() &&
        baseName.compare(baseName.size() - dll.size(), dll.size(), dll) == 0) {
      baseName = baseName.substr(0, baseName.size() - dll.size());
    }
    const std::string outPath =
        JoinPath(run.options.dumpImageDir, baseName + "_dump.dll");
    std::string err;
    const std::uint64_t written = DumpModuleImageViaDma(
        *er2::g_ctx.memory, er2::g_ctx.gameAssembly.base,
        er2::g_ctx.gameAssembly.size, outPath, err);
    if (written > 0) {
      run.moduleImages.gameAssemblyPath = outPath;
      run.moduleImages.gameAssemblyBytes = written;
      std::ostringstream ss;
      ss << "[SUCCESS] GameAssembly image dumped: " << outPath << " ("
         << written << " bytes)";
      AddRunLog(run, ss.str());
    } else {
      run.moduleImages.error = "GameAssembly image dump failed: " + err;
      AddRunLog(run, "[ERROR] " + run.moduleImages.error);
    }
  }

  // 2. UnityPlayer* image
  if (er2::g_ctx.unityPlayer.base && er2::g_ctx.unityPlayer.size) {
    const std::string upName =
        run.modules.unityPlayerName.empty()
            ? std::string("UnityPlayer.dll")
            : run.modules.unityPlayerName;
    std::string baseName = upName;
    const std::string dll = ".dll";
    if (baseName.size() >= dll.size() &&
        baseName.compare(baseName.size() - dll.size(), dll.size(), dll) == 0) {
      baseName = baseName.substr(0, baseName.size() - dll.size());
    }
    const std::string outPath =
        JoinPath(run.options.dumpImageDir, baseName + "_dump.dll");
    std::string err;
    const std::uint64_t written = DumpModuleImageViaDma(
        *er2::g_ctx.memory, er2::g_ctx.unityPlayer.base,
        er2::g_ctx.unityPlayer.size, outPath, err);
    if (written > 0) {
      run.moduleImages.unityPlayerPath = outPath;
      run.moduleImages.unityPlayerBytes = written;
      std::ostringstream ss;
      ss << "[SUCCESS] UnityPlayer image dumped: " << outPath << " ("
         << written << " bytes)";
      AddRunLog(run, ss.str());
    } else {
      run.moduleImages.error = "UnityPlayer image dump failed: " + err;
      AddRunLog(run, "[ERROR] " + run.moduleImages.error);
    }
  }

  // 3. Optional: all modules -> <dir>/modules/<name>.dll.dump
  if (run.options.dumpImageAllModules) {
    const std::string modsDir =
        JoinPath(run.options.dumpImageDir, "modules");
    std::filesystem::create_directories(std::filesystem::path(modsDir), ec);
    const std::vector<er2::DMAMemoryAccessor::DmaModuleInfo> modules =
        dma->EnumerateModules();
    std::uint32_t dumped = 0;
    for (const auto &m : modules) {
      if (!m.baseAddress || m.imageSize == 0) {
        continue;
      }
      // Apply optional per-module size cap (0 = no cap).
      if (run.options.dumpImageMaxMb != 0 &&
          m.imageSize > run.options.dumpImageMaxMb * 1024u * 1024u) {
        continue;
      }
      std::string safeName = m.name;
      if (safeName.empty()) {
        safeName = "module_" + std::to_string(dumped);
      }
      const std::string outPath =
          JoinPath(modsDir, safeName + ".dump");
      std::string err;
      const std::uint64_t written = DumpModuleImageViaDma(
          *er2::g_ctx.memory, m.baseAddress, m.imageSize, outPath, err);
      if (written > 0) {
        ++dumped;
      } else {
        std::ostringstream ss;
        ss << "[WARN] Module dump skipped: " << safeName << " - " << err;
        AddRunLog(run, ss.str());
      }
    }
    std::ostringstream ss;
    ss << "[INFO] All-module dump complete: " << dumped << " modules -> "
       << modsDir;
    AddRunLog(run, ss.str());
    run.moduleImages.moduleCount = dumped;
    if (dumped > 0) {
      run.moduleImages.ok = true;
    }
  }

  run.moduleImages.ok =
      (!run.moduleImages.gameAssemblyPath.empty() ||
       !run.moduleImages.unityPlayerPath.empty() ||
       run.moduleImages.moduleCount > 0);
}

void RunSelfDumpCsIfRequested(HeadlessRunResult &run) {
  run.selfDump.requested = !run.options.selfDumpCsPath.empty();
  if (!run.selfDump.requested) {
    return;
  }
  if (!er2::g_ctx.memory) {
    run.selfDump.error = "memory accessor not available";
    AddRunLog(run, "[ERROR] SelfDumpCs skipped: " + run.selfDump.error);
    return;
  }

  er2::SelfDumpCsOptions opts;
  opts.outPath = run.options.selfDumpCsPath;
  opts.logPath = run.options.selfDumpCsLog;
  opts.maxKlass = run.options.selfDumpMaxKlass;

  er2::SelfDumpCsStats stats;
  if (!er2::RunSelfDumpCs(*er2::g_ctx.memory, opts, stats)) {
    run.selfDump.error = stats.error;
    run.selfDump.typeInfoTable = stats.typeInfoTable;
    run.selfDump.typeInfoCount = stats.typeInfoCount;
    AddRunLog(run, "[ERROR] SelfDumpCs failed: " + stats.error);
    return;
  }

  run.selfDump.ok = true;
  run.selfDump.path = stats.typeInfoTable ? opts.outPath : std::string();
  run.selfDump.typeInfoTable = stats.typeInfoTable;
  run.selfDump.typeInfoCount = stats.typeInfoCount;
  run.selfDump.klassResolved = stats.klassResolved;
  run.selfDump.fieldRows = stats.fieldRows;
  run.selfDump.methodRows = stats.methodRows;
  run.selfDump.methodRva = stats.methodRva;

  std::ostringstream ss;
  ss << "[SUCCESS] SelfDumpCs written to " << opts.outPath
     << " klass=" << stats.klassResolved << " fields=" << stats.fieldRows
     << " methods=" << stats.methodRows
     << " methodsWithRva=" << stats.methodRva
     << " table=" << er2::HexPtr(stats.typeInfoTable)
     << " count=" << stats.typeInfoCount;
  AddRunLog(run, ss.str());
}

void RunDmaInjectIfRequested(HeadlessRunResult &run,
                             const std::shared_ptr<MetickAdapter> &dma) {
  run.dmaInject.requested = !run.options.dmaInjectDll.empty();
  if (!run.dmaInject.requested) {
    return;
  }
  if (!dma) {
    run.dmaInject.error = "DMA adapter not available";
    AddRunLog(run, "[ERROR] DmaInject skipped: " + run.dmaInject.error);
    return;
  }

  er2::DmaInjectOptions opts;
  opts.dllPath = run.options.dmaInjectDll;
  opts.pid = run.options.dmaInjectPid;
  opts.waitMs = run.options.dmaInjectWaitMs;
  opts.testOnly = run.options.dmaInjectTest;
  if (!run.options.dmaInjectHook.empty()) {
    const std::size_t bang = run.options.dmaInjectHook.find('!');
    if (bang != std::string::npos) {
      opts.hookModule = run.options.dmaInjectHook.substr(0, bang);
      opts.hookFunction = run.options.dmaInjectHook.substr(bang + 1);
    }
  }
  if (!opts.pid) {
    opts.pid = er2::g_ctx.pid;
  }

  run.dmaInject.dllPath = opts.dllPath;
  run.dmaInject.hookPoint = opts.hookModule + "!" + opts.hookFunction;
  if (!opts.pid) {
    run.dmaInject.error = "no target pid (use --dma-inject-pid or attach first)";
    AddRunLog(run, "[ERROR] DmaInject: " + run.dmaInject.error);
    return;
  }

  er2::DmaInjectResult result;
  AddRunLog(run, "[DMAINJ] Injecting " + opts.dllPath + " into pid " +
                    std::to_string(opts.pid) + " via " + opts.hookModule +
                    "!" + opts.hookFunction);
  if (!er2::RunDmaInject(*dma, opts, result)) {
    run.dmaInject.error = result.error;
    AddRunLog(run, "[ERROR] DmaInject failed: " + result.error);
    return;
  }

  run.dmaInject.ok = result.ok;
  run.dmaInject.moduleName = result.moduleName;
  run.dmaInject.iatThunk = result.iatThunk;
  run.dmaInject.iatFunction = result.iatFunction;
  run.dmaInject.codeCave = result.codeCave;
  run.dmaInject.dataCave = result.dataCave;
  run.dmaInject.loadLibraryAddr = result.loadLibraryAddr;
  run.dmaInject.loadLibraryResult = result.loadLibraryResult;
  run.dmaInject.executed = result.executed;
  run.dmaInject.restored = result.restored;
  run.dmaInject.loadLibraryResult = result.loadLibraryResult;

  std::ostringstream ss;
  ss << "[DMAINJ] result: executed=" << (result.executed ? "true" : "false")
     << " iatThunk=0x" << er2::HexPtr(result.iatThunk)
     << " codeCave=0x" << er2::HexPtr(result.codeCave)
     << " dataCave=0x" << er2::HexPtr(result.dataCave)
     << " LoadLibraryA=0x" << er2::HexPtr(result.loadLibraryAddr)
     << " result=0x" << er2::HexPtr(result.loadLibraryResult)
     << " restored=" << (result.restored ? "true" : "false");
  AddRunLog(run, ss.str());
  if (!result.executed) {
    run.dmaInject.error = "injected but shellcode did not execute within wait";
    AddRunLog(run, "[WARN] " + run.dmaInject.error);
  }
}

void ProbeNarakaBuffManagerIfRequested(HeadlessRunResult &run) {
  run.narakaBuffProbe.requested = run.options.narakaBuffProbe;
  if (!run.options.narakaBuffProbe) {
    return;
  }

  if (!er2::g_ctx.memory || !er2::g_ctx.gameAssembly.base) {
    run.narakaBuffProbe.error =
        "GameAssembly or memory accessor is not available";
    AddRunLog(run, "[ERROR] Naraka BuffManager probe skipped: " +
                       run.narakaBuffProbe.error);
    return;
  }

  const std::size_t maxEntries =
      std::max<std::uint32_t>(1, run.options.narakaBuffMaxEntries);
  er2::NarakaBuffManagerOptions options =
      er2::DefaultNarakaBuffManagerOptions();
  options.offsets.scanEnd =
      std::max<std::uint32_t>(options.offsets.scanStart,
                              run.options.narakaBuffScanEnd);
  AddRunLog(run, "[INFO] Probing Naraka BuffManager via TypeInfo/static_fields");
  er2::NarakaBuffManagerSnapshot snapshot;
  if (!er2::CaptureNarakaBuffManagerSnapshot(
          er2::Mem(), er2::g_ctx.gameAssembly.base, options, maxEntries,
          snapshot)) {
    run.narakaBuffProbe.snapshot = std::move(snapshot);
    run.narakaBuffProbe.error = run.narakaBuffProbe.snapshot.error;
    AddRunLog(run, "[ERROR] Naraka BuffManager probe failed: " +
                       run.narakaBuffProbe.error);
    return;
  }

  run.narakaBuffProbe.ok = true;
  run.narakaBuffProbe.snapshot = std::move(snapshot);
  std::ostringstream line;
  line << "[SUCCESS] Naraka BuffManager class="
       << er2::HexPtr(run.narakaBuffProbe.snapshot.buffManagerClass)
       << " static="
       << er2::HexPtr(run.narakaBuffProbe.snapshot.staticFields)
       << " instance=" << er2::HexPtr(run.narakaBuffProbe.snapshot.instance)
       << " containers=" << run.narakaBuffProbe.snapshot.containers.size()
       << " entries=" << run.narakaBuffProbe.snapshot.entries.size();
  AddRunLog(run, line.str());
}

bool TryPickNarakaActorForProbe(HeadlessRunResult &run,
                                std::uintptr_t &actorModel,
                                std::string *errorOut = nullptr) {
  actorModel = run.options.narakaActorAddress;
  if (er2::IsCanonicalUserPtr(actorModel)) {
    return true;
  }

  auto setError = [&](const std::string &message) {
    if (errorOut) {
      *errorOut = message;
      return;
    }
    run.narakaActorContainerProbe.error = message;
  };

  if (!er2::g_ctx.memory || !er2::g_ctx.gameAssembly.base) {
    setError("GameAssembly or memory accessor is not available");
    return false;
  }

  std::uintptr_t characterManager = 0;
  if (!er2::ResolveNarakaCharacterManagerInstance(
          er2::Mem(), er2::g_ctx.gameAssembly.base, characterManager)) {
    setError("failed to resolve CharacterManager instance");
    return false;
  }

  std::vector<std::uintptr_t> actors;
  std::string error;
  if (!er2::EnumerateNarakaActorModelsFromCharacterManager(
          er2::Mem(), characterManager, {}, 256, actors, &error)) {
    setError(error);
    return false;
  }

  std::uintptr_t firstValid = 0;
  for (const std::uintptr_t actor : actors) {
    er2::NarakaActorRuntimePropertyCandidate candidate;
    if (!er2::ReadNarakaActorRuntimePropertyCandidate(
            er2::Mem(), actor, er2::DefaultNarakaRuntimePropertyOptions(),
            candidate)) {
      continue;
    }
    if (!firstValid) {
      firstValid = actor;
    }
    if (run.options.narakaActorOnlyTarka && candidate.heroId == 1000006u) {
      actorModel = actor;
      return true;
    }
  }

  actorModel = firstValid;
  return er2::IsCanonicalUserPtr(actorModel);
}

void ProbeNarakaRuntimePropertiesIfRequested(HeadlessRunResult &run) {
  run.narakaRuntimePropertyProbe.requested =
      run.options.narakaRuntimePropertyProbe;
  if (!run.options.narakaRuntimePropertyProbe) {
    return;
  }

  std::uintptr_t actorModel = 0;
  std::string pickError;
  if (!TryPickNarakaActorForProbe(run, actorModel, &pickError)) {
    run.narakaRuntimePropertyProbe.error =
        pickError.empty() ? "no ActorModel candidate found" : pickError;
    AddRunLog(run, "[ERROR] Naraka RuntimeProperty probe skipped: " +
                       run.narakaRuntimePropertyProbe.error);
    return;
  }

  run.narakaRuntimePropertyProbe.actorModel = actorModel;
  const std::size_t maxRows =
      std::max<std::uint32_t>(1, run.options.narakaRuntimePropertyMaxRows);
  const std::uint32_t samples =
      std::max<std::uint32_t>(1, run.options.narakaRuntimePropertySamples);
  const std::uint32_t intervalMs =
      run.options.narakaRuntimePropertyIntervalMs;

  std::ostringstream begin;
  begin << "[INFO] Probing Naraka RuntimeProperty actor="
        << er2::HexPtr(actorModel) << " samples=" << samples
        << " intervalMs=" << intervalMs << " maxRows=" << maxRows;
  AddRunLog(run, begin.str());

  run.narakaRuntimePropertyProbe.snapshots.reserve(samples);
  for (std::uint32_t i = 0; i < samples; ++i) {
    er2::NarakaRuntimePropertySnapshot snapshot;
    if (!er2::CaptureNarakaRuntimeProperties(er2::Mem(), actorModel, maxRows,
                                             snapshot)) {
      run.narakaRuntimePropertyProbe.error = snapshot.error;
      AddRunLog(run, "[ERROR] Naraka RuntimeProperty probe failed: " +
                         run.narakaRuntimePropertyProbe.error);
      return;
    }
    run.narakaRuntimePropertyProbe.snapshots.push_back(std::move(snapshot));
    if (i + 1 < samples && intervalMs > 0) {
      std::this_thread::sleep_for(std::chrono::milliseconds(intervalMs));
    }
  }

  if (run.narakaRuntimePropertyProbe.snapshots.size() >= 2) {
    run.narakaRuntimePropertyProbe.diff =
        er2::DiffNarakaRuntimePropertySnapshots(
            run.narakaRuntimePropertyProbe.snapshots.front(),
            run.narakaRuntimePropertyProbe.snapshots.back());
  }

  if (!run.options.narakaRuntimePropertyCsvPath.empty()) {
    run.narakaRuntimePropertyProbe.csvPath =
        run.options.narakaRuntimePropertyCsvPath;
    if (!WriteTextFile(run.options.narakaRuntimePropertyCsvPath,
                       BuildNarakaRuntimePropertyCsv(
                           run.narakaRuntimePropertyProbe.snapshots))) {
      run.narakaRuntimePropertyProbe.error =
          "failed to write RuntimeProperty CSV output";
      AddRunLog(run, "[ERROR] Naraka RuntimeProperty probe failed: " +
                         run.narakaRuntimePropertyProbe.error);
      return;
    }
  }

  run.narakaRuntimePropertyProbe.ok = true;
  const auto &last = run.narakaRuntimePropertyProbe.snapshots.back();
  const std::size_t changedCount = static_cast<std::size_t>(std::count_if(
      run.narakaRuntimePropertyProbe.diff.begin(),
      run.narakaRuntimePropertyProbe.diff.end(),
      [](const er2::NarakaRuntimePropertyDiffRow &row) {
        return row.changed;
      }));
  std::ostringstream line;
  line << "[SUCCESS] Naraka RuntimeProperty actor="
       << er2::HexPtr(last.actorModel) << " hero=" << last.heroId
       << " property=" << er2::HexPtr(last.propertyData)
       << " runtime=" << er2::HexPtr(last.runtimePropertyData)
       << " rows=" << last.rows.size() << " changed=" << changedCount;
  if (!run.narakaRuntimePropertyProbe.csvPath.empty()) {
    line << " csv=" << run.narakaRuntimePropertyProbe.csvPath;
  }
  AddRunLog(run, line.str());
}

void ProbeNarakaGroundIfRequested(HeadlessRunResult &run) {
  run.narakaGroundProbe.requested = run.options.narakaGroundProbe;
  if (!run.options.narakaGroundProbe) {
    return;
  }

  if (!er2::g_ctx.memory || !er2::g_ctx.gameAssembly.base) {
    run.narakaGroundProbe.error =
        "GameAssembly or memory accessor is not available";
    AddRunLog(run, "[ERROR] Naraka ground probe skipped: " +
                       run.narakaGroundProbe.error);
    return;
  }

  constexpr std::uint32_t kCharacterManagerLocalActor = 0x18;
  constexpr std::uint32_t kActorModelActorKit = 0xC8;
  constexpr std::uint32_t kEntityKitActorPhysics = 0xD8;
  constexpr std::uint32_t kEntityKitAdvanceGroundStatus = 0x168;
  constexpr std::uint32_t kEntityKitAlignToGroundData = 0x178;
  constexpr std::uint32_t kActorKitPhysicsCurrentCollisionFlags = 0xEC;
  constexpr std::uint32_t kGroundStatusNearGround = 0x10;
  constexpr std::uint32_t kGroundStatusGroundDistance = 0x14;
  constexpr std::uint32_t kAlignToGroundRootGroundDistance = 0x18;
  constexpr std::int32_t kCollisionFlagsBelow = 4;

  auto fail = [&](const std::string &message) {
    run.narakaGroundProbe.error = message;
    AddRunLog(run, "[ERROR] Naraka ground probe failed: " + message);
  };

  if (!er2::ResolveNarakaCharacterManagerInstance(
          er2::Mem(), er2::g_ctx.gameAssembly.base,
          run.narakaGroundProbe.characterManager)) {
    fail("failed to resolve CharacterManager instance");
    return;
  }

  if (!er2::ReadPtr(er2::Mem(),
                    run.narakaGroundProbe.characterManager +
                        kCharacterManagerLocalActor,
                    run.narakaGroundProbe.actorModel) ||
      !er2::IsCanonicalUserPtr(run.narakaGroundProbe.actorModel)) {
    fail("failed to read local ActorModel");
    return;
  }

  if (!er2::ReadPtr(er2::Mem(),
                    run.narakaGroundProbe.actorModel + kActorModelActorKit,
                    run.narakaGroundProbe.actorKit) ||
      !er2::IsCanonicalUserPtr(run.narakaGroundProbe.actorKit)) {
    fail("failed to read ActorModel.actorKit");
    return;
  }

  if (!er2::ReadPtr(er2::Mem(),
                    run.narakaGroundProbe.actorKit + kEntityKitActorPhysics,
                    run.narakaGroundProbe.actorPhysics) ||
      !er2::IsCanonicalUserPtr(run.narakaGroundProbe.actorPhysics)) {
    fail("failed to read EntityKit.actorPhysics");
    return;
  }

  if (!er2::ReadValue(er2::Mem(),
                      run.narakaGroundProbe.actorPhysics +
                          kActorKitPhysicsCurrentCollisionFlags,
                      run.narakaGroundProbe.currentCollisionFlags)) {
    fail("failed to read ActorKitPhysics._currentCollisionFlags");
    return;
  }
  run.narakaGroundProbe.isGroundedByCollisionFlags =
      (run.narakaGroundProbe.currentCollisionFlags & kCollisionFlagsBelow) != 0;

  (void)er2::ReadPtr(er2::Mem(),
                     run.narakaGroundProbe.actorKit +
                         kEntityKitAdvanceGroundStatus,
                     run.narakaGroundProbe.advanceGroundStatus);
  if (er2::IsCanonicalUserPtr(run.narakaGroundProbe.advanceGroundStatus)) {
    (void)er2::ReadValue(er2::Mem(),
                         run.narakaGroundProbe.advanceGroundStatus +
                             kGroundStatusNearGround,
                         run.narakaGroundProbe.advanceNearGround);
    (void)er2::ReadValue(er2::Mem(),
                         run.narakaGroundProbe.advanceGroundStatus +
                             kGroundStatusGroundDistance,
                         run.narakaGroundProbe.advanceGroundDistance);
  }

  (void)er2::ReadPtr(er2::Mem(),
                     run.narakaGroundProbe.actorKit +
                         kEntityKitAlignToGroundData,
                     run.narakaGroundProbe.alignToGroundData);
  if (er2::IsCanonicalUserPtr(run.narakaGroundProbe.alignToGroundData)) {
    (void)er2::ReadValue(er2::Mem(),
                         run.narakaGroundProbe.alignToGroundData +
                             kGroundStatusNearGround,
                         run.narakaGroundProbe.alignNearGround);
    (void)er2::ReadValue(er2::Mem(),
                         run.narakaGroundProbe.alignToGroundData +
                             kGroundStatusGroundDistance,
                         run.narakaGroundProbe.alignGroundDistance);
    (void)er2::ReadValue(er2::Mem(),
                         run.narakaGroundProbe.alignToGroundData +
                             kAlignToGroundRootGroundDistance,
                         run.narakaGroundProbe.alignRootGroundDistance);
  }

  run.narakaGroundProbe.ok = true;
  std::ostringstream line;
  line << "[SUCCESS] Naraka ground actor="
       << er2::HexPtr(run.narakaGroundProbe.actorModel)
       << " actorKit=" << er2::HexPtr(run.narakaGroundProbe.actorKit)
       << " actorPhysics=" << er2::HexPtr(run.narakaGroundProbe.actorPhysics)
       << " collisionFlags=" << run.narakaGroundProbe.currentCollisionFlags
       << " grounded="
       << (run.narakaGroundProbe.isGroundedByCollisionFlags ? "true"
                                                            : "false")
       << " advanceNear=" << (run.narakaGroundProbe.advanceNearGround ? "true"
                                                                      : "false")
       << " advanceDist=" << run.narakaGroundProbe.advanceGroundDistance
       << " alignNear=" << (run.narakaGroundProbe.alignNearGround ? "true"
                                                                  : "false")
       << " alignDist=" << run.narakaGroundProbe.alignGroundDistance
       << " rootDist=" << run.narakaGroundProbe.alignRootGroundDistance;
  AddRunLog(run, line.str());
}

void ProbeNarakaActorContainersIfRequested(HeadlessRunResult &run) {
  run.narakaActorContainerProbe.requested =
      run.options.narakaActorContainerProbe;
  if (!run.options.narakaActorContainerProbe) {
    return;
  }

  std::uintptr_t actorModel = 0;
  if (!TryPickNarakaActorForProbe(run, actorModel)) {
    if (run.narakaActorContainerProbe.error.empty()) {
      run.narakaActorContainerProbe.error = "no ActorModel candidate found";
    }
    AddRunLog(run, "[ERROR] Naraka Actor container probe skipped: " +
                       run.narakaActorContainerProbe.error);
    return;
  }

  er2::NarakaActorContainerOptions options =
      er2::DefaultNarakaActorContainerOptions();
  options.offsets.scanEnd =
      std::max<std::uint32_t>(options.offsets.scanStart,
                              run.options.narakaActorScanEnd);
  const std::size_t maxEntries =
      std::max<std::uint32_t>(1, run.options.narakaActorMaxEntries);

  std::ostringstream begin;
  begin << "[INFO] Probing Naraka Actor containers actor="
        << er2::HexPtr(actorModel) << " scanEnd="
        << er2::HexPtr(options.offsets.scanEnd);
  AddRunLog(run, begin.str());

  er2::NarakaActorContainerSnapshot snapshot;
  if (!er2::CaptureNarakaActorContainerSnapshot(er2::Mem(), actorModel,
                                                options, maxEntries,
                                                snapshot)) {
    run.narakaActorContainerProbe.snapshot = std::move(snapshot);
    run.narakaActorContainerProbe.error =
        run.narakaActorContainerProbe.snapshot.error;
    AddRunLog(run, "[ERROR] Naraka Actor container probe failed: " +
                       run.narakaActorContainerProbe.error);
    return;
  }

  run.narakaActorContainerProbe.ok = true;
  run.narakaActorContainerProbe.snapshot = std::move(snapshot);
  std::ostringstream line;
  line << "[SUCCESS] Naraka Actor containers actor="
       << er2::HexPtr(run.narakaActorContainerProbe.snapshot.actorModel)
       << " hero=" << run.narakaActorContainerProbe.snapshot.heroId
       << " property="
       << er2::HexPtr(run.narakaActorContainerProbe.snapshot.propertyData)
       << " runtime="
       << er2::HexPtr(
              run.narakaActorContainerProbe.snapshot.runtimePropertyData)
       << " containers="
       << run.narakaActorContainerProbe.snapshot.containers.size()
       << " entries="
       << run.narakaActorContainerProbe.snapshot.entries.size();
  AddRunLog(run, line.str());
}

std::string BuildOffsetHeader(const HeadlessRunResult &run) {
  std::ostringstream h;
  h << "#pragma once\n";
  h << "#include <cstdint>\n\n";
  h << "// Generated by ExternalResolveConsole --headless.\n";
  h << "// Scope: local DMA/CTF runtime snapshot. Re-run after every target "
       "update.\n";
  h << "// JSON source: " << run.options.outPath << "\n\n";

  h << "namespace Offset {\n";
  h << "inline std::uint64_t UnityPlayer = "
    << er2::HexPtr(er2::g_ctx.unityPlayer.base) << "ull;\n";
  h << "inline std::uint64_t GameAssembly = "
    << er2::HexPtr(er2::g_ctx.gameAssembly.base) << "ull;\n";
  h << "inline std::uint64_t NarakaBladepoint = 0x0ull;\n";
  h << "inline std::uint64_t MatrixAddr = 0x0ull;\n\n";

  h << "namespace RuntimeConfirmed {\n";
  h << "inline constexpr std::uint64_t UnityPlayerBase = "
    << er2::HexPtr(er2::g_ctx.unityPlayer.base) << "ull;\n";
  h << "inline constexpr std::uint64_t GameAssemblyBase = "
    << er2::HexPtr(er2::g_ctx.gameAssembly.base) << "ull;\n";
  h << "inline constexpr std::uint64_t MsIdToPointerSlot = "
    << er2::HexPtr(run.msidSlot) << "ull;\n";
  h << "inline constexpr std::uint64_t MsIdToPointerSlotRva = "
    << er2::HexPtr(run.msidSlot && er2::g_ctx.unityPlayer.base
                       ? run.msidSlot - er2::g_ctx.unityPlayer.base
                       : 0)
    << "ull;\n";
  h << "inline constexpr std::uint32_t MsIdCount = " << run.msidCount
    << "u;\n";
  h << "inline constexpr std::uint32_t MatchedGameObjectCount = "
    << static_cast<std::uint32_t>(run.matchedObjectCount) << "u;\n";
  if (!run.objects.empty()) {
    h << "inline constexpr std::uint64_t SampleGameObjectNative = "
      << er2::HexPtr(run.objects.front().info.native) << "ull;\n";
    h << "inline constexpr std::uint64_t SampleGameObjectManaged = "
      << er2::HexPtr(run.objects.front().managed) << "ull;\n";
    h << "inline constexpr std::uint64_t GameObjectKlass = "
      << er2::HexPtr(run.objects.front().klass) << "ull;\n";
  } else {
    h << "inline constexpr std::uint64_t SampleGameObjectNative = 0x0ull;\n";
    h << "inline constexpr std::uint64_t SampleGameObjectManaged = 0x0ull;\n";
    h << "inline constexpr std::uint64_t GameObjectKlass = 0x0ull;\n";
  }
  h << "} // namespace RuntimeConfirmed\n\n";

  h << "// These are the fixed native Unity offsets currently validated by the "
       "browser path.\n";
  h << "struct UnityNativeObject_t {\n";
  h << "  std::uint32_t InstanceId = 0x8;\n";
  h << "  std::uint32_t ManagedPtr = 0x28;\n";
  h << "  std::uint32_t ComponentPool = 0x30;\n";
  h << "  std::uint32_t ComponentCount = 0x40;\n";
  h << "  std::uint32_t NamePtr = 0x60;\n";
  h << "};\n";
  h << "inline UnityNativeObject_t UnityNativeObject;\n\n";

  h << "struct UnityNativeComponent_t {\n";
  h << "  std::uint32_t ManagedPtr = 0x28;\n";
  h << "  std::uint32_t GameObject = 0x30;\n";
  h << "  std::uint32_t Enabled = 0x38;\n";
  h << "};\n";
  h << "inline UnityNativeComponent_t UnityNativeComponent;\n\n";

  h << "namespace LegacyCandidates {\n";
  for (const LegacyRvaValidation &validation : run.legacyCandidates) {
    h << "// " << validation.candidate.comment << "\n";
    h << "inline constexpr std::uint64_t " << validation.candidate.name
      << " = " << er2::HexPtr(validation.selectedRva) << "ull;\n";
    h << "inline constexpr std::uint64_t " << validation.candidate.name
      << "_Address = " << er2::HexPtr(validation.address) << "ull;\n";
    h << "inline constexpr bool " << validation.candidate.name
      << "_Readable = " << FormatBoolLiteral(validation.readable) << ";\n";
    h << "inline constexpr std::uint64_t " << validation.candidate.name
      << "_PointerValue = " << er2::HexPtr(validation.pointerValue) << "ull;\n";
    h << "inline constexpr bool " << validation.candidate.name
      << "_PointerCanonical = "
      << FormatBoolLiteral(validation.pointerCanonical) << ";\n\n";
  }
  h << "} // namespace LegacyCandidates\n\n";

  h << "namespace CurrentDiscovered {\n";
  h << "// Live GameAssembly scan results. These are current-process RVAs, not "
       "legacy constants.\n";
  const er2::ManagerRvaKind kinds[] = {
      er2::ManagerRvaKind::CharacterManager,
      er2::ManagerRvaKind::EntityManager,
      er2::ManagerRvaKind::GlobalTime,
      er2::ManagerRvaKind::CharactorSync,
      er2::ManagerRvaKind::UserDataManager,
  };
  for (const er2::ManagerRvaKind kind : kinds) {
    const er2::ManagerRvaCandidate *best =
        er2::FindBestConfirmedManagerRvaCandidate(run.managerScan, kind);
    const er2::ManagerRvaCandidate *bestAny =
        er2::FindBestManagerRvaCandidate(run.managerScan, kind);
    const char *name = er2::detail_manager_rva::KindName(kind);
    h << "inline constexpr std::uint64_t " << name << " = "
      << er2::HexPtr(best ? best->rva : 0) << "ull;\n";
    h << "inline constexpr std::uint64_t " << name << "_Address = "
      << er2::HexPtr(best ? best->slotVa : 0) << "ull;\n";
    h << "inline constexpr std::uint64_t " << name << "_ResolvedPtr = "
      << er2::HexPtr(best ? best->resolvedPtr : 0) << "ull;\n";
    h << "inline constexpr std::uint32_t " << name << "_Score = "
      << (best ? best->score : 0) << "u;\n";
    h << "inline constexpr bool " << name << "_Confirmed = "
      << FormatBoolLiteral(best && best->confirmed) << ";\n\n";
    h << "inline constexpr std::uint64_t " << name << "_BestCandidate = "
      << er2::HexPtr(bestAny ? bestAny->rva : 0) << "ull;\n";
    h << "inline constexpr std::uint32_t " << name
      << "_BestCandidateScore = " << (bestAny ? bestAny->score : 0)
      << "u;\n\n";
  }
  h << "} // namespace CurrentDiscovered\n\n";

  h << "// Old field offsets retained as hypotheses. Treat as confirmed only "
       "after class/field probe proves them for this build.\n";
  h << "struct Character_t {\n";
  h << "  std::uint32_t lastSPLFramCount = 0xB8;\n";
  h << "  std::uint32_t AllAliveCharList = 0x58;\n";
  h << "  std::uint32_t ArrayStart = 0x30;\n";
  h << "  std::uint32_t fpsFilter = 0x48;\n";
  h << "};\n";
  h << "inline Character_t Character;\n\n";

  h << "struct ActorModel_t {\n";
  h << "  std::uint32_t ObjectMsg = 0xD8;\n";
  h << "  std::uint32_t EventTracksForLayer = 0x30;\n";
  h << "  std::uint32_t Transform = 0x118;\n";
  h << "  std::uint32_t ActorWeapon = 0xF8;\n";
  h << "  std::uint32_t WeaponType = 0x68;\n";
  h << "  std::uint32_t RuntimePropertyData = 0xA8;\n";
  h << "  std::uint32_t PropertyData = 0x348;\n";
  h << "  std::uint32_t cullingVisible = 0x6C;\n";
  h << "  std::uint32_t ActorKit = 0xC8;\n";
  h << "  struct ProPerty_t {\n";
  h << "    std::uint32_t Heroid = 0x108;\n";
  h << "    std::uint32_t Group = 0x118;\n";
  h << "    std::uint32_t WeaponId = 0x128;\n";
  h << "    std::uint32_t RobotId = 0x130;\n";
  h << "    std::uint32_t PlayerName = 0x200;\n";
  h << "    std::uint32_t SkillStatus = 0x198;\n";
  h << "  } ProPerty;\n";
  h << "};\n";
  h << "inline ActorModel_t ActorModel;\n";
  h << "} // namespace Offset\n";

  return h.str();
}

bool InitHeadlessDma(HeadlessRunResult &run,
                     std::shared_ptr<MetickAdapter> &dma) {
  er2::ResetContext();

  AddRunLog(run, "[INFO] Initializing DMA connection (Metick + FixCR3)...");
  dma = std::make_shared<MetickAdapter>();

  AddRunLog(run, "[INFO] Initializing Metick DMA for: " +
                     run.options.targetProcess);
  if (!dma->Initialize(run.options.targetProcess)) {
    const std::string detail = dma->GetLastError();
    run.error = detail.empty() ? "failed to initialize DMA adapter"
                               : "failed to initialize DMA adapter: " + detail;
    AddRunLog(run, "[ERROR] " + run.error);
    return false;
  }

  AddRunLog(run, "[INFO] Attempting to fix CR3 (DTB)...");
  if (dma->FixCr3()) {
    AddRunLog(run, "[SUCCESS] CR3 fixed");
  } else {
    AddRunLog(run, "[WARN] CR3 fix failed; continuing for diagnostics");
  }

  er2::g_ctx.pid =
      static_cast<std::uint32_t>(mem::Get_Process_Id(
          run.options.targetProcess.c_str()));
  er2::g_ctx.memory = dma;

  if (!er2::g_ctx.pid) {
    run.error = "target process PID was not found after DMA init";
    return false;
  }

  {
    std::ostringstream ss;
    ss << "[INFO] Attached to PID: " << er2::g_ctx.pid;
    AddRunLog(run, ss.str());
  }

  AddRunLog(run, "[INFO] Enumerating loaded modules...");
  std::vector<er2::UnityModuleCandidate> candidates;
  const auto modules = dma->EnumerateModules();
  run.modules.moduleCount = modules.size();
  candidates.reserve(modules.size());
  for (const auto &module : modules) {
    candidates.push_back(
        {module.name, module.baseAddress, module.imageSize});
  }

  {
    std::ostringstream ss;
    ss << "[INFO] Found " << candidates.size() << " modules in process";
    AddRunLog(run, ss.str());
  }

  const er2::UnityRuntimeModules runtimeModules =
      er2::FindUnityRuntimeModules(candidates);
  if (!runtimeModules.unityPlayer.base) {
    run.error = "UnityPlayer*.dll module was not found";
    return false;
  }

  er2::g_ctx.unityPlayer.base = runtimeModules.unityPlayer.base;
  er2::g_ctx.unityPlayer.size = runtimeModules.unityPlayer.size;
  er2::g_ctx.unityPlayerRange.base = runtimeModules.unityPlayer.base;
  er2::g_ctx.unityPlayerRange.size = runtimeModules.unityPlayer.size;
  run.modules.unityPlayerName = runtimeModules.unityPlayer.name;

  {
    std::ostringstream ss;
    ss << "[INFO] UnityPlayer module: " << runtimeModules.unityPlayer.name
       << " @ " << er2::HexPtr(runtimeModules.unityPlayer.base);
    AddRunLog(run, ss.str());
  }

  if (runtimeModules.gameAssembly.base) {
    er2::g_ctx.runtime = er2::ManagedBackend::Il2Cpp;
    er2::g_ctx.gameAssembly.base = runtimeModules.gameAssembly.base;
    er2::g_ctx.gameAssembly.size = runtimeModules.gameAssembly.size;
    run.modules.gameAssemblyName = runtimeModules.gameAssembly.name;

    std::ostringstream ss;
    ss << "[INFO] GameAssembly module: " << runtimeModules.gameAssembly.name
       << " @ " << er2::HexPtr(runtimeModules.gameAssembly.base);
    AddRunLog(run, ss.str());
  } else {
    er2::g_ctx.runtime = er2::ManagedBackend::Mono;
    AddRunLog(run, "[WARN] GameAssembly*.dll module not found; assuming Mono");
  }

  const er2::IMemoryAccessor &mem = er2::Mem();
  run.modules.unityDosOk =
      ReadDosHeaderOk(mem, er2::g_ctx.unityPlayer.base);
  run.modules.gameAssemblyDosOk =
      ReadDosHeaderOk(mem, er2::g_ctx.gameAssembly.base);

  if (!run.modules.unityDosOk) {
    run.error = "UnityPlayer DOS header is unreadable or invalid";
    return false;
  }

  if (er2::g_ctx.runtime == er2::ManagedBackend::Il2Cpp &&
      !run.modules.gameAssemblyDosOk) {
    run.error = "GameAssembly DOS header is unreadable or invalid";
    return false;
  }

  AddRunLog(run, "[INFO] PE header access verified");
  return true;
}

void ScanMsidIfRequested(HeadlessRunResult &run) {
  if (run.options.scanMode == er2::HeadlessScanMode::None) {
    return;
  }

  AddRunLog(run, "[INFO] Scanning MSID slot...");
  if (!er2::FindMsIdToPointerSlotVaByScan(
          er2::Mem(), er2::g_ctx.unityPlayer, er2::g_ctx.gomOff,
          run.msidSlot, &run.msidScanScore)) {
    AddRunLog(run, "[ERROR] MSID slot scan failed");
    return;
  }

  er2::g_ctx.msIdToPointerSlotVa = run.msidSlot;
  er2::g_ctx.msIdToPointerSlotRva =
      static_cast<std::uint64_t>(run.msidSlot - er2::g_ctx.unityPlayer.base);
  run.msidCount = er2::MsIdCount();

  {
    std::ostringstream ss;
    ss << "[SUCCESS] MSID slot found at " << er2::HexPtr(run.msidSlot)
       << ", count=" << run.msidCount;
    AddRunLog(run, ss.str());
  }
}

void EnumerateObjectsIfRequested(
    HeadlessRunResult &run, const er2::Il2CppLayoutProfile *profile) {
  if (!run.msidSlot || run.options.scanMode == er2::HeadlessScanMode::None) {
    return;
  }

  er2::EnumerateOptions opt;
  if (run.options.scanMode == er2::HeadlessScanMode::GameObjects) {
    opt.onlyGameObject = true;
    opt.onlyScriptableObject = false;
  } else if (run.options.scanMode ==
             er2::HeadlessScanMode::ScriptableObjects) {
    opt.onlyGameObject = false;
    opt.onlyScriptableObject = true;
  } else {
    opt.onlyGameObject = false;
    opt.onlyScriptableObject = false;
  }
  opt.filterLower = er2::HeadlessToLowerAscii(run.options.filter);

  AddRunLog(run, "[INFO] Enumerating MSID objects...");
  const bool ok = er2::EnumerateMsIdToPointerObjects(
      er2::g_ctx.runtime, er2::Mem(), run.msidSlot, er2::g_ctx.off,
      er2::g_ctx.unityPlayerRange, opt, [&](const er2::ObjectInfo &info) {
        ++run.matchedObjectCount;
        if (run.objects.size() >= run.options.limit) {
          return;
        }

        HeadlessObjectRow row;
        row.info = info;
        (void)TryReadManagedAndKlassFromNative(er2::Mem(), info.native, profile,
                                               row.managed, row.klass);
        run.objects.push_back(std::move(row));
      });

  if (!ok) {
    AddRunLog(run, "[ERROR] MSID object enumeration failed");
    return;
  }

  {
    std::ostringstream ss;
    ss << "[INFO] MSID object enumeration matched "
       << run.matchedObjectCount << ", stored " << run.objects.size();
    AddRunLog(run, ss.str());
  }
}

void BuildKlassReports(HeadlessRunResult &run,
                       const er2::Il2CppLayoutProfile *profile) {
  if (!profile || er2::g_ctx.runtime != er2::ManagedBackend::Il2Cpp) {
    return;
  }

  std::map<std::uintptr_t, std::vector<std::string>> klassSources;
  std::map<std::uintptr_t, std::vector<std::uintptr_t>> klassInstances;
  auto addKlass = [&](std::uintptr_t klass, std::string source,
                      std::uintptr_t instance = 0) {
    if (!er2::IsCanonicalUserPtr(klass)) {
      return;
    }
    std::vector<std::string> &sources = klassSources[klass];
    if (std::find(sources.begin(), sources.end(), source) == sources.end()) {
      sources.push_back(std::move(source));
    }
    if (er2::IsCanonicalUserPtr(instance)) {
      std::vector<std::uintptr_t> &instances = klassInstances[klass];
      if (std::find(instances.begin(), instances.end(), instance) ==
          instances.end()) {
        instances.push_back(instance);
      }
    }
  };

  if (er2::IsCanonicalUserPtr(run.options.klass)) {
    addKlass(run.options.klass, "--klass");
  }

  if (er2::g_ctx.gameAssembly.base) {
    for (const std::uint64_t rva : run.options.runtimeDumpRvas) {
      std::uintptr_t slotPointer = 0;
      const std::uintptr_t slotVa =
          er2::g_ctx.gameAssembly.base + static_cast<std::uintptr_t>(rva);
      if (er2::ReadPtr(er2::Mem(), slotVa, slotPointer) &&
          er2::IsCanonicalUserPtr(slotPointer)) {
        const std::string label = er2::RuntimeDumpRvaLabel(rva);
        std::ostringstream source;
        source << "--runtime-dump-rvas rva=" << er2::HexPtr(rva)
               << " slotVa=" << er2::HexPtr(slotVa)
               << " mode=slot-value-as-klass";
        if (!label.empty()) {
          source << " name=" << label;
        }
        addKlass(slotPointer, source.str(), slotPointer);

        std::uintptr_t objectKlass = 0;
        if (er2::ReadPtr(er2::Mem(), slotPointer, objectKlass) &&
            er2::IsCanonicalUserPtr(objectKlass)) {
          std::ostringstream objectSource;
          objectSource << "--runtime-dump-rvas rva=" << er2::HexPtr(rva)
                       << " slotVa=" << er2::HexPtr(slotVa)
                       << " object=" << er2::HexPtr(slotPointer)
                       << " mode=slot-value-object-klass";
          if (!label.empty()) {
            objectSource << " name=" << label;
          }
          addKlass(objectKlass, objectSource.str(), slotPointer);
        }
      }
    }
  }

  const std::size_t maxKlasses =
      run.options.runtimeDumpCsPath.empty()
          ? 32
          : (run.options.runtimeDumpLimit == 0
                 ? 256
                 : static_cast<std::size_t>(run.options.runtimeDumpLimit));

  for (const HeadlessObjectRow &row : run.objects) {
    if (er2::IsCanonicalUserPtr(row.klass)) {
      std::ostringstream source;
      source << "MSID native=" << er2::HexPtr(row.info.native)
             << " instanceId=" << row.info.instanceId
             << " object=" << row.info.objectName
             << " type=" << row.info.typeFullName;
      addKlass(row.klass, source.str(), row.managed);
    }

    if (klassSources.size() >= maxKlasses) {
      break;
    }
  }

  for (const er2::ManagerRvaCandidate &candidate :
       run.managerScan.candidates) {
    if (er2::IsCanonicalUserPtr(candidate.slotPointer)) {
      std::ostringstream source;
      source << "manager-scan name=" << candidate.name
             << " rva=" << er2::HexPtr(candidate.rva)
             << " slotVa=" << er2::HexPtr(candidate.slotVa)
             << " score=" << candidate.score;
      addKlass(candidate.slotPointer, source.str(), candidate.slotPointer);
    }
    if (klassSources.size() >= maxKlasses) {
      break;
    }
  }

  for (const auto &entry : klassSources) {
    std::vector<std::uintptr_t> instances;
    const auto instanceIt = klassInstances.find(entry.first);
    if (instanceIt != klassInstances.end()) {
      instances = instanceIt->second;
    }
    run.klassReports.push_back(
        BuildKlassReport(er2::Mem(), entry.first, *profile, entry.second,
                         instances));
  }

  if (!run.klassReports.empty()) {
    std::ostringstream ss;
    ss << "[INFO] Built " << run.klassReports.size() << " klass reports";
    AddRunLog(run, ss.str());
  }
}

std::string PickRuntimeClassName(const HeadlessKlassReport &klass) {
  if (klass.runtimeValid && !klass.runtime.className.empty() &&
      IsIdentifierLikeRuntimeName(klass.runtime.className)) {
    return klass.runtime.className;
  }
  if (klass.header.valid && !klass.header.bestName.empty() &&
      IsIdentifierLikeRuntimeName(klass.header.bestName)) {
    return klass.header.bestName;
  }
  return "";
}

std::string PickRuntimeNamespace(const HeadlessKlassReport &klass) {
  if (klass.runtimeValid && !klass.runtime.namespaze.empty() &&
      IsIdentifierLikeRuntimeName(klass.runtime.namespaze, true)) {
    return klass.runtime.namespaze;
  }
  if (klass.header.valid && !klass.header.bestNamespace.empty() &&
      IsIdentifierLikeRuntimeName(klass.header.bestNamespace, true)) {
    return klass.header.bestNamespace;
  }
  return "";
}

std::string ExtractRuntimeSourceValue(const std::string &source,
                                      const std::string &key) {
  const std::string needle = key + "=";
  const std::size_t begin = source.find(needle);
  if (begin == std::string::npos) {
    return "";
  }

  const std::size_t valueBegin = begin + needle.size();
  const std::size_t valueEnd = source.find(' ', valueBegin);
  return source.substr(valueBegin, valueEnd == std::string::npos
                                      ? std::string::npos
                                      : valueEnd - valueBegin);
}

bool SplitRuntimeFullName(const std::string &fullName, std::string &namespaze,
                          std::string &className) {
  namespaze.clear();
  className.clear();
  if (!IsIdentifierLikeRuntimeName(fullName, true)) {
    return false;
  }

  if (fullName == "UnityEngine" || fullName == "UnityEngine.UI" ||
      fullName == "System" || fullName == "System.Core" ||
      fullName == "mscorlib") {
    return false;
  }

  const std::size_t dot = fullName.rfind('.');
  if (dot == std::string::npos) {
    if (!IsIdentifierLikeRuntimeName(fullName)) {
      return false;
    }
    className = fullName;
    return true;
  }

  const std::string ns = fullName.substr(0, dot);
  const std::string cn = fullName.substr(dot + 1);
  if (cn.empty() || !IsIdentifierLikeRuntimeName(cn) ||
      !IsIdentifierLikeRuntimeName(ns, true)) {
    return false;
  }

  namespaze = ns;
  className = cn;
  return true;
}

bool TryPickRuntimeSourceTypeName(const HeadlessKlassReport &klass,
                                  std::string &namespaze,
                                  std::string &className) {
  for (const std::string &source : klass.sources) {
    const std::string typeName = ExtractRuntimeSourceValue(source, "type");
    if (!typeName.empty() && SplitRuntimeFullName(typeName, namespaze,
                                                  className)) {
      return true;
    }
  }
  return false;
}

std::string TryPickRuntimeSourceLabel(const HeadlessKlassReport &klass) {
  for (const std::string &source : klass.sources) {
    std::string label = ExtractRuntimeSourceValue(source, "name");
    if (label.rfind("m_", 0) == 0 && label.size() > 2) {
      label = label.substr(2);
    }
    if (!label.empty() && IsIdentifierLikeRuntimeName(label)) {
      return label;
    }
  }
  return "";
}

bool CanExportRuntimeDumpClass(const HeadlessKlassReport &klass) {
  return ExplainRuntimeDumpExportDecision(klass).exportable;
}

std::string BuildAnonymousRuntimeClassName(const HeadlessKlassReport &klass) {
  std::ostringstream name;
  name << "RuntimeClass_" << std::uppercase << std::hex
       << static_cast<unsigned long long>(klass.klass);
  return name.str();
}

er2::RuntimeDumpCsClassRow ToRuntimeDumpClassRow(
    const HeadlessKlassReport &klass, std::uint32_t runtimeLimit) {
  er2::RuntimeDumpCsClassRow row;
  row.namespaze = PickRuntimeNamespace(klass);
  row.className = PickRuntimeClassName(klass);
  if (row.className.empty()) {
    std::string sourceNamespace;
    std::string sourceClassName;
    if (TryPickRuntimeSourceTypeName(klass, sourceNamespace,
                                     sourceClassName)) {
      row.namespaze = sourceNamespace;
      row.className = sourceClassName;
    }
  }
  if (row.className.empty()) {
    row.className = TryPickRuntimeSourceLabel(klass);
  }
  const std::string weakClassName = row.className;
  if (row.className.empty()) {
    row.className = BuildAnonymousRuntimeClassName(klass);
  }
  row.sources = klass.sources;
  if (!weakClassName.empty() && !klass.runtimeValid && !klass.header.valid) {
    row.sources.push_back("weak-name=" + weakClassName);
  }
  row.instances = klass.instances;
  row.klass = klass.klass;
  row.parent = klass.parent;
  row.fieldsPtr = klass.fields;
  row.methodsPtr = klass.methods;
  row.staticFields = klass.staticFields;
  row.fieldCount = klass.fieldCount;
  row.methodCount = klass.methodCount;

  const std::size_t fieldLimit =
      runtimeLimit == 0 ? klass.fieldsSample.size()
                        : (std::min<std::size_t>)(klass.fieldsSample.size(),
                                                  runtimeLimit);
  const std::size_t trustedFieldLimit =
      (std::min)(fieldLimit,
                 CountTrustedRuntimeFieldPrefix(klass.fieldsSample));
  row.fields.reserve(trustedFieldLimit);
  for (std::size_t i = 0; i < trustedFieldLimit; ++i) {
    const HeadlessFieldReport &field = klass.fieldsSample[i];
    er2::RuntimeDumpCsFieldRow outField;
    outField.name = field.name;
    outField.typeName = field.typeName;
    outField.offset = field.offset;
    outField.type = field.type;
    outField.typeData = field.typeData;
    outField.typeMetaBits = field.typeMetaBits;
    outField.typeEnum = field.typeEnum;
    outField.typeKind = field.typeKind;
    outField.isStatic = field.isStatic;
    row.fields.push_back(std::move(outField));
  }

  const std::size_t methodLimit =
      runtimeLimit == 0 ? klass.methodsSample.size()
                        : (std::min<std::size_t>)(klass.methodsSample.size(),
                                                  runtimeLimit);
  row.methods.reserve(methodLimit);
  for (std::size_t i = 0; i < methodLimit; ++i) {
    const HeadlessMethodReport &method = klass.methodsSample[i];
    er2::RuntimeDumpCsMethodRow outMethod;
    outMethod.name = method.name;
    outMethod.returnTypeName = method.returnTypeName;
    outMethod.methodInfo = method.methodInfo;
    outMethod.methodPointer = method.methodPointer;
    outMethod.methodRva = method.methodRva;
    outMethod.invokerMethod = method.invokerMethod;
    outMethod.returnType = method.returnType;
    outMethod.parameters = method.parameters;
    outMethod.token = method.token;
    outMethod.flags = method.flags;
    outMethod.iflags = method.iflags;
    outMethod.slot = method.slot;
    outMethod.parameterCount = method.parameterCount;
    row.methods.push_back(std::move(outMethod));
  }

  return row;
}

void ExportRuntimeDumpCsIfRequested(HeadlessRunResult &run) {
  if (run.options.runtimeDumpCsPath.empty()) {
    return;
  }

  run.runtimeDump.requested = true;
  run.runtimeDump.path = run.options.runtimeDumpCsPath;

  if (er2::g_ctx.runtime != er2::ManagedBackend::Il2Cpp) {
    run.runtimeDump.error = "runtime dump requires IL2CPP";
    AddRunLog(run, "[ERROR] Runtime dump.cs skipped: " +
                       run.runtimeDump.error);
    return;
  }

  std::vector<er2::RuntimeDumpCsClassRow> classes;
  classes.reserve(run.klassReports.size());
  for (const HeadlessKlassReport &klass : run.klassReports) {
    if (!CanExportRuntimeDumpClass(klass)) {
      continue;
    }
    classes.push_back(ToRuntimeDumpClassRow(
        klass, run.options.runtimeDumpLimit));
  }

  if (classes.empty()) {
    run.runtimeDump.error =
        "no runtime Il2CppClass entries with readable names or field tables";
    AddRunLog(run, "[ERROR] Runtime dump.cs skipped: " +
                       run.runtimeDump.error);
    return;
  }

  er2::RuntimeDumpCsBuildStats stats;
  const std::string dump = er2::BuildRuntimeDumpCs(classes, &stats);
  if (!WriteTextFile(run.options.runtimeDumpCsPath, dump)) {
    run.runtimeDump.error = "failed to write runtime dump.cs output";
    AddRunLog(run, "[ERROR] Runtime dump.cs failed: " +
                       run.runtimeDump.error);
    return;
  }

  run.runtimeDump.ok = true;
  run.runtimeDump.classCount = stats.classCount;
  run.runtimeDump.fieldCount = stats.fieldCount;
  run.runtimeDump.methodCount = stats.methodCount;

  std::ostringstream ss;
  ss << "[SUCCESS] Runtime dump.cs written to "
     << run.options.runtimeDumpCsPath
     << " classes=" << run.runtimeDump.classCount
     << " fields=" << run.runtimeDump.fieldCount
     << " methods=" << run.runtimeDump.methodCount;
  AddRunLog(run, ss.str());
}

void ExportMetadataIfRequested(HeadlessRunResult &run) {
  if (run.options.metadataOutPath.empty()) {
    return;
  }

  run.metadataExport.requested = true;
  run.metadataExport.path = run.options.metadataOutPath;

  if (er2::g_ctx.runtime != er2::ManagedBackend::Il2Cpp ||
      !er2::g_ctx.gameAssembly.base) {
    run.metadataExport.error = "GameAssembly module is not available";
    AddRunLog(run, "[ERROR] Metadata export skipped: " +
                       run.metadataExport.error);
    return;
  }

  AddRunLog(run, "[INFO] Exporting global metadata through DMA...");
  std::vector<std::uint8_t> bytes;
  if (!er2::ExportMetadataByScore(er2::Mem(), er2::g_ctx.gameAssembly.base,
                                  0x200000u, 8192, 15.0, false, 0,
                                  0x200000u, bytes)) {
    run.metadataExport.error = "ExportMetadataByScore returned false";
    AddRunLog(run, "[ERROR] Metadata export failed: " +
                       run.metadataExport.error);
    return;
  }

  if (bytes.size() < 8) {
    run.metadataExport.error = "metadata buffer is too small";
    AddRunLog(run, "[ERROR] Metadata export failed: " +
                       run.metadataExport.error);
    return;
  }

  run.metadataExport.bytes = bytes.size();
  run.metadataExport.magic = er2::detail_metadata_header::ReadU32LE(bytes.data());
  run.metadataExport.version =
      er2::detail_metadata_header::ReadU32LE(bytes.data() + 4);

  if (run.metadataExport.magic != 0xFAB11BAFu) {
    run.metadataExport.error = "metadata magic is not FAB11BAF";
    AddRunLog(run, "[ERROR] Metadata export failed: " +
                       run.metadataExport.error);
    return;
  }

  if (!er2::ReadMetadataHeaderFieldsFromBytes(
          bytes, run.metadataExport.header)) {
    run.metadataExport.error = "failed to parse metadata header fields";
    AddRunLog(run, "[ERROR] Metadata export failed: " +
                       run.metadataExport.error);
    return;
  }

  if (!WriteBinaryFile(run.options.metadataOutPath, bytes)) {
    run.metadataExport.error = "failed to write metadata output file";
    AddRunLog(run, "[ERROR] Metadata export failed: " +
                       run.metadataExport.error);
    return;
  }

  run.metadataExport.ok = true;
  std::ostringstream ss;
  ss << "[SUCCESS] Metadata exported: " << run.options.metadataOutPath
     << " bytes=" << run.metadataExport.bytes
     << " version=" << run.metadataExport.version;
  AddRunLog(run, ss.str());
}

int RunHeadless(er2::HeadlessOptions options) {
  HeadlessRunResult run;
  run.options = std::move(options);

  er2::g_logCallback = [&](const std::string &message) {
    run.logs.push_back(message);
    std::printf("%s\n", message.c_str());
  };

  if (run.options.runMode != "dma") {
    run.error = "only --mode dma is supported in headless mode";
    const std::string json = BuildRunJson(run);
    (void)WriteTextFile(run.options.outPath, json);
    return 2;
  }

  std::shared_ptr<MetickAdapter> dma;
  const er2::Il2CppLayoutProfile *profile =
      er2::FindIl2CppLayoutProfileForUnity2019_4("2019.4.41");

  if (InitHeadlessDma(run, dma)) {
    ScanMsidIfRequested(run);
    EnumerateObjectsIfRequested(run, profile);
    ValidateLegacyRvaCandidates(run);
    ExportMetadataIfRequested(run);
    RunMetadataDiagnosticsIfRequested(run, *dma);
    ScanManagerRvasIfRequested(run);
    DumpModuleImagesIfRequested(run, dma);
    RunSelfDumpCsIfRequested(run);
    RunDmaInjectIfRequested(run, dma);
    ProbeNarakaBuffManagerIfRequested(run);
    ProbeNarakaRuntimePropertiesIfRequested(run);
    ProbeNarakaGroundIfRequested(run);
    ProbeNarakaActorContainersIfRequested(run);
    BuildKlassReports(run, profile);
    ExportRuntimeDumpCsIfRequested(run);
    run.ok = true;
  }

  if (!run.ok && run.error.empty()) {
    run.error = "headless probe failed";
  }

  const std::string json = BuildRunJson(run);
  if (!WriteTextFile(run.options.outPath, json)) {
    std::fprintf(stderr, "[ERROR] Failed to write JSON output: %s\n",
                 run.options.outPath.c_str());
    return 3;
  }

  if (!run.options.logPath.empty()) {
    std::ostringstream logText;
    for (const std::string &line : run.logs) {
      logText << line << "\n";
    }
    if (!WriteTextFile(run.options.logPath, logText.str())) {
      std::fprintf(stderr, "[WARN] Failed to write log output: %s\n",
                   run.options.logPath.c_str());
    }
  }

  if (!run.options.offsetHeaderPath.empty()) {
    const std::string header = BuildOffsetHeader(run);
    if (!WriteTextFile(run.options.offsetHeaderPath, header)) {
      std::fprintf(stderr, "[WARN] Failed to write offset header output: %s\n",
                   run.options.offsetHeaderPath.c_str());
    } else {
      std::printf("[INFO] Offset header written to %s\n",
                  run.options.offsetHeaderPath.c_str());
    }
  }

  std::printf("[INFO] JSON written to %s\n", run.options.outPath.c_str());

  // Controlled teardown: release the process-wide MemProcFS session while the
  // process is still in normal execution, so VMMDLL_Close runs before the C++
  // static/global destructor phase. Without this, the MetickAdapter owned by
  // er2::g_ctx (a global) is torn down after main() returns, when VMMDLL and
  // other globals may already be destructing -> access violation on exit.
  if (dma) {
    er2::ResetContext();           // drops g_ctx.memory ref; runs ~MetickAdapter
    dma.reset();                   // drops the local ref (no double shutdown)
    std::printf("[INFO] DMA session closed cleanly\n");
  }

  return run.ok ? 0 : 1;
}

} // namespace

int main(int argc, char **argv) {
  const er2::HeadlessParseResult parsed = er2::ParseHeadlessArgs(argc, argv);
  if (!parsed.ok) {
    std::fprintf(stderr, "[ERROR] %s\n", parsed.error.c_str());
    PrintUsage();
    return 2;
  }

  if (parsed.options.help) {
    PrintUsage();
    return 0;
  }

  if (parsed.options.headless) {
    return RunHeadless(parsed.options);
  }

  std::printf("[*] Waiting for Unity process...\n");
  while (!er2::AutoInit()) {
    std::this_thread::sleep_for(std::chrono::milliseconds(1000));
    std::printf(".");
  }
  std::printf("\n[+] Unity Process Found & Initialized!\n");

  std::printf("Runtime: %s\n",
              er2::Runtime() == er2::ManagedBackend::Il2Cpp ? "IL2CPP"
                                                            : "Mono");

  while (true) {
    const std::uintptr_t cam = er2::FindMainCamera();
    if (cam) {
      auto viewProjOpt = er2::GetCameraMatrix(cam);
      if (viewProjOpt.has_value()) {
      }
    }

    auto players = er2::GetGameObjectByName("Player");
    if (!players.empty()) {
      std::printf("Player Found: 0x%llX (total: %zu)\n",
                  static_cast<unsigned long long>(players[0]),
                  players.size());
    }

    std::this_thread::sleep_for(std::chrono::milliseconds(1000));
  }

  return 0;
}
