/*
	WiFi.h 
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

#include "WiFiClient.h"
#include "WiFiClientSecure.h"
#include "WiFiServer.h"

#include <string.h>
#if defined(_WIN32)
#  include <winsock2.h>
#  include <ws2tcpip.h>
#else
#  include <netdb.h>
#  include <netinet/in.h>
#  include <sys/socket.h>
#endif

/**
 * @brief We provide the WiFi class to simulate the Arduino WIFI. In in Linux
 * we can expect that networking is already active. So we dont need to do
 * anything
 *
 */

namespace arduino {

typedef enum { WIFI_OFF = 0, WIFI_STA = 1, WIFI_AP = 2, WIFI_AP_STA = 3 } WiFiMode_t;  // For ESP compatibility

enum wifi_ps_type_t { WIFI_PS_NONE, WIFI_PS_MIN_MODEM, WIFI_PS_MAX_MODEM };

class WifiMock {
 public:
  virtual void begin(const char* name, const char* pwd) {
    // nothing to be done
  }

  // we assume the network to be available on a desktop or host machine
  wl_status_t status() { return WL_CONNECTED; }

  IPAddress& localIP() {
    SocketImpl sock;
    adress.fromString(sock.getIPAddress());
    return adress;
  }

  void setSleep(bool) {}

  void setSleep(wifi_ps_type_t) {}

  int macAddress() { return mac; }

  void setClientInsecure() {}

  // WIFI_STA is the best approximation to interface enabled
  void mode(WiFiMode_t m) { wifi_mode = m; }
  WiFiMode_t getMode() { return wifi_mode; };

  /// Access point (ESP32): the computer is already in the network, so this
  /// only changes the mode; softAPIP() is the address of the computer
  bool softAP(const char* ssid, const char* passphrase = nullptr,
              int channel = 1, int ssid_hidden = 0, int max_connection = 4) {
    wifi_mode = WIFI_AP;
    return true;
  }
  bool softAPdisconnect(bool wifioff = false) {
    wifi_mode = WIFI_STA;
    return true;
  }
  IPAddress& softAPIP() { return localIP(); }

  /// Resolves a host name (or an IP address in text form) to an IPv4 address
  int hostByName(const char* host, IPAddress& result) {
    if (host == nullptr) return 0;
    if (result.fromString(host)) return 1;
    struct addrinfo hints;
    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_INET;
    struct addrinfo* info = nullptr;
    if (getaddrinfo(host, nullptr, &hints, &info) != 0 || info == nullptr) return 0;
    const uint8_t* ip = reinterpret_cast<const uint8_t*>(
        &reinterpret_cast<struct sockaddr_in*>(info->ai_addr)->sin_addr.s_addr);
    result = IPAddress(ip[0], ip[1], ip[2], ip[3]);
    freeaddrinfo(info);
    return 1;
  }

 protected:
  IPAddress adress;
  int mac = 0;
  WiFiMode_t wifi_mode = WIFI_STA;
};

extern WifiMock WiFi;

}  // namespace arduino
