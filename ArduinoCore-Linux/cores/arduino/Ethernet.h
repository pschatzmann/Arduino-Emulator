
#pragma once

#include "ArduinoLogger.h"
#include "Client.h"
#include "Common.h"
#include "IPAddress.h"
#include "RingBufferExt.h"
#include "SocketImpl.h"

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

 protected:
  IPAddress adress;
};

inline EthernetImpl Ethernet;

class EthernetClient : public Client {
 public:
  EthernetClient() {
    readBuffer = RingBufferExt(bufferSize);
    writeBuffer = RingBufferExt(bufferSize);
    setTimeout(2000);
  }
  EthernetClient(SocketImpl sock, int bufferSize = 256, long timeout = 2000) {
    this->bufferSize = bufferSize;
    readBuffer = RingBufferExt(bufferSize);
    writeBuffer = RingBufferExt(bufferSize);
    this->sock = sock;
    setTimeout(timeout);
    is_connected = sock.connected();
  }

  //EthernetClient(const EthernetClient&) = delete;

  // checks if we are connected - using a timeout
  virtual uint8_t connected() override {
    if (!is_connected) return false; // connect has failed
    if (sock.connected()) return true; // check socket
    long timeout = millis() + getTimeout();
    uint8_t result = sock.connected();
    while (result <= 0 && millis() < timeout) {
      delay(200);
      result = sock.connected();
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

    return connect(str.c_str(), port);
  }

  // opens a conection
  virtual int connect(const char* address, uint16_t port) override {
    Logger.info(WIFICLIENT, "connect");
    if (connectedFast()) {
      sock.close();
    }
    sock.connect(address, port);
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
  virtual size_t write(const uint8_t* str, size_t len) override{
    flush();
    return sock.write(str, len);
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
      sock.write(rbuffer, flushSize);
    }
  }

  // provides the available bytes from the read buffer or from the socket
  virtual int available() override {
    Logger.debug(WIFICLIENT, "available");
    if (readBuffer.available() > 0) {
      return readBuffer.available();
    }
    long timeout = millis() + getTimeout();
    int result = sock.available();
    while (result <= 0 && millis() < timeout) {
      delay(200);
      result = sock.available();
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
    // Logger.debug(WIFICLIENT,"read-1");
    // if (readBuffer.available()==0){
    //     uint8_t buffer[bufferSize];
    //     int len = read((uint8_t*)buffer, bufferSize);
    //     readBuffer.write(buffer, len);
    // }
    // return readBuffer.read();
    return result;
    ;
  }

  size_t readBytes(char* buffer, size_t len) override {
    return read((uint8_t*)buffer, len);
  }

  size_t readBytes(uint8_t* buffer, size_t len) override {
    return read(buffer, len);
  }

  // direct read with timeout

  // peeks one character
  virtual int peek() override {
    // Logger.debug(WIFICLIENT, "peek");
    // if (readBuffer.available()>0){
    //     return readBuffer.peek();
    // }
    return sock.peek();
    return -1;
  }

  // close the connection
  virtual void stop() override { sock.close(); }

  void setInsecure() {}

 protected:
  const char* WIFICLIENT = "EthernetClient";
  SocketImpl sock;
  int bufferSize = 256;
  RingBufferExt readBuffer;
  RingBufferExt writeBuffer;
  bool is_connected = false;

  int read(uint8_t* buffer, size_t len) override {
    Logger.debug(WIFICLIENT, "read");
    int result = 0;
    // if (readBuffer.available()>0){
    //     result = readBuffer.read(buffer, len);
    // } else {
    long timeout = millis() + getTimeout();
    result = sock.read(buffer, len);
    while (result <= 0 && millis() < timeout) {
      delay(200);
      result = sock.read(buffer, len);
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
