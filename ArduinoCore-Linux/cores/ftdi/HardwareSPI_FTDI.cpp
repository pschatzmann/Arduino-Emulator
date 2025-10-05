/*
  HardwareSPI_FTDI.cpp 
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
#include "HardwareSPI_FTDI.h"
#include "Arduino.h"

namespace arduino {

HardwareSPI_FTDI::HardwareSPI_FTDI(int channel) : ftdi_channel(channel) {
}

HardwareSPI_FTDI::~HardwareSPI_FTDI() {
  end();
}

bool HardwareSPI_FTDI::begin(int vendor_id, int product_id, 
                            const char* description, const char* serial) {
  Logger.info("Initializing FTDI SPI interface");
  
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

  // Configure MPSSE mode
  if (!configureMPSSE()) {
    Logger.error("Failed to configure MPSSE mode");
    end();
    return false;
  }

  is_open = true;
  Logger.info("FTDI SPI interface initialized successfully");
  return true;
}

void HardwareSPI_FTDI::end() {
  if (ftdi_context) {
    ftdi_usb_close(ftdi_context);
    ftdi_free(ftdi_context);
    ftdi_context = nullptr;
  }
  is_open = false;
}

uint8_t HardwareSPI_FTDI::transfer(uint8_t data) {
  if (!is_open) {
    Logger.error("FTDI SPI not initialized");
    return 0;
  }

  uint8_t command = getMPSSECommand(true, true);
  uint8_t tx_buffer[3] = {command, 0, data}; // Command, length-1, data
  uint8_t rx_data = 0;

  if (sendData(tx_buffer, 3) != 3) {
    Logger.error("Failed to send SPI data");
    return 0;
  }

  if (receiveData(&rx_data, 1) != 1) {
    Logger.error("Failed to receive SPI data");
    return 0;
  }

  return rx_data;
}

uint16_t HardwareSPI_FTDI::transfer16(uint16_t data) {
  uint16_t result;
  if (current_bit_order == MSBFIRST) {
    result = transfer((data >> 8) & 0xFF) << 8;
    result |= transfer(data & 0xFF);
  } else {
    result = transfer(data & 0xFF);
    result |= transfer((data >> 8) & 0xFF) << 8;
  }
  return result;
}

void HardwareSPI_FTDI::transfer(void* buf, size_t count) {
  if (!is_open || !buf || count == 0) {
    Logger.error("Invalid parameters for SPI transfer");
    return;
  }

  uint8_t* data = static_cast<uint8_t*>(buf);
  uint8_t command = getMPSSECommand(true, true);
  
  // MPSSE can handle up to 65536 bytes at once
  for (size_t i = 0; i < count; i += 65536) {
    size_t chunk_size = (count - i > 65536) ? 65536 : count - i;
    
    // Prepare command buffer
    uint8_t cmd_buffer[3];
    cmd_buffer[0] = command;
    cmd_buffer[1] = (chunk_size - 1) & 0xFF;       // Length low byte
    cmd_buffer[2] = ((chunk_size - 1) >> 8) & 0xFF; // Length high byte
    
    // Send command
    if (sendData(cmd_buffer, 3) != 3) {
      Logger.error("Failed to send SPI command");
      return;
    }
    
    // Send data
    if (sendData(&data[i], chunk_size) != (int)chunk_size) {
      Logger.error("Failed to send SPI data chunk");
      return;
    }
    
    // Receive data (overwrites the same buffer)
    if (receiveData(&data[i], chunk_size) != (int)chunk_size) {
      Logger.error("Failed to receive SPI data chunk");
      return;
    }
  }
}

void HardwareSPI_FTDI::usingInterrupt(int interruptNumber) {
  Logger.warning("Interrupt handling not supported by FTDI SPI");
}

void HardwareSPI_FTDI::notUsingInterrupt(int interruptNumber) {
  Logger.warning("Interrupt handling not supported by FTDI SPI");
}

void HardwareSPI_FTDI::beginTransaction(SPISettings settings) {
  if (!is_open) {
    Logger.error("FTDI SPI not initialized");
    return;
  }

  current_clock = settings.getClockFreq();
  current_mode = settings.getDataMode();
  current_bit_order = settings.getBitOrder();

  // Update clock frequency
  setClockFrequency(current_clock);
}

void HardwareSPI_FTDI::endTransaction(void) {
  // Nothing specific to do for FTDI
}

void HardwareSPI_FTDI::attachInterrupt() {
  Logger.warning("Interrupt attachment not supported by FTDI SPI");
}

void HardwareSPI_FTDI::detachInterrupt() {
  Logger.warning("Interrupt detachment not supported by FTDI SPI");
}

bool HardwareSPI_FTDI::configureMPSSE() {
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

  // Set initial clock frequency
  setClockFrequency(current_clock);

  // Configure initial pin states
  // ADBUS0 = SCK (output), ADBUS1 = MOSI (output), ADBUS2 = MISO (input)
  uint8_t pin_config[] = {
    0x80, 0x00, 0x0B  // Set pins 0,1,3 as output, pin 2 as input
  };

  if (sendData(pin_config, sizeof(pin_config)) != sizeof(pin_config)) {
    Logger.error("Failed to configure SPI pins");
    return false;
  }

  return true;
}

bool HardwareSPI_FTDI::setClockFrequency(uint32_t frequency) {
  if (!ftdi_context) {
    return false;
  }

  // Calculate divisor for MPSSE clock
  // MPSSE base clock is 60MHz, we need divisor = (60MHz / (2 * frequency)) - 1
  uint32_t divisor = (30000000 / frequency) - 1;
  
  if (divisor > 65535) {
    divisor = 65535; // Minimum frequency ~457 Hz
  }

  uint8_t clock_cmd[] = {
    0x86,                          // Set clock divisor command
    (uint8_t)(divisor & 0xFF),     // Divisor low byte
    (uint8_t)((divisor >> 8) & 0xFF) // Divisor high byte
  };

  return sendData(clock_cmd, sizeof(clock_cmd)) == sizeof(clock_cmd);
}

uint8_t HardwareSPI_FTDI::getMPSSECommand(bool read_enable, bool write_enable) {
  uint8_t command = 0;

  if (write_enable) {
    command |= CMD_DO_WRITE;
  }
  if (read_enable) {
    command |= CMD_DO_READ;
  }

  // Handle SPI mode (clock polarity and phase)
  switch (current_mode) {
    case SPI_MODE1:
    case SPI_MODE3:
      command |= CMD_WRITE_NEG; // Write on negative edge
      break;
    default:
      break;
  }

  // Handle bit order
  if (current_bit_order == LSBFIRST) {
    command |= CMD_LSB;
  }

  return command;
}

int HardwareSPI_FTDI::sendData(const uint8_t* data, size_t length) {
  if (!ftdi_context || !data) {
    return -1;
  }
  return ftdi_write_data(ftdi_context, const_cast<uint8_t*>(data), length);
}

int HardwareSPI_FTDI::receiveData(uint8_t* data, size_t length) {
  if (!ftdi_context || !data) {
    return -1;
  }
  return ftdi_read_data(ftdi_context, data, length);
}

}  // namespace arduino

#endif  // USE_FTDI