#include "ui-utilities/general-ui-utilities.hpp"

#include "imgui.h"

#include <string>

// Constructor and Destructor definition to reduce vtable lookup
UIWindow::UIWindow(const std::string& name, bool startOpen) : window_name(name), is_open(startOpen) {}
UIWindow::~UIWindow() = default;

void UIWindow::display_error_buffer() {
    if (!error_buffer.empty()) {
        ImGui::TextColored(
            (success) ? ImVec4(0.2f,1.0f,0.1f,1.0f) : ImVec4(1.0f,0.0f,0.0f,1.0f), 
            "%s", error_buffer.c_str()
        );
    }
}

namespace UI {
void SimpleFonts::PushSmall()  {ImGui::PushFont(Small);}
void SimpleFonts::PushMedium() {ImGui::PushFont(Medium);}
void SimpleFonts::PushLarge()  {ImGui::PushFont(Large);}
void SimpleFonts::Pop()        {ImGui::PopFont();}

void GlobalWindowFlags::toggle_lock() {
    is_locked = !is_locked;
    if (is_locked) {
        flag |= (ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize);
    } else {
        flag &= ~(ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize);
    }
}
}