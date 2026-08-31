#pragma once
/*
  HardwareSPI_FIR.h
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
#include "api/HardwareSPI.h"
#include "FirmataTransport.h"
#include <condition_variable>
#include <memory>
#include <mutex>
#include <vector>

namespace arduino {

/**
 * @class HardwareSPI_FIRMATA
 * @brief SPI master implementation that speaks the SysEx SPI protocol
 * implemented by ConfigurableFirmata's SpiFirmata feature.
 *
 * @note Plain StandardFirmata does NOT support SPI - the remote device must
 *       be flashed with ConfigurableFirmata with the Spi feature enabled
 *       for this class to work.
 * @note This class is only available when USE_FIRMATA is defined.
 * @note The firmware only supports SPI channel 0, and expects `deviceId`'s
 *       low 2 bits to be 0 (see SpiFirmata::handleSpiConfig). Only one
 *       HardwareSPI_FIRMATA instance should be attached to a given
 *       FirmataTransport at a time - SPI replies are dispatched by the
 *       transport per top-level SysEx command, not per device.
 */
class HardwareSPI_FIRMATA : public HardwareSPI {
 public:
  /**
   * @param deviceId Device identifier sent in SPI_DEVICE_CONFIG; low 2 bits
   *        must be 0.
   * @param csPin Remote-device pin number to use as chip-select, or -1 to
   *        let the caller toggle CS itself via GPIO.
   */
  explicit HardwareSPI_FIRMATA(uint8_t deviceId = 0, int8_t csPin = -1);
  ~HardwareSPI_FIRMATA();

  /// Start talking Firmata over the given stream (owns a private transport).
  bool begin(Stream &stream);
  /// Attach to an already-started, externally-owned transport, e.g. to
  /// share one Firmata connection with HardwareGPIO_FIRMATA/HardwareI2C_FIRMATA.
  bool begin(FirmataTransport &transport);

  void begin() override;
  void end() override;

  uint8_t transfer(uint8_t data) override;
  uint16_t transfer16(uint16_t data) override;
  void transfer(void *buf, size_t count) override;

  void usingInterrupt(int interruptNumber) override {}
  void notUsingInterrupt(int interruptNumber) override {}
  void beginTransaction(SPISettings settings) override;
  void endTransaction(void) override {}

  void attachInterrupt() override {}
  void detachInterrupt() override {}

  /// Maximum time transfer() waits for the SPI_REPLY, in milliseconds.
  void setTimeout(unsigned long timeout_ms) { timeout_ms_ = timeout_ms; }

  operator bool() { return is_open; }

 protected:
  FirmataTransport *transport_ = nullptr;             // non-owning
  std::unique_ptr<FirmataTransport> owned_transport_;  // set by begin(Stream&)
  bool is_open = false;
  bool configured_ = false;
  unsigned long timeout_ms_ = 1000;

  uint8_t device_id_;
  int8_t cs_pin_;
  SPISettings settings_;

  std::mutex reply_mutex_;
  std::condition_variable reply_cv_;
  std::vector<uint8_t> reply_buffer_;
  bool reply_pending_ = false;

  void sendDeviceConfig();
  void doTransfer(uint8_t *buf, size_t count);
  void handleSysex(const FirmataSysexMessage &msg);
};

}  // namespace arduino

#endif  // USE_FIRMATA
