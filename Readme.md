# An Arduino C++ Emulator Library

## Jupyter

I really wanted to have an interactive [Jupyter](https://jupyter.org/) environemnt in which I could play around with [Arduino](https://www.arduino.cc/) commands and when I discovered that Arduino provides a good starting point with their [ArduinoCore-API](https://github.com/arduino/ArduinoCore-API/tree/105276f8d81413391b14a3dc6c80180ee9e33d56) I decided to draft this prototype.

I am using [xeus-cling](https://github.com/jupyter-xeus/xeus-cling) as a runtime environment to simulate an Arduino Development board and I have added the missing implementation using C or the C++ std library.

The goal is finally to be able to provide different alternative implementations for accessing the pins
- Using a Stream interface to communicate with an real Arduino Device (e.g. via udp)
- Using some standard Linux userspace APIs for GPIO
- Using wiringPI to be run on a Raspberry PI
- Output to a file from where it can be analysed

I have a __first working prototype__ ready but naturally there is still a lot to do.

### Content
- [Setup](01-Setup.ipynb)
- [Introduction into some Basic Commands](02-BasicCommands.ipynb)
- [The Network Stack](03-Network.ipynb)
- [Using Pins](04-Pins.ipynb)
- [Using Serial](05-Serial.ipynb)

## Using this Project as a library

If you have an Arduino Sketch that you want to run e.g in Linux or OS/X you can include this library with cmake. 
Here is an [example cmake file](https://github.com/pschatzmann/arduino-audio-tools/blob/main/examples-desktop/generator/CMakeLists.txt) for a [Arduino Audio Sketch](https://github.com/pschatzmann/arduino-audio-tools/tree/main/examples-desktop/generator).


