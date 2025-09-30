cmake_minimum_required(VERSION 3.11...3.19)

project(blink)
set(CMAKE_CXX_STANDARD 11)
set(DCMAKE_CXX_FLAGS "-Werror")

set_source_files_properties(blink.ino PROPERTIES LANGUAGE CXX)
add_executable(blink blink.ino)
target_compile_options(blink PRIVATE -x c++)
target_compile_definitions(blink PUBLIC -DUSE_RPI)
target_link_libraries(blink arduino_emulator)