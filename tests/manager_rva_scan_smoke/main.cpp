#include "er2/unity2/headless/manager_rva_scan.hpp"

#include <Windows.h>

#include <cstdint>
#include <cstdio>
#include <cstring>
#include <vector>

namespace {

class LocalMemoryAccessor final : public er2::IMemoryAccessor {
public:
  bool Read(std::uintptr_t address, void *buffer,
            std::size_t size) const override {
    if (!address || !buffer || !size) {
      return false;
    }
    std::memcpy(buffer, reinterpret_cast<const void *>(address), size);
    return true;
  }

  bool Write(std::uintptr_t address, const void *buffer,
             std::size_t size) const override {
    if (!address || !buffer || !size) {
      return false;
    }
    std::memcpy(reinterpret_cast<void *>(address), buffer, size);
    return true;
  }
};

bool Expect(bool condition, const char *message) {
  if (condition) {
    return true;
  }
  std::printf("[manager_rva_scan] FAIL: %s\n", message);
  return false;
}

template <typename T> void Put(std::vector<std::uint8_t> &buf,
                               std::size_t offset, const T &value) {
  std::memcpy(buf.data() + offset, &value, sizeof(T));
}

void BuildFakePe(std::vector<std::uint8_t> &module) {
  IMAGE_DOS_HEADER dos{};
  dos.e_magic = IMAGE_DOS_SIGNATURE;
  dos.e_lfanew = 0x80;
  Put(module, 0, dos);

  IMAGE_NT_HEADERS64 nt{};
  nt.Signature = IMAGE_NT_SIGNATURE;
  nt.FileHeader.Machine = IMAGE_FILE_MACHINE_AMD64;
  nt.FileHeader.NumberOfSections = 1;
  nt.FileHeader.SizeOfOptionalHeader = sizeof(IMAGE_OPTIONAL_HEADER64);
  nt.OptionalHeader.Magic = IMAGE_NT_OPTIONAL_HDR64_MAGIC;
  nt.OptionalHeader.SizeOfImage = static_cast<DWORD>(module.size());
  Put(module, dos.e_lfanew, nt);

  IMAGE_SECTION_HEADER sh{};
  std::memcpy(sh.Name, ".data", 5);
  sh.VirtualAddress = 0x1000;
  sh.Misc.VirtualSize = 0x1000;
  const std::size_t secOff = static_cast<std::size_t>(dos.e_lfanew) +
                             sizeof(DWORD) + sizeof(IMAGE_FILE_HEADER) +
                             sizeof(IMAGE_OPTIONAL_HEADER64);
  Put(module, secOff, sh);
}

} // namespace

int main() {
  std::vector<std::uint8_t> module(0x3000);
  BuildFakePe(module);

  std::vector<std::uint8_t> typeInfo(0x200);
  std::vector<std::uint8_t> staticFields(0x200);
  std::vector<std::uint8_t> manager(0x100);
  std::vector<std::uint8_t> list(0x40);
  std::vector<std::uint8_t> array(0x80);
  std::vector<std::uint8_t> actor(0x80);

  const auto typeInfoPtr =
      reinterpret_cast<std::uintptr_t>(typeInfo.data());
  const auto staticPtr =
      reinterpret_cast<std::uintptr_t>(staticFields.data());
  const auto managerPtr = reinterpret_cast<std::uintptr_t>(manager.data());
  const auto listPtr = reinterpret_cast<std::uintptr_t>(list.data());
  const auto arrayPtr = reinterpret_cast<std::uintptr_t>(array.data());
  const auto actorPtr = reinterpret_cast<std::uintptr_t>(actor.data());

  Put(module, 0x1230, typeInfoPtr);
  Put(typeInfo, 0xB8, staticPtr);
  Put(staticFields, 0x8, managerPtr);
  Put(manager, 0x18, actorPtr);
  Put(manager, 0x58, listPtr);
  Put(list, 0x18, arrayPtr);
  Put(array, 0x18, std::uint32_t{3});
  Put(array, 0x30, actorPtr);

  LocalMemoryAccessor mem;
  er2::ManagerRvaScanOptions options;
  options.maxSectionBytes = 0x1000;
  er2::ManagerRvaScanResult result;
  const bool scanOk = er2::ScanGameAssemblyManagerRvas(
      mem, reinterpret_cast<std::uintptr_t>(module.data()),
      static_cast<std::uint32_t>(module.size()), options, result);

  bool ok = true;
  ok &= Expect(scanOk, "scan should succeed");
  ok &= Expect(result.sectionsVisited == 1, "data section should be scanned");
  const er2::ManagerRvaCandidate *best = er2::FindBestManagerRvaCandidate(
      result, er2::ManagerRvaKind::CharacterManager);
  ok &= Expect(best != nullptr, "character manager should be found");
  if (best) {
    ok &= Expect(best->rva == 0x1230, "current RVA should come from live slot");
    ok &= Expect(best->resolvedPtr == managerPtr,
                 "manager instance should be resolved");
    ok &= Expect(best->count == 3, "alive character count should be read");
  }

  if (!ok) {
    return 1;
  }
  std::printf("[manager_rva_scan] PASS\n");
  return 0;
}
