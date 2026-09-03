/*
  SocketImpl.h 
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
#include "DesktopSocket.h"
#include <cstdint>
#if !ARDUINO_EMULATOR_WINDOWS
#include <netinet/in.h>
#endif
#include <cstring>

namespace arduino {

class SocketImpl {
 public:
  SocketImpl() = default;
  SocketImpl(SocketHandle socket) {
    sock = socket;
    is_connected = true;
    memset(&serv_addr, 0, sizeof(serv_addr));
  };
  SocketImpl(SocketHandle socket, struct sockaddr_in* address) {
    sock = socket;
    is_connected = true;
    serv_addr = *address;
  };
  virtual ~SocketImpl() {
    if (sock != INVALID_SOCKET_HANDLE) {
      close();
    }
  }
  // checks if we are connected
  virtual uint8_t connected();
  // opens a conection
  virtual int connect(const char* address, uint16_t port);
  // opens a connection with a timeout in milliseconds
  virtual int connect(const char* address, uint16_t port, int32_t timeout_ms);
  // sends some data
  virtual size_t write(const uint8_t* str, size_t len);
  // provides the available bytes
  virtual size_t available();
  // direct read
  virtual int read(uint8_t* buffer, size_t len);
  // peeks one character
  virtual int peek();
  // coloses the connection
  virtual void close();

  // determines the IP Adress
  const char* getIPAddress();
  // determines the IP Adress
  const char* getIPAddress(const char* validEntries[]);

  virtual void setCACert(const char* cert)  {}
  virtual void setInsecure() {}
  SocketHandle fd() { return sock; }

 protected:
  bool is_connected = false;
  SocketHandle sock = INVALID_SOCKET_HANDLE;
  int valread = 0;
  struct sockaddr_in serv_addr;
};

}  // namespace arduino
