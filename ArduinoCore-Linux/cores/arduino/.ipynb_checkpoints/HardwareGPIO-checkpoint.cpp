
#include "Common.h"
#include "Hardware.h"

/**
 * We support different implementations for GPIO
 **/
namespace arduino {

void pinMode(pin_size_t pinNumber, PinMode pinMode){
    Hardware.gpio->pinMode(pinNumber,pinMode);
}

void digitalWrite(pin_size_t pinNumber, PinStatus status){
    Hardware.gpio->digitalWrite(pinNumber, status);
}

PinStatus digitalRead(pin_size_t pinNumber){
    return Hardware.gpio->digitalRead(pinNumber);
}

int analogRead(pin_size_t pinNumber){
    return Hardware.gpio->analogRead(pinNumber);
}

void analogReference(uint8_t mode){
    Hardware.gpio->analogReference(mode);
}

void analogWrite(pin_size_t pinNumber, int value){
    Hardware.gpio->analogWrite(pinNumber, value);
}
    
}
