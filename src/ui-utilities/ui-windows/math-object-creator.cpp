#include "ui-utilities/ui-windows.hpp"
#include "ui-utilities/general-ui-utilities.hpp"

#include "imgui.h"

namespace UI {
MathObjectCreatorWindow::MathObjectCreatorWindow() :
UIWindow(">>> MATH OBJECT CREATOR"){

}
// Public
void MathObjectCreatorWindow::Render() {
    // Window settings
    ImGui::SetNextWindowSize(ImVec2(450, 500), ImGuiCond_FirstUseEver);
    ImGui::SetNextWindowPos(ImVec2(0, 400), ImGuiCond_FirstUseEver);
    ImGui::Begin(window_name.c_str());

    //

    ImGui::End();
}
}