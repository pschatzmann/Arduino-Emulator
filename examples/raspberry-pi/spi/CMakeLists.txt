cmake_minimum_required(VERSION 3.11...3.19)

project(spi-rpi)
set(CMAKE_CXX_STANDARD 11)
set(DCMAKE_CXX_FLAGS "-Werror")

set_source_files_properties(spi.ino PROPERTIES LANGUAGE CXX)
add_executable(spi-rpi spi.ino)
target_compile_options(spi-rpi PRIVATE -x c++)
target_compile_definitions(spi-rpi PUBLIC -DUSE_RPI)
target_link_libraries(spi-rpi arduino_emulator gpiod)
