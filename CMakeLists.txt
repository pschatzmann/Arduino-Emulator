# Works with 3.11 and tested through 3.19
cmake_minimum_required(VERSION 3.11...3.19)

# Project name and a few useful settings. Other commands can pick up the results
project(
  arduino_emulator
  VERSION 0.1
  DESCRIPTION "Arduino Emulator for Linux"
  LANGUAGES CXX)

add_definitions(-DHOST)

# Define the directories with the h files
include_directories("${CMAKE_SOURCE_DIR}/ArduinoCore-API/api"
    "${CMAKE_SOURCE_DIR}/ArduinoCore-Linux/cores/arduino")

# glob, but only for CMake 3.12 or later:
file(GLOB HEADER_LIST CONFIGURE_DEPENDS "${PROJECT_SOURCE_DIR}/ArduinoCore-API/api/*.h")
file(GLOB SRC_LIST CONFIGURE_DEPENDS "${PROJECT_SOURCE_DIR}/ArduinoCore-API/api/*.cpp" "${PROJECT_SOURCE_DIR}/ArduinoCore-Linux/cores/arduino/*.cpp")

# Make an dynamic library 
add_library(arduino_emulator SHARED ${SRC_LIST} ${HEADER_LIST})

# All users of this library will need at least C++17
target_compile_features(arduino_emulator PUBLIC cxx_std_17)
target_compile_options(arduino_emulator PRIVATE -Wno-nonportable-include-path)


