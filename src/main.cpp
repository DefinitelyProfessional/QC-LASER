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
#include <chrono>
#include <thread>
#include <string>
#include <ratio>

namespace fs = std::filesystem;

// Define target frame duration (1000 milliseconds / 60 FPS = 16.666 ms per frame)
constexpr std::chrono::duration<double, std::milli> targetFrameTime(1000.0 / 60.0);

// Global pointer to Thread Pool
std::unique_ptr<ThreadPool> G_threadpool;
std::unique_ptr<ResultPool> G_resultpool;

int main() {
    // Initialize ThreadPool, ResultPool, and directory locations =========================================================
    G_threadpool = std::make_unique<ThreadPool>(3); // CONCURRENCY CAUTION
    G_resultpool = std::make_unique<ResultPool>(); // Will be executed at main thread
    fs::path ROOT_DIR, SAVED_DATA_DIR, ASSETS_DIR;
    STAGE::InitializeDirectories(ROOT_DIR, SAVED_DATA_DIR, ASSETS_DIR);

    
    // IMGUI UI SUBSYSTEMS INITIALIZATION =====================================================================
    GLFWwindow* host_window = STAGE::InitializeApplication(750, 1000, "QC Linear Algebra Sandbox Engine R.", ROOT_DIR);
    if (!host_window) {std::cerr << "Fatal Error: Failed to initialize application stages." << std::endl; return -1;}
    ImVec4 clear_color = ImVec4(0.12f, 0.12f, 0.14f, 1.00f);
    // ========================================================================================================


    // Load the Default SandboxManager being "MAIN_sandbox.h5" ===============================================
    SandboxManager active_sandbox(SAVED_DATA_DIR, "MAIN_sandbox.h5");
    // WindowManager to handle unified rendering of all windows ===============================================
    STAGE::WindowManager win_manager;
    // Register windows and get their pointers for event listeners ============================================
    SandboxManagerWindow* sandbox_win = win_manager.RegisterWindow<SandboxManagerWindow>(
        SAVED_DATA_DIR, active_sandbox.get_active_filename());
    auto  switch_whole_sandbox = [&active_sandbox, sandbox_win](std::string filename) {
        sandbox_win->set_error_buffer(true, "Loading " + filename + " ...");

        G_threadpool->assign_task([&active_sandbox, sandbox_win, target = std::move(filename)]() {
            StatusPayload status = active_sandbox.switch_whole_sandbox(target);

            G_resultpool->enqueue([&active_sandbox, sandbox_win, result = std::move(status)]() {
                sandbox_win->set_error_buffer(result.success, result.msg);
                if (result.success) {sandbox_win->set_active_filename(active_sandbox.get_active_filename());}
            });
        });
    };
    sandbox_win->EVENT_OnSelectSandbox =  switch_whole_sandbox;
    sandbox_win->EVENT_OnCreateSandbox =  switch_whole_sandbox;
    sandbox_win->EVENT_OnSaveCurrentSandbox = [&active_sandbox, sandbox_win]() {
        sandbox_win->set_error_buffer(true, "Saving active sandbox ...");

        G_threadpool->assign_task([&active_sandbox, sandbox_win]{
            StatusPayload status = active_sandbox.save_whole_sandbox();

            G_resultpool->enqueue([sandbox_win, result = std::move(status)]{
                sandbox_win->set_error_buffer(result.success, result.msg);
            });
        });
    };


    // CORE IMGUI RENDER LOOP =================================================================================
    while (!glfwWindowShouldClose(host_window)) {
        // FOR FRAME CAPPING : Mark the exact time the frame started 
        auto frameStartTime = std::chrono::high_resolution_clock::now();
        STAGE::StartRenderLoop();

        ImGui::ShowDemoWindow();
        // -----------------------------------------------------------
        // Unified rendering of all window elements
        // -----------------------------------------------------------
        win_manager.RenderAll();
        G_resultpool->execute_results();
        // -----------------------------------------------------------
        // Finalize geometry and push to the GPU
        // -----------------------------------------------------------
        STAGE::EndRenderLoop(host_window, clear_color);
        // FOR FRAME CAPPING : Calculate how long the CPU took to draw the UI and do the math
        auto frameEndTime = std::chrono::high_resolution_clock::now();
        auto timeSpentComputing = frameEndTime - frameStartTime;
        if (timeSpentComputing < targetFrameTime) {std::this_thread::sleep_for(targetFrameTime - timeSpentComputing);}
    }
    // ========================================================================================================


    // DEALLOCATE EVERYTHING AND EXIT =========================================================================
    active_sandbox.save_whole_sandbox(); // Save current data before exit
    STAGE::ShutdownApplication(host_window);
    G_threadpool.reset(); // Destroy all threads
    G_resultpool.reset(); // Destroy all results
    // ========================================================================================================
    std::cout << "End of Program : Thank you and see you later." << std::endl;
    return 0;
}