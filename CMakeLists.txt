cmake_minimum_required(VERSION 3.20)
project(cpp_token_counting CXX)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

set(CMAKE_CXX_STANDARD 23)

set(CMAKE_CXX_STANDARD 23)

find_package(re2 REQUIRED)

# Ищем все файлы .cpp в текущей папке
file(GLOB SOURCES "*.cpp")

add_executable(token_counter ${SOURCES})
target_link_libraries(token_counter PRIVATE re2::re2)
