#include "ui-utilities/stage-utilities.hpp"

// Resource ID definition
#include "ui-utilities/icon-resource.hpp"

// Required thirdparties
#include "imgui_impl_opengl3.h"
#include "imgui_impl_glfw.h"
#include <imgui.h>

#include <GLFW/glfw3.h>
#include <GL/gl.h>

#include <H5Exception.h>
#include <H5Cpp.h>
#include <ostream>

// Expose GLFW's native Win32 functions
#define GLFW_EXPOSE_NATIVE_WIN32
#include <GLFW/glfw3native.h>


#include <libloaderapi.h>
#include <minwindef.h>
#include <windows.h>
#include <windef.h>
#include <timeapi.h>

#include <filesystem>
#include <iostream>
#include <cstddef>
#include <string>


namespace fs = std::filesystem;

namespace STAGE {
    // Public
    void WindowManager::RenderAll() {
        for (const auto& window : windows_registry) {
            if (window->IsOpen()) {window->Render();}
        }
    }

    // Public
    // UIWindow* WindowManager::FindWindow(const std::string& name) {
    //     for (const auto& window : windows_registry) {
    //         if (window->GetName() == name) {return window.get();}
    //     }
    //     return nullptr;
    // }

    void InitializeDirectories(std::filesystem::path& ROOT, 
        std::filesystem::path& SAVED_DATA_DIR, std::filesystem::path& ASSETS_DIR) {
        // Setup HDF5 to enable manual control over HDF5 error handling
        H5::Exception::dontPrint();
        // Reliable way to obtain the executable's root directory for Windows11
        wchar_t buffer[MAX_PATH]; GetModuleFileNameW(NULL, buffer, MAX_PATH);
        ROOT = fs::path(buffer).parent_path();

        // Ensure saved-data directory exists
        SAVED_DATA_DIR = ROOT / "saved-data";
        if (!fs::exists(SAVED_DATA_DIR)) {fs::create_directories(SAVED_DATA_DIR);}
        else if (!fs::is_directory(SAVED_DATA_DIR)) {// && fs::exist(SAVED_DATA_DIR)
            fs::remove(SAVED_DATA_DIR); fs::create_directories(SAVED_DATA_DIR); // Remove rogue n create directory.
        }
        // Ensure assets directory exists
        ASSETS_DIR = ROOT / "assets";
        if (!fs::exists(ASSETS_DIR)) {fs::create_directories(ASSETS_DIR);}
        else if (!fs::is_directory(ASSETS_DIR)) {// && fs::exist(SAVED_DATA_DIR)
            fs::remove(ASSETS_DIR); fs::create_directories(ASSETS_DIR); // Remove rogue n create directory.
        }
    }

    // Internal GLFW error callback
    static void glfw_error_callback(int error, const char* description) {
        std::cerr << "GLFW Error " << error << ": " << description << std::endl;
    }

    GLFWwindow* InitializeApplication(int width, int height, const char* TITLE, const fs::path& ROOT) {
        // Enable 1ms OS timer resolution for accurate sleep
        // timeBeginPeriod(1);
        // GLFW Setup
        glfwSetErrorCallback(glfw_error_callback);
        if (!glfwInit()) {
            // timeEndPeriod(1);
            return nullptr;
        }

        glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
        glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
        glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
        glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);

        // Window Creation
        GLFWwindow* window = glfwCreateWindow(width, height, TITLE, nullptr, nullptr);
        if (!window) {
            glfwTerminate();
            // timeEndPeriod(1);
            return nullptr;
        }
        glfwMakeContextCurrent(window);
        glfwSwapInterval(1); // Enable VSync to cap FPS

        // Apply the native icon
        HWND hwnd = glfwGetWin32Window(window);
        if (hwnd) {
            HINSTANCE hInst = GetModuleHandle(NULL);
            // Load the large icon for Taskbar / Alt+Tab
            HICON hIconBig = (HICON)LoadImage(hInst, MAKEINTRESOURCE(IDI_APP_ICON), IMAGE_ICON, 
                GetSystemMetrics(SM_CXICON), GetSystemMetrics(SM_CYICON), 0);
            // Load the small icon for the Window Title Bar
            HICON hIconSmall = (HICON)LoadImage(hInst, MAKEINTRESOURCE(IDI_APP_ICON), IMAGE_ICON, 
                GetSystemMetrics(SM_CXSMICON), GetSystemMetrics(SM_CYSMICON), 0);
            // Send messages to the Win32 window to update the icons
            if (hIconBig) SendMessage(hwnd, WM_SETICON, ICON_BIG, (LPARAM)hIconBig);
            if (hIconSmall) SendMessage(hwnd, WM_SETICON, ICON_SMALL, (LPARAM)hIconSmall);
        }

        // ImGui Context Setup
        IMGUI_CHECKVERSION();
        ImGui::CreateContext();
        ImGuiIO& io = ImGui::GetIO(); (void)io;
        io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
        io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;
        ImGui::StyleColorsDark();

        // ImGui Backend Binding
        ImGui_ImplGlfw_InitForOpenGL(window, true);
        ImGui_ImplOpenGL3_Init("#version 330");

        // Extract and locate the imgui.ini to portable dist directory
        fs::path iniPath = ROOT / "imgui.ini";
        static std::string persistentIniPath = iniPath.string();
        io.IniFilename = persistentIniPath.c_str();
        
        return window;
    }

    void StartRenderLoop() {
        // glfwPollEvents();
        glfwWaitEventsTimeout(1.0 / 60.0);
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();
    }

    void EndRenderLoop(GLFWwindow* window, const ImVec4& clear_color) {
        ImGui::Render();
        int display_w, display_h;
        glfwGetFramebufferSize(window, &display_w, &display_h);
        glViewport(0, 0, display_w, display_h);
        glClearColor(clear_color.x * clear_color.w, clear_color.y * clear_color.w, clear_color.z * clear_color.w, clear_color.w);
        glClear(GL_COLOR_BUFFER_BIT);
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
        glfwSwapBuffers(window);
    }

    void ShutdownApplication(GLFWwindow* window) {
        ImGui_ImplOpenGL3_Shutdown();
        ImGui_ImplGlfw_Shutdown();
        ImGui::DestroyContext();

        if (window) {glfwDestroyWindow(window);}
        glfwTerminate();
        // Reset to Windows default timer resolution
        // timeEndPeriod(1);
    }
}