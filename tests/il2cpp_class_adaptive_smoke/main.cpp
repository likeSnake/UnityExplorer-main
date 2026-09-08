#include "er2/unity2/object/managed/il2cpp_class.hpp"

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

  std::printf("[il2cpp_class_adaptive] FAIL: %s\n", message);
  return false;
}

} // namespace

int main() {
  FakeMemory mem;
  er2::Offsets off;

  const std::uintptr_t klass = 0x100000;
  const std::uintptr_t emptyName = 0x200000;
  const std::uintptr_t shiftedName = 0x210000;
  const std::uintptr_t shiftedNs = 0x220000;

  std::vector<std::uint8_t> klassBytes(0x100);
  Put(klassBytes, off.il2cppclass_name_ptr, emptyName);
  Put(klassBytes, 0x30, shiftedName);
  Put(klassBytes, 0x38, shiftedNs);
  mem.Add(klass, std::move(klassBytes));

  mem.Add(emptyName, std::vector<std::uint8_t>{0});
  mem.Add(shiftedName,
          std::vector<std::uint8_t>{'G', 'a', 'm', 'e', 'O', 'b', 'j',
                                    'e', 'c', 't', 0});
  mem.Add(shiftedNs, std::vector<std::uint8_t>{'U', 'n', 'i', 't', 'y',
                                               'E', 'n', 'g', 'i', 'n',
                                               'e', 0});

  std::string ns;
  std::string cn;
  const bool okRead =
      er2::ReadIl2CppClassNameAdaptive(mem, klass, off, ns, cn);
  std::string legacyNs;
  std::string legacyCn;

  bool ok = true;
  ok &= Expect(!er2::ReadIl2CppClassName(mem, klass, off, legacyNs,
                                         legacyCn) ||
                   legacyCn.empty(),
               "legacy reader should not return the shifted class name");
  ok &= Expect(okRead, "adaptive reader should recover shifted strings");
  ok &= Expect(ns == "UnityEngine", "namespace should match shifted slot");
  ok &= Expect(cn == "GameObject", "class name should match shifted slot");
  ok &= Expect(er2::IsClassOrParent(mem, klass, off, "UnityEngine",
                                    "GameObject"),
               "adaptive parent matcher should match current class");

  if (!ok) {
    return 1;
  }

  std::printf("[il2cpp_class_adaptive] PASS\n");
  return 0;
}
