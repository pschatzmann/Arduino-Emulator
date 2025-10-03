/*
	RemoteSerial.h 
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

#include "HardwareService.h"
#include "RingBufferExt.h"
#include "api/Stream.h"

namespace arduino {

/**
 * @brief Remote Serial implementation that operates over a communication stream
 *
 * RemoteSerialClass provides Serial/UART functionality by forwarding all operations
 * to a remote Serial controller via a communication stream. This enables Serial
 * communication to be performed on remote hardware while maintaining the standard
 * Stream interface with buffering capabilities.
 * 
 * Key features:
 * - Complete Stream interface implementation
 * - Stream-based remote communication protocol
 * - Bidirectional buffering for read and write operations
 * - Support for multiple serial ports via port numbering
 * - Standard Serial operations (begin, end, baud rate configuration)
 * - Optimized buffering for single character and bulk operations
 * - Real-time bidirectional communication with remote Serial hardware
 * 
 * The class uses HardwareService for protocol handling and maintains separate
 * read and write buffers for efficient data transfer. It can work with any
 * Stream implementation (Serial, TCP, etc.) for remote connectivity.
 *
 * @see Stream
 * @see HardwareService
 * @see RingBufferExt
 */
class RemoteSerialClass : public Stream {
 public:
  RemoteSerialClass(Stream& stream, uint8_t no) {
    this->no = no;
    this->service.setStream(&stream);
  }

  virtual void begin(unsigned long baudrate) {
    service.send(SerialBegin);
    service.send(no);
    service.send((uint64_t)baudrate);
    service.flush();
  }

  virtual void begin(unsigned long baudrate, uint16_t config) {
    service.send(SerialBegin);
    service.send(no);
    service.send((uint64_t)baudrate);
    service.flush();
  }

  virtual void end() {
    service.send(SerialEnd);
    service.send(no);
    service.flush();
  }

  virtual int available() {
    if (read_buffer.available() > 0) {
      return read_buffer.available();
    }
    // otherwise we get it from the remote system
    service.send(SerialAvailable);
    service.send(no);
    service.flush();
    return service.receive16();
  }

  virtual int read() {
    if (read_buffer.available() == 0) {
      uint8_t buffer[max_buffer_len];
      int len = readBytes(buffer, max_buffer_len);
      read_buffer.write(buffer, len);
    }
    if (read_buffer.available() == 0) {
      return -1;
    }
    return read_buffer.read();
  }

  virtual size_t readBytes(uint8_t* buffer, size_t length) {
    if (read_buffer.available() > 0) {
      return read_buffer.read(buffer, length);
    }
    service.send(SerialRead);
    service.send(no);
    service.send((uint64_t)length);
    service.flush();
    int len = service.receive(buffer, length);
    return len;
  }

  virtual int peek() {
    if (read_buffer.available() > 0) {
      return read_buffer.peek();
    }
    service.send(SerialPeek);
    service.flush();
    return service.receive16();
  }

  virtual size_t write(uint8_t c) {
    if (write_buffer.availableToWrite() == 0) {
      flush();
    }
    return write_buffer.write(c);
  }

  virtual size_t write(uint8_t* str, size_t len) {
    flush();
    service.send(SerialWrite);
    service.send(no);
    service.send((uint64_t)len);
    service.send(str, len);
    service.flush();
    return service.receive16();
  }

  void flush() {
#if defined(_MSC_VER)
    int available;
    while ((available = write_buffer.available()) > 0) {
      uint8_t buffer[max_buffer_len];
      write_buffer.read(buffer, min(available, max_buffer_len));
      write(buffer, min(available, max_buffer_len));
    }
#else
    int available = write_buffer.available();
    if (available > 0) {
      uint8_t buffer[available];
      write_buffer.read(buffer, available);
      write(buffer, available);
    }
#endif
    service.send(SerialFlush);
    service.send(no);
    service.flush();
  }

  operator boolean() { return service; }

 protected:
  HardwareService service;
  uint8_t no;
#if defined(_MSC_VER)
  static constexpr int max_buffer_len = 512;  // MSVC does not support VLA
#else
  int max_buffer_len = 512;
#endif
  RingBufferExt write_buffer;
  RingBufferExt read_buffer;
};

}  // namespace arduino
