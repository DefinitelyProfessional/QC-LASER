#include "ui-utilities/stage-utilities.hpp"

// Resource ID definition
#include "implot3d.h"
#include "ui-utilities/icon-resource.hpp"

// Initialize teh global font
#include "general-ui-utilities.hpp"

// Required thirdparties
#include "imgui_impl_opengl3.h"
#include "imgui_impl_glfw.h"
#include "imgui.h"
#include "implot.h"

// MicroTex initialization
// #include "baked_clm1.h"  // Exposes: lm_math_clm, lm_math_clm_LEN
// #include "baked_font.h"  // Exposes: lm_math_otf, lm_math_otf_LEN
// #include "imgui_microtex.h"
// #include "microtex.h"

#include <GLFW/glfw3.h>
#include <GL/gl.h>

#include <H5Exception.h>
#include <H5Cpp.h>

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
        if (window->is_open) {window->Render();}
    }
}

// Public
// UIWindow* WindowManager::FindWindow(const std::string& name) {
//     for (const auto& window : windows_registry) {
//         if (window->GetName() == name) {return window.get();}
//     }
//     return nullptr;
// }

void InitializeDirectories(fs::path& ROOT_DIR, fs::path& SAVED_DATA_DIR, fs::path& ASSETS_DIR) {
    // Setup HDF5 to enable manual control over HDF5 error handling
    H5::Exception::dontPrint();
    // Reliable way to obtain the executable's root directory for Windows11
    wchar_t buffer[MAX_PATH]; GetModuleFileNameW(NULL, buffer, MAX_PATH);
    ROOT_DIR = fs::path(buffer).parent_path();

    auto ensure_dir = [](fs::path& dir) {
        if (!fs::exists(dir)) {fs::create_directories(dir);}
        else if (!fs::is_directory(dir)) {// && fs::exist(SAVED_DATA_DIR)
            fs::remove(dir); fs::create_directories(dir); // Remove rogue n create directory.
        }
    };

    // Ensure saved-data directory exists
    SAVED_DATA_DIR = ROOT_DIR / "saved-data";
    ensure_dir(SAVED_DATA_DIR);
    // Ensure assets directory exists
    ASSETS_DIR = ROOT_DIR / "assets";
    ensure_dir(ASSETS_DIR);
    // Ensure assets/fonts directory exists
    fs::path FONTS_DIR = ROOT_DIR / "assets" / "fonts";
    ensure_dir(FONTS_DIR);
    // Ensure asset/fonts size options
    fs::path FONTS_small  = FONTS_DIR / "small";  ensure_dir(FONTS_small);
    fs::path FONTS_medium = FONTS_DIR / "medium"; ensure_dir(FONTS_medium);
    fs::path FONTS_large  = FONTS_DIR / "large";  ensure_dir(FONTS_large);
}

    
// Helper for font initialization.
// Scans directory and returns the path to the first .ttf or .otf file found
static std::string GetFirstTTF(const fs::path& dirPath) {
    if (!fs::exists(dirPath) || !fs::is_directory(dirPath)) {
        return "";
    }

    for (const auto& entry : fs::directory_iterator(dirPath)) {
        if (entry.is_regular_file()) {
            std::string ext = entry.path().extension().string();
            if (ext == ".ttf" || ext == ".otf") {return entry.path().string();}
        }
    }
    return ""; // No font file found in directory
}

// Internal GLFW error callback
static void glfw_error_callback(int error, const char* description) {
    std::cerr << "[GLFW Error] " << error << ": " << description << std::endl;
}

SimpleFonts G_Fonts; // To be initialized here
GLFWwindow* InitializeUI(int width, int height, const char* TITLE, const fs::path& ROOT) {
    glfwSetErrorCallback(glfw_error_callback);
    if (!glfwInit()) {return nullptr;}

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);

    // Window Creation
    GLFWwindow* window = glfwCreateWindow(width, height, TITLE, nullptr, nullptr);
    if (!window) {
        glfwTerminate();
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
    ImPlot::CreateContext(); // Create ImPlot context AFTER ImGui
    ImPlot3D::CreateContext(); // Create ImPlot3D context AFTER ImGui

    // ImGui IO config duh
    ImGuiIO& io = ImGui::GetIO();
    io.MouseWheel = 1.0f;
    io.ConfigWindowsMoveFromTitleBarOnly = true;
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;
    ImGui::StyleColorsDark();


    // Initialize Global Fonts
    ImFontConfig fontConfig;
    fontConfig.PixelSnapH = true; // Snap glyph rendering to whole pixel boundaries for max crisp
    fontConfig.OversampleH = 2; // 2x horizontal oversampling for sharp edges without heavy blur
    fontConfig.OversampleV = 1; // 1x vertical oversampling to keep horizontal lines pixel-perfect
    fontConfig.RasterizerMultiply = 1.15f; // Slightly boosts font contrast for dark theme
    io.Fonts->Clear();
    // Load ImGui's built-in font to guarantee atlas is never empty
    G_Fonts.Fallback = io.Fonts->AddFontDefault();
    // Discover first font file in each directory
    std::string smallPath  = GetFirstTTF("assets/fonts/small");
    std::string mediumPath = GetFirstTTF("assets/fonts/medium");
    std::string largePath  = GetFirstTTF("assets/fonts/large");
    auto LoadFontOrDefault = [&io](const std::string& fontPath, float pixelSize) -> ImFont* {
        if (!fontPath.empty()) {
            ImFont* loadedFont = io.Fonts->AddFontFromFileTTF(fontPath.c_str(), pixelSize);
            if (loadedFont != nullptr) {
                std::cout << "[FontInit] Loaded: " << fontPath << " (" << pixelSize << "px)\n";
                return loadedFont;
            }
        }
        return G_Fonts.Fallback; // Use default if no file present or loading failed
    };
    // Load font tiers with target font sizes
    G_Fonts.Small  = LoadFontOrDefault(smallPath, 13.0f);
    G_Fonts.Medium = LoadFontOrDefault(mediumPath, 18.0f);
    G_Fonts.Large  = LoadFontOrDefault(largePath, 20.0f);
    // Setup fallbacks to imgui's default
    if (!G_Fonts.Medium) G_Fonts.Medium = G_Fonts.Fallback;
    if (!G_Fonts.Small)  G_Fonts.Small  = G_Fonts.Medium;
    if (!G_Fonts.Large)  G_Fonts.Large  = G_Fonts.Medium;
    // Set the medium size as the default global font
    io.FontDefault = G_Fonts.Medium;
    // Automatically scale all ImGui padding/spacing relative
    // from the standard 13px baseline to the Medium default 
    if (io.FontDefault != G_Fonts.Fallback) {
        ImGui::GetStyle().ScaleAllSizes(18.0f / 13.0f);
    }
    // Texture atlas will be built immidiately the line after this
    // io.Fonts->Build();


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

void ShutdownUI(GLFWwindow* window) {
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();

    ImPlot3D::DestroyContext(); // Destroy ImPlot3D context BEFORE ImGui
    ImPlot::DestroyContext(); // Destroy ImPlot context BEFORE ImGui
    ImGui::DestroyContext();

    if (window) {glfwDestroyWindow(window);}
    glfwTerminate();
}
}