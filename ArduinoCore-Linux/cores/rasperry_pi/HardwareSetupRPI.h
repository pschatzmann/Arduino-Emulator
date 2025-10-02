
#pragma once
#if defined(USE_RPI) && !defined(SKIP_HARDWARE_SETUP)
#include "Hardware.h" // for Hardware; 
#include "HardwareGPIO_RPI.h"
#include "HardwareI2C_RPI.h"
#include "HardwareSPI_RPI.h"
#include "FileStream.h"

namespace arduino {

static HardwareI2C_RPI Wire;
static HardwareSPI_RPI SPI;;

/**
 * @class HardwareSetupRPI
 * @brief Sets up hardware interfaces for Raspberry Pi (GPIO, I2C, SPI).
 */
class HardwareSetupRPI {
 public:
  /**
   * @brief Constructor. Initializes hardware interfaces.
   */
  HardwareSetupRPI() = default;

  /**
   * @brief Destructor. Cleans up hardware interfaces.
   */
  ~HardwareSetupRPI() { end(); }

  /**
   * @brief Initializes hardware pointers to Raspberry Pi interfaces.
   */
  bool begin() {
    Logger.info("Using Raspberry Pi hardware interfaces");
    gpio.begin();
    i2c.begin();
    spi.begin();
    // setup hardware pointers
    Hardware.gpio = &gpio;
    Hardware.i2c = &i2c;
    Hardware.spi = &spi;
    // setup global instances
    Wire = HardwareSetupRPI::get_i2c();
    SPI = HardwareSetupRPI::get_spi();
    return gpio && i2c && spi;
  }

  /**
   * @brief Resets hardware pointers to nullptr.
   */
  void end() {
    Hardware.gpio = nullptr;
    Hardware.i2c = nullptr;
    Hardware.spi = nullptr;
  }

  HardwareGPIO_RPI& get_gpio() { return gpio; }
  HardwareI2C_RPI& get_i2c() { return i2c; }
  HardwareSPI_RPI& get_spi() { return spi; }

 protected:
  /** GPIO interface for Raspberry Pi */
  HardwareGPIO_RPI gpio;
  /** I2C interface for Raspberry Pi */
  HardwareI2C_RPI i2c;
  /** SPI interface for Raspberry Pi */
  HardwareSPI_RPI spi;
};

/**
 * @brief Global instance for Raspberry Pi hardware setup.
 *
 * Use this object to access and initialize GPIO, I2C, and SPI interfaces on
 * Raspberry Pi.
 */
static HardwareSetupRPI RPI;

/**
 * @brief Second hardware serial port for Raspberry Pi.
 *
 * Serial2 provides access to the Raspberry Pi's primary UART device (usually /dev/serial0).
 * This can be used for serial communication with external devices, similar to Serial1/Serial2 on Arduino boards.
 * Example usage:
 *   Serial2.begin(9600);
 *   Serial2.println("Hello from Serial2");
 */
static FileStream Serial2("/dev/serial0");


}  // namespace arduino

#endif