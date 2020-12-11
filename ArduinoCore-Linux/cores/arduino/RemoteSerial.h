#ifndef REMOTESERIAL_H
#define REMOTESERIAL_H

#include "Stream.h"
#include "HardwareService.h"
#include "RingBufferExt.h"

namespace arduino {

/**
 * RemoteSerialImpl shim that privodes access to a Stream on a 
 * remote device. Only Single character operations are buffered.
 *
 **/
    
class RemoteSerialImpl : public Stream {
  public:
    RemoteSerialImpl(Stream &stream, uint8_t no){
        this->no = no;
        service = new HardwareService(&stream);
    }
    
    virtual int available(){
        if (read_buffer.available()>0){
           return read_buffer.available();   
        } 
        // otherwise we get it from the remote system
        service->send(SerialAvailable);
        service->send(no);
        return service->receive16();                
    }
    
    virtual int read(){
        if (read_buffer.available()==0){
            service->send(SerialRead);
            service->send(no);
            uint8_t buffer[max_buffer_len];
            int len = service->receive(buffer, max_buffer_len); 
            read_buffer.write(buffer,len);
        }
        if (read_buffer.available()==0){
            return -1;
        }
        return read_buffer.read();      
    }
    
    virtual size_t readBytes(char *buffer, size_t length){
        if (read_buffer.available()>0){
            return read_buffer.read((uint8_t*)buffer, length);
        }
        service->send(SerialRead);
        service->send(no);
        int len = service->receive(buffer, 512);  
        return len;
    }

    virtual int peek(){
        if (read_buffer.available()>0){
            return read_buffer.peek();
        }
        service->send(SerialPeek);
        service->send(no);
        return service->receive16();
    }
    
    virtual size_t write(uint8_t c){
        if (write_buffer.availableToWrite()==0){
            flush();
        }
        return write_buffer.write(c);
    }
    
    virtual size_t write(uint8_t *str, size_t len){
        flush();
        service->send(SerialWrite);
        service->send(len);
        service->send(str, len);
        return service->receive16();
    }
    
    void flush() {
        int available = write_buffer.available();
        if (available>0){
            uint8_t buffer[available];
            write_buffer.read(buffer, available);
            write(buffer, available); 
        }
        service->send(SerialFlush);
        service->flush();

    }

        
  protected:
    HardwareService *service;            
    uint8_t no;
    int max_buffer_len = 512;
    RingBufferExt write_buffer;
    RingBufferExt read_buffer;
    
};
    
    
}

#endif