#include "ui-utilities/ui-windows/math-object-explorer/math-object-explorer.hpp"
#include "ui-utilities/general-ui-utilities.hpp"

#include "imgui.h"

namespace UI {
// Constructor 
MathObjExplorerWindow::MathObjExplorerWindow(bool start_open) : 
UIWindow(">>> MATH OBJECT EXPLORER", start_open){}

// Public
void MathObjExplorerWindow::Render(){
    // Window settings
    ImGui::SetNextWindowSize(ImVec2(330, 330), ImGuiCond_FirstUseEver);
    ImGui::SetNextWindowPos(ImVec2(0, 426), ImGuiCond_FirstUseEver);
    ImGuiWindowFlags final_flags = ImGuiWindowFlags_NoSavedSettings | UI::G_WindowFlags.flag;
    ImGui::Begin(window_name.c_str(), &is_open, final_flags);
    
    display_error_buffer();

    ImGui::End();
}
}