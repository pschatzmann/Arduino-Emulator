#pragma once
#ifdef USE_RPI
#include "HardwareGPIO.h"

namespace arduino {

class HardwareGPIO_RPI : public HardwareGPIO {
 public:
  HardwareGPIO_RPI();
  ~HardwareGPIO_RPI();
  void pinMode(pin_size_t pinNumber, PinMode pinMode) override;
  void digitalWrite(pin_size_t pinNumber, PinStatus status) override;
  PinStatus digitalRead(pin_size_t pinNumber) override;
  int analogRead(pin_size_t pinNumber) override;
  void analogReference(uint8_t mode) override;
  void analogWrite(pin_size_t pinNumber, int value) override;
  void tone(uint8_t _pin, unsigned int frequency,
            unsigned long duration = 0) override;
  void noTone(uint8_t _pin) override;
  unsigned long pulseIn(uint8_t pin, uint8_t state,
                        unsigned long timeout = 1000000L) override;
  unsigned long pulseInLong(uint8_t pin, uint8_t state,
                            unsigned long timeout = 1000000L) override;
  void analogWriteFrequency(uint8_t pin, uint32_t freq);

 private:
  int m_analogReference = 0;  // Default to AR_DEFAULT
  std::map<pin_size_t, uint32_t> gpio_frequencies;
  int pwm_pins[4] = {12, 13, 18, 19};
};

}  // namespace arduino

#endif