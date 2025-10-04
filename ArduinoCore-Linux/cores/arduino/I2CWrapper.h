/*
  I2CWrapper.h 
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

#pragma once
#include "Sources.h"
#include "api/HardwareI2C.h"

namespace arduino {

/**
 * @brief I2C wrapper class that provides flexible hardware abstraction
 *
 * I2CWrapper is a concrete implementation of the HardwareI2C interface that
 * supports multiple delegation patterns for I2C communication. It can delegate
 * operations to:
 * - An injected HardwareI2C implementation
 * - An I2CSource provider that supplies the I2C implementation
 * - A default fallback implementation
 *
 * This class implements the complete I2C interface including:
 * - Master/slave mode initialization and control
 * - Data transmission and reception methods
 * - Stream-like read/write operations with buffering
 * - Callback registration for slave mode operations
 * - Clock frequency configuration
 *
 * The wrapper automatically handles null safety and provides appropriate
 * default return values when no underlying I2C implementation is available.
 *
 * A global `Wire` instance is automatically provided for Arduino compatibility.
 *
 * @see HardwareI2C
 * @see I2CSource
 */
class I2CWrapper : public HardwareI2C {
 public:
  I2CWrapper() = default;
  I2CWrapper(I2CSource& source) { setSource(&source); }
  I2CWrapper(HardwareI2C& i2c) { setI2C(&i2c); }
  ~I2CWrapper() = default;
  void begin();
  void begin(uint8_t address);
  void end();
  void setClock(uint32_t freq);
  void beginTransmission(uint8_t address);
  uint8_t endTransmission(bool stopBit);
  uint8_t endTransmission(void);
  size_t requestFrom(uint8_t address, size_t len, bool stopBit);
  size_t requestFrom(uint8_t address, size_t len);
  void onReceive(void (*)(int));
  void onRequest(void (*)(void));
  size_t write(uint8_t);
  int available();
  int read();
  int peek();

  /// defines the i2c implementation: use nullptr to reset.
  void setI2C(HardwareI2C* i2c) {
    p_i2c = i2c;
    p_source = nullptr;
  }
  /// alternatively defines a class that provides the I2C implementation
  void setSource(I2CSource* source) {
    p_source = source;
    p_i2c = nullptr;
  }

 protected:
  HardwareI2C* p_i2c = nullptr;
  I2CSource* p_source = nullptr;

  HardwareI2C* getI2C() {
    HardwareI2C* result = p_i2c;
    if (result == nullptr && p_source != nullptr) {
      result = p_source->getI2C();
    }
    return result;
  }
};

/// Global Wire instance used by Arduino API functions and direct access
extern I2CWrapper Wire;

/// Type alias for Arduino compatibility - TwoWire refers to I2CWrapper
using TwoWire = I2CWrapper;

}  // namespace arduino
