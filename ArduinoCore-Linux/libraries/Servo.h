#pragma once
/*
  Servo.h - Servo library for Raspberry Pi using pigpio
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

#if defined(USE_RPI)
#include <pigpio.h>
#include <stdint.h>
#include <algorithm>

/// Minimum and maximum pulse widths in microseconds (standard servo range)
#define SERVO_MIN_PULSE_US   544
#define SERVO_MAX_PULSE_US   2400
#define SERVO_DEFAULT_MIN_US 544
#define SERVO_DEFAULT_MAX_US 2400

/// Invalid pin sentinel
#define SERVO_NO_PIN  255

namespace arduino {

/**
 * @class Servo
 * @brief Arduino-compatible Servo library for Raspberry Pi using pigpio.
 *
 * Controls RC servo motors via GPIO PWM pulses (50 Hz, 500–2500 µs pulse width).
 * Follows the standard Arduino Servo API.
 *
 * Usage:
 * @code
 *   Servo myServo;
 *   myServo.attach(18);   // GPIO pin 18
 *   myServo.write(90);    // Move to 90 degrees
 * @endcode
 *
 * @note Requires pigpio to be initialized (gpioInitialise()) before use.
 *       On Raspberry Pi, pigpio must run as root or with appropriate permissions.
 */
class Servo {
 public:
  Servo()
      : _pin(SERVO_NO_PIN),
        _min(SERVO_DEFAULT_MIN_US),
        _max(SERVO_DEFAULT_MAX_US),
        _pulseUs((SERVO_DEFAULT_MIN_US + SERVO_DEFAULT_MAX_US) / 2) {}

  ~Servo() {
    if (attached()) detach();
  }

  /**
   * @brief Attach the servo to a GPIO pin using default pulse range.
   * @param pin GPIO pin number (BCM numbering).
   * @return 0 on success.
   */
  uint8_t attach(int pin) {
    return attach(pin, SERVO_DEFAULT_MIN_US, SERVO_DEFAULT_MAX_US);
  }

  /**
   * @brief Attach the servo to a GPIO pin with custom min/max pulse widths.
   * @param pin GPIO pin number (BCM numbering).
   * @param min Minimum pulse width in microseconds (corresponds to 0°).
   * @param max Maximum pulse width in microseconds (corresponds to 180°).
   * @return 0 on success.
   */
  uint8_t attach(int pin, int min, int max) {
    _pin = pin;
    _min = std::max(min, 500);   // pigpio lower bound
    _max = std::min(max, 2500);  // pigpio upper bound
    gpioSetMode(_pin, PI_OUTPUT);
    gpioServo(_pin, _pulseUs);
    return 0;
  }

  /**
   * @brief Detach the servo from its pin and stop PWM output.
   */
  void detach() {
    if (attached()) {
      gpioServo(_pin, 0);  // 0 = stop pulses
      _pin = SERVO_NO_PIN;
    }
  }

  /**
   * @brief Write a position to the servo.
   * @param value Angle in degrees (0–180), or pulse width in µs if > 200.
   */
  void write(int value) {
    if (value <= 200) {
      value = std::max(0, std::min(value, 180));
      writeMicroseconds(angleToPulse(value));
    } else {
      writeMicroseconds(value);
    }
  }

  /**
   * @brief Write a pulse width directly in microseconds.
   * @param us Pulse width in microseconds (typically 500–2500).
   */
  void writeMicroseconds(int us) {
    _pulseUs = std::max(_min, std::min(us, _max));
    if (attached()) gpioServo(_pin, _pulseUs);
  }

  /**
   * @brief Read the current servo position in degrees.
   * @return Angle in degrees (0–180).
   */
  int read() const {
    return pulseToAngle(_pulseUs);
  }

  /**
   * @brief Read the current pulse width in microseconds.
   * @return Pulse width in microseconds.
   */
  int readMicroseconds() const {
    return _pulseUs;
  }

  /**
   * @brief Check if the servo is attached to a pin.
   * @return true if attached, false otherwise.
   */
  bool attached() const {
    return _pin != SERVO_NO_PIN;
  }

 private:
  int _pin;      ///< GPIO pin (BCM), SERVO_NO_PIN if unattached
  int _min;      ///< Minimum pulse width in µs
  int _max;      ///< Maximum pulse width in µs
  int _pulseUs;  ///< Current pulse width in µs

  /// Map angle (0–180) to pulse width in µs
  int angleToPulse(int angle) const {
    return _min + (angle * (_max - _min)) / 180;
  }

  /// Map pulse width in µs to angle (0–180)
  int pulseToAngle(int us) const {
    if (_max == _min) return 0;
    int angle = ((us - _min) * 180) / (_max - _min);
    return std::max(0, std::min(angle, 180));
  }
};

} // namespace arduino

// Pull into global namespace for Arduino sketch compatibility
using arduino::Servo;

#endif // USE_RPI
