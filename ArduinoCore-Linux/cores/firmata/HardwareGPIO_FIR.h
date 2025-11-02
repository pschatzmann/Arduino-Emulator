#pragma once
/*
  HardwareGPIO_FIR.h 
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
#ifdef USE_FIRMATA
// Undefine DEPRECATED macro from libftdi1 to avoid conflict with Arduino API
#ifdef DEPRECATED
#undef DEPRECATED
#endif
#include "HardwareGPIO.h"
#include <map>
#include <thread>
#include <atomic>
#include <mutex>
#include <chrono>

namespace arduino {

/**
 * @class HardwareGPIO_FIRMATA
 */
class HardwareGPIO_FIRMATA : public HardwareGPIO {
 public:
  /**
   * @brief Constructor for HardwareGPIO_FIRMATA.
   */
  HardwareGPIO_FIRMA() = default;

  /**
   * @brief Destructor for HardwareGPIO_FTDI.
   */
  ~HardwareGPIO_FIRMA();

  /**
   * @brief Initialize the GPIO hardware interface for FTDI FT2232HL.
   * @param vendor_id USB vendor ID (default: 0x0403)
   * @param product_id USB product ID (default: 0x6010 for FT2232HL)
   * @param description Device description string (optional)
   * @param serial Device serial number (optional)
   * @return true if initialization successful, false otherwise
   */
  bool begin(Stream &stream);

  /**
   * @brief Close the FTDI connection and cleanup resources.
   */
  void end();

  /**
   * @brief Set the mode of a GPIO pin (INPUT, OUTPUT, etc).
   * @param pinNumber Pin number (0-15 for FT2232HL)
   * @param pinMode Pin mode (INPUT, OUTPUT, INPUT_PULLUP)
   */
  void pinMode(pin_size_t pinNumber, PinMode pinMode) override;

  /**
   * @brief Write a digital value to a GPIO pin.
   * @param pinNumber Pin number (0-15)
   * @param status Pin status (HIGH or LOW)
   */
  void digitalWrite(pin_size_t pinNumber, PinStatus status) override;

  /**
   * @brief Read a digital value from a GPIO pin.
   * @param pinNumber Pin number (0-15)
   * @return Pin status (HIGH or LOW)
   */
  PinStatus digitalRead(pin_size_t pinNumber) override;

  /**
   * @brief Read an analog value from a pin (not supported by FT2232HL).
   * @param pinNumber Pin number
   * @return Always returns 0 (no ADC on FT2232HL)
   */
  int analogRead(pin_size_t pinNumber) override;

  /**
   * @brief Set the analog reference mode (not supported by FT2232HL).
   * @param mode Reference mode (ignored)
   */
  void analogReference(uint8_t mode) override;

  /**
   * @brief Write an analog value (PWM) to a pin using software PWM.
   * @param pinNumber Pin number (0-15)
   * @param value PWM duty cycle (0-255, where 0=0% and 255=100%)
   */
  void analogWrite(pin_size_t pinNumber, int value) override;

  /**
   * @brief Set the PWM frequency for analogWrite() on a specific pin.
   * @param pinNumber Pin number (0-15)
   * @param frequency PWM frequency in Hz (default: 1000Hz)
   */
  void analogWriteFrequency(pin_size_t pinNumber, uint32_t frequency);

  /**
   * @brief Generate a tone on a pin (not supported by FT2232HL).
   * @param _pin Pin number
   * @param frequency Frequency in Hz (ignored)
   * @param duration Duration in ms (ignored)
   */
  void tone(uint8_t _pin, unsigned int frequency,
            unsigned long duration = 0) override;

  /**
   * @brief Stop tone generation on a pin (not supported by FT2232HL).
   * @param _pin Pin number (ignored)
   */
  void noTone(uint8_t _pin) override;

  /**
   * @brief Measure pulse duration on a pin 
   * @param pin Pin number
   * @param state Pin state to measure
   * @param timeout Timeout in microseconds
   * @return Always returns 0 (not implemented)
   */
  unsigned long pulseIn(uint8_t pin, uint8_t state,
                        unsigned long timeout = 1000000L) override;

  /**
   * @brief Measure long pulse duration on a pin
   * @param pin Pin number
   * @param state Pin state to measure
   * @param timeout Timeout in microseconds
   * @return Always returns 0 (not implemented)
   */
  unsigned long pulseInLong(uint8_t pin, uint8_t state,
                            unsigned long timeout = 1000000L) override;

  /**
   * @brief Set the resolution for analogWrite() operations.
   * @param bits The resolution in bits (8-bit by default for FTDI)
   * @note FT2232HL supports 8-bit PWM resolution (0-255)
   */
  void analogWriteResolution(uint8_t bits) override;

  /**
   * @brief Boolean conversion operator.
   * @return true if the FTDI interface is open and initialized, false otherwise.
   */
  operator bool() { return is_open && ftdi_context != nullptr; }


 protected:
  bool is_open = false;
  
};

}  // namespace arduino

#endif  // USE_FTDI