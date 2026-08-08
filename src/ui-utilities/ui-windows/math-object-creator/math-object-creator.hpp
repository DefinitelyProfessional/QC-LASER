#pragma once

// For UIWindow 
#include "ui-utilities/general-ui-utilities.hpp"

#include <cstdint>

namespace UI {
// Accept input as entries to create new math objects
class MathObjCreatorWindow : public UIWindow {
private:
    uint32_t vector_dim_buffer = 2;
    uint32_t matrix_row_buffer = 2;
    uint32_t matrix_col_buffer = 2;

public:
    // Constructor simply sets UIWindow window_name
    explicit MathObjCreatorWindow(bool start_open);

    // Render function definition
    void Render() override;
};
}