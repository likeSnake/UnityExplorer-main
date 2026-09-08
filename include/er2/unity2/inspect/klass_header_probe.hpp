#pragma once

#include "runtime_class_probe.hpp"

#include <algorithm>
#include <cctype>
#include <cstdint>
#include <limits>
#include <string>
#include <vector>

namespace er2 {

struct KlassHeaderStringEntry {
  std::uint32_t offset = 0;
  std::uintptr_t pointer = 0;
  bool readable = false;
  std::string value;
  int score = 0;
  std::string reason;
};

struct KlassHeaderProbeResult {
  bool valid = false;
  std::uintptr_t klass = 0;
  std::uint32_t bestNameOffset = std::numeric_limits<std::uint32_t>::max();
  std::uint32_t bestNamespaceOffset =
      std::numeric_limits<std::uint32_t>::max();
  std::uintptr_t bestNamePointer = 0;
  std::uintptr_t bestNamespacePointer = 0;
  std::string bestName;
  std::string bestNamespace;
  std::vector<KlassHeaderStringEntry> entries;
  std::string failureReason;
};

namespace detail_klass_probe {

inline bool IsPrintableAsciiString(const std::string &value) {
  if (value.empty() || value.size() > 192) {
    return false;
  }

  for (unsigned char ch : value) {
    if (ch < 0x20 || ch > 0x7E) {
      return false;
    }
  }

  return true;
}

inline bool IsIdentifierChar(unsigned char ch) {
  return std::isalnum(ch) != 0 || ch == '_' || ch == '`';
}

inline bool IsIdentifierLike(const std::string &value) {
  if (value.empty()) {
    return false;
  }

  const unsigned char first = static_cast<unsigned char>(value.front());
  if (std::isdigit(first) != 0 || value.front() == '.') {
    return false;
  }

  for (unsigned char ch : value) {
    if (!IsIdentifierChar(ch) && ch != '.' && ch != '+') {
      return false;
    }
  }

  return true;
}

inline int ScoreClassNameString(const std::string &value) {
  if (!IsPrintableAsciiString(value) || !IsIdentifierLike(value)) {
    return 0;
  }

  int score = 20;
  const unsigned char first = static_cast<unsigned char>(value.front());
  if (std::isupper(first) != 0) {
    score += 12;
  }
  if (value.find('.') == std::string::npos) {
    score += 8;
  }
  if (value.find("::") == std::string::npos) {
    score += 4;
  }

  const std::size_t len = value.size();
  score += static_cast<int>(std::min<std::size_t>(len, 24));
  if (len >= 3 && len <= 96) {
    score += 10;
  }
  if (value == "Object" || value == "Component" || value == "GameObject" ||
      value == "Transform" || value == "MonoBehaviour") {
    score += 8;
  }

  return score;
}

inline int ScoreClassNameSlot(const std::string &value, std::uint32_t offset) {
  int score = ScoreClassNameString(value);
  if (score <= 0) {
    return 0;
  }

  if (offset <= 0x40u) {
    score += 10;
  }
  if ((offset % 0x10u) == 0) {
    score += 6;
  }

  return score;
}

inline int ScoreNamespaceString(const std::string &value) {
  if (!IsPrintableAsciiString(value) || !IsIdentifierLike(value)) {
    return 0;
  }

  int score = 12;
  if (value.find('.') != std::string::npos) {
    score += 14;
  }
  if (value.find("Unity") != std::string::npos ||
      value.find("Game") != std::string::npos ||
      value.find("System") != std::string::npos) {
    score += 8;
  }

  const std::size_t len = value.size();
  score += static_cast<int>(std::min<std::size_t>(len, 24));
  if (len <= 128) {
    score += 4;
  }

  return score;
}

inline int ScoreNamespaceSlot(const std::string &value, std::uint32_t offset,
                              std::uint32_t nameOffset) {
  int score = ScoreNamespaceString(value);
  if (score <= 0) {
    return 0;
  }

  if (offset == nameOffset + sizeof(std::uintptr_t)) {
    score += 35;
  } else if (offset + sizeof(std::uintptr_t) == nameOffset) {
    score += 8;
  }

  return score;
}

inline std::string FormatStringReason(const std::string &value, int score) {
  if (value.empty()) {
    return "empty string";
  }

  if (!IsPrintableAsciiString(value)) {
    return "string contains non-printable bytes";
  }

  if (score <= 0) {
    return "string is readable but not identifier-like";
  }

  return "readable identifier-like string";
}

} // namespace detail_klass_probe

inline bool ProbeKlassHeaderStrings(const IMemoryAccessor &mem,
                                    std::uintptr_t klass,
                                    KlassHeaderProbeResult &out,
                                    std::uint32_t scanBytes = 0xC0u) {
  out = KlassHeaderProbeResult{};
  out.klass = klass;

  if (!IsCanonicalUserPtr(klass)) {
    out.failureReason = "klass is not a canonical user pointer";
    return false;
  }

  if (scanBytes < sizeof(std::uintptr_t)) {
    out.failureReason = "scan range is too small";
    return false;
  }

  scanBytes = std::min<std::uint32_t>(scanBytes, 0x200u);
  int bestNameScore = 0;
  int bestNamespaceScore = 0;

  for (std::uint32_t offset = 0; offset + sizeof(std::uintptr_t) <= scanBytes;
       offset += static_cast<std::uint32_t>(sizeof(std::uintptr_t))) {
    KlassHeaderStringEntry entry;
    entry.offset = offset;

    if (!ReadPtr(mem, klass + offset, entry.pointer)) {
      entry.reason = "pointer slot is unreadable";
      out.entries.push_back(std::move(entry));
      continue;
    }

    if (!IsCanonicalUserPtr(entry.pointer)) {
      entry.reason = "pointer is not canonical";
      out.entries.push_back(std::move(entry));
      continue;
    }

    entry.readable = ReadRuntimeCString(mem, entry.pointer, entry.value, 192);
    if (!entry.readable) {
      entry.reason = "string is unreadable";
      out.entries.push_back(std::move(entry));
      continue;
    }

    const int classScore =
        detail_klass_probe::ScoreClassNameSlot(entry.value, entry.offset);
    const int namespaceScore =
        detail_klass_probe::ScoreNamespaceString(entry.value);
    entry.score = std::max(classScore, namespaceScore);
    entry.reason =
        detail_klass_probe::FormatStringReason(entry.value, entry.score);
    out.entries.push_back(std::move(entry));
  }

  int bestPairScore = 0;
  for (const auto &nameEntry : out.entries) {
    if (!nameEntry.readable || nameEntry.value.empty()) {
      continue;
    }

    const int nameScore = detail_klass_probe::ScoreClassNameSlot(
        nameEntry.value, nameEntry.offset);
    if (nameScore <= 0) {
      continue;
    }

    const KlassHeaderStringEntry *namespaceEntry = nullptr;
    int namespaceScore = 0;
    for (const auto &candidateNs : out.entries) {
      if (!candidateNs.readable || candidateNs.value.empty() ||
          candidateNs.offset == nameEntry.offset) {
        continue;
      }

      const int candidateScore = detail_klass_probe::ScoreNamespaceSlot(
          candidateNs.value, candidateNs.offset, nameEntry.offset);
      if (candidateScore > namespaceScore) {
        namespaceScore = candidateScore;
        namespaceEntry = &candidateNs;
      }
    }

    int pairScore = nameScore;
    if (namespaceEntry != nullptr) {
      pairScore += namespaceScore;
      if (namespaceEntry->offset ==
          nameEntry.offset + sizeof(std::uintptr_t)) {
        pairScore += 80;
      }
    }

    if (pairScore > bestPairScore) {
      bestPairScore = pairScore;
      bestNameScore = nameScore;
      out.bestNameOffset = nameEntry.offset;
      out.bestNamePointer = nameEntry.pointer;
      out.bestName = nameEntry.value;

      bestNamespaceScore = namespaceScore;
      if (namespaceEntry != nullptr) {
        out.bestNamespaceOffset = namespaceEntry->offset;
        out.bestNamespacePointer = namespaceEntry->pointer;
        out.bestNamespace = namespaceEntry->value;
      } else {
        out.bestNamespaceOffset = std::numeric_limits<std::uint32_t>::max();
        out.bestNamespacePointer = 0;
        out.bestNamespace.clear();
      }
    }
  }

  if (bestNameScore <= 0) {
    out.failureReason = "no readable class-like string found in klass header";
    return false;
  }

  (void)bestNamespaceScore;
  out.valid = true;
  return true;
}

} // namespace er2
