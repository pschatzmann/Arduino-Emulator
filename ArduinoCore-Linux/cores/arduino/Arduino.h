#pragma once

/*
  Arduino.h - Main include file for the Arduino SDK
  Copyright (c) 2005-2013 Arduino Team.  All right reserved.
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
  Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
*/

#ifndef HOST
#  define HOST
#endif

#if defined(_MSC_VER) && !defined(M_PI) && !defined(_USE_MATH_DEFINES)
#define _USE_MATH_DEFINES // to provide const like M_PI via <math.h>
#endif

#if defined(_MSC_VER)
// Not available under MSVC
#define __attribute__(x) // nothing
#define __builtin_constant_p(x) (0) // non-constant
#endif

#if defined(_MSC_VER)
// Temporary unsupported under Win/MSVC
#define SKIP_HARDWARE_SETUP
#define SKIP_HARDWARE_WIFI
#endif

#include "ArduinoAPI.h"
#include "ArdStdio.h"
#include "ArduinoLogger.h"
#include "HardwareSetup.h"
#include "Serial.h"
#include "RemoteSerial.h"

extern HardwareImpl Hardware; // implementation for gpio, spi, i2c

using namespace arduino;