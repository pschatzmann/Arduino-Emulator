#ifndef REMOTESPI_H
#define REMOTESPI_H

#include "HardwareSPI.h"

/**
 * We virtualize the hardware and send the requests and replys over
 * a stream.
 *
 **/

namespace arduino {

class RemoteSPI : public HardwareSPI{
  public:
    RemoteSPI(Stream *stream){
        service = new HardwareService(stream);
    }
    
    ~RemoteSPI(){
         delete service;
    }

    uint8_t transfer(uint8_t data) {
        service->send(SpiTransfer8);    
        service->send(data);   
        service->flush();
        return service->receive8();    
    }

    uint16_t transfer16(uint16_t data) {
        service->send(SpiTransfer16);    
        service->send(data);   
        service->flush();
        return service->receive16();    
    }

    void transfer(void *buf, size_t count) {
        service->send(SpiTransfer);    
        service->send((uint32_t)count);   
        service->send(buf, count);   
        service->flush();
        service->receive16();    
    }

    void usingInterrupt(int interruptNumber) {
        service->send(SpiUsingInterrupt);    
        service->send(interruptNumber);       
        service->flush();
    }

    void notUsingInterrupt(int interruptNumber){
        service->send(SpiNotUsingInterrupt);    
        service->send(interruptNumber);           
        service->flush();
    }

    void beginTransaction(SPISettings settings){
        service->send(SpiBeginTransaction);    
        //uint32_t clock, uint8_t bitOrder, uint8_t dataMode
        service->send((uint32_t)settings.getClockFreq());    
        service->send((uint8_t)settings.getBitOrder());    
        service->send((uint8_t)settings.getDataMode());        
        service->flush();
    }

    void endTransaction(void){
        service->send(SpiEndTransaction);        
        service->flush();
    }

    void attachInterrupt() {
        service->send(SpiAttachInterrupt);            
        service->flush();
    }

    void detachInterrupt(){
        service->send(SpiDetachInterrupt);                
        service->flush();
    }

    void begin() {
        service->send(SpiBegin);                    
        service->flush();
    }

    void end() {
        service->send(SpiEnd);                        
        service->flush();
    }

  protected:
    HardwareService *service;            

};
    
}

#endif