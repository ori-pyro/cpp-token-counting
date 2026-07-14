cmake_minimum_required(VERSION 3.24)

project(cpp_token_counting CXX)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(RE2_BUILD_TESTING OFF CACHE BOOL "Disable RE2 testing" FORCE)

set(CPR_BUILD_TESTS OFF CACHE BOOL "Disable CPR tests" FORCE)
set(CPR_BUILD_TESTS_SSL OFF CACHE BOOL "Disable CPR SSL tests" FORCE)
set(CPR_BUILD_TESTS_PROXY OFF CACHE BOOL "Disable CPR proxy tests" FORCE)

set(BUILD_TESTING OFF CACHE BOOL "Disable curl testing" FORCE)

set(CPR_CURL_USE_LIBPSL OFF CACHE BOOL "Disable curl libpsl" FORCE)
set(CPR_USE_SYSTEM_LIB_PSL OFF CACHE BOOL "Disable libpsl" FORCE)

set(BUILD_SHARED_LIBS OFF CACHE BOOL "Build libraries statically" FORCE)

set(GLFW_BUILD_WAYLAND ON CACHE BOOL "" FORCE)
set(GLFW_BUILD_X11 OFF CACHE BOOL "" FORCE)

include(FetchContent)
set(FETCHCONTENT_QUIET OFF)

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

FetchContent_Declare(
    json
    GIT_REPOSITORY https://github.com/nlohmann/json.git
    GIT_TAG        v3.12.0
)

FetchContent_Declare(
    cpr
    GIT_REPOSITORY https://github.com/libcpr/cpr.git
    GIT_TAG        1.14.2
)

FetchContent_MakeAvailable(
    portable-file-dialogs
    raylib
    imgui
    rlimgui
    re2
    json
    cpr
)

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

add_library(rlimgui_lib STATIC "${rlimgui_SOURCE_DIR}/rlImGui.cpp" )
target_include_directories(rlimgui_lib PUBLIC "${rlimgui_SOURCE_DIR}" "${imgui_SOURCE_DIR}")
target_link_libraries(rlimgui_lib PUBLIC raylib imgui)

set(MAIN_SOURCES
    main.cpp
    framework.cpp
    framework.h
    file_manager.cpp
    file_manager.h
    structures.h
    parser.cpp
    parser.h
)

add_executable(token_counter ${MAIN_SOURCES} )
if(NOT DEFINED GITHUB_TOKEN)
  message(WARNING "GITHUB_TOKEN не задан. Передай его через -DGITHUB_TOKEN=...")
endif()
target_compile_definitions(token_counter PRIVATE GITHUB_TOKEN="${GITHUB_TOKEN}")
set_source_files_properties(main.cpp PROPERTIES
    COMPILE_DEFINITIONS "WIN32_LEAN_AND_MEAN;NOMINMAX"
)
target_link_libraries(token_counter PRIVATE
    rlimgui_lib
    imgui
    re2::re2
    raylib
    portable-file-dialogs
    nlohmann_json::nlohmann_json
    cpr::cpr
)

if(UNIX AND NOT APPLE)
  add_executable(token_counter_linux ${MAIN_SOURCES})
  if(NOT DEFINED GITHUB_TOKEN)
    message(WARNING "GITHUB_TOKEN не задан. Передай его через -DGITHUB_TOKEN=...")
  endif()
  target_compile_definitions(token_counter_linux PRIVATE GITHUB_TOKEN="${GITHUB_TOKEN}")
  target_link_libraries(token_counter_linux PRIVATE
        rlimgui_lib
        imgui
        re2::re2
        raylib
        portable-file-dialogs
        nlohmann_json::nlohmann_json
        cpr::cpr
    )
  set_target_properties(token_counter_linux PROPERTIES
        RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}"
        OUTPUT_NAME "token_counter_linux"
    )
endif()
