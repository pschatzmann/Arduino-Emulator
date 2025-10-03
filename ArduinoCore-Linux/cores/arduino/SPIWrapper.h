#pragma once
#include "Sources.h"
#include "api/HardwareSPI.h"

namespace arduino {

/**
 * @brief SPI wrapper class that provides flexible hardware abstraction
 *
 * SPIWrapper is a concrete implementation of the HardwareSPI interface that
 * supports multiple delegation patterns for SPI communication. It can delegate
 * operations to:
 * - An injected HardwareSPI implementation
 * - An SPISource provider that supplies the SPI implementation
 * - A default fallback implementation
 *
 * This class implements the complete SPI interface including:
 * - Bus initialization and termination (begin/end)
 * - Data transfer operations (8-bit, 16-bit, and buffer transfers)
 * - Transaction management with settings control
 * - Interrupt handling and configuration
 * - Clock, data order, and mode configuration via SPISettings
 *
 * The wrapper automatically handles null safety and provides appropriate
 * default return values when no underlying SPI implementation is available. It
 * supports both polling and interrupt-driven SPI operations.
 *
 * A global `SPI` instance is automatically provided for Arduino compatibility.
 *
 * @see HardwareSPI
 * @see SPISource
 * @see SPISettings
 */
class SPIWrapper : public HardwareSPI {
 public:
  SPIWrapper() = default;
  SPIWrapper(SPISource& source) { setSource(&source); }
  SPIWrapper(HardwareSPI& spi) { setSPI(&spi); }
  ~SPIWrapper() = default;
  void begin();
  void end();
  uint8_t transfer(uint8_t data);
  uint16_t transfer16(uint16_t data);
  void transfer(void* data, size_t count);
  void usingInterrupt(int interruptNumber);
  void notUsingInterrupt(int interruptNumber);
  void beginTransaction(SPISettings settings);
  void endTransaction(void);
  void attachInterrupt();
  void detachInterrupt();

  /// defines the spi implementation: use nullptr to reset.
  void setSPI(HardwareSPI* spi) {
    p_spi = spi;
    p_source = nullptr;
  }
  /// alternatively defines a class that provides the SPI implementation
  void setSource(SPISource* source) {
    p_source = source;
    p_spi = nullptr;
  }

 protected:
  HardwareSPI* p_spi = nullptr;
  SPISource* p_source = nullptr;

  HardwareSPI* getSPI() {
    HardwareSPI* result = p_spi;
    if (result == nullptr && p_source != nullptr) {
      result = p_source->getSPI();
    }
    return result;
  }
};

/// Global SPI instance used by Arduino API and direct access
extern SPIWrapper SPI;

}  // namespace arduino
