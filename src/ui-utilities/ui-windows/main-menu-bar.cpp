#include "ui-utilities/ui-windows.hpp"
#include "ui-utilities/general-ui-utilities.hpp"
#include "thread-core/thread-pool.hpp"

#include "imgui.h"

namespace UI {
// Constructor, Note the open_flags store 8 bools in one byte, used for windows
MainMenuBar::MainMenuBar(
    MULTI::ThreadPool* TP,
    SandboxManagerWindow* SM,
    MathObjCreatorWindow* MC,
    MathObjExplorerWindow* ME
) : UIWindow("MAIN MENU BAR"), 
    threadpool(TP), 
    sandbox_manager_win(SM), 
    math_obj_creator_win(MC),
    math_obj_explorer_win(ME)
{}

// Public
void MainMenuBar::Render() {
    if (!ImGui::BeginMainMenuBar()) {return;}

    //
    if (ImGui::BeginMenu("DATA")) {
        ImGui::MenuItem("SANDBOX MANAGER", nullptr, &sandbox_manager_win->is_open);
        if (ImGui::IsItemHovered(ImGuiHoveredFlags_DelayNormal)) {
            ImGui::SetTooltip("Manage sandbox files");
        }

        ImGui::MenuItem("MATH OBJECT EXPLORER", nullptr, &math_obj_explorer_win->is_open);
        if (ImGui::IsItemHovered(ImGuiHoveredFlags_DelayNormal)) {
            ImGui::SetTooltip("Manage existing math objects");
        }

        ImGui::MenuItem("MATH OBJECT CREATOR", nullptr, &math_obj_creator_win->is_open);
        if (ImGui::IsItemHovered(ImGuiHoveredFlags_DelayNormal)) {
            ImGui::SetTooltip("Create new math objects");
        }

        if (ImGui::MenuItem("close all")) {
            sandbox_manager_win->is_open = false;
            math_obj_explorer_win->is_open = false;
            math_obj_creator_win->is_open = false;
        }

        if (ImGui::MenuItem("open all")) {
            sandbox_manager_win->is_open = true;
            math_obj_explorer_win->is_open = true;
            math_obj_creator_win->is_open = true;
        }

        ImGui::EndMenu();
    }
    ImGui::Text("   Using");
    ImGui::TextColored(
        ImVec4(1.0f,0.5f,0.0f,1.0f),
        "%s", sandbox_manager_win->get_active_filename().c_str()
    );

    sandbox_manager_win->save_current_active_button();

    ImGui::TextColored(
        ImVec4(0.0f,1.0f,1.0f,1.0f),
        "   < %i Threads Active >", threadpool->get_active_threads()
    );

    ImGui::EndMainMenuBar();
}
}