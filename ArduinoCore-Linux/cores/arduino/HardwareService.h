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
    I2cWrite,
    I2cAvailable,
    I2cRead,
    I2cPeek,
    SpiTransfer,
    SpiTransfer8,
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
    GpioTone,
    GpioNoTone,
    GpioPulseIn,
    GpioPulseInLong,
    SerialBegin,
    SerialEnd,
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
 * Stream over which we tunnel the SPI, I2C, I2S and GPIO messages. Since most embedded divices
 * are little endian we communicate in little endian!
 *
 **/
    
class HardwareService {
    
  public:
    HardwareService(){
    }
    
    HardwareService(void *str){
        setStream((Stream*)str);
    }
    
    void setStream(Stream *str){
        io = str;
    }
    
    void send(HWCalls call){
        uint16_t val = (uint16_t) call;
        io->write((uint8_t*)&val,sizeof(uint16_t));
    }
    
    void send(uint8_t data){
        io->write((uint8_t*)&data,sizeof(data));
    }

    void send(uint16_t dataIn){
        uint16_t data = swap_uint16(dataIn);
        io->write((uint8_t*)&data,sizeof(data));
    }

    void send(uint32_t dataIn){
        uint32_t data = swap_uint32(dataIn);
        io->write((uint8_t*)&data,sizeof(data));
    }

    void send(uint64_t dataIn){
        uint32_t data = swap_uint64(dataIn);
        io->write((uint8_t*)&data,sizeof(data));
    }

    void send(int32_t dataIn){
        int32_t data = swap_int32(dataIn);
        io->write((uint8_t*)&data,sizeof(data));

    }
    void send(int64_t dataIn){
        int32_t data = swap_int64(dataIn);
        io->write((uint8_t*)&data,sizeof(data));
    }

    void send(bool data){
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
        return swap_uint16(result);
    }

    uint32_t receive32(){
        uint16_t result;
        io->readBytes((char*)&result, sizeof(uint32_t));
        return swap_uint32(result);
    }
    
    uint64_t receive64(){
        uint16_t result;
        io->readBytes((char*)&result, sizeof(uint64_t));
        return swap_uint64(result);
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
    bool isLittleEndian = !is_big_endian();
    
    // check if the system is big endian
    bool is_big_endian(void){
        union {
            uint32_t i;
            char c[4];
        } bint = {0x01020304};

        return bint.c[0] == 1; 
    }


    //! Byte swap unsigned short
    uint16_t swap_uint16( uint16_t val ) {
        if (isLittleEndian) return val;
        return (val << 8) | (val >> 8 );
    }

    //! Byte swap short
    int16_t swap_int16( int16_t val ) {
        if (isLittleEndian) return val;
        return (val << 8) | ((val >> 8) & 0xFF);
    }

    //! Byte swap unsigned int
    uint32_t swap_uint32( uint32_t val ){
        if (isLittleEndian) return val;
        val = ((val << 8) & 0xFF00FF00 ) | ((val >> 8) & 0xFF00FF ); 
        return (val << 16) | (val >> 16);
    }

    //! Byte swap int
    int32_t swap_int32( int32_t val ){
        if (isLittleEndian) return val;
        val = ((val << 8) & 0xFF00FF00) | ((val >> 8) & 0xFF00FF ); 
        return (val << 16) | ((val >> 16) & 0xFFFF);
    }

    int64_t swap_int64( int64_t val ){
        if (isLittleEndian) return val;
        val = ((val << 8) & 0xFF00FF00FF00FF00ULL ) | ((val >> 8) & 0x00FF00FF00FF00FFULL );
        val = ((val << 16) & 0xFFFF0000FFFF0000ULL ) | ((val >> 16) & 0x0000FFFF0000FFFFULL );
        return (val << 32) | ((val >> 32) & 0xFFFFFFFFULL);
    }

    uint64_t swap_uint64( uint64_t val ){
        if (isLittleEndian) return val;
        val = ((val << 8) & 0xFF00FF00FF00FF00ULL ) | ((val >> 8) & 0x00FF00FF00FF00FFULL );
        val = ((val << 16) & 0xFFFF0000FFFF0000ULL ) | ((val >> 16) & 0x0000FFFF0000FFFFULL );
        return (val << 32) | (val >> 32);
    }
 
};

}

#endif

