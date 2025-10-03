cmake_minimum_required(VERSION 3.11...3.19)

project(serial2)
set(CMAKE_CXX_STANDARD 11)
set(DCMAKE_CXX_FLAGS "-Werror")

set_source_files_properties(serial2.ino PROPERTIES LANGUAGE CXX)
add_executable(serial2 serial2.ino)
target_compile_options(serial2 PRIVATE -x c++)
target_link_libraries(serial2 arduino_emulator)
if (USE_RPI)
    target_link_libraries(serial2 gpiod)
endif(USE_RPI)
