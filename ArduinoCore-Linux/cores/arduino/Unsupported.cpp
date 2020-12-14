#include "Common.h"
#include "ArduinoLogger.h"

void attachInterrupt(pin_size_t interruptNumber, voidFuncPtr callback, PinStatus mode){
    Logger.error("attachInterrupt","not supported");
}

void attachInterruptParam(pin_size_t interruptNumber, voidFuncPtrParam callback, PinStatus mode, void* param){
    Logger.error("attachInterruptParam","not supported");  
}

void detachInterrupt(pin_size_t interruptNumber){
    Logger.error("detachInterrupt","not supported");
}

// to compile pluggable usb
void* epBuffer(unsigned int n){
    return nullptr;
}