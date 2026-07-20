cmake_minimum_required(VERSION 3.20)
project(QC-LASER CXX RC)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Force CMake to output compile_commands.json for VS Code IntelliSense auto-configuration
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
# DEPENDENCY RESOLUTION
# ---------------------------------------------------------
find_package(glfw3 CONFIG REQUIRED) 
find_package(SQLite3 REQUIRED)
find_package(OpenGL REQUIRED)

# ---------------------------------------------------------
# SOURCE & HEADER MAPPINGS
# ---------------------------------------------------------
# Listing headers here tells VSCode that they belong to this target.
set(APP_SOURCES
    # Core Entry
    src/main.cpp
    src/app.rc

    # Math Core Module
    src/math-core/math-functions.cpp
    src/math-core/math-functions.hpp
    src/math-core/math-objects.cpp
    src/math-core/math-objects.hpp

    # Storage Utilities Module
    src/storage-utilities/storage-utilities.cpp
    src/storage-utilities/storage-utilities.hpp

    # UI Utilities Module
    src/ui-utilities/general-utilities.cpp
    src/ui-utilities/general-utilities.hpp
    src/ui-utilities/sandbox-manager.cpp
    src/ui-utilities/sandbox-manager.hpp
    src/ui-utilities/stage-utilities.cpp
    src/ui-utilities/stage-utilities.hpp
    src/ui-utilities/icon-resource.hpp
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

# ---------------------------------------------------------
# TARGET DEFINITIONS
# ---------------------------------------------------------
# Compile heavy, unchanging dependencies into a local static library
add_library(QC-LASER-ThirdParty STATIC ${THIRDPARTY_SOURCES})

target_include_directories(QC-LASER-ThirdParty PUBLIC
    thirdparty/imgui
    thirdparty/imgui/backends
    thirdparty/sqlite3
)

# Define primary application executable
add_executable(${PROJECT_NAME} ${APP_SOURCES})

# 'src' as the root include directory allows root-relative includes across modules
target_include_directories(${PROJECT_NAME} PRIVATE
    src
)

# ---------------------------------------------------------
# COMPILER-SPECIFIC OPTIMIZATIONS
# ---------------------------------------------------------
if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    target_compile_options(QC-LASER-ThirdParty PRIVATE $<$<CONFIG:Release>:-O3>)
    target_compile_options(${PROJECT_NAME} PRIVATE
        $<$<CONFIG:Debug>:-g -O0 -Wall -Wextra>
        $<$<CONFIG:Release>:-O3 -march=native -mfma -ffast-math>
    )

    target_link_options(${PROJECT_NAME} PRIVATE
        $<$<CONFIG:Release>:-flto=auto>
    )
elseif(MSVC)
    target_compile_options(QC-LASER-ThirdParty PRIVATE $<$<CONFIG:Release>:/O2>)
    target_compile_options(${PROJECT_NAME} PRIVATE
        $<$<CONFIG:Debug>:/Zi /Od>
        $<$<CONFIG:Release>:/O2 /Oi /Ot /arch:AVX2>
    )
endif()

# ---------------------------------------------------------
# PRECOMPILED HEADERS
# ---------------------------------------------------------
target_precompile_headers(${PROJECT_NAME} PRIVATE
    <vector>
    <string>
    <memory>
    <iostream>
    <GLFW/glfw3.h>
    "thirdparty/imgui/imgui.h"
)

# ---------------------------------------------------------
# LINKING & DISTRIBUTION
# ---------------------------------------------------------
target_link_libraries(${PROJECT_NAME} PRIVATE
    QC-LASER-ThirdParty
    glfw
    OpenGL::GL
    SQLite3::SQLite3
)

if(MINGW)
    target_link_libraries(${PROJECT_NAME} PRIVATE imm32 gdi32)
    target_link_options(${PROJECT_NAME} PRIVATE -static-libgcc -static-libstdc++ -static)
endif()