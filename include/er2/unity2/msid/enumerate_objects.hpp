#pragma once

#include <cstdint>
#include <functional>
#include <sstream>
#include <string>
#include <unordered_map>
#include <vector>

#include "../../mem/memory_read.hpp"
#include "../core/offsets.hpp"
#include "../init/context.hpp"
#include "../object/managed/il2cpp_class.hpp"
#include "../object/managed/managed_backend.hpp"
#include "../object/managed/mono_class.hpp"
#include "../object/native/native_game_object_name.hpp"
#include "../object/native/native_object.hpp"
#include "../object/native/native_scriptable_object_name.hpp"
#include "ms_id_to_pointer.hpp"

namespace er2 {

enum class ObjectKind : std::uint8_t {
  GameObject = 1,
  ScriptableObject = 2,
  Other = 3,
};

struct ObjectInfo {
  std::uintptr_t native = 0;
  std::uint32_t instanceId = 0;
  std::string objectName;
  std::string typeNamespace; // Namespace (e.g. "UnityEngine")
  std::string typeClassName; // Class name only (e.g. "GameObject")
  std::string typeFullName;  // Full name (e.g. "UnityEngine.GameObject")
  ObjectKind kind = ObjectKind::Other;
};

struct EnumerateOptions {
  bool onlyGameObject = true;
  bool onlyScriptableObject = true;
  std::string filterLower;
};

inline bool MatchFilterLower(const std::string &s,
                             const std::string &filterLower) {
  if (filterLower.empty())
    return true;
  std::string tmp;
  tmp.reserve(s.size());
  for (char c : s) {
    if (c >= 'A' && c <= 'Z')
      tmp.push_back(static_cast<char>(c - 'A' + 'a'));
    else
      tmp.push_back(c);
  }
  return tmp.find(filterLower) != std::string::npos;
}

inline bool LooksLikeNativeGameObjectByComponentBackPointer(
    const IMemoryAccessor &mem, std::uintptr_t nativeObject,
    const UnityPlayerRange &unityPlayer, const std::uint8_t *objBuffer,
    std::uint32_t objBufferSize) {
  if (!IsCanonicalUserPtr(nativeObject) || !objBuffer) {
    return false;
  }

  constexpr std::uint32_t kComponentPoolOffset = 0x30;
  constexpr std::uint32_t kComponentCountOffset = 0x40;
  constexpr std::uint32_t kComponentSlotStride = 0x10;
  constexpr std::uint32_t kComponentSlotNativeOffset = 0x08;
  constexpr std::uint32_t kComponentGameObjectOffset = 0x30;

  std::uintptr_t pool = 0;
  if (kComponentPoolOffset + sizeof(pool) <= objBufferSize) {
    pool =
        *reinterpret_cast<const std::uintptr_t *>(objBuffer + kComponentPoolOffset);
  } else if (!ReadPtr(mem, nativeObject + kComponentPoolOffset, pool)) {
    return false;
  }

  if (!IsCanonicalUserPtr(pool)) {
    return false;
  }

  std::int32_t count = 0;
  if (kComponentCountOffset + sizeof(count) <= objBufferSize) {
    count =
        *reinterpret_cast<const std::int32_t *>(objBuffer + kComponentCountOffset);
  } else if (!ReadValue(mem, nativeObject + kComponentCountOffset, count)) {
    return false;
  }

  if (count <= 0 || count > 1024) {
    return false;
  }

  const std::int32_t probeCount = count < 8 ? count : 8;
  for (std::int32_t i = 0; i < probeCount; ++i) {
    const std::uintptr_t slot =
        pool + static_cast<std::uintptr_t>(i) * kComponentSlotStride;

    std::uintptr_t nativeComponent = 0;
    if (!ReadPtr(mem, slot + kComponentSlotNativeOffset, nativeComponent) ||
        !IsCanonicalUserPtr(nativeComponent)) {
      continue;
    }

    std::uintptr_t componentVtable = 0;
    if (!ReadPtr(mem, nativeComponent, componentVtable) ||
        componentVtable < unityPlayer.base ||
        componentVtable >= unityPlayer.base + unityPlayer.size) {
      continue;
    }

    std::uintptr_t ownerGameObject = 0;
    if (ReadPtr(mem, nativeComponent + kComponentGameObjectOffset,
                ownerGameObject) &&
        ownerGameObject == nativeObject) {
      return true;
    }
  }

  return false;
}

inline bool EnumerateMsIdToPointerObjects(
    ManagedBackend runtime, const IMemoryAccessor &mem,
    std::uintptr_t msIdToPointerAddr, const Offsets &off,
    const UnityPlayerRange &unityPlayer, const EnumerateOptions &opt,
    const std::function<void(const ObjectInfo &)> &cb) {
  // [Debug] Early Entry Log
  er2::Log("[DEBUG] EnumerateMsIdToPointerObjects Called");
  {
    std::stringstream ss;
    ss << "  msIdToPointerAddr: 0x" << std::hex << msIdToPointerAddr;
    er2::Log(ss.str());
  }

  // [Debug] Log UnityPlayer Range
  {
    std::stringstream ss;
    ss << "[DEBUG] UnityPlayer Range: Base=0x" << std::hex << unityPlayer.base
       << " Size=0x" << unityPlayer.size << " End=0x"
       << (unityPlayer.base + unityPlayer.size);
    er2::Log(ss.str());
  }

  MsIdToPointerSet set;
  if (!ReadMsIdToPointerSet(mem, msIdToPointerAddr, set)) {
    er2::Log("[DEBUG] ERROR: ReadMsIdToPointerSet failed!");
    // Try to read raw value to see what's there
    std::uintptr_t rawVal = 0;
    if (mem.Read(msIdToPointerAddr, &rawVal, sizeof(rawVal))) {
      std::stringstream ss;
      ss << "  Raw value at addr: 0x" << std::hex << rawVal;
      er2::Log(ss.str());
    } else {
      er2::Log("  Failed to read raw value at addr.");
    }
    return false;
  }
  {
    std::stringstream ss;
    ss << "[DEBUG] ReadMsIdToPointerSet Success. set.set = 0x" << std::hex
       << set.set;
    er2::Log(ss.str());
  }

  std::uintptr_t entriesBase = 0;
  std::uint32_t capacity = 0;
  std::uint32_t count = 0;
  if (!ReadMsIdEntriesHeader(mem, set, entriesBase, capacity, count)) {
    er2::Log("[DEBUG] ERROR: ReadMsIdEntriesHeader failed!");
    {
      std::stringstream ss;
      ss << "  Trying to debug header at set.set (0x" << std::hex << set.set
         << std::dec << ")...";
      er2::Log(ss.str());
    }
    // Dump header manually
    std::uintptr_t debugEntriesBase = 0;
    std::uint32_t debugCapacity = 0;
    std::uint32_t debugCount = 0;
    // offsets from ms_id_to_pointer.hpp: entries=0x0, capacity=0x8, count=0xC
    mem.Read(set.set + 0x0, &debugEntriesBase, sizeof(debugEntriesBase));
    mem.Read(set.set + 0x8, &debugCapacity, sizeof(debugCapacity));
    mem.Read(set.set + 0xC, &debugCount, sizeof(debugCount));
    {
      std::stringstream ss;
      ss << "  Raw Header: EntriesBase=0x" << std::hex << debugEntriesBase
         << " Capacity=" << std::dec << debugCapacity
         << " Count=" << debugCount;
      er2::Log(ss.str());
    }
    return false;
  }

  const std::uint32_t kChunkSize =
      256; // [FIX] Reduced to 256 (4KB) for page alignment
  std::vector<std::uint8_t> buffer;
  buffer.resize(kChunkSize * kMsIdEntryStride);

  const std::uint32_t kObjHeaderSize = 0x60;
  std::uint8_t objBuffer[kObjHeaderSize];

  const std::uint32_t kNameBufferSize = 128;
  std::uint8_t nameBuffer[kNameBufferSize];

  struct CachedClassInfo {
    bool valid;
    bool isGo;
    bool isSo;
    std::string ns;
    std::string cn;
  };
  std::unordered_map<std::uintptr_t, CachedClassInfo> classCache;

  // [Debug] Counters
  er2::Log("[DEBUG] MSID Enumeration Start");
  {
    std::stringstream ss;
    ss << "  entriesBase: 0x" << std::hex << entriesBase;
    er2::Log(ss.str());
  }
  {
    std::stringstream ss;
    ss << "  capacity: " << capacity;
    er2::Log(ss.str());
  }
  {
    std::stringstream ss;
    ss << "  count: " << count;
    er2::Log(ss.str());
  }

  int skippedHash = 0;
  int skippedKey = 0;
  int skippedObjRead = 0;
  int skippedVtable = 0;
  int skippedRange = 0;
  int acceptedCount = 0;
  int nativeFallbackCount = 0;

  for (std::uint32_t i = 0; i < capacity; i += kChunkSize) {
    std::uint32_t currentChunkCount = capacity - i;
    if (currentChunkCount > kChunkSize)
      currentChunkCount = kChunkSize;

    const std::uintptr_t chunkStart =
        entriesBase + static_cast<std::uintptr_t>(i) * kMsIdEntryStride;

    if (!mem.Read(chunkStart, buffer.data(),
                  currentChunkCount * kMsIdEntryStride)) {
      // [FIX] Fallback Strategy: Read entry by entry
      // If a large 4KB read fails (e.g. one page is swapped out), we try to
      // salvage the valid pages/entries within this range.
      // std::stringstream ss;
      // ss << "[DEBUG] Failed to read chunk at offset " << i << ". Retrying
      // entry-by-entry..."; er2::Log(ss.str());

      std::memset(buffer.data(), 0, buffer.size()); // Clear buffer

      for (std::uint32_t k = 0; k < currentChunkCount; ++k) {
        std::uintptr_t entryAddr =
            chunkStart + static_cast<std::uintptr_t>(k) * kMsIdEntryStride;
        // Try to read single entry
        mem.Read(entryAddr, buffer.data() + k * kMsIdEntryStride,
                 kMsIdEntryStride);
        // We ignore failure here; if it fails, the buffer remains 0 and
        // subsequent checks (key/hash) will skip it.
      }
    }

    for (std::uint32_t j = 0; j < currentChunkCount; ++j) {
      const MsIdToPointerEntryRaw *raw =
          reinterpret_cast<const MsIdToPointerEntryRaw *>(buffer.data() +
                                                          j * kMsIdEntryStride);

      if (raw->hashMask >= 0xFFFFFFFEu) {
        skippedHash++;
        continue;
      }
      if (raw->key == 0) {
        skippedKey++;
        continue;
      }

      const std::uintptr_t obj = raw->object;

      if (!mem.Read(obj, objBuffer, kObjHeaderSize)) {
        skippedObjRead++;
        continue;
      }
      std::uintptr_t vtable = *reinterpret_cast<std::uintptr_t *>(objBuffer);
      if (!IsCanonicalUserPtr(vtable)) {
        skippedVtable++;
        continue;
      }

      // [STRICT] UnityPlayer Range Check - Fail if outside
      if (vtable < unityPlayer.base ||
          vtable >= (unityPlayer.base + unityPlayer.size)) {
        if (skippedRange < 10) {
          std::stringstream ss;
          ss << "[DEBUG] Skipped Range: Vtable=0x" << std::hex << vtable
             << " (Outside module)";
          er2::Log(ss.str());
        }
        skippedRange++;
        continue;
      }

      // [STRICT] Managed Pointer Check
      std::uintptr_t managed = 0;
      if (off.unity_object_managed_ptr < kObjHeaderSize) {
        managed = *reinterpret_cast<std::uintptr_t *>(
            objBuffer + off.unity_object_managed_ptr);
      } else {
        ReadPtr(mem, obj + off.unity_object_managed_ptr, managed);
      }

      if (!IsCanonicalUserPtr(managed)) {
        // Strict: Invalid managed pointer -> Skip
        continue;
      }

      std::uintptr_t klass = 0;
      if (!ReadPtr(mem, managed, klass)) {
        // Strict: Failed to read class ptr -> Skip
        continue;
      }
      // Logic branch: Strict Managed Only
      bool isGo = false;
      bool isSo = false;
      std::string ns;
      std::string cn;

      {
        auto it = classCache.find(klass);
        if (it != classCache.end()) {
          if (it->second.valid) {
            isGo = it->second.isGo;
            isSo = it->second.isSo;
            ns = it->second.ns;
            cn = it->second.cn;
          }
        } else {
          CachedClassInfo cacheEntry;
          cacheEntry.valid = false;

          if (runtime == ManagedBackend::Il2Cpp) {
            cacheEntry.isGo =
                IsClassOrParent(mem, klass, off, "UnityEngine", "GameObject");
            cacheEntry.isSo = IsClassOrParent(mem, klass, off, "UnityEngine",
                                              "ScriptableObject");
          } else {
            cacheEntry.isGo = IsMonoClassOrParent(mem, klass, off,
                                                  "UnityEngine", "GameObject");
            cacheEntry.isSo = IsMonoClassOrParent(
                mem, klass, off, "UnityEngine", "ScriptableObject");
          }

          // Always read class name regardless of type
          bool nameSuccess = false;
          if (runtime == ManagedBackend::Il2Cpp)
            nameSuccess = ReadIl2CppClassNameAdaptive(
                mem, klass, off, cacheEntry.ns, cacheEntry.cn);
          else
            nameSuccess = ReadMonoClassName(mem, klass, off, cacheEntry.ns,
                                            cacheEntry.cn);

          if (nameSuccess)
            cacheEntry.valid = true;

          classCache[klass] = cacheEntry;

          if (cacheEntry.valid) {
            isGo = cacheEntry.isGo;
            isSo = cacheEntry.isSo;
            ns = cacheEntry.ns;
            cn = cacheEntry.cn;
          }
        }
      }
      // Strict: No Native Fallback else block here.

      if (!isGo && runtime == ManagedBackend::Il2Cpp &&
          LooksLikeNativeGameObjectByComponentBackPointer(
              mem, obj, unityPlayer, objBuffer, kObjHeaderSize)) {
        isGo = true;
        isSo = false;
        ns = "UnityEngine";
        cn = "GameObject";
        nativeFallbackCount++;
      }

      // Determine ObjectKind
      ObjectKind kind;
      if (isGo)
        kind = ObjectKind::GameObject;
      else if (isSo)
        kind = ObjectKind::ScriptableObject;
      else
        kind = ObjectKind::Other;

      // Filtering logic:
      if (opt.onlyGameObject && !opt.onlyScriptableObject) {
        if (kind != ObjectKind::GameObject)
          continue;
      } else if (!opt.onlyGameObject && opt.onlyScriptableObject) {
        if (kind != ObjectKind::ScriptableObject)
          continue;
      } else if (opt.onlyGameObject && opt.onlyScriptableObject) {
        if (kind != ObjectKind::GameObject &&
            kind != ObjectKind::ScriptableObject)
          continue;
      }
      // else: both false, include everything (INCLUDING Native Only)

      std::string name;
      std::uintptr_t namePtr = 0;

      if (kind == ObjectKind::GameObject) {
        if (off.game_object_name_ptr < kObjHeaderSize) {
          namePtr = *reinterpret_cast<std::uintptr_t *>(
              objBuffer + off.game_object_name_ptr);
        } else {
          ReadPtr(mem, obj + off.game_object_name_ptr, namePtr);
        }
      } else if (kind == ObjectKind::ScriptableObject) {
        ReadScriptableObjectName(mem, obj, off, name);
      } else {
        // Try to read name for "Other" too (often works for Native Objects)
        if (off.game_object_name_ptr < kObjHeaderSize) {
          namePtr = *reinterpret_cast<std::uintptr_t *>(
              objBuffer + off.game_object_name_ptr);
        } else {
          ReadPtr(mem, obj + off.game_object_name_ptr, namePtr);
        }
      }

      if (namePtr != 0 && name.empty()) {
        if (mem.Read(namePtr, nameBuffer, kNameBufferSize)) {
          nameBuffer[kNameBufferSize - 1] = '\0';
          name = reinterpret_cast<char *>(nameBuffer);
        } else {
          ReadCString(mem, namePtr, name);
        }
      }

      // Format name for display if empty
      if (name.empty())
        name = "(Unnamed)";
      // Strict: No Native suffix needed

      std::string full = ns.empty() ? cn : (ns + "." + cn);
      if (!MatchFilterLower(name, opt.filterLower) &&
          !MatchFilterLower(full, opt.filterLower))
        continue;

      ObjectInfo info;
      info.native = obj;
      info.instanceId = raw->key;
      info.objectName = name;
      info.typeNamespace = ns;
      info.typeClassName = cn;
      info.typeFullName = full;
      info.kind = kind;
      cb(info);
      acceptedCount++;
    }
  }

  er2::Log("[DEBUG] MSID Enumeration End");
  {
    std::stringstream ss;
    ss << "  Skipped Hash: " << skippedHash;
    er2::Log(ss.str());
  }
  {
    std::stringstream ss;
    ss << "  Skipped Key: " << skippedKey;
    er2::Log(ss.str());
  }
  {
    std::stringstream ss;
    ss << "  Skipped ObjRead: " << skippedObjRead;
    er2::Log(ss.str());
  }
  {
    std::stringstream ss;
    ss << "  Skipped Vtable: " << skippedVtable;
    er2::Log(ss.str());
  }
  {
    std::stringstream ss;
    ss << "  Skipped Range: " << skippedRange;
    er2::Log(ss.str());
  }
  {
    std::stringstream ss;
    ss << "  Accepted: " << acceptedCount;
    er2::Log(ss.str());
  }
  {
    std::stringstream ss;
    ss << "  Native Fallback: " << nativeFallbackCount;
    er2::Log(ss.str());
  }

  return true;
}

} // namespace er2
