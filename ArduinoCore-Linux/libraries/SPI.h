#pragma once
#include "HardwareSPI.h"

// this usually needs PIN_CS defined
#define PIN_CS -1

/**
 * @brief Mock SPIClass is a class that implements the HardwareSPI interface.
 * e.g. Using Files do not need SPI, but usually do some SPI setup.
 */

class SPIClass : public HardwareSPI {
 public:
  SPIClass() = default;
  virtual ~SPIClass() = default;

  uint8_t transfer(uint8_t data) override { return 0; }
  uint16_t transfer16(uint16_t data) override { return 0; }
  void transfer(void *buf, size_t count) override {}

  void usingInterrupt(int interruptNumber) override {}
  void notUsingInterrupt(int interruptNumber) override {}
  void beginTransaction(SPISettings settings) override {}
  void endTransaction(void) override {}

  void attachInterrupt() override {}
  void detachInterrupt() override {}

  void begin() override {}
  void end() override {}
};

static SPIClass SPI;
static SPIClass SPI1;
