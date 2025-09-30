#!/bin/bash

if [[ ! -d ./build ]] || [ "$1" == "-f" ]
then
    # apk add git cmake make binutils build-base doxygen musl-dev 
    # git clone https://github.com/arduino/ArduinoCore-API.git 
    rm -r ./build
    cmake -S ../Arduino-Emulator -B ./build -DUSE_HTTPS=ON 
    make -C ./build clean
    if make -C ./build; then
        echo "Build completed successfully!"
    else
        echo "Build failed!"
        exit 1
    fi
else
    echo "Rebuild not necessary..."
fi
