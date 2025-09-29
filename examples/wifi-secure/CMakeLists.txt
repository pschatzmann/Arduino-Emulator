cmake_minimum_required(VERSION 3.11...3.19)

project(wifi-secure)
set(CMAKE_CXX_STANDARD 11)
set(DCMAKE_CXX_FLAGS "-Werror")

add_executable(wifi-secure wifi-secure.cpp)

target_compile_definitions(wifi-secure PUBLIC -DUSE_HTTPS)
target_link_libraries(wifi-secure arduino_emulator)