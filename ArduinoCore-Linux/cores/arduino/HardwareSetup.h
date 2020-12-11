#ifndef HARDWARESETUP_H
#define HARDWARESETUP_H

#include "Hardware.h"
#include "RemoteI2C.h"
#include "RemoteSPI.h"
#include "RemoteGPIO.h"
//#include "RemoteI2S.h"


namespace arduino {

/**
 * Class which is used to configure the actual Hardware APIs
 */

class HardwareSetupImpl {
  public:
    // assigns the different protocols to the stream
    void begin(Stream *s){
            if (Hardware.i2c!=nullptr) delete Hardware.i2c;
            Hardware.i2c = new RemoteI2C(s);
            if (Hardware.spi!=nullptr) delete Hardware.spi;
            Hardware.spi = new RemoteSPI(s);
            if (Hardware.gpio!=nullptr) delete Hardware.gpio;
            Hardware.gpio = new RemoteGPIO(s);
            //if (Hardware.i2s!=nullptr) delete i2s;
            //Hardware.i2s = new RemoteI2S(s);           
    }

};

extern HardwareSetupImpl HardwareSetup;

}

#endif