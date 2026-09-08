#pragma once

#include "../../../deps/UnityResolve.hpp/External/Core/UnityExternalMemory.hpp"
#include "../../../include/er2/mem/memory_accessor.hpp"
#include <memory>

// Adapter to bridge er2 IMemoryAccessor to UnityExternal IMemoryAccessor
class ER2MemoryAdapter : public UnityExternal::IMemoryAccessor {
private:
  std::shared_ptr<er2::IMemoryAccessor> m_er2Accessor;

public:
  ER2MemoryAdapter(std::shared_ptr<er2::IMemoryAccessor> accessor)
      : m_er2Accessor(accessor) {}

  virtual bool Read(std::uintptr_t address, void *buffer,
                    std::size_t size) const override {
    if (!m_er2Accessor)
      return false;
    return m_er2Accessor->Read(address, buffer, size);
  }

  virtual bool Write(std::uintptr_t address, const void *buffer,
                     std::size_t size) const override {
    if (!m_er2Accessor)
      return false;
    return m_er2Accessor->Write(address, buffer, size);
  }

  // Allow updating the accessor if context changes
  void UpdateAccessor(std::shared_ptr<er2::IMemoryAccessor> accessor) {
    m_er2Accessor = accessor;
  }
};
