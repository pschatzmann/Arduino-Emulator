#ifndef REMOTESERIAL_H
#define REMOTESERIAL_H

#include "Stream.h"
#include "HardwareService.h"

namespace arduino {

class RemoteSerialImpl : public Stream {
  public:
    RemoteSerialImpl(Stream &stream, int no){
        this->no = no;
        service = new HardwareService(stream);
    }
    
    virtual int available(){
        // if we use single char read via buffer
        if (len>0 && pos<len-1){
           return len-pos;   
        } 
        // otherwise we get it from the remote system
        service->send(SerialAvailable);
        service->send(no);
        return service->receive16();                
    }
    
    virtual int read(){
        if (len==pos){
            service->send(SerialRead);
            service->send(no);
            len = service->receive(buffer, 512); 
            pos = 0;
        }
        if (len==0 && pos == 0){
            return -1;
        }
        return buffer[pos++];      
    }
    
    virtual size_t readBytes(char *buffer, size_t length){
        service->send(SerialRead);
        service->send(no);
        len = service->receive(buffer, 512);  
    }

    virtual int peek(){
        service->send(SerialPeek);
        service->send(no);
        return service->receive16();
    }
        
  protected:
    HardwareService *service;            
    int no;
    char buffer[512];
    int len=0;
    int pos=0;
};
    
    
}

#endif