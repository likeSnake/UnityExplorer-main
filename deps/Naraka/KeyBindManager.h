#pragma once
#include <unordered_map>
#include <string>
#include <Windows.h>

struct KeyBind {
    std::string name;   // 功能名（比如 “自动拾取武器”）
    int keyCode;        // 虚拟键值（VK_F5 等）
    std::string keyName; // 显示的键名（“F5”）
};

// 全局快捷键管理器
namespace KeyBinds {
    // 快捷键映射表
    inline std::unordered_map<std::string, KeyBind> binds = {
        { "MENU_TOGGLE", { u8"界面显示隐藏", VK_HOME, "HOME" } },
        { "F_SHOCKKNIFE", { u8"振刀开关", VK_F5, "F5" } },
        { "NO_SHOCKKNIFE_3A", { u8"不振3A开关", VK_F8, "F8" } },
        //{ "SHOCKKNIFE_KEY", { u8"振刀按键", VK_HOME, "HOME" } },
        { "IS_DODGE", { u8"闪避开关", VK_F6, "F6" } },
        { "AIMBOT_TOGGLE", { u8"自瞄开关", VK_F7, "F7" } },
        //{ "AIMBOT_TOGGLE_HOT", { u8"自瞄热键", VK_CONTROL, "Ctrl" } },
        { "MENU_TOGGLE_2", { u8"左侧开关提示文字", VK_INSERT, "INSERT" } },
        { "EXIT_APP", { u8"退出程序", VK_DELETE, "DELETE" } },
    };
}
