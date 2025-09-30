#pragma once

#include "HardwareGPIO.h"
#include "HardwareService.h"

namespace arduino {

class RemoteGPIO : public HardwareGPIO {
 public:
  RemoteGPIO() = default;
  RemoteGPIO(Stream* stream) { service.setStream(stream); }
  void setStream(Stream* stream) { service.setStream(stream); }

  void pinMode(pin_size_t pinNumber, PinMode pinMode) {
    service.send((uint16_t)GpioPinMode);
    service.send((int8_t)pinNumber);
    service.send((int8_t)pinMode);
    service.flush();
  }

  void digitalWrite(pin_size_t pinNumber, PinStatus status) {
    service.send((uint16_t)GpioDigitalWrite);
    service.send((uint8_t)pinNumber);
    service.send((uint8_t)status);
    service.flush();
  }

  PinStatus digitalRead(pin_size_t pinNumber) {
    service.send((uint16_t)GpioDigitalRead);
    service.send((uint8_t)pinNumber);
    service.flush();
    return (PinStatus)service.receive8();
  }

  int analogRead(pin_size_t pinNumber) {
    service.send((uint16_t)GpioAnalogRead);
    service.send((uint8_t)pinNumber);
    service.flush();
    return service.receive16();
  }

  void analogReference(uint8_t mode) {
    service.send((uint16_t)GpioAnalogReference);
    service.send(mode);
    service.flush();
  }

  void analogWrite(pin_size_t pinNumber, int value) {
    service.send((uint16_t)GpioAnalogWrite);
    service.send((uint8_t)pinNumber);
    service.send(value);
    service.flush();
  }

  virtual void tone(uint8_t pinNumber, unsigned int frequency,
                    unsigned long duration = 0) {
    service.send((uint16_t)GpioTone);
    service.send((uint8_t)pinNumber);
    service.send(frequency);
    service.send((uint64_t)duration);
    service.flush();
  }

  virtual void noTone(uint8_t pinNumber) {
    service.send((uint16_t)GpioNoTone);
    service.send((uint8_t)pinNumber);
    service.flush();
  }

  virtual unsigned long pulseIn(uint8_t pinNumber, uint8_t state,
                                unsigned long timeout = 1000000L) {
    service.send((uint16_t)GpioPulseIn);
    service.send((uint8_t)pinNumber);
    service.send(state);
    service.send((uint64_t)timeout);
    service.flush();
    return service.receive64();
  }

  virtual unsigned long pulseInLong(uint8_t pinNumber, uint8_t state,
                                    unsigned long timeout = 1000000L) {
    service.send((uint16_t)GpioPulseInLong);
    service.send((uint8_t)pinNumber);
    service.send(state);
    service.send((uint64_t)timeout);
    service.flush();
    return service.receive64();
  }

 protected:
  HardwareService service;
};

}  // namespace arduino
