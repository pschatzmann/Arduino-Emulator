#include "Hardware.h"
#include "HardwareI2CEx.h"
#include "HardwareService.h"
/**
 * We suppport different implementations for the I2C
 **/

namespace arduino {


void HardwareI2CEx::begin() {
    if (Hardware.i2c != nullptr) {
        Hardware.i2c->begin();
    }
}
    
void HardwareI2CEx::begin(uint8_t address) {
    if (Hardware.i2c != nullptr) {
        Hardware.i2c->begin(address);
    }
}
    
void HardwareI2CEx::end(){
    if (Hardware.i2c != nullptr) {
        Hardware.i2c->end();
    }
}

void HardwareI2CEx::setClock(uint32_t freq){
    if (Hardware.i2c != nullptr) {
        Hardware.i2c->setClock(freq);
    }
}

void HardwareI2CEx::beginTransmission(uint8_t address){
    if (Hardware.i2c != nullptr) {
        Hardware.i2c->beginTransmission(address);
    }
}
    
uint8_t HardwareI2CEx::endTransmission(bool stopBit){
    if (Hardware.i2c != nullptr) {
        return Hardware.i2c->endTransmission(stopBit);
    } else {
        return 0;
    }
}
    
uint8_t HardwareI2CEx::endTransmission(void){
    if (Hardware.i2c != nullptr) {
        return Hardware.i2c->endTransmission();
    } else {
        return 0;
    }
}

size_t HardwareI2CEx::requestFrom(uint8_t address, size_t len, bool stopBit){
    if (Hardware.i2c != nullptr) {
        return Hardware.i2c->requestFrom(address,len,stopBit);
    } else {
        return 0;
    }
}
    
size_t HardwareI2CEx::requestFrom(uint8_t address, size_t len){
    if (Hardware.i2c != nullptr) {
        return Hardware.i2c->requestFrom(address,len);
    } else {
        return 0;
    }
}


void HardwareI2CEx::onReceive(void(*)(int)){
}
    
void HardwareI2CEx::onRequest(void(*)(void)){
}
        
}