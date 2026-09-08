#include "OS-ImGui_Base.h"
#include <filesystem>

/****************************************************
* Copyright (C)	: Liv
* @file			: OS-ImGui_Base.cpp
* @author		: Liv
* @email		: 1319923129@qq.com
* @version		: 1.0
* @date			: 2023/6/18	11:21
****************************************************/

namespace OSImGui
{
    namespace
    {
        std::string FindExistingFont(const std::initializer_list<std::string>& candidates)
        {
            for (const auto& path : candidates)
            {
                if (std::filesystem::exists(path))
                {
                    return path;
                }
            }
            return {};
        }
    }

    bool OSImGui_Base::InitImGui(ID3D11Device* device, ID3D11DeviceContext* device_context)
    {
        ImGui::CreateContext();
        ImGuiIO& io = ImGui::GetIO(); (void)io;
        const ImWchar* glyph_range = io.Fonts->GetGlyphRangesChineseFull();

        ImFontConfig base_cfg{};
        base_cfg.OversampleH = 3;
        base_cfg.OversampleV = 3;
        base_cfg.PixelSnapH = false;

        const std::string latin_font = FindExistingFont({
            "C:\\Windows\\Fonts\\Inter-Medium.ttf",
            "C:\\Windows\\Fonts\\Inter-Regular.ttf",
            "C:\\Windows\\Fonts\\Roboto-Medium.ttf",
            "C:\\Windows\\Fonts\\Roboto-Regular.ttf"
            });
        const std::string chinese_font = FindExistingFont({
            "C:\\Windows\\Fonts\\msyh.ttc",
            "C:\\Windows\\Fonts\\msyhbd.ttc"
            });

        ImFont* font = nullptr;
        if (!latin_font.empty())
        {
            font = io.Fonts->AddFontFromFileTTF(latin_font.c_str(), 16.f, &base_cfg);
            if (!chinese_font.empty())
            {
                ImFontConfig merge_cfg = base_cfg;
                merge_cfg.MergeMode = true;
                io.Fonts->AddFontFromFileTTF(chinese_font.c_str(), 16.f, &merge_cfg, glyph_range);
            }
        }

        if (font == nullptr)
        {
            const std::string fallback_font = !chinese_font.empty() ? chinese_font : "C:\\Windows\\Fonts\\msyhbd.ttc";
            font = io.Fonts->AddFontFromFileTTF(fallback_font.c_str(), 16.f, NULL, glyph_range);
        }

      
        //ImGui::StyleColorsLight();
        
        io.LogFilename = nullptr;

        if (!ImGui_ImplWin32_Init(Window.hWnd))
            throw OSException("ImGui_ImplWin32_Init() call failed.");
        if (!ImGui_ImplDX11_Init(device, device_context))
            throw OSException("ImGui_ImplDX11_Init() call failed.");

        return true;
    }

    void OSImGui_Base::CleanImGui()
    {
        ImGui_ImplDX11_Shutdown();
        ImGui_ImplWin32_Shutdown();
        ImGui::DestroyContext();

        g_Device.CleanupDeviceD3D();
        DestroyWindow(Window.hWnd);
        UnregisterClassA(Window.ClassName.c_str(), Window.hInstance);
    }

    std::wstring OSImGui_Base::StringToWstring(std::string& str)
    {
        std::wstring_convert<std::codecvt_utf8<wchar_t>> converter;
        return converter.from_bytes(str);
    }

}
