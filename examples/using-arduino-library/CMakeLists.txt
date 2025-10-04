cmake_minimum_required(VERSION 3.11...3.19)

project(using-arduino-library)
set(CMAKE_CXX_STANDARD 11)
set(DCMAKE_CXX_FLAGS "-Werror")

# add the arduino-SAM library
include(${CMAKE_SOURCE_DIR}/ArduinoLibrary.cmake)
arduino_library(sam "https://github.com/pschatzmann/arduino-SAM")

# Define the sketch
set_source_files_properties(using-arduino-library.ino PROPERTIES LANGUAGE CXX)
add_executable(using-arduino-library using-arduino-library.ino)
target_compile_options(using-arduino-library PRIVATE -x c++)
target_link_libraries(using-arduino-library arduino_emulator sam)
if (USE_RPI)
    target_link_libraries(using-arduino-library gpiod)
endif(USE_RPI)
