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
