/*
  Serial.h 
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

#pragma once

#if USE_SERIALLIB

#include "serialib.h"
#include "api/HardwareSerial.h"

namespace arduino {

/**
 * Arduino Stream implementation which is using serialib
 * https://github.com/imabot2/serialib
 */

class SerialImpl : public HardwareSerial {
 public:
  SerialImpl(const char* device = "/dev/ttyACM0") { this->device = device; }

  virtual void begin(unsigned long baudrate) { open(baudrate); }

  virtual void begin(unsigned long baudrate, uint16_t config) {
    open(baudrate);
  }

  virtual void end() {
    is_open = false;
    serial.closeDevice();
  }

  virtual int available(void) { return serial.available(); };

  virtual int peek(void) {
    if (peek_char == -1) {
      peek_char = read();
    }
    return peek_char;
  }

  virtual int read(void) {
    int result = -1;
    if (peek_char != -1) {
      result = peek_char;
      peek_char = -1;
    } else {
      char c;
      result = serial.readChar(&c, timeout);
    }
    return result;
  };

  virtual void flush(void) {};

  virtual size_t write(uint8_t c) { return serial.writeChar(c); }

  virtual operator bool() { return is_open; }

  // sets maximum milliseconds to wait for stream data, default is 1 second
  void setTimeout(unsigned long timeout) {
    this->timeout = timeout;
    HardwareSerial::setTimeout(timeout);
  }

 protected:
  const char* device;
  serialib serial;
  bool is_open = false;
  int peek_char = -1;
  long timeout = 1000;

  virtual void open(unsigned long baudrate) {
    if (!serial.openDevice(device, baudrate)) {
      Logger.error("SerialImpl", "could not open", device);
    }
    is_open = true;
  }
};

}  // namespace arduino

#endif