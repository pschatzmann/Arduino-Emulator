# Arduino.cmake
# Defines a function to easily add Arduino-style libraries to your CMake project.
#
# Example usage for arduino-SAM library from GitHub
# Place this in your CMakeLists.txt after including Arduino.cmake
#
# include(${CMAKE_SOURCE_DIR}/Arduino.cmake)
# arduino_library(arduino-SAM "https://github.com/pschatzmann/arduino-SAM")
# target_link_libraries(your_target PRIVATE arduino-SAM)


# arduino_library(<name> <path_or_url> [<tag>])
function(arduino_library LIB_NAME LIB_PATH)
    set(options)
    set(oneValueArgs TAG)
    set(multiValueArgs)
    cmake_parse_arguments(ARG "" "TAG" "" ${ARGN})

    # If LIB_PATH is a URL, clone it to a local directory
    if(LIB_PATH MATCHES "^(https?://.*)$")
        set(GIT_URL "${LIB_PATH}")
        set(CLONE_DIR "${CMAKE_BINARY_DIR}/arduino_libs/${LIB_NAME}")
        if(NOT EXISTS "${CLONE_DIR}")
            if(ARG_TAG)
                message(STATUS "Cloning ${GIT_URL} (tag: ${ARG_TAG}) into ${CLONE_DIR}")
                execute_process(COMMAND git clone --branch "${ARG_TAG}" --depth 1 "${GIT_URL}" "${CLONE_DIR}")
            else()
                message(STATUS "Cloning ${GIT_URL} into ${CLONE_DIR}")
                execute_process(COMMAND git clone --depth 1 "${GIT_URL}" "${CLONE_DIR}")
            endif()
        endif()
        set(LIB_PATH "${CLONE_DIR}")
    endif()
    if(EXISTS "${LIB_PATH}/src")
        set(INC_DIR "${LIB_PATH}/src")
        file(GLOB_RECURSE SRC_FILES
            "${LIB_PATH}/src/*.c"
            "${LIB_PATH}/src/*.cpp"
        )
    else()
        # Legacy libraries without src folder
        set(INC_DIR "${LIB_PATH}")
        file(GLOB_RECURSE SRC_FILES
            "${LIB_PATH}/*.c"
            "${LIB_PATH}/*.cpp"
        )
    endif()
    # Only create library if there are source files
    if(SRC_FILES)
        add_library(${LIB_NAME} STATIC ${SRC_FILES})
        target_compile_options(${LIB_NAME} PRIVATE -DPROGMEM=)
        # Ensure C files are compiled as C, not C++
        set_target_properties(${LIB_NAME} PROPERTIES LINKER_LANGUAGE C)
    else()
        # Create a header-only interface library if no source files
        add_library(${LIB_NAME} INTERFACE)
    endif()
    target_include_directories(${LIB_NAME} PUBLIC ${INC_DIR})
    # Link arduino_emulator to propagate its include directories
    if(TARGET arduino_emulator)
        if(SRC_FILES)
            target_link_libraries(${LIB_NAME} PUBLIC arduino_emulator)
        else()
            # For interface libraries, use INTERFACE linkage
            target_link_libraries(${LIB_NAME} INTERFACE arduino_emulator)
        endif()
    endif()
endfunction()

# arduino_sketch(<name> <ino_file> [LIBRARIES <lib1> <lib2> ...] [DEFINITIONS <def1> <def2> ...])
# Creates an executable target from an Arduino sketch (.ino file)
# 
# Parameters:
#   name - The name of the executable target
#   ino_file - The .ino source file to compile
#   LIBRARIES - Optional list of additional libraries to link
#   DEFINITIONS - Optional list of compile definitions (without -D prefix)
#
# Example usage:
#   arduino_sketch(blink blink.ino)
#   arduino_sketch(my_project main.ino LIBRARIES arduino-SAM SdFat)
#   arduino_sketch(sensor_demo sensor.ino LIBRARIES Wire SPI DEFINITIONS DEBUG=1)
#   arduino_sketch(wifi_demo wifi.ino DEFINITIONS USE_HTTPS USE_SSL)
function(arduino_sketch name ino_file)
    # Parse optional arguments
    set(options)
    set(oneValueArgs)
    set(multiValueArgs LIBRARIES DEFINITIONS)
    cmake_parse_arguments(ARG "${options}" "${oneValueArgs}" "${multiValueArgs}" ${ARGN})
    
    # Set the source file properties to be treated as C++
    set_source_files_properties(${ino_file} PROPERTIES LANGUAGE CXX)
    
    # Create the executable
    add_executable(${name} ${ino_file})
    
    # Set C++ standard (inherit from parent or use C++11 minimum)
    target_compile_features(${name} PRIVATE cxx_std_11)
    
    # Add compiler flags to treat .ino files as C++
    target_compile_options(${name} PRIVATE -x c++)
    
    # Add error flags for better code quality
    target_compile_options(${name} PRIVATE -Werror)
    
    # Link with arduino emulator library (always required)
    target_link_libraries(${name} arduino_emulator)
    
    # Link additional libraries if specified
    if(ARG_LIBRARIES)
        target_link_libraries(${name} ${ARG_LIBRARIES})
    endif()
    
    # Add compile definitions if specified
    if(ARG_DEFINITIONS)
        target_compile_definitions(${name} PUBLIC ${ARG_DEFINITIONS})
    endif()
    
    # Add platform-specific libraries
    if (USE_RPI)
        target_link_libraries(${name} gpiod)
    endif(USE_RPI)
endfunction()

