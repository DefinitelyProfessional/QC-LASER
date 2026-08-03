#pragma once

// for ThreadPool
#include "thread-core/thread-pool.hpp"

// for UIWindow 
#include "ui-utilities/general-ui-utilities.hpp"

#include <filesystem>
#include <functional>
#include <cstdint>
#include <vector>
#include <string>

namespace UI {
// Manage Sandbox files, Create, Select, Delete, Rename, etc
class SandboxManagerWindow : public UIWindow {
private:
    // File variables
    const std::filesystem::path& saved_data_dir;
    std::string active_filename;
    std::vector<std::string> db_filenames;
    std::vector<bool> selected_delete;
    int selected_delete_count = 0; 
    int selected_index = -1;
    int db_filenames_size;
    char new_sandbox_input[64] = "";
    
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
    SandboxManagerWindow(bool start_open,const std::filesystem::path& data_dir, std::string active_file);

    // Set the active filename
    inline void set_active_filename(const std::string& new_active) {active_filename = new_active;};

    // Set the active filename
    inline const std::string& get_active_filename() const {return active_filename;};

    // Refresh filenames and other states and functionalities
    void refresh_filenames();

    // Create the imgui button to save current active sandbox
    void save_current_active_button();

    // Render function definition
    void Render() override;
};


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


// Manage math objects present in the active sandbox, basically CRUD
class MathObjExplorerWindow : public UIWindow {
private:
public:
    // Constructor simply sets UIWindow window_name
    explicit MathObjExplorerWindow(bool start_open);

    // Render function definition
    void Render() override;
};


// 
class MainMenuBar : public UIWindow {
private:
    // UI variables
    MULTI::ThreadPool* threadpool;
    SandboxManagerWindow* sandbox_manager_win;
    MathObjCreatorWindow* math_obj_creator_win;
    MathObjExplorerWindow* math_obj_explorer_win;

public:
    // Constructor simply sets UIWindow window_name
    explicit MainMenuBar(
        MULTI::ThreadPool* TP,
        SandboxManagerWindow* SM,
        MathObjCreatorWindow* MC,
        MathObjExplorerWindow* ME
    );

    // Render function definition
    void Render() override;
};
}