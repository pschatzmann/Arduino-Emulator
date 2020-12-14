#pragma once

namespace arduino {

// forware declarations for references
class HardwareI2CEx;
class HardwareSPI;
class HardwareGPIO;
//class I2SClass;
    
/**
 * Class which is used to represent the actual implementation which is used to drive the
 * specific interface
 **/
struct HardwareImpl {
    public:    
        HardwareGPIO *gpio;
        HardwareI2CEx *i2c;
        HardwareSPI *spi;
        //I2SClass *i2s;        
};

extern HardwareImpl Hardware;
    

}

