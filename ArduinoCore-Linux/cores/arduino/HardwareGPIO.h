#ifndef HARDWAREGPIO_H
#define HARDWAREGPIO_H

#include "Common.h"      

namespace arduino {
        
/**
 * Base class for GPIO functions
 **/
class HardwareGPIO {
  public:
    HardwareGPIO(){};
    virtual ~HardwareGPIO(){};
    virtual void pinMode(pin_size_t pinNumber, PinMode pinMode){};
    virtual void digitalWrite(pin_size_t pinNumber, PinStatus status){};
    virtual PinStatus digitalRead(pin_size_t pinNumber){return LOW;};
    virtual int analogRead(pin_size_t pinNumber){return -1;};
    virtual void analogReference(uint8_t mode){};
    virtual void analogWrite(pin_size_t pinNumber, int value){};
    virtual void tone(uint8_t _pin, unsigned int frequency, unsigned long duration = 0){};
    virtual void noTone(uint8_t _pin){};
    virtual unsigned long pulseIn(uint8_t pin, uint8_t state, unsigned long timeout = 1000000L) {return -1;};
    virtual unsigned long pulseInLong(uint8_t pin, uint8_t state, unsigned long timeout = 1000000L) {return -1;};
    
};


}

#endif