
#ifdef USE_RPI
#include "HardwareGPIO_RPI.h"
#include "HardwareI2C_RPI.h"
#include "HardwareSPI_RPI.h"


/**
 * @class HardwareSetupRPI
 * @brief Sets up hardware interfaces for Raspberry Pi (GPIO, I2C, SPI).
 */
class HardwareSetupRPI {
 public:
  /**
   * @brief Constructor. Initializes hardware interfaces.
   */
  HardwareSetupRPI() { begin(); }

  /**
   * @brief Destructor. Cleans up hardware interfaces.
   */
  ~HardwareSetupRPI() { end(); }

  /**
   * @brief Initializes hardware pointers to Raspberry Pi interfaces.
   */
  void begin() {
    hardware.gpio = &gpio;
    hardware.i2c = &i2c;
    hardware.spi = &spi;
  }

  /**
   * @brief Resets hardware pointers to nullptr.
   */
  void end() {
    hardware.gpio = nullptr;
    hardware.i2c = nullptr;
    hardware.spi = nullptr;
  }

 protected:
  /** GPIO interface for Raspberry Pi */
  HardwareGPIO_RPI gpio;
  /** I2C interface for Raspberry Pi */
  HardwareI2C_RPI i2c;
  /** SPI interface for Raspberry Pi */
  HardwareSPI_RPI spi;
  /** Hardware interface struct */
  Hardware hardware;
};

/**
 * @brief Global instance for Raspberry Pi hardware setup.
 *
 * Use this object to access and initialize GPIO, I2C, and SPI interfaces on Raspberry Pi.
 */
static HardwareSetupRPI RPI;

#endif