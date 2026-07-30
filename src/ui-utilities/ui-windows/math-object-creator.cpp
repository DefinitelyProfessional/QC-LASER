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
    ImGui::SetNextWindowSize(ImVec2(400, 400), ImGuiCond_FirstUseEver);
    ImGui::SetNextWindowPos(ImVec2(0, 350), ImGuiCond_FirstUseEver);
    ImGui::Begin(window_name.c_str());

    // Error buffer display
    if (!error_buffer.empty()) {
        ImGui::TextColored(
            (success) ? ImVec4(0.2f,1.0f,0.1f,1.0f) : ImVec4(1.0f,0.0f,0.0f,1.0f), 
            "%s", error_buffer.c_str());
    }
    // +++ RealVector
    if (ImGui::CollapsingHeader("+ Create RealVector")) {
        ImGui::Indent();
        ImGui::Text("Vectors with REAL number entries.");

        ImGui::Unindent();
    }
    // +++ ComplexVector
    if (ImGui::CollapsingHeader("+ Create ComplexVector")) {
        ImGui::Indent();
        ImGui::Text("Vectors with COMPLEX number entries.");
        ImGui::Unindent();
    }
    // +++ RealMatrix
    if (ImGui::CollapsingHeader("+ Create RealMatrix")) {
        ImGui::Indent();
        ImGui::Text("Matrices with REAL number entries.");
        ImGui::Unindent();
    }
    // +++ ComplexMatrix
    if (ImGui::CollapsingHeader("+ Create ComplexMatrix")) {
        ImGui::Indent();
        ImGui::Text("Matrices with COMPLEX number entries.");
        ImGui::Unindent();
    }

    ImGui::End();
}
}