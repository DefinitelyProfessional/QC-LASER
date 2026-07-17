#pragma once

#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"

#include <GLFW/glfw3.h>

// Expose GLFW's native Win32 functions
#define GLFW_EXPOSE_NATIVE_WIN32
#include <GLFW/glfw3native.h>

#include <windows.h>
#include <filesystem>
#include <iostream>
#include <string>

// Include the resource ID definition
#include "icon-resource.hpp"

namespace STAGE {

    // Internal robust GLFW error callback ====================================================================
    inline void glfw_error_callback(int error, const char* description) {
        std::cerr << "GLFW Error " << error << ": " << description << std::endl;
    }

    // Encapsulates the complete boot sequence for GLFW, OpenGL, and ImGui ====================================
    inline GLFWwindow* InitializeApplication(int width, int height, const char* title) {
        // GLFW Setup
        glfwSetErrorCallback(glfw_error_callback);
        if (!glfwInit()) return nullptr;

        glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
        glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
        glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
        glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);

        // Window Creation
        GLFWwindow* window = glfwCreateWindow(width, height, title, nullptr, nullptr);
        if (!window) {
            glfwTerminate();
            return nullptr;
        }
        glfwMakeContextCurrent(window);
        glfwSwapInterval(1); // Enable VSync

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
        wchar_t buffer[MAX_PATH];
        GetModuleFileNameW(NULL, buffer, MAX_PATH);
        std::filesystem::path exeDir = std::filesystem::path(buffer).parent_path();
        std::filesystem::path iniPath = exeDir / "imgui.ini";
        static std::string persistentIniPath = iniPath.string();
        io.IniFilename = persistentIniPath.c_str();
        
        return window;
    }

    // Encapsulates the complete memory cleanup sequence ======================================================
    inline void ShutdownApplication(GLFWwindow* window) {
        ImGui_ImplOpenGL3_Shutdown();
        ImGui_ImplGlfw_Shutdown();
        ImGui::DestroyContext();

        if (window) {
            glfwDestroyWindow(window);
        }
        glfwTerminate();
    }

}