#pragma once

// for UIWindow 
#include "ui-utilities/general-utilities.hpp"

#include <filesystem>
#include <functional>
#include <vector>
#include <string>

// Manage Sandbox files, Create, Select, Delete
class SandboxManagerWindow : public UIWindow {
private:
    std::filesystem::path saved_data_dir;
    const std::string& active_filename; // read-only reference from SandboxSessionManager
    std::vector<std::string> db_filenames;
    int db_filenames_size;
    
    // UI State Trackers
    std::vector<bool> selected_delete;
    int selected_delete_count = 0; 
    int selected_index = -1;
    std::string error_buffer = "";
    char new_sandbox_input[64] = "";

    // Validate new sandbox filename
    bool is_valid_new_filename(std::string& filename);

    // Windows Shell API to move to Recycle Bin
    void execute_delete_to_trash();

public:
    // Event Listeners to be defined in main.cpp
    std::function<void(std::string)> Event_OnSelectSandbox;
    std::function<void(std::string)> Event_OnCreateSandbox;
    std::function<void(void)> Event_OnSaveCurrentSandbox;

    // Constructor simply sets UIWindow window_name and directory filepath
    SandboxManagerWindow(const std::filesystem::path& data_dir, const std::string& active_file);

    // Refresh filenames and other states and functionalities
    void refresh_filenames();

    // Render function definition
    void Render() override;
};