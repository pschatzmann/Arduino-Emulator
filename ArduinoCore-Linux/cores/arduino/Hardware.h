#pragma once

namespace arduino {

// forware declarations for references
class HardwareI2C;
class HardwareSPI;
class HardwareGPIO;

/**
 * Class which is used to represent the actual implementation which is used to
 * drive the specific interface
 **/
struct HardwareImpl {
  HardwareImpl() = default;
  HardwareGPIO* gpio = nullptr;
  HardwareI2C* i2c = nullptr;
  HardwareSPI* spi = nullptr;
};

static HardwareImpl Hardware;

}  // namespace arduino
