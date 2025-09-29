cmake_minimum_required(VERSION 3.11...3.19)

project(blink)
set(CMAKE_CXX_STANDARD 11)
set(DCMAKE_CXX_FLAGS "-Werror")

add_executable(blink blink.cpp)

target_compile_definitions(blink PUBLIC -DUSE_RPI)
target_link_libraries(blink arduino_emulator)