
#ifdef USE_RPI

#include <gpiod.h> // sudo apt-get install libgpiod-dev
#include <map>
#include "HardwareGPIO_RPI.h"
#include "Hardware.h"
#include "ArduinoLogger.h"

namespace arduino {

// Helper: map pin number to gpiod_line*
static std::map<pin_size_t, gpiod_line*> gpio_lines;
static gpiod_chip* gpio_chip = nullptr;

HardwareGPIO_RPI::HardwareGPIO_RPI() {
  Logger.warning("Activating Rasperry PI: GPIO");
  gpio_chip = gpiod_chip_open_by_name("gpiochip0");
  if (!gpio_chip) {
    Logger.error("HardwareGPIO_RPI", "Failed to open gpiochip0");
  } else {
    is_open = true;
  }
}

HardwareGPIO_RPI::~HardwareGPIO_RPI() {
  Hardware.gpio = nullptr;
  for (auto& kv : gpio_lines) {
    gpiod_line_release(kv.second);
  }
  gpio_lines.clear();
  if (gpio_chip) {
    gpiod_chip_close(gpio_chip);
    gpio_chip = nullptr;
  }
}

void HardwareGPIO_RPI::pinMode(pin_size_t pinNumber, PinMode pinMode) {
  if (!gpio_chip) return;
  gpiod_line* line = gpio_lines[pinNumber];
  if (!line) {
    line = gpiod_chip_get_line(gpio_chip, pinNumber);
    if (!line) {
      Logger.error("HardwareGPIO_RPI", "Failed to get line");
      return;
    }
    gpio_lines[pinNumber] = line;
  }
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

int HardwareGPIO_RPI::analogRead(pin_size_t pinNumber) {
  Logger.error("HardwareGPIO_RPI",
               "analogRead not supported on Raspberry Pi GPIO");
  return 0;
}

void HardwareGPIO_RPI::analogReference(uint8_t mode) {
  m_analogReference = mode;
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

  // Set period to 20000 ns (50 kHz)
  FILE* fperiod = fopen(period_path, "w");
  if (fperiod) {
    fprintf(fperiod, "%d", 20000);
    fclose(fperiod);
  }
  // Duty cycle: value in range 0-255
  int duty = (value < 0) ? 0 : (value > 255) ? 255 : value;
  int duty_ns = (duty * 20000) / 255;
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
  Logger.error("HardwareGPIO_RPI", "tone not supported on Raspberry Pi GPIO");
}

void HardwareGPIO_RPI::noTone(uint8_t _pin) {
  Logger.error("HardwareGPIO_RPI", "noTone not supported on Raspberry Pi GPIO");
}

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

void HardwareGPIO_RPI::analogWriteFrequency(uint8_t pin, uint32_t freq) {
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

}  // namespace arduino

#endif