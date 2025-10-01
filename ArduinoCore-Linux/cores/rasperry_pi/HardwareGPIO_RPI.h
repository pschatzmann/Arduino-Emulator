#pragma once
#ifdef USE_RPI
#include "HardwareGPIO.h"


namespace arduino {

/**
 * @class HardwareGPIO_RPI
 * @brief GPIO hardware abstraction for Raspberry Pi in the Arduino emulator.
 *
 * This class implements the Arduino-style GPIO interface for Raspberry Pi platforms, allowing
 * digital and analog I/O, PWM, and timing functions to be used in a manner compatible with Arduino code.
 * It provides methods to configure pin modes, read and write digital/analog values, generate PWM signals,
 * and perform timing operations such as pulse measurement and tone generation.
 *
 * The class inherits from HardwareGPIO and is intended for use within the emulator when running on
 * Raspberry Pi hardware. It manages pin state, analog reference, and PWM frequency settings for supported pins.
 *
 * @note This class is only available when USE_RPI is defined.
 */
class HardwareGPIO_RPI : public HardwareGPIO {
 public:
  /**
   * @brief Constructor for HardwareGPIO_RPI.
   */
  HardwareGPIO_RPI() = default;
  HardwareGPIO_RPI(const char* devName) : device_name(devName) {}

  /**
   * @brief Destructor for HardwareGPIO_RPI.
   */
  ~HardwareGPIO_RPI();

  void begin();

  /**
   * @brief Set the mode of a GPIO pin (INPUT, OUTPUT, etc).
   */
  void pinMode(pin_size_t pinNumber, PinMode pinMode) override;

  /**
   * @brief Write a digital value to a GPIO pin.
   */
  void digitalWrite(pin_size_t pinNumber, PinStatus status) override;

  /**
   * @brief Read a digital value from a GPIO pin.
   */
  PinStatus digitalRead(pin_size_t pinNumber) override;

  /**
   * @brief Read an analog value from a pin (if supported).
   */
  int analogRead(pin_size_t pinNumber) override;

  /**
   * @brief Set the analog reference mode.
   */
  void analogReference(uint8_t mode) override;

  /**
   * @brief Write an analog value (PWM) to a pin.
   */
  void analogWrite(pin_size_t pinNumber, int value) override;

  /**
   * @brief Generate a tone on a pin.
   * @param _pin Pin number
   * @param frequency Frequency in Hz
   * @param duration Duration in ms (optional)
   */
  void tone(uint8_t _pin, unsigned int frequency,
            unsigned long duration = 0) override;

  /**
   * @brief Stop tone generation on a pin.
   */
  void noTone(uint8_t _pin) override;

  /**
   * @brief Measure pulse duration on a pin.
   * @param pin Pin number
   * @param state Pin state to measure
   * @param timeout Timeout in microseconds (default 1000000)
   */
  unsigned long pulseIn(uint8_t pin, uint8_t state,
                        unsigned long timeout = 1000000L) override;

  /**
   * @brief Measure long pulse duration on a pin.
   * @param pin Pin number
   * @param state Pin state to measure
   * @param timeout Timeout in microseconds (default 1000000)
   */
  unsigned long pulseInLong(uint8_t pin, uint8_t state,
                            unsigned long timeout = 1000000L) override;

  /**
   * @brief Set PWM frequency for a pin.
   */
  void analogWriteFrequency(uint8_t pin, uint32_t freq);

  operator bool() { return is_open; }

 private:
  /** Analog reference mode (default AR_DEFAULT) */
  int m_analogReference = 0;
  /** Map of pin numbers to their PWM frequencies */
  std::map<pin_size_t, uint32_t> gpio_frequencies;
  /** PWM-capable pins on Raspberry Pi */
  int pwm_pins[4] = {12, 13, 18, 19};
  bool is_open = false;
  const char* device_name = "gpiochip0"
};

}  // namespace arduino

#endif