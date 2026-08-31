/*
  HardwareSPI_FIR.cpp
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
#include "HardwareSPI_FIR.h"
#include <algorithm>
#include <chrono>

namespace arduino {

// SPI_DATA is the single SysEx command byte all SPI sub-messages travel
// under; the actual operation is the first payload byte (the subcommand).
static constexpr uint8_t SPI_DATA = 0x68;

static constexpr uint8_t SPI_BEGIN = 0x00;
static constexpr uint8_t SPI_DEVICE_CONFIG = 0x01;
static constexpr uint8_t SPI_TRANSFER = 0x02;
static constexpr uint8_t SPI_REPLY = 0x05;
static constexpr uint8_t SPI_END = 0x06;

HardwareSPI_FIRMATA::HardwareSPI_FIRMATA(uint8_t deviceId, int8_t csPin)
    : device_id_(deviceId), cs_pin_(csPin) {}

HardwareSPI_FIRMATA::~HardwareSPI_FIRMATA() { end(); }

bool HardwareSPI_FIRMATA::begin(Stream &stream) {
  end();
  owned_transport_ = std::make_unique<FirmataTransport>();
  owned_transport_->begin(stream);
  return begin(*owned_transport_);
}

bool HardwareSPI_FIRMATA::begin(FirmataTransport &transport) {
  if (&transport != owned_transport_.get()) {
    end();
  }
  transport_ = &transport;
  is_open = true;

  transport_->addSysexHandler(
      SPI_DATA, [this](const FirmataSysexMessage &msg) { handleSysex(msg); });

  // SPI_BEGIN: only channel 0 is supported by the firmware.
  const uint8_t msg[5] = {0xF0, SPI_DATA, SPI_BEGIN, 0, 0xF7};
  transport_->writeBytes(msg, sizeof(msg));
  return true;
}

void HardwareSPI_FIRMATA::begin() {
  // begin(Stream&)/begin(FirmataTransport&) must be called first; nothing
  // further to do for the parameterless HardwareSPI::begin().
}

void HardwareSPI_FIRMATA::end() {
  if (transport_) {
    const uint8_t msg[4] = {0xF0, SPI_DATA, SPI_END, 0xF7};
    transport_->writeBytes(msg, sizeof(msg));
    transport_->removeSysexHandler(SPI_DATA);
  }
  if (owned_transport_) {
    owned_transport_->end();
    owned_transport_.reset();
  }
  transport_ = nullptr;
  is_open = false;
  configured_ = false;
}

void HardwareSPI_FIRMATA::beginTransaction(SPISettings settings) {
  settings_ = settings;
  sendDeviceConfig();
}

void HardwareSPI_FIRMATA::sendDeviceConfig() {
  if (!transport_) return;

  const uint32_t speed = settings_.getClockFreq();
  const uint8_t bitOrderBit = settings_.getBitOrder() == MSBFIRST ? 1 : 0;
  const uint8_t dataModeBits =
      static_cast<uint8_t>(settings_.getDataMode()) & 0x3;
  const uint8_t configByte = bitOrderBit | (dataModeBits << 1);
  // packedData bit (bit 3) left at 0: we always use the simple
  // 2-Firmata-bytes-per-data-byte encoding, not the 7-bit packed form.

  std::vector<uint8_t> msg;
  msg.reserve(16);
  msg.push_back(0xF0);
  msg.push_back(SPI_DATA);
  msg.push_back(SPI_DEVICE_CONFIG);
  msg.push_back(device_id_);
  msg.push_back(configByte);
  msg.push_back(static_cast<uint8_t>(speed & 0x7F));
  msg.push_back(static_cast<uint8_t>((speed >> 7) & 0x7F));
  msg.push_back(static_cast<uint8_t>((speed >> 14) & 0x7F));
  msg.push_back(static_cast<uint8_t>((speed >> 21) & 0x7F));
  msg.push_back(static_cast<uint8_t>((speed >> 28) & 0x0F));
  msg.push_back(0);  // word size: 0 = default (8 bit)
  msg.push_back(cs_pin_ >= 0 ? 1 : 0);  // csPinOptions: bit0 = device-managed CS
  msg.push_back(cs_pin_ >= 0 ? static_cast<uint8_t>(cs_pin_) : 0);
  msg.push_back(0xF7);
  transport_->writeBytes(msg.data(), msg.size());

  configured_ = true;
}

void HardwareSPI_FIRMATA::doTransfer(uint8_t *buf, size_t count) {
  if (!transport_ || count == 0) return;
  if (!configured_) sendDeviceConfig();

  {
    std::lock_guard<std::mutex> lock(reply_mutex_);
    reply_buffer_.clear();
    reply_pending_ = true;
  }

  std::vector<uint8_t> msg;
  msg.reserve(9 + count * 2);
  msg.push_back(0xF0);
  msg.push_back(SPI_DATA);
  msg.push_back(SPI_TRANSFER);
  msg.push_back(device_id_);
  msg.push_back(0);  // tag (unused; only one transfer in flight at a time)
  msg.push_back(1);  // deselect CS after this transfer
  msg.push_back(0);  // padding (unused in non-dummy/transfer mode)
  for (size_t i = 0; i < count; ++i) {
    msg.push_back(buf[i] & 0x7F);
    msg.push_back((buf[i] >> 7) & 0x7F);
  }
  msg.push_back(0xF7);
  transport_->writeBytes(msg.data(), msg.size());

  std::unique_lock<std::mutex> lock(reply_mutex_);
  reply_cv_.wait_for(
      lock, std::chrono::milliseconds(timeout_ms_),
      [this] { return !reply_pending_ || !reply_buffer_.empty(); });
  reply_pending_ = false;

  const size_t received = std::min(count, reply_buffer_.size());
  for (size_t i = 0; i < received; ++i) {
    buf[i] = reply_buffer_[i];
  }
}

uint8_t HardwareSPI_FIRMATA::transfer(uint8_t data) {
  uint8_t buf[1] = {data};
  doTransfer(buf, 1);
  return buf[0];
}

uint16_t HardwareSPI_FIRMATA::transfer16(uint16_t data) {
  // Matches the common Arduino core convention: MSB first regardless of
  // the configured bit order.
  uint8_t buf[2] = {static_cast<uint8_t>((data >> 8) & 0xFF),
                     static_cast<uint8_t>(data & 0xFF)};
  doTransfer(buf, 2);
  return (static_cast<uint16_t>(buf[0]) << 8) | buf[1];
}

void HardwareSPI_FIRMATA::transfer(void *buf, size_t count) {
  doTransfer(static_cast<uint8_t *>(buf), count);
}

void HardwareSPI_FIRMATA::handleSysex(const FirmataSysexMessage &msg) {
  // Payload: subcommand, deviceId, tag, numBytes, then numBytes 7-bit pairs.
  if (msg.data.size() < 4) return;
  if (msg.data[0] != SPI_REPLY) return;
  if (msg.data[1] != device_id_) return;

  const uint8_t numBytes = msg.data[3];
  std::vector<uint8_t> decoded;
  decoded.reserve(numBytes);
  for (size_t i = 4; i + 1 < msg.data.size() && decoded.size() < numBytes;
       i += 2) {
    decoded.push_back(msg.data[i] | (msg.data[i + 1] << 7));
  }

  std::lock_guard<std::mutex> lock(reply_mutex_);
  reply_buffer_ = std::move(decoded);
  reply_pending_ = false;
  reply_cv_.notify_all();
}

}  // namespace arduino

#endif  // USE_FIRMATA
