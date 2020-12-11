#!/bin/bash

if [[ ! -d ./build ]] || [ "$1" == "-f" ]
then
    apk add git cmake make binutils build-base doxygen musl-dev 
    git clone https://github.com/arduino/ArduinoCore-API.git ../Arduino/ArduinoCore-API
    mkdir ./Arduino/Arduino-Emulator/ArduinoCore-linux
    mkdir -p ./Arduino/Arduino-Emulator/ArduinoCore-linux/cores/arduino
    mkdir -p ./Arduino/Arduino-Emulator/ArduinoCore-linux/libraries
    rm -r /home/xeus-cling/Arduino/Arduino-Emulator/build
    cmake -S /home/xeus-cling/Arduino/Arduino-Emulator -B /home/xeus-cling/Arduino/Arduino-Emulator/build
    make -C /home/xeus-cling/Arduino/Arduino-Emulator/build 
else
    echo "Rebuild not necessary..."
fi
