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
    ImGui::SetNextWindowSize(ImVec2(370, 330), ImGuiCond_FirstUseEver);
    ImGui::SetNextWindowPos(ImVec2(0, 370), ImGuiCond_FirstUseEver);
    ImGui::Begin(window_name.c_str(), &is_open);
    
    // Error buffer display
    if (!error_buffer.empty()) {
        ImGui::TextColored(
            (success) ? ImVec4(0.2f,1.0f,0.1f,1.0f) : ImVec4(1.0f,0.0f,0.0f,1.0f), 
            "%s", error_buffer.c_str());
    }

    ImGui::End();
}
}