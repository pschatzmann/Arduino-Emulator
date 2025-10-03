#pragma once
#include "api/HardwareI2C.h"
#include "api/HardwareSPI.h"
#include "HardwareGPIO.h"

namespace arduino {

/**
 * @brief Abstract interface for providing I2C hardware implementations
 *
 * I2CSource defines a factory interface for supplying HardwareI2C implementations.
 * This abstraction allows wrapper classes to obtain I2C hardware instances from
 * various sources without coupling to specific hardware providers.
 *
 * @see HardwareI2C
 * @see I2CWrapper
 */
class I2CSource {
  public:
  virtual HardwareI2C* getI2C() = 0;
};

/**
 * @brief Abstract interface for providing SPI hardware implementations
 *
 * SPISource defines a factory interface for supplying HardwareSPI implementations.
 * This abstraction allows wrapper classes to obtain SPI hardware instances from
 * various sources without coupling to specific hardware providers.
 *
 * @see HardwareSPI
 * @see SPIWrapper
 */
class SPISource {
  public:
  virtual HardwareSPI* getSPI() = 0;
};

/**
 * @brief Abstract interface for providing GPIO hardware implementations
 *
 * GPIOSource defines a factory interface for supplying HardwareGPIO implementations.
 * This abstraction allows wrapper classes to obtain GPIO hardware instances from
 * various sources without coupling to specific hardware providers.
 *
 * @see HardwareGPIO
 * @see GPIOWrapper
 */
class GPIOSource {
  public:
  virtual HardwareGPIO* getGPIO() = 0;
};


}  // namespace arduino
