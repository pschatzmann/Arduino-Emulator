
#ifdef USE_RPI
#include "Hardware.h" // for Hardware; 
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
    Logger.info("Using Raspberry Pi hardware interfaces");
    Hardware.gpio = &gpio;
    Hardware.i2c = &i2c;
    Hardware.spi = &spi;
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

#endif