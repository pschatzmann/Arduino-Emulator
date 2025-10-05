/*
  Arduino.cpp
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

/**
 * @brief Pause the program for the amount of time (in milliseconds) specified as parameter
 * @param ms The number of milliseconds to pause (unsigned long)
 */
void delay(unsigned long ms) {
  std::this_thread::sleep_for(std::chrono::milliseconds(ms));
}

/**
 * @brief Pause the program for the amount of time (in microseconds) specified as parameter
 * @param us The number of microseconds to pause (unsigned int)
 */
void delayMicroseconds(unsigned int us) {
  std::this_thread::sleep_for(std::chrono::microseconds(us));
}

/**
 * @brief Convert a floating point number to string
 * @param val The double value to convert
 * @param width The minimum field width (signed char)
 * @param prec The precision (number of digits after decimal point)
 * @param sout The output string buffer
 * @return Pointer to the output string
 */
char* dtostrf(double val, signed char width, unsigned char prec, char* sout) {
  sprintf(sout, "%*.*lf", width, prec, val);
  return sout;
}

/**
 * @brief Returns the number of milliseconds passed since the Arduino board began running the current program
 * @return Number of milliseconds passed since the program started (unsigned long)
 */
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

/**
 * @brief Returns the number of microseconds since the Arduino board began running the current program
 * @return Number of microseconds passed since the program started (unsigned long)
 */
unsigned long micros(void) {
  using namespace std::chrono;
  // Get current time with precision of milliseconds
  auto now = time_point_cast<microseconds>(system_clock::now());
  // sys_milliseconds is type time_point<system_clock, milliseconds>
  using sys_milliseconds = decltype(now);
  // Convert time_point to signed integral type
  return now.time_since_epoch().count();
}

/**
 * @brief Configure the specified pin to behave either as an input or an output
 * @param pinNumber The pin whose mode you wish to set
 * @param pinMode INPUT, OUTPUT, or INPUT_PULLUP
 */
void pinMode(pin_size_t pinNumber, PinMode pinMode) {
  GPIO.pinMode(pinNumber, pinMode);
}

/**
 * @brief Write a HIGH or a LOW value to a digital pin
 * @param pinNumber The pin number
 * @param status HIGH or LOW
 */
void digitalWrite(pin_size_t pinNumber, PinStatus status) {
  GPIO.digitalWrite(pinNumber, status);
}

/**
 * @brief Reads the value from a specified digital pin, either HIGH or LOW
 * @param pinNumber The number of the digital pin you want to read
 * @return HIGH or LOW
 */
PinStatus digitalRead(pin_size_t pinNumber) {
  return GPIO.digitalRead(pinNumber);
}

/**
 * @brief Reads the value from the specified analog pin
 * @param pinNumber The number of the analog input pin to read from (0 to 5 on most boards)
 * @return The analog reading on the pin (int 0 to 1023)
 */
int analogRead(pin_size_t pinNumber) { return GPIO.analogRead(pinNumber); }

/**
 * @brief Configures the reference voltage used for analog input
 * @param mode Which type of reference to use (DEFAULT, INTERNAL, EXTERNAL)
 */
void analogReference(uint8_t mode) { GPIO.analogReference(mode); }

/**
 * @brief Writes an analog value (PWM wave) to a pin
 * @param pinNumber The pin to write to
 * @param value The duty cycle: between 0 (always off) and 255 (always on)
 */
void analogWrite(pin_size_t pinNumber, int value) {
  GPIO.analogWrite(pinNumber, value);
}


/**
 * @brief Generates a square wave of the specified frequency (and 50% duty cycle) on a pin
 * @param pinNumber The pin on which to generate the tone
 * @param frequency The frequency of the tone in hertz - unsigned int
 * @param duration The duration of the tone in milliseconds (optional) - unsigned long
 */
void tone(uint8_t pinNumber, unsigned int frequency, unsigned long duration) {
  GPIO.tone(pinNumber, frequency, duration);
}

/**
 * @brief Stops the generation of a square wave triggered by tone()
 * @param pinNumber The pin on which to stop generating the tone
 */
void noTone(uint8_t pinNumber) { GPIO.noTone(pinNumber); }

/**
 * @brief Reads a pulse (either HIGH or LOW) on a pin
 * @param pinNumber The number of the pin on which you want to read the pulse
 * @param state Type of pulse to read: either HIGH or LOW
 * @param timeout The number of microseconds to wait for the pulse to be completed (optional)
 * @return The length of the pulse (in microseconds) or 0 if no complete pulse was received within the timeout
 */
unsigned long pulseIn(uint8_t pinNumber, uint8_t state, unsigned long timeout) {
  return GPIO.pulseIn(pinNumber, state, timeout);
}

/**
 * @brief Alternative to pulseIn() which is better at handling long pulses and interrupt affected systems
 * @param pinNumber The number of the pin on which you want to read the pulse
 * @param state Type of pulse to read: either HIGH or LOW
 * @param timeout The number of microseconds to wait for the pulse to be completed (optional)
 * @return The length of the pulse (in microseconds) or 0 if no complete pulse was received within the timeout
 */
unsigned long pulseInLong(uint8_t pinNumber, uint8_t state,
                          unsigned long timeout) {
  return GPIO.pulseInLong(pinNumber, state, timeout);
}

/**
 * @brief Set the PWM frequency for analogWrite() on the specified pin
 * @param pin The pin number to configure PWM frequency for
 * @param freq The desired PWM frequency in Hz
 */
void analogWriteFrequency(pin_size_t pin, uint32_t freq) {
  GPIO.analogWriteFrequency(pin, freq);
}

/**
 * @brief Set the resolution of the analogWrite() values
 * @param bits The resolution in bits (e.g., 8 for 0-255, 10 for 0-1023)
 */
void analogWriteResolution(uint8_t bits) { 
  GPIO.analogWriteResolution(bits); 
}

/**
 * @brief Passes control to other tasks when called
 * @note This is used to prevent watchdog timer resets in long-running loops
 */
void yield() {}
