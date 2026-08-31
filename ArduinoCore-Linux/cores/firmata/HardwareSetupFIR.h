#pragma once
/*
  HardwareSetupFIR.h
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
#if defined(USE_FIRMATA) && !defined(SKIP_HARDWARE_SETUP)
#include "FirmataTransport.h"
#include "HardwareGPIO_FIR.h"
#include "HardwareI2C_FIR.h"
#include "HardwareSPI_FIR.h"

namespace arduino {

/**
 * @class HardwareSetupFIRMATA
 * @brief Sets up hardware interfaces (GPIO, I2C, SPI) that talk to a
 * remote board running Firmata, all multiplexed over one FirmataTransport.
 *
 * Unlike HardwareSetupRPI/HardwareSetupFTDI, which reach local hardware
 * directly, Firmata needs a Stream connected to the remote device - so
 * begin() takes one, instead of being callable with no arguments.
 *
 * @note GPIO and I2C work against plain StandardFirmata. SPI requires the
 *       remote device to run ConfigurableFirmata with its Spi feature
 *       enabled (see HardwareSPI_FIR.h).
 */
class HardwareSetupFIRMATA : public I2CSource, public SPISource, public GPIOSource {
 public:
  HardwareSetupFIRMATA() = default;

  /**
   * @param spiDeviceId Device identifier used for SPI_DEVICE_CONFIG; low 2
   *        bits must be 0. Only relevant if SPI is used.
   * @param spiCsPin Remote-device pin number to use as chip-select, or -1
   *        to toggle CS externally via GPIO.
   */
  explicit HardwareSetupFIRMATA(uint8_t spiDeviceId, int8_t spiCsPin = -1)
      : spi(spiDeviceId, spiCsPin) {}

  ~HardwareSetupFIRMATA() { end(); }

  /**
   * @brief Start the shared Firmata transport and all three backends.
   * @param stream Already-opened stream connected to the Firmata device.
   * @param asDefault If true, sets up global Arduino objects (Wire, SPI, GPIO)
   * @return true if initialization successful, false otherwise
   */
  bool begin(Stream &stream, bool asDefault = true) {
    Logger.info("Using Firmata hardware interfaces");
    is_default_objects_active = asDefault;

    transport.begin(stream);
    bool gpio_ok = gpio.begin(transport);
    bool i2c_ok = i2c.begin(transport);
    bool spi_ok = spi.begin(transport);

    // define the global hardware interfaces
    if (asDefault) {
      Logger.warning("GPIO, I2C, SPI set up for Firmata");
      GPIO.setGPIO(&gpio);
      SPI.setSPI(&spi);
      Wire.setI2C(&i2c);
    }

    return gpio_ok && i2c_ok && spi_ok;
  }

  /**
   * @brief Resets hardware pointers to nullptr and stops the transport.
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
    transport.end();
  }

  HardwareGPIO_FIRMATA *getGPIO() { return &gpio; }
  HardwareI2C_FIRMATA *getI2C() { return &i2c; }
  HardwareSPI_FIRMATA *getSPI() { return &spi; }

 protected:
  FirmataTransport transport;
  HardwareGPIO_FIRMATA gpio;
  HardwareI2C_FIRMATA i2c;
  HardwareSPI_FIRMATA spi;
  bool is_default_objects_active = false;
};

/**
 * @brief Global instance for Firmata hardware setup.
 *
 * Use this object to access and initialize GPIO, I2C, and SPI interfaces on
 * a remote Firmata device, e.g.:
 *   FIRMATA.begin(mySerialStream);
 *   pinMode(13, OUTPUT);      // routed to GPIO over Firmata
 *   Wire.beginTransmission(0x76); ...
 */
static HardwareSetupFIRMATA FIRMATA;

}  // namespace arduino

#endif
