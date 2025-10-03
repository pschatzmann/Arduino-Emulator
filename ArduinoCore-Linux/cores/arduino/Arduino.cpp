#include "Arduino.h"

#include <stdio.h>

#include <chrono>
#include <cstring>
#include <ctime>
#include <thread>

#include "GPIOWrapper.h"
#include "HardwareGPIO.h"
#include "RemoteSerial.h"
#include "api/Common.h"
#include "api/PluggableUSB.h"
#include "api/Stream.h"
#if !defined(SKIP_HARDWARE_SETUP)
#include "HardwareSetupRemote.h"
#endif
#if !defined(SKIP_HARDWARE_WIFI)
#include "WiFi.h"
#include "WiFiClient.h"
#endif
#if defined(USE_RPI)
#include "HardwareGPIO_RPI.h"
#include "HardwareI2C_RPI.h"
#endif
#include "ArduinoLogger.h"
#include "Serial.h"
#include "StdioDevice.h"
#include "api/PluggableUSB.h"
#include "api/deprecated-avr-comp/avr/dtostrf.h"

namespace arduino {

#if !defined(SKIP_HARDWARE_WIFI)
WifiMock WiFi;  // So that we can use the WiFi
#endif

// static PluggableUSB_ obj;
PluggableUSB_::PluggableUSB_() {}

}  // namespace arduino

// sleep ms milliseconds
void delay(unsigned long ms) {
  std::this_thread::sleep_for(std::chrono::milliseconds(ms));
}

// sleep us milliseconds
void delayMicroseconds(unsigned int us) {
  std::this_thread::sleep_for(std::chrono::microseconds(us));
}

// double to string conversion -> we can use sprintf which is complete in linux
char* dtostrf(double val, signed char width, unsigned char prec, char* sout) {
  sprintf(sout, "%*.*lf", width, prec, val);
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
unsigned long micros(void) {
  using namespace std::chrono;
  // Get current time with precision of milliseconds
  auto now = time_point_cast<microseconds>(system_clock::now());
  // sys_milliseconds is type time_point<system_clock, milliseconds>
  using sys_milliseconds = decltype(now);
  // Convert time_point to signed integral type
  return now.time_since_epoch().count();
}

void pinMode(pin_size_t pinNumber, PinMode pinMode) {
  GPIO.pinMode(pinNumber, pinMode);
}
void digitalWrite(pin_size_t pinNumber, PinStatus status) {
  GPIO.digitalWrite(pinNumber, status);
}
PinStatus digitalRead(pin_size_t pinNumber) {
  return GPIO.digitalRead(pinNumber);
}
int analogRead(pin_size_t pinNumber) { return GPIO.analogRead(pinNumber); }
void analogReference(uint8_t mode) { GPIO.analogReference(mode); }
void analogWrite(pin_size_t pinNumber, int value) {
  GPIO.analogWrite(pinNumber, value);
}
void tone(uint8_t pinNumber, unsigned int frequency, unsigned long duration) {
  GPIO.tone(pinNumber, frequency, duration);
}
void noTone(uint8_t pinNumber) { GPIO.noTone(pinNumber); }
unsigned long pulseIn(uint8_t pinNumber, uint8_t state, unsigned long timeout) {
  return GPIO.pulseIn(pinNumber, state, timeout);
}
unsigned long pulseInLong(uint8_t pinNumber, uint8_t state,
                          unsigned long timeout) {
  return GPIO.pulseInLong(pinNumber, state, timeout);
}

void yield() {}
