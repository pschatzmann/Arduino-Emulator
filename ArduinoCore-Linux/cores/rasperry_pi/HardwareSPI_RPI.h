#pragma once
#ifdef USE_RPI
#include <inttypes.h>

#include "Common.h"
#include "HardwareSPI.h"
#include "Stream.h"

namespace arduino {

class HardwareSPI_RPI : public HardwareSPI {
 public:
  HardwareSPI_RPI(const char* device = "/dev/spidev0.0");
  ~HardwareSPI_RPI() override;

  uint8_t transfer(uint8_t data) override;
  uint16_t transfer16(uint16_t data) override;
  void transfer(void* buf, size_t count) override;

  // Transaction Functions
  void usingInterrupt(int interruptNumber) override;
  void notUsingInterrupt(int interruptNumber) override;
  void beginTransaction(SPISettings settings) override;
  void endTransaction(void) override;

  // SPI Configuration methods
  void attachInterrupt() override;
  void detachInterrupt() override;

  void begin() override;
  void end() override;

 protected:
  int spi_fd = -1;
  const char* device = "/dev/spidev0.0";
  uint32_t spi_speed = 500000;  // Default to 500kHz
  uint8_t spi_mode = 0;         // Default to SPI mode 0
  uint8_t spi_bits = 8;         // Default to 8 bits per word
};

}  // namespace arduino

#endif  // USE_RPI
