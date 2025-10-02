#pragma once
#ifdef USE_RPI

#include <fcntl.h>   // for O_RDWR
#include <unistd.h>  // for open(), close(), etc.
#include <vector>
#include "Hardware.h"
#include "api/HardwareI2C.h"
#include "ArduinoLogger.h"

namespace arduino {

/**
 * @class HardwareI2C_RPI
 * @brief Implementation of I2C communication for Raspberry Pi using Linux I2C device interface.
 *
 * This class provides an interface to the I2C bus on Raspberry Pi platforms by accessing
 * the Linux device (e.g., /dev/i2c-1). It inherits from HardwareI2C and implements all
 * required methods for I2C communication, including transmission, reception, and configuration.
 *
 * @note This class is only available when USE_RPI is defined.
 */
class HardwareI2C_RPI : public HardwareI2C {
 public:
  HardwareI2C_RPI(const char* device = "/dev/i2c-1") {
    i2c_device = device;
  }
  ~HardwareI2C_RPI() {
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

  operator bool() { return is_open; }

 private:
  int i2c_fd = -1;
  uint8_t current_address = 0;
  uint32_t i2c_clock = 100000;  // default 100kHz
  std::vector<uint8_t> i2c_rx_buffer;
  std::vector<uint8_t> i2c_tx_buffer;
  int i2c_rx_pos = 0;
  const char* i2c_device;
  bool is_open = false;
};

}  // namespace arduino

#endif
