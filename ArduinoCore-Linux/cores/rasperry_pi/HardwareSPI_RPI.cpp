#ifdef USE_RPI

#include "HardwareSPI_RPI.h"
#include "ArduinoLogger.h"

#include <fcntl.h>
#include <linux/spi/spidev.h>
#include <sys/ioctl.h>
#include <unistd.h>

#include <cstdio>
#include <cstring>

namespace arduino {

HardwareSPI_RPI::HardwareSPI_RPI(const char* device) {
  spi_fd = -1;
  this->device = device;
  begin();
}

HardwareSPI_RPI::~HardwareSPI_RPI() {
  end();
}
void HardwareSPI_RPI::begin() {
  spi_fd = open(device, O_RDWR);
  if (spi_fd < 0) {
    Logger.error("HardwareSPI_RPI: Failed to open SPI device");
  }
  ioctl(spi_fd, SPI_IOC_WR_MODE, &spi_mode);
  ioctl(spi_fd, SPI_IOC_WR_BITS_PER_WORD, &spi_bits);
  ioctl(spi_fd, SPI_IOC_WR_MAX_SPEED_HZ, &spi_speed);
}

void HardwareSPI_RPI::end() {
  if (spi_fd >= 0) close(spi_fd);
  spi_fd = -1;
}
uint8_t HardwareSPI_RPI::transfer(uint8_t data) {
  uint8_t rx = 0;
  struct spi_ioc_transfer tr = {};
  tr.tx_buf = (unsigned long)&data;
  tr.rx_buf = (unsigned long)&rx;
  tr.len = 1;
  tr.speed_hz = spi_speed;
  tr.bits_per_word = spi_bits;
  tr.delay_usecs = 0;
  ioctl(spi_fd, SPI_IOC_MESSAGE(1), &tr);
  return rx;
}
uint16_t HardwareSPI_RPI::transfer16(uint16_t data) {
  uint16_t rx = 0;
  struct spi_ioc_transfer tr = {};
  tr.tx_buf = (unsigned long)&data;
  tr.rx_buf = (unsigned long)&rx;
  tr.len = 2;
  tr.speed_hz = spi_speed;
  tr.bits_per_word = spi_bits;
  tr.delay_usecs = 0;
  ioctl(spi_fd, SPI_IOC_MESSAGE(1), &tr);
  return rx;
}
void HardwareSPI_RPI::transfer(void* buf, size_t count) {
  struct spi_ioc_transfer tr = {};
  tr.tx_buf = (unsigned long)buf;
  tr.rx_buf = (unsigned long)buf;
  tr.len = count;
  tr.speed_hz = spi_speed;
  tr.bits_per_word = spi_bits;
  tr.delay_usecs = 0;
  ioctl(spi_fd, SPI_IOC_MESSAGE(1), &tr);
}
void HardwareSPI_RPI::usingInterrupt(int interruptNumber) {
  Logger.error("HardwareSPI_RPI: usingInterrupt not implemented");
}
void HardwareSPI_RPI::notUsingInterrupt(int interruptNumber) {
  Logger.error("HardwareSPI_RPI: notUsingInterrupt not implemented");
}
void HardwareSPI_RPI::beginTransaction(SPISettings settings) {
  spi_mode = settings.getDataMode();
  spi_speed = settings.getClockFreq();
  BitOrder order = settings.getBitOrder();
  uint8_t lsb_first = (order == LSBFIRST) ? 1 : 0;
  ioctl(spi_fd, SPI_IOC_WR_MODE, &spi_mode);
  ioctl(spi_fd, SPI_IOC_WR_LSB_FIRST, &lsb_first);
  ioctl(spi_fd, SPI_IOC_WR_MAX_SPEED_HZ, &spi_speed);
}
void HardwareSPI_RPI::endTransaction(void) {}
void HardwareSPI_RPI::attachInterrupt() {
  Logger.error("HardwareSPI_RPI: attachInterrupt not implemented");
}
void HardwareSPI_RPI::detachInterrupt() {
  Logger.error("HardwareSPI_RPI: detachInterrupt not implemented");
}

}  // namespace arduino

#endif  // USE_RPI
