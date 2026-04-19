# Works with 3.11 and tested through 3.19
cmake_minimum_required(VERSION 3.11...3.19)

# Project name and a few useful settings. Other commands can pick up the results
project(arduino_emulator VERSION 0.1 DESCRIPTION "Arduino Emulator for Linux" LANGUAGES C CXX)

# Https takes quite some time and is not always neede
option(USE_HTTPS "Https Support" OFF)
option(USE_RPI "Raspberry Pi Support" OFF)
option(USE_REMOTE "Remote API Support" OFF)
option(BUILD_SHARED_LIBS "Build with shared libraries" OFF)
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
endif()

# FTDI support: Build with libftdi1
if(USE_FTDI)
    add_definitions(-DUSE_FTDI)
endif()

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
    FILES_MATCHING
    PATTERN "*.h"
)
install(DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/ArduinoCore-Linux/cores/arduino/" # source directory
    DESTINATION "include/arduino" # target directory
    FILES_MATCHING
    PATTERN "*.h"
)

# All users of this library will need at least C++17
target_compile_features(arduino_emulator PUBLIC cxx_std_17)

# Include Arduino library functions
include(${CMAKE_CURRENT_SOURCE_DIR}/Arduino.cmake)

if (USE_RPI)
    message(STATUS "USE_RPI=${USE_RPI}")

    # Try pkg-config first (handles include dirs and version info). If pkg-config
    # or the .pc file is missing, fall back to find_library.
    find_package(PkgConfig QUIET)

    set(_gpiod_candidates libgpiod gpiod libgpiod2)
    set(_GPIOD_FOUND FALSE)
    if(PkgConfig_FOUND)
        foreach(_name IN LISTS _gpiod_candidates)
            pkg_check_modules(GPIOD_PKG QUIET ${_name})
            if(GPIOD_PKG_FOUND)
                set(_GPIOD_FOUND TRUE)
                set(GPIOD_INCLUDE_DIRS ${GPIOD_PKG_INCLUDE_DIRS})
                set(GPIOD_LIBRARIES ${GPIOD_PKG_LIBRARIES})
                set(GPIOD_VERSION ${GPIOD_PKG_VERSION})
                message(STATUS "Found libgpiod via pkg-config (${_name}) version ${GPIOD_VERSION}")
                break()
            endif()
        endforeach()
    endif()

    # Fallback: search for the library file if pkg-config didn't find anything
    if(NOT _GPIOD_FOUND)
        find_library(GPIOD_LIB NAMES gpiod gpiod2 libgpiod libgpiod2)
        if(GPIOD_LIB)
            set(_GPIOD_FOUND TRUE)
            message(STATUS "Found libgpiod (via find_library): ${GPIOD_LIB}")
        endif()
    endif()

    if(_GPIOD_FOUND)
        # Add compile definition to enable RPi-specific code paths
        target_compile_definitions(arduino_emulator PUBLIC USE_RPI)

        # Prefer pkg-config provided include dirs/libs, otherwise use the found library
        if(DEFINED GPIOD_INCLUDE_DIRS)
            target_include_directories(arduino_emulator PUBLIC ${GPIOD_INCLUDE_DIRS})
        endif()

        if(DEFINED GPIOD_LIBRARIES)
            # GPIOD_LIBRARIES from pkg-config may contain linker flags; use them directly
            target_link_libraries(arduino_emulator PUBLIC ${GPIOD_LIBRARIES})
        elseif(GPIOD_LIB)
            target_link_libraries(arduino_emulator PUBLIC ${GPIOD_LIB})
        endif()

        # If version information is available and < 2.0, add a compatibility define
        if(DEFINED GPIOD_VERSION AND NOT GPIOD_VERSION STREQUAL "")
            if(GPIOD_VERSION VERSION_LESS "2.0")
                target_compile_definitions(arduino_emulator PUBLIC LIBGPIOD_V1)
                message(STATUS "Assuming libgpiod v1 compatibility (version: ${GPIOD_VERSION})")
            else()
                message(STATUS "libgpiod v2 detected (version: ${GPIOD_VERSION})")
            endif()
        else()
            # If we only found the library file, try to guess v1 vs v2 from the library name
            if(GPIOD_LIB AND GPIOD_LIB MATCHES "gpiod2")
                message(STATUS "Detected libgpiod2 library file; assuming v2")
            else()
                # conservative default: assume v1 compatibility when uncertain
                target_compile_definitions(arduino_emulator PUBLIC LIBGPIOD_V1)
                message(STATUS "libgpiod version unknown: enabling v1 compatibility define")
            endif()
        endif()
    else()
        message(STATUS "libgpiod not found — Raspberry Pi gpio support disabled")
    endif()

endif(USE_RPI)

if (USE_REMOTE)
    message(STATUS "USE_REMOTE=${USE_REMOTE}")
    target_compile_options(arduino_emulator PUBLIC -DUSE_REMOTE)
endif(USE_REMOTE)

if(USE_HTTPS)
    message(STATUS "USE_HTTPS=${USE_HTTPS}")
    # Add external libraries
    target_link_libraries(arduino_emulator wolfssl)
endif(USE_HTTPS)

if(USE_FTDI)
    message(STATUS "USE_FTDI=${USE_FTDI}")
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
    message(STATUS "BUILD_EXAMPLES=${BUILD_EXAMPLES}")
    add_subdirectory("examples")
endif(BUILD_EXAMPLES)
