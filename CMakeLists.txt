# >>> Configure debug tree in "build_debug"
# cmake -S . -B build_Debug -G "Ninja" -DCMAKE_BUILD_TYPE=Debug
# cmake --build build_Debug
# >>> Configure release tree in "build_release"
# cmake -S . -B build_Release -G "Ninja" -DCMAKE_BUILD_TYPE=Release
# cmake --build build_Release


cmake_minimum_required(VERSION 3.20)
project(QC-LASER CXX RC)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Force CMake to output compilation paths for VS Code IntelliSense
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# ---------------------------------------------------------
# LINK-TIME OPTIMIZATION (LTO / IPO)
# ---------------------------------------------------------
include(CheckIPOSupported)
check_ipo_supported(RESULT lto_supported OUTPUT error)
if(lto_supported)
    set(CMAKE_INTERPROCEDURAL_OPTIMIZATION_RELEASE ON)
    message(STATUS "Link-Time Optimization (LTO): Enabled for Release builds")
else()
    message(STATUS "Link-Time Optimization (LTO): Not supported (${error})")
endif()

# ---------------------------------------------------------
# PORTABLE BUNDLE CONFIGURATION
# ---------------------------------------------------------
set(BUNDLE_DIR "${CMAKE_BINARY_DIR}/QC-LASER-Portable-Distribution")
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${BUNDLE_DIR}")
file(MAKE_DIRECTORY "${BUNDLE_DIR}/assets")
file(MAKE_DIRECTORY "${BUNDLE_DIR}/saved-data")
# ---------------------------------------------------------

# Locate system graphics dependencies via MSYS2 pacman
find_package(glfw3 CONFIG REQUIRED) 
find_package(SQLite3 REQUIRED)
find_package(OpenGL REQUIRED)

# Source mappings
set(APP_SOURCES
    src/main.cpp
    src/app.rc
    src/headers/math-core/function-definitions.hpp
    src/headers/math-core/variable-definitions.hpp
    src/headers/ui-utilities/general-utilities.hpp
    src/headers/ui-utilities/sandbox-manager.hpp
    src/headers/ui-utilities/stage-utilities.hpp
    src/headers/ui-utilities/icon-resource.hpp
    src/headers/storage-utilities/storage-utilities.hpp
)

set(THIRDPARTY_SOURCES
    thirdparty/imgui/imgui.cpp
    thirdparty/imgui/imgui_demo.cpp
    thirdparty/imgui/imgui_draw.cpp
    thirdparty/imgui/imgui_tables.cpp
    thirdparty/imgui/imgui_widgets.cpp
    thirdparty/imgui/backends/imgui_impl_glfw.cpp
    thirdparty/imgui/backends/imgui_impl_opengl3.cpp
    thirdparty/sqlite3/sqlite3.c
)

# Construct executable
add_executable(${PROJECT_NAME} ${APP_SOURCES} ${THIRDPARTY_SOURCES})

# ---------------------------------------------------------
# COMPILER-SPECIFIC OPTIMIZATIONS (GCC / Clang / MinGW)
# ---------------------------------------------------------
if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    target_compile_options(${PROJECT_NAME} PRIVATE
        # DEBUG FLAGS: Keep flags safe, generate full debug info, no optimization
        $<$<CONFIG:Debug>:-g -O0 -Wall -Wextra>

        # RELEASE FLAGS: Max stable optimizations + Parallel LTO
        $<$<CONFIG:Release>:-O3 -march=native -mfma -ffast-math -flto=8>
    )

    # Force the linker to use all CPU cores during the LTO phase
    target_link_options(${PROJECT_NAME} PRIVATE
        $<$<CONFIG:Release>:-flto=8>
    )
elseif(MSVC)
    # If you ever switch to native Visual Studio compiler
    target_compile_options(${PROJECT_NAME} PRIVATE
        $<$<CONFIG:Debug>:/Zi /Od>
        $<$<CONFIG:Release>:/O2 /Oi /Ot /arch:AVX2>
    )
endif()

# Map header search paths globally
target_include_directories(${PROJECT_NAME} PRIVATE
    src
    src/headers
    thirdparty/imgui
    thirdparty/imgui/backends
    thirdparty/sqlite3
)

# Link against the necessary graphics and system packages
target_link_libraries(${PROJECT_NAME} PRIVATE
    glfw
    OpenGL::GL
    SQLite3::SQLite3
)

if(MINGW)
    target_link_libraries(${PROJECT_NAME} PRIVATE imm32 gdi32)
endif()