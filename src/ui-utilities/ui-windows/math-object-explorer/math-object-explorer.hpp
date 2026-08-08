#pragma once

// For UIWindow 
#include "ui-utilities/general-ui-utilities.hpp"

namespace UI {
// Manage math objects present in the active sandbox, basically CRUD
class MathObjExplorerWindow : public UIWindow {
private:
public:
    // Constructor simply sets UIWindow window_name
    explicit MathObjExplorerWindow(bool start_open);

    // Render function definition
    void Render() override;
};
}