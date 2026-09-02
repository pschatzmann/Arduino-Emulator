#pragma once
#include "Platform.h"

#if ARDUINO_EMULATOR_WINDOWS
#include "DesktopSocketWindows.h"
#else
#include "DesktopSocketPosix.h"
#endif
