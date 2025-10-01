#include <cstring>
#include <chrono> 
#include <ctime>
#include <stdio.h>
#include <thread>
#include <chrono>
#include "Arduino.h"
#include "Common.h"
#include "Stream.h"
#include "PluggableUSB.h"
#include "RemoteSerial.h"
#include "Hardware.h"
#if !defined(SKIP_HARDWARE_SETUP)
#  include "HardwareSetupRemote.h"
#endif
#if !defined(SKIP_HARDWARE_WIFI)
#  include "WiFi.h"
#  include "WiFiClient.h"
#endif
#if defined(USE_RPI)
#  include "HardwareGPIO_RPI.h"
#  include "HardwareI2C_RPI.h"
#endif
#include "PluggableUSB.h"
#include "deprecated-avr-comp/avr/dtostrf.h"
#include "ArduinoLogger.h"
#include "StdioDevice.h"
#include "Serial.h"

namespace arduino {

#if !defined(SKIP_HARDWARE_WIFI)
WifiMock WiFi;         // So that we can use the WiFi
#endif


//static PluggableUSB_ obj;
PluggableUSB_::PluggableUSB_(){}

}

    
// sleep ms milliseconds
void delay(unsigned long ms){
    std::this_thread::sleep_for(std::chrono::milliseconds(ms));    
}

// sleep us milliseconds
void delayMicroseconds(unsigned int us){
    std::this_thread::sleep_for(std::chrono::microseconds(us));        
}

// double to string conversion -> we can use sprintf which is complete in linux
char *dtostrf(double val, signed char width, unsigned char prec, char *sout){
    sprintf(sout,"%*.*lf",width,prec, val);
    return sout;
}

// Returns the number of milliseconds passed since epich
unsigned long millis() {
    static uint64_t start = 0;
    using namespace std::chrono;
    // Get current time with precision of milliseconds
    auto now = time_point_cast<milliseconds>(system_clock::now());
    // sys_milliseconds is type time_point<system_clock, milliseconds>
    using sys_milliseconds = decltype(now);
    // Convert time_point to signed integral type
    auto result = now.time_since_epoch().count();
    if (start == 0) {
        start = result;
    }
    return result - start;

}

// Returns the micros of milliseconds passed since epich
unsigned long micros(void){
    using namespace std::chrono;
    // Get current time with precision of milliseconds
    auto now = time_point_cast<microseconds>(system_clock::now());
    // sys_milliseconds is type time_point<system_clock, milliseconds>
    using sys_milliseconds = decltype(now);
    // Convert time_point to signed integral type
    return now.time_since_epoch().count();
}

void pinMode(pin_size_t pinNumber, PinMode pinMode){
    if (Hardware.gpio != nullptr) {
        Hardware.gpio->pinMode(pinNumber,pinMode);
    }
}
void digitalWrite(pin_size_t pinNumber, PinStatus status) {
    if (Hardware.gpio != nullptr) {
        Hardware.gpio->digitalWrite(pinNumber,status);
    }
}
PinStatus digitalRead(pin_size_t pinNumber) {
    if (Hardware.gpio != nullptr) {
        return Hardware.gpio->digitalRead(pinNumber);
    } else {
        return HIGH; //sumulate input pullup
    }
}
int analogRead(pin_size_t pinNumber){
    if (Hardware.gpio != nullptr) {
        return Hardware.gpio->analogRead(pinNumber);
    } else {
        return 0;
    }

}
void analogReference(uint8_t mode){
    if (Hardware.gpio != nullptr) {
        Hardware.gpio->analogReference(mode);
    }
}
void analogWrite(pin_size_t pinNumber, int value) {
    if (Hardware.gpio != nullptr) {
        Hardware.gpio->analogWrite(pinNumber,value);
    }
}
void tone(uint8_t pinNumber, unsigned int frequency, unsigned long duration) {
    if (Hardware.gpio != nullptr) {
        Hardware.gpio->tone(pinNumber,frequency,duration);
    }
}
void noTone(uint8_t pinNumber) {
    if (Hardware.gpio != nullptr) {
        Hardware.gpio->noTone(pinNumber);
    }
}
unsigned long pulseIn(uint8_t pinNumber, uint8_t state, unsigned long timeout){
    if (Hardware.gpio != nullptr) {
        return Hardware.gpio->pulseIn(pinNumber, state, timeout);
    } else {
        return 0;
    }
}
unsigned long pulseInLong(uint8_t pinNumber, uint8_t state, unsigned long timeout){
    if (Hardware.gpio != nullptr) {
        return Hardware.gpio->pulseInLong(pinNumber, state, timeout);
    } else {
        return 0;
    }
}

void yield(){
}


#if defined(_MSC_VER)

// For some reason bodies is not available under MSVC

#include <cstdlib>

void randomSeed(unsigned long seed)
{
  if (seed != 0) {
    srand(seed);
  }
}

long random(long howbig)
{
  if (howbig == 0) {
    return 0;
  }
  return rand() % howbig;
}

long random(long howsmall, long howbig)
{
  if (howsmall >= howbig) {
    return howsmall;
  }
  long diff = howbig - howsmall;
  return random(diff) + howsmall;
}

#endif

/*
Notes on how to build under Windows with MSVC
=============================================
Under windows there is well known conflict between Arduino String.h and system string.h.
If you read this, you probably seen a lot of problems with functiosn like strlen, strcmp, strcpy, memcpy etc.

BACKROUND:
Main problem reason is that windows file system is case insensitive and when some module trying to include 
<string.h> or <String.h> will include file that is found first on include path.

SOLUTION:
To partially solve this problem, system <string.h> always included in angle brackets, while Arduino "String.h" in quotes with correct path.
This way IF (and only if) system include paths are BEFORE Arduino include paths, THEN <string.h> will always include system file.
Note: While you do not need to include String.h directly, it is included by Arduino.h, but you still have to setup path correctly.

IMPLEMENTATION: HOW TO ENSURE THAT SYSTEM PATHS ARE BEFORE ARDUINO PATHS:
You have to ensure that system include paths are before any Arduino include paths.
On systems like VSCODE+MSVC system include paths are set via INCLUDE env var, while project-related path are in command line.
Unfortunately %INCLUDE% have lower priority than path in command line (and /I compiler option accept only one path),
you have to do some tricks to achive the goal.

For VSCODE, you have to set paths in two places:
1. .vscode/c_cpp_properties.json 
Here are paths for IntelliSense and (code completion) you have to add ${env:INCLUDE} as first entry in "includePath" array
"includePath": [ "${env:INCLUDE}",
2. .vscode/tasks.json
Here are paths for build (compilation), but since adding /I with multiple paths does not work,
you cannot add /I${env:INCLUDE} as first entry in "args" array
2.1 Option A: add add VS system paths one by one as separate /I entries BEFORE any Arduino related paths
"/I${env:VCINSTALLDIR}Tools\\MSVC\\${env:VCToolsVersion}\\ATLMFC\\include",
"/I${env:VCINSTALLDIR}Tools\\MSVC\\${env:VCToolsVersion}\\include",
"/I${env:NETFXSDKDir}include\\um",
"/I${env:WindowsSdkDir}include\\${env:UCRTVersion}\\ucrt",
"/I${env:WindowsSdkDir}include\\${env:UCRTVersion}\\shared",
"/I${env:WindowsSdkDir}include\\${env:UCRTVersion}\\um",
"/I${env:WindowsSdkDir}include\\${env:UCRTVersion}\\winrt",
"/I${env:WindowsSdkDir}include\\${env:UCRTVersion}\\cppwinrt",
2.2 Option B: appens your project Arduino-related path to INCLUDE env var after system paths
You have to add LIB, LIBPATH among with INCLUDE env vars to "env" section of "options" in tasks.json
Example:
"options": {
...
"env": { "LIB": "${env:LIB}", "LIBPATH": "${env:LIBPATH}",
    "INCLUDE": "${env:INCLUDE};${workspaceFolder};${workspaceFolder}/tests;${workspaceFolder}/tests/sim;${workspaceFolder}/tests/sim/Arduino-Emulator/ArduinoCore-API/api;${workspaceFolder}/tests/sim/Arduino-Emulator/ArduinoCore-API/api/deprecated-avr-comp;${workspaceFolder}/tests/sim/Arduino-Emulator/ArduinoCore-Linux/cores/arduino",
}
2.3 Option C: invent something more elegant by yourself :)
*/
