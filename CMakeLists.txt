cmake_minimum_required(VERSION 3.24)

project(cpp_token_counting CXX C)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(BUILD_SHARED_LIBS OFF CACHE BOOL "Build libraries statically" FORCE)
set(RE2_BUILD_TESTING OFF CACHE BOOL "Disable RE2 testing" FORCE)
set(BUILD_SHARED_LIBS OFF CACHE BOOL "Build static libraries" FORCE)

include(FetchContent)
set(FETCHCONTENT_QUIET OFF)

# Готовый парсер для сравнения
FetchContent_Declare(
    tree-sitter
    GIT_REPOSITORY https://github.com/tree-sitter/tree-sitter.git
    GIT_TAG        v0.25.2
)
FetchContent_Declare(
    tree-sitter-cpp
    GIT_REPOSITORY https://github.com/tree-sitter/tree-sitter-cpp.git
    GIT_TAG        v0.23.4
)

FetchContent_Declare(
    portable-file-dialogs
    GIT_REPOSITORY https://github.com/samhocevar/portable-file-dialogs.git
    GIT_TAG        0.1.0
    SOURCE_SUBDIR  "IGNORE_INTERNAL_CMAKE"
)

FetchContent_Declare(
    raylib
    GIT_REPOSITORY https://github.com/raysan5/raylib.git
    GIT_TAG        6.0
)

FetchContent_Declare(
    imgui
    GIT_REPOSITORY https://github.com/ocornut/imgui.git
    GIT_TAG        master
)

FetchContent_Declare(
    rlimgui
    GIT_REPOSITORY https://github.com/raylib-extras/rlImGui.git
    GIT_TAG        main
)

FetchContent_Declare(
    re2
    GIT_REPOSITORY https://github.com/google/re2.git
    GIT_TAG        2025-11-05
)

FetchContent_MakeAvailable(
    portable-file-dialogs
    raylib
    imgui
    rlimgui
    re2
)

FetchContent_Populate(tree-sitter)
FetchContent_Populate(tree-sitter-cpp)

add_library(tree-sitter STATIC "${tree-sitter_SOURCE_DIR}/lib/src/lib.c")
target_include_directories(tree-sitter PUBLIC
    "${tree-sitter_SOURCE_DIR}/lib/include"
    "${tree-sitter_SOURCE_DIR}/lib/src"
)

add_library(tree-sitter-cpp STATIC
    "${tree-sitter-cpp_SOURCE_DIR}/src/parser.c"
    "${tree-sitter-cpp_SOURCE_DIR}/src/scanner.c"
)
target_include_directories(tree-sitter-cpp PUBLIC "${tree-sitter-cpp_SOURCE_DIR}/src")
target_link_libraries(tree-sitter-cpp PRIVATE tree-sitter)

add_library(portable-file-dialogs INTERFACE)
target_include_directories(portable-file-dialogs INTERFACE "${portable-file-dialogs_SOURCE_DIR}")

add_library(imgui STATIC
    "${imgui_SOURCE_DIR}/imgui.cpp"
    "${imgui_SOURCE_DIR}/imgui_demo.cpp"
    "${imgui_SOURCE_DIR}/imgui_draw.cpp"
    "${imgui_SOURCE_DIR}/imgui_tables.cpp"
    "${imgui_SOURCE_DIR}/imgui_widgets.cpp"
    "${imgui_SOURCE_DIR}/misc/cpp/imgui_stdlib.cpp"
)
target_include_directories(imgui PUBLIC "${imgui_SOURCE_DIR}" "${imgui_SOURCE_DIR}/backends")
target_link_libraries(imgui PUBLIC raylib)

add_library(rlimgui_lib STATIC "${rlimgui_SOURCE_DIR}/rlImGui.cpp")
target_include_directories(rlimgui_lib PUBLIC "${rlimgui_SOURCE_DIR}" "${imgui_SOURCE_DIR}")
target_link_libraries(rlimgui_lib PUBLIC raylib imgui)

set(MAIN_SOURCES
    main.cpp
    framework.cpp
    framework.h
    save_dialog.cpp
    save_dialog.h
    parser.cpp
    parser.h
)

add_executable(token_counter ${MAIN_SOURCES})

target_link_libraries(token_counter PRIVATE
    rlimgui_lib
    imgui
    re2::re2
    raylib
    portable-file-dialogs
)

# делаем доступными пути к библотекам
include_directories("${tree-sitter_SOURCE_DIR}/lib/include")
include_directories("${CMAKE_CURRENT_SOURCE_DIR}")

set(TESTER_SOURCES
    test/tester.cpp
    parser.cpp
    parser.h
)

add_executable(tester ${TESTER_SOURCES})

target_link_libraries(tester PRIVATE
    re2::re2
    tree-sitter-cpp
    tree-sitter
)
