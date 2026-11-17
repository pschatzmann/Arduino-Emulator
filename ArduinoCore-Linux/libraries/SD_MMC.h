/*
  SD_MMC.h
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

/// Directory which contains the files of the SD card. Can be changed at
/// compile time (-DSD_MMC_ROOT=\"dir\") or with the environment variable
/// SD_MMC_ROOT.
#ifndef SD_MMC_ROOT
#  define SD_MMC_ROOT "."
#endif

/**
 * @brief ESP32 compatible SD_MMC which is a directory of the computer:
 * SD_MMC.open("/test.txt") opens SD_MMC_ROOT/test.txt. The pins are ignored.
 */
class SDMMCFS : public DirectoryFS {
 public:
  SDMMCFS() : DirectoryFS("SD_MMC_ROOT", SD_MMC_ROOT) {}

  bool setPins(int clk, int cmd, int d0, int d1 = -1, int d2 = -1, int d3 = -1) { return true; }

  bool begin(const char* mountpoint = "/sdcard", bool mode1bit = false,
             bool format_if_mount_failed = false, int sdmmc_frequency = 20000,
             uint8_t maxOpenFiles = 5) {
    return beginRoot();
  }

  uint64_t cardSize() { return totalBytes(); }
};

inline SDMMCFS SD_MMC;
