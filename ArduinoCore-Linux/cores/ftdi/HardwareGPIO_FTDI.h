#pragma once
/*
  HardwareGPIO_FTDI.h 
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
#ifdef USE_FTDI
#include <ftdi.h>
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
 * @class HardwareGPIO_FTDI
 * @brief GPIO hardware abstraction for FTDI FT2232HL in the Arduino emulator.
 *
 * This class implements the Arduino-style GPIO interface for FTDI FT2232HL devices, allowing
 * digital I/O operations to be used in a manner compatible with Arduino code. The FT2232HL
 * provides 16 GPIO pins (8 per channel) that can be configured for digital input/output.
 *
 * Pin mapping for FT2232HL:
 * - Channel A (ADBUS): Pins 0-7 (ADBUS0-ADBUS7)
 * - Channel B (BDBUS): Pins 8-15 (BDBUS0-BDBUS7)
 *
 * The class inherits from HardwareGPIO and is intended for use within the emulator when 
 * communicating with FTDI FT2232HL devices. It manages pin state and direction for supported pins.
 *
 * @note This class is only available when USE_FTDI is defined.
 * @note Requires libftdi1 development library to be installed.
 */
class HardwareGPIO_FTDI : public HardwareGPIO {
 public:
  /**
   * @brief Constructor for HardwareGPIO_FTDI.
   */
  HardwareGPIO_FTDI() = default;

  /**
   * @brief Destructor for HardwareGPIO_FTDI.
   */
  ~HardwareGPIO_FTDI();

  /**
   * @brief Initialize the GPIO hardware interface for FTDI FT2232HL.
   * @param vendor_id USB vendor ID (default: 0x0403)
   * @param product_id USB product ID (default: 0x6010 for FT2232HL)
   * @param description Device description string (optional)
   * @param serial Device serial number (optional)
   * @return true if initialization successful, false otherwise
   */
  bool begin(int vendor_id = 0x0403, int product_id = 0x6010, 
            const char* description = nullptr, const char* serial = nullptr);

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
   * @brief Measure pulse duration on a pin (not supported by FT2232HL).
   * @param pin Pin number
   * @param state Pin state to measure
   * @param timeout Timeout in microseconds
   * @return Always returns 0 (not implemented)
   */
  unsigned long pulseIn(uint8_t pin, uint8_t state,
                        unsigned long timeout = 1000000L) override;

  /**
   * @brief Measure long pulse duration on a pin (not supported by FT2232HL).
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

  /**
   * @brief Get PWM statistics for monitoring timing accuracy.
   * @param pin Pin number
   * @param cycles Total number of PWM cycles completed
   * @param max_jitter_us Maximum jitter observed in microseconds
   * @param avg_jitter_us Average jitter in microseconds
   */
  void getPWMStatistics(pin_size_t pin, uint64_t& cycles, 
                       uint64_t& max_jitter_us, uint64_t& avg_jitter_us);

 protected:
  struct ftdi_context* ftdi_context = nullptr;
  bool is_open = false;
  
  // Pin state tracking
  uint8_t pin_directions_a = 0x00;  // Channel A direction mask (1=output, 0=input)
  uint8_t pin_values_a = 0x00;      // Channel A output values
  uint8_t pin_directions_b = 0x00;  // Channel B direction mask
  uint8_t pin_values_b = 0x00;      // Channel B output values
  
  // Pin mode tracking
  std::map<pin_size_t, PinMode> pin_modes;
  
  // PWM state tracking
  struct PWMPin {
    bool enabled = false;
    uint8_t duty_cycle = 0;     // 0-255
    uint32_t frequency = 1000;  // Hz
    uint32_t period_us = 1000;  // Period in microseconds
    uint32_t on_time_us = 0;    // On time in microseconds
    std::chrono::high_resolution_clock::time_point period_start;  // Start of current period
    bool current_state = false;
    uint64_t cycle_count = 0;   // Total cycles for drift correction
    // Statistics for monitoring
    uint64_t max_jitter_us = 0;
    uint64_t total_jitter_us = 0;
  };
  
  std::map<pin_size_t, PWMPin> pwm_pins;
  std::thread pwm_thread;
  std::atomic<bool> pwm_thread_running{false};
  std::mutex pwm_mutex;

  /**
   * @brief Update the GPIO state on the FTDI device.
   * @param channel Channel to update (0 for A, 1 for B)
   * @return true if successful, false on error
   */
  bool updateGPIOState(int channel);

  /**
   * @brief Read current GPIO state from the FTDI device.
   * @param channel Channel to read (0 for A, 1 for B)
   * @param value Reference to store the read value
   * @return true if successful, false on error
   */
  bool readGPIOState(int channel, uint8_t& value);

  /**
   * @brief Get the channel number for a given pin.
   * @param pin Pin number (0-15)
   * @return Channel number (0 for pins 0-7, 1 for pins 8-15)
   */
  int getChannel(pin_size_t pin) { return (pin >= 8) ? 1 : 0; }

  /**
   * @brief Get the bit position within a channel for a given pin.
   * @param pin Pin number (0-15)
   * @return Bit position (0-7)
   */
  int getBitPosition(pin_size_t pin) { return pin % 8; }
  
  /**
   * @brief PWM thread function that handles software PWM generation.
   */
  void pwmThreadFunction();
  
  /**
   * @brief Start the PWM background thread.
   */
  void startPWMThread();
  
  /**
   * @brief Stop the PWM background thread.
   */
  void stopPWMThread();
  
  /**
   * @brief Update PWM pin configuration.
   * @param pin Pin number
   * @param duty_cycle Duty cycle (0-255)
   * @param frequency Frequency in Hz
   */
  void updatePWMPin(pin_size_t pin, uint8_t duty_cycle, uint32_t frequency = 1000);
};

}  // namespace arduino

#endif  // USE_FTDI