#include "MemIO.h"
#include "libs/vmmdll.h"
#include <atomic>
#include <cstdlib>
#include <cstring>
#include <mutex>
#include <shared_mutex>
#include <thread>
#include <unordered_map>
#include <vector>
#include <winternl.h>

static constexpr auto PAGE_SIZE = 0x1000;
static VMM_HANDLE hVmm = nullptr;
static unsigned __int32 ProcessId = 0;
static unsigned __int64 DTB = 0;

struct Page {
  void *Data = nullptr;
  // A page is leased while in use. An atomic flag (rather than an owning
  // std::mutex) lets shutdown release caches from a different control thread.
  std::atomic<bool> Leased{false};
};

static std::shared_mutex PageMutex;
static std::vector<Page *> Pages;

static std::shared_mutex ThreadsMutex;
static std::unordered_map<std::thread::id,
                          std::unordered_map<unsigned __int64, Page *>>
    Threads;

// 保护DTBPages的独立锁
static std::shared_mutex DTBMutex;
static std::unordered_map<unsigned __int64, Page *> DTBPages;

struct PML4Entry {
  uint64_t present : 1;
  uint64_t read_write : 1;
  uint64_t user_supervisor : 1;
  uint64_t page_write_through : 1;
  uint64_t page_cache_disable : 1;
  uint64_t accessed : 1;
  uint64_t ignored_1 : 1;
  uint64_t large_page : 1;
  uint64_t ignored_2 : 4;
  uint64_t address : 40;
  uint64_t ignored_3 : 11;
  uint64_t no_execute : 1;

  PML4Entry() { memset(this, 0, sizeof(PML4Entry)); }
  PML4Entry(unsigned __int64 Value) {
    memset(this, 0, sizeof(PML4Entry));
    *(unsigned __int64 *)this = Value;
  }
  unsigned __int64 GetAddress() const { return address << 12; }
};

struct VirtualAddress {
  uint64_t Offset : 12;
  uint64_t PTIndex : 9;
  uint64_t PDIndex : 9;
  uint64_t PDPTIndex : 9;
  uint64_t PML4Index : 9;
  uint64_t Reserved : 16;
  VirtualAddress() { memset(this, 0, sizeof(VirtualAddress)); }
  VirtualAddress(unsigned __int64 Value) {
    memset(this, 0, sizeof(VirtualAddress));
    *(unsigned __int64 *)this = Value;
  }
};

Page *GetPage() {
  std::shared_lock<std::shared_mutex> sharedLock(PageMutex);
  for (auto &Page : Pages) {
    bool expected = false;
    if (Page->Leased.compare_exchange_strong(expected, true,
                                             std::memory_order_acq_rel)) {
      sharedLock.unlock();
      return Page;
    }
  }
  sharedLock.unlock();
  auto NewPage = new Page();
  NewPage->Data = malloc(PAGE_SIZE);
  if (!NewPage->Data) {
    delete NewPage;
    return nullptr;
  }
  memset(NewPage->Data, 0, PAGE_SIZE); // Zero init to prevent reading garbage
  NewPage->Leased.store(true, std::memory_order_release);
  std::unique_lock<std::shared_mutex> uniqueLock(PageMutex);
  Pages.push_back(NewPage);
  return NewPage;
}

void FreePage(Page *Page) {
  if (!Page)
    return;

  Page->Leased.store(false, std::memory_order_release);
}

void MemIO::Init(void *hVmm_) {
  const auto newHandle = static_cast<VMM_HANDLE>(hVmm_);
  if (::hVmm != newHandle)
    ResetAll();
  ::hVmm = newHandle;
}

void MemIO::ResetAll() {
  // The caller is responsible for draining active readers first. Keep this
  // lock order stable with translation code: DTB -> Thread cache -> Page pool.
  {
    std::unique_lock<std::shared_mutex> dtbLock(DTBMutex);
    for (auto &entry : DTBPages)
      FreePage(entry.second);
    DTBPages.clear();
    DTB = 0;
  }

  {
    std::unique_lock<std::shared_mutex> threadsLock(ThreadsMutex);
    for (auto &threadEntry : Threads) {
      for (auto &cacheEntry : threadEntry.second)
        FreePage(cacheEntry.second);
      threadEntry.second.clear();
    }
    Threads.clear();
  }

  {
    std::unique_lock<std::shared_mutex> pagesLock(PageMutex);
    for (Page *page : Pages) {
      if (!page)
        continue;
      page->Leased.store(false, std::memory_order_release);
      free(page->Data);
      page->Data = nullptr;
      delete page;
    }
    Pages.clear();
  }

  ::ProcessId = 0;
}

void MemIO::Shutdown() {
  ResetAll();
  ::hVmm = nullptr;
}

bool MemIO::SetProcessId(unsigned __int32 ProcessId_, unsigned __int64 NewDTB) {
  // 独占DTB页表和物理地址转换锁，避免中间态
  std::unique_lock<std::shared_mutex> dtbLock(DTBMutex);

  if (!hVmm || ProcessId_ == 0 || NewDTB == 0)
    return false;

  ::ProcessId = ProcessId_;

  // 遍历删除
  for (auto it = DTBPages.begin(); it != DTBPages.end();) {
    FreePage(it->second);
    it = DTBPages.erase(it);
  }
  DTBPages.clear();

  Page *rootPage = GetPage();
  if (!rootPage)
    return false;
  DTBPages[NewDTB] = rootPage;
  VMMDLL_MemReadEx(hVmm, -1, NewDTB, (PBYTE)rootPage->Data, PAGE_SIZE, 0,
                   VMMDLL_FLAG_NOCACHE);

  unsigned __int64 *Buffer = (unsigned __int64 *)rootPage->Data;
  for (unsigned __int64 i = 0; i < PAGE_SIZE / 0x8; ++i) {
    PML4Entry PML4E = Buffer[i];
    if (PML4E.present) {
      const auto pageAddress = PML4E.GetAddress();
      if (DTBPages.find(pageAddress) != DTBPages.end())
        continue;
      Page *page = GetPage();
      if (!page)
        continue;
      DTBPages[pageAddress] = page;
      VMMDLL_MemReadEx(hVmm, -1, pageAddress, (PBYTE)page->Data, PAGE_SIZE, 0,
                       VMMDLL_FLAG_NOCACHE);
    }
  }
  DTB = NewDTB;
  return true;
}

// 清理当前线程的缓存
void MemIO::Clear() {
  std::unique_lock<std::shared_mutex> uniqueLock(ThreadsMutex);
  auto it = Threads.find(std::this_thread::get_id());
  if (it != Threads.end()) {
    for (auto &Cache : it->second)
      FreePage(Cache.second);
    Threads.erase(it);
  }
}

unsigned __int64 MemIO::Vir2Phy(unsigned __int64 Address) {
  if (!hVmm || !DTB || !Address)
    return 0;

  VirtualAddress VA(Address);

  auto ReadPage = [](unsigned __int64 Address) {
    // 第一次只读
    {
      std::shared_lock<std::shared_mutex> readLock(DTBMutex);
      auto it = DTBPages.find(Address);
      if (it != DTBPages.end()) {
        return (unsigned __int64 *)it->second->Data;
      }
    }
    // 没找到，写锁
    {
      std::unique_lock<std::shared_mutex> writeLock(DTBMutex);
      auto it = DTBPages.find(Address);
      if (it == DTBPages.end()) {
        Page *page = GetPage();
        if (!page)
          return static_cast<unsigned __int64 *>(nullptr);
        VMMDLL_MemReadEx(hVmm, -1, Address, (PBYTE)page->Data, PAGE_SIZE, 0,
                         VMMDLL_FLAG_NOCACHE);
        DTBPages[Address] = page;
        return (unsigned __int64 *)page->Data;
      }
      return (unsigned __int64 *)it->second->Data;
    }
  };

  // 提前加phyLock，ReadPage做锁保护
  auto *pml4Page = ReadPage(DTB);
  if (!pml4Page)
    return 0;
  PML4Entry PML4 = pml4Page[VA.PML4Index];
  if (!PML4.present)
    return 0;

  auto *pdptPage = ReadPage(PML4.GetAddress());
  if (!pdptPage)
    return 0;
  PML4Entry PDPT = pdptPage[VA.PDPTIndex];
  if (!PDPT.present)
    return 0;
  if (PDPT.large_page)
    return (PDPT.GetAddress() & ~((1ULL << 30) - 1)) +
           (Address & ((1ULL << 30) - 1));

  auto *pdPage = ReadPage(PDPT.GetAddress());
  if (!pdPage)
    return 0;
  PML4Entry PD = pdPage[VA.PDIndex];
  if (!PD.present)
    return 0;
  if (PD.large_page)
    return (PD.GetAddress() & ~((1ULL << 21) - 1)) +
           (Address & ((1ULL << 21) - 1));

  auto *ptPage = ReadPage(PD.GetAddress());
  if (!ptPage)
    return 0;
  PML4Entry PT = ptPage[VA.PTIndex];
  if (!PT.present)
    return 0;

  return PT.GetAddress() + VA.Offset;
}

bool MemIO::Read(unsigned __int64 Address, void *Buffer, unsigned __int32 Size,
                 bool Cache) {
  if (!hVmm || !Buffer || Size == 0 || ProcessId == 0)
    return false;

  const auto physicalAddress = Vir2Phy(Address);
  if (!physicalAddress)
    return false;

  DWORD bytesRead = 0;
  const DWORD flags = Cache ? 0 : VMMDLL_FLAG_NOCACHE;
  return VMMDLL_MemReadEx(hVmm, -1, physicalAddress,
                          static_cast<PBYTE>(Buffer), Size, &bytesRead,
                          flags) &&
         bytesRead == Size;
}
