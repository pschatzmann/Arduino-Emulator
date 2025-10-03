/*
	Unsupported.cpp 
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
#include "ArduinoLogger.h"
#include "api/Common.h"

void attachInterrupt(pin_size_t interruptNumber, voidFuncPtr callback,
                     PinStatus mode) {
  arduino::Logger.error("attachInterrupt", "not supported");
}

void attachInterruptParam(pin_size_t interruptNumber, voidFuncPtrParam callback,
                          PinStatus mode, void* param) {
  arduino::Logger.error("attachInterruptParam", "not supported");
}

void detachInterrupt(pin_size_t interruptNumber) {
  arduino::Logger.error("detachInterrupt", "not supported");
}

// to compile pluggable usb
void* epBuffer(unsigned int n) { return nullptr; }