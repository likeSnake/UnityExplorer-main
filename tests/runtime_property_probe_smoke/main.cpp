#include "er2/unity2/naraka/runtime_property_probe.hpp"

#include <cstdint>
#include <cstdio>
#include <cstring>
#include <unordered_map>
#include <vector>

namespace {

class FakeMemory final : public er2::IMemoryAccessor {
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

private:
  mutable std::unordered_map<std::uintptr_t, std::uint8_t> bytes_;
};

bool Expect(bool condition, const char *message) {
  if (condition) {
    return true;
  }
  std::printf("[runtime_property_probe] FAIL: %s\n", message);
  return false;
}

} // namespace

int main() {
  FakeMemory mem;

  const std::uintptr_t actor = 0x100000;
  const std::uintptr_t propertyData = 0x101000;
  const std::uintptr_t runtimeProperty = 0x102000;
  const std::uintptr_t propertyRoot = 0x103000;
  const std::uintptr_t idHolder = 0x104000;
  const std::uintptr_t idEntries = 0x105000;
  const std::uintptr_t valuesHolder = 0x106000;
  const std::uintptr_t values = 0x107000;
  const std::uintptr_t characterManagerClass = 0x108000;
  const std::uintptr_t characterManagerStatic = 0x108800;
  const std::uintptr_t characterManager = 0x108C00;
  const std::uintptr_t aliveList = 0x109000;
  const std::uintptr_t actorArray = 0x10A000;
  const std::uintptr_t gameAssembly = 0x200000;

  mem.Put(actor + 0x348, propertyData);
  mem.Put(actor + 0xA8, runtimeProperty);
  mem.Put(propertyData + 0x108, std::uint32_t{1000006});
  mem.Put(propertyData + 0x118, std::uint32_t{2});
  mem.Put(runtimeProperty + 0x10, propertyRoot);
  mem.Put(propertyRoot + 0x10, idHolder);
  mem.Put(idHolder + 0x18, idEntries);
  mem.Put(idEntries + 0x18, std::int32_t{2});
  mem.Put(idEntries + 0x28, std::int32_t{11002});
  mem.Put(idEntries + 0x2C, std::int32_t{3});
  mem.Put(idEntries + 0x38, std::int32_t{999001});
  mem.Put(idEntries + 0x3C, std::int32_t{7});
  mem.Put(propertyRoot + 0x28, valuesHolder);
  mem.Put(valuesHolder + 0x10, values);
  mem.Put(values + 0x20 + 3 * 4, std::int32_t{5800});
  mem.Put(values + 0x20 + 7 * 4, std::int32_t{0});
  mem.Put(gameAssembly + er2::DefaultNarakaManagerRvas().characterManager,
          characterManagerClass);
  mem.Put(characterManagerClass + 0xB8, characterManagerStatic);
  mem.Put(characterManagerStatic + 0x8, characterManager);
  mem.Put(characterManager + 0x58, aliveList);
  mem.Put(aliveList + 0x18, actorArray);
  mem.Put(actorArray + 0x18, std::int32_t{1});
  mem.Put(actorArray + 0x30, actor);

  er2::NarakaRuntimePropertySnapshot baseline;
  const bool baselineOk =
      er2::CaptureNarakaRuntimeProperties(mem, actor, {}, 16, baseline);

  mem.Put(values + 0x20 + 7 * 4, std::int32_t{1});
  er2::NarakaRuntimePropertySnapshot current;
  const bool currentOk =
      er2::CaptureNarakaRuntimeProperties(mem, actor, {}, 16, current);
  const auto diff = er2::DiffNarakaRuntimePropertySnapshots(baseline, current);

  bool ok = true;
  ok &= Expect(baselineOk, "baseline capture should succeed");
  ok &= Expect(currentOk, "current capture should succeed");
  ok &= Expect(baseline.heroId == 1000006, "hero id should be read");
  ok &= Expect(baseline.teamId == 2, "team id should be read");
  ok &= Expect(baseline.rows.size() == 2, "two runtime properties expected");
  ok &= Expect(baseline.rows[0].dataId == 11002, "first DataID should match");
  ok &= Expect(baseline.rows[0].index == 3, "first index should match");
  ok &= Expect(baseline.rows[0].intValue == 5800, "first value should match");
  ok &= Expect(diff.size() == 2, "diff should keep both rows");
  ok &= Expect(diff[1].changed, "changed flag should be set");
  ok &= Expect(diff[1].currentInt == 1, "current value should update");
  ok &= Expect(diff[1].intDelta == 1, "int delta should be one");

  baseline.rows.clear();
  current.rows.clear();
  er2::NarakaRuntimePropertyRow baseRowA;
  baseRowA.dataId = 42;
  baseRowA.index = 1;
  baseRowA.intValue = 10;
  baseline.rows.push_back(baseRowA);
  er2::NarakaRuntimePropertyRow baseRowB;
  baseRowB.dataId = 42;
  baseRowB.index = 2;
  baseRowB.intValue = 20;
  baseline.rows.push_back(baseRowB);
  er2::NarakaRuntimePropertyRow currentRowA = baseRowA;
  currentRowA.intValue = 10;
  current.rows.push_back(currentRowA);
  er2::NarakaRuntimePropertyRow currentRowB = baseRowB;
  currentRowB.intValue = 21;
  current.rows.push_back(currentRowB);
  const auto multiIndexDiff =
      er2::DiffNarakaRuntimePropertySnapshots(baseline, current);
  ok &= Expect(multiIndexDiff.size() == 2,
               "diff should keep duplicate DataID rows with different index");
  ok &= Expect(!multiIndexDiff[0].changed,
               "diff should match duplicate DataID row by index");
  ok &= Expect(multiIndexDiff[1].changed,
               "diff should flag changed duplicate DataID index");
  ok &= Expect(er2::DefaultNarakaManagerRvas().characterManager == 0xe1406f0ull,
               "Naraka probe CharacterManager RVA should match live DMA "
               "confirmed Naraka build (GameAssembly.dll Default)");

  std::uintptr_t resolvedClass = 0;
  std::uintptr_t resolvedStatic = 0;
  std::uintptr_t resolvedInstance = 0;
  const bool resolveOk = er2::ResolveNarakaCharacterManagerInstance(
      mem, gameAssembly, er2::DefaultNarakaManagerRvas(), {},
      resolvedClass, resolvedStatic, resolvedInstance);
  ok &= Expect(resolveOk, "CharacterManager TypeInfo chain should resolve");
  ok &= Expect(resolvedClass == characterManagerClass,
               "CharacterManager class pointer should match");
  ok &= Expect(resolvedStatic == characterManagerStatic,
               "CharacterManager static fields pointer should match");
  ok &= Expect(resolvedInstance == characterManager,
               "CharacterManager instance pointer should match");

  std::vector<std::uintptr_t> actors;
  std::string actorError;
  const bool actorsOk = er2::EnumerateNarakaActorModelsFromCharacterManager(
      mem, characterManager, {}, 16, actors, &actorError);
  ok &= Expect(actorsOk, "alive ActorModel enumeration should succeed");
  ok &= Expect(actors.size() == 1, "one alive ActorModel should be enumerated");
  ok &= Expect(!actors.empty() && actors[0] == actor,
               "enumerated ActorModel pointer should match array entry");

  if (!ok) {
    return 1;
  }
  std::printf("[runtime_property_probe] PASS\n");
  return 0;
}
