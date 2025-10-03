#pragma once

#include "api/HardwareSPI.h"

namespace arduino {

/**
 * @brief Remote SPI implementation that operates over a communication stream
 *
 * RemoteSPI provides SPI functionality by forwarding all operations to a remote
 * SPI controller via a communication stream (serial, network, etc.). This enables
 * SPI operations to be performed on remote hardware while maintaining the standard
 * HardwareSPI interface.
 * 
 * Key features:
 * - Complete HardwareSPI interface implementation
 * - Stream-based remote communication protocol
 * - Support for all SPI transfer modes (8-bit, 16-bit, buffer transfers)
 * - Transaction management with SPISettings support
 * - Interrupt handling and configuration
 * - Real-time bidirectional communication with remote SPI hardware
 * 
 * The class uses HardwareService for protocol handling and can work with any
 * Stream implementation (Serial, TCP, etc.) for remote connectivity.
 *
 * @see HardwareSPI
 * @see HardwareService
 * @see SPISettings
 * @see Stream
 */
class RemoteSPI : public HardwareSPI {
 public:
  RemoteSPI() = default;
  RemoteSPI(Stream* stream) { service.setStream(stream); }
  void setStream(Stream* stream) { service.setStream(stream); }

  uint8_t transfer(uint8_t data) {
    service.send(SpiTransfer8);
    service.send(data);
    service.flush();
    return service.receive8();
  }

  uint16_t transfer16(uint16_t data) {
    service.send(SpiTransfer16);
    service.send(data);
    service.flush();
    return service.receive16();
  }

  void transfer(void* buf, size_t count) {
    service.send(SpiTransfer);
    service.send((uint32_t)count);
    service.send(buf, count);
    service.flush();
    for (int j=0; j<count; j++) {
      ((uint8_t*)buf)[j] = service.receive8();
    }
  }

  void usingInterrupt(int interruptNumber) {
    service.send(SpiUsingInterrupt);
    service.send(interruptNumber);
    service.flush();
  }

  void notUsingInterrupt(int interruptNumber) {
    service.send(SpiNotUsingInterrupt);
    service.send(interruptNumber);
    service.flush();
  }

  void beginTransaction(SPISettings settings) {
    service.send(SpiBeginTransaction);
    // uint32_t clock, uint8_t bitOrder, uint8_t dataMode
    service.send((uint32_t)settings.getClockFreq());
    service.send((uint8_t)settings.getBitOrder());
    service.send((uint8_t)settings.getDataMode());
    service.flush();
  }

  void endTransaction(void) {
    service.send(SpiEndTransaction);
    service.flush();
  }

  void attachInterrupt() {
    service.send(SpiAttachInterrupt);
    service.flush();
  }

  void detachInterrupt() {
    service.send(SpiDetachInterrupt);
    service.flush();
  }

  void begin() {
    service.send(SpiBegin);
    service.flush();
  }

  void end() {
    service.send(SpiEnd);
    service.flush();
  }

  operator boolean() { return service; }

 protected:
  HardwareService service;
};

}  // namespace arduino
