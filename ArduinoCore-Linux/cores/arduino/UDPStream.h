#pragma once
#include <WiFi.h>
#include <WiFiUdp.h>
#if defined(ESP32)
#include <esp_wifi.h>
#endif
#include "ArduinoLogger.h"

namespace arduino {

/**
 * @brief A UDP class which makes sure that we can use UDP as
 * simple Stream w/o additional complexity.
 * When we did not specify a target IP address we send the data
 * to the address we received the data.
 * Single character writes will be buffered up to the defined write
 * buffer size (default 256 bytes)
 * @author Phil Schatzmann
 * @copyright GPLv3
 */

class UDPStream : public Stream {
 public:
  /// Default Constructor
  UDPStream() = default;

  /// Short hand constructor to start sending data to the indicated address /
  /// port
  UDPStream(IPAddress a, uint16_t port) { begin(a, port); }

  /// @brief Defines an alternative UDP object. By default we use WiFiUDP
  /// @param udp
  void setUDP(UDP& udp) { p_udp = &udp; };

  /// Always return 1492 (MTU 1500 - 8 byte header) as UDP packet available to
  /// write
  int availableForWrite() override { return 1492; }

  /**
   * Provides the available size of the current package and if this is used up
   * of the next package
   */
  int available() override {
    int size = p_udp->available();
    // if the curren package is used up we prvide the info for the next
    if (size == 0) {
      size = p_udp->parsePacket();
    }
    return size;
  }

  /// Starts to send data to the indicated address / port
  bool begin(IPAddress a, uint16_t port) {
    remote_address_ext = a;
    remote_port_ext = port;
    return p_udp->begin(port);
  }

  /// Starts to receive data from/with the indicated port
  bool begin(uint16_t port, uint16_t port_ext = 0) {
    remote_address_ext = IPAddress((uint32_t)0);
    remote_port_ext = port_ext != 0 ? port_ext : port;
    return p_udp->begin(port);
  }

  /// Starts to receive data in multicast from/with the indicated address / port
  bool beginMulticast(IPAddress address, uint16_t port) {
    return p_udp->beginMulticast(address, port);
  }

  /// Alternative to begin(IPAddress a, uint16_t port)
  bool setTarget(IPAddress a, uint16_t port) { return begin(a, port); }

  /// We use the same remote port as defined in begin for write
  uint16_t remotePort() {
    uint16_t result = p_udp->remotePort();
    return result != 0 ? result : remote_port_ext;
  }

  /// We use the same remote ip as defined in begin for write
  IPAddress remoteIP() {
    // Determine address if it has not been specified
    if ((uint32_t)remote_address_ext == 0) {
      remote_address_ext = p_udp->remoteIP();
    }
    // EmulatorLogger.log(EmulatorLogger::INFO, "ip: %u", remote_address_ext);
    return remote_address_ext;
  }

  /// Replys will be sent to the initial remote caller
  size_t write(const uint8_t* data, size_t len) override {
    IPAddress remote = remoteIP();
    if ((uint32_t)remote == 0) {
      Logger.error("UDPStream", "no remote address defined");
      return 0;
    }
    p_udp->beginPacket(remote, remotePort());
    size_t result = p_udp->write(data, len);
    p_udp->endPacket();
    return result;
  }

  /// Reads bytes using WiFi::readBytes
  size_t readBytes(uint8_t* data, size_t len) {
    size_t avail = available();
    size_t bytes_read = 0;
    if (avail > 0) {
      // get the data now
      bytes_read = p_udp->readBytes((uint8_t*)data, len);
    }
    return bytes_read;
  }

  void stop() { p_udp->stop(); }

  int peek() override { return p_udp->peek(); }

  int read() override { return p_udp->read(); }

  size_t write(uint8_t data) override {
    if (write_buffer.capacity() < write_buffer_size) {
      write_buffer.reserve(write_buffer_size);
    }
    write_buffer.push_back(data);
    if (write_buffer.size() >= write_buffer_size) {
      flush();
    }
    return 1;
  }

  void flush() override {
    if (write_buffer.empty()) {
      return;
    }
    write(write_buffer.data(), write_buffer.size());
    write_buffer.clear();
  }

  void setWriteBufferSize(size_t size) { write_buffer_size = size; }

 protected:
  WiFiUDP default_udp;
  UDP* p_udp = &default_udp;
  uint16_t remote_port_ext = 0;
  IPAddress remote_address_ext;
  int write_buffer_size = 256;
  std::vector<uint8_t> write_buffer;
};

}  // namespace arduino

using WiFiUDPStream = arduino::UDPStream;
