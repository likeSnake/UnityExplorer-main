#include "er2/unity2/inspect/runtime_class_probe.hpp"

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

  std::printf("[runtime_class_probe] FAIL: %s\n", message);
  return false;
}

} // namespace

int main() {
  FakeMemory mem;
  const er2::Il2CppLayoutProfile &profile =
      er2::kIl2CppUnity2019_4_Lts24_5_Profile;

  const std::uintptr_t klass = 0x100000;
  const std::uintptr_t name = 0x200000;
  const std::uintptr_t ns = 0x200100;
  const std::uintptr_t parent = 0x110000;
  const std::uintptr_t fields = 0x300000;
  const std::uintptr_t methods = 0x400000;
  const std::uintptr_t statics = 0x500000;

  std::vector<std::uint8_t> klassBytes(0x140);
  Put(klassBytes, profile.klass.name, name);
  Put(klassBytes, profile.klass.namespaze, ns);
  Put(klassBytes, profile.klass.parent, parent);
  Put(klassBytes, profile.klass.fields, fields);
  Put(klassBytes, profile.klass.methods, methods);
  Put(klassBytes, profile.klass.staticFields, statics);
  Put<std::uint16_t>(klassBytes, profile.klass.fieldCount, 7);
  Put<std::uint16_t>(klassBytes, profile.klass.methodCount, 42);
  mem.Add(klass, std::move(klassBytes));

  mem.Add(name, std::vector<std::uint8_t>{'P', 'l', 'a', 'y', 'e', 'r', 0});
  mem.Add(ns, std::vector<std::uint8_t>{'G', 'a', 'm', 'e', 0});

  er2::RuntimeClassProbeResult result;
  const bool readOk = er2::ProbeRuntimeClass(mem, klass, profile, result);

  bool ok = true;
  ok &= Expect(readOk, "ProbeRuntimeClass should read fake class");
  ok &= Expect(result.valid, "result should be valid");
  ok &= Expect(result.className == "Player", "class name should match");
  ok &= Expect(result.namespaze == "Game", "namespace should match");
  ok &= Expect(result.fullName == "Game.Player", "full name should match");
  ok &= Expect(result.parent == parent, "parent pointer should match");
  ok &= Expect(result.fields == fields, "fields pointer should match");
  ok &= Expect(result.methods == methods, "methods pointer should match");
  ok &= Expect(result.staticFields == statics, "static fields should match");
  ok &= Expect(result.fieldCount == 7, "field count should match");
  ok &= Expect(result.methodCount == 42, "method count should match");

  er2::RuntimeClassProbeResult invalid;
  ok &= Expect(!er2::ProbeRuntimeClass(mem, 0, profile, invalid),
               "zero klass should fail");

  if (!ok) {
    return 1;
  }

  std::printf("[runtime_class_probe] PASS\n");
  return 0;
}
