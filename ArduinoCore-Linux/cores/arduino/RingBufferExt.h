/*
  RingBufferExt.h 
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

#include "stddef.h"
#include "stdint.h"
#include "vector"

namespace arduino {

/**
 * @brief Implementation of a Simple Circular Buffer. Instead of comparing the
 * position of the read and write pointer in order to figure out if we still
 * have characters available or space left to write we keep track of the actual
 * length which is easier to follow. This class was implemented to support the
 * reading and writing of arrays.
 */

class RingBufferExt {
 public:
  RingBufferExt(int size = 1024) {
    max_len = size;
    buffer.resize(size);
  }

  // available to read
  int available() { return actual_len; }

  int availableToWrite() { return max_len - actual_len; }

  // reads a single character and makes it availble on the buffer
  int read() {
    int result = peek();
    if (result > -1) {
      actual_read_pos++;
      actual_len--;
    }
    // wrap to the start
    if (actual_read_pos >= max_len) {
      actual_read_pos = 0;
    }
    return result;
  }

  int read(char* str, int len) { return read((uint8_t*)str, len); }

  int read(uint8_t* str, int len) {
    for (int j = 0; j < len; j++) {
      int current = read();
      if (current <= 0) {
        return j;
      }
      str[j] = current;
    }
    return len;
  }

  // peeks the actual character
  int peek() {
    int result = -1;
    if (actual_len > 0) {
      result = buffer[actual_read_pos];
    }
    return result;
  };

  size_t write(uint8_t ch) {
    int result = 0;
    if (actual_len < max_len) {
      result = 1;
      buffer[actual_write_pos] = ch;
      actual_write_pos++;
      actual_len++;
      if (actual_write_pos >= max_len) {
        actual_write_pos = 0;
      }
    }
    return result;
  }

  size_t write(char* str, int len) { return write((uint8_t*)str, len); }

  size_t write(uint8_t* str, int len) {
    for (int j = 0; j < len; j++) {
      int result = write(str[j]);
      if (result == 0) {
        return j;
      }
    }
    return len;
  }

 protected:
  std::vector<char> buffer;
  int max_len;
  int actual_len = 0;
  int actual_read_pos = 0;
  int actual_write_pos = 0;
};

}  // namespace arduino
