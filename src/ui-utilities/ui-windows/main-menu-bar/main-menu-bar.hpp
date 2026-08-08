#pragma once

// For UIWindow 
#include "ui-utilities/general-ui-utilities.hpp"

// Forward declare
namespace MULTI {class ThreadPool;}
namespace UI {class SandboxManagerWindow;}
namespace UI {class MathObjExplorerWindow;}
namespace UI {class MathObjCreatorWindow;}

namespace UI {
// That top menu bar in primary window
class MainMenuBar : public UIWindow {
private:
    // UI variables
    MULTI::ThreadPool* threadpool;
    SandboxManagerWindow* sandbox_manager_win;
    MathObjExplorerWindow* math_obj_explorer_win;
    MathObjCreatorWindow* math_obj_creator_win;

public:
    // Constructor simply sets UIWindow window_name
    explicit MainMenuBar(
        MULTI::ThreadPool* TP,
        SandboxManagerWindow* SM,
        MathObjExplorerWindow* ME,
        MathObjCreatorWindow* MC
    );

    // Render function definition
    void Render() override;
};
}