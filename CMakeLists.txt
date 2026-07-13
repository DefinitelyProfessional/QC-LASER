# cmake -S . -B build -G "Ninja"
# cmake --build build

cmake_minimum_required(VERSION 3.20)
project(QC-LASER CXX)

# Enforce modern C++ standards
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Force CMake to output compilation paths for VS Code IntelliSense
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# ---------------------------------------------------------
# PORTABLE BUNDLE CONFIGURATION
# ---------------------------------------------------------
set(BUNDLE_DIR "${CMAKE_BINARY_DIR}/QC-LASER-Portable-Distribution")
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${BUNDLE_DIR}")
file(MAKE_DIRECTORY "${BUNDLE_DIR}/assets")
file(MAKE_DIRECTORY "${BUNDLE_DIR}/saved-data")
# ---------------------------------------------------------

# Locate system graphics dependencies via MSYS2 pacman
# Using components search ensures compatibility with MSYS2's pathing
find_package(glfw3 CONFIG REQUIRED) 
find_package(OpenGL REQUIRED)

# Define your custom application source files
set(APP_SOURCES
    src/main.cpp
    src/headers/math-core/function-definitions.hpp
    src/headers/math-core/variable-definitions.hpp
    src/headers/ui-utilities/general-utilities.hpp
    src/headers/ui-utilities/stage-utilities.hpp
)

# Explicitly map the Dear ImGui sources
set(IMGUI_SOURCES
    thirdparty/imgui/imgui.cpp
    thirdparty/imgui/imgui_demo.cpp
    thirdparty/imgui/imgui_draw.cpp
    thirdparty/imgui/imgui_tables.cpp
    thirdparty/imgui/imgui_widgets.cpp
    thirdparty/imgui/backends/imgui_impl_glfw.cpp
    thirdparty/imgui/backends/imgui_impl_opengl3.cpp
)

# Construct the final executable target
add_executable(${PROJECT_NAME} ${APP_SOURCES} ${IMGUI_SOURCES})

# Map header search paths globally
target_include_directories(${PROJECT_NAME} PRIVATE
    src
    src/headers
    thirdparty/imgui
    thirdparty/imgui/backends
)

# Link against the necessary graphics and system packages
target_link_libraries(${PROJECT_NAME} PRIVATE
    glfw
    OpenGL::GL
)

# Apply specific system adjustments for MinGW/MSYS2 compilers on Windows
if(MINGW)
    target_link_libraries(${PROJECT_NAME} PRIVATE imm32 gdi32)
endif()