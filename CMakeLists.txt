cmake_minimum_required(VERSION 3.24)

project(D3D11-Demystify)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

if(NOT WIN32)
    message(FATAL_ERROR "This project targets Windows / DirectX 11 only.")
endif()

file(GLOB_RECURSE SRC_FILES Code/*.cpp Code/*.hpp)

# WIN32 tells the linker to build a GUI app (uses WinMain, not main)
add_executable(${PROJECT_NAME} WIN32)

target_sources(${PROJECT_NAME} PRIVATE ${SRC_FILES})

# D3D11       - Direct3D 11 runtime API (Devices, Contexts, Resources)
# DXGI        - DirectX Graphics Infrastructure (Swapchains, Adapters, Output)
# D3DCOMPILER - HLSL Shader Compiler
target_link_libraries(${PROJECT_NAME} PRIVATE d3d11 dxgi d3dcompiler winmm)

# When you hit F5 this makes the working directory where .EXE lives
set_property(TARGET ${PROJECT_NAME} PROPERTY VS_DEBUGGER_WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}")

# Tiny OBJ
target_include_directories(${PROJECT_NAME} PRIVATE "${CMAKE_SOURCE_DIR}/External/TinyObjLoader")
# True type
target_include_directories(${PROJECT_NAME} PRIVATE "${CMAKE_SOURCE_DIR}/External/TrueType")

# Compile definitions
target_compile_definitions(${PROJECT_NAME} PRIVATE NOMINMAX)
