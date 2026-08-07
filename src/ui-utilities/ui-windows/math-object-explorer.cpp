#include "ui-utilities/ui-windows.hpp"
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
    ImGui::Begin(window_name.c_str(), &is_open);
    
    display_error_buffer();

    ImGui::End();
}
}