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

        ImGui::MenuItem("MATH OBJECT CREATOR", nullptr, &math_obj_creator_win->is_open);
        if (ImGui::IsItemHovered(ImGuiHoveredFlags_DelayNormal)) {
            ImGui::SetTooltip("Create new math objects");
        }

        ImGui::MenuItem("MATH OBJECT EXPLORER", nullptr, &math_obj_explorer_win->is_open);
        if (ImGui::IsItemHovered(ImGuiHoveredFlags_DelayNormal)) {
            ImGui::SetTooltip("Manage existing math objects");
        }
        ImGui::EndMenu();
    }

    ImGui::TextColored(
        ImVec4(1.0f,0.5f,0.0f,1.0f),
        "< %i Threads Active >", threadpool->get_active_threads()
    );

    ImGui::EndMainMenuBar();
}
}