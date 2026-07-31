#include "ui-utilities/ui-windows.hpp"
#include "ui-utilities/general-ui-utilities.hpp"

#include "imgui.h"

struct SandboxManagerWindow;
struct MathObjectCreatorWindow;

namespace UI {
// Constructor, Note the open_flags store 8 bools in one byte, used for windows
MainMenuBar::MainMenuBar() : UIWindow("MAIN MENU BAR") {}

// Public
void MainMenuBar::Render() {
    if (!ImGui::BeginMainMenuBar()) {return;}

    //
    if (ImGui::BeginMenu("Manage Data")) {
        if (ImGui::MenuItem("Open Sandbox Manager")) {
            if (EVENT_OnOpenSandboxManager) {EVENT_OnOpenSandboxManager();}
        }
        if (ImGui::MenuItem("Open Math Object Creator")) {
            if (EVENT_OnOpenMathObjectCreator) {EVENT_OnOpenMathObjectCreator();}
        }
        ImGui::EndMenu();
    }

    ImGui::EndMainMenuBar();
}
}