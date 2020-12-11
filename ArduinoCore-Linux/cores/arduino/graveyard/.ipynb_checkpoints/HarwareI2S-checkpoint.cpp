#include "I2S.h"
#include "Hardware.h"


void I2SClass::create(uint8_t deviceIndex, uint8_t clockGenerator, uint8_t sdPin, uint8_t sckPin, uint8_t fsPin) {
    return Hardware.i2s->create(deviceIndex,clockGenerator, sdPin,sckPin,fsPin);    
}

// the SCK and FS pins are driven as outputs using the sample rate
int I2SClass::begin(int mode, long sampleRate, int bitsPerSample){
    return Hardware.i2s->begin(mode, sampleRate, bitsPerSample);
}

// the SCK and FS pins are inputs, other side controls sample rate
int I2SClass::begin(int mode, int bitsPerSample){
    return Hardware.i2s->begin(mode, bitsPerSample);
}

void I2SClass::end(){
    Hardware.i2s->end();
}    

// from Stream
int I2SClass::available(){
    return Hardware.i2s->available();
}

int I2SClass::read(){
    return Hardware.i2s->read();
}

int I2SClass::peek(){
    return Hardware.i2s->peek();      
}

void I2SClass::flush(){
    Hardware.i2s->flush();      
}

int I2SClass::availableForWrite(){
    return Hardware.i2s->availableForWrite();                  
}

int I2SClass::read(void* buffer, size_t size){
    return Hardware.i2s->read(buffer, size);                  
}

size_t I2SClass::write(uint8_t value){
    return Hardware.i2s->write(value);      
}

size_t I2SClass::write(int32_t value){
    return Hardware.i2s->write(value);                              
}

size_t I2SClass::write(const uint8_t *buffer, size_t size){
    return Hardware.i2s->write(buffer, size);            
}

size_t I2SClass::write(const void *buffer, size_t size){
    return Hardware.i2s->write(buffer, size);                        
}

//  void onTransmit(void(*)(void));
//  void onReceive(void(*)(void));

void I2SClass::setBufferSize(int bufferSize){
  Hardware.i2s->setBufferSize(bufferSize);  
}