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

#include "I2CWrapper.h"

namespace arduino {

I2CWrapper Wire;

void I2CWrapper::begin() {
  HardwareI2C* i2c = getI2C();
  if (i2c != nullptr) {
    i2c->begin();
  }
}

void I2CWrapper::begin(uint8_t address) {
  HardwareI2C* i2c = getI2C();
  if (i2c != nullptr) {
    i2c->begin(address);
  }
}

void I2CWrapper::end() {
  HardwareI2C* i2c = getI2C();
  if (i2c != nullptr) {
    i2c->end();
  }
}

void I2CWrapper::setClock(uint32_t freq) {
  HardwareI2C* i2c = getI2C();
  if (i2c != nullptr) {
    i2c->setClock(freq);
  }
}

void I2CWrapper::beginTransmission(uint8_t address) {
  HardwareI2C* i2c = getI2C();
  if (i2c != nullptr) {
    i2c->beginTransmission(address);
  }
}

uint8_t I2CWrapper::endTransmission(bool stopBit) {
  HardwareI2C* i2c = getI2C();
  if (i2c != nullptr) {
    return i2c->endTransmission(stopBit);
  } else {
    return 0;
  }
}

uint8_t I2CWrapper::endTransmission(void) {
  HardwareI2C* i2c = getI2C();
  if (i2c != nullptr) {
    return i2c->endTransmission();
  } else {
    return 0;
  }
}

size_t I2CWrapper::requestFrom(uint8_t address, size_t len, bool stopBit) {
  HardwareI2C* i2c = getI2C();
  if (i2c != nullptr) {
    return i2c->requestFrom(address, len, stopBit);
  } else {
    return 0;
  }
}

size_t I2CWrapper::requestFrom(uint8_t address, size_t len) {
  HardwareI2C* i2c = getI2C();
  if (i2c != nullptr) {
    return i2c->requestFrom(address, len);
  } else {
    return 0;
  }
}

void I2CWrapper::onReceive(void (*)(int)) {}

void I2CWrapper::onRequest(void (*)(void)) {}

size_t I2CWrapper::write(uint8_t data) {
  HardwareI2C* i2c = getI2C();
  if (i2c != nullptr) {
    return i2c->write(data);
  } else {
    return 0;
  }
}

int I2CWrapper::available() {
  HardwareI2C* i2c = getI2C();
  if (i2c != nullptr) {
    return i2c->available();
  } else {
    return 0;
  }
}

int I2CWrapper::read() {
  HardwareI2C* i2c = getI2C();
  if (i2c != nullptr) {
    return i2c->read();
  } else {
    return -1;
  }
}

int I2CWrapper::peek() {
  HardwareI2C* i2c = getI2C();
  if (i2c != nullptr) {
    return i2c->peek();
  } else {
    return -1;
  }
}

}  // namespace arduino