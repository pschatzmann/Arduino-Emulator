#include "Hardware.h"
#include "HardwareI2C.h"
#include "HardwareService.h"
/**
 * We suppport different implementations for the I2C
 **/

namespace arduino {


void HardwareI2C::begin() {
    Hardware.i2c->begin();
}
    
void HardwareI2C::begin(uint8_t address) {
    Hardware.i2c->begin(address);
}
    
void HardwareI2C::end(){
    Hardware.i2c->end();
}

void HardwareI2C::setClock(uint32_t freq){
    Hardware.i2c->setClock(freq);
}

void HardwareI2C::beginTransmission(uint8_t address){
    Hardware.i2c->beginTransmission(address);
}
    
uint8_t HardwareI2C::endTransmission(bool stopBit){
    return Hardware.i2c->endTransmission(stopBit);
}
    
uint8_t HardwareI2C::endTransmission(void){
    return Hardware.i2c->endTransmission();
}

uint8_t HardwareI2C::requestFrom(uint8_t address, size_t len, bool stopBit){
    return Hardware.i2c->requestFrom(address,len,stopBit);

}
    
uint8_t HardwareI2C::requestFrom(uint8_t address, size_t len){
    return Hardware.i2c->requestFrom(address,len);
}

//void HardwareI2C::onReceive(void(*)(int)onReceiveCB){
//    Hardware.i2c->onReceive(onReceiveCB);
//}
    
//void HardwareI2C::onRequest(void(*)(void)onRequestCB){
//    Hardware.i2c->onRequest(onRequestCB);
//}

}