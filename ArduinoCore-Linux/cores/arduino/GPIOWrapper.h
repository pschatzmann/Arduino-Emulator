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
  void pinMode(pin_size_t pinNumber, PinMode pinMode);
  void digitalWrite(pin_size_t pinNumber, PinStatus status);
  PinStatus digitalRead(pin_size_t pinNumber);
  int analogRead(pin_size_t pinNumber);
  void analogReference(uint8_t mode);
  void analogWrite(pin_size_t pinNumber, int value);
  void tone(uint8_t _pin, unsigned int frequency, unsigned long duration = 0);
  void noTone(uint8_t _pin);
  unsigned long pulseIn(uint8_t pin, uint8_t state,
                        unsigned long timeout = 1000000L);
  unsigned long pulseInLong(uint8_t pin, uint8_t state,
                            unsigned long timeout = 1000000L);

  /// defines the gpio implementation: use nullptr to reset.
  void setGPIO(HardwareGPIO* gpio) {
    p_gpio = gpio;
    p_source = nullptr;
  }
  /// alternatively defines a class that provides the GPIO implementation
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
static GPIOWrapper GPIO;

}  // namespace arduino
