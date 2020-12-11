#ifndef HARDWARE_H
#define HARDWARE_H

#include "Arduino.h"

namespace arduino {

// forware declarations for references
class HardwareI2C;
class HardwareSPI;
class HardwareGPIO;
class I2SClass;
      
    
/**
 * GPIO functions
 **/
class HardwareGPIO {
  public:
    virtual void pinMode(pin_size_t pinNumber, PinMode pinMode) = 0;
    virtual void digitalWrite(pin_size_t pinNumber, PinStatus status) = 0;
    virtual PinStatus digitalRead(pin_size_t pinNumber) = 0;
    virtual int analogRead(pin_size_t pinNumber) = 0;
    virtual void analogReference(uint8_t mode) = 0;
    virtual void analogWrite(pin_size_t pinNumber, int value) = 0;
};

/**
 * Class which is used to represent the actual setting
 **/
class HardwareImpl {
    public:
        HardwareI2C *i2c;
        HardwareSPI *spi;
        HardwareGPIO *gpio;
        I2SClass *i2s;
        
};

HardwareImpl Hardware;

}

#endif