#pragma once

#include "api/Common.h"

namespace arduino {

/**
 * Abstract base class for GPIO functions
 **/
class HardwareGPIO {
 public:
  HardwareGPIO() = default;
  virtual ~HardwareGPIO() = default;

  virtual void pinMode(pin_size_t pinNumber, PinMode pinMode) = 0;
  virtual void digitalWrite(pin_size_t pinNumber, PinStatus status) = 0;
  virtual PinStatus digitalRead(pin_size_t pinNumber) = 0;
  virtual int analogRead(pin_size_t pinNumber) = 0;
  virtual void analogReference(uint8_t mode) = 0;
  virtual void analogWrite(pin_size_t pinNumber, int value) = 0;
  virtual void tone(uint8_t _pin, unsigned int frequency, unsigned long duration = 0) = 0;
  virtual void noTone(uint8_t _pin) = 0;
  virtual unsigned long pulseIn(uint8_t pin, uint8_t state, unsigned long timeout = 1000000L) = 0;
  virtual unsigned long pulseInLong(uint8_t pin, uint8_t state, unsigned long timeout = 1000000L) = 0;
};

}  // namespace arduino
