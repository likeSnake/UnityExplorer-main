#include "nav_elements.h"

struct tab_element {
    float element_opacity, rect_opacity, text_opacity;
};

bool elements::tab(const char* name, bool boolean)
{
    ImGuiWindow* window = ImGui::GetCurrentWindow();
    if (window->SkipItems)
        return false;

    ImGuiContext& g = *GImGui;
    const ImGuiStyle& style = g.Style;
    const ImGuiID id = window->GetID(name);
    const ImVec2 label_size = tab_icons->CalcTextSizeA(15.0f, FLT_MAX, 0.0f, name);
    ImVec2 pos = window->DC.CursorPos;

    const ImRect rect(pos, ImVec2(pos.x + 36, pos.y + 36));
    ImGui::ItemSize(rect, style.FramePadding.y);
    if (!ImGui::ItemAdd(rect, id))
        return false;

    bool hovered, held;
    bool pressed = ImGui::ButtonBehavior(rect, id, &hovered, &held, NULL);

    static std::map <ImGuiID, tab_element> anim;
    auto it_anim = anim.find(id);
    if (it_anim == anim.end()) {
        anim.insert({ id, { 0.0f, 0.0f, 0.0f } });
        it_anim = anim.find(id);
    }

    const float target_glow = boolean ? 1.0f : hovered ? 0.45f : 0.0f;
    it_anim->second.element_opacity = ImLerp(it_anim->second.element_opacity, hovered ? 0.12f : 0.04f, 0.16f);
    it_anim->second.rect_opacity = ImLerp(it_anim->second.rect_opacity, target_glow, 0.18f);
    it_anim->second.text_opacity = ImLerp(it_anim->second.text_opacity, (boolean ? 1.0f : hovered ? 0.7f : 0.45f), 0.14f);

    ImColor base_bg(20, 22, 34, 200);
    ImColor gradient_a(126, 87, 255, static_cast<int>(210 * it_anim->second.rect_opacity));
    ImColor gradient_b(64, 196, 255, static_cast<int>(210 * it_anim->second.rect_opacity));
    ImColor border(255, 255, 255, static_cast<int>(40 + 80 * it_anim->second.rect_opacity));

    window->DrawList->AddRectFilled(rect.Min, rect.Max, base_bg, 12.0f);
    window->DrawList->AddRectFilledMultiColor(rect.Min, rect.Max, gradient_a, gradient_b, gradient_b, gradient_a);
    window->DrawList->AddRect(rect.Min, rect.Max, border, 12.0f);

    ImVec2 temp = { (rect.Min.x + rect.Max.x) - label_size.x, (rect.Min.y + rect.Max.y) - label_size.y };
    temp.x /= 2;
    temp.y /= 2;
    window->DrawList->AddText(tab_icons, 15.0f, temp, ImColor(235, 239, 248, static_cast<int>(255 * it_anim->second.text_opacity)), name);

    return pressed;
}

struct subtab_element {
    float element_opacity, rect_opacity, text_opacity;
};

bool elements::subtab(const char* name, bool boolean)
{
    ImGuiWindow* window = ImGui::GetCurrentWindow();
    if (window->SkipItems)
        return false;

    ImGuiContext& g = *GImGui;
    const ImGuiStyle& style = g.Style;
    const ImGuiID id = window->GetID(name);
    const ImVec2 label_size = ImGui::CalcTextSize(name);
    ImVec2 pos = window->DC.CursorPos;

    const ImRect rect(pos, ImVec2(pos.x + 158, pos.y + 44));
    ImGui::ItemSize(rect, style.FramePadding.y);
    if (!ImGui::ItemAdd(rect, id))
        return false;

    bool hovered, held;
    bool pressed = ImGui::ButtonBehavior(rect, id, &hovered, &held, NULL);

    static std::map <ImGuiID, subtab_element> anim;
    auto it_anim = anim.find(id);
    if (it_anim == anim.end()) {
        anim.insert({ id, { 0.0f, 0.0f, 0.0f } });
        it_anim = anim.find(id);
    }

    const float target_glow = boolean ? 1.0f : hovered ? 0.55f : 0.0f;
    it_anim->second.element_opacity = ImLerp(it_anim->second.element_opacity, hovered ? 0.10f : 0.04f, 0.18f);
    it_anim->second.rect_opacity = ImLerp(it_anim->second.rect_opacity, target_glow, 0.22f);
    it_anim->second.text_opacity = ImLerp(it_anim->second.text_opacity, boolean ? 1.0f : hovered ? 0.75f : 0.45f, 0.18f);

    ImColor base_bg(18, 20, 30, 200);
    ImColor gradient_a(131, 88, 255, static_cast<int>(200 * it_anim->second.rect_opacity));
    ImColor gradient_b(74, 210, 186, static_cast<int>(220 * it_anim->second.rect_opacity));
    ImColor border(255, 255, 255, static_cast<int>(40 + 90 * it_anim->second.rect_opacity));

    window->DrawList->AddRectFilled(rect.Min, rect.Max, base_bg, 12.0f);
    window->DrawList->AddRectFilledMultiColor(rect.Min, rect.Max, gradient_a, gradient_b, gradient_b, gradient_a);
    window->DrawList->AddRect(rect.Min, rect.Max, border, 12.0f);

    window->DrawList->AddText(ImVec2(rect.Min.x + 18, (rect.Min.y + rect.Max.y) / 2 - label_size.y / 2), ImColor(235, 239, 248, static_cast<int>(255 * it_anim->second.text_opacity)), name); 

    if (boolean || hovered)
    {
        ImVec2 glowMin = ImVec2(rect.Min.x + 8, rect.Max.y - 6);
        ImVec2 glowMax = ImVec2(rect.Max.x - 8, rect.Max.y - 2);
        window->DrawList->AddRectFilled(glowMin, glowMax, ImColor(140, 110, 255, static_cast<int>(120 * target_glow)), 6.0f);
    }

    return pressed;
}
