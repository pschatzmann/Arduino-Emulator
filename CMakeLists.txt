# Works with 3.11 and tested through 3.19
cmake_minimum_required(VERSION 3.11...3.19)

# Project name and a few useful settings. Other commands can pick up the results
project(
  arduino_emulator
  VERSION 0.1
  DESCRIPTION "Arduino Emulator for Linux"
  LANGUAGES CXX)

# Https takes quite some time and is not always neede
option(BUILD_HTTPS "Https Support" OFF)

# Https support: Build with wolfssl 
if(BUILD_HTTPS)
    include(FetchContent)
    FetchContent_Declare(wolfssl GIT_REPOSITORY "https://github.com/wolfSSL/wolfssl.git" GIT_TAG v5.1.1-stable )
    FetchContent_GetProperties(wolfssl)
    if(NOT wolfssl)
        FetchContent_Populate(wolfssl)
        add_subdirectory(${wolfssl_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR}/wolfssl)
    endif()
    add_definitions(-DUSE_HTTPS)
endif(BUILD_HTTPS)

add_definitions(-DHOST)
option(BUILD_SHARED_LIBS "Build with shared libraries" ON)
option(PROVIDE_SERIALLIB "Serial Interface" OFF)

# glob, but only for CMake 3.12 or later:
file(GLOB HEADER_LIST CONFIGURE_DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/ArduinoCore-API/api/*.h")
file(GLOB SRC_LIST CONFIGURE_DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/ArduinoCore-API/api/*.cpp" "${CMAKE_CURRENT_SOURCE_DIR}/ArduinoCore-Linux/cores/arduino/*.cpp")

# Make an dynamic library 
add_library(arduino_emulator ${SRC_LIST} ${HEADER_LIST})

# Define the directories with the h files
target_include_directories (arduino_emulator PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/ArduinoCore-API/api"
    "${CMAKE_CURRENT_SOURCE_DIR}/ArduinoCore-Linux/cores/arduino" )

# Install header files 
install(DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/ArduinoCore-API/api/" # source directory
    DESTINATION "include/arduino" # target directory
    FILES_MATCHING # install only matched files
    PATTERN "*.h" # select header files
)
install(DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/ArduinoCore-Linux/cores/arduino/" # source directory
    DESTINATION "include/arduino" # target directory
    FILES_MATCHING # install only matched files
    PATTERN "*.h" # select header files
)

# All users of this library will need at least C++17
target_compile_features(arduino_emulator PUBLIC cxx_std_17)
target_compile_options(arduino_emulator PRIVATE -Wno-nonportable-include-path)

if(BUILD_HTTPS)
    # Add external libraries
    target_link_libraries(arduino_emulator wolfssl)
endif(BUILD_HTTPS)
