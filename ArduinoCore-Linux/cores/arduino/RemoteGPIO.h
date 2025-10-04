/*
  RemoteGPIO.h 
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

namespace arduino {

/**
 * @brief Remote GPIO implementation that operates over a communication stream
 *
 * RemoteGPIO provides GPIO functionality by forwarding all operations to a remote
 * GPIO controller via a communication stream (serial, network, etc.). This enables
 * GPIO operations to be performed on remote hardware while maintaining the standard
 * HardwareGPIO interface.
 * 
 * Key features:
 * - Complete HardwareGPIO interface implementation
 * - Stream-based remote communication protocol
 * - Digital I/O operations (pinMode, digitalWrite, digitalRead)
 * - Analog I/O operations (analogRead, analogWrite, analogReference)
 * - PWM and tone generation (analogWrite, tone, noTone)
 * - Pulse measurement and timing functions (pulseIn, pulseInLong)
 * - Real-time bidirectional communication with remote GPIO hardware
 * 
 * The class uses HardwareService for protocol handling and can work with any
 * Stream implementation (Serial, TCP, etc.) for remote connectivity.
 *
 * @see HardwareGPIO
 * @see HardwareService
 * @see Stream
 */
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

  operator bool() { return service; }

 protected:
  HardwareService service;
};

}  // namespace arduino
