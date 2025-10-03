#pragma once
/*
  HardwareSPI_RPI.h 
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
#ifdef USE_RPI
#include <inttypes.h>

#include "api/Common.h"
#include "api/HardwareSPI.h"
#include "api/Stream.h"

namespace arduino {

/**
 * @class HardwareSPI_RPI
 * @brief Implementation of SPI communication for Raspberry Pi using Linux SPI device interface.
 *
 * This class provides an interface to the SPI bus on Raspberry Pi platforms by accessing
 * the Linux device (e.g., /dev/spidev0.0). It inherits from HardwareSPI and implements all
 * required methods for SPI communication, including data transfer, transaction management, and configuration.
 *
 * @note This class is only available when USE_RPI is defined.
 */
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

  operator bool() { return is_open;  }
  
 protected:
  int spi_fd = -1;
  const char* device = "/dev/spidev0.0";
  uint32_t spi_speed = 500000;  // Default to 500kHz
  uint8_t spi_mode = 0;         // Default to SPI mode 0
  uint8_t spi_bits = 8;         // Default to 8 bits per word
  bool is_open = false;
};

}  // namespace arduino

#endif  // USE_RPI
