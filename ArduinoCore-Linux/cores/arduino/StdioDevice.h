/*
	StdioDevice.h 
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

#include <iostream>
#include <algorithm>
#include <deque>
#if defined(__unix__) || defined(__APPLE__)
#include <sys/ioctl.h>
#include <unistd.h>
#endif
#include <streambuf>
#include "api/Stream.h"
#include "api/Printable.h"

namespace arduino {

/**
 * @class StdioDevice
 * @brief Provides a Stream interface for standard input/output operations outside the Arduino environment.
 *
 * This class implements the Arduino Stream interface using standard C++ streams (std::cout and std::cin),
 * allowing code that expects Arduino Serial-like objects to work in non-Arduino environments. It supports
 * printing, reading, and flushing, and can be used to provide Serial, Serial1, and Serial2 objects for
 * desktop or emulated environments. The class supports auto-flushing after each print/write operation.
 *
 * Key features:
 * - Implements print, println, and write methods for various data types.
 * - Supports auto-flush to ensure output is immediately visible.
 * - Provides input methods compatible with Arduino's Stream interface.
 * - Can be used as a drop-in replacement for Serial in non-Arduino builds.
 * - Designed for use in emulators or desktop testing of Arduino code.
 */
class StdioDevice : public Stream {
 public:
  StdioDevice(bool autoFlush = true) { auto_flush = autoFlush; }

  ~StdioDevice() {}

  operator bool() const {
    return true;
  }  // For classic while(!Serial) { ... } pattern for USB ready wait

  virtual void begin(int speed) {
    // nothing to be done
  }

  virtual size_t print(const char* str) {
    std::cout << str;
    if (auto_flush) flush();
    return strlen(str);
  }

  virtual size_t println(const char* str = "") {
    std::cout << str << "\n";
    if (auto_flush) flush();
    return strlen(str) + 1;
  }

  virtual size_t print(int val, int radix = DEC) {
    size_t result = Stream::print(val, radix);
    if (auto_flush) flush();
    return result;
  }

  virtual size_t println(int val, int radix = DEC) {
    size_t result = Stream::println(val, radix);
    if (auto_flush) flush();
    return result;
  }

  virtual size_t println(String& str) { return println(str.c_str()); }

  virtual size_t print(String& str) { return print(str.c_str()); }

  virtual size_t println(Printable& p) {
    size_t result = p.printTo(*this);
    std::cout << "\n";
    if (auto_flush) flush();
    return result + 1;
  }

  virtual size_t print(Printable& p) {
    auto result = p.printTo(*this);
    if (auto_flush) flush();
    return result;
  }

  void flush() override { std::cout.flush(); }

  virtual size_t write(const char* str, size_t len) {
    std::cout.write(str, len);
    if (auto_flush) flush();
    return len;
  }

  virtual size_t write(uint8_t* str, size_t len) {
    std::cout.write((const char*)str, len);
    if (auto_flush) flush();
    return len;
  }
  size_t write(const uint8_t* str, size_t len) override {
    std::cout.write((const char*)str, len);
    if (auto_flush) flush();
    return len;
  }

  virtual size_t write(int32_t value) {
    std::cout.put(value);
    if (auto_flush) flush();
    return 1;
  }

  size_t write(uint8_t value) override {
    std::cout.put(value);
    if (auto_flush) flush();
    return 1;
  }

  int available() override {
#if defined(__unix__) || defined(__APPLE__)
    fillInputBuffer();
    return static_cast<int>(inputBuffer_.size());
#endif
    return std::cin.rdbuf()->in_avail();
  }

  int read() override {
#if defined(__unix__) || defined(__APPLE__)
    if (available() == 0) return -1;
    const int value = inputBuffer_.front();
    inputBuffer_.pop_front();
    return value;
#else
    return std::cin.get();
#endif
  }

  int peek() override {
#if defined(__unix__) || defined(__APPLE__)
    return available() > 0 ? inputBuffer_.front() : -1;
#else
    return std::cin.peek();
#endif
  }

 protected:
  bool auto_flush = true;

#if defined(__unix__) || defined(__APPLE__)
 private:
  void fillInputBuffer() {
    int byteCount = 0;
    if (ioctl(STDIN_FILENO, FIONREAD, &byteCount) != 0 || byteCount <= 0) {
      return;
    }

    char buffer[256];
    const size_t bytesToRead = std::min(static_cast<size_t>(byteCount), sizeof(buffer));
    const ssize_t bytesRead = ::read(STDIN_FILENO, buffer, bytesToRead);
    if (bytesRead <= 0) {
      return;
    }
    inputBuffer_.insert(inputBuffer_.end(), buffer, buffer + bytesRead);
  }

  std::deque<unsigned char> inputBuffer_;
#endif
};

inline StdioDevice Serial;
#ifndef USE_RPI
inline StdioDevice Serial2;
#endif

}  // namespace arduino
