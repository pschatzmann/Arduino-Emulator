
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
  EthernetImpl() {
    // Set some defaults
    _macAddress[0] = 0xDE; _macAddress[1] = 0xAD; _macAddress[2] = 0xBE; _macAddress[3] = 0xEF; _macAddress[4] = 0xFE; _macAddress[5] = 0xED;
    _localIP = IPAddress(192,168,1,177);
    _subnetMask = IPAddress(255,255,255,0);
    _gatewayIP = IPAddress(192,168,1,1);
    _dnsServerIP = IPAddress(8,8,8,8);
    _hardwareStatus = 1; // Assume present
    _linkStatus = 2; // Assume linkON
    _retransmissionCount = 8;
    _retransmissionTimeout = 2000;
  }

  // Begin with MAC only
  bool begin(uint8_t macAddress[6]) {
    setMACAddress(macAddress);
    return true;
  }
  // Begin with full config
  bool begin(uint8_t macAddress[6], IPAddress localIP, IPAddress dnsServerIP, IPAddress gateway, IPAddress subnet) {
    setMACAddress(macAddress);
    setLocalIP(localIP);
    setDnsServerIP(dnsServerIP);
    setGatewayIP(gateway);
    setSubnetMask(subnet);
    return true;
  }

  // Returns the local IP address
  IPAddress localIP() { return _localIP; }
  // Returns the subnet mask
  IPAddress subnetMask() { return _subnetMask; }
  // Returns the gateway IP
  IPAddress gatewayIP() { return _gatewayIP; }
  // Returns the DNS server IP
  IPAddress dnsServerIP() { return _dnsServerIP; }
  // Returns the MAC address
  void MACAddress(uint8_t* mac) { for (int i=0; i<6; ++i) mac[i] = _macAddress[i]; }
  // Set the MAC address
  void setMACAddress(const uint8_t* mac) { for (int i=0; i<6; ++i) _macAddress[i] = mac[i]; }
  // Set the local IP
  void setLocalIP(const IPAddress& ip) { _localIP = ip; }
  // Set the subnet mask
  void setSubnetMask(const IPAddress& mask) { _subnetMask = mask; }
  // Set the gateway IP
  void setGatewayIP(const IPAddress& ip) { _gatewayIP = ip; }
  // Set the DNS server IP
  void setDnsServerIP(const IPAddress& ip) { _dnsServerIP = ip; }
  // Set retransmission count
  void setRetransmissionCount(uint8_t count) { _retransmissionCount = count; }
  // Set retransmission timeout
  void setRetransmissionTimeout(uint16_t timeout) { _retransmissionTimeout = timeout; }

  // Returns hardware status (1 = present, 0 = absent)
  int hardwareStatus() { return _hardwareStatus; }
  // Returns link status (2 = linkON, 1 = linkOFF, 0 = unknown)
  int linkStatus() { return _linkStatus; }
  // Init (returns true for compatibility)
  bool init(uint8_t socketCount = 4) { _hardwareStatus = 1; return true; }
  // Maintain (returns 0 for compatibility)
  int maintain() { return 0; }

protected:
  uint8_t _macAddress[6];
  IPAddress _localIP;
  IPAddress _subnetMask;
  IPAddress _gatewayIP;
  IPAddress _dnsServerIP;
  int _hardwareStatus = 1; // 1 = present, 0 = absent
  int _linkStatus = 2;     // 2 = linkON, 1 = linkOFF, 0 = unknown
  uint8_t _retransmissionCount = 8;
  uint16_t _retransmissionTimeout = 2000;
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
