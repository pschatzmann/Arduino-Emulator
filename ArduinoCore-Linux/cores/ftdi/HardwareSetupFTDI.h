#pragma once
/*
  HardwareSetupFTDI.h 
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
#if defined(USE_FTDI) && !defined(SKIP_HARDWARE_SETUP)
#include "FileStream.h"
#include "HardwareGPIO_FTDI.h"
#include "HardwareI2C_FTDI.h"
#include "HardwareSPI_FTDI.h"

namespace arduino {

/**
 * @class HardwareSetupFTDI
 * @brief Sets up hardware interfaces for FTDI FT2232HL (GPIO, I2C, SPI).
 * 
 * This class provides hardware abstraction for the FTDI FT2232HL dual-channel
 * USB-to-UART/SPI/I2C converter. The FT2232HL has two independent channels:
 * - Channel A: Can be configured for UART, SPI, I2C, or GPIO
 * - Channel B: Can be configured for UART, SPI, I2C, or GPIO
 * 
 * This implementation uses libftdi1 to communicate with the FT2232HL device.
 */
class HardwareSetupFTDI : public I2CSource, public SPISource, public GPIOSource {
 public:
  /**
   * @brief Constructor. Initializes FTDI hardware interfaces.
   */
  HardwareSetupFTDI() = default;

  /**
   * @brief Constructor with custom device parameters.
   * @param vendor_id USB vendor ID (default: 0x0403 for FTDI)
   * @param product_id USB product ID (default: 0x6010 for FT2232HL)
   * @param description Device description string (optional)
   * @param serial Serial number string (optional)
   */
  HardwareSetupFTDI(int vendor_id, int product_id, 
                    const char* description = nullptr, 
                    const char* serial = nullptr) 
    : vid(vendor_id), pid(product_id), desc(description), ser(serial) {}

  /**
   * @brief Destructor. Cleans up FTDI hardware interfaces.
   */
  ~HardwareSetupFTDI() { end(); }

  /**
   * @brief Initializes hardware pointers to FTDI interfaces.
   * @param asDefault If true, sets up global Arduino objects (Wire, SPI, GPIO)
   * @return true if initialization successful, false otherwise
   */
  bool begin(bool asDefault = true) {
    Logger.info("Using FTDI FT2232HL hardware interfaces");
    is_default_objects_active = asDefault;
    
    // Initialize GPIO first as it opens the FTDI context
    bool gpio_ok = gpio.begin(vid, pid, desc, ser);
    bool i2c_ok = i2c.begin(vid, pid, desc, ser);
    bool spi_ok = spi.begin(vid, pid, desc, ser);

    // Define the global hardware interfaces
    if (asDefault) {
      GPIO.setGPIO(&gpio);
      SPI.setSPI(&spi);
      Wire.setI2C(&i2c);
    }

    return gpio_ok && i2c_ok && spi_ok;
  }

  /**
   * @brief Resets hardware pointers to nullptr.
   */
  void end() {
    if (is_default_objects_active) {
      GPIO.setGPIO(nullptr);
      SPI.setSPI(nullptr);
      Wire.setI2C(nullptr);
    }
    gpio.end();
    i2c.end();
    spi.end();
  }

  HardwareGPIO_FTDI* getGPIO() { return &gpio; }
  HardwareI2C_FTDI* getI2C() { return &i2c; }
  HardwareSPI_FTDI* getSPI() { return &spi; }

  /**
   * @brief Set custom device parameters for targeting specific FTDI device.
   * @param vendor_id USB vendor ID
   * @param product_id USB product ID  
   * @param description Device description string (optional)
   * @param serial Serial number string (optional)
   */
  void setDeviceParams(int vendor_id, int product_id, 
                      const char* description = nullptr, 
                      const char* serial = nullptr) {
    vid = vendor_id;
    pid = product_id;
    desc = description;
    ser = serial;
  }

 protected:
  HardwareGPIO_FTDI gpio;
  HardwareI2C_FTDI i2c;
  HardwareSPI_FTDI spi;
  bool is_default_objects_active = false;
  
  // Device identification parameters
  int vid = 0x0403;          // FTDI vendor ID
  int pid = 0x6010;          // FT2232HL product ID
  const char* desc = nullptr; // Device description
  const char* ser = nullptr;  // Device serial number
};

/**
 * @brief Global instance for FTDI FT2232HL hardware setup.
 *
 * Use this object to access and initialize GPIO, I2C, and SPI interfaces on
 * FTDI FT2232HL devices.
 */
static HardwareSetupFTDI FTDI;

}  // namespace arduino

#endif