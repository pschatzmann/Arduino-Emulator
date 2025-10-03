#include "GPIOWrapper.h"

#include "HardwareGPIO.h"
#include "HardwareService.h"

/**
 * We support different implementations for the GPIO
 */

namespace arduino {

void GPIOWrapper::pinMode(pin_size_t pinNumber, PinMode pinMode) {
  HardwareGPIO* gpio = getGPIO();
  if (gpio != nullptr) {
    gpio->pinMode(pinNumber, pinMode);
  }
}

void GPIOWrapper::digitalWrite(pin_size_t pinNumber, PinStatus status) {
  HardwareGPIO* gpio = getGPIO();
  if (gpio != nullptr) {
    gpio->digitalWrite(pinNumber, status);
  }
}

PinStatus GPIOWrapper::digitalRead(pin_size_t pinNumber) {
  HardwareGPIO* gpio = getGPIO();
  if (gpio != nullptr) {
    return gpio->digitalRead(pinNumber);
  } else {
    return LOW;
  }
}

int GPIOWrapper::analogRead(pin_size_t pinNumber) {
  HardwareGPIO* gpio = getGPIO();
  if (gpio != nullptr) {
    return gpio->analogRead(pinNumber);
  } else {
    return 0;
  }
}

void GPIOWrapper::analogReference(uint8_t mode) {
  HardwareGPIO* gpio = getGPIO();
  if (gpio != nullptr) {
    gpio->analogReference(mode);
  }
}

void GPIOWrapper::analogWrite(pin_size_t pinNumber, int value) {
  HardwareGPIO* gpio = getGPIO();
  if (gpio != nullptr) {
    gpio->analogWrite(pinNumber, value);
  }
}

void GPIOWrapper::tone(uint8_t _pin, unsigned int frequency,
                       unsigned long duration) {
  HardwareGPIO* gpio = getGPIO();
  if (gpio != nullptr) {
    gpio->tone(_pin, frequency, duration);
  }
}

void GPIOWrapper::noTone(uint8_t _pin) {
  HardwareGPIO* gpio = getGPIO();
  if (gpio != nullptr) {
    gpio->noTone(_pin);
  }
}

unsigned long GPIOWrapper::pulseIn(uint8_t pin, uint8_t state,
                                   unsigned long timeout) {
  HardwareGPIO* gpio = getGPIO();
  if (gpio != nullptr) {
    return gpio->pulseIn(pin, state, timeout);
  } else {
    return 0;
  }
}

unsigned long GPIOWrapper::pulseInLong(uint8_t pin, uint8_t state,
                                       unsigned long timeout) {
  HardwareGPIO* gpio = getGPIO();
  if (gpio != nullptr) {
    return gpio->pulseInLong(pin, state, timeout);
  } else {
    return 0;
  }
}

}  // namespace arduino
