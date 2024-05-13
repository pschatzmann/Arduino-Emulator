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


class WifiMock {
public:
  virtual void begin(const char *name, const char *pwd) {
    // nothing to be done
  }

  // we assume the network to be available on a desktop or host machine
  wl_status_t status() { return WL_CONNECTED; }

  IPAddress &localIP() {
    SocketImpl sock;
    adress.fromString(sock.getIPAddress());
    return adress;
  }

  void setSleep(bool){
  }

  int macAddress() {
    return mac;
  }

  void setClientInsecure() {}

protected:
  IPAddress adress;
  int mac = 0;
};

extern WifiMock WiFi;

} // namespace arduino
