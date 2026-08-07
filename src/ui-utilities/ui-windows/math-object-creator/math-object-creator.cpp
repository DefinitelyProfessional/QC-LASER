#include "ui-utilities/ui-windows.hpp"
#include "ui-utilities/general-ui-utilities.hpp"
#include "ui-utilities/ui-windows/math-object-creator/subwin/real-vector-subwin.cpp"


#include "imgui.h"

namespace UI {
// Constructor
MathObjCreatorWindow::MathObjCreatorWindow(bool start_open) :
UIWindow(">>> MATH OBJECT CREATOR", start_open){}
// Public
void MathObjCreatorWindow::Render() {
    // Window settings
    ImGui::SetNextWindowSize(ImVec2(330, 330), ImGuiCond_FirstUseEver);
    ImGui::SetNextWindowPos(ImVec2(0, 756), ImGuiCond_FirstUseEver);
    ImGui::Begin(window_name.c_str(), &is_open);


    // +++ RealVector
    if (ImGui::CollapsingHeader("+ Create RealVector")) {
        ImGui::Indent();
        ImGui::Text("Vectors with REAL number entries.");

        display_error_buffer();
        
        // Input box for the vector's dimensions
        ImGui::Text("Enter vector dimension : ");
        bool create_input_entered = ImGui::InputScalar(
            "###SetDimInputText",
            ImGuiDataType_U32,
            &vector_dim_buffer
        );
        ImGui::SameLine();
        ImGui::BeginDisabled(is_busy);
        bool create_button_pressed = ImGui::Button("Create###CreateNewRealVectorButton");
        ImGui::EndDisabled();

        if (!is_busy && (create_input_entered || create_button_pressed)) {
            if (vector_dim_buffer < 2 || vector_dim_buffer > 50) {
                set_error_buffer(false, "Manual input only supports from 2 up to 50 entries");
                vector_dim_buffer = 2;
            } else {
                //
                set_error_buffer(true, "We got it good.");
            }
        }

        ImGui::Unindent();
        ImGui::Separator();
    }


    // +++ ComplexVector
    if (ImGui::CollapsingHeader("+ Create ComplexVector")) {
        ImGui::Indent();
        ImGui::Text("Vectors with COMPLEX number entries.");

        display_error_buffer();

        ImGui::Unindent();
        ImGui::Separator();
    }


    // +++ RealMatrix
    if (ImGui::CollapsingHeader("+ Create RealMatrix")) {
        ImGui::Indent();
        ImGui::Text("Matrices with REAL number entries.");

        display_error_buffer();

        ImGui::Unindent();
        ImGui::Separator();
    }


    // +++ ComplexMatrix
    if (ImGui::CollapsingHeader("+ Create ComplexMatrix")) {
        ImGui::Indent();
        ImGui::Text("Matrices with COMPLEX number entries.");

        display_error_buffer();

        ImGui::Unindent();
        ImGui::Separator();
    }

    
    ImGui::End();
}
}