# An Arduino C++ Emulator Library

[![Linux build](../../actions/workflows/c-cpp-cmake.yml/badge.svg)](../../actions/workflows/c-cpp-cmake.yml)
[![Unit tests](../../actions/workflows/unit-tests.yml/badge.svg)](../../actions/workflows/unit-tests.yml)

## Purpuse

This project provides a C++ emulator library for Arduino, allowing you to run Arduino sketches on platforms like Linux, OS X, or Windows. It enables cross-platform development and testing of Arduino code on your Desktop without requiring a Microcontroller.

## GPIO/SPI/I2C

We provide some alternative implementations:

- Mock Implementation which provides all functions doing nothing
- Communicate changes to/from a Microcontroller using UDP or any Arduino Stream
- Rasperry Pi using gpiod.h, linux/i2c-dev.h, linux/spi/spidev.h
- FTDI support using the FT2232HL


## Documentation

- The documentation can be found in the [Wiki](https://github.com/pschatzmann/Arduino-Emulator/wiki)

