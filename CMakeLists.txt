cmake_minimum_required(VERSION 3.20)

project(cpp_token_counting CXX)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_CXX_STANDARD 23)

include(FetchContent)
set(FETCHCONTENT_QUIET OFF)

# Fetch raylib
FetchContent_Declare(
    raylib
    GIT_REPOSITORY https://github.com/raysan5/raylib.git
    GIT_TAG 6.0
)
set(BUILD_SHARED_LIBS OFF)
FetchContent_MakeAvailable(raylib)

# Fetch ImGui
FetchContent_Declare(
    imgui
    GIT_REPOSITORY https://github.com/ocornut/imgui.git
    GIT_TAG docking
)
FetchContent_MakeAvailable(imgui)

# Create ImGui library
add_library(imgui STATIC
    ${imgui_SOURCE_DIR}/imgui.cpp
    ${imgui_SOURCE_DIR}/imgui_demo.cpp
    ${imgui_SOURCE_DIR}/imgui_draw.cpp
    ${imgui_SOURCE_DIR}/imgui_tables.cpp
    ${imgui_SOURCE_DIR}/imgui_widgets.cpp
)
target_include_directories(imgui PUBLIC ${imgui_SOURCE_DIR})
target_link_libraries(imgui PUBLIC raylib)

# Fetch re2
set(RE2_BUILD_TESTING OFF CACHE BOOL "")
FetchContent_Declare(
    re2
    GIT_REPOSITORY https://github.com/google/re2.git
    GIT_TAG 2025-11-05
)
FetchContent_MakeAvailable(re2)

# Your executable
file(GLOB SOURCES "*.cpp")
add_executable(token_counter ${SOURCES})
target_link_libraries(token_counter PRIVATE
    imgui
    re2::re2
    raylib
)
