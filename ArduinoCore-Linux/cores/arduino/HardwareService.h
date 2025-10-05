/*
	HardwareService.h
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

#include "api/Stream.h"

namespace arduino {

/**
 * @brief We virtualize the hardware and send the requests and replys over
 * a stream.
 *
 **/

enum HWCalls {
  I2cBegin0,
  I2cBegin1,
  I2cEnd,
  I2cSetClock,
  I2cBeginTransmission,
  I2cEndTransmission1,
  I2cEndTransmission,
  I2cRequestFrom3,
  I2cRequestFrom2,
  I2cOnReceive,
  I2cOnRequest,
  I2cWrite,
  I2cAvailable,
  I2cRead,
  I2cPeek,
  SpiTransfer,
  SpiTransfer8,
  SpiTransfer16,
  SpiUsingInterrupt,
  SpiNotUsingInterrupt,
  SpiBeginTransaction,
  SpiEndTransaction,
  SpiAttachInterrupt,
  SpiDetachInterrupt,
  SpiBegin,
  SpiEnd,
  GpioPinMode,
  GpioDigitalWrite,
  GpioDigitalRead,
  GpioAnalogRead,
  GpioAnalogReference,
  GpioAnalogWrite,
  GpioTone,
  GpioNoTone,
  GpioPulseIn,
  GpioPulseInLong,
  GpioAnalogWriteFrequency,
  GpioAnalogWriteResolution,
  SerialBegin,
  SerialEnd,
  SerialWrite,
  SerialRead,
  SerialAvailable,
  SerialPeek,
  SerialFlush,
  I2sSetup,
  I2sBegin3,
  I2sBegin2,
  I2sEnd,
  I2sAvailable,
  I2sRead,
  I2sPeek,
  I2sFlush,
  I2sWrite,
  I2sAvailableForWrite,
  I2sSetBufferSize
};

/**
 * @class HardwareService
 * @brief Provides a virtualized hardware communication service for SPI, I2C, I2S, and GPIO over a stream.
 *
 * This class encapsulates the logic to tunnel hardware protocol messages (SPI, I2C, I2S, GPIO, Serial, etc.)
 * over a generic stream interface. It handles serialization and deserialization of protocol calls and data,
 * ensuring correct endianness (little endian by default for embedded devices). The class provides methods
 * to send and receive various data types, manage the underlying stream, and perform byte-swapping as needed.
 *
 * Key features:
 * - Tunnels hardware protocol messages over a stream (e.g., network, serial).
 * - Supports sending and receiving multiple data types with correct endianness.
 * - Provides blocking read with timeout for reliable communication.
 * - Handles byte order conversion for cross-platform compatibility.
 * - Can be used as a base for remote hardware emulation or proxying.
 *
 * Usage:
 *   - Set the stream using setStream().
 *   - Use send() methods to transmit protocol calls and data.
 *   - Use receive methods to read responses from the remote hardware.
 *   - Use flush() to ensure all data is sent.
 *
 */

class HardwareService {
 public:
  HardwareService() {}

  void setStream(Stream* str) { io = str; }

  void send(HWCalls call) {
    uint16_t val = (uint16_t)call;
    io->write((uint8_t*)&val, sizeof(uint16_t));
  }

  void send(uint8_t data) { io->write((uint8_t*)&data, sizeof(data)); }

  void send(uint16_t dataIn) {
    uint16_t data = swap_uint16(dataIn);
    io->write((uint8_t*)&data, sizeof(data));
  }

  void send(uint32_t dataIn) {
    uint32_t data = swap_uint32(dataIn);
    io->write((uint8_t*)&data, sizeof(data));
  }

  void send(uint64_t dataIn) {
    uint32_t data = swap_uint64(dataIn);
    io->write((uint8_t*)&data, sizeof(data));
  }

  void send(int32_t dataIn) {
    int32_t data = swap_int32(dataIn);
    io->write((uint8_t*)&data, sizeof(data));
  }
  void send(int64_t dataIn) {
    int32_t data = swap_int64(dataIn);
    io->write((uint8_t*)&data, sizeof(data));
  }

  void send(bool data) { io->write((uint8_t*)&data, sizeof(data)); }

  void send(void* data, size_t len) { io->write((uint8_t*)data, len); }

  void flush() { io->flush(); }

  uint16_t receive16() {
    uint16_t result;
    blockingRead((char*)&result, sizeof(uint16_t));
    return swap_uint16(result);
  }

  uint32_t receive32() {
    uint32_t result;
    blockingRead((char*)&result, sizeof(uint32_t));
    return swap_uint32(result);
  }

  uint64_t receive64() {
    uint64_t result;
    blockingRead((char*)&result, sizeof(uint64_t));
    return swap_uint64(result);
  }

  uint8_t receive8() {
    uint8_t result;
    blockingRead((char*)&result, sizeof(uint8_t));
    return result;
  }

  uint16_t receive(void* data, int len) {
    return blockingRead((char*)data, len);
  }

  operator bool() { return io != nullptr; }

 protected:
  Stream* io = nullptr;
  bool isLittleEndian = !is_big_endian();
  int timeout_ms = 1000;

  uint16_t blockingRead(void* data, int len, int timeout = 1000) {
    int offset = 0;
    long start = millis();
    while (offset < len && (millis() - start) < timeout) {
      int n = io->readBytes((char*)data + offset, len - offset);
      offset += n;
    }
    return offset;
  }

  // check if the system is big endian
  bool is_big_endian(void) {
    union {
      uint32_t i;
      char c[4];
    } bint = {0x01020304};

    return bint.c[0] == 1;
  }

  //! Byte swap unsigned short
  uint16_t swap_uint16(uint16_t val) {
    if (isLittleEndian) return val;
    return (val << 8) | (val >> 8);
  }

  //! Byte swap short
  int16_t swap_int16(int16_t val) {
    if (isLittleEndian) return val;
    return (val << 8) | ((val >> 8) & 0xFF);
  }

  //! Byte swap unsigned int
  uint32_t swap_uint32(uint32_t val) {
    if (isLittleEndian) return val;
    val = ((val << 8) & 0xFF00FF00) | ((val >> 8) & 0xFF00FF);
    return (val << 16) | (val >> 16);
  }

  //! Byte swap int
  int32_t swap_int32(int32_t val) {
    if (isLittleEndian) return val;
    val = ((val << 8) & 0xFF00FF00) | ((val >> 8) & 0xFF00FF);
    return (val << 16) | ((val >> 16) & 0xFFFF);
  }

  int64_t swap_int64(int64_t val) {
    if (isLittleEndian) return val;
    val = ((val << 8) & 0xFF00FF00FF00FF00ULL) |
          ((val >> 8) & 0x00FF00FF00FF00FFULL);
    val = ((val << 16) & 0xFFFF0000FFFF0000ULL) |
          ((val >> 16) & 0x0000FFFF0000FFFFULL);
    return (val << 32) | ((val >> 32) & 0xFFFFFFFFULL);
  }

  uint64_t swap_uint64(uint64_t val) {
    if (isLittleEndian) return val;
    val = ((val << 8) & 0xFF00FF00FF00FF00ULL) |
          ((val >> 8) & 0x00FF00FF00FF00FFULL);
    val = ((val << 16) & 0xFFFF0000FFFF0000ULL) |
          ((val >> 16) & 0x0000FFFF0000FFFFULL);
    return (val << 32) | (val >> 32);
  }
};

}  // namespace arduino
