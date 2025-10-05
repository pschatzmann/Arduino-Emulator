#pragma once
/*
  HardwareSPI_FTDI.h 
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
#ifdef USE_FTDI
#include <inttypes.h>
#include <ftdi.h>
// Undefine DEPRECATED macro from libftdi1 to avoid conflict with Arduino API
#ifdef DEPRECATED
#undef DEPRECATED
#endif

#include "api/Common.h"
#include "api/HardwareSPI.h"
#include "api/Stream.h"

namespace arduino {

/**
 * @class HardwareSPI_FTDI
 * @brief Implementation of SPI communication for FTDI FT2232HL using MPSSE mode.
 *
 * This class provides an interface to the SPI bus on FTDI FT2232HL devices using
 * the Multi-Protocol Synchronous Serial Engine (MPSSE) mode. The FT2232HL can
 * operate both channels in MPSSE mode, allowing for high-speed SPI communication.
 *
 * Pin mapping for SPI on FT2232HL (Channel A):
 * - ADBUS0: SCK (Clock)
 * - ADBUS1: MOSI (Data Out)
 * - ADBUS2: MISO (Data In)
 * - ADBUS3-7: Available for chip select or other GPIO
 *
 * The class inherits from HardwareSPI and implements all required methods for
 * SPI communication, including data transfer, transaction management, and configuration.
 *
 * @note This class is only available when USE_FTDI is defined.
 * @note Requires libftdi1 development library to be installed.
 */
class HardwareSPI_FTDI : public HardwareSPI {
 public:
  /**
   * @brief Constructor for FTDI SPI interface.
   * @param channel FTDI channel to use (0 for Channel A, 1 for Channel B)
   */
  HardwareSPI_FTDI(int channel = 0);

  /**
   * @brief Destructor - closes FTDI connection.
   */
  ~HardwareSPI_FTDI() override;

  void begin() override {
    begin(0x0403, 0x6010);
  }

  /**
   * @brief Initialize the SPI interface.
   * @param vendor_id USB vendor ID (default: 0x0403)
   * @param product_id USB product ID (default: 0x6010 for FT2232HL)
   * @param description Device description string (optional)
   * @param serial Device serial number (optional)
   * @return true if initialization successful, false otherwise
   */
  bool begin(int vendor_id, int product_id, 
            const char* description = nullptr, const char* serial = nullptr);

  /**
   * @brief Close the SPI interface and cleanup resources.
   */
  void end() override;

  /**
   * @brief Transfer a single byte over SPI.
   * @param data Byte to send
   * @return Received byte
   */
  uint8_t transfer(uint8_t data) override;

  /**
   * @brief Transfer a 16-bit value over SPI.
   * @param data 16-bit value to send
   * @return Received 16-bit value
   */
  uint16_t transfer16(uint16_t data) override;

  /**
   * @brief Transfer a buffer of data over SPI.
   * @param buf Buffer containing data to send (will be overwritten with received data)
   * @param count Number of bytes to transfer
   */
  void transfer(void* buf, size_t count) override;

  // Transaction Functions
  /**
   * @brief Register interrupt usage (not implemented for FTDI).
   * @param interruptNumber Interrupt number (ignored)
   */
  void usingInterrupt(int interruptNumber) override;

  /**
   * @brief Unregister interrupt usage (not implemented for FTDI).
   * @param interruptNumber Interrupt number (ignored)
   */
  void notUsingInterrupt(int interruptNumber) override;

  /**
   * @brief Begin an SPI transaction with specific settings.
   * @param settings SPI settings (clock, bit order, mode)
   */
  void beginTransaction(SPISettings settings) override;

  /**
   * @brief End the current SPI transaction.
   */
  void endTransaction(void) override;

  // SPI Configuration methods
  /**
   * @brief Attach interrupt (not implemented for FTDI).
   */
  void attachInterrupt() override;

  /**
   * @brief Detach interrupt (not implemented for FTDI).
   */
  void detachInterrupt() override;

  /**
   * @brief Boolean conversion operator.
   * @return true if the FTDI SPI interface is open and initialized, false otherwise.
   */
  operator bool() { return is_open && ftdi_context != nullptr; }

 protected:
  struct ftdi_context* ftdi_context = nullptr;
  int ftdi_channel = 0;  // 0 for Channel A, 1 for Channel B
  bool is_open = false;
  
  // Current SPI settings
  uint32_t current_clock = 1000000;     // Default 1MHz
  SPIMode current_mode = SPI_MODE0;     // Default mode 0
  BitOrder current_bit_order = MSBFIRST; // Default MSB first
  
    // MPSSE command definitions for SPI (using different names to avoid macro conflicts)
  static const uint8_t CMD_WRITE_NEG = 0x01;     // Write TDI/DO on negative clock edge
  static const uint8_t CMD_BITMODE = 0x02;       // Write in bit mode
  static const uint8_t CMD_READ_NEG = 0x04;      // Read TDO/DI on negative clock edge
  static const uint8_t CMD_LSB = 0x08;           // LSB first
  static const uint8_t CMD_DO_WRITE = 0x10;      // Write TDI/DO
  static const uint8_t CMD_DO_READ = 0x20;       // Read TDO/DI
  static const uint8_t CMD_WRITE_TMS = 0x40;     // Write TMS/CS

  /**
   * @brief Configure FTDI device for MPSSE mode.
   * @return true if successful, false on error
   */
  bool configureMPSSE();

  /**
   * @brief Set the SPI clock frequency.
   * @param frequency Clock frequency in Hz
   * @return true if successful, false on error
   */
  bool setClockFrequency(uint32_t frequency);

  /**
   * @brief Get MPSSE command byte based on current SPI settings.
   * @param read_enable Enable reading data
   * @param write_enable Enable writing data
   * @return MPSSE command byte
   */
  uint8_t getMPSSECommand(bool read_enable, bool write_enable);

  /**
   * @brief Send raw data to FTDI device.
   * @param data Pointer to data buffer
   * @param length Number of bytes to send
   * @return Number of bytes actually sent, or negative on error
   */
  int sendData(const uint8_t* data, size_t length);

  /**
   * @brief Receive raw data from FTDI device.
   * @param data Pointer to receive buffer
   * @param length Number of bytes to receive
   * @return Number of bytes actually received, or negative on error
   */
  int receiveData(uint8_t* data, size_t length);
};

}  // namespace arduino

#endif  // USE_FTDI