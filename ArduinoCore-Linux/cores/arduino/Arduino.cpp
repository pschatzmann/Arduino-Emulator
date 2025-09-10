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
#if defined(PROVIDE_HARDWARE_SETUP_SKIP)
// Not available for Windows / MSVC
#else
#include "HardwareSetup.h"
#endif
#if defined(PROVIDE_HARDWARE_WIFI_SKIP)
// Not available for Windows / MSVC
#else
#include "WiFi.h"
#include "WiFiClient.h"
#endif
#include "PluggableUSB.h"
#include "deprecated-avr-comp/avr/dtostrf.h"
#include "ArduinoLogger.h"
#include "ArdStdio.h"
#include "Serial.h"

namespace arduino {

ArduinoLogger Logger;  // Support for logging
StdioDevice Serial;    // output to screen
HardwareImpl Hardware; // implementation for gpio, spi, i2c
#if defined(PROVIDE_HARDWARE_WIFI_SKIP)
#else
WifiMock WiFi;         // So that we can use the WiFi
#endif
#if defined(PROVIDE_HARDWARE_SETUP_SKIP)
#else
HardwareSetupImpl HardwareSetup; // setup for implementation
#endif
#if PROVIDE_SERIALLIB    
SerialImpl Serial1("/dev/ttyACM0");    // output to serial port
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
    using namespace std::chrono;
    // Get current time with precision of milliseconds
    auto now = time_point_cast<milliseconds>(system_clock::now());
    // sys_milliseconds is type time_point<system_clock, milliseconds>
    using sys_milliseconds = decltype(now);
    // Convert time_point to signed integral type
    return now.time_since_epoch().count();

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


