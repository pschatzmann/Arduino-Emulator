#!/bin/bash

if [[ ! -d ./build ]] || [ "$1" == "-f" ]
then
    # apk add git cmake make binutils build-base doxygen musl-dev 
    # git clone https://github.com/arduino/ArduinoCore-API.git 
    rm -r ./build
    cmake -S ../Arduino-Emulator -B ./build -DBUILD_HTTPS=ON 
    make -C ./build clean
    make -C ./build 
else
    echo "Rebuild not necessary..."
fi
