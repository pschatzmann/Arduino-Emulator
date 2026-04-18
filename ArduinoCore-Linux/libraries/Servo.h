#pragma once
/*
  Servo.h - Servo library for Raspberry Pi using HardwareGPIO_RPI (sysfs PWM)
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
#include <stdint.h>
#include <algorithm>
#include <cstdio>
#include "HardwareGPIO_RPI.h"

/// Standard servo pulse range in microseconds
#define SERVO_DEFAULT_MIN_US  544
#define SERVO_DEFAULT_MAX_US  2400

/// Servo PWM frequency: 50 Hz => 20,000,000 ns period
#define SERVO_PERIOD_NS       20000000UL

/// Invalid pin sentinel
#define SERVO_NO_PIN  255

namespace arduino {

/**
 * @class Servo
 * @brief Arduino-compatible Servo library for Raspberry Pi.
 *
 * Uses the Linux sysfs PWM interface via HardwareGPIO_RPI.
 * Supports hardware PWM pins: 12, 13, 18, 19 (BCM numbering).
 *
 * The servo period is fixed at 50 Hz (20 ms). Pulse widths in the range
 * SERVO_DEFAULT_MIN_US–SERVO_DEFAULT_MAX_US map to 0–180 degrees.
 *
 * @note Call gpio.begin() on your HardwareGPIO_RPI instance before
 *       attaching any servos.
 *
 * Usage:
 * @code
 *   HardwareGPIO_RPI gpio;
 *   gpio.begin();
 *   Servo myServo(gpio);
 *   myServo.attach(18);
 *   myServo.write(90);
 * @endcode
 */
class Servo {
 public:
  /**
   * @brief Constructor. Requires a reference to an initialised HardwareGPIO_RPI.
   */
  Servo(HardwareGPIO_RPI& gpio)
      : _gpio(gpio),
        _pin(SERVO_NO_PIN),
        _min(SERVO_DEFAULT_MIN_US),
        _max(SERVO_DEFAULT_MAX_US),
        _pulseUs((SERVO_DEFAULT_MIN_US + SERVO_DEFAULT_MAX_US) / 2) {}

  ~Servo() {
    if (attached()) detach();
  }

  /**
   * @brief Attach the servo to a GPIO pin using default pulse range.
   * @param pin GPIO pin number (BCM), must be a hardware PWM pin (12,13,18,19).
   * @return 0 on success.
   */
  uint8_t attach(int pin) {
    return attach(pin, SERVO_DEFAULT_MIN_US, SERVO_DEFAULT_MAX_US);
  }

  /**
   * @brief Attach the servo to a GPIO pin with custom min/max pulse widths.
   * @param pin GPIO pin number (BCM), must be a hardware PWM pin (12,13,18,19).
   * @param min Minimum pulse width in microseconds (corresponds to 0°).
   * @param max Maximum pulse width in microseconds (corresponds to 180°).
   * @return 0 on success.
   */
  uint8_t attach(int pin, int min, int max) {
    _pin = pin;
    _min = min;
    _max = max;
    _gpio.pinMode(_pin, OUTPUT);
    // Fix PWM frequency to 50 Hz for servo control
    _gpio.analogWriteFrequency(_pin, 50);
    // Write current position
    _writePulse(_pulseUs);
    return 0;
  }

  /**
   * @brief Detach the servo: set duty cycle to 0 and release the pin.
   */
  void detach() {
    if (attached()) {
      _writePulse(0);
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
      writeMicroseconds(_angleToPulse(value));
    } else {
      writeMicroseconds(value);
    }
  }

  /**
   * @brief Write a pulse width directly in microseconds.
   * @param us Pulse width in microseconds.
   */
  void writeMicroseconds(int us) {
    _pulseUs = std::max(_min, std::min(us, _max));
    if (attached()) _writePulse(_pulseUs);
  }

  /**
   * @brief Read the current servo position in degrees (0–180).
   */
  int read() const {
    return _pulseToAngle(_pulseUs);
  }

  /**
   * @brief Read the current pulse width in microseconds.
   */
  int readMicroseconds() const {
    return _pulseUs;
  }

  /**
   * @brief Check if the servo is attached to a pin.
   */
  bool attached() const {
    return _pin != SERVO_NO_PIN;
  }

 private:
  HardwareGPIO_RPI& _gpio;
  int _pin;
  int _min;
  int _max;
  int _pulseUs;

  /// Map angle (0–180°) to pulse width in µs
  int _angleToPulse(int angle) const {
    return _min + (angle * (_max - _min)) / 180;
  }

  /// Map pulse width in µs to angle (0–180°)
  int _pulseToAngle(int us) const {
    if (_max == _min) return 0;
    int angle = ((us - _min) * 180) / (_max - _min);
    return std::max(0, std::min(angle, 180));
  }

  /**
   * @brief Write pulse width directly to sysfs PWM duty_cycle.
   *
   * HardwareGPIO_RPI::analogWrite() maps a 0–max_value integer to a
   * duty_cycle fraction of the period. For servo control we need the
   * duty cycle expressed as a fraction of the 20 ms period.
   *
   * We bypass analogWrite() and write the duty_cycle sysfs file directly
   * so we can specify an exact nanosecond value.
   *
   * @param us Pulse width in microseconds (0 = off).
   */
  void _writePulse(int us) {
    // Determine sysfs PWM channel for the pin
    int pwm_channel = -1;
    if (_pin == 18 || _pin == 12) pwm_channel = 0;
    else if (_pin == 19 || _pin == 13) pwm_channel = 1;
    if (pwm_channel < 0) return;

    // Ensure channel is exported
    char export_path[64];
    snprintf(export_path, sizeof(export_path),
             "/sys/class/pwm/pwmchip0/export");
    FILE* fexp = fopen(export_path, "w");
    if (fexp) { fprintf(fexp, "%d", pwm_channel); fclose(fexp); }

    char period_path[64], duty_path[64], enable_path[64];
    snprintf(period_path, sizeof(period_path),
             "/sys/class/pwm/pwmchip0/pwm%d/period", pwm_channel);
    snprintf(duty_path, sizeof(duty_path),
             "/sys/class/pwm/pwmchip0/pwm%d/duty_cycle", pwm_channel);
    snprintf(enable_path, sizeof(enable_path),
             "/sys/class/pwm/pwmchip0/pwm%d/enable", pwm_channel);

    // Set period to 20,000,000 ns (50 Hz)
    FILE* fp = fopen(period_path, "w");
    if (fp) { fprintf(fp, "%lu", SERVO_PERIOD_NS); fclose(fp); }

    // Convert µs to ns and write duty cycle
    unsigned long duty_ns = (us > 0) ? (unsigned long)us * 1000UL : 0UL;
    FILE* fd = fopen(duty_path, "w");
    if (fd) { fprintf(fd, "%lu", duty_ns); fclose(fd); }

    // Enable (or disable if us==0)
    FILE* fe = fopen(enable_path, "w");
    if (fe) { fprintf(fe, "%d", us > 0 ? 1 : 0); fclose(fe); }
  }
};

} // namespace arduino

// Pull into global namespace for Arduino sketch compatibility
using arduino::Servo;

#endif // USE_RPI
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
