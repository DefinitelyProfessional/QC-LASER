#pragma once

// For UIWindow 
#include "ui-utilities/general-ui-utilities.hpp"

// Forward declare
namespace MULTI {class ThreadPool;}
namespace UI {
    class SandboxManagerWindow;
    class MathObjExplorerWindow;
    class MathObjCreatorWindow;
}

namespace UI {
// That top menu bar in primary window
class MainMenuBar : public UIWindow {
private:
    // UI variables
    MULTI::ThreadPool* threadpool;
    SandboxManagerWindow* sandbox_manager_win;
    MathObjExplorerWindow* math_obj_explorer_win;
    MathObjCreatorWindow* math_obj_creator_win;
    // 
    bool lock_all_windows = false;
public:
    // the main menu controls its visibility but the debug metrics window itself is in render loop
    bool debug_metrics_is_open = false;

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