#ifndef HARDWARESERVICE_H
#define HARDWARESERVICE_H

#include "Stream.h"

namespace arduino {

/**
 * We virtualize the hardware and send the requests and replys over
 * a stream.
 *
 **/

enum HWCalls {
    I2cBegin0,
    I2cBegin1,
    I2cEnd,
    I2cSetClock,
    I2cBeginTransmission,
    I2cEndTransmission1,
    I2cEndTransmission,
    I2cRequestFrom3,
    I2cRequestFrom2,
    I2cOnReceive,
    I2cOnRequest,
    SpiTransfer,
    SpiTransfer16,
    SpiUsingInterrupt,
    SpiNotUsingInterrupt,
    SpiBeginTransaction,
    SpiEndTransaction,
    SpiAttachInterrupt,
    SpiDetachInterrupt,
    SpiBegin,
    SpiEnd,
    GpioPinMode,
    GpioDigitalWrite,
    GpioDigitalRead,
    GpioAnalogRead,
    GpioAnalogReference,
    GpioAnalogWrite,
    SerialWrite,
    SerialRead,
    SerialAvailable,
    SerialPeek,
    SerialFlush,
    I2sSetup,
    I2sBegin3,
    I2sBegin2,
    I2sEnd,
    I2sAvailable,
    I2sRead,
    I2sPeek,
    I2sFlush,
    I2sWrite,
    I2sAvailableForWrite,
    I2sSetBufferSize
};

/**
 * Stream over which we tunnel the SPI, I2C, I2S and GPIO messages
 *
 **/
    
class HardwareService {
    
  public:
    HardwareService(){
    }
    
    HardwareService(Stream &str){
        setStream(str);
    }
    
    void setStream(Stream &str){
        io = &str;
    }
    
    void send(HWCalls call){
        io->write((uint8_t*)&call,sizeof(call));
    }
    
    void send(uint8_t data){
        io->write((uint8_t*)&data,sizeof(data));
    }

    void send(uint16_t data){
        io->write((uint8_t*)&data,sizeof(data));
    }

    void send(uint32_t data){
        io->write((uint8_t*)&data,sizeof(data));
    }

    void send(int data){
        io->write((uint8_t*)&data,sizeof(data));
    }

    void send(bool data){
        io->write((uint8_t*)&data,sizeof(data));
    }

    void send(size_t data){
        io->write((uint8_t*)&data,sizeof(data));
    }

    void send(void *data, size_t len){
        io->write((uint8_t*)data, len);
    }
    
    void flush() {
        io->flush();
    }
    
    uint16_t receive16(){
        uint16_t result;
        io->readBytes((char*)&result, sizeof(uint16_t));
        return result;
    }

    uint8_t receive8(){
        uint8_t result;
        io->readBytes((char*)&result, sizeof(uint8_t));
        return result;
    }
    
    uint16_t receive(void* data, int len){
        return io->readBytes((char*)data,len);
    }
   
  protected:
    Stream *io;
    
};

}

#endif

