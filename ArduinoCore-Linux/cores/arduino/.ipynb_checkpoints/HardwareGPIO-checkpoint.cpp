
#include "Common.h"
#include "Hardware.h"
#include "HardwareGPIO.h"
#include "HardwareService.h"

/**
 * We support different implementations for GPIO. These are the public methods needed by
 * Common.h
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
    

// Generates a square wave of the specified frequency (and 50% duty cycle) on a pin
void tone(uint8_t pinNumber, unsigned int frequency, unsigned long duration = 0){
    Hardware.gpio->tone(pinNumber, frequency, duration);
    
}

// Stops the generation of a square wave triggered by tone() 
void noTone(uint8_t pinNumber){
    Hardware.gpio->noTone(pinNumber);    
}
    
/// Reads a pulse (either HIGH or LOW) on a pin
unsigned long pulseIn(uint8_t pinNumber, uint8_t state, unsigned long timeout = 1000000L){
    return Hardware.gpio->pulseIn(pinNumber, state, timeout);    
    
}
    
/// Reads a pulse (either HIGH or LOW) on a pin
unsigned long pulseInLong(uint8_t pinNumber, uint8_t state, unsigned long timeout = 1000000L){
    return Hardware.gpio->pulseInLong(pinNumber, state, timeout);        
}
    
    
}
