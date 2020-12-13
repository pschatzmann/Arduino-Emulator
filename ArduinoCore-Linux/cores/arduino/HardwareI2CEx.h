#ifndef HARDWAREI2CEX_H
#define HARDWAREI2CEX_H

#include "HardwareI2C.h"

namespace arduino {

class HardwareI2CEx : HardwareI2C {
  public:
    HardwareI2CEx(){}
    virtual ~HardwareI2CEx(){};
    virtual void begin();
    virtual void begin(uint8_t address);
    virtual void end();
    virtual void setClock(uint32_t freq);
    virtual void beginTransmission(uint8_t address);
    virtual uint8_t endTransmission(bool stopBit);
    virtual uint8_t endTransmission(void);
    virtual size_t requestFrom(uint8_t address, size_t len, bool stopBit);
    virtual size_t requestFrom(uint8_t address, size_t len);
    virtual void onReceive(void(*)(int)) = 0;
    virtual void onRequest(void(*)(void)) = 0;
};
    
}

#endif