/*
	SPIWrapper.cpp
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
#include "SPIWrapper.h"
/**
 * We suppport different implementations for the SPI
 *
 **/

namespace arduino {

SPIWrapper SPI;

uint8_t SPIWrapper::transfer(uint8_t data) {
  HardwareSPI* spi = getSPI();
  if (spi != nullptr) {
    return spi->transfer(data);
  } else {
    return 0;
  }
}

uint16_t SPIWrapper::transfer16(uint16_t data) {
  HardwareSPI* spi = getSPI();
  if (spi != nullptr) {
    return spi->transfer16(data);
  } else {
    return 0;
  }
}

void SPIWrapper::transfer(void* data, size_t count) {
  HardwareSPI* spi = getSPI();
  if (spi != nullptr) {
    spi->transfer(data, count);
  }
}

void SPIWrapper::usingInterrupt(int interruptNumber) {
  HardwareSPI* spi = getSPI();
  if (spi != nullptr) {
    spi->usingInterrupt(interruptNumber);
  }
}

void SPIWrapper::notUsingInterrupt(int interruptNumber) {
  HardwareSPI* spi = getSPI();
  if (spi != nullptr) {
    spi->notUsingInterrupt(interruptNumber);
  }
}

void SPIWrapper::beginTransaction(SPISettings settings) {
  HardwareSPI* spi = getSPI();
  if (spi != nullptr) {
    spi->beginTransaction(settings);
  }
}

void SPIWrapper::endTransaction(void) {
  HardwareSPI* spi = getSPI();
  if (spi != nullptr) {
    spi->endTransaction();
  }
}

void SPIWrapper::attachInterrupt() {
  HardwareSPI* spi = getSPI();
  if (spi != nullptr) {
    spi->attachInterrupt();
  }
}

void SPIWrapper::detachInterrupt() {
  HardwareSPI* spi = getSPI();
  if (spi != nullptr) {
    spi->detachInterrupt();
  }
}

void SPIWrapper::begin() {
  HardwareSPI* spi = getSPI();
  if (spi != nullptr) {
    spi->begin();
  }
}

void SPIWrapper::end() {
  HardwareSPI* spi = getSPI();
  if (spi != nullptr) {
    spi->end();
  }
}

}  // namespace arduino