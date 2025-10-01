cmake_minimum_required(VERSION 3.11...3.19)

project(i2c)
set(CMAKE_CXX_STANDARD 11)
set(DCMAKE_CXX_FLAGS "-Werror")

set_source_files_properties(i2c.ino PROPERTIES LANGUAGE CXX)
add_executable(i2c i2c.ino)
target_compile_options(i2c PRIVATE -x c++)
target_link_libraries(i2c arduino_emulator)
if (USE_RPI)
    target_link_libraries(blink gpiod)
endif(USE_RPI)
