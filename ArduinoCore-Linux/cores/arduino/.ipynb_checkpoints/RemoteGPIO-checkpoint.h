#ifndef REMOTEGPIO_H
#define REMOTEGPIO_H

#include "HardwareGPIO.h"
#include "HardwareService.h"

namespace arduino {

class RemoteGPIO : public HardwareGPIO {
  public:
    RemoteGPIO(Stream &stream){
        service = new HardwareService(stream);
    }
    
    ~RemoteGPIO(){
         delete service;
    }
    
    void pinMode(pin_size_t pinNumber, PinMode pinMode){
        service->send(GpioPinMode);
        service->send(pinNumber);  
        service->send((int16_t)pinMode
        service->flush();
    }

    void digitalWrite(pin_size_t pinNumber, PinStatus status){
        service->send(GpioDigitalWrite);
        service->send(pinNumber);  
        service->send((int16_t)status);  
        service->flush();
    }

    PinStatus digitalRead(pin_size_t pinNumber){
        service->send(GpioDigitalRead);
        service->send(pinNumber);  
        return (PinStatus) service->receive16();
    }

    int analogRead(pin_size_t pinNumber){
        service->send(GpioAnalogRead);
        service->send(pinNumber);  
        return service->receive16();   
    }

    void analogReference(uint8_t mode){
        service->send(GpioAnalogReference);
        service->send(mode);    
        service->flush();
    }

    void analogWrite(pin_size_t pinNumber, int value){
        service->send(GpioAnalogWrite);
        service->send(pinNumber);
        service->send(value);    
        service->flush();
    }
  protected:
    HardwareService *service;            
    
};
                      
}   
                      
#endif
                      
              