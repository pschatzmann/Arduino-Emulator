#include "Hardware.h"
#include "HardwareI2CEx.h"
#include "HardwareService.h"
/**
 * We suppport different implementations for the I2C
 **/

namespace arduino {


void HardwareI2CEx::begin() {
    Hardware.i2c->begin();
}
    
void HardwareI2CEx::begin(uint8_t address) {
    Hardware.i2c->begin(address);
}
    
void HardwareI2CEx::end(){
    Hardware.i2c->end();
}

void HardwareI2CEx::setClock(uint32_t freq){
    Hardware.i2c->setClock(freq);
}

void HardwareI2CEx::beginTransmission(uint8_t address){
    Hardware.i2c->beginTransmission(address);
}
    
uint8_t HardwareI2CEx::endTransmission(bool stopBit){
    return Hardware.i2c->endTransmission(stopBit);
}
    
uint8_t HardwareI2CEx::endTransmission(void){
    return Hardware.i2c->endTransmission();
}

size_t HardwareI2CEx::requestFrom(uint8_t address, size_t len, bool stopBit){
    return Hardware.i2c->requestFrom(address,len,stopBit);

}
    
size_t HardwareI2CEx::requestFrom(uint8_t address, size_t len){
    return Hardware.i2c->requestFrom(address,len);
}


void HardwareI2CEx::onReceive(void(*)(int)){
}
    
void HardwareI2CEx::onRequest(void(*)(void)){
}
        
}