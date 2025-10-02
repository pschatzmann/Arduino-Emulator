cmake_minimum_required(VERSION 3.11...3.19)

project(blink-rpi)
set(CMAKE_CXX_STANDARD 11)
set(DCMAKE_CXX_FLAGS "-Werror")

set_source_files_properties(blink.ino PROPERTIES LANGUAGE CXX)
add_executable(blink-rpi blink.ino)
target_compile_options(blink-rpi PRIVATE -x c++)
target_compile_definitions(blink-rpi PUBLIC -DUSE_RPI)
target_link_libraries(blink-rpi arduino_emulator gpiod)
