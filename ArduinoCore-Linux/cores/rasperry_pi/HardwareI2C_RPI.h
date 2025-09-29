#pragma once
#ifdef USE_RPI

#include <fcntl.h>   // for O_RDWR
#include <unistd.h>  // for open(), close(), etc.
#include <vector>
#include "Hardware.h"
#include "HardwareI2C.h"
#include "ArduinoLogger.h"

namespace arduino {

class HardwareI2C_RPI : public HardwareI2C {
 public:
  HardwareI2C_RPI(const char* device = "/dev/i2c-1") {
    Hardware.i2c = this;
    i2c_device = device;
    i2c_fd = open(device, O_RDWR);
    if (i2c_fd < 0) {
      Logger.error("HardwareI2C_RPI: Failed to open /dev/i2c-1");
    }
  }
  ~HardwareI2C_RPI() {
    Hardware.i2c = nullptr;
    end();
  }
  void begin() override;
  void begin(uint8_t address) override;
  void end() override;
  void setClock(uint32_t freq) override;
  void beginTransmission(uint8_t address) override;
  uint8_t endTransmission(bool stopBit) override;
  uint8_t endTransmission(void) { return endTransmission(true);};
  size_t requestFrom(uint8_t address, size_t len, bool stopBit) override;
  size_t requestFrom(uint8_t address, size_t len) override;
  void onReceive(void (*)(int)) override;
  void onRequest(void (*)(void)) override;
  size_t write(uint8_t) override;
  size_t write(const uint8_t*, size_t) override;
  int available() override;
  int read() override;
  int peek() override;
  void flush() override { fsync(i2c_fd);}

 private:
  int i2c_fd = -1;
  uint8_t current_address = 0;
  uint32_t i2c_clock = 100000;  // default 100kHz
  std::vector<uint8_t> i2c_rx_buffer;
  std::vector<uint8_t> i2c_tx_buffer;
  int i2c_rx_pos = 0;
  const char* i2c_device;
};

}  // namespace arduino

#endif
