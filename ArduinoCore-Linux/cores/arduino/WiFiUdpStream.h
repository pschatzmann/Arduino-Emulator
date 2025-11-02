/*
  WiFiUdpStream.h 
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
#include <string.h>
#include <thread>
#include "ArduinoLogger.h"
#include "WiFiUdp.h"
#include "api/ArduinoAPI.h"
#include "api/IPAddress.h"

namespace arduino {

/**
 * @brief UDP Stream implementation for single-target communication
 *
 * WiFiUDPStream extends WiFiUDP to provide a stream-like interface for UDP
 * communication with a single target endpoint. It automatically manages packet
 * boundaries and provides buffered read/write operations suitable for streaming
 * protocols over UDP.
 * 
 * Key features:
 * - Stream-based UDP communication with automatic packet management
 * - Single target endpoint configuration (IP address and port)
 * - Automatic target discovery from incoming packets when target unknown
 * - Buffered operations with flush-triggered packet transmission
 * - Thread-safe operations for concurrent access
 * - Integration with WiFiUDP for underlying network operations
 * 
 * The class maintains an internal target address and automatically begins/ends
 * UDP packets as needed. Data is sent when flush() is called, making it suitable
 * for protocols that require message boundaries or batch transmission.
 *
 * @see WiFiUDP
 * @see IPAddress
 * @see Stream
 */
class WiFiUDPStream : public WiFiUDP {
 public:
  // unknown target -> we wait for a hallo until we get one
  WiFiUDPStream() = default;

  // known target
  WiFiUDPStream(IPAddress targetAdress, int port) {
    setTarget(targetAdress, port);
  }

  void flush() {
    WiFiUDP::flush();
    endPacket();
    if (!targetDefined()) {
      target_adress = remoteIP();
    }
    beginPacket(target_adress, port);
  }

  void stop() {
    if (active) {
      endPacket();
      WiFiUDP::stop();
      active = false;
    }
  }

  bool targetDefined() { return ((uint32_t)target_adress) != 0; }

  void setTarget(IPAddress targetAdress, int port) {
    this->target_adress = targetAdress;
    this->port = port;
    beginPacket(targetAdress, port);
  }

  size_t readBytes(uint8_t* values, size_t len) {
    if (this->available() == 0) {
      // we need to receive the next packet
      this->parsePacket();
      Logger.debug("parsePacket");
    } else {
      Logger.debug("no parsePacket()");
    }
    size_t result = read(values, len);

    char msg[50];
    sprintf(msg, "->len %ld", result);
    Logger.debug(msg);

    return result;
  }

  bool isActive() { return active; }

 protected:
  bool active = false;
  IPAddress target_adress{0, 0, 0, 0};
  int port = 0;
};

// Define a global function which will be used to start a thread

}  // namespace arduino
