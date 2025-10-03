/*
  FileStream.h 
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
#include <fstream>
#include <iostream>
#include "api/Stream.h"

namespace arduino {

/**
 * @brief We use the FileStream class to be able to provide Serail, Serial1 and
 * Serial2 outside of the Arduino environment;
 */
class FileStream : public Stream {
 public:
  FileStream(const char* outDevice = "/dev/stdout",
             const char* inDevice = "/dev/stdin") {
    open(outDevice, inDevice);
  }

  ~FileStream() {
    in.close();
    out.close();
  }

  void open(const char* outDevice, const char* inDevice) {
    if (outDevice != nullptr) out.open(outDevice, std::ios::out);
    if (inDevice != nullptr) in.open(inDevice, std::ios::in);
  }

  virtual void begin(int speed) {
    // nothing to be done
  }

  virtual void print(const char* str) {
    out << str;
    out.flush();
  }

  virtual void println(const char* str = "") {
    out << str << "\n";
    out.flush();
  }

  virtual void print(int str) {
    out << str;
    out.flush();
  }

  virtual void println(int str) {
    out << str << "\n";
    out.flush();
  }

  virtual void flush() { out.flush(); }

  virtual void write(const char* str, int len) { out.write(str, len); }

  virtual void write(uint8_t* str, int len) {
    out.write((const char*)str, len);
  }

  virtual size_t write(int32_t value) {
    out.put(value);
    return 1;
  }

  virtual size_t write(uint8_t value) {
    out.put(value);
    return 1;
  }

  virtual int available() { return in.rdbuf()->in_avail(); };

  virtual int read() { return in.get(); }

  virtual int peek() { return in.peek(); }

 protected:
  std::fstream out;
  std::fstream in;
};

/**
 * @brief Global Serial1 instance for secondary serial communication.
 *
 * This object provides access to the /dev/ttyACM0 device, typically used for
 * USB serial or secondary UART on Linux systems. Use this instance in your
 * sketches to perform serial communication, similar to the standard Arduino
 * Serial1 object. Example: Serial1.begin(9600); Serial1.println("Hello from
 * Serial1");
 */
static FileStream Serial1("/dev/ttyACM0");

}  // namespace arduino
