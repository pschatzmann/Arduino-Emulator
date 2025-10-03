#include "SPIWrapper.h"
/**
 * We suppport different implementations for the SPI
 *
 **/

namespace arduino {

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