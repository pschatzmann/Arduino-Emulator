
#pragma once
#if defined(USE_RPI) && !defined(SKIP_HARDWARE_SETUP)
#include "FileStream.h"
#include "HardwareGPIO_RPI.h"
#include "HardwareI2C_RPI.h"
#include "HardwareSPI_RPI.h"

namespace arduino {

/**
 * @class HardwareSetupRPI
 * @brief Sets up hardware interfaces for Raspberry Pi (GPIO, I2C, SPI).
 */
class HardwareSetupRPI : public I2CSource, public SPISource, public GPIOSource {
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
  bool begin(bool asDefault = true) {
    Logger.info("Using Raspberry Pi hardware interfaces");
    is_default_objects_active = asDefault;
    gpio.begin();
    i2c.begin();
    spi.begin();

    // define the global hardware interfaces
    if (asDefault) {
      GPIO.setGPIO(&gpio);
      SPI.setSPI(&spi);
      Wire.setI2C(&i2c);
    }

    return gpio && i2c && spi;
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
  }

  HardwareGPIO_RPI* getGPIO() { return &gpio; }
  HardwareI2C_RPI* getI2C() { return &i2c; }
  HardwareSPI_RPI* getSPI() { return &spi; }

 protected:
  HardwareGPIO_RPI gpio;
  HardwareI2C_RPI i2c;
  HardwareSPI_RPI spi;
  bool is_default_objects_active = false;
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
 * Serial2 provides access to the Raspberry Pi's primary UART device (usually
 * /dev/serial0). This can be used for serial communication with external
 * devices, similar to Serial1/Serial2 on Arduino boards. Example usage:
 *   Serial2.begin(9600);
 *   Serial2.println("Hello from Serial2");
 */
static FileStream Serial2("/dev/serial0");

}  // namespace arduino

#endif