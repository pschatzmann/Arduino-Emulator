# An Arduino C++ Emulator Library

[![Linux build](../../actions/workflows/c-cpp-cmake.yml/badge.svg)](../../actions/workflows/c-cpp-cmake.yml)
[![Unit tests](../../actions/workflows/unit-tests.yml/badge.svg)](../../actions/workflows/unit-tests.yml)

## Using this Project as a library

If you have an Arduino Sketch that you want to run e.g in Linux, OS X or Windows you can include this library with cmake. 
Here is an [example cmake file](https://github.com/pschatzmann/arduino-audio-tools/blob/main/examples/examples-desktop/generator/CMakeLists.txt) for an [Arduino Audio Sketch](https://github.com/pschatzmann/arduino-audio-tools/tree/main/examples/examples-desktop/generator)).

## GPIO/SPI/I2C

We provide some alternative implementations:

- Dummy Implementation which does nothing
- Communicate changes to/from a Microcontroller using UDP or any Arduino Stream
- Linux / Rasperry Pi 


## Documentation

- [Class documentation](https://pschatzmann.github.io/Arduino-Emulator/html/annotated.html). 
- [Wiki](https://github.com/pschatzmann/Arduino-Emulator/wiki)

