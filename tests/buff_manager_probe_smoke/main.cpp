#include "er2/unity2/naraka/buff_manager_probe.hpp"

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
  std::printf("[buff_manager_probe] FAIL: %s\n", message);
  return false;
}

} // namespace

int main() {
  FakeMemory mem;

  const std::uintptr_t gameAssembly = 0x7FFF00000000ull;
  const std::uintptr_t buffKlass = 0x18001000000ull;
  const std::uintptr_t staticFields = 0x18002000000ull;
  const std::uintptr_t buffManager = 0x18003000000ull;
  const std::uintptr_t dictionary = 0x18004000000ull;
  const std::uintptr_t entries = 0x18005000000ull;
  const std::uintptr_t buffObject = 0x18006000000ull;

  mem.Put(gameAssembly + er2::DefaultNarakaManagerRvas().buffManager,
          buffKlass);
  mem.Put(buffKlass + 0xB8, staticFields);
  mem.Put(staticFields + 0x0, buffManager);
  mem.Put(buffManager, buffKlass);
  mem.Put(buffManager + 0x28, dictionary);
  mem.Put(dictionary + 0x18, entries);
  mem.Put(dictionary + 0x20, std::int32_t{1});
  mem.Put(entries + 0x18, std::int32_t{4});
  mem.Put(entries + 0x20 + 0x00, std::int32_t{0});
  mem.Put(entries + 0x20 + 0x08, std::int32_t{11001006});
  mem.Put(entries + 0x20 + 0x10, buffObject);
  mem.Put(buffObject + 0x0, 0x18007000000ull);
  mem.Put(buffObject + 0x10, std::int32_t{26001});
  mem.Put(buffObject + 0x14, std::int32_t{1000006});
  mem.Put(buffObject + 0x18, std::int32_t{30});

  er2::NarakaBuffManagerSnapshot baseline;
  const bool baselineOk = er2::CaptureNarakaBuffManagerSnapshot(
      mem, gameAssembly, {}, 16, baseline);

  mem.Put(dictionary + 0x20, std::int32_t{2});
  mem.Put(entries + 0x18, std::int32_t{4});
  mem.Put(buffObject + 0x18, std::int32_t{25});
  mem.Put(entries + 0x20 + 0x18 + 0x00, std::int32_t{0});
  mem.Put(entries + 0x20 + 0x18 + 0x08, std::int32_t{11001007});
  mem.Put(entries + 0x20 + 0x18 + 0x10, buffObject + 0x1000);
  mem.Put(buffObject + 0x1000 + 0x0, 0x18007100000ull);
  mem.Put(buffObject + 0x1000 + 0x10, std::int32_t{26002});
  mem.Put(buffObject + 0x1000 + 0x14, std::int32_t{1000006});
  mem.Put(buffObject + 0x1000 + 0x18, std::int32_t{18});

  er2::NarakaBuffManagerSnapshot current;
  const bool currentOk = er2::CaptureNarakaBuffManagerSnapshot(
      mem, gameAssembly, {}, 16, current);
  const auto diff = er2::DiffNarakaBuffManagerSnapshots(baseline, current);

  bool ok = true;
  ok &= Expect(baselineOk, "baseline capture should succeed");
  ok &= Expect(currentOk, "current capture should succeed");
  ok &= Expect(baseline.buffManagerClass == buffKlass,
               "BuffManager class should resolve from GameAssembly RVA");
  ok &= Expect(baseline.staticFields == staticFields,
               "static fields should be read from BuffManager class");
  ok &= Expect(baseline.instance == buffManager,
               "_instance should be read from static fields");
  ok &= Expect(!baseline.containers.empty(),
               "dictionary container should be discovered");
  ok &= Expect(baseline.entries.size() == 1,
               "baseline should contain one buff-like entry");
  ok &= Expect(baseline.entries[0].valueKlass == 0x18007000000ull,
               "entry should preserve managed object klass pointer");
  ok &= Expect(baseline.entries[0].int0 == 26001,
               "entry field sampling should skip the IL2CPP object header");
  ok &= Expect(current.entries.size() == 2,
               "current should contain two buff-like entries");
  ok &= Expect(diff.size() == 2, "diff should include current entries");
  ok &= Expect(diff[0].changed, "changed remaining value should be detected");
  ok &= Expect(diff[1].added, "new buff-like entry should be marked added");

  if (!ok) {
    return 1;
  }
  std::printf("[buff_manager_probe] PASS\n");
  return 0;
}
