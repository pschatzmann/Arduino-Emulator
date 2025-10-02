#pragma once

#include <exception>

#include "ArduinoLogger.h"
#include "Hardware.h"
#include "RemoteGPIO.h"
#include "RemoteI2C.h"
#include "RemoteSPI.h"
#include "WiFiUdpStream.h"

namespace arduino {
#if !defined(SKIP_HARDWARE_SETUP)
#if !defined(USE_RPI)
static RemoteI2C Wire;
static RemoteSPI SPI;
#endif
#endif

/**
 * Class which is used to configure the actual Hardware APIs
 */

class HardwareSetupRemoteClass {
 public:
  /// default constructor: you need to call begin() afterwards
  HardwareSetupRemoteClass() = default;

  /// HardwareSetup uses the indicated stream
  HardwareSetupRemoteClass(Stream& stream) {
    begin(&stream, false);
  }

 /// HardwareSetup that uses udp
  HardwareSetupRemoteClass(int port) {
    this->port = port;
  }

  /// assigns the different protocols to the stream
  void begin(Stream* s, bool doHandShake = true) {
    p_stream = s;

    Hardware.i2c = &i2c;
    Hardware.spi = &spi;
    Hardware.gpio = &gpio;

    i2c.setStream(s);
    spi.setStream(s);
    gpio.setStream(s);

    // setup global objects
    SPI = spi;
    Wire = i2c;

    if (doHandShake) {
      handShake(s);
    }
  }

  /// start with udp on the indicatd port
  void begin(int port){
    this->port = port;
    begin();
  }

  /// start with the default udp stream.
  void begin() {
    if (p_stream == nullptr) {
      default_stream.begin(port);
      IPAddress ip = default_stream.remoteIP();
      int remote_port = default_stream.remotePort();
      default_stream.setTarget(ip, remote_port);
      default_stream.write((const uint8_t*)"OK", 2);
      default_stream.flush();
      begin(&default_stream, true);
    } else {
      begin(p_stream, true);
    }
  }

  void end() {
    if (p_stream == &default_stream) {
      default_stream.stop();
    }
    Hardware.i2c = nullptr;
    Hardware.spi = nullptr;
    Hardware.gpio = nullptr;
  }

  auto& get_gpio() { return gpio; }
  auto& get_i2c() { return i2c; }
  auto& get_spi() { return spi; }

 protected:
  WiFiUDPStream default_stream;
  Stream *p_stream = nullptr;
  RemoteI2C i2c;
  RemoteSPI spi;
  RemoteGPIO gpio;
  int port;

  void handShake(Stream* s) {
    while (true) {
      Logger.warning("HardwareSetup", "waiting for device...");
      try {
        // we wait for the Arduino to send us the Arduino-Emulator string
        if (s->available() >= 16) {
          char buffer[30];
          int len = s->readBytes(buffer, 18);
          buffer[len] = 0;
          if (strncmp(buffer, "Arduino-Emulator", 16)) {
            Logger.info("WiFiUDPStream", "device found!");
            break;
          } else {
            Logger.info("WiFiUDPStream", "unknown command", buffer);
          }
        }
        delay(10000);
      } catch (const std::exception& ex) {
        Logger.error("WiFiUDPStream", ex.what());
      }
    }
  }
};

#if !defined(SKIP_HARDWARE_SETUP)
static HardwareSetupRemoteClass HardwareSetupRemote{7000};
#endif
}  // namespace arduino
