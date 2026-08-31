#pragma once
/*
  HardwareI2C_FIR.h
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
#ifdef USE_FIRMATA
#include "api/HardwareI2C.h"
#include "FirmataTransport.h"
#include <condition_variable>
#include <deque>
#include <memory>
#include <mutex>
#include <vector>

namespace arduino {

/**
 * @class HardwareI2C_FIRMATA
 * @brief I2C master implementation that speaks the Firmata protocol
 * (I2C_REQUEST/I2C_REPLY/I2C_CONFIG SysEx messages, as implemented by
 * StandardFirmata and ConfigurableFirmata's I2CFirmata feature).
 *
 * Writes (beginTransmission/write/endTransmission) are forwarded as a
 * single I2C_REQUEST write message. requestFrom() sends an I2C_REQUEST read
 * message and then blocks (bounded by setTimeout()) until the matching
 * I2C_REPLY has been parsed by the shared FirmataTransport.
 *
 * @note This class is only available when USE_FIRMATA is defined.
 * @note Firmata has no I2C slave-mode support; begin(address) and
 *       onReceive()/onRequest() are no-ops.
 * @note Firmata's I2C_CONFIG message configures a read delay, not a bus
 *       clock frequency - setClock() has nothing to forward it to and is a
 *       documented no-op.
 */
class HardwareI2C_FIRMATA : public HardwareI2C {
 public:
  HardwareI2C_FIRMATA() = default;
  ~HardwareI2C_FIRMATA();

  /// Start talking Firmata over the given stream (owns a private transport).
  bool begin(Stream &stream);
  /// Attach to an already-started, externally-owned transport, e.g. to
  /// share one Firmata connection with HardwareGPIO_FIRMATA/HardwareSPI_FIRMATA.
  bool begin(FirmataTransport &transport);

  void begin() override;
  void begin(uint8_t address) override;  // I2C slave mode: unsupported, no-op
  void end() override;

  void setClock(uint32_t freq) override;  // no-op, see class note

  void beginTransmission(uint8_t address) override;
  size_t write(uint8_t data) override;
  size_t write(const uint8_t *data, size_t len) override;
  uint8_t endTransmission(bool stopBit) override;
  uint8_t endTransmission(void) override;

  size_t requestFrom(uint8_t address, size_t len, bool stopBit) override;
  size_t requestFrom(uint8_t address, size_t len) override;

  int available() override;
  int peek() override;
  int read() override;
  void flush() override {}

  void onReceive(void (*)(int)) override {}   // slave mode unsupported
  void onRequest(void (*)(void)) override {}  // slave mode unsupported

  /// Maximum time requestFrom() waits for the I2C_REPLY, in milliseconds.
  void setTimeout(unsigned long timeout_ms) { timeout_ms_ = timeout_ms; }

  operator bool() { return is_open; }

 protected:
  FirmataTransport *transport_ = nullptr;             // non-owning
  std::unique_ptr<FirmataTransport> owned_transport_;  // set by begin(Stream&)
  bool is_open = false;
  unsigned long timeout_ms_ = 1000;

  uint8_t current_address_ = 0;
  std::vector<uint8_t> tx_buffer_;

  std::mutex rx_mutex_;
  std::condition_variable rx_cv_;
  std::deque<uint8_t> rx_buffer_;
  bool request_pending_ = false;

  void handleSysex(const FirmataSysexMessage &msg);
};

}  // namespace arduino

#endif  // USE_FIRMATA
