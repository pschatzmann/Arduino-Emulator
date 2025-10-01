# An Arduino C++ Emulator Library

[![Linux build](../../actions/workflows/c-cpp-cmake.yml/badge.svg)](../../actions/workflows/c-cpp-cmake.yml)
[![Unit tests](../../actions/workflows/unit-tests.yml/badge.svg)](../../actions/workflows/unit-tests.yml)

## Using this Project as a library

If you have an Arduino Sketch that you want to run e.g in Linux, OS/X or Windows you can include this library with cmake. 
Here is an [example cmake file](https://github.com/pschatzmann/arduino-audio-tools/blob/main/examples/examples-desktop/generator/CMakeLists.txt) for a [Arduino Audio Sketch](https://github.com/pschatzmann/arduino-audio-tools/tree/main/examples/examples-desktop/generator)).

## GPIO/SPI/I2C

We provide some alternative implementations:

- Dummy Implementatin which does nothing
- Communicates changes to/from a Microcontroller using UDP
- Rasperry PI 

## Rasperry PI

You can run this emulator on an Rasperry PI

- make sure that the -DUSE_RPI cmake option is set to ON
- install libgpiod-dev (sudo apt install libgpiod-dev)
- activate SPI and I2C with raspi-config


## Jupyter

I initially really wanted to have an interactive [Jupyter](https://jupyter.org/) environemnt in which I could play around with [Arduino](https://www.arduino.cc/) commands and when I discovered that Arduino provides a good starting point with their [ArduinoCore-API](https://github.com/arduino/ArduinoCore-API/tree/105276f8d81413391b14a3dc6c80180ee9e33d56) I decided to start this project.

You can also use [xeus-cling](https://github.com/jupyter-xeus/xeus-cling) as a runtime environment to simulate an Arduino Development board and I have added the missing implementation using C or the C++ std library.

Here is a quick demo:

- [Setup](examples/jupyter/01-Setup.ipynb)
- [Introduction into some Basic Commands](examples/jupyter/02-BasicCommands.ipynb)
- [The Network Stack](examples/jupyter/03-Network.ipynb)
- [Using Pins](examples/jupyter/04-Pins.ipynb)
- [Using Serial](examples/jupyter/05-Serial.ipynb)

## Supported Defines

You can activate/deactivate some functionality with the help of the following defines:

- __USE_RPI__: activates support for Rasperry PI
- __USE_REMOTE__: activates support for GPIO/SPI/I2C using UDP or Stream
- __USE_HTTPS__: provide https support using wolfSSL
- __SKIP_HARDWARE_SETUP__: deactivate all GPIO/SPI/I2C implementations
- __SKIP_HARDWARE_WIFI__: deactivate WiFi

## Build instructions

Execute the following commends in the root of the Arduino-Emulator:

```Bash
mkdir build
cd build
cmake -DUSE_RPI=OFF -DUSE_HTTPS=OFF -DCMAKE_BUILD_TYPE=Debug ..
make
```

Adjust the cmake parameters dependent on your requirements.

## Documentation

The generated [Class documentation](https://pschatzmann.github.io/Arduino-Emulator/html/annotated.html). 

