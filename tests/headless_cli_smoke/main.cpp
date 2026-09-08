#include "er2/unity2/headless/probe_cli.hpp"
#include "er2/unity2/headless/runtime_dump_cs.hpp"
#include "er2/unity2/headless/runtime_type_resolver.hpp"

#include <cstdint>
#include <cstdio>
#include <cstring>
#include <string>
#include <unordered_map>
#include <vector>

namespace {

bool Expect(bool condition, const char *message) {
  if (condition) {
    return true;
  }

  std::printf("[headless_cli] FAIL: %s\n", message);
  return false;
}

class FakeMemoryAccessor final : public er2::IMemoryAccessor {
public:
  bool Read(std::uintptr_t address, void *buffer,
            std::size_t size) const override {
    auto *out = static_cast<std::uint8_t *>(buffer);
    for (std::size_t i = 0; i < size; ++i) {
      const auto it = bytes_.find(address + i);
      if (it == bytes_.end()) {
        return false;
      }
      out[i] = it->second;
    }
    return true;
  }

  bool Write(std::uintptr_t address, const void *buffer,
             std::size_t size) const override {
    const auto *in = static_cast<const std::uint8_t *>(buffer);
    for (std::size_t i = 0; i < size; ++i) {
      bytes_[address + i] = in[i];
    }
    return true;
  }

  template <typename T> void Put(std::uintptr_t address, const T &value) {
    (void)Write(address, &value, sizeof(value));
  }

  void PutBytes(std::uintptr_t address, std::size_t size) {
    std::vector<std::uint8_t> zero(size);
    (void)Write(address, zero.data(), zero.size());
  }

  void PutCString(std::uintptr_t address, const char *value) {
    (void)Write(address, value, std::strlen(value) + 1);
  }

private:
  mutable std::unordered_map<std::uintptr_t, std::uint8_t> bytes_;
};

std::uint32_t RuntimeTypeBits(std::uint32_t typeEnum) {
  return typeEnum << 16;
}

} // namespace

int main() {
  bool ok = true;

  const char *argv[] = {"ExternalResolveConsole.exe",
                        "--headless",
                        "--target",
                        "NarakaBladepoint.exe",
                        "--mode",
                        "dma",
                        "--scan",
                        "gameobjects",
                        "--filter",
                        "Player",
                        "--limit",
                        "32",
                        "--klass",
                        "0x256DCD03FF0",
                        "--scan-managers",
                        "--manager-scan-mb",
                        "64",
                        "--out",
                        "probe.json",
                        "--log",
                        "probe.log",
                        "--offset-header",
                        "OffsetRuntime.h",
                        "--runtime-dump-cs",
                        "dump.cs",
                        "--runtime-dump-limit",
                        "16",
                        "--runtime-dump-rvas",
                        "0x35E25C0,0x35F4240",
                        "--metadata-out",
                        "global-metadata.dat",
                        "--metadata-diagnose",
                        "--metadata-diagnose-mb",
                        "512",
                        "--metadata-probe-va",
                        "0x226A19A0000",
                        "--metadata-diagnose-anchor",
                        "0x2280C38C5F0",
                        "--metadata-diagnose-heuristic",
                        "--naraka-buff-probe",
                        "--naraka-buff-max-entries",
                        "128",
                        "--naraka-buff-scan-end",
                        "0x800",
                        "--naraka-runtime-property-probe",
                        "--naraka-runtime-property-max-rows",
                        "2048",
                        "--naraka-runtime-property-samples",
                        "3",
                        "--naraka-runtime-property-interval-ms",
                        "250",
                        "--naraka-runtime-property-csv",
                        "runtime_property.csv",
                        "--naraka-actor-container-probe",
                        "--naraka-actor",
                        "0x24200010000",
                        "--naraka-actor-max-entries",
                        "256",
                        "--naraka-actor-scan-end",
                        "0x1200",
                        "--naraka-actor-any-hero"};
  auto parsed =
      er2::ParseHeadlessArgs(static_cast<int>(sizeof(argv) / sizeof(argv[0])),
                             const_cast<char **>(argv));

  ok &= Expect(parsed.ok, "parser should accept full headless command");
  ok &= Expect(parsed.options.headless, "--headless should be true");
  ok &= Expect(parsed.options.targetProcess == "NarakaBladepoint.exe",
               "target should be parsed");
  ok &= Expect(parsed.options.runMode == "dma", "mode should be parsed");
  ok &= Expect(parsed.options.scanMode == er2::HeadlessScanMode::GameObjects,
               "scan mode should be gameobjects");
  ok &= Expect(parsed.options.filter == "Player", "filter should be parsed");
  ok &= Expect(parsed.options.limit == 32, "limit should be parsed");
  ok &= Expect(parsed.options.klass == 0x256DCD03FF0ull,
               "klass should parse as hex");
  ok &= Expect(parsed.options.scanManagers,
               "scan managers should be enabled");
  ok &= Expect(parsed.options.managerScanMaxMb == 64,
               "manager scan MB should be parsed");
  ok &= Expect(parsed.options.outPath == "probe.json",
               "output path should be parsed");
  ok &= Expect(parsed.options.logPath == "probe.log",
               "log path should be parsed");
  ok &= Expect(parsed.options.offsetHeaderPath == "OffsetRuntime.h",
               "offset header path should be parsed");
  ok &= Expect(parsed.options.runtimeDumpCsPath == "dump.cs",
               "runtime dump.cs path should be parsed");
  ok &= Expect(parsed.options.runtimeDumpLimit == 16,
               "runtime dump limit should be parsed");
  ok &= Expect(parsed.options.runtimeDumpRvas.size() == 2,
               "runtime dump RVA list should be parsed");
  ok &= Expect(parsed.options.runtimeDumpRvas[0] == 0x35E25C0ull,
               "runtime dump first RVA should parse as hex");
  ok &= Expect(parsed.options.runtimeDumpRvas[1] == 0x35F4240ull,
               "runtime dump second RVA should parse as hex");
  ok &= Expect(parsed.options.metadataOutPath == "global-metadata.dat",
               "metadata output path should be parsed");
  ok &= Expect(parsed.options.metadataDiagnose,
               "metadata diagnostics should be enabled");
  ok &= Expect(parsed.options.metadataDiagnoseMaxMb == 512,
               "metadata diagnostics MB should be parsed");
  ok &= Expect(parsed.options.metadataProbeVa == 0x226A19A0000ull,
               "metadata probe VA should parse as hex");
  ok &= Expect(parsed.options.metadataDiagnoseAnchor == 0x2280C38C5F0ull,
               "metadata diagnostics anchor should parse as hex");
  ok &= Expect(parsed.options.metadataDiagnoseHeuristic,
               "metadata diagnostics heuristic should be enabled");
  ok &= Expect(parsed.options.narakaBuffProbe,
               "Naraka BuffManager probe should be enabled");
  ok &= Expect(parsed.options.narakaBuffMaxEntries == 128,
               "Naraka BuffManager max entries should be parsed");
  ok &= Expect(parsed.options.narakaBuffScanEnd == 0x800,
               "Naraka BuffManager scan end should be parsed");
  ok &= Expect(parsed.options.narakaRuntimePropertyProbe,
               "Naraka RuntimeProperty probe should be enabled");
  ok &= Expect(parsed.options.narakaRuntimePropertyMaxRows == 2048,
               "Naraka RuntimeProperty max rows should be parsed");
  ok &= Expect(parsed.options.narakaRuntimePropertySamples == 3,
               "Naraka RuntimeProperty sample count should be parsed");
  ok &= Expect(parsed.options.narakaRuntimePropertyIntervalMs == 250,
               "Naraka RuntimeProperty interval should be parsed");
  ok &= Expect(parsed.options.narakaRuntimePropertyCsvPath ==
                   "runtime_property.csv",
               "Naraka RuntimeProperty CSV path should be parsed");
  ok &= Expect(parsed.options.narakaActorContainerProbe,
               "Naraka Actor container probe should be enabled");
  ok &= Expect(parsed.options.narakaActorAddress == 0x24200010000ull,
               "Naraka Actor address should parse as hex");
  ok &= Expect(parsed.options.narakaActorMaxEntries == 256,
               "Naraka Actor max entries should be parsed");
  ok &= Expect(parsed.options.narakaActorScanEnd == 0x1200,
               "Naraka Actor scan end should be parsed");
  ok &= Expect(!parsed.options.narakaActorOnlyTarka,
               "Naraka Actor any hero flag should disable Tarka filtering");

  const char *badArgv[] = {"ExternalResolveConsole.exe", "--headless",
                           "--scan", "bad"};
  auto bad = er2::ParseHeadlessArgs(
      static_cast<int>(sizeof(badArgv) / sizeof(badArgv[0])),
      const_cast<char **>(badArgv));
  ok &= Expect(!bad.ok, "invalid scan value should fail");

  ok &= Expect(er2::HeadlessJsonString("a\"b\\c\n") == "\"a\\\"b\\\\c\\n\"",
               "JSON string should escape quotes, slash and newline");
  ok &= Expect(er2::HexPtr(0x7ffe09d80000ull) == "0x7FFE09D80000",
               "HexPtr should format uppercase pointer");
  ok &= Expect(er2::RuntimeDumpRvaLabel(0x35E25C0ull) ==
                   "m_CharacterManager",
               "runtime dump should label confirmed character manager RVA");
  ok &= Expect(er2::RuntimeDumpRvaLabel(0x35F4240ull) ==
                   "m_EntityManager",
               "runtime dump should label confirmed entity manager RVA");
  ok &= Expect(er2::RuntimeDumpRvaLabel(0x359CF28ull) ==
                   "m_GlobalTime_BestCandidate",
               "runtime dump should label best-candidate global time RVA");
  ok &= Expect(er2::RuntimeDumpRvaLabel(0x35E1250ull) == "m_BuffManager",
               "runtime dump should label confirmed BuffManager RVA");
  ok &= Expect(er2::RuntimeDumpRvaLabel(0x1234ull).empty(),
               "runtime dump should leave unknown RVA labels empty");

  FakeMemoryAccessor fakeMem;
  const er2::Il2CppLayoutProfile *profile =
      er2::FindIl2CppLayoutProfileForMetadataVersion(24);
  ok &= Expect(profile != nullptr, "runtime type resolver needs IL2CPP profile");

  constexpr std::uintptr_t intType = 0x100000ull;
  fakeMem.Put<std::uintptr_t>(intType + 0x00, 0);
  fakeMem.Put<std::uint32_t>(intType + 0x08, RuntimeTypeBits(0x08));
  er2::RuntimeIl2CppTypeDescription intDesc;
  ok &= Expect(er2::DescribeRuntimeIl2CppType(fakeMem, *profile, intType,
                                              intDesc),
               "runtime type resolver should describe primitive type");
  ok &= Expect(intDesc.name == "System.Int32",
               "runtime type resolver should decode primitive Int32");

  constexpr std::uintptr_t className = 0x110000ull;
  constexpr std::uintptr_t classNamespace = 0x110100ull;
  constexpr std::uintptr_t classKlass = 0x120000ull;
  constexpr std::uintptr_t classType = 0x130000ull;
  fakeMem.PutCString(className, "PlayerState");
  fakeMem.PutCString(classNamespace, "Naraka.Game");
  fakeMem.PutBytes(classKlass, 0x130);
  fakeMem.Put<std::uintptr_t>(classKlass + profile->klass.name, className);
  fakeMem.Put<std::uintptr_t>(classKlass + profile->klass.namespaze,
                              classNamespace);
  fakeMem.Put<std::uintptr_t>(classType + 0x00, classKlass);
  fakeMem.Put<std::uint32_t>(classType + 0x08, RuntimeTypeBits(0x12));
  er2::RuntimeIl2CppTypeDescription classDesc;
  ok &= Expect(er2::DescribeRuntimeIl2CppType(fakeMem, *profile, classType,
                                              classDesc),
               "runtime type resolver should describe class type");
  ok &= Expect(classDesc.name == "Naraka.Game.PlayerState",
               "runtime type resolver should decode class name");

  constexpr std::uintptr_t shiftedClassName = 0x130800ull;
  constexpr std::uintptr_t shiftedClassNamespace = 0x130900ull;
  constexpr std::uintptr_t shiftedKlass = 0x131000ull;
  constexpr std::uintptr_t shiftedType = 0x132000ull;
  fakeMem.PutCString(shiftedClassName, "ShiftedPlayer");
  fakeMem.PutCString(shiftedClassNamespace, "Naraka.Runtime");
  fakeMem.PutBytes(shiftedKlass, 0x130);
  fakeMem.Put<std::uintptr_t>(shiftedKlass + 0x30, shiftedClassName);
  fakeMem.Put<std::uintptr_t>(shiftedKlass + 0x38, shiftedClassNamespace);
  fakeMem.Put<std::uintptr_t>(shiftedType + 0x00, shiftedKlass);
  fakeMem.Put<std::uint32_t>(shiftedType + 0x08, RuntimeTypeBits(0x12));
  er2::RuntimeIl2CppTypeDescription shiftedDesc;
  ok &= Expect(er2::DescribeRuntimeIl2CppType(fakeMem, *profile, shiftedType,
                                              shiftedDesc),
               "runtime type resolver should recover header-probed class type");
  ok &= Expect(shiftedDesc.name == "Naraka.Runtime.ShiftedPlayer",
               "runtime type resolver should use class header fallback");

  constexpr std::uintptr_t szArrayType = 0x140000ull;
  fakeMem.Put<std::uintptr_t>(szArrayType + 0x00, intType);
  fakeMem.Put<std::uint32_t>(szArrayType + 0x08, RuntimeTypeBits(0x1D));
  er2::RuntimeIl2CppTypeDescription szArrayDesc;
  ok &= Expect(er2::DescribeRuntimeIl2CppType(fakeMem, *profile, szArrayType,
                                              szArrayDesc),
               "runtime type resolver should describe SZARRAY type");
  ok &= Expect(szArrayDesc.name == "System.Int32[]",
               "runtime type resolver should decode SZARRAY element");

  constexpr std::uintptr_t arrayInfo = 0x150000ull;
  constexpr std::uintptr_t arrayType = 0x150100ull;
  fakeMem.PutBytes(arrayInfo, 0x10);
  fakeMem.Put<std::uintptr_t>(arrayInfo + 0x00, intType);
  fakeMem.Put<std::uint8_t>(arrayInfo + 0x08, 2);
  fakeMem.Put<std::uintptr_t>(arrayType + 0x00, arrayInfo);
  fakeMem.Put<std::uint32_t>(arrayType + 0x08, RuntimeTypeBits(0x14));
  er2::RuntimeIl2CppTypeDescription arrayDesc;
  ok &= Expect(er2::DescribeRuntimeIl2CppType(fakeMem, *profile, arrayType,
                                              arrayDesc),
               "runtime type resolver should describe ARRAY type");
  ok &= Expect(arrayDesc.name == "System.Int32[,]",
               "runtime type resolver should decode ARRAY rank");

  constexpr std::uintptr_t listName = 0x151000ull;
  constexpr std::uintptr_t listNamespace = 0x151100ull;
  constexpr std::uintptr_t listKlass = 0x151200ull;
  constexpr std::uintptr_t listBaseType = 0x151300ull;
  constexpr std::uintptr_t genericClass = 0x151400ull;
  constexpr std::uintptr_t genericInst = 0x151500ull;
  constexpr std::uintptr_t genericArgv = 0x151600ull;
  constexpr std::uintptr_t genericType = 0x151700ull;
  fakeMem.PutCString(listName, "List`1");
  fakeMem.PutCString(listNamespace, "System.Collections.Generic");
  fakeMem.PutBytes(listKlass, 0x130);
  fakeMem.Put<std::uintptr_t>(listKlass + profile->klass.name, listName);
  fakeMem.Put<std::uintptr_t>(listKlass + profile->klass.namespaze,
                              listNamespace);
  fakeMem.Put<std::uintptr_t>(listBaseType + 0x00, listKlass);
  fakeMem.Put<std::uint32_t>(listBaseType + 0x08, RuntimeTypeBits(0x12));
  fakeMem.Put<std::uintptr_t>(genericClass + 0x00, listKlass);
  fakeMem.Put<std::uintptr_t>(genericClass + 0x08, genericInst);
  fakeMem.Put<std::uint32_t>(genericInst + 0x00, 1);
  fakeMem.Put<std::uintptr_t>(genericInst + 0x08, genericArgv);
  fakeMem.Put<std::uintptr_t>(genericArgv + 0x00, intType);
  fakeMem.Put<std::uintptr_t>(genericType + 0x00, genericClass);
  fakeMem.Put<std::uint32_t>(genericType + 0x08, RuntimeTypeBits(0x15));
  er2::RuntimeIl2CppTypeDescription genericDesc;
  ok &= Expect(er2::DescribeRuntimeIl2CppType(fakeMem, *profile, genericType,
                                              genericDesc),
               "runtime type resolver should describe generic class layout");
  ok &= Expect(genericDesc.name ==
                   "System.Collections.Generic.List`1<System.Int32>",
               "runtime type resolver should decode generic class args");
  constexpr std::uintptr_t cachedGenericClass = 0x151800ull;
  constexpr std::uintptr_t cachedGenericType = 0x151900ull;
  fakeMem.Put<std::uintptr_t>(cachedGenericClass + 0x00, 0x44ull);
  fakeMem.Put<std::uintptr_t>(cachedGenericClass + 0x08, genericInst);
  fakeMem.Put<std::uintptr_t>(cachedGenericClass + 0x10, 0);
  fakeMem.Put<std::uintptr_t>(cachedGenericClass + 0x18, listKlass);
  fakeMem.Put<std::uintptr_t>(cachedGenericType + 0x00, cachedGenericClass);
  fakeMem.Put<std::uint32_t>(cachedGenericType + 0x08, RuntimeTypeBits(0x15));
  er2::RuntimeIl2CppTypeDescription cachedGenericDesc;
  ok &= Expect(er2::DescribeRuntimeIl2CppType(fakeMem, *profile,
                                              cachedGenericType,
                                              cachedGenericDesc),
               "runtime type resolver should describe cached generic class layout");
  ok &= Expect(cachedGenericDesc.name ==
                   "System.Collections.Generic.List`1<System.Int32>",
               "runtime type resolver should decode cached generic class args");
  std::vector<std::uintptr_t> genericWords;
  ok &= Expect(er2::SampleRuntimePointerWords(fakeMem, genericClass,
                                              genericWords, 2),
               "runtime diagnostics should sample pointer words");
  ok &= Expect(genericWords.size() == 2,
               "runtime diagnostics should keep requested word count");
  ok &= Expect(genericWords[0] == listKlass && genericWords[1] == genericInst,
               "runtime diagnostics should preserve sampled word values");

  er2::RuntimeDumpCsClassRow dumpClass;
  dumpClass.namespaze = "Naraka.Game";
  dumpClass.className = "PlayerState";
  dumpClass.sources.push_back("--runtime-dump-rvas rva=0x35E25C0 slotVa=0x1000");
  dumpClass.instances.push_back(0x12345678ull);
  dumpClass.klass = 0x12340000ull;
  dumpClass.fieldCount = 1;
  dumpClass.methodCount = 1;
  dumpClass.fields.push_back({"m_HP", "System.Int32", 0x20, 0x55550000ull,
                              0x66650000ull, RuntimeTypeBits(0x08), 0x08,
                              "I4", false});
  dumpClass.methods.push_back({"Update", "System.Void", 0x66660000ull,
                               0x77770000ull, 0x1234ull, 0x88880000ull,
                               0x99990000ull, 0xAAAA0000ull, 0x06000001u,
                               0x0006u, 0x0000u, 7, 0});

  er2::RuntimeDumpCsBuildStats dumpStats;
  const std::string dumpText =
      er2::BuildRuntimeDumpCs({dumpClass}, &dumpStats);
  ok &= Expect(dumpStats.classCount == 1, "runtime dump should count classes");
  ok &= Expect(dumpStats.fieldCount == 1, "runtime dump should count fields");
  ok &= Expect(dumpStats.methodCount == 1, "runtime dump should count methods");
  ok &= Expect(dumpText.find("// Class: PlayerState") != std::string::npos,
               "runtime dump should emit standard class header");
  ok &= Expect(dumpText.find("// Namespace: Naraka.Game") != std::string::npos,
               "runtime dump should emit standard namespace header");
  ok &= Expect(dumpText.find("// Instance: 0x12345678") != std::string::npos,
               "runtime dump should emit runtime instance evidence");
  ok &= Expect(dumpText.find("// Source: --runtime-dump-rvas rva=0x35E25C0") !=
                   std::string::npos,
               "runtime dump should emit class source evidence");
  ok &= Expect(dumpText.find("// RuntimeRawFieldCount=1 RuntimeRawMethodCount=1") !=
                   std::string::npos,
               "runtime dump should label raw runtime counters");
  ok &= Expect(dumpText.find("// EmittedFieldCount=1 EmittedMethodCount=1") !=
                   std::string::npos,
               "runtime dump should label emitted member counters");
  ok &= Expect(dumpText.find("\t// Fields") != std::string::npos ||
                   dumpText.find("    // Fields") != std::string::npos,
               "runtime dump should emit standard fields section");
  ok &= Expect(dumpText.find("public int m_HP; // offset:0x20") !=
                   std::string::npos,
               "runtime dump should emit resolved C# primitive field type");
  ok &= Expect(dumpText.find("data=0x66650000") != std::string::npos,
               "runtime dump should emit field type data pointer");
  ok &= Expect(dumpText.find("metaBits=0x80000 typeEnum=0x8 typeKind=I4") !=
                   std::string::npos,
               "runtime dump should emit field type metadata bits");
  ok &= Expect(dumpText.find("typeName=System.Int32") != std::string::npos,
               "runtime dump should emit resolved field type name");
  ok &= Expect(dumpText.find("// Methods:1") != std::string::npos,
               "runtime dump should emit standard methods section");
  ok &= Expect(dumpText.find("// RVA: 0x1234 VA: 0x77770000 Slot: 7") !=
                   std::string::npos,
               "runtime dump should emit standard method address evidence");
  ok &= Expect(dumpText.find("public void Update() { } // MethodInfo=0x66660000") !=
                   std::string::npos,
               "runtime dump should emit method signature stub");
  ok &= Expect(dumpText.find("Token=0x6000001") != std::string::npos,
               "runtime dump should emit method token evidence");
  er2::RuntimeDumpCsClassRow badClass;
  badClass.className = "123 bad-name";
  const std::string sanitizedText = er2::BuildRuntimeDumpCs({badClass});
  ok &= Expect(sanitizedText.find("public class Class_0") !=
                   std::string::npos,
               "runtime dump should sanitize invalid class names");
  er2::RuntimeDumpCsClassRow anonymousClass;
  anonymousClass.className = "RuntimeClass_1234";
  anonymousClass.fields.push_back({"", "", 0x30, 0x44440000ull, 0, 0, 0, "",
                                   false});
  const std::string anonymousText =
      er2::BuildRuntimeDumpCs({anonymousClass});
  ok &= Expect(anonymousText.find("public object field_0; // offset:0x30") !=
                   std::string::npos,
               "runtime dump should keep unresolved fields as object");
  er2::RuntimeDumpCsClassRow duplicateClass;
  duplicateClass.className = "DuplicateFields";
  duplicateClass.fields.push_back(
      {"same-field", "", 0x10, 0x11110000ull, 0, 0, 0, "", false});
  duplicateClass.fields.push_back(
      {"same-field", "", 0x18, 0x22220000ull, 0, 0, 0, "", false});
  const std::string duplicateText =
      er2::BuildRuntimeDumpCs({duplicateClass});
  ok &= Expect(duplicateText.find("public object same_field; // offset:0x10") !=
                   std::string::npos,
               "runtime dump should sanitize duplicate field base name");
  ok &= Expect(duplicateText.find("public object same_field_1; // offset:0x18") !=
                   std::string::npos,
               "runtime dump should make duplicate field names unique");

  if (!ok) {
    return 1;
  }

  std::printf("[headless_cli] PASS\n");
  return 0;
}
