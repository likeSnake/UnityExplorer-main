#include "er2/unity2/inspect/klass_header_probe.hpp"

#include <cstdio>
#include <cstring>
#include <unordered_map>
#include <vector>

class FakeMemory final : public er2::IMemoryAccessor {
public:
  bool Read(std::uintptr_t address, void *buffer,
            std::size_t size) const override {
    for (const auto &region : regions_) {
      const std::uintptr_t base = region.first;
      const std::vector<std::uint8_t> &bytes = region.second;
      if (address >= base && address + size <= base + bytes.size()) {
        std::memcpy(buffer, bytes.data() + (address - base), size);
        return true;
      }
    }
    return false;
  }

  bool Write(std::uintptr_t address, const void *buffer,
             std::size_t size) const override {
    (void)address;
    (void)buffer;
    (void)size;
    return false;
  }

  void Add(std::uintptr_t address, std::vector<std::uint8_t> bytes) {
    regions_[address] = std::move(bytes);
  }

private:
  std::unordered_map<std::uintptr_t, std::vector<std::uint8_t>> regions_;
};

namespace {

template <typename T>
void Put(std::vector<std::uint8_t> &bytes, std::size_t offset, T value) {
  std::memcpy(bytes.data() + offset, &value, sizeof(T));
}

bool Expect(bool condition, const char *message) {
  if (condition) {
    return true;
  }

  std::printf("[klass_header_probe] FAIL: %s\n", message);
  return false;
}

} // namespace

int main() {
  FakeMemory mem;
  const std::uintptr_t klass = 0x100000;
  const std::uintptr_t oldName = 0x200000;
  const std::uintptr_t realName = 0x210000;
  const std::uintptr_t realNs = 0x220000;
  const std::uintptr_t shortNameKlass = 0x300000;
  const std::uintptr_t shortName = 0x310000;
  const std::uintptr_t longNs = 0x320000;

  std::vector<std::uint8_t> klassBytes(0x100);
  Put(klassBytes, 0x10, oldName);
  Put(klassBytes, 0x30, realName);
  Put(klassBytes, 0x38, realNs);
  mem.Add(klass, std::move(klassBytes));

  mem.Add(oldName, std::vector<std::uint8_t>{0});
  mem.Add(realName, std::vector<std::uint8_t>{'P', 'l', 'a', 'y', 'e', 'r',
                                             'C', 't', 'r', 'l', 0});
  mem.Add(realNs, std::vector<std::uint8_t>{'G', 'a', 'm', 'e', 0});

  er2::KlassHeaderProbeResult result;
  const bool probeOk = er2::ProbeKlassHeaderStrings(mem, klass, result);

  bool ok = true;
  ok &= Expect(probeOk, "probe should find readable strings");
  ok &= Expect(result.valid, "result should be valid");
  ok &= Expect(result.bestNameOffset == 0x30,
               "best name offset should prefer readable class-like string");
  ok &= Expect(result.bestNamespaceOffset == 0x38,
               "best namespace offset should be selected");
  ok &= Expect(result.bestName == "PlayerCtrl", "best name should match");
  ok &= Expect(result.bestNamespace == "Game", "best namespace should match");
  ok &= Expect(!result.entries.empty(), "entries should be recorded");

  std::vector<std::uint8_t> shortNameKlassBytes(0x100);
  Put(shortNameKlassBytes, 0x30, shortName);
  Put(shortNameKlassBytes, 0x38, longNs);
  mem.Add(shortNameKlass, std::move(shortNameKlassBytes));

  mem.Add(shortName, std::vector<std::uint8_t>{'A', 'I', 0});
  mem.Add(longNs,
          std::vector<std::uint8_t>{'V', 'e', 'r', 'y', 'L', 'o', 'n',
                                    'g', 'G', 'a', 'm', 'e', 'N', 'a',
                                    'm', 'e', 's', 'p', 'a', 'c', 'e', 0});

  er2::KlassHeaderProbeResult shortNameResult;
  ok &= Expect(er2::ProbeKlassHeaderStrings(mem, shortNameKlass,
                                            shortNameResult),
               "probe should handle short class names");
  ok &= Expect(shortNameResult.bestNameOffset == 0x30,
               "best name should prefer adjacent class slot");
  ok &= Expect(shortNameResult.bestNamespaceOffset == 0x38,
               "best namespace should prefer adjacent namespace slot");
  ok &= Expect(shortNameResult.bestName == "AI",
               "short class name should not lose to namespace length");
  ok &= Expect(shortNameResult.bestNamespace == "VeryLongGameNamespace",
               "long namespace should match");

  er2::KlassHeaderProbeResult invalid;
  ok &= Expect(!er2::ProbeKlassHeaderStrings(mem, 0, invalid),
               "zero klass should fail");

  if (!ok) {
    return 1;
  }

  std::printf("[klass_header_probe] PASS\n");
  return 0;
}
