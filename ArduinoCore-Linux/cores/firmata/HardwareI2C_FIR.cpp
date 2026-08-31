/*
  HardwareI2C_FIR.cpp
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
#include "HardwareI2C_FIR.h"
#include <chrono>

namespace arduino {

static constexpr uint8_t I2C_REQUEST = 0x76;
static constexpr uint8_t I2C_REPLY = 0x77;
static constexpr uint8_t I2C_CONFIG = 0x78;

static constexpr uint8_t I2C_WRITE = 0x00;
static constexpr uint8_t I2C_READ = 0x08;
static constexpr uint8_t I2C_END_TX_MASK = 0x40;  // set = restart (no stop)

HardwareI2C_FIRMATA::~HardwareI2C_FIRMATA() { end(); }

bool HardwareI2C_FIRMATA::begin(Stream &stream) {
  end();
  owned_transport_ = std::make_unique<FirmataTransport>();
  owned_transport_->begin(stream);
  return begin(*owned_transport_);
}

bool HardwareI2C_FIRMATA::begin(FirmataTransport &transport) {
  if (&transport != owned_transport_.get()) {
    end();
  }
  transport_ = &transport;
  is_open = true;

  transport_->addSysexHandler(
      I2C_REPLY, [this](const FirmataSysexMessage &msg) { handleSysex(msg); });

  // I2C_CONFIG with a 0 delay tells the device to use its own default.
  const uint8_t config[5] = {0xF0, I2C_CONFIG, 0, 0, 0xF7};
  transport_->writeBytes(config, sizeof(config));
  return true;
}

void HardwareI2C_FIRMATA::begin() {
  // Master mode is the only mode Firmata supports; nothing further to do
  // once begin(Stream&)/begin(FirmataTransport&) has been called.
}

void HardwareI2C_FIRMATA::begin(uint8_t address) {
  // Firmata (StandardFirmata/ConfigurableFirmata) has no I2C slave mode.
}

void HardwareI2C_FIRMATA::end() {
  if (transport_) {
    transport_->removeSysexHandler(I2C_REPLY);
  }
  if (owned_transport_) {
    owned_transport_->end();
    owned_transport_.reset();
  }
  transport_ = nullptr;
  is_open = false;
  tx_buffer_.clear();
  std::lock_guard<std::mutex> lock(rx_mutex_);
  rx_buffer_.clear();
  request_pending_ = false;
}

void HardwareI2C_FIRMATA::setClock(uint32_t freq) {
  // Firmata's I2C_CONFIG message configures a read delay, not a bus clock
  // frequency; the device chooses its own I2C bus speed. No-op.
}

void HardwareI2C_FIRMATA::beginTransmission(uint8_t address) {
  current_address_ = address;
  tx_buffer_.clear();
}

size_t HardwareI2C_FIRMATA::write(uint8_t data) {
  tx_buffer_.push_back(data);
  return 1;
}

size_t HardwareI2C_FIRMATA::write(const uint8_t *data, size_t len) {
  tx_buffer_.insert(tx_buffer_.end(), data, data + len);
  return len;
}

uint8_t HardwareI2C_FIRMATA::endTransmission(bool stopBit) {
  if (!transport_) return 1;

  std::vector<uint8_t> msg;
  msg.reserve(5 + tx_buffer_.size() * 2);
  msg.push_back(0xF0);  // START_SYSEX
  msg.push_back(I2C_REQUEST);
  msg.push_back(current_address_ & 0x7F);
  msg.push_back(I2C_WRITE | (stopBit ? 0 : I2C_END_TX_MASK));
  for (uint8_t b : tx_buffer_) {
    msg.push_back(b & 0x7F);
    msg.push_back((b >> 7) & 0x7F);
  }
  msg.push_back(0xF7);  // END_SYSEX
  transport_->writeBytes(msg.data(), msg.size());

  tx_buffer_.clear();
  return 0;
}

uint8_t HardwareI2C_FIRMATA::endTransmission(void) {
  return endTransmission(true);
}

size_t HardwareI2C_FIRMATA::requestFrom(uint8_t address, size_t len,
                                         bool stopBit) {
  if (!transport_ || len == 0) return 0;

  {
    std::lock_guard<std::mutex> lock(rx_mutex_);
    rx_buffer_.clear();
    request_pending_ = true;
  }

  const uint8_t msg[8] = {
      0xF0,  // START_SYSEX
      I2C_REQUEST,
      static_cast<uint8_t>(address & 0x7F),
      static_cast<uint8_t>(I2C_READ | (stopBit ? 0 : I2C_END_TX_MASK)),
      static_cast<uint8_t>(len & 0x7F),
      static_cast<uint8_t>((len >> 7) & 0x7F),
      0xF7,  // END_SYSEX
  };
  transport_->writeBytes(msg, 7);

  std::unique_lock<std::mutex> lock(rx_mutex_);
  rx_cv_.wait_for(lock, std::chrono::milliseconds(timeout_ms_),
                   [this] { return !request_pending_ || !rx_buffer_.empty(); });
  request_pending_ = false;
  return rx_buffer_.size();
}

size_t HardwareI2C_FIRMATA::requestFrom(uint8_t address, size_t len) {
  return requestFrom(address, len, true);
}

int HardwareI2C_FIRMATA::available() {
  std::lock_guard<std::mutex> lock(rx_mutex_);
  return static_cast<int>(rx_buffer_.size());
}

int HardwareI2C_FIRMATA::peek() {
  std::lock_guard<std::mutex> lock(rx_mutex_);
  return rx_buffer_.empty() ? -1 : rx_buffer_.front();
}

int HardwareI2C_FIRMATA::read() {
  std::lock_guard<std::mutex> lock(rx_mutex_);
  if (rx_buffer_.empty()) return -1;
  const int value = rx_buffer_.front();
  rx_buffer_.pop_front();
  return value;
}

void HardwareI2C_FIRMATA::handleSysex(const FirmataSysexMessage &msg) {
  // Payload: addr, seq/tag, then (register, data...) as 7-bit pairs.
  if (msg.data.size() < 4) return;

  std::vector<uint8_t> decoded;
  decoded.reserve((msg.data.size() - 2) / 2);
  for (size_t i = 2; i + 1 < msg.data.size(); i += 2) {
    decoded.push_back(msg.data[i] | (msg.data[i + 1] << 7));
  }
  if (decoded.empty()) return;

  std::lock_guard<std::mutex> lock(rx_mutex_);
  // decoded[0] is the (possibly dummy) register byte; the rest is data.
  for (size_t i = 1; i < decoded.size(); ++i) {
    rx_buffer_.push_back(decoded[i]);
  }
  request_pending_ = false;
  rx_cv_.notify_all();
}

}  // namespace arduino

#endif  // USE_FIRMATA
