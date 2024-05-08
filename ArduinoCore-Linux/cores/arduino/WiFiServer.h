#pragma once

#include "WiFiClient.h"
#include "Server.h"

namespace arduino {

class WiFiServer : public Server {
private:
  uint16_t _port;
  void*     pcb;
public:
  WiFiServer();
  WiFiClient available(uint8_t* status = NULL);
  void begin(int port=80);
  virtual size_t write(uint8_t);
  virtual size_t write(const uint8_t *buf, size_t size);
  uint8_t status();

  using Print::write;
};

}


