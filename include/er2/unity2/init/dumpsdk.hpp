#pragma once

#include "context.hpp"

#include "../dumpsdk.hpp"

namespace er2 {

inline bool DumpSdk6Dump(DumpSdk6Paths &outPaths) {
  outPaths = DumpSdk6Paths{};
  if (!IsInited()) {
    return false;
  }

  return DumpSdk6DumpByPid(g_ctx.pid, outPaths);
}

inline bool DumpSdk6Dump(const IMemoryAccessor &mem,
                         std::uintptr_t gameAssemblyBase,
                         std::uint32_t gameAssemblySize,
                         const std::string &gameAssemblyName,
                         DumpSdk6Paths &outPaths) {
  outPaths = DumpSdk6Paths{};
  if (!IsInited()) {
    return false;
  }
  return DumpSdk6DumpWithAccessor(mem, g_ctx.pid, gameAssemblyBase,
                                  gameAssemblySize, gameAssemblyName, outPaths);
}

} // namespace er2
