/*
  HardwareGPIO_RPI.cpp
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

#ifdef USE_RPI

#include "HardwareGPIO_RPI.h"

#include <gpiod.h>  // sudo apt-get install libgpiod-dev

#include <map>
#include <string>

#include "ArduinoLogger.h"

namespace arduino {

// -------------------------------------------------------------------------
// libgpiod v1 vs v2 compatibility layer
//
// Version is detected at cmake time via pkg-config and passed as LIBGPIOD_V2.
// v1: gpiod_chip_open_by_name(), gpiod_line*, gpiod_line_request_output(), etc.
// v2: gpiod_chip_open(path), gpiod_line_request*, gpiod_line_settings, etc.
// -------------------------------------------------------------------------

#if defined(LIBGPIOD_V2)

// ---- libgpiod v2 implementation ----------------------------------------

// Each pin gets its own line request (one line per request for simplicity)
static std::map<pin_size_t, gpiod_line_request*> gpio_requests;
static gpiod_chip* gpio_chip = nullptr;

void HardwareGPIO_RPI::begin() {
  Logger.warning("Activating Raspberry PI: GPIO (libgpiod v2)");
  // Build device path: "gpiochip0" -> "/dev/gpiochip0"
  std::string path = std::string("/dev/") + device_name;
  gpio_chip = gpiod_chip_open(path.c_str());
  if (!gpio_chip) {
    Logger.error("HardwareGPIO_RPI", "Failed to open", path.c_str());
  } else {
    is_open = true;
  }
}

HardwareGPIO_RPI::~HardwareGPIO_RPI() {
  for (auto& kv : gpio_requests) {
    if (kv.second) gpiod_line_request_release(kv.second);
  }
  gpio_requests.clear();
  if (gpio_chip) {
    gpiod_chip_close(gpio_chip);
    gpio_chip = nullptr;
  }
}

static gpiod_line_request* requestLine(gpiod_chip* chip, pin_size_t pin,
                                       gpiod_line_direction direction,
                                       gpiod_line_value initial = GPIOD_LINE_VALUE_INACTIVE) {
  // Release existing request if any
  auto it = gpio_requests.find(pin);
  if (it != gpio_requests.end() && it->second) {
    gpiod_line_request_release(it->second);
    gpio_requests[pin] = nullptr;
  }

  gpiod_line_settings* settings = gpiod_line_settings_new();
  gpiod_line_settings_set_direction(settings, direction);
  if (direction == GPIOD_LINE_DIRECTION_OUTPUT) {
    gpiod_line_settings_set_output_value(settings, initial);
  }

  gpiod_line_config* line_cfg = gpiod_line_config_new();
  unsigned int offset = pin;
  gpiod_line_config_add_line_settings(line_cfg, &offset, 1, settings);

  gpiod_request_config* req_cfg = gpiod_request_config_new();
  gpiod_request_config_set_consumer(req_cfg, "arduino-emulator");

  gpiod_line_request* request = gpiod_chip_request_lines(chip, req_cfg, line_cfg);

  gpiod_line_settings_free(settings);
  gpiod_line_config_free(line_cfg);
  gpiod_request_config_free(req_cfg);

  return request;
}

void HardwareGPIO_RPI::pinMode(pin_size_t pinNumber, PinMode pinMode) {
  if (!gpio_chip) return;
  gpiod_line_direction dir = (pinMode == OUTPUT)
                                 ? GPIOD_LINE_DIRECTION_OUTPUT
                                 : GPIOD_LINE_DIRECTION_INPUT;
  gpiod_line_request* req = requestLine(gpio_chip, pinNumber, dir);
  if (!req) {
    Logger.error("HardwareGPIO_RPI", "Failed to set pin mode");
    return;
  }
  gpio_requests[pinNumber] = req;
}

void HardwareGPIO_RPI::digitalWrite(pin_size_t pinNumber, PinStatus status) {
  // Ensure pin is configured as output
  if (gpio_requests.find(pinNumber) == gpio_requests.end() || !gpio_requests[pinNumber]) {
    pinMode(pinNumber, OUTPUT);
  }
  gpiod_line_request* req = gpio_requests[pinNumber];
  if (req) {
    gpiod_line_value val = (status == HIGH) ? GPIOD_LINE_VALUE_ACTIVE
                                            : GPIOD_LINE_VALUE_INACTIVE;
    if (gpiod_line_request_set_value(req, pinNumber, val) < 0) {
      Logger.error("HardwareGPIO_RPI", "Failed to write value");
    }
  }
}

PinStatus HardwareGPIO_RPI::digitalRead(pin_size_t pinNumber) {
  if (gpio_requests.find(pinNumber) == gpio_requests.end() || !gpio_requests[pinNumber]) {
    pinMode(pinNumber, INPUT);
  }
  gpiod_line_request* req = gpio_requests[pinNumber];
  if (req) {
    gpiod_line_value val = gpiod_line_request_get_value(req, pinNumber);
    if (val == GPIOD_LINE_VALUE_ERROR) {
      Logger.error("HardwareGPIO_RPI", "Failed to read value");
      return LOW;
    }
    return (val == GPIOD_LINE_VALUE_ACTIVE) ? HIGH : LOW;
  }
  return LOW;
}

#else  // libgpiod v1

// ---- libgpiod v1 implementation ----------------------------------------

static std::map<pin_size_t, gpiod_line*> gpio_lines;
static gpiod_chip* gpio_chip = nullptr;

void HardwareGPIO_RPI::begin() {
  Logger.warning("Activating Raspberry PI: GPIO (libgpiod v1)");
  gpio_chip = gpiod_chip_open_by_name(device_name);
  if (!gpio_chip) {
    Logger.error("HardwareGPIO_RPI", "Failed to open", device_name);
  } else {
    is_open = true;
  }
}

HardwareGPIO_RPI::~HardwareGPIO_RPI() {
  for (auto& kv : gpio_lines) {
    if (kv.second) gpiod_line_release(kv.second);
  }
  gpio_lines.clear();
  if (gpio_chip) {
    gpiod_chip_close(gpio_chip);
    gpio_chip = nullptr;
  }
}

void HardwareGPIO_RPI::pinMode(pin_size_t pinNumber, PinMode pinMode) {
  if (!gpio_chip) return;
  // Release existing line if mode is changing
  auto it = gpio_lines.find(pinNumber);
  if (it != gpio_lines.end() && it->second) {
    gpiod_line_release(it->second);
    gpio_lines[pinNumber] = nullptr;
  }
  gpiod_line* line = gpiod_chip_get_line(gpio_chip, pinNumber);
  if (!line) {
    Logger.error("HardwareGPIO_RPI", "Failed to get line");
    return;
  }
  gpio_lines[pinNumber] = line;
  int ret = 0;
  if (pinMode == OUTPUT) {
    ret = gpiod_line_request_output(line, "arduino-emulator", 0);
  } else {
    ret = gpiod_line_request_input(line, "arduino-emulator");
  }
  if (ret < 0) {
    Logger.error("HardwareGPIO_RPI", "Failed to set pin mode");
  }
}

void HardwareGPIO_RPI::digitalWrite(pin_size_t pinNumber, PinStatus status) {
  auto it = gpio_lines.find(pinNumber);
  if (it == gpio_lines.end() || !it->second) {
    pinMode(pinNumber, OUTPUT);
  }
  gpiod_line* line = gpio_lines[pinNumber];
  if (line) {
    int value = (status == HIGH) ? 1 : 0;
    if (gpiod_line_set_value(line, value) < 0) {
      Logger.error("HardwareGPIO_RPI", "Failed to write value");
    }
  }
}

PinStatus HardwareGPIO_RPI::digitalRead(pin_size_t pinNumber) {
  auto it = gpio_lines.find(pinNumber);
  if (it == gpio_lines.end() || !it->second) {
    pinMode(pinNumber, INPUT);
  }
  gpiod_line* line = gpio_lines[pinNumber];
  if (line) {
    int value = gpiod_line_get_value(line);
    if (value < 0) {
      Logger.error("HardwareGPIO_RPI", "Failed to read value");
      return LOW;
    }
    return value ? HIGH : LOW;
  }
  return LOW;
}

#endif  // GPIOD_API_VERSION

int HardwareGPIO_RPI::analogRead(pin_size_t pinNumber) {
  Logger.error("HardwareGPIO_RPI",
               "analogRead not supported on Raspberry Pi GPIO");
  return 0;
}

void HardwareGPIO_RPI::analogReference(uint8_t mode) {
  m_analogReference = mode;
}
uint32_t HardwareGPIO_RPI::getFrequency(int pin) {
  auto it = gpio_frequencies.find(pin);
  if (it != gpio_frequencies.end()) {
    return it->second;
  }
  return 1000;  // default frequency
}

void HardwareGPIO_RPI::analogWrite(pin_size_t pinNumber, int value) {
  // Supported hardware PWM pins on Raspberry Pi: 12, 13, 18, 19
  // Use sysfs interface: /sys/class/pwm/pwmchip0
  bool supported = false;
  for (int i = 0; i < 4; ++i) {
    if (pinNumber == pwm_pins[i]) {
      supported = true;
      break;
    }
  }
  if (!supported) {
    Logger.error("HardwareGPIO_RPI",
                 "analogWrite: pin does not support hardware PWM");
    return;
  }

  // Map pin to PWM channel (chip0: pwm0 for GPIO18, pwm1 for GPIO19, etc.)
  int pwm_channel = -1;
  if (pinNumber == 18)
    pwm_channel = 0;
  else if (pinNumber == 19)
    pwm_channel = 1;
  else if (pinNumber == 12)
    pwm_channel = 0;  // On some models
  else if (pinNumber == 13)
    pwm_channel = 1;  // On some models

  if (pwm_channel < 0) {
    Logger.error("HardwareGPIO_RPI",
                 "analogWrite: could not map pin to PWM channel");
    return;
  }

  // Export PWM channel if not already exported
  char export_path[64];
  snprintf(export_path, sizeof(export_path), "/sys/class/pwm/pwmchip0/export");
  FILE* fexp = fopen(export_path, "w");
  if (fexp) {
    fprintf(fexp, "%d", pwm_channel);
    fclose(fexp);
  }
  // Set period and duty cycle
  char period_path[64], duty_path[64], enable_path[64];
  snprintf(period_path, sizeof(period_path),
           "/sys/class/pwm/pwmchip0/pwm%d/period", pwm_channel);
  snprintf(duty_path, sizeof(duty_path),
           "/sys/class/pwm/pwmchip0/pwm%d/duty_cycle", pwm_channel);
  snprintf(enable_path, sizeof(enable_path),
           "/sys/class/pwm/pwmchip0/pwm%d/enable", pwm_channel);

  // Determine period from frequency using getFrequency()
  uint32_t freq = getFrequency(pinNumber);
  uint32_t period_ns = (freq > 0)
                           ? (1000000000UL / freq)
                           : 20000;  // fallback to 20,000 ns if freq is 0
  FILE* fperiod = fopen(period_path, "w");
  if (fperiod) {
    fprintf(fperiod, "%u", period_ns);
    fclose(fperiod);
  }
  // Duty cycle: value in range 0-max_value
  int duty = (value < 0) ? 0 : (value > max_value) ? max_value : value;
  int duty_ns = (duty * period_ns) / max_value;
  FILE* fduty = fopen(duty_path, "w");
  if (fduty) {
    fprintf(fduty, "%d", duty_ns);
    fclose(fduty);
  }
  // Enable PWM
  FILE* fenable = fopen(enable_path, "w");
  if (fenable) {
    fprintf(fenable, "%d", 1);
    fclose(fenable);
  }
}

void HardwareGPIO_RPI::tone(uint8_t _pin, unsigned int frequency,
                            unsigned long duration) {
  analogWriteFrequency(_pin, frequency);
  analogWrite(_pin, max_value / 2);  // 50% duty cycle for 8-bit PWM
  if (duration > 0) {
    delay(duration);
    noTone(_pin);
  }
}

void HardwareGPIO_RPI::noTone(uint8_t _pin) { analogWrite(_pin, 0); }

unsigned long HardwareGPIO_RPI::pulseIn(uint8_t pin, uint8_t state,
                                        unsigned long timeout) {
  Logger.error("HardwareGPIO_RPI", "pulseIn not implemented");
  return 0;
}

unsigned long HardwareGPIO_RPI::pulseInLong(uint8_t pin, uint8_t state,
                                            unsigned long timeout) {
  Logger.error("HardwareGPIO_RPI", "pulseInLong not implemented");
  return 0;
}

void HardwareGPIO_RPI::analogWriteFrequency(pin_size_t pin, uint32_t freq) {
  bool supported = false;
  for (int i = 0; i < 4; ++i) {
    if (pin == pwm_pins[i]) {
      supported = true;
      break;
    }
  }
  if (!supported) {
    Logger.error("HardwareGPIO_RPI",
                 "analogWriteFrequency: pin does not support hardware PWM");
    return;
  }
  gpio_frequencies[pin] = freq;
}

void HardwareGPIO_RPI::analogWriteResolution(uint8_t bits) {
  if (bits == 0 || bits > 16) {
    Logger.error("HardwareGPIO_RPI",
                 "analogWriteResolution: bits must be between 1 and 16");
    return;
  }
  max_value = (1 << bits) - 1;  // Calculate max value based on bits
}

}  // namespace arduino

#endif