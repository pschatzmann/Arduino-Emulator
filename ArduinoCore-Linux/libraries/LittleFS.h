/*
  LittleFS.h
  Copyright (c) 2026 Phil Schatzmann. All right reserved.

  This library is free software; you can redistribute it and/or
  modify it under the terms of the GNU Lesser General Public
  License as published by the Free Software Foundation; either
  version 2.1 of the License, or (at your option) any later version.

  This library is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  Lesser General Public License for more details.
*/
#pragma once
#include "DirectoryFS.h"

/// Directory which contains the files of the file system. Can be changed at
/// compile time (-DLITTLEFS_ROOT=\"dir\") or with the environment variable
/// LITTLEFS_ROOT.
#ifndef LITTLEFS_ROOT
#  define LITTLEFS_ROOT "."
#endif

/**
 * @brief ESP32/ESP8266/RP2040 compatible LittleFS which is a directory of the
 * computer: LittleFS.open("/test.txt") opens LITTLEFS_ROOT/test.txt. The files
 * which are uploaded to the microcontroller (e.g. the data folder of the
 * sketch) can be used directly.
 */
class LittleFSClass : public DirectoryFS {
 public:
  LittleFSClass() : DirectoryFS("LITTLEFS_ROOT", LITTLEFS_ROOT) {}
  bool begin(bool formatOnFail = false, const char* basePath = "/littlefs",
             uint8_t maxOpenFiles = 10, const char* partitionLabel = "spiffs") {
    return beginRoot();
  }
};

inline LittleFSClass LittleFS;
