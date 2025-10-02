#pragma once

#include "api/Stream.h"
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
    
    virtual void begin(unsigned long baudrate) {
        service->send(SerialBegin);
        service->send(no);
        service->send((uint64_t)baudrate);
        service->flush();
    }

    virtual void begin(unsigned long baudrate, uint16_t config) {    
        service->send(SerialBegin);
        service->send(no);
        service->send((uint64_t)baudrate);
        service->flush();
    }

    virtual void end() {
        service->send(SerialEnd);
        service->send(no);
        service->flush();
    }
    
    virtual int available(){
        if (read_buffer.available()>0){
           return read_buffer.available();   
        } 
        // otherwise we get it from the remote system
        service->send(SerialAvailable);
        service->send(no);
        service->flush();
        return service->receive16();                
    }
    
    virtual int read(){
        if (read_buffer.available()==0){
            uint8_t buffer[max_buffer_len];
            int len = readBytes(buffer, max_buffer_len); 
            read_buffer.write(buffer,len);
        }
        if (read_buffer.available()==0){
            return -1;
        }
        return read_buffer.read();      
    }
    
    virtual size_t readBytes(uint8_t *buffer, size_t length){
        if (read_buffer.available()>0){
            return read_buffer.read(buffer, length);
        }
        service->send(SerialRead);
        service->send(no);
        service->send((uint64_t)length);
        service->flush();
        int len = service->receive(buffer, length);  
        return len;
    }

    virtual int peek(){
        if (read_buffer.available()>0){
            return read_buffer.peek();
        }
        service->send(SerialPeek);
        service->flush();
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
        service->send(no);
        service->send((uint64_t)len);
        service->send(str, len);
        service->flush();
        return service->receive16();
    }
    
    void flush() {
        #if defined(_MSC_VER)
        int available;
        while((available = write_buffer.available()) > 0){
            uint8_t buffer[max_buffer_len];
            write_buffer.read(buffer, min(available, max_buffer_len));
            write(buffer, min(available, max_buffer_len));
        }
        #else
        int available = write_buffer.available();
        if (available>0){
            uint8_t buffer[available];
            write_buffer.read(buffer, available);
            write(buffer, available); 
        }
        #endif
        service->send(SerialFlush);
        service->send(no);
        service->flush();
    }

        
  protected:
    HardwareService *service;            
    uint8_t no;
    #if defined(_MSC_VER)
    static constexpr int max_buffer_len = 512; // MSVC does not support VLA
    #else
    int max_buffer_len = 512;
    #endif
    RingBufferExt write_buffer;
    RingBufferExt read_buffer;
    
};
    
    
}
