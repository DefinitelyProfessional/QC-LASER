#pragma once

// for UIWindow 
#include "ui-utilities/general-ui-utilities.hpp"

#include <filesystem>
#include <functional>
#include <vector>
#include <string>

// Manage Sandbox files, Create, Select, Delete
class SandboxManagerWindow : public UIWindow {
private:
    // File variables
    const std::filesystem::path& saved_data_dir;
    std::string active_filename;
    std::vector<std::string> db_filenames;
    int db_filenames_size;
    
    // UI State Trackers
    std::vector<bool> selected_delete;
    int selected_delete_count = 0; 
    int selected_index = -1;
    char new_sandbox_input[64] = "";
    std::string error_buffer = "";
    bool success = false;

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
    void set_active_filename(std::string new_active) {active_filename = new_active;}

    // Set error buffer for communication
    void set_error_buffer(bool success, std::string msg);

    // Refresh filenames and other states and functionalities
    void refresh_filenames();

    // Render function definition
    void Render() override;
};