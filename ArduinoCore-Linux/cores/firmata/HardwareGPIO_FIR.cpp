/*
  HardwareGPIO_FIR.cpp
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
#include "HardwareGPIO_FIR.h"

namespace arduino {

static constexpr uint8_t DIGITAL_MESSAGE = 0x90;
static constexpr uint8_t ANALOG_MESSAGE = 0xE0;
static constexpr uint8_t REPORT_ANALOG = 0xC0;
static constexpr uint8_t REPORT_DIGITAL = 0xD0;
static constexpr uint8_t SET_PIN_MODE = 0xF4;

static constexpr uint8_t FIRMATA_MODE_INPUT = 0x00;
static constexpr uint8_t FIRMATA_MODE_OUTPUT = 0x01;
static constexpr uint8_t FIRMATA_MODE_PULLUP = 0x0B;

static constexpr pin_size_t MAX_FIRMATA_PIN = 15;

HardwareGPIO_FIRMATA::~HardwareGPIO_FIRMATA() { end(); }

uint8_t HardwareGPIO_FIRMATA::toFirmataPinMode(PinMode mode) {
  switch (mode) {
    case INPUT:
    case INPUT_PULLDOWN:  // classic Firmata has no pulldown mode; best effort
      return FIRMATA_MODE_INPUT;
    case INPUT_PULLUP:
      return FIRMATA_MODE_PULLUP;
    case OUTPUT:
    case OUTPUT_OPENDRAIN:  // classic Firmata has no open-drain mode
      return FIRMATA_MODE_OUTPUT;
    default:
      return FIRMATA_MODE_INPUT;
  }
}

bool HardwareGPIO_FIRMATA::begin(Stream &stream) {
  end();
  owned_transport_ = std::make_unique<FirmataTransport>();
  owned_transport_->begin(stream);
  return begin(*owned_transport_);
}

bool HardwareGPIO_FIRMATA::begin(FirmataTransport &transport) {
  if (&transport != owned_transport_.get()) {
    end();
  }
  transport_ = &transport;
  is_open = true;

  transport_->onDigitalMessage(
      [this](const FirmataDigitalMessage &msg) { handleDigitalMessage(msg); });
  transport_->onAnalogMessage(
      [this](const FirmataAnalogMessage &msg) { handleAnalogMessage(msg); });
  return true;
}

void HardwareGPIO_FIRMATA::end() {
  if (owned_transport_) {
    owned_transport_->end();
    owned_transport_.reset();
  }

  std::lock_guard<std::mutex> lock(state_mutex_);
  is_open = false;
  transport_ = nullptr;
  pin_modes_.clear();
  pin_states_.clear();
  analog_values_.clear();
  digital_reporting_enabled_.clear();
  analog_reporting_enabled_.clear();
}

void HardwareGPIO_FIRMATA::handleDigitalMessage(
    const FirmataDigitalMessage &msg) {
  std::lock_guard<std::mutex> lock(state_mutex_);
  for (int bit = 0; bit < 8; ++bit) {
    const pin_size_t pin = msg.port * 8 + bit;
    pin_states_[pin] = (msg.value & (1 << bit)) ? HIGH : LOW;
  }
}

void HardwareGPIO_FIRMATA::handleAnalogMessage(
    const FirmataAnalogMessage &msg) {
  std::lock_guard<std::mutex> lock(state_mutex_);
  analog_values_[msg.pin] = msg.value;
}

void HardwareGPIO_FIRMATA::enableDigitalReporting(pin_size_t pinNumber) {
  if (pinNumber > MAX_FIRMATA_PIN) return;
  const uint8_t port = pinNumber / 8;

  std::lock_guard<std::mutex> lock(state_mutex_);
  if (digital_reporting_enabled_[port]) return;
  digital_reporting_enabled_[port] = true;

  if (transport_) {
    const uint8_t msg[2] = {static_cast<uint8_t>(REPORT_DIGITAL | port), 1};
    transport_->writeBytes(msg, sizeof(msg));
  }
}

void HardwareGPIO_FIRMATA::enableAnalogReporting(pin_size_t pinNumber) {
  if (pinNumber > MAX_FIRMATA_PIN) return;

  std::lock_guard<std::mutex> lock(state_mutex_);
  if (analog_reporting_enabled_[pinNumber]) return;
  analog_reporting_enabled_[pinNumber] = true;

  if (transport_) {
    const uint8_t msg[2] = {
        static_cast<uint8_t>(REPORT_ANALOG | pinNumber), 1};
    transport_->writeBytes(msg, sizeof(msg));
  }
}

void HardwareGPIO_FIRMATA::pinMode(pin_size_t pinNumber, PinMode mode) {
  if (pinNumber > MAX_FIRMATA_PIN) return;

  {
    std::lock_guard<std::mutex> lock(state_mutex_);
    pin_modes_[pinNumber] = mode;
    if (transport_) {
      const uint8_t msg[3] = {SET_PIN_MODE, static_cast<uint8_t>(pinNumber),
                               toFirmataPinMode(mode)};
      transport_->writeBytes(msg, sizeof(msg));
    }
  }

  if (mode == INPUT || mode == INPUT_PULLUP || mode == INPUT_PULLDOWN) {
    enableDigitalReporting(pinNumber);
  }
}

void HardwareGPIO_FIRMATA::digitalWrite(pin_size_t pinNumber,
                                         PinStatus status) {
  if (pinNumber > MAX_FIRMATA_PIN) return;

  std::lock_guard<std::mutex> lock(state_mutex_);
  pin_states_[pinNumber] = status;
  if (!transport_) return;

  const uint8_t port = pinNumber / 8;
  uint8_t portValue = 0;
  for (int bit = 0; bit < 8; ++bit) {
    const pin_size_t pin = port * 8 + bit;
    const auto it = pin_states_.find(pin);
    if (it != pin_states_.end() && it->second == HIGH) {
      portValue |= (1 << bit);
    }
  }
  const uint8_t msg[3] = {static_cast<uint8_t>(DIGITAL_MESSAGE | port),
                           static_cast<uint8_t>(portValue & 0x7F),
                           static_cast<uint8_t>((portValue >> 7) & 0x7F)};
  transport_->writeBytes(msg, sizeof(msg));
}

PinStatus HardwareGPIO_FIRMATA::digitalRead(pin_size_t pinNumber) {
  if (pinNumber > MAX_FIRMATA_PIN) return LOW;
  enableDigitalReporting(pinNumber);

  std::lock_guard<std::mutex> lock(state_mutex_);
  const auto it = pin_states_.find(pinNumber);
  return it != pin_states_.end() ? it->second : LOW;
}

int HardwareGPIO_FIRMATA::analogRead(pin_size_t pinNumber) {
  if (pinNumber > MAX_FIRMATA_PIN) return 0;
  enableAnalogReporting(pinNumber);

  std::lock_guard<std::mutex> lock(state_mutex_);
  const auto it = analog_values_.find(pinNumber);
  return it != analog_values_.end() ? it->second : 0;
}

void HardwareGPIO_FIRMATA::analogReference(uint8_t mode) {
  // Not supported by Firmata, do nothing.
}

void HardwareGPIO_FIRMATA::analogWrite(pin_size_t pinNumber, int value) {
  if (pinNumber > MAX_FIRMATA_PIN) return;

  std::lock_guard<std::mutex> lock(state_mutex_);
  if (!transport_) return;
  const uint8_t clamped = static_cast<uint8_t>(
      value < 0 ? 0 : (value > 255 ? 255 : value));
  const uint8_t msg[3] = {static_cast<uint8_t>(ANALOG_MESSAGE | pinNumber),
                           static_cast<uint8_t>(clamped & 0x7F),
                           static_cast<uint8_t>((clamped >> 7) & 0x7F)};
  transport_->writeBytes(msg, sizeof(msg));
}

void HardwareGPIO_FIRMATA::analogWriteFrequency(pin_size_t pinNumber,
                                                 uint32_t frequency) {
  // Not supported by classic Firmata, do nothing.
}

void HardwareGPIO_FIRMATA::tone(uint8_t _pin, unsigned int frequency,
                                 unsigned long duration) {
  // Not supported by classic Firmata, do nothing.
}

void HardwareGPIO_FIRMATA::noTone(uint8_t _pin) {
  // Not supported by classic Firmata, do nothing.
}

unsigned long HardwareGPIO_FIRMATA::pulseIn(uint8_t pin, uint8_t state,
                                             unsigned long timeout) {
  // Not supported by classic Firmata.
  return 0;
}

unsigned long HardwareGPIO_FIRMATA::pulseInLong(uint8_t pin, uint8_t state,
                                                 unsigned long timeout) {
  // Not supported by classic Firmata.
  return 0;
}

void HardwareGPIO_FIRMATA::analogWriteResolution(uint8_t bits) {
  // Classic Firmata always uses 8-bit resolution, do nothing.
}

}  // namespace arduino

#endif  // USE_FIRMATA
