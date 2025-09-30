cmake_minimum_required(VERSION 3.11...3.19)

project(hallo-world)
set(CMAKE_CXX_STANDARD 11)
set(DCMAKE_CXX_FLAGS "-Werror")

set_source_files_properties(hallo-world.ino PROPERTIES LANGUAGE CXX)
add_executable(hallo-world hallo-world.ino)
target_compile_options(hallo-world PRIVATE -x c++)
target_link_libraries(hallo-world arduino_emulator)
