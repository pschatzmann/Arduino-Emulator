# Works with 3.11 and tested through 3.19
cmake_minimum_required(VERSION 3.11...3.19)

# Project name and a few useful settings. Other commands can pick up the results
project(arduino_emulator VERSION 0.1 DESCRIPTION "Arduino Emulator for Linux" LANGUAGES C CXX)

# Https takes quite some time and is not always neede
option(USE_HTTPS "Https Support" OFF)
option(USE_RPI "Raspberry Pi Support" OFF)
option(USE_REMOTE "Remote API Support" OFF)
option(BUILD_SHARED_LIBS "Build with shared libraries" ON)
option(BUILD_EXAMPLES "Build with examples" OFF)

# Https support: Build with wolfssl 
if(USE_HTTPS)
    include(FetchContent)
    FetchContent_Declare(wolfssl GIT_REPOSITORY "https://github.com/wolfSSL/wolfssl.git" GIT_TAG v5.8.2-stable )
    FetchContent_GetProperties(wolfssl)
    if(NOT wolfssl)
        FetchContent_Populate(wolfssl)
        add_subdirectory(${wolfssl_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR}/wolfssl)
    endif()
    add_definitions(-DUSE_HTTPS -DDEBUG_WOLFSSL)
endif(USE_HTTPS)

# FTDI support: Build with libftdi1
if(USE_FTDI)
    add_definitions(-DUSE_FTDI)
endif(USE_FTDI)

add_definitions(-DHOST)

# glob, but only for CMake 3.12 or later:
file(GLOB HEADER_LIST CONFIGURE_DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/ArduinoCore-API/api/*.h")
file(GLOB SRC_LIST CONFIGURE_DEPENDS 
    "${CMAKE_CURRENT_SOURCE_DIR}/ArduinoCore-API/api/*.cpp" 
    "${CMAKE_CURRENT_SOURCE_DIR}/ArduinoCore-Linux/cores/arduino/*.cpp" 
    "${CMAKE_CURRENT_SOURCE_DIR}/ArduinoCore-Linux/cores/rasperry_pi/*.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/ArduinoCore-Linux/cores/ftdi/*.cpp")

# Make an dynamic library 
add_library(arduino_emulator ${SRC_LIST} ${HEADER_LIST})

# Define the directories with the h files
target_include_directories (arduino_emulator PUBLIC 
    "${CMAKE_CURRENT_SOURCE_DIR}/ArduinoCore-API"
    "${CMAKE_CURRENT_SOURCE_DIR}/ArduinoCore-Linux/cores/arduino"
    "${CMAKE_CURRENT_SOURCE_DIR}/ArduinoCore-Linux/cores/rasperry_pi"
    "${CMAKE_CURRENT_SOURCE_DIR}/ArduinoCore-Linux/cores/ftdi"
    "${CMAKE_CURRENT_SOURCE_DIR}/ArduinoCore-Linux/libraries" 
    "${CMAKE_CURRENT_SOURCE_DIR}/ArduinoCore-API/api/deprecated"
    )   

# Install header files 
install(DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/ArduinoCore-API/api/" # source directory
    DESTINATION "include/arduino/api" # target directory
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

# Include Arduino library functions
include(${CMAKE_CURRENT_SOURCE_DIR}/Arduino.cmake)

if (USE_RPI)
    target_compile_options(arduino_emulator PUBLIC -DUSE_RPI)
    target_link_libraries(arduino_emulator gpiod)
endif(USE_RPI)

if (USE_REMOTE)
    target_compile_options(arduino_emulator PUBLIC -DUSE_REMOTE)
endif(USE_REMOTE)

if(USE_HTTPS)
    # Add external libraries
    target_link_libraries(arduino_emulator wolfssl)
endif(USE_HTTPS)

if(USE_FTDI)
    # Link with libftdi1
    find_package(PkgConfig REQUIRED)
    pkg_check_modules(FTDI REQUIRED libftdi1)
    target_link_libraries(arduino_emulator ${FTDI_LIBRARIES})
    target_include_directories(arduino_emulator PUBLIC ${FTDI_INCLUDE_DIRS})
    target_compile_options(arduino_emulator PUBLIC ${FTDI_CFLAGS_OTHER})
    # Suppress deprecation warnings for FTDI-related code
    target_compile_options(arduino_emulator PRIVATE -Wno-deprecated-declarations)
endif(USE_FTDI)

# build examples
if(BUILD_EXAMPLES)
    add_subdirectory("examples")
endif(BUILD_EXAMPLES)
