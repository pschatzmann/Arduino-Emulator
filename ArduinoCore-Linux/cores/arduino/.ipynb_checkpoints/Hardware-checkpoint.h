#ifndef HARDWARE_H
#define HARDWARE_H

namespace arduino {

// forware declarations for references
class HardwareI2C;
class HardwareSPI;
class HardwareGPIO;
class I2SClass;
    
/**
 * Class which is used to represent the actual implementation which is used to drive the
 * specific interface
 **/
struct HardwareImpl {
    public:    
        HardwareI2C *i2c;
        HardwareSPI *spi;
        HardwareGPIO *gpio;
        I2SClass *i2s;        
};

extern HardwareImpl Hardware;

}

#endif