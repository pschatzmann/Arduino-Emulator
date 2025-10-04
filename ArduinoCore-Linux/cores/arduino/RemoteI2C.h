#pragma once
#include "Stream.h"
#include "api/HardwareI2C.h"
#include "HardwareService.h"

namespace arduino {

/**
 * @brief Remote I2C implementation that operates over a communication stream
 *
 * RemoteI2C provides I2C functionality by forwarding all operations to a remote
 * I2C controller via a communication stream (serial, network, etc.). This enables
 * I2C operations to be performed on remote hardware while maintaining the standard
 * HardwareI2C interface.
 * 
 * Key features:
 * - Complete HardwareI2C interface implementation
 * - Stream-based remote communication protocol
 * - Automatic command serialization and response handling
 * - Support for all I2C operations (master/slave, read/write, transactions)
 * - Real-time bidirectional communication with remote I2C hardware
 * 
 * The class uses HardwareService for protocol handling and can work with any
 * Stream implementation (Serial, TCP, etc.) for remote connectivity.
 *
 * @see HardwareI2C
 * @see HardwareService
 * @see Stream
 */
class RemoteI2C : public HardwareI2C {
 public:
  RemoteI2C() = default;
  RemoteI2C(Stream* stream) { service.setStream(static_cast<Stream*>(stream)); }
  void setStream(Stream* stream) {
    service.setStream(static_cast<Stream*>(stream));
  }

  virtual void begin() {
    service.send(I2cBegin0);
    service.flush();
  }

  virtual void begin(uint8_t address) {
    service.send(I2cBegin1);
    service.send(address);
    service.flush();
  }
  virtual void end() {
    service.send(I2cEnd);
    service.flush();
  }

  virtual void setClock(uint32_t freq) {
    service.send(I2cSetClock);
    service.send(freq);
    service.flush();
  }

  virtual void beginTransmission(uint8_t address) {
    service.send(I2cBeginTransmission);
    service.send(address);
    service.flush();
  }

  virtual uint8_t endTransmission(bool stopBit) {
    service.send(I2cEndTransmission1);
    service.send(stopBit);
    return service.receive8();
  }

  virtual uint8_t endTransmission(void) {
    service.send(I2cEndTransmission);
    return service.receive8();
  }

  virtual size_t requestFrom(uint8_t address, size_t len, bool stopBit) {
    service.send(I2cRequestFrom3);
    service.send(address);
    service.send((uint64_t)len);
    service.send(stopBit);
    return service.receive8();
  }

  virtual size_t requestFrom(uint8_t address, size_t len) {
    service.send(I2cRequestFrom2);
    service.send(address);
    service.send((uint64_t)len);
    return service.receive8();
  }

  virtual void onReceive(void (*)(int)) {}

  virtual void onRequest(void (*)(void)) {}

  size_t write(uint8_t c) {
    service.send(I2cWrite);
    service.send(c);
    return service.receive16();
  }

  int available() {
    service.send(I2cAvailable);
    return service.receive16();
  }

  int read() {
    service.send(I2cRead);
    return service.receive16();
  }

  int peek() {
    service.send(I2cPeek);
    return service.receive16();
  }

  operator bool() { return service; }

 protected:
  HardwareService service;
};

}  // namespace arduino
