
/*
  Ethernet.h 
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

#include <arpa/inet.h>  // for inet_pton
#include <netdb.h>      // for gethostbyname, struct hostent
#include <unistd.h>     // for close
#include <memory>  // This is the include you need

#include "ArduinoLogger.h"
#include "RingBufferExt.h"
#include "SocketImpl.h"
#include "SignalHandler.h"
#include "api/Client.h"
#include "api/Common.h"
#include "api/IPAddress.h"

namespace arduino {

typedef enum {
  WL_NO_SHIELD = 255,
  WL_IDLE_STATUS = 0,
  WL_NO_SSID_AVAIL,
  WL_SCAN_COMPLETED,
  WL_CONNECTED,
  WL_CONNECT_FAILED,
  WL_CONNECTION_LOST,
  WL_DISCONNECTED
} wl_status_t;

class EthernetImpl {
 public:
  IPAddress& localIP() {
    SocketImpl sock;
    adress.fromString(sock.getIPAddress());
    return adress;
  }

  /// To be compatible with Ethernet library: do nothing
  void begin(IPAddress macAddress ) {}
  /// To be compatible with Ethernet library: do nothing
  void begin(IPAddress macAddress, IPAddress staticIP, IPAddress staticDNS, IPAddress staticGateway, IPAddress staticSubnet) {}

 protected:
  IPAddress adress;
};

inline EthernetImpl Ethernet;

class EthernetClient : public Client {
 private:
  static std::vector<EthernetClient*>& active_clients() {
    static std::vector<EthernetClient*> clients;
    return clients;
  }
  static void cleanupAll(int sig) {
    for (auto* client : active_clients()) {
      if (client) {
        client->stop();
      }
    }
  }

 public:
  EthernetClient() {
    setTimeout(2000);
    p_sock = std::make_shared<SocketImpl>();
    readBuffer = RingBufferExt(bufferSize);
    writeBuffer = RingBufferExt(bufferSize);
    registerCleanup();
    active_clients().push_back(this);
  }
  EthernetClient(std::shared_ptr<SocketImpl> sock, int bufferSize = 256, long timeout = 2000) {
    if (sock) {
      setTimeout(timeout);
      this->bufferSize = bufferSize;
      readBuffer = RingBufferExt(bufferSize);
      writeBuffer = RingBufferExt(bufferSize);
      p_sock = sock;
      is_connected = p_sock->connected();
      registerCleanup();
      active_clients().push_back(this);
    }
  }
  EthernetClient(int socket) {
    setTimeout(2000);
    readBuffer = RingBufferExt(bufferSize);
    writeBuffer = RingBufferExt(bufferSize);
    p_sock = std::make_shared<SocketImpl>(socket);
    is_connected = p_sock->connected();
  }

  // checks if we are connected - using a timeout
  virtual uint8_t connected() override {
    if (!is_connected) return false;       // connect has failed
    if (p_sock->connected()) return true;  // check socket
    long timeout = millis() + getTimeout();
    uint8_t result = p_sock->connected();
    while (result <= 0 && millis() < timeout) {
      delay(200);
      result = p_sock->connected();
    }
    return result;
  }

  // support conversion to bool
  operator bool() override { return connected(); }

  // opens a conection
  virtual int connect(IPAddress ipAddress, uint16_t port) override {
    String str = String(ipAddress[0]) + String(".") + String(ipAddress[1]) +
                 String(".") + String(ipAddress[2]) + String(".") +
                 String(ipAddress[3]);
    this->address = ipAddress;
    this->port = port;
    return connect(str.c_str(), port);
  }

  // opens a connection
  virtual int connect(const char* address, uint16_t port) override {
    Logger.info(WIFICLIENT, "connect");
    this->port = port;
    if (connectedFast()) {
      p_sock->close();
    }
    IPAddress adr = resolveAddress(address, port);
    if (adr == IPAddress(0, 0, 0, 0)) {
      is_connected = false;
      return 0;
    }
    // performs the actual connection
    String str = adr.toString();
    Logger.info("Connecting to ", str.c_str());
    p_sock->connect(str.c_str(), port);
    is_connected = true;
    return 1;
  }

  virtual size_t write(char c) { return write((uint8_t)c); }

  // writes an individual character into the buffer. We flush the buffer when it
  // is full
  virtual size_t write(uint8_t c) override {
    if (writeBuffer.availableToWrite() == 0) {
      flush();
    }
    return writeBuffer.write(c);
  }

  virtual size_t write(const char* str, int len) {
    return write((const uint8_t*)str, len);
  }

  // direct write - if we have anything in the buffer we write that out first
  virtual size_t write(const uint8_t* str, size_t len) override {
    flush();
    return p_sock->write(str, len);
  }

  virtual int print(const char* str = "") {
    int len = strlen(str);
    return write(str, len);
  }

  virtual int println(const char* str = "") {
    int len = strlen(str);
    int result = write(str, len);
    char eol[1];
    eol[0] = '\n';
    write(eol, 1);
    return result;
  }

  // flush write buffer
  virtual void flush() override {
    Logger.debug(WIFICLIENT, "flush");

    int flushSize = writeBuffer.available();
    if (flushSize > 0) {
      uint8_t rbuffer[flushSize];
      writeBuffer.read(rbuffer, flushSize);
      p_sock->write(rbuffer, flushSize);
    }
  }

  // provides the available bytes from the read buffer or from the socket
  virtual int available() override {
    Logger.debug(WIFICLIENT, "available");
    if (readBuffer.available() > 0) {
      return readBuffer.available();
    }
    long timeout = millis() + getTimeout();
    int result = p_sock->available();
    while (result <= 0 && millis() < timeout) {
      delay(200);
      result = p_sock->available();
    }
    return result;
  }

  // read via ring buffer
  virtual int read() override {
    int result = -1;
    uint8_t c;
    if (readBytes(&c, 1) == 1) {
      result = c;
    }
    return result;
  }

  virtual size_t readBytes(char* buffer, size_t len) {
    return read((uint8_t*)buffer, len);
  }

  virtual size_t readBytes(uint8_t* buffer, size_t len) {
    return read(buffer, len);
  }

  // peeks one character
  virtual int peek() override {
    return p_sock->peek();
    return -1;
  }

  // close the connection
  virtual void stop() override { p_sock->close(); }

  virtual void setInsecure() {}

  int fd() { return p_sock->fd(); }

  uint16_t remotePort() { return port; }

  IPAddress remoteIP() { return address; }

  virtual void setCACert(const char* cert) {
    Logger.error(WIFICLIENT, "setCACert not supported");
  }

 protected:
  const char* WIFICLIENT = "EthernetClient";
  std::shared_ptr<SocketImpl> p_sock = nullptr;
  int bufferSize = 256;
  RingBufferExt readBuffer;
  RingBufferExt writeBuffer;
  bool is_connected = false;
  IPAddress address{0, 0, 0, 0};
  uint16_t port = 0;

  // resolves the address and returns sockaddr_in
  IPAddress resolveAddress(const char* address, uint16_t port) {
    struct sockaddr_in serv_addr4;
    memset(&serv_addr4, 0, sizeof(serv_addr4));
    serv_addr4.sin_family = AF_INET;
    serv_addr4.sin_port = htons(port);
    if (::inet_pton(AF_INET, address, &serv_addr4.sin_addr) <= 0) {
      // Not an IP, try to resolve hostname
      struct hostent* he = ::gethostbyname(address);
      if (he == nullptr || he->h_addr_list[0] == nullptr) {
        Logger.error(WIFICLIENT, "Hostname resolution failed");
        serv_addr4.sin_addr.s_addr = 0;
      } else {
        memcpy(&serv_addr4.sin_addr, he->h_addr_list[0], he->h_length);
      }
    }
    return IPAddress(serv_addr4.sin_addr.s_addr);
  }

  void registerCleanup() {
    static bool signal_registered = false;
    if (!signal_registered) {
      SignalHandler::registerHandler(SIGINT, cleanupAll);
      SignalHandler::registerHandler(SIGTERM, cleanupAll);
      signal_registered = true;
    }
  }

  int read(uint8_t* buffer, size_t len) override {
    Logger.debug(WIFICLIENT, "read");
    int result = 0;
    long timeout = millis() + getTimeout();
    result = p_sock->read(buffer, len);
    while (result <= 0 && millis() < timeout) {
      delay(200);
      result = p_sock->read(buffer, len);
    }
    //}
    char lenStr[16];
    sprintf(lenStr, "%d", result);
    Logger.debug(WIFICLIENT, "read->", lenStr);

    return result;
  }

  bool connectedFast() { return is_connected; }
};

}  // namespace arduino
