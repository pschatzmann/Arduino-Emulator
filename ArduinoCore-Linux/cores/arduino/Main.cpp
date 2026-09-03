/*
	Main.cpp - Main entry point for Arduino sketches
    Part of Arduino - http://www.arduino.cc
	Copyright (c) 2025 Phil Schatzmann. All right reserved.

	This library is free software; you can redistribute it and/or
	modify it under the terms of the GNU Lesser General Public
	License as published by the Free Software Foundation; either
	version 2.1 of the License, or (at your option) any later version.

	This library is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
	Lesser General Public License for more details.

	You should have received a copy of the GNU Lesser General Public
	License along with this library; if not, write to the Free Software
	Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301 USA
*/

#include "Arduino.h"
#include "HardwareSetup.h"


#if defined(__GNUC__)
#define ARDUINO_EMULATOR_WEAK __attribute__((weak))
#else
#define ARDUINO_EMULATOR_WEAK
#endif

ARDUINO_EMULATOR_WEAK void setup() {}
ARDUINO_EMULATOR_WEAK void loop() {}

void hardwareSetup(){
#if !defined(SKIP_HARDWARE_SETUP)
#  if (defined(USE_RPI))
    RPI.begin();
#  elif defined(USE_FTDI)
	FTDI.begin();
#  elif defined(USE_REMOTE)
    Remote.begin();
#  endif
#endif
}

ARDUINO_EMULATOR_WEAK int main () {
    hardwareSetup();
    setup();
    while(true){
        loop();
    }
}	

