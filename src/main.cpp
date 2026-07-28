#include "data-utilities/data-utilities.hpp"
#include "data-utilities/data-payload.hpp"
#include "ui-utilities/stage-utilities.hpp"
#include "ui-utilities/ui-windows.hpp"
#include "thread-core/thread-pool.hpp"
#include "thread-core/result-pool.hpp"

#include <GLFW/glfw3.h>
#include "imgui.h"

#define WIN32_LEAN_AND_MEAN // Trim down Windows header
#include <windows.h>

#include <filesystem>
#include <iostream>
#include <utility>
#include <memory>
#include <string>

namespace fs = std::filesystem;

// Global pointer to Thread Pool
std::unique_ptr<ThreadPool> G_threadpool;
std::unique_ptr<OutputPool> G_outputpool;

int main() {
    // Initialize ThreadPool, OutputPool, and directory locations =========================================================
    G_threadpool = std::make_unique<ThreadPool>(3); // CONCURRENCY CAUTION
    G_outputpool = std::make_unique<OutputPool>(); // Will be executed at main thread
    fs::path ROOT_DIR, SAVED_DATA_DIR, ASSETS_DIR;
    STAGE::InitializeDirectories(ROOT_DIR, SAVED_DATA_DIR, ASSETS_DIR);


    // IMGUI UI SUBSYSTEMS INITIALIZATION =====================================================================
    GLFWwindow* host_window = STAGE::InitializeUI(
        750, 1000, "QC Linear Algebra Sandbox Engine R.", ROOT_DIR);
    if (!host_window) {std::cerr << "Fatal Error: Failed to initialize application stages."; return -1;}
    ImVec4 clear_color = ImVec4(0.12f, 0.12f, 0.14f, 1.00f);
    // ========================================================================================================


    // Load the Default SandboxDataManager being "MAIN_sandbox.h5" ===============================================
    SandboxDataManager active_sandbox(SAVED_DATA_DIR, "MAIN_sandbox.h5");
    // WindowManager to handle unified rendering of all windows ===============================================
    STAGE::WindowManager win_manager;
    // Register windows and get their pointers for event listeners ============================================
    SandboxManagerWindow* sandbox_win = win_manager.RegisterWindow<SandboxManagerWindow>(
        SAVED_DATA_DIR, active_sandbox.get_active_filename());

    auto  switch_whole_sandbox = [&active_sandbox, sandbox_win](std::string filename) {
        sandbox_win->set_error_buffer(true, "Loading " + filename + " ...");

        G_threadpool->assign_task([&active_sandbox, sandbox_win, target = std::move(filename)]() {
            StatusPayload status = active_sandbox.switch_whole_sandbox(target);

            G_outputpool->enqueue([&active_sandbox, sandbox_win, result = std::move(status)]() {
                sandbox_win->set_error_buffer(result.success, result.msg);
                if (result.success) {sandbox_win->set_active_filename(active_sandbox.get_active_filename());}
                sandbox_win->reset_busy_status();
                glfwPostEmptyEvent(); // Wake up Main Thread to process UI changes
            });
        });
    };
    sandbox_win->EVENT_OnSelectSandbox =  switch_whole_sandbox;
    sandbox_win->EVENT_OnCreateSandbox =  switch_whole_sandbox;
    sandbox_win->EVENT_OnSaveCurrentSandbox = [&active_sandbox, sandbox_win]() {
        sandbox_win->set_error_buffer(true, "Saving active sandbox ...");

        G_threadpool->assign_task([&active_sandbox, sandbox_win]{
            StatusPayload status = active_sandbox.save_whole_sandbox();

            G_outputpool->enqueue([sandbox_win, result = std::move(status)]{
                sandbox_win->set_error_buffer(result.success, result.msg);
                sandbox_win->reset_busy_status();
                glfwPostEmptyEvent(); // Wake up Main Thread to process UI changes
            });
        });
    };


    // CORE IMGUI RENDER LOOP =================================================================================
    while (!glfwWindowShouldClose(host_window)) {
        STAGE::StartRenderLoop();

        ImGui::ShowDemoWindow();
        // -----------------------------------------------------------
        // Unified rendering of all window elements
        // -----------------------------------------------------------
        win_manager.RenderAll();
        G_outputpool->execute_results();
        // -----------------------------------------------------------
        // Finalize geometry and push to the GPU
        // -----------------------------------------------------------
        STAGE::EndRenderLoop(host_window, clear_color);
    }
    // ========================================================================================================


    // DEALLOCATE EVERYTHING AND EXIT =========================================================================
    active_sandbox.save_whole_sandbox(); // Save current data before exit
    STAGE::ShutdownUI(host_window);
    G_threadpool.reset(); // Destroy all threads
    G_outputpool.reset(); // Destroy all results
    // ========================================================================================================
    std::cout << "End of Program : Thank you and see you later." << std::endl;
    return 0;
}