cmake_minimum_required(VERSION 3.11...3.19)

project(i2c-rpi)
set(CMAKE_CXX_STANDARD 11)
set(DCMAKE_CXX_FLAGS "-Werror")

set_source_files_properties(i2c.ino PROPERTIES LANGUAGE CXX)
add_executable(i2c-rpi i2c.ino)
target_compile_options(i2c-rpi PRIVATE -x c++)
target_compile_definitions(i2c-rpi PUBLIC -DUSE_RPI)
target_link_libraries(i2c-rpi arduino_emulator gpiod)
