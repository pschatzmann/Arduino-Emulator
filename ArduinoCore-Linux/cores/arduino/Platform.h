#pragma once

#if defined(_WIN32)
#define ARDUINO_EMULATOR_WINDOWS 1
#else
#define ARDUINO_EMULATOR_WINDOWS 0
#endif

#if defined(__linux__)
#define ARDUINO_EMULATOR_LINUX 1
#else
#define ARDUINO_EMULATOR_LINUX 0
#endif
