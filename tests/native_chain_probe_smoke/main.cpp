#include "er2/unity2/inspect/native_chain_probe.hpp"

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

  std::printf("[native_chain_probe] FAIL: %s\n", message);
  return false;
}

} // namespace

int main() {
  FakeMemory mem;
  const er2::Il2CppLayoutProfile &profile =
      er2::kIl2CppUnity2019_4_Lts24_5_Profile;

  const std::uintptr_t native = 0x100000;
  const std::uintptr_t managedWrong = 0x200000;
  const std::uintptr_t managed = 0x210000;
  const std::uintptr_t klass = 0x300000;
  const std::uintptr_t className = 0x400000;
  const std::uintptr_t ns = 0x400100;

  std::vector<std::uint8_t> nativeBytes(0x80);
  Put(nativeBytes, 0x28, managedWrong);
  Put(nativeBytes, 0x48, managed);
  mem.Add(native, std::move(nativeBytes));

  std::vector<std::uint8_t> wrongManagedBytes(0x20);
  Put(wrongManagedBytes, 0x00, static_cast<std::uintptr_t>(0x500000));
  Put(wrongManagedBytes, 0x10, static_cast<std::uintptr_t>(0));
  mem.Add(managedWrong, std::move(wrongManagedBytes));

  std::vector<std::uint8_t> managedBytes(0x20);
  Put(managedBytes, 0x00, klass);
  Put(managedBytes, 0x10, native);
  mem.Add(managed, std::move(managedBytes));

  std::vector<std::uint8_t> klassBytes(0x140);
  Put(klassBytes, profile.klass.name, className);
  Put(klassBytes, profile.klass.namespaze, ns);
  Put<std::uint16_t>(klassBytes, profile.klass.fieldCount, 3);
  Put<std::uint16_t>(klassBytes, profile.klass.methodCount, 11);
  mem.Add(klass, std::move(klassBytes));

  mem.Add(className, std::vector<std::uint8_t>{'G', 'a', 'm', 'e', 'O', 'b',
                                               'j', 'e', 'c', 't', 0});
  mem.Add(ns, std::vector<std::uint8_t>{'U', 'n', 'i', 't', 'y', 'E', 'n',
                                        'g', 'i', 'n', 'e', 0});

  er2::NativeChainProbeResult result;
  const bool okProbe = er2::ProbeNativeObjectChain(mem, native, profile, result);

  bool ok = true;
  ok &= Expect(okProbe, "probe should find the valid managed offset");
  ok &= Expect(result.valid, "result should be valid");
  ok &= Expect(result.bestIndex >= 0, "best candidate index should be valid");
  ok &= Expect(result.best().managedOffset == 0x48,
               "managed offset should be 0x48");
  ok &= Expect(result.best().managed == managed, "managed pointer should match");
  ok &= Expect(result.best().klass == klass, "klass pointer should match");
  ok &= Expect(result.best().hasNativeBackPointer,
               "native back pointer should be detected");
  ok &= Expect(result.best().runtimeClass.fullName == "UnityEngine.GameObject",
               "full class name should match");
  ok &= Expect(result.suggestedManagedOffset == 0x48,
               "suggested managed offset should match");

  er2::NativeChainProbeResult invalid;
  ok &= Expect(!er2::ProbeNativeObjectChain(mem, 0, profile, invalid),
               "zero native should fail");

  if (!ok) {
    return 1;
  }

  std::printf("[native_chain_probe] PASS\n");
  return 0;
}
