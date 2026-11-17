/*
  Esp.h
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
#include <stdint.h>
#include <stdlib.h>

#if defined(_WIN32)
#  include <windows.h>
#elif defined(__APPLE__)
#  include <sys/sysctl.h>
#  include <unistd.h>
#else
#  include <unistd.h>
#endif

namespace arduino {

/**
 * @brief ESP32 compatible ESP object (ESP.getFreeHeap() ...), so that ESP32
 * sketches also run on the desktop. The heap is the memory of the computer;
 * there is no PSRAM: the psram functions return 0.
 */
class EspClass {
 public:
  /// Physical memory of the computer
  uint32_t getHeapSize() { return clamp(physicalMemory(false)); }
  /// Available physical memory of the computer
  uint32_t getFreeHeap() { return clamp(physicalMemory(true)); }
  uint32_t getMinFreeHeap() { return getFreeHeap(); }
  uint32_t getMaxAllocHeap() { return getFreeHeap(); }
  uint32_t getPsramSize() { return 0; }
  uint32_t getFreePsram() { return 0; }
  uint32_t getMinFreePsram() { return 0; }
  uint32_t getMaxAllocPsram() { return 0; }
  uint32_t getCpuFreqMHz() { return 1000; }
  const char* getChipModel() { return "Desktop"; }
  uint8_t getChipRevision() { return 0; }
  uint8_t getChipCores() {
#if defined(_WIN32)
    SYSTEM_INFO info;
    GetSystemInfo(&info);
    return static_cast<uint8_t>(info.dwNumberOfProcessors);
#else
    long n = sysconf(_SC_NPROCESSORS_ONLN);
    return n > 0 ? static_cast<uint8_t>(n) : 1;
#endif
  }
  const char* getSdkVersion() { return "arduino-emulator"; }
  uint32_t getFlashChipSize() { return 0; }
  uint64_t getEfuseMac() { return 0; }
  void restart() { exit(0); }

 protected:
  static uint32_t clamp(uint64_t v) { return v > 0xFFFFFFFFull ? 0xFFFFFFFFu : static_cast<uint32_t>(v); }

  static uint64_t physicalMemory(bool available) {
#if defined(_WIN32)
    MEMORYSTATUSEX status;
    status.dwLength = sizeof(status);
    if (!GlobalMemoryStatusEx(&status)) return 0;
    return available ? status.ullAvailPhys : status.ullTotalPhys;
#elif defined(__APPLE__)
    int64_t size = 0;
    size_t len = sizeof(size);
    if (sysctlbyname("hw.memsize", &size, &len, nullptr, 0) != 0) return 0;
    return static_cast<uint64_t>(size);  // no simple "available" on macOS
#else
    long pages = sysconf(available ? _SC_AVPHYS_PAGES : _SC_PHYS_PAGES);
    long page_size = sysconf(_SC_PAGESIZE);
    if (pages < 0 || page_size < 0) return 0;
    return static_cast<uint64_t>(pages) * static_cast<uint64_t>(page_size);
#endif
  }
};

inline EspClass ESP;

/// ESP32: cpu frequency in MHz
inline uint32_t getCpuFrequencyMhz() { return ESP.getCpuFreqMHz(); }

}  // namespace arduino
