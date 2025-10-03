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

/**
 * @brief We provide the WiFi class to simulate the Arduino WIFI. In in Linux
 * we can expect that networking is already active. So we dont need to do
 * anything
 *
 */

namespace arduino {

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

 protected:
  IPAddress adress;
  int mac = 0;
};

extern WifiMock WiFi;

}  // namespace arduino
