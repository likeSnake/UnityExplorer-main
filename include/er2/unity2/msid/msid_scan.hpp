#pragma once

#include <cstddef>

#include <cstdint>

#include <cstring>

#include <string>

#include <vector>

#include "../../os/win/win_module.hpp"

#include "../core/offsets.hpp"

#include "../metadata/pe.hpp"

#include "../gom/gom_offsets.hpp"

#include "../gom/gom_walker.hpp"

#include "ms_id_to_pointer.hpp"

#include "../../mem/memory_read.hpp"

namespace er2

{

inline bool IsReadableByte(const IMemoryAccessor &mem, std::uintptr_t address)

{

  if (!address)

  {

    return false;
  }

  std::uint8_t b = 0;

  return mem.Read(address, &b, 1);
}

// Validate Unity Object via Managed<->Native cross-reference

inline bool ValidateUnityObject(

    const IMemoryAccessor &mem,

    std::uintptr_t obj,

    const GomOffsets &gomOff)

{

  if (!IsCanonicalUserPtr(obj))

  {

    return false;
  }

  std::uintptr_t managed = 0;

  if (!GetManagedFromNative(mem, obj, gomOff, managed))

  {

    return true;
  }

  if (!IsCanonicalUserPtr(managed))

  {

    return false;
  }

  constexpr std::size_t kManagedNativeOffset = 0x10;

  std::uintptr_t native = 0;

  if (!ReadPtr(mem, managed + kManagedNativeOffset, native))

  {

    return false;
  }

  return native == obj;
}

inline std::uint32_t CountUnityObjectsInMsIdEntriesPool(

    const IMemoryAccessor &mem,

    std::uintptr_t entriesBase,

    std::uint32_t capacity,

    std::uint32_t expectedCount,

    const GomOffsets &gomOff)

{

  if (!IsCanonicalUserPtr(entriesBase))

  {

    return 0;
  }

  if (expectedCount == 0 || expectedCount > 5000000)

  {

    return 0;
  }

  if (capacity == 0 || capacity > 20000000)

  {

    return 0;
  }

  constexpr std::uint32_t kBatchSize = 10000;

  std::vector<std::uint8_t> batch;

  batch.resize(static_cast<std::size_t>(kBatchSize) *
               static_cast<std::size_t>(kMsIdEntryStride));

  std::uint32_t objCount = 0;

  for (std::uint32_t batchStart = 0; batchStart < capacity;
       batchStart += kBatchSize)

  {

    const std::uint32_t batchEnd = (batchStart + kBatchSize < capacity)
                                       ? (batchStart + kBatchSize)
                                       : capacity;

    const std::uint32_t batchCount = batchEnd - batchStart;

    const std::size_t batchSize = static_cast<std::size_t>(batchCount) *
                                  static_cast<std::size_t>(kMsIdEntryStride);

    const std::uintptr_t batchAddr =
        entriesBase + static_cast<std::uintptr_t>(batchStart) *
                          static_cast<std::uintptr_t>(kMsIdEntryStride);

    if (!mem.Read(batchAddr, batch.data(), batchSize))

    {

      continue;
    }

    // [Super Optimized] Full Batch Scatter Validation
    // We collect all potential objects in this batch and validate them in
    // parallel using 2 stages of Scatter Reads. This reduces 20,000+ IO ops to
    // ~2 ops.

    struct ValidationCandidate {
      std::uintptr_t obj;
      std::uintptr_t managed;    // Stage 1 Result
      std::uintptr_t nativeBack; // Stage 2 Result
      bool validStage1;
    };
    std::vector<ValidationCandidate> candidates;
    candidates.reserve(batchCount);

    // 1. Parse Batch & Collect Candidates
    for (std::uint32_t i = 0; i < batchCount; ++i) {
      const std::size_t entryOff = static_cast<std::size_t>(i) *
                                   static_cast<std::size_t>(kMsIdEntryStride);

      std::uint32_t hashMask = 0;
      std::memcpy(&hashMask,
                  batch.data() + entryOff +
                      static_cast<std::size_t>(kMsIdEntryHashMask),
                  sizeof(hashMask));

      if (hashMask == 0xFFFFFFFFu || hashMask == 0xFFFFFFFEu) {
        continue;
      }

      std::uintptr_t obj = 0;
      std::memcpy(&obj,
                  batch.data() + entryOff +
                      static_cast<std::size_t>(kMsIdEntryObject),
                  sizeof(obj));

      if (IsCanonicalUserPtr(obj)) {
        candidates.push_back({obj, 0, 0, false});
      }
    }

    if (candidates.empty())
      continue;

    // 2. Scatter Stage 1: Read Managed Pointer from Native Object
    // managed = *(obj + off.game_object.managed)
    for (auto &cand : candidates) {
      mem.PrepareScatterRead(cand.obj + gomOff.game_object.managed,
                             &cand.managed, sizeof(std::uintptr_t));
    }
    mem.ExecuteScatterRead();

    // 3. Filter & Prepare Stage 2
    // Check if managed pointer looks valid, then prepare to read back-pointer
    std::vector<ValidationCandidate *> stage2Candidates;
    stage2Candidates.reserve(candidates.size());

    for (auto &cand : candidates) {
      if (cand.managed != 0 && IsCanonicalUserPtr(cand.managed)) {
        cand.validStage1 = true;
        // Scatter Stage 2: Read Native Back-Pointer from Managed Object
        // nativeBack = *(managed + 0x10)
        mem.PrepareScatterRead(cand.managed + 0x10, &cand.nativeBack,
                               sizeof(std::uintptr_t));
        stage2Candidates.push_back(&cand);
      }
    }

    if (stage2Candidates.empty())
      continue;

    // 4. Scatter Stage 2 Execution
    mem.ExecuteScatterRead();

    // 5. Final In-Memory Validation
    for (auto *cand : stage2Candidates) {
      if (cand->nativeBack == cand->obj) {
        ++objCount;
      }
    }

    if (objCount >= expectedCount)

    {

      break;
    }
  }

  return objCount;
}

inline bool FindMsIdToPointerSlotVaByScan(
    const IMemoryAccessor &mem, const ModuleInfo &unityPlayer,
    const GomOffsets &gomOff, std::uintptr_t &outMsIdToPointerSlotVa,
    std::uint32_t *outBestObjCount = nullptr) {
  outMsIdToPointerSlotVa = 0;
  if (outBestObjCount) {
    *outBestObjCount = 0;
  }

  std::uint32_t sizeOfImage = 0;
  std::vector<ModuleSection> sections;
  if (!ReadModuleSections(mem, unityPlayer.base, sizeOfImage, sections)) {
    return false;
  }

  std::uintptr_t bestAddr = 0;
  std::uint32_t bestObjCount = 0;

  for (const auto &s : sections) {
    if (!(s.name == ".data" || s.name == ".rdata")) {
      continue;
    }
    if (s.size == 0) {
      continue;
    }

    const std::uintptr_t secVa =
        unityPlayer.base + static_cast<std::uintptr_t>(s.rva);

    // [Super Optimized] Pipeline Scatter Scan Strategy
    // Stage 1: Reading 2MB SuperChunks
    constexpr std::size_t kSuperChunkSize = 0x200000; // 2MB
    constexpr std::size_t kPageSize = 0x1000;         // 4KB

    for (std::size_t superOffset = 0; superOffset < s.size;
         superOffset += kSuperChunkSize) {
      std::size_t superBytes = kSuperChunkSize;
      if (superOffset + superBytes > s.size) {
        superBytes = s.size - superOffset;
      }

      // --- Pipeline Stage 1: SuperChunk Batch Read ---
      std::vector<std::vector<std::uint8_t>> pageBuffers;
      std::size_t numPages = (superBytes + kPageSize - 1) / kPageSize;
      pageBuffers.resize(numPages);

      for (std::size_t p = 0; p < numPages; ++p) {
        pageBuffers[p].resize(kPageSize);
        std::uintptr_t pageAddr =
            secVa + superOffset + static_cast<std::uintptr_t>(p * kPageSize);
        std::size_t readSize = kPageSize;
        if (p == numPages - 1) {
          std::size_t remain = superBytes - p * kPageSize;
          if (remain < kPageSize)
            readSize = remain;
        }
        mem.PrepareScatterRead(pageAddr, pageBuffers[p].data(), readSize);
      }
      mem.ExecuteScatterRead();

      // --- Pipeline Stage 2: Candidate Hunt ---
      struct HeaderCandidate {
        std::uintptr_t ptr;          // The pointer found in .data
        std::uint8_t baseData[0x10]; // The header data at that pointer
        std::size_t offsetAbs;       // Where we found it in .data

        // Parsed fields
        std::uintptr_t entriesBase;
        std::uint32_t capacity;
        std::uint32_t count;

        // Batch Buffer
        std::vector<std::uint8_t> batchBuf;
      };

      std::vector<HeaderCandidate> headerCandidates;
      headerCandidates.reserve(numPages * 16);

      for (std::size_t p = 0; p < numPages; ++p) {
        std::size_t pageOffsetBase = superOffset + p * kPageSize;
        std::size_t validBytes = kPageSize;
        if (p == numPages - 1) {
          std::size_t remain = superBytes - p * kPageSize;
          if (remain < kPageSize)
            validBytes = remain;
        }

        for (std::size_t i = 0; i + sizeof(std::uintptr_t) <= validBytes;
             i += sizeof(std::uintptr_t)) {
          std::uintptr_t ptr = 0;
          std::memcpy(&ptr, pageBuffers[p].data() + i, sizeof(ptr));
          if (!IsCanonicalUserPtr(ptr)) {
            continue;
          }
          HeaderCandidate cand;
          cand.ptr = ptr;
          cand.offsetAbs = pageOffsetBase + i;
          headerCandidates.push_back(cand);
        }
      }

      if (headerCandidates.empty())
        continue;

      // --- Pipeline Stage 3: Read Headers ---
      for (auto &cand : headerCandidates) {
        mem.PrepareScatterRead(cand.ptr, cand.baseData, sizeof(cand.baseData));
      }
      mem.ExecuteScatterRead();

      // --- Pipeline Stage 4: Filter & Prepare Batch Read ---
      std::vector<HeaderCandidate *> validBatchCandidates;
      validBatchCandidates.reserve(headerCandidates.size());

      for (auto &cand : headerCandidates) {
        std::memcpy(&cand.entriesBase,
                    cand.baseData +
                        static_cast<std::size_t>(kMsIdSetEntriesBase),
                    sizeof(cand.entriesBase));
        if (!IsCanonicalUserPtr(cand.entriesBase))
          continue;

        std::memcpy(&cand.capacity,
                    cand.baseData + static_cast<std::size_t>(kMsIdSetCapacity),
                    sizeof(cand.capacity));
        std::memcpy(&cand.count,
                    cand.baseData + static_cast<std::size_t>(kMsIdSetCount),
                    sizeof(cand.count));

        if (cand.count == 0 || cand.count > 5000000)
          continue;
        if (cand.capacity == 0 || cand.capacity > 20000000)
          continue;

        // Found a plausible candidate. Instead of IsReadableByte(sync), we TRY
        // to scatter read the batch. We read only the first batch (10000
        // entries max) to validate.

        constexpr std::uint32_t kBatchSize = 10000;
        std::size_t entriesBytes = static_cast<std::size_t>(kBatchSize) *
                                   static_cast<std::size_t>(kMsIdEntryStride);
        cand.batchBuf.resize(entriesBytes);

        mem.PrepareScatterRead(cand.entriesBase, cand.batchBuf.data(),
                               entriesBytes);
        validBatchCandidates.push_back(&cand);
      }

      if (validBatchCandidates.empty())
        continue;

      // --- Pipeline Stage 5: Execute Batch Read ---
      mem.ExecuteScatterRead();

      // --- Pipeline Stage 6: Parse Objects & Prepare Validation ---
      struct ObjectValidation {
        std::uintptr_t objPtr;
        std::uintptr_t managedPtr;
        std::uintptr_t nativePtr;
        size_t candidateIndex; // Index in validBatchCandidates
      };

      std::vector<ObjectValidation> objValidations;
      objValidations.reserve(validBatchCandidates.size() * 100);

      constexpr std::uint32_t kBatchSize = 10000;

      for (size_t cIdx = 0; cIdx < validBatchCandidates.size(); ++cIdx) {
        auto *cand = validBatchCandidates[cIdx];
        // Determine how many entries to check. kBatchSize or capacity if
        // smaller. Note: batchBuf was sized to kBatchSize, so we check up to
        // that.
        std::uint32_t checkCount = kBatchSize;
        if (cand->capacity < checkCount)
          checkCount = cand->capacity;

        for (std::uint32_t i = 0; i < checkCount; ++i) {
          const std::size_t entryOff = i * kMsIdEntryStride;
          // Check Hash
          std::uint32_t hashMask = 0;
          std::memcpy(&hashMask,
                      cand->batchBuf.data() + entryOff + kMsIdEntryHashMask,
                      sizeof(hashMask));
          if (hashMask == 0xFFFFFFFFu || hashMask == 0xFFFFFFFEu)
            continue;

          // Check Object Ptr
          std::uintptr_t obj = 0;
          std::memcpy(&obj, cand->batchBuf.data() + entryOff + kMsIdEntryObject,
                      sizeof(obj));

          if (IsCanonicalUserPtr(obj)) {
            objValidations.push_back({obj, 0, 0, cIdx});
          }
        }
      }

      if (objValidations.empty())
        continue;

      // --- Pipeline Stage 7: Read Managed Ptr (Scatter) ---
      for (auto &v : objValidations) {
        mem.PrepareScatterRead(v.objPtr + gomOff.game_object.managed,
                               &v.managedPtr, sizeof(std::uintptr_t));
      }
      mem.ExecuteScatterRead();

      // --- Pipeline Stage 8: Read Native Ptr (Scatter) ---
      for (auto &v : objValidations) {
        if (IsCanonicalUserPtr(v.managedPtr)) {
          mem.PrepareScatterRead(v.managedPtr + 0x10, &v.nativePtr,
                                 sizeof(std::uintptr_t));
        }
      }
      mem.ExecuteScatterRead();

      // --- Pipeline Stage 9: Tally Votes ---
      // We use a temporary map or vector to hold valid counts for each
      // candidate
      std::vector<std::uint32_t> candidateScores(validBatchCandidates.size(),
                                                 0);

      for (const auto &v : objValidations) {
        if (v.nativePtr == v.objPtr && v.objPtr != 0) {
          candidateScores[v.candidateIndex]++;
        }
      }

      // --- Final Decision ---
      for (size_t i = 0; i < validBatchCandidates.size(); ++i) {
        if (candidateScores[i] >= validBatchCandidates[i]->count) {
          // Exact match or more (unlikely more) -> perfect!
          // Actually we only checked the first batch, so score will be <=
          // count. We need a threshold. If we found valid objects >=
          // min(expected, checked), it's good. But usually, finding > 0 valid
          // objects is already a very strong sign. Let's stick to the
          // comparison logic.
        }

        if (candidateScores[i] > bestObjCount) {
          bestObjCount = candidateScores[i];
          bestAddr = secVa + validBatchCandidates[i]->offsetAbs;
        }
        // Alternative: if the calculated count based on pool header is
        // plausible and we validated some. Trust the header count if we
        // validated a good chunk.
        if (candidateScores[i] >
            10) { // If we validated at least 10 objects successfully
                  // Trust the header count
          if (validBatchCandidates[i]->count > bestObjCount) {
            bestObjCount = validBatchCandidates[i]->count;
            bestAddr = secVa + validBatchCandidates[i]->offsetAbs;
          }
        }
      }
    }
  }

  if (!bestAddr) {
    return false;
  }

  outMsIdToPointerSlotVa = bestAddr;
  if (outBestObjCount) {
    *outBestObjCount = bestObjCount;
  }
  return true;
}

} // namespace er2
