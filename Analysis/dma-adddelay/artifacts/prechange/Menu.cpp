#include "Menu.h"
#include "DhzBoxAdapter.h"
#include "ConfigDefaults.h"
#include "Hook.h"
#include "KeyBindManager.h"
#include "NarakaIcon.h"
#include "OS-ImGui/NeverloseGUI.hpp"
#include "OS-ImGui/NeverloseIcons.hpp"
#include "Tool.h"
#include "log.h"

#include <algorithm>
#include <cmath>
#include <cstdio>
#include <cstdint>
#include <cstring>
#include <mutex>

std::string moniVersionname = "v1.1";

namespace {
const char *kShockConfigVersionKey = "模拟振配置版本";
const char *kDodgeConfigVersionKey = "闪避模式配置版本";
const char *kDodgeConfigFileName = "闪避模式配置.ini";
const char *kLegacyDodgeConfigFileName = "闪避模式配置.bin";
const char *kDodgeConfigVersion = "2";
bool g_SkipDodgeParamSave = false;
std::mutex g_KmNetConnectionMutex;

void TrimConfigToken(std::string *value) {
  while (!value->empty()) {
    char head = value->front();
    if (head == ' ' || head == '\t') {
      value->erase(value->begin());
      continue;
    }
    break;
  }

  while (!value->empty()) {
    char tail = value->back();
    if (tail == '\r' || tail == ';' || tail == ' ' || tail == '\t') {
      value->pop_back();
      continue;
    }
    break;
  }
}

bool ParseConfigVersionLine(const std::string &line, const char *versionKey,
                            std::string *outVersion) {
  const std::string prefix = std::string(versionKey) + "=";
  if (line.rfind(prefix, 0) != 0) {
    return false;
  }
  *outVersion = line.substr(prefix.size());
  TrimConfigToken(outVersion);
  return true;
}

bool ParseShockConfigVersionLine(const std::string &line,
                                 std::string *outVersion) {
  return ParseConfigVersionLine(line, kShockConfigVersionKey, outVersion);
}

constexpr bool kShowCrossFadeSpeedControls = false; // 临时隐藏独立动画加速开关和倍率控件
constexpr bool kShowMemoryShockModeControl = false; // 临时隐藏内存振逻辑选择，固定使用 CrossFade 动作加速

bool ParseDodgeConfigVersionLine(const std::string &line,
                                 std::string *outVersion) {
  return ParseConfigVersionLine(line, kDodgeConfigVersionKey, outVersion);
}

void WriteShockConfigVersionLine(std::ofstream &file) {
  file << kShockConfigVersionKey << "=" << moniVersionname << "\n";
}

void WriteDodgeConfigVersionLine(std::ofstream &file) {
  file << kDodgeConfigVersionKey << "=" << kDodgeConfigVersion << "\n";
}

enum class DodgeConfigValueType {
  Int,
  Float,
  Bool,
};

struct DodgeConfigEntry {
  const char *key;
  DodgeConfigValueType type;
  void *address;
};

#define DODGE_CONFIG_ENTRY_INT(key, value)                                      \
  { key, DodgeConfigValueType::Int, &(value) },
#define DODGE_CONFIG_ENTRY_FLOAT(key, value)                                    \
  { key, DodgeConfigValueType::Float, &(value) },
#define DODGE_CONFIG_ENTRY_BOOL(key, value)                                     \
  { key, DodgeConfigValueType::Bool, &(value) },

static DodgeConfigEntry kDodgeConfigEntries[] = {
#include "DodgeConfigEntries.inc"
  DODGE_CONFIG_ENTRY_BOOL("阔刀右蓄是否短闪振",
                          闪避参数::特殊处理::阔刀右蓄是否短闪振)
  DODGE_CONFIG_ENTRY_BOOL("双截棍左蓄是否短闪振",
                          闪避参数::特殊处理::双截棍左蓄是否短闪振)
  DODGE_CONFIG_ENTRY_BOOL("斩马刀左蓄是否长闪振",
                          闪避参数::特殊处理::斩马刀左蓄是否长闪振)
  DODGE_CONFIG_ENTRY_BOOL("扇子左蓄是否中闪振",
                          闪避参数::特殊处理::扇子左蓄是否中闪振)
  DODGE_CONFIG_ENTRY_BOOL("扇子右蓄是否中闪振",
                          闪避参数::特殊处理::扇子右蓄是否中闪振)
  DODGE_CONFIG_ENTRY_BOOL("横刀左蓄是否只短闪",
                          闪避参数::特殊处理::横刀左蓄是否只短闪)
  DODGE_CONFIG_ENTRY_BOOL("拳刃右蓄是否短闪振",
                          闪避参数::特殊处理::拳刃右蓄是否短闪振)
  DODGE_CONFIG_ENTRY_BOOL("长枪左蓄是否短闪振",
                          闪避参数::特殊处理::长枪左蓄是否短闪振)
  DODGE_CONFIG_ENTRY_BOOL("是否长闪振", 闪避参数::特殊处理::是否长闪振)
  DODGE_CONFIG_ENTRY_BOOL("胡为开大振刀", 闪避参数::特殊处理::胡为开大振刀)
};

#undef DODGE_CONFIG_ENTRY_BOOL
#undef DODGE_CONFIG_ENTRY_FLOAT
#undef DODGE_CONFIG_ENTRY_INT

const DodgeConfigEntry *FindDodgeConfigEntry(const std::string &key) {
  for (const DodgeConfigEntry &entry : kDodgeConfigEntries) {
    if (key == entry.key) {
      return &entry;
    }
  }
  return nullptr;
}

void WriteDodgeConfigEntry(std::ofstream &file, const DodgeConfigEntry &entry) {
  file << entry.key << "=";
  switch (entry.type) {
  case DodgeConfigValueType::Int:
    file << *static_cast<const int *>(entry.address);
    break;
  case DodgeConfigValueType::Float:
    file << *static_cast<const float *>(entry.address);
    break;
  case DodgeConfigValueType::Bool:
    file << (*static_cast<const bool *>(entry.address) ? 1 : 0);
    break;
  }
  file << "\n";
}

bool ReadDodgeConfigEntryValue(const DodgeConfigEntry &entry,
                               std::string valueText) {
  TrimConfigToken(&valueText);
  if (valueText.empty()) {
    return false;
  }

  try {
    switch (entry.type) {
    case DodgeConfigValueType::Int: {
      size_t parsedLength = 0;
      const int value = std::stoi(valueText, &parsedLength);
      if (parsedLength != valueText.size()) {
        return false;
      }
      *static_cast<int *>(entry.address) = value;
      return true;
    }
    case DodgeConfigValueType::Float: {
      if (!valueText.empty() &&
          (valueText.back() == 'f' || valueText.back() == 'F')) {
        valueText.pop_back();
      }
      size_t parsedLength = 0;
      const float value = std::stof(valueText, &parsedLength);
      if (parsedLength != valueText.size()) {
        return false;
      }
      *static_cast<float *>(entry.address) = value;
      return true;
    }
    case DodgeConfigValueType::Bool:
      if (valueText == "1" || valueText == "true" || valueText == "TRUE") {
        *static_cast<bool *>(entry.address) = true;
        return true;
      }
      if (valueText == "0" || valueText == "false" || valueText == "FALSE") {
        *static_cast<bool *>(entry.address) = false;
        return true;
      }
      return false;
    }
  } catch (...) {
    return false;
  }

  return false;
}

void 保存闪避参数配置() {
  std::ofstream file(kDodgeConfigFileName, std::ios::out | std::ios::trunc);
  if (!file.is_open()) {
    return;
  }

  file << "; 闪避模式配置，可直接用中文修改\n";
  WriteDodgeConfigVersionLine(file);
  for (const DodgeConfigEntry &entry : kDodgeConfigEntries) {
    WriteDodgeConfigEntry(file, entry);
  }
  file.close();
  std::remove(kLegacyDodgeConfigFileName);
}

void 读取闪避参数配置() {
  std::ifstream file(kDodgeConfigFileName);
  if (!file.is_open()) {
    保存闪避参数配置();
    return;
  }

  std::string line;
  std::string fileVersion;
  bool versionFound = false;
  bool needsRewrite = false;

  while (std::getline(file, line)) {
    std::string trimmedLine = line;
    TrimConfigToken(&trimmedLine);
    if (trimmedLine.empty() || trimmedLine[0] == ';' || trimmedLine[0] == '#') {
      continue;
    }

    std::string parsedVersion;
    if (ParseDodgeConfigVersionLine(trimmedLine, &parsedVersion)) {
      versionFound = true;
      fileVersion = parsedVersion;
      continue;
    }

    const size_t equalPos = trimmedLine.find('=');
    if (equalPos == std::string::npos) {
      continue;
    }

    std::string key = trimmedLine.substr(0, equalPos);
    std::string value = trimmedLine.substr(equalPos + 1);
    TrimConfigToken(&key);
    TrimConfigToken(&value);
    if (key.empty()) {
      continue;
    }

    const DodgeConfigEntry *entry = FindDodgeConfigEntry(key);
    if (entry == nullptr) {
      continue;
    }

    if (!ReadDodgeConfigEntryValue(*entry, value)) {
      needsRewrite = true;
    }
  }
  file.close();

  if (!versionFound || fileVersion != kDodgeConfigVersion || needsRewrite) {
    ConfigDefaults::初始化闪避配置();
    保存闪避参数配置();
  }
}
} // namespace
struct KeyMap {
  int vk;
  int hid;
};

static const int kHidGraveAccent = 0x35;
static const int kHidCapsLock = 0x39;
static const int kHidScrollLock = 0x47;

static const KeyMap kVkToHidMap[] = {
    {VK_A, KEY_A},
    {VK_B, KEY_B},
    {VK_C, KEY_C},
    {VK_D, KEY_D},
    {VK_E, KEY_E},
    {VK_F, KEY_F},
    {VK_G, KEY_G},
    {VK_H, KEY_H},
    {VK_I, KEY_I},
    {VK_J, KEY_J},
    {VK_K, KEY_K},
    {VK_L, KEY_L},
    {VK_M, KEY_M},
    {VK_N, KEY_N},
    {VK_O, KEY_O},
    {VK_P, KEY_P},
    {VK_Q, KEY_Q},
    {VK_R, KEY_R},
    {VK_S, KEY_S},
    {VK_T, KEY_T},
    {VK_U, KEY_U},
    {VK_V, KEY_V},
    {VK_W, KEY_W},
    {VK_X, KEY_X},
    {VK_Y, KEY_Y},
    {VK_Z, KEY_Z},

    {'1', KEY_1_EXCLAMATION_MARK},
    {'2', KEY_2_AT},
    {'3', KEY_3_NUMBER_SIGN},
    {'4', KEY_4_DOLLAR},
    {'5', KEY_5_PERCENT},
    {'6', KEY_6_CARET},
    {'7', KEY_7_AMPERSAND},
    {'8', KEY_8_ASTERISK},
    {'9', KEY_9_OPARENTHESIS},
    {'0', KEY_0_CPARENTHESIS},

    {VK_RETURN, KEY_ENTER},
    {VK_ESCAPE, KEY_ESCAPE},
    {VK_BACK, KEY_BACKSPACE},
    {VK_TAB, KEY_TAB},
    {VK_SPACE, KEY_SPACEBAR},
    {VK_OEM_MINUS, KEY_MINUS_UNDERSCORE},
    {VK_OEM_PLUS, KEY_EQUAL_PLUS},
    {VK_OEM_4, KEY_OBRACKET_AND_OBRACE},
    {VK_OEM_6, KEY_CBRACKET_AND_CBRACE},
    {VK_OEM_5, KEY_BACKSLASH_VERTICAL_BAR},
#ifdef VK_OEM_102
    {VK_OEM_102, KEY_NONUS_BACK_SLASH_VERTICAL_BAR},
#endif
    {VK_OEM_1, KEY_SEMICOLON_COLON},
    {VK_OEM_7, KEY_SINGLE_AND_DOUBLE_QUOTE},
    {VK_OEM_3, kHidGraveAccent},
    {VK_OEM_COMMA, KEY_COMMA_AND_LESS},
    {VK_OEM_PERIOD, KEY_DOT_GREATER},
    {VK_OEM_2, KEY_SLASH_QUESTION},
    {VK_CAPITAL, kHidCapsLock},

    {VK_F1, KEY_F1},
    {VK_F2, KEY_F2},
    {VK_F3, KEY_F3},
    {VK_F4, KEY_F4},
    {VK_F5, KEY_F5},
    {VK_F6, KEY_F6},
    {VK_F7, KEY_F7},
    {VK_F8, KEY_F8},
    {VK_F9, KEY_F9},
    {VK_F10, KEY_F10},
    {VK_F11, KEY_F11},
    {VK_F12, KEY_F12},
    {VK_F13, KEY_F13},
    {VK_F14, KEY_F14},
    {VK_F15, KEY_F15},
    {VK_F16, KEY_F16},
    {VK_F17, KEY_F17},
    {VK_F18, KEY_F18},
    {VK_F19, KEY_F19},
    {VK_F20, KEY_F20},
    {VK_F21, KEY_F21},
    {VK_F22, KEY_F22},
    {VK_F23, KEY_F23},
    {VK_F24, KEY_F24},

    {VK_SNAPSHOT, KEY_PRINTSCREEN},
    {VK_SCROLL, kHidScrollLock},
    {VK_PAUSE, KEY_PAUSE},
    {VK_INSERT, KEY_INSERT},
    {VK_HOME, KEY_HOME},
    {VK_PRIOR, KEY_PAGEUP},
    {VK_DELETE, KEY_DELETE},
    {VK_END, KEY_END1},
    {VK_NEXT, KEY_PAGEDOWN},
    {VK_RIGHT, KEY_RIGHTARROW},
    {VK_LEFT, KEY_LEFTARROW},
    {VK_DOWN, KEY_DOWNARROW},
    {VK_UP, KEY_UPARROW},

    {VK_NUMLOCK, KEY_KEYPAD_NUM_LOCK_AND_CLEAR},
    {VK_DIVIDE, KEY_KEYPAD_SLASH},
    {VK_MULTIPLY, KEY_KEYPAD_ASTERIKS},
    {VK_SUBTRACT, KEY_KEYPAD_MINUS},
    {VK_ADD, KEY_KEYPAD_PLUS},
    {VK_NUMPAD1, KEY_KEYPAD_1_END},
    {VK_NUMPAD2, KEY_KEYPAD_2_DOWN_ARROW},
    {VK_NUMPAD3, KEY_KEYPAD_3_PAGEDN},
    {VK_NUMPAD4, KEY_KEYPAD_4_LEFT_ARROW},
    {VK_NUMPAD5, KEY_KEYPAD_5},
    {VK_NUMPAD6, KEY_KEYPAD_6_RIGHT_ARROW},
    {VK_NUMPAD7, KEY_KEYPAD_7_HOME},
    {VK_NUMPAD8, KEY_KEYPAD_8_UP_ARROW},
    {VK_NUMPAD9, KEY_KEYPAD_9_PAGEUP},
    {VK_NUMPAD0, KEY_KEYPAD_0_INSERT},
    {VK_DECIMAL, KEY_KEYPAD_DECIMAL_SEPARATOR_DELETE},

    {VK_APPS, KEY_APPLICATION},

    {VK_LCONTROL, KEY_LEFTCONTROL},
    {VK_LSHIFT, KEY_LEFTSHIFT},
    {VK_LMENU, KEY_LEFTALT},
    {VK_LWIN, KEY_LEFT_GUI},
    {VK_RCONTROL, KEY_RIGHTCONTROL},
    {VK_RSHIFT, KEY_RIGHTSHIFT},
    {VK_RMENU, KEY_RIGHTALT},
    {VK_RWIN, KEY_RIGHT_GUI},
};

static int VkToHidKey(int vk) {
  for (const auto &map : kVkToHidMap) {
    if (map.vk == vk) {
      return map.hid;
    }
  }
  return 0;
}

static int HidToVkKey(int hid) {
  for (const auto &map : kVkToHidMap) {
    if (map.hid == hid) {
      return map.vk;
    }
  }
  return 0;
}

static bool IsMouseVkKey(int vk) {
  return vk == VK_LBUTTON || vk == VK_RBUTTON || vk == VK_MBUTTON ||
         vk == VK_XBUTTON1 || vk == VK_XBUTTON2;
}

static const char *GetMouseVkName(int vk) {
  switch (vk) {
  case VK_LBUTTON:
    return "MouseLeft";
  case VK_RBUTTON:
    return "MouseRight";
  case VK_MBUTTON:
    return "MouseMiddle";
  case VK_XBUTTON1:
    return "MouseX1";
  case VK_XBUTTON2:
    return "MouseX2";
  default:
    return "Mouse";
  }
}
void 保存模拟振配置() {
  ofstream file("模拟振配置.ini");
  if (file.is_open()) {
    WriteShockConfigVersionLine(file);
    file << "振刀全局延迟修正=" << Function::Shock::振刀全局延迟修正 << "f;\n";
    file << "长剑左3闪避=" << 振刀参数::闪避::长剑.左3 << "f;\n";
    file << "长剑右3闪避=" << 振刀参数::闪避::长剑.右3 << "f;\n";
    file << "长剑左剑气1闪避=" << 振刀参数::闪避::长剑.左剑气1 << "f;\n";
    file << "长剑左剑气2闪避=" << 振刀参数::闪避::长剑.左剑气2 << "f;\n";
    file << "长剑右剑气1闪避=" << 振刀参数::闪避::长剑.右剑气1 << "f;\n";
    file << "长剑右剑气2闪避=" << 振刀参数::闪避::长剑.右剑气2 << "f;\n";
    file << "长剑凤凰羽1闪避=" << 振刀参数::闪避::长剑.凤凰羽1 << "f;\n";
    file << "长剑凤凰羽2闪避=" << 振刀参数::闪避::长剑.凤凰羽2 << "f;\n";
    file << "长剑蓝月闪避=" << 振刀参数::闪避::长剑.苍牙 << "f;\n";
    file << "长剑跳斩闪避=" << 振刀参数::闪避::长剑.跳斩 << "f;\n";
    file << "长剑壁击闪避=" << 振刀参数::闪避::长剑.壁击 << "f;\n";
    file << "长剑七星夺窍闪避=" << 振刀参数::闪避::长剑.七星夺窍 << "f;\n";
    file << "长剑天外飞仙闪避=" << 振刀参数::闪避::长剑.天外飞仙 << "f;\n";

    file << "长剑左3振刀方式=" << 振刀参数::振刀方式::长剑.左3 << "f;\n";
    file << "长剑右3振刀方式=" << 振刀参数::振刀方式::长剑.右3 << "f;\n";
    file << "长剑左剑气1振刀方式=" << 振刀参数::振刀方式::长剑.左剑气1
         << "f;\n";
    file << "长剑左剑气2振刀方式=" << 振刀参数::振刀方式::长剑.左剑气2
         << "f;\n";
    file << "长剑右剑气1振刀方式=" << 振刀参数::振刀方式::长剑.右剑气1
         << "f;\n";
    file << "长剑右剑气2振刀方式=" << 振刀参数::振刀方式::长剑.右剑气2
         << "f;\n";
    file << "长剑凤凰羽1振刀方式=" << 振刀参数::振刀方式::长剑.凤凰羽1
         << "f;\n";
    file << "长剑凤凰羽2振刀方式=" << 振刀参数::振刀方式::长剑.凤凰羽2
         << "f;\n";
    file << "长剑蓝月振刀方式=" << 振刀参数::振刀方式::长剑.苍牙 << "f;\n";
    file << "长剑跳斩振刀方式=" << 振刀参数::振刀方式::长剑.跳斩 << "f;\n";
    file << "长剑壁击振刀方式=" << 振刀参数::振刀方式::长剑.壁击 << "f;\n";
    file << "长剑七星夺窍振刀方式=" << 振刀参数::振刀方式::长剑.七星夺窍
         << "f;\n";

    file << "长剑左3距离=" << 振刀参数::距离::长剑.左3 << "f;\n";
    file << "长剑右3距离=" << 振刀参数::距离::长剑.右3 << "f;\n";
    file << "长剑左剑气距离=" << 振刀参数::距离::长剑.左剑气 << "f;\n";
    file << "长剑右剑气距离=" << 振刀参数::距离::长剑.右剑气 << "f;\n";
    file << "长剑凤凰羽距离=" << 振刀参数::距离::长剑.凤凰羽 << "f;\n";
    file << "长剑蓝月距离=" << 振刀参数::距离::长剑.苍牙 << "f;\n";
    file << "长剑跳斩距离=" << 振刀参数::距离::长剑.跳斩 << "f;\n";
    file << "长剑壁击距离=" << 振刀参数::距离::长剑.壁击 << "f;\n";
    file << "长剑七星夺窍距离=" << 振刀参数::距离::长剑.七星夺窍 << "f;\n";

    file << "长剑左3角度=" << 振刀参数::角度::长剑.左3 << "f;\n";
    file << "长剑右3角度=" << 振刀参数::角度::长剑.右3 << "f;\n";
    file << "长剑左剑气角度=" << 振刀参数::角度::长剑.左剑气 << "f;\n";
    file << "长剑右剑气角度=" << 振刀参数::角度::长剑.右剑气 << "f;\n";
    file << "长剑凤凰羽角度=" << 振刀参数::角度::长剑.凤凰羽 << "f;\n";
    file << "长剑苍牙角度=" << 振刀参数::角度::长剑.苍牙 << "f;\n";
    file << "长剑跳斩角度=" << 振刀参数::角度::长剑.跳斩 << "f;\n";
    file << "长剑壁击角度=" << 振刀参数::角度::长剑.壁击 << "f;\n";
    file << "长剑七星夺窍角度=" << 振刀参数::角度::长剑.七星夺窍 << "f;\n";

    file << "长剑左3延迟=" << 振刀参数::延迟::长剑.左3 << "f;\n";
    file << "长剑右3延迟=" << 振刀参数::延迟::长剑.右3 << "f;\n";
    file << "长剑左剑气延迟=" << 振刀参数::延迟::长剑.左剑气 << "f;\n";
    file << "长剑右剑气延迟=" << 振刀参数::延迟::长剑.右剑气 << "f;\n";
    file << "长剑凤凰羽延迟=" << 振刀参数::延迟::长剑.凤凰羽 << "f;\n";
    file << "长剑蓝月延迟=" << 振刀参数::延迟::长剑.苍牙 << "f;\n";
    file << "长剑跳斩延迟=" << 振刀参数::延迟::长剑.跳斩 << "f;\n";
    file << "长剑七星夺窍延迟=" << 振刀参数::延迟::长剑.七星夺窍 << "f;\n";
    file << "长剑壁击延迟=" << 振刀参数::延迟::长剑.壁击 << "f;\n";

    file << "链剑左3距离=" << 振刀参数::距离::链剑.左3 << "f;\n";
    file << "链剑右3距离=" << 振刀参数::距离::链剑.右3 << "f;\n";
    file << "链剑左蓄力距离=" << 振刀参数::距离::链剑.左蓄力 << "f;\n";
    file << "链剑右蓄力距离=" << 振刀参数::距离::链剑.右蓄力 << "f;\n";
    file << "链剑地龙滚堂刹距离=" << 振刀参数::距离::链剑.地龙滚堂刹 << "f;\n";
    file << "链剑苍牙距离=" << 振刀参数::距离::链剑.苍牙 << "f;\n";
    file << "链剑火龙卷云距离=" << 振刀参数::距离::链剑.火龙卷云 << "f;\n";
    file << "链剑断罪碎蜂距离=" << 振刀参数::距离::链剑.断罪碎蜂 << "f;\n";
    file << "链剑蓄力追击距离=" << 振刀参数::距离::链剑.蓄力追击 << "f;\n";
    file << "链剑长剑同源距离=" << 振刀参数::距离::链剑.链剑长剑同源 << "f;\n";

    file << "链剑左3振刀方式=" << 振刀参数::振刀方式::链剑.左3 << "f;\n";
    file << "链剑右3振刀方式=" << 振刀参数::振刀方式::链剑.右3 << "f;\n";
    file << "链剑左蓄力1振刀方式=" << 振刀参数::振刀方式::链剑.左蓄力1
         << "f;\n";
    file << "链剑左蓄力2振刀方式=" << 振刀参数::振刀方式::链剑.左蓄力2
         << "f;\n";
    file << "链剑右蓄力1振刀方式=" << 振刀参数::振刀方式::链剑.右蓄力1
         << "f;\n";
    file << "链剑右蓄力2振刀方式=" << 振刀参数::振刀方式::链剑.右蓄力2
         << "f;\n";
    file << "链剑地龙滚堂刹振刀方式=" << 振刀参数::振刀方式::链剑.地龙滚堂刹
         << "f;\n";
    file << "链剑苍牙振刀方式=" << 振刀参数::振刀方式::链剑.苍牙 << "f;\n";
    file << "链剑火龙卷云振刀方式=" << 振刀参数::振刀方式::链剑.火龙卷云
         << "f;\n";
    file << "链剑断罪碎蜂振刀方式=" << 振刀参数::振刀方式::链剑.断罪碎蜂
         << "f;\n";
    file << "链剑蓄力追击振刀方式=" << 振刀参数::振刀方式::链剑.蓄力追击
         << "f;\n";
    file << "链剑长剑同源振刀方式=" << 振刀参数::振刀方式::链剑.链剑长剑同源
        << "f;\n";

    file << "链剑左3闪避=" << 振刀参数::闪避::链剑.左3 << "f;\n";
    file << "链剑右3闪避=" << 振刀参数::闪避::链剑.右3 << "f;\n";
    file << "链剑左蓄力1闪避=" << 振刀参数::闪避::链剑.左蓄力1 << "f;\n";
    file << "链剑左蓄力2闪避=" << 振刀参数::闪避::链剑.左蓄力2 << "f;\n";
    file << "链剑右蓄力1闪避=" << 振刀参数::闪避::链剑.右蓄力1 << "f;\n";
    file << "链剑右蓄力2闪避=" << 振刀参数::闪避::链剑.右蓄力2 << "f;\n";
    file << "链剑地龙滚堂刹闪避=" << 振刀参数::闪避::链剑.地龙滚堂刹 << "f;\n";
    file << "链剑苍牙闪避=" << 振刀参数::闪避::链剑.苍牙 << "f;\n";
    file << "链剑跳斩闪避=" << 振刀参数::闪避::链剑.跳斩 << "f;\n";
    file << "链剑壁击闪避=" << 振刀参数::闪避::链剑.壁击 << "f;\n";
    file << "链剑火龙卷云闪避=" << 振刀参数::闪避::链剑.火龙卷云 << "f;\n";
    file << "链剑断罪碎蜂闪避=" << 振刀参数::闪避::链剑.断罪碎蜂 << "f;\n";
    file << "链剑蓄力追击闪避=" << 振刀参数::闪避::链剑.蓄力追击 << "f;\n";
    file << "链剑长剑同源闪避=" << 振刀参数::闪避::链剑.链剑长剑同源 << "f;\n";

    file << "链剑左3角度=" << 振刀参数::角度::链剑.左3 << "f;\n";
    file << "链剑右3角度=" << 振刀参数::角度::链剑.右3 << "f;\n";
    file << "链剑左蓄力角度=" << 振刀参数::角度::链剑.左蓄力 << "f;\n";
    file << "链剑右蓄力角度=" << 振刀参数::角度::链剑.右蓄力 << "f;\n";
    file << "链剑地龙滚堂刹角度=" << 振刀参数::角度::链剑.地龙滚堂刹 << "f;\n";
    file << "链剑苍牙角度=" << 振刀参数::角度::链剑.苍牙 << "f;\n";
    file << "链剑火龙卷云角度=" << 振刀参数::角度::链剑.火龙卷云 << "f;\n";
    file << "链剑断罪碎蜂角度=" << 振刀参数::角度::链剑.断罪碎蜂 << "f;\n";
    file << "链剑蓄力追击角度=" << 振刀参数::角度::链剑.蓄力追击 << "f;\n";
    file << "链剑长剑同源角度=" << 振刀参数::角度::链剑.链剑长剑同源 << "f;\n";

    file << "链剑左3延迟=" << 振刀参数::延迟::链剑.左3 << "f;\n";
    file << "链剑右3延迟=" << 振刀参数::延迟::链剑.右3 << "f;\n";
    file << "链剑左蓄力延迟=" << 振刀参数::延迟::链剑.左蓄力 << "f;\n";
    file << "链剑右蓄力延迟=" << 振刀参数::延迟::链剑.右蓄力 << "f;\n";
    file << "链剑地龙滚堂刹延迟=" << 振刀参数::延迟::链剑.地龙滚堂刹 << "f;\n";
    file << "链剑苍牙延迟=" << 振刀参数::延迟::链剑.苍牙 << "f;\n";
    file << "链剑火龙卷云延迟=" << 振刀参数::延迟::链剑.火龙卷云 << "f;\n";
    file << "链剑断罪碎蜂延迟=" << 振刀参数::延迟::链剑.断罪碎蜂 << "f;\n";
    file << "链剑蓄力追击延迟=" << 振刀参数::延迟::链剑.蓄力追击 << "f;\n";
    file << "链剑长剑同源延迟=" << 振刀参数::延迟::链剑.链剑长剑同源 << "f;\n";

    file << "千机伞左3距离=" << 振刀参数::距离::千机伞.左3 << "f;\n";
    file << "千机伞伞左蓄距离=" << 振刀参数::距离::千机伞.伞左蓄 << "f;\n";
    file << "千机伞盾左蓄距离=" << 振刀参数::距离::千机伞.盾左蓄 << "f;\n";
    file << "千机伞斧左蓄距离=" << 振刀参数::距离::千机伞.斧左蓄 << "f;\n";
    file << "千机伞伞右蓄距离=" << 振刀参数::距离::千机伞.伞右蓄 << "f;\n";
    file << "千机伞盾右蓄距离=" << 振刀参数::距离::千机伞.盾右蓄 << "f;\n";
    file << "千机伞斧右蓄距离=" << 振刀参数::距离::千机伞.斧右蓄 << "f;\n";
    file << "千机伞听雨封喉距离=" << 振刀参数::距离::千机伞.听雨封喉 << "f;\n";
    file << "千机伞苍牙距离=" << 振刀参数::距离::千机伞.苍牙 << "f;\n";

    file << "千机伞左3振刀方式=" << 振刀参数::振刀方式::千机伞.左3 << "f;\n";
    file << "千机伞伞左蓄振刀方式=" << 振刀参数::振刀方式::千机伞.伞左蓄 << "f;\n";
    file << "千机伞盾左蓄振刀方式=" << 振刀参数::振刀方式::千机伞.盾左蓄 << "f;\n";
    file << "千机伞斧左蓄振刀方式=" << 振刀参数::振刀方式::千机伞.斧左蓄 << "f;\n";
    file << "千机伞伞右蓄振刀方式=" << 振刀参数::振刀方式::千机伞.伞右蓄 << "f;\n";
    file << "千机伞盾右蓄振刀方式=" << 振刀参数::振刀方式::千机伞.盾右蓄 << "f;\n";
    file << "千机伞斧右蓄振刀方式=" << 振刀参数::振刀方式::千机伞.斧右蓄 << "f;\n";
    file << "千机伞听雨封喉振刀方式=" << 振刀参数::振刀方式::千机伞.听雨封喉 << "f;\n";
    file << "千机伞苍牙振刀方式=" << 振刀参数::振刀方式::千机伞.苍牙 << "f;\n";

    file << "千机伞左3闪避=" << 振刀参数::闪避::千机伞.左3 << "f;\n";
    file << "千机伞伞左蓄闪避=" << 振刀参数::闪避::千机伞.伞左蓄 << "f;\n";
    file << "千机伞盾左蓄闪避=" << 振刀参数::闪避::千机伞.盾左蓄 << "f;\n";
    file << "千机伞斧左蓄闪避=" << 振刀参数::闪避::千机伞.斧左蓄 << "f;\n";
    file << "千机伞伞右蓄闪避=" << 振刀参数::闪避::千机伞.伞右蓄 << "f;\n";
    file << "千机伞盾右蓄闪避=" << 振刀参数::闪避::千机伞.盾右蓄 << "f;\n";
    file << "千机伞斧右蓄闪避=" << 振刀参数::闪避::千机伞.斧右蓄 << "f;\n";
    file << "千机伞听雨封喉闪避=" << 振刀参数::闪避::千机伞.听雨封喉 << "f;\n";
    file << "千机伞苍牙闪避=" << 振刀参数::闪避::千机伞.苍牙 << "f;\n";

    file << "千机伞左3角度=" << 振刀参数::角度::千机伞.左3 << "f;\n";
    file << "千机伞伞左蓄角度=" << 振刀参数::角度::千机伞.伞左蓄 << "f;\n";
    file << "千机伞盾左蓄角度=" << 振刀参数::角度::千机伞.盾左蓄 << "f;\n";
    file << "千机伞斧左蓄角度=" << 振刀参数::角度::千机伞.斧左蓄 << "f;\n";
    file << "千机伞伞右蓄角度=" << 振刀参数::角度::千机伞.伞右蓄 << "f;\n";
    file << "千机伞盾右蓄角度=" << 振刀参数::角度::千机伞.盾右蓄 << "f;\n";
    file << "千机伞斧右蓄角度=" << 振刀参数::角度::千机伞.斧右蓄 << "f;\n";
    file << "千机伞听雨封喉角度=" << 振刀参数::角度::千机伞.听雨封喉 << "f;\n";
    file << "千机伞苍牙角度=" << 振刀参数::角度::千机伞.苍牙 << "f;\n";

    file << "千机伞左3延迟=" << 振刀参数::延迟::千机伞.左3 << "f;\n";
    file << "千机伞伞左蓄延迟=" << 振刀参数::延迟::千机伞.伞左蓄 << "f;\n";
    file << "千机伞盾左蓄延迟=" << 振刀参数::延迟::千机伞.盾左蓄 << "f;\n";
    file << "千机伞斧左蓄延迟=" << 振刀参数::延迟::千机伞.斧左蓄 << "f;\n";
    file << "千机伞伞右蓄延迟=" << 振刀参数::延迟::千机伞.伞右蓄 << "f;\n";
    file << "千机伞盾右蓄延迟=" << 振刀参数::延迟::千机伞.盾右蓄 << "f;\n";
    file << "千机伞斧右蓄延迟=" << 振刀参数::延迟::千机伞.斧右蓄 << "f;\n";
    file << "千机伞听雨封喉延迟=" << 振刀参数::延迟::千机伞.听雨封喉 << "f;\n";
    file << "千机伞苍牙延迟=" << 振刀参数::延迟::千机伞.苍牙 << "f;\n";

    file << "太刀左3闪避=" << 振刀参数::闪避::太刀.左3 << "f;\n";
    file << "太刀右3闪避=" << 振刀参数::闪避::太刀.右3 << "f;\n";
    file << "太刀左蓄1闪避=" << 振刀参数::闪避::太刀.左蓄1 << "f;\n";
    file << "太刀左蓄2闪避=" << 振刀参数::闪避::太刀.左蓄2 << "f;\n";
    file << "太刀右蓄1闪避=" << 振刀参数::闪避::太刀.右蓄1 << "f;\n";
    file << "太刀右蓄2闪避=" << 振刀参数::闪避::太刀.右蓄2 << "f;\n";
    file << "太刀苍牙闪避=" << 振刀参数::闪避::太刀.苍牙 << "f;\n";
    file << "太刀壁击闪避=" << 振刀参数::闪避::太刀.壁击 << "f;\n";
    file << "太刀刹那斩1闪避=" << 振刀参数::闪避::太刀.刹那斩1 << "f;\n";
    file << "太刀刹那斩2闪避=" << 振刀参数::闪避::太刀.刹那斩2 << "f;\n";
    file << "太刀惊雷闪避=" << 振刀参数::闪避::太刀.惊雷 << "f;\n";
    file << "太刀惊雷十劫闪避=" << 振刀参数::闪避::太刀.惊雷十劫 << "f;\n";
    file << "太刀青鬼闪避=" << 振刀参数::闪避::太刀.青鬼 << "f;\n";
    file << "太刀同源闪避=" << 振刀参数::闪避::太刀.同源 << "f;\n";
    file << "太刀噬魂斩1闪避=" << 振刀参数::闪避::太刀.噬魂斩1 << "f;\n";
    file << "太刀噬魂斩2闪避=" << 振刀参数::闪避::太刀.噬魂斩2 << "f;\n";

    file << "太刀左3振刀方式=" << 振刀参数::振刀方式::太刀.左3 << "f;\n";
    file << "太刀右3振刀方式=" << 振刀参数::振刀方式::太刀.右3 << "f;\n";
    file << "太刀左蓄1振刀方式=" << 振刀参数::振刀方式::太刀.左蓄1 << "f;\n";
    file << "太刀左蓄2振刀方式=" << 振刀参数::振刀方式::太刀.左蓄2 << "f;\n";
    file << "太刀右蓄1振刀方式=" << 振刀参数::振刀方式::太刀.右蓄1 << "f;\n";
    file << "太刀右蓄2振刀方式=" << 振刀参数::振刀方式::太刀.右蓄2 << "f;\n";
    file << "太刀苍牙振刀方式=" << 振刀参数::振刀方式::太刀.苍牙 << "f;\n";
    file << "太刀壁击振刀方式=" << 振刀参数::振刀方式::太刀.壁击 << "f;\n";
    file << "太刀刹那斩1振刀方式=" << 振刀参数::振刀方式::太刀.刹那斩1
         << "f;\n";
    file << "太刀刹那斩2振刀方式=" << 振刀参数::振刀方式::太刀.刹那斩2
         << "f;\n";
    file << "太刀惊雷振刀方式=" << 振刀参数::振刀方式::太刀.惊雷 << "f;\n";
    file << "太刀惊雷十劫振刀方式=" << 振刀参数::振刀方式::太刀.惊雷十劫
         << "f;\n";
    file << "太刀青鬼振刀方式=" << 振刀参数::振刀方式::太刀.青鬼 << "f;\n";
    file << "太刀同源振刀方式=" << 振刀参数::振刀方式::太刀.同源 << "f;\n";
    file << "太刀噬魂斩1振刀方式=" << 振刀参数::振刀方式::太刀.噬魂斩1
         << "f;\n";
    file << "太刀噬魂斩2振刀方式=" << 振刀参数::振刀方式::太刀.噬魂斩2
         << "f;\n";

    file << "太刀左3距离=" << 振刀参数::距离::太刀.左3 << "f;\n";
    file << "太刀右3距离=" << 振刀参数::距离::太刀.右3 << "f;\n";
    file << "太刀左蓄距离=" << 振刀参数::距离::太刀.左蓄 << "f;\n";
    file << "太刀右蓄距离=" << 振刀参数::距离::太刀.右蓄 << "f;\n";
    file << "太刀苍牙距离=" << 振刀参数::距离::太刀.苍牙 << "f;\n";
    file << "太刀壁击距离=" << 振刀参数::距离::太刀.壁击 << "f;\n";
    file << "太刀刹那斩距离=" << 振刀参数::距离::太刀.刹那斩 << "f;\n";
    file << "太刀右惊雷距离=" << 振刀参数::距离::太刀.惊雷 << "f;\n";
    file << "太刀惊雷十劫距离=" << 振刀参数::距离::太刀.惊雷十劫 << "f;\n";
    file << "太刀青鬼距离=" << 振刀参数::距离::太刀.青鬼 << "f;\n";
    file << "太刀同源距离=" << 振刀参数::距离::太刀.同源 << "f;\n";
    file << "太刀噬魂斩距离=" << 振刀参数::距离::太刀.噬魂斩 << "f;\n";

    file << "太刀左3角度=" << 振刀参数::角度::太刀.左3 << "f;\n";
    file << "太刀右3角度=" << 振刀参数::角度::太刀.右3 << "f;\n";
    file << "太刀左蓄角度=" << 振刀参数::角度::太刀.左蓄 << "f;\n";
    file << "太刀右蓄角度=" << 振刀参数::角度::太刀.右蓄 << "f;\n";
    file << "太刀苍牙角度=" << 振刀参数::角度::太刀.苍牙 << "f;\n";
    file << "太刀壁击角度=" << 振刀参数::角度::太刀.壁击 << "f;\n";
    file << "太刀刹那斩角度=" << 振刀参数::角度::太刀.刹那斩 << "f;\n";
    file << "太刀惊雷&惊雷十劫角度=" << 振刀参数::角度::太刀.惊雷十劫 << "f;\n";
    file << "太刀青鬼角度=" << 振刀参数::角度::太刀.青鬼 << "f;\n";
    file << "太刀同源角度=" << 振刀参数::角度::太刀.同源 << "f;\n";

    file << "太刀左3延迟=" << 振刀参数::延迟::太刀.左3 << "f;\n";
    file << "太刀右3延迟=" << 振刀参数::延迟::太刀.右3 << "f;\n";
    file << "太刀左蓄延迟=" << 振刀参数::延迟::太刀.左蓄 << "f;\n";
    file << "太刀右蓄延迟=" << 振刀参数::延迟::太刀.右蓄 << "f;\n";
    file << "太刀苍牙延迟=" << 振刀参数::延迟::太刀.苍牙 << "f;\n";
    file << "太刀刹那斩延迟=" << 振刀参数::延迟::太刀.刹那斩 << "f;\n";
    file << "太刀右惊雷延迟=" << 振刀参数::延迟::太刀.右惊雷 << "f;\n";
    file << "太刀青鬼延迟=" << 振刀参数::延迟::太刀.青鬼 << "f;\n";
    file << "太刀同源延迟=" << 振刀参数::延迟::太刀.同源 << "f;\n";
    file << "太刀噬魂斩延迟=" << 振刀参数::延迟::太刀.噬魂斩 << "f;\n";
    file << "太刀壁击延迟=" << 振刀参数::延迟::太刀.壁击 << "f;\n";

    file << "阔刀左左闪避=" << 振刀参数::闪避::阔刀.左左 << "f;\n";
    file << "阔刀左右闪避=" << 振刀参数::闪避::阔刀.左右 << "f;\n";
    file << "阔刀右右闪避=" << 振刀参数::闪避::阔刀.右右 << "f;\n";
    file << "阔刀右左闪避=" << 振刀参数::闪避::阔刀.右左 << "f;\n";
    file << "阔刀左蓄一段闪避=" << 振刀参数::闪避::阔刀.左蓄一段 << "f;\n";
    file << "阔刀左蓄二段闪避=" << 振刀参数::闪避::阔刀.左蓄二段 << "f;\n";
    file << "阔刀左蓄三段闪避=" << 振刀参数::闪避::阔刀.左蓄三段 << "f;\n";
    file << "阔刀右蓄1闪避=" << 振刀参数::闪避::阔刀.右蓄1 << "f;\n";
    file << "阔刀右蓄2闪避=" << 振刀参数::闪避::阔刀.右蓄2 << "f;\n";
    file << "阔刀右右蓄闪避=" << 振刀参数::闪避::阔刀.右右蓄 << "f;\n";
    file << "阔刀雷刀闪避=" << 振刀参数::闪避::阔刀.雷刀 << "f;\n";
    file << "阔刀壁击闪避=" << 振刀参数::闪避::阔刀.壁击 << "f;\n";
    file << "阔刀同源闪避=" << 振刀参数::闪避::阔刀.同源 << "f;\n";
    file << "阔刀雷刀下劈闪避=" << 振刀参数::闪避::阔刀.雷刀下劈 << "f;\n";
    file << "阔刀翻江倒海闪避=" << 振刀参数::闪避::阔刀.翻江倒海 << "f;\n";

    file << "阔刀左左振刀方式=" << 振刀参数::振刀方式::阔刀.左左 << "f;\n";
    file << "阔刀左右振刀方式=" << 振刀参数::振刀方式::阔刀.左右 << "f;\n";
    file << "阔刀右右振刀方式=" << 振刀参数::振刀方式::阔刀.右右 << "f;\n";
    file << "阔刀右左振刀方式=" << 振刀参数::振刀方式::阔刀.右左 << "f;\n";
    file << "阔刀左蓄一段振刀方式=" << 振刀参数::振刀方式::阔刀.左蓄一段
         << "f;\n";
    file << "阔刀左蓄二段振刀方式=" << 振刀参数::振刀方式::阔刀.左蓄二段
         << "f;\n";
    file << "阔刀左蓄三段振刀方式=" << 振刀参数::振刀方式::阔刀.左蓄三段
         << "f;\n";
    file << "阔刀右蓄1振刀方式=" << 振刀参数::振刀方式::阔刀.右蓄1 << "f;\n";
    file << "阔刀右蓄2振刀方式=" << 振刀参数::振刀方式::阔刀.右蓄2 << "f;\n";
    file << "阔刀右右蓄振刀方式=" << 振刀参数::振刀方式::阔刀.右右蓄 << "f;\n";
    file << "阔刀雷刀振刀方式=" << 振刀参数::振刀方式::阔刀.雷刀 << "f;\n";
    file << "阔刀壁击振刀方式=" << 振刀参数::振刀方式::阔刀.壁击 << "f;\n";
    file << "阔刀同源振刀方式=" << 振刀参数::振刀方式::阔刀.同源 << "f;\n";
    file << "阔刀雷刀下劈振刀方式=" << 振刀参数::振刀方式::阔刀.雷刀下劈
         << "f;\n";
    file << "阔刀翻江倒海振刀方式=" << 振刀参数::振刀方式::阔刀.翻江倒海
         << "f;\n";

    file << "阔刀左左距离=" << 振刀参数::距离::阔刀.左左 << "f;\n";
    file << "阔刀左右距离=" << 振刀参数::距离::阔刀.左右 << "f;\n";
    file << "阔刀右右距离=" << 振刀参数::距离::阔刀.右右 << "f;\n";
    file << "阔刀右左距离=" << 振刀参数::距离::阔刀.右左 << "f;\n";
    file << "阔刀壁击距离=" << 振刀参数::距离::阔刀.壁击 << "f;\n";
    file << "阔刀左蓄一段距离=" << 振刀参数::距离::阔刀.左蓄一段 << "f;\n";
    file << "阔刀左蓄二段距离=" << 振刀参数::距离::阔刀.左蓄二段 << "f;\n";
    file << "阔刀左蓄三段距离=" << 振刀参数::距离::阔刀.左蓄三段 << "f;\n";
    file << "阔刀右蓄距离=" << 振刀参数::距离::阔刀.右蓄 << "f;\n";
    file << "阔刀右右蓄距离=" << 振刀参数::距离::阔刀.右右蓄 << "f;\n";
    file << "阔刀雷刀距离=" << 振刀参数::距离::阔刀.雷刀 << "f;\n";
    file << "阔刀雷刀下劈距离=" << 振刀参数::距离::阔刀.雷刀下劈 << "f;\n";
    file << "阔刀同源距离=" << 振刀参数::距离::阔刀.同源 << "f;\n";

    file << "阔刀左左角度=" << 振刀参数::角度::阔刀.左左 << "f;\n";
    file << "阔刀左右角度=" << 振刀参数::角度::阔刀.左右 << "f;\n";
    file << "阔刀右右角度=" << 振刀参数::角度::阔刀.右右 << "f;\n";
    file << "阔刀壁击角度=" << 振刀参数::角度::阔刀.壁击 << "f;\n";
    file << "阔刀右左角度=" << 振刀参数::角度::阔刀.右左 << "f;\n";
    file << "阔刀左蓄一段角度=" << 振刀参数::角度::阔刀.左蓄一段 << "f;\n";
    file << "阔刀左蓄二段角度=" << 振刀参数::角度::阔刀.左蓄二段 << "f;\n";
    file << "阔刀左蓄三段角度=" << 振刀参数::角度::阔刀.左蓄三段 << "f;\n";
    file << "阔刀右蓄角度=" << 振刀参数::角度::阔刀.右蓄 << "f;\n";
    file << "阔刀右右蓄角度=" << 振刀参数::角度::阔刀.右右蓄 << "f;\n";
    file << "阔刀同源角度=" << 振刀参数::角度::阔刀.同源 << "f;\n";

    file << "阔刀左左延迟=" << 振刀参数::延迟::阔刀.左左 << "f;\n";
    file << "阔刀左右延迟=" << 振刀参数::延迟::阔刀.左右 << "f;\n";
    file << "阔刀右右延迟=" << 振刀参数::延迟::阔刀.右右 << "f;\n";
    file << "阔刀右左延迟=" << 振刀参数::延迟::阔刀.右左 << "f;\n";
    file << "阔刀壁击延迟=" << 振刀参数::延迟::阔刀.壁击 << "f;\n";
    file << "阔刀同源延迟=" << 振刀参数::延迟::阔刀.同源 << "f;\n";
    file << "阔刀左蓄一段延迟=" << 振刀参数::延迟::阔刀.左蓄一段 << "f;\n";
    file << "阔刀左蓄二段延迟=" << 振刀参数::延迟::阔刀.左蓄二段 << "f;\n";
    file << "阔刀左蓄三段延迟=" << 振刀参数::延迟::阔刀.左蓄三段 << "f;\n";
    file << "阔刀右蓄延迟=" << 振刀参数::延迟::阔刀.右蓄 << "f;\n";
    file << "阔刀右右蓄延迟=" << 振刀参数::延迟::阔刀.右右蓄 << "f;\n";
    file << "阔刀雷刀延迟=" << 振刀参数::延迟::阔刀.雷刀 << "f;\n";
    file << "阔刀雷刀下劈延迟=" << 振刀参数::延迟::阔刀.雷刀下劈 << "f;\n";

    file << "长枪左3闪避=" << 振刀参数::闪避::长枪.左3 << "f;\n";
    file << "长枪右3闪避=" << 振刀参数::闪避::长枪.右3 << "f;\n";
    file << "长枪左蓄闪避=" << 振刀参数::闪避::长枪.左蓄 << "f;\n";
    file << "长枪六合枪闪避=" << 振刀参数::闪避::长枪.六合枪 << "f;\n";
    file << "长枪右蓄闪避=" << 振刀参数::闪避::长枪.右蓄 << "f;\n";
    file << "长枪壁击闪避=" << 振刀参数::闪避::长枪.壁击 << "f;\n";
    file << "长枪同源闪避=" << 振刀参数::闪避::长枪.同源 << "f;\n";
    file << "长枪大圣游闪避=" << 振刀参数::闪避::长枪.大圣游 << "f;\n";
    file << "长枪风卷云残闪避=" << 振刀参数::闪避::长枪.风卷云残 << "f;\n";
    file << "长枪双环扫闪避=" << 振刀参数::闪避::长枪.双环扫 << "f;\n";
    file << "长枪龙王破闪避=" << 振刀参数::闪避::长枪.龙王破 << "f;\n";
    file << "长枪穿心脚闪避=" << 振刀参数::闪避::长枪.穿心脚 << "f;\n";

    file << "长枪左3振刀方式=" << 振刀参数::振刀方式::长枪.左3 << "f;\n";
    file << "长枪右3振刀方式=" << 振刀参数::振刀方式::长枪.右3 << "f;\n";
    file << "长枪左蓄振刀方式=" << 振刀参数::振刀方式::长枪.左蓄 << "f;\n";
    file << "长枪六合枪振刀方式=" << 振刀参数::振刀方式::长枪.六合枪 << "f;\n";
    file << "长枪右蓄振刀方式=" << 振刀参数::振刀方式::长枪.右蓄 << "f;\n";
    file << "长枪壁击振刀方式=" << 振刀参数::振刀方式::长枪.壁击 << "f;\n";
    file << "长枪同源振刀方式=" << 振刀参数::振刀方式::长枪.同源 << "f;\n";
    file << "长枪大圣游振刀方式=" << 振刀参数::振刀方式::长枪.大圣游 << "f;\n";
    file << "长枪风卷云残振刀方式=" << 振刀参数::振刀方式::长枪.风卷云残
         << "f;\n";
    file << "长枪双环扫振刀方式=" << 振刀参数::振刀方式::长枪.双环扫 << "f;\n";
    file << "长枪龙王破振刀方式=" << 振刀参数::振刀方式::长枪.龙王破 << "f;\n";
    file << "长枪穿心脚振刀方式=" << 振刀参数::振刀方式::长枪.穿心脚 << "f;\n";

    file << "长枪左3距离=" << 振刀参数::距离::长枪.左3 << "f;\n";
    file << "长枪右3距离=" << 振刀参数::距离::长枪.右3 << "f;\n";
    file << "长枪左蓄距离=" << 振刀参数::距离::长枪.左蓄 << "f;\n";
    file << "长枪六合枪距离=" << 振刀参数::距离::长枪.六合枪 << "f;\n";
    file << "长枪右蓄距离=" << 振刀参数::距离::长枪.右蓄 << "f;\n";
    file << "长枪壁击距离=" << 振刀参数::距离::长枪.壁击 << "f;\n";
    file << "长枪同源距离=" << 振刀参数::距离::长枪.同源 << "f;\n";
    file << "长枪大圣游距离=" << 振刀参数::距离::长枪.大圣游 << "f;\n";
    file << "长枪风卷云残距离=" << 振刀参数::距离::长枪.风卷云残 << "f;\n";
    file << "长枪双环扫距离=" << 振刀参数::距离::长枪.双环扫 << "f;\n";
    file << "长枪龙王破距离=" << 振刀参数::距离::长枪.龙王破 << "f;\n";
    file << "长枪穿心脚距离=" << 振刀参数::距离::长枪.穿心脚 << "f;\n";

    file << "长枪左3延迟=" << 振刀参数::延迟::长枪.左3 << "f;\n";
    file << "长枪右3延迟=" << 振刀参数::延迟::长枪.右3 << "f;\n";
    file << "长枪左蓄延迟=" << 振刀参数::延迟::长枪.左蓄 << "f;\n";
    file << "长枪六合枪延迟=" << 振刀参数::延迟::长枪.六合枪 << "f;\n";
    file << "长枪右蓄延迟=" << 振刀参数::延迟::长枪.右蓄 << "f;\n";
    file << "长枪大圣游延迟=" << 振刀参数::延迟::长枪.大圣游 << "f;\n";
    file << "长枪风卷云残延迟=" << 振刀参数::延迟::长枪.风卷云残 << "f;\n";
    file << "长枪双环扫延迟=" << 振刀参数::延迟::长枪.双环扫 << "f;\n";
    file << "长枪龙王破延迟=" << 振刀参数::延迟::长枪.龙王破 << "f;\n";
    file << "长枪穿心脚延迟=" << 振刀参数::延迟::长枪.穿心脚 << "f;\n";
    file << "长枪壁击延迟=" << 振刀参数::延迟::长枪.壁击 << "f;\n";
    file << "长枪同源延迟=" << 振刀参数::延迟::长枪.同源 << "f;\n";

    file << "长枪左3角度=" << 振刀参数::角度::长枪.左3 << "f;\n";
    file << "长枪右3角度=" << 振刀参数::角度::长枪.右3 << "f;\n";
    file << "长枪左蓄角度=" << 振刀参数::角度::长枪.左蓄 << "f;\n";
    file << "长枪六合枪角度=" << 振刀参数::角度::长枪.六合枪 << "f;\n";
    file << "长枪右蓄角度=" << 振刀参数::角度::长枪.右蓄 << "f;\n";
    file << "长枪壁击角度=" << 振刀参数::角度::长枪.壁击 << "f;\n";
    file << "长枪双环扫角度=" << 振刀参数::角度::长枪.双环扫 << "f;\n";
    file << "长枪同源角度=" << 振刀参数::角度::长枪.同源 << "f;\n";
    file << "长枪大圣游角度=" << 振刀参数::角度::长枪.大圣游 << "f;\n";
    file << "长枪风卷云残角度=" << 振刀参数::角度::长枪.风卷云残 << "f;\n";
    file << "长枪龙王破角度=" << 振刀参数::角度::长枪.龙王破 << "f;\n";
    file << "长枪穿心脚角度=" << 振刀参数::角度::长枪.穿心脚 << "f;\n";

    file << "匕首左3闪避=" << 振刀参数::闪避::匕首.左3 << "f;\n";
    file << "匕首右3闪避=" << 振刀参数::闪避::匕首.右3 << "f;\n";
    file << "匕首左蓄闪避=" << 振刀参数::闪避::匕首.左蓄 << "f;\n";
    file << "匕首右蓄闪避=" << 振刀参数::闪避::匕首.右蓄 << "f;\n";
    file << "匕首鬼反闪避=" << 振刀参数::闪避::匕首.鬼反 << "f;\n";
    file << "匕首荆轲献匕闪避=" << 振刀参数::闪避::匕首.荆轲献匕 << "f;\n";
    file << "匕首壁击闪避=" << 振刀参数::闪避::匕首.壁击 << "f;\n";
    file << "匕首鬼刃暗扎闪避=" << 振刀参数::闪避::匕首.鬼刃暗扎 << "f;\n";
    file << "匕首鬼哭神嚎闪避=" << 振刀参数::闪避::匕首.鬼哭神嚎 << "f;\n";
    file << "匕首亢龙有悔闪避=" << 振刀参数::闪避::匕首.亢龙有悔 << "f;\n";

    file << "匕首左3振刀方式=" << 振刀参数::振刀方式::匕首.左3 << "f;\n";
    file << "匕首右3振刀方式=" << 振刀参数::振刀方式::匕首.右3 << "f;\n";
    file << "匕首左蓄振刀方式=" << 振刀参数::振刀方式::匕首.左蓄 << "f;\n";
    file << "匕首右蓄振刀方式=" << 振刀参数::振刀方式::匕首.右蓄 << "f;\n";
    file << "匕首鬼反振刀方式=" << 振刀参数::振刀方式::匕首.鬼反 << "f;\n";
    file << "匕首荆轲献匕振刀方式=" << 振刀参数::振刀方式::匕首.荆轲献匕
         << "f;\n";
    file << "匕首壁击振刀方式=" << 振刀参数::振刀方式::匕首.壁击 << "f;\n";
    file << "匕首鬼刃暗扎振刀方式=" << 振刀参数::振刀方式::匕首.鬼刃暗扎
         << "f;\n";
    file << "匕首鬼哭神嚎振刀方式=" << 振刀参数::振刀方式::匕首.鬼哭神嚎
         << "f;\n";
    file << "匕首亢龙有悔振刀方式=" << 振刀参数::振刀方式::匕首.亢龙有悔
         << "f;\n";

    file << "匕首左3延迟=" << 振刀参数::延迟::匕首.左3 << "f;\n";
    file << "匕首右3延迟=" << 振刀参数::延迟::匕首.右3 << "f;\n";
    file << "匕首左蓄延迟=" << 振刀参数::延迟::匕首.左蓄 << "f;\n";
    file << "匕首右蓄延迟=" << 振刀参数::延迟::匕首.右蓄 << "f;\n";
    file << "匕首鬼反延迟=" << 振刀参数::延迟::匕首.鬼反 << "f;\n";
    file << "匕首壁击延迟=" << 振刀参数::延迟::匕首.壁击 << "f;\n";
    file << "匕首荆轲献匕延迟=" << 振刀参数::延迟::匕首.荆轲献匕 << "f;\n";
    file << "匕首鬼刃暗扎延迟=" << 振刀参数::延迟::匕首.鬼刃暗扎 << "f;\n";
    file << "匕首鬼哭神嚎延迟=" << 振刀参数::延迟::匕首.鬼哭神嚎 << "f;\n";
    file << "匕首亢龙有悔延迟=" << 振刀参数::延迟::匕首.亢龙有悔 << "f;\n";

    file << "匕首左3距离=" << 振刀参数::距离::匕首.左3 << "f;\n";
    file << "匕首右3距离=" << 振刀参数::距离::匕首.右3 << "f;\n";
    file << "匕首左蓄距离=" << 振刀参数::距离::匕首.左蓄 << "f;\n";
    file << "匕首右蓄距离=" << 振刀参数::距离::匕首.右蓄 << "f;\n";
    file << "匕首壁击距离=" << 振刀参数::距离::匕首.壁击 << "f;\n";
    file << "匕首鬼反距离=" << 振刀参数::距离::匕首.鬼反 << "f;\n";
    file << "匕首荆轲献匕距离=" << 振刀参数::距离::匕首.荆轲献匕 << "f;\n";
    file << "匕首鬼刃暗扎距离=" << 振刀参数::距离::匕首.鬼刃暗扎 << "f;\n";
    file << "匕首鬼哭神嚎距离=" << 振刀参数::距离::匕首.鬼哭神嚎 << "f;\n";
    file << "匕首亢龙有悔距离=" << 振刀参数::距离::匕首.亢龙有悔 << "f;\n";

    file << "匕首左3角度=" << 振刀参数::角度::匕首.左3 << "f;\n";
    file << "匕首右3角度=" << 振刀参数::角度::匕首.右3 << "f;\n";
    file << "匕首左蓄角度=" << 振刀参数::角度::匕首.左蓄 << "f;\n";
    file << "匕首右蓄角度=" << 振刀参数::角度::匕首.右蓄 << "f;\n";
    file << "匕首鬼反角度=" << 振刀参数::角度::匕首.鬼反 << "f;\n";
    file << "匕首壁击角度=" << 振刀参数::角度::匕首.壁击 << "f;\n";
    file << "匕首荆轲献匕角度=" << 振刀参数::角度::匕首.荆轲献匕 << "f;\n";
    file << "匕首鬼刃暗扎角度=" << 振刀参数::角度::匕首.鬼刃暗扎 << "f;\n";
    file << "匕首亢龙有悔角度=" << 振刀参数::角度::匕首.亢龙有悔 << "f;\n";
    file << "匕首鬼哭神嚎角度=" << 振刀参数::角度::匕首.鬼哭神嚎 << "f;\n";

    file << "双截棍左右3闪避=" << 振刀参数::闪避::双截棍.左右3 << "f;\n";
    file << "双截棍左蓄闪避=" << 振刀参数::闪避::双截棍.左蓄 << "f;\n";
    file << "双截棍右蓄闪避=" << 振刀参数::闪避::双截棍.右蓄 << "f;\n";
    file << "双截棍飞踢闪避=" << 振刀参数::闪避::双截棍.飞踢 << "f;\n";
    file << "双截棍壁击闪避=" << 振刀参数::闪避::双截棍.壁击 << "f;\n";
    file << "双截棍扬鞭劲闪避=" << 振刀参数::闪避::双截棍.扬鞭劲 << "f;\n";
    file << "双截棍横栏闪避=" << 振刀参数::闪避::双截棍.横栏 << "f;\n";
    file << "双截棍龙虎乱舞闪避=" << 振刀参数::闪避::双截棍.龙虎乱舞 << "f;\n";
    file << "双截棍三龙灭阳棍闪避=" << 振刀参数::闪避::双截棍.三龙灭阳棍
         << "f;\n";

    file << "双截棍左右3振刀方式=" << 振刀参数::振刀方式::双截棍.左右3
         << "f;\n";
    file << "双截棍左蓄振刀方式=" << 振刀参数::振刀方式::双截棍.左蓄 << "f;\n";
    file << "双截棍右蓄振刀方式=" << 振刀参数::振刀方式::双截棍.右蓄 << "f;\n";
    file << "双截棍飞踢振刀方式=" << 振刀参数::振刀方式::双截棍.飞踢 << "f;\n";
    file << "双截棍壁击振刀方式=" << 振刀参数::振刀方式::双截棍.壁击 << "f;\n";
    file << "双截棍扬鞭劲振刀方式=" << 振刀参数::振刀方式::双截棍.扬鞭劲
         << "f;\n";
    file << "双截棍横栏振刀方式=" << 振刀参数::振刀方式::双截棍.横栏 << "f;\n";
    file << "双截棍龙虎乱舞振刀方式=" << 振刀参数::振刀方式::双截棍.龙虎乱舞
         << "f;\n";
    file << "双截棍三龙灭阳棍振刀方式=" << 振刀参数::振刀方式::双截棍.三龙灭阳棍
         << "f;\n";

    file << "双截棍左右3距离=" << 振刀参数::距离::双截棍.左右3 << "f;\n";
    file << "双截棍左蓄距离=" << 振刀参数::距离::双截棍.左蓄 << "f;\n";
    file << "双截棍右蓄距离=" << 振刀参数::距离::双截棍.右蓄 << "f;\n";
    file << "双截棍飞踢距离=" << 振刀参数::距离::双截棍.飞踢 << "f;\n";
    file << "双截棍壁击距离=" << 振刀参数::距离::双截棍.壁击 << "f;\n";
    file << "双截棍扬鞭劲距离=" << 振刀参数::距离::双截棍.扬鞭劲 << "f;\n";
    file << "双截棍横栏距离=" << 振刀参数::距离::双截棍.横栏 << "f;\n";
    file << "双截棍龙虎乱舞距离=" << 振刀参数::距离::双截棍.龙虎乱舞 << "f;\n";

    file << "双截棍左右3延迟=" << 振刀参数::延迟::双截棍.左右3 << "f;\n";
    file << "双截棍左蓄延迟=" << 振刀参数::延迟::双截棍.左蓄 << "f;\n";
    file << "双截棍右蓄延迟=" << 振刀参数::延迟::双截棍.右蓄 << "f;\n";
    file << "双截棍飞踢延迟=" << 振刀参数::延迟::双截棍.飞踢 << "f;\n";
    file << "双截棍壁击延迟=" << 振刀参数::延迟::双截棍.壁击 << "f;\n";
    file << "双截棍扬鞭劲延迟=" << 振刀参数::延迟::双截棍.扬鞭劲 << "f;\n";
    file << "双截棍横栏延迟=" << 振刀参数::延迟::双截棍.横栏 << "f;\n";
    file << "双截棍龙虎乱舞延迟=" << 振刀参数::延迟::双截棍.龙虎乱舞 << "f;\n";

    file << "双截棍左右3角度=" << 振刀参数::角度::双截棍.左右3 << "f;\n";
    file << "双截棍左蓄角度=" << 振刀参数::角度::双截棍.左蓄 << "f;\n";
    file << "双截棍右蓄角度=" << 振刀参数::角度::双截棍.右蓄 << "f;\n";
    file << "双截棍飞踢角度=" << 振刀参数::角度::双截棍.飞踢 << "f;\n";
    file << "双截棍壁击角度=" << 振刀参数::角度::双截棍.壁击 << "f;\n";
    file << "双截棍扬鞭劲角度=" << 振刀参数::角度::双截棍.扬鞭劲 << "f;\n";
    file << "双截棍横栏角度=" << 振刀参数::角度::双截棍.横栏 << "f;\n";
    file << "双截棍龙虎乱舞角度=" << 振刀参数::角度::双截棍.龙虎乱舞 << "f;\n";

    file << "双刀左右3闪避=" << 振刀参数::闪避::双刀.左右3 << "f;\n";
    file << "双刀左蓄闪避=" << 振刀参数::闪避::双刀.左蓄 << "f;\n";
    file << "双刀右蓄1闪避=" << 振刀参数::闪避::双刀.右蓄1 << "f;\n";
    file << "双刀右蓄2闪避=" << 振刀参数::闪避::双刀.右蓄2 << "f;\n";
    file << "双刀铁马残红闪避=" << 振刀参数::闪避::双刀.铁马残红 << "f;\n";
    file << "双刀乾坤日月斩闪避=" << 振刀参数::闪避::双刀.乾坤日月斩 << "f;\n";
    file << "双刀惊雷闪避=" << 振刀参数::闪避::双刀.惊雷 << "f;\n";
    file << "双刀壁击闪避=" << 振刀参数::闪避::双刀.壁击 << "f;\n";
    file << "双刀八斩刀闪避=" << 振刀参数::闪避::双刀.八斩刀 << "f;\n";
    file << "双刀分水斩闪避=" << 振刀参数::闪避::双刀.分水斩 << "f;\n";

    file << "双刀左右3振刀方式=" << 振刀参数::振刀方式::双刀.左右3 << "f;\n";
    file << "双刀左蓄振刀方式=" << 振刀参数::振刀方式::双刀.左蓄 << "f;\n";
    file << "双刀右蓄1振刀方式=" << 振刀参数::振刀方式::双刀.右蓄1 << "f;\n";
    file << "双刀右蓄2振刀方式=" << 振刀参数::振刀方式::双刀.右蓄2 << "f;\n";
    file << "双刀铁马残红振刀方式=" << 振刀参数::振刀方式::双刀.铁马残红
         << "f;\n";
    file << "双刀乾坤日月斩振刀方式=" << 振刀参数::振刀方式::双刀.乾坤日月斩
         << "f;\n";
    file << "双刀惊雷振刀方式=" << 振刀参数::振刀方式::双刀.惊雷 << "f;\n";
    file << "双刀壁击振刀方式=" << 振刀参数::振刀方式::双刀.壁击 << "f;\n";
    file << "双刀八斩刀振刀方式=" << 振刀参数::振刀方式::双刀.八斩刀 << "f;\n";
    file << "双刀分水斩振刀方式=" << 振刀参数::振刀方式::双刀.分水斩 << "f;\n";

    file << "双刀左右3距离=" << 振刀参数::距离::双刀.左右3 << "f;\n";
    file << "双刀左蓄距离=" << 振刀参数::距离::双刀.左蓄 << "f;\n";
    file << "双刀右蓄距离=" << 振刀参数::距离::双刀.右蓄 << "f;\n";
    file << "双刀壁击距离=" << 振刀参数::距离::双刀.壁击 << "f;\n";
    file << "双刀铁马残红距离=" << 振刀参数::距离::双刀.铁马残红 << "f;\n";
    file << "双刀乾坤日月斩距离=" << 振刀参数::距离::双刀.乾坤日月斩 << "f;\n";
    file << "双刀惊雷距离=" << 振刀参数::距离::双刀.惊雷 << "f;\n";
    file << "双刀八斩刀距离=" << 振刀参数::距离::双刀.八斩刀 << "f;\n";

    file << "双刀左右3延迟=" << 振刀参数::延迟::双刀.左右3 << "f;\n";
    file << "双刀左蓄延迟=" << 振刀参数::延迟::双刀.左蓄 << "f;\n";
    file << "双刀右蓄延迟=" << 振刀参数::延迟::双刀.右蓄 << "f;\n";
    file << "双刀铁马残红延迟=" << 振刀参数::延迟::双刀.铁马残红 << "f;\n";
    file << "双刀乾坤日月斩延迟=" << 振刀参数::延迟::双刀.乾坤日月斩 << "f;\n";
    file << "双刀惊雷延迟=" << 振刀参数::延迟::双刀.惊雷 << "f;\n";
    file << "双刀壁击延迟=" << 振刀参数::延迟::双刀.壁击 << "f;\n";
    file << "双刀八斩刀延迟=" << 振刀参数::延迟::双刀.八斩刀 << "f;\n";

    file << "双刀左右3角度=" << 振刀参数::角度::双刀.左右3 << "f;\n";
    file << "双刀左蓄角度=" << 振刀参数::角度::双刀.左蓄 << "f;\n";
    file << "双刀右蓄角度=" << 振刀参数::角度::双刀.右蓄 << "f;\n";
    file << "双刀壁击角度=" << 振刀参数::角度::双刀.壁击 << "f;\n";
    file << "双刀铁马残红角度=" << 振刀参数::角度::双刀.铁马残红 << "f;\n";
    file << "双刀乾坤日月斩角度=" << 振刀参数::角度::双刀.乾坤日月斩 << "f;\n";
    file << "双刀惊雷角度=" << 振刀参数::角度::双刀.惊雷 << "f;\n";
    file << "双刀八斩刀角度=" << 振刀参数::角度::双刀.八斩刀 << "f;\n";

    file << "长棍左3闪避=" << 振刀参数::闪避::长棍.左3 << "f;\n";
    file << "长棍右3闪避=" << 振刀参数::闪避::长棍.右3 << "f;\n";
    file << "长棍左蓄闪避=" << 振刀参数::闪避::长棍.左蓄 << "f;\n";
    file << "长棍右蓄闪避=" << 振刀参数::闪避::长棍.右蓄 << "f;\n";
    file << "长棍壁击闪避=" << 振刀参数::闪避::长棍.壁击 << "f;\n";
    file << "长棍同源闪避=" << 振刀参数::闪避::长棍.同源 << "f;\n";
    file << "长棍腾云式闪避=" << 振刀参数::闪避::长棍.腾云式 << "f;\n";
    file << "长棍乱点天宫闪避=" << 振刀参数::闪避::长棍.乱点天宫 << "f;\n";
    file << "长棍桶劲闪避=" << 振刀参数::闪避::长棍.桶劲 << "f;\n";
    file << "长棍五情七灭镇闪避=" << 振刀参数::闪避::长棍.五情七灭镇 << "f;\n";
    file << "长棍双环扫闪避=" << 振刀参数::闪避::长棍.双环扫 << "f;\n";
    file << "长棍少林棍闪避=" << 振刀参数::闪避::长棍.少林棍 << "f;\n";
    file << "长棍镇地撑天闪避=" << 振刀参数::闪避::长棍.镇地撑天 << "f;\n";

    file << "长棍左3振刀方式=" << 振刀参数::振刀方式::长棍.左3 << "f;\n";
    file << "长棍右3振刀方式=" << 振刀参数::振刀方式::长棍.右3 << "f;\n";
    file << "长棍左蓄振刀方式=" << 振刀参数::振刀方式::长棍.左蓄 << "f;\n";
    file << "长棍右蓄振刀方式=" << 振刀参数::振刀方式::长棍.右蓄 << "f;\n";
    file << "长棍壁击振刀方式=" << 振刀参数::振刀方式::长棍.壁击 << "f;\n";
    file << "长棍同源振刀方式=" << 振刀参数::振刀方式::长棍.同源 << "f;\n";
    file << "长棍腾云式振刀方式=" << 振刀参数::振刀方式::长棍.腾云式 << "f;\n";
    file << "长棍乱点天宫振刀方式=" << 振刀参数::振刀方式::长棍.乱点天宫
         << "f;\n";
    file << "长棍桶劲振刀方式=" << 振刀参数::振刀方式::长棍.桶劲 << "f;\n";
    file << "长棍五情七灭镇振刀方式=" << 振刀参数::振刀方式::长棍.五情七灭镇
         << "f;\n";
    file << "长棍双环扫振刀方式=" << 振刀参数::振刀方式::长棍.双环扫 << "f;\n";
    file << "长棍少林棍振刀方式=" << 振刀参数::振刀方式::长棍.少林棍 << "f;\n";
    file << "长棍镇地撑天振刀方式=" << 振刀参数::振刀方式::长棍.镇地撑天
         << "f;\n";

    file << "长棍左3距离=" << 振刀参数::距离::长棍.左3 << "f;\n";
    file << "长棍右3距离=" << 振刀参数::距离::长棍.右3 << "f;\n";
    file << "长棍左蓄距离=" << 振刀参数::距离::长棍.左蓄 << "f;\n";
    file << "长棍右蓄距离=" << 振刀参数::距离::长棍.右蓄 << "f;\n";
    file << "长棍壁击距离=" << 振刀参数::距离::长棍.壁击 << "f;\n";
    file << "长棍同源距离=" << 振刀参数::距离::长棍.同源 << "f;\n";
    file << "长棍腾云式距离=" << 振刀参数::距离::长棍.腾云式 << "f;\n";
    file << "长棍乱点天宫距离=" << 振刀参数::距离::长棍.乱点天宫 << "f;\n";
    file << "长棍桶劲距离=" << 振刀参数::距离::长棍.桶劲 << "f;\n";
    file << "长棍五情七灭镇距离=" << 振刀参数::距离::长棍.五情七灭镇 << "f;\n";
    file << "长棍双环扫距离=" << 振刀参数::距离::长棍.双环扫 << "f;\n";
    file << "长棍倒海棍距离=" << 振刀参数::距离::长棍.少林棍 << "f;\n";
    file << "长棍镇地撑天距离=" << 振刀参数::距离::长棍.镇地撑天 << "f;\n";

    file << "长棍左3延迟=" << 振刀参数::延迟::长棍.左3 << "f;\n";
    file << "长棍右3延迟=" << 振刀参数::延迟::长棍.右3 << "f;\n";
    file << "长棍少林棍延迟=" << 振刀参数::延迟::长棍.少林棍 << "f;\n";
    file << "长棍左蓄延迟=" << 振刀参数::延迟::长棍.左蓄 << "f;\n";
    file << "长棍右蓄延迟=" << 振刀参数::延迟::长棍.右蓄 << "f;\n";
    file << "长棍腾云式延迟=" << 振刀参数::延迟::长棍.腾云式 << "f;\n";
    file << "长棍乱点天宫延迟=" << 振刀参数::延迟::长棍.乱点天宫 << "f;\n";
    file << "长棍桶劲延迟=" << 振刀参数::延迟::长棍.桶劲 << "f;\n";
    file << "长棍壁击延迟=" << 振刀参数::延迟::长棍.壁击 << "f;\n";
    file << "长棍同源延迟=" << 振刀参数::延迟::长棍.同源 << "f;\n";
    file << "长棍五情七灭镇延迟=" << 振刀参数::延迟::长棍.五情七灭镇 << "f;\n";
    file << "长棍双环扫延迟=" << 振刀参数::延迟::长棍.双环扫 << "f;\n";
    file << "长棍倒海棍延迟=" << 振刀参数::延迟::长棍.倒海棍 << "f;\n";
    file << "长棍镇地撑天延迟=" << 振刀参数::延迟::长棍.镇地撑天 << "f;\n";

    file << "长棍左3角度=" << 振刀参数::角度::长棍.左3 << "f;\n";
    file << "长棍右3角度=" << 振刀参数::角度::长棍.右3 << "f;\n";
    file << "长棍左蓄角度=" << 振刀参数::角度::长棍.左蓄 << "f;\n";
    file << "长棍右蓄角度=" << 振刀参数::角度::长棍.右蓄 << "f;\n";
    file << "长棍壁击角度=" << 振刀参数::角度::长棍.壁击 << "f;\n";
    file << "长棍同源角度=" << 振刀参数::角度::长棍.同源 << "f;\n";
    file << "长棍腾云式角度=" << 振刀参数::角度::长棍.腾云式 << "f;\n";
    file << "长棍乱点天宫角度=" << 振刀参数::角度::长棍.乱点天宫 << "f;\n";
    file << "长棍桶劲角度=" << 振刀参数::角度::长棍.桶劲 << "f;\n";
    file << "长棍五情七灭镇角度=" << 振刀参数::角度::长棍.五情七灭镇 << "f;\n";
    file << "长棍双环扫角度=" << 振刀参数::角度::长棍.双环扫 << "f;\n";
    file << "长棍倒海棍角度=" << 振刀参数::角度::长棍.少林棍 << "f;\n";
    file << "长棍镇地撑天角度=" << 振刀参数::角度::长棍.镇地撑天 << "f;\n";

    file << "斩马刀左左闪避=" << 振刀参数::闪避::斩马刀.左左 << "f;\n";
    file << "斩马刀左右闪避=" << 振刀参数::闪避::斩马刀.左右 << "f;\n";
    file << "斩马刀右右闪避=" << 振刀参数::闪避::斩马刀.右右 << "f;\n";
    file << "斩马刀右左闪避=" << 振刀参数::闪避::斩马刀.右左 << "f;\n";
    file << "斩马刀左蓄1闪避=" << 振刀参数::闪避::斩马刀.左蓄1 << "f;\n";
    file << "斩马刀左蓄2闪避=" << 振刀参数::闪避::斩马刀.左蓄2 << "f;\n";
    file << "斩马刀左蓄3闪避=" << 振刀参数::闪避::斩马刀.左蓄3 << "f;\n";
    file << "斩马刀右蓄1闪避=" << 振刀参数::闪避::斩马刀.右蓄1 << "f;\n";
    file << "斩马刀右蓄2闪避=" << 振刀参数::闪避::斩马刀.右蓄2 << "f;\n";
    file << "斩马刀右蓄3闪避=" << 振刀参数::闪避::斩马刀.右蓄3 << "f;\n";
    file << "斩马刀柄击闪避=" << 振刀参数::闪避::斩马刀.柄击 << "f;\n";
    file << "斩马刀壁击闪避=" << 振刀参数::闪避::斩马刀.壁击 << "f;\n";
    file << "斩马刀同源闪避=" << 振刀参数::闪避::斩马刀.同源 << "f;\n";
    file << "斩马刀惊雷闪避=" << 振刀参数::闪避::斩马刀.惊雷 << "f;\n";
    file << "斩马刀炽焰斩闪避=" << 振刀参数::闪避::斩马刀.炽焰斩 << "f;\n";
    file << "斩马刀过关斩将闪避=" << 振刀参数::闪避::斩马刀.过关斩将 << "f;\n";
    file << "斩马刀奔雷入阵闪避=" << 振刀参数::闪避::斩马刀.奔雷入阵 << "f;\n";

    file << "斩马刀左左振刀方式=" << 振刀参数::振刀方式::斩马刀.左左 << "f;\n";
    file << "斩马刀左右振刀方式=" << 振刀参数::振刀方式::斩马刀.左右 << "f;\n";
    file << "斩马刀右右振刀方式=" << 振刀参数::振刀方式::斩马刀.右右 << "f;\n";
    file << "斩马刀右左振刀方式=" << 振刀参数::振刀方式::斩马刀.右左 << "f;\n";
    file << "斩马刀左蓄1振刀方式=" << 振刀参数::振刀方式::斩马刀.左蓄1
         << "f;\n";
    file << "斩马刀左蓄2振刀方式=" << 振刀参数::振刀方式::斩马刀.左蓄2
         << "f;\n";
    file << "斩马刀左蓄3振刀方式=" << 振刀参数::振刀方式::斩马刀.左蓄3
         << "f;\n";
    file << "斩马刀右蓄1振刀方式=" << 振刀参数::振刀方式::斩马刀.右蓄1
         << "f;\n";
    file << "斩马刀右蓄2振刀方式=" << 振刀参数::振刀方式::斩马刀.右蓄2
         << "f;\n";
    file << "斩马刀右蓄3振刀方式=" << 振刀参数::振刀方式::斩马刀.右蓄3
         << "f;\n";
    file << "斩马刀柄击振刀方式=" << 振刀参数::振刀方式::斩马刀.柄击 << "f;\n";
    file << "斩马刀壁击振刀方式=" << 振刀参数::振刀方式::斩马刀.壁击 << "f;\n";
    file << "斩马刀同源振刀方式=" << 振刀参数::振刀方式::斩马刀.同源 << "f;\n";
    file << "斩马刀惊雷振刀方式=" << 振刀参数::振刀方式::斩马刀.惊雷 << "f;\n";
    file << "斩马刀炽焰斩振刀方式=" << 振刀参数::振刀方式::斩马刀.炽焰斩
         << "f;\n";
    file << "斩马刀过关斩将振刀方式=" << 振刀参数::振刀方式::斩马刀.过关斩将
         << "f;\n";
    file << "斩马刀奔雷入阵振刀方式=" << 振刀参数::振刀方式::斩马刀.奔雷入阵
         << "f;\n";

    file << "斩马刀左左距离=" << 振刀参数::距离::斩马刀.左左 << "f;\n";
    file << "斩马刀左右距离=" << 振刀参数::距离::斩马刀.左右 << "f;\n";
    file << "斩马刀右右距离=" << 振刀参数::距离::斩马刀.右右 << "f;\n";
    file << "斩马刀右左距离=" << 振刀参数::距离::斩马刀.右左 << "f;\n";
    file << "斩马刀左蓄距离=" << 振刀参数::距离::斩马刀.左蓄 << "f;\n";
    file << "斩马刀右蓄距离=" << 振刀参数::距离::斩马刀.右蓄 << "f;\n";
    file << "斩马刀壁击距离=" << 振刀参数::距离::斩马刀.壁击 << "f;\n";
    file << "斩马刀柄击距离=" << 振刀参数::距离::斩马刀.柄击 << "f;\n";
    file << "斩马刀惊雷距离=" << 振刀参数::距离::斩马刀.惊雷 << "f;\n";
    file << "斩马刀同源距离=" << 振刀参数::距离::斩马刀.同源 << "f;\n";
    file << "斩马刀炽焰斩距离=" << 振刀参数::距离::斩马刀.炽焰斩 << "f;\n";
    file << "斩马刀过关斩将距离=" << 振刀参数::距离::斩马刀.过关斩将 << "f;\n";

    file << "斩马刀左左延迟=" << 振刀参数::延迟::斩马刀.左左 << "f;\n";
    file << "斩马刀左右延迟=" << 振刀参数::延迟::斩马刀.左右 << "f;\n";
    file << "斩马刀右右延迟=" << 振刀参数::延迟::斩马刀.右右 << "f;\n";
    file << "斩马刀右左延迟=" << 振刀参数::延迟::斩马刀.右左 << "f;\n";
    file << "斩马刀左蓄延迟=" << 振刀参数::延迟::斩马刀.左蓄 << "f;\n";
    file << "斩马刀右蓄延迟=" << 振刀参数::延迟::斩马刀.右蓄 << "f;\n";
    file << "斩马刀柄击延迟=" << 振刀参数::延迟::斩马刀.柄击 << "f;\n";
    file << "斩马刀惊雷延迟=" << 振刀参数::延迟::斩马刀.惊雷 << "f;\n";
    file << "斩马刀壁击延迟=" << 振刀参数::延迟::斩马刀.壁击 << "f;\n";
    file << "斩马刀同源延迟=" << 振刀参数::延迟::斩马刀.同源 << "f;\n";
    file << "斩马刀炽焰斩延迟=" << 振刀参数::延迟::斩马刀.炽焰斩 << "f;\n";
    file << "斩马刀过关斩将延迟=" << 振刀参数::延迟::斩马刀.过关斩将 << "f;\n";

    file << "斩马刀左左角度=" << 振刀参数::角度::斩马刀.左左 << "f;\n";
    file << "斩马刀左右角度=" << 振刀参数::角度::斩马刀.左右 << "f;\n";
    file << "斩马刀右右角度=" << 振刀参数::角度::斩马刀.右右 << "f;\n";
    file << "斩马刀右左角度=" << 振刀参数::角度::斩马刀.右左 << "f;\n";
    file << "斩马刀左蓄角度=" << 振刀参数::角度::斩马刀.左蓄 << "f;\n";
    file << "斩马刀右蓄角度=" << 振刀参数::角度::斩马刀.右蓄 << "f;\n";
    file << "斩马刀壁击角度=" << 振刀参数::角度::斩马刀.壁击 << "f;\n";
    file << "斩马刀柄击角度=" << 振刀参数::角度::斩马刀.柄击 << "f;\n";
    file << "斩马刀惊雷角度=" << 振刀参数::角度::斩马刀.惊雷 << "f;\n";
    file << "斩马刀同源角度=" << 振刀参数::角度::斩马刀.同源 << "f;\n";
    file << "斩马刀炽焰斩角度=" << 振刀参数::角度::斩马刀.炽焰斩 << "f;\n";
    file << "斩马刀过关斩将角度=" << 振刀参数::角度::斩马刀.过关斩将 << "f;\n";

    file << "双戟左3闪避=" << 振刀参数::闪避::双戟.左3 << "f;\n";
    file << "双戟右3闪避=" << 振刀参数::闪避::双戟.右3 << "f;\n";
    file << "双戟左蓄闪避=" << 振刀参数::闪避::双戟.左蓄 << "f;\n";
    file << "双戟右蓄1闪避=" << 振刀参数::闪避::双戟.右蓄1 << "f;\n";
    file << "双戟右蓄2闪避=" << 振刀参数::闪避::双戟.右蓄2 << "f;\n";
    file << "双戟惊雷闪避=" << 振刀参数::闪避::双戟.惊雷 << "f;\n";
    file << "双戟勾旋斩闪避=" << 振刀参数::闪避::双戟.勾旋斩 << "f;\n";
    file << "双戟探海蛟闪避=" << 振刀参数::闪避::双戟.探海蛟 << "f;\n";
    file << "双戟钩挂闪避=" << 振刀参数::闪避::双戟.钩挂 << "f;\n";
    file << "双戟壁击闪避=" << 振刀参数::闪避::双戟.壁击 << "f;\n";
    file << "双戟战龙在天闪避=" << 振刀参数::闪避::双戟.战龙在天 << "f;\n";
    file << "双戟形意钩闪避=" << 振刀参数::闪避::双戟.形意钩 << "f;\n";

    file << "双戟左3振刀方式=" << 振刀参数::振刀方式::双戟.左3 << "f;\n";
    file << "双戟右3振刀方式=" << 振刀参数::振刀方式::双戟.右3 << "f;\n";
    file << "双戟左蓄振刀方式=" << 振刀参数::振刀方式::双戟.左蓄 << "f;\n";
    file << "双戟右蓄1振刀方式=" << 振刀参数::振刀方式::双戟.右蓄1 << "f;\n";
    file << "双戟右蓄2振刀方式=" << 振刀参数::振刀方式::双戟.右蓄2 << "f;\n";
    file << "双戟惊雷振刀方式=" << 振刀参数::振刀方式::双戟.惊雷 << "f;\n";
    file << "双戟勾旋斩振刀方式=" << 振刀参数::振刀方式::双戟.勾旋斩 << "f;\n";
    file << "双戟探海蛟振刀方式=" << 振刀参数::振刀方式::双戟.探海蛟 << "f;\n";
    file << "双戟钩挂振刀方式=" << 振刀参数::振刀方式::双戟.钩挂 << "f;\n";
    file << "双戟壁击振刀方式=" << 振刀参数::振刀方式::双戟.壁击 << "f;\n";
    file << "双戟战龙在天振刀方式=" << 振刀参数::振刀方式::双戟.战龙在天
         << "f;\n";
    file << "双戟形意钩振刀方式=" << 振刀参数::振刀方式::双戟.形意钩 << "f;\n";

    file << "双戟左右3距离=" << 振刀参数::距离::双戟.左右3 << "f;\n";
    file << "双戟左蓄距离=" << 振刀参数::距离::双戟.左蓄 << "f;\n";
    file << "双戟右蓄距离=" << 振刀参数::距离::双戟.右蓄 << "f;\n";
    file << "双戟壁击距离=" << 振刀参数::距离::双戟.壁击 << "f;\n";
    file << "双戟勾旋斩距离=" << 振刀参数::距离::双戟.勾旋斩 << "f;\n";
    file << "双戟探海蛟距离=" << 振刀参数::距离::双戟.探海蛟 << "f;\n";
    file << "双戟钩挂距离=" << 振刀参数::距离::双戟.钩挂 << "f;\n";
    file << "双戟战龙在天距离=" << 振刀参数::距离::双戟.战龙在天 << "f;\n";
    file << "双戟形意钩距离=" << 振刀参数::距离::双戟.形意钩 << "f;\n";

    file << "双戟左右3延迟=" << 振刀参数::延迟::双戟.左右3 << "f;\n";
    file << "双戟左蓄延迟=" << 振刀参数::延迟::双戟.左蓄 << "f;\n";
    file << "双戟右蓄延迟=" << 振刀参数::延迟::双戟.右蓄 << "f;\n";
    file << "双戟勾旋斩延迟=" << 振刀参数::延迟::双戟.勾旋斩 << "f;\n";
    file << "双戟探海蛟延迟=" << 振刀参数::延迟::双戟.探海蛟 << "f;\n";
    file << "双戟钩挂延迟=" << 振刀参数::延迟::双戟.钩挂 << "f;\n";
    file << "双戟壁击延迟=" << 振刀参数::延迟::双戟.壁击 << "f;\n";
    file << "双戟战龙在天延迟=" << 振刀参数::延迟::双戟.战龙在天 << "f;\n";
    file << "双戟形意钩延迟=" << 振刀参数::延迟::双戟.形意钩 << "f;\n";

    file << "双戟左右3角度=" << 振刀参数::角度::双戟.左右3 << "f;\n";
    file << "双戟左蓄角度=" << 振刀参数::角度::双戟.左蓄 << "f;\n";
    file << "双戟右蓄角度=" << 振刀参数::角度::双戟.右蓄 << "f;\n";
    file << "双戟壁击角度=" << 振刀参数::角度::双戟.壁击 << "f;\n";
    file << "双戟勾旋斩角度=" << 振刀参数::角度::双戟.勾旋斩 << "f;\n";
    file << "双戟探海蛟角度=" << 振刀参数::角度::双戟.探海蛟 << "f;\n";
    file << "双戟钩挂角度=" << 振刀参数::角度::双戟.钩挂 << "f;\n";
    file << "双戟战龙在天角度=" << 振刀参数::角度::双戟.战龙在天 << "f;\n";
    file << "双戟形意钩角度=" << 振刀参数::角度::双戟.形意钩 << "f;\n";

    file << "扇子左3闪避=" << 振刀参数::闪避::扇子.左3 << "f;\n";
    file << "扇子右3闪避=" << 振刀参数::闪避::扇子.右3 << "f;\n";
    file << "扇子左蓄闪避=" << 振刀参数::闪避::扇子.左蓄 << "f;\n";
    file << "扇子右蓄闪避=" << 振刀参数::闪避::扇子.右蓄 << "f;\n";
    file << "扇子双开圆闪避=" << 振刀参数::闪避::扇子.双开圆 << "f;\n";
    file << "扇子三风摆闪避=" << 振刀参数::闪避::扇子.三风摆 << "f;\n";
    file << "扇子鬼反闪避=" << 振刀参数::闪避::扇子.鬼反 << "f;\n";
    file << "扇子惊雷闪避=" << 振刀参数::闪避::扇子.惊雷 << "f;\n";
    file << "扇子壁击闪避=" << 振刀参数::闪避::扇子.壁击 << "f;\n";
    file << "扇子缠龙奔野闪避=" << 振刀参数::闪避::扇子.缠龙奔野 << "f;\n";
    file << "扇子泽风上六闪避=" << 振刀参数::闪避::扇子.泽风上六 << "f;\n";

    file << "扇子左3振刀方式=" << 振刀参数::振刀方式::扇子.左3 << "f;\n";
    file << "扇子右3振刀方式=" << 振刀参数::振刀方式::扇子.右3 << "f;\n";
    file << "扇子左蓄振刀方式=" << 振刀参数::振刀方式::扇子.左蓄 << "f;\n";
    file << "扇子右蓄振刀方式=" << 振刀参数::振刀方式::扇子.右蓄 << "f;\n";
    file << "扇子双开圆振刀方式=" << 振刀参数::振刀方式::扇子.双开圆 << "f;\n";
    file << "扇子三风摆振刀方式=" << 振刀参数::振刀方式::扇子.三风摆 << "f;\n";
    file << "扇子鬼反振刀方式=" << 振刀参数::振刀方式::扇子.鬼反 << "f;\n";
    file << "扇子惊雷振刀方式=" << 振刀参数::振刀方式::扇子.惊雷 << "f;\n";
    file << "扇子壁击振刀方式=" << 振刀参数::振刀方式::扇子.壁击 << "f;\n";
    file << "扇子缠龙奔野振刀方式=" << 振刀参数::振刀方式::扇子.缠龙奔野
         << "f;\n";
    file << "扇子泽风上六振刀方式=" << 振刀参数::振刀方式::扇子.泽风上六
         << "f;\n";

    file << "扇子左3距离=" << 振刀参数::距离::扇子.左3 << "f;\n";
    file << "扇子右3距离=" << 振刀参数::距离::扇子.右3 << "f;\n";
    file << "扇子左蓄距离=" << 振刀参数::距离::扇子.左蓄 << "f;\n";
    file << "扇子右蓄距离=" << 振刀参数::距离::扇子.右蓄 << "f;\n";
    file << "扇子壁击距离=" << 振刀参数::距离::扇子.壁击 << "f;\n";
    file << "扇子双开圆距离=" << 振刀参数::距离::扇子.双开圆 << "f;\n";
    file << "扇子三风摆距离=" << 振刀参数::距离::扇子.三风摆 << "f;\n";
    file << "扇子惊雷距离=" << 振刀参数::距离::扇子.惊雷 << "f;\n";
    file << "扇子缠龙奔野距离=" << 振刀参数::距离::扇子.缠龙奔野 << "f;\n";
    file << "扇子鬼反距离=" << 振刀参数::距离::扇子.鬼反 << "f;\n";

    file << "扇子左3延迟=" << 振刀参数::延迟::扇子.左3 << "f;\n";
    file << "扇子右3延迟=" << 振刀参数::延迟::扇子.右3 << "f;\n";
    file << "扇子左蓄延迟=" << 振刀参数::延迟::扇子.左蓄 << "f;\n";
    file << "扇子右蓄延迟=" << 振刀参数::延迟::扇子.右蓄 << "f;\n";
    file << "扇子双开圆延迟=" << 振刀参数::延迟::扇子.双开圆 << "f;\n";
    file << "扇子三风摆延迟=" << 振刀参数::延迟::扇子.三风摆 << "f;\n";
    file << "扇子惊雷延迟=" << 振刀参数::延迟::扇子.惊雷 << "f;\n";
    file << "扇子壁击延迟=" << 振刀参数::延迟::扇子.壁击 << "f;\n";
    file << "扇子缠龙奔野延迟=" << 振刀参数::延迟::扇子.缠龙奔野 << "f;\n";
    file << "扇子鬼反延迟=" << 振刀参数::延迟::扇子.鬼反 << "f;\n";

    file << "扇子左3角度=" << 振刀参数::角度::扇子.左3 << "f;\n";
    file << "扇子右3角度=" << 振刀参数::角度::扇子.右3 << "f;\n";
    file << "扇子左蓄角度=" << 振刀参数::角度::扇子.左蓄 << "f;\n";
    file << "扇子右蓄角度=" << 振刀参数::角度::扇子.右蓄 << "f;\n";
    file << "扇子双开圆角度=" << 振刀参数::角度::扇子.双开圆 << "f;\n";
    file << "扇子三风摆角度=" << 振刀参数::角度::扇子.三风摆 << "f;\n";
    file << "扇子惊雷角度=" << 振刀参数::角度::扇子.惊雷 << "f;\n";
    file << "扇子壁击角度=" << 振刀参数::角度::扇子.壁击 << "f;\n";
    file << "扇子缠龙奔野角度=" << 振刀参数::角度::扇子.缠龙奔野 << "f;\n";
    file << "扇子鬼反角度=" << 振刀参数::角度::扇子.鬼反 << "f;\n";

    file << "横刀左3闪避=" << 振刀参数::闪避::横刀.左3 << "f;\n";
    file << "横刀右3闪避=" << 振刀参数::闪避::横刀.右3 << "f;\n";
    file << "横刀左蓄闪避=" << 振刀参数::闪避::横刀.左蓄 << "f;\n";
    file << "横刀右蓄1闪避=" << 振刀参数::闪避::横刀.右蓄1 << "f;\n";
    file << "横刀右蓄2闪避=" << 振刀参数::闪避::横刀.右蓄2 << "f;\n";
    file << "横刀苍牙闪避=" << 振刀参数::闪避::横刀.苍牙 << "f;\n";
    file << "横刀壁击闪避=" << 振刀参数::闪避::横刀.壁击 << "f;\n";
    file << "横刀同源闪避=" << 振刀参数::闪避::横刀.同源 << "f;\n";
    file << "横刀踏空闪闪避=" << 振刀参数::闪避::横刀.踏空闪 << "f;\n";
    file << "横刀破千军闪避=" << 振刀参数::闪避::横刀.破千军 << "f;\n";
    file << "横刀极光破云闪闪避=" << 振刀参数::闪避::横刀.极光破云闪 << "f;\n";
    file << "横刀乾坤一掷闪避=" << 振刀参数::闪避::横刀.乾坤一掷 << "f;\n";

    file << "横刀左3振刀方式=" << 振刀参数::振刀方式::横刀.左3 << "f;\n";
    file << "横刀右3振刀方式=" << 振刀参数::振刀方式::横刀.右3 << "f;\n";
    file << "横刀左蓄振刀方式=" << 振刀参数::振刀方式::横刀.左蓄 << "f;\n";
    file << "横刀右蓄1振刀方式=" << 振刀参数::振刀方式::横刀.右蓄1 << "f;\n";
    file << "横刀右蓄2振刀方式=" << 振刀参数::振刀方式::横刀.右蓄2 << "f;\n";
    file << "横刀苍牙振刀方式=" << 振刀参数::振刀方式::横刀.苍牙 << "f;\n";
    file << "横刀壁击振刀方式=" << 振刀参数::振刀方式::横刀.壁击 << "f;\n";
    file << "横刀同源振刀方式=" << 振刀参数::振刀方式::横刀.同源 << "f;\n";
    file << "横刀踏空闪振刀方式=" << 振刀参数::振刀方式::横刀.踏空闪 << "f;\n";
    file << "横刀破千军振刀方式=" << 振刀参数::振刀方式::横刀.破千军 << "f;\n";
    file << "横刀极光破云闪振刀方式=" << 振刀参数::振刀方式::横刀.极光破云闪
         << "f;\n";
    file << "横刀乾坤一掷振刀方式=" << 振刀参数::振刀方式::横刀.乾坤一掷
         << "f;\n";

    file << "横刀左3距离=" << 振刀参数::距离::横刀.左3 << "f;\n";
    file << "横刀右3距离=" << 振刀参数::距离::横刀.右3 << "f;\n";
    file << "横刀左蓄距离=" << 振刀参数::距离::横刀.左蓄 << "f;\n";
    file << "横刀右蓄距离=" << 振刀参数::距离::横刀.右蓄 << "f;\n";
    file << "横刀壁击距离=" << 振刀参数::距离::横刀.壁击 << "f;\n";
    file << "横刀苍牙距离=" << 振刀参数::距离::横刀.苍牙 << "f;\n";
    file << "横刀同源距离=" << 振刀参数::距离::横刀.同源 << "f;\n";
    file << "横刀踏空闪距离=" << 振刀参数::距离::横刀.踏空闪 << "f;\n";
    file << "横刀破千军距离=" << 振刀参数::距离::横刀.破千军 << "f;\n";
    file << "横刀极光破云闪距离=" << 振刀参数::距离::横刀.极光破云闪 << "f;\n";

    file << "横刀左3延迟=" << 振刀参数::延迟::横刀.左3 << "f;\n";
    file << "横刀右3延迟=" << 振刀参数::延迟::横刀.右3 << "f;\n";
    file << "横刀左蓄延迟=" << 振刀参数::延迟::横刀.左蓄 << "f;\n";
    file << "横刀右蓄延迟=" << 振刀参数::延迟::横刀.右蓄 << "f;\n";
    file << "横刀壁击延迟=" << 振刀参数::延迟::横刀.壁击 << "f;\n";
    file << "横刀苍牙延迟=" << 振刀参数::延迟::横刀.苍牙 << "f;\n";
    file << "横刀同源延迟=" << 振刀参数::延迟::横刀.同源 << "f;\n";
    file << "横刀踏空闪延迟=" << 振刀参数::延迟::横刀.踏空闪 << "f;\n";
    file << "横刀破千军延迟=" << 振刀参数::延迟::横刀.破千军 << "f;\n";
    file << "横刀极光破云闪延迟=" << 振刀参数::延迟::横刀.极光破云闪 << "f;\n";

    file << "横刀左3角度=" << 振刀参数::角度::横刀.左3 << "f;\n";
    file << "横刀右3角度=" << 振刀参数::角度::横刀.右3 << "f;\n";
    file << "横刀左蓄角度=" << 振刀参数::角度::横刀.左蓄 << "f;\n";
    file << "横刀右蓄角度=" << 振刀参数::角度::横刀.右蓄 << "f;\n";
    file << "横刀壁击角度=" << 振刀参数::角度::横刀.壁击 << "f;\n";
    file << "横刀苍牙角度=" << 振刀参数::角度::横刀.苍牙 << "f;\n";
    file << "横刀同源角度=" << 振刀参数::角度::横刀.同源 << "f;\n";
    file << "横刀踏空闪角度=" << 振刀参数::角度::横刀.踏空闪 << "f;\n";
    file << "横刀破千军角度=" << 振刀参数::角度::横刀.破千军 << "f;\n";
    file << "横刀极光破云闪角度=" << 振刀参数::角度::横刀.极光破云闪 << "f;\n";

    file << "拳刃左3闪避=" << 振刀参数::闪避::拳刃.左3 << "f;\n";
    file << "拳刃右3闪避=" << 振刀参数::闪避::拳刃.右3 << "f;\n";
    file << "拳刃左蓄力1闪避=" << 振刀参数::闪避::拳刃.左蓄力1 << "f;\n";
    file << "拳刃左蓄力2闪避=" << 振刀参数::闪避::拳刃.左蓄力2 << "f;\n";
    file << "拳刃右蓄力1闪避=" << 振刀参数::闪避::拳刃.右蓄力1 << "f;\n";
    file << "拳刃右蓄力2闪避=" << 振刀参数::闪避::拳刃.右蓄力2 << "f;\n";
    file << "拳刃右蓄力2段闪避=" << 振刀参数::闪避::拳刃.右蓄力2段 << "f;\n";
    file << "拳刃百裂腿闪避=" << 振刀参数::闪避::拳刃.百裂腿 << "f;\n";
    file << "拳刃破空拳闪避=" << 振刀参数::闪避::拳刃.破空拳 << "f;\n";
    file << "拳刃苍牙闪避=" << 振刀参数::闪避::拳刃.苍牙 << "f;\n";
    file << "拳刃壁击闪避=" << 振刀参数::闪避::拳刃.壁击 << "f;\n";
    file << "拳刃太极弄云手闪避=" << 振刀参数::闪避::拳刃.太极弄云手 << "f;\n";

    file << "拳刃左3振刀方式=" << 振刀参数::振刀方式::拳刃.左3 << "f;\n";
    file << "拳刃右3振刀方式=" << 振刀参数::振刀方式::拳刃.右3 << "f;\n";
    file << "拳刃左蓄力1振刀方式=" << 振刀参数::振刀方式::拳刃.左蓄力1
         << "f;\n";
    file << "拳刃左蓄力2振刀方式=" << 振刀参数::振刀方式::拳刃.左蓄力2
         << "f;\n";
    file << "拳刃右蓄力1振刀方式=" << 振刀参数::振刀方式::拳刃.右蓄力1
         << "f;\n";
    file << "拳刃右蓄力2振刀方式=" << 振刀参数::振刀方式::拳刃.右蓄力2
         << "f;\n";
    file << "拳刃右蓄力2段振刀方式=" << 振刀参数::振刀方式::拳刃.右蓄力2段
         << "f;\n";
    file << "拳刃百裂腿振刀方式=" << 振刀参数::振刀方式::拳刃.百裂腿 << "f;\n";
    file << "拳刃破空拳振刀方式=" << 振刀参数::振刀方式::拳刃.破空拳 << "f;\n";
    file << "拳刃苍牙振刀方式=" << 振刀参数::振刀方式::拳刃.苍牙 << "f;\n";
    file << "拳刃壁击振刀方式=" << 振刀参数::振刀方式::拳刃.壁击 << "f;\n";
    file << "拳刃太极弄云手振刀方式=" << 振刀参数::振刀方式::拳刃.太极弄云手
         << "f;\n";

    file << "拳刃左3延迟=" << 振刀参数::延迟::拳刃.左3 << "f;\n";
    file << "拳刃右3延迟=" << 振刀参数::延迟::拳刃.右3 << "f;\n";
    file << "拳刃左蓄延迟=" << 振刀参数::延迟::拳刃.左蓄力 << "f;\n";
    file << "拳刃右蓄延迟=" << 振刀参数::延迟::拳刃.右蓄力 << "f;\n";
    file << "拳刃苍牙延迟=" << 振刀参数::延迟::拳刃.苍牙 << "f;\n";
    file << "拳刃壁击延迟=" << 振刀参数::延迟::拳刃.壁击 << "f;\n";
    file << "拳刃百裂腿延迟=" << 振刀参数::延迟::拳刃.百裂腿 << "f;\n";
    file << "拳刃破空拳延迟=" << 振刀参数::延迟::拳刃.破空拳 << "f;\n";
    file << "拳刃右蓄2段延迟=" << 振刀参数::延迟::拳刃.右蓄力2段 << "f;\n";
    file << "拳刃太极弄云手延迟=" << 振刀参数::延迟::拳刃.太极弄云手 << "f;\n";

    file << "拳刃左3距离=" << 振刀参数::距离::拳刃.左3 << "f;\n";
    file << "拳刃右3距离=" << 振刀参数::距离::拳刃.右3 << "f;\n";
    file << "拳刃左蓄距离=" << 振刀参数::距离::拳刃.左蓄力 << "f;\n";
    file << "拳刃右蓄距离=" << 振刀参数::距离::拳刃.右蓄力 << "f;\n";
    file << "拳刃壁击距离=" << 振刀参数::距离::拳刃.壁击 << "f;\n";
    file << "拳刃右蓄2段距离=" << 振刀参数::距离::拳刃.右蓄力2段 << "f;\n";
    file << "拳刃苍牙距离=" << 振刀参数::距离::拳刃.苍牙 << "f;\n";
    file << "拳刃百裂腿距离=" << 振刀参数::距离::拳刃.百裂腿 << "f;\n";
    file << "拳刃破空拳距离=" << 振刀参数::距离::拳刃.破空拳 << "f;\n";
    file << "拳刃太极弄云手距离=" << 振刀参数::距离::拳刃.太极弄云手 << "f;\n";

    file << "拳刃左3角度=" << 振刀参数::角度::拳刃.左3 << "f;\n";
    file << "拳刃右3角度=" << 振刀参数::角度::拳刃.右3 << "f;\n";
    file << "拳刃左蓄角度=" << 振刀参数::角度::拳刃.左蓄力 << "f;\n";
    file << "拳刃右蓄角度=" << 振刀参数::角度::拳刃.右蓄力 << "f;\n";
    file << "拳刃壁击角度=" << 振刀参数::角度::拳刃.壁击 << "f;\n";
    file << "拳刃苍牙角度=" << 振刀参数::角度::拳刃.苍牙 << "f;\n";
    file << "拳刃百裂腿角度=" << 振刀参数::角度::拳刃.百裂腿 << "f;\n";
    file << "拳刃破空拳角度=" << 振刀参数::角度::拳刃.破空拳 << "f;\n";
    file << "拳刃右蓄2段角度=" << 振刀参数::角度::拳刃.右蓄力2段 << "f;\n";
    file << "拳刃太极弄云手角度=" << 振刀参数::角度::拳刃.太极弄云手 << "f;\n";

    file << "飞刀左3闪避=" << 振刀参数::闪避::飞刀.左3 << "f;\n";
    file << "飞刀右3闪避=" << 振刀参数::闪避::飞刀.右3 << "f;\n";
    file << "飞刀左蓄力闪避=" << 振刀参数::闪避::飞刀.左蓄力 << "f;\n";
    file << "飞刀左蓄力2段闪避=" << 振刀参数::闪避::飞刀.左蓄力2段 << "f;\n";
    file << "飞刀右蓄力闪避=" << 振刀参数::闪避::飞刀.右蓄力 << "f;\n";
    file << "飞刀掌心雷闪避=" << 振刀参数::闪避::飞刀.掌心雷 << "f;\n";
    file << "飞刀苍牙闪避=" << 振刀参数::闪避::飞刀.苍牙 << "f;\n";
    file << "飞刀太极闪避=" << 振刀参数::闪避::飞刀.太极 << "f;\n";
    file << "飞刀壁击闪避=" << 振刀参数::闪避::飞刀.壁击 << "f;\n";
    file << "飞刀断月千刃舞闪避=" << 振刀参数::闪避::飞刀.断月千刃舞 << "f;\n";
    file << "飞刀万点寒梅闪避=" << 振刀参数::闪避::飞刀.万点寒梅 << "f;\n";

    file << "飞刀左3振刀方式=" << 振刀参数::振刀方式::飞刀.左3 << "f;\n";
    file << "飞刀右3振刀方式=" << 振刀参数::振刀方式::飞刀.右3 << "f;\n";
    file << "飞刀左蓄力振刀方式=" << 振刀参数::振刀方式::飞刀.左蓄力 << "f;\n";
    file << "飞刀左蓄力2段振刀方式=" << 振刀参数::振刀方式::飞刀.左蓄力2段
         << "f;\n";
    file << "飞刀右蓄力振刀方式=" << 振刀参数::振刀方式::飞刀.右蓄力 << "f;\n";
    file << "飞刀掌心雷振刀方式=" << 振刀参数::振刀方式::飞刀.掌心雷 << "f;\n";
    file << "飞刀苍牙振刀方式=" << 振刀参数::振刀方式::飞刀.苍牙 << "f;\n";
    file << "飞刀太极振刀方式=" << 振刀参数::振刀方式::飞刀.太极 << "f;\n";
    file << "飞刀壁击振刀方式=" << 振刀参数::振刀方式::飞刀.壁击 << "f;\n";
    file << "飞刀断月千刃舞振刀方式=" << 振刀参数::振刀方式::飞刀.断月千刃舞
         << "f;\n";
    file << "飞刀万点寒梅振刀方式=" << 振刀参数::振刀方式::飞刀.万点寒梅
         << "f;\n";

    file << "飞刀左3距离=" << 振刀参数::距离::飞刀.左3 << "f;\n";
    file << "飞刀右3距离=" << 振刀参数::距离::飞刀.右3 << "f;\n";
    file << "飞刀左蓄距离=" << 振刀参数::距离::飞刀.左蓄力 << "f;\n";
    file << "飞刀左蓄2段距离=" << 振刀参数::距离::飞刀.左蓄力2段 << "f;\n";
    file << "飞刀右蓄距离=" << 振刀参数::距离::飞刀.右蓄力 << "f;\n";
    file << "飞刀壁击距离=" << 振刀参数::距离::飞刀.壁击 << "f;\n";
    file << "飞刀掌心雷距离=" << 振刀参数::距离::飞刀.掌心雷 << "f;\n";
    file << "飞刀苍牙距离=" << 振刀参数::距离::飞刀.苍牙 << "f;\n";
    file << "飞刀太极距离=" << 振刀参数::距离::飞刀.太极 << "f;\n";
    file << "飞刀太极弄云手距离=" << 振刀参数::距离::飞刀.断月千刃舞 << "f;\n";

    file << "飞刀左3角度=" << 振刀参数::角度::飞刀.左3 << "f;\n";
    file << "飞刀右3角度=" << 振刀参数::角度::飞刀.右3 << "f;\n";
    file << "飞刀左蓄角度=" << 振刀参数::角度::飞刀.左蓄力 << "f;\n";
    file << "飞刀左蓄2段角度=" << 振刀参数::角度::飞刀.左蓄力2段 << "f;\n";
    file << "飞刀右蓄角度=" << 振刀参数::角度::飞刀.右蓄力 << "f;\n";
    file << "飞刀壁击角度=" << 振刀参数::角度::飞刀.壁击 << "f;\n";
    file << "飞刀掌心雷角度=" << 振刀参数::角度::飞刀.掌心雷 << "f;\n";
    file << "飞刀太极角度=" << 振刀参数::角度::飞刀.太极 << "f;\n";
    file << "飞刀苍牙角度=" << 振刀参数::角度::飞刀.苍牙 << "f;\n";
    file << "飞刀断月千刃舞角度=" << 振刀参数::角度::飞刀.断月千刃舞 << "f;\n";

    file << "飞刀左3延迟=" << 振刀参数::延迟::飞刀.左3 << "f;\n";
    file << "飞刀右3延迟=" << 振刀参数::延迟::飞刀.右3 << "f;\n";
    file << "飞刀左蓄延迟=" << 振刀参数::延迟::飞刀.左蓄力 << "f;\n";
    file << "飞刀左蓄力2段延迟=" << 振刀参数::延迟::飞刀.左蓄力2段 << "f;\n";
    file << "飞刀右蓄延迟=" << 振刀参数::延迟::飞刀.右蓄力 << "f;\n";
    file << "飞刀太极延迟=" << 振刀参数::延迟::飞刀.太极 << "f;\n";
    file << "飞刀苍牙延迟=" << 振刀参数::延迟::飞刀.苍牙 << "f;\n";
    file << "飞刀断月千刃舞延迟=" << 振刀参数::延迟::飞刀.断月千刃舞 << "f;\n";
    //----------------------------------------------------------------------

    file.close();
  }

  if (!g_SkipDodgeParamSave) {
    保存闪避参数配置();
  }
}

void 重置间隔配置() {
  ConfigDefaults::ApplyShockIntervalConfig();
  保存模拟振配置();
  读取模拟振配置();
}

static void 重置间隔配置_旧实现_UNUSED() {
  振刀参数::闪避::长剑.左3 = 70;
  振刀参数::闪避::长剑.右3 = 70;
  振刀参数::闪避::长剑.左剑气1 = 70;
  振刀参数::闪避::长剑.左剑气2 = 70;
  振刀参数::闪避::长剑.右剑气2 = 70;
  振刀参数::闪避::长剑.右剑气1 = 70;
  振刀参数::闪避::长剑.凤凰羽1 = 70;
  振刀参数::闪避::长剑.凤凰羽2 = 70;
  振刀参数::闪避::长剑.苍牙 = 70;
  振刀参数::闪避::长剑.跳斩 = 70;
  振刀参数::闪避::长剑.壁击 = 70;
  振刀参数::闪避::长剑.七星夺窍 = 70;
  振刀参数::闪避::长剑.天外飞仙 = 70;
  振刀参数::闪避::链剑.左3 = 70;
  振刀参数::闪避::链剑.右3 = 70;
  振刀参数::闪避::链剑.左蓄力1 = 70;
  振刀参数::闪避::链剑.左蓄力2 = 70;
  振刀参数::闪避::链剑.右蓄力2 = 70;
  振刀参数::闪避::链剑.右蓄力1 = 70;
  振刀参数::闪避::链剑.地龙滚堂刹 = 70;
  振刀参数::闪避::链剑.苍牙 = 70;
  振刀参数::闪避::链剑.跳斩 = 70;
  振刀参数::闪避::链剑.壁击 = 70;
  振刀参数::闪避::链剑.火龙卷云 = 70;
  振刀参数::闪避::链剑.断罪碎蜂 = 70;
  振刀参数::闪避::链剑.蓄力追击 = 70;
  振刀参数::闪避::链剑.链剑长剑同源 = 70;
  振刀参数::闪避::太刀.左3 = 70;
  振刀参数::闪避::太刀.右3 = 70;
  振刀参数::闪避::太刀.左蓄1 = 70;
  振刀参数::闪避::太刀.左蓄2 = 70;
  振刀参数::闪避::太刀.右蓄2 = 70;
  振刀参数::闪避::太刀.右蓄1 = 70;
  振刀参数::闪避::太刀.刹那斩1 = 70;
  振刀参数::闪避::太刀.苍牙 = 70;
  振刀参数::闪避::太刀.惊雷 = 70;
  振刀参数::闪避::太刀.壁击 = 70;
  振刀参数::闪避::太刀.青鬼 = 70;
  振刀参数::闪避::太刀.同源 = 70;
  振刀参数::闪避::太刀.噬魂斩1 = 70;
  振刀参数::闪避::太刀.噬魂斩2 = 70;
  振刀参数::闪避::阔刀.左左 = 70;
  振刀参数::闪避::阔刀.左右 = 70;
  振刀参数::闪避::阔刀.右右 = 70;
  振刀参数::闪避::阔刀.右左 = 70;
  振刀参数::闪避::阔刀.左蓄一段 = 70;
  振刀参数::闪避::阔刀.左蓄二段 = 70;
  振刀参数::闪避::阔刀.左蓄三段 = 70;
  振刀参数::闪避::阔刀.右蓄1 = 70;
  振刀参数::闪避::阔刀.右蓄2 = 70;
  振刀参数::闪避::阔刀.右右蓄 = 70;
  振刀参数::闪避::阔刀.雷刀 = 70;
  振刀参数::闪避::阔刀.壁击 = 70;
  振刀参数::闪避::阔刀.同源 = 70;
  振刀参数::闪避::阔刀.雷刀下劈 = 70;
  振刀参数::闪避::阔刀.翻江倒海 = 70;
  振刀参数::闪避::长枪.左3 = 70;
  振刀参数::闪避::长枪.右3 = 70;
  振刀参数::闪避::长枪.左蓄 = 70;
  振刀参数::闪避::长枪.六合枪 = 70;
  振刀参数::闪避::长枪.右蓄 = 70;
  振刀参数::闪避::长枪.大圣游 = 70;
  振刀参数::闪避::长枪.风卷云残 = 70;
  振刀参数::闪避::长枪.壁击 = 70;
  振刀参数::闪避::长枪.同源 = 70;
  振刀参数::闪避::长枪.双环扫 = 70;
  振刀参数::闪避::长枪.龙王破 = 70;
  振刀参数::闪避::长枪.穿心脚 = 70;
  振刀参数::闪避::匕首.左3 = 70;
  振刀参数::闪避::匕首.右3 = 70;
  振刀参数::闪避::匕首.左蓄 = 70;
  振刀参数::闪避::匕首.右蓄 = 70;
  振刀参数::闪避::匕首.鬼反 = 70;
  振刀参数::闪避::匕首.荆轲献匕 = 70;
  振刀参数::闪避::匕首.壁击 = 70;
  振刀参数::闪避::匕首.鬼刃暗扎 = 70;
  振刀参数::闪避::匕首.鬼哭神嚎 = 70;
  振刀参数::闪避::匕首.亢龙有悔 = 70;
  振刀参数::闪避::双截棍.左右3 = 70;
  振刀参数::闪避::双截棍.左蓄 = 70;
  振刀参数::闪避::双截棍.右蓄 = 70;
  振刀参数::闪避::双截棍.飞踢 = 70;
  振刀参数::闪避::双截棍.壁击 = 70;
  振刀参数::闪避::双截棍.扬鞭劲 = 70;
  振刀参数::闪避::双截棍.横栏 = 70;
  振刀参数::闪避::双截棍.龙虎乱舞 = 70;
  振刀参数::闪避::双截棍.三龙灭阳棍 = 70;
  振刀参数::闪避::双刀.左右3 = 70;
  振刀参数::闪避::双刀.左蓄 = 70;
  振刀参数::闪避::双刀.右蓄1 = 70;
  振刀参数::闪避::双刀.右蓄2 = 70;
  振刀参数::闪避::双刀.铁马残红 = 70;
  振刀参数::闪避::双刀.壁击 = 70;
  振刀参数::闪避::双刀.乾坤日月斩 = 70;
  振刀参数::闪避::双刀.惊雷 = 70;
  振刀参数::闪避::双刀.八斩刀 = 70;
  振刀参数::闪避::双刀.分水斩 = 70;
  振刀参数::闪避::长棍.右3 = 70;
  振刀参数::闪避::长棍.左3 = 70;
  振刀参数::闪避::长棍.左蓄 = 70;
  振刀参数::闪避::长棍.右蓄 = 70;
  振刀参数::闪避::长棍.壁击 = 70;
  振刀参数::闪避::长棍.同源 = 70;
  振刀参数::闪避::长棍.腾云式 = 70;
  振刀参数::闪避::长棍.乱点天宫 = 70;
  振刀参数::闪避::长棍.桶劲 = 70;
  振刀参数::闪避::长棍.五情七灭镇 = 70;
  振刀参数::闪避::长棍.双环扫 = 70;
  振刀参数::闪避::长棍.少林棍 = 70;
  振刀参数::闪避::长棍.镇地撑天 = 70;
  振刀参数::闪避::斩马刀.左左 = 70;
  振刀参数::闪避::斩马刀.左右 = 70;
  振刀参数::闪避::斩马刀.右右 = 70;
  振刀参数::闪避::斩马刀.右左 = 70;
  振刀参数::闪避::斩马刀.左蓄1 = 70;
  振刀参数::闪避::斩马刀.左蓄2 = 70;
  振刀参数::闪避::斩马刀.左蓄3 = 70;
  振刀参数::闪避::斩马刀.右蓄1 = 70;
  振刀参数::闪避::斩马刀.右蓄2 = 70;
  振刀参数::闪避::斩马刀.右蓄3 = 70;
  振刀参数::闪避::斩马刀.柄击 = 70;
  振刀参数::闪避::斩马刀.壁击 = 70;
  振刀参数::闪避::斩马刀.同源 = 70;
  振刀参数::闪避::斩马刀.惊雷 = 70;
  振刀参数::闪避::斩马刀.炽焰斩 = 70;
  振刀参数::闪避::斩马刀.过关斩将 = 70;
  振刀参数::闪避::斩马刀.奔雷入阵 = 70;
  振刀参数::闪避::双戟.左3 = 70;
  振刀参数::闪避::双戟.右3 = 70;
  振刀参数::闪避::双戟.左蓄 = 70;
  振刀参数::闪避::双戟.右蓄1 = 70;
  振刀参数::闪避::双戟.右蓄2 = 70;
  振刀参数::闪避::双戟.惊雷 = 70;
  振刀参数::闪避::双戟.勾旋斩 = 70;
  振刀参数::闪避::双戟.探海蛟 = 70;
  振刀参数::闪避::双戟.钩挂 = 70;
  振刀参数::闪避::双戟.壁击 = 70;
  振刀参数::闪避::双戟.战龙在天 = 70;
  振刀参数::闪避::双戟.形意钩 = 70;
  振刀参数::闪避::扇子.左3 = 70;
  振刀参数::闪避::扇子.右3 = 70;
  振刀参数::闪避::扇子.左蓄 = 70;
  振刀参数::闪避::扇子.右蓄 = 70;
  振刀参数::闪避::扇子.双开圆 = 70;
  振刀参数::闪避::扇子.三风摆 = 70;
  振刀参数::闪避::扇子.鬼反 = 70;
  振刀参数::闪避::扇子.惊雷 = 70;
  振刀参数::闪避::扇子.壁击 = 70;
  振刀参数::闪避::扇子.缠龙奔野 = 70;
  振刀参数::闪避::扇子.泽风上六 = 70;
  振刀参数::闪避::横刀.左3 = 70;
  振刀参数::闪避::横刀.右3 = 70;
  振刀参数::闪避::横刀.左蓄 = 70;
  振刀参数::闪避::横刀.右蓄1 = 70;
  振刀参数::闪避::横刀.右蓄2 = 70;
  振刀参数::闪避::横刀.苍牙 = 70;
  振刀参数::闪避::横刀.壁击 = 70;
  振刀参数::闪避::横刀.同源 = 70;
  振刀参数::闪避::横刀.踏空闪 = 70;
  振刀参数::闪避::横刀.破千军 = 70;
  振刀参数::闪避::横刀.极光破云闪 = 70;
  振刀参数::闪避::横刀.乾坤一掷 = 70;
  振刀参数::闪避::拳刃.左3 = 70;
  振刀参数::闪避::拳刃.右3 = 70;
  振刀参数::闪避::拳刃.左蓄力1 = 70;
  振刀参数::闪避::拳刃.左蓄力2 = 70;
  振刀参数::闪避::拳刃.右蓄力1 = 70;
  振刀参数::闪避::拳刃.右蓄力2 = 70;
  振刀参数::闪避::拳刃.右蓄力2段 = 70;
  振刀参数::闪避::拳刃.百裂腿 = 70;
  振刀参数::闪避::拳刃.破空拳 = 70;
  振刀参数::闪避::拳刃.苍牙 = 70;
  振刀参数::闪避::拳刃.壁击 = 70;
  振刀参数::闪避::拳刃.太极弄云手 = 70;
  振刀参数::闪避::飞刀.左3 = 70;
  振刀参数::闪避::飞刀.右3 = 70;
  振刀参数::闪避::飞刀.左蓄力 = 70;
  振刀参数::闪避::飞刀.左蓄力2段 = 70;
  振刀参数::闪避::飞刀.右蓄力 = 70;
  振刀参数::闪避::飞刀.掌心雷 = 70;
  振刀参数::闪避::飞刀.苍牙 = 70;
  振刀参数::闪避::飞刀.太极 = 70;
  振刀参数::闪避::飞刀.壁击 = 70;
  振刀参数::闪避::飞刀.断月千刃舞 = 70;
  振刀参数::闪避::飞刀.万点寒梅 = 70;
  保存模拟振配置();
  读取模拟振配置();
}

void 重置模拟振配置() {
  ConfigDefaults::初始化振刀配置();
  保存模拟振配置();
  读取模拟振配置();
}

static void 重置模拟振配置_旧实现_UNUSED() {
  振刀参数::距离::长剑.左3 = 7.0f;
  振刀参数::距离::长剑.右3 = 7.0f;
  振刀参数::距离::长剑.左剑气 = 12.f;
  振刀参数::距离::长剑.右剑气 = 12.f;
  振刀参数::距离::长剑.凤凰羽 = 14.f;
  振刀参数::距离::长剑.苍牙 = 14.f;
  振刀参数::距离::长剑.跳斩 = 5.5f;
  振刀参数::距离::长剑.壁击 = 14.f;
  振刀参数::距离::长剑.七星夺窍 = 12.f;
  振刀参数::距离::长剑.天外飞仙 = 15.f;
  振刀参数::距离::链剑.左3 = 7.0f;
  振刀参数::距离::链剑.右3 = 7.0f;
  振刀参数::距离::链剑.左蓄力 = 12.f;
  振刀参数::距离::链剑.右蓄力 = 12.f;
  振刀参数::距离::链剑.地龙滚堂刹 = 16.f;
  振刀参数::距离::链剑.火龙卷云 = 14.f;
  振刀参数::距离::链剑.断罪碎蜂 = 14.5f;
  振刀参数::距离::链剑.壁击 = 14.f;
  振刀参数::距离::链剑.蓄力追击 = 7.f;
  振刀参数::距离::链剑.链剑长剑同源 = 14.f;
  振刀参数::距离::链剑.苍牙 = 7.f;
  振刀参数::距离::太刀.左3 = 6.5f;
  振刀参数::距离::太刀.右3 = 6.5f;
  振刀参数::距离::太刀.左蓄 = 5.3f;
  振刀参数::距离::太刀.右蓄 = 5.3f;
  振刀参数::距离::太刀.苍牙 = 5.f;
  振刀参数::距离::太刀.刹那斩 = 5.5f;
  振刀参数::距离::太刀.壁击 = 14.f;
  振刀参数::距离::太刀.惊雷 = 7.5f;
  振刀参数::距离::太刀.惊雷十劫 = 10.5f;
  振刀参数::距离::太刀.青鬼 = 6.0f;
  振刀参数::距离::太刀.同源 = 7.5f;
  振刀参数::距离::太刀.噬魂斩 = 12.5f;
  振刀参数::距离::阔刀.左左 = 6.5f;
  振刀参数::距离::阔刀.左右 = 5.5f;
  振刀参数::距离::阔刀.右右 = 4.2f;
  振刀参数::距离::阔刀.右左 = 4.3f;
  振刀参数::距离::阔刀.壁击 = 10.5f;
  振刀参数::距离::阔刀.左蓄一段 = 7.f;
  振刀参数::距离::阔刀.左蓄二段 = 8.f;
  振刀参数::距离::阔刀.左蓄三段 = 9.f;
  振刀参数::距离::阔刀.右蓄 = 7.f;
  振刀参数::距离::阔刀.右右蓄 = 8.5f;
  振刀参数::距离::阔刀.雷刀 = 7.f;
  振刀参数::距离::阔刀.雷刀下劈 = 4.5f;
  振刀参数::距离::阔刀.同源 = 7.5f;
  振刀参数::距离::长枪.左3 = 4.5f;
  振刀参数::距离::长枪.右3 = 4.5f;
  振刀参数::距离::长枪.左蓄 = 4.5f;
  振刀参数::距离::长枪.六合枪 = 8.5f;
  振刀参数::距离::长枪.右蓄 = 8.3f;
  振刀参数::距离::长枪.大圣游 = 6.2f;
  振刀参数::距离::长枪.同源 = 6.2f;
  振刀参数::距离::长枪.壁击 = 12.5f;
  振刀参数::距离::长枪.风卷云残 = 5.f;
  振刀参数::距离::长枪.双环扫 = 4.5f;
  振刀参数::距离::长枪.龙王破 = 13.f;
  振刀参数::距离::长枪.穿心脚 = 10.f;
  振刀参数::距离::匕首.左3 = 4.5f;
  振刀参数::距离::匕首.右3 = 4.f;
  振刀参数::距离::匕首.左蓄 = 5.8f;
  振刀参数::距离::匕首.右蓄 = 4.f;
  振刀参数::距离::匕首.鬼反 = 5.5f;
  振刀参数::距离::匕首.壁击 = 12.5f;
  振刀参数::距离::匕首.荆轲献匕 = 8.f;
  振刀参数::距离::匕首.鬼刃暗扎 = 6.5f;
  振刀参数::距离::匕首.鬼哭神嚎 = 6.f;
  振刀参数::距离::匕首.亢龙有悔 = 10.f;
  振刀参数::距离::双截棍.左右3 = 4.5f;
  振刀参数::距离::双截棍.左蓄 = 6.f;
  振刀参数::距离::双截棍.壁击 = 12.5f;
  振刀参数::距离::双截棍.右蓄 = 4.5f;
  振刀参数::距离::双截棍.飞踢 = 4.5f;
  振刀参数::距离::双截棍.扬鞭劲 = 4.5f;
  振刀参数::距离::双截棍.横栏 = 3.5f;
  振刀参数::距离::双截棍.龙虎乱舞 = 6.f;
  振刀参数::距离::双刀.左右3 = 4.5f;
  振刀参数::距离::双刀.左蓄 = 5.5f;
  振刀参数::距离::双刀.右蓄 = 4.8f;
  振刀参数::距离::双刀.壁击 = 12.5f;
  振刀参数::距离::双刀.铁马残红 = 5.f;
  振刀参数::距离::双刀.乾坤日月斩 = 10.f;
  振刀参数::距离::双刀.惊雷 = 4.5f;
  振刀参数::距离::双刀.八斩刀 = 6.f;
  振刀参数::距离::双刀.分水斩 = 4.5f;
  振刀参数::距离::长棍.左3 = 4.5f;
  振刀参数::距离::长棍.右3 = 4.5f;
  振刀参数::距离::长棍.少林棍 = 4.5f;
  振刀参数::距离::长棍.左蓄 = 5.7f;
  振刀参数::距离::长棍.右蓄 = 6.8f;
  振刀参数::距离::长棍.壁击 = 12.5f;
  振刀参数::距离::长棍.腾云式 = 7.5f;
  振刀参数::距离::长棍.乱点天宫 = 5.f;
  振刀参数::距离::长棍.桶劲 = 8.f;
  振刀参数::距离::长棍.同源 = 5.7f;
  振刀参数::距离::长棍.五情七灭镇 = 13.f;
  振刀参数::距离::长棍.双环扫 = 4.5f;
  振刀参数::距离::长棍.镇地撑天 = 5.5f;
  振刀参数::距离::斩马刀.左左 = 6.f;
  振刀参数::距离::斩马刀.左右 = 5.5f;
  振刀参数::距离::斩马刀.右右 = 4.f;
  振刀参数::距离::斩马刀.右左 = 4.5f;
  振刀参数::距离::斩马刀.左蓄 = 5.f;
  振刀参数::距离::斩马刀.右蓄 = 5.8f;
  振刀参数::距离::斩马刀.柄击 = 6.5f;
  振刀参数::距离::斩马刀.惊雷 = 5.f;
  振刀参数::距离::斩马刀.同源 = 5.f;
  振刀参数::距离::斩马刀.壁击 = 12.5f;
  振刀参数::距离::斩马刀.炽焰斩 = 7.f;
  振刀参数::距离::斩马刀.过关斩将 = 8.3f;
  振刀参数::距离::斩马刀.奔雷入阵 = 4.f;
  振刀参数::距离::双戟.左右3 = 4.5f;
  振刀参数::距离::双戟.左蓄 = 5.f;
  振刀参数::距离::双戟.右蓄 = 6.f;
  振刀参数::距离::双戟.勾旋斩 = 5.f;
  振刀参数::距离::双戟.探海蛟 = 7.f;
  振刀参数::距离::双戟.钩挂 = 6.f;
  振刀参数::距离::双戟.壁击 = 12.5f;
  振刀参数::距离::双戟.战龙在天 = 6.4f;
  振刀参数::距离::双戟.形意钩 = 11.f;
  振刀参数::距离::扇子.左3 = 4.5f;
  振刀参数::距离::扇子.右3 = 4.f;
  振刀参数::距离::扇子.左蓄 = 8.f;
  振刀参数::距离::扇子.右蓄 = 8.f;
  振刀参数::距离::扇子.双开圆 = 6.f;
  振刀参数::距离::扇子.三风摆 = 9.5f;
  振刀参数::距离::扇子.鬼反 = 6.f;
  振刀参数::距离::扇子.惊雷 = 5.5f;
  振刀参数::距离::扇子.壁击 = 12.5f;
  振刀参数::距离::扇子.缠龙奔野 = 11.5f;
  振刀参数::距离::扇子.泽风上六 = 5.5f;
  振刀参数::距离::横刀.左3 = 4.5f;
  振刀参数::距离::横刀.右3 = 4.8f;
  振刀参数::距离::横刀.左蓄 = 5.f;
  振刀参数::距离::横刀.右蓄 = 6.5f;
  振刀参数::距离::横刀.苍牙 = 5.5f;
  振刀参数::距离::横刀.壁击 = 12.5f;
  振刀参数::距离::横刀.同源 = 6.5f;
  振刀参数::距离::横刀.踏空闪 = 4.5f;
  振刀参数::距离::横刀.破千军 = 5.f;
  振刀参数::距离::横刀.乾坤一掷 = 10.f;
  振刀参数::距离::横刀.极光破云闪 = 8.f;
  振刀参数::距离::拳刃.左3 = 4.5f;
  振刀参数::距离::拳刃.右3 = 4.5f;
  振刀参数::距离::拳刃.左蓄力 = 4.f;
  振刀参数::距离::拳刃.右蓄力 = 8.f;
  振刀参数::距离::拳刃.右蓄力2段 = 8.f;
  振刀参数::距离::拳刃.苍牙 = 4.5f;
  振刀参数::距离::拳刃.百裂腿 = 5.5f;
  振刀参数::距离::拳刃.破空拳 = 4.5f;
  振刀参数::距离::拳刃.壁击 = 12.5f;
  振刀参数::距离::拳刃.太极弄云手 = 8.f;
  振刀参数::距离::飞刀.左3 = 4.5f;
  振刀参数::距离::飞刀.右3 = 4.5f;
  振刀参数::距离::飞刀.左蓄力 = 9.5f;
  振刀参数::距离::飞刀.左蓄力2段 = 13.5f;
  振刀参数::距离::飞刀.右蓄力 = 13.5f;
  振刀参数::距离::飞刀.掌心雷 = 14.5f;
  振刀参数::距离::飞刀.苍牙 = 5.5f;
  振刀参数::距离::飞刀.太极 = 7.5f;
  振刀参数::距离::飞刀.壁击 = 12.5f;
  振刀参数::距离::飞刀.断月千刃舞 = 14.5f;
  振刀参数::距离::飞刀.万点寒梅 = 7.5f;

  振刀参数::角度::长剑.左3 = 90.f;
  振刀参数::角度::长剑.右3 = 90.f;
  振刀参数::角度::长剑.左剑气 = 90.f;
  振刀参数::角度::长剑.右剑气 = 60.f;
  振刀参数::角度::长剑.凤凰羽 = 90.f;
  振刀参数::角度::长剑.苍牙 = 90.f;
  振刀参数::角度::长剑.跳斩 = 120.f;
  振刀参数::角度::长剑.壁击 = 90.f;
  振刀参数::角度::长剑.七星夺窍 = 45.f;
  振刀参数::角度::长剑.天外飞仙 = 120.f;
  振刀参数::角度::链剑.左3 = 90.f;
  振刀参数::角度::链剑.右3 = 90.f;
  振刀参数::角度::链剑.左蓄力 = 90.f;
  振刀参数::角度::链剑.右蓄力 = 90.f;
  振刀参数::角度::链剑.地龙滚堂刹 = 90.f;
  振刀参数::角度::链剑.火龙卷云 = 90.f;
  振刀参数::角度::链剑.断罪碎蜂 = 90.5f;
  振刀参数::角度::链剑.壁击 = 90.f;
  振刀参数::角度::链剑.蓄力追击 = 90.f;
  振刀参数::角度::链剑.链剑长剑同源 = 90.f;
  振刀参数::角度::太刀.左3 = 90.f;
  振刀参数::角度::太刀.右3 = 90.f;
  振刀参数::角度::太刀.左蓄 = 90.f;
  振刀参数::角度::太刀.右蓄 = 90.f;
  振刀参数::角度::太刀.苍牙 = 90.f;
  振刀参数::角度::太刀.壁击 = 90.f;
  振刀参数::角度::太刀.刹那斩 = 120.f;
  振刀参数::角度::太刀.惊雷十劫 = 90.f;
  振刀参数::角度::太刀.青鬼 = 90.f;
  振刀参数::角度::太刀.同源 = 90.f;
  振刀参数::角度::太刀.噬魂斩 = 360.f;
  振刀参数::角度::阔刀.左左 = 90.f;
  振刀参数::角度::阔刀.左右 = 60.f;
  振刀参数::角度::阔刀.右右 = 30.f;
  振刀参数::角度::阔刀.右左 = 90.f;
  振刀参数::角度::阔刀.壁击 = 90.f;
  振刀参数::角度::阔刀.同源 = 90.f;
  振刀参数::角度::阔刀.左蓄一段 = 90.f;
  振刀参数::角度::阔刀.左蓄二段 = 90.f;
  振刀参数::角度::阔刀.左蓄三段 = 190.f;
  振刀参数::角度::阔刀.右蓄 = 90.f;
  振刀参数::角度::阔刀.右右蓄 = 90.f;
  振刀参数::角度::长枪.左3 = 90.f;
  振刀参数::角度::长枪.右3 = 90.f;
  振刀参数::角度::长枪.左蓄 = 90.f;
  振刀参数::角度::长枪.六合枪 = 90.f;
  振刀参数::角度::长枪.右蓄 = 90.f;
  振刀参数::角度::长枪.壁击 = 90.f;
  振刀参数::角度::长枪.同源 = 90.f;
  振刀参数::角度::长枪.大圣游 = 90.f;
  振刀参数::角度::长枪.双环扫 = 90.f;
  振刀参数::角度::长枪.风卷云残 = 90.f;
  振刀参数::角度::长枪.龙王破 = 90.f;
  振刀参数::角度::长枪.穿心脚 = 90.f;
  振刀参数::角度::匕首.左3 = 90.f;
  振刀参数::角度::匕首.右3 = 90.f;
  振刀参数::角度::匕首.左蓄 = 90.f;
  振刀参数::角度::匕首.右蓄 = 90.f;
  振刀参数::角度::匕首.鬼反 = 90.f;
  振刀参数::角度::匕首.壁击 = 90.f;
  振刀参数::角度::匕首.荆轲献匕 = 90.f;
  振刀参数::角度::匕首.鬼刃暗扎 = 90.f;
  振刀参数::角度::匕首.亢龙有悔 = 90.f;
  振刀参数::角度::双截棍.左右3 = 90.f;
  振刀参数::角度::双截棍.左蓄 = 90.f;
  振刀参数::角度::双截棍.右蓄 = 90.f;
  振刀参数::角度::双截棍.飞踢 = 90.f;
  振刀参数::角度::双截棍.壁击 = 90.f;
  振刀参数::角度::双截棍.扬鞭劲 = 30.f;
  振刀参数::角度::双截棍.横栏 = 60.f;
  振刀参数::角度::双刀.左右3 = 90.f;
  振刀参数::角度::双刀.左蓄 = 60.f;
  振刀参数::角度::双刀.右蓄 = 90.f;
  振刀参数::角度::双刀.壁击 = 90.f;
  振刀参数::角度::双刀.分水斩 = 90.f;
  振刀参数::角度::双刀.铁马残红 = 90.f;
  振刀参数::角度::双刀.乾坤日月斩 = 90.f;
  振刀参数::角度::双刀.惊雷 = 90.f;
  振刀参数::角度::双刀.八斩刀 = 60.f;
  振刀参数::角度::长棍.左3 = 90.f;
  振刀参数::角度::长棍.右3 = 90.f;
  振刀参数::角度::长棍.左蓄 = 90.f;
  振刀参数::角度::长棍.右蓄 = 90.f;
  振刀参数::角度::长棍.壁击 = 90.f;
  振刀参数::角度::长棍.腾云式 = 90.f;
  振刀参数::角度::长棍.乱点天宫 = 90.f;
  振刀参数::角度::长棍.桶劲 = 90.f;
  振刀参数::角度::长棍.同源 = 90.f;
  振刀参数::角度::长棍.五情七灭镇 = 90.f;
  振刀参数::角度::长棍.双环扫 = 90.f;
  振刀参数::角度::长棍.少林棍 = 90.f;
  振刀参数::角度::长棍.镇地撑天 = 120.f;
  振刀参数::角度::斩马刀.左左 = 90.f;
  振刀参数::角度::斩马刀.左右 = 90.f;
  振刀参数::角度::斩马刀.右右 = 30.f;
  振刀参数::角度::斩马刀.右左 = 90.f;
  振刀参数::角度::斩马刀.左蓄 = 90.f;
  振刀参数::角度::斩马刀.右蓄 = 90.f;
  振刀参数::角度::斩马刀.柄击 = 90.f;
  振刀参数::角度::斩马刀.壁击 = 90.f;
  振刀参数::角度::斩马刀.同源 = 90.f;
  振刀参数::角度::斩马刀.惊雷 = 90.f;
  振刀参数::角度::斩马刀.炽焰斩 = 90.f;
  振刀参数::角度::斩马刀.过关斩将 = 90.f;
  振刀参数::角度::斩马刀.奔雷入阵 = 90.f;
  振刀参数::角度::双戟.左右3 = 90.f;
  振刀参数::角度::双戟.左蓄 = 90.f;
  振刀参数::角度::双戟.右蓄 = 90.f;
  振刀参数::角度::双戟.勾旋斩 = 90.f;
  振刀参数::角度::双戟.探海蛟 = 90.f;
  振刀参数::角度::双戟.钩挂 = 90.f;
  振刀参数::角度::双戟.壁击 = 90.f;
  振刀参数::角度::双戟.战龙在天 = 90.f;
  振刀参数::角度::双戟.形意钩 = 90.f;
  振刀参数::角度::扇子.左3 = 90.f;
  振刀参数::角度::扇子.右3 = 90.f;
  振刀参数::角度::扇子.左蓄 = 90.f;
  振刀参数::角度::扇子.右蓄 = 90.f;
  振刀参数::角度::扇子.双开圆 = 90.f;
  振刀参数::角度::扇子.三风摆 = 90.f;
  振刀参数::角度::扇子.鬼反 = 90.f;
  振刀参数::角度::扇子.惊雷 = 90.f;
  振刀参数::角度::扇子.壁击 = 90.f;
  振刀参数::角度::扇子.缠龙奔野 = 90.f;
  振刀参数::角度::扇子.泽风上六 = 90.f;
  振刀参数::角度::横刀.左3 = 90.f;
  振刀参数::角度::横刀.右3 = 90.f;
  振刀参数::角度::横刀.左蓄 = 90.f;
  振刀参数::角度::横刀.右蓄 = 90.f;
  振刀参数::角度::横刀.苍牙 = 90.f;
  振刀参数::角度::横刀.壁击 = 90.f;
  振刀参数::角度::横刀.同源 = 90.f;
  振刀参数::角度::横刀.踏空闪 = 90.f;
  振刀参数::角度::横刀.破千军 = 90.f;
  振刀参数::角度::横刀.极光破云闪 = 180.f;
  振刀参数::角度::横刀.乾坤一掷 = 180.f;
  振刀参数::角度::拳刃.左3 = 90.f;
  振刀参数::角度::拳刃.右3 = 90.f;
  振刀参数::角度::拳刃.左蓄力 = 90.f;
  振刀参数::角度::拳刃.右蓄力 = 90.f;
  振刀参数::角度::拳刃.右蓄力2段 = 90.f;
  振刀参数::角度::拳刃.苍牙 = 90.f;
  振刀参数::角度::拳刃.壁击 = 90.f;
  振刀参数::角度::拳刃.百裂腿 = 90.f;
  振刀参数::角度::拳刃.破空拳 = 90.f;
  振刀参数::角度::拳刃.太极弄云手 = 90.f;
  振刀参数::角度::飞刀.左3 = 90.f;
  振刀参数::角度::飞刀.右3 = 90.f;
  振刀参数::角度::飞刀.左蓄力 = 90.f;
  振刀参数::角度::飞刀.左蓄力2段 = 90.f;
  振刀参数::角度::飞刀.右蓄力 = 90.f;
  振刀参数::角度::飞刀.苍牙 = 90.f;
  振刀参数::角度::飞刀.掌心雷 = 90.f;
  振刀参数::角度::飞刀.太极 = 90.f;
  振刀参数::角度::飞刀.壁击 = 90.f;
  振刀参数::角度::飞刀.断月千刃舞 = 90.f;
  振刀参数::角度::飞刀.万点寒梅 = 90.f;

  振刀参数::延迟::长剑.左3 = 0.25f;
  振刀参数::延迟::长剑.右3 = 0.249f;
  振刀参数::延迟::长剑.左剑气 = 0.05f;
  振刀参数::延迟::长剑.右剑气 = 0.05f;
  振刀参数::延迟::长剑.凤凰羽 = 0.f;
  振刀参数::延迟::长剑.苍牙 = 0.f;
  振刀参数::延迟::长剑.跳斩 = 0.f;
  振刀参数::延迟::长剑.七星夺窍 = 0.f;
  振刀参数::延迟::长剑.壁击 = 0.f;
  振刀参数::延迟::长剑.天外飞仙 = 0.f;
  振刀参数::延迟::太刀.左3 = 0.2f;
  振刀参数::延迟::太刀.右3 = 0.2f;
  振刀参数::延迟::太刀.左蓄 = 0.f;
  振刀参数::延迟::太刀.右蓄 = 0.f;
  振刀参数::延迟::太刀.苍牙 = 0.f;
  振刀参数::延迟::太刀.刹那斩 = 0.f;
  振刀参数::延迟::太刀.右惊雷 = 0.f; // 惊雷十劫和此参数一样
  振刀参数::延迟::太刀.青鬼 = 0.f;
  振刀参数::延迟::太刀.青鬼 = 0.f;
  振刀参数::延迟::太刀.同源 = 0.f;
  振刀参数::延迟::太刀.噬魂斩 = 0.f;
  振刀参数::延迟::太刀.壁击 = 0.f;
  振刀参数::延迟::阔刀.左左 = 0.35f;
  振刀参数::延迟::阔刀.左右 = 0.f;
  振刀参数::延迟::阔刀.右右 = 0.25f;
  振刀参数::延迟::阔刀.右左 = 0.25f;
  振刀参数::延迟::阔刀.左蓄一段 = 0.2f;
  振刀参数::延迟::阔刀.左蓄二段 = 0.08f;
  振刀参数::延迟::阔刀.左蓄三段 = 0.f;
  振刀参数::延迟::阔刀.右蓄 = 0.f;
  振刀参数::延迟::阔刀.右右蓄 = 0.f;
  振刀参数::延迟::阔刀.雷刀 = 0.05f;
  振刀参数::延迟::阔刀.同源 = 0.1f;
  振刀参数::延迟::阔刀.雷刀下劈 = 0.35f;
  振刀参数::延迟::阔刀.壁击 = 0.f;
  振刀参数::延迟::长枪.左3 = 0.15f;
  振刀参数::延迟::长枪.右3 = 0.31f;
  振刀参数::延迟::长枪.左蓄 = 0.2f;
  振刀参数::延迟::长枪.六合枪 = 0.f;
  振刀参数::延迟::长枪.右蓄 = 0.105f;
  振刀参数::延迟::长枪.大圣游 = 0.f;
  振刀参数::延迟::长枪.风卷云残 = 0.f;
  振刀参数::延迟::长枪.双环扫 = 0.f;
  振刀参数::延迟::长枪.龙王破 = 0.075f;
  振刀参数::延迟::长枪.穿心脚 = 0.f;
  振刀参数::延迟::长枪.壁击 = 0.f;
  振刀参数::延迟::长枪.同源 = 0.f;
  振刀参数::延迟::匕首.左3 = 0.105f;
  振刀参数::延迟::匕首.右3 = 0.105f;
  振刀参数::延迟::匕首.左蓄 = 0.f;
  振刀参数::延迟::匕首.右蓄 = 0.f;
  振刀参数::延迟::匕首.鬼反 = 0.05f;
  振刀参数::延迟::匕首.壁击 = 0.05f;
  振刀参数::延迟::匕首.荆轲献匕 = 0.f;
  振刀参数::延迟::匕首.鬼刃暗扎 = 0.f;
  振刀参数::延迟::匕首.鬼哭神嚎 = 0.f;
  振刀参数::延迟::匕首.亢龙有悔 = 0.f;
  振刀参数::延迟::双截棍.左右3 = 0.1f;
  振刀参数::延迟::双截棍.左蓄 = 0.f;
  振刀参数::延迟::双截棍.右蓄 = 0.f;
  振刀参数::延迟::双截棍.飞踢 = 0.f;
  振刀参数::延迟::双截棍.扬鞭劲 = 0.f;
  振刀参数::延迟::双截棍.横栏 = 0.f;
  振刀参数::延迟::双截棍.壁击 = 0.f;
  振刀参数::延迟::双截棍.龙虎乱舞 = 0.85f;
  振刀参数::延迟::双刀.左右3 = 0.1f;
  振刀参数::延迟::双刀.左蓄 = 0.077f;
  振刀参数::延迟::双刀.右蓄 = 0.f;
  振刀参数::延迟::双刀.铁马残红 = 0.f;
  振刀参数::延迟::双刀.乾坤日月斩 = 0.f;
  振刀参数::延迟::双刀.惊雷 = 0.25f;
  振刀参数::延迟::双刀.壁击 = 0.f;
  振刀参数::延迟::双刀.八斩刀 = 0.f;
  振刀参数::延迟::双刀.分水斩 = 0.1f;
  振刀参数::延迟::长棍.左3 = 0.25f;
  振刀参数::延迟::长棍.右3 = 0.35f;
  振刀参数::延迟::长棍.少林棍 = 0.11f;
  振刀参数::延迟::长棍.左蓄 = 0.095f;
  振刀参数::延迟::长棍.右蓄 = 0.f;
  振刀参数::延迟::长棍.腾云式 = 0.f;
  振刀参数::延迟::长棍.乱点天宫 = 0.f;
  振刀参数::延迟::长棍.桶劲 = 0.f;
  振刀参数::延迟::长棍.壁击 = 0.f;
  振刀参数::延迟::长棍.同源 = 0.f;
  振刀参数::延迟::长棍.五情七灭镇 = 0.f;
  振刀参数::延迟::长棍.双环扫 = 0.f;
  振刀参数::延迟::长棍.倒海棍 = 0.f;
  振刀参数::延迟::长棍.镇地撑天 = 0.f;
  振刀参数::延迟::斩马刀.左左 = 0.25f;
  振刀参数::延迟::斩马刀.左右 = 0.f;
  振刀参数::延迟::斩马刀.右右 = 0.10f;
  振刀参数::延迟::斩马刀.右左 = 0.35f;
  振刀参数::延迟::斩马刀.左蓄 = 0.f;
  振刀参数::延迟::斩马刀.右蓄 = 0.075f;
  振刀参数::延迟::斩马刀.柄击 = 0.05f;
  振刀参数::延迟::斩马刀.惊雷 = 0.f;
  振刀参数::延迟::斩马刀.同源 = 0.f;
  振刀参数::延迟::斩马刀.壁击 = 0.f;
  振刀参数::延迟::斩马刀.炽焰斩 = 0.f;
  振刀参数::延迟::斩马刀.奔雷入阵 = 0.055f;
  振刀参数::延迟::斩马刀.过关斩将 = 0.149f;
  振刀参数::延迟::双戟.左右3 = 0.15f;
  振刀参数::延迟::双戟.左蓄 = 0.f;
  振刀参数::延迟::双戟.右蓄 = 0.f;
  振刀参数::延迟::双戟.勾旋斩 = 0.14f;
  振刀参数::延迟::双戟.探海蛟 = 0.f;
  振刀参数::延迟::双戟.钩挂 = 0.13f;
  振刀参数::延迟::双戟.壁击 = 0.f;
  振刀参数::延迟::双戟.战龙在天 = 0.f;
  振刀参数::延迟::双戟.形意钩 = 0.11f;
  振刀参数::延迟::扇子.左3 = 0.1f;
  振刀参数::延迟::扇子.右3 = 0.1f;
  振刀参数::延迟::扇子.左蓄 = 0.f;
  振刀参数::延迟::扇子.右蓄 = 0.f;
  振刀参数::延迟::扇子.双开圆 = 0.f;
  振刀参数::延迟::扇子.三风摆 = 0.f;
  振刀参数::延迟::扇子.鬼反 = 0.f;
  振刀参数::延迟::扇子.惊雷 = 0.f;
  振刀参数::延迟::扇子.壁击 = 0.f;
  振刀参数::延迟::扇子.缠龙奔野 = 0.f;
  振刀参数::延迟::扇子.泽风上六 = 0.f;
  振刀参数::延迟::横刀.左3 = 0.1f;
  振刀参数::延迟::横刀.右3 = 0.1f;
  振刀参数::延迟::横刀.左蓄 = 0.13f;
  振刀参数::延迟::横刀.右蓄 = 0.f;
  振刀参数::延迟::横刀.苍牙 = 0.f;
  振刀参数::延迟::横刀.壁击 = 0.f;
  振刀参数::延迟::横刀.同源 = 0.f;
  振刀参数::延迟::横刀.踏空闪 = 0.f;
  振刀参数::延迟::横刀.破千军 = 0.f;
  振刀参数::延迟::横刀.极光破云闪 = 0.f;
  振刀参数::延迟::横刀.乾坤一掷 = 0.f;
  振刀参数::延迟::拳刃.左3 = 0.1f;
  振刀参数::延迟::拳刃.右3 = 0.1f;
  振刀参数::延迟::拳刃.左蓄力 = 0.05f;
  振刀参数::延迟::拳刃.右蓄力 = 0.05f;
  振刀参数::延迟::拳刃.右蓄力2段 = 0.08f;
  振刀参数::延迟::拳刃.苍牙 = 0.f;
  振刀参数::延迟::拳刃.壁击 = 0.f;
  振刀参数::延迟::拳刃.百裂腿 = 0.11f;
  振刀参数::延迟::拳刃.破空拳 = 0.f;
  振刀参数::延迟::拳刃.太极弄云手 = 0.07f;
  振刀参数::延迟::飞刀.左3 = 0.1f;
  振刀参数::延迟::飞刀.右3 = 0.1f;
  振刀参数::延迟::飞刀.左蓄力 = 0.05f;
  振刀参数::延迟::飞刀.左蓄力2段 = 0.095f;
  振刀参数::延迟::飞刀.右蓄力 = 0.135f;
  振刀参数::延迟::飞刀.苍牙 = 0.1f;
  振刀参数::延迟::飞刀.太极 = 0.07f;
  振刀参数::延迟::飞刀.壁击 = 0.f;
  振刀参数::延迟::飞刀.掌心雷 = 0.125f;
  振刀参数::延迟::飞刀.断月千刃舞 = 0.f;
  振刀参数::延迟::飞刀.万点寒梅 = 0.f;

  // { 0"拼刀", 1"站震", 2"跳震", 3"后闪震", 4"闪避" , 5"后闪修刀震",
  // 6"强行站振", 7"智能振/闪/拼刀"};
  振刀参数::振刀方式::长剑.左3 = 1;
  振刀参数::振刀方式::长剑.右3 = 1;
  振刀参数::振刀方式::长剑.左剑气1 = 1;
  振刀参数::振刀方式::长剑.左剑气2 = 1;
  振刀参数::振刀方式::长剑.右剑气2 = 1;
  振刀参数::振刀方式::长剑.右剑气1 = 1;
  振刀参数::振刀方式::长剑.凤凰羽1 = 1;
  振刀参数::振刀方式::长剑.凤凰羽2 = 1;
  振刀参数::振刀方式::长剑.苍牙 = 1;
  振刀参数::振刀方式::长剑.跳斩 = 1;
  振刀参数::振刀方式::长剑.壁击 = 1;
  振刀参数::振刀方式::长剑.七星夺窍 = 1;
  振刀参数::振刀方式::长剑.天外飞仙 = 1;
  振刀参数::振刀方式::链剑.左3 = 1;
  振刀参数::振刀方式::链剑.右3 = 1;
  振刀参数::振刀方式::链剑.左蓄力1 = 6;
  振刀参数::振刀方式::链剑.左蓄力2 = 1;
  振刀参数::振刀方式::链剑.右蓄力2 = 1;
  振刀参数::振刀方式::链剑.右蓄力1 = 6;
  振刀参数::振刀方式::链剑.地龙滚堂刹 = 1;
  振刀参数::振刀方式::链剑.苍牙 = 1;
  振刀参数::振刀方式::链剑.跳斩 = 1;
  振刀参数::振刀方式::链剑.壁击 = 1;
  振刀参数::振刀方式::链剑.火龙卷云 = 1;
  振刀参数::振刀方式::链剑.断罪碎蜂 = 1;
  振刀参数::振刀方式::链剑.蓄力追击 = 1;
  振刀参数::振刀方式::链剑.链剑长剑同源 = 1;
  振刀参数::振刀方式::太刀.左3 = 1;
  振刀参数::振刀方式::太刀.右3 = 1;
  振刀参数::振刀方式::太刀.左蓄1 = 7;
  振刀参数::振刀方式::太刀.左蓄2 = 7;
  振刀参数::振刀方式::太刀.右蓄2 = 7;
  振刀参数::振刀方式::太刀.右蓄1 = 7;
  振刀参数::振刀方式::太刀.刹那斩1 = 1;
  振刀参数::振刀方式::太刀.苍牙 = 1;
  振刀参数::振刀方式::太刀.惊雷 = 6;
  振刀参数::振刀方式::太刀.壁击 = 7;
  振刀参数::振刀方式::太刀.青鬼 = 1;
  振刀参数::振刀方式::太刀.同源 = 1;
  振刀参数::振刀方式::太刀.噬魂斩1 = 1;
  振刀参数::振刀方式::太刀.噬魂斩2 = 1;
  振刀参数::振刀方式::阔刀.左左 = 6;
  振刀参数::振刀方式::阔刀.左右 = 7;
  振刀参数::振刀方式::阔刀.右右 = 6;
  振刀参数::振刀方式::阔刀.右左 = 6;
  振刀参数::振刀方式::阔刀.左蓄一段 = 6;
  振刀参数::振刀方式::阔刀.左蓄二段 = 6;
  振刀参数::振刀方式::阔刀.左蓄三段 = 6;
  振刀参数::振刀方式::阔刀.右蓄1 = 3;
  振刀参数::振刀方式::阔刀.右蓄2 = 3;
  振刀参数::振刀方式::阔刀.右右蓄 = 6;
  振刀参数::振刀方式::阔刀.雷刀 = 1;
  振刀参数::振刀方式::阔刀.壁击 = 1;
  振刀参数::振刀方式::阔刀.同源 = 1;
  振刀参数::振刀方式::阔刀.雷刀下劈 = 1;
  振刀参数::振刀方式::阔刀.翻江倒海 = 1;
  振刀参数::振刀方式::长枪.左3 = 1;
  振刀参数::振刀方式::长枪.右3 = 1;
  振刀参数::振刀方式::长枪.左蓄 = 3;
  振刀参数::振刀方式::长枪.六合枪 = 1;
  振刀参数::振刀方式::长枪.右蓄 = 6;
  振刀参数::振刀方式::长枪.大圣游 = 6;
  振刀参数::振刀方式::长枪.风卷云残 = 1;
  振刀参数::振刀方式::长枪.壁击 = 1;
  振刀参数::振刀方式::长枪.同源 = 1;
  振刀参数::振刀方式::长枪.双环扫 = 1;
  振刀参数::振刀方式::长枪.龙王破 = 1;
  振刀参数::振刀方式::长枪.穿心脚 = 1;
  振刀参数::振刀方式::匕首.左3 = 1;
  振刀参数::振刀方式::匕首.右3 = 1;
  振刀参数::振刀方式::匕首.左蓄 = 1;
  振刀参数::振刀方式::匕首.右蓄 = 1;
  振刀参数::振刀方式::匕首.鬼反 = 1;
  振刀参数::振刀方式::匕首.荆轲献匕 = 1;
  振刀参数::振刀方式::匕首.壁击 = 1;
  振刀参数::振刀方式::匕首.鬼刃暗扎 = 1;
  振刀参数::振刀方式::匕首.鬼哭神嚎 = 1;
  振刀参数::振刀方式::匕首.亢龙有悔 = 1;
  振刀参数::振刀方式::双截棍.左右3 = 1;
  振刀参数::振刀方式::双截棍.左蓄 = 3;
  振刀参数::振刀方式::双截棍.右蓄 = 7;
  振刀参数::振刀方式::双截棍.飞踢 = 1;
  振刀参数::振刀方式::双截棍.壁击 = 1;
  振刀参数::振刀方式::双截棍.扬鞭劲 = 7;
  振刀参数::振刀方式::双截棍.横栏 = 3;
  振刀参数::振刀方式::双截棍.龙虎乱舞 = 1;
  振刀参数::振刀方式::双截棍.三龙灭阳棍 = 1;
  振刀参数::振刀方式::双刀.左右3 = 1;
  振刀参数::振刀方式::双刀.左蓄 = 6;
  振刀参数::振刀方式::双刀.右蓄1 = 7;
  振刀参数::振刀方式::双刀.右蓄2 = 7;
  振刀参数::振刀方式::双刀.铁马残红 = 1;
  振刀参数::振刀方式::双刀.壁击 = 1;
  振刀参数::振刀方式::双刀.乾坤日月斩 = 7;
  振刀参数::振刀方式::双刀.惊雷 = 6;
  振刀参数::振刀方式::双刀.八斩刀 = 1;
  振刀参数::振刀方式::双刀.分水斩 = 1;
  振刀参数::振刀方式::长棍.右3 = 1;
  振刀参数::振刀方式::长棍.左3 = 1;
  振刀参数::振刀方式::长棍.左蓄 = 1;
  振刀参数::振刀方式::长棍.右蓄 = 1;
  振刀参数::振刀方式::长棍.壁击 = 1;
  振刀参数::振刀方式::长棍.同源 = 1;
  振刀参数::振刀方式::长棍.腾云式 = 1;
  振刀参数::振刀方式::长棍.乱点天宫 = 1;
  振刀参数::振刀方式::长棍.桶劲 = 7;
  振刀参数::振刀方式::长棍.五情七灭镇 = 1;
  振刀参数::振刀方式::长棍.双环扫 = 1;
  振刀参数::振刀方式::长棍.少林棍 = 1;
  振刀参数::振刀方式::长棍.镇地撑天 = 7;
  振刀参数::振刀方式::斩马刀.左左 = 1;
  振刀参数::振刀方式::斩马刀.左右 = 1;
  振刀参数::振刀方式::斩马刀.右右 = 1;
  振刀参数::振刀方式::斩马刀.右左 = 1;
  振刀参数::振刀方式::斩马刀.左蓄1 = 3;
  振刀参数::振刀方式::斩马刀.左蓄2 = 3;
  振刀参数::振刀方式::斩马刀.左蓄3 = 3;
  振刀参数::振刀方式::斩马刀.右蓄1 = 6;
  振刀参数::振刀方式::斩马刀.右蓄2 = 6;
  振刀参数::振刀方式::斩马刀.右蓄3 = 6;
  振刀参数::振刀方式::斩马刀.柄击 = 1;
  振刀参数::振刀方式::斩马刀.壁击 = 1;
  振刀参数::振刀方式::斩马刀.同源 = 1;
  振刀参数::振刀方式::斩马刀.惊雷 = 1;
  振刀参数::振刀方式::斩马刀.炽焰斩 = 7;
  振刀参数::振刀方式::斩马刀.过关斩将 = 1;
  振刀参数::振刀方式::斩马刀.奔雷入阵 = 1;
  振刀参数::振刀方式::双戟.左3 = 1;
  振刀参数::振刀方式::双戟.右3 = 1;
  振刀参数::振刀方式::双戟.左蓄 = 6;
  振刀参数::振刀方式::双戟.右蓄1 = 7;
  振刀参数::振刀方式::双戟.右蓄2 = 7;
  振刀参数::振刀方式::双戟.惊雷 = 1;
  振刀参数::振刀方式::双戟.勾旋斩 = 1;
  振刀参数::振刀方式::双戟.探海蛟 = 1;
  振刀参数::振刀方式::双戟.钩挂 = 1;
  振刀参数::振刀方式::双戟.壁击 = 1;
  振刀参数::振刀方式::双戟.战龙在天 = 1;
  振刀参数::振刀方式::双戟.形意钩 = 7;
  振刀参数::振刀方式::扇子.左3 = 1;
  振刀参数::振刀方式::扇子.右3 = 1;
  振刀参数::振刀方式::扇子.左蓄 = 1;
  振刀参数::振刀方式::扇子.右蓄 = 7;
  振刀参数::振刀方式::扇子.双开圆 = 1;
  振刀参数::振刀方式::扇子.三风摆 = 1;
  振刀参数::振刀方式::扇子.鬼反 = 1;
  振刀参数::振刀方式::扇子.惊雷 = 1;
  振刀参数::振刀方式::扇子.壁击 = 1;
  振刀参数::振刀方式::扇子.缠龙奔野 = 1;
  振刀参数::振刀方式::扇子.泽风上六 = 1;
  振刀参数::振刀方式::横刀.左3 = 1;
  振刀参数::振刀方式::横刀.右3 = 1;
  振刀参数::振刀方式::横刀.左蓄 = 1;
  振刀参数::振刀方式::横刀.右蓄1 = 7;
  振刀参数::振刀方式::横刀.右蓄2 = 7;
  振刀参数::振刀方式::横刀.苍牙 = 1;
  振刀参数::振刀方式::横刀.壁击 = 1;
  振刀参数::振刀方式::横刀.同源 = 1;
  振刀参数::振刀方式::横刀.踏空闪 = 1;
  振刀参数::振刀方式::横刀.破千军 = 1;
  振刀参数::振刀方式::横刀.极光破云闪 = 1;
  振刀参数::振刀方式::横刀.乾坤一掷 = 1;
  振刀参数::振刀方式::拳刃.左3 = 1;
  振刀参数::振刀方式::拳刃.右3 = 1;
  振刀参数::振刀方式::拳刃.左蓄力1 = 6;
  振刀参数::振刀方式::拳刃.左蓄力2 = 6;
  振刀参数::振刀方式::拳刃.右蓄力1 = 6;
  振刀参数::振刀方式::拳刃.右蓄力2 = 6;
  振刀参数::振刀方式::拳刃.右蓄力2段 = 6;
  振刀参数::振刀方式::拳刃.百裂腿 = 6;
  振刀参数::振刀方式::拳刃.破空拳 = 7;
  振刀参数::振刀方式::拳刃.苍牙 = 1;
  振刀参数::振刀方式::拳刃.壁击 = 1;
  振刀参数::振刀方式::拳刃.太极弄云手 = 1;
  振刀参数::振刀方式::飞刀.左3 = 1;
  振刀参数::振刀方式::飞刀.右3 = 1;
  振刀参数::振刀方式::飞刀.左蓄力 = 6;
  振刀参数::振刀方式::飞刀.左蓄力2段 = 1;
  振刀参数::振刀方式::飞刀.右蓄力 = 6;
  振刀参数::振刀方式::飞刀.掌心雷 = 1;
  振刀参数::振刀方式::飞刀.苍牙 = 1;
  振刀参数::振刀方式::飞刀.太极 = 1;
  振刀参数::振刀方式::飞刀.壁击 = 1;
  振刀参数::振刀方式::飞刀.断月千刃舞 = 1;
  振刀参数::振刀方式::飞刀.万点寒梅 = 7;

  振刀参数::闪避::长剑.左3 = 70;
  振刀参数::闪避::长剑.右3 = 70;
  振刀参数::闪避::长剑.左剑气1 = 70;
  振刀参数::闪避::长剑.左剑气2 = 70;
  振刀参数::闪避::长剑.右剑气2 = 70;
  振刀参数::闪避::长剑.右剑气1 = 70;
  振刀参数::闪避::长剑.凤凰羽1 = 70;
  振刀参数::闪避::长剑.凤凰羽2 = 70;
  振刀参数::闪避::长剑.苍牙 = 70;
  振刀参数::闪避::长剑.跳斩 = 70;
  振刀参数::闪避::长剑.壁击 = 70;
  振刀参数::闪避::长剑.七星夺窍 = 70;
  振刀参数::闪避::长剑.天外飞仙 = 70;
  振刀参数::闪避::链剑.左3 = 70;
  振刀参数::闪避::链剑.右3 = 70;
  振刀参数::闪避::链剑.左蓄力1 = 70;
  振刀参数::闪避::链剑.左蓄力2 = 70;
  振刀参数::闪避::链剑.右蓄力2 = 70;
  振刀参数::闪避::链剑.右蓄力1 = 70;
  振刀参数::闪避::链剑.地龙滚堂刹 = 70;
  振刀参数::闪避::链剑.苍牙 = 70;
  振刀参数::闪避::链剑.跳斩 = 70;
  振刀参数::闪避::链剑.壁击 = 70;
  振刀参数::闪避::链剑.火龙卷云 = 70;
  振刀参数::闪避::链剑.断罪碎蜂 = 70;
  振刀参数::闪避::链剑.蓄力追击 = 70;
  振刀参数::闪避::链剑.链剑长剑同源 = 70;
  振刀参数::闪避::太刀.左3 = 70;
  振刀参数::闪避::太刀.右3 = 70;
  振刀参数::闪避::太刀.左蓄1 = 70;
  振刀参数::闪避::太刀.左蓄2 = 70;
  振刀参数::闪避::太刀.右蓄2 = 70;
  振刀参数::闪避::太刀.右蓄1 = 70;
  振刀参数::闪避::太刀.刹那斩1 = 70;
  振刀参数::闪避::太刀.苍牙 = 70;
  振刀参数::闪避::太刀.惊雷 = 70;
  振刀参数::闪避::太刀.壁击 = 70;
  振刀参数::闪避::太刀.青鬼 = 70;
  振刀参数::闪避::太刀.同源 = 70;
  振刀参数::闪避::太刀.噬魂斩1 = 70;
  振刀参数::闪避::太刀.噬魂斩2 = 70;
  振刀参数::闪避::阔刀.左左 = 70;
  振刀参数::闪避::阔刀.左右 = 70;
  振刀参数::闪避::阔刀.右右 = 70;
  振刀参数::闪避::阔刀.右左 = 70;
  振刀参数::闪避::阔刀.左蓄一段 = 70;
  振刀参数::闪避::阔刀.左蓄二段 = 70;
  振刀参数::闪避::阔刀.左蓄三段 = 70;
  振刀参数::闪避::阔刀.右蓄1 = 70;
  振刀参数::闪避::阔刀.右蓄2 = 70;
  振刀参数::闪避::阔刀.右右蓄 = 70;
  振刀参数::闪避::阔刀.雷刀 = 70;
  振刀参数::闪避::阔刀.壁击 = 70;
  振刀参数::闪避::阔刀.同源 = 70;
  振刀参数::闪避::阔刀.雷刀下劈 = 70;
  振刀参数::闪避::阔刀.翻江倒海 = 70;
  振刀参数::闪避::长枪.左3 = 70;
  振刀参数::闪避::长枪.右3 = 70;
  振刀参数::闪避::长枪.左蓄 = 70;
  振刀参数::闪避::长枪.六合枪 = 70;
  振刀参数::闪避::长枪.右蓄 = 70;
  振刀参数::闪避::长枪.大圣游 = 70;
  振刀参数::闪避::长枪.风卷云残 = 70;
  振刀参数::闪避::长枪.壁击 = 70;
  振刀参数::闪避::长枪.同源 = 70;
  振刀参数::闪避::长枪.双环扫 = 70;
  振刀参数::闪避::长枪.龙王破 = 70;
  振刀参数::闪避::长枪.穿心脚 = 70;
  振刀参数::闪避::匕首.左3 = 70;
  振刀参数::闪避::匕首.右3 = 70;
  振刀参数::闪避::匕首.左蓄 = 70;
  振刀参数::闪避::匕首.右蓄 = 70;
  振刀参数::闪避::匕首.鬼反 = 70;
  振刀参数::闪避::匕首.荆轲献匕 = 70;
  振刀参数::闪避::匕首.壁击 = 70;
  振刀参数::闪避::匕首.鬼刃暗扎 = 70;
  振刀参数::闪避::匕首.鬼哭神嚎 = 70;
  振刀参数::闪避::匕首.亢龙有悔 = 70;
  振刀参数::闪避::双截棍.左右3 = 70;
  振刀参数::闪避::双截棍.左蓄 = 70;
  振刀参数::闪避::双截棍.右蓄 = 70;
  振刀参数::闪避::双截棍.飞踢 = 70;
  振刀参数::闪避::双截棍.壁击 = 70;
  振刀参数::闪避::双截棍.扬鞭劲 = 70;
  振刀参数::闪避::双截棍.横栏 = 70;
  振刀参数::闪避::双截棍.龙虎乱舞 = 70;
  振刀参数::闪避::双截棍.三龙灭阳棍 = 70;
  振刀参数::闪避::双刀.左右3 = 70;
  振刀参数::闪避::双刀.左蓄 = 70;
  振刀参数::闪避::双刀.右蓄1 = 70;
  振刀参数::闪避::双刀.右蓄2 = 70;
  振刀参数::闪避::双刀.铁马残红 = 70;
  振刀参数::闪避::双刀.壁击 = 70;
  振刀参数::闪避::双刀.乾坤日月斩 = 70;
  振刀参数::闪避::双刀.惊雷 = 70;
  振刀参数::闪避::双刀.八斩刀 = 70;
  振刀参数::闪避::双刀.分水斩 = 70;
  振刀参数::闪避::长棍.右3 = 70;
  振刀参数::闪避::长棍.左3 = 70;
  振刀参数::闪避::长棍.左蓄 = 70;
  振刀参数::闪避::长棍.右蓄 = 70;
  振刀参数::闪避::长棍.壁击 = 70;
  振刀参数::闪避::长棍.同源 = 70;
  振刀参数::闪避::长棍.腾云式 = 70;
  振刀参数::闪避::长棍.乱点天宫 = 70;
  振刀参数::闪避::长棍.桶劲 = 70;
  振刀参数::闪避::长棍.五情七灭镇 = 70;
  振刀参数::闪避::长棍.双环扫 = 70;
  振刀参数::闪避::长棍.少林棍 = 70;
  振刀参数::闪避::长棍.镇地撑天 = 70;
  振刀参数::闪避::斩马刀.左左 = 70;
  振刀参数::闪避::斩马刀.左右 = 70;
  振刀参数::闪避::斩马刀.右右 = 70;
  振刀参数::闪避::斩马刀.右左 = 70;
  振刀参数::闪避::斩马刀.左蓄1 = 70;
  振刀参数::闪避::斩马刀.左蓄2 = 70;
  振刀参数::闪避::斩马刀.左蓄3 = 70;
  振刀参数::闪避::斩马刀.右蓄1 = 70;
  振刀参数::闪避::斩马刀.右蓄2 = 70;
  振刀参数::闪避::斩马刀.右蓄3 = 70;
  振刀参数::闪避::斩马刀.柄击 = 70;
  振刀参数::闪避::斩马刀.壁击 = 70;
  振刀参数::闪避::斩马刀.同源 = 70;
  振刀参数::闪避::斩马刀.惊雷 = 70;
  振刀参数::闪避::斩马刀.炽焰斩 = 70;
  振刀参数::闪避::斩马刀.过关斩将 = 70;
  振刀参数::闪避::斩马刀.奔雷入阵 = 70;
  振刀参数::闪避::双戟.左3 = 70;
  振刀参数::闪避::双戟.右3 = 70;
  振刀参数::闪避::双戟.左蓄 = 70;
  振刀参数::闪避::双戟.右蓄1 = 70;
  振刀参数::闪避::双戟.右蓄2 = 70;
  振刀参数::闪避::双戟.惊雷 = 70;
  振刀参数::闪避::双戟.勾旋斩 = 70;
  振刀参数::闪避::双戟.探海蛟 = 70;
  振刀参数::闪避::双戟.钩挂 = 70;
  振刀参数::闪避::双戟.壁击 = 70;
  振刀参数::闪避::双戟.战龙在天 = 70;
  振刀参数::闪避::双戟.形意钩 = 70;
  振刀参数::闪避::扇子.左3 = 70;
  振刀参数::闪避::扇子.右3 = 70;
  振刀参数::闪避::扇子.左蓄 = 70;
  振刀参数::闪避::扇子.右蓄 = 70;
  振刀参数::闪避::扇子.双开圆 = 70;
  振刀参数::闪避::扇子.三风摆 = 70;
  振刀参数::闪避::扇子.鬼反 = 70;
  振刀参数::闪避::扇子.惊雷 = 70;
  振刀参数::闪避::扇子.壁击 = 70;
  振刀参数::闪避::扇子.缠龙奔野 = 70;
  振刀参数::闪避::扇子.泽风上六 = 70;
  振刀参数::闪避::横刀.左3 = 70;
  振刀参数::闪避::横刀.右3 = 70;
  振刀参数::闪避::横刀.左蓄 = 70;
  振刀参数::闪避::横刀.右蓄1 = 70;
  振刀参数::闪避::横刀.右蓄2 = 70;
  振刀参数::闪避::横刀.苍牙 = 70;
  振刀参数::闪避::横刀.壁击 = 70;
  振刀参数::闪避::横刀.同源 = 70;
  振刀参数::闪避::横刀.踏空闪 = 70;
  振刀参数::闪避::横刀.破千军 = 70;
  振刀参数::闪避::横刀.极光破云闪 = 70;
  振刀参数::闪避::横刀.乾坤一掷 = 70;
  振刀参数::闪避::拳刃.左3 = 70;
  振刀参数::闪避::拳刃.右3 = 70;
  振刀参数::闪避::拳刃.左蓄力1 = 70;
  振刀参数::闪避::拳刃.左蓄力2 = 70;
  振刀参数::闪避::拳刃.右蓄力1 = 70;
  振刀参数::闪避::拳刃.右蓄力2 = 70;
  振刀参数::闪避::拳刃.右蓄力2段 = 70;
  振刀参数::闪避::拳刃.百裂腿 = 70;
  振刀参数::闪避::拳刃.破空拳 = 70;
  振刀参数::闪避::拳刃.苍牙 = 70;
  振刀参数::闪避::拳刃.壁击 = 70;
  振刀参数::闪避::拳刃.太极弄云手 = 70;
  振刀参数::闪避::飞刀.左3 = 70;
  振刀参数::闪避::飞刀.右3 = 70;
  振刀参数::闪避::飞刀.左蓄力 = 70;
  振刀参数::闪避::飞刀.左蓄力2段 = 70;
  振刀参数::闪避::飞刀.右蓄力 = 70;
  振刀参数::闪避::飞刀.掌心雷 = 70;
  振刀参数::闪避::飞刀.苍牙 = 70;
  振刀参数::闪避::飞刀.太极 = 70;
  振刀参数::闪避::飞刀.壁击 = 70;
  振刀参数::闪避::飞刀.断月千刃舞 = 70;
  振刀参数::闪避::飞刀.万点寒梅 = 70;

  保存模拟振配置();
  读取模拟振配置();
}
void 判断模拟振配置(const std::string &line) {
  std::istringstream iss(line);
  std::string token;
  std::string paramName;
  float paramValue;

  if (std::getline(iss, token, '=')) {
    paramName = token;
    if (std::getline(iss, token)) {
      paramValue = std::stof(token);

      if (strstr(paramName.c_str(), "振刀全局延迟修正") != NULL) {
        Function::Shock::振刀全局延迟修正 = std::clamp(paramValue, 0.0f, 2.0f);
      }
#define 读取千机伞浮点参数(key, value)                                           \
  if (paramName == key) {                                                        \
    value = paramValue;                                                          \
  }
#define 读取千机伞整数参数(key, value)                                           \
  if (paramName == key) {                                                        \
    value = static_cast<int>(paramValue);                                        \
  }
      读取千机伞浮点参数("千机伞左3距离", 振刀参数::距离::千机伞.左3);
      读取千机伞浮点参数("千机伞伞左蓄距离", 振刀参数::距离::千机伞.伞左蓄);
      读取千机伞浮点参数("千机伞盾左蓄距离", 振刀参数::距离::千机伞.盾左蓄);
      读取千机伞浮点参数("千机伞斧左蓄距离", 振刀参数::距离::千机伞.斧左蓄);
      读取千机伞浮点参数("千机伞伞右蓄距离", 振刀参数::距离::千机伞.伞右蓄);
      读取千机伞浮点参数("千机伞盾右蓄距离", 振刀参数::距离::千机伞.盾右蓄);
      读取千机伞浮点参数("千机伞斧右蓄距离", 振刀参数::距离::千机伞.斧右蓄);
      读取千机伞浮点参数("千机伞听雨封喉距离", 振刀参数::距离::千机伞.听雨封喉);
      读取千机伞浮点参数("千机伞苍牙距离", 振刀参数::距离::千机伞.苍牙);
      读取千机伞整数参数("千机伞左3振刀方式", 振刀参数::振刀方式::千机伞.左3);
      读取千机伞整数参数("千机伞伞左蓄振刀方式", 振刀参数::振刀方式::千机伞.伞左蓄);
      读取千机伞整数参数("千机伞盾左蓄振刀方式", 振刀参数::振刀方式::千机伞.盾左蓄);
      读取千机伞整数参数("千机伞斧左蓄振刀方式", 振刀参数::振刀方式::千机伞.斧左蓄);
      读取千机伞整数参数("千机伞伞右蓄振刀方式", 振刀参数::振刀方式::千机伞.伞右蓄);
      读取千机伞整数参数("千机伞盾右蓄振刀方式", 振刀参数::振刀方式::千机伞.盾右蓄);
      读取千机伞整数参数("千机伞斧右蓄振刀方式", 振刀参数::振刀方式::千机伞.斧右蓄);
      读取千机伞整数参数("千机伞听雨封喉振刀方式", 振刀参数::振刀方式::千机伞.听雨封喉);
      读取千机伞整数参数("千机伞苍牙振刀方式", 振刀参数::振刀方式::千机伞.苍牙);
      读取千机伞整数参数("千机伞左3闪避", 振刀参数::闪避::千机伞.左3);
      读取千机伞整数参数("千机伞伞左蓄闪避", 振刀参数::闪避::千机伞.伞左蓄);
      读取千机伞整数参数("千机伞盾左蓄闪避", 振刀参数::闪避::千机伞.盾左蓄);
      读取千机伞整数参数("千机伞斧左蓄闪避", 振刀参数::闪避::千机伞.斧左蓄);
      读取千机伞整数参数("千机伞伞右蓄闪避", 振刀参数::闪避::千机伞.伞右蓄);
      读取千机伞整数参数("千机伞盾右蓄闪避", 振刀参数::闪避::千机伞.盾右蓄);
      读取千机伞整数参数("千机伞斧右蓄闪避", 振刀参数::闪避::千机伞.斧右蓄);
      读取千机伞整数参数("千机伞听雨封喉闪避", 振刀参数::闪避::千机伞.听雨封喉);
      读取千机伞整数参数("千机伞苍牙闪避", 振刀参数::闪避::千机伞.苍牙);
      读取千机伞浮点参数("千机伞左3角度", 振刀参数::角度::千机伞.左3);
      读取千机伞浮点参数("千机伞伞左蓄角度", 振刀参数::角度::千机伞.伞左蓄);
      读取千机伞浮点参数("千机伞盾左蓄角度", 振刀参数::角度::千机伞.盾左蓄);
      读取千机伞浮点参数("千机伞斧左蓄角度", 振刀参数::角度::千机伞.斧左蓄);
      读取千机伞浮点参数("千机伞伞右蓄角度", 振刀参数::角度::千机伞.伞右蓄);
      读取千机伞浮点参数("千机伞盾右蓄角度", 振刀参数::角度::千机伞.盾右蓄);
      读取千机伞浮点参数("千机伞斧右蓄角度", 振刀参数::角度::千机伞.斧右蓄);
      读取千机伞浮点参数("千机伞听雨封喉角度", 振刀参数::角度::千机伞.听雨封喉);
      读取千机伞浮点参数("千机伞苍牙角度", 振刀参数::角度::千机伞.苍牙);
      读取千机伞浮点参数("千机伞左3延迟", 振刀参数::延迟::千机伞.左3);
      读取千机伞浮点参数("千机伞伞左蓄延迟", 振刀参数::延迟::千机伞.伞左蓄);
      读取千机伞浮点参数("千机伞盾左蓄延迟", 振刀参数::延迟::千机伞.盾左蓄);
      读取千机伞浮点参数("千机伞斧左蓄延迟", 振刀参数::延迟::千机伞.斧左蓄);
      读取千机伞浮点参数("千机伞伞右蓄延迟", 振刀参数::延迟::千机伞.伞右蓄);
      读取千机伞浮点参数("千机伞盾右蓄延迟", 振刀参数::延迟::千机伞.盾右蓄);
      读取千机伞浮点参数("千机伞斧右蓄延迟", 振刀参数::延迟::千机伞.斧右蓄);
      读取千机伞浮点参数("千机伞听雨封喉延迟", 振刀参数::延迟::千机伞.听雨封喉);
      读取千机伞浮点参数("千机伞苍牙延迟", 振刀参数::延迟::千机伞.苍牙);
#undef 读取千机伞整数参数
#undef 读取千机伞浮点参数
      // 重置模拟振配置();
      // 长剑
      if (strstr(paramName.c_str(), "长剑左3距离") != NULL) {
        振刀参数::距离::长剑.左3 = paramValue;
        if (振刀参数::距离::长剑.左3 == 0) {
          MyLogTrue("检测到模拟振配置异常，已恢复默认配置");
          重置模拟振配置();
          return;
        }
      }
      if (strstr(paramName.c_str(), "长剑右3距离") != NULL) {
        振刀参数::距离::长剑.右3 = paramValue;
        if (振刀参数::距离::长剑.右3 == 0) {
          MyLogTrue("检测到模拟振配置异常，已恢复默认配置");
          重置模拟振配置();
          return;
        }
      }

      if (strstr(paramName.c_str(), "长剑左剑气距离") != NULL) {
        振刀参数::距离::长剑.左剑气 = paramValue;
      }
      if (strstr(paramName.c_str(), "长剑右剑气距离") != NULL) {
        振刀参数::距离::长剑.右剑气 = paramValue;
      }
      if (strstr(paramName.c_str(), "长剑凤凰羽距离") != NULL) {
        振刀参数::距离::长剑.凤凰羽 = paramValue;
      }
      if (strstr(paramName.c_str(), "长剑蓝月距离") != NULL) {
        振刀参数::距离::长剑.苍牙 = paramValue;
      }
      if (strstr(paramName.c_str(), "长剑跳斩距离") != NULL) {
        振刀参数::距离::长剑.跳斩 = paramValue;
      }
      if (strstr(paramName.c_str(), "长剑壁击距离") != NULL) {
        振刀参数::距离::长剑.壁击 = paramValue;
      }
      if (strstr(paramName.c_str(), "长剑七星夺窍距离") != NULL) {
        振刀参数::距离::长剑.七星夺窍 = paramValue;
      }
      // 延迟
      if (strstr(paramName.c_str(), "长剑左3延迟") != NULL) {
        振刀参数::延迟::长剑.左3 = paramValue;
      }
      if (strstr(paramName.c_str(), "长剑右3延迟") != NULL) {
        振刀参数::延迟::长剑.右3 = paramValue;
      }
      if (strstr(paramName.c_str(), "长剑左剑气延迟") != NULL) {
        振刀参数::延迟::长剑.左剑气 = paramValue;
      }
      if (strstr(paramName.c_str(), "长剑右剑气延迟") != NULL) {
        振刀参数::延迟::长剑.右剑气 = paramValue;
      }
      if (strstr(paramName.c_str(), "长剑凤凰羽延迟") != NULL) {
        振刀参数::延迟::长剑.凤凰羽 = paramValue;
      }
      if (strstr(paramName.c_str(), "长剑蓝月延迟") != NULL) {
        振刀参数::延迟::长剑.苍牙 = paramValue;
      }
      if (strstr(paramName.c_str(), "长剑跳斩延迟") != NULL) {
        振刀参数::延迟::长剑.跳斩 = paramValue;
      }
      if (strstr(paramName.c_str(), "长剑七星夺窍延迟") != NULL) {
        振刀参数::延迟::长剑.七星夺窍 = paramValue;
      }
      if (strstr(paramName.c_str(), "长剑壁击延迟") != NULL) {
        振刀参数::延迟::长剑.壁击 = paramValue;
      }
      // 角度
      if (strstr(paramName.c_str(), "长剑左3角度") != NULL) {
        振刀参数::角度::长剑.左3 = paramValue;
      }
      if (strstr(paramName.c_str(), "长剑右3角度") != NULL) {
        振刀参数::角度::长剑.右3 = paramValue;
      }
      if (strstr(paramName.c_str(), "长剑左剑气角度") != NULL) {
        振刀参数::角度::长剑.左剑气 = paramValue;
      }
      if (strstr(paramName.c_str(), "长剑右剑气角度") != NULL) {
        振刀参数::角度::长剑.右剑气 = paramValue;
      }
      if (strstr(paramName.c_str(), "长剑凤凰羽角度") != NULL) {
        振刀参数::角度::长剑.凤凰羽 = paramValue;
      }
      if (strstr(paramName.c_str(), "长剑苍牙角度") != NULL) {
        振刀参数::角度::长剑.苍牙 = paramValue;
      }
      if (strstr(paramName.c_str(), "长剑跳斩角度") != NULL) {
        振刀参数::角度::长剑.跳斩 = paramValue;
      }
      if (strstr(paramName.c_str(), "长剑壁击角度") != NULL) {
        振刀参数::角度::长剑.壁击 = paramValue;
      }
      if (strstr(paramName.c_str(), "长剑七星夺窍角度") != NULL) {
        振刀参数::角度::长剑.七星夺窍 = paramValue;
      }

      // 链剑
      if (strstr(paramName.c_str(), "链剑左3距离") != NULL) {
        振刀参数::距离::链剑.左3 = paramValue;
      }
      if (strstr(paramName.c_str(), "链剑右3距离") != NULL) {
        振刀参数::距离::链剑.右3 = paramValue;
      }
      if (strstr(paramName.c_str(), "链剑左蓄力距离") != NULL) {
        振刀参数::距离::链剑.左蓄力 = paramValue;
      }
      if (strstr(paramName.c_str(), "链剑右蓄力距离") != NULL) {
        振刀参数::距离::链剑.右蓄力 = paramValue;
      }
      if (strstr(paramName.c_str(), "链剑地龙滚堂刹距离") != NULL) {
        振刀参数::距离::链剑.地龙滚堂刹 = paramValue;
      }
      if (strstr(paramName.c_str(), "链剑苍牙距离") != NULL) {
        振刀参数::距离::链剑.苍牙 = paramValue;
      }
      if (strstr(paramName.c_str(), "链剑火龙卷云距离") != NULL) {
        振刀参数::距离::链剑.火龙卷云 = paramValue;
      }
      if (strstr(paramName.c_str(), "链剑蓄力追击距离") != NULL) {
        振刀参数::距离::链剑.蓄力追击 = paramValue;
      }
      if (strstr(paramName.c_str(), "链剑长剑同源距离") != NULL) {
        振刀参数::距离::链剑.链剑长剑同源 = paramValue;
      }
      if (strstr(paramName.c_str(), "链剑断罪碎蜂距离") != NULL) {
        振刀参数::距离::链剑.断罪碎蜂 = paramValue;
      }
      // 延迟
      if (strstr(paramName.c_str(), "链剑左3延迟") != NULL) {
        振刀参数::延迟::链剑.左3 = paramValue;
      }
      if (strstr(paramName.c_str(), "链剑右3延迟") != NULL) {
        振刀参数::延迟::链剑.右3 = paramValue;
      }
      if (strstr(paramName.c_str(), "链剑左蓄力延迟") != NULL) {
        振刀参数::延迟::链剑.左蓄力 = paramValue;
      }
      if (strstr(paramName.c_str(), "链剑右蓄力延迟") != NULL) {
        振刀参数::延迟::链剑.右蓄力 = paramValue;
      }
      if (strstr(paramName.c_str(), "链剑地龙滚堂刹延迟") != NULL) {
        振刀参数::延迟::链剑.地龙滚堂刹 = paramValue;
      }
      if (strstr(paramName.c_str(), "链剑苍牙延迟") != NULL) {
        振刀参数::延迟::链剑.苍牙 = paramValue;
      }
      if (strstr(paramName.c_str(), "链剑火龙卷云延迟") != NULL) {
        振刀参数::延迟::链剑.火龙卷云 = paramValue;
      }
      if (strstr(paramName.c_str(), "链剑断罪碎蜂延迟") != NULL) {
        振刀参数::延迟::链剑.断罪碎蜂 = paramValue;
      }
      if (strstr(paramName.c_str(), "链剑蓄力追击延迟") != NULL) {
        振刀参数::延迟::链剑.蓄力追击 = paramValue;
      }
      if (strstr(paramName.c_str(), "链剑长剑同源延迟") != NULL) {
          振刀参数::延迟::链剑.链剑长剑同源 = paramValue;
      }
      // 角度
      if (strstr(paramName.c_str(), "链剑左3角度") != NULL) {
        振刀参数::角度::链剑.左3 = paramValue;
      }
      if (strstr(paramName.c_str(), "链剑右3角度") != NULL) {
        振刀参数::角度::链剑.右3 = paramValue;
      }
      if (strstr(paramName.c_str(), "链剑左蓄力角度") != NULL) {
        振刀参数::角度::链剑.左蓄力 = paramValue;
      }
      if (strstr(paramName.c_str(), "链剑右蓄力角度") != NULL) {
        振刀参数::角度::链剑.右蓄力 = paramValue;
      }
      if (strstr(paramName.c_str(), "链剑地龙滚堂刹角度") != NULL) {
        振刀参数::角度::链剑.地龙滚堂刹 = paramValue;
      }
      if (strstr(paramName.c_str(), "链剑苍牙角度") != NULL) {
        振刀参数::角度::链剑.苍牙 = paramValue;
      }
      if (strstr(paramName.c_str(), "链剑火龙卷云角度") != NULL) {
        振刀参数::角度::链剑.火龙卷云 = paramValue;
      }
      if (strstr(paramName.c_str(), "链剑蓄力追击角度") != NULL) {
        振刀参数::角度::链剑.蓄力追击 = paramValue;
      }
      if (strstr(paramName.c_str(), "链剑断罪碎蜂角度") != NULL) {
        振刀参数::角度::链剑.断罪碎蜂 = paramValue;
      }
      if (strstr(paramName.c_str(), "链剑长剑同源角度") != NULL) {
          振刀参数::角度::链剑.链剑长剑同源 = paramValue;
      }
      // 振刀
      if (strstr(paramName.c_str(), "长剑左3振刀方式") != NULL) {
        振刀参数::振刀方式::长剑.左3 = paramValue;
      }
      if (strstr(paramName.c_str(), "长剑右3振刀方式") != NULL) {
        振刀参数::振刀方式::长剑.右3 = paramValue;
      }
      if (strstr(paramName.c_str(), "长剑左剑气1振刀方式") != NULL) {
        振刀参数::振刀方式::长剑.左剑气1 = paramValue;
      }
      if (strstr(paramName.c_str(), "长剑左剑气2振刀方式") != NULL) {
        振刀参数::振刀方式::长剑.左剑气2 = paramValue;
      }
      if (strstr(paramName.c_str(), "长剑右剑气1振刀方式") != NULL) {
        振刀参数::振刀方式::长剑.右剑气1 = paramValue;
      }
      if (strstr(paramName.c_str(), "长剑右剑气2振刀方式") != NULL) {
        振刀参数::振刀方式::长剑.右剑气2 = paramValue;
      }
      if (strstr(paramName.c_str(), "长剑凤凰羽1振刀方式") != NULL) {
        振刀参数::振刀方式::长剑.凤凰羽1 = paramValue;
      }
      if (strstr(paramName.c_str(), "长剑凤凰羽2振刀方式") != NULL) {
        振刀参数::振刀方式::长剑.凤凰羽2 = paramValue;
      }
      if (strstr(paramName.c_str(), "长剑蓝月振刀方式") != NULL) {
        振刀参数::振刀方式::长剑.苍牙 = paramValue;
      }
      if (strstr(paramName.c_str(), "长剑跳斩振刀方式") != NULL) {
        振刀参数::振刀方式::长剑.跳斩 = paramValue;
      }
      if (strstr(paramName.c_str(), "长剑壁击振刀方式") != NULL) {
        振刀参数::振刀方式::长剑.壁击 = paramValue;
      }
      if (strstr(paramName.c_str(), "长剑七星夺窍振刀方式") != NULL) {
        振刀参数::振刀方式::长剑.七星夺窍 = paramValue;
      }
      if (strstr(paramName.c_str(), "链剑左3振刀方式") != NULL) {
        振刀参数::振刀方式::链剑.左3 = paramValue;
      }
      if (strstr(paramName.c_str(), "链剑右3振刀方式") != NULL) {
        振刀参数::振刀方式::链剑.右3 = paramValue;
      }
      if (strstr(paramName.c_str(), "链剑左蓄力1振刀方式") != NULL) {
        振刀参数::振刀方式::链剑.左蓄力1 = paramValue;
      }
      if (strstr(paramName.c_str(), "链剑左蓄力2振刀方式") != NULL) {
        振刀参数::振刀方式::链剑.左蓄力2 = paramValue;
      }
      if (strstr(paramName.c_str(), "链剑右蓄力1振刀方式") != NULL) {
        振刀参数::振刀方式::链剑.右蓄力1 = paramValue;
      }
      if (strstr(paramName.c_str(), "链剑右蓄力2振刀方式") != NULL) {
        振刀参数::振刀方式::链剑.右蓄力2 = paramValue;
      }
      if (strstr(paramName.c_str(), "链剑地龙滚堂刹振刀方式") != NULL) {
        振刀参数::振刀方式::链剑.地龙滚堂刹 = paramValue;
      }
      if (strstr(paramName.c_str(), "链剑苍牙振刀方式") != NULL) {
        振刀参数::振刀方式::链剑.苍牙 = paramValue;
      }
      if (strstr(paramName.c_str(), "链剑火龙卷云振刀方式") != NULL) {
        振刀参数::振刀方式::链剑.火龙卷云 = paramValue;
      }
      if (strstr(paramName.c_str(), "链剑断罪碎蜂振刀方式") != NULL) {
        振刀参数::振刀方式::链剑.断罪碎蜂 = paramValue;
      }
      if (strstr(paramName.c_str(), "链剑蓄力追击振刀方式") != NULL) {
        振刀参数::振刀方式::链剑.蓄力追击 = paramValue;
      }
      if (strstr(paramName.c_str(), "链剑长剑同源振刀方式") != NULL) {
          振刀参数::振刀方式::链剑.链剑长剑同源 = paramValue;
      }
      if (strstr(paramName.c_str(), "太刀左3振刀方式") != NULL) {
        振刀参数::振刀方式::太刀.左3 = paramValue;
      }
      if (strstr(paramName.c_str(), "太刀右3振刀方式") != NULL) {
        振刀参数::振刀方式::太刀.右3 = paramValue;
      }
      if (strstr(paramName.c_str(), "太刀左蓄1振刀方式") != NULL) {
        振刀参数::振刀方式::太刀.左蓄1 = paramValue;
      }
      if (strstr(paramName.c_str(), "太刀左蓄2振刀方式") != NULL) {
        振刀参数::振刀方式::太刀.左蓄2 = paramValue;
      }
      if (strstr(paramName.c_str(), "太刀右蓄1振刀方式") != NULL) {
        振刀参数::振刀方式::太刀.右蓄1 = paramValue;
      }
      if (strstr(paramName.c_str(), "太刀右蓄2振刀方式") != NULL) {
        振刀参数::振刀方式::太刀.右蓄2 = paramValue;
      }
      if (strstr(paramName.c_str(), "太刀苍牙振刀方式") != NULL) {
        振刀参数::振刀方式::太刀.苍牙 = paramValue;
      }
      if (strstr(paramName.c_str(), "太刀壁击振刀方式") != NULL) {
        振刀参数::振刀方式::太刀.壁击 = paramValue;
      }
      if (strstr(paramName.c_str(), "太刀刹那斩1振刀方式") != NULL) {
        振刀参数::振刀方式::太刀.刹那斩1 = paramValue;
      }
      if (strstr(paramName.c_str(), "太刀刹那斩2振刀方式") != NULL) {
        振刀参数::振刀方式::太刀.刹那斩2 = paramValue;
      }
      if (strstr(paramName.c_str(), "太刀惊雷振刀方式") != NULL) {
        振刀参数::振刀方式::太刀.惊雷 = paramValue;
      }
      if (strstr(paramName.c_str(), "太刀惊雷十劫振刀方式") != NULL) {
        振刀参数::振刀方式::太刀.惊雷十劫 = paramValue;
      }
      if (strstr(paramName.c_str(), "太刀青鬼振刀方式") != NULL) {
        振刀参数::振刀方式::太刀.青鬼 = paramValue;
      }
      if (strstr(paramName.c_str(), "太刀同源振刀方式") != NULL) {
        振刀参数::振刀方式::太刀.同源 = paramValue;
      }
      if (strstr(paramName.c_str(), "太刀噬魂斩1振刀方式") != NULL) {
        振刀参数::振刀方式::太刀.噬魂斩1 = paramValue;
      }
      if (strstr(paramName.c_str(), "太刀噬魂斩2振刀方式") != NULL) {
        振刀参数::振刀方式::太刀.噬魂斩2 = paramValue;
      }
      if (strstr(paramName.c_str(), "阔刀左左振刀方式") != NULL) {
        振刀参数::振刀方式::阔刀.左左 = paramValue;
      }
      if (strstr(paramName.c_str(), "阔刀左右振刀方式") != NULL) {
        振刀参数::振刀方式::阔刀.左右 = paramValue;
      }
      if (strstr(paramName.c_str(), "阔刀右右振刀方式") != NULL) {
        振刀参数::振刀方式::阔刀.右右 = paramValue;
      }
      if (strstr(paramName.c_str(), "阔刀右左振刀方式") != NULL) {
        振刀参数::振刀方式::阔刀.右左 = paramValue;
      }
      if (strstr(paramName.c_str(), "阔刀左蓄一段振刀方式") != NULL) {
        振刀参数::振刀方式::阔刀.左蓄一段 = paramValue;
      }
      if (strstr(paramName.c_str(), "阔刀左蓄二段振刀方式") != NULL) {
        振刀参数::振刀方式::阔刀.左蓄二段 = paramValue;
      }
      if (strstr(paramName.c_str(), "阔刀左蓄三段振刀方式") != NULL) {
        振刀参数::振刀方式::阔刀.左蓄三段 = paramValue;
      }
      if (strstr(paramName.c_str(), "阔刀右蓄1振刀方式") != NULL) {
        振刀参数::振刀方式::阔刀.右蓄1 = paramValue;
      }
      if (strstr(paramName.c_str(), "阔刀右蓄2振刀方式") != NULL) {
        振刀参数::振刀方式::阔刀.右蓄2 = paramValue;
      }
      if (strstr(paramName.c_str(), "阔刀右右蓄振刀方式") != NULL) {
        振刀参数::振刀方式::阔刀.右右蓄 = paramValue;
      }
      if (strstr(paramName.c_str(), "阔刀雷刀振刀方式") != NULL) {
        振刀参数::振刀方式::阔刀.雷刀 = paramValue;
      }
      if (strstr(paramName.c_str(), "阔刀壁击振刀方式") != NULL) {
        振刀参数::振刀方式::阔刀.壁击 = paramValue;
      }
      if (strstr(paramName.c_str(), "阔刀同源振刀方式") != NULL) {
        振刀参数::振刀方式::阔刀.同源 = paramValue;
      }
      if (strstr(paramName.c_str(), "阔刀雷刀下劈振刀方式") != NULL) {
        振刀参数::振刀方式::阔刀.雷刀下劈 = paramValue;
      }
      if (strstr(paramName.c_str(), "阔刀翻江倒海振刀方式") != NULL) {
        振刀参数::振刀方式::阔刀.翻江倒海 = paramValue;
      }
      if (strstr(paramName.c_str(), "长枪左3振刀方式") != NULL) {
        振刀参数::振刀方式::长枪.左3 = paramValue;
      }
      if (strstr(paramName.c_str(), "长枪右3振刀方式") != NULL) {
        振刀参数::振刀方式::长枪.右3 = paramValue;
      }
      if (strstr(paramName.c_str(), "长枪左蓄振刀方式") != NULL) {
        振刀参数::振刀方式::长枪.左蓄 = paramValue;
      }
      if (strstr(paramName.c_str(), "长枪六合枪振刀方式") != NULL) {
        振刀参数::振刀方式::长枪.六合枪 = paramValue;
      }
      if (strstr(paramName.c_str(), "长枪右蓄振刀方式") != NULL) {
        振刀参数::振刀方式::长枪.右蓄 = paramValue;
      }
      if (strstr(paramName.c_str(), "长枪壁击振刀方式") != NULL) {
        振刀参数::振刀方式::长枪.壁击 = paramValue;
      }
      if (strstr(paramName.c_str(), "长枪同源振刀方式") != NULL) {
        振刀参数::振刀方式::长枪.同源 = paramValue;
      }
      if (strstr(paramName.c_str(), "长枪大圣游振刀方式") != NULL) {
        振刀参数::振刀方式::长枪.大圣游 = paramValue;
      }
      if (strstr(paramName.c_str(), "长枪风卷云残振刀方式") != NULL) {
        振刀参数::振刀方式::长枪.风卷云残 = paramValue;
      }
      if (strstr(paramName.c_str(), "长枪双环扫振刀方式") != NULL) {
        振刀参数::振刀方式::长枪.双环扫 = paramValue;
      }
      if (strstr(paramName.c_str(), "长枪龙王破振刀方式") != NULL) {
        振刀参数::振刀方式::长枪.龙王破 = paramValue;
      }
      if (strstr(paramName.c_str(), "长枪穿心脚振刀方式") != NULL) {
        振刀参数::振刀方式::长枪.穿心脚 = paramValue;
      }
      if (strstr(paramName.c_str(), "匕首左3振刀方式") != NULL) {
        振刀参数::振刀方式::匕首.左3 = paramValue;
      }
      if (strstr(paramName.c_str(), "匕首右3振刀方式") != NULL) {
        振刀参数::振刀方式::匕首.右3 = paramValue;
      }
      if (strstr(paramName.c_str(), "匕首左蓄振刀方式") != NULL) {
        振刀参数::振刀方式::匕首.左蓄 = paramValue;
      }
      if (strstr(paramName.c_str(), "匕首右蓄振刀方式") != NULL) {
        振刀参数::振刀方式::匕首.右蓄 = paramValue;
      }
      if (strstr(paramName.c_str(), "匕首鬼反振刀方式") != NULL) {
        振刀参数::振刀方式::匕首.鬼反 = paramValue;
      }
      if (strstr(paramName.c_str(), "匕首荆轲献匕振刀方式") != NULL) {
        振刀参数::振刀方式::匕首.荆轲献匕 = paramValue;
      }
      if (strstr(paramName.c_str(), "匕首壁击振刀方式") != NULL) {
        振刀参数::振刀方式::匕首.壁击 = paramValue;
      }
      if (strstr(paramName.c_str(), "匕首鬼刃暗扎振刀方式") != NULL) {
        振刀参数::振刀方式::匕首.鬼刃暗扎 = paramValue;
      }
      if (strstr(paramName.c_str(), "匕首鬼哭神嚎振刀方式") != NULL) {
        振刀参数::振刀方式::匕首.鬼哭神嚎 = paramValue;
      }
      if (strstr(paramName.c_str(), "匕首亢龙有悔振刀方式") != NULL) {
        振刀参数::振刀方式::匕首.亢龙有悔 = paramValue;
      }
      if (strstr(paramName.c_str(), "双截棍左右3振刀方式") != NULL) {
        振刀参数::振刀方式::双截棍.左右3 = paramValue;
      }
      if (strstr(paramName.c_str(), "双截棍左蓄振刀方式") != NULL) {
        振刀参数::振刀方式::双截棍.左蓄 = paramValue;
      }
      if (strstr(paramName.c_str(), "双截棍右蓄振刀方式") != NULL) {
        振刀参数::振刀方式::双截棍.右蓄 = paramValue;
      }
      if (strstr(paramName.c_str(), "双截棍飞踢振刀方式") != NULL) {
        振刀参数::振刀方式::双截棍.飞踢 = paramValue;
      }
      if (strstr(paramName.c_str(), "双截棍壁击振刀方式") != NULL) {
        振刀参数::振刀方式::双截棍.壁击 = paramValue;
      }
      if (strstr(paramName.c_str(), "双截棍扬鞭劲振刀方式") != NULL) {
        振刀参数::振刀方式::双截棍.扬鞭劲 = paramValue;
      }
      if (strstr(paramName.c_str(), "双截棍横栏振刀方式") != NULL) {
        振刀参数::振刀方式::双截棍.横栏 = paramValue;
      }
      if (strstr(paramName.c_str(), "双截棍龙虎乱舞振刀方式") != NULL) {
        振刀参数::振刀方式::双截棍.龙虎乱舞 = paramValue;
      }
      if (strstr(paramName.c_str(), "双截棍三龙灭阳棍振刀方式") != NULL) {
        振刀参数::振刀方式::双截棍.三龙灭阳棍 = paramValue;
      }
      if (strstr(paramName.c_str(), "双刀左右3振刀方式") != NULL) {
        振刀参数::振刀方式::双刀.左右3 = paramValue;
      }
      if (strstr(paramName.c_str(), "双刀左蓄振刀方式") != NULL) {
        振刀参数::振刀方式::双刀.左蓄 = paramValue;
      }
      if (strstr(paramName.c_str(), "双刀右蓄1振刀方式") != NULL) {
        振刀参数::振刀方式::双刀.右蓄1 = paramValue;
      }
      if (strstr(paramName.c_str(), "双刀右蓄2振刀方式") != NULL) {
        振刀参数::振刀方式::双刀.右蓄2 = paramValue;
      }
      if (strstr(paramName.c_str(), "双刀铁马残红振刀方式") != NULL) {
        振刀参数::振刀方式::双刀.铁马残红 = paramValue;
      }
      if (strstr(paramName.c_str(), "双刀乾坤日月斩振刀方式") != NULL) {
        振刀参数::振刀方式::双刀.乾坤日月斩 = paramValue;
      }
      if (strstr(paramName.c_str(), "双刀惊雷振刀方式") != NULL) {
        振刀参数::振刀方式::双刀.惊雷 = paramValue;
      }
      if (strstr(paramName.c_str(), "双刀壁击振刀方式") != NULL) {
        振刀参数::振刀方式::双刀.壁击 = paramValue;
      }
      if (strstr(paramName.c_str(), "双刀八斩刀振刀方式") != NULL) {
        振刀参数::振刀方式::双刀.八斩刀 = paramValue;
      }
      if (strstr(paramName.c_str(), "双刀分水斩振刀方式") != NULL) {
        振刀参数::振刀方式::双刀.分水斩 = paramValue;
      }
      if (strstr(paramName.c_str(), "长棍左3振刀方式") != NULL) {
        振刀参数::振刀方式::长棍.左3 = paramValue;
      }
      if (strstr(paramName.c_str(), "长棍右3振刀方式") != NULL) {
        振刀参数::振刀方式::长棍.右3 = paramValue;
      }
      if (strstr(paramName.c_str(), "长棍左蓄振刀方式") != NULL) {
        振刀参数::振刀方式::长棍.左蓄 = paramValue;
      }
      if (strstr(paramName.c_str(), "长棍右蓄振刀方式") != NULL) {
        振刀参数::振刀方式::长棍.右蓄 = paramValue;
      }
      if (strstr(paramName.c_str(), "长棍壁击振刀方式") != NULL) {
        振刀参数::振刀方式::长棍.壁击 = paramValue;
      }
      if (strstr(paramName.c_str(), "长棍同源振刀方式") != NULL) {
        振刀参数::振刀方式::长棍.同源 = paramValue;
      }
      if (strstr(paramName.c_str(), "长棍腾云式振刀方式") != NULL) {
        振刀参数::振刀方式::长棍.腾云式 = paramValue;
      }
      if (strstr(paramName.c_str(), "长棍乱点天宫振刀方式") != NULL) {
        振刀参数::振刀方式::长棍.乱点天宫 = paramValue;
      }
      if (strstr(paramName.c_str(), "长棍桶劲振刀方式") != NULL) {
        振刀参数::振刀方式::长棍.桶劲 = paramValue;
      }
      if (strstr(paramName.c_str(), "长棍五情七灭镇振刀方式") != NULL) {
        振刀参数::振刀方式::长棍.五情七灭镇 = paramValue;
      }
      if (strstr(paramName.c_str(), "长棍双环扫振刀方式") != NULL) {
        振刀参数::振刀方式::长棍.双环扫 = paramValue;
      }
      if (strstr(paramName.c_str(), "长棍少林棍振刀方式") != NULL) {
        振刀参数::振刀方式::长棍.少林棍 = paramValue;
      }
      if (strstr(paramName.c_str(), "长棍镇地撑天振刀方式") != NULL) {
        振刀参数::振刀方式::长棍.镇地撑天 = paramValue;
      }
      if (strstr(paramName.c_str(), "斩马刀左左振刀方式") != NULL) {
        振刀参数::振刀方式::斩马刀.左左 = paramValue;
      }
      if (strstr(paramName.c_str(), "斩马刀左右振刀方式") != NULL) {
        振刀参数::振刀方式::斩马刀.左右 = paramValue;
      }
      if (strstr(paramName.c_str(), "斩马刀右右振刀方式") != NULL) {
        振刀参数::振刀方式::斩马刀.右右 = paramValue;
      }
      if (strstr(paramName.c_str(), "斩马刀右左振刀方式") != NULL) {
        振刀参数::振刀方式::斩马刀.右左 = paramValue;
      }
      if (strstr(paramName.c_str(), "斩马刀左蓄1振刀方式") != NULL) {
        振刀参数::振刀方式::斩马刀.左蓄1 = paramValue;
      }
      if (strstr(paramName.c_str(), "斩马刀左蓄2振刀方式") != NULL) {
        振刀参数::振刀方式::斩马刀.左蓄2 = paramValue;
      }
      if (strstr(paramName.c_str(), "斩马刀左蓄3振刀方式") != NULL) {
        振刀参数::振刀方式::斩马刀.左蓄3 = paramValue;
      }
      if (strstr(paramName.c_str(), "斩马刀右蓄1振刀方式") != NULL) {
        振刀参数::振刀方式::斩马刀.右蓄1 = paramValue;
      }
      if (strstr(paramName.c_str(), "斩马刀右蓄2振刀方式") != NULL) {
        振刀参数::振刀方式::斩马刀.右蓄2 = paramValue;
      }
      if (strstr(paramName.c_str(), "斩马刀右蓄3振刀方式") != NULL) {
        振刀参数::振刀方式::斩马刀.右蓄3 = paramValue;
      }
      if (strstr(paramName.c_str(), "斩马刀柄击振刀方式") != NULL) {
        振刀参数::振刀方式::斩马刀.柄击 = paramValue;
      }
      if (strstr(paramName.c_str(), "斩马刀壁击振刀方式") != NULL) {
        振刀参数::振刀方式::斩马刀.壁击 = paramValue;
      }
      if (strstr(paramName.c_str(), "斩马刀同源振刀方式") != NULL) {
        振刀参数::振刀方式::斩马刀.同源 = paramValue;
      }
      if (strstr(paramName.c_str(), "斩马刀惊雷振刀方式") != NULL) {
        振刀参数::振刀方式::斩马刀.惊雷 = paramValue;
      }
      if (strstr(paramName.c_str(), "斩马刀炽焰斩振刀方式") != NULL) {
        振刀参数::振刀方式::斩马刀.炽焰斩 = paramValue;
      }
      if (strstr(paramName.c_str(), "斩马刀过关斩将振刀方式") != NULL) {
        振刀参数::振刀方式::斩马刀.过关斩将 = paramValue;
      }
      if (strstr(paramName.c_str(), "斩马刀奔雷入阵振刀方式") != NULL) {
        振刀参数::振刀方式::斩马刀.奔雷入阵 = paramValue;
      }
      if (strstr(paramName.c_str(), "双戟左3振刀方式") != NULL) {
        振刀参数::振刀方式::双戟.左3 = paramValue;
      }
      if (strstr(paramName.c_str(), "双戟右3振刀方式") != NULL) {
        振刀参数::振刀方式::双戟.右3 = paramValue;
      }
      if (strstr(paramName.c_str(), "双戟左蓄振刀方式") != NULL) {
        振刀参数::振刀方式::双戟.左蓄 = paramValue;
      }
      if (strstr(paramName.c_str(), "双戟右蓄1振刀方式") != NULL) {
        振刀参数::振刀方式::双戟.右蓄1 = paramValue;
      }
      if (strstr(paramName.c_str(), "双戟右蓄2振刀方式") != NULL) {
        振刀参数::振刀方式::双戟.右蓄2 = paramValue;
      }
      if (strstr(paramName.c_str(), "双戟惊雷振刀方式") != NULL) {
        振刀参数::振刀方式::双戟.惊雷 = paramValue;
      }
      if (strstr(paramName.c_str(), "双戟勾旋斩振刀方式") != NULL) {
        振刀参数::振刀方式::双戟.勾旋斩 = paramValue;
      }
      if (strstr(paramName.c_str(), "双戟探海蛟振刀方式") != NULL) {
        振刀参数::振刀方式::双戟.探海蛟 = paramValue;
      }
      if (strstr(paramName.c_str(), "双戟钩挂振刀方式") != NULL) {
        振刀参数::振刀方式::双戟.钩挂 = paramValue;
      }
      if (strstr(paramName.c_str(), "双戟壁击振刀方式") != NULL) {
        振刀参数::振刀方式::双戟.壁击 = paramValue;
      }
      if (strstr(paramName.c_str(), "双戟战龙在天振刀方式") != NULL) {
        振刀参数::振刀方式::双戟.战龙在天 = paramValue;
      }
      if (strstr(paramName.c_str(), "双戟形意钩振刀方式") != NULL) {
        振刀参数::振刀方式::双戟.形意钩 = paramValue;
      }
      if (strstr(paramName.c_str(), "扇子左3振刀方式") != NULL) {
        振刀参数::振刀方式::扇子.左3 = paramValue;
      }
      if (strstr(paramName.c_str(), "扇子右3振刀方式") != NULL) {
        振刀参数::振刀方式::扇子.右3 = paramValue;
      }
      if (strstr(paramName.c_str(), "扇子左蓄振刀方式") != NULL) {
        振刀参数::振刀方式::扇子.左蓄 = paramValue;
      }
      if (strstr(paramName.c_str(), "扇子右蓄振刀方式") != NULL) {
        振刀参数::振刀方式::扇子.右蓄 = paramValue;
      }
      if (strstr(paramName.c_str(), "扇子双开圆振刀方式") != NULL) {
        振刀参数::振刀方式::扇子.双开圆 = paramValue;
      }
      if (strstr(paramName.c_str(), "扇子三风摆振刀方式") != NULL) {
        振刀参数::振刀方式::扇子.三风摆 = paramValue;
      }
      if (strstr(paramName.c_str(), "扇子鬼反振刀方式") != NULL) {
        振刀参数::振刀方式::扇子.鬼反 = paramValue;
      }
      if (strstr(paramName.c_str(), "扇子惊雷振刀方式") != NULL) {
        振刀参数::振刀方式::扇子.惊雷 = paramValue;
      }
      if (strstr(paramName.c_str(), "扇子壁击振刀方式") != NULL) {
        振刀参数::振刀方式::扇子.壁击 = paramValue;
      }
      if (strstr(paramName.c_str(), "扇子缠龙奔野振刀方式") != NULL) {
        振刀参数::振刀方式::扇子.缠龙奔野 = paramValue;
      }
      if (strstr(paramName.c_str(), "扇子泽风上六振刀方式") != NULL) {
        振刀参数::振刀方式::扇子.泽风上六 = paramValue;
      }
      if (strstr(paramName.c_str(), "横刀左3振刀方式") != NULL) {
        振刀参数::振刀方式::横刀.左3 = paramValue;
      }
      if (strstr(paramName.c_str(), "横刀右3振刀方式") != NULL) {
        振刀参数::振刀方式::横刀.右3 = paramValue;
      }
      if (strstr(paramName.c_str(), "横刀左蓄振刀方式") != NULL) {
        振刀参数::振刀方式::横刀.左蓄 = paramValue;
      }
      if (strstr(paramName.c_str(), "横刀右蓄1振刀方式") != NULL) {
        振刀参数::振刀方式::横刀.右蓄1 = paramValue;
      }
      if (strstr(paramName.c_str(), "横刀右蓄2振刀方式") != NULL) {
        振刀参数::振刀方式::横刀.右蓄2 = paramValue;
      }
      if (strstr(paramName.c_str(), "横刀苍牙振刀方式") != NULL) {
        振刀参数::振刀方式::横刀.苍牙 = paramValue;
      }
      if (strstr(paramName.c_str(), "横刀壁击振刀方式") != NULL) {
        振刀参数::振刀方式::横刀.壁击 = paramValue;
      }
      if (strstr(paramName.c_str(), "横刀同源振刀方式") != NULL) {
        振刀参数::振刀方式::横刀.同源 = paramValue;
      }
      if (strstr(paramName.c_str(), "横刀踏空闪振刀方式") != NULL) {
        振刀参数::振刀方式::横刀.踏空闪 = paramValue;
      }
      if (strstr(paramName.c_str(), "横刀破千军振刀方式") != NULL) {
        振刀参数::振刀方式::横刀.破千军 = paramValue;
      }
      if (strstr(paramName.c_str(), "横刀极光破云闪振刀方式") != NULL) {
        振刀参数::振刀方式::横刀.极光破云闪 = paramValue;
      }
      if (strstr(paramName.c_str(), "横刀乾坤一掷振刀方式") != NULL) {
        振刀参数::振刀方式::横刀.乾坤一掷 = paramValue;
      }
      if (strstr(paramName.c_str(), "拳刃左3振刀方式") != NULL) {
        振刀参数::振刀方式::拳刃.左3 = paramValue;
      }
      if (strstr(paramName.c_str(), "拳刃右3振刀方式") != NULL) {
        振刀参数::振刀方式::拳刃.右3 = paramValue;
      }
      if (strstr(paramName.c_str(), "拳刃左蓄力1振刀方式") != NULL) {
        振刀参数::振刀方式::拳刃.左蓄力1 = paramValue;
      }
      if (strstr(paramName.c_str(), "拳刃左蓄力2振刀方式") != NULL) {
        振刀参数::振刀方式::拳刃.左蓄力2 = paramValue;
      }
      if (strstr(paramName.c_str(), "拳刃右蓄力1振刀方式") != NULL) {
        振刀参数::振刀方式::拳刃.右蓄力1 = paramValue;
      }
      if (strstr(paramName.c_str(), "拳刃右蓄力2振刀方式") != NULL) {
        振刀参数::振刀方式::拳刃.右蓄力2 = paramValue;
      }
      if (strstr(paramName.c_str(), "拳刃右蓄力2段振刀方式") != NULL) {
        振刀参数::振刀方式::拳刃.右蓄力2段 = paramValue;
      }
      if (strstr(paramName.c_str(), "拳刃百裂腿振刀方式") != NULL) {
        振刀参数::振刀方式::拳刃.百裂腿 = paramValue;
      }
      if (strstr(paramName.c_str(), "拳刃破空拳振刀方式") != NULL) {
        振刀参数::振刀方式::拳刃.破空拳 = paramValue;
      }
      if (strstr(paramName.c_str(), "拳刃苍牙振刀方式") != NULL) {
        振刀参数::振刀方式::拳刃.苍牙 = paramValue;
      }
      if (strstr(paramName.c_str(), "拳刃壁击振刀方式") != NULL) {
        振刀参数::振刀方式::拳刃.壁击 = paramValue;
      }
      if (strstr(paramName.c_str(), "拳刃太极弄云手振刀方式") != NULL) {
        振刀参数::振刀方式::拳刃.太极弄云手 = paramValue;
      }
      if (strstr(paramName.c_str(), "飞刀左3振刀方式") != NULL) {
        振刀参数::振刀方式::飞刀.左3 = paramValue;
      }
      if (strstr(paramName.c_str(), "飞刀右3振刀方式") != NULL) {
        振刀参数::振刀方式::飞刀.右3 = paramValue;
      }
      if (strstr(paramName.c_str(), "飞刀左蓄力振刀方式") != NULL) {
        振刀参数::振刀方式::飞刀.左蓄力 = paramValue;
      }
      if (strstr(paramName.c_str(), "飞刀左蓄力2段振刀方式") != NULL) {
        振刀参数::振刀方式::飞刀.左蓄力2段 = paramValue;
      }
      if (strstr(paramName.c_str(), "飞刀右蓄力振刀方式") != NULL) {
        振刀参数::振刀方式::飞刀.右蓄力 = paramValue;
      }
      if (strstr(paramName.c_str(), "飞刀掌心雷振刀方式") != NULL) {
        振刀参数::振刀方式::飞刀.掌心雷 = paramValue;
      }
      if (strstr(paramName.c_str(), "飞刀苍牙振刀方式") != NULL) {
        振刀参数::振刀方式::飞刀.苍牙 = paramValue;
      }
      if (strstr(paramName.c_str(), "飞刀太极振刀方式") != NULL) {
        振刀参数::振刀方式::飞刀.太极 = paramValue;
      }
      if (strstr(paramName.c_str(), "飞刀壁击振刀方式") != NULL) {
        振刀参数::振刀方式::飞刀.壁击 = paramValue;
      }
      if (strstr(paramName.c_str(), "飞刀断月千刃舞振刀方式") != NULL) {
        振刀参数::振刀方式::飞刀.断月千刃舞 = paramValue;
      }
      if (strstr(paramName.c_str(), "飞刀万点寒梅振刀方式") != NULL) {
        振刀参数::振刀方式::飞刀.万点寒梅 = paramValue;
      }

      // 间隔
      if (strstr(paramName.c_str(), "长剑左3闪避") != NULL) {
        振刀参数::闪避::长剑.左3 = paramValue;
      }
      if (strstr(paramName.c_str(), "长剑右3闪避") != NULL) {
        振刀参数::闪避::长剑.右3 = paramValue;
      }
      if (strstr(paramName.c_str(), "长剑左剑气1闪避") != NULL) {
        振刀参数::闪避::长剑.左剑气1 = paramValue;
      }
      if (strstr(paramName.c_str(), "长剑左剑气2闪避") != NULL) {
        振刀参数::闪避::长剑.左剑气2 = paramValue;
      }
      if (strstr(paramName.c_str(), "长剑右剑气1闪避") != NULL) {
        振刀参数::闪避::长剑.右剑气1 = paramValue;
      }
      if (strstr(paramName.c_str(), "长剑右剑气2闪避") != NULL) {
        振刀参数::闪避::长剑.右剑气2 = paramValue;
      }
      if (strstr(paramName.c_str(), "长剑凤凰羽1闪避") != NULL) {
        振刀参数::闪避::长剑.凤凰羽1 = paramValue;
      }
      if (strstr(paramName.c_str(), "长剑凤凰羽2闪避") != NULL) {
        振刀参数::闪避::长剑.凤凰羽2 = paramValue;
      }
      if (strstr(paramName.c_str(), "长剑蓝月闪避") != NULL) {
        振刀参数::闪避::长剑.苍牙 = paramValue;
      }
      if (strstr(paramName.c_str(), "长剑跳斩闪避") != NULL) {
        振刀参数::闪避::长剑.跳斩 = paramValue;
      }
      if (strstr(paramName.c_str(), "长剑壁击闪避") != NULL) {
        振刀参数::闪避::长剑.壁击 = paramValue;
      }
      if (strstr(paramName.c_str(), "长剑七星夺窍闪避") != NULL) {
        振刀参数::闪避::长剑.七星夺窍 = paramValue;
      }
      if (strstr(paramName.c_str(), "链剑左3闪避") != NULL) {
        振刀参数::闪避::链剑.左3 = paramValue;
      }
      if (strstr(paramName.c_str(), "链剑右3闪避") != NULL) {
        振刀参数::闪避::链剑.右3 = paramValue;
      }
      if (strstr(paramName.c_str(), "链剑左蓄力1闪避") != NULL) {
        振刀参数::闪避::链剑.左蓄力1 = paramValue;
      }
      if (strstr(paramName.c_str(), "链剑左蓄力2闪避") != NULL) {
        振刀参数::闪避::链剑.左蓄力2 = paramValue;
      }
      if (strstr(paramName.c_str(), "链剑右蓄力1闪避") != NULL) {
        振刀参数::闪避::链剑.右蓄力1 = paramValue;
      }
      if (strstr(paramName.c_str(), "链剑右蓄力2闪避") != NULL) {
        振刀参数::闪避::链剑.右蓄力2 = paramValue;
      }
      if (strstr(paramName.c_str(), "链剑地龙滚堂刹闪避") != NULL) {
        振刀参数::闪避::链剑.地龙滚堂刹 = paramValue;
      }
      if (strstr(paramName.c_str(), "链剑苍牙闪避") != NULL) {
        振刀参数::闪避::链剑.苍牙 = paramValue;
      }
      if (strstr(paramName.c_str(), "链剑火龙卷云闪避") != NULL) {
        振刀参数::闪避::链剑.火龙卷云 = paramValue;
      }
      if (strstr(paramName.c_str(), "链剑断罪碎蜂闪避") != NULL) {
        振刀参数::闪避::链剑.断罪碎蜂 = paramValue;
      }
      if (strstr(paramName.c_str(), "链剑蓄力追击闪避") != NULL) {
        振刀参数::闪避::链剑.蓄力追击 = paramValue;
      }
      if (strstr(paramName.c_str(), "链剑长剑同源") != NULL) {
        振刀参数::闪避::链剑.链剑长剑同源 = paramValue;
      }
      if (strstr(paramName.c_str(), "太刀左3闪避") != NULL) {
        振刀参数::闪避::太刀.左3 = paramValue;
      }
      if (strstr(paramName.c_str(), "太刀右3闪避") != NULL) {
        振刀参数::闪避::太刀.右3 = paramValue;
      }
      if (strstr(paramName.c_str(), "太刀左蓄1闪避") != NULL) {
        振刀参数::闪避::太刀.左蓄1 = paramValue;
      }
      if (strstr(paramName.c_str(), "太刀左蓄2闪避") != NULL) {
        振刀参数::闪避::太刀.左蓄2 = paramValue;
      }
      if (strstr(paramName.c_str(), "太刀右蓄1闪避") != NULL) {
        振刀参数::闪避::太刀.右蓄1 = paramValue;
      }
      if (strstr(paramName.c_str(), "太刀右蓄2闪避") != NULL) {
        振刀参数::闪避::太刀.右蓄2 = paramValue;
      }
      if (strstr(paramName.c_str(), "太刀苍牙闪避") != NULL) {
        振刀参数::闪避::太刀.苍牙 = paramValue;
      }
      if (strstr(paramName.c_str(), "太刀壁击闪避") != NULL) {
        振刀参数::闪避::太刀.壁击 = paramValue;
      }
      if (strstr(paramName.c_str(), "太刀刹那斩1闪避") != NULL) {
        振刀参数::闪避::太刀.刹那斩1 = paramValue;
      }
      if (strstr(paramName.c_str(), "太刀刹那斩2闪避") != NULL) {
        振刀参数::闪避::太刀.刹那斩2 = paramValue;
      }
      if (strstr(paramName.c_str(), "太刀惊雷闪避") != NULL) {
        振刀参数::闪避::太刀.惊雷 = paramValue;
      }
      if (strstr(paramName.c_str(), "太刀惊雷十劫闪避") != NULL) {
        振刀参数::闪避::太刀.惊雷十劫 = paramValue;
      }
      if (strstr(paramName.c_str(), "太刀青鬼闪避") != NULL) {
        振刀参数::闪避::太刀.青鬼 = paramValue;
      }
      if (strstr(paramName.c_str(), "太刀同源闪避") != NULL) {
        振刀参数::闪避::太刀.同源 = paramValue;
      }
      if (strstr(paramName.c_str(), "太刀噬魂斩1闪避") != NULL) {
        振刀参数::闪避::太刀.噬魂斩1 = paramValue;
      }
      if (strstr(paramName.c_str(), "太刀噬魂斩2闪避") != NULL) {
        振刀参数::闪避::太刀.噬魂斩2 = paramValue;
      }
      if (strstr(paramName.c_str(), "阔刀左左闪避") != NULL) {
        振刀参数::闪避::阔刀.左左 = paramValue;
      }
      if (strstr(paramName.c_str(), "阔刀左右闪避") != NULL) {
        振刀参数::闪避::阔刀.左右 = paramValue;
      }
      if (strstr(paramName.c_str(), "阔刀右右闪避") != NULL) {
        振刀参数::闪避::阔刀.右右 = paramValue;
      }
      if (strstr(paramName.c_str(), "阔刀右左闪避") != NULL) {
        振刀参数::闪避::阔刀.右左 = paramValue;
      }
      if (strstr(paramName.c_str(), "阔刀左蓄一段闪避") != NULL) {
        振刀参数::闪避::阔刀.左蓄一段 = paramValue;
      }
      if (strstr(paramName.c_str(), "阔刀左蓄二段闪避") != NULL) {
        振刀参数::闪避::阔刀.左蓄二段 = paramValue;
      }
      if (strstr(paramName.c_str(), "阔刀左蓄三段闪避") != NULL) {
        振刀参数::闪避::阔刀.左蓄三段 = paramValue;
      }
      if (strstr(paramName.c_str(), "阔刀右蓄1闪避") != NULL) {
        振刀参数::闪避::阔刀.右蓄1 = paramValue;
      }
      if (strstr(paramName.c_str(), "阔刀右蓄2闪避") != NULL) {
        振刀参数::闪避::阔刀.右蓄2 = paramValue;
      }
      if (strstr(paramName.c_str(), "阔刀右右蓄闪避") != NULL) {
        振刀参数::闪避::阔刀.右右蓄 = paramValue;
      }
      if (strstr(paramName.c_str(), "阔刀雷刀闪避") != NULL) {
        振刀参数::闪避::阔刀.雷刀 = paramValue;
      }
      if (strstr(paramName.c_str(), "阔刀壁击闪避") != NULL) {
        振刀参数::闪避::阔刀.壁击 = paramValue;
      }
      if (strstr(paramName.c_str(), "阔刀同源闪避") != NULL) {
        振刀参数::闪避::阔刀.同源 = paramValue;
      }
      if (strstr(paramName.c_str(), "阔刀雷刀下劈闪避") != NULL) {
        振刀参数::闪避::阔刀.雷刀下劈 = paramValue;
      }
      if (strstr(paramName.c_str(), "阔刀翻江倒海闪避") != NULL) {
        振刀参数::闪避::阔刀.翻江倒海 = paramValue;
      }
      if (strstr(paramName.c_str(), "长枪左3闪避") != NULL) {
        振刀参数::闪避::长枪.左3 = paramValue;
      }
      if (strstr(paramName.c_str(), "长枪右3闪避") != NULL) {
        振刀参数::闪避::长枪.右3 = paramValue;
      }
      if (strstr(paramName.c_str(), "长枪左蓄闪避") != NULL) {
        振刀参数::闪避::长枪.左蓄 = paramValue;
      }
      if (strstr(paramName.c_str(), "长枪六合枪闪避") != NULL) {
        振刀参数::闪避::长枪.六合枪 = paramValue;
      }
      if (strstr(paramName.c_str(), "长枪右蓄闪避") != NULL) {
        振刀参数::闪避::长枪.右蓄 = paramValue;
      }
      if (strstr(paramName.c_str(), "长枪壁击闪避") != NULL) {
        振刀参数::闪避::长枪.壁击 = paramValue;
      }
      if (strstr(paramName.c_str(), "长枪同源闪避") != NULL) {
        振刀参数::闪避::长枪.同源 = paramValue;
      }
      if (strstr(paramName.c_str(), "长枪大圣游闪避") != NULL) {
        振刀参数::闪避::长枪.大圣游 = paramValue;
      }
      if (strstr(paramName.c_str(), "长枪风卷云残闪避") != NULL) {
        振刀参数::闪避::长枪.风卷云残 = paramValue;
      }
      if (strstr(paramName.c_str(), "长枪双环扫闪避") != NULL) {
        振刀参数::闪避::长枪.双环扫 = paramValue;
      }
      if (strstr(paramName.c_str(), "长枪龙王破闪避") != NULL) {
        振刀参数::闪避::长枪.龙王破 = paramValue;
      }
      if (strstr(paramName.c_str(), "长枪穿心脚闪避") != NULL) {
        振刀参数::闪避::长枪.穿心脚 = paramValue;
      }
      if (strstr(paramName.c_str(), "匕首左3闪避") != NULL) {
        振刀参数::闪避::匕首.左3 = paramValue;
      }
      if (strstr(paramName.c_str(), "匕首右3闪避") != NULL) {
        振刀参数::闪避::匕首.右3 = paramValue;
      }
      if (strstr(paramName.c_str(), "匕首左蓄闪避") != NULL) {
        振刀参数::闪避::匕首.左蓄 = paramValue;
      }
      if (strstr(paramName.c_str(), "匕首右蓄闪避") != NULL) {
        振刀参数::闪避::匕首.右蓄 = paramValue;
      }
      if (strstr(paramName.c_str(), "匕首鬼反闪避") != NULL) {
        振刀参数::闪避::匕首.鬼反 = paramValue;
      }
      if (strstr(paramName.c_str(), "匕首荆轲献匕闪避") != NULL) {
        振刀参数::闪避::匕首.荆轲献匕 = paramValue;
      }
      if (strstr(paramName.c_str(), "匕首壁击闪避") != NULL) {
        振刀参数::闪避::匕首.壁击 = paramValue;
      }
      if (strstr(paramName.c_str(), "匕首鬼刃暗扎闪避") != NULL) {
        振刀参数::闪避::匕首.鬼刃暗扎 = paramValue;
      }
      if (strstr(paramName.c_str(), "匕首鬼哭神嚎闪避") != NULL) {
        振刀参数::闪避::匕首.鬼哭神嚎 = paramValue;
      }
      if (strstr(paramName.c_str(), "匕首亢龙有悔闪避") != NULL) {
        振刀参数::闪避::匕首.亢龙有悔 = paramValue;
      }
      if (strstr(paramName.c_str(), "双截棍左右3闪避") != NULL) {
        振刀参数::闪避::双截棍.左右3 = paramValue;
      }
      if (strstr(paramName.c_str(), "双截棍左蓄闪避") != NULL) {
        振刀参数::闪避::双截棍.左蓄 = paramValue;
      }
      if (strstr(paramName.c_str(), "双截棍右蓄闪避") != NULL) {
        振刀参数::闪避::双截棍.右蓄 = paramValue;
      }
      if (strstr(paramName.c_str(), "双截棍飞踢闪避") != NULL) {
        振刀参数::闪避::双截棍.飞踢 = paramValue;
      }
      if (strstr(paramName.c_str(), "双截棍壁击闪避") != NULL) {
        振刀参数::闪避::双截棍.壁击 = paramValue;
      }
      if (strstr(paramName.c_str(), "双截棍扬鞭劲闪避") != NULL) {
        振刀参数::闪避::双截棍.扬鞭劲 = paramValue;
      }
      if (strstr(paramName.c_str(), "双截棍横栏闪避") != NULL) {
        振刀参数::闪避::双截棍.横栏 = paramValue;
      }
      if (strstr(paramName.c_str(), "双截棍龙虎乱舞闪避") != NULL) {
        振刀参数::闪避::双截棍.龙虎乱舞 = paramValue;
      }
      if (strstr(paramName.c_str(), "双截棍三龙灭阳棍闪避") != NULL) {
        振刀参数::闪避::双截棍.三龙灭阳棍 = paramValue;
      }
      if (strstr(paramName.c_str(), "双刀左右3闪避") != NULL) {
        振刀参数::闪避::双刀.左右3 = paramValue;
      }
      if (strstr(paramName.c_str(), "双刀左蓄闪避") != NULL) {
        振刀参数::闪避::双刀.左蓄 = paramValue;
      }
      if (strstr(paramName.c_str(), "双刀右蓄1闪避") != NULL) {
        振刀参数::闪避::双刀.右蓄1 = paramValue;
      }
      if (strstr(paramName.c_str(), "双刀右蓄2闪避") != NULL) {
        振刀参数::闪避::双刀.右蓄2 = paramValue;
      }
      if (strstr(paramName.c_str(), "双刀铁马残红闪避") != NULL) {
        振刀参数::闪避::双刀.铁马残红 = paramValue;
      }
      if (strstr(paramName.c_str(), "双刀乾坤日月斩闪避") != NULL) {
        振刀参数::闪避::双刀.乾坤日月斩 = paramValue;
      }
      if (strstr(paramName.c_str(), "双刀惊雷闪避") != NULL) {
        振刀参数::闪避::双刀.惊雷 = paramValue;
      }
      if (strstr(paramName.c_str(), "双刀壁击闪避") != NULL) {
        振刀参数::闪避::双刀.壁击 = paramValue;
      }
      if (strstr(paramName.c_str(), "双刀八斩刀闪避") != NULL) {
        振刀参数::闪避::双刀.八斩刀 = paramValue;
      }
      if (strstr(paramName.c_str(), "双刀分水斩闪避") != NULL) {
        振刀参数::闪避::双刀.分水斩 = paramValue;
      }
      if (strstr(paramName.c_str(), "长棍左3闪避") != NULL) {
        振刀参数::闪避::长棍.左3 = paramValue;
      }
      if (strstr(paramName.c_str(), "长棍右3闪避") != NULL) {
        振刀参数::闪避::长棍.右3 = paramValue;
      }
      if (strstr(paramName.c_str(), "长棍左蓄闪避") != NULL) {
        振刀参数::闪避::长棍.左蓄 = paramValue;
      }
      if (strstr(paramName.c_str(), "长棍右蓄闪避") != NULL) {
        振刀参数::闪避::长棍.右蓄 = paramValue;
      }
      if (strstr(paramName.c_str(), "长棍壁击闪避") != NULL) {
        振刀参数::闪避::长棍.壁击 = paramValue;
      }
      if (strstr(paramName.c_str(), "长棍同源闪避") != NULL) {
        振刀参数::闪避::长棍.同源 = paramValue;
      }
      if (strstr(paramName.c_str(), "长棍腾云式闪避") != NULL) {
        振刀参数::闪避::长棍.腾云式 = paramValue;
      }
      if (strstr(paramName.c_str(), "长棍乱点天宫闪避") != NULL) {
        振刀参数::闪避::长棍.乱点天宫 = paramValue;
      }
      if (strstr(paramName.c_str(), "长棍桶劲闪避") != NULL) {
        振刀参数::闪避::长棍.桶劲 = paramValue;
      }
      if (strstr(paramName.c_str(), "长棍五情七灭镇闪避") != NULL) {
        振刀参数::闪避::长棍.五情七灭镇 = paramValue;
      }
      if (strstr(paramName.c_str(), "长棍双环扫闪避") != NULL) {
        振刀参数::闪避::长棍.双环扫 = paramValue;
      }
      if (strstr(paramName.c_str(), "长棍少林棍闪避") != NULL) {
        振刀参数::闪避::长棍.少林棍 = paramValue;
      }
      if (strstr(paramName.c_str(), "长棍镇地撑天闪避") != NULL) {
        振刀参数::闪避::长棍.镇地撑天 = paramValue;
      }
      if (strstr(paramName.c_str(), "斩马刀左左闪避") != NULL) {
        振刀参数::闪避::斩马刀.左左 = paramValue;
      }
      if (strstr(paramName.c_str(), "斩马刀左右闪避") != NULL) {
        振刀参数::闪避::斩马刀.左右 = paramValue;
      }
      if (strstr(paramName.c_str(), "斩马刀右右闪避") != NULL) {
        振刀参数::闪避::斩马刀.右右 = paramValue;
      }
      if (strstr(paramName.c_str(), "斩马刀右左闪避") != NULL) {
        振刀参数::闪避::斩马刀.右左 = paramValue;
      }
      if (strstr(paramName.c_str(), "斩马刀左蓄1闪避") != NULL) {
        振刀参数::闪避::斩马刀.左蓄1 = paramValue;
      }
      if (strstr(paramName.c_str(), "斩马刀左蓄2闪避") != NULL) {
        振刀参数::闪避::斩马刀.左蓄2 = paramValue;
      }
      if (strstr(paramName.c_str(), "斩马刀左蓄3闪避") != NULL) {
        振刀参数::闪避::斩马刀.左蓄3 = paramValue;
      }
      if (strstr(paramName.c_str(), "斩马刀右蓄1闪避") != NULL) {
        振刀参数::闪避::斩马刀.右蓄1 = paramValue;
      }
      if (strstr(paramName.c_str(), "斩马刀右蓄2闪避") != NULL) {
        振刀参数::闪避::斩马刀.右蓄2 = paramValue;
      }
      if (strstr(paramName.c_str(), "斩马刀右蓄3闪避") != NULL) {
        振刀参数::闪避::斩马刀.右蓄3 = paramValue;
      }
      if (strstr(paramName.c_str(), "斩马刀柄击闪避") != NULL) {
        振刀参数::闪避::斩马刀.柄击 = paramValue;
      }
      if (strstr(paramName.c_str(), "斩马刀壁击闪避") != NULL) {
        振刀参数::闪避::斩马刀.壁击 = paramValue;
      }
      if (strstr(paramName.c_str(), "斩马刀同源闪避") != NULL) {
        振刀参数::闪避::斩马刀.同源 = paramValue;
      }
      if (strstr(paramName.c_str(), "斩马刀惊雷闪避") != NULL) {
        振刀参数::闪避::斩马刀.惊雷 = paramValue;
      }
      if (strstr(paramName.c_str(), "斩马刀炽焰斩闪避") != NULL) {
        振刀参数::闪避::斩马刀.炽焰斩 = paramValue;
      }
      if (strstr(paramName.c_str(), "斩马刀过关斩将闪避") != NULL) {
        振刀参数::闪避::斩马刀.过关斩将 = paramValue;
      }
      if (strstr(paramName.c_str(), "斩马刀奔雷入阵闪避") != NULL) {
        振刀参数::闪避::斩马刀.奔雷入阵 = paramValue;
      }
      if (strstr(paramName.c_str(), "双戟左3闪避") != NULL) {
        振刀参数::闪避::双戟.左3 = paramValue;
      }
      if (strstr(paramName.c_str(), "双戟右3闪避") != NULL) {
        振刀参数::闪避::双戟.右3 = paramValue;
      }
      if (strstr(paramName.c_str(), "双戟左蓄闪避") != NULL) {
        振刀参数::闪避::双戟.左蓄 = paramValue;
      }
      if (strstr(paramName.c_str(), "双戟右蓄1闪避") != NULL) {
        振刀参数::闪避::双戟.右蓄1 = paramValue;
      }
      if (strstr(paramName.c_str(), "双戟右蓄2闪避") != NULL) {
        振刀参数::闪避::双戟.右蓄2 = paramValue;
      }
      if (strstr(paramName.c_str(), "双戟惊雷闪避") != NULL) {
        振刀参数::闪避::双戟.惊雷 = paramValue;
      }
      if (strstr(paramName.c_str(), "双戟勾旋斩闪避") != NULL) {
        振刀参数::闪避::双戟.勾旋斩 = paramValue;
      }
      if (strstr(paramName.c_str(), "双戟探海蛟闪避") != NULL) {
        振刀参数::闪避::双戟.探海蛟 = paramValue;
      }
      if (strstr(paramName.c_str(), "双戟钩挂闪避") != NULL) {
        振刀参数::闪避::双戟.钩挂 = paramValue;
      }
      if (strstr(paramName.c_str(), "双戟壁击闪避") != NULL) {
        振刀参数::闪避::双戟.壁击 = paramValue;
      }
      if (strstr(paramName.c_str(), "双戟战龙在天闪避") != NULL) {
        振刀参数::闪避::双戟.战龙在天 = paramValue;
      }
      if (strstr(paramName.c_str(), "双戟形意钩闪避") != NULL) {
        振刀参数::闪避::双戟.形意钩 = paramValue;
      }
      if (strstr(paramName.c_str(), "扇子左3闪避") != NULL) {
        振刀参数::闪避::扇子.左3 = paramValue;
      }
      if (strstr(paramName.c_str(), "扇子右3闪避") != NULL) {
        振刀参数::闪避::扇子.右3 = paramValue;
      }
      if (strstr(paramName.c_str(), "扇子左蓄闪避") != NULL) {
        振刀参数::闪避::扇子.左蓄 = paramValue;
      }
      if (strstr(paramName.c_str(), "扇子右蓄闪避") != NULL) {
        振刀参数::闪避::扇子.右蓄 = paramValue;
      }
      if (strstr(paramName.c_str(), "扇子双开圆闪避") != NULL) {
        振刀参数::闪避::扇子.双开圆 = paramValue;
      }
      if (strstr(paramName.c_str(), "扇子三风摆闪避") != NULL) {
        振刀参数::闪避::扇子.三风摆 = paramValue;
      }
      if (strstr(paramName.c_str(), "扇子鬼反闪避") != NULL) {
        振刀参数::闪避::扇子.鬼反 = paramValue;
      }
      if (strstr(paramName.c_str(), "扇子惊雷闪避") != NULL) {
        振刀参数::闪避::扇子.惊雷 = paramValue;
      }
      if (strstr(paramName.c_str(), "扇子壁击闪避") != NULL) {
        振刀参数::闪避::扇子.壁击 = paramValue;
      }
      if (strstr(paramName.c_str(), "扇子缠龙奔野闪避") != NULL) {
        振刀参数::闪避::扇子.缠龙奔野 = paramValue;
      }
      if (strstr(paramName.c_str(), "扇子泽风上六闪避") != NULL) {
        振刀参数::闪避::扇子.泽风上六 = paramValue;
      }
      if (strstr(paramName.c_str(), "横刀左3闪避") != NULL) {
        振刀参数::闪避::横刀.左3 = paramValue;
      }
      if (strstr(paramName.c_str(), "横刀右3闪避") != NULL) {
        振刀参数::闪避::横刀.右3 = paramValue;
      }
      if (strstr(paramName.c_str(), "横刀左蓄闪避") != NULL) {
        振刀参数::闪避::横刀.左蓄 = paramValue;
      }
      if (strstr(paramName.c_str(), "横刀右蓄1闪避") != NULL) {
        振刀参数::闪避::横刀.右蓄1 = paramValue;
      }
      if (strstr(paramName.c_str(), "横刀右蓄2闪避") != NULL) {
        振刀参数::闪避::横刀.右蓄2 = paramValue;
      }
      if (strstr(paramName.c_str(), "横刀苍牙闪避") != NULL) {
        振刀参数::闪避::横刀.苍牙 = paramValue;
      }
      if (strstr(paramName.c_str(), "横刀壁击闪避") != NULL) {
        振刀参数::闪避::横刀.壁击 = paramValue;
      }
      if (strstr(paramName.c_str(), "横刀同源闪避") != NULL) {
        振刀参数::闪避::横刀.同源 = paramValue;
      }
      if (strstr(paramName.c_str(), "横刀踏空闪闪避") != NULL) {
        振刀参数::闪避::横刀.踏空闪 = paramValue;
      }
      if (strstr(paramName.c_str(), "横刀破千军闪避") != NULL) {
        振刀参数::闪避::横刀.破千军 = paramValue;
      }
      if (strstr(paramName.c_str(), "横刀极光破云闪闪避") != NULL) {
        振刀参数::闪避::横刀.极光破云闪 = paramValue;
      }
      if (strstr(paramName.c_str(), "横刀乾坤一掷闪避") != NULL) {
        振刀参数::闪避::横刀.乾坤一掷 = paramValue;
      }
      if (strstr(paramName.c_str(), "拳刃左3闪避") != NULL) {
        振刀参数::闪避::拳刃.左3 = paramValue;
      }
      if (strstr(paramName.c_str(), "拳刃右3闪避") != NULL) {
        振刀参数::闪避::拳刃.右3 = paramValue;
      }
      if (strstr(paramName.c_str(), "拳刃左蓄力1闪避") != NULL) {
        振刀参数::闪避::拳刃.左蓄力1 = paramValue;
      }
      if (strstr(paramName.c_str(), "拳刃左蓄力2闪避") != NULL) {
        振刀参数::闪避::拳刃.左蓄力2 = paramValue;
      }
      if (strstr(paramName.c_str(), "拳刃右蓄力1闪避") != NULL) {
        振刀参数::闪避::拳刃.右蓄力1 = paramValue;
      }
      if (strstr(paramName.c_str(), "拳刃右蓄力2闪避") != NULL) {
        振刀参数::闪避::拳刃.右蓄力2 = paramValue;
      }
      if (strstr(paramName.c_str(), "拳刃右蓄力2段闪避") != NULL) {
        振刀参数::闪避::拳刃.右蓄力2段 = paramValue;
      }
      if (strstr(paramName.c_str(), "拳刃百裂腿闪避") != NULL) {
        振刀参数::闪避::拳刃.百裂腿 = paramValue;
      }
      if (strstr(paramName.c_str(), "拳刃破空拳闪避") != NULL) {
        振刀参数::闪避::拳刃.破空拳 = paramValue;
      }
      if (strstr(paramName.c_str(), "拳刃苍牙闪避") != NULL) {
        振刀参数::闪避::拳刃.苍牙 = paramValue;
      }
      if (strstr(paramName.c_str(), "拳刃壁击闪避") != NULL) {
        振刀参数::闪避::拳刃.壁击 = paramValue;
      }
      if (strstr(paramName.c_str(), "拳刃太极弄云手闪避") != NULL) {
        振刀参数::闪避::拳刃.太极弄云手 = paramValue;
      }
      if (strstr(paramName.c_str(), "飞刀左3闪避") != NULL) {
        振刀参数::闪避::飞刀.左3 = paramValue;
      }
      if (strstr(paramName.c_str(), "飞刀右3闪避") != NULL) {
        振刀参数::闪避::飞刀.右3 = paramValue;
      }
      if (strstr(paramName.c_str(), "飞刀左蓄力闪避") != NULL) {
        振刀参数::闪避::飞刀.左蓄力 = paramValue;
      }
      if (strstr(paramName.c_str(), "飞刀左蓄力2段闪避") != NULL) {
        振刀参数::闪避::飞刀.左蓄力2段 = paramValue;
      }
      if (strstr(paramName.c_str(), "飞刀右蓄力闪避") != NULL) {
        振刀参数::闪避::飞刀.右蓄力 = paramValue;
      }
      if (strstr(paramName.c_str(), "飞刀掌心雷闪避") != NULL) {
        振刀参数::闪避::飞刀.掌心雷 = paramValue;
      }
      if (strstr(paramName.c_str(), "飞刀苍牙闪避") != NULL) {
        振刀参数::闪避::飞刀.苍牙 = paramValue;
      }
      if (strstr(paramName.c_str(), "飞刀太极闪避") != NULL) {
        振刀参数::闪避::飞刀.太极 = paramValue;
      }
      if (strstr(paramName.c_str(), "飞刀壁击闪避") != NULL) {
        振刀参数::闪避::飞刀.壁击 = paramValue;
      }
      if (strstr(paramName.c_str(), "飞刀断月千刃舞闪避") != NULL) {
        振刀参数::闪避::飞刀.断月千刃舞 = paramValue;
      }
      if (strstr(paramName.c_str(), "飞刀万点寒梅闪避") != NULL) {
        振刀参数::闪避::飞刀.万点寒梅 = paramValue;
      }

      // 太刀
      if (strstr(paramName.c_str(), "太刀左3距离") != NULL) {
        振刀参数::距离::太刀.左3 = paramValue;
      }
      if (strstr(paramName.c_str(), "太刀右3距离") != NULL) {
        振刀参数::距离::太刀.右3 = paramValue;
      }
      if (strstr(paramName.c_str(), "太刀左蓄距离") != NULL) {
        振刀参数::距离::太刀.左蓄 = paramValue;
      }
      if (strstr(paramName.c_str(), "太刀右蓄距离") != NULL) {
        振刀参数::距离::太刀.右蓄 = paramValue;
      }
      if (strstr(paramName.c_str(), "太刀壁击距离") != NULL) {
        振刀参数::距离::太刀.壁击 = paramValue;
      }
      if (strstr(paramName.c_str(), "太刀苍牙距离") != NULL) {
        振刀参数::距离::太刀.苍牙 = paramValue;
      }
      if (strstr(paramName.c_str(), "太刀刹那斩距离") != NULL) {
        振刀参数::距离::太刀.刹那斩 = paramValue;
      }
      if (strstr(paramName.c_str(), "太刀右惊雷距离") != NULL) {
        振刀参数::距离::太刀.惊雷 = paramValue;
      }
      if (strstr(paramName.c_str(), "太刀惊雷十劫距离") != NULL) {
        振刀参数::距离::太刀.惊雷十劫 = paramValue;
      }
      if (strstr(paramName.c_str(), "太刀青鬼距离") != NULL) {
        振刀参数::距离::太刀.青鬼 = paramValue;
      }
      if (strstr(paramName.c_str(), "太刀同源距离") != NULL) {
        振刀参数::距离::太刀.同源 = paramValue;
      }
      if (strstr(paramName.c_str(), "太刀噬魂斩距离") != NULL) {
        振刀参数::距离::太刀.噬魂斩 = paramValue;
      }
      // 延迟
      if (strstr(paramName.c_str(), "太刀左3延迟") != NULL) {
        振刀参数::延迟::太刀.左3 = paramValue;
      }
      if (strstr(paramName.c_str(), "太刀右3延迟") != NULL) {
        振刀参数::延迟::太刀.右3 = paramValue;
      }
      if (strstr(paramName.c_str(), "太刀左蓄延迟") != NULL) {
        振刀参数::延迟::太刀.左蓄 = paramValue;
      }
      if (strstr(paramName.c_str(), "太刀右蓄延迟") != NULL) {
        振刀参数::延迟::太刀.右蓄 = paramValue;
      }
      if (strstr(paramName.c_str(), "太刀苍牙延迟") != NULL) {
        振刀参数::延迟::太刀.苍牙 = paramValue;
      }
      if (strstr(paramName.c_str(), "太刀刹那斩延迟") != NULL) {
        振刀参数::延迟::太刀.刹那斩 = paramValue;
      }
      if (strstr(paramName.c_str(), "太刀右惊雷延迟") != NULL) {
        振刀参数::延迟::太刀.右惊雷 = paramValue;
      }
      if (strstr(paramName.c_str(), "太刀青鬼延迟") != NULL) {
        振刀参数::延迟::太刀.青鬼 = paramValue;
      }
      if (strstr(paramName.c_str(), "太刀噬魂斩延迟") != NULL) {
        振刀参数::延迟::太刀.噬魂斩 = paramValue;
      }
      if (strstr(paramName.c_str(), "太刀壁击延迟") != NULL) {
        振刀参数::延迟::太刀.壁击 = paramValue;
      }
      if (strstr(paramName.c_str(), "太刀同源延迟") != NULL) {
        振刀参数::延迟::太刀.同源 = paramValue;
      }
      // 角度
      if (strstr(paramName.c_str(), "太刀左3角度") != NULL) {
        振刀参数::角度::太刀.左3 = paramValue;
      }
      if (strstr(paramName.c_str(), "太刀右3角度") != NULL) {
        振刀参数::角度::太刀.右3 = paramValue;
      }
      if (strstr(paramName.c_str(), "太刀左蓄角度") != NULL) {
        振刀参数::角度::太刀.左蓄 = paramValue;
      }
      if (strstr(paramName.c_str(), "太刀右蓄角度") != NULL) {
        振刀参数::角度::太刀.右蓄 = paramValue;
      }
      if (strstr(paramName.c_str(), "太刀苍牙角度") != NULL) {
        振刀参数::角度::太刀.苍牙 = paramValue;
      }
      if (strstr(paramName.c_str(), "太刀壁击角度") != NULL) {
        振刀参数::角度::太刀.壁击 = paramValue;
      }
      if (strstr(paramName.c_str(), "太刀刹那斩角度") != NULL) {
        振刀参数::角度::太刀.刹那斩 = paramValue;
      }
      if (strstr(paramName.c_str(), "太刀惊雷&惊雷十劫角度") != NULL) {
        振刀参数::角度::太刀.惊雷十劫 = paramValue;
      }
      if (strstr(paramName.c_str(), "太刀青鬼角度") != NULL) {
        振刀参数::角度::太刀.青鬼 = paramValue;
      }
      if (strstr(paramName.c_str(), "太刀同源角度") != NULL) {
        振刀参数::角度::太刀.同源 = paramValue;
      }
      // 阔刀
      if (strstr(paramName.c_str(), "阔刀左左距离") != NULL) {
        振刀参数::距离::阔刀.左左 = paramValue;
      }
      if (strstr(paramName.c_str(), "阔刀左右距离") != NULL) {
        振刀参数::距离::阔刀.左右 = paramValue;
      }
      if (strstr(paramName.c_str(), "阔刀右右距离") != NULL) {
        振刀参数::距离::阔刀.右右 = paramValue;
      }
      if (strstr(paramName.c_str(), "阔刀右左距离") != NULL) {
        振刀参数::距离::阔刀.右左 = paramValue;
      }
      if (strstr(paramName.c_str(), "阔刀壁击距离") != NULL) {
        振刀参数::距离::阔刀.壁击 = paramValue;
      }
      if (strstr(paramName.c_str(), "阔刀左蓄一段距离") != NULL) {
        振刀参数::距离::阔刀.左蓄一段 = paramValue;
      }
      if (strstr(paramName.c_str(), "阔刀左蓄二段距离") != NULL) {
        振刀参数::距离::阔刀.左蓄二段 = paramValue;
      }
      if (strstr(paramName.c_str(), "阔刀左蓄三段距离") != NULL) {
        振刀参数::距离::阔刀.左蓄三段 = paramValue;
      }
      if (strstr(paramName.c_str(), "阔刀右蓄距离") != NULL) {
        振刀参数::距离::阔刀.右蓄 = paramValue;
      }
      if (strstr(paramName.c_str(), "阔刀右右蓄距离") != NULL) {
        振刀参数::距离::阔刀.右右蓄 = paramValue;
      }
      if (strstr(paramName.c_str(), "阔刀雷刀距离") != NULL) {
        振刀参数::距离::阔刀.雷刀 = paramValue;
      }
      if (strstr(paramName.c_str(), "阔刀雷刀下劈距离") != NULL) {
        振刀参数::距离::阔刀.雷刀下劈 = paramValue;
      }
      if (strstr(paramName.c_str(), "阔刀同源距离") != NULL) {
        振刀参数::距离::阔刀.同源 = paramValue;
      }
      // 延迟
      if (strstr(paramName.c_str(), "阔刀左左延迟") != NULL) {
        振刀参数::延迟::阔刀.左左 = paramValue;
      }
      if (strstr(paramName.c_str(), "阔刀左右延迟") != NULL) {
        振刀参数::延迟::阔刀.左右 = paramValue;
      }
      if (strstr(paramName.c_str(), "阔刀右右延迟") != NULL) {
        振刀参数::延迟::阔刀.右右 = paramValue;
      }
      if (strstr(paramName.c_str(), "阔刀右左延迟") != NULL) {
        振刀参数::延迟::阔刀.右左 = paramValue;
      }
      if (strstr(paramName.c_str(), "阔刀左蓄一段延迟") != NULL) {
        振刀参数::延迟::阔刀.左蓄一段 = paramValue;
      }
      if (strstr(paramName.c_str(), "阔刀左蓄二段延迟") != NULL) {
        振刀参数::延迟::阔刀.左蓄二段 = paramValue;
      }
      if (strstr(paramName.c_str(), "阔刀左蓄三段延迟") != NULL) {
        振刀参数::延迟::阔刀.左蓄三段 = paramValue;
      }
      if (strstr(paramName.c_str(), "阔刀右蓄延迟") != NULL) {
        振刀参数::延迟::阔刀.右蓄 = paramValue;
      }
      if (strstr(paramName.c_str(), "阔刀右右蓄延迟") != NULL) {
        振刀参数::延迟::阔刀.右右蓄 = paramValue;
      }
      if (strstr(paramName.c_str(), "阔刀雷刀延迟") != NULL) {
        振刀参数::延迟::阔刀.雷刀 = paramValue;
      }
      if (strstr(paramName.c_str(), "阔刀壁击延迟") != NULL) {
        振刀参数::延迟::阔刀.壁击 = paramValue;
      }
      if (strstr(paramName.c_str(), "阔刀同源延迟") != NULL) {
        振刀参数::延迟::阔刀.同源 = paramValue;
      }
      if (strstr(paramName.c_str(), "阔刀雷刀下劈延迟") != NULL) {
        振刀参数::延迟::阔刀.雷刀下劈 = paramValue;
      }
      // 角度
      if (strstr(paramName.c_str(), "阔刀左左角度") != NULL) {
        振刀参数::角度::阔刀.左左 = paramValue;
      }
      if (strstr(paramName.c_str(), "阔刀左右角度") != NULL) {
        振刀参数::角度::阔刀.左右 = paramValue;
      }
      if (strstr(paramName.c_str(), "阔刀右右角度") != NULL) {
        振刀参数::角度::阔刀.右右 = paramValue;
      }
      if (strstr(paramName.c_str(), "阔刀右左角度") != NULL) {
        振刀参数::角度::阔刀.右左 = paramValue;
      }
      if (strstr(paramName.c_str(), "阔刀壁击角度") != NULL) {
        振刀参数::角度::阔刀.壁击 = paramValue;
      }
      if (strstr(paramName.c_str(), "阔刀同源角度") != NULL) {
        振刀参数::角度::阔刀.同源 = paramValue;
      }
      if (strstr(paramName.c_str(), "阔刀左蓄一段角度") != NULL) {
        振刀参数::角度::阔刀.左蓄一段 = paramValue;
      }
      if (strstr(paramName.c_str(), "阔刀左蓄二段角度") != NULL) {
        振刀参数::角度::阔刀.左蓄二段 = paramValue;
      }
      if (strstr(paramName.c_str(), "阔刀左蓄三段角度") != NULL) {
        振刀参数::角度::阔刀.左蓄三段 = paramValue;
      }
      if (strstr(paramName.c_str(), "阔刀右蓄角度") != NULL) {
        振刀参数::角度::阔刀.右蓄 = paramValue;
      }
      if (strstr(paramName.c_str(), "阔刀右右蓄角度") != NULL) {
        振刀参数::角度::阔刀.右右蓄 = paramValue;
      }
      // 长枪
      if (strstr(paramName.c_str(), "长枪左3距离") != NULL) {
        振刀参数::距离::长枪.左3 = paramValue;
      }
      if (strstr(paramName.c_str(), "长枪右3距离") != NULL) {
        振刀参数::距离::长枪.右3 = paramValue;
      }
      if (strstr(paramName.c_str(), "长枪左蓄距离") != NULL) {
        振刀参数::距离::长枪.左蓄 = paramValue;
      }
      if (strstr(paramName.c_str(), "长枪六合枪距离") != NULL) {
        振刀参数::距离::长枪.六合枪 = paramValue;
      }
      if (strstr(paramName.c_str(), "长枪右蓄距离") != NULL) {
        振刀参数::距离::长枪.右蓄 = paramValue;
      }
      if (strstr(paramName.c_str(), "长枪壁击距离") != NULL) {
        振刀参数::距离::长枪.壁击 = paramValue;
      }
      if (strstr(paramName.c_str(), "长枪同源距离") != NULL) {
        振刀参数::距离::长枪.同源 = paramValue;
      }
      if (strstr(paramName.c_str(), "长枪大圣游距离") != NULL) {
        振刀参数::距离::长枪.大圣游 = paramValue;
      }
      if (strstr(paramName.c_str(), "长枪风卷云残距离") != NULL) {
        振刀参数::距离::长枪.风卷云残 = paramValue;
      }
      if (strstr(paramName.c_str(), "长枪双环扫距离") != NULL) {
        振刀参数::距离::长枪.双环扫 = paramValue;
      }
      if (strstr(paramName.c_str(), "长枪龙王破距离") != NULL) {
        振刀参数::距离::长枪.龙王破 = paramValue;
      }
      if (strstr(paramName.c_str(), "长枪穿心脚距离") != NULL) {
        振刀参数::距离::长枪.穿心脚 = paramValue;
      }
      // 延迟
      if (strstr(paramName.c_str(), "长枪左3延迟") != NULL) {
        振刀参数::延迟::长枪.左3 = paramValue;
      }
      if (strstr(paramName.c_str(), "长枪右3延迟") != NULL) {
        振刀参数::延迟::长枪.右3 = paramValue;
      }
      if (strstr(paramName.c_str(), "长枪左蓄延迟") != NULL) {
        振刀参数::延迟::长枪.左蓄 = paramValue;
      }
      if (strstr(paramName.c_str(), "长枪六合枪延迟") != NULL) {
        振刀参数::延迟::长枪.六合枪 = paramValue;
      }
      if (strstr(paramName.c_str(), "长枪右蓄延迟") != NULL) {
        振刀参数::延迟::长枪.右蓄 = paramValue;
      }
      if (strstr(paramName.c_str(), "长枪大圣游延迟") != NULL) {
        振刀参数::延迟::长枪.大圣游 = paramValue;
      }
      if (strstr(paramName.c_str(), "长枪风卷云残延迟") != NULL) {
        振刀参数::延迟::长枪.风卷云残 = paramValue;
      }
      if (strstr(paramName.c_str(), "长枪双环扫延迟") != NULL) {
        振刀参数::延迟::长枪.双环扫 = paramValue;
      }
      if (strstr(paramName.c_str(), "长枪龙王破延迟") != NULL) {
        振刀参数::延迟::长枪.龙王破 = paramValue;
      }
      if (strstr(paramName.c_str(), "长枪穿心脚延迟") != NULL) {
        振刀参数::延迟::长枪.穿心脚 = paramValue;
      }
      if (strstr(paramName.c_str(), "长枪壁击延迟") != NULL) {
        振刀参数::延迟::长枪.壁击 = paramValue;
      }
      if (strstr(paramName.c_str(), "长枪同源延迟") != NULL) {
        振刀参数::延迟::长枪.同源 = paramValue;
      }
      // 角度
      if (strstr(paramName.c_str(), "长枪左3角度") != NULL) {
        振刀参数::角度::长枪.左3 = paramValue;
      }
      if (strstr(paramName.c_str(), "长枪右3角度") != NULL) {
        振刀参数::角度::长枪.右3 = paramValue;
      }
      if (strstr(paramName.c_str(), "长枪左蓄角度") != NULL) {
        振刀参数::角度::长枪.左蓄 = paramValue;
      }
      if (strstr(paramName.c_str(), "长枪六合枪角度") != NULL) {
        振刀参数::角度::长枪.六合枪 = paramValue;
      }
      if (strstr(paramName.c_str(), "长枪右蓄角度") != NULL) {
        振刀参数::角度::长枪.右蓄 = paramValue;
      }
      if (strstr(paramName.c_str(), "长枪壁击角度") != NULL) {
        振刀参数::角度::长枪.壁击 = paramValue;
      }
      if (strstr(paramName.c_str(), "长枪同源角度") != NULL) {
        振刀参数::角度::长枪.同源 = paramValue;
      }
      if (strstr(paramName.c_str(), "长枪双环扫角度") != NULL) {
        振刀参数::角度::长枪.双环扫 = paramValue;
      }
      if (strstr(paramName.c_str(), "长枪大圣游角度") != NULL) {
        振刀参数::角度::长枪.大圣游 = paramValue;
      }
      if (strstr(paramName.c_str(), "长枪风卷云残角度") != NULL) {
        振刀参数::角度::长枪.风卷云残 = paramValue;
      }
      if (strstr(paramName.c_str(), "长枪龙王破角度") != NULL) {
        振刀参数::角度::长枪.龙王破 = paramValue;
      }
      if (strstr(paramName.c_str(), "长枪穿心脚角度") != NULL) {
        振刀参数::角度::长枪.穿心脚 = paramValue;
      }
      // 匕首
      if (strstr(paramName.c_str(), "匕首左3距离") != NULL) {
        振刀参数::距离::匕首.左3 = paramValue;
      }
      if (strstr(paramName.c_str(), "匕首右3距离") != NULL) {
        振刀参数::距离::匕首.右3 = paramValue;
      }
      if (strstr(paramName.c_str(), "匕首左蓄距离") != NULL) {
        振刀参数::距离::匕首.左蓄 = paramValue;
      }
      if (strstr(paramName.c_str(), "匕首右蓄距离") != NULL) {
        振刀参数::距离::匕首.右蓄 = paramValue;
      }
      if (strstr(paramName.c_str(), "匕首鬼反距离") != NULL) {
        振刀参数::距离::匕首.鬼反 = paramValue;
      }
      if (strstr(paramName.c_str(), "匕首壁击距离") != NULL) {
        振刀参数::距离::匕首.壁击 = paramValue;
      }
      if (strstr(paramName.c_str(), "匕首荆轲献匕距离") != NULL) {
        振刀参数::距离::匕首.荆轲献匕 = paramValue;
      }
      if (strstr(paramName.c_str(), "匕首鬼刃暗扎距离") != NULL) {
        振刀参数::距离::匕首.鬼刃暗扎 = paramValue;
      }
      if (strstr(paramName.c_str(), "匕首鬼哭神嚎距离") != NULL) {
        振刀参数::距离::匕首.鬼哭神嚎 = paramValue;
      }
      if (strstr(paramName.c_str(), "匕首亢龙有悔距离") != NULL) {
        振刀参数::距离::匕首.亢龙有悔 = paramValue;
      }
      // 延迟
      if (strstr(paramName.c_str(), "匕首左3延迟") != NULL) {
        振刀参数::延迟::匕首.左3 = paramValue;
      }
      if (strstr(paramName.c_str(), "匕首右3延迟") != NULL) {
        振刀参数::延迟::匕首.右3 = paramValue;
      }
      if (strstr(paramName.c_str(), "匕首左蓄延迟") != NULL) {
        振刀参数::延迟::匕首.左蓄 = paramValue;
      }
      if (strstr(paramName.c_str(), "匕首右蓄延迟") != NULL) {
        振刀参数::延迟::匕首.右蓄 = paramValue;
      }
      if (strstr(paramName.c_str(), "匕首鬼反延迟") != NULL) {
        振刀参数::延迟::匕首.鬼反 = paramValue;
      }
      if (strstr(paramName.c_str(), "匕首壁击延迟") != NULL) {
        振刀参数::延迟::匕首.壁击 = paramValue;
      }
      if (strstr(paramName.c_str(), "匕首荆轲献匕延迟") != NULL) {
        振刀参数::延迟::匕首.荆轲献匕 = paramValue;
      }
      if (strstr(paramName.c_str(), "匕首鬼刃暗扎延迟") != NULL) {
        振刀参数::延迟::匕首.鬼刃暗扎 = paramValue;
      }
      if (strstr(paramName.c_str(), "匕首鬼哭神嚎延迟") != NULL) {
        振刀参数::延迟::匕首.鬼哭神嚎 = paramValue;
      }
      if (strstr(paramName.c_str(), "匕首亢龙有悔延迟") != NULL) {
        振刀参数::延迟::匕首.亢龙有悔 = paramValue;
      }
      // 角度
      if (strstr(paramName.c_str(), "匕首左3角度") != NULL) {
        振刀参数::角度::匕首.左3 = paramValue;
      }
      if (strstr(paramName.c_str(), "匕首右3角度") != NULL) {
        振刀参数::角度::匕首.右3 = paramValue;
      }
      if (strstr(paramName.c_str(), "匕首左蓄角度") != NULL) {
        振刀参数::角度::匕首.左蓄 = paramValue;
      }
      if (strstr(paramName.c_str(), "匕首右蓄角度") != NULL) {
        振刀参数::角度::匕首.右蓄 = paramValue;
      }
      if (strstr(paramName.c_str(), "匕首鬼反角度") != NULL) {
        振刀参数::角度::匕首.鬼反 = paramValue;
      }
      if (strstr(paramName.c_str(), "匕首壁击角度") != NULL) {
        振刀参数::角度::匕首.壁击 = paramValue;
      }
      if (strstr(paramName.c_str(), "匕首荆轲献匕角度") != NULL) {
        振刀参数::角度::匕首.荆轲献匕 = paramValue;
      }
      if (strstr(paramName.c_str(), "匕首鬼刃暗扎角度") != NULL) {
        振刀参数::角度::匕首.鬼刃暗扎 = paramValue;
      }
      if (strstr(paramName.c_str(), "匕首亢龙有悔角度") != NULL) {
        振刀参数::角度::匕首.亢龙有悔 = paramValue;
      }
      if (strstr(paramName.c_str(), "匕首鬼哭神嚎角度") != NULL) {
        振刀参数::角度::匕首.鬼哭神嚎 = paramValue;
      }
      // 双截棍
      if (strstr(paramName.c_str(), "双截棍左右3距离") != NULL) {
        振刀参数::距离::双截棍.左右3 = paramValue;
      }
      if (strstr(paramName.c_str(), "双截棍左蓄距离") != NULL) {
        振刀参数::距离::双截棍.左蓄 = paramValue;
      }
      if (strstr(paramName.c_str(), "双截棍右蓄距离") != NULL) {
        振刀参数::距离::双截棍.右蓄 = paramValue;
      }
      if (strstr(paramName.c_str(), "双截棍壁击距离") != NULL) {
        振刀参数::距离::双截棍.壁击 = paramValue;
      }
      if (strstr(paramName.c_str(), "双截棍飞踢距离") != NULL) {
        振刀参数::距离::双截棍.飞踢 = paramValue;
      }
      if (strstr(paramName.c_str(), "双截棍扬鞭劲距离") != NULL) {
        振刀参数::距离::双截棍.扬鞭劲 = paramValue;
      }
      if (strstr(paramName.c_str(), "双截棍横栏距离") != NULL) {
        振刀参数::距离::双截棍.横栏 = paramValue;
      }
      if (strstr(paramName.c_str(), "双截棍龙虎乱舞距离") != NULL) {
        振刀参数::距离::双截棍.龙虎乱舞 = paramValue;
      }
      // 延迟
      if (strstr(paramName.c_str(), "双截棍左右3延迟") != NULL) {
        振刀参数::延迟::双截棍.左右3 = paramValue;
      }
      if (strstr(paramName.c_str(), "双截棍左蓄延迟") != NULL) {
        振刀参数::延迟::双截棍.左蓄 = paramValue;
      }
      if (strstr(paramName.c_str(), "双截棍右蓄延迟") != NULL) {
        振刀参数::延迟::双截棍.右蓄 = paramValue;
      }
      if (strstr(paramName.c_str(), "双截棍飞踢延迟") != NULL) {
        振刀参数::延迟::双截棍.飞踢 = paramValue;
      }
      if (strstr(paramName.c_str(), "双截棍扬鞭劲延迟") != NULL) {
        振刀参数::延迟::双截棍.扬鞭劲 = paramValue;
      }
      if (strstr(paramName.c_str(), "双截棍横栏延迟") != NULL) {
        振刀参数::延迟::双截棍.横栏 = paramValue;
      }
      if (strstr(paramName.c_str(), "双截棍壁击延迟") != NULL) {
        振刀参数::延迟::双截棍.壁击 = paramValue;
      }
      if (strstr(paramName.c_str(), "双截棍龙虎乱舞延迟") != NULL) {
        振刀参数::延迟::双截棍.龙虎乱舞 = paramValue;
      }
      // 角度
      if (strstr(paramName.c_str(), "双截棍左右3角度") != NULL) {
        振刀参数::角度::双截棍.左右3 = paramValue;
      }
      if (strstr(paramName.c_str(), "双截棍左蓄角度") != NULL) {
        振刀参数::角度::双截棍.左蓄 = paramValue;
      }
      if (strstr(paramName.c_str(), "双截棍右蓄角度") != NULL) {
        振刀参数::角度::双截棍.右蓄 = paramValue;
      }
      if (strstr(paramName.c_str(), "双截棍飞踢角度") != NULL) {
        振刀参数::角度::双截棍.飞踢 = paramValue;
      }
      if (strstr(paramName.c_str(), "双截棍壁击角度") != NULL) {
        振刀参数::角度::双截棍.壁击 = paramValue;
      }
      if (strstr(paramName.c_str(), "双截棍扬鞭劲角度") != NULL) {
        振刀参数::角度::双截棍.扬鞭劲 = paramValue;
      }
      if (strstr(paramName.c_str(), "双截棍横栏角度") != NULL) {
        振刀参数::角度::双截棍.横栏 = paramValue;
      }
      if (strstr(paramName.c_str(), "双截棍龙虎乱舞角度") != NULL) {
        振刀参数::角度::双截棍.龙虎乱舞 = paramValue;
      }
      // 双刀
      if (strstr(paramName.c_str(), "双刀左右3距离") != NULL) {
        振刀参数::距离::双刀.左右3 = paramValue;
      }
      if (strstr(paramName.c_str(), "双刀左蓄距离") != NULL) {
        振刀参数::距离::双刀.左蓄 = paramValue;
      }
      if (strstr(paramName.c_str(), "双刀右蓄距离") != NULL) {
        振刀参数::距离::双刀.右蓄 = paramValue;
      }
      if (strstr(paramName.c_str(), "双刀壁击距离") != NULL) {
        振刀参数::距离::双刀.壁击 = paramValue;
      }
      if (strstr(paramName.c_str(), "双刀铁马残红距离") != NULL) {
        振刀参数::距离::双刀.铁马残红 = paramValue;
      }
      if (strstr(paramName.c_str(), "双刀乾坤日月斩距离") != NULL) {
        振刀参数::距离::双刀.乾坤日月斩 = paramValue;
      }
      if (strstr(paramName.c_str(), "双刀惊雷距离") != NULL) {
        振刀参数::距离::双刀.惊雷 = paramValue;
      }
      if (strstr(paramName.c_str(), "双刀八斩刀距离") != NULL) {
        振刀参数::距离::双刀.八斩刀 = paramValue;
      }
      // 延迟
      if (strstr(paramName.c_str(), "双刀左右3延迟") != NULL) {
        振刀参数::延迟::双刀.左右3 = paramValue;
      }
      if (strstr(paramName.c_str(), "双刀左蓄延迟") != NULL) {
        振刀参数::延迟::双刀.左蓄 = paramValue;
      }
      if (strstr(paramName.c_str(), "双刀右蓄延迟") != NULL) {
        振刀参数::延迟::双刀.右蓄 = paramValue;
      }
      if (strstr(paramName.c_str(), "双刀铁马残红延迟") != NULL) {
        振刀参数::延迟::双刀.铁马残红 = paramValue;
      }
      if (strstr(paramName.c_str(), "双刀乾坤日月斩延迟") != NULL) {
        振刀参数::延迟::双刀.乾坤日月斩 = paramValue;
      }
      if (strstr(paramName.c_str(), "双刀惊雷延迟") != NULL) {
        振刀参数::延迟::双刀.惊雷 = paramValue;
      }
      if (strstr(paramName.c_str(), "双刀壁击延迟") != NULL) {
        振刀参数::延迟::双刀.壁击 = paramValue;
      }
      if (strstr(paramName.c_str(), "双刀八斩刀延迟") != NULL) {
        振刀参数::延迟::双刀.八斩刀 = paramValue;
      }
      // 角度
      if (strstr(paramName.c_str(), "双刀左右3角度") != NULL) {
        振刀参数::角度::双刀.左右3 = paramValue;
      }
      if (strstr(paramName.c_str(), "双刀左蓄角度") != NULL) {
        振刀参数::角度::双刀.左蓄 = paramValue;
      }
      if (strstr(paramName.c_str(), "双刀右蓄角度") != NULL) {
        振刀参数::角度::双刀.右蓄 = paramValue;
      }
      if (strstr(paramName.c_str(), "双刀壁击角度") != NULL) {
        振刀参数::角度::双刀.壁击 = paramValue;
      }
      if (strstr(paramName.c_str(), "双刀铁马残红角度") != NULL) {
        振刀参数::角度::双刀.铁马残红 = paramValue;
      }
      if (strstr(paramName.c_str(), "双刀乾坤日月斩角度") != NULL) {
        振刀参数::角度::双刀.乾坤日月斩 = paramValue;
      }
      if (strstr(paramName.c_str(), "双刀惊雷角度") != NULL) {
        振刀参数::角度::双刀.惊雷 = paramValue;
      }
      if (strstr(paramName.c_str(), "双刀八斩刀角度") != NULL) {
        振刀参数::角度::双刀.八斩刀 = paramValue;
      }
      // 长棍
      if (strstr(paramName.c_str(), "长棍左3距离") != NULL) {
        振刀参数::距离::长棍.左3 = paramValue;
      }
      if (strstr(paramName.c_str(), "长棍右3距离") != NULL) {
        振刀参数::距离::长棍.右3 = paramValue;
      }
      if (strstr(paramName.c_str(), "长棍左蓄距离") != NULL) {
        振刀参数::距离::长棍.左蓄 = paramValue;
      }
      if (strstr(paramName.c_str(), "长棍右蓄距离") != NULL) {
        振刀参数::距离::长棍.右蓄 = paramValue;
      }
      if (strstr(paramName.c_str(), "长棍壁击距离") != NULL) {
        振刀参数::距离::长棍.壁击 = paramValue;
      }
      if (strstr(paramName.c_str(), "长棍同源距离") != NULL) {
        振刀参数::距离::长棍.同源 = paramValue;
      }
      if (strstr(paramName.c_str(), "长棍腾云式距离") != NULL) {
        振刀参数::距离::长棍.腾云式 = paramValue;
      }
      if (strstr(paramName.c_str(), "长棍乱点天宫距离") != NULL) {
        振刀参数::距离::长棍.乱点天宫 = paramValue;
      }
      if (strstr(paramName.c_str(), "长棍桶劲距离") != NULL) {
        振刀参数::距离::长棍.桶劲 = paramValue;
      }
      if (strstr(paramName.c_str(), "长棍五情七灭镇距离") != NULL) {
        振刀参数::距离::长棍.五情七灭镇 = paramValue;
      }
      if (strstr(paramName.c_str(), "长棍双环扫距离") != NULL) {
        振刀参数::距离::长棍.双环扫 = paramValue;
      }
      if (strstr(paramName.c_str(), "长棍倒海棍距离") != NULL) {
        振刀参数::距离::长棍.少林棍 = paramValue;
      }
      if (strstr(paramName.c_str(), "长棍镇地撑天距离") != NULL) {
        振刀参数::距离::长棍.镇地撑天 = paramValue;
      }
      // 延迟
      if (strstr(paramName.c_str(), "长棍左3延迟") != NULL) {
        振刀参数::延迟::长棍.左3 = paramValue;
      }
      if (strstr(paramName.c_str(), "长棍右3延迟") != NULL) {
        振刀参数::延迟::长棍.右3 = paramValue;
      }
      if (strstr(paramName.c_str(), "长棍少林棍延迟") != NULL) {
        振刀参数::延迟::长棍.少林棍 = paramValue;
      }
      if (strstr(paramName.c_str(), "长棍左蓄延迟") != NULL) {
        振刀参数::延迟::长棍.左蓄 = paramValue;
      }
      if (strstr(paramName.c_str(), "长棍右蓄延迟") != NULL) {
        振刀参数::延迟::长棍.右蓄 = paramValue;
      }
      if (strstr(paramName.c_str(), "长棍腾云式延迟") != NULL) {
        振刀参数::延迟::长棍.腾云式 = paramValue;
      }
      if (strstr(paramName.c_str(), "长棍乱点天宫延迟") != NULL) {
        振刀参数::延迟::长棍.乱点天宫 = paramValue;
      }
      if (strstr(paramName.c_str(), "长棍桶劲延迟") != NULL) {
        振刀参数::延迟::长棍.桶劲 = paramValue;
      }
      if (strstr(paramName.c_str(), "长棍壁击延迟") != NULL) {
        振刀参数::延迟::长棍.壁击 = paramValue;
      }
      if (strstr(paramName.c_str(), "长棍同源延迟") != NULL) {
        振刀参数::延迟::长棍.同源 = paramValue;
      }
      if (strstr(paramName.c_str(), "长棍五情七灭镇延迟") != NULL) {
        振刀参数::延迟::长棍.五情七灭镇 = paramValue;
      }
      if (strstr(paramName.c_str(), "长棍双环扫延迟") != NULL) {
        振刀参数::延迟::长棍.双环扫 = paramValue;
      }
      if (strstr(paramName.c_str(), "长棍倒海棍延迟") != NULL) {
        振刀参数::延迟::长棍.倒海棍 = paramValue;
      }
      if (strstr(paramName.c_str(), "长棍镇地撑天延迟") != NULL) {
        振刀参数::延迟::长棍.镇地撑天 = paramValue;
      }
      // 角度
      if (strstr(paramName.c_str(), "长棍左3角度") != NULL) {
        振刀参数::角度::长棍.左3 = paramValue;
      }
      if (strstr(paramName.c_str(), "长棍右3角度") != NULL) {
        振刀参数::角度::长棍.右3 = paramValue;
      }
      if (strstr(paramName.c_str(), "长棍左蓄角度") != NULL) {
        振刀参数::角度::长棍.左蓄 = paramValue;
      }
      if (strstr(paramName.c_str(), "长棍右蓄角度") != NULL) {
        振刀参数::角度::长棍.右蓄 = paramValue;
      }
      if (strstr(paramName.c_str(), "长棍壁击角度") != NULL) {
        振刀参数::角度::长棍.壁击 = paramValue;
      }
      if (strstr(paramName.c_str(), "长棍同源角度") != NULL) {
        振刀参数::角度::长棍.同源 = paramValue;
      }
      if (strstr(paramName.c_str(), "长棍腾云式角度") != NULL) {
        振刀参数::角度::长棍.腾云式 = paramValue;
      }
      if (strstr(paramName.c_str(), "长棍乱点天宫角度") != NULL) {
        振刀参数::角度::长棍.乱点天宫 = paramValue;
      }
      if (strstr(paramName.c_str(), "长棍桶劲角度") != NULL) {
        振刀参数::角度::长棍.桶劲 = paramValue;
      }
      if (strstr(paramName.c_str(), "长棍五情七灭镇角度") != NULL) {
        振刀参数::角度::长棍.五情七灭镇 = paramValue;
      }
      if (strstr(paramName.c_str(), "长棍双环扫角度") != NULL) {
        振刀参数::角度::长棍.双环扫 = paramValue;
      }
      if (strstr(paramName.c_str(), "长棍倒海棍角度") != NULL) {
        振刀参数::角度::长棍.少林棍 = paramValue;
      }
      if (strstr(paramName.c_str(), "长棍镇地撑天角度") != NULL) {
        振刀参数::角度::长棍.镇地撑天 = paramValue;
      }
      // 斩马刀
      if (strstr(paramName.c_str(), "斩马刀左左距离") != NULL) {
        振刀参数::距离::斩马刀.左左 = paramValue;
      }
      if (strstr(paramName.c_str(), "斩马刀左右距离") != NULL) {
        振刀参数::距离::斩马刀.左右 = paramValue;
      }
      if (strstr(paramName.c_str(), "斩马刀右右距离") != NULL) {
        振刀参数::距离::斩马刀.右右 = paramValue;
      }
      if (strstr(paramName.c_str(), "斩马刀右左距离") != NULL) {
        振刀参数::距离::斩马刀.右左 = paramValue;
      }
      if (strstr(paramName.c_str(), "斩马刀左蓄距离") != NULL) {
        振刀参数::距离::斩马刀.左蓄 = paramValue;
      }
      if (strstr(paramName.c_str(), "斩马刀右蓄距离") != NULL) {
        振刀参数::距离::斩马刀.右蓄 = paramValue;
      }
      if (strstr(paramName.c_str(), "斩马刀壁击距离") != NULL) {
        振刀参数::距离::斩马刀.壁击 = paramValue;
      }
      if (strstr(paramName.c_str(), "斩马刀柄击距离") != NULL) {
        振刀参数::距离::斩马刀.柄击 = paramValue;
      }
      if (strstr(paramName.c_str(), "斩马刀惊雷距离") != NULL) {
        振刀参数::距离::斩马刀.惊雷 = paramValue;
      }
      if (strstr(paramName.c_str(), "斩马刀同源距离") != NULL) {
        振刀参数::距离::斩马刀.同源 = paramValue;
      }
      if (strstr(paramName.c_str(), "斩马刀炽焰斩距离") != NULL) {
        振刀参数::距离::斩马刀.炽焰斩 = paramValue;
      }
      if (strstr(paramName.c_str(), "斩马刀过关斩将距离") != NULL) {
        振刀参数::距离::斩马刀.过关斩将 = paramValue;
      }
      // 延迟
      if (strstr(paramName.c_str(), "斩马刀左左延迟") != NULL) {
        振刀参数::延迟::斩马刀.左左 = paramValue;
      }
      if (strstr(paramName.c_str(), "斩马刀左右延迟") != NULL) {
        振刀参数::延迟::斩马刀.左右 = paramValue;
      }
      if (strstr(paramName.c_str(), "斩马刀右右延迟") != NULL) {
        振刀参数::延迟::斩马刀.右右 = paramValue;
      }
      if (strstr(paramName.c_str(), "斩马刀右左延迟") != NULL) {
        振刀参数::延迟::斩马刀.右左 = paramValue;
      }
      if (strstr(paramName.c_str(), "斩马刀左蓄延迟") != NULL) {
        振刀参数::延迟::斩马刀.左蓄 = paramValue;
      }
      if (strstr(paramName.c_str(), "斩马刀右蓄延迟") != NULL) {
        振刀参数::延迟::斩马刀.右蓄 = paramValue;
      }
      if (strstr(paramName.c_str(), "斩马刀柄击延迟") != NULL) {
        振刀参数::延迟::斩马刀.柄击 = paramValue;
      }
      if (strstr(paramName.c_str(), "斩马刀壁击延迟") != NULL) {
        振刀参数::延迟::斩马刀.壁击 = paramValue;
      }
      if (strstr(paramName.c_str(), "斩马刀惊雷延迟") != NULL) {
        振刀参数::延迟::斩马刀.惊雷 = paramValue;
      }
      if (strstr(paramName.c_str(), "斩马刀同源延迟") != NULL) {
        振刀参数::延迟::斩马刀.同源 = paramValue;
      }
      if (strstr(paramName.c_str(), "斩马刀炽焰斩延迟") != NULL) {
        振刀参数::延迟::斩马刀.炽焰斩 = paramValue;
      }
      if (strstr(paramName.c_str(), "斩马刀过关斩将延迟") != NULL) {
        振刀参数::延迟::斩马刀.过关斩将 = paramValue;
      }
      // 角度
      if (strstr(paramName.c_str(), "斩马刀左左角度") != NULL) {
        振刀参数::角度::斩马刀.左左 = paramValue;
      }
      if (strstr(paramName.c_str(), "斩马刀左右角度") != NULL) {
        振刀参数::角度::斩马刀.左右 = paramValue;
      }
      if (strstr(paramName.c_str(), "斩马刀右右角度") != NULL) {
        振刀参数::角度::斩马刀.右右 = paramValue;
      }
      if (strstr(paramName.c_str(), "斩马刀右左角度") != NULL) {
        振刀参数::角度::斩马刀.右左 = paramValue;
      }
      if (strstr(paramName.c_str(), "斩马刀左蓄角度") != NULL) {
        振刀参数::角度::斩马刀.左蓄 = paramValue;
      }
      if (strstr(paramName.c_str(), "斩马刀右蓄角度") != NULL) {
        振刀参数::角度::斩马刀.右蓄 = paramValue;
      }
      if (strstr(paramName.c_str(), "斩马刀柄击角度") != NULL) {
        振刀参数::角度::斩马刀.柄击 = paramValue;
      }
      if (strstr(paramName.c_str(), "斩马刀壁击角度") != NULL) {
        振刀参数::角度::斩马刀.壁击 = paramValue;
      }
      if (strstr(paramName.c_str(), "斩马刀惊雷角度") != NULL) {
        振刀参数::角度::斩马刀.惊雷 = paramValue;
      }
      if (strstr(paramName.c_str(), "斩马刀同源角度") != NULL) {
        振刀参数::角度::斩马刀.同源 = paramValue;
      }
      if (strstr(paramName.c_str(), "斩马刀炽焰斩角度") != NULL) {
        振刀参数::角度::斩马刀.炽焰斩 = paramValue;
      }
      if (strstr(paramName.c_str(), "斩马刀过关斩将角度") != NULL) {
        振刀参数::角度::斩马刀.过关斩将 = paramValue;
      }
      // 双戟
      if (strstr(paramName.c_str(), "双戟左右3距离") != NULL) {
        振刀参数::距离::双戟.左右3 = paramValue;
      }
      if (strstr(paramName.c_str(), "双戟左蓄距离") != NULL) {
        振刀参数::距离::双戟.左蓄 = paramValue;
      }
      if (strstr(paramName.c_str(), "双戟右蓄距离") != NULL) {
        振刀参数::距离::双戟.右蓄 = paramValue;
      }
      if (strstr(paramName.c_str(), "双戟壁击距离") != NULL) {
        振刀参数::距离::双戟.壁击 = paramValue;
      }
      if (strstr(paramName.c_str(), "双戟勾旋斩距离") != NULL) {
        振刀参数::距离::双戟.勾旋斩 = paramValue;
      }
      if (strstr(paramName.c_str(), "双戟探海蛟距离") != NULL) {
        振刀参数::距离::双戟.探海蛟 = paramValue;
      }
      if (strstr(paramName.c_str(), "双戟钩挂距离") != NULL) {
        振刀参数::距离::双戟.钩挂 = paramValue;
      }
      if (strstr(paramName.c_str(), "双戟战龙在天距离") != NULL) {
        振刀参数::距离::双戟.战龙在天 = paramValue;
      }
      if (strstr(paramName.c_str(), "双戟形意钩距离") != NULL) {
        振刀参数::距离::双戟.形意钩 = paramValue;
      }
      // 延迟
      if (strstr(paramName.c_str(), "双戟左右3延迟") != NULL) {
        振刀参数::延迟::双戟.左右3 = paramValue;
      }
      if (strstr(paramName.c_str(), "双戟左蓄延迟") != NULL) {
        振刀参数::延迟::双戟.左蓄 = paramValue;
      }
      if (strstr(paramName.c_str(), "双戟右蓄延迟") != NULL) {
        振刀参数::延迟::双戟.右蓄 = paramValue;
      }
      if (strstr(paramName.c_str(), "双戟勾旋斩延迟") != NULL) {
        振刀参数::延迟::双戟.勾旋斩 = paramValue;
      }
      if (strstr(paramName.c_str(), "双戟探海蛟延迟") != NULL) {
        振刀参数::延迟::双戟.探海蛟 = paramValue;
      }
      if (strstr(paramName.c_str(), "双戟钩挂延迟") != NULL) {
        振刀参数::延迟::双戟.钩挂 = paramValue;
      }
      if (strstr(paramName.c_str(), "双戟壁击延迟") != NULL) {
        振刀参数::延迟::双戟.壁击 = paramValue;
      }
      if (strstr(paramName.c_str(), "双戟战龙在天延迟") != NULL) {
        振刀参数::延迟::双戟.战龙在天 = paramValue;
      }
      if (strstr(paramName.c_str(), "双戟形意钩延迟") != NULL) {
        振刀参数::延迟::双戟.形意钩 = paramValue;
      }
      // 角度
      if (strstr(paramName.c_str(), "双戟左右3角度") != NULL) {
        振刀参数::角度::双戟.左右3 = paramValue;
      }
      if (strstr(paramName.c_str(), "双戟左蓄角度") != NULL) {
        振刀参数::角度::双戟.左蓄 = paramValue;
      }
      if (strstr(paramName.c_str(), "双戟壁击角度") != NULL) {
        振刀参数::角度::双戟.壁击 = paramValue;
      }
      if (strstr(paramName.c_str(), "双戟右蓄角度") != NULL) {
        振刀参数::角度::双戟.右蓄 = paramValue;
      }
      if (strstr(paramName.c_str(), "双戟勾旋斩角度") != NULL) {
        振刀参数::角度::双戟.勾旋斩 = paramValue;
      }
      if (strstr(paramName.c_str(), "双戟探海蛟角度") != NULL) {
        振刀参数::角度::双戟.探海蛟 = paramValue;
      }
      if (strstr(paramName.c_str(), "双戟钩挂角度") != NULL) {
        振刀参数::角度::双戟.钩挂 = paramValue;
      }
      if (strstr(paramName.c_str(), "双戟战龙在天角度") != NULL) {
        振刀参数::角度::双戟.战龙在天 = paramValue;
      }
      if (strstr(paramName.c_str(), "双戟形意钩角度") != NULL) {
        振刀参数::角度::双戟.形意钩 = paramValue;
      }
      // 扇子
      if (strstr(paramName.c_str(), "扇子左3距离") != NULL) {
        振刀参数::距离::扇子.左3 = paramValue;
      }
      if (strstr(paramName.c_str(), "扇子右3距离") != NULL) {
        振刀参数::距离::扇子.右3 = paramValue;
      }
      if (strstr(paramName.c_str(), "扇子左蓄距离") != NULL) {
        振刀参数::距离::扇子.左蓄 = paramValue;
      }
      if (strstr(paramName.c_str(), "扇子右蓄距离") != NULL) {
        振刀参数::距离::扇子.右蓄 = paramValue;
      }
      if (strstr(paramName.c_str(), "扇子壁击距离") != NULL) {
        振刀参数::距离::扇子.壁击 = paramValue;
      }
      if (strstr(paramName.c_str(), "扇子双开圆距离") != NULL) {
        振刀参数::距离::扇子.双开圆 = paramValue;
      }
      if (strstr(paramName.c_str(), "扇子三风摆距离") != NULL) {
        振刀参数::距离::扇子.三风摆 = paramValue;
      }
      if (strstr(paramName.c_str(), "扇子惊雷距离") != NULL) {
        振刀参数::距离::扇子.惊雷 = paramValue;
      }
      if (strstr(paramName.c_str(), "扇子缠龙奔野距离") != NULL) {
        振刀参数::距离::扇子.缠龙奔野 = paramValue;
      }
      if (strstr(paramName.c_str(), "扇子鬼反距离") != NULL) {
        振刀参数::距离::扇子.鬼反 = paramValue;
      }
      // 延迟
      if (strstr(paramName.c_str(), "扇子左3延迟") != NULL) {
        振刀参数::延迟::扇子.左3 = paramValue;
      }
      if (strstr(paramName.c_str(), "扇子右3延迟") != NULL) {
        振刀参数::延迟::扇子.右3 = paramValue;
      }
      if (strstr(paramName.c_str(), "扇子左蓄延迟") != NULL) {
        振刀参数::延迟::扇子.左蓄 = paramValue;
      }
      if (strstr(paramName.c_str(), "扇子右蓄延迟") != NULL) {
        振刀参数::延迟::扇子.右蓄 = paramValue;
      }
      if (strstr(paramName.c_str(), "扇子双开圆延迟") != NULL) {
        振刀参数::延迟::扇子.双开圆 = paramValue;
      }
      if (strstr(paramName.c_str(), "扇子三风摆延迟") != NULL) {
        振刀参数::延迟::扇子.三风摆 = paramValue;
      }
      if (strstr(paramName.c_str(), "扇子惊雷延迟") != NULL) {
        振刀参数::延迟::扇子.惊雷 = paramValue;
      }
      if (strstr(paramName.c_str(), "扇子壁击延迟") != NULL) {
        振刀参数::延迟::扇子.壁击 = paramValue;
      }
      if (strstr(paramName.c_str(), "扇子缠龙奔野延迟") != NULL) {
        振刀参数::延迟::扇子.缠龙奔野 = paramValue;
      }
      if (strstr(paramName.c_str(), "扇子鬼反延迟") != NULL) {
        振刀参数::延迟::扇子.鬼反 = paramValue;
      }
      // 角度
      if (strstr(paramName.c_str(), "扇子左3角度") != NULL) {
        振刀参数::角度::扇子.左3 = paramValue;
      }
      if (strstr(paramName.c_str(), "扇子右3角度") != NULL) {
        振刀参数::角度::扇子.右3 = paramValue;
      }
      if (strstr(paramName.c_str(), "扇子左蓄角度") != NULL) {
        振刀参数::角度::扇子.左蓄 = paramValue;
      }
      if (strstr(paramName.c_str(), "扇子右蓄角度") != NULL) {
        振刀参数::角度::扇子.右蓄 = paramValue;
      }
      if (strstr(paramName.c_str(), "扇子壁击角度") != NULL) {
        振刀参数::角度::扇子.壁击 = paramValue;
      }
      if (strstr(paramName.c_str(), "扇子双开圆角度") != NULL) {
        振刀参数::角度::扇子.双开圆 = paramValue;
      }
      if (strstr(paramName.c_str(), "扇子三风摆角度") != NULL) {
        振刀参数::角度::扇子.三风摆 = paramValue;
      }
      if (strstr(paramName.c_str(), "扇子惊雷角度") != NULL) {
        振刀参数::角度::扇子.惊雷 = paramValue;
      }
      if (strstr(paramName.c_str(), "扇子缠龙奔野角度") != NULL) {
        振刀参数::角度::扇子.缠龙奔野 = paramValue;
      }
      if (strstr(paramName.c_str(), "扇子鬼反角度") != NULL) {
        振刀参数::角度::扇子.鬼反 = paramValue;
      }
      // 横刀
      if (strstr(paramName.c_str(), "横刀左3距离") != NULL) {
        振刀参数::距离::横刀.左3 = paramValue;
      }
      if (strstr(paramName.c_str(), "横刀右3距离") != NULL) {
        振刀参数::距离::横刀.右3 = paramValue;
      }
      if (strstr(paramName.c_str(), "横刀左蓄距离") != NULL) {
        振刀参数::距离::横刀.左蓄 = paramValue;
      }
      if (strstr(paramName.c_str(), "横刀右蓄距离") != NULL) {
        振刀参数::距离::横刀.右蓄 = paramValue;
      }
      if (strstr(paramName.c_str(), "横刀壁击距离") != NULL) {
        振刀参数::距离::横刀.壁击 = paramValue;
      }
      if (strstr(paramName.c_str(), "横刀苍牙距离") != NULL) {
        振刀参数::距离::横刀.苍牙 = paramValue;
      }
      if (strstr(paramName.c_str(), "横刀同源距离") != NULL) {
        振刀参数::距离::横刀.同源 = paramValue;
      }
      if (strstr(paramName.c_str(), "横刀踏空闪距离") != NULL) {
        振刀参数::距离::横刀.踏空闪 = paramValue;
      }
      if (strstr(paramName.c_str(), "横刀破千军距离") != NULL) {
        振刀参数::距离::横刀.破千军 = paramValue;
      }
      if (strstr(paramName.c_str(), "横刀极光破云闪距离") != NULL) {
        振刀参数::距离::横刀.极光破云闪 = paramValue;
      }
      // 延迟
      if (strstr(paramName.c_str(), "横刀左3延迟") != NULL) {
        振刀参数::延迟::横刀.左3 = paramValue;
      }
      if (strstr(paramName.c_str(), "横刀右3延迟") != NULL) {
        振刀参数::延迟::横刀.右3 = paramValue;
      }
      if (strstr(paramName.c_str(), "横刀左蓄延迟") != NULL) {
        振刀参数::延迟::横刀.左蓄 = paramValue;
      }
      if (strstr(paramName.c_str(), "横刀右蓄延迟") != NULL) {
        振刀参数::延迟::横刀.右蓄 = paramValue;
      }
      if (strstr(paramName.c_str(), "横刀壁击延迟") != NULL) {
        振刀参数::延迟::横刀.壁击 = paramValue;
      }
      if (strstr(paramName.c_str(), "横刀苍牙延迟") != NULL) {
        振刀参数::延迟::横刀.苍牙 = paramValue;
      }
      if (strstr(paramName.c_str(), "横刀同源延迟") != NULL) {
        振刀参数::延迟::横刀.同源 = paramValue;
      }
      if (strstr(paramName.c_str(), "横刀踏空闪延迟") != NULL) {
        振刀参数::延迟::横刀.踏空闪 = paramValue;
      }
      if (strstr(paramName.c_str(), "横刀破千军延迟") != NULL) {
        振刀参数::延迟::横刀.破千军 = paramValue;
      }
      if (strstr(paramName.c_str(), "横刀极光破云闪延迟") != NULL) {
        振刀参数::延迟::横刀.极光破云闪 = paramValue;
      }
      // 角度
      if (strstr(paramName.c_str(), "横刀左3角度") != NULL) {
        振刀参数::角度::横刀.左3 = paramValue;
      }
      if (strstr(paramName.c_str(), "横刀右3角度") != NULL) {
        振刀参数::角度::横刀.右3 = paramValue;
      }
      if (strstr(paramName.c_str(), "横刀左蓄角度") != NULL) {
        振刀参数::角度::横刀.左蓄 = paramValue;
      }
      if (strstr(paramName.c_str(), "横刀壁击角度") != NULL) {
        振刀参数::角度::横刀.壁击 = paramValue;
      }
      if (strstr(paramName.c_str(), "横刀右蓄角度") != NULL) {
        振刀参数::角度::横刀.右蓄 = paramValue;
      }
      if (strstr(paramName.c_str(), "横刀苍牙角度") != NULL) {
        振刀参数::角度::横刀.苍牙 = paramValue;
      }
      if (strstr(paramName.c_str(), "横刀同源角度") != NULL) {
        振刀参数::角度::横刀.同源 = paramValue;
      }
      if (strstr(paramName.c_str(), "横刀踏空闪角度") != NULL) {
        振刀参数::角度::横刀.踏空闪 = paramValue;
      }
      if (strstr(paramName.c_str(), "横刀破千军角度") != NULL) {
        振刀参数::角度::横刀.破千军 = paramValue;
      }
      if (strstr(paramName.c_str(), "横刀极光破云闪角度") != NULL) {
        振刀参数::角度::横刀.极光破云闪 = paramValue;
      }
      // 拳刃
      if (strstr(paramName.c_str(), "拳刃左3距离") != NULL) {
        振刀参数::距离::拳刃.左3 = paramValue;
      }
      if (strstr(paramName.c_str(), "拳刃右3距离") != NULL) {
        振刀参数::距离::拳刃.右3 = paramValue;
      }
      if (strstr(paramName.c_str(), "拳刃左蓄距离") != NULL) {
        振刀参数::距离::拳刃.左蓄力 = paramValue;
      }
      if (strstr(paramName.c_str(), "拳刃右蓄距离") != NULL) {
        振刀参数::距离::拳刃.右蓄力 = paramValue;
      }
      if (strstr(paramName.c_str(), "拳刃壁击距离") != NULL) {
        振刀参数::距离::拳刃.壁击 = paramValue;
      }
      if (strstr(paramName.c_str(), "拳刃右蓄2段距离") != NULL) {
        振刀参数::距离::拳刃.右蓄力2段 = paramValue;
      }
      if (strstr(paramName.c_str(), "拳刃苍牙距离") != NULL) {
        振刀参数::距离::拳刃.苍牙 = paramValue;
      }
      if (strstr(paramName.c_str(), "拳刃百裂腿距离") != NULL) {
        振刀参数::距离::拳刃.百裂腿 = paramValue;
      }
      if (strstr(paramName.c_str(), "拳刃破空拳距离") != NULL) {
        振刀参数::距离::拳刃.破空拳 = paramValue;
      }
      if (strstr(paramName.c_str(), "拳刃太极弄云手距离") != NULL) {
        振刀参数::距离::拳刃.太极弄云手 = paramValue;
      }

      // 延迟
      if (strstr(paramName.c_str(), "拳刃左3延迟") != NULL) {
        振刀参数::延迟::拳刃.左3 = paramValue;
      }
      if (strstr(paramName.c_str(), "拳刃右3延迟") != NULL) {
        振刀参数::延迟::拳刃.右3 = paramValue;
      }
      if (strstr(paramName.c_str(), "拳刃左蓄延迟") != NULL) {
        振刀参数::延迟::拳刃.左蓄力 = paramValue;
      }
      if (strstr(paramName.c_str(), "拳刃右蓄延迟") != NULL) {
        振刀参数::延迟::拳刃.右蓄力 = paramValue;
      }
      if (strstr(paramName.c_str(), "拳刃苍牙延迟") != NULL) {
        振刀参数::延迟::拳刃.苍牙 = paramValue;
      }
      if (strstr(paramName.c_str(), "拳刃壁击延迟") != NULL) {
        振刀参数::延迟::拳刃.壁击 = paramValue;
      }
      if (strstr(paramName.c_str(), "拳刃百裂腿延迟") != NULL) {
        振刀参数::延迟::拳刃.百裂腿 = paramValue;
      }
      if (strstr(paramName.c_str(), "拳刃破空拳延迟") != NULL) {
        振刀参数::延迟::拳刃.破空拳 = paramValue;
      }
      if (strstr(paramName.c_str(), "拳刃右蓄2段延迟") != NULL) {
        振刀参数::延迟::拳刃.右蓄力2段 = paramValue;
      }
      if (strstr(paramName.c_str(), "拳刃太极弄云手延迟") != NULL) {
        振刀参数::延迟::拳刃.太极弄云手 = paramValue;
      }
      // 角度
      if (strstr(paramName.c_str(), "拳刃左3角度") != NULL) {
        振刀参数::角度::拳刃.左3 = paramValue;
      }
      if (strstr(paramName.c_str(), "拳刃右3角度") != NULL) {
        振刀参数::角度::拳刃.右3 = paramValue;
      }
      if (strstr(paramName.c_str(), "拳刃左蓄角度") != NULL) {
        振刀参数::角度::拳刃.左蓄力 = paramValue;
      }
      if (strstr(paramName.c_str(), "拳刃右蓄角度") != NULL) {
        振刀参数::角度::拳刃.右蓄力 = paramValue;
      }
      if (strstr(paramName.c_str(), "拳刃苍牙角度") != NULL) {
        振刀参数::角度::拳刃.苍牙 = paramValue;
      }
      if (strstr(paramName.c_str(), "拳刃百裂腿角度") != NULL) {
        振刀参数::角度::拳刃.百裂腿 = paramValue;
      }
      if (strstr(paramName.c_str(), "拳刃破空拳角度") != NULL) {
        振刀参数::角度::拳刃.破空拳 = paramValue;
      }
      if (strstr(paramName.c_str(), "拳刃壁击角度") != NULL) {
        振刀参数::角度::拳刃.壁击 = paramValue;
      }
      if (strstr(paramName.c_str(), "拳刃右蓄2段角度") != NULL) {
        振刀参数::角度::拳刃.右蓄力2段 = paramValue;
      }
      if (strstr(paramName.c_str(), "拳刃太极弄云手角度") != NULL) {
        振刀参数::角度::拳刃.太极弄云手 = paramValue;
      }
      // 飞刀
      if (strstr(paramName.c_str(), "飞刀左3距离") != NULL) {
        振刀参数::距离::飞刀.左3 = paramValue;
      }
      if (strstr(paramName.c_str(), "飞刀右3距离") != NULL) {
        振刀参数::距离::飞刀.右3 = paramValue;
      }
      if (strstr(paramName.c_str(), "飞刀左蓄距离") != NULL) {
        振刀参数::距离::飞刀.左蓄力 = paramValue;
      }
      if (strstr(paramName.c_str(), "飞刀左蓄2段距离") != NULL) {
        振刀参数::距离::飞刀.左蓄力2段 = paramValue;
      }
      if (strstr(paramName.c_str(), "飞刀右蓄距离") != NULL) {
        振刀参数::距离::飞刀.右蓄力 = paramValue;
      }
      if (strstr(paramName.c_str(), "飞刀壁击距离") != NULL) {
        振刀参数::距离::飞刀.壁击 = paramValue;
      }
      if (strstr(paramName.c_str(), "飞刀掌心雷距离") != NULL) {
        振刀参数::距离::飞刀.掌心雷 = paramValue;
      }
      if (strstr(paramName.c_str(), "飞刀左蓄2段距离") != NULL) {
        振刀参数::距离::飞刀.左蓄力2段 = paramValue;
      }
      if (strstr(paramName.c_str(), "飞刀苍牙距离") != NULL) {
        振刀参数::距离::飞刀.苍牙 = paramValue;
      }
      if (strstr(paramName.c_str(), "飞刀太极距离") != NULL) {
        振刀参数::距离::飞刀.太极 = paramValue;
      }
      if (strstr(paramName.c_str(), "飞刀断月千刃舞距离") != NULL) {
        振刀参数::距离::飞刀.断月千刃舞 = paramValue;
      }

      // 延迟
      if (strstr(paramName.c_str(), "飞刀左3延迟") != NULL) {
        振刀参数::延迟::飞刀.左3 = paramValue;
      }
      if (strstr(paramName.c_str(), "飞刀右3延迟") != NULL) {
        振刀参数::延迟::飞刀.右3 = paramValue;
      }
      if (strstr(paramName.c_str(), "飞刀左蓄延迟") != NULL) {
        振刀参数::延迟::飞刀.左蓄力 = paramValue;
      }
      if (strstr(paramName.c_str(), "飞刀左蓄力2段延迟") != NULL) {
        振刀参数::延迟::飞刀.左蓄力2段 = paramValue;
      }
      if (strstr(paramName.c_str(), "飞刀右蓄延迟") != NULL) {
        振刀参数::延迟::飞刀.右蓄力 = paramValue;
      }
      if (strstr(paramName.c_str(), "飞刀苍牙延迟") != NULL) {
        振刀参数::延迟::飞刀.苍牙 = paramValue;
      }
      if (strstr(paramName.c_str(), "飞刀壁击延迟") != NULL) {
        振刀参数::延迟::飞刀.壁击 = paramValue;
      }
      if (strstr(paramName.c_str(), "飞刀太极延迟") != NULL) {
        振刀参数::延迟::飞刀.太极 = paramValue;
      }
      if (strstr(paramName.c_str(), "飞刀掌心雷延迟") != NULL) {
        振刀参数::延迟::飞刀.掌心雷 = paramValue;
      }
      if (strstr(paramName.c_str(), "飞刀断月千刃舞延迟") != NULL) {
        振刀参数::延迟::飞刀.断月千刃舞 = paramValue;
      }
      // 角度
      if (strstr(paramName.c_str(), "飞刀左3角度") != NULL) {
        振刀参数::角度::飞刀.左3 = paramValue;
      }
      if (strstr(paramName.c_str(), "飞刀右3角度") != NULL) {
        振刀参数::角度::飞刀.右3 = paramValue;
      }
      if (strstr(paramName.c_str(), "飞刀左蓄角度") != NULL) {
        振刀参数::角度::飞刀.左蓄力 = paramValue;
      }
      if (strstr(paramName.c_str(), "飞刀左蓄2段角度") != NULL) {
        振刀参数::角度::飞刀.左蓄力2段 = paramValue;
      }
      if (strstr(paramName.c_str(), "飞刀右蓄角度") != NULL) {
        振刀参数::角度::飞刀.右蓄力 = paramValue;
      }
      if (strstr(paramName.c_str(), "飞刀壁击角度") != NULL) {
        振刀参数::角度::飞刀.壁击 = paramValue;
      }
      if (strstr(paramName.c_str(), "飞刀掌心雷角度") != NULL) {
        振刀参数::角度::飞刀.掌心雷 = paramValue;
      }
      if (strstr(paramName.c_str(), "飞刀苍牙角度") != NULL) {
        振刀参数::角度::飞刀.苍牙 = paramValue;
      }
      if (strstr(paramName.c_str(), "飞刀太极角度") != NULL) {
        振刀参数::角度::飞刀.太极 = paramValue;
      }
      if (strstr(paramName.c_str(), "飞刀断月千刃舞角度") != NULL) {
        振刀参数::角度::飞刀.断月千刃舞 = paramValue;
      }
    }
  }
}
void 读取模拟振配置() {
  ConfigDefaults::ApplyShockConfig();
  std::ifstream file("模拟振配置.ini");
  if (!file.is_open()) {
    g_SkipDodgeParamSave = true;
    保存模拟振配置();
    g_SkipDodgeParamSave = false;
    读取闪避参数配置();
    return;
  }

  std::string line;
  std::string fileVersion;
  bool versionFound = false;
  while (std::getline(file, line)) {
    std::string parsedVersion;
    if (ParseShockConfigVersionLine(line, &parsedVersion)) {
      versionFound = true;
      fileVersion = parsedVersion;
      continue;
    }
    判断模拟振配置(line);
  }
  file.close();

  if (!versionFound || fileVersion != moniVersionname) {
    g_SkipDodgeParamSave = true;
    保存模拟振配置();
    g_SkipDodgeParamSave = false;
  }

  读取闪避参数配置();
}
void 判断活化配置(const std::string &line) {
  std::istringstream iss(line);
  std::string paramName, token;
  float paramValue;

  if (std::getline(iss, paramName, '=') && std::getline(iss, token)) {
    paramValue = std::stof(token);

    static const std::unordered_map<std::string, float *> 距离 = {
        {"火男冲拳距离", &活化参数::距离::火男冲拳},
        {"火男顶膝距离", &活化参数::距离::火男顶膝},
        {"火男F距离", &活化参数::距离::火男F},
        {"岳山F距离", &活化参数::距离::岳山F},
        {"妖刀大招距离", &活化参数::距离::妖刀大招},
        {"顾倾寒大招距离", &活化参数::距离::顾倾寒大招},
        {"武田F2距离", &活化参数::距离::武田F2},
        {"武田F3距离", &活化参数::距离::武田F3},
        {"胡为F1距离", &活化参数::距离::胡为F1},
        {"胡为F2距离", &活化参数::距离::胡为F2},
        {"胡为跃击距离", &活化参数::距离::胡为跃击},
        {"胡为V1V2距离", &活化参数::距离::胡为V1V2},
        {"宁红叶F1距离", &活化参数::距离::宁红叶F1},
        {"宁红叶F2距离", &活化参数::距离::宁红叶F2},
        {"宁红叶F3距离", &活化参数::距离::宁红叶F3},
        {"崔三娘V2距离", &活化参数::距离::三娘V2},
        {"狐狸V1距离", &活化参数::距离::狐狸V1},
        {"狐狸V2距离", &活化参数::距离::狐狸V2},
        {"魏轻F1距离", &活化参数::距离::魏轻F1},
        {"魏轻V1距离", &活化参数::距离::魏轻V1},
        {"魏轻V2距离", &活化参数::距离::魏轻V2},
        {"迦南F2距离", &活化参数::距离::迦南F2},
        {"迦南V2距离", &活化参数::距离::迦南V2},
        {"刘炼V1距离", &活化参数::距离::刘炼V1},
        {"刘炼V2距离", &活化参数::距离::刘炼V2},
        {"季盈盈V1蓄力距离", &活化参数::距离::季盈盈V1蓄力},
        {"季盈盈V2蓄力距离", &活化参数::距离::季盈盈V2蓄力},
        {"哈迪1段距离", &活化参数::距离::哈迪大招1段},
        {"哈迪2段距离", &活化参数::距离::哈迪大招2段},
        {"哈迪3段距离", &活化参数::距离::哈迪大招3段},
        {"张起灵F2距离", &活化参数::距离::张起灵F2},
        {"张起灵V1距离", &活化参数::距离::张起灵V1},
        {"张起灵V12段距离", &活化参数::距离::张起灵V12段},
        {"张起灵V2距离", &活化参数::距离::张起灵V2},
        {"无尘V2距离", &活化参数::距离::无尘V2},
        {"希拉F距离", &活化参数::距离::希拉F},
        {"彩戏师F距离", &活化参数::距离::彩戏师F}};

    static const std::unordered_map<std::string, float *> 角度 = {
        {"火男冲拳角度", &活化参数::角度::火男冲拳},
        {"火男F角度", &活化参数::角度::火男F},
        {"岳山F角度", &活化参数::角度::岳山F},
        {"妖刀大招3M角度", &活化参数::角度::妖刀大招3M},
        {"妖刀大招3M外角度", &活化参数::角度::妖刀大招3M外},
        {"顾倾寒大招3M角度", &活化参数::角度::顾倾寒大招3M},
        {"顾倾寒大招3M外角度", &活化参数::角度::顾倾寒大招3M外},
        {"武田F21M角度", &活化参数::角度::武田F21M},
        {"武田F21M外角度", &活化参数::角度::武田F21M外},
        {"武田F31M角度", &活化参数::角度::武田F31M},
        {"武田F31M外角度", &活化参数::角度::武田F31M外},
        {"胡为F2角度", &活化参数::角度::胡为F2},
        {"胡为跃击角度", &活化参数::角度::胡为跃击},
        {"胡为V1V2角度", &活化参数::角度::胡为V1V2},
        {"宁红叶F1角度", &活化参数::角度::宁红叶F1},
        {"宁红叶F2角度", &活化参数::角度::宁红叶F2},
        {"宁红叶F3角度", &活化参数::角度::宁红叶F3},
        {"狐狸大招4M角度", &活化参数::角度::狐狸大招4M},
        {"狐狸大招8M角度", &活化参数::角度::狐狸大招8M},
        {"狐狸大招12M角度", &活化参数::角度::狐狸大招12M},
        {"狐狸大招12M外角度", &活化参数::角度::狐狸大招12M外},
        {"叶修大招矛角度", &活化参数::角度::叶修大招矛},
        {"叶修大招镰角度", &活化参数::角度::叶修大招镰},
        {"叶修炮形态4M角度", &活化参数::角度::叶修炮形态4M},
        {"叶修炮形态8M角度", &活化参数::角度::叶修炮形态8M},
        {"叶修炮形态12M角度", &活化参数::角度::叶修炮形态12M},
        {"叶修炮形态12M外角度", &活化参数::角度::叶修炮形态12M外},
        {"魏轻V1角度", &活化参数::角度::魏轻V1},
        {"魏轻V2角度", &活化参数::角度::魏轻V2},
        {"迦南F2角度", &活化参数::角度::迦南F2},
        {"迦南V2角度", &活化参数::角度::迦南V2},
        {"刘炼V1角度", &活化参数::角度::刘炼V1},
        {"刘炼V2角度", &活化参数::角度::刘炼V2},
        {"季盈盈V1角度", &活化参数::角度::季盈盈V1蓄力},
        {"季盈盈V2角度", &活化参数::角度::季盈盈V2蓄力},
        {"哈迪V12角度", &活化参数::角度::哈迪大招},
        {"张起灵F2角度", &活化参数::角度::张起灵F2},
        {"张起灵V1角度", &活化参数::角度::张起灵V1},
        {"张起灵V12段角度", &活化参数::角度::张起灵V12段},
        {"张起灵V2角度", &活化参数::角度::张起灵V2},
        {"无尘V2角度", &活化参数::角度::无尘V2},
        {"希拉F角度", &活化参数::角度::希拉F},
        {"彩戏师F角度", &活化参数::角度::彩戏师F},
        {"三娘V2 4M角度", &活化参数::角度::三娘V24M},
        {"三娘V2 10M角度", &活化参数::角度::三娘V210M},
        {"三娘V2 10M外角度", &活化参数::角度::三娘V210M外}};

    auto 距离迭代 = 距离.find(paramName);
    if (距离迭代 != 距离.end()) {
      *距离迭代->second = paramValue;
    }

    auto 角度迭代 = 角度.find(paramName);
    if (角度迭代 != 角度.end()) {
      *角度迭代->second = paramValue;
    }
  }
}

void 保存活化配置() {
  ofstream file("活化配置.ini");
  if (file.is_open()) {

    // 距离参数映射
    static const std::unordered_map<std::string, float *> 距离 = {
        {"火男冲拳距离", &活化参数::距离::火男冲拳},
        {"火男顶膝距离", &活化参数::距离::火男顶膝},
        {"火男F距离", &活化参数::距离::火男F},
        {"岳山F距离", &活化参数::距离::岳山F},
        {"妖刀大招距离", &活化参数::距离::妖刀大招},
        {"顾倾寒大招距离", &活化参数::距离::顾倾寒大招},
        {"武田F2距离", &活化参数::距离::武田F2},
        {"武田F3距离", &活化参数::距离::武田F3},
        {"胡为F1距离", &活化参数::距离::胡为F1},
        {"胡为F2距离", &活化参数::距离::胡为F2},
        {"胡为跃击距离", &活化参数::距离::胡为跃击},
        {"胡为V1V2距离", &活化参数::距离::胡为V1V2},
        {"宁红叶F1距离", &活化参数::距离::宁红叶F1},
        {"宁红叶F2距离", &活化参数::距离::宁红叶F2},
        {"宁红叶F3距离", &活化参数::距离::宁红叶F3},
        {"狐狸V1距离", &活化参数::距离::狐狸V1},
        {"狐狸V2距离", &活化参数::距离::狐狸V2},
        {"魏轻F1距离", &活化参数::距离::魏轻F1},
        {"魏轻V1距离", &活化参数::距离::魏轻V1},
        {"魏轻V2距离", &活化参数::距离::魏轻V2},
        {"迦南F2距离", &活化参数::距离::迦南F2},
        {"迦南V2距离", &活化参数::距离::迦南V2},
        {"刘炼V1距离", &活化参数::距离::刘炼V1},
        {"刘炼V2距离", &活化参数::距离::刘炼V2},
        {"季盈盈V1蓄力距离", &活化参数::距离::季盈盈V1蓄力},
        {"季盈盈V2蓄力距离", &活化参数::距离::季盈盈V2蓄力},
        {"哈迪1段距离", &活化参数::距离::哈迪大招1段},
        {"哈迪2段距离", &活化参数::距离::哈迪大招2段},
        {"哈迪3段距离", &活化参数::距离::哈迪大招3段},
        {"张起灵F2距离", &活化参数::距离::张起灵F2},
        {"张起灵V1距离", &活化参数::距离::张起灵V1},
        {"张起灵V12段距离", &活化参数::距离::张起灵V12段},
        {"张起灵V2距离", &活化参数::距离::张起灵V2},
        {"无尘V2距离", &活化参数::距离::无尘V2},
        {"希拉F距离", &活化参数::距离::希拉F},
        {"彩戏师F距离", &活化参数::距离::彩戏师F}};

    // 角度参数映射
    static const std::unordered_map<std::string, float *> 角度 = {
        {"火男冲拳角度", &活化参数::角度::火男冲拳},
        {"火男F角度", &活化参数::角度::火男F},
        {"岳山F角度", &活化参数::角度::岳山F},
        {"妖刀大招3M角度", &活化参数::角度::妖刀大招3M},
        {"妖刀大招3M外角度", &活化参数::角度::妖刀大招3M外},
        {"顾倾寒大招3M角度", &活化参数::角度::顾倾寒大招3M},
        {"顾倾寒大招3M外角度", &活化参数::角度::顾倾寒大招3M外},
        {"武田F21M角度", &活化参数::角度::武田F21M},
        {"武田F21M外角度", &活化参数::角度::武田F21M外},
        {"武田F31M角度", &活化参数::角度::武田F31M},
        {"武田F31M外角度", &活化参数::角度::武田F31M外},
        {"胡为F2角度", &活化参数::角度::胡为F2},
        {"胡为跃击角度", &活化参数::角度::胡为跃击},
        {"胡为V1V2角度", &活化参数::角度::胡为V1V2},
        {"宁红叶F1角度", &活化参数::角度::宁红叶F1},
        {"宁红叶F2角度", &活化参数::角度::宁红叶F2},
        {"宁红叶F3角度", &活化参数::角度::宁红叶F3},
        {"狐狸大招4M角度", &活化参数::角度::狐狸大招4M},
        {"狐狸大招8M角度", &活化参数::角度::狐狸大招8M},
        {"狐狸大招12M角度", &活化参数::角度::狐狸大招12M},
        {"狐狸大招12M外角度", &活化参数::角度::狐狸大招12M外},
        {"叶修大招矛角度", &活化参数::角度::叶修大招矛},
        {"叶修大招镰角度", &活化参数::角度::叶修大招镰},
        {"叶修炮形态4M角度", &活化参数::角度::叶修炮形态4M},
        {"叶修炮形态8M角度", &活化参数::角度::叶修炮形态8M},
        {"叶修炮形态12M角度", &活化参数::角度::叶修炮形态12M},
        {"叶修炮形态12M外角度", &活化参数::角度::叶修炮形态12M外},
        {"魏轻V1角度", &活化参数::角度::魏轻V1},
        {"魏轻V2角度", &活化参数::角度::魏轻V2},
        {"迦南F2角度", &活化参数::角度::迦南F2},
        {"迦南V2角度", &活化参数::角度::迦南V2},
        {"刘炼V1角度", &活化参数::角度::刘炼V1},
        {"刘炼V2角度", &活化参数::角度::刘炼V2},
        {"季盈盈V1角度", &活化参数::角度::季盈盈V1蓄力},
        {"季盈盈V2角度", &活化参数::角度::季盈盈V2蓄力},
        {"哈迪V12角度", &活化参数::角度::哈迪大招},
        {"张起灵F2角度", &活化参数::角度::张起灵F2},
        {"张起灵V1角度", &活化参数::角度::张起灵V1},
        {"张起灵V12段角度", &活化参数::角度::张起灵V12段},
        {"张起灵V2角度", &活化参数::角度::张起灵V2},
        {"无尘V2角度", &活化参数::角度::无尘V2},
        {"希拉F角度", &活化参数::角度::希拉F},
        {"彩戏师F角度", &活化参数::角度::彩戏师F},
        {"三娘V24M角度", &活化参数::角度::三娘V24M},
        {"三娘V210M角度", &活化参数::角度::三娘V210M},
        {"三娘V210M外角度", &活化参数::角度::三娘V210M外}};

    // 保存距离参数
    for (const auto &映射项 : 距离) {
      file << 映射项.first << "=" << *映射项.second << "\n";
    }

    // 保存角度参数
    for (const auto &映射项 : 角度) {
      file << 映射项.first << "=" << *映射项.second << "\n";
    }

    file.close();
  }
}

void 读取活化配置() {
  std::ifstream file("活化配置.ini");
  if (file.is_open()) {
    std::string line;
    while (std::getline(file, line)) {
      判断活化配置(line);
    }

    file.close();
    // cout << "活化配置读取完毕" << endl;
  }
}

void 保存Kmbox() {
  ofstream file("Kmbox.ini");
  if (file.is_open()) {
    file << "IP=" << Function::Kmbox::Net::IP << "\n";
    file << "Port=" << Function::Kmbox::Net::Port << "\n";
    file << "UUID=" << Function::Kmbox::Net::mac << "\n";

    file << "端口号=" << Function::Kmbox::B_Pro::PorttextBuffer << "\n";
    file << "波特率=" << Function::Kmbox::B_Pro::SpeedBuffer << "\n";

    file << "DIP=" << Function::Kmbox::DhzBox_Lite::IP << "\n";
    file << "DPort=" << Function::Kmbox::DhzBox_Lite::Port << "\n";
    file << "RANDOM=" << Function::Kmbox::DhzBox_Lite::RANDOM << "\n";

    file.close();
  }
}
void 判断Kmbox配置(const std::string &line) {
  std::istringstream iss(line);
  std::string token;
  std::string paramName;
  // float paramValue;
  if (std::getline(iss, token, '=')) {
    paramName = token;
    if (std::getline(iss, token)) {
      // paramValue = std::stof(token); Port UUID 端口号 波特率 DIP DPort RANDOM
      if (paramName.find("IP") == 0) {
        strcpy(Function::Kmbox::Net::IP,
               token.c_str()); /*printf("%s\n", Function::Kmbox::Net::IP);*/
      }
      if (paramName.find("Port") == 0) {
        strcpy(Function::Kmbox::Net::Port,
               token.c_str()); /*printf("%s\n", Function::Kmbox::Net::Port);*/
      }
      if (paramName.find("UUID") == 0) {
        strcpy(Function::Kmbox::Net::mac,
               token.c_str()); /*printf("%s\n", Function::Kmbox::Net::mac);*/
      }

      if (paramName.find("端口号") == 0) {
        strcpy(Function::Kmbox::B_Pro::PorttextBuffer, token.c_str());
      }
      if (paramName.find("波特率") == 0) {
        strcpy(Function::Kmbox::B_Pro::SpeedBuffer, token.c_str());
      }

      if (paramName.find("DIP") == 0) {
        strncpy_s(Function::Kmbox::DhzBox_Lite::IP,
                  sizeof(Function::Kmbox::DhzBox_Lite::IP), token.c_str(),
                  _TRUNCATE);
      }
      if (paramName.find("DPort") == 0) {
        strncpy_s(Function::Kmbox::DhzBox_Lite::Port,
                  sizeof(Function::Kmbox::DhzBox_Lite::Port), token.c_str(),
                  _TRUNCATE);
      }
      if (paramName.find("RANDOM") == 0) {
        strncpy_s(Function::Kmbox::DhzBox_Lite::RANDOM,
                  sizeof(Function::Kmbox::DhzBox_Lite::RANDOM), token.c_str(),
                  _TRUNCATE);
      }
    }
  }
}
void 读取Kmbox配置() {
  std::ifstream file("Kmbox.ini");
  if (file.is_open()) {
    std::string line;
    while (std::getline(file, line)) {
      判断Kmbox配置(line);
    }

    file.close();
  }
}

inline void SaveKeyBinds(const std::string &path = "keybinds.json") {
  nlohmann::json j;
  for (auto &[id, b] : KeyBinds::binds)
    j[id] = {{"keyCode", b.keyCode}, {"keyName", b.keyName}};
  std::ofstream(path) << j.dump(4);
}

inline void LoadKeyBinds(const std::string &path = "keybinds.json") {
  std::ifstream f(path);
  if (!f.is_open())
    return;
  nlohmann::json j;
  f >> j;
  for (auto &[id, b] : KeyBinds::binds) {
    if (j.contains(id)) {
      b.keyCode = j[id]["keyCode"];
      b.keyName = j[id]["keyName"];
    }
  }
}
void 保存博弈参数() {
  ofstream file("博弈配置.ini");
  if (file.is_open()) {
    file << "蓄力抓长闪=" << Function::侧键博弈::蓄力抓长闪 << "\n";
    file << "蓄力抓短闪=" << Function::侧键博弈::蓄力抓短闪 << "\n";
    file << "蓄力抓白刀=" << Function::侧键博弈::蓄力抓白刀 << "\n";
    file << "蓄力抓倒地=" << Function::侧键博弈::蓄力抓倒地 << "\n";
    file << "蓄力抓受击=" << Function::侧键博弈::蓄力抓受击 << "\n";
    file << "蓄力抓起跳=" << Function::侧键博弈::蓄力抓起跳 << "\n";
    file << "蓄力抓下蹲=" << Function::侧键博弈::蓄力抓下蹲 << "\n";
    file << "博弈切刀抓振刀=" << Function::侧键博弈::博弈切刀抓振刀 << "\n";
    file << "背包切蓄力=" << Function::侧键博弈::背包切蓄力 << "\n";
    file << "背包切蓄力概率=" << Function::侧键博弈::背包切蓄力概率 << "\n";

    file << "白刀抓长闪=" << Function::侧键博弈::白刀抓长闪 << "\n";
    file << "白刀抓短闪=" << Function::侧键博弈::白刀抓短闪 << "\n";
    file << "白刀抓蓄力后摇=" << Function::侧键博弈::白刀抓蓄力后摇 << "\n";
    file << "白刀抓振=" << Function::侧键博弈::白刀抓振 << "\n";
    file << "白刀蓝顶=" << Function::侧键博弈::白刀蓝顶 << "\n";

    file << "蓄力抓振延迟=" << Function::侧键博弈::蓄力抓振延迟 << "\n";
    file << "白刀蓝顶延迟=" << Function::侧键博弈::白刀蓝顶延迟 << "\n";
    file << "白刀抓短闪延迟=" << Function::侧键博弈::白刀抓短闪延迟 << "\n";

    file << "白刀抓短闪最远距离=" << Function::侧键博弈::白刀抓短闪最远距离
         << "\n";
    file << "白刀抓长闪最远距离=" << Function::侧键博弈::白刀抓长闪最远距离
         << "\n";

    file << "动态线程=" << Function::侧键博弈::动态线程 << "\n";
    file << "切拳起身=" << Function::侧键博弈::切拳起身 << "\n";
    file << "断续切刀时间=" << Function::侧键博弈::断续切刀时间 << "\n";
    file << "断续切刀最小时间=" << Function::侧键博弈::断续切刀最小时间 << "\n";
    file << "振刀模式热键=" << Function::侧键博弈::振刀模式热键 << "\n";
    file << "闪避模式热键=" << Function::侧键博弈::闪避模式热键 << "\n";
    file << "禁用CPU0=" << Function::侧键博弈::禁用CPU0 << "\n";
    file << "缓存效率模式=" << Function::侧键博弈::缓存效率模式 << "\n";
    // file << "钩锁模式=" << Function::侧键博弈::钩锁模式 << "\n";

    file << "半自动蓄抓长闪=" << Function::半自动博弈::蓄力抓长闪 << "\n";
    file << "半自动蓄抓短闪=" << Function::半自动博弈::蓄力抓短闪 << "\n";
    file << "半自动蓄抓白刀=" << Function::半自动博弈::蓄力抓白刀 << "\n";
    file << "半自动蓄抓倒地=" << Function::半自动博弈::蓄力抓倒地 << "\n";
    file << "半自动蓄抓受击=" << Function::半自动博弈::蓄力抓受击 << "\n";
    file << "半自动蓄抓起跳=" << Function::半自动博弈::蓄力抓起跳 << "\n";
    file << "半自动蓄抓下蹲=" << Function::半自动博弈::蓄力抓下蹲 << "\n";

    file << "半自动白抓长闪=" << Function::半自动博弈::白刀抓长闪 << "\n";
    file << "半自动白抓短闪=" << Function::半自动博弈::白刀抓短闪 << "\n";
    file << "半自动白抓蓄力后摇=" << Function::半自动博弈::白刀抓蓄力后摇
         << "\n";
    file << "半自动白抓振=" << Function::半自动博弈::白刀抓振 << "\n";
    file << "半自动白蓝顶=" << Function::半自动博弈::白刀蓝顶 << "\n";

    file << "半自动蓄抓振延迟=" << Function::半自动博弈::蓄力抓振延迟 << "\n";
    file << "半自动白蓝顶延迟=" << Function::半自动博弈::白刀蓝顶延迟 << "\n";

    file << "半自动白抓短闪最远距离="
         << Function::半自动博弈::白刀抓短闪最远距离 << "\n";
    file << "半自动白抓长闪最远距离="
         << Function::半自动博弈::白刀抓长闪最远距离 << "\n";
    file << "半自动总开关=" << Function::半自动博弈::总开关 << "\n";
    file << "半自动自动连招=" << Function::半自动博弈::自动连招 << "\n";
    file << "半自动自动断蓄=" << Function::半自动博弈::自动断蓄 << "\n";
    file << "半自动解键按键=" << Function::半自动博弈::解键按键 << "\n";
    file << "半自动切刀时间=" << Function::半自动博弈::断续切刀时间 << "\n";
    file << "半自动白抓受击=" << Function::半自动博弈::白刀抓受击 << "\n";
    file << "半自动白刀抓总开关=" << Function::半自动博弈::白刀抓总开关 << "\n";
    file << "半自动蓄力抓总开关=" << Function::半自动博弈::蓄力抓总开关 << "\n";
  }
  file.close();
}

void 保存连招参数() {
  ofstream file("连招配置.ini");
  if (file.is_open()) {
    file << "连招二按键=" << 连招方式::第二套按键 << "\n";

    file << "太刀连招方式=" << 连招方式::太刀 << "\n";
    file << "横刀连招方式=" << 连招方式::横刀 << "\n";
    file << "长剑连招方式=" << 连招方式::长剑 << "\n";
    file << "链剑连招方式=" << 连招方式::链剑 << "\n";
    file << "长枪连招方式=" << 连招方式::长枪 << "\n";
    file << "匕首连招方式=" << 连招方式::匕首 << "\n";
    file << "双截棍连招方式=" << 连招方式::双截棍 << "\n";
    file << "双刀连招方式=" << 连招方式::双刀 << "\n";
    file << "长棍连招方式=" << 连招方式::长棍 << "\n";
    file << "斩马刀连招方式=" << 连招方式::斩马刀 << "\n";
    file << "双戟连招方式=" << 连招方式::双戟 << "\n";
    file << "扇子连招方式=" << 连招方式::扇子 << "\n";
    file << "拳刃连招方式 = " << 连招方式::拳刃 << "\n";
    file << "飞刀连招方式=" << 连招方式::飞刀 << "\n";
    file << "阔刀连招方式=" << 连招方式::阔刀 << "\n";

    file << "太刀一连招方式=" << 连招方式::太刀1 << "\n";
    file << "横刀一连招方式=" << 连招方式::横刀1 << "\n";
    file << "长剑一连招方式=" << 连招方式::长剑1 << "\n";
    file << "链剑一连招方式=" << 连招方式::链剑1 << "\n";
    file << "长枪一连招方式=" << 连招方式::长枪1 << "\n";
    file << "匕首一连招方式=" << 连招方式::匕首1 << "\n";
    file << "双截棍一连招方式=" << 连招方式::双截棍1 << "\n";
    file << "双刀一连招方式=" << 连招方式::双刀1 << "\n";
    file << "长棍一连招方式=" << 连招方式::长棍1 << "\n";
    file << "斩马刀一连招方式=" << 连招方式::斩马刀1 << "\n";
    file << "双戟一连招方式=" << 连招方式::双戟1 << "\n";
    file << "扇子一连招方式=" << 连招方式::扇子1 << "\n";
    file << "拳刃一连招方式 = " << 连招方式::拳刃1 << "\n";
    file << "飞刀一连招方式=" << 连招方式::飞刀1 << "\n";
    file << "阔刀一连招方式=" << 连招方式::阔刀1 << "\n";
  }
  file.close();
}

void 保存其他参数() {
  ofstream file("其他配置.ini");
  if (file.is_open()) {
    file << "daoJuTextSize=" << Function::ESP::daoJuTextSize << "\n";
    file << "userTextSize=" << Function::ESP::userTextSize << "\n";
    file << "方框=" << Function::ESP::方框 << "\n";
    file << "骨骼=" << Function::ESP::骨骼 << "\n";
    file << "信息=" << Function::ESP::信息 << "\n";
    file << "血甲=" << Function::ESP::血条 << "\n";
    file << "手持=" << Function::ESP::手持 << "\n";
    file << "预警=" << Function::ESP::预警 << "\n";
    file << "射线=" << Function::ESP::射线 << "\n";
    file << "血条样式=" << Function::ESP::血条样式 << "\n";
    file << "盲区样式=" << Function::ESP::预警样式 << "\n";
    file << "灵魂=" << Function::ESP::灵魂 << "\n";

    file << "人机颜色=" << Function::ESP::人机颜色.Value.x << ","
         << Function::ESP::人机颜色.Value.y << ","
         << Function::ESP::人机颜色.Value.z << ","
         << Function::ESP::人机颜色.Value.w << "\n";

    file << "玩家颜色=" << Function::ESP::玩家颜色.Value.x << ","
         << Function::ESP::玩家颜色.Value.y << ","
         << Function::ESP::玩家颜色.Value.z << ","
         << Function::ESP::玩家颜色.Value.w << "\n";
    // 振刀
    file << "启用振刀=" << Function::Shock::F_ShockKnife << "\n";
    file << "内存振刀=" << Function::Shock::M_ShockKnife << "\n";
    Function::Shock::MemoryShockMode = 0;
    file << "内存振逻辑=0\n";
    file << "内存振变态模式=" << Function::Shock::内存振变态模式 << "\n";
    file << "动画加速=" << Function::Shock::动画加速 << "\n";
    file << "动画加速倍率=" << Function::Shock::动画加速倍率 << "\n";
    file << "扬鞭劲振刀=" << Function::Shock::扬鞭劲振刀 << "\n";
    file << "横栏振刀=" << Function::Shock::横栏振刀 << "\n";
    file << "横刀左蓄振刀=" << Function::Shock::横刀左蓄振刀 << "\n";
    file << "只振视野范围内=" << Function::Shock::只振视野范围内 << "\n";
    file << "是否闪避=" << Function::Shock::Flag_ShortDodge << "\n";
    file << "是否拼刀=" << Function::Shock::Flag_CollideKnife << "\n";
    file << "输出调试执行=" << 调试::输出调试执行 << "\n";
    file << "输出逻辑执行=" << 调试::输出逻辑执行 << "\n";
    file << "按键方式="
         << 配置按键方式.load(std::memory_order_acquire) << "\n";
    file << "振刀按键=" << Function::Shock::振刀按键 << "\n";
    file << "切拳按键=" << Function::Shock::切拳按键 << "\n";
    file << "滞空按键=" << Function::Shock::滞空按键 << "\n";
    file << "振刀方式=" << 振刀按鍵 << "\n";
    file << "断蓄方式=" << 自动断蓄 << "\n";
    file << "双截棍左蓄短闪振=" << 振刀参数::特殊处理::双截棍左蓄是否短闪振
         << "\n";
    file << "扇子左蓄中闪振=" << 振刀参数::特殊处理::扇子左蓄是否中闪振 << "\n";
    file << "扇子右蓄中闪振=" << 振刀参数::特殊处理::扇子右蓄是否中闪振 << "\n";
    file << "斩马刀左蓄长闪振=" << 振刀参数::特殊处理::斩马刀左蓄是否长闪振
         << "\n";
    file << "阔刀右蓄短闪振=" << 振刀参数::特殊处理::阔刀右蓄是否短闪振 << "\n";
    file << "横刀左蓄是否只短闪=" << 振刀参数::特殊处理::横刀左蓄是否只短闪
         << "\n";
    file << "胡为开大振刀=" << 振刀参数::特殊处理::胡为开大振刀 << "\n";
    file << "叶修开大振刀=" << 振刀参数::特殊处理::叶修开大振刀 << "\n";
    // 物品
    file << "近战-白=" << Function::ESP::近战_白 << "\n";
    file << "近战-蓝=" << Function::ESP::近战_蓝 << "\n";
    file << "近战-紫=" << Function::ESP::近战_紫 << "\n";
    file << "近战-金=" << Function::ESP::近战_金 << "\n";
    file << "远程-白=" << Function::ESP::远程_白 << "\n";
    file << "远程-蓝=" << Function::ESP::远程_蓝 << "\n";
    file << "远程-紫=" << Function::ESP::远程_紫 << "\n";
    file << "远程-金=" << Function::ESP::远程_金 << "\n";
    file << "道具=" << Function::ESP::道具 << "\n";
    file << "扩容=" << Function::ESP::扩容 << "\n";
    file << "金魂=" << Function::ESP::金魂 << "\n";
    file << "夺魂=" << Function::ESP::夺魂 << "\n";
    file << "属性=" << Function::ESP::属性 << "\n";
    file << "盒子=" << Function::ESP::盒子 << "\n";
    file << "白甲=" << Function::ESP::护甲Lv1 << "\n";
    file << "蓝甲=" << Function::ESP::护甲Lv2 << "\n";
    file << "紫甲=" << Function::ESP::护甲Lv3 << "\n";
    file << "金甲=" << Function::ESP::护甲Lv4 << "\n";
    file << "红甲=" << Function::ESP::护甲Lv5 << "\n";
    file << "换甲护甲=" << Function::ESP::换甲护甲 << "\n";
    file << "果实=" << Function::ESP::果实 << "\n";
    file << "萤火虫=" << Function::ESP::萤火虫 << "\n";
    file << "金堆=" << Function::ESP::金堆 << "\n";
    file << "蓝堆=" << Function::ESP::蓝堆 << "\n";
    file << "绿堆=" << Function::ESP::绿堆 << "\n";
    file << "任务=" << Function::ESP::任务 << "\n";
    file << "秘籍=" << Function::ESP::秘籍 << "\n";
    // 活化
    file << "启用闪避=" << Function::HeroActivation::IsDodge << "\n";
    file << "技能活化=" << Function::HeroActivation::技能活化 << "\n";
    file << "躲避火男冲拳=" << Function::HeroActivation::躲避火男冲拳 << "\n";
    file << "躲避火男F=" << Function::HeroActivation::躲避火男F << "\n";
    file << "躲避妖刀大招=" << Function::HeroActivation::躲避妖刀大招 << "\n";
    file << "躲避顾清寒V1V2=" << Function::HeroActivation::躲避顾清寒V1V2
         << "\n";
    file << "躲避武田F=" << Function::HeroActivation::躲避武田F << "\n";
    file << "躲避胡为=" << Function::HeroActivation::躲避胡为 << "\n";
    file << "躲避宁红叶F=" << Function::HeroActivation::躲避宁红叶F << "\n";
    file << "躲避狐狸大招=" << Function::HeroActivation::躲避狐狸大招 << "\n";
    file << "躲避乌真大招=" << Function::HeroActivation::躲避乌真大招 << "\n";
    file << "躲避三娘V2=" << Function::HeroActivation::躲避三娘V2 << "\n";
    file << "躲避哈迪大招=" << Function::HeroActivation::躲避哈迪大招 << "\n";
    file << "躲避岳山F=" << Function::HeroActivation::躲避岳山F << "\n";
    file << "躲避魏轻F1=" << Function::HeroActivation::躲避魏轻F1 << "\n";
    file << "躲避魏轻大招=" << Function::HeroActivation::躲避魏轻V << "\n";
    file << "躲避迦南F2=" << Function::HeroActivation::躲避迦南F2 << "\n";
    file << "躲避迦南V2=" << Function::HeroActivation::躲避迦南V2 << "\n";
    file << "躲避刘炼V1V2=" << Function::HeroActivation::躲避刘炼V << "\n";
    file << "躲避季盈盈V蓄力=" << Function::HeroActivation::躲避季盈盈V蓄力
         << "\n";
    file << "躲避张起灵=" << Function::HeroActivation::躲避张起灵 << "\n";
    file << "躲避无尘V2=" << Function::HeroActivation::躲避无尘V2 << "\n";

    file << "蓄力抓振刀=" << Function::LogicKnife::BlueAttackShock << "\n";
    file << "蓄力抓白刀=" << Function::LogicKnife::BlueAttackWrite << "\n";
    file << "抓闪=" << Function::LogicKnife::BlueAttackdodge << "\n";
    file << "蓝刀=" << Function::LogicKnife::BlueAttacklanding << "\n";
    file << "自动断蓄=" << Function::LogicKnife::BlueAttackDuanXu << "\n";
    file << "自动辅助连招=" << Function::LogicKnife::AutoCombo << "\n";
    file << "自动毛刀=" << Function::LogicKnife::auto_pickup_weapon << "\n";
    file << "双持快速c切=" << Function::LogicKnife::BlueAttackDuanXu_fast
         << "\n";
    file << "不振3A=" << Function::LogicKnife::NO_Shock_3A << "\n";
    file << "打药时只振闪贴脸敌人="
         << Function::LogicKnife::MedicineOnlyShockDodgeCloseEnemy << "\n";
    file << "打印物品ID=" << Function::Flag::打印物品ID << "\n";
    file << "打印魂玉ID=" << Function::Flag::打印魂玉ID << "\n";

    file << "出蓄抓振延迟=" << 活化参数::出蓄抓振延迟 << "\n";
    file << "蓝顶延迟=" << 活化参数::蓝顶延迟 << "\n";
  }
  file.close();
}

void 保存自瞄配置() {
  ofstream file("自瞄配置.ini");
  if (file.is_open()) {
    file << "启用自瞄=" << Function::AiMBot::AiMBotState << "\n";
    file << "自瞄按键=" << Function::AiMBot::AimKey << "\n";
    file << "总平滑度=" << Function::AiMBot::总平滑度 << "\n";
    file << "X平滑速度=" << Function::AiMBot::smoothnessX << "\n";
    file << "Y平滑速度=" << Function::AiMBot::smoothnessY << "\n";
    file << "自瞄范围=" << Function::AiMBot::AimRange << "\n";
    file << "大范围移动限制=" << Function::AiMBot::maxSpeed << "\n";
    file << "小范围平滑系数=" << Function::AiMBot::smoothingFactor << "\n";
    file << "死区阈值=" << Function::AiMBot::deadZone << "\n";
    file << "锁定误差=" << Function::AiMBot::lockError << "\n";
    file << "预判程度=" << Function::AiMBot::Dt << "\n";
    file << "自瞄热键=" << Function::AiMBot::自瞄热键 << "\n";
  }
  file.close();
}

void 判断自瞄配置(const std::string &line) {
  std::istringstream iss(line);
  std::string token;
  std::string paramName;
  if (!std::getline(iss, token, '=')) {
    return;
  }
  paramName = token;
  if (!std::getline(iss, token)) {
    return;
  }

  if (paramName == "启用自瞄") {
    Function::AiMBot::AiMBotState = stoi(token);
    return;
  }
  if (paramName == "自瞄按键") {
    strcpy(Function::AiMBot::AimKey, token.c_str());
    return;
  }
  if (paramName == "总平滑度") {
    Function::AiMBot::总平滑度 = stoi(token);
    return;
  }
  if (paramName == "X平滑速度") {
    Function::AiMBot::smoothnessX = stof(token);
    return;
  }
  if (paramName == "Y平滑速度" || paramName == "y平滑速度") {
    Function::AiMBot::smoothnessY = stof(token);
    return;
  }
  if (paramName == "自瞄范围") {
    Function::AiMBot::AimRange = stof(token);
    return;
  }
  if (paramName == "大范围移动限制") {
    Function::AiMBot::maxSpeed = stof(token);
    return;
  }
  if (paramName == "小范围平滑系数") {
    Function::AiMBot::smoothingFactor = stof(token);
    return;
  }
  if (paramName == "死区阈值") {
    Function::AiMBot::deadZone = stof(token);
    return;
  }
  if (paramName == "锁定误差") {
    Function::AiMBot::lockError = stof(token);
    return;
  }
  if (paramName == "预判程度" || paramName == "预值") {
    Function::AiMBot::Dt = stof(token);
    return;
  }
  if (paramName == "自瞄热键") {
    Function::AiMBot::自瞄热键 = stoi(token);
  }
}

void 判断其他配置(const std::string &line) {
  判断自瞄配置(line);
  std::istringstream iss(line);
  std::string token;
  std::string paramName;
  if (std::getline(iss, token, '=')) {
    paramName = token;
    if (std::getline(iss, token)) {
      if (paramName == "半自动解键按键") {
        Function::半自动博弈::解键按键 = stoi(token);
      }
      if (strstr(paramName.c_str(), "骨骼") != NULL) {
        Function::ESP::骨骼 = stoi(token);
      }
      if (strstr(paramName.c_str(), "人机颜色") != NULL) {

        float r, g, b, a;
        char comma;
        std::istringstream colorStream(token);
        if (colorStream >> r >> comma >> g >> comma >> b >> comma >> a) {
          Function::ESP::人机颜色 = ImColor(r, g, b, a);
        }
      }
      if (strstr(paramName.c_str(), "玩家颜色") != NULL) {

        float r, g, b, a;
        char comma;
        std::istringstream colorStream(token);
        if (colorStream >> r >> comma >> g >> comma >> b >> comma >> a) {
          Function::ESP::玩家颜色 = ImColor(r, g, b, a);
        }
      }
      if (strstr(paramName.c_str(), "方框") != NULL) {
        Function::ESP::方框 = stoi(token);
      }
      if (strstr(paramName.c_str(), "信息") != NULL) {
        Function::ESP::信息 = stoi(token);
      }
      if (strstr(paramName.c_str(), "血甲") != NULL) {
        Function::ESP::血条 = stoi(token);
      }
      if (strstr(paramName.c_str(), "手持") != NULL) {
        Function::ESP::手持 = stoi(token);
      }
      if (strstr(paramName.c_str(), "预警") != NULL) {
        Function::ESP::预警 = stoi(token);
      }
      if (strstr(paramName.c_str(), "射线") != NULL) {
        Function::ESP::射线 = stoi(token);
      }
      if (strstr(paramName.c_str(), "灵魂") != NULL) {
        Function::ESP::灵魂 = stoi(token);
      }
      if (strstr(paramName.c_str(), "血条样式") != NULL) {
        Function::ESP::血条样式 = stoi(token);
      }
      if (strstr(paramName.c_str(), "盲区样式") != NULL) {
        Function::ESP::预警样式 = stoi(token);
      }
      if (strstr(paramName.c_str(), "daoJuTextSize") != NULL) {
        Function::ESP::daoJuTextSize = stoi(token);
      }
      if (strstr(paramName.c_str(), "userTextSize") != NULL) {
        Function::ESP::userTextSize = stoi(token);
      }

      if (strstr(paramName.c_str(), "近战-白") != NULL) {
        Function::ESP::近战_白 = stoi(token);
      }
      if (strstr(paramName.c_str(), "近战-蓝") != NULL) {
        Function::ESP::近战_蓝 = stoi(token);
      }
      if (strstr(paramName.c_str(), "近战-紫") != NULL) {
        Function::ESP::近战_紫 = stoi(token);
      }
      if (strstr(paramName.c_str(), "近战-金") != NULL) {
        Function::ESP::近战_金 = stoi(token);
      }
      if (strstr(paramName.c_str(), "远程-白") != NULL) {
        Function::ESP::远程_白 = stoi(token);
      }
      if (strstr(paramName.c_str(), "远程-蓝") != NULL) {
        Function::ESP::远程_蓝 = stoi(token);
      }
      if (strstr(paramName.c_str(), "远程-紫") != NULL) {
        Function::ESP::远程_紫 = stoi(token);
      }
      if (strstr(paramName.c_str(), "远程-金") != NULL) {
        Function::ESP::远程_金 = stoi(token);
      }
      if (strstr(paramName.c_str(), "道具") != NULL) {
        Function::ESP::道具 = stoi(token);
      }
      if (strstr(paramName.c_str(), "扩容") != NULL) {
        Function::ESP::扩容 = stoi(token);
      }
      if (strstr(paramName.c_str(), "金魂") != NULL) {
        Function::ESP::金魂 = stoi(token);
      }
      if (strstr(paramName.c_str(), "夺魂") != NULL) {
        Function::ESP::夺魂 = stoi(token);
      }
      if (strstr(paramName.c_str(), "属性") != NULL) {
        Function::ESP::属性 = stoi(token);
      }
      if (strstr(paramName.c_str(), "盒子") != NULL) {
        Function::ESP::盒子 = stoi(token);
      }
      if (strstr(paramName.c_str(), "白甲") != NULL) {
        Function::ESP::护甲Lv1 = stoi(token);
      }
      if (strstr(paramName.c_str(), "蓝甲") != NULL) {
        Function::ESP::护甲Lv2 = stoi(token);
      }
      if (strstr(paramName.c_str(), "紫甲") != NULL) {
        Function::ESP::护甲Lv3 = stoi(token);
      }
      if (strstr(paramName.c_str(), "金甲") != NULL) {
        Function::ESP::护甲Lv4 = stoi(token);
      }
      if (strstr(paramName.c_str(), "红甲") != NULL) {
        Function::ESP::护甲Lv5 = stoi(token);
      }
      if (strstr(paramName.c_str(), "换甲护甲") != NULL) {
        Function::ESP::换甲护甲 = stoi(token);
      }
      if (strstr(paramName.c_str(), "果实") != NULL) {
        Function::ESP::果实 = stoi(token);
      }
      if (strstr(paramName.c_str(), "萤火虫") != NULL) {
        Function::ESP::萤火虫 = stoi(token);
      }
      if (strstr(paramName.c_str(), "金堆") != NULL) {
        Function::ESP::金堆 = stoi(token);
      }
      if (strstr(paramName.c_str(), "蓝堆") != NULL) {
        Function::ESP::蓝堆 = stoi(token);
      }
      if (strstr(paramName.c_str(), "绿堆") != NULL) {
        Function::ESP::绿堆 = stoi(token);
      }
      if (strstr(paramName.c_str(), "任务") != NULL) {
        Function::ESP::任务 = stoi(token);
      }
      if (strstr(paramName.c_str(), "秘籍") != NULL) {
        Function::ESP::秘籍 = stoi(token);
      }
      if (strstr(paramName.c_str(), "所有对象") != NULL) {
        Function::ESP::所有对象 = stoi(token);
      }

      if (strstr(paramName.c_str(), "启用振刀") != NULL) {
        Function::Shock::F_ShockKnife = stoi(token);
      }
      if (strstr(paramName.c_str(), "内存振逻辑") != NULL) {
        Function::Shock::MemoryShockMode = 0;
      }
      if (paramName == "内存振变态模式") {
        Hook::SetMemoryShockBerserkMode(stoi(token) != 0 && Function::Shock::M_ShockKnife);
      }
      if (paramName == "动画加速") {
        Function::Shock::动画加速 = stoi(token) != 0;
      }
      if (paramName == "动画加速倍率") {
        Function::Shock::动画加速倍率 = std::clamp(static_cast<float>(atof(token.c_str())), 0.1f, 50.0f);
        Hook::SetCrossFadeSpeed(Function::Shock::动画加速倍率);
      }
      if (strstr(paramName.c_str(), "内存振刀") != NULL) {
        Function::Shock::M_ShockKnife = stoi(token);
        if (!Function::Shock::M_ShockKnife) {
          Hook::SetMemoryShockBerserkMode(false);
        }
      }
      if (strstr(paramName.c_str(), "扬鞭劲振刀") != NULL) {
        Function::Shock::扬鞭劲振刀 = stoi(token);
      }
      if (strstr(paramName.c_str(), "横栏振刀") != NULL) {
        Function::Shock::横栏振刀 = stoi(token);
      }
      if (strstr(paramName.c_str(), "横刀左蓄振刀") != NULL) {
        Function::Shock::横刀左蓄振刀 = stoi(token);
      }
      if (strstr(paramName.c_str(), "只振视野范围内") != NULL) {
        Function::Shock::只振视野范围内 = stoi(token);
      }
      if (strstr(paramName.c_str(), "振刀按键") != NULL) {
        Function::Shock::振刀按键 = stoi(token);
      }
      if (strstr(paramName.c_str(), "切拳按键") != NULL) {
        Function::Shock::切拳按键 = stoi(token);
      }
      if (strstr(paramName.c_str(), "滞空按键") != NULL) {
        Function::Shock::滞空按键 = stoi(token);
      }
      if (strstr(paramName.c_str(), "是否闪避") != NULL) {
        Function::Shock::Flag_ShortDodge = stoi(token);
      }
      if (strstr(paramName.c_str(), "是否拼刀") != NULL) {
        Function::Shock::Flag_CollideKnife = stoi(token);
      }
      if (strstr(paramName.c_str(), "输出调试执行") != NULL) {
        调试::输出调试执行 = stoi(token);
      }
      if (strstr(paramName.c_str(), "输出逻辑执行") != NULL) {
        调试::输出逻辑执行 = stoi(token);
      }
      if (paramName == "按键方式") {
        配置按键方式.store(std::clamp(stoi(token), 0, 2),
                               std::memory_order_release);
      }
      if (strstr(paramName.c_str(), "振刀方式") != NULL) {
        振刀按鍵 = stoi(token);
      }
      if (strstr(paramName.c_str(), "断蓄方式") != NULL) {
        自动断蓄 = stoi(token);
      }
      if (strstr(paramName.c_str(), "双截棍左蓄短闪振") != NULL) {
        振刀参数::特殊处理::双截棍左蓄是否短闪振 = stoi(token);
      }
      if (strstr(paramName.c_str(), "扇子左蓄中闪振") != NULL) {
        振刀参数::特殊处理::扇子左蓄是否中闪振 = stoi(token);
      }
      if (strstr(paramName.c_str(), "扇子右蓄中闪振") != NULL) {
        振刀参数::特殊处理::扇子右蓄是否中闪振 = stoi(token);
      }
      if (strstr(paramName.c_str(), "斩马刀左蓄长闪振") != NULL) {
        振刀参数::特殊处理::斩马刀左蓄是否长闪振 = stoi(token);
      }
      if (strstr(paramName.c_str(), "阔刀右蓄短闪振") != NULL) {
        振刀参数::特殊处理::阔刀右蓄是否短闪振 = stoi(token);
      }
      if (strstr(paramName.c_str(), "横刀左蓄是否只短闪") != NULL) {
        振刀参数::特殊处理::横刀左蓄是否只短闪 = stoi(token);
      }
      if (strstr(paramName.c_str(), "胡为开大振刀") != NULL) {
        振刀参数::特殊处理::胡为开大振刀 = stoi(token);
      }
      if (strstr(paramName.c_str(), "叶修开大振刀") != NULL) {
        振刀参数::特殊处理::叶修开大振刀 = stoi(token);
      }

      if (strstr(paramName.c_str(), "启用闪避") != NULL) {
        Function::HeroActivation::IsDodge = stoi(token);
      }
      if (strstr(paramName.c_str(), "技能活化") != NULL) {
        Function::HeroActivation::技能活化 = stoi(token);
      }
      if (strstr(paramName.c_str(), "躲避火男冲拳") != NULL) {
        Function::HeroActivation::躲避火男冲拳 = stoi(token);
      }
      if (strstr(paramName.c_str(), "躲避火男F") != NULL) {
        Function::HeroActivation::躲避火男F = stoi(token);
      }
      if (strstr(paramName.c_str(), "躲避妖刀大招") != NULL) {
        Function::HeroActivation::躲避妖刀大招 = stoi(token);
      }
      if (strstr(paramName.c_str(), "躲避顾清寒V1V2") != NULL) {
        Function::HeroActivation::躲避顾清寒V1V2 = stoi(token);
      }
      if (strstr(paramName.c_str(), "躲避武田F") != NULL) {
        Function::HeroActivation::躲避武田F = stoi(token);
      }
      if (strstr(paramName.c_str(), "躲避胡为") != NULL) {
        Function::HeroActivation::躲避胡为 = stoi(token);
      }
      if (strstr(paramName.c_str(), "躲避宁红叶F") != NULL) {
        Function::HeroActivation::躲避宁红叶F = stoi(token);
      }
      if (strstr(paramName.c_str(), "躲避狐狸大招") != NULL) {
        Function::HeroActivation::躲避狐狸大招 = stoi(token);
      }
      if (strstr(paramName.c_str(), "躲避乌真大招") != NULL) {
        Function::HeroActivation::躲避乌真大招 = stoi(token);
      }
      if (strstr(paramName.c_str(), "躲避三娘V2") != NULL) {
        Function::HeroActivation::躲避三娘V2 = stoi(token);
      }
      if (strstr(paramName.c_str(), "躲避哈迪大招") != NULL) {
        Function::HeroActivation::躲避哈迪大招 = stoi(token);
      }
      if (strstr(paramName.c_str(), "躲避岳山F") != NULL) {
        Function::HeroActivation::躲避岳山F = stoi(token);
      }
      if (strstr(paramName.c_str(), "躲避魏轻F1") != NULL) {
        Function::HeroActivation::躲避魏轻F1 = stoi(token);
      }
      if (strstr(paramName.c_str(), "躲避魏轻大招") != NULL) {
        Function::HeroActivation::躲避魏轻V = stoi(token);
      }
      if (strstr(paramName.c_str(), "躲避迦南F2") != NULL) {
        Function::HeroActivation::躲避迦南F2 = stoi(token);
      }
      if (strstr(paramName.c_str(), "躲避迦南V2") != NULL) {
        Function::HeroActivation::躲避迦南V2 = stoi(token);
      }
      if (strstr(paramName.c_str(), "躲避刘炼V1V2") != NULL) {
        Function::HeroActivation::躲避刘炼V = stoi(token);
      }
      if (strstr(paramName.c_str(), "躲避张起灵F2V1V2") != NULL) {
        Function::HeroActivation::躲避张起灵 = stoi(token);
      }
      if (strstr(paramName.c_str(), "躲避无尘V2") != NULL) {
        Function::HeroActivation::躲避无尘V2 = stoi(token);
      }
      if (strstr(paramName.c_str(), "躲避季盈盈V蓄力") != NULL) {
          Function::HeroActivation::躲避季盈盈V蓄力 = stoi(token);
      }

      if (strstr(paramName.c_str(), "蓄力抓振刀") != NULL) {
        Function::LogicKnife::BlueAttackShock = stoi(token);
      }
      if (strstr(paramName.c_str(), "蓄力抓白刀") != NULL) {
        Function::LogicKnife::BlueAttackWrite = stoi(token);
      }
      if (strstr(paramName.c_str(), "抓闪") != NULL) {
        Function::LogicKnife::BlueAttackdodge = stoi(token);
      }
      if (strstr(paramName.c_str(), "蓝刀") != NULL) {
        Function::LogicKnife::BlueAttacklanding = stoi(token);
      }
      if (strstr(paramName.c_str(), "自动断蓄") != NULL) {
        Function::LogicKnife::BlueAttackDuanXu = stoi(token);
      }
      if (strstr(paramName.c_str(), "自动辅助连招") != NULL) {
        Function::LogicKnife::AutoCombo = stoi(token);
      }
      if (strstr(paramName.c_str(), "出蓄抓振延迟") != NULL) {
        活化参数::出蓄抓振延迟 = stoi(token);
      }
      if (strstr(paramName.c_str(), "蓝顶延迟") != NULL) {
        活化参数::蓝顶延迟 = stoi(token);
      }
      if (strstr(paramName.c_str(), "自动毛刀") != NULL) {
        Function::LogicKnife::auto_pickup_weapon = stoi(token);
      }
      if (strstr(paramName.c_str(), "双持快速c切") != NULL) {
        Function::LogicKnife::BlueAttackDuanXu_fast = stoi(token);
      }
      if (strstr(paramName.c_str(), "不振3A") != NULL) {
        Function::LogicKnife::NO_Shock_3A = stoi(token);
      }
      if (strstr(paramName.c_str(), "打药时只振闪贴脸敌人") != NULL) {
        Function::LogicKnife::MedicineOnlyShockDodgeCloseEnemy = stoi(token);
      }

      // 数据遍历
      if (strstr(paramName.c_str(), "开启数据遍历") != NULL) {
        Function::Flag::数据遍历 = stoi(token);
      }
      if (strstr(paramName.c_str(), "人物数据") != NULL) {
        Function::Flag::人物数据 = stoi(token);
      }
      if (strstr(paramName.c_str(), "全局数据") != NULL) {
        Function::Flag::全局数据 = stoi(token);
      }
      if (strstr(paramName.c_str(), "对象数据") != NULL) {
        Function::Flag::对象数据 = stoi(token);
      }
      if (strstr(paramName.c_str(), "堆堆数据") != NULL) {
        Function::Flag::堆堆数据 = stoi(token);
      }
      if (strstr(paramName.c_str(), "物品数据") != NULL) {
        Function::Flag::物品数据 = stoi(token);
      }
      if (strstr(paramName.c_str(), "打印物品ID") != NULL) {
        Function::Flag::打印物品ID = stoi(token);
      }
      if (strstr(paramName.c_str(), "打印魂玉ID") != NULL) {
        Function::Flag::打印魂玉ID = stoi(token);
      }
      if (strstr(paramName.c_str(), "显示未知物品ID") != NULL) {
        Function::Flag::显示未知物品ID = stoi(token);
      }
      if (strstr(paramName.c_str(), "显示未知堆ID") != NULL) {
        Function::Flag::显示未知堆ID = stoi(token);
      }
    }
  }
}
void 读取其他配置() {
  ConfigDefaults::ApplySemiAutoConfig();
  ConfigDefaults::ApplySideGameConfig();
  std::ifstream file("其他配置.ini");
  if (file.is_open()) {
    std::string line;
    while (std::getline(file, line)) {
      判断其他配置(line);
    }

    file.close();
  }
}

void 读取自瞄配置() {
  ConfigDefaults::ApplyAimbotConfig();
  std::ifstream file("自瞄配置.ini");
  if (file.is_open()) {
    std::string line;
    while (std::getline(file, line)) {
      判断自瞄配置(line);
    }

    file.close();
  }
}

void ConfigDefaults::ApplyAimbotConfig() {
  Function::AiMBot::AiMBotState = true;
  strcpy(Function::AiMBot::AimKey, "17");
  Function::AiMBot::smoothnessX = 1.42f;
  Function::AiMBot::smoothnessY = 1.0f;
  Function::AiMBot::AimRange = 200.0f;
  Function::AiMBot::maxSpeed = 25.0f;
  Function::AiMBot::smoothingFactor = 0.4f;
  Function::AiMBot::deadZone = 0.25f;
  Function::AiMBot::lockError = 1.0f;
  Function::AiMBot::Dt = 1.1f;
  Function::AiMBot::总平滑度 = 1;
  Function::AiMBot::自瞄热键 = 1;
}

void 重置自瞄配置() {
  ConfigDefaults::ApplyAimbotConfig();
  保存自瞄配置();
  读取自瞄配置();
}

ImFont *medium;
ImFont *bold;
ImFont *tab_icons;
ImFont *logo;
ImFont *tab_title;
ImFont *tab_title_icon;
ImFont *subtab_title;
ImFont *combo_arrow;
bool connectIP = true;
enum heads { 启动 };

enum sub_heads {
  透视配置,
  振刀配置,
  物品配置,
  活化配置,
  博弈配置,
  博弈连招,
  自瞄配置,
  杂项配置,
  半自动配置
};

void SetCustomTheme() {
  ImGuiIO &io = ImGui::GetIO();
  ImGuiStyle &style = ImGui::GetStyle();
  ImVec4 *colors = style.Colors;

  // [FIX] Force Rounding for Neverlose Style
  style.WindowRounding = 10.0f;
  style.ChildRounding = 6.0f;
  style.FrameRounding = 6.0f;
  style.PopupRounding = 6.0f;
  style.ScrollbarRounding = 10.0f;
  style.GrabRounding = 6.0f;

  // [Fix] Removed conflicting style overrides to prevent "Purple Flash".
  // The global style is already set by SetupNeverloseStyle() in
  // OS-ImGui_External.cpp. We only need to initialize the global font pointers
  // here.

  // 字体回退，避免空指针
  ImFont *fallbackFont = ImGui::GetFont();
  if (!fallbackFont && !io.Fonts->Fonts.empty()) {
    fallbackFont = io.Fonts->Fonts[0];
  }
  medium = fallbackFont;
  bold = fallbackFont;
  tab_icons = fallbackFont;
  tab_title = fallbackFont;
  tab_title_icon = fallbackFont;
  subtab_title = fallbackFont;
  combo_arrow = fallbackFont;
}
static bool 隐藏界面 = false, 初始化读取 = false;

void 透视配置GUI() {

  const char *血条样式[3] = {u8"自定义 0", u8"自定义 1", u8"自定义 2"};
  const char *预警样式[2] = {u8"自定义 0", u8"自定义 1"};

  ImGui::SetCursorPos({12, 12});

  // 玩家部分
  e_elements::begin_child(u8"玩家", ImVec2(250, 450));
  {
    gui.checkbox(u8"方框", &Function::ESP::方框);
    // gui.checkbox(u8"骨骼", &Function::ESP::骨骼);
    gui.checkbox(u8"射线", &Function::ESP::射线);
    gui.checkbox(u8"信息", &Function::ESP::信息);
    gui.checkbox(u8"名字", &Function::ESP::名字);
    gui.checkbox(u8"血甲", &Function::ESP::血条);
    gui.checkbox(u8"手持", &Function::ESP::手持);
    gui.checkbox(u8"盲区预警", &Function::ESP::预警);
    gui.checkbox(u8"灵魂", &Function::ESP::灵魂);
    gui.combo(u8"血甲样式", &Function::ESP::血条样式, 血条样式,
              IM_ARRAYSIZE(血条样式));
    gui.combo(u8"预警样式", &Function::ESP::预警样式, 预警样式,
              IM_ARRAYSIZE(预警样式));
    ImGui::ColorEdit4(u8"人机颜色",
                      reinterpret_cast<float *>(&Function::ESP::人机颜色),
                      ImGuiColorEditFlags_NoInputs);
    ImGui::ColorEdit4(u8"玩家颜色",
                      reinterpret_cast<float *>(&Function::ESP::玩家颜色),
                      ImGuiColorEditFlags_NoInputs);
    ImGui::ColorEdit4(u8"名字颜色",
                      reinterpret_cast<float *>(&Function::ESP::名字颜色),
                      ImGuiColorEditFlags_NoInputs);
    ImGui::ColorEdit4(u8"信息颜色",
                      reinterpret_cast<float *>(&Function::ESP::信息颜色),
                      ImGuiColorEditFlags_NoInputs);
  }
  e_elements::end_child();

  ImGui::SameLine();

  // 物品部分
  e_elements::begin_child(u8"物品", ImVec2(420, 450));
  {

    // 下拉框内容
    const char *items[] = {u8"只显示未开扩容类型", u8"全显示扩容",
                           u8"不显示扩容"};
    static int currentItem = 0; // 选中的下拉框项，默认选中第一个
    bool isTestStart = true;
    if (gui.combo(u8"扩容显示", &currentItem, items, IM_ARRAYSIZE(items))) {
      if (currentItem == 0) {
        Function::ESP::智能扩容 = true;
        Function::ESP::扩容 = false;
      } else if (currentItem == 1) {
        Function::ESP::智能扩容 = false;
        Function::ESP::扩容 = true;
      } else {
        if (isTestStart) {
          // MyLog("屏蔽左右键");
          isTestStart = false;
          // BlockLeftandRight();
          // kmNet_mask_keyboard(KEY_A);
        }
        Function::ESP::智能扩容 = false;
        Function::ESP::扩容 = false;
      }
    }

    gui.slider_float(u8"道具文字大小", &Function::ESP::daoJuTextSize, 5.f,
                     20.f);
    gui.slider_float(u8"人物文字大小", &Function::ESP::userTextSize, 5.f, 20.f);

    gui.checkbox(u8"近战-白", &Function::ESP::近战_白);
    ImGui::SameLine();
    gui.checkbox(u8"近战-蓝", &Function::ESP::近战_蓝);
    ImGui::SameLine();
    gui.checkbox(u8"近战-紫", &Function::ESP::近战_紫);
    ImGui::SameLine();
    gui.checkbox(u8"近战-金", &Function::ESP::近战_金);

    gui.checkbox(u8"远程-白", &Function::ESP::远程_白);
    ImGui::SameLine();
    gui.checkbox(u8"远程-蓝", &Function::ESP::远程_蓝);
    ImGui::SameLine();
    gui.checkbox(u8"远程-紫", &Function::ESP::远程_紫);
    ImGui::SameLine();
    gui.checkbox(u8"远程-金", &Function::ESP::远程_金);

    gui.checkbox(u8"   魂冢", &Function::ESP::盒子);
    ImGui::SameLine();
    gui.checkbox(u8"   金魂", &Function::ESP::金魂);
    ImGui::SameLine();
    gui.checkbox(u8"   夺魂", &Function::ESP::夺魂);
    // ImGui::SameLine();
    // gui.checkbox(u8"   扩容", &Function::ESP::扩容);

    gui.checkbox(u8" 萤火虫", &Function::ESP::萤火虫);
    ImGui::SameLine();
    gui.checkbox(u8"   果实", &Function::ESP::果实);
    ImGui::SameLine();
    gui.checkbox(u8"   金堆", &Function::ESP::金堆);
    ImGui::SameLine();
    gui.checkbox(u8"   蓝堆", &Function::ESP::蓝堆);

    gui.checkbox(u8"   绿堆", &Function::ESP::绿堆);
    ImGui::SameLine();
    gui.checkbox(u8"   任务", &Function::ESP::任务);
    ImGui::SameLine();
    gui.checkbox(u8"   白甲", &Function::ESP::护甲Lv1);
    ImGui::SameLine();
    gui.checkbox(u8"   蓝甲", &Function::ESP::护甲Lv2);

    gui.checkbox(u8"   紫甲", &Function::ESP::护甲Lv3);
    ImGui::SameLine();
    gui.checkbox(u8"   金甲", &Function::ESP::护甲Lv4);
    ImGui::SameLine();
    gui.checkbox(u8"   红甲", &Function::ESP::护甲Lv5);

    // gui.checkbox(u8"低护甲时护甲全显示", &Function::ESP::换甲护甲);
    ImGui::SameLine();
    gui.checkbox(u8"   道具", &Function::ESP::道具);
    ImGui::SameLine();
    gui.checkbox(u8"神鹰堡解密", &Function::ESP::神鹰堡解密);

    // gui.checkbox(u8"   秘籍", &Function::ESP::秘籍);
    // ImGui::Checkbox(u8"   世界对象", &Function::ESP::所有对象);
    ImGui::SameLine();
  }
  e_elements::end_child();
}
void 振刀配置GUI() {
  static int 振刀页面标签 = 0;
  const bool 显示闪避参数页 = 振刀页面标签 == 1;

  auto 绘制页面按钮 = [&](const char *label, int target, float width) {
    const bool selected = 振刀页面标签 == target;
    ImGui::PushStyleColor(ImGuiCol_Button,
                          selected ? ImVec4(0.23f, 0.56f, 0.97f, 0.95f)
                                   : ImVec4(0.16f, 0.17f, 0.20f, 0.95f));
    ImGui::PushStyleColor(ImGuiCol_ButtonHovered,
                          selected ? ImVec4(0.28f, 0.61f, 1.0f, 1.0f)
                                   : ImVec4(0.22f, 0.24f, 0.29f, 1.0f));
    ImGui::PushStyleColor(ImGuiCol_ButtonActive,
                          selected ? ImVec4(0.18f, 0.48f, 0.9f, 1.0f)
                                   : ImVec4(0.20f, 0.22f, 0.27f, 1.0f));
    ImGui::PushStyleColor(ImGuiCol_Border,
                          selected ? ImVec4(0.36f, 0.67f, 1.0f, 0.90f)
                                   : ImVec4(0.28f, 0.30f, 0.35f, 0.90f));
    ImGui::PushStyleColor(ImGuiCol_Text,
                          selected ? ImVec4(1.0f, 1.0f, 1.0f, 1.0f)
                                   : ImVec4(0.72f, 0.75f, 0.82f, 1.0f));
    ImGui::PushStyleVar(ImGuiStyleVar_FrameBorderSize, 1.0f);
    ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 6.0f);

    if (gui.button(label, ImVec2(width, 30.f))) {
      振刀页面标签 = target;
    }

    if (selected) {
      ImDrawList *draw = ImGui::GetWindowDrawList();
      ImVec2 min = ImGui::GetItemRectMin();
      ImVec2 max = ImGui::GetItemRectMax();
      draw->AddRectFilled(ImVec2(min.x + 10.f, max.y - 3.f),
                          ImVec2(max.x - 10.f, max.y - 1.f),
                          ImColor(115, 183, 255, 255), 2.0f);
    }

    ImGui::PopStyleVar(2);
    ImGui::PopStyleColor(5);
  };

  ImGui::SetCursorPos({12, 12});
  绘制页面按钮(u8"振刀", 0, 96.f);
  ImGui::SameLine();
  绘制页面按钮(u8"闪避", 1, 96.f);

  if (!显示闪避参数页) {
    ImGui::SetCursorPos({12, 50});
    e_elements::begin_child(u8"选择", ImVec2(240, 360));
    {
      if (gui.checkbox(u8"模拟振刀", &Function::Shock::F_ShockKnife)) {
        if (Function::Shock::F_ShockKnife) {
          Function::Shock::M_ShockKnife = false;
          Hook::SetMemoryShockEnabled(false);
          Hook::SetMemoryShockBerserkMode(false);
          读取模拟振配置();
        }
      }
      gui.checkbox(u8"自动毛刀", &Function::LogicKnife::auto_pickup_weapon);
      gui.checkbox(u8"不振3A", &Function::LogicKnife::NO_Shock_3A);
      gui.checkbox(u8"打药/救队友时只 振/闪 贴脸敌人",
                   &Function::LogicKnife::MedicineOnlyShockDodgeCloseEnemy);
      /*ImGui::RadioButton(u8"G键振刀", &振刀按鍵, 0);
      ImGui::SameLine();
      ImGui::RadioButton(u8"下侧键", &振刀按鍵, 1);*/

      static bool waitingGameHotkeyZd = false;
      static bool waitingGameHotkeyQq = false;
      static bool waitingGameHotkeyZk = false;
      static ULONGLONG waitingGameHotkeyZdStart = 0;
      static ULONGLONG waitingGameHotkeyQqStart = 0;
      auto GetGameHotkeyName = [](int key) {
        if (key < 0) {
          return std::string(GetMouseVkName(-key));
        }
        char name[64]{};
        int vkKey = HidToVkKey(key);
        if (vkKey != 0 &&
            GetKeyNameTextA(MapVirtualKeyA(vkKey, MAPVK_VK_TO_VSC) << 16, name,
                            sizeof(name))) {
          return std::string(name);
        }
        return std::to_string(key);
      };
      auto DrawGameHotkey = [&](const char *label, int *keyValue,
                                bool *waiting, ULONGLONG *waitingStart,
                                const char *id) {
        std::string gameHotkeyName = GetGameHotkeyName(*keyValue);
        ImGui::AlignTextToFramePadding();
        ImGui::Text(label);
        ImGui::SameLine(80);
        if (*waiting) {
          ImGui::TextColored(ImVec4(1, 0.8f, 0.2f, 1), u8"[请按键...]");
          if (GetTickCount64() - *waitingStart > 150) {
            for (int key = 0x01; key <= 0xFE; ++key) {
              if (GetAsyncKeyState(key) & 0x8000) {
                if (IsMouseVkKey(key)) {
                  *keyValue = -key;
                  *waiting = false;
                  Sleep(150);
                  break;
                }
                int hidKey = VkToHidKey(key);
                if (hidKey != 0) {
                  *keyValue = hidKey;
                  *waiting = false;
                  Sleep(150);
                  break;
                }
              }
            }
          }
          if (GetAsyncKeyState(VK_ESCAPE) & 0x8000) {
            *waiting = false;
            Sleep(150);
          }
        } else {
          ImGui::PushID(id);
          if (gui.button(gameHotkeyName.c_str(), ImVec2(100, 0))) {
            *waiting = true;
            *waitingStart = GetTickCount64();
          }
          ImGui::PopID();
        }
      };
      auto GetVkHotkeyName = [](int hotkey) {
        char name[64]{};
        if (GetKeyNameTextA(MapVirtualKeyA(hotkey, MAPVK_VK_TO_VSC) << 16,
                            name, sizeof(name))) {
          return std::string(name);
        }
        return std::to_string(hotkey);
      };
      auto DrawVkHotkey = [&](const char *label, int &hotkey,
                              bool &waitingForKey, const char *id) {
        std::string hotkeyName = GetVkHotkeyName(hotkey);
        ImGui::AlignTextToFramePadding();
        ImGui::Text(label);
        ImGui::SameLine(80);
        ImGui::PushID(id);
        if (waitingForKey) {
          ImGui::TextColored(ImVec4(1, 0.8f, 0.2f, 1), u8"[请按键...]");
          for (int key = 0x01; key <= 0xFE; ++key) {
            if (key != 179 && (GetAsyncKeyState(key) & 0x8000)) {
              hotkey = key;
              waitingForKey = false;
              Sleep(150);
              break;
            }
          }
          if (GetAsyncKeyState(VK_ESCAPE) & 0x8000) {
            waitingForKey = false;
            Sleep(150);
          }
        } else {
          if (gui.button(hotkeyName.c_str(), ImVec2(100, 0))) {
            waitingForKey = true;
          }
        }
        ImGui::PopID();
      };
      DrawGameHotkey(u8"振刀键", &Function::Shock::振刀按键,
                     &waitingGameHotkeyZd, &waitingGameHotkeyZdStart,
                     "shock_key");
      DrawGameHotkey(u8"切拳键", &Function::Shock::切拳按键,
                     &waitingGameHotkeyQq, &waitingGameHotkeyQqStart,
                     "fist_key");
      DrawVkHotkey(u8"滞空键", Function::Shock::滞空按键,
                   waitingGameHotkeyZk, "air_combo_key");

      gui.checkbox(u8"只振视野范围内", &Function::Shock::只振视野范围内);
      if (gui.checkbox(u8"初始化内存振-需开启双键振刀", &Function::Shock::M_ShockKnife)) {
        Hook::SetMemoryShockEnabled(false);
        if (Function::Shock::M_ShockKnife) {
          Function::Shock::F_ShockKnife = false;
        } else {
          Hook::SetMemoryShockBerserkMode(false);
        }
      }
      if (!Function::Shock::M_ShockKnife && Function::Shock::内存振变态模式) {
        Hook::SetMemoryShockBerserkMode(false);
      }
      if (!Function::Shock::M_ShockKnife) {
        ImGui::BeginDisabled();
      }
      const bool berserkChanged =
          gui.checkbox(u8"内存振变态模式", &Function::Shock::内存振变态模式);
      if (!Function::Shock::M_ShockKnife) {
        ImGui::EndDisabled();
      }
      if (berserkChanged) {
        Hook::SetMemoryShockBerserkMode(Function::Shock::内存振变态模式);
      }
      ImGui::Dummy(ImVec2(0, 10));
      if constexpr (kShowCrossFadeSpeedControls) {
        if (gui.checkbox(u8"动画加速", &Function::Shock::动画加速)) {
          Hook::EnableCrossFadeSpeedHook(Function::Shock::动画加速);
        }
        if (gui.slider_float(u8"动画倍率", &Function::Shock::动画加速倍率, 0.1f,
                             50.0f, "%.1f")) {
          Hook::SetCrossFadeSpeed(Function::Shock::动画加速倍率);
        }
      }
      Function::Shock::MemoryShockMode = 0;
      if constexpr (kShowMemoryShockModeControl) {
        const char *memoryShockModes[2] = {u8"CrossFade动作加速", u8"Legacy常驻写入"};
        if (gui.combo(u8"内存振逻辑", &Function::Shock::MemoryShockMode,
                      memoryShockModes, IM_ARRAYSIZE(memoryShockModes))) {
          Function::Shock::MemoryShockMode = 0;
          Hook::SetMemoryShockEnabled(false);
          Hook::SetMemoryShockBerserkMode(Function::Shock::内存振变态模式);
        }
      }
      gui.checkbox(u8"振横刀左蓄", &Function::Shock::横刀左蓄振刀);
      // ImGui::Checkbox(u8"是否闪避", &Function::Shock::Flag_ShortDodge);
      // ImGui::Checkbox(u8"是否拼刀", &Function::Shock::Flag_CollideKnife);
      gui.checkbox(u8"技能活化", &Function::HeroActivation::技能活化);
      /*if (ImGui::IsItemHovered())
      {
              ImGui::SetTooltip(u8"数值越高，振刀时间点越早，适用于能站振招式");
      }*/
      /*ImGui::RadioButton("KmBox-Net", &按键方式, 0);
      ImGui::SameLine();
      ImGui::RadioButton("KmBox-Pro", &按键方式, 1);*/
    }
    e_elements::end_child();

    ImGui::SetCursorPos({12, 422});
    e_elements::begin_child(u8"特殊处理", ImVec2(220, 184));
    {
      gui.checkbox(u8"胡为开大振刀", &振刀参数::特殊处理::胡为开大振刀);
      gui.checkbox(u8"叶修开大振刀", &振刀参数::特殊处理::叶修开大振刀);
      // ImGui::Checkbox(u8"扇子左蓄中闪振",
      // &振刀参数::特殊处理::扇子左蓄是否中闪振);
      // ImGui::Checkbox(u8"扇子右蓄中闪振 ",
      // &振刀参数::特殊处理::扇子右蓄是否中闪振);
      // ImGui::Checkbox(u8"斩马刀左蓄长闪振",
      // &振刀参数::特殊处理::斩马刀左蓄是否长闪振);
      // ImGui::Checkbox(u8"阔刀右蓄短闪振",
      // &振刀参数::特殊处理::阔刀右蓄是否短闪振);
      // ImGui::Checkbox(u8"横刀是否只短闪",
      // &振刀参数::特殊处理::横刀左蓄是否只短闪);
    }
    e_elements::end_child();
  }

  ImGui::SetCursorPos(显示闪避参数页 ? ImVec2(12, 50) : ImVec2(255, 50));
  e_elements::begin_child(u8"参数",
                          显示闪避参数页 ? ImVec2(643, 556)
                                         : ImVec2(400, 556));
  {
    if (显示闪避参数页) {
      gui.slider_float(u8"全局延迟修正", &闪避参数::全局延迟修正, 0.f,
                       0.2f);
      ImGui::Spacing();
    } else {
      gui.slider_float(u8"振刀全局延迟修正",
                       &Function::Shock::振刀全局延迟修正, 0.f, 2.f);
      ImGui::Spacing();
    }

    // { 0"拼刀", 1"站震", 2"跳震", 3"后闪震", 4"闪避" , 5"后闪修刀震",
    // 6"强行站振", 7"智能振/闪/拼刀"};
    static const char *振刀方式列表[] = {
        u8"直接拼刀",   u8"自身1.0前站震,1.0后拼刀",
        u8"跳震",       u8"后闪震",
        u8"随机左右闪", u8"后闪修刀震",
        u8"强行站振",   u8"智能振/闪/拼刀",
        u8"AS闪",       u8"DS闪"};
    static const char *闪避方式占位列表[] = {
        u8"不处理 0", u8"智能判断 1"/*, u8"s修刀闪", u8"占位 3", u8"占位 4",
        u8"占位 5", u8"占位 6", u8"占位 7", u8"占位 8", u8"占位 9"*/};
    const char *当前方式标签 = 显示闪避参数页 ? u8"闪避方式" : u8"振刀方式";
    const char *const *当前方式列表 =
        显示闪避参数页 ? 闪避方式占位列表 : 振刀方式列表;
    const int 当前方式列表数量 =
        显示闪避参数页 ? IM_ARRAYSIZE(闪避方式占位列表)
                       : IM_ARRAYSIZE(振刀方式列表);

#define 当前距离地址(path)                                                      \
  (显示闪避参数页 ? &闪避参数::距离::path : &振刀参数::距离::path)
#define 当前延迟地址(path)                                                      \
  (显示闪避参数页 ? &闪避参数::延迟::path : &振刀参数::延迟::path)
#define 当前角度地址(path)                                                      \
  (显示闪避参数页 ? &闪避参数::角度::path : &振刀参数::角度::path)
#define 当前间隔地址(path)                                                      \
  (显示闪避参数页 ? &闪避参数::闪避::path : &振刀参数::闪避::path)
#define 当前方式地址(path)                                                      \
  (显示闪避参数页 ? &闪避参数::闪避方式::path                                  \
                  : &振刀参数::振刀方式::path)
#define 武器配置区(label)                                                       \
  if (gui.collapsing_header(label))                                             \
    for (bool _weapon_section_once = (ImGui::Indent(16.f), true);               \
         _weapon_section_once;                                                  \
         ImGui::Unindent(16.f), _weapon_section_once = false)
#define 千机伞动作配置(label, field)                                            \
  if (gui.tree_node(label)) {                                                    \
    gui.slider_float(u8"距离", 当前距离地址(千机伞.field), 1.f, 20.f);          \
    gui.slider_float(u8"延迟", 当前延迟地址(千机伞.field), 0.f, 2.f);           \
    gui.slider_float(u8"角度", 当前角度地址(千机伞.field), 1.f, 360.f);         \
    ImGui::PushItemWidth(200);                                                   \
    gui.slider_int(u8"振刀/闪避 间隔", 当前间隔地址(千机伞.field), 0, 300);     \
    gui.combo(当前方式标签, 当前方式地址(千机伞.field), 当前方式列表,            \
              当前方式列表数量);                                                \
    ImGui::TreePop();                                                            \
  }

    武器配置区(u8"长剑") {
      if (gui.tree_node(u8"左三")) {
        gui.slider_float(u8"距离", 当前距离地址(长剑.左3), 1.f, 20.f);
        gui.slider_float(u8"延迟", 当前延迟地址(长剑.左3), 0.0f, 2.f);
        gui.slider_float(u8"角度", 当前角度地址(长剑.左3), 1.f, 360.f);

        ImGui::PushItemWidth(200); // 设置滑动条的宽度
        gui.slider_int(u8"振刀/闪避 间隔", 当前间隔地址(长剑.左3), 0, 300.f);
        gui.combo(当前方式标签, 当前方式地址(长剑.左3), 当前方式列表, 当前方式列表数量);
        ImGui::TreePop();
      }
      if (gui.tree_node(u8"右三")) {
        gui.slider_float(u8"距离", 当前距离地址(长剑.右3), 1.f, 20.f);
        gui.slider_float(u8"延迟", 当前延迟地址(长剑.右3), 0.f, 2.f);
        gui.slider_float(u8"角度", 当前角度地址(长剑.右3), 1.f, 360.f);

        ImGui::PushItemWidth(200); // 设置滑动条的宽度
        gui.slider_int(u8"振刀/闪避 间隔", 当前间隔地址(长剑.右3), 0, 300.f);
        gui.combo(当前方式标签, 当前方式地址(长剑.右3), 当前方式列表, 当前方式列表数量);
        ImGui::TreePop();
      }
      if (gui.tree_node(u8"左剑气")) {
        gui.slider_float(u8"距离", 当前距离地址(长剑.左剑气), 1.f, 20.f);
        gui.slider_float(u8"延迟", 当前延迟地址(长剑.左剑气), 0.f, 2.f);
        gui.slider_float(u8"角度", 当前角度地址(长剑.左剑气), 1.f, 360.f);

        if (gui.tree_node(u8"1.0")) {
          ImGui::PushItemWidth(200); // 设置滑动条的宽度
          gui.slider_int(u8"振刀/闪避 间隔", 当前间隔地址(长剑.左剑气1), 0,
                         300.f);
          gui.combo(当前方式标签, 当前方式地址(长剑.左剑气1),
                    当前方式列表, 当前方式列表数量);
          ImGui::TreePop();
        }

        if (gui.tree_node(u8"2.0")) {
          ImGui::PushItemWidth(200); // 设置滑动条的宽度
          gui.slider_int(u8"振刀/闪避 间隔", 当前间隔地址(长剑.左剑气2), 0,
                         300.f);
          gui.combo(当前方式标签, 当前方式地址(长剑.左剑气2),
                    当前方式列表, 当前方式列表数量);
          ImGui::TreePop();
        }

        ImGui::TreePop();
      }
      if (gui.tree_node(u8"右剑气")) {
        gui.slider_float(u8"距离", 当前距离地址(长剑.右剑气), 1.f, 20.f);
        gui.slider_float(u8"延迟", 当前延迟地址(长剑.右剑气), 0.f, 2.f);
        gui.slider_float(u8"角度", 当前角度地址(长剑.右剑气), 1.f, 360.f);

        if (gui.tree_node(u8"1.0")) {
          ImGui::PushItemWidth(200); // 设置滑动条的宽度
          gui.slider_int(u8"振刀/闪避 间隔", 当前间隔地址(长剑.右剑气1), 0,
                         300.f);
          gui.combo(当前方式标签, 当前方式地址(长剑.右剑气1),
                    当前方式列表, 当前方式列表数量);
          ImGui::TreePop();
        }

        if (gui.tree_node(u8"2.0")) {
          ImGui::PushItemWidth(200); // 设置滑动条的宽度
          gui.slider_int(u8"振刀/闪避 间隔", 当前间隔地址(长剑.右剑气2), 0,
                         300.f);
          gui.combo(当前方式标签, 当前方式地址(长剑.右剑气2),
                    当前方式列表, 当前方式列表数量);
          ImGui::TreePop();
        }

        ImGui::TreePop();
      }
      if (gui.tree_node(u8"跳斩")) {
        gui.slider_float(u8"距离", 当前距离地址(长剑.跳斩), 1.f, 20.f);
        gui.slider_float(u8"延迟", 当前延迟地址(长剑.跳斩), 0.f, 2.f);
        gui.slider_float(u8"角度", 当前角度地址(长剑.跳斩), 1.f, 360.f);

        ImGui::PushItemWidth(200); // 设置滑动条的宽度
        gui.slider_int(u8"振刀/闪避 间隔", 当前间隔地址(长剑.跳斩), 0,
                       300.f);
        gui.combo(当前方式标签, 当前方式地址(长剑.跳斩), 当前方式列表, 当前方式列表数量);
        ImGui::TreePop();
      }
      if (gui.tree_node(u8"凤凰羽")) {
        gui.slider_float(u8"距离", 当前距离地址(长剑.凤凰羽), 1.f, 20.f);
        gui.slider_float(u8"延迟", 当前延迟地址(长剑.凤凰羽), 0.f, 2.f);
        gui.slider_float(u8"角度", 当前角度地址(长剑.凤凰羽), 1.f, 360.f);

        if (gui.tree_node(u8"1.0")) {
          ImGui::PushItemWidth(200); // 设置滑动条的宽度
          gui.slider_int(u8"振刀/闪避 间隔", 当前间隔地址(长剑.凤凰羽1), 0,
                         300.f);
          gui.combo(当前方式标签, 当前方式地址(长剑.凤凰羽1),
                    当前方式列表, 当前方式列表数量);
          ImGui::TreePop();
        }

        if (gui.tree_node(u8"2.0")) {
          ImGui::PushItemWidth(200); // 设置滑动条的宽度
          gui.slider_int(u8"振刀/闪避 间隔", 当前间隔地址(长剑.凤凰羽2), 0,
                         300.f);
          gui.combo(当前方式标签, 当前方式地址(长剑.凤凰羽2),
                    当前方式列表, 当前方式列表数量);
          ImGui::TreePop();
        }
        ImGui::TreePop();
      }
      if (gui.tree_node(u8"苍牙")) {
        gui.slider_float(u8"距离", 当前距离地址(长剑.苍牙), 1.f, 20.f);
        gui.slider_float(u8"延迟", 当前延迟地址(长剑.苍牙), 0.f, 2.f);
        gui.slider_float(u8"2M角度", 当前角度地址(长剑.苍牙), 1.f, 360.f);

        ImGui::PushItemWidth(200); // 设置滑动条的宽度
        gui.slider_int(u8"振刀/闪避 间隔", 当前间隔地址(长剑.苍牙), 0,
                       300.f);
        gui.combo(当前方式标签, 当前方式地址(长剑.苍牙), 当前方式列表, 当前方式列表数量);
        ImGui::TreePop();
      }
      if (gui.tree_node(u8"壁击")) {
        gui.slider_float(u8"距离", 当前距离地址(长剑.壁击), 1.f, 20.f);
        gui.slider_float(u8"延迟", 当前延迟地址(长剑.壁击), 0.0f, 1.f);
        gui.slider_float(u8"角度", 当前角度地址(长剑.壁击), 1.f, 360.f);

        ImGui::PushItemWidth(200); // 设置滑动条的宽度
        gui.slider_int(u8"振刀/闪避 间隔", 当前间隔地址(长剑.壁击), 0,
                       300.f);
        gui.combo(当前方式标签, 当前方式地址(长剑.壁击), 当前方式列表, 当前方式列表数量);
        ImGui::TreePop();
      }
      if (gui.tree_node(u8"七星夺窍")) {
        gui.slider_float(u8"距离", 当前距离地址(长剑.七星夺窍), 1.f, 20.f);
        gui.slider_float(u8"延迟", 当前延迟地址(长剑.七星夺窍), 0.f, 2.f);
        gui.slider_float(u8"角度", 当前角度地址(长剑.七星夺窍), 1.f, 360.f);

        ImGui::PushItemWidth(200); // 设置滑动条的宽度
        gui.slider_int(u8"振刀/闪避 间隔", 当前间隔地址(长剑.七星夺窍), 0,
                       300.f);
        gui.combo(当前方式标签, 当前方式地址(长剑.七星夺窍),
                  当前方式列表, 当前方式列表数量);
        ImGui::TreePop();
      }
      if (gui.tree_node(u8"链剑长剑同源")) {
          gui.slider_float(u8"距离", 当前距离地址(链剑.链剑长剑同源), 1.f, 20.f);
          gui.slider_float(u8"延迟", 当前延迟地址(链剑.链剑长剑同源), 0.0f, 1.f);
          gui.slider_float(u8"角度", 当前角度地址(链剑.链剑长剑同源), 1.f, 360.f);

          ImGui::PushItemWidth(200); // 设置滑动条的宽度
          gui.slider_int(u8"振刀/闪避 间隔", 当前间隔地址(链剑.链剑长剑同源), 0,
              300.f);
          gui.combo(当前方式标签, 当前方式地址(链剑.链剑长剑同源),
              当前方式列表, 当前方式列表数量);
          ImGui::TreePop();
      }
    }
    武器配置区(u8"链剑") {
      if (gui.tree_node(u8"左三")) {
        gui.slider_float(u8"距离", 当前距离地址(链剑.左3), 1.f, 20.f);
        gui.slider_float(u8"延迟", 当前延迟地址(链剑.左3), 0.0f, 2.f);
        gui.slider_float(u8"角度", 当前角度地址(链剑.左3), 1.f, 360.f);

        ImGui::PushItemWidth(200); // 设置滑动条的宽度
        gui.slider_int(u8"振刀/闪避 间隔", 当前间隔地址(链剑.左3), 0, 300.f);
        gui.combo(当前方式标签, 当前方式地址(链剑.左3), 当前方式列表, 当前方式列表数量);
        ImGui::TreePop();
      }
      if (gui.tree_node(u8"右三")) {
        gui.slider_float(u8"距离", 当前距离地址(链剑.右3), 1.f, 20.f);
        gui.slider_float(u8"延迟", 当前延迟地址(链剑.右3), 0.f, 2.f);
        gui.slider_float(u8"角度", 当前角度地址(链剑.右3), 1.f, 360.f);

        ImGui::PushItemWidth(200); // 设置滑动条的宽度
        gui.slider_int(u8"振刀/闪避 间隔", 当前间隔地址(链剑.右3), 0, 300.f);
        gui.combo(当前方式标签, 当前方式地址(链剑.右3), 当前方式列表, 当前方式列表数量);
        ImGui::TreePop();
      }
      if (gui.tree_node(u8"左蓄力")) {
        gui.slider_float(u8"距离", 当前距离地址(链剑.左蓄力), 1.f, 20.f);
        gui.slider_float(u8"延迟", 当前延迟地址(链剑.左蓄力), 0.f, 2.f);
        gui.slider_float(u8"角度", 当前角度地址(链剑.左蓄力), 1.f, 360.f);

        if (gui.tree_node(u8"1.0")) {
          ImGui::PushItemWidth(200); // 设置滑动条的宽度
          gui.slider_int(u8"振刀/闪避 间隔", 当前间隔地址(链剑.左蓄力1), 0,
                         300.f);
          gui.combo(当前方式标签, 当前方式地址(链剑.左蓄力1),
                    当前方式列表, 当前方式列表数量);
          ImGui::TreePop();
        }
        if (gui.tree_node(u8"2.0")) {
          ImGui::PushItemWidth(200); // 设置滑动条的宽度
          gui.slider_int(u8"振刀/闪避 间隔", 当前间隔地址(链剑.左蓄力2), 0,
                         300.f);
          gui.combo(当前方式标签, 当前方式地址(链剑.左蓄力2),
                    当前方式列表, 当前方式列表数量);
          ImGui::TreePop();
        }
        ImGui::TreePop();
      }
      if (gui.tree_node(u8"右蓄力")) {
        gui.slider_float(u8"距离", 当前距离地址(链剑.右蓄力), 1.f, 20.f);
        gui.slider_float(u8"延迟", 当前延迟地址(链剑.右蓄力), 0.f, 2.f);
        gui.slider_float(u8"角度", 当前角度地址(链剑.右蓄力), 1.f, 360.f);

        if (gui.tree_node(u8"1.0")) {
          ImGui::PushItemWidth(200); // 设置滑动条的宽度
          gui.slider_int(u8"振刀/闪避 间隔", 当前间隔地址(链剑.右蓄力1), 0,
                         300.f);
          gui.combo(当前方式标签, 当前方式地址(链剑.右蓄力1),
                    当前方式列表, 当前方式列表数量);
          ImGui::TreePop();
        }
        if (gui.tree_node(u8"2.0")) {
          ImGui::PushItemWidth(200); // 设置滑动条的宽度
          gui.slider_int(u8"振刀/闪避 间隔", 当前间隔地址(链剑.右蓄力2), 0,
                         300.f);
          gui.combo(当前方式标签, 当前方式地址(链剑.右蓄力2),
                    当前方式列表, 当前方式列表数量);
          ImGui::TreePop();
        }
        ImGui::TreePop();
      }
      if (gui.tree_node(u8"苍牙")) {
        gui.slider_float(u8"距离", 当前距离地址(链剑.苍牙), 1.f, 20.f);
        gui.slider_float(u8"延迟", 当前延迟地址(链剑.苍牙), 0.f, 2.f);
        gui.slider_float(u8"角度", 当前角度地址(链剑.苍牙), 1.f, 360.f);

        ImGui::PushItemWidth(200); // 设置滑动条的宽度
        gui.slider_int(u8"振刀/闪避 间隔", 当前间隔地址(链剑.苍牙), 0,
                       300.f);
        gui.combo(当前方式标签, 当前方式地址(链剑.苍牙), 当前方式列表, 当前方式列表数量);
        ImGui::TreePop();
      }
      if (gui.tree_node(u8"地龙滚堂刹")) {
        gui.slider_float(u8"距离", 当前距离地址(链剑.地龙滚堂刹), 1.f, 20.f);
        gui.slider_float(u8"延迟", 当前延迟地址(链剑.地龙滚堂刹), 0.f, 2.f);
        gui.slider_float(u8"角度", 当前角度地址(链剑.地龙滚堂刹), 1.f,
                         360.f);

        ImGui::PushItemWidth(200); // 设置滑动条的宽度
        gui.slider_int(u8"振刀/闪避 间隔", 当前间隔地址(链剑.地龙滚堂刹), 0,
                       300.f);
        gui.combo(当前方式标签, 当前方式地址(链剑.地龙滚堂刹),
                  当前方式列表, 当前方式列表数量);
        ImGui::TreePop();
      }
      if (gui.tree_node(u8"火龙卷云")) {
        gui.slider_float(u8"距离", 当前距离地址(链剑.火龙卷云), 1.f, 20.f);
        gui.slider_float(u8"延迟", 当前延迟地址(链剑.火龙卷云), 0.f, 2.f);
        gui.slider_float(u8"2M角度", 当前角度地址(链剑.火龙卷云), 1.f,
                         360.f);

        ImGui::PushItemWidth(200); // 设置滑动条的宽度
        gui.slider_int(u8"振刀/闪避 间隔", 当前间隔地址(链剑.火龙卷云), 0,
                       300.f);
        gui.combo(当前方式标签, 当前方式地址(链剑.火龙卷云),
                  当前方式列表, 当前方式列表数量);
        ImGui::TreePop();
      }
      if (gui.tree_node(u8"断罪碎蜂")) {
        gui.slider_float(u8"距离", 当前距离地址(链剑.断罪碎蜂), 1.f, 20.f);
        gui.slider_float(u8"延迟", 当前延迟地址(链剑.断罪碎蜂), 0.0f, 1.f);
        gui.slider_float(u8"角度", 当前角度地址(链剑.断罪碎蜂), 1.f, 360.f);

        ImGui::PushItemWidth(200); // 设置滑动条的宽度
        gui.slider_int(u8"振刀/闪避 间隔", 当前间隔地址(链剑.断罪碎蜂), 0,
                       300.f);
        gui.combo(当前方式标签, 当前方式地址(链剑.断罪碎蜂),
                  当前方式列表, 当前方式列表数量);
        ImGui::TreePop();
      }
      if (gui.tree_node(u8"链剑长剑同源")) {
          gui.slider_float(u8"距离", 当前距离地址(链剑.链剑长剑同源), 1.f, 20.f);
          gui.slider_float(u8"延迟", 当前延迟地址(链剑.链剑长剑同源), 0.0f, 1.f);
          gui.slider_float(u8"角度", 当前角度地址(链剑.链剑长剑同源), 1.f, 360.f);

          ImGui::PushItemWidth(200); // 设置滑动条的宽度
          gui.slider_int(u8"振刀/闪避 间隔", 当前间隔地址(链剑.链剑长剑同源), 0,
              300.f);
          gui.combo(当前方式标签, 当前方式地址(链剑.链剑长剑同源),
              当前方式列表, 当前方式列表数量);
          ImGui::TreePop();
      }
      if (gui.tree_node(u8"蓄力追击")) {
        gui.slider_float(u8"距离", 当前距离地址(链剑.蓄力追击), 1.f, 20.f);
        gui.slider_float(u8"延迟", 当前延迟地址(链剑.蓄力追击), 0.f, 2.f);
        gui.slider_float(u8"角度", 当前角度地址(链剑.蓄力追击), 1.f, 360.f);

        ImGui::PushItemWidth(200); // 设置滑动条的宽度
        gui.slider_int(u8"振刀/闪避 间隔", 当前间隔地址(链剑.蓄力追击), 0,
                       300.f);
        gui.combo(当前方式标签, 当前方式地址(链剑.蓄力追击),
                  当前方式列表, 当前方式列表数量);
        ImGui::TreePop();
      }
    }
    武器配置区(u8"千机伞") {
      千机伞动作配置(u8"左三", 左3);
      千机伞动作配置(u8"伞左蓄", 伞左蓄);
      千机伞动作配置(u8"盾左蓄", 盾左蓄);
      千机伞动作配置(u8"斧左蓄", 斧左蓄);
      千机伞动作配置(u8"伞右蓄", 伞右蓄);
      千机伞动作配置(u8"盾右蓄", 盾右蓄);
      千机伞动作配置(u8"斧右蓄", 斧右蓄);
      千机伞动作配置(u8"听雨封喉", 听雨封喉);
      千机伞动作配置(u8"苍牙", 苍牙);
    }
    武器配置区(u8"太刀") {
      if (gui.tree_node(u8"左三")) {
        gui.slider_float(u8"距离", 当前距离地址(太刀.左3), 1.f, 20.f);
        gui.slider_float(u8"延迟", 当前延迟地址(太刀.左3), 0.f, 2.f);
        gui.slider_float(u8"角度", 当前角度地址(太刀.左3), 1.f, 360.f);

        ImGui::PushItemWidth(200); // 设置滑动条的宽度
        gui.slider_int(u8"振刀/闪避 间隔", 当前间隔地址(太刀.左3), 0, 300.f);
        gui.combo(当前方式标签, 当前方式地址(太刀.左3), 当前方式列表, 当前方式列表数量);
        ImGui::TreePop();
      }
      if (gui.tree_node(u8"右三")) {
        gui.slider_float(u8"距离", 当前距离地址(太刀.右3), 1.f, 20.f);
        gui.slider_float(u8"延迟", 当前延迟地址(太刀.右3), 0.f, 2.f);
        gui.slider_float(u8"角度", 当前角度地址(太刀.右3), 1.f, 360.f);

        ImGui::PushItemWidth(200); // 设置滑动条的宽度
        gui.slider_int(u8"振刀/闪避 间隔", 当前间隔地址(太刀.右3), 0, 300.f);
        gui.combo(当前方式标签, 当前方式地址(太刀.右3), 当前方式列表, 当前方式列表数量);
        ImGui::TreePop();
      }
      if (gui.tree_node(u8"左百裂")) {
        gui.slider_float(u8"距离", 当前距离地址(太刀.左蓄), 1.f, 20.f);
        gui.slider_float(u8"延迟", 当前延迟地址(太刀.左蓄), 0.f, 2.f);
        gui.slider_float(u8"角度", 当前角度地址(太刀.左蓄), 1.f, 360.f);

        if (gui.tree_node(u8"1.0")) {
          ImGui::PushItemWidth(200); // 设置滑动条的宽度
          gui.slider_int(u8"振刀/闪避 间隔", 当前间隔地址(太刀.左蓄1), 0,
                         300.f);
          gui.combo(当前方式标签, 当前方式地址(太刀.左蓄1), 当前方式列表, 当前方式列表数量);
          ImGui::TreePop();
        }
        if (gui.tree_node(u8"2.0")) {
          ImGui::PushItemWidth(200); // 设置滑动条的宽度
          gui.slider_int(u8"振刀/闪避 间隔", 当前间隔地址(太刀.左蓄2), 0,
                         300.f);
          gui.combo(当前方式标签, 当前方式地址(太刀.左蓄2), 当前方式列表, 当前方式列表数量);
          ImGui::TreePop();
        }
        ImGui::TreePop();
      }
      if (gui.tree_node(u8"右百裂")) {
        gui.slider_float(u8"距离", 当前距离地址(太刀.右蓄), 1.f, 20.f);
        gui.slider_float(u8"延迟", 当前延迟地址(太刀.右蓄), 0.f, 2.f);
        gui.slider_float(u8"角度", 当前角度地址(太刀.右蓄), 1.f, 360.f);

        if (gui.tree_node(u8"1.0")) {
          ImGui::PushItemWidth(200); // 设置滑动条的宽度
          gui.slider_int(u8"振刀/闪避 间隔", 当前间隔地址(太刀.右蓄1), 0,
                         300.f);
          gui.combo(当前方式标签, 当前方式地址(太刀.右蓄1), 当前方式列表, 当前方式列表数量);
          ImGui::TreePop();
        }
        if (gui.tree_node(u8"2.0")) {
          ImGui::PushItemWidth(200); // 设置滑动条的宽度
          gui.slider_int(u8"振刀/闪避 间隔", 当前间隔地址(太刀.右蓄2), 0,
                         300.f);
          gui.combo(当前方式标签, 当前方式地址(太刀.右蓄2), 当前方式列表, 当前方式列表数量);
          ImGui::TreePop();
        }
        ImGui::TreePop();
      }
      if (gui.tree_node(u8"苍牙")) {
        gui.slider_float(u8"距离", 当前距离地址(太刀.苍牙), 1.f, 20.f);
        gui.slider_float(u8"延迟", 当前延迟地址(太刀.苍牙), 0.f, 2.f);
        gui.slider_float(u8"角度", 当前角度地址(太刀.苍牙), 1.f, 360.f);

        ImGui::PushItemWidth(200); // 设置滑动条的宽度
        gui.slider_int(u8"振刀/闪避 间隔", 当前间隔地址(太刀.苍牙), 0,
                       300.f);
        gui.combo(当前方式标签, 当前方式地址(太刀.苍牙), 当前方式列表, 当前方式列表数量);
        ImGui::TreePop();
      }
      if (gui.tree_node(u8"刹那斩")) {
        gui.slider_float(u8"距离", 当前距离地址(太刀.刹那斩), 1.f, 20.f);
        gui.slider_float(u8"延迟", 当前延迟地址(太刀.刹那斩), 0.f, 2.f);
        gui.slider_float(u8"角度", 当前角度地址(太刀.刹那斩), 1.f, 360.f);

        ImGui::PushItemWidth(200); // 设置滑动条的宽度
        gui.slider_int(u8"振刀/闪避 间隔", 当前间隔地址(太刀.刹那斩1), 0,
                       300.f);
        gui.combo(当前方式标签, 当前方式地址(太刀.刹那斩1), 当前方式列表, 当前方式列表数量);
        ImGui::TreePop();
      }
      if (gui.tree_node(u8"右惊雷&惊雷十劫")) {
        gui.slider_float(u8"距离", 当前距离地址(太刀.惊雷), 1.f, 20.f);
        gui.slider_float(u8"延迟", 当前延迟地址(太刀.右惊雷), 0.f, 2.f);
        gui.slider_float(u8"角度", 当前角度地址(太刀.惊雷十劫), 1.f, 360.f);

        ImGui::PushItemWidth(200); // 设置滑动条的宽度
        gui.slider_int(u8"振刀/闪避 间隔", 当前间隔地址(太刀.惊雷), 0,
                       300.f);
        gui.combo(当前方式标签, 当前方式地址(太刀.惊雷), 当前方式列表, 当前方式列表数量);
        ImGui::TreePop();
      }
      if (gui.tree_node(u8"青鬼")) {
        gui.slider_float(u8"距离", 当前距离地址(太刀.青鬼), 1.f, 20.f);
        gui.slider_float(u8"延迟", 当前延迟地址(太刀.青鬼), 0.f, 2.f);
        gui.slider_float(u8"角度", 当前角度地址(太刀.青鬼), 1.f, 360.f);

        ImGui::PushItemWidth(200); // 设置滑动条的宽度
        gui.slider_int(u8"振刀/闪避 间隔", 当前间隔地址(太刀.青鬼), 0,
                       300.f);
        gui.combo(当前方式标签, 当前方式地址(太刀.青鬼), 当前方式列表, 当前方式列表数量);
        ImGui::TreePop();
      }
      if (gui.tree_node(u8"壁击")) {
        gui.slider_float(u8"距离", 当前距离地址(太刀.壁击), 1.f, 20.f);
        gui.slider_float(u8"延迟", 当前延迟地址(太刀.壁击), 0.f, 2.f);
        gui.slider_float(u8"角度", 当前角度地址(太刀.壁击), 1.f, 360.f);

        ImGui::PushItemWidth(200); // 设置滑动条的宽度
        gui.slider_int(u8"振刀/闪避 间隔", 当前间隔地址(太刀.壁击), 0,
                       300.f);
        gui.combo(当前方式标签, 当前方式地址(太刀.壁击), 当前方式列表, 当前方式列表数量);
        ImGui::TreePop();
      }
      if (gui.tree_node(u8"同源")) {
        gui.slider_float(u8"距离", 当前距离地址(太刀.同源), 1.f, 20.f);
        gui.slider_float(u8"延迟", 当前延迟地址(太刀.同源), 0.f, 2.f);
        gui.slider_float(u8"角度", 当前角度地址(太刀.同源), 1.f, 360.f);

        ImGui::PushItemWidth(200); // 设置滑动条的宽度
        gui.slider_int(u8"振刀/闪避 间隔", 当前间隔地址(太刀.同源), 0,
                       300.f);
        gui.combo(当前方式标签, 当前方式地址(太刀.同源), 当前方式列表, 当前方式列表数量);
        ImGui::TreePop();
      }
      if (gui.tree_node(u8"噬魂斩")) {
        gui.slider_float(u8"距离", 当前距离地址(太刀.噬魂斩), 1.f, 20.f);
        gui.slider_float(u8"延迟", 当前延迟地址(太刀.噬魂斩), 0.f, 2.f);

        ImGui::PushItemWidth(200); // 设置滑动条的宽度
        gui.slider_int(u8"振刀/闪避 间隔", 当前间隔地址(太刀.噬魂斩1), 0,
                       300.f);
        gui.combo(当前方式标签, 当前方式地址(太刀.噬魂斩1), 当前方式列表, 当前方式列表数量);
        ImGui::TreePop();
      }
    }
    武器配置区(u8"阔刀") {
      if (gui.tree_node(u8"左左")) {
        gui.slider_float(u8"距离", 当前距离地址(阔刀.左左), 1.f, 20.f);
        gui.slider_float(u8"延迟", 当前延迟地址(阔刀.左左), 0.f, 2.f);
        gui.slider_float(u8"角度", 当前角度地址(阔刀.左左), 1.f, 360.f);

        ImGui::PushItemWidth(200); // 设置滑动条的宽度
        gui.slider_int(u8"振刀/闪避 间隔", 当前间隔地址(阔刀.左左), 0,
                       300.f);
        gui.combo(当前方式标签, 当前方式地址(阔刀.左左), 当前方式列表, 当前方式列表数量);
        ImGui::TreePop();
      }
      if (gui.tree_node(u8"左右")) {
        gui.slider_float(u8"距离", 当前距离地址(阔刀.左右), 1.f, 20.f);
        gui.slider_float(u8"延迟", 当前延迟地址(阔刀.左右), 0.f, 2.f);
        gui.slider_float(u8"角度", 当前角度地址(阔刀.左右), 1.f, 360.f);

        ImGui::PushItemWidth(200); // 设置滑动条的宽度
        gui.slider_int(u8"振刀/闪避 间隔", 当前间隔地址(阔刀.左右), 0,
                       300.f);
        gui.combo(当前方式标签, 当前方式地址(阔刀.左右), 当前方式列表, 当前方式列表数量);
        ImGui::TreePop();
      }
      if (gui.tree_node(u8"右右")) {
        gui.slider_float(u8"距离", 当前距离地址(阔刀.右右), 1.f, 20.f);
        gui.slider_float(u8"延迟", 当前延迟地址(阔刀.右右), 0.f, 2.f);
        gui.slider_float(u8"角度", 当前角度地址(阔刀.右右), 1.f, 360.f);

        ImGui::PushItemWidth(200); // 设置滑动条的宽度
        gui.slider_int(u8"振刀/闪避 间隔", 当前间隔地址(阔刀.右右), 0,
                       300.f);
        gui.combo(当前方式标签, 当前方式地址(阔刀.右右), 当前方式列表, 当前方式列表数量);
        ImGui::TreePop();
      }
      if (gui.tree_node(u8"左蓄一段")) {
        gui.slider_float(u8"距离", 当前距离地址(阔刀.左蓄一段), 1.f, 20.f);
        gui.slider_float(u8"延迟", 当前延迟地址(阔刀.左蓄一段), 0.f, 2.f);
        gui.slider_float(u8"角度", 当前角度地址(阔刀.左蓄一段), 1.f, 360.f);

        ImGui::PushItemWidth(200); // 设置滑动条的宽度
        gui.slider_int(u8"振刀/闪避 间隔", 当前间隔地址(阔刀.左蓄一段), 0,
                       300.f);
        gui.combo(当前方式标签, 当前方式地址(阔刀.左蓄一段),
                  当前方式列表, 当前方式列表数量);
        ImGui::TreePop();
      }
      if (gui.tree_node(u8"左蓄二段")) {
        gui.slider_float(u8"距离", 当前距离地址(阔刀.左蓄二段), 1.f, 20.f);
        gui.slider_float(u8"延迟", 当前延迟地址(阔刀.左蓄二段), 0.f, 2.f);
        gui.slider_float(u8"角度", 当前角度地址(阔刀.左蓄二段), 1.f, 360.f);

        ImGui::PushItemWidth(200); // 设置滑动条的宽度
        gui.slider_int(u8"振刀/闪避 间隔", 当前间隔地址(阔刀.左蓄二段), 0,
                       300.f);
        gui.combo(当前方式标签, 当前方式地址(阔刀.左蓄二段),
                  当前方式列表, 当前方式列表数量);
        ImGui::TreePop();
      }
      if (gui.tree_node(u8"左蓄三段")) {
        gui.slider_float(u8"距离", 当前距离地址(阔刀.左蓄三段), 1.f, 20.f);
        gui.slider_float(u8"延迟", 当前延迟地址(阔刀.左蓄三段), 0.f, 2.f);
        gui.slider_float(u8"角度", 当前角度地址(阔刀.左蓄三段), 1.f, 360.f);

        ImGui::PushItemWidth(200); // 设置滑动条的宽度
        gui.slider_int(u8"振刀/闪避 间隔", 当前间隔地址(阔刀.左蓄三段), 0,
                       300.f);
        gui.combo(当前方式标签, 当前方式地址(阔刀.左蓄三段),
                  当前方式列表, 当前方式列表数量);
        ImGui::TreePop();
      }
      if (gui.tree_node(u8"右蓄")) {
        gui.slider_float(u8"距离", 当前距离地址(阔刀.右蓄), 1.f, 20.f);
        gui.slider_float(u8"延迟", 当前延迟地址(阔刀.右蓄), 0.f, 2.f);
        gui.slider_float(u8"角度", 当前角度地址(阔刀.右蓄), 1.f, 360.f);

        if (gui.tree_node(u8"1.0")) {
          ImGui::PushItemWidth(200); // 设置滑动条的宽度
          gui.slider_int(u8"振刀/闪避 间隔", 当前间隔地址(阔刀.右蓄1), 0,
                         300.f);
          gui.combo(当前方式标签, 当前方式地址(阔刀.右蓄1), 当前方式列表, 当前方式列表数量);
          ImGui::TreePop();
        }
        if (gui.tree_node(u8"2.0")) {
          ImGui::PushItemWidth(200); // 设置滑动条的宽度
          gui.slider_int(u8"振刀/闪避 间隔", 当前间隔地址(阔刀.右蓄2), 0,
                         300.f);
          gui.combo(当前方式标签, 当前方式地址(阔刀.右蓄2), 当前方式列表, 当前方式列表数量);
          ImGui::TreePop();
        }
        ImGui::TreePop();
      }
      if (gui.tree_node(u8"右右蓄")) {
        gui.slider_float(u8"距离", 当前距离地址(阔刀.右右蓄), 1.f, 20.f);
        gui.slider_float(u8"延迟", 当前延迟地址(阔刀.右右蓄), 0.f, 2.f);
        gui.slider_float(u8"角度", 当前角度地址(阔刀.右右蓄), 1.f, 360.f);

        ImGui::PushItemWidth(200); // 设置滑动条的宽度
        gui.slider_int(u8"振刀/闪避 间隔", 当前间隔地址(阔刀.右右蓄), 0,
                       300.f);
        gui.combo(当前方式标签, 当前方式地址(阔刀.右右蓄), 当前方式列表, 当前方式列表数量);
        ImGui::TreePop();
      }
      if (gui.tree_node(u8"壁击")) {
        gui.slider_float(u8"距离", 当前距离地址(阔刀.壁击), 1.f, 20.f);
        gui.slider_float(u8"延迟", 当前延迟地址(阔刀.壁击), 0.f, 2.f);
        gui.slider_float(u8"角度", 当前角度地址(阔刀.壁击), 1.f, 360.f);

        ImGui::PushItemWidth(200); // 设置滑动条的宽度
        gui.slider_int(u8"振刀/闪避 间隔", 当前间隔地址(阔刀.壁击), 0,
                       300.f);
        gui.combo(当前方式标签, 当前方式地址(阔刀.壁击), 当前方式列表, 当前方式列表数量);
        ImGui::TreePop();
      }
      if (gui.tree_node(u8"雷刀")) {
        gui.slider_float(u8"距离", 当前距离地址(阔刀.雷刀), 1.f, 20.f);
        gui.slider_float(u8"延迟", 当前延迟地址(阔刀.雷刀), 0.f, 2.f);
        gui.slider_float(u8"角度", 当前角度地址(阔刀.左蓄三段), 1.f, 360.f);

        ImGui::PushItemWidth(200); // 设置滑动条的宽度
        gui.slider_int(u8"振刀/闪避 间隔", 当前间隔地址(阔刀.雷刀), 0,
                       300.f);
        gui.combo(当前方式标签, 当前方式地址(阔刀.雷刀), 当前方式列表, 当前方式列表数量);
        ImGui::TreePop();
      }
      if (gui.tree_node(u8"雷刀下劈")) {
        gui.slider_float(u8"距离", 当前距离地址(阔刀.雷刀下劈), 1.f, 20.f);
        gui.slider_float(u8"延迟", 当前延迟地址(阔刀.雷刀下劈), 0.f, 2.f);

        ImGui::PushItemWidth(200); // 设置滑动条的宽度
        gui.slider_int(u8"振刀/闪避 间隔", 当前间隔地址(阔刀.雷刀下劈), 0,
                       300.f);
        gui.combo(当前方式标签, 当前方式地址(阔刀.雷刀下劈),
                  当前方式列表, 当前方式列表数量);
        ImGui::TreePop();
      }
      if (gui.tree_node(u8"同源")) {
        gui.slider_float(u8"距离", 当前距离地址(阔刀.同源), 1.f, 20.f);
        gui.slider_float(u8"延迟", 当前延迟地址(阔刀.同源), 0.f, 2.f);
        gui.slider_float(u8"角度", 当前角度地址(阔刀.同源), 1.f, 360.f);

        ImGui::PushItemWidth(200); // 设置滑动条的宽度
        gui.slider_int(u8"振刀/闪避 间隔", 当前间隔地址(阔刀.同源), 0,
                       300.f);
        gui.combo(当前方式标签, 当前方式地址(阔刀.同源), 当前方式列表, 当前方式列表数量);
        ImGui::TreePop();
      }
    }
    武器配置区(u8"长枪") {
      if (gui.tree_node(u8"左三连")) {
        gui.slider_float(u8"距离", 当前距离地址(长枪.左3), 1.f, 20.f);
        gui.slider_float(u8"延迟", 当前延迟地址(长枪.左3), 0.f, 2.f);
        gui.slider_float(u8"角度", 当前角度地址(长枪.左3), 1.f, 360.f);

        ImGui::PushItemWidth(200); // 设置滑动条的宽度
        gui.slider_int(u8"振刀/闪避 间隔", 当前间隔地址(长枪.左3), 0, 300.f);
        gui.combo(当前方式标签, 当前方式地址(长枪.左3), 当前方式列表, 当前方式列表数量);
        ImGui::TreePop();
      }
      if (gui.tree_node(u8"右三连")) {
        gui.slider_float(u8"距离", 当前距离地址(长枪.右3), 1.f, 20.f);
        gui.slider_float(u8"延迟", 当前延迟地址(长枪.右3), 0.f, 2.f);
        gui.slider_float(u8"角度", 当前角度地址(长枪.右3), 1.f, 360.f);

        ImGui::PushItemWidth(200); // 设置滑动条的宽度
        gui.slider_int(u8"振刀/闪避 间隔", 当前间隔地址(长枪.右3), 0, 300.f);
        gui.combo(当前方式标签, 当前方式地址(长枪.右3), 当前方式列表, 当前方式列表数量);
        ImGui::TreePop();
      }
      if (gui.tree_node(u8"左蓄")) {
        gui.slider_float(u8"距离", 当前距离地址(长枪.左蓄), 1.f, 20.f);
        gui.slider_float(u8"延迟", 当前延迟地址(长枪.左蓄), 0.f, 2.f);
        gui.slider_float(u8"角度", 当前角度地址(长枪.左蓄), 1.f, 360.f);

        ImGui::PushItemWidth(200); // 设置滑动条的宽度
        gui.slider_int(u8"振刀/闪避 间隔", 当前间隔地址(长枪.左蓄), 0,
                       300.f);
        gui.combo(当前方式标签, 当前方式地址(长枪.左蓄), 当前方式列表, 当前方式列表数量);
        ImGui::TreePop();
      }
      if (gui.tree_node(u8"右蓄")) {
        gui.slider_float(u8"距离", 当前距离地址(长枪.右蓄), 1.f, 20.f);
        gui.slider_float(u8"延迟", 当前延迟地址(长枪.右蓄), 0.f, 2.f);
        gui.slider_float(u8"角度", 当前角度地址(长枪.右蓄), 1.f, 360.f);

        ImGui::PushItemWidth(200); // 设置滑动条的宽度
        gui.slider_int(u8"振刀/闪避 间隔", 当前间隔地址(长枪.右蓄), 0,
                       300.f);
        gui.combo(当前方式标签, 当前方式地址(长枪.右蓄), 当前方式列表, 当前方式列表数量);
        ImGui::TreePop();
      }
      if (gui.tree_node(u8"六合枪")) {
        gui.slider_float(u8"距离", 当前距离地址(长枪.六合枪), 1.f, 20.f);
        gui.slider_float(u8"延迟", 当前延迟地址(长枪.六合枪), 0.f, 2.f);
        gui.slider_float(u8"角度", 当前角度地址(长枪.六合枪), 1.f, 360.f);

        ImGui::PushItemWidth(200); // 设置滑动条的宽度
        gui.slider_int(u8"振刀/闪避 间隔", 当前间隔地址(长枪.六合枪), 0,
                       300.f);
        gui.combo(当前方式标签, 当前方式地址(长枪.六合枪), 当前方式列表, 当前方式列表数量);
        ImGui::TreePop();
      }
      if (gui.tree_node(u8"大圣游")) {
        gui.slider_float(u8"距离", 当前距离地址(长枪.大圣游), 1.f, 20.f);
        gui.slider_float(u8"延迟", 当前延迟地址(长枪.大圣游), 0.f, 2.f);
        gui.slider_float(u8"角度", 当前角度地址(长枪.大圣游), 1.f, 360.f);

        ImGui::PushItemWidth(200); // 设置滑动条的宽度
        gui.slider_int(u8"振刀/闪避 间隔", 当前间隔地址(长枪.大圣游), 0,
                       300.f);
        gui.combo(当前方式标签, 当前方式地址(长枪.大圣游), 当前方式列表, 当前方式列表数量);
        ImGui::TreePop();
      }
      if (gui.tree_node(u8"风卷云残")) {
        gui.slider_float(u8"距离", 当前距离地址(长枪.风卷云残), 1.f, 20.f);
        gui.slider_float(u8"延迟", 当前延迟地址(长枪.风卷云残), 0.f, 2.f);
        gui.slider_float(u8"角度", 当前角度地址(长枪.风卷云残), 1.f, 360.f);

        ImGui::PushItemWidth(200); // 设置滑动条的宽度
        gui.slider_int(u8"振刀/闪避 间隔", 当前间隔地址(长枪.风卷云残), 0,
                       300.f);
        gui.combo(当前方式标签, 当前方式地址(长枪.风卷云残),
                  当前方式列表, 当前方式列表数量);
        ImGui::TreePop();
      }
      if (gui.tree_node(u8"双环扫")) {
        gui.slider_float(u8"距离", 当前距离地址(长枪.双环扫), 1.f, 20.f);
        gui.slider_float(u8"延迟", 当前延迟地址(长枪.双环扫), 0.f, 2.f);
        gui.slider_float(u8"角度", 当前角度地址(长枪.双环扫), 1.f, 360.f);

        ImGui::PushItemWidth(200); // 设置滑动条的宽度
        gui.slider_int(u8"振刀/闪避 间隔", 当前间隔地址(长枪.双环扫), 0,
                       300.f);
        gui.combo(当前方式标签, 当前方式地址(长枪.双环扫), 当前方式列表, 当前方式列表数量);
        ImGui::TreePop();
      }
      if (gui.tree_node(u8"壁击")) {
        gui.slider_float(u8"距离", 当前距离地址(长枪.壁击), 1.f, 20.f);
        gui.slider_float(u8"延迟", 当前延迟地址(长枪.壁击), 0.f, 2.f);
        gui.slider_float(u8"角度", 当前角度地址(长枪.壁击), 1.f, 360.f);

        ImGui::PushItemWidth(200); // 设置滑动条的宽度
        gui.slider_int(u8"振刀/闪避 间隔", 当前间隔地址(长枪.壁击), 0,
                       300.f);
        gui.combo(当前方式标签, 当前方式地址(长枪.壁击), 当前方式列表, 当前方式列表数量);
        ImGui::TreePop();
      }
      if (gui.tree_node(u8"龙王破")) {
        gui.slider_float(u8"距离", 当前距离地址(长枪.龙王破), 1.f, 20.f);
        gui.slider_float(u8"延迟", 当前延迟地址(长枪.龙王破), 0.f, 2.f);
        gui.slider_float(u8"角度", 当前角度地址(长枪.龙王破), 1.f, 360.f);

        ImGui::PushItemWidth(200); // 设置滑动条的宽度
        gui.slider_int(u8"振刀/闪避 间隔", 当前间隔地址(长枪.龙王破), 0,
                       300.f);
        gui.combo(当前方式标签, 当前方式地址(长枪.龙王破), 当前方式列表, 当前方式列表数量);
        ImGui::TreePop();
      }
      if (gui.tree_node(u8"穿心脚")) {
        gui.slider_float(u8"距离", 当前距离地址(长枪.穿心脚), 1.f, 20.f);
        gui.slider_float(u8"延迟", 当前延迟地址(长枪.穿心脚), 0.f, 2.f);
        gui.slider_float(u8"角度", 当前角度地址(长枪.穿心脚), 1.f, 360.f);

        ImGui::PushItemWidth(200); // 设置滑动条的宽度
        gui.slider_int(u8"振刀/闪避 间隔", 当前间隔地址(长枪.穿心脚), 0,
                       300.f);
        gui.combo(当前方式标签, 当前方式地址(长枪.穿心脚), 当前方式列表, 当前方式列表数量);
        ImGui::TreePop();
      }
      if (gui.tree_node(u8"同源")) {
        gui.slider_float(u8"距离", 当前距离地址(长枪.同源), 1.f, 20.f);
        gui.slider_float(u8"延迟", 当前延迟地址(长枪.同源), 0.f, 2.f);
        gui.slider_float(u8"角度", 当前角度地址(长枪.同源), 1.f, 360.f);

        ImGui::PushItemWidth(200); // 设置滑动条的宽度
        gui.slider_int(u8"振刀/闪避 间隔", 当前间隔地址(长枪.同源), 0,
                       300.f);
        gui.combo(当前方式标签, 当前方式地址(长枪.同源), 当前方式列表, 当前方式列表数量);
        ImGui::TreePop();
      }
    }
    武器配置区(u8"匕首") {
      if (gui.tree_node(u8"左三连")) {
        gui.slider_float(u8"距离", 当前距离地址(匕首.左3), 1.f, 20.f);
        gui.slider_float(u8"延迟", 当前延迟地址(匕首.左3), 0.f, 2.f);
        gui.slider_float(u8"角度", 当前角度地址(匕首.左3), 1.f, 360.f);

        ImGui::PushItemWidth(200); // 设置滑动条的宽度
        gui.slider_int(u8"振刀/闪避 间隔", 当前间隔地址(匕首.左3), 0, 300.f);
        gui.combo(当前方式标签, 当前方式地址(匕首.左3), 当前方式列表, 当前方式列表数量);
        ImGui::TreePop();
      }
      if (gui.tree_node(u8"右三连")) {
        gui.slider_float(u8"距离", 当前距离地址(匕首.右3), 1.f, 20.f);
        gui.slider_float(u8"延迟", 当前延迟地址(匕首.右3), 0.f, 2.f);
        gui.slider_float(u8"角度", 当前角度地址(匕首.右3), 1.f, 360.f);

        ImGui::PushItemWidth(200); // 设置滑动条的宽度
        gui.slider_int(u8"振刀/闪避 间隔", 当前间隔地址(匕首.右3), 0, 300.f);
        gui.combo(当前方式标签, 当前方式地址(匕首.右3), 当前方式列表, 当前方式列表数量);
        ImGui::TreePop();
      }
      if (gui.tree_node(u8"右蓄")) {
        gui.slider_float(u8"距离", 当前距离地址(匕首.右蓄), 1.f, 20.f);
        gui.slider_float(u8"延迟", 当前延迟地址(匕首.右蓄), 0.f, 2.f);
        gui.slider_float(u8"角度", 当前角度地址(匕首.右蓄), 1.f, 360.f);

        ImGui::PushItemWidth(200); // 设置滑动条的宽度
        gui.slider_int(u8"振刀/闪避 间隔", 当前间隔地址(匕首.右蓄), 0,
                       300.f);
        gui.combo(当前方式标签, 当前方式地址(匕首.右蓄), 当前方式列表, 当前方式列表数量);
        ImGui::TreePop();
      }
      if (gui.tree_node(u8"左蓄")) {
        gui.slider_float(u8"距离", 当前距离地址(匕首.左蓄), 1.f, 20.f);
        gui.slider_float(u8"延迟", 当前延迟地址(匕首.左蓄), 0.f, 2.f);
        gui.slider_float(u8"角度", 当前角度地址(匕首.左蓄), 1.f, 360.f);

        ImGui::PushItemWidth(200); // 设置滑动条的宽度
        gui.slider_int(u8"振刀/闪避 间隔", 当前间隔地址(匕首.左蓄), 0,
                       300.f);
        gui.combo(当前方式标签, 当前方式地址(匕首.左蓄), 当前方式列表, 当前方式列表数量);
        ImGui::TreePop();
      }
      if (gui.tree_node(u8"鬼返")) {
        gui.slider_float(u8"距离", 当前距离地址(匕首.鬼反), 1.f, 20.f);
        gui.slider_float(u8"延迟", 当前延迟地址(匕首.鬼反), 0.f, 2.f);
        gui.slider_float(u8"角度", 当前角度地址(匕首.鬼反), 1.f, 360.f);

        ImGui::PushItemWidth(200); // 设置滑动条的宽度
        gui.slider_int(u8"振刀/闪避 间隔", 当前间隔地址(匕首.鬼反), 0,
                       300.f);
        gui.combo(当前方式标签, 当前方式地址(匕首.鬼反), 当前方式列表, 当前方式列表数量);
        ImGui::TreePop();
      }
      if (gui.tree_node(u8"荆轲献匕")) {
        gui.slider_float(u8"距离", 当前距离地址(匕首.荆轲献匕), 1.f, 20.f);
        gui.slider_float(u8"延迟", 当前延迟地址(匕首.荆轲献匕), 0.f, 2.f);
        gui.slider_float(u8"角度", 当前角度地址(匕首.荆轲献匕), 1.f, 360.f);

        ImGui::PushItemWidth(200); // 设置滑动条的宽度
        gui.slider_int(u8"振刀/闪避 间隔", 当前间隔地址(匕首.荆轲献匕), 0,
                       300.f);
        gui.combo(当前方式标签, 当前方式地址(匕首.荆轲献匕),
                  当前方式列表, 当前方式列表数量);
        ImGui::TreePop();
      }
      if (gui.tree_node(u8"鬼刃暗扎")) {
        gui.slider_float(u8"距离", 当前距离地址(匕首.鬼刃暗扎), 1.f, 20.f);
        gui.slider_float(u8"延迟", 当前延迟地址(匕首.鬼刃暗扎), 0.f, 2.f);
        gui.slider_float(u8"角度", 当前角度地址(匕首.鬼刃暗扎), 1.f, 360.f);

        ImGui::PushItemWidth(200); // 设置滑动条的宽度
        gui.slider_int(u8"振刀/闪避 间隔", 当前间隔地址(匕首.鬼刃暗扎), 0,
                       300.f);
        gui.combo(当前方式标签, 当前方式地址(匕首.鬼刃暗扎),
                  当前方式列表, 当前方式列表数量);
        ImGui::TreePop();
      }
      if (gui.tree_node(u8"鬼哭神嚎")) {
        gui.slider_float(u8"距离", 当前距离地址(匕首.鬼哭神嚎), 1.f, 20.f);
        gui.slider_float(u8"延迟", 当前延迟地址(匕首.鬼哭神嚎), 0.f, 2.f);
        gui.slider_float(u8"角度", 当前角度地址(匕首.鬼哭神嚎), 1.f, 360.f);

        ImGui::PushItemWidth(200); // 设置滑动条的宽度
        gui.slider_int(u8"振刀/闪避 间隔", 当前间隔地址(匕首.鬼哭神嚎), 0,
                       300.f);
        gui.combo(当前方式标签, 当前方式地址(匕首.鬼哭神嚎),
                  当前方式列表, 当前方式列表数量);
        ImGui::TreePop();
      }
      if (gui.tree_node(u8"亢龙有悔")) {
        gui.slider_float(u8"距离", 当前距离地址(匕首.亢龙有悔), 1.f, 20.f);
        gui.slider_float(u8"延迟", 当前延迟地址(匕首.亢龙有悔), 0.f, 2.f);
        gui.slider_float(u8"角度", 当前角度地址(匕首.亢龙有悔), 1.f, 360.f);

        ImGui::PushItemWidth(200); // 设置滑动条的宽度
        gui.slider_int(u8"振刀/闪避 间隔", 当前间隔地址(匕首.亢龙有悔), 0,
                       300.f);
        gui.combo(当前方式标签, 当前方式地址(匕首.亢龙有悔),
                  当前方式列表, 当前方式列表数量);
        ImGui::TreePop();
      }
      if (gui.tree_node(u8"壁击")) {
        gui.slider_float(u8"距离", 当前距离地址(匕首.壁击), 1.f, 20.f);
        gui.slider_float(u8"延迟", 当前延迟地址(匕首.壁击), 0.f, 2.f);
        gui.slider_float(u8"角度", 当前角度地址(匕首.壁击), 1.f, 360.f);

        ImGui::PushItemWidth(200); // 设置滑动条的宽度
        gui.slider_int(u8"振刀/闪避 间隔", 当前间隔地址(匕首.壁击), 0,
                       300.f);
        gui.combo(当前方式标签, 当前方式地址(匕首.壁击), 当前方式列表, 当前方式列表数量);
        ImGui::TreePop();
      }
    }
    武器配置区(u8"双截棍") {
      if (gui.tree_node(u8"左右三")) {
        gui.slider_float(u8"距离", 当前距离地址(双截棍.左右3), 1.f, 20.f);
        gui.slider_float(u8"延迟", 当前延迟地址(双截棍.左右3), 0.f, 2.f);
        gui.slider_float(u8"角度", 当前角度地址(双截棍.左右3), 1.f, 360.f);

        ImGui::PushItemWidth(200); // 设置滑动条的宽度
        gui.slider_int(u8"振刀/闪避 间隔", 当前间隔地址(双截棍.左右3), 0,
                       300.f);
        gui.combo(当前方式标签, 当前方式地址(双截棍.左右3), 当前方式列表, 当前方式列表数量);
        ImGui::TreePop();
      }
      if (gui.tree_node(u8"左蓄")) {
        gui.slider_float(u8"距离", 当前距离地址(双截棍.左蓄), 1.f, 20.f);
        gui.slider_float(u8"延迟", 当前延迟地址(双截棍.左蓄), 0.f, 2.f);
        gui.slider_float(u8"角度", 当前角度地址(双截棍.左蓄), 1.f, 360.f);

        ImGui::PushItemWidth(200); // 设置滑动条的宽度
        gui.slider_int(u8"振刀/闪避 间隔", 当前间隔地址(双截棍.左蓄), 0,
                       300.f);
        gui.combo(当前方式标签, 当前方式地址(双截棍.左蓄), 当前方式列表, 当前方式列表数量);
        ImGui::TreePop();
      }
      if (gui.tree_node(u8"右蓄")) {
        gui.slider_float(u8"距离", 当前距离地址(双截棍.右蓄), 1.f, 20.f);
        gui.slider_float(u8"延迟", 当前延迟地址(双截棍.右蓄), 0.f, 2.f);
        gui.slider_float(u8"角度", 当前角度地址(双截棍.右蓄), 1.f, 360.f);

        ImGui::PushItemWidth(200); // 设置滑动条的宽度
        gui.slider_int(u8"振刀/闪避 间隔", 当前间隔地址(双截棍.右蓄), 0,
                       300.f);
        gui.combo(当前方式标签, 当前方式地址(双截棍.右蓄), 当前方式列表, 当前方式列表数量);
        ImGui::TreePop();
      }
      if (gui.tree_node(u8"飞踢")) {
        gui.slider_float(u8"距离", 当前距离地址(双截棍.飞踢), 1.f, 20.f);
        gui.slider_float(u8"延迟", 当前延迟地址(双截棍.飞踢), 0.f, 2.f);
        gui.slider_float(u8"角度", 当前角度地址(双截棍.飞踢), 1.f, 360.f);

        ImGui::PushItemWidth(200); // 设置滑动条的宽度
        gui.slider_int(u8"振刀/闪避 间隔", 当前间隔地址(双截棍.飞踢), 0,
                       300.f);
        gui.combo(当前方式标签, 当前方式地址(双截棍.飞踢), 当前方式列表, 当前方式列表数量);
        ImGui::TreePop();
      }
      if (gui.tree_node(u8"扬鞭劲")) {
        gui.slider_float(u8"距离", 当前距离地址(双截棍.扬鞭劲), 1.f, 20.f);
        gui.slider_float(u8"延迟", 当前延迟地址(双截棍.扬鞭劲), 0.f, 2.f);
        gui.slider_float(u8"角度", 当前角度地址(双截棍.扬鞭劲), 1.f, 360.f);

        ImGui::PushItemWidth(200); // 设置滑动条的宽度
        gui.slider_int(u8"振刀/闪避 间隔", 当前间隔地址(双截棍.扬鞭劲), 0,
                       300.f);
        gui.combo(当前方式标签, 当前方式地址(双截棍.扬鞭劲),
                  当前方式列表, 当前方式列表数量);
        ImGui::TreePop();
      }
      if (gui.tree_node(u8"横栏")) {
        gui.slider_float(u8"距离", 当前距离地址(双截棍.横栏), 1.f, 20.f);
        gui.slider_float(u8"延迟", 当前延迟地址(双截棍.横栏), 0.f, 2.f);
        gui.slider_float(u8"角度", 当前角度地址(双截棍.横栏), 1.f, 360.f);

        ImGui::PushItemWidth(200); // 设置滑动条的宽度
        gui.slider_int(u8"振刀/闪避 间隔", 当前间隔地址(双截棍.横栏), 0,
                       300.f);
        gui.combo(当前方式标签, 当前方式地址(双截棍.横栏), 当前方式列表, 当前方式列表数量);
        ImGui::TreePop();
      }
      if (gui.tree_node(u8"龙虎乱舞")) {
        gui.slider_float(u8"距离", 当前距离地址(双截棍.龙虎乱舞), 1.f, 20.f);
        gui.slider_float(u8"延迟", 当前延迟地址(双截棍.龙虎乱舞), 0.f, 2.f);
        gui.slider_float(u8"角度", 当前角度地址(双截棍.龙虎乱舞), 1.f,
                         360.f);

        ImGui::PushItemWidth(200); // 设置滑动条的宽度
        gui.slider_int(u8"振刀/闪避 间隔", 当前间隔地址(双截棍.龙虎乱舞), 0,
                       300.f);
        gui.combo(当前方式标签, 当前方式地址(双截棍.龙虎乱舞),
                  当前方式列表, 当前方式列表数量);
        ImGui::TreePop();
      }
      if (gui.tree_node(u8"壁击")) {
        gui.slider_float(u8"距离", 当前距离地址(双截棍.壁击), 1.f, 20.f);
        gui.slider_float(u8"延迟", 当前延迟地址(双截棍.壁击), 0.f, 2.f);
        gui.slider_float(u8"角度", 当前角度地址(双截棍.壁击), 1.f, 360.f);

        ImGui::PushItemWidth(200); // 设置滑动条的宽度
        gui.slider_int(u8"振刀/闪避 间隔", 当前间隔地址(双截棍.壁击), 0,
                       300.f);
        gui.combo(当前方式标签, 当前方式地址(双截棍.壁击), 当前方式列表, 当前方式列表数量);
        ImGui::TreePop();
      }
    }
    武器配置区(u8"双刀") {
      if (gui.tree_node(u8"左右三")) {
        gui.slider_float(u8"距离", 当前距离地址(双刀.左右3), 1.f, 20.f);
        gui.slider_float(u8"延迟", 当前延迟地址(双刀.左右3), 0.f, 2.f);
        gui.slider_float(u8"角度", 当前角度地址(双刀.左右3), 1.f, 360.f);

        ImGui::PushItemWidth(200); // 设置滑动条的宽度
        gui.slider_int(u8"振刀/闪避 间隔", 当前间隔地址(双刀.左右3), 0,
                       300.f);
        gui.combo(当前方式标签, 当前方式地址(双刀.左右3), 当前方式列表, 当前方式列表数量);
        ImGui::TreePop();
      }
      if (gui.tree_node(u8"左蓄")) {
        gui.slider_float(u8"距离", 当前距离地址(双刀.左蓄), 1.f, 20.f);
        gui.slider_float(u8"延迟", 当前延迟地址(双刀.左蓄), 0.f, 2.f);
        gui.slider_float(u8"角度", 当前角度地址(双刀.左蓄), 1.f, 360.f);

        ImGui::PushItemWidth(200); // 设置滑动条的宽度
        gui.slider_int(u8"振刀/闪避 间隔", 当前间隔地址(双刀.左蓄), 0,
                       300.f);
        gui.combo(当前方式标签, 当前方式地址(双刀.左蓄), 当前方式列表, 当前方式列表数量);
        ImGui::TreePop();
      }
      if (gui.tree_node(u8"右蓄")) {
        gui.slider_float(u8"距离", 当前距离地址(双刀.右蓄), 1.f, 20.f);
        gui.slider_float(u8"延迟", 当前延迟地址(双刀.右蓄), 0.f, 2.f);
        gui.slider_float(u8"角度", 当前角度地址(双刀.右蓄), 1.f, 360.f);

        if (gui.tree_node(u8"1.0")) {
          ImGui::PushItemWidth(200); // 设置滑动条的宽度
          gui.slider_int(u8"振刀/闪避 间隔", 当前间隔地址(双刀.右蓄1), 0,
                         300.f);
          gui.combo(当前方式标签, 当前方式地址(双刀.右蓄1), 当前方式列表, 当前方式列表数量);
          ImGui::TreePop();
        }
        if (gui.tree_node(u8"2.0")) {
          ImGui::PushItemWidth(200); // 设置滑动条的宽度
          gui.slider_int(u8"振刀/闪避 间隔", 当前间隔地址(双刀.右蓄2), 0,
                         300.f);
          gui.combo(当前方式标签, 当前方式地址(双刀.右蓄2), 当前方式列表, 当前方式列表数量);
          ImGui::TreePop();
        }
        ImGui::TreePop();
      }
      if (gui.tree_node(u8"铁马残红")) {
        gui.slider_float(u8"距离", 当前距离地址(双刀.铁马残红), 1.f, 20.f);
        gui.slider_float(u8"延迟", 当前延迟地址(双刀.铁马残红), 0.f, 2.f);
        gui.slider_float(u8"角度", 当前角度地址(双刀.铁马残红), 1.f, 360.f);

        ImGui::PushItemWidth(200); // 设置滑动条的宽度
        gui.slider_int(u8"振刀/闪避 间隔", 当前间隔地址(双刀.铁马残红), 0,
                       300.f);
        gui.combo(当前方式标签, 当前方式地址(双刀.铁马残红),
                  当前方式列表, 当前方式列表数量);
        ImGui::TreePop();
      }
      if (gui.tree_node(u8"乾坤日月斩")) {
        gui.slider_float(u8"距离", 当前距离地址(双刀.乾坤日月斩), 1.f, 20.f);
        gui.slider_float(u8"延迟", 当前延迟地址(双刀.乾坤日月斩), 0.f, 2.f);
        gui.slider_float(u8"角度", 当前角度地址(双刀.乾坤日月斩), 1.f,
                         360.f);

        ImGui::PushItemWidth(200); // 设置滑动条的宽度
        gui.slider_int(u8"振刀/闪避 间隔", 当前间隔地址(双刀.乾坤日月斩), 0,
                       300.f);
        gui.combo(当前方式标签, 当前方式地址(双刀.乾坤日月斩),
                  当前方式列表, 当前方式列表数量);
        ImGui::TreePop();
      }
      if (gui.tree_node(u8"惊雷")) {
        gui.slider_float(u8"距离", 当前距离地址(双刀.惊雷), 1.f, 20.f);
        gui.slider_float(u8"延迟", 当前延迟地址(双刀.惊雷), 0.f, 2.f);
        gui.slider_float(u8"角度", 当前角度地址(双刀.惊雷), 1.f, 360.f);

        ImGui::PushItemWidth(200); // 设置滑动条的宽度
        gui.slider_int(u8"振刀/闪避 间隔", 当前间隔地址(双刀.惊雷), 0,
                       300.f);
        gui.combo(当前方式标签, 当前方式地址(双刀.惊雷), 当前方式列表, 当前方式列表数量);
        ImGui::TreePop();
      }
      if (gui.tree_node(u8"八斩刀")) {
        gui.slider_float(u8"距离", 当前距离地址(双刀.八斩刀), 1.f, 20.f);
        gui.slider_float(u8"延迟", 当前延迟地址(双刀.八斩刀), 0.f, 2.f);
        gui.slider_float(u8"角度", 当前角度地址(双刀.八斩刀), 1.f, 360.f);

        ImGui::PushItemWidth(200); // 设置滑动条的宽度
        gui.slider_int(u8"振刀/闪避 间隔", 当前间隔地址(双刀.八斩刀), 0,
                       300.f);
        gui.combo(当前方式标签, 当前方式地址(双刀.八斩刀), 当前方式列表, 当前方式列表数量);
        ImGui::TreePop();
      }
      if (gui.tree_node(u8"分水斩")) {
        gui.slider_float(u8"距离", 当前距离地址(双刀.分水斩), 1.f, 20.f);
        gui.slider_float(u8"延迟", 当前延迟地址(双刀.分水斩), 0.f, 2.f);
        gui.slider_float(u8"角度", 当前角度地址(双刀.分水斩), 1.f, 360.f);

        ImGui::PushItemWidth(200); // 设置滑动条的宽度
        gui.slider_int(u8"振刀/闪避 间隔", 当前间隔地址(双刀.分水斩), 0,
                       300.f);
        gui.combo(当前方式标签, 当前方式地址(双刀.分水斩), 当前方式列表, 当前方式列表数量);
        ImGui::TreePop();
      }
      if (gui.tree_node(u8"壁击")) {
        gui.slider_float(u8"距离", 当前距离地址(双刀.壁击), 1.f, 20.f);
        gui.slider_float(u8"延迟", 当前延迟地址(双刀.壁击), 0.f, 2.f);
        gui.slider_float(u8"角度", 当前角度地址(双刀.壁击), 1.f, 360.f);

        ImGui::PushItemWidth(200); // 设置滑动条的宽度
        gui.slider_int(u8"振刀/闪避 间隔", 当前间隔地址(双刀.壁击), 0,
                       300.f);
        gui.combo(当前方式标签, 当前方式地址(双刀.壁击), 当前方式列表, 当前方式列表数量);
        ImGui::TreePop();
      }
    }
    武器配置区(u8"长棍") {
      if (gui.tree_node(u8"左三连")) {
        gui.slider_float(u8"距离", 当前距离地址(长棍.左3), 0.f, 20.f);
        gui.slider_float(u8"延迟", 当前延迟地址(长棍.左3), 0.f, 2.f);
        gui.slider_float(u8"角度", 当前角度地址(长棍.左3), 1.f, 360.f);

        ImGui::PushItemWidth(200); // 设置滑动条的宽度
        gui.slider_int(u8"振刀/闪避 间隔", 当前间隔地址(长棍.左3), 0, 300.f);
        gui.combo(当前方式标签, 当前方式地址(长棍.左3), 当前方式列表, 当前方式列表数量);
        ImGui::TreePop();
      }
      if (gui.tree_node(u8"右三连")) {
        gui.slider_float(u8"距离", 当前距离地址(长棍.右3), 0.f, 20.f);
        gui.slider_float(u8"延迟", 当前延迟地址(长棍.右3), 0.f, 2.f);
        gui.slider_float(u8"角度", 当前角度地址(长棍.右3), 1.f, 360.f);

        ImGui::PushItemWidth(200); // 设置滑动条的宽度
        gui.slider_int(u8"振刀/闪避 间隔", 当前间隔地址(长棍.右3), 0, 300.f);
        gui.combo(当前方式标签, 当前方式地址(长棍.右3), 当前方式列表, 当前方式列表数量);
        ImGui::TreePop();
      }
      if (gui.tree_node(u8"左蓄")) {
        gui.slider_float(u8"距离", 当前距离地址(长棍.左蓄), 0.f, 20.f);
        gui.slider_float(u8"延迟", 当前延迟地址(长棍.左蓄), 0.f, 2.f);
        gui.slider_float(u8"角度", 当前角度地址(长棍.左蓄), 1.f, 360.f);

        ImGui::PushItemWidth(200); // 设置滑动条的宽度
        gui.slider_int(u8"振刀/闪避 间隔", 当前间隔地址(长棍.左蓄), 0,
                       300.f);
        gui.combo(当前方式标签, 当前方式地址(长棍.左蓄), 当前方式列表, 当前方式列表数量);
        ImGui::TreePop();
      }
      if (gui.tree_node(u8"右蓄")) {
        gui.slider_float(u8"距离", 当前距离地址(长棍.右蓄), 0.f, 20.f);
        gui.slider_float(u8"延迟", 当前延迟地址(长棍.右蓄), 0.f, 2.f);
        gui.slider_float(u8"角度", 当前角度地址(长棍.右蓄), 1.f, 360.f);

        ImGui::PushItemWidth(200); // 设置滑动条的宽度
        gui.slider_int(u8"振刀/闪避 间隔", 当前间隔地址(长棍.右蓄), 0,
                       300.f);
        gui.combo(当前方式标签, 当前方式地址(长棍.右蓄), 当前方式列表, 当前方式列表数量);
        ImGui::TreePop();
      }
      if (gui.tree_node(u8"腾云式")) {
        gui.slider_float(u8"距离", 当前距离地址(长棍.腾云式), 0.f, 20.f);
        gui.slider_float(u8"延迟", 当前延迟地址(长棍.腾云式), 0.f, 2.f);
        gui.slider_float(u8"角度", 当前角度地址(长棍.腾云式), 1.f, 360.f);

        ImGui::PushItemWidth(200); // 设置滑动条的宽度
        gui.slider_int(u8"振刀/闪避 间隔", 当前间隔地址(长棍.腾云式), 0,
                       300.f);
        gui.combo(当前方式标签, 当前方式地址(长棍.腾云式), 当前方式列表, 当前方式列表数量);
        ImGui::TreePop();
      }
      if (gui.tree_node(u8"乱点天宫")) {
        gui.slider_float(u8"距离", 当前距离地址(长棍.乱点天宫), 0.f, 20.f);
        gui.slider_float(u8"延迟", 当前延迟地址(长棍.乱点天宫), 0.f, 2.f);
        gui.slider_float(u8"角度", 当前角度地址(长棍.乱点天宫), 1.f, 360.f);

        ImGui::PushItemWidth(200); // 设置滑动条的宽度
        gui.slider_int(u8"振刀/闪避 间隔", 当前间隔地址(长棍.乱点天宫), 0,
                       300.f);
        gui.combo(当前方式标签, 当前方式地址(长棍.乱点天宫),
                  当前方式列表, 当前方式列表数量);
        ImGui::TreePop();
      }
      if (gui.tree_node(u8"桶劲")) {
        gui.slider_float(u8"距离", 当前距离地址(长棍.桶劲), 0.f, 20.f);
        gui.slider_float(u8"延迟", 当前延迟地址(长棍.桶劲), 0.f, 2.f);
        gui.slider_float(u8"角度", 当前角度地址(长棍.桶劲), 1.f, 360.f);

        ImGui::PushItemWidth(200); // 设置滑动条的宽度
        gui.slider_int(u8"振刀/闪避 间隔", 当前间隔地址(长棍.桶劲), 0,
                       300.f);
        gui.combo(当前方式标签, 当前方式地址(长棍.桶劲), 当前方式列表, 当前方式列表数量);
        ImGui::TreePop();
      }
      if (gui.tree_node(u8"五情七灭镇")) {
        gui.slider_float(u8"距离", 当前距离地址(长棍.五情七灭镇), 0.f, 20.f);
        gui.slider_float(u8"延迟", 当前延迟地址(长棍.五情七灭镇), 0.f, 2.f);
        gui.slider_float(u8"角度", 当前角度地址(长棍.五情七灭镇), 1.f,
                         360.f);

        ImGui::PushItemWidth(200); // 设置滑动条的宽度
        gui.slider_int(u8"振刀/闪避 间隔", 当前间隔地址(长棍.五情七灭镇), 0,
                       300.f);
        gui.combo(当前方式标签, 当前方式地址(长棍.五情七灭镇),
                  当前方式列表, 当前方式列表数量);
        ImGui::TreePop();
      }
      if (gui.tree_node(u8"双环扫")) {
        gui.slider_float(u8"距离", 当前距离地址(长棍.双环扫), 0.f, 20.f);
        gui.slider_float(u8"延迟", 当前延迟地址(长棍.双环扫), 0.f, 2.f);
        gui.slider_float(u8"角度", 当前角度地址(长棍.双环扫), 1.f, 360.f);

        ImGui::PushItemWidth(200); // 设置滑动条的宽度
        gui.slider_int(u8"振刀/闪避 间隔", 当前间隔地址(长棍.双环扫), 0,
                       300.f);
        gui.combo(当前方式标签, 当前方式地址(长棍.双环扫), 当前方式列表, 当前方式列表数量);
        ImGui::TreePop();
      }
      if (gui.tree_node(u8"少林棍")) {
        gui.slider_float(u8"距离", 当前距离地址(长棍.少林棍), 0.f, 20.f);
        gui.slider_float(u8"延迟", 当前延迟地址(长棍.少林棍), 0.f, 2.f);
        gui.slider_float(u8"角度", 当前角度地址(长棍.少林棍), 1.f, 360.f);

        ImGui::PushItemWidth(200); // 设置滑动条的宽度
        gui.slider_int(u8"振刀/闪避 间隔", 当前间隔地址(长棍.少林棍), 0,
                       300.f);
        gui.combo(当前方式标签, 当前方式地址(长棍.少林棍), 当前方式列表, 当前方式列表数量);
        ImGui::TreePop();
      }
      if (gui.tree_node(u8"镇地撑天")) {
        gui.slider_float(u8"距离", 当前距离地址(长棍.镇地撑天), 0.f, 20.f);
        gui.slider_float(u8"延迟", 当前延迟地址(长棍.镇地撑天), 0.f, 2.f);
        gui.slider_float(u8"角度", 当前角度地址(长棍.镇地撑天), 1.f, 360.f);

        ImGui::PushItemWidth(200); // 设置滑动条的宽度
        gui.slider_int(u8"振刀/闪避 间隔", 当前间隔地址(长棍.镇地撑天), 0,
                       300.f);
        gui.combo(当前方式标签, 当前方式地址(长棍.镇地撑天),
                  当前方式列表, 当前方式列表数量);
        ImGui::TreePop();
      }
      if (gui.tree_node(u8"壁击")) {
        gui.slider_float(u8"距离", 当前距离地址(长棍.壁击), 1.f, 20.f);
        gui.slider_float(u8"延迟", 当前延迟地址(长棍.壁击), 0.f, 2.f);
        gui.slider_float(u8"角度", 当前角度地址(长棍.壁击), 1.f, 360.f);

        ImGui::PushItemWidth(200); // 设置滑动条的宽度
        gui.slider_int(u8"振刀/闪避 间隔", 当前间隔地址(长棍.壁击), 0,
                       300.f);
        gui.combo(当前方式标签, 当前方式地址(长棍.壁击), 当前方式列表, 当前方式列表数量);
        ImGui::TreePop();
      }
      if (gui.tree_node(u8"同源")) {
        gui.slider_float(u8"距离", 当前距离地址(长棍.同源), 1.f, 20.f);
        gui.slider_float(u8"延迟", 当前延迟地址(长棍.同源), 0.f, 2.f);
        gui.slider_float(u8"角度", 当前角度地址(长棍.同源), 1.f, 360.f);

        ImGui::PushItemWidth(200); // 设置滑动条的宽度
        gui.slider_int(u8"振刀/闪避 间隔", 当前间隔地址(长棍.同源), 0,
                       300.f);
        gui.combo(当前方式标签, 当前方式地址(长棍.同源), 当前方式列表, 当前方式列表数量);
        ImGui::TreePop();
      }
    }
    武器配置区(u8"斩马刀") {
      if (gui.tree_node(u8"左左")) {
        gui.slider_float(u8"距离", 当前距离地址(斩马刀.左左), 1.f, 20.f);
        gui.slider_float(u8"延迟", 当前延迟地址(斩马刀.左左), 0.f, 2.f);
        gui.slider_float(u8"角度", 当前角度地址(斩马刀.左左), 1.f, 360.f);

        ImGui::PushItemWidth(200); // 设置滑动条的宽度
        gui.slider_int(u8"振刀/闪避 间隔", 当前间隔地址(斩马刀.左左), 0,
                       300.f);
        gui.combo(当前方式标签, 当前方式地址(斩马刀.左左), 当前方式列表, 当前方式列表数量);
        ImGui::TreePop();
      }
      if (gui.tree_node(u8"左右")) {
        gui.slider_float(u8"距离", 当前距离地址(斩马刀.左右), 1.f, 20.f);
        gui.slider_float(u8"延迟", 当前延迟地址(斩马刀.左右), 0.f, 2.f);
        gui.slider_float(u8"角度", 当前角度地址(斩马刀.左右), 1.f, 360.f);

        ImGui::PushItemWidth(200); // 设置滑动条的宽度
        gui.slider_int(u8"振刀/闪避 间隔", 当前间隔地址(斩马刀.左右), 0,
                       300.f);
        gui.combo(当前方式标签, 当前方式地址(斩马刀.左右), 当前方式列表, 当前方式列表数量);
        ImGui::TreePop();
      }
      if (gui.tree_node(u8"右右")) {
        gui.slider_float(u8"距离", 当前距离地址(斩马刀.右右), 1.f, 20.f);
        gui.slider_float(u8"延迟", 当前延迟地址(斩马刀.右右), 0.f, 2.f);
        gui.slider_float(u8"角度", 当前角度地址(斩马刀.右右), 1.f, 360.f);

        ImGui::PushItemWidth(200); // 设置滑动条的宽度
        gui.slider_int(u8"振刀/闪避 间隔", 当前间隔地址(斩马刀.右右), 0,
                       300.f);
        gui.combo(当前方式标签, 当前方式地址(斩马刀.右右), 当前方式列表, 当前方式列表数量);
        ImGui::TreePop();
      }
      if (gui.tree_node(u8"右左")) {
        gui.slider_float(u8"距离", 当前距离地址(斩马刀.右左), 1.f, 20.f);
        gui.slider_float(u8"延迟", 当前延迟地址(斩马刀.右左), 0.f, 2.f);
        gui.slider_float(u8"角度", 当前角度地址(斩马刀.右左), 1.f, 360.f);

        ImGui::PushItemWidth(200); // 设置滑动条的宽度
        gui.slider_int(u8"振刀/闪避 间隔", 当前间隔地址(斩马刀.右左), 0,
                       300.f);
        gui.combo(当前方式标签, 当前方式地址(斩马刀.右左), 当前方式列表, 当前方式列表数量);
        ImGui::TreePop();
      }
      if (gui.tree_node(u8"左蓄")) {
        gui.slider_float(u8"距离", 当前距离地址(斩马刀.左蓄), 1.f, 20.f);
        gui.slider_float(u8"延迟", 当前延迟地址(斩马刀.左蓄), 0.f, 2.f);
        gui.slider_float(u8"角度", 当前角度地址(斩马刀.左蓄), 1.f, 360.f);

        if (gui.tree_node(u8"1.0")) {
          ImGui::PushItemWidth(200); // 设置滑动条的宽度
          gui.slider_int(u8"振刀/闪避 间隔", 当前间隔地址(斩马刀.左蓄1), 0,
                         300.f);
          gui.combo(当前方式标签, 当前方式地址(斩马刀.左蓄1),
                    当前方式列表, 当前方式列表数量);
          ImGui::TreePop();
        }
        if (gui.tree_node(u8"2.0")) {
          ImGui::PushItemWidth(200); // 设置滑动条的宽度
          gui.slider_int(u8"振刀/闪避 间隔", 当前间隔地址(斩马刀.左蓄2), 0,
                         300.f);
          gui.combo(当前方式标签, 当前方式地址(斩马刀.左蓄2),
                    当前方式列表, 当前方式列表数量);
          ImGui::TreePop();
        }
        if (gui.tree_node(u8"3.0")) {
          ImGui::PushItemWidth(200); // 设置滑动条的宽度
          gui.slider_int(u8"振刀/闪避 间隔", 当前间隔地址(斩马刀.左蓄3), 0,
                         300.f);
          gui.combo(当前方式标签, 当前方式地址(斩马刀.左蓄3),
                    当前方式列表, 当前方式列表数量);
          ImGui::TreePop();
        }
        ImGui::TreePop();
      }
      if (gui.tree_node(u8"右蓄")) {
        gui.slider_float(u8"距离", 当前距离地址(斩马刀.右蓄), 1.f, 20.f);
        gui.slider_float(u8"延迟", 当前延迟地址(斩马刀.右蓄), 0.f, 2.f);
        gui.slider_float(u8"角度", 当前角度地址(斩马刀.右蓄), 1.f, 360.f);

        if (gui.tree_node(u8"1.0")) {
          ImGui::PushItemWidth(200); // 设置滑动条的宽度
          gui.slider_int(u8"振刀/闪避 间隔", 当前间隔地址(斩马刀.右蓄1), 0,
                         300.f);
          gui.combo(当前方式标签, 当前方式地址(斩马刀.右蓄1),
                    当前方式列表, 当前方式列表数量);
          ImGui::TreePop();
        }
        if (gui.tree_node(u8"2.0")) {
          ImGui::PushItemWidth(200); // 设置滑动条的宽度
          gui.slider_int(u8"振刀/闪避 间隔", 当前间隔地址(斩马刀.右蓄2), 0,
                         300.f);
          gui.combo(当前方式标签, 当前方式地址(斩马刀.右蓄2),
                    当前方式列表, 当前方式列表数量);
          ImGui::TreePop();
        }
        if (gui.tree_node(u8"3.0")) {
          ImGui::PushItemWidth(200); // 设置滑动条的宽度
          gui.slider_int(u8"振刀/闪避 间隔", 当前间隔地址(斩马刀.右蓄3), 0,
                         300.f);
          gui.combo(当前方式标签, 当前方式地址(斩马刀.右蓄3),
                    当前方式列表, 当前方式列表数量);
          ImGui::TreePop();
        }
        ImGui::TreePop();
      }
      if (gui.tree_node(u8"柄击")) {
        gui.slider_float(u8"距离", 当前距离地址(斩马刀.柄击), 1.f, 20.f);
        gui.slider_float(u8"延迟", 当前延迟地址(斩马刀.柄击), 0.f, 2.f);
        gui.slider_float(u8"角度", 当前角度地址(斩马刀.柄击), 1.f, 360.f);

        ImGui::PushItemWidth(200); // 设置滑动条的宽度
        gui.slider_int(u8"振刀/闪避 间隔", 当前间隔地址(斩马刀.柄击), 0,
                       300.f);
        gui.combo(当前方式标签, 当前方式地址(斩马刀.柄击), 当前方式列表, 当前方式列表数量);
        ImGui::TreePop();
      }
      if (gui.tree_node(u8"惊雷")) {
        gui.slider_float(u8"距离", 当前距离地址(斩马刀.惊雷), 1.f, 20.f);
        gui.slider_float(u8"延迟", 当前延迟地址(斩马刀.惊雷), 0.f, 2.f);
        gui.slider_float(u8"角度", 当前角度地址(斩马刀.惊雷), 1.f, 360.f);

        ImGui::PushItemWidth(200); // 设置滑动条的宽度
        gui.slider_int(u8"振刀/闪避 间隔", 当前间隔地址(斩马刀.惊雷), 0,
                       300.f);
        gui.combo(当前方式标签, 当前方式地址(斩马刀.惊雷), 当前方式列表, 当前方式列表数量);
        ImGui::TreePop();
      }
      if (gui.tree_node(u8"炽焰斩")) {
        gui.slider_float(u8"距离", 当前距离地址(斩马刀.炽焰斩), 1.f, 20.f);
        gui.slider_float(u8"延迟", 当前延迟地址(斩马刀.炽焰斩), 0.f, 2.f);
        gui.slider_float(u8"角度", 当前角度地址(斩马刀.炽焰斩), 1.f, 360.f);

        ImGui::PushItemWidth(200); // 设置滑动条的宽度
        gui.slider_int(u8"振刀/闪避 间隔", 当前间隔地址(斩马刀.炽焰斩), 0,
                       300.f);
        gui.combo(当前方式标签, 当前方式地址(斩马刀.炽焰斩),
                  当前方式列表, 当前方式列表数量);
        ImGui::TreePop();
      }
      if (gui.tree_node(u8"过关斩将")) {
        gui.slider_float(u8"距离", 当前距离地址(斩马刀.过关斩将), 1.f, 20.f);
        gui.slider_float(u8"延迟", 当前延迟地址(斩马刀.过关斩将), 0.f, 2.f);
        gui.slider_float(u8"角度", 当前角度地址(斩马刀.过关斩将), 1.f,
                         360.f);

        ImGui::PushItemWidth(200); // 设置滑动条的宽度
        gui.slider_int(u8"振刀/闪避 间隔", 当前间隔地址(斩马刀.过关斩将), 0,
                       300.f);
        gui.combo(当前方式标签, 当前方式地址(斩马刀.过关斩将),
                  当前方式列表, 当前方式列表数量);
        ImGui::TreePop();
      }
      if (gui.tree_node(u8"壁击")) {
        gui.slider_float(u8"距离", 当前距离地址(斩马刀.壁击), 1.f, 20.f);
        gui.slider_float(u8"延迟", 当前延迟地址(斩马刀.壁击), 0.f, 2.f);
        gui.slider_float(u8"角度", 当前角度地址(斩马刀.壁击), 1.f, 360.f);

        ImGui::PushItemWidth(200); // 设置滑动条的宽度
        gui.slider_int(u8"振刀/闪避 间隔", 当前间隔地址(斩马刀.壁击), 0,
                       300.f);
        gui.combo(当前方式标签, 当前方式地址(斩马刀.壁击), 当前方式列表, 当前方式列表数量);
        ImGui::TreePop();
      }
      if (gui.tree_node(u8"同源")) {
        gui.slider_float(u8"距离", 当前距离地址(斩马刀.同源), 1.f, 20.f);
        gui.slider_float(u8"延迟", 当前延迟地址(斩马刀.同源), 0.f, 2.f);
        gui.slider_float(u8"角度", 当前角度地址(斩马刀.同源), 1.f, 360.f);

        ImGui::PushItemWidth(200); // 设置滑动条的宽度
        gui.slider_int(u8"振刀/闪避 间隔", 当前间隔地址(斩马刀.同源), 0,
                       300.f);
        gui.combo(当前方式标签, 当前方式地址(斩马刀.同源), 当前方式列表, 当前方式列表数量);
        ImGui::TreePop();
      }
    }
    武器配置区(u8"双戟") {
      if (gui.tree_node(u8"左右三")) {
        gui.slider_float(u8"距离", 当前距离地址(双戟.左右3), 0.f, 20.f);
        gui.slider_float(u8"延迟", 当前延迟地址(双戟.左右3), 0.f, 2.f);
        gui.slider_float(u8"角度", 当前角度地址(双戟.左右3), 1.f, 360.f);

        if (gui.tree_node(u8"左键")) {
          ImGui::PushItemWidth(200); // 设置滑动条的宽度
          gui.slider_int(u8"振刀/闪避 间隔", 当前间隔地址(双戟.左3), 0,
                         300.f);
          gui.combo(当前方式标签, 当前方式地址(双戟.左3), 当前方式列表, 当前方式列表数量);
          ImGui::TreePop();
        }
        if (gui.tree_node(u8"右键")) {
          ImGui::PushItemWidth(200); // 设置滑动条的宽度
          gui.slider_int(u8"振刀/闪避 间隔", 当前间隔地址(双戟.右3), 0,
                         300.f);
          gui.combo(当前方式标签, 当前方式地址(双戟.右3), 当前方式列表, 当前方式列表数量);
          ImGui::TreePop();
        }
        ImGui::TreePop();
      }
      if (gui.tree_node(u8"左蓄")) {
        gui.slider_float(u8"距离", 当前距离地址(双戟.左蓄), 0.f, 20.f);
        gui.slider_float(u8"延迟", 当前延迟地址(双戟.左蓄), 0.f, 2.f);
        gui.slider_float(u8"角度", 当前角度地址(双戟.左蓄), 1.f, 360.f);

        ImGui::PushItemWidth(200); // 设置滑动条的宽度
        gui.slider_int(u8"振刀/闪避 间隔", 当前间隔地址(双戟.左蓄), 0,
                       300.f);
        gui.combo(当前方式标签, 当前方式地址(双戟.左蓄), 当前方式列表, 当前方式列表数量);
        ImGui::TreePop();
      }
      if (gui.tree_node(u8"右蓄")) {
        gui.slider_float(u8"距离", 当前距离地址(双戟.右蓄), 0.f, 20.f);
        gui.slider_float(u8"延迟", 当前延迟地址(双戟.右蓄), 0.f, 2.f);
        gui.slider_float(u8"角度", 当前角度地址(双戟.右蓄), 1.f, 360.f);

        if (gui.tree_node(u8"1.0")) {
          ImGui::PushItemWidth(200); // 设置滑动条的宽度
          gui.slider_int(u8"振刀/闪避 间隔", 当前间隔地址(双戟.右蓄1), 0,
                         300.f);
          gui.combo(当前方式标签, 当前方式地址(双戟.右蓄1), 当前方式列表, 当前方式列表数量);
          ImGui::TreePop();
        }
        if (gui.tree_node(u8"2.0")) {
          ImGui::PushItemWidth(200); // 设置滑动条的宽度
          gui.slider_int(u8"振刀/闪避 间隔", 当前间隔地址(双戟.右蓄2), 0,
                         300.f);
          gui.combo(当前方式标签, 当前方式地址(双戟.右蓄2), 当前方式列表, 当前方式列表数量);
          ImGui::TreePop();
        }
        ImGui::TreePop();
      }
      if (gui.tree_node(u8"勾旋斩")) {
        gui.slider_float(u8"距离", 当前距离地址(双戟.勾旋斩), 0.f, 20.f);
        gui.slider_float(u8"延迟", 当前延迟地址(双戟.勾旋斩), 0.f, 2.f);
        gui.slider_float(u8"角度", 当前角度地址(双戟.勾旋斩), 1.f, 360.f);

        ImGui::PushItemWidth(200); // 设置滑动条的宽度
        gui.slider_int(u8"振刀/闪避 间隔", 当前间隔地址(双戟.勾旋斩), 0,
                       300.f);
        gui.combo(当前方式标签, 当前方式地址(双戟.勾旋斩), 当前方式列表, 当前方式列表数量);
        ImGui::TreePop();
      }
      if (gui.tree_node(u8"探海蛟")) {
        gui.slider_float(u8"距离", 当前距离地址(双戟.探海蛟), 0.f, 20.f);
        gui.slider_float(u8"延迟", 当前延迟地址(双戟.探海蛟), 0.f, 2.f);
        gui.slider_float(u8"角度", 当前角度地址(双戟.探海蛟), 1.f, 360.f);

        ImGui::PushItemWidth(200); // 设置滑动条的宽度
        gui.slider_int(u8"振刀/闪避 间隔", 当前间隔地址(双戟.探海蛟), 0,
                       300.f);
        gui.combo(当前方式标签, 当前方式地址(双戟.探海蛟), 当前方式列表, 当前方式列表数量);
        ImGui::TreePop();
      }
      if (gui.tree_node(u8"钩挂")) {
        gui.slider_float(u8"距离", 当前距离地址(双戟.钩挂), 0.f, 20.f);
        gui.slider_float(u8"延迟", 当前延迟地址(双戟.钩挂), 0.f, 2.f);
        gui.slider_float(u8"角度", 当前角度地址(双戟.钩挂), 1.f, 360.f);

        ImGui::PushItemWidth(200); // 设置滑动条的宽度
        gui.slider_int(u8"振刀/闪避 间隔", 当前间隔地址(双戟.钩挂), 0,
                       300.f);
        gui.combo(当前方式标签, 当前方式地址(双戟.钩挂), 当前方式列表, 当前方式列表数量);
        ImGui::TreePop();
      }
      if (gui.tree_node(u8"战龙在天")) {
        gui.slider_float(u8"距离", 当前距离地址(双戟.战龙在天), 0.f, 20.f);
        gui.slider_float(u8"延迟", 当前延迟地址(双戟.战龙在天), 0.f, 2.f);
        gui.slider_float(u8"角度", 当前角度地址(双戟.战龙在天), 1.f, 360.f);

        ImGui::PushItemWidth(200); // 设置滑动条的宽度
        gui.slider_int(u8"振刀/闪避 间隔", 当前间隔地址(双戟.战龙在天), 0,
                       300.f);
        gui.combo(当前方式标签, 当前方式地址(双戟.战龙在天),
                  当前方式列表, 当前方式列表数量);
        ImGui::TreePop();
      }
      if (gui.tree_node(u8"形意钩")) {
        gui.slider_float(u8"距离", 当前距离地址(双戟.形意钩), 0.f, 20.f);
        gui.slider_float(u8"延迟", 当前延迟地址(双戟.形意钩), 0.f, 2.f);
        gui.slider_float(u8"角度", 当前角度地址(双戟.形意钩), 1.f, 360.f);

        ImGui::PushItemWidth(200); // 设置滑动条的宽度
        gui.slider_int(u8"振刀/闪避 间隔", 当前间隔地址(双戟.形意钩), 0,
                       300.f);
        gui.combo(当前方式标签, 当前方式地址(双戟.形意钩), 当前方式列表, 当前方式列表数量);
        ImGui::TreePop();
      }
      if (gui.tree_node(u8"壁击")) {
        gui.slider_float(u8"距离", 当前距离地址(双戟.壁击), 1.f, 20.f);
        gui.slider_float(u8"延迟", 当前延迟地址(双戟.壁击), 0.f, 2.f);
        gui.slider_float(u8"角度", 当前角度地址(双戟.壁击), 1.f, 360.f);

        ImGui::PushItemWidth(200); // 设置滑动条的宽度
        gui.slider_int(u8"振刀/闪避 间隔", 当前间隔地址(双戟.壁击), 0,
                       300.f);
        gui.combo(当前方式标签, 当前方式地址(双戟.壁击), 当前方式列表, 当前方式列表数量);
        ImGui::TreePop();
      }
    }
    武器配置区(u8"扇子") {
      if (gui.tree_node(u8"左三")) {
        gui.slider_float(u8"距离", 当前距离地址(扇子.左3), 0.f, 20.f);
        gui.slider_float(u8"延迟", 当前延迟地址(扇子.左3), 0.f, 2.f);
        gui.slider_float(u8"角度", 当前角度地址(扇子.左3), 1.f, 360.f);

        ImGui::PushItemWidth(200); // 设置滑动条的宽度
        gui.slider_int(u8"振刀/闪避 间隔", 当前间隔地址(扇子.左3), 0, 300.f);
        gui.combo(当前方式标签, 当前方式地址(扇子.左3), 当前方式列表, 当前方式列表数量);
        ImGui::TreePop();
      }
      if (gui.tree_node(u8"右三")) {
        gui.slider_float(u8"距离", 当前距离地址(扇子.右3), 0.f, 20.f);
        gui.slider_float(u8"延迟", 当前延迟地址(扇子.右3), 0.f, 2.f);
        gui.slider_float(u8"角度", 当前角度地址(扇子.右3), 1.f, 360.f);

        ImGui::PushItemWidth(200); // 设置滑动条的宽度
        gui.slider_int(u8"振刀/闪避 间隔", 当前间隔地址(扇子.右3), 0, 300.f);
        gui.combo(当前方式标签, 当前方式地址(扇子.右3), 当前方式列表, 当前方式列表数量);
        ImGui::TreePop();
      }
      if (gui.tree_node(u8"左蓄")) {
        gui.slider_float(u8"距离", 当前距离地址(扇子.左蓄), 0.f, 20.f);
        gui.slider_float(u8"延迟", 当前延迟地址(扇子.左蓄), 0.f, 2.f);
        gui.slider_float(u8"角度", 当前角度地址(扇子.左蓄), 1.f, 360.f);

        ImGui::PushItemWidth(200); // 设置滑动条的宽度
        gui.slider_int(u8"振刀/闪避 间隔", 当前间隔地址(扇子.左蓄), 0,
                       300.f);
        gui.combo(当前方式标签, 当前方式地址(扇子.左蓄), 当前方式列表, 当前方式列表数量);
        ImGui::TreePop();
      }
      if (gui.tree_node(u8"右蓄")) {
        gui.slider_float(u8"距离", 当前距离地址(扇子.右蓄), 0.f, 20.f);
        gui.slider_float(u8"延迟", 当前延迟地址(扇子.右蓄), 0.f, 2.f);
        gui.slider_float(u8"角度", 当前角度地址(扇子.右蓄), 1.f, 360.f);

        ImGui::PushItemWidth(200); // 设置滑动条的宽度
        gui.slider_int(u8"振刀/闪避 间隔", 当前间隔地址(扇子.右蓄), 0,
                       300.f);
        gui.combo(当前方式标签, 当前方式地址(扇子.右蓄), 当前方式列表, 当前方式列表数量);
        ImGui::TreePop();
      }
      if (gui.tree_node(u8"双开圆")) {
        gui.slider_float(u8"距离", 当前距离地址(扇子.双开圆), 0.f, 20.f);
        gui.slider_float(u8"延迟", 当前延迟地址(扇子.双开圆), 0.f, 2.f);
        gui.slider_float(u8"角度", 当前角度地址(扇子.双开圆), 1.f, 360.f);

        ImGui::PushItemWidth(200); // 设置滑动条的宽度
        gui.slider_int(u8"振刀/闪避 间隔", 当前间隔地址(扇子.双开圆), 0,
                       300.f);
        gui.combo(当前方式标签, 当前方式地址(扇子.双开圆), 当前方式列表, 当前方式列表数量);
        ImGui::TreePop();
      }
      if (gui.tree_node(u8"三风摆")) {
        gui.slider_float(u8"距离", 当前距离地址(扇子.三风摆), 0.f, 20.f);
        gui.slider_float(u8"延迟", 当前延迟地址(扇子.三风摆), 0.f, 2.f);
        gui.slider_float(u8"角度", 当前角度地址(扇子.三风摆), 1.f, 360.f);

        ImGui::PushItemWidth(200); // 设置滑动条的宽度
        gui.slider_int(u8"振刀/闪避 间隔", 当前间隔地址(扇子.三风摆), 0,
                       300.f);
        gui.combo(当前方式标签, 当前方式地址(扇子.三风摆), 当前方式列表, 当前方式列表数量);
        ImGui::TreePop();
      }
      if (gui.tree_node(u8"惊雷")) {
        gui.slider_float(u8"距离", 当前距离地址(扇子.惊雷), 0.f, 20.f);
        gui.slider_float(u8"延迟", 当前延迟地址(扇子.惊雷), 0.f, 2.f);
        gui.slider_float(u8"3M内角度", 当前角度地址(扇子.惊雷), 1.f, 360.f);

        ImGui::PushItemWidth(200); // 设置滑动条的宽度
        gui.slider_int(u8"振刀/闪避 间隔", 当前间隔地址(扇子.惊雷), 0,
                       300.f);
        gui.combo(当前方式标签, 当前方式地址(扇子.惊雷), 当前方式列表, 当前方式列表数量);
        ImGui::TreePop();
      }
      if (gui.tree_node(u8"缠龙奔野")) {
        gui.slider_float(u8"距离", 当前距离地址(扇子.缠龙奔野), 0.f, 20.f);
        gui.slider_float(u8"延迟", 当前延迟地址(扇子.缠龙奔野), 0.f, 2.f);
        gui.slider_float(u8"角度", 当前角度地址(扇子.缠龙奔野), 1.f, 360.f);

        ImGui::PushItemWidth(200); // 设置滑动条的宽度
        gui.slider_int(u8"振刀/闪避 间隔", 当前间隔地址(扇子.缠龙奔野), 0,
                       300.f);
        gui.combo(当前方式标签, 当前方式地址(扇子.缠龙奔野),
                  当前方式列表, 当前方式列表数量);
        ImGui::TreePop();
      }
      if (gui.tree_node(u8"鬼返")) {
        gui.slider_float(u8"距离", 当前距离地址(扇子.鬼反), 0.f, 20.f);
        gui.slider_float(u8"延迟", 当前延迟地址(扇子.鬼反), 0.f, 2.f);
        gui.slider_float(u8"角度", 当前角度地址(扇子.鬼反), 1.f, 360.f);

        ImGui::PushItemWidth(200); // 设置滑动条的宽度
        gui.slider_int(u8"振刀/闪避 间隔", 当前间隔地址(扇子.鬼反), 0,
                       300.f);
        gui.combo(当前方式标签, 当前方式地址(扇子.鬼反), 当前方式列表, 当前方式列表数量);
        ImGui::TreePop();
      }
      if (gui.tree_node(u8"壁击")) {
        gui.slider_float(u8"距离", 当前距离地址(扇子.壁击), 1.f, 20.f);
        gui.slider_float(u8"延迟", 当前延迟地址(扇子.壁击), 0.f, 2.f);
        gui.slider_float(u8"角度", 当前角度地址(扇子.壁击), 1.f, 360.f);

        ImGui::PushItemWidth(200); // 设置滑动条的宽度
        gui.slider_int(u8"振刀/闪避 间隔", 当前间隔地址(扇子.壁击), 0,
                       300.f);
        gui.combo(当前方式标签, 当前方式地址(扇子.壁击), 当前方式列表, 当前方式列表数量);
        ImGui::TreePop();
      }
    }
    武器配置区(u8"横刀") {
      if (gui.tree_node(u8"左三")) {
        gui.slider_float(u8"距离", 当前距离地址(横刀.左3), 0.f, 20.f);
        gui.slider_float(u8"延迟", 当前延迟地址(横刀.左3), 0.f, 2.f);
        gui.slider_float(u8"角度", 当前角度地址(横刀.左3), 1.f, 360.f);

        ImGui::PushItemWidth(200); // 设置滑动条的宽度
        gui.slider_int(u8"振刀/闪避 间隔", 当前间隔地址(横刀.左3), 0, 300.f);
        gui.combo(当前方式标签, 当前方式地址(横刀.左3), 当前方式列表, 当前方式列表数量);
        ImGui::TreePop();
      }
      if (gui.tree_node(u8"右三")) {
        gui.slider_float(u8"距离", 当前距离地址(横刀.右3), 0.f, 20.f);
        gui.slider_float(u8"延迟", 当前延迟地址(横刀.右3), 0.f, 2.f);
        gui.slider_float(u8"角度", 当前角度地址(横刀.右3), 1.f, 360.f);

        ImGui::PushItemWidth(200); // 设置滑动条的宽度
        gui.slider_int(u8"振刀/闪避 间隔", 当前间隔地址(横刀.右3), 0, 300.f);
        gui.combo(当前方式标签, 当前方式地址(横刀.右3), 当前方式列表, 当前方式列表数量);
        ImGui::TreePop();
      }
      if (gui.tree_node(u8"左蓄")) {
        gui.slider_float(u8"距离", 当前距离地址(横刀.左蓄), 0.f, 20.f);
        gui.slider_float(u8"延迟", 当前延迟地址(横刀.左蓄), 0.f, 2.f);
        gui.slider_float(u8"角度", 当前角度地址(横刀.左蓄), 1.f, 360.f);

        ImGui::PushItemWidth(200); // 设置滑动条的宽度
        gui.slider_int(u8"振刀/闪避 间隔", 当前间隔地址(横刀.左蓄), 0,
                       300.f);
        gui.combo(当前方式标签, 当前方式地址(横刀.左蓄), 当前方式列表, 当前方式列表数量);
        ImGui::TreePop();
      }
      if (gui.tree_node(u8"右蓄")) {
        gui.slider_float(u8"距离", 当前距离地址(横刀.右蓄), 0.f, 20.f);
        gui.slider_float(u8"延迟", 当前延迟地址(横刀.右蓄), 0.f, 2.f);
        gui.slider_float(u8"角度", 当前角度地址(横刀.右蓄), 1.f, 360.f);

        if (gui.tree_node(u8"1.0")) {
          ImGui::PushItemWidth(200); // 设置滑动条的宽度
          gui.slider_int(u8"振刀/闪避 间隔", 当前间隔地址(横刀.右蓄1), 0,
                         300.f);
          gui.combo(当前方式标签, 当前方式地址(横刀.右蓄1), 当前方式列表, 当前方式列表数量);
          ImGui::TreePop();
        }
        if (gui.tree_node(u8"2.0")) {
          ImGui::PushItemWidth(200); // 设置滑动条的宽度
          gui.slider_int(u8"振刀/闪避 间隔", 当前间隔地址(横刀.右蓄2), 0,
                         300.f);
          gui.combo(当前方式标签, 当前方式地址(横刀.右蓄2), 当前方式列表, 当前方式列表数量);
          ImGui::TreePop();
        }
        ImGui::TreePop();
      }
      if (gui.tree_node(u8"苍牙")) {
        gui.slider_float(u8"距离", 当前距离地址(横刀.苍牙), 0.f, 20.f);
        gui.slider_float(u8"延迟", 当前延迟地址(横刀.苍牙), 0.f, 2.f);
        gui.slider_float(u8"角度", 当前角度地址(横刀.苍牙), 1.f, 360.f);

        ImGui::PushItemWidth(200); // 设置滑动条的宽度
        gui.slider_int(u8"振刀/闪避 间隔", 当前间隔地址(横刀.苍牙), 0,
                       300.f);
        gui.combo(当前方式标签, 当前方式地址(横刀.苍牙), 当前方式列表, 当前方式列表数量);
        ImGui::TreePop();
      }
      if (gui.tree_node(u8"踏空闪")) {
        gui.slider_float(u8"距离", 当前距离地址(横刀.踏空闪), 0.f, 20.f);
        gui.slider_float(u8"延迟", 当前延迟地址(横刀.踏空闪), 0.f, 2.f);
        gui.slider_float(u8"角度", 当前角度地址(横刀.踏空闪), 1.f, 360.f);

        ImGui::PushItemWidth(200); // 设置滑动条的宽度
        gui.slider_int(u8"振刀/闪避 间隔", 当前间隔地址(横刀.踏空闪), 0,
                       300.f);
        gui.combo(当前方式标签, 当前方式地址(横刀.踏空闪), 当前方式列表, 当前方式列表数量);
        ImGui::TreePop();
      }
      if (gui.tree_node(u8"破千军")) {
        gui.slider_float(u8"距离", 当前距离地址(横刀.破千军), 0.f, 20.f);
        gui.slider_float(u8"延迟", 当前延迟地址(横刀.破千军), 0.f, 2.f);
        gui.slider_float(u8"角度", 当前角度地址(横刀.破千军), 1.f, 360.f);

        ImGui::PushItemWidth(200); // 设置滑动条的宽度
        gui.slider_int(u8"振刀/闪避 间隔", 当前间隔地址(横刀.破千军), 0,
                       300.f);
        gui.combo(当前方式标签, 当前方式地址(横刀.破千军), 当前方式列表, 当前方式列表数量);
        ImGui::TreePop();
      }
      if (gui.tree_node(u8"极光破云闪")) {
        gui.slider_float(u8"距离", 当前距离地址(横刀.极光破云闪), 0.f, 20.f);
        gui.slider_float(u8"延迟", 当前延迟地址(横刀.极光破云闪), 0.f, 2.f);
        gui.slider_float(u8"角度", 当前角度地址(横刀.极光破云闪), 1.f,
                         360.f);

        ImGui::PushItemWidth(200); // 设置滑动条的宽度
        gui.slider_int(u8"振刀/闪避 间隔", 当前间隔地址(横刀.极光破云闪), 0,
                       300.f);
        gui.combo(当前方式标签, 当前方式地址(横刀.极光破云闪),
                  当前方式列表, 当前方式列表数量);
        ImGui::TreePop();
      }
      if (gui.tree_node(u8"壁击")) {
        gui.slider_float(u8"距离", 当前距离地址(横刀.壁击), 1.f, 20.f);
        gui.slider_float(u8"延迟", 当前延迟地址(横刀.壁击), 0.f, 2.f);
        gui.slider_float(u8"角度", 当前角度地址(横刀.壁击), 1.f, 360.f);

        ImGui::PushItemWidth(200); // 设置滑动条的宽度
        gui.slider_int(u8"振刀/闪避 间隔", 当前间隔地址(横刀.壁击), 0,
                       300.f);
        gui.combo(当前方式标签, 当前方式地址(横刀.壁击), 当前方式列表, 当前方式列表数量);
        ImGui::TreePop();
      }
      if (gui.tree_node(u8"同源")) {
        gui.slider_float(u8"距离", 当前距离地址(横刀.同源), 1.f, 20.f);
        gui.slider_float(u8"延迟", 当前延迟地址(横刀.同源), 0.f, 2.f);
        gui.slider_float(u8"角度", 当前角度地址(横刀.同源), 1.f, 360.f);

        ImGui::PushItemWidth(200); // 设置滑动条的宽度
        gui.slider_int(u8"振刀/闪避 间隔", 当前间隔地址(横刀.同源), 0,
                       300.f);
        gui.combo(当前方式标签, 当前方式地址(横刀.同源), 当前方式列表, 当前方式列表数量);
        ImGui::TreePop();
      }
    }
    武器配置区(u8"拳刃") {
      if (gui.tree_node(u8"左三")) {
        gui.slider_float(u8"距离", 当前距离地址(拳刃.左3), 0.f, 20.f);
        gui.slider_float(u8"延迟", 当前延迟地址(拳刃.左3), 0.f, 2.f);
        gui.slider_float(u8"角度", 当前角度地址(拳刃.左3), 1.f, 360.f);

        ImGui::PushItemWidth(200); // 设置滑动条的宽度
        gui.slider_int(u8"振刀/闪避 间隔", 当前间隔地址(拳刃.左3), 0, 300.f);
        gui.combo(当前方式标签, 当前方式地址(拳刃.左3), 当前方式列表, 当前方式列表数量);
        ImGui::TreePop();
      }
      if (gui.tree_node(u8"右三")) {
        gui.slider_float(u8"距离", 当前距离地址(拳刃.右3), 0.f, 20.f);
        gui.slider_float(u8"延迟", 当前延迟地址(拳刃.右3), 0.f, 2.f);
        gui.slider_float(u8"角度", 当前角度地址(拳刃.右3), 1.f, 360.f);

        ImGui::PushItemWidth(200); // 设置滑动条的宽度
        gui.slider_int(u8"振刀/闪避 间隔", 当前间隔地址(拳刃.右3), 0, 300.f);
        gui.combo(当前方式标签, 当前方式地址(拳刃.右3), 当前方式列表, 当前方式列表数量);
        ImGui::TreePop();
      }
      if (gui.tree_node(u8"左蓄")) {
        gui.slider_float(u8"距离", 当前距离地址(拳刃.左蓄力), 0.f, 20.f);
        gui.slider_float(u8"延迟", 当前延迟地址(拳刃.左蓄力), 0.f, 2.f);
        gui.slider_float(u8"角度", 当前角度地址(拳刃.左蓄力), 1.f, 360.f);

        if (gui.tree_node(u8"1.0")) {
          ImGui::PushItemWidth(200); // 设置滑动条的宽度
          gui.slider_int(u8"振刀/闪避 间隔", 当前间隔地址(拳刃.左蓄力1), 0,
                         300.f);
          gui.combo(当前方式标签, 当前方式地址(拳刃.左蓄力1),
                    当前方式列表, 当前方式列表数量);
          ImGui::TreePop();
        }
        if (gui.tree_node(u8"2.0")) {
          ImGui::PushItemWidth(200); // 设置滑动条的宽度
          gui.slider_int(u8"振刀/闪避 间隔", 当前间隔地址(拳刃.左蓄力2), 0,
                         300.f);
          gui.combo(当前方式标签, 当前方式地址(拳刃.左蓄力2),
                    当前方式列表, 当前方式列表数量);
          ImGui::TreePop();
        }
        ImGui::TreePop();
      }
      if (gui.tree_node(u8"右蓄")) {
        gui.slider_float(u8"距离", 当前距离地址(拳刃.右蓄力), 0.f, 20.f);
        gui.slider_float(u8"延迟", 当前延迟地址(拳刃.右蓄力), 0.f, 2.f);
        gui.slider_float(u8"角度", 当前角度地址(拳刃.右蓄力), 1.f, 360.f);
        gui.slider_float(u8"2段距离", 当前距离地址(拳刃.右蓄力2段), 0.f,
                         20.f);
        gui.slider_float(u8"2段延迟", 当前延迟地址(拳刃.右蓄力2段), 0.f,
                         2.f);
        gui.slider_float(u8"2段角度", 当前角度地址(拳刃.右蓄力2段), 1.f,
                         360.f);

        if (gui.tree_node(u8"1.0")) {
          ImGui::PushItemWidth(200); // 设置滑动条的宽度
          gui.slider_int(u8"振刀/闪避 间隔", 当前间隔地址(拳刃.右蓄力1), 0,
                         300.f);
          gui.combo(当前方式标签, 当前方式地址(拳刃.右蓄力1),
                    当前方式列表, 当前方式列表数量);
          ImGui::TreePop();
        }
        if (gui.tree_node(u8"2.0")) {
          ImGui::PushItemWidth(200); // 设置滑动条的宽度
          gui.slider_int(u8"振刀/闪避 间隔", 当前间隔地址(拳刃.右蓄力2), 0,
                         300.f);
          gui.combo(当前方式标签, 当前方式地址(拳刃.右蓄力2),
                    当前方式列表, 当前方式列表数量);
          ImGui::TreePop();
        }
        ImGui::TreePop();
      }
      if (gui.tree_node(u8"苍牙")) {
        gui.slider_float(u8"距离", 当前距离地址(拳刃.苍牙), 0.f, 20.f);
        gui.slider_float(u8"延迟", 当前延迟地址(拳刃.苍牙), 0.f, 2.f);
        gui.slider_float(u8"角度", 当前角度地址(拳刃.苍牙), 1.f, 360.f);

        ImGui::PushItemWidth(200); // 设置滑动条的宽度
        gui.slider_int(u8"振刀/闪避 间隔", 当前间隔地址(拳刃.苍牙), 0,
                       300.f);
        gui.combo(当前方式标签, 当前方式地址(拳刃.苍牙), 当前方式列表, 当前方式列表数量);
        ImGui::TreePop();
      }
      if (gui.tree_node(u8"百裂腿")) {
        gui.slider_float(u8"距离", 当前距离地址(拳刃.百裂腿), 0.f, 20.f);
        gui.slider_float(u8"延迟", 当前延迟地址(拳刃.百裂腿), 0.f, 2.f);
        gui.slider_float(u8"角度", 当前角度地址(拳刃.百裂腿), 1.f, 360.f);

        ImGui::PushItemWidth(200); // 设置滑动条的宽度
        gui.slider_int(u8"振刀/闪避 间隔", 当前间隔地址(拳刃.百裂腿), 0,
                       300.f);
        gui.combo(当前方式标签, 当前方式地址(拳刃.百裂腿), 当前方式列表, 当前方式列表数量);
        ImGui::TreePop();
      }
      if (gui.tree_node(u8"破空拳")) {
        gui.slider_float(u8"距离", 当前距离地址(拳刃.破空拳), 0.f, 20.f);
        gui.slider_float(u8"延迟", 当前延迟地址(拳刃.破空拳), 0.f, 2.f);
        gui.slider_float(u8"角度", 当前角度地址(拳刃.破空拳), 1.f, 360.f);

        ImGui::PushItemWidth(200); // 设置滑动条的宽度
        gui.slider_int(u8"振刀/闪避 间隔", 当前间隔地址(拳刃.破空拳), 0,
                       300.f);
        gui.combo(当前方式标签, 当前方式地址(拳刃.破空拳), 当前方式列表, 当前方式列表数量);
        ImGui::TreePop();
      }
      if (gui.tree_node(u8"太极弄云手")) {
        gui.slider_float(u8"距离", 当前距离地址(拳刃.太极弄云手), 0.f, 20.f);
        gui.slider_float(u8"延迟", 当前延迟地址(拳刃.太极弄云手), 0.f, 2.f);
        gui.slider_float(u8"角度", 当前角度地址(拳刃.太极弄云手), 1.f,
                         360.f);

        ImGui::PushItemWidth(200); // 设置滑动条的宽度
        gui.slider_int(u8"振刀/闪避 间隔", 当前间隔地址(拳刃.太极弄云手), 0,
                       300.f);
        gui.combo(当前方式标签, 当前方式地址(拳刃.太极弄云手),
                  当前方式列表, 当前方式列表数量);
        ImGui::TreePop();
      }
      if (gui.tree_node(u8"壁击")) {
        gui.slider_float(u8"距离", 当前距离地址(拳刃.壁击), 1.f, 20.f);
        gui.slider_float(u8"角度", 当前角度地址(拳刃.壁击), 1.f, 360.f);

        ImGui::PushItemWidth(200); // 设置滑动条的宽度
        gui.slider_int(u8"振刀/闪避 间隔", 当前间隔地址(拳刃.壁击), 0,
                       300.f);
        gui.combo(当前方式标签, 当前方式地址(拳刃.壁击), 当前方式列表, 当前方式列表数量);
        ImGui::TreePop();
      }
    }
    武器配置区(u8"飞刀") {
      if (gui.tree_node(u8"左三")) {
        gui.slider_float(u8"距离", 当前距离地址(飞刀.左3), 0.f, 20.f);
        gui.slider_float(u8"延迟", 当前延迟地址(飞刀.左3), 0.f, 2.f);
        gui.slider_float(u8"角度", 当前角度地址(飞刀.左3), 1.f, 360.f);

        ImGui::PushItemWidth(200); // 设置滑动条的宽度
        gui.slider_int(u8"振刀/闪避 间隔", 当前间隔地址(飞刀.左3), 0, 300.f);
        gui.combo(当前方式标签, 当前方式地址(飞刀.左3), 当前方式列表, 当前方式列表数量);
        ImGui::TreePop();
      }
      if (gui.tree_node(u8"右三")) {
        gui.slider_float(u8"距离", 当前距离地址(飞刀.右3), 0.f, 20.f);
        gui.slider_float(u8"延迟", 当前延迟地址(飞刀.右3), 0.f, 2.f);
        gui.slider_float(u8"角度", 当前角度地址(飞刀.右3), 1.f, 360.f);

        ImGui::PushItemWidth(200); // 设置滑动条的宽度
        gui.slider_int(u8"振刀/闪避 间隔", 当前间隔地址(飞刀.右3), 0, 300.f);
        gui.combo(当前方式标签, 当前方式地址(飞刀.右3), 当前方式列表, 当前方式列表数量);
        ImGui::TreePop();
      }
      if (gui.tree_node(u8"左蓄")) {
        gui.slider_float(u8"距离", 当前距离地址(飞刀.左蓄力), 0.f, 20.f);
        gui.slider_float(u8"延迟", 当前延迟地址(飞刀.左蓄力), 0.f, 2.f);
        gui.slider_float(u8"角度", 当前角度地址(飞刀.左蓄力), 1.f, 360.f);
        gui.slider_float(u8"左蓄力2段距离", 当前距离地址(飞刀.左蓄力2段),
                         0.f, 20.f);
        gui.slider_float(u8"左蓄力2段延迟", 当前延迟地址(飞刀.左蓄力2段),
                         0.f, 2.f);
        gui.slider_float(u8"左蓄力2段角度", 当前角度地址(飞刀.左蓄力2段),
                         1.f, 360.f);

        if (gui.tree_node(u8"1.0")) {
          ImGui::PushItemWidth(200); // 设置滑动条的宽度
          gui.slider_int(u8"振刀/闪避 间隔", 当前间隔地址(飞刀.左蓄力), 0,
                         300.f);
          gui.combo(当前方式标签, 当前方式地址(飞刀.左蓄力),
                    当前方式列表, 当前方式列表数量);
          ImGui::TreePop();
        }
        if (gui.tree_node(u8"2.0")) {
          ImGui::PushItemWidth(200); // 设置滑动条的宽度
          gui.slider_int(u8"振刀/闪避 间隔", 当前间隔地址(飞刀.左蓄力2段), 0,
                         300.f);
          gui.combo(当前方式标签, 当前方式地址(飞刀.左蓄力2段),
                    当前方式列表, 当前方式列表数量);
          ImGui::TreePop();
        }
        ImGui::TreePop();
      }
      if (gui.tree_node(u8"右蓄")) {
        gui.slider_float(u8"距离", 当前距离地址(飞刀.右蓄力), 0.f, 20.f);
        gui.slider_float(u8"延迟", 当前延迟地址(飞刀.右蓄力), 0.f, 2.f);
        gui.slider_float(u8"角度", 当前角度地址(飞刀.右蓄力), 1.f, 360.f);

        ImGui::PushItemWidth(200); // 设置滑动条的宽度
        gui.slider_int(u8"振刀/闪避 间隔", 当前间隔地址(飞刀.右蓄力), 0,
                       300.f);
        gui.combo(当前方式标签, 当前方式地址(飞刀.右蓄力), 当前方式列表, 当前方式列表数量);
        ImGui::TreePop();
      }
      if (gui.tree_node(u8"掌心雷")) {
        gui.slider_float(u8"距离", 当前距离地址(飞刀.掌心雷), 0.f, 20.f);
        gui.slider_float(u8"延迟", 当前延迟地址(飞刀.掌心雷), 0.f, 2.f);
        gui.slider_float(u8"角度", 当前角度地址(飞刀.掌心雷), 1.f, 360.f);

        ImGui::PushItemWidth(200); // 设置滑动条的宽度
        gui.slider_int(u8"振刀/闪避 间隔", 当前间隔地址(飞刀.掌心雷), 0,
                       300.f);
        gui.combo(当前方式标签, 当前方式地址(飞刀.掌心雷), 当前方式列表, 当前方式列表数量);
        ImGui::TreePop();
      }
      if (gui.tree_node(u8"苍牙")) {
        gui.slider_float(u8"距离", 当前距离地址(飞刀.苍牙), 0.f, 20.f);
        gui.slider_float(u8"延迟", 当前延迟地址(飞刀.苍牙), 0.f, 2.f);
        gui.slider_float(u8"角度", 当前角度地址(飞刀.苍牙), 1.f, 360.f);

        ImGui::PushItemWidth(200); // 设置滑动条的宽度
        gui.slider_int(u8"振刀/闪避 间隔", 当前间隔地址(飞刀.苍牙), 0,
                       300.f);
        gui.combo(当前方式标签, 当前方式地址(飞刀.苍牙), 当前方式列表, 当前方式列表数量);
        ImGui::TreePop();
      }
      if (gui.tree_node(u8"太极")) {
        gui.slider_float(u8"距离", 当前距离地址(飞刀.太极), 0.f, 20.f);
        gui.slider_float(u8"延迟", 当前延迟地址(飞刀.太极), 0.f, 2.f);
        gui.slider_float(u8"角度", 当前角度地址(飞刀.太极), 1.f, 360.f);

        ImGui::PushItemWidth(200); // 设置滑动条的宽度
        gui.slider_int(u8"振刀/闪避 间隔", 当前间隔地址(飞刀.太极), 0,
                       300.f);
        gui.combo(当前方式标签, 当前方式地址(飞刀.太极), 当前方式列表, 当前方式列表数量);
        ImGui::TreePop();
      }
      if (gui.tree_node(u8"断月千刃舞")) {
        gui.slider_float(u8"距离", 当前距离地址(飞刀.断月千刃舞), 0.f, 20.f);
        gui.slider_float(u8"延迟", 当前延迟地址(飞刀.断月千刃舞), 0.f, 2.f);
        gui.slider_float(u8"角度", 当前角度地址(飞刀.断月千刃舞), 1.f,
                         360.f);

        ImGui::PushItemWidth(200); // 设置滑动条的宽度
        gui.slider_int(u8"振刀/闪避 间隔", 当前间隔地址(飞刀.断月千刃舞), 0,
                       300.f);
        gui.combo(当前方式标签, 当前方式地址(飞刀.断月千刃舞),
                  当前方式列表, 当前方式列表数量);
        ImGui::TreePop();
      }
      if (gui.tree_node(u8"壁击")) {
        gui.slider_float(u8"距离", 当前距离地址(飞刀.壁击), 1.f, 20.f);
        gui.slider_float(u8"延迟", 当前延迟地址(飞刀.壁击), 0.f, 2.f);
        gui.slider_float(u8"角度", 当前角度地址(飞刀.壁击), 1.f, 360.f);

        ImGui::PushItemWidth(200); // 设置滑动条的宽度
        gui.slider_int(u8"振刀/闪避 间隔", 当前间隔地址(飞刀.壁击), 0,
                       300.f);
        gui.combo(当前方式标签, 当前方式地址(飞刀.壁击), 当前方式列表, 当前方式列表数量);
        ImGui::TreePop();
      }
    }
    if (gui.button(u8"保存配置")) {
      保存模拟振配置();
      保存活化配置();
      保存自瞄配置();
      保存其他参数();
      保存博弈参数();
      保存连招参数();
    }
    ImGui::SameLine();
    if (gui.button(u8"读取配置")) {
      读取模拟振配置();
    }
    ImGui::SameLine();
    if (gui.button(u8"恢复默认配置")) {
      重置模拟振配置();
    }
  }
  e_elements::end_child();

#undef 当前特殊处理地址
#undef 当前方式地址
#undef 当前间隔地址
#undef 当前角度地址
#undef 当前延迟地址
#undef 当前距离地址
#undef 千机伞动作配置
#undef 武器配置区
}
void 活化配置GUI() {
  ImGui::SetCursorPos({12, 12});
  e_elements::begin_child(u8"闪避", ImVec2(220, 550));
  {
    if (gui.checkbox(u8"启用闪避", &Function::HeroActivation::IsDodge)) {
      if (Function::HeroActivation::IsDodge) {
        读取活化配置();
      }
    }
    ImGui::SameLine();
    ImGui::Text(u8"F6快捷开关");
    gui.checkbox(u8"躲避火男冲拳", &Function::HeroActivation::躲避火男冲拳);
    gui.checkbox(u8"躲避火男F", &Function::HeroActivation::躲避火男F);
    gui.checkbox(u8"躲避妖刀大招", &Function::HeroActivation::躲避妖刀大招);
    gui.checkbox(u8"躲避顾清寒V1V2", &Function::HeroActivation::躲避顾清寒V1V2);
    gui.checkbox(u8"永远躲避顾清寒V1",
                 &Function::HeroActivation::特殊处理::永远躲避顾倾寒V1);
    gui.checkbox(u8"躲避武田F", &Function::HeroActivation::躲避武田F);
    gui.checkbox(u8"躲避胡为", &Function::HeroActivation::躲避胡为);
    gui.checkbox(u8"躲避宁红夜F", &Function::HeroActivation::躲避宁红叶F);
    gui.checkbox(u8"躲避岳山F", &Function::HeroActivation::躲避岳山F);
    gui.checkbox(u8"躲避狐狸V1V2", &Function::HeroActivation::躲避狐狸大招);
    gui.checkbox(u8"躲避乌真大招", &Function::HeroActivation::躲避乌真大招);
    gui.checkbox(u8"躲避三娘V2", &Function::HeroActivation::躲避三娘V2);
    gui.checkbox(u8"躲避哈迪大招", &Function::HeroActivation::躲避哈迪大招);
    gui.checkbox(u8"躲避魏轻F1", &Function::HeroActivation::躲避魏轻F1);
    gui.checkbox(u8"躲避魏轻V", &Function::HeroActivation::躲避魏轻V);
    gui.checkbox(u8"躲避迦南F2", &Function::HeroActivation::躲避迦南F2);
    gui.checkbox(u8"躲避迦南V2", &Function::HeroActivation::躲避迦南V2);
    gui.checkbox(u8"躲避刘炼V1V2", &Function::HeroActivation::躲避刘炼V);
    gui.checkbox(u8"躲避季盈盈V蓄力",
                 &Function::HeroActivation::躲避季盈盈V蓄力);
    gui.checkbox(u8"躲避张起灵F2V1V2", &Function::HeroActivation::躲避张起灵);
    gui.checkbox(u8"躲避无尘V2", &Function::HeroActivation::躲避无尘V2);
  }
  e_elements::end_child();
  ImGui::SetCursorPos({284, 12});
  e_elements::begin_child(u8"参数", ImVec2(330, 550));
  {
    if (gui.tree_node(u8"叶修大招")) {
      gui.slider_float(u8"矛角度", &活化参数::角度::叶修大招矛, 0.f, 360.f);
      gui.slider_float(u8"镰角度", &活化参数::角度::叶修大招镰, 0.f, 360.f);
      gui.slider_float(u8"炮形态4M角度", &活化参数::角度::叶修炮形态4M,
                       0.f, 360.f);
      gui.slider_float(u8"炮形态8M角度", &活化参数::角度::叶修炮形态8M,
                       0.f, 360.f);
      gui.slider_float(u8"炮形态12M角度", &活化参数::角度::叶修炮形态12M,
                       0.f, 360.f);
      gui.slider_float(u8"炮形态12M外角度", &活化参数::角度::叶修炮形态12M外,
                       0.f, 360.f);
      ImGui::TreePop();
    }
    if (gui.tree_node(u8"火男冲拳")) {
      gui.slider_float(u8"距离", &活化参数::距离::火男冲拳, 0.f, 35.f);
      gui.slider_float(u8"顶膝距离", &活化参数::距离::火男顶膝, 0.f, 35.f);
      gui.slider_float(u8"角度", &活化参数::角度::火男冲拳, 1.f, 360.f);
      ImGui::TreePop();
    }
    if (gui.tree_node(u8"火男F")) {
      gui.slider_float(u8"距离", &活化参数::距离::火男F, 0.f, 35.f);
      gui.slider_float(u8"角度", &活化参数::角度::火男F, 1.f, 360.f);
      ImGui::TreePop();
    }
    if (gui.tree_node(u8"妖刀大招")) {
      gui.slider_float(u8"距离", &活化参数::距离::妖刀大招, 0.f, 35.f);
      gui.slider_float(u8"3M角度", &活化参数::角度::妖刀大招3M, 1.f, 360.f);
      gui.slider_float(u8"3M外角度", &活化参数::角度::妖刀大招3M外, 1.f, 360.f);
      ImGui::TreePop();
    }
    if (gui.tree_node(u8"顾倾寒大招")) {
      gui.slider_float(u8"距离", &活化参数::距离::顾倾寒大招, 0.f, 35.f);
      gui.slider_float(u8"3M内角度", &活化参数::角度::顾倾寒大招3M, 1.f, 360.f);
      gui.slider_float(u8"3M外角度", &活化参数::角度::顾倾寒大招3M外, 1.f,
                       360.f);
      ImGui::TreePop();
    }
    if (gui.tree_node(u8"武田F2")) {
      gui.slider_float(u8"距离", &活化参数::距离::武田F2, 0.f, 35.f);
      gui.slider_float(u8"1M内角度", &活化参数::角度::武田F21M, 1.f, 360.f);
      gui.slider_float(u8"1M外角度", &活化参数::角度::武田F21M外, 1.f, 360.f);
      ImGui::TreePop();
    }
    if (gui.tree_node(u8"武田F3")) {
      gui.slider_float(u8"距离", &活化参数::距离::武田F2, 0.f, 35.f);
      gui.slider_float(u8"1M内角度", &活化参数::角度::武田F31M, 1.f, 360.f);
      gui.slider_float(u8"1M外角度", &活化参数::角度::武田F31M外, 1.f, 360.f);
      ImGui::TreePop();
    }
    if (gui.tree_node(u8"胡为F1")) {
      gui.slider_float(u8"距离", &活化参数::距离::胡为F1, 0.f, 35.f);
      ImGui::TreePop();
    }
    if (gui.tree_node(u8"胡为F2")) {
      gui.slider_float(u8"距离", &活化参数::距离::胡为F2, 0.f, 35.f);
      gui.slider_float(u8"角度", &活化参数::角度::胡为F2, 1.f, 360.f);
      ImGui::TreePop();
    }
    if (gui.tree_node(u8"胡为跃击")) {
      gui.slider_float(u8"距离", &活化参数::距离::胡为跃击, 0.f, 35.f);
      gui.slider_float(u8"角度", &活化参数::角度::胡为跃击, 1.f, 360.f);
      ImGui::TreePop();
    }
    if (gui.tree_node(u8"胡为V1V2")) {
      gui.slider_float(u8"距离", &活化参数::距离::胡为V1V2, 0.f, 35.f);
      gui.slider_float(u8"角度", &活化参数::角度::胡为V1V2, 1.f, 360.f);
      ImGui::TreePop();
    }
    if (gui.tree_node(u8"宁红夜F")) {
      gui.slider_float(u8"F1距离", &活化参数::距离::宁红叶F1, 0.f, 35.f);
      gui.slider_float(u8"F2距离", &活化参数::距离::宁红叶F2, 0.f, 35.f);
      gui.slider_float(u8"F3距离", &活化参数::距离::宁红叶F3, 0.f, 35.f);
      gui.slider_float(u8"F1角度", &活化参数::角度::宁红叶F1, 0.f, 360.f);
      gui.slider_float(u8"F2角度", &活化参数::角度::宁红叶F2, 0.f, 360.f);
      gui.slider_float(u8"F3角度", &活化参数::角度::宁红叶F3, 0.f, 360.f);
      ImGui::TreePop();
    }
    if (gui.tree_node(u8"岳山F")) {
      gui.slider_float(u8"F距离", &活化参数::距离::宁红叶F1, 0.f, 35.f);
      gui.slider_float(u8"F角度", &活化参数::角度::宁红叶F1, 0.f, 360.f);
      ImGui::TreePop();
    }
    if (gui.tree_node(u8"狐狸V1V2")) {
      gui.slider_float(u8"V1距离", &活化参数::距离::狐狸V1, 0.f, 20.f);
      gui.slider_float(u8"V2距离", &活化参数::距离::狐狸V2, 0.f, 35.f);
      gui.slider_float(u8"4M角度", &活化参数::角度::狐狸大招4M, 0.f, 360.f);
      gui.slider_float(u8"8M角度", &活化参数::角度::狐狸大招8M, 0.f, 360.f);
      gui.slider_float(u8"12M角度", &活化参数::角度::狐狸大招12M, 0.f, 360.f);
      gui.slider_float(u8"12M外角度", &活化参数::角度::狐狸大招12M外, 0.f,
                       360.f);
      ImGui::TreePop();
    }
    if (gui.tree_node(u8"三娘V2")) {
      gui.slider_float(u8"距离", &活化参数::距离::三娘V2, 0.f, 35.f);

      gui.slider_float(u8"4M角度", &活化参数::角度::三娘V24M, 0.f, 360.f);
      gui.slider_float(u8"10M角度", &活化参数::角度::三娘V210M, 0.f, 360.f);
      gui.slider_float(u8"10M外角度", &活化参数::角度::三娘V210M外, 0.f, 360.f);
      ImGui::TreePop();
    }
    if (gui.tree_node(u8"哈迪大招")) {
      gui.slider_float(u8"1段距离", &活化参数::距离::哈迪大招1段, 0.f, 35.f);
      gui.slider_float(u8"2段距离", &活化参数::距离::哈迪大招2段, 0.f, 35.f);
      gui.slider_float(u8"3段距离", &活化参数::距离::哈迪大招3段, 0.f, 35.f);
      gui.slider_float(u8"角度", &活化参数::角度::哈迪大招, 0.f, 360.f);
      ImGui::TreePop();
    }
    if (gui.tree_node(u8"魏轻F1")) {
      gui.slider_float(u8"距离", &活化参数::距离::魏轻F1, 0.f, 35.f);
      ImGui::TreePop();
    }
    if (gui.tree_node(u8"魏轻V1V2")) {
      gui.slider_float(u8"V1距离", &活化参数::距离::魏轻V1, 0.f, 35.f);
      gui.slider_float(u8"V2距离", &活化参数::距离::魏轻V2, 0.f, 35.f);
      gui.slider_float(u8"V1角度", &活化参数::角度::魏轻V1, 0.f, 360.f);
      gui.slider_float(u8"V2角度", &活化参数::角度::魏轻V2, 0.f, 360.f);
      ImGui::TreePop();
    }
    if (gui.tree_node(u8"迦南F2")) {
      gui.slider_float(u8"距离", &活化参数::距离::迦南F2, 0.f, 35.f);
      gui.slider_float(u8"角度", &活化参数::角度::迦南F2, 0.f, 360.f);
      ImGui::TreePop();
    }
    if (gui.tree_node(u8"迦南V2")) {
      gui.slider_float(u8"距离", &活化参数::距离::迦南V2, 0.f, 35.f);
      gui.slider_float(u8"角度", &活化参数::角度::迦南V2, 0.f, 360.f);
      ImGui::TreePop();
    }
    if (gui.tree_node(u8"刘炼V1V2")) {
      gui.slider_float(u8"V1距离", &活化参数::距离::刘炼V1, 0.f, 35.f);
      gui.slider_float(u8"V2距离", &活化参数::距离::刘炼V2, 0.f, 35.f);
      gui.slider_float(u8"V1角度", &活化参数::角度::刘炼V1, 0.f, 360.f);
      gui.slider_float(u8"V2角度", &活化参数::角度::刘炼V2, 0.f, 360.f);
      ImGui::TreePop();
    }
    if (gui.tree_node(u8"季盈盈V蓄力")) {
      gui.slider_float(u8"V1距离", &活化参数::距离::季盈盈V1蓄力, 0.f, 35.f);
      gui.slider_float(u8"V2距离", &活化参数::距离::季盈盈V2蓄力, 0.f, 35.f);
      gui.slider_float(u8"V1角度", &活化参数::角度::季盈盈V1蓄力, 0.f, 360.f);
      gui.slider_float(u8"V2角度", &活化参数::角度::季盈盈V2蓄力, 0.f, 360.f);
      ImGui::TreePop();
    }
    if (gui.tree_node(u8"张起灵F2V1V2")) {
      gui.slider_float(u8"F2距离", &活化参数::距离::张起灵F2, 0.f, 35.f);
      gui.slider_float(u8"V1距离", &活化参数::距离::张起灵V1, 0.f, 35.f);
      gui.slider_float(u8"V12段距离", &活化参数::距离::张起灵V12段, 0.f, 20.f);
      gui.slider_float(u8"V2距离", &活化参数::距离::张起灵V2, 0.f, 35.f);
      gui.slider_float(u8"F2角度", &活化参数::角度::张起灵F2, 0.f, 360.f);
      gui.slider_float(u8"V1角度", &活化参数::角度::张起灵V1, 0.f, 360.f);
      gui.slider_float(u8"V12段角度", &活化参数::角度::张起灵V12段, 0.f, 360.f);
      gui.slider_float(u8"V2角度", &活化参数::角度::张起灵V2, 0.f, 360.f);
      ImGui::TreePop();
    }
    if (gui.tree_node(u8"无尘V2")) {
      gui.slider_float(u8"距离", &活化参数::距离::无尘V2, 0.f, 35.f);
      gui.slider_float(u8"角度", &活化参数::角度::无尘V2, 0.f, 360.f);
      ImGui::TreePop();
    }
    if (gui.tree_node(u8"希拉")) {
      gui.slider_float(u8"距离", &活化参数::距离::希拉F, 0.f, 35.f);
      gui.slider_float(u8"角度", &活化参数::角度::希拉F, 0.f, 360.f);
      ImGui::TreePop();
    }
    if (gui.tree_node(u8"彩戏师F")) {
      gui.slider_float(u8"距离", &活化参数::距离::彩戏师F, 0.f, 35.f);
      gui.slider_float(u8"角度", &活化参数::角度::彩戏师F, 0.f, 360.f);
      ImGui::TreePop();
    }
    if (gui.button(u8"保存配置")) {
      保存活化配置();
    }
    ImGui::SameLine();
    if (gui.button(u8"读取配置")) {
      读取活化配置();
    }
  }
  e_elements::end_child();
}

void ConfigDefaults::ApplySemiAutoConfig() {
  Function::半自动博弈::总开关 = true;
  Function::半自动博弈::蓄力抓振 = true;
  Function::半自动博弈::蓄力抓长闪 = false;
  Function::半自动博弈::蓄力抓短闪 = true;
  Function::半自动博弈::蓄力抓白刀 = true;
  Function::半自动博弈::蓄力抓倒地 = true;
  Function::半自动博弈::蓄力抓受击 = true;
  Function::半自动博弈::蓄力抓起跳 = false;
  Function::半自动博弈::蓄力抓下蹲 = false;
  Function::半自动博弈::自动连招 = true;

  Function::半自动博弈::白刀抓长闪 = true;
  Function::半自动博弈::白刀抓短闪 = true;
  Function::半自动博弈::白刀抓蓄力后摇 = true;
  Function::半自动博弈::白刀抓振 = true;
  Function::半自动博弈::白刀蓝顶 = true;

  Function::半自动博弈::动态线程 = false;

  Function::半自动博弈::蓄力抓振延迟 = 220;
  Function::半自动博弈::白刀抓蓄力后摇延迟 = 100;
  Function::半自动博弈::白刀蓝顶延迟 = 0.1f;
  Function::半自动博弈::白刀抓短闪最远距离 = 5.0f;
  Function::半自动博弈::白刀抓长闪最远距离 = 5.0f;

  Function::半自动博弈::钩锁模式 = 1;
  Function::半自动博弈::断续切刀时间 = 1.2f;
  Function::半自动博弈::博弈热键 = VK_XBUTTON2;
  Function::半自动博弈::解键按键 = VK_CONTROL;
}

void 重置半自动配置() {
  ConfigDefaults::ApplySemiAutoConfig();
}

void ConfigDefaults::ApplySideGameConfig() {
  Function::侧键博弈::蓄力抓振 = true;
  Function::侧键博弈::博弈切刀抓振刀 = true;
  Function::侧键博弈::蓄力抓长闪 = false;
  Function::侧键博弈::蓄力抓短闪 = true;
  Function::侧键博弈::蓄力抓白刀 = true;
  Function::侧键博弈::蓄力抓倒地 = true;
  Function::侧键博弈::蓄力抓受击 = true;
  Function::侧键博弈::蓄力抓起跳 = false;
  Function::侧键博弈::蓄力抓下蹲 = false;
  Function::侧键博弈::自动连招 = true;
  Function::侧键博弈::禁用CPU0 = true;
  Function::侧键博弈::背包切蓄力 = true;
  Function::侧键博弈::背包切蓄力概率 = 100;

  Function::侧键博弈::切拳起身 = true;
  Function::侧键博弈::白刀抓长闪 = true;
  Function::侧键博弈::白刀抓短闪 = true;
  Function::侧键博弈::白刀抓蓄力后摇 = true;
  Function::侧键博弈::白刀抓振 = true;
  Function::侧键博弈::白刀蓝顶 = true;

  Function::侧键博弈::动态线程 = false;

  Function::侧键博弈::蓄力抓振延迟 = 230;
  Function::侧键博弈::白刀抓短闪延迟 = 200;
  Function::侧键博弈::白刀抓蓄力后摇延迟 = 100;
  Function::侧键博弈::白刀蓝顶延迟 = 0.1f;
  Function::侧键博弈::白刀抓短闪最远距离 = 5.0f;
  Function::侧键博弈::白刀抓长闪最远距离 = 5.0f;

  Function::侧键博弈::钩锁模式 = 1;
  Function::侧键博弈::断续切刀时间 = 1.15f;
  Function::侧键博弈::断续切刀最小时间 = 0.50f;
  Function::侧键博弈::振刀模式热键 = VK_XBUTTON2;
  Function::侧键博弈::闪避模式热键 = VK_XBUTTON1;
}

void 重置博弈配置() {
  ConfigDefaults::ApplySideGameConfig();
}
void 半自动配置GUI() {
  ImGui::SetCursorPos({8, 6});
  ImVec2 region = ImGui::GetContentRegionAvail();
  float childWidth = ImMax(340.f, region.x - 8.f);
  float childHeight = ImMax(540.f, region.y - 8.f);

  e_elements::begin_child(u8"半自动博弈--与侧键博弈独立开关",
                          ImVec2(childWidth, childHeight));
  {
    ImGui::PushItemWidth(ImMax(140.f, childWidth * 0.42f));

    static bool waitingSemiUnlockKey = false;
    auto GetSemiVkHotkeyName = [](int hotkey) {
      char name[64]{};
      if (GetKeyNameTextA(MapVirtualKeyA(hotkey, MAPVK_VK_TO_VSC) << 16,
                          name, sizeof(name))) {
        return std::string(name);
      }
      return std::to_string(hotkey);
    };
    auto DrawSemiVkHotkey = [&](const char *label, int &hotkey,
                                bool &waitingForKey, const char *id) {
      std::string hotkeyName = GetSemiVkHotkeyName(hotkey);
      ImGui::AlignTextToFramePadding();
      ImGui::Text(label);
      ImGui::SameLine(80);
      ImGui::PushID(id);
      if (waitingForKey) {
        ImGui::TextColored(ImVec4(1, 0.8f, 0.2f, 1), u8"[请按键...]");
        for (int key = 0x01; key <= 0xFE; ++key) {
          if (key != 179 && (GetAsyncKeyState(key) & 0x8000)) {
            hotkey = key;
            waitingForKey = false;
            Sleep(150);
            break;
          }
        }
        if (GetAsyncKeyState(VK_ESCAPE) & 0x8000) {
          waitingForKey = false;
          Sleep(150);
        }
      } else {
        if (gui.button(hotkeyName.c_str(), ImVec2(100, 0))) {
          waitingForKey = true;
        }
      }
      ImGui::PopID();
    };

    DrawSemiVkHotkey(u8"解键按键", Function::半自动博弈::解键按键,
                     waitingSemiUnlockKey, "semi_unlock_key");
    ImGui::Dummy(ImVec2(0, 8));
    gui.checkbox(u8"半自动总开关", &Function::半自动博弈::总开关);
    ImGui::SameLine();
    ImGui::Dummy(ImVec2(30, 0)); // 设置水平间隔为 20
    ImGui::SameLine();
    gui.checkbox(u8"自动连招", &Function::半自动博弈::自动连招);
    ImGui::SameLine();
    ImGui::Dummy(ImVec2(30, 0)); // 设置水平间隔为 20
    ImGui::SameLine();
    gui.checkbox(u8"自动断蓄", &Function::半自动博弈::自动断蓄);

    ImGui::Dummy(ImVec2(0, 10));
    if (ImGui::BeginTable("##grab_combo", 2,
                          ImGuiTableFlags_SizingStretchSame)) {
      ImGui::TableNextColumn();
      // ImGui::Text(u8"蓄力抓");
      gui.checkbox(u8"蓄力抓-总开关", &Function::半自动博弈::蓄力抓总开关);
      ImGui::Separator();
      if (ImGui::BeginTable("##charge_grab", 2,
                            ImGuiTableFlags_SizingStretchSame)) {
        ImGui::TableNextColumn();
        gui.checkbox(u8"蓄力抓长闪", &Function::半自动博弈::蓄力抓长闪);
        ImGui::TableNextColumn();
        gui.checkbox(u8"蓄力抓短闪", &Function::半自动博弈::蓄力抓短闪);
        ImGui::TableNextColumn();
        gui.checkbox(u8"蓄力抓白刀", &Function::半自动博弈::蓄力抓白刀);
        ImGui::TableNextColumn();
        gui.checkbox(u8"蓄力抓倒地", &Function::半自动博弈::蓄力抓倒地);
        ImGui::TableNextColumn();
        gui.checkbox(u8"蓄力抓受击", &Function::半自动博弈::蓄力抓受击);
        ImGui::TableNextColumn();
        gui.checkbox(u8"蓄力抓起跳", &Function::半自动博弈::蓄力抓起跳);
        ImGui::TableNextColumn();
        gui.checkbox(u8"蓄力抓下蹲", &Function::半自动博弈::蓄力抓下蹲);
        ImGui::EndTable();
      }

      ImGui::TableNextColumn();
      // ImGui::Text(u8"白刀抓-总开关");
      gui.checkbox(u8"白刀抓-总开关", &Function::半自动博弈::白刀抓总开关);
      ImGui::Separator();
      if (ImGui::BeginTable("##white_grab", 2,
                            ImGuiTableFlags_SizingStretchSame)) {
        ImGui::TableNextColumn();
        gui.checkbox(u8"白刀抓长闪", &Function::半自动博弈::白刀抓长闪);
        ImGui::TableNextColumn();
        gui.checkbox(u8"白刀抓短闪", &Function::半自动博弈::白刀抓短闪);
        ImGui::TableNextColumn();
        gui.checkbox(u8"白刀抓蓄力后摇", &Function::半自动博弈::白刀抓蓄力后摇);
        ImGui::TableNextColumn();
        gui.checkbox(u8"白刀抓振", &Function::半自动博弈::白刀抓振);
        ImGui::TableNextColumn();
        gui.checkbox(u8"白刀蓝顶", &Function::半自动博弈::白刀蓝顶);
        ImGui::TableNextColumn();
        gui.checkbox(u8"白刀抓受击", &Function::半自动博弈::白刀抓受击);
        ImGui::EndTable();
      }

      ImGui::EndTable();
    }
    ImGui::Dummy(ImVec2(0, 8)); // 可选的垂直间距
    ImGui::Text(u8"延迟/距离");

    // 计算保存按钮的宽度
    float saveButtonWidth = ImGui::CalcTextSize(u8"保存博弈参数").x +
                            ImGui::GetStyle().FramePadding.x * 2.0f;
    // 计算重置按钮的宽度
    float resetButtonWidth = ImGui::CalcTextSize(u8"重置半自动").x +
                             ImGui::GetStyle().FramePadding.x * 2.0f;
    // 计算按钮之间的间距
    float buttonSpacing = 10.0f; // 可以根据需要调整间距
    // 先绘制保存按钮
    ImGui::SameLine(ImGui::GetWindowContentRegionMax().x - saveButtonWidth -
                    resetButtonWidth - buttonSpacing);
    if (gui.button(u8"保存博弈参数")) {
      保存博弈参数(); // 保存半自动配置
    }

    // 然后绘制重置按钮
    ImGui::SameLine(ImGui::GetWindowContentRegionMax().x - resetButtonWidth);
    if (gui.button(u8"重置半自动")) {
      重置半自动配置(); // 重置半自动配置
      保存博弈参数();   // 保存博弈参数
    }
    ImGui::Separator();
    if (ImGui::BeginTable("##delay_distance", 2,
                          ImGuiTableFlags_SizingStretchSame)) {
      ImGui::TableSetupColumn("label", ImGuiTableColumnFlags_WidthFixed, 160.f);
      ImGui::TableSetupColumn("value", ImGuiTableColumnFlags_WidthStretch);

      ImGui::TableNextRow();
      ImGui::TableNextColumn();
      ImGui::Text(u8"断续/切刀 时机");
      ImGui::TableNextColumn();
      ImGui::SetNextItemWidth(-1);
      gui.slider_float("##semi_cut_time", &Function::半自动博弈::断续切刀时间,
                       0.1f, 1.5f);
      if (ImGui::IsItemHovered()) {
        ImGui::SetTooltip(u8"数值越高,断续/切刀 越慢");
      }

      ImGui::TableNextRow();
      ImGui::TableNextColumn();
      ImGui::Text(u8"蓄力抓振延迟");
      ImGui::TableNextColumn();
      ImGui::SetNextItemWidth(-1);
      float temp_delay = (float)Function::半自动博弈::蓄力抓振延迟;
      if (gui.slider_float("##semi_charge_parry_delay", &temp_delay, 0.f, 500.f,
                           "%.0f")) {
        Function::半自动博弈::蓄力抓振延迟 = (int)temp_delay;
      }
      if (ImGui::IsItemHovered()) {
        ImGui::SetTooltip(u8"数值越高,抓振的越慢");
      }

      ImGui::TableNextRow();
      ImGui::TableNextColumn();
      ImGui::Text(u8"博弈蓝顶延迟");
      ImGui::TableNextColumn();
      ImGui::SetNextItemWidth(-1);
      gui.slider_float("##semi_blue_top_delay", &Function::半自动博弈::白刀蓝顶延迟,
                       0.f,
                       0.5f);
      if (ImGui::IsItemHovered()) {
        ImGui::SetTooltip(u8"数值越高,抓的越慢");
      }

      ImGui::TableNextRow();
      ImGui::TableNextColumn();
      ImGui::Text(u8"白刀抓短闪最远距离");
      ImGui::TableNextColumn();
      ImGui::SetNextItemWidth(-1);
      gui.slider_float("##semi_white_short_dash_max_distance",
                       &Function::半自动博弈::白刀抓短闪最远距离, 0.f, 5.0f);
      if (ImGui::IsItemHovered()) {
        ImGui::SetTooltip(u8"数值越高,抓的越慢");
      }

      ImGui::TableNextRow();
      ImGui::TableNextColumn();
      ImGui::Text(u8"白刀抓长闪最远距离");
      ImGui::TableNextColumn();
      ImGui::SetNextItemWidth(-1);
      gui.slider_float("##semi_white_long_dash_max_distance",
                       &Function::半自动博弈::白刀抓长闪最远距离,
                       0.f, 5.0f);
      if (ImGui::IsItemHovered()) {
        ImGui::SetTooltip(u8"数值越高,抓的越慢");
      }
      ImGui::EndTable();
    }

    ImGui::PopItemWidth();
  }
  e_elements::end_child();
}

void 连招配置GUI() {
  ImGui::SetCursorPos({8, 6});
  ImVec2 region = ImGui::GetContentRegionAvail();
  float childWidth = ImMax(340.f, region.x - 8.f);
  float childHeight = ImMax(540.f, region.y - 8.f);

  e_elements::begin_child(u8"连招配置", ImVec2(childWidth, childHeight));
  {
    ImGui::PushItemWidth(ImMax(140.f, childWidth * 0.42f));

    static bool waitingGameHotkeyLz = false;
    auto GetGameHotkeyName = []() {
      char name[64]{};
      // 获取第二套按键的名称
      if (GetKeyNameTextA(MapVirtualKeyA(连招方式::第二套按键, MAPVK_VK_TO_VSC)
                              << 16,
                          name, sizeof(name))) {
        return std::string(name);
      }
      return std::to_string(连招方式::第二套按键);
    };

    std::string gameHotkeyName = GetGameHotkeyName(); // 获取当前的按键名称
    ImGui::AlignTextToFramePadding();
    ImGui::Text(u8"连招二按键"); // 显示文本
    ImGui::SameLine(80);

    if (waitingGameHotkeyLz) {
      // 如果正在等待按键输入，显示提示文本
      ImGui::TextColored(ImVec4(1, 0.8f, 0.2f, 1), u8"[请按键...]");

      // 检测用户按下的键
      for (int key = 0x01; key <= 0xFE; ++key) {
        if (key != 179 && GetAsyncKeyState(key) & 0x8000) { // 检测按键
          MyLogTrue("按下键:", key);
          连招方式::第二套按键 = key;  // 更新按键
          waitingGameHotkeyLz = false; // 停止等待按键
          Sleep(150);                  // 延迟避免误操作
          break;
        }
      }

      // 如果用户按下Esc键，取消等待
      if (GetAsyncKeyState(VK_ESCAPE) & 0x8000) {
        waitingGameHotkeyLz = false;
        Sleep(150);
      }
    } else {
      // 如果没有在等待按键，显示按钮让用户点击开始设置按键
      if (gui.button(gameHotkeyName.c_str(), ImVec2(100, 0))) {
        waitingGameHotkeyLz = true; // 开始等待按键输入
      }
    }

    // 处理武器和连招方式的选择
#define 连招武器配置区(label)                                                   \
  if (gui.collapsing_header(label))                                             \
    for (bool _combo_weapon_once = (ImGui::Indent(16.f), true);                 \
         _combo_weapon_once;                                                    \
         ImGui::Unindent(16.f), _combo_weapon_once = false)

    连招武器配置区(u8"太刀") {
      if (gui.tree_node(u8"选择连招方式一")) {
        static const char *连招方式列表[] = {u8"钩锁/985", u8"双升龙跳A",
                                             u8"拳切颠勺"};
        gui.combo(u8"连招方式", &连招方式::太刀, 连招方式列表,
                  IM_ARRAYSIZE(连招方式列表));
        ImGui::TreePop();
      }

      if (gui.tree_node(u8"选择连招方式二")) {
        static const char *连招方式列表[] = {u8"钩锁/985", u8"双升龙跳A",
                                             u8"拳切颠勺"};
        gui.combo(u8"连招方式1", &连招方式::太刀1, 连招方式列表,
                  IM_ARRAYSIZE(连招方式列表));
        ImGui::TreePop();
      }
    }

    连招武器配置区(u8"横刀") {
      if (gui.tree_node(u8"选择连招方式一")) {
        static const char *连招方式列表[] = {u8"钩锁/985", u8"双升龙跳A",
                                             u8"拳切颠勺"};
        gui.combo(u8"连招方式", &连招方式::横刀, 连招方式列表,
                  IM_ARRAYSIZE(连招方式列表));
        ImGui::TreePop();
      }

      if (gui.tree_node(u8"选择连招方式二")) {
        static const char *连招方式列表[] = {u8"钩锁/985", u8"双升龙跳A",
                                             u8"拳切颠勺"};
        gui.combo(u8"连招方式1", &连招方式::横刀1, 连招方式列表,
                  IM_ARRAYSIZE(连招方式列表));
        ImGui::TreePop();
      }
    }

    连招武器配置区(u8"长剑") {
      if (gui.tree_node(u8"选择连招方式一")) {
        static const char *连招方式列表[] = {u8"钩锁/985", u8"双升龙跳A",
                                             u8"浮空3a"};
        gui.combo(u8"连招方式", &连招方式::长剑, 连招方式列表,
                  IM_ARRAYSIZE(连招方式列表));
        ImGui::TreePop();
      }

      if (gui.tree_node(u8"选择连招方式二")) {
        static const char *连招方式列表[] = {u8"钩锁/985", u8"双升龙跳A",
                                             u8"浮空3a"};
        gui.combo(u8"连招方式1", &连招方式::长剑1, 连招方式列表,
                  IM_ARRAYSIZE(连招方式列表));
        ImGui::TreePop();
      }
    }

    连招武器配置区(u8"链剑") {
      if (gui.tree_node(u8"选择连招方式一")) {
        static const char *连招方式列表[] = {u8"钩锁/985", u8"双升龙跳A"};
        gui.combo(u8"连招方式", &连招方式::链剑, 连招方式列表,
                  IM_ARRAYSIZE(连招方式列表));
        ImGui::TreePop();
      }

      if (gui.tree_node(u8"选择连招方式二")) {
        static const char *连招方式列表[] = {u8"钩锁/985", u8"双升龙跳A"};
        gui.combo(u8"连招方式1", &连招方式::链剑1, 连招方式列表,
                  IM_ARRAYSIZE(连招方式列表));
        ImGui::TreePop();
      }
    }

    连招武器配置区(u8"长枪") {
      if (gui.tree_node(u8"选择连招方式一")) {
        static const char *连招方式列表[] = {u8"浮空三A", u8"钩锁/985"};
        gui.combo(u8"连招方式", &连招方式::长枪, 连招方式列表,
                  IM_ARRAYSIZE(连招方式列表));
        ImGui::TreePop();
      }

      if (gui.tree_node(u8"选择连招方式二")) {
        static const char *连招方式列表[] = {u8"浮空三A", u8"钩锁/985"};
        gui.combo(u8"连招方式1", &连招方式::长枪1, 连招方式列表,
                  IM_ARRAYSIZE(连招方式列表));
        ImGui::TreePop();
      }
    }

#undef 连招武器配置区
  }
  e_elements::end_child();
}

void 博弈配置GUI() {
  ImGui::SetCursorPos({8, 6});
  ImVec2 region = ImGui::GetContentRegionAvail();
  float childWidth = ImMax(340.f, region.x - 8.f);
  float childHeight = ImMax(540.f, region.y - 8.f);
  e_elements::begin_child(u8"侧键博弈", ImVec2(childWidth, childHeight));
  {
    ImGui::PushItemWidth(ImMax(140.f, childWidth * 0.42f));

    ImGui::Text(u8"基础");
    ImGui::Separator();
    static bool waitingParryModeHotkey = false;
    static bool waitingDodgeModeHotkey = false;
    auto GetHotkeyName = [](int hotkey) {
      char name[64]{};
      if (GetKeyNameTextA(MapVirtualKeyA(hotkey, MAPVK_VK_TO_VSC) << 16, name,
                          sizeof(name))) {
        return std::string(name);
      }
      return std::to_string(hotkey);
    };
    auto RenderModeHotkey = [&](const char *label, int &hotkey,
                                bool &waitingForKey) {
      std::string hotkeyName = GetHotkeyName(hotkey);
      ImGui::PushID(label);
      ImGui::AlignTextToFramePadding();
      ImGui::Text("%s", label);
      ImGui::SameLine(0.f, 8.f);
      if (waitingForKey) {
        ImGui::TextColored(ImVec4(1, 0.8f, 0.2f, 1), u8"[请按键...]");
        for (int key = 0x01; key <= 0xFE; ++key) {
          if (key != 179 && GetAsyncKeyState(key) & 0x8000) {
            hotkey = key;
            waitingForKey = false;
            Sleep(150);
            break;
          }
        }
        if (GetAsyncKeyState(VK_ESCAPE) & 0x8000) {
          waitingForKey = false;
          Sleep(150);
        }
      } else {
        if (gui.button(hotkeyName.c_str(), ImVec2(100, 0))) {
          waitingForKey = true;
        }
      }
      ImGui::PopID();
    };

    ImGui::AlignTextToFramePadding();
    ImGui::Text(u8"模式热键");
    ImGui::SameLine(80);
    RenderModeHotkey(u8"振刀模式", Function::侧键博弈::振刀模式热键,
        waitingParryModeHotkey);

    // 禁用闪避模式热键并添加提示文字
    ImGui::SameLine(0.f, 24.f);
   // ImGui::BeginDisabled();  // 禁用此控件
    RenderModeHotkey(u8"闪避模式", Function::侧键博弈::闪避模式热键,
        waitingDodgeModeHotkey);
    //ImGui::SameLine();
   // ImGui::Text(u8"开发中，敬请期待...");
   // ImGui::EndDisabled();  // 结束禁用

    ImGui::Dummy(ImVec2(0, 4));
    gui.checkbox(u8"高性能模式-8核及以上建议开启(保存重启生效)",
        &Function::侧键博弈::禁用CPU0);

    ImGui::SameLine();
    ImGui::Dummy(ImVec2(40, 0)); // 设置水平间隔为 20
    ImGui::SameLine();
    gui.checkbox(u8"缓存效率超频模式",
        &Function::侧键博弈::缓存效率模式);

    // ImGui::RadioButton(u8"TS线程", &Function::侧键博弈::动态线程, 3);

    ImGui::RadioButton(u8"线程模式1-自行测试运行效率(保存重启生效)",
                       &Function::侧键博弈::动态线程, 0);
    ImGui::RadioButton(u8"线程模式2-自行测试运行效率(保存重启生效)",
                       &Function::侧键博弈::动态线程, 1);
    ImGui::RadioButton(u8"线程模式3-自行测试运行效率(保存重启生效)",
                       &Function::侧键博弈::动态线程, 2);
    ImGui::Dummy(ImVec2(0, 10));

    ImGui::Text(u8"博弈");
    ImGui::Separator();
   // gui.checkbox(u8"快速起身", &Function::侧键博弈::切拳起身);
   // ImGui::SameLine();
    //ImGui::Dummy(ImVec2(30, 0)); // 设置水平间隔为 20
    //ImGui::SameLine();
    gui.checkbox(u8"敌人振刀是否切刀抓", &Function::侧键博弈::博弈切刀抓振刀);

    constexpr float backpackChargeSliderHeight = 24.f;
    constexpr float backpackChargeSliderWidth = 220.f;
    const float backpackChargeLabelWidth =
        ImGui::CalcTextSize(u8"背包切蓄力").x +
        ImGui::GetStyle().CellPadding.x * 2.f;
    if (ImGui::BeginTable("##backpack_charge_probability", 3,
                          ImGuiTableFlags_SizingStretchSame)) {
      ImGui::TableSetupColumn("label", ImGuiTableColumnFlags_WidthFixed,
                              backpackChargeLabelWidth);
      ImGui::TableSetupColumn("value", ImGuiTableColumnFlags_WidthFixed,
                              backpackChargeSliderWidth);
      ImGui::TableSetupColumn("spacing", ImGuiTableColumnFlags_WidthStretch);
      ImGui::TableNextRow();
      ImGui::TableNextColumn();
      ImGui::SetCursorPosY(
          ImGui::GetCursorPosY() +
          ImMax(0.f, (backpackChargeSliderHeight - ImGui::GetTextLineHeight()) *
                         0.5f));
      ImGui::Text(u8"背包切蓄力");
      ImGui::TableNextColumn();
      ImGui::SetNextItemWidth(-1);
      gui.slider_int("##backpack_charge_probability_slider",
                     &Function::侧键博弈::背包切蓄力概率, 0, 100, "%d%%");
      ImGui::EndTable();
    }

    if (ImGui::BeginTable("##grab_combo", 2,
                          ImGuiTableFlags_SizingStretchSame)) {
      ImGui::TableNextColumn();
      ImGui::Text(u8"蓄力抓");
      ImGui::Separator();
      if (ImGui::BeginTable("##charge_grab", 2,
                            ImGuiTableFlags_SizingStretchSame)) {
        ImGui::TableNextColumn();
        gui.checkbox(u8"蓄力抓长闪", &Function::侧键博弈::蓄力抓长闪);
        ImGui::TableNextColumn();
        gui.checkbox(u8"蓄力抓短闪", &Function::侧键博弈::蓄力抓短闪);
        ImGui::TableNextColumn();
        gui.checkbox(u8"蓄力抓白刀", &Function::侧键博弈::蓄力抓白刀);
        ImGui::TableNextColumn();
        gui.checkbox(u8"蓄力抓倒地", &Function::侧键博弈::蓄力抓倒地);
        ImGui::TableNextColumn();
        gui.checkbox(u8"蓄力抓受击", &Function::侧键博弈::蓄力抓受击);
        ImGui::TableNextColumn();
        gui.checkbox(u8"蓄力抓起跳", &Function::侧键博弈::蓄力抓起跳);
        ImGui::TableNextColumn();
        gui.checkbox(u8"蓄力抓下蹲", &Function::侧键博弈::蓄力抓下蹲);
        ImGui::EndTable();
      }

      ImGui::TableNextColumn();
      ImGui::Text(u8"白刀抓");
      ImGui::Separator();
      if (ImGui::BeginTable("##white_grab", 2,
                            ImGuiTableFlags_SizingStretchSame)) {
        ImGui::TableNextColumn();
        gui.checkbox(u8"白刀抓长闪", &Function::侧键博弈::白刀抓长闪);
        ImGui::TableNextColumn();
        gui.checkbox(u8"白刀抓短闪", &Function::侧键博弈::白刀抓短闪);
        ImGui::TableNextColumn();
        gui.checkbox(u8"白刀抓蓄力后摇", &Function::侧键博弈::白刀抓蓄力后摇);
        ImGui::TableNextColumn();
        gui.checkbox(u8"白刀抓振", &Function::侧键博弈::白刀抓振);
        ImGui::TableNextColumn();
        gui.checkbox(u8"白刀蓝顶", &Function::侧键博弈::白刀蓝顶);
        ImGui::EndTable();
      }

      ImGui::EndTable();
    }
    ImGui::Dummy(ImVec2(0, 10)); // 可选的垂直间距
    ImGui::Text(u8"延迟/距离");

    // 计算保存按钮的宽度
    float saveButtonWidth = ImGui::CalcTextSize(u8"保存博弈参数").x +
                            ImGui::GetStyle().FramePadding.x * 2.0f;
    // 计算重置按钮的宽度
    float resetButtonWidth = ImGui::CalcTextSize(u8"重置博弈配置").x +
                             ImGui::GetStyle().FramePadding.x * 2.0f;
    // 计算两个按钮之间的间距
    float buttonSpacing = 10.0f; // 你可以调整这个值来控制按钮之间的间距

    // 先绘制保存按钮
    ImGui::SameLine(ImGui::GetWindowContentRegionMax().x - saveButtonWidth -
                    resetButtonWidth - buttonSpacing);
    if (gui.button(u8"保存博弈参数")) {
      保存博弈参数(); // 保存博弈配置
    }

    // 然后绘制重置按钮
    ImGui::SameLine(ImGui::GetWindowContentRegionMax().x - resetButtonWidth);
    if (gui.button(u8"重置博弈配置")) {
      重置博弈配置(); // 重置博弈配置
      保存博弈参数(); // 保存博弈参数
    }

    ImGui::Separator();
    if (ImGui::BeginTable("##delay_distance", 2,
                          ImGuiTableFlags_SizingStretchSame)) {
      ImGui::TableSetupColumn("label", ImGuiTableColumnFlags_WidthFixed, 160.f);
      ImGui::TableSetupColumn("value", ImGuiTableColumnFlags_WidthStretch);

      // 博弈距离范围内

      ImGui::TableNextRow();
      ImGui::TableNextColumn();
      ImGui::Text(u8"博弈最大距离范围");
      ImGui::TableNextColumn();
      ImGui::SetNextItemWidth(-1);
      gui.slider_float("##game_range_max_distance", &Function::侧键博弈::博弈距离范围,
                       4.f, 10.f);

      ImGui::TableNextRow();
      ImGui::TableNextColumn();
      ImGui::Text(u8"博弈对蓄力距离");
      ImGui::TableNextColumn();
      ImGui::SetNextItemWidth(-1);
      gui.slider_float("##game_charge_distance",
                       &Function::侧键博弈::博弈对蓄力距离, 1.f, 10.f);

      ImGui::TableNextRow();
      ImGui::TableNextColumn();
      ImGui::Text(u8"断续/切刀 最大时间");
      ImGui::TableNextColumn();
      ImGui::SetNextItemWidth(-1);
      gui.slider_float("##game_cut_time_max", &Function::侧键博弈::断续切刀时间,
                       0.1f, 1.5f);
      if (ImGui::IsItemHovered()) {
        ImGui::SetTooltip(u8"断续/切刀 会在区间内智能判断");
      }

      ImGui::TableNextRow();
      ImGui::TableNextColumn();
      ImGui::Text(u8"断续/切刀 最小时间");
      ImGui::TableNextColumn();
      ImGui::SetNextItemWidth(-1);
      gui.slider_float("##game_cut_time_min",
                       &Function::侧键博弈::断续切刀最小时间, 0.1f, 1.5f);
      if (ImGui::IsItemHovered()) {
        ImGui::SetTooltip(u8"断续/切刀 会在区间内智能判断");
      }

      ImGui::TableNextRow();
      ImGui::TableNextColumn();
      ImGui::Text(u8"蓄力抓振延迟");
      ImGui::TableNextColumn();
      ImGui::SetNextItemWidth(-1);
      gui.slider_int("##game_charge_parry_delay",
                     &Function::侧键博弈::蓄力抓振延迟, 0,
                     500);
      if (ImGui::IsItemHovered()) {
        ImGui::SetTooltip(u8"数值越高,抓振的越慢");
      }

      ImGui::TableNextRow();
      ImGui::TableNextColumn();
      ImGui::Text(u8"博弈蓝顶延迟");
      ImGui::TableNextColumn();
      ImGui::SetNextItemWidth(-1);
      gui.slider_float("##game_blue_top_delay", &Function::侧键博弈::白刀蓝顶延迟,
                       0.f,
                       0.5f);
      if (ImGui::IsItemHovered()) {
        ImGui::SetTooltip(u8"数值越高,抓的越慢");
      }

      ImGui::TableNextRow();
      ImGui::TableNextColumn();
      ImGui::Text(u8"白刀抓短闪延迟");
      ImGui::TableNextColumn();
      ImGui::SetNextItemWidth(-1);
      gui.slider_int("##game_white_short_dash_delay",
                     &Function::侧键博弈::白刀抓短闪延迟, 0,
                     500);
      if (ImGui::IsItemHovered()) {
        ImGui::SetTooltip(u8"数值越高,抓振的越慢");
      }

      ImGui::TableNextRow();
      ImGui::TableNextColumn();
      ImGui::Text(u8"白刀抓短闪最远距离");
      ImGui::TableNextColumn();
      ImGui::SetNextItemWidth(-1);
      gui.slider_float("##game_white_short_dash_max_distance",
                       &Function::侧键博弈::白刀抓短闪最远距离,
                       0.f, 5.0f);
      if (ImGui::IsItemHovered()) {
        ImGui::SetTooltip(u8"数值越高,抓的越慢");
      }

      ImGui::TableNextRow();
      ImGui::TableNextColumn();
      ImGui::Text(u8"白刀抓长闪最远距离");
      ImGui::TableNextColumn();
      ImGui::SetNextItemWidth(-1);
      gui.slider_float("##game_white_long_dash_max_distance",
                       &Function::侧键博弈::白刀抓长闪最远距离,
                       0.f, 5.0f);
      if (ImGui::IsItemHovered()) {
        ImGui::SetTooltip(u8"数值越高,抓的越慢");
      }

      ImGui::EndTable();
    }

    ImGui::PopItemWidth();
  }
  e_elements::end_child();
}

void 自瞄配置GUI() {
  ImGui::SetCursorPos({12, 12});
  e_elements::begin_child(u8"自瞄", ImVec2(360, 520));
  {
    gui.checkbox(u8"启用自瞄", &Function::AiMBot::AiMBotState);

    static bool waitingGameHotkeyZm = false;
    auto GetGameHotkeyName = []() {
      char name[64]{};
      if (GetKeyNameTextA(
              MapVirtualKeyA(Function::AiMBot::自瞄热键, MAPVK_VK_TO_VSC) << 16,
              name, sizeof(name))) {
        return std::string(name);
      }
      return std::to_string(Function::AiMBot::自瞄热键);
    };
    std::string gameHotkeyName = GetGameHotkeyName();
    ImGui::AlignTextToFramePadding();
    ImGui::Text(u8"自瞄热键");
    ImGui::SameLine(80);
    if (waitingGameHotkeyZm) {
      ImGui::TextColored(ImVec4(1, 0.8f, 0.2f, 1), u8"[请按键...]");
      for (int key = 0x01; key <= 0xFE; ++key) {
        if (key != 179 && GetAsyncKeyState(key) & 0x8000) {
          Function::AiMBot::自瞄热键 = key;
          waitingGameHotkeyZm = false;
          Sleep(150);
          break;
        }
      }
      if (GetAsyncKeyState(VK_ESCAPE) & 0x8000) {
        waitingGameHotkeyZm = false;
        Sleep(150);
      }
    } else {
      if (gui.button(gameHotkeyName.c_str(), ImVec2(100, 0))) {
        waitingGameHotkeyZm = true;
      }
    }

    // ImGui::InputTextEx(u8"自瞄按键", "", Function::AiMBot::AimKey,
    // sizeof(Function::AiMBot::AimKey), ImVec2(120.f, 28.f),
    // ImGuiInputTextFlags_None);

    gui.slider_float(u8"自瞄范围", &Function::AiMBot::AimRange, 10.f, 400.f);
    gui.slider_int(u8"总平滑度", &Function::AiMBot::总平滑度, 0, 100);

    gui.slider_float(u8"X平滑速度", &Function::AiMBot::smoothnessX, 0.1f, 10.f);

    gui.slider_float(u8"y平滑速度", &Function::AiMBot::smoothnessY, 0.1f, 10.f);

    gui.slider_float(u8"大范围移动限制", &Function::AiMBot::maxSpeed, 1.f,
                     100.f);

    gui.slider_float(u8"小范围平滑系数", &Function::AiMBot::smoothingFactor,
                     0.1f, 10.f);
    gui.slider_float(u8"死区阈值", &Function::AiMBot::deadZone, 0.1f, 10.f);

    gui.slider_float(u8"锁定误差", &Function::AiMBot::lockError, 0.1f, 10.f);

    gui.slider_float(u8"预判程度", &Function::AiMBot::Dt, 1.0f, 20.f);

    ImGui::Dummy(ImVec2(0.f, 8.f));
    float buttonWidth =
        (ImGui::GetContentRegionAvail().x - ImGui::GetStyle().ItemSpacing.x) *
        0.5f;
    if (gui.button(u8"保存配置", ImVec2(buttonWidth, 0))) {
      保存自瞄配置();
    }
    ImGui::SameLine();
    if (gui.button(u8"恢复默认配置", ImVec2(buttonWidth, 0))) {
      重置自瞄配置();
    }
  }
  e_elements::end_child();
}
void 杂项配置GUI() {
  // 定义一个静态变量保存选择结果
  static int kmboxMode = -1; // 0 = Net, 1 = B-Pro 2 = DhzBox_Lite
  if (kmboxMode < 0) {
    const int requestedMode =
        配置按键方式.load(std::memory_order_acquire);
    kmboxMode = (requestedMode >= 0 && requestedMode <= 2)
                    ? requestedMode
                    : 0;
  }

  ImGui::SetCursorPos({12, 12});
  ImGui::Text(u8"选择连接方式:");
  ImGui::SameLine();

  // 下拉框
  const char *items[] = {u8"KmBox-Net", u8"KmBox-B-Pro", u8"DhzBox_Lite"};
  ImGui::SetNextItemWidth(150); // 设置宽度为 150 像素
  gui.combo("##KmBoxMode", &kmboxMode, items, IM_ARRAYSIZE(items));

  // 根据选择显示不同界面
  if (kmboxMode == 0) {
    ImGui::SetCursorPos({12, 52});
    e_elements::begin_child(u8"KmBox-Net", ImVec2(240, 240));
    {
      ImGui::InputTextEx(u8"IP", "", Function::Kmbox::Net::IP,
                         sizeof(Function::Kmbox::Net::IP), ImVec2(120.f, 28.f),
                         ImGuiInputTextFlags_None);
      ImGui::InputTextEx(u8"Port", "", Function::Kmbox::Net::Port,
                         sizeof(Function::Kmbox::Net::Port),
                         ImVec2(120.f, 28.f), ImGuiInputTextFlags_None);
      ImGui::InputTextEx(u8"UUID", "", Function::Kmbox::Net::mac,
                         sizeof(Function::Kmbox::Net::mac), ImVec2(120.f, 28.f),
                         ImGuiInputTextFlags_None);

      if (gui.button(u8"连接") || connectIP) {
        connectIP = false;
        保存Kmbox();
        const InputBackendRequestGeneration generation =
            BeginInputBackendConnection(0);
        std::string ip = Function::Kmbox::Net::IP;
        std::string port = Function::Kmbox::Net::Port;
        std::string mac = Function::Kmbox::Net::mac;
        std::thread([generation, ip, port, mac]() mutable {
          for (int attempt = 0; attempt < 3; attempt++) {
            if (!IsInputBackendConnectionCurrent(generation, 0)) {
              return;
            }
            int result = 0;
            {
              std::lock_guard<std::mutex> connectLock(g_KmNetConnectionMutex);
              if (!IsInputBackendConnectionCurrent(generation, 0)) {
                return;
              }
              result = kmNet_init(ip.data(), port.data(), mac.data());
            }
            if (!CompleteKmNetConnection(generation, result)) {
              return;
            }
            if (result == 0) {
              break;
            }
            if (attempt < 2) {
              for (int waitSlice = 0; waitSlice < 50; ++waitSlice) {
                if (!IsInputBackendConnectionCurrent(generation, 0)) {
                  return;
                }
                std::this_thread::sleep_for(std::chrono::milliseconds(100));
              }
            }
          }
        }).detach();
      }
      ImGui::SameLine();
      if (gui.button(u8"测试移动鼠标")) {
        Move_mouse(20, 10);
        Sleep(50);
      }
      const int netState =
          Function::Kmbox::Net::NetState.load(std::memory_order_acquire);
      if (netState == 0) {
        ImGui::Text(u8"NET已连接");
      } else {
        ImGui::Text(u8"未连接或失败ID:[%d]", netState);
      }
    }
    e_elements::end_child();
  } else if (kmboxMode == 1) {
    ImGui::SetCursorPos({12, 52});
    e_elements::begin_child(u8"KmBox-B-Pro", ImVec2(240, 240));
    {
      ImGui::InputTextEx(u8"端口号", "", Function::Kmbox::B_Pro::PorttextBuffer,
                         sizeof(Function::Kmbox::B_Pro::PorttextBuffer),
                         ImVec2(120.f, 28.f), ImGuiInputTextFlags_None);
      ImGui::InputTextEx(u8"波特率", "", Function::Kmbox::B_Pro::SpeedBuffer,
                         sizeof(Function::Kmbox::B_Pro::SpeedBuffer),
                         ImVec2(120.f, 28.f), ImGuiInputTextFlags_None);

      if (gui.button(u8"连接")) {
        保存Kmbox();
        const InputBackendRequestGeneration generation =
            BeginInputBackendConnection(1);
        Function::Kmbox::B_Pro::BproState =
            myserial.open(atoi(Function::Kmbox::B_Pro::PorttextBuffer),
                          atoi(Function::Kmbox::B_Pro::SpeedBuffer));
        if (Function::Kmbox::B_Pro::BproState) {
          (void)CommitInputBackendConnection(generation, 1);
        }
      }
      ImGui::SameLine();
      if (gui.button(u8"测试移动鼠标")) {
        Move_mouse(20, 10);
        Sleep(50);
      }
      if (Function::Kmbox::B_Pro::BproState) {
        ImGui::Text(u8"Bpro连接成功");
      } else {
        ImGui::Text(u8"未连接或失败");
      }
    }
    e_elements::end_child();
  } else if (kmboxMode == 2) {
    ImGui::SetCursorPos({12, 52});
    e_elements::begin_child(u8"DhzBox_Lite", ImVec2(300, 330));
    {
      ImGui::InputTextEx(u8"DIP", "", Function::Kmbox::DhzBox_Lite::IP,
                         sizeof(Function::Kmbox::DhzBox_Lite::IP),
                         ImVec2(120.f, 28.f), ImGuiInputTextFlags_None);
      ImGui::InputTextEx(u8"DPort", "", Function::Kmbox::DhzBox_Lite::Port,
                         sizeof(Function::Kmbox::DhzBox_Lite::Port),
                         ImVec2(120.f, 28.f), ImGuiInputTextFlags_None);
      ImGui::InputTextEx(u8"RANDOM", "", Function::Kmbox::DhzBox_Lite::RANDOM,
                         sizeof(Function::Kmbox::DhzBox_Lite::RANDOM),
                         ImVec2(120.f, 28.f), ImGuiInputTextFlags_None);

      if (gui.button(u8"连接")) {
        保存Kmbox();
        (void)TryActivateConfiguredDhzBackend();
      }
      ImGui::SameLine();
      if (gui.button(u8"测试移动鼠标")) {
        DhzBoxAdapter::Move(20, 10);
        Sleep(50);
      }
      const DhzBoxAdapter::Status dhzStatus = DhzBoxAdapter::GetStatus();
      Function::Kmbox::DhzBox_Lite::DproState = dhzStatus.isOpen;
      if (dhzStatus.isOpen) {
        ImGui::Text(u8"DhzBox_Lite连接成功");
        ImGui::Text(u8"发送: %s",
                    dhzStatus.hasSendResult
                        ? (dhzStatus.lastSendSucceeded ? u8"正常" : u8"失败")
                        : u8"未发送");
        ImGui::Text(u8"物理键监听: %s",
                    dhzStatus.monitorHealthy ? u8"正常" : u8"等待/异常");
      } else {
        ImGui::Text(u8"未连接或失败");
      }
      if (!dhzStatus.lastError.empty()) {
        ImGui::TextWrapped("%s", dhzStatus.lastError.c_str());
      }
    }
    e_elements::end_child();
  }

  /*ImGui::SetCursorPos({190, 16});
  e_elements::begin_child(u8"KmBox-Net", ImVec2(240, 240)); {
          ImGui::InputTextEx(u8"IP", "", Function::Kmbox::Net::IP,
  sizeof(Function::Kmbox::Net::IP), ImVec2(120.f, 28.f),
  ImGuiInputTextFlags_None); ImGui::InputTextEx(u8"Port", "",
  Function::Kmbox::Net::Port, sizeof(Function::Kmbox::Net::Port),
  ImVec2(120.f, 28.f), ImGuiInputTextFlags_None); ImGui::InputTextEx(u8"UUID",
  "", Function::Kmbox::Net::mac, sizeof(Function::Kmbox::Net::mac),
  ImVec2(120.f, 28.f), ImGuiInputTextFlags_None);
          // 自动尝试连接一次
          if (ImGui::Button(u8"连接") || connectIP) {
                  connectIP = false;
                  保存Kmbox();
                  std::thread([] {
                          Function::Kmbox::Net::NetState = kmNet_init(
                                  Function::Kmbox::Net::IP,
                                  Function::Kmbox::Net::Port,
                                  Function::Kmbox::Net::mac
                          );
                          }).detach();  // 异步执行，无需等待

                          //Function::Kmbox::Net::NetState =
  kmNet_init(Function::Kmbox::Net::IP, Function::Kmbox::Net::Port,
  Function::Kmbox::Net::mac);
          }
          if (Function::Kmbox::Net::NetState == 0) {
                  ImGui::Text(u8"NET已连接"); 按键方式 = 0;
          }
          if (Function::Kmbox::Net::NetState != 0) {
  ImGui::Text(u8"未连接或失败ID:[%d]", Function::Kmbox::Net::NetState); }
  }
  e_elements::end_child();

  ImGui::SetCursorPos({ 284, 12 });
  e_elements::begin_child(u8"KmBox-B-Pro", ImVec2(240, 240)); {
          ImGui::InputTextEx(u8"端口号", "",
  Function::Kmbox::B_Pro::PorttextBuffer,
  sizeof(Function::Kmbox::B_Pro::PorttextBuffer), ImVec2(120.f, 28.f),
  ImGuiInputTextFlags_None); ImGui::InputTextEx(u8"波特率", "",
  Function::Kmbox::B_Pro::SpeedBuffer,
  sizeof(Function::Kmbox::B_Pro::SpeedBuffer), ImVec2(120.f, 28.f),
  ImGuiInputTextFlags_None); if (ImGui::Button(u8"连接")) { 保存Kmbox();
                  Function::Kmbox::B_Pro::BproState =
  myserial.open(atoi(Function::Kmbox::B_Pro::PorttextBuffer),
  atoi(Function::Kmbox::B_Pro::SpeedBuffer));	  //打开串口号COM3 波特率115200
          }
          if (Function::Kmbox::B_Pro::BproState) {
                  ImGui::Text(u8"Bpro连接成功"); 按键方式 = 1;

          }
          if (!Function::Kmbox::B_Pro::BproState) {
  ImGui::Text(u8"未连接或失败"); }
  }
  e_elements::end_child();*/

  ImGui::SetCursorPos({284, 12});
  e_elements::begin_child(u8"快捷键", ImVec2(240, 540));
  {

    ImGui::Text(u8"当前快捷键绑定：");
    ImGui::Separator();

    static std::string waitingKey = "";

    for (auto &[id, bind] : KeyBinds::binds) {
      ImGui::Text(u8"%s：", bind.name.c_str());
      ImGui::SameLine(150);

      if (waitingKey == id) {
        ImGui::TextColored(ImVec4(1, 0.8f, 0.2f, 1), u8"[请按键...]");

        for (int key = 0x01; key <= 0xFE; key++) {
          if (GetAsyncKeyState(key) & 0x8000) {
            bind.keyCode = key;

            // 获取名称（简单方式）
            char name[64];
            if (GetKeyNameTextA(MapVirtualKeyA(key, MAPVK_VK_TO_VSC) << 16,
                                name, sizeof(name)))
              bind.keyName = name;
            else
              bind.keyName = std::to_string(key);

            waitingKey.clear();
            Sleep(150);
            break;
          }
        }

        if (GetAsyncKeyState(VK_ESCAPE) & 0x8000) {
          waitingKey.clear();
          Sleep(150);
        }
      } else {
        if (gui.button(bind.keyName.c_str(), ImVec2(70, 0))) {
          waitingKey = id;
        }
      }
    }

    gui.checkbox(u8"控制台日志", &Function::Flag::开发日志);
    // gui.checkbox(u8"Mimalloc日志", &MimallocStats);
  }
  e_elements::end_child();

  if (true) {
    ImGui::SetCursorPos({284, 556});
    e_elements::begin_child(u8"数据遍历", ImVec2(240, 280));
    {
        gui.slider_int(u8"通用测试参数-用户不用管",
            &Function::Flag::通用测试延迟, 0, 2000);
       // ImGui::Spacing();
      gui.checkbox(u8"开启数据遍历", &Function::Flag::数据遍历);
      gui.checkbox(u8"打印角色状态", &Function::Flag::打印角色状态);
      gui.checkbox(u8"全局数据", &Function::Flag::全局数据);
      gui.checkbox(u8"人物数据", &Function::Flag::人物数据);
      gui.checkbox(u8"对象数据", &Function::Flag::对象数据);
      gui.checkbox(u8"堆堆数据", &Function::Flag::堆堆数据);
      gui.checkbox(u8"物品数据", &Function::Flag::物品数据);
      gui.checkbox(u8"打印物品ID", &Function::Flag::打印物品ID);
      gui.checkbox(u8"打印魂玉ID", &Function::Flag::打印魂玉ID);
      gui.checkbox(u8"显示未知物品ID", &Function::Flag::显示未知物品ID);
      gui.checkbox(u8"显示未知堆ID", &Function::Flag::显示未知堆ID);
      gui.checkbox(u8"开发日志", &Function::Flag::开发日志);
    }
    e_elements::end_child();
  }

  ImGui::SetCursorPos({12, 290});
  e_elements::begin_child(u8"融合器", ImVec2(240, 150));
  {
    if (gui.button(u8"开启")) {
      Gui.Window.BgColor = ImColor(0, 0, 0, 255);
    }
    ImGui::SameLine();
    if (gui.button(u8"关闭")) {
      Gui.Window.BgColor = ImColor(0, 0, 0, 0);
    }
    ImGui::SameLine();
    if (gui.button(u8"退出")) {
      // 退出逻辑
      ShutdownInputBackend();
      exit(0); // 或者设置一个标志，在主循环里处理
    }

    if (gui.button(u8"保存所有配置")) {
      保存模拟振配置();
      保存活化配置();
      保存自瞄配置();
      保存其他参数();
      SaveKeyBinds();
      保存博弈参数();
      保存连招参数();
    }
  }
  e_elements::end_child();

  /*ImGui::SetCursorPos({ 12, 428 });
  e_elements::begin_child(u8"延迟设置", ImVec2(240, 120)); {
          ImGui::SliderFloat(u8"博弈蓝顶延迟",
  &Function::侧键博弈::白刀蓝顶延迟, 0.f, 0.5f);
  }
  e_elements::end_child();*/

  ImGui::SetCursorPos({12, 440});
  e_elements::begin_child(u8"性能测试-控制台查看", ImVec2(240, 120));
  {
    if (gui.button(u8"开始-200左右正常值")) {
      Function::Flag::性能测试开关 = true;
    }
    gui.checkbox(u8"打印振刀效率", &Function::Flag::打印振刀效率);
  }
  e_elements::end_child();
}

void 使用说明GUI() {
  ImGui::SetCursorPos({12, 12});
  e_elements::begin_child(u8"使用说明", ImVec2(760, 680));
  {
    ImGui::PushTextWrapPos(0.0f);

    ImGui::TextColored(ImVec4(0.45f, 0.68f, 1.0f, 1.0f), u8"快速上手");
    ImGui::Separator();
    ImGui::TextWrapped(
        u8"1. 先到主页配置 KmBox 连接方式，确认连接成功后，再去开启其他功能。");
    ImGui::TextWrapped(
        u8"2. 在主页里检查快捷键绑定，常用开关建议先确认按键是否符合自己的使用习惯。");
    if (g_EnableAutoGameTheory) {
      ImGui::TextWrapped(
          u8"3. 按需进入透视、振刀、闪避、侧键博弈、博弈连招、半自动、自瞄页面，先少量开启功能，再逐步调整参数。");
    } else {
      ImGui::TextWrapped(
          u8"3. 按需进入透视、振刀、闪避、博弈连招、半自动、自瞄页面，先少量开启功能，再逐步调整参数。");
    }
    ImGui::TextWrapped(
        u8"4. 调整完成后记得保存配置，进入游戏后优先在训练或低风险环境中测试。");

    if (g_EnableAutoGameTheory) {
      ImGui::TextWrapped(u8"\n使用侧键全自动博弈时，如出现乱振刀，需关闭双键振刀");

      ImGui::Spacing();
      ImGui::TextColored(ImVec4(0.45f, 0.68f, 1.0f, 1.0f), u8"侧键博弈武器推荐");
      ImGui::Separator();

      ImGui::TextWrapped(u8"全自动博弈武器推荐: 太刀，双刀，横刀，双戟，长剑，链剑，双截棍");

      ImGui::TextWrapped(u8"优先双持武器组合推荐: ");
      ImGui::TextWrapped(u8"太刀，双刀 | 太刀，双戟 | 太刀，长剑 | 太刀，链剑 | 太刀，双截棍 | 太刀，横刀");
      ImGui::TextWrapped(u8"横刀，双刀 | 横刀，双戟 | 横刀，长剑 | 横刀，链剑 | 横刀，双截棍");
      ImGui::TextWrapped(u8"长剑，双刀 | 长剑，双戟 | 长剑，链剑 | 长剑，双截棍");
    }

    ImGui::Spacing();
    ImGui::TextColored(ImVec4(0.45f, 0.68f, 1.0f, 1.0f), u8"参数调整建议");
    ImGui::Separator();
    ImGui::TextWrapped(
        u8"距离、角度、延迟这类参数会直接影响出招时机和手感，建议每次只改一类，避免同时调整过多导致不好排查。");
    ImGui::TextWrapped(
        u8"如果某个武器手感异常，先恢复该武器默认配置，再逐项微调。");
    ImGui::TextWrapped(
        u8"闪避方式目前先使用占位配置，后续如果新增正式逻辑，再按实际功能更新说明。");

    ImGui::Spacing();
    ImGui::TextColored(ImVec4(0.45f, 0.68f, 1.0f, 1.0f), u8"注意事项");
    ImGui::Separator();
    ImGui::TextWrapped(
        u8"首次使用建议先只开一个功能页测试，确认稳定后再叠加其他功能。");
    ImGui::TextWrapped(
        u8"如果修改后效果不符合预期，可以先读取配置、恢复默认或回到主页重新保存全部配置。");
    ImGui::TextWrapped(
        u8"这份说明目前是简版，后续可以继续补充每个功能页的详细解释和推荐参数。");

    ImGui::PopTextWrapPos();
  }
  e_elements::end_child();
}

void MenuESP() {

  if (初始化读取 == false) {
    初始化读取 = true;
    SetCustomTheme();

    Hook::SetMemoryShockEnabled(false);
    Sleep(150);
    读取自瞄配置();
    Sleep(150);
    读取模拟振配置();
    Sleep(150);
    读取活化配置();
    Sleep(150);
    LoadKeyBinds();
    Sleep(150);
    ConfigDefaults::读取博弈配置(); Sleep(150);
  }

  if (Function::ESP::菜单) {
    // Animation logic
    gui.m_anim = ImLerp(gui.m_anim, 1.f, 0.045f);

    const float sidebarWidth = 170.0f;
    const float headerHeight =
        0.0f; // No header bar in Neverlose style usually, or integrated

    ImGui::SetNextWindowSize({1000, 750}); // Larger size
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, {0, 0});

    ImGui::Begin("Neverlose", nullptr,
                 ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoBackground);
    {
      auto window = ImGui::GetCurrentWindow();
      auto draw = window->DrawList;
      auto pos = window->Pos;
      auto size = window->Size;

      // Draw Background
      ImGuiStyle style = ImGui::GetStyle();
      // Main Background - Dark like Image 1 - With Rounding
      draw->AddRectFilled(pos, ImVec2(pos.x + size.x, pos.y + size.y),
                          ImColor(15, 15, 15, 255), style.WindowRounding);

      // Sidebar Background - Flat Dark
      ImVec2 sidebar_min = pos;
      ImVec2 sidebar_max = ImVec2(pos.x + sidebarWidth, pos.y + size.y);
      draw->AddRectFilled(sidebar_min, sidebar_max, ImColor(15, 15, 20, 255),
                          style.WindowRounding, ImDrawFlags_RoundCornersLeft);

      // Logo Text - "TS PRO" in White using Large Font (Index 1)
      string logoText = "TS PRO";
      if (!g_EnableAutoGameTheory) {
		  logoText = "TS PRO-Lite";
      }
      draw->AddText(ImGui::GetIO().Fonts->Fonts[1], 28.0f,
                    ImVec2(pos.x + 20, pos.y + 25), ImColor(255, 255, 255),
                    logoText.c_str());
      // Deleted "Pulse UI" text line

      // Sidebar Tabs
      ImGui::SetCursorPos(ImVec2(10, 80));
      ImGui::BeginChild("##tabs", ImVec2(150, size.y - 100), false,
                        ImGuiWindowFlags_NoScrollbar |
                            ImGuiWindowFlags_NoBackground);
      {
        int temp_tab = gui.m_tab;
        if (!g_EnableAutoGameTheory && temp_tab == 3) {
          temp_tab = 7;
        }

        if (gui.tab(ICON_FA_EYE, u8" 透视", gui.m_tab == 0))
          temp_tab = 0;
        if (gui.tab(ICON_FA_BOLT, u8" 振刀/闪避", gui.m_tab == 1))
          temp_tab = 1;
        if (gui.tab(ICON_FA_RUNNING, u8" 技能闪避", gui.m_tab == 2))
          temp_tab = 2; // Running icon
        if (g_EnableAutoGameTheory &&
            gui.tab(ICON_FA_GAMEPAD, u8" 侧键博弈", gui.m_tab == 3))
          temp_tab = 3;
        if (gui.tab(ICON_FA_FIST_RAISED, u8" 博弈连招", gui.m_tab == 4))
          temp_tab = 4;
        if (gui.tab(ICON_FA_MAGIC, u8" 半自动", gui.m_tab == 5))
          temp_tab = 5;
        if (gui.tab(ICON_FA_CROSSHAIRS, u8" 自瞄", gui.m_tab == 6))
          temp_tab = 6;
        if (gui.tab(ICON_FA_HOME, u8" 主页", gui.m_tab == 7))
          temp_tab = 7;
        if (gui.tab(ICON_FA_BOOK_OPEN, u8" 使用说明", gui.m_tab == 8))
          temp_tab = 8;

        if (temp_tab != gui.m_tab) {
          gui.m_tab = temp_tab;
          gui.m_anim = 0.0f; // Reset animation on switch
        }
      }
      ImGui::EndChild();

      // User Profile Section (Bottom Left)
      {
        ImVec2 user_box_min = ImVec2(pos.x, pos.y + size.y - 70);
        ImVec2 user_box_max = ImVec2(pos.x + sidebarWidth, pos.y + size.y);

        // Separator line
        draw->AddLine(ImVec2(user_box_min.x + 10, user_box_min.y),
                      ImVec2(user_box_max.x - 10, user_box_min.y),
                      ImColor(255, 255, 255, 20));

        // Avatar Circle (Placeholder)
        float avatar_radius = 18.0f;
        ImVec2 avatar_center = ImVec2(user_box_min.x + 30, user_box_min.y + 35);
        // Load Avatar if not loaded
        static ID3D11ShaderResourceView *UserAvatar = nullptr;
        static int AvatarWidth = 0, AvatarHeight = 0;
        if (UserAvatar == nullptr) {
          Gui.LoadTextureFromMemory(naraka_icon_data, naraka_icon_size,
                                    &UserAvatar, &AvatarWidth, &AvatarHeight);
        }

        if (UserAvatar) {
          draw->AddImage((ImTextureID)UserAvatar,
                         ImVec2(avatar_center.x - 16, avatar_center.y - 16),
                         ImVec2(avatar_center.x + 16, avatar_center.y + 16));
        } else {
          draw->AddCircleFilled(
              avatar_center, avatar_radius,
              ImColor(235, 235, 220, 255)); // Mixes well with Off-White Accent
        }

        // User Info
        draw->AddText(ImVec2(avatar_center.x + 25, avatar_center.y - 15),
                      ImColor(255, 255, 255), "Naraka");
        draw->AddText(ImVec2(avatar_center.x + 25, avatar_center.y + 2),
                      ImColor(100, 100, 120), "Till: Lifetime");
      }

      // Content Area
      ImGui::SetCursorPos(ImVec2(sidebarWidth + 20, 20));
      ImVec2 contentSize = ImVec2(size.x - sidebarWidth - 40, size.y - 40);
      ImGui::BeginChild("##content", contentSize, false,
                        ImGuiWindowFlags_NoBackground);
      {
        ImGui::PushStyleVar(ImGuiStyleVar_Alpha, gui.m_anim);

        // Helper to wrap content in a group box style if needed,
        // but our 透视配置GUI already calls e_elements::begin_child which we
        // adapted to group_box! So we just call the function.

        switch (gui.m_tab) {
        case 0:
          透视配置GUI();
          break;
        case 1:
          振刀配置GUI();
          break;
        case 2:
          活化配置GUI();
          break;
        case 3:
          if (g_EnableAutoGameTheory) {
            博弈配置GUI();
          } else {
            杂项配置GUI();
          }
          break;
        case 4:
          连招配置GUI();
          break;
        case 5:
          半自动配置GUI();
          break;
        case 6:
          自瞄配置GUI();
          break;
        case 7:
          杂项配置GUI();
          break;
        case 8:
          使用说明GUI();
          break;
        }

        ImGui::PopStyleVar();
      }
      ImGui::EndChild();
    }
    ImGui::End();
    ImGui::PopStyleVar();
  }

  // Draw overlay for "菜单_2" (Mini Status) - Keeping existing logic
  if (Function::ESP::菜单_2) {
    KeyBind kb = KeyBinds::binds["MENU_TOGGLE_2"];
    KeyBind kb_zhendao = KeyBinds::binds["F_SHOCKKNIFE"];
    KeyBind kb_dodge = KeyBinds::binds["IS_DODGE"];
    KeyBind kb_aimbot = KeyBinds::binds["AIMBOT_TOGGLE"];
    KeyBind kb_zhen3a = KeyBinds::binds["NO_SHOCKKNIFE_3A"];
    KeyBind kb_exit = KeyBinds::binds["EXIT_APP"];

    // Using Gui (OSImGui instance) which is fine
    Gui.Text(kb.keyName + u8"-显示/隐藏", Vec2{10, 280},
             ImColor{255, 255, 255});
    string 振刀状态 = Function::Shock::F_ShockKnife ? u8"启用" : u8"关闭";
    Gui.Text(kb_zhendao.keyName + u8"-模拟振刀状态 [" + 振刀状态 + "]",
             Vec2{10, 300}, ImColor{255, 255, 255});
    string 不振3a状态 = Function::LogicKnife::NO_Shock_3A ? u8"启用" : u8"关闭";
    Gui.Text(kb_zhen3a.keyName + u8"-不振3a [" + 不振3a状态 + "]",
             Vec2{10, 320}, ImColor{255, 255, 255});
    string 闪避状态 = Function::HeroActivation::IsDodge ? u8"启用" : u8"关闭";
    Gui.Text(kb_dodge.keyName + u8"-闪避状态 [" + 闪避状态 + "]", Vec2{10, 340},
             ImColor{255, 255, 255});
    string 自瞄状态 = Function::AiMBot::AiMBotState ? u8"启用" : u8"关闭";
    Gui.Text(kb_aimbot.keyName + u8"-自瞄状态 [" + 自瞄状态 + "]",
             Vec2{10, 360}, ImColor{255, 255, 255});
    Gui.Text(kb_exit.keyName + u8"-退出快捷键", Vec2{10, 390},
             ImColor(255, 255, 255, 255), 15.0f);
  }
}

void MENUKEY() {

  using namespace KeyBinds;

  if (IsRootInputKeyDown(binds["MENU_TOGGLE"].keyCode)) {
    Function::ESP::菜单 = !Function::ESP::菜单;
    Sleep(150);
  }

  if (IsRootInputKeyDown(binds["MENU_TOGGLE_2"].keyCode)) {
    Function::ESP::菜单_2 = !Function::ESP::菜单_2;
    Sleep(150);
  }

  if (IsRootInputKeyDown(binds["EXIT_APP"].keyCode)) {
    ShutdownInputBackend();
    VMMDLL_Close(mem.vHandle);
    Sleep(600);
    exit(0);
  }

  if (IsRootInputKeyDown(binds["F_SHOCKKNIFE"].keyCode)) {
    Function::Shock::F_ShockKnife = !Function::Shock::F_ShockKnife;
    Sleep(150);
  }

  if (IsRootInputKeyDown(binds["IS_DODGE"].keyCode)) {
    Function::HeroActivation::IsDodge = !Function::HeroActivation::IsDodge;
    Sleep(150);
  }

  if (IsRootInputKeyDown(binds["AIMBOT_TOGGLE"].keyCode)) {
    Function::AiMBot::AiMBotState = !Function::AiMBot::AiMBotState;
    Sleep(150);
  }

  if (IsRootInputKeyDown(binds["NO_SHOCKKNIFE_3A"].keyCode)) {
    Function::LogicKnife::NO_Shock_3A = !Function::LogicKnife::NO_Shock_3A;
    Sleep(150);
  }
}

void DrawRun() {
  static uint64_t lastMatrixAddr = 0;

  MenuESP();
  MENUKEY();
  uint64_t addr = Offset::MatrixAddr ? Offset::MatrixAddr : lastMatrixAddr;
  if (addr) {
    Global::WorldPtr.MatrixAddr = addr;
  }
  if (!addr) {
    Global::WorldPtr.Character->GetCharacterManagerPtr();
    if (Global::WorldPtr.Character->CharacterManagerPtr) {
      const uint64_t newAddr =
          Tool::GetMatrixAddr(Global::WorldPtr.Character->CharacterManagerPtr);
      if (newAddr > 0x10000) {
        Offset::MatrixAddr = newAddr;
        Global::WorldPtr.MatrixAddr = newAddr;
        addr = newAddr;
      }
    }
  }
  if (addr) {
    float tmp[4][4];
    mem.Read(addr, &tmp, sizeof(tmp));
    bool sane = true;
    float sum = 0.0f;
    for (int r = 0; r < 4 && sane; ++r) {
      for (int c = 0; c < 4; ++c) {
        const float v = tmp[r][c];
        if (!std::isfinite(v)) {
          sane = false;
          break;
        }
        sum += std::fabs(v);
      }
    }
    if (sane && sum > 0.001f) {
      std::memcpy(MATRIX, tmp, sizeof(tmp));
      lastMatrixAddr = addr;
      Global::WorldPtr.MatrixAddr = addr;
    }
  }
  PlayerESP();
  ItemESP();
  InteractiveESP();
  this_thread::sleep_for(chrono::milliseconds(1));
}
