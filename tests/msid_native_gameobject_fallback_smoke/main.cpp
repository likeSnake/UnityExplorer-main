#include "er2/unity2/msid/enumerate_objects.hpp"

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

  std::printf("[msid_native_gameobject_fallback] FAIL: %s\n", message);
  return false;
}

} // namespace

int main() {
  FakeMemory mem;
  er2::Offsets off;
  er2::UnityPlayerRange unityPlayer;
  unityPlayer.base = 0x700000;
  unityPlayer.size = 0x100000;

  const std::uintptr_t msidSlot = 0x100000;
  const std::uintptr_t set = 0x110000;
  const std::uintptr_t entries = 0x120000;
  const std::uintptr_t nativeGo = 0x200000;
  const std::uintptr_t managed = 0x210000;
  const std::uintptr_t klass = 0x220000;
  const std::uintptr_t pool = 0x230000;
  const std::uintptr_t nativeTransform = 0x240000;
  const std::uintptr_t name = 0x250000;
  const std::uintptr_t vtable = unityPlayer.base + 0x1234;

  std::vector<std::uint8_t> slotBytes(sizeof(std::uintptr_t));
  Put(slotBytes, 0, set);
  mem.Add(msidSlot, std::move(slotBytes));

  std::vector<std::uint8_t> setBytes(0x10);
  Put(setBytes, er2::kMsIdSetEntriesBase, entries);
  Put<std::uint32_t>(setBytes, er2::kMsIdSetCapacity, 1);
  Put<std::uint32_t>(setBytes, er2::kMsIdSetCount, 1);
  mem.Add(set, std::move(setBytes));

  std::vector<std::uint8_t> entryBytes(er2::kMsIdEntryStride);
  er2::MsIdToPointerEntryRaw raw;
  raw.hashMask = 0;
  raw.key = 1337;
  raw.object = nativeGo;
  std::memcpy(entryBytes.data(), &raw, sizeof(raw));
  mem.Add(entries, std::move(entryBytes));

  std::vector<std::uint8_t> goBytes(0x80);
  Put(goBytes, 0x00, vtable);
  Put(goBytes, off.unity_object_managed_ptr, managed);
  Put(goBytes, 0x30, pool);
  Put<std::int32_t>(goBytes, 0x40, 1);
  Put(goBytes, off.game_object_name_ptr, name);
  mem.Add(nativeGo, std::move(goBytes));

  std::vector<std::uint8_t> managedBytes(0x20);
  Put(managedBytes, 0x00, klass);
  Put(managedBytes, 0x10, nativeGo);
  mem.Add(managed, std::move(managedBytes));

  std::vector<std::uint8_t> klassBytes(0x80);
  mem.Add(klass, std::move(klassBytes));

  std::vector<std::uint8_t> poolBytes(0x10);
  Put<std::int32_t>(poolBytes, 0x00, 4);
  Put(poolBytes, 0x08, nativeTransform);
  mem.Add(pool, std::move(poolBytes));

  std::vector<std::uint8_t> transformBytes(0x40);
  Put(transformBytes, 0x00, vtable);
  Put(transformBytes, 0x30, nativeGo);
  mem.Add(nativeTransform, std::move(transformBytes));

  std::vector<std::uint8_t> nameBytes(128);
  const char objectName[] = "TestGO";
  std::memcpy(nameBytes.data(), objectName, sizeof(objectName));
  mem.Add(name, std::move(nameBytes));

  er2::EnumerateOptions opt;
  opt.onlyGameObject = false;
  opt.onlyScriptableObject = false;

  std::vector<er2::ObjectInfo> objects;
  const bool okEnum = er2::EnumerateMsIdToPointerObjects(
      er2::ManagedBackend::Il2Cpp, mem, msidSlot, off, unityPlayer, opt,
      [&](const er2::ObjectInfo &info) { objects.push_back(info); });

  bool ok = true;
  ok &= Expect(okEnum, "enumeration should succeed");
  ok &= Expect(objects.size() == 1,
               "native GameObject fallback should keep the object");
  if (!objects.empty()) {
    ok &= Expect(objects.front().kind == er2::ObjectKind::GameObject,
                 "object should be classified as GameObject");
    ok &= Expect(objects.front().typeFullName == "UnityEngine.GameObject",
                 "fallback type name should be UnityEngine.GameObject");
    ok &= Expect(objects.front().objectName == "TestGO",
                 "fallback should still read native object name");
  }

  if (!ok) {
    return 1;
  }

  std::printf("[msid_native_gameobject_fallback] PASS\n");
  return 0;
}
