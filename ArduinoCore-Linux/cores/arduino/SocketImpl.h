#pragma once

/**
 * Separate implementation class for the WIFI client to prevent import conflicts
 ***/
#include <netinet/in.h>

namespace arduino {

class SocketImpl {
 public:
  SocketImpl() = default;
  SocketImpl(int socket, struct sockaddr_in* address) {
    sock = socket;
    is_connected = true;
    serv_addr = *address;
  };
  // checks if we are connected
  virtual uint8_t connected();
  // opens a conection
  virtual int connect(const char* address, uint16_t port);
  // sends some data
  virtual size_t write(const uint8_t* str, size_t len);
  // provides the available bytes
  virtual size_t available();
  // direct read
  virtual size_t read(uint8_t* buffer, size_t len);
  // peeks one character
  virtual int peek();
  // coloses the connection
  virtual void close();

  // determines the IP Adress
  const char* getIPAddress();
  // determines the IP Adress
  const char* getIPAddress(const char* validEntries[]);

 protected:
  bool is_connected = false;
  int sock = -1, valread;
  struct sockaddr_in serv_addr;
};

}  // namespace arduino
