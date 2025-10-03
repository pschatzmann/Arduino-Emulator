/*
  ArduinoLogger.h 
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
#include "StdioDevice.h"
#include "api/Stream.h"

namespace arduino {

/**
 * @brief A simple Logger that writes messages dependent on the log level
 *
 */

class ArduinoLogger {
 public:
  ArduinoLogger() = default;

  /**
   * @brief Supported log levels
   *
   */
  enum LogLevel { Debug, Info, Warning, Error };

  const char* LogLevelTxt[4] = {"Debug", "Info", "Warning", "Error"};

  // activate the logging
  void begin(Stream& out, LogLevel level = Warning) {
    this->log_stream_ptr = &out;
    this->log_level = level;
  }

  // checks if the logging is active
  bool isLogging() { return log_stream_ptr != nullptr; }

  void error(const char* str, const char* str1 = nullptr,
             const char* str2 = nullptr) {
    log(Error, str, str1, str2);
  }

  void info(const char* str, const char* str1 = nullptr,
            const char* str2 = nullptr) {
    log(Info, str, str1, str2);
  }

  void warning(const char* str, const char* str1 = nullptr,
               const char* str2 = nullptr) {
    log(Warning, str, str1, str2);
  }

  void debug(const char* str, const char* str1 = nullptr,
             const char* str2 = nullptr) {
    log(Debug, str, str1, str2);
  }

  // write an message to the log
  void log(LogLevel current_level, const char* str, const char* str1 = nullptr,
           const char* str2 = nullptr) {
    if (log_stream_ptr != nullptr) {
      if (current_level >= log_level) {
        log_stream_ptr->print("Emulator - ");
        log_stream_ptr->print((char*)LogLevelTxt[current_level]);
        log_stream_ptr->print(": ");
        log_stream_ptr->print((char*)str);
        if (str1 != nullptr) {
          log_stream_ptr->print(" ");
          log_stream_ptr->print((char*)str1);
        }
        if (str2 != nullptr) {
          log_stream_ptr->print(" ");
          log_stream_ptr->print((char*)str2);
        }
        log_stream_ptr->println();
        log_stream_ptr->flush();
      }
    }
  }

 protected:
  Stream* log_stream_ptr = &Serial;
  LogLevel log_level = Warning;
};

static ArduinoLogger Logger;

}  // namespace arduino
