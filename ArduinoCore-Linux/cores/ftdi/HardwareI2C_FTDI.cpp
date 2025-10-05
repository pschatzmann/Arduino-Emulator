/*
  HardwareI2C_FTDI.cpp 
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
#include "HardwareI2C_FTDI.h"
#include "Arduino.h"
#include <chrono>
#include <thread>

// Helper function for microsecond delays
// Forward declarations

namespace arduino {

HardwareI2C_FTDI::HardwareI2C_FTDI(int channel) : ftdi_channel(channel) {
}

HardwareI2C_FTDI::~HardwareI2C_FTDI() {
  end();
}

bool HardwareI2C_FTDI::begin(int vendor_id, int product_id, 
                            const char* description, const char* serial) {
  Logger.info("Initializing FTDI I2C interface");
  
  // Initialize FTDI context
  ftdi_context = ftdi_new();
  if (!ftdi_context) {
    Logger.error("Failed to create FTDI context");
    return false;
  }

  // Set interface (Channel A or B)
  int ret = ftdi_set_interface(ftdi_context, ftdi_channel == 0 ? INTERFACE_A : INTERFACE_B);
  if (ret < 0) {
    Logger.error("Failed to set FTDI interface: %s", ftdi_get_error_string(ftdi_context));
    ftdi_free(ftdi_context);
    ftdi_context = nullptr;
    return false;
  }

  // Open device
  if (serial) {
    ret = ftdi_usb_open_desc(ftdi_context, vendor_id, product_id, description, serial);
  } else if (description) {
    ret = ftdi_usb_open_desc(ftdi_context, vendor_id, product_id, description, nullptr);
  } else {
    ret = ftdi_usb_open(ftdi_context, vendor_id, product_id);
  }

  if (ret < 0) {
    Logger.error("Failed to open FTDI device: %s", ftdi_get_error_string(ftdi_context));
    ftdi_free(ftdi_context);
    ftdi_context = nullptr;
    return false;
  }

  // Reset device
  ret = ftdi_usb_reset(ftdi_context);
  if (ret < 0) {
    Logger.error("Failed to reset FTDI device: %s", ftdi_get_error_string(ftdi_context));
    end();
    return false;
  }

  // Configure MPSSE mode for I2C
  if (!configureMPSSE()) {
    Logger.error("Failed to configure MPSSE mode for I2C");
    end();
    return false;
  }

  is_open = true;
  Logger.info("FTDI I2C interface initialized successfully");
  return true;
}

void HardwareI2C_FTDI::begin(uint8_t address) {
  Logger.error("I2C slave mode not supported by FTDI");
}

void HardwareI2C_FTDI::end() {
  if (ftdi_context) {
    ftdi_usb_close(ftdi_context);
    ftdi_free(ftdi_context);
    ftdi_context = nullptr;
  }
  is_open = false;
  tx_buffer_length = 0;
  rx_buffer_length = 0;
  rx_buffer_index = 0;
}

void HardwareI2C_FTDI::setClock(uint32_t freq) {
  i2c_clock_frequency = freq;
  if (is_open) {
    setClockFrequency(freq);
  }
}

void HardwareI2C_FTDI::beginTransmission(uint8_t address) {
  current_slave_address = address;
  tx_buffer_length = 0;
}

size_t HardwareI2C_FTDI::write(uint8_t data) {
  if (tx_buffer_length >= sizeof(tx_buffer)) {
    Logger.error("I2C transmit buffer full");
    return 0;
  }
  
  tx_buffer[tx_buffer_length++] = data;
  return 1;
}

size_t HardwareI2C_FTDI::write(const uint8_t* data, size_t len) {
  size_t written = 0;
  for (size_t i = 0; i < len && tx_buffer_length < sizeof(tx_buffer); i++) {
    tx_buffer[tx_buffer_length++] = data[i];
    written++;
  }
  return written;
}

uint8_t HardwareI2C_FTDI::endTransmission(bool stopBit) {
  if (!is_open) {
    Logger.error("FTDI I2C not initialized");
    return 4; // Other error
  }

  // Send start condition
  if (!sendStart()) {
    Logger.error("Failed to send I2C start condition");
    return 4;
  }

  // Send slave address with write bit
  if (!sendByteCheckAck((current_slave_address << 1) | 0)) {
    Logger.error("I2C slave address NACK");
    if (stopBit) sendStop();
    return 2; // Address NACK
  }

  // Send data bytes
  for (size_t i = 0; i < tx_buffer_length; i++) {
    if (!sendByteCheckAck(tx_buffer[i])) {
      Logger.error("I2C data NACK");
      if (stopBit) sendStop();
      return 3; // Data NACK
    }
  }

  // Send stop condition if requested
  if (stopBit) {
    if (!sendStop()) {
      Logger.error("Failed to send I2C stop condition");
      return 4;
    }
  }

  tx_buffer_length = 0;
  return 0; // Success
}

size_t HardwareI2C_FTDI::requestFrom(uint8_t address, size_t quantity, bool stopBit) {
  if (!is_open) {
    Logger.error("FTDI I2C not initialized");
    return 0;
  }

  if (quantity > sizeof(rx_buffer)) {
    quantity = sizeof(rx_buffer);
  }

  // Send start condition
  if (!sendStart()) {
    Logger.error("Failed to send I2C start condition");
    return 0;
  }

  // Send slave address with read bit
  if (!sendByteCheckAck((address << 1) | 1)) {
    Logger.error("I2C slave address NACK on read");
    if (stopBit) sendStop();
    return 0;
  }

  // Read data bytes
  rx_buffer_length = 0;
  rx_buffer_index = 0;
  
  for (uint8_t i = 0; i < quantity; i++) {
    bool send_ack = (i < quantity - 1); // ACK all but last byte
    rx_buffer[rx_buffer_length++] = receiveByteSendAck(send_ack);
  }

  // Send stop condition if requested
  if (stopBit) {
    if (!sendStop()) {
      Logger.error("Failed to send I2C stop condition");
    }
  }

  return rx_buffer_length;
}

int HardwareI2C_FTDI::available() {
  return rx_buffer_length - rx_buffer_index;
}

int HardwareI2C_FTDI::peek() {
  if (rx_buffer_index >= rx_buffer_length) {
    return -1;
  }
  return rx_buffer[rx_buffer_index];
}

int HardwareI2C_FTDI::read() {
  if (rx_buffer_index >= rx_buffer_length) {
    return -1;
  }
  return rx_buffer[rx_buffer_index++];
}

void HardwareI2C_FTDI::flush() {
  // No-op for I2C
}

bool HardwareI2C_FTDI::configureMPSSE() {
  if (!ftdi_context) {
    return false;
  }

  // Set MPSSE mode
  int ret = ftdi_set_bitmode(ftdi_context, 0x00, BITMODE_MPSSE);
  if (ret < 0) {
    Logger.error("Failed to set MPSSE mode: %s", ftdi_get_error_string(ftdi_context));
    return false;
  }

  // Purge buffers (using modern API to avoid deprecation warnings)
  ftdi_usb_purge_rx_buffer(ftdi_context);
  ftdi_usb_purge_tx_buffer(ftdi_context);

  // Send MPSSE initialization commands
  uint8_t init_commands[] = {
    0x8A,  // Disable divide-by-5
    0x97,  // Turn off adaptive clocking
    0x8D   // Disable three-phase clocking
  };

  if (sendData(init_commands, sizeof(init_commands)) != sizeof(init_commands)) {
    Logger.error("Failed to send MPSSE initialization commands");
    return false;
  }

  // Set clock frequency
  setClockFrequency(i2c_clock_frequency);

  // Configure I2C pins: SCL=ADBUS0, SDA_OUT=ADBUS1, SDA_IN=ADBUS2
  // Set SCL and SDA high initially (idle state)
  return setLineStates(true, true);
}

bool HardwareI2C_FTDI::setClockFrequency(uint32_t frequency) {
  if (!ftdi_context) {
    return false;
  }

  // Calculate divisor for I2C clock
  // For I2C, we need lower frequency, so use different calculation
  uint32_t divisor = (12000000 / (frequency * 4)) - 1;
  
  if (divisor > 65535) {
    divisor = 65535;
  }

  uint8_t clock_cmd[] = {
    0x86,                          // Set clock divisor command
    (uint8_t)(divisor & 0xFF),     // Divisor low byte
    (uint8_t)((divisor >> 8) & 0xFF) // Divisor high byte
  };

  return sendData(clock_cmd, sizeof(clock_cmd)) == sizeof(clock_cmd);
}

bool HardwareI2C_FTDI::sendStart() {
  // I2C start: SDA high->low while SCL high
  if (!setLineStates(true, true)) return false;   // Both high
  std::this_thread::sleep_for(std::chrono::microseconds(1));
  if (!setLineStates(false, true)) return false;  // SDA low, SCL high  
  std::this_thread::sleep_for(std::chrono::microseconds(1));
  if (!setLineStates(false, false)) return false; // Both low
  return true;
}

bool HardwareI2C_FTDI::sendStop() {
  // I2C stop: SDA low->high while SCL high
  if (!setLineStates(false, false)) return false; // Both low
  std::this_thread::sleep_for(std::chrono::microseconds(1));
  if (!setLineStates(false, true)) return false;  // SDA low, SCL high
  std::this_thread::sleep_for(std::chrono::microseconds(1));
  if (!setLineStates(true, true)) return false;   // Both high
  return true;
}

bool HardwareI2C_FTDI::sendRepeatedStart() {
  return sendStart();
}

bool HardwareI2C_FTDI::sendByteCheckAck(uint8_t data) {
  // Send 8 data bits
  uint8_t send_cmd[] = {
    I2C_DATA_SHIFT_OUT,
    0x07,  // 8 bits - 1
    data
  };
  
  if (sendData(send_cmd, sizeof(send_cmd)) != sizeof(send_cmd)) {
    return false;
  }

  // Read ACK bit
  uint8_t read_ack_cmd[] = {
    I2C_DATA_SHIFT_IN,
    0x00  // 1 bit - 1
  };
  
  if (sendData(read_ack_cmd, sizeof(read_ack_cmd)) != sizeof(read_ack_cmd)) {
    return false;
  }

  uint8_t ack_bit;
  if (receiveData(&ack_bit, 1) != 1) {
    return false;
  }

  return (ack_bit & 0x01) == 0; // ACK is low
}

uint8_t HardwareI2C_FTDI::receiveByteSendAck(bool send_ack) {
  // Read 8 data bits
  uint8_t read_cmd[] = {
    I2C_DATA_SHIFT_IN,
    0x07  // 8 bits - 1
  };
  
  if (sendData(read_cmd, sizeof(read_cmd)) != sizeof(read_cmd)) {
    return 0;
  }

  uint8_t data;
  if (receiveData(&data, 1) != 1) {
    return 0;
  }

  // Send ACK/NACK
  uint8_t ack_bit = send_ack ? 0x00 : 0x80; // ACK=low, NACK=high
  uint8_t send_ack_cmd[] = {
    I2C_DATA_SHIFT_OUT,
    0x00,   // 1 bit - 1
    ack_bit
  };
  
  sendData(send_ack_cmd, sizeof(send_ack_cmd));

  return data;
}

int HardwareI2C_FTDI::sendData(const uint8_t* data, size_t length) {
  if (!ftdi_context || !data) {
    return -1;
  }
  return ftdi_write_data(ftdi_context, const_cast<uint8_t*>(data), length);
}

int HardwareI2C_FTDI::receiveData(uint8_t* data, size_t length) {
  if (!ftdi_context || !data) {
    return -1;
  }
  return ftdi_read_data(ftdi_context, data, length);
}

bool HardwareI2C_FTDI::setLineStates(bool sda_state, bool scl_state) {
  uint8_t pin_value = 0;
  uint8_t pin_direction = (1 << SCL_BIT) | (1 << SDA_OUT_BIT); // SCL and SDA_OUT as outputs
  
  if (scl_state) {
    pin_value |= (1 << SCL_BIT);
  }
  if (sda_state) {
    pin_value |= (1 << SDA_OUT_BIT);
  }

  uint8_t cmd[] = {
    I2C_SET_DATA_BITS_LOW_BYTE,
    pin_value,
    pin_direction
  };

  return sendData(cmd, sizeof(cmd)) == sizeof(cmd);
}



size_t HardwareI2C_FTDI::requestFrom(uint8_t address, size_t quantity) {
  return requestFrom(address, quantity, true);
}

uint8_t HardwareI2C_FTDI::endTransmission(void) {
  return endTransmission(true);
}

void HardwareI2C_FTDI::onReceive(void(*function)(int)) {
  Logger.warning("I2C slave callbacks not supported by FTDI master-only mode");
  // FTDI operates in master mode only - slave functionality not available
}

void HardwareI2C_FTDI::onRequest(void(*function)(void)) {
  Logger.warning("I2C slave callbacks not supported by FTDI master-only mode");
  // FTDI operates in master mode only - slave functionality not available
}

}  // namespace arduino

#endif  // USE_FTDI