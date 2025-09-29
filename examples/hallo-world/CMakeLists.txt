cmake_minimum_required(VERSION 3.11...3.19)

project(hallo-world)
set(CMAKE_CXX_STANDARD 11)
set(DCMAKE_CXX_FLAGS "-Werror")

add_executable(hallo-world hallo-world.cpp)

target_link_libraries(hallo-world arduino_emulator)