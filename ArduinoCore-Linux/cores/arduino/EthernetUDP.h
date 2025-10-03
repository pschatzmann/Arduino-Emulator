/*
  EthernetUDP.h
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
#include "api/IPAddress.h"
#include "api/Udp.h"
#include <RingBufferExt.h>
#include "SignalHandler.h"
#include "ArduinoLogger.h"

namespace arduino {


class EthernetUDP : public UDP {
 private:
  int udp_server;
  IPAddress multicast_ip;
  IPAddress remote_ip;
  uint16_t server_port;
  uint16_t remote_port;
  char* tx_buffer;
  size_t tx_buffer_len;
  RingBufferExt* rx_buffer;
  void log_e(const char* msg, int errorNo);

  static std::vector<EthernetUDP*>& active_udp() {
    static std::vector<EthernetUDP*> udp_list;
    return udp_list;
  }
  static void cleanupAll(int sig) {
    for (auto* udp : active_udp()) {
      if (udp) {
        udp->stop();
      }
    }
  }

 public:
  EthernetUDP();
  ~EthernetUDP();
  uint8_t begin(IPAddress a, uint16_t p);
  uint8_t begin(uint16_t p);
  uint8_t beginMulticast(IPAddress a, uint16_t p);
  void stop();
  int beginMulticastPacket();
  int beginPacket();
  int beginPacket(IPAddress ip, uint16_t port);
  int beginPacket(const char* host, uint16_t port);
  int endPacket();
  size_t write(uint8_t);
  size_t write(const uint8_t* buffer, size_t size);
  int parsePacket();
  int available();
  int read();
  int read(unsigned char* buffer, size_t len);
  int read(char* buffer, size_t len);
  int peek();
  void flush();
  IPAddress remoteIP();
  uint16_t remotePort();
  
protected:
  void registerCleanup() {
    static bool signal_registered = false;
    if (!signal_registered) {
      SignalHandler::registerHandler(SIGINT, cleanupAll);
      SignalHandler::registerHandler(SIGTERM, cleanupAll);
      signal_registered = true;
    }
  }

};

}  // namespace arduino
