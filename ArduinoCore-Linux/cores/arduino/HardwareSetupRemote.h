#pragma once

#include <exception>

#include "ArduinoLogger.h"
#include "GPIOWrapper.h"
#include "I2CWrapper.h"
#include "RemoteGPIO.h"
#include "RemoteI2C.h"
#include "RemoteSPI.h"
#include "SPIWrapper.h"
#include "WiFiUdpStream.h"

namespace arduino {

/**
 * Class which is used to configure the actual Hardware APIs
 */

class HardwareSetupRemoteClass : public I2CSource,
                                 public SPISource,
                                 public GPIOSource {
 public:
  /// default constructor: you need to call begin() afterwards
  HardwareSetupRemoteClass() = default;

  /// HardwareSetup uses the indicated stream
  HardwareSetupRemoteClass(Stream& stream) { begin(&stream, false); }

  /// HardwareSetup that uses udp
  HardwareSetupRemoteClass(int port) { this->port = port; }

  /// assigns the different protocols to the stream
  bool begin(Stream* s, bool asDefault = true, bool doHandShake = true) {
    p_stream = s;

    i2c.setStream(s);
    spi.setStream(s);
    gpio.setStream(s);

    // setup global objects
    if (asDefault) {
      SPI.setSPI(&spi);
      Wire.setI2C(&i2c);
      GPIO.setGPIO(&gpio);
    }

    if (doHandShake) {
      handShake(s);
    }
    return i2c && spi && gpio;
  }

  /// start with udp on the indicatd port
  void begin(int port, bool asDefault) {
    this->port = port;
    begin(asDefault);
  }

  /// start with the default udp stream.
  void begin(bool asDefault = true) {
    is_default_objects_active = asDefault;
    if (p_stream == nullptr) {
      default_stream.begin(port);
      handShake(&default_stream);
      IPAddress ip = default_stream.remoteIP();
      int remote_port = default_stream.remotePort();
      default_stream.setTarget(ip, remote_port);
      default_stream.write((const uint8_t*)"OK", 2);
      default_stream.flush();
      begin(&default_stream, asDefault, false);
    } else {
      begin(p_stream, asDefault, true);
    }
  }

  void end() {
    if (is_default_objects_active) {
      GPIO.setGPIO(nullptr);
      SPI.setSPI(nullptr);
      Wire.setI2C(nullptr);
    }
    if (p_stream == &default_stream) {
      default_stream.stop();
    }
  }

  HardwareGPIO* getGPIO() { return &gpio; }
  HardwareI2C* getI2C() { return &i2c; }
  HardwareSPI* getSPI() { return &spi; }

 protected:
  WiFiUDPStream default_stream;
  Stream* p_stream = nullptr;
  RemoteI2C i2c;
  RemoteSPI spi;
  RemoteGPIO gpio;
  int port;
  bool is_default_objects_active = false;

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
static HardwareSetupRemoteClass Remote{7000};
#endif

}  // namespace arduino
