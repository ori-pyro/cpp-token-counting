cmake_minimum_required(VERSION 3.24)

project(cpp_token_counting CXX)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_CXX_STANDARD 23)

include(FetchContent)
set(FETCHCONTENT_QUIET OFF)

# Fetch pfd
add_library(portable-file-dialogs INTERFACE)
FetchContent_Declare(
    portable-file-dialogs
    GIT_REPOSITORY https://github.com/samhocevar/portable-file-dialogs.git
    GIT_TAG        0.1.0
    SOURCE_SUBDIR  "IGNORE_INTERNAL_CMAKE"
)
FetchContent_MakeAvailable(portable-file-dialogs)

target_include_directories(portable-file-dialogs INTERFACE
    ${portable-file-dialogs_SOURCE_DIR}
)

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
    GIT_TAG master
)
FetchContent_MakeAvailable(imgui)

# Fetch rlImGui
FetchContent_Declare(
    rlimgui
    GIT_REPOSITORY https://github.com/raylib-extras/rlImGui.git
    GIT_TAG main
)
FetchContent_MakeAvailable(rlimgui)

# Create ImGui library (core only, without backend)
add_library(imgui STATIC
    ${imgui_SOURCE_DIR}/imgui.cpp
    ${imgui_SOURCE_DIR}/imgui_demo.cpp
    ${imgui_SOURCE_DIR}/imgui_draw.cpp
    ${imgui_SOURCE_DIR}/imgui_tables.cpp
    ${imgui_SOURCE_DIR}/imgui_widgets.cpp
    ${imgui_SOURCE_DIR}/misc/cpp/imgui_stdlib.cpp
)
target_include_directories(imgui PUBLIC ${imgui_SOURCE_DIR} ${imgui_SOURCE_DIR}/backends)
target_link_libraries(imgui PUBLIC raylib)

# Create rlImGui library (raylib + ImGui integration)
add_library(rlimgui_lib STATIC
    ${rlimgui_SOURCE_DIR}/rlImGui.cpp
)
target_include_directories(rlimgui_lib PUBLIC ${rlimgui_SOURCE_DIR} ${imgui_SOURCE_DIR})
target_link_libraries(rlimgui_lib PUBLIC raylib imgui)

# Fetch re2
set(RE2_BUILD_TESTING OFF CACHE BOOL "")
FetchContent_Declare(
    re2
    GIT_REPOSITORY https://github.com/google/re2.git
    GIT_TAG 2025-11-05
)
FetchContent_MakeAvailable(re2)

# Main executable
file(GLOB SOURCES "*.cpp")

add_executable(token_counter ${SOURCES})
# add_executable(token_counter WIN32 ${SOURCES})

target_include_directories(token_counter PRIVATE
    ${imgui_SOURCE_DIR}
    ${rlimgui_SOURCE_DIR}
)
target_link_libraries(token_counter PRIVATE
    rlimgui_lib
    imgui
    re2::re2
    raylib
    portable-file-dialogs
)
