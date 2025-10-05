/*
        GPIOWrapper.h
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

#pragma once
#include "HardwareGPIO.h"
#include "HardwareService.h"
#include "Sources.h"

namespace arduino {

/**
 * @brief GPIO wrapper class that provides flexible hardware abstraction
 *
 * GPIOWrapper is a concrete implementation of the HardwareGPIO interface that
 * supports multiple delegation patterns for GPIO operations. It can delegate
 * operations to:
 * - An injected HardwareGPIO implementation
 * - A GPIOSource provider that supplies the GPIO implementation
 * - A default fallback implementation
 *
 * This class implements the complete GPIO interface including:
 * - Digital I/O operations (pinMode, digitalWrite, digitalRead)
 * - Analog I/O operations (analogRead, analogWrite, analogReference)
 * - PWM and tone generation (analogWrite, tone, noTone)
 * - Pulse measurement and timing functions (pulseIn, pulseInLong)
 * - Pin mode configuration for input, output, and special modes
 *
 * The wrapper automatically handles null safety and provides appropriate
 * default return values when no underlying GPIO implementation is available. It
 * supports all standard Arduino GPIO operations with hardware abstraction.
 *
 * A global `GPIO` instance is automatically provided for system-wide GPIO
 * access.
 *
 * @see HardwareGPIO
 * @see GPIOSource
 */
class GPIOWrapper : public HardwareGPIO {
 public:
  GPIOWrapper() = default;
  GPIOWrapper(GPIOSource& source) { setSource(&source); }
  GPIOWrapper(HardwareGPIO& gpio) { setGPIO(&gpio); }
  ~GPIOWrapper() = default;
  
  /**
   * @brief Configure the digital pin mode for input/output operations
   * @param pinNumber The pin number to configure
   * @param pinMode The pin mode (INPUT, OUTPUT, INPUT_PULLUP, etc.)
   */
  void pinMode(pin_size_t pinNumber, PinMode pinMode);
  
  /**
   * @brief Write a digital value to a pin
   * @param pinNumber The pin number to write to
   * @param status The digital value to write (HIGH or LOW)
   */
  void digitalWrite(pin_size_t pinNumber, PinStatus status);
  
  /**
   * @brief Read the digital value from a pin
   * @param pinNumber The pin number to read from
   * @return The digital value (HIGH or LOW), returns LOW if no GPIO available
   */
  PinStatus digitalRead(pin_size_t pinNumber);
  
  /**
   * @brief Read an analog value from a pin
   * @param pinNumber The pin number to read from
   * @return The analog value (0-1023), returns 0 if no GPIO available
   */
  int analogRead(pin_size_t pinNumber);
  
  /**
   * @brief Set the analog reference voltage for ADC measurements
   * @param mode The reference mode (DEFAULT, EXTERNAL, etc.)
   */
  void analogReference(uint8_t mode);
  
  /**
   * @brief Write an analog value (PWM) to a pin
   * @param pinNumber The pin number to write to
   * @param value The analog value (0-255 for 8-bit resolution)
   */
  void analogWrite(pin_size_t pinNumber, int value);
  
  /**
   * @brief Generate a square wave tone on a pin
   * @param _pin The pin number to generate tone on
   * @param frequency The tone frequency in Hz
   * @param duration The duration in milliseconds (0 for continuous)
   */
  void tone(uint8_t _pin, unsigned int frequency, unsigned long duration = 0);
  
  /**
   * @brief Stop tone generation on a pin
   * @param _pin The pin number to stop tone on
   */
  void noTone(uint8_t _pin);
  
  /**
   * @brief Measure the duration of a pulse on a pin (microseconds precision)
   * @param pin The pin number to measure
   * @param state The pulse state to measure (HIGH or LOW)
   * @param timeout Maximum time to wait for pulse in microseconds
   * @return Pulse duration in microseconds, 0 on timeout or no GPIO
   */
  unsigned long pulseIn(uint8_t pin, uint8_t state,
                        unsigned long timeout = 1000000L);
  
  /**
   * @brief Measure long pulse duration on a pin (microseconds precision)
   * @param pin The pin number to measure
   * @param state The pulse state to measure (HIGH or LOW)  
   * @param timeout Maximum time to wait for pulse in microseconds
   * @return Pulse duration in microseconds, 0 on timeout or no GPIO
   */
  unsigned long pulseInLong(uint8_t pin, uint8_t state,
                            unsigned long timeout = 1000000L);
  
  /**
   * @brief Set the PWM frequency for a pin (legacy 8-bit interface)
   * @param pin The pin number to configure
   * @param freq The PWM frequency in Hz
   */
  void analogWriteFrequency(pin_size_t pin, uint32_t freq);
  
  /**
   * @brief Set the resolution for analog write operations
   * @param bits The resolution in bits (8-16 bits typically)
   */
  void analogWriteResolution(uint8_t bits);
  
  /**
   * @brief Set the GPIO implementation directly
   * @param gpio Pointer to HardwareGPIO implementation (use nullptr to reset)
   */
  void setGPIO(HardwareGPIO* gpio) {
    p_gpio = gpio;
    p_source = nullptr;
  }
  
  /**
   * @brief Set a GPIO source that provides the GPIO implementation
   * @param source Pointer to GPIOSource that provides HardwareGPIO
   */
  void setSource(GPIOSource* source) {
    p_source = source;
    p_gpio = nullptr;
  }

 protected:
  HardwareGPIO* p_gpio = nullptr;
  GPIOSource* p_source = nullptr;

  HardwareGPIO* getGPIO() {
    HardwareGPIO* result = p_gpio;
    if (result == nullptr && p_source != nullptr) {
      result = p_source->getGPIO();
    }
    return result;
  }
};

/// Global GPIO instance used by Arduino API functions and direct access
extern GPIOWrapper GPIO;

}  // namespace arduino
