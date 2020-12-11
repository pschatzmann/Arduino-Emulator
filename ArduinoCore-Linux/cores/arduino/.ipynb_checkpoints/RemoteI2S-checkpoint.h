#ifndef "REMOTESI2S_H"
#define "REMOTESI2S_H"

#include "I2S.h"

namespace arduino {

/**
 * Virtual I2S which sends the data via a Stream to the final target.
 * The singe character operations are buffered.
 *
 **/
class RemoteI2S : public I2SClass {
  public:
    RemoteI2S(Stream &stream){
        service = new HardwareService(stream);
        buffer = new uint8_t[buffer_size];
        buffer_pos = 0;
    }

    ~RemoteI2S(){
        delete service;
        delete[] buffer;
    }

    // the device index and pins must map to the "COM" pads in Table 6-1 of the datasheet 
    void setup(uint8_t deviceIndex, uint8_t clockGenerator, uint8_t sdPin, uint8_t sckPin, uint8_t fsPin){
        service.send(I2sSetup);
        service.send(deviceIndex);
        service.send(clockGenerator);
        service.send(sdPin);
        service.send(sckPin);
        service.send(fsPin);
        service.flush();
    }

    // the SCK and FS pins are driven as outputs using the sample rate
    int begin(int mode, long sampleRate, int bitsPerSample){
        service.send(I2sBegin3);
        service.send(mode);
        service.send(sampleRate);
        service.send(bitsPerSample);
        service.flush();
    }

    // the SCK and FS pins are inputs, other side controls sample rate
    int begin(int mode, int bitsPerSample){
        service.send(I2sBegin2);
        service.send(mode);
        service.send(bitsPerSample);
        service.flush();
    }

    void end(){
        service.send(I2sEnd);
        service.flush();
    }

    // from Stream
    virtual int available(){
        // check buffer first
        if (buffer_pos>0 and buffer_pos<buffer_max_read){
            return buffer_max_read - buffer_pos;
        }
        service->send(I2sAvailable);  
        return service->read16();
    }

    virtual int read(){
        int result = -1;
        // if buffer has been used we re-fill it
        if (buffer_pos==buffer_max_read){
            buffer_max_read = read(buffer, buffer_size);
            buffer_pos = 0;
        }  
        // if we have some data we provide it as result
        if (buffer_pos<buffer_max_read){
            result = buffer[buffer_pos];
        }
        return result;
    }
    
    virtual int read(uint8_t *data, int len){
        service->send(I2sRead);  
        service->send(len);
        int result = service->read(data, len);
        return result;
    }
    
    virtual int peek(){
        // get result from buffer
        if (buffer_pos<buffer_max_read){
            return buffer[buffer_pos];
        }
        return service->send(I2sPeek);    
    }
    
    virtual void flush(){
        if (buffer_pos>0){
            service->send(I2sWrite);      
            service.send(size);
            service.send(buffer);
            service->flush();
            buffer_pos = 0;
        }    
        return service->send(I2sFlush);      
    }

    // from Print
    virtual size_t write(uint8_t data){
        if (buffer_pos + sizeof(data) >= buffer_len-1){
            flush();
        }
        buffer[buffer_pos] = data;
        buffer_pos++;
    }

    size_t write(int16_t data){
        if (buffer_pos + sizeof(data) >= buffer_len-1){
            flush();
        }
        int16_t *pt = &buffer[buffer_pos];
        *pt = data;
        buffer_pos+=2;
    }

    size_t write(int32_t data){
        if (buffer_pos + sizeof(data) >= buffer_len-1){
            flush();
        }
        int32_t *pt = &buffer[buffer_pos];
        *pt = data;
        buffer_pos+=4;
    }

    virtual size_t write(const uint8_t *buffer, size_t size){
        service->send(I2sWrite);      
        service.send(size);
        service.send(buffer);
        return service->receive16();   
    }

    virtual int availableForWrite(){
        service->send(I2sAvailableForWrite);      
        return service->receive16();   
    }

    int read(void* buffer, size_t size){
        service->send(I2sRead);      
        return service->receive(buffer, size);   
    }
    
    size_t write(const void *buffer, size_t size){
        service->send(I2sWrite);      
        service.send(size);
        service.send(buffer);
        return service->receive16();   
    }

    //  void onTransmit(void(*)(void));
    //  void onReceive(void(*)(void));

    void setBufferSize(int bufferSize){
        this->buffer_size = bufferSize;
        delete[] buffer;
        buffer = new uint8_t[buffer_size];
        // expand buffer on target as well
        service->send(I2sSetBufferSize);      
        service.send(bufferSize);
        service.flush();
    }
    
  protected:
      int buffer_size = 256;
      uint8_t *buffer;
      int buffer_pos;
      int buffer_max_read;
  

};
    
}

#endif