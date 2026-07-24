#include "storage-utilities/storage-utilities.hpp"
#include "math-core/math-functions.hpp"
#include "ui-utilities/stage-utilities.hpp"
#include "ui-utilities/sandbox-manager.hpp"

#include <GLFW/glfw3.h>
#include "imgui.h"

#define WIN32_LEAN_AND_MEAN // Trims down the massive Windows header to speed up compilation
#include <windows.h>

#include <filesystem>
#include <iostream>
#include <chrono>
#include <thread>
#include <string>

namespace fs = std::filesystem;

// Define target frame duration (1000 milliseconds / 60 FPS = 16.666 ms per frame)
const std::chrono::duration<double, std::milli> targetFrameTime(1000.0 / 60.0);

int main() {
    fs::path ROOT_DIR, SAVED_DATA_DIR, ASSETS_DIR;
    STAGE::InitializeDirectories(ROOT_DIR, SAVED_DATA_DIR, ASSETS_DIR);
    // IMGUI UI SUBSYSTEMS INITIALIZATION =====================================================================
    GLFWwindow* window = STAGE::InitializeApplication(750, 1000, "QC Linear Algebra Sandbox Engine R.", ROOT_DIR);
    if (!window) {std::cerr << "Fatal Error: Failed to initialize application stages." << std::endl; return -1;}
    ImVec4 clear_color = ImVec4(0.12f, 0.12f, 0.14f, 1.00f);
    // ========================================================================================================

    // Load the Default Sandbox Session being "MAIN_sandbox.h5" ===============================================
    SandboxSessionManager active_sandbox(SAVED_DATA_DIR, "MAIN_sandbox.h5");
    // WindowManager to handle unified rendering of all windows ===============================================
    STAGE::WindowManager win_manager;
    // Register windows and get their pointers for event listeners ============================================
    SandboxManagerWindow* sandbox_manager = 
        win_manager.RegisterWindow<SandboxManagerWindow>(SAVED_DATA_DIR, active_sandbox.get_active_filename());
    auto  switch_whole_sandbox = [&active_sandbox](std::string filename) {
        active_sandbox. switch_whole_sandbox(std::move(filename));
    };
    sandbox_manager->Event_OnSelectSandbox =  switch_whole_sandbox;
    sandbox_manager->Event_OnCreateSandbox =  switch_whole_sandbox;
    sandbox_manager->Event_OnSaveCurrentSandbox = [&active_sandbox]() {active_sandbox.save_whole_sandbox();};

    // CORE IMGUI RENDER LOOP =================================================================================
    while (!glfwWindowShouldClose(window)) {
        // FOR FRAME CAPPING : Mark the exact time the frame started 
        auto frameStartTime = std::chrono::high_resolution_clock::now();
        STAGE::StartRenderLoop();

        ImGui::ShowDemoWindow();
        // -----------------------------------------------------------
        // Unified rendering of all window elements
        // -----------------------------------------------------------
        win_manager.RenderAll();
        // -----------------------------------------------------------
        // Finalize geometry and push to the GPU
        // -----------------------------------------------------------
        STAGE::EndRenderLoop(window, clear_color);
        // FOR FRAME CAPPING : Calculate how long the CPU took to draw the UI and do the math
        auto frameEndTime = std::chrono::high_resolution_clock::now();
        auto timeSpentComputing = frameEndTime - frameStartTime;
        if (timeSpentComputing < targetFrameTime) {std::this_thread::sleep_for(targetFrameTime - timeSpentComputing);}
    }
    // ========================================================================================================

    // Deallocate everything and exit =========================================================================
    active_sandbox.save_whole_sandbox(); // Save current data before exit
    STAGE::ShutdownApplication(window);
    // ========================================================================================================
    std::cout << "End of Program : Thank you and see you later." << std::endl;
    return 0;
}