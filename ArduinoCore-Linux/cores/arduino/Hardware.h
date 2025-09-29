#pragma once

namespace arduino {

// forware declarations for references
class HardwareI2CEx;
class HardwareSPI;
class HardwareGPIO;
// class I2SClass;

/**
 * Class which is used to represent the actual implementation which is used to
 * drive the specific interface
 **/
struct HardwareImpl {
 public:
  HardwareImpl() {}
  HardwareGPIO* gpio = nullptr;
  HardwareI2CEx* i2c = nullptr;
  HardwareSPI* spi = nullptr;
};

extern HardwareImpl Hardware;

}  // namespace arduino
