#include "er2/unity2/naraka/actor_container_probe.hpp"

#include <cstdint>
#include <cstdio>
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
  std::printf("[actor_container_probe] FAIL: %s\n", message);
  return false;
}

} // namespace

int main() {
  FakeMemory mem;

  const std::uintptr_t actor = 0x18001000000ull;
  const std::uintptr_t propertyData = 0x18001100000ull;
  const std::uintptr_t runtimeProperty = 0x18001200000ull;
  const std::uintptr_t list = 0x18002000000ull;
  const std::uintptr_t listItems = 0x18002100000ull;
  const std::uintptr_t dictionary = 0x18003000000ull;
  const std::uintptr_t dictionaryEntries = 0x18003100000ull;
  const std::uintptr_t buffA = 0x18004000000ull;
  const std::uintptr_t buffB = 0x18004100000ull;

  mem.Put(actor + 0x348, propertyData);
  mem.Put(actor + 0xA8, runtimeProperty);
  mem.Put(propertyData + 0x108, std::uint32_t{1000006});
  mem.Put(propertyData + 0x118, std::uint32_t{2});

  mem.Put(actor + 0x400, list);
  mem.Put(list + 0x10, listItems);
  mem.Put(list + 0x18, std::int32_t{1});
  mem.Put(listItems + 0x18, std::int32_t{4});
  mem.Put(listItems + 0x20, buffA);
  mem.Put(buffA + 0x0, 0x18005000000ull);
  mem.Put(buffA + 0x10, std::int32_t{26001});
  mem.Put(buffA + 0x14, std::int32_t{1000006});
  mem.Put(buffA + 0x18, std::int32_t{30});

  mem.Put(runtimeProperty + 0x208, dictionary);
  mem.Put(dictionary + 0x18, dictionaryEntries);
  mem.Put(dictionary + 0x20, std::int32_t{1});
  mem.Put(dictionaryEntries + 0x18, std::int32_t{4});
  mem.Put(dictionaryEntries + 0x20 + 0x00, std::int32_t{123});
  mem.Put(dictionaryEntries + 0x20 + 0x04, std::int32_t{-1});
  mem.Put(dictionaryEntries + 0x20 + 0x08, std::int32_t{11001006});
  mem.Put(dictionaryEntries + 0x20 + 0x10, buffA);

  er2::NarakaActorContainerSnapshot baseline;
  const bool baselineOk = er2::CaptureNarakaActorContainerSnapshot(
      mem, actor, {}, 16, baseline);

  mem.Put(list + 0x18, std::int32_t{2});
  mem.Put(listItems + 0x28, buffB);
  mem.Put(buffA + 0x18, std::int32_t{24});
  mem.Put(buffB + 0x0, 0x18005010000ull);
  mem.Put(buffB + 0x10, std::int32_t{26002});
  mem.Put(buffB + 0x14, std::int32_t{1000006});
  mem.Put(buffB + 0x18, std::int32_t{15});

  er2::NarakaActorContainerSnapshot current;
  const bool currentOk = er2::CaptureNarakaActorContainerSnapshot(
      mem, actor, {}, 16, current);
  const auto containerDiff =
      er2::DiffNarakaActorContainers(baseline, current);
  const auto entryDiff =
      er2::DiffNarakaActorContainerEntries(baseline, current);

  bool ok = true;
  ok &= Expect(baselineOk, "baseline capture should succeed");
  ok &= Expect(currentOk, "current capture should succeed");
  ok &= Expect(baseline.heroId == 1000006, "hero id should be read");
  ok &= Expect(baseline.teamId == 2, "team id should be read");
  ok &= Expect(baseline.containers.size() == 2,
               "two containers should be discovered");
  ok &= Expect(current.entries.size() == 3,
               "current should sample list and dictionary entries");
  ok &= Expect(!containerDiff.empty() && containerDiff[0].changed,
               "container count change should be detected");

  bool sawAdded = false;
  bool sawChanged = false;
  for (const auto &row : entryDiff) {
    sawAdded = sawAdded || row.added;
    sawChanged = sawChanged || (row.changed && !row.added);
  }
  ok &= Expect(sawAdded, "new entry should be marked added");
  ok &= Expect(sawChanged, "field mutation should be marked changed");

  if (!ok) {
    return 1;
  }
  std::printf("[actor_container_probe] PASS\n");
  return 0;
}
