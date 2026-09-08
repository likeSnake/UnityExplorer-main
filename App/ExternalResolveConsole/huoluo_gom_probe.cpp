#ifndef NOMINMAX
#define NOMINMAX
#endif

#include "../../Resolve202x.hpp"
#include "../UnityExplorer/MetickAdapter.hpp"

#include "er2/unity2/gom/gom_scan.hpp"
#include "er2/unity2/init/component.hpp"
#include "er2/unity2/init/gom.hpp"
#include "er2/unity2/init/module_match.hpp"
#include "er2/unity2/init/object.hpp"
#include "er2/unity2/init/transform.hpp"
#include "er2/unity2/msid/enumerate_objects.hpp"
#include "er2/unity2/msid/msid_scan.hpp"
#include "er2/unity2/object/managed/managed_object.hpp"
#include "er2/unity2/object/native/native_component.hpp"
#include "er2/unity2/object/native/native_game_object.hpp"

#include <Windows.h>

#include <algorithm>
#include <chrono>
#include <cmath>
#include <cstdint>
#include <iomanip>
#include <iostream>
#include <memory>
#include <sstream>
#include <string>
#include <unordered_set>
#include <vector>

namespace {

struct Vec3 {
  float x = 0.0f;
  float y = 0.0f;
  float z = 0.0f;
};

struct Quat {
  float x = 0.0f;
  float y = 0.0f;
  float z = 0.0f;
  float w = 1.0f;
};

struct NarakaOffsets {
  std::uint64_t m_CharacterManager = 0;
  std::uint64_t m_TransparentTagHandler = 0;
  std::uint32_t actorTransform = 0x128;
};

struct ModuleChoice {
  er2::UnityModuleCandidate normal;
  er2::UnityModuleCandidate super;
  er2::UnityModuleCandidate ibt;
  er2::UnityModuleCandidate selected;
};

struct TransformSnapshot {
  std::uintptr_t transform = 0;
  std::uintptr_t nodeData = 0;
  std::uintptr_t parentIndices = 0;
  std::int32_t index = -1;
  std::int32_t parent = -1;
  Vec3 localPos{};
  Quat localRot{};
  Vec3 localScale{};
  Vec3 localEuler{};
};

struct CandidateRow {
  float dist = 0.0f;
  std::uintptr_t native = 0;
  std::uintptr_t managed = 0;
  std::uintptr_t transform = 0;
  std::string name;
  std::int32_t tag = 0;
  std::int32_t compCount = 0;
  Vec3 world{};
  TransformSnapshot tr{};
  bool keyword = false;
  bool hasWorld = false;
  std::vector<std::int32_t> typeIds;
};

struct SceneDeviceSnapshot {
  bool ok = false;
  std::uintptr_t model = 0;
  std::uintptr_t klass = 0;
  std::string className;
  std::uintptr_t propertyData = 0;
  std::uintptr_t gameObject = 0;
  std::uintptr_t fastTransform = 0;
  std::uintptr_t deviceKit = 0;
  std::uintptr_t deviceComponent = 0;
  std::uintptr_t configData = 0;
  std::int32_t fid = 0;
  std::int32_t fighterDeviceState = 0;
  std::int32_t fighterDeviceStateHash = 0;
  std::int32_t propDeviceType = 0;
  std::int32_t propTid = 0;
  std::int32_t propState = 0;
  std::int32_t propGroup = 0;
  std::int32_t propStateHash = 0;
  std::int32_t propParentFid = 0;
  Vec3 propPosition{};
  Quat propQuaternion{};
  std::vector<std::int32_t> extraInts;
  std::vector<std::int32_t> childFids;
  std::int32_t configId = 0;
  std::int32_t configType = 0;
  std::string prefabPath;
};

std::string Hex(std::uint64_t value) {
  std::ostringstream oss;
  oss << "0x" << std::hex << std::uppercase << value;
  return oss.str();
}

bool IsUserPtr(std::uintptr_t value) {
  return value >= 0x10000ull && value < 0x0000800000000000ull;
}

template <typename T>
bool ReadValue(std::uintptr_t address, T &out) {
  out = {};
  return IsUserPtr(address) && er2::Mem().Read(address, &out, sizeof(T));
}

bool ReadPtr(std::uintptr_t address, std::uintptr_t &out) {
  out = 0;
  return ReadValue(address, out) && IsUserPtr(out);
}

std::string ToLower(std::string s) {
  std::transform(s.begin(), s.end(), s.begin(), [](unsigned char ch) {
    return static_cast<char>(std::tolower(ch));
  });
  return s;
}

std::string ManagedClassName(std::uintptr_t managed, std::uintptr_t *klassOut = nullptr) {
  if (klassOut) {
    *klassOut = 0;
  }

  std::uintptr_t klass = 0;
  if (!er2::ReadManagedObjectKlass(er2::Mem(), managed, er2::g_ctx.off,
                                   klass)) {
    return "";
  }
  if (klassOut) {
    *klassOut = klass;
  }

  std::string ns;
  std::string name;
  if (!er2::ReadIl2CppClassNameAdaptive(er2::Mem(), klass, er2::g_ctx.off, ns,
                                        name)) {
    return "";
  }
  return ns.empty() ? name : (ns + "." + name);
}

bool ManagedIsClassOrParent(std::uintptr_t managed, const char *name) {
  std::uintptr_t klass = 0;
  if (!er2::ReadManagedObjectKlass(er2::Mem(), managed, er2::g_ctx.off,
                                   klass)) {
    return false;
  }
  return er2::IsClassOrParent(er2::Mem(), klass, er2::g_ctx.off, "", name);
}

const char *PuzzleStateName(std::int32_t state) {
  switch (state) {
  case 0:
    return "IDLE_01";
  case 1:
    return "IDLE_02";
  case 2:
    return "IDLE_03";
  case 3:
    return "IDLE_04";
  case 4:
    return "ROTATE_0102";
  case 5:
    return "ROTATE_0203";
  case 6:
    return "ROTATE_0304";
  case 7:
    return "ROTATE_0401";
  case 8:
    return "CLOSE";
  default:
    return "UNKNOWN";
  }
}

std::string ReadIl2CppString(std::uintptr_t stringObj) {
  if (!IsUserPtr(stringObj)) {
    return "";
  }

  std::int32_t length = 0;
  if (!ReadValue(stringObj + 0x10, length) || length <= 0 || length > 512) {
    return "";
  }

  std::vector<wchar_t> chars(static_cast<std::size_t>(length) + 1);
  if (!er2::Mem().Read(stringObj + 0x14, chars.data(),
                       static_cast<std::size_t>(length) * sizeof(wchar_t))) {
    return "";
  }

  std::string out;
  out.reserve(static_cast<std::size_t>(length));
  for (int i = 0; i < length; ++i) {
    const wchar_t ch = chars[static_cast<std::size_t>(i)];
    if (ch >= 0x20 && ch <= 0x7E) {
      out.push_back(static_cast<char>(ch));
    } else if (ch == L'\t') {
      out.push_back('\t');
    } else {
      out.push_back('?');
    }
  }
  return out;
}

std::vector<std::int32_t> ReadIntList(std::uintptr_t listObj,
                                      std::size_t maxItems = 16) {
  std::vector<std::int32_t> out;
  if (!IsUserPtr(listObj)) {
    return out;
  }

  std::uintptr_t items = 0;
  std::int32_t size = 0;
  if (!ReadPtr(listObj + 0x10, items) || !ReadValue(listObj + 0x18, size) ||
      size <= 0 || size > 4096 || !IsUserPtr(items)) {
    return out;
  }

  const std::size_t limit =
      std::min<std::size_t>(static_cast<std::size_t>(size), maxItems);
  out.reserve(limit);
  for (std::size_t i = 0; i < limit; ++i) {
    std::int32_t value = 0;
    if (!ReadValue(items + 0x20 + i * sizeof(std::int32_t), value)) {
      break;
    }
    out.push_back(value);
  }
  return out;
}

bool ReadSceneDeviceSnapshot(std::uintptr_t model, SceneDeviceSnapshot &out) {
  out = {};
  if (!IsUserPtr(model)) {
    return false;
  }

  std::uintptr_t klass = 0;
  const std::string className = ManagedClassName(model, &klass);
  if (className.empty()) {
    return false;
  }

  const bool isSceneDevice = ManagedIsClassOrParent(model, "SceneDeviceModel");
  const bool isPuzzle = ManagedIsClassOrParent(model, "PuzzlesDevice") ||
                        ManagedIsClassOrParent(model, "PuzzlesMainDevice");
  if (!isSceneDevice && !isPuzzle) {
    return false;
  }

  out.ok = true;
  out.model = model;
  out.klass = klass;
  out.className = className;

  (void)ReadValue(model + 0x58, out.fid);
  (void)ReadPtr(model + 0x60, out.propertyData);
  (void)ReadPtr(model + 0x70, out.gameObject);
  (void)ReadPtr(model + 0x90, out.fastTransform);
  (void)ReadPtr(model + 0xA0, out.deviceKit);
  (void)ReadValue(model + 0xC8, out.fighterDeviceState);
  (void)ReadValue(model + 0xCC, out.fighterDeviceStateHash);
  (void)ReadPtr(model + 0x110, out.configData);
  (void)ReadPtr(model + 0x138, out.deviceComponent);

  if (IsUserPtr(out.propertyData)) {
    (void)ReadValue(out.propertyData + 0xD8, out.propDeviceType);
    (void)ReadValue(out.propertyData + 0xDC, out.propTid);
    (void)ReadValue(out.propertyData + 0xE0, out.propState);
    (void)ReadValue(out.propertyData + 0xE4, out.propGroup);
    (void)ReadValue(out.propertyData + 0xE8, out.propStateHash);
    (void)ReadValue(out.propertyData + 0xF4, out.propParentFid);
    (void)ReadValue(out.propertyData + 0x100, out.propPosition);
    (void)ReadValue(out.propertyData + 0x144, out.propQuaternion);

    std::uintptr_t extra = 0;
    if (ReadPtr(out.propertyData + 0x158, extra)) {
      out.extraInts = ReadIntList(extra);
    }
    std::uintptr_t childFids = 0;
    if (ReadPtr(out.propertyData + 0x160, childFids)) {
      out.childFids = ReadIntList(childFids);
    }
  }

  if (IsUserPtr(out.configData)) {
    (void)ReadValue(out.configData + 0x10, out.configId);
    (void)ReadValue(out.configData + 0x14, out.configType);
    std::uintptr_t prefab = 0;
    if (ReadPtr(out.configData + 0x50, prefab)) {
      out.prefabPath = ReadIl2CppString(prefab);
    }
  }

  return true;
}

void PrintIntVector(const std::vector<std::int32_t> &values) {
  std::cout << "[";
  for (std::size_t i = 0; i < values.size(); ++i) {
    if (i) {
      std::cout << ",";
    }
    std::cout << values[i];
  }
  std::cout << "]";
}

void PrintSceneDeviceSnapshot(const SceneDeviceSnapshot &s,
                              const std::string &source) {
  std::cout << "[SCENE_DEVICE] source=" << source << " model=" << Hex(s.model)
            << " klass=" << Hex(s.klass) << " class=\"" << s.className
            << "\" fid=" << s.fid << " prop=" << Hex(s.propertyData)
            << " gameObject=" << Hex(s.gameObject)
            << " fastTransform=" << Hex(s.fastTransform)
            << " deviceComponent=" << Hex(s.deviceComponent)
            << " state=" << s.propState << "(" << PuzzleStateName(s.propState)
            << ") stateHash=" << s.propStateHash
            << " fighterState=" << s.fighterDeviceState << "("
            << PuzzleStateName(s.fighterDeviceState)
            << ") fighterHash=" << s.fighterDeviceStateHash
            << " deviceType=" << s.propDeviceType << " tid=" << s.propTid
            << " group=" << s.propGroup << " parentFid=" << s.propParentFid
            << " config=" << Hex(s.configData) << " configId=" << s.configId
            << " configType=" << s.configType << " prefab=\""
            << s.prefabPath << "\" propPos=(" << s.propPosition.x << ","
            << s.propPosition.y << "," << s.propPosition.z << ") propQ=("
            << s.propQuaternion.x << "," << s.propQuaternion.y << ","
            << s.propQuaternion.z << "," << s.propQuaternion.w
            << ") extraInts=";
  PrintIntVector(s.extraInts);
  std::cout << " childFids=";
  PrintIntVector(s.childFids);
  std::cout << std::endl;
}

bool IsPuzzleObjectName(const std::string &name) {
  const std::string s = ToLower(name);
  return s.find("device_puzzles") != std::string::npos ||
         s.find("puzzles_answer") != std::string::npos ||
         s.find("public_device_puzzles") != std::string::npos ||
         s.find("shengying") != std::string::npos ||
         s.find("shenying") != std::string::npos;
}

void PrintPuzzleComponentDetails(const CandidateRow &r) {
  if (!IsPuzzleObjectName(r.name)) {
    return;
  }

  std::uintptr_t pool = 0;
  std::int32_t count = 0;
  if (!er2::GetComponentPool(er2::Mem(), r.native, er2::g_ctx.gomOff, pool) ||
      !ReadValue(r.native + er2::g_ctx.gomOff.game_object.component_count,
                 count) ||
      count <= 0) {
    std::cout << "[PUZZLE_COMPONENTS] native=" << Hex(r.native)
              << " name=\"" << r.name << "\" failed" << std::endl;
    return;
  }

  if (count > 64) {
    count = 64;
  }

  std::cout << "[PUZZLE_COMPONENTS] native=" << Hex(r.native) << " name=\""
            << r.name << "\" pool=" << Hex(pool) << " count=" << count
            << " world=(" << r.world.x << "," << r.world.y << ","
            << r.world.z << ")" << std::endl;

  std::unordered_set<std::uintptr_t> printedModels;
  for (std::int32_t i = 0; i < count; ++i) {
    std::int32_t slotTypeId = 0;
    std::uintptr_t nativeComp = 0;
    (void)er2::GetComponentSlotTypeId(er2::Mem(), pool, er2::g_ctx.gomOff, i,
                                      slotTypeId);
    if (!er2::GetComponentSlotNative(er2::Mem(), pool, er2::g_ctx.gomOff, i,
                                     nativeComp)) {
      continue;
    }

    std::uintptr_t managedComp = 0;
    std::string typeName;
    if (er2::GetNativeComponentManaged(er2::Mem(), nativeComp,
                                       er2::g_ctx.gomOff, managedComp)) {
      typeName = ManagedClassName(managedComp);
    }
    const bool enabled =
        er2::IsComponentEnabled(er2::Mem(), nativeComp, er2::g_ctx.gomOff);

    std::cout << "[COMP] go=" << Hex(r.native) << " idx=" << i
              << " typeId=" << slotTypeId << " native=" << Hex(nativeComp)
              << " managed=" << Hex(managedComp) << " enabled="
              << (enabled ? 1 : 0) << " class=\"" << typeName << "\""
              << std::endl;

    SceneDeviceSnapshot ownModel;
    if (ReadSceneDeviceSnapshot(managedComp, ownModel) &&
        printedModels.insert(ownModel.model).second) {
      PrintSceneDeviceSnapshot(ownModel, "component_self[" + std::to_string(i) +
                                             "]");
    }

    std::uintptr_t scene = 0;
    if (ReadPtr(managedComp + 0x18, scene)) {
      SceneDeviceSnapshot model;
      if (ReadSceneDeviceSnapshot(scene, model) &&
          printedModels.insert(model.model).second) {
        PrintSceneDeviceSnapshot(model, "component_scene[" +
                                            std::to_string(i) + "]+0x18");
      } else if (IsUserPtr(scene)) {
        std::cout << "[COMP_SCENE_PTR] idx=" << i << " scene=" << Hex(scene)
                  << " class=\"" << ManagedClassName(scene) << "\""
                  << std::endl;
      }
    }
  }
}

float Dist(const Vec3 &a, const Vec3 &b) {
  const float dx = a.x - b.x;
  const float dy = a.y - b.y;
  const float dz = a.z - b.z;
  return std::sqrt(dx * dx + dy * dy + dz * dz);
}

float RadToDeg(float v) { return v * 57.29577951308232f; }

Vec3 QuatToEulerDeg(const Quat &q) {
  const float sinr_cosp = 2.0f * (q.w * q.x + q.y * q.z);
  const float cosr_cosp = 1.0f - 2.0f * (q.x * q.x + q.y * q.y);
  const float roll = std::atan2(sinr_cosp, cosr_cosp);

  const float sinp = 2.0f * (q.w * q.y - q.z * q.x);
  float pitch = 0.0f;
  if (std::fabs(sinp) >= 1.0f) {
    pitch = std::copysign(1.5707963267948966f, sinp);
  } else {
    pitch = std::asin(sinp);
  }

  const float siny_cosp = 2.0f * (q.w * q.z + q.x * q.y);
  const float cosy_cosp = 1.0f - 2.0f * (q.y * q.y + q.z * q.z);
  const float yaw = std::atan2(siny_cosp, cosy_cosp);
  return {RadToDeg(roll), RadToDeg(pitch), RadToDeg(yaw)};
}

bool HasKeyword(const std::string &name) {
  const std::string s = ToLower(name);
  static const char *const kWords[] = {
      "eagle",      "statue",   "shen",      "shenying",
      "shenhuo",    "huoluo",   "square",    "mains",
      "switch",     "device",   "puzzle",    "rotation",
      "rotate",     "golden",   "4013298",   "4013536",
      "4013540",    "4013541",  "4013542",   "4013543",
      "scene_device"};
  for (const char *word : kWords) {
    if (s.find(word) != std::string::npos) {
      return true;
    }
  }
  return false;
}

ModuleChoice ChooseModules(const std::vector<er2::UnityModuleCandidate> &mods) {
  ModuleChoice out;
  for (const auto &m : mods) {
    const std::string lower = ToLower(m.name);
    if (lower == "gameassembly.dll") {
      out.normal = m;
    } else if (lower == "gameassembly_super.dll") {
      out.super = m;
    } else if (lower == "gameassembly_super_ibt.dll") {
      out.ibt = m;
    }
  }
  if (out.super.base) {
    out.selected = out.super;
  } else if (out.ibt.base) {
    out.selected = out.ibt;
  } else {
    out.selected = out.normal;
  }
  return out;
}

NarakaOffsets OffsetsForModule(const std::string &moduleName) {
  const std::string lower = ToLower(moduleName);
  if (lower == "gameassembly_super.dll" ||
      lower == "gameassembly_super_ibt.dll") {
    return {0x0378C4B8ull, 0x37AB158ull, 0x128};
  }
  return {0xE9D96C8ull, 0xE8248D8ull, 0x128};
}

bool ResolveCharacterManager(std::uintptr_t gameAssemblyBase,
                             const NarakaOffsets &off, std::uintptr_t &out,
                             std::string &source) {
  out = 0;
  source.clear();

  std::uintptr_t klass = 0;
  std::uintptr_t stat = 0;
  std::uintptr_t inst = 0;
  std::uintptr_t cm = 0;
  if (ReadPtr(gameAssemblyBase + off.m_TransparentTagHandler, klass) &&
      ReadPtr(klass + 0xB8, stat) && ReadPtr(stat + 0x8, inst) &&
      ReadPtr(inst + 0xA8, cm)) {
    out = cm;
    source = "TransparentTagHandler.s_instance.refCharacterMgr";
    return true;
  }

  if (ReadPtr(gameAssemblyBase + off.m_CharacterManager, klass) &&
      ReadPtr(klass + 0xB8, stat) && ReadPtr(stat + 0x8, cm)) {
    out = cm;
    source = "CharacterManager._instance";
    return true;
  }

  return false;
}

bool ReadActorPos(std::uintptr_t actor, const NarakaOffsets &off, Vec3 &pos,
                  std::uintptr_t &actorRoot) {
  actorRoot = 0;
  std::uintptr_t transform = 0;
  std::uintptr_t cached = 0;
  if (ReadPtr(actor + off.actorTransform, transform) &&
      ReadPtr(transform + 0x10, cached) && ReadPtr(cached + 0x38, actorRoot) &&
      ReadValue(actorRoot + 0x90, pos)) {
    return true;
  }

  std::uintptr_t t = 0;
  if (ReadPtr(actor + off.actorTransform, t) && ReadPtr(t + 0x10, t) &&
      ReadPtr(t + 0x30, t) && ReadPtr(t + 0x30, t) &&
      ReadPtr(t + 0x08, t) && ReadPtr(t + 0x38, t) &&
      ReadValue(t + 0x90, pos)) {
    actorRoot = t;
    return true;
  }
  return false;
}

bool ReadTransformSnapshot(std::uintptr_t transform, TransformSnapshot &out) {
  out = {};
  out.transform = transform;
  er2::TransformHierarchyState state{};
  std::int32_t index = -1;
  if (!er2::ReadTransformHierarchyState(er2::Mem(), transform,
                                        er2::g_ctx.transformOff, state,
                                        index)) {
    return false;
  }

  float node[12] = {};
  const std::uintptr_t nodeAddr =
      state.nodeData + static_cast<std::uintptr_t>(index) *
                           er2::g_ctx.transformOff.node_stride;
  if (!er2::Mem().Read(nodeAddr, node, sizeof(node))) {
    return false;
  }

  std::int32_t parent = -1;
  ReadValue(state.parentIndices + static_cast<std::uintptr_t>(index) *
                                      sizeof(std::int32_t),
            parent);

  out.nodeData = state.nodeData;
  out.parentIndices = state.parentIndices;
  out.index = index;
  out.parent = parent;
  out.localPos = {node[0], node[1], node[2]};
  out.localRot = {node[4], node[5], node[6], node[7]};
  out.localScale = {node[8], node[9], node[10]};
  out.localEuler = QuatToEulerDeg(out.localRot);
  return true;
}

void PrintRow(const CandidateRow &r) {
  std::cout << "dist=" << std::setw(8) << r.dist
            << " keyword=" << (r.keyword ? 1 : 0)
            << " native=" << Hex(r.native) << " managed=" << Hex(r.managed)
            << " tag=" << r.tag << " comps=" << r.compCount
            << " transform=" << Hex(r.transform) << " pos=(" << r.world.x
            << "," << r.world.y << "," << r.world.z << ")"
            << " localEuler=(" << r.tr.localEuler.x << ","
            << r.tr.localEuler.y << "," << r.tr.localEuler.z << ")"
            << " localQ=(" << r.tr.localRot.x << "," << r.tr.localRot.y
            << "," << r.tr.localRot.z << "," << r.tr.localRot.w << ")"
            << " idx=" << r.tr.index << " parent=" << r.tr.parent
            << " typeIds=[";
  for (std::size_t i = 0; i < r.typeIds.size(); ++i) {
    if (i) {
      std::cout << ",";
    }
    std::cout << r.typeIds[i];
  }
  std::cout << "] name=\"" << r.name << "\"" << std::endl;
}

bool BuildCandidateRow(std::uintptr_t nativeObject, std::uintptr_t managedObject,
                       const Vec3 &localPos, float radius,
                       CandidateRow &out) {
  out = {};
  out.native = nativeObject;
  out.managed = managedObject;

  if (!er2::ReadNativeGameObjectName(er2::Mem(), nativeObject,
                                     er2::g_ctx.gomOff, out.name)) {
    out.name = "";
  }
  out.keyword = HasKeyword(out.name);

  er2::GetNativeGameObjectTag(er2::Mem(), nativeObject, er2::g_ctx.gomOff,
                              out.tag);
  er2::GetComponentCount(er2::Mem(), nativeObject, er2::g_ctx.gomOff,
                         out.compCount);
  er2::GetNativeGameObjectComponentTypeIds(er2::Mem(), nativeObject,
                                           er2::g_ctx.gomOff, out.typeIds);

  out.transform = er2::GetTransformComponent(nativeObject);
  if (!out.transform) {
    return out.keyword;
  }

  glm::vec3 pos{};
  if (er2::GetTransformWorldPosition(out.transform, pos)) {
    out.world = {pos.x, pos.y, pos.z};
    out.hasWorld = true;
    out.dist = Dist(localPos, out.world);
  }
  ReadTransformSnapshot(out.transform, out.tr);

  return out.keyword || (out.hasWorld && out.dist <= radius);
}

} // namespace

int main(int argc, char **argv) {
  float radius = 180.0f;
  if (argc >= 2) {
    radius = std::max(20.0f, std::strtof(argv[1], nullptr));
  }

  std::cout << std::fixed << std::setprecision(3);
  const auto start = std::chrono::steady_clock::now();

  er2::ResetContext();
  auto dma = std::make_shared<MetickAdapter>();
  const std::string target = "NarakaBladepoint.exe";
  std::cout << "[INIT] target=" << target << std::endl;
  if (!dma->Initialize(target)) {
    std::cout << "[FAIL] DMA init failed: " << dma->GetLastError()
              << std::endl;
    return 1;
  }
  if (dma->FixCr3()) {
    std::cout << "[INIT] FixCr3 ok" << std::endl;
  } else {
    std::cout << "[WARN] FixCr3 failed; continue" << std::endl;
  }

  er2::g_ctx.pid =
      static_cast<std::uint32_t>(mem::Get_Process_Id(target.c_str()));
  er2::g_ctx.memory = dma;
  if (!er2::g_ctx.pid) {
    std::cout << "[FAIL] pid not found" << std::endl;
    return 2;
  }
  std::cout << "[INIT] pid=" << er2::g_ctx.pid << std::endl;

  std::vector<er2::UnityModuleCandidate> modules;
  for (const auto &m : dma->EnumerateModules()) {
    modules.push_back({m.name, static_cast<std::uintptr_t>(m.baseAddress),
                       m.imageSize});
  }

  const auto runtimeModules = er2::FindUnityRuntimeModules(modules);
  const auto chosen = ChooseModules(modules);
  if (!runtimeModules.unityPlayer.base || !chosen.selected.base) {
    std::cout << "[FAIL] UnityPlayer/GameAssembly module missing"
              << " unity=" << Hex(runtimeModules.unityPlayer.base)
              << " ga=" << Hex(chosen.selected.base) << std::endl;
    return 3;
  }

  er2::g_ctx.unityPlayer.base = runtimeModules.unityPlayer.base;
  er2::g_ctx.unityPlayer.size = runtimeModules.unityPlayer.size;
  er2::g_ctx.unityPlayerRange.base = runtimeModules.unityPlayer.base;
  er2::g_ctx.unityPlayerRange.size = runtimeModules.unityPlayer.size;
  er2::g_ctx.gameAssembly.base = chosen.selected.base;
  er2::g_ctx.gameAssembly.size = chosen.selected.size;
  er2::g_ctx.runtime = er2::ManagedBackend::Il2Cpp;

  std::cout << "[MODULE] UnityPlayer " << runtimeModules.unityPlayer.name
            << " base=" << Hex(runtimeModules.unityPlayer.base)
            << " size=" << Hex(runtimeModules.unityPlayer.size) << std::endl;
  std::cout << "[MODULE] selected " << chosen.selected.name
            << " base=" << Hex(chosen.selected.base)
            << " size=" << Hex(chosen.selected.size)
            << " normal=" << Hex(chosen.normal.base)
            << " super=" << Hex(chosen.super.base)
            << " ibt=" << Hex(chosen.ibt.base) << std::endl;

  const NarakaOffsets narakaOff = OffsetsForModule(chosen.selected.name);
  std::uintptr_t cm = 0;
  std::string cmSource;
  Vec3 localPos{1015.734f, 95.162f, 665.269f};
  std::uintptr_t localActor = 0;
  std::uintptr_t actorRoot = 0;
  if (ResolveCharacterManager(chosen.selected.base, narakaOff, cm, cmSource)) {
    ReadPtr(cm + 0x18, localActor);
    if (ReadActorPos(localActor, narakaOff, localPos, actorRoot)) {
      std::cout << "[LOCAL] source=" << cmSource << " cm=" << Hex(cm)
                << " localActor=" << Hex(localActor)
                << " actorRoot=" << Hex(actorRoot) << " pos=(" << localPos.x
                << "," << localPos.y << "," << localPos.z << ")"
                << std::endl;
    } else {
      std::cout << "[WARN] local actor pos failed; fallback center=("
                << localPos.x << "," << localPos.y << "," << localPos.z
                << ")" << std::endl;
    }
  } else {
    std::cout << "[WARN] CharacterManager resolve failed; fallback center=("
              << localPos.x << "," << localPos.y << "," << localPos.z << ")"
              << std::endl;
  }

  std::vector<CandidateRow> rows;
  std::size_t noName = 0;
  std::size_t noTransform = 0;
  std::size_t noWorld = 0;

  std::size_t totalObjects = 0;
  std::string enumSource;

  std::uint64_t gomRva = 0;
  if (er2::FindGomGlobalSlotRvaByScan(er2::Mem(), er2::g_ctx.unityPlayer.base,
                                      er2::g_ctx.gomOff, gomRva)) {
    er2::g_ctx.gomGlobalSlotRva = gomRva;
    er2::g_ctx.gomGlobalSlotVa =
        er2::g_ctx.unityPlayer.base + static_cast<std::uintptr_t>(gomRva);
    std::cout << "[GOM] slotRva=" << Hex(gomRva)
              << " slotVa=" << Hex(er2::g_ctx.gomGlobalSlotVa)
              << " manager=" << Hex(er2::GomManager()) << std::endl;

    auto entriesOpt = er2::EnumerateGameObjects();
    if (entriesOpt.has_value()) {
      enumSource = "GOM";
      const auto &entries = entriesOpt.value();
      totalObjects = entries.size();
      rows.reserve(std::min<std::size_t>(entries.size(), 2048));
      for (const auto &entry : entries) {
        CandidateRow r{};
        const bool keep =
            BuildCandidateRow(entry.nativeObject, entry.managedObject,
                              localPos, radius, r);
        if (r.name.empty()) {
          ++noName;
        }
        if (!r.transform) {
          ++noTransform;
        }
        if (r.transform && !r.hasWorld) {
          ++noWorld;
        }
        if (keep) {
          rows.push_back(std::move(r));
        }
      }
    } else {
      std::cout << "[WARN] EnumerateGameObjects failed after GOM slot"
                << std::endl;
    }
  } else {
    std::cout << "[WARN] GOM scan failed; trying MSID fallback" << std::endl;
  }

  if (enumSource.empty()) {
    std::uintptr_t msid = 0;
    std::uint32_t score = 0;
    if (!er2::FindMsIdToPointerSlotVaByScan(er2::Mem(), er2::g_ctx.unityPlayer,
                                            er2::g_ctx.gomOff, msid,
                                            &score)) {
      std::cout << "[FAIL] MSID fallback scan failed" << std::endl;
      return 4;
    }
    er2::g_ctx.msIdToPointerSlotVa = msid;
    er2::g_ctx.msIdToPointerSlotRva =
        static_cast<std::uint64_t>(msid - er2::g_ctx.unityPlayer.base);
    enumSource = "MSID";
    std::cout << "[MSID] slotVa=" << Hex(msid)
              << " slotRva=" << Hex(er2::g_ctx.msIdToPointerSlotRva)
              << " score=" << score << std::endl;

    er2::EnumerateOptions opt;
    opt.onlyGameObject = true;
    opt.onlyScriptableObject = false;
    opt.filterLower.clear();

    const bool ok = er2::EnumerateMsIdToPointerObjects(
        er2::g_ctx.runtime, er2::Mem(), er2::g_ctx.msIdToPointerSlotVa,
        er2::g_ctx.off, er2::g_ctx.unityPlayerRange, opt,
        [&](const er2::ObjectInfo &info) {
          ++totalObjects;
          CandidateRow r{};
          const bool keep =
              BuildCandidateRow(info.native, 0, localPos, radius, r);
          if (r.name.empty()) {
            r.name = info.objectName;
          }
          if (r.name.empty()) {
            ++noName;
          }
          if (!r.transform) {
            ++noTransform;
          }
          if (r.transform && !r.hasWorld) {
            ++noWorld;
          }
          if (keep || HasKeyword(info.objectName)) {
            if (!r.keyword) {
              r.keyword = HasKeyword(info.objectName);
            }
            rows.push_back(std::move(r));
          }
        });
    if (!ok) {
      std::cout << "[FAIL] MSID fallback enumeration failed" << std::endl;
      return 5;
    }
  }

  std::sort(rows.begin(), rows.end(), [](const CandidateRow &a,
                                         const CandidateRow &b) {
    if (a.keyword != b.keyword) {
      return a.keyword > b.keyword;
    }
    return a.dist < b.dist;
  });

  const auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(
      std::chrono::steady_clock::now() - start);
  std::cout << "[SUMMARY] source=" << enumSource
            << " totalGameObjects=" << totalObjects
            << " storedCandidates=" << rows.size() << " noName=" << noName
            << " noTransform=" << noTransform << " noWorld=" << noWorld
            << " radius=" << radius << " elapsedMs=" << elapsed.count()
            << std::endl;

  std::cout << "[PUZZLE_OBJECTS detail pass]" << std::endl;
  std::size_t puzzlePrinted = 0;
  for (const CandidateRow &r : rows) {
    if (!IsPuzzleObjectName(r.name)) {
      continue;
    }
    ++puzzlePrinted;
    PrintRow(r);
    PrintPuzzleComponentDetails(r);
  }
  std::cout << "[PUZZLE_OBJECTS] count=" << puzzlePrinted << std::endl;

  std::cout << "[CANDIDATES keyword first, then near]" << std::endl;
  std::size_t printed = 0;
  for (const CandidateRow &r : rows) {
    if (printed++ >= 600) {
      std::cout << "[TRUNCATED] printed 600 rows" << std::endl;
      break;
    }
    PrintRow(r);
  }

  return 0;
}
