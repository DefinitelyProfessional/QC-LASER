#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"
#include <GLFW/glfw3.h>
#include <iostream>

// Robust GLFW error callback to catch initialization or driver faults
static void glfw_error_callback(int error, const char* description) {
    std::cerr << "GLFW Error " << error << ": " << description << std::endl;
}

int main() {
    // 1. Initialize GLFW & Bind Error Callback
    glfwSetErrorCallback(glfw_error_callback);
    if (!glfwInit()) {
        return -1;
    }

    // 2. Configure OpenGL 3.3 Core Profile
    // This perfectly matches standard desktop utility requirements
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE); // Required for Mac compatibility, harmless on Windows

    // 3. Create the Main Application Window
    GLFWwindow* window = glfwCreateWindow(1280, 720, "Linear Algebra Engine", nullptr, nullptr);
    if (!window) {
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);
    
    // Enable VSync (Locks frame rate to your monitor's refresh rate to save CPU/GPU power)
    glfwSwapInterval(1); 

    // 4. Initialize the Dear ImGui Context
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;
    
    // Enable essential versatility features
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;       // Enable Keyboard Controls
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;        // Enable Gamepad Controls

    // 5. Setup ImGui Visual Style
    ImGui::StyleColorsDark();

    // 6. Initialize Platform and Renderer Backends
    // The "#version 330" explicitly matches our OpenGL 3.3 Core Profile request
    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init("#version 330");

    // Define a clear color for the background
    ImVec4 clear_color = ImVec4(0.12f, 0.12f, 0.14f, 1.00f);

    // 7. The Core Application Rendering Loop
    while (!glfwWindowShouldClose(window)) {
        // Poll and handle hardware events (inputs, window resize, etc.)
        glfwPollEvents();

        // Start a new ImGui frame
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();


        // -----------------------------------------------------------
        // YOUR ENGINE UI CODE BEGINS HERE
        // -----------------------------------------------------------
        
        ImGui::Begin("Matrix Operations");
        ImGui::Text("Welcome to the Linear Algebra Engine.");
        ImGui::Separator();
        
        if (ImGui::Button("Perform Identity Matrix Check")) {
            std::cout << "[Engine] Executing mathematical operation..." << std::endl;
        }
        ImGui::End();

        // Uncomment the line below to see everything Dear ImGui can do
        // ImGui::ShowDemoWindow();

        // -----------------------------------------------------------
        // YOUR ENGINE UI CODE ENDS HERE
        // -----------------------------------------------------------

        // 8. Render the Frame
        ImGui::Render();
        
        int display_w, display_h;
        glfwGetFramebufferSize(window, &display_w, &display_h);
        glViewport(0, 0, display_w, display_h);
        glClearColor(clear_color.x * clear_color.w, clear_color.y * clear_color.w, clear_color.z * clear_color.w, clear_color.w);
        glClear(GL_COLOR_BUFFER_BIT);
        
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

        // Swap front and back buffers
        glfwSwapBuffers(window);
    }

    // 10. Graceful Shutdown & Memory Cleanup
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();

    glfwDestroyWindow(window);
    glfwTerminate();

    return 0;
}