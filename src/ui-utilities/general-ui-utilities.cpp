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