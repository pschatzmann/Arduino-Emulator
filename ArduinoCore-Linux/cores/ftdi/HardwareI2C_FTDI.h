#pragma once
/*
  HardwareI2C_FTDI.h 
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
#include "api/HardwareI2C.h"
#include "api/Stream.h"

namespace arduino {

/**
 * @class HardwareI2C_FTDI
 * @brief Implementation of I2C communication for FTDI FT2232HL using MPSSE mode.
 *
 * This class provides an interface to the I2C bus on FTDI FT2232HL devices using
 * the Multi-Protocol Synchronous Serial Engine (MPSSE) mode. The FT2232HL can
 * operate in MPSSE mode to provide I2C master functionality.
 *
 * Pin mapping for I2C on FT2232HL (Channel A):
 * - ADBUS0: SCL (Clock) - requires 4.7kΩ pull-up resistor
 * - ADBUS1: SDA_OUT (Data Out)
 * - ADBUS2: SDA_IN (Data In) - requires 4.7kΩ pull-up resistor
 * - ADBUS3-7: Available for other functions
 *
 * The class inherits from HardwareI2C and implements all required methods for
 * I2C communication, including device addressing, data transfer, and bus management.
 *
 * @note This class is only available when USE_FTDI is defined.
 * @note Requires libftdi1 development library to be installed.
 * @note External pull-up resistors (typically 4.7kΩ) are required on SCL and SDA lines.
 */
class HardwareI2C_FTDI : public HardwareI2C {
 public:
  /**
   * @brief Constructor for FTDI I2C interface.
   * @param channel FTDI channel to use (0 for Channel A, 1 for Channel B)
   */
  HardwareI2C_FTDI(int channel = 0);

  /**
   * @brief Destructor - closes FTDI connection.
   */
  ~HardwareI2C_FTDI();

  /**
   * @brief Initialize the I2C interface.
   * @param vendor_id USB vendor ID (default: 0x0403)
   * @param product_id USB product ID (default: 0x6010 for FT2232HL)
   * @param description Device description string (optional)
   * @param serial Device serial number (optional)
   * @return true if initialization successful, false otherwise
   */
  bool begin(int vendor_id = 0x0403, int product_id = 0x6010, 
            const char* description = nullptr, const char* serial = nullptr);

  /**
   * @brief Initialize I2C as master (same as begin()).
   */
  void begin() override { begin(0x0403, 0x6010); }

  /**
   * @brief Initialize I2C as slave (not supported by FTDI).
   * @param address Slave address (ignored)
   */
  void begin(uint8_t address) override;

  /**
   * @brief Close the I2C interface and cleanup resources.
   */
  void end() override;

  /**
   * @brief Set the I2C clock frequency.
   * @param freq Frequency in Hz (default: 100000 for 100kHz)
   */
  void setClock(uint32_t freq) override;

  /**
   * @brief Begin transmission to an I2C slave device.
   * @param address 7-bit slave address
   */
  void beginTransmission(uint8_t address) override;

  /**
   * @brief Write a single byte to the I2C bus.
   * @param data Byte to write
   * @return Number of bytes written (1 on success, 0 on error)
   */
  size_t write(uint8_t data) override;

  /**
   * @brief Write multiple bytes to the I2C bus.
   * @param data Pointer to data buffer
   * @param len Number of bytes to write
   * @return Number of bytes written
   */
  size_t write(const uint8_t* data, size_t len) override;

  /**
   * @brief End transmission and send data to slave device.
   * @param stopBit Whether to send stop condition (default: true)
   * @return 0 on success, error code on failure
   */
  uint8_t endTransmission(bool stopBit = true) override;
  
  /**
   * @brief End transmission and send data to slave device (with stop condition).
   * @return 0 on success, error code on failure
   */
  uint8_t endTransmission(void) override;

  /**
   * @brief Request data from an I2C slave device.
   * @param address 7-bit slave address
   * @param quantity Number of bytes to request
   * @param stopBit Whether to send stop condition (default: true)
   * @return Number of bytes received
   */
  size_t requestFrom(uint8_t address, size_t quantity, bool stopBit = true) override;
  size_t requestFrom(uint8_t address, size_t quantity) override;

  /**
   * @brief Get the number of bytes available for reading.
   * @return Number of bytes available
   */
  int available() override;

  /**
   * @brief Peek at the next byte without removing it from buffer.
   * @return Next byte, or -1 if no data available
   */
  int peek() override;

  /**
   * @brief Read a byte from the receive buffer.
   * @return Received byte, or -1 if no data available
   */
  int read() override;

  /**
   * @brief Flush the output buffer (no-op for I2C).
   */
  void flush() override;

  /**
   * @brief Register a function to be called when data is received as a slave.
   * @param function Callback function (not supported by FTDI master-only mode)
   * @note FTDI operates in master mode only - slave callbacks not supported
   */
  void onReceive(void(*function)(int)) override;

  /**
   * @brief Register a function to be called when master requests data from slave.
   * @param function Callback function (not supported by FTDI master-only mode)
   * @note FTDI operates in master mode only - slave callbacks not supported
   */
  void onRequest(void(*function)(void)) override;

  /**
   * @brief Boolean conversion operator.
   * @return true if the FTDI I2C interface is open and initialized, false otherwise.
   */
  operator bool() { return is_open && ftdi_context != nullptr; }

 protected:
  struct ftdi_context* ftdi_context = nullptr;
  int ftdi_channel = 0;  // 0 for Channel A, 1 for Channel B
  bool is_open = false;
  
  // I2C state
  uint8_t current_slave_address = 0;
  uint32_t i2c_clock_frequency = 100000;  // Default 100kHz
  
  // Transmission buffer
  uint8_t tx_buffer[32];  // Buffer for outgoing data
  size_t tx_buffer_length = 0;
  
  // Receive buffer
  uint8_t rx_buffer[32];  // Buffer for incoming data
  size_t rx_buffer_length = 0;
  size_t rx_buffer_index = 0;
  
  // MPSSE I2C command constants
  static const uint8_t I2C_DATA_SHIFT_OUT = 0x11;
  static const uint8_t I2C_DATA_SHIFT_IN = 0x24;
  static const uint8_t I2C_DATA_SHIFT_OUT_IN = 0x31;
  static const uint8_t I2C_SET_DATA_BITS_LOW_BYTE = 0x80;
  static const uint8_t I2C_SET_DATA_BITS_HIGH_BYTE = 0x82;
  static const uint8_t I2C_GET_DATA_BITS_LOW_BYTE = 0x81;
  static const uint8_t I2C_GET_DATA_BITS_HIGH_BYTE = 0x83;

  // Pin assignments for I2C on ADBUS
  static const uint8_t SCL_BIT = 0;   // ADBUS0
  static const uint8_t SDA_OUT_BIT = 1; // ADBUS1
  static const uint8_t SDA_IN_BIT = 2;  // ADBUS2

  /**
   * @brief Configure FTDI device for I2C MPSSE mode.
   * @return true if successful, false on error
   */
  bool configureMPSSE();

  /**
   * @brief Set the I2C clock frequency.
   * @param frequency Clock frequency in Hz
   * @return true if successful, false on error
   */
  bool setClockFrequency(uint32_t frequency);

  /**
   * @brief Send I2C start condition.
   * @return true if successful, false on error
   */
  bool sendStart();

  /**
   * @brief Send I2C stop condition.
   * @return true if successful, false on error
   */
  bool sendStop();

  /**
   * @brief Send I2C repeated start condition.
   * @return true if successful, false on error
   */
  bool sendRepeatedStart();

  /**
   * @brief Send a byte over I2C and check for ACK.
   * @param data Byte to send
   * @return true if ACK received, false if NACK or error
   */
  bool sendByteCheckAck(uint8_t data);

  /**
   * @brief Receive a byte over I2C and send ACK/NACK.
   * @param send_ack Whether to send ACK (true) or NACK (false)
   * @return Received byte
   */
  uint8_t receiveByteSendAck(bool send_ack);

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

  /**
   * @brief Set the state of SDA and SCL lines.
   * @param sda_state State of SDA line (true = high, false = low)
   * @param scl_state State of SCL line (true = high, false = low)
   * @return true if successful, false on error
   */
  bool setLineStates(bool sda_state, bool scl_state);
};

}  // namespace arduino

#endif  // USE_FTDI