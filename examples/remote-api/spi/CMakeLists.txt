cmake_minimum_required(VERSION 3.11...3.19)

project(spi)
set(CMAKE_CXX_STANDARD 11)
set(DCMAKE_CXX_FLAGS "-Werror")

set_source_files_properties(spi.ino PROPERTIES LANGUAGE CXX)
add_executable(spi spi.ino)
target_compile_options(spi PRIVATE -x c++)
target_compile_definitions(spi PUBLIC -DUSE_UDP)
target_link_libraries(spi arduino_emulator)
