# ArduinoLibrary.cmake
# Defines a function to easily add Arduino-style libraries to your CMake project.


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
    set(INC_DIR "${LIB_PATH}/src")
    file(GLOB SRC_FILES
        "${LIB_PATH}/src/*.c"
        "${LIB_PATH}/src/*.cpp"
    )
    add_library(${LIB_NAME} STATIC ${SRC_FILES})
    target_compile_options(${LIB_NAME} PRIVATE -DPROGMEM=)
    target_include_directories(${LIB_NAME} PUBLIC ${INC_DIR})
    # Link arduino_emulator to propagate its include directories
    if(TARGET arduino_emulator)
        target_link_libraries(${LIB_NAME} PUBLIC arduino_emulator)
    endif()
endfunction()

# Example usage for arduino-SAM library from GitHub
# Place this in your CMakeLists.txt after including ArduinoLibrary.cmake
#
# include(${CMAKE_SOURCE_DIR}/ArduinoLibrary.cmake)
# arduino_library(arduino-SAM "https://github.com/pschatzmann/arduino-SAM")
# target_link_libraries(your_target PRIVATE arduino-SAM)
