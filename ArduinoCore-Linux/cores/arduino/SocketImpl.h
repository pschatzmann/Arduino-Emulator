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
#include <netinet/in.h>
#include <string.h>

namespace arduino {

class SocketImpl {
 public:
  SocketImpl() = default;
  SocketImpl(int socket) {
    sock = socket;
    is_connected = true;
    memset(&serv_addr, 0, sizeof(serv_addr));
  };
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

  virtual void setCACert(const char* cert)  {}
  virtual void setInsecure() {}
  int fd() { return sock; }

 protected:
  bool is_connected = false;
  int sock = -1, valread;
  struct sockaddr_in serv_addr;
};

}  // namespace arduino
