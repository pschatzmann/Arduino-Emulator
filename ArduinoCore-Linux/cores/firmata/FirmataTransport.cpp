/*
  FirmataTransport.cpp
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
#include "FirmataTransport.h"
#include <chrono>

namespace arduino {

static constexpr uint8_t DIGITAL_MESSAGE = 0x90;
static constexpr uint8_t ANALOG_MESSAGE = 0xE0;
static constexpr uint8_t START_SYSEX = 0xF0;
static constexpr uint8_t END_SYSEX = 0xF7;

FirmataTransport::~FirmataTransport() { end(); }

bool FirmataTransport::begin(Stream &stream) {
  end();  // idempotent

  stream_ = &stream;
  running_ = true;
  thread_ = std::thread(&FirmataTransport::readerLoop, this);
  return true;
}

void FirmataTransport::end() {
  running_ = false;
  if (thread_.joinable()) {
    thread_.join();
  }
  stream_ = nullptr;
}

void FirmataTransport::writeBytes(const uint8_t *data, size_t len) {
  std::lock_guard<std::mutex> lock(write_mutex_);
  if (!stream_) return;
  for (size_t i = 0; i < len; ++i) {
    stream_->write(data[i]);
  }
}

void FirmataTransport::addSysexHandler(uint8_t command, SysexHandler handler) {
  std::lock_guard<std::mutex> lock(sysex_handlers_mutex_);
  sysex_handlers_[command] = std::move(handler);
}

void FirmataTransport::removeSysexHandler(uint8_t command) {
  std::lock_guard<std::mutex> lock(sysex_handlers_mutex_);
  sysex_handlers_.erase(command);
}

void FirmataTransport::readerLoop() {
  enum class State { kWaitCommand, kWaitLsb, kWaitMsb, kInSysex };
  State state = State::kWaitCommand;
  uint8_t command = 0;
  uint8_t channel = 0;
  uint8_t lsb = 0;
  std::vector<uint8_t> sysex_buffer;

  while (running_.load()) {
    if (stream_ == nullptr || stream_->available() <= 0) {
      std::this_thread::sleep_for(std::chrono::milliseconds(2));
      continue;
    }

    const int value = stream_->read();
    if (value < 0) continue;
    const uint8_t byte = static_cast<uint8_t>(value);

    if (state == State::kInSysex) {
      if (byte == END_SYSEX) {
        state = State::kWaitCommand;
        if (!sysex_buffer.empty()) {
          const uint8_t sysexCommand = sysex_buffer.front();
          SysexHandler handler;
          {
            std::lock_guard<std::mutex> lock(sysex_handlers_mutex_);
            const auto it = sysex_handlers_.find(sysexCommand);
            if (it != sysex_handlers_.end()) handler = it->second;
          }
          if (handler) {
            FirmataSysexMessage msg;
            msg.command = sysexCommand;
            msg.data.assign(sysex_buffer.begin() + 1, sysex_buffer.end());
            handler(msg);
          }
        }
        sysex_buffer.clear();
      } else {
        sysex_buffer.push_back(byte);
      }
      continue;
    }

    switch (state) {
      case State::kWaitCommand:
        if (byte == START_SYSEX) {
          state = State::kInSysex;
          sysex_buffer.clear();
        } else if ((byte & 0xF0) == DIGITAL_MESSAGE) {
          command = DIGITAL_MESSAGE;
          channel = byte & 0x0F;
          state = State::kWaitLsb;
        } else if ((byte & 0xF0) == ANALOG_MESSAGE) {
          command = ANALOG_MESSAGE;
          channel = byte & 0x0F;
          state = State::kWaitLsb;
        }
        // Any other command byte (REPORT_VERSION, SYSTEM_RESET, ...) is
        // outside the subset this client needs and is simply discarded.
        break;
      case State::kWaitLsb:
        lsb = byte & 0x7F;
        state = State::kWaitMsb;
        break;
      case State::kWaitMsb: {
        const uint8_t msb = byte & 0x7F;
        if (command == DIGITAL_MESSAGE) {
          if (digital_handler_) {
            FirmataDigitalMessage msg;
            msg.port = channel;
            msg.value = lsb | (static_cast<uint16_t>(msb) << 7);
            digital_handler_(msg);
          }
        } else {
          if (analog_handler_) {
            FirmataAnalogMessage msg;
            msg.pin = channel;
            msg.value = lsb | (static_cast<int>(msb) << 7);
            analog_handler_(msg);
          }
        }
        state = State::kWaitCommand;
        break;
      }
      case State::kInSysex:
        // handled above
        break;
    }
  }
}

}  // namespace arduino

#endif  // USE_FIRMATA
