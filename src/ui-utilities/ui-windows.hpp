#pragma once

// for UIWindow 
#include "ui-utilities/general-ui-utilities.hpp"

#include <filesystem>
#include <functional>
#include <vector>
#include <string>

namespace UI {
// Manage Sandbox files, Create, Select, Delete
class SandboxManagerWindow : public UIWindow {
private:
    // File variables
    const std::filesystem::path& saved_data_dir;
    std::string active_filename;
    std::string error_buffer = "";
    std::vector<std::string> db_filenames;
    std::vector<bool> selected_delete;
    int selected_delete_count = 0; 
    int selected_index = -1;
    int db_filenames_size;
    char new_sandbox_input[64] = "";
    bool success = false;
    bool is_busy = false;
    
    // Validate new sandbox filename
    bool is_valid_new_filename(std::string& filename);

    // Windows Shell API to move to Recycle Bin
    void execute_delete_to_trash();

public:
    // Event Listeners to be defined in main.cpp
    std::function<void(std::string)> EVENT_OnSelectSandbox;
    std::function<void(std::string)> EVENT_OnCreateSandbox;
    std::function<void(void)> EVENT_OnSaveCurrentSandbox;

    // Constructor simply sets UIWindow window_name and directory filepath
    SandboxManagerWindow(const std::filesystem::path& data_dir, std::string active_file);

    // Set the active filename
    inline void set_active_filename(std::string new_active) {active_filename = new_active;};

    // Set error buffer for communication
    inline void set_error_buffer(bool status, std::string msg) {success = status; error_buffer = msg;};

    // Reset the busy status to false
    inline void reset_busy_status() {is_busy = false;};

    // Refresh filenames and other states and functionalities
    void refresh_filenames();

    // Render function definition
    void Render() override;
};
}