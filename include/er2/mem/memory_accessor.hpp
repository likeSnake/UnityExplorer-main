#pragma once

#include <cstddef>
#include <cstdint>

namespace er2 {

/// <summary>
/// Abstract interface for memory access.
/// </summary>
class IMemoryAccessor {
public:
  virtual ~IMemoryAccessor() = default;

  /// <summary>
  /// Reads memory from the target process.
  /// </summary>
  virtual bool Read(std::uintptr_t address, void *buffer,
                    std::size_t size) const = 0;

  /// <summary>
  /// Writes memory to the target process.
  /// </summary>
  virtual bool Write(std::uintptr_t address, const void *buffer,
                     std::size_t size) const = 0;

  /// <summary>
  /// Prepares a scatter read request.
  /// Default implementation performs immediate synchronous read (fallback).
  /// </summary>
  virtual void PrepareScatterRead(std::uintptr_t address, void *buffer,
                                  std::size_t size) const {
    Read(address, buffer, size);
  }

  /// <summary>
  /// Executes all prepared scatter read requests.
  /// Default implementation returns true (as reads are already done).
  /// </summary>
  virtual bool ExecuteScatterRead() const { return true; }
};

} // namespace er2
