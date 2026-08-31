#pragma once
/*
  FirmataTransport.h
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
#include "api/Stream.h"
#include <atomic>
#include <cstdint>
#include <functional>
#include <map>
#include <mutex>
#include <thread>
#include <vector>

namespace arduino {

/// A digital port report (device -> host), 8 pins packed into one message.
struct FirmataDigitalMessage {
  uint8_t port;
  uint16_t value;
};

/// A single analog pin report (device -> host).
struct FirmataAnalogMessage {
  uint8_t pin;
  int value;
};

/// A raw SysEx message: `command` is the byte right after START_SYSEX,
/// `data` holds everything up to (excluding) END_SYSEX, undecoded.
struct FirmataSysexMessage {
  uint8_t command;
  std::vector<uint8_t> data;
};

/**
 * @class FirmataTransport
 * @brief Single reader thread + byte-stream parser shared by every
 * Firmata-based hardware backend (GPIO, I2C, SPI, ...) that talks to the
 * same device.
 *
 * A real Firmata device is reached over exactly one Stream (typically a
 * serial port). Digital/analog GPIO reports, I2C replies and SPI replies
 * are all multiplexed over that single connection, so only one thread may
 * ever read from the Stream. Each hardware backend registers the message
 * types it cares about via onDigitalMessage()/onAnalogMessage()/
 * addSysexHandler() and shares one FirmataTransport instance; writes are
 * serialized with an internal mutex so requests from different backends
 * don't interleave on the wire.
 */
class FirmataTransport {
 public:
  using DigitalHandler = std::function<void(const FirmataDigitalMessage&)>;
  using AnalogHandler = std::function<void(const FirmataAnalogMessage&)>;
  using SysexHandler = std::function<void(const FirmataSysexMessage&)>;

  ~FirmataTransport();

  /// Start the background reader thread on the given stream.
  bool begin(Stream& stream);

  /// Stop the reader thread and detach from the stream.
  void end();

  operator bool() const { return running_.load(); }

  /// Thread-safe write of a raw byte sequence (e.g. a whole SysEx message).
  void writeBytes(const uint8_t* data, size_t len);
  void writeByte(uint8_t b) { writeBytes(&b, 1); }

  /// Register the (single) handler for digital port reports.
  void onDigitalMessage(DigitalHandler handler) {
    digital_handler_ = std::move(handler);
  }

  /// Register the (single) handler for analog pin reports.
  void onAnalogMessage(AnalogHandler handler) {
    analog_handler_ = std::move(handler);
  }

  /// Register a handler for SysEx messages carrying the given command byte
  /// (e.g. I2C_REPLY or SPI_DATA). Replaces any previously registered
  /// handler for the same command.
  void addSysexHandler(uint8_t command, SysexHandler handler);

  /// Remove a previously registered SysEx handler.
  void removeSysexHandler(uint8_t command);

 private:
  Stream* stream_ = nullptr;
  std::atomic<bool> running_{false};
  std::thread thread_;
  std::mutex write_mutex_;

  DigitalHandler digital_handler_;
  AnalogHandler analog_handler_;
  std::mutex sysex_handlers_mutex_;
  std::map<uint8_t, SysexHandler> sysex_handlers_;

  void readerLoop();
};

}  // namespace arduino

#endif  // USE_FIRMATA
