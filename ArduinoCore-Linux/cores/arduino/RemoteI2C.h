#ifndef REMOTEI2C_H
#define REMOTEI2C_H

#include "HardwareI2C.h"
#include "HardwareService.h"

namespace arduino {

void(*)(int) I2ConReceive;
void(*)(void) I2ConRequest;
    

class RemoteI2C : public HardwareI2C {
  public:
    RemoteI2C(Stream &stream){
        service = new HardwareService(stream);
    }
    
    ~RemoteI2C(){
         delete service;
    }

    virtual void begin(){
        service->send(I2cBegin0);  
        service->flush();
    }
    
    virtual void begin(uint8_t address){
        service->send(I2cBegin1);    
        service->send(address);   
        service->flush();
    }
    virtual void end(){
        service->send(I2cEnd);        
        service->flush();
    }

    virtual void setClock(uint32_t freq){
        service->send(I2cSetClock);
        service->send(freq);
        service->flush();
    }
  
    virtual void beginTransmission(uint8_t address){
        service->send(I2cBeginTransmission);
        service->send(address);    
        service->flush();
    }
    
    virtual uint8_t endTransmission(bool stopBit){
        service->send(I2cEndTransmission1);
        service->send(stopBit);
        return service->receive8();
    }
    
    virtual uint8_t endTransmission(void){
        service->send(I2cEndTransmission);
        return service->receive8();
    }

    virtual uint8_t requestFrom(uint8_t address, size_t len, bool stopBit){
        service->send(I2cRequestFrom3);
        service->send(address);
        service->send(len);
        service->send(stopBit);
        return service->receive8();
    }
    
    virtual uint8_t requestFrom(uint8_t address, size_t len){
        service->send(I2cRequestFrom2);
        service->send(address);
        service->send(len);
        return service->receive8();   
    }

    virtual void onReceive(void(*)(int)cb){
        I2ConReceive = cb;
    }
    
    virtual void onRequest(void(*)(void)cb){
        I2ConRequest = cb;    
    }
    
  protected:
    HardwareService *service;            
    
};

}

#endif

