#ifdef USE_RPI

#include "HardwareI2C_RPI.h"
#include <fcntl.h>
#include <linux/i2c-dev.h>
#include <sys/ioctl.h>
#include <unistd.h>
#include <cstdio>
#include <cstring>

namespace arduino {


void HardwareI2C_RPI::begin(uint8_t address) {
  begin();
  current_address = address;
  if (ioctl(i2c_fd, I2C_SLAVE, address) < 0) {
    Logger.error("HardwareI2C_RPI: Failed to set I2C address");
    is_open = false;
  } else {
    is_open = true;
  }
}

void HardwareI2C_RPI::begin() {
  Logger.warning("Activating Rasperry PI: I2C");
  if (i2c_fd < 0) {
    i2c_fd = open(i2c_device, O_RDWR);
    if (i2c_fd < 0) {
      Logger.error("HardwareI2C_RPI: Failed to open I2C device");
      is_open = false;
    } else {
      is_open = true;
    }
  }
}

void HardwareI2C_RPI::end() {
  if (i2c_fd >= 0) {
    close(i2c_fd);
    i2c_fd = -1;
  }
}

void HardwareI2C_RPI::setClock(uint32_t freq) {
  i2c_clock = freq;
  // Changing I2C clock at runtime is not supported via /dev/i2c-1
  // Usually set via device tree overlays or raspi-config
  Logger.warning("HardwareI2C_RPI", "setClock not implemented on Linux I2C");
}

void HardwareI2C_RPI::beginTransmission(uint8_t address) {
  current_address = address;
  if (ioctl(i2c_fd, I2C_SLAVE, address) < 0) {
    Logger.error("HardwareI2C_RPI: Failed to set I2C address");
  }
}

size_t HardwareI2C_RPI::write(uint8_t data) {
  i2c_tx_buffer.push_back(data);
  return 1;
}

size_t HardwareI2C_RPI::write(const uint8_t* data, size_t len) {
  i2c_tx_buffer.insert(i2c_tx_buffer.end(), data, data + len);
  return len;
}

int HardwareI2C_RPI::available() { return i2c_rx_buffer.size() - i2c_rx_pos; }

int HardwareI2C_RPI::peek() {
  if (i2c_rx_pos < i2c_rx_buffer.size()) {
    return i2c_rx_buffer[i2c_rx_pos];
  }
  return -1;
}

int HardwareI2C_RPI::read() {
  if (i2c_rx_pos < i2c_rx_buffer.size()) {
    return i2c_rx_buffer[i2c_rx_pos++];
  }
  return -1;
}

uint8_t HardwareI2C_RPI::endTransmission(bool stopBit) {
  if (i2c_fd < 0 || i2c_tx_buffer.empty()) return 4;  // error
  ssize_t w = ::write(i2c_fd, i2c_tx_buffer.data(), i2c_tx_buffer.size());
  i2c_tx_buffer.clear();
  return (w < 0) ? 4 : 0;
}

size_t HardwareI2C_RPI::requestFrom(uint8_t address, size_t len, bool stopBit) {
  beginTransmission(address);
  i2c_rx_buffer.resize(len);
  ssize_t r = ::read(i2c_fd, i2c_rx_buffer.data(), len);
  i2c_rx_pos = 0;
  if (r < 0) {
    Logger.error("HardwareI2C_RPI: Failed to read");
    i2c_rx_buffer.clear();
    return 0;
  }
  i2c_rx_buffer.resize(r);
  return (size_t)r;
}

size_t HardwareI2C_RPI::requestFrom(uint8_t address, size_t len) {
  return requestFrom(address, len, true);
}

void HardwareI2C_RPI::onReceive(void (*cb)(int)) {
  Logger.error("HardwareI2C_RPI", "onReceive not implemented on Linux I2C");
}

void HardwareI2C_RPI::onRequest(void (*cb)(void)) {
  Logger.error("HardwareI2C_RPI", "onRequest not implemented on Linux I2C");
}

}  // namespace arduino
#endif
