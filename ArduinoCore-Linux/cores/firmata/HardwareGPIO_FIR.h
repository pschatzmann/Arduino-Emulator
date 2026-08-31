#pragma once
/*
  HardwareGPIO_FIR.h
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
#ifdef USE_FIRMATA
#include "HardwareGPIO.h"
#include "FirmataTransport.h"
#include <map>
#include <memory>
#include <mutex>

namespace arduino {

/**
 * @class HardwareGPIO_FIRMATA
 * @brief GPIO hardware abstraction that speaks the Firmata protocol over an
 * arbitrary Stream (e.g. a serial connection to a device running
 * StandardFirmata).
 *
 * Digital/analog pin reports are pushed asynchronously by the remote device
 * once reporting has been enabled for a port/pin. A FirmataTransport drains
 * the Stream and dispatches parsed reports here so digitalRead()/
 * analogRead() can return immediately.
 *
 * Use begin(Stream&) for a standalone GPIO-only connection, or
 * begin(FirmataTransport&) to share one connection with
 * HardwareI2C_FIRMATA/HardwareSPI_FIRMATA (see HardwareSetupFIR.h).
 *
 * @note This class is only available when USE_FIRMATA is defined.
 * @note Only the classic (non-SysEx) subset of the Firmata protocol is
 *       supported: pins 0-15, digital/analog messages, pin mode and
 *       reporting requests.
 */
class HardwareGPIO_FIRMATA : public HardwareGPIO {
 public:
  HardwareGPIO_FIRMATA() = default;
  ~HardwareGPIO_FIRMATA();

  /**
   * @brief Start talking Firmata over the given stream. Owns a private
   * FirmataTransport for this connection alone.
   * @param stream Already-opened stream connected to the Firmata device.
   * @return true if initialization successful, false otherwise.
   */
  bool begin(Stream &stream);

  /**
   * @brief Attach to an already-started, externally-owned transport, e.g.
   * to share one Firmata connection with I2C/SPI backends.
   * @param transport A transport whose begin() has already been called.
   * @return true if initialization successful, false otherwise.
   */
  bool begin(FirmataTransport &transport);

  /**
   * @brief Stop the reader thread (if owned) and detach.
   */
  void end();

  void pinMode(pin_size_t pinNumber, PinMode pinMode) override;
  void digitalWrite(pin_size_t pinNumber, PinStatus status) override;
  PinStatus digitalRead(pin_size_t pinNumber) override;
  int analogRead(pin_size_t pinNumber) override;
  void analogReference(uint8_t mode) override;
  void analogWrite(pin_size_t pinNumber, int value) override;
  void analogWriteFrequency(pin_size_t pinNumber, uint32_t frequency);
  void tone(uint8_t _pin, unsigned int frequency,
            unsigned long duration = 0) override;
  void noTone(uint8_t _pin) override;
  unsigned long pulseIn(uint8_t pin, uint8_t state,
                        unsigned long timeout = 1000000L) override;
  unsigned long pulseInLong(uint8_t pin, uint8_t state,
                            unsigned long timeout = 1000000L) override;
  void analogWriteResolution(uint8_t bits) override;

  operator bool() { return is_open; }

 protected:
  FirmataTransport *transport_ = nullptr;              // non-owning
  std::unique_ptr<FirmataTransport> owned_transport_;   // set by begin(Stream&)
  bool is_open = false;

  std::mutex state_mutex_;
  std::map<pin_size_t, PinMode> pin_modes_;
  std::map<pin_size_t, PinStatus> pin_states_;
  std::map<pin_size_t, int> analog_values_;
  std::map<uint8_t, bool> digital_reporting_enabled_;
  std::map<pin_size_t, bool> analog_reporting_enabled_;

  void handleDigitalMessage(const FirmataDigitalMessage &msg);
  void handleAnalogMessage(const FirmataAnalogMessage &msg);

  /// Enable REPORT_DIGITAL for the port containing pinNumber, once.
  void enableDigitalReporting(pin_size_t pinNumber);
  /// Enable REPORT_ANALOG for pinNumber, once.
  void enableAnalogReporting(pin_size_t pinNumber);

  /// Translate the Arduino PinMode enum to the Firmata SET_PIN_MODE value.
  static uint8_t toFirmataPinMode(PinMode mode);
};

}  // namespace arduino

#endif  // USE_FIRMATA
