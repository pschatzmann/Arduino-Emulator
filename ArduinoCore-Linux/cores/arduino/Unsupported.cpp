#include "Common.h"
#include "ArduinoLogger.h"

void attachInterrupt(pin_size_t interruptNumber, voidFuncPtr callback, PinStatus mode){
    Logger.log(Error,"attachInterrupt","not supported");
}

void attachInterruptParam(pin_size_t interruptNumber, voidFuncPtrParam callback, PinStatus mode, void* param){
    Logger.log(Error,"attachInterruptParam","not supported");  
}

void detachInterrupt(pin_size_t interruptNumber){
    Logger.log(Error,"detachInterrupt","not supported");
}

// to compile pluggable usb
void* epBuffer(unsigned int n){
    return nullptr;
}