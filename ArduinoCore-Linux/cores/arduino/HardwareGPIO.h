#pragma once

#include "api/Common.h"

namespace arduino {

/**
 * @brief Abstract base class for GPIO (General Purpose Input/Output) functions
 *
 * HardwareGPIO defines the interface for hardware abstraction of GPIO operations
 * across different platforms and microcontrollers. This class provides a unified
 * API for digital and analog I/O operations that can be implemented by platform-specific
 * classes.
 *
 * The class supports:
 * - Digital pin operations (pinMode, digitalWrite, digitalRead)
 * - Analog operations (analogRead, analogWrite, analogReference)
 * - PWM and tone generation (analogWrite, tone, noTone)
 * - Pulse measurement (pulseIn, pulseInLong)
 *
 * Platform-specific implementations should inherit from this class and provide
 * concrete implementations for all pure virtual methods. Examples include
 * HardwareGPIO_RPI for Raspberry Pi, or similar classes for other platforms.
 *
 * @note All methods in this class are pure virtual and must be implemented
 *       by derived classes to provide platform-specific functionality.
 *
 * @see HardwareGPIO_RPI
 * @see PinMode
 * @see PinStatus
 *
 */
class HardwareGPIO {
 public:
  HardwareGPIO() = default;
  virtual ~HardwareGPIO() = default;

  /**
   * @brief Configure the specified pin to behave as an input or output
   * @param pinNumber The pin number to configure
   * @param pinMode The mode to set (INPUT, OUTPUT, INPUT_PULLUP, etc.)
   */
  virtual void pinMode(pin_size_t pinNumber, PinMode pinMode) = 0;

  /**
   * @brief Write a HIGH or LOW value to a digital pin
   * @param pinNumber The pin number to write to
   * @param status The value to write (HIGH or LOW)
   */
  virtual void digitalWrite(pin_size_t pinNumber, PinStatus status) = 0;

  /**
   * @brief Read the value from a specified digital pin
   * @param pinNumber The pin number to read from
   * @return HIGH or LOW
   */
  virtual PinStatus digitalRead(pin_size_t pinNumber) = 0;

  /**
   * @brief Read the value from the specified analog pin
   * @param pinNumber The analog pin to read from (A0, A1, etc.)
   * @return The analog reading on the pin (0-1023 for 10-bit ADC)
   */
  virtual int analogRead(pin_size_t pinNumber) = 0;

  /**
   * @brief Configure the reference voltage used for analog input
   * @param mode The reference type (DEFAULT, INTERNAL, EXTERNAL, etc.)
   */
  virtual void analogReference(uint8_t mode) = 0;

  /**
   * @brief Write an analog value (PWM wave) to a pin
   * @param pinNumber The pin to write to
   * @param value The duty cycle (0-255 for 8-bit PWM)
   */
  virtual void analogWrite(pin_size_t pinNumber, int value) = 0;

  /**
   * @brief Generate a square wave of the specified frequency on a pin
   * @param _pin The pin on which to generate the tone
   * @param frequency The frequency of the tone in hertz
   * @param duration The duration of the tone in milliseconds (0 = continuous)
   */
  virtual void tone(uint8_t _pin, unsigned int frequency, unsigned long duration = 0) = 0;

  /**
   * @brief Stop the generation of a square wave triggered by tone()
   * @param _pin The pin on which to stop generating the tone
   */
  virtual void noTone(uint8_t _pin) = 0;

  /**
   * @brief Read a pulse (HIGH or LOW) on a pin
   * @param pin The pin on which you want to read the pulse
   * @param state Type of pulse to read (HIGH or LOW)
   * @param timeout Timeout in microseconds (default 1 second)
   * @return The length of the pulse in microseconds, or 0 if timeout
   */
  virtual unsigned long pulseIn(uint8_t pin, uint8_t state, unsigned long timeout = 1000000L) = 0;

  /**
   * @brief Alternative to pulseIn() which is better at handling long pulses
   * @param pin The pin on which you want to read the pulse
   * @param state Type of pulse to read (HIGH or LOW)
   * @param timeout Timeout in microseconds (default 1 second)
   * @return The length of the pulse in microseconds, or 0 if timeout
   */
  virtual unsigned long pulseInLong(uint8_t pin, uint8_t state, unsigned long timeout = 1000000L) = 0;
};

}  // namespace arduino
