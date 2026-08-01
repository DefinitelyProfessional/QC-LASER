#include "ui-utilities/ui-windows.hpp"
#include "ui-utilities/general-ui-utilities.hpp"

#include "imgui.h"

namespace UI {
// Constructor
MathObjCreatorWindow::MathObjCreatorWindow(bool start_open) :
UIWindow(">>> MATH OBJECT CREATOR", start_open){}
// Public
void MathObjCreatorWindow::Render() {
    // Window settings
    ImGui::SetNextWindowSize(ImVec2(370, 300), ImGuiCond_FirstUseEver);
    ImGui::SetNextWindowPos(ImVec2(0, 700), ImGuiCond_FirstUseEver);
    ImGui::Begin(window_name.c_str(), &is_open);

    // +++ RealVector
    if (ImGui::CollapsingHeader("+ Create RealVector")) {
        ImGui::Indent();
        ImGui::Text("Vectors with REAL number entries.");

        // Error buffer display
        if (!error_buffer.empty()) {
            ImGui::TextColored(
                (success) ? ImVec4(0.2f,1.0f,0.1f,1.0f) : ImVec4(1.0f,0.0f,0.0f,1.0f), 
                "%s", error_buffer.c_str());
        }

        ImGui::Unindent();
        ImGui::Separator();
    }

    // +++ ComplexVector
    if (ImGui::CollapsingHeader("+ Create ComplexVector")) {
        ImGui::Indent();
        ImGui::Text("Vectors with COMPLEX number entries.");

        // Error buffer display
        if (!error_buffer.empty()) {
            ImGui::TextColored(
                (success) ? ImVec4(0.2f,1.0f,0.1f,1.0f) : ImVec4(1.0f,0.0f,0.0f,1.0f), 
                "%s", error_buffer.c_str());
        }

        ImGui::Unindent();
        ImGui::Separator();
    }

    // +++ RealMatrix
    if (ImGui::CollapsingHeader("+ Create RealMatrix")) {
        ImGui::Indent();
        ImGui::Text("Matrices with REAL number entries.");

        // Error buffer display
        if (!error_buffer.empty()) {
            ImGui::TextColored(
                (success) ? ImVec4(0.2f,1.0f,0.1f,1.0f) : ImVec4(1.0f,0.0f,0.0f,1.0f), 
                "%s", error_buffer.c_str());
        }

        ImGui::Unindent();
        ImGui::Separator();
    }

    // +++ ComplexMatrix
    if (ImGui::CollapsingHeader("+ Create ComplexMatrix")) {
        ImGui::Indent();
        ImGui::Text("Matrices with COMPLEX number entries.");

        // Error buffer display
        if (!error_buffer.empty()) {
            ImGui::TextColored(
                (success) ? ImVec4(0.2f,1.0f,0.1f,1.0f) : ImVec4(1.0f,0.0f,0.0f,1.0f), 
                "%s", error_buffer.c_str());
        }

        ImGui::Unindent();
        ImGui::Separator();
    }

    ImGui::End();
}
}