#ifndef HARDWARESETUP_H
#define HARDWARESETUP_H

#include "Hardware.h"
#include "RemoteI2C.h"
#include "RemoteSPI.h"
#include "RemoteGPIO.h"
#include "WiFiUdpStream.h"

//#include "RemoteI2S.h"


namespace arduino {

/**
 * Class which is used to configure the actual Hardware APIs
 */

class HardwareSetupImpl {
  public:
    // as a default we use udp
    HardwareSetupImpl(int port=7000){
        this->port = port;
        default_stream = new WiFiUDPStream();
        default_stream->begin(port);
    }
    
    ~HardwareSetupImpl(){
        cleanup();
    }

    // assigns the different protocols to the stream
    void begin(Stream *s, bool doHandShake=true){
        cleanup();
        if (doHandShake){
            handShake(s);
        }
        
        if (Hardware.i2c!=nullptr) delete Hardware.i2c;
        Hardware.i2c = new RemoteI2C(s);
        if (Hardware.spi!=nullptr) delete Hardware.spi;
        Hardware.spi = new RemoteSPI(s);
        if (Hardware.gpio!=nullptr) delete Hardware.gpio;
        Hardware.gpio = new RemoteGPIO(s);
        //if (Hardware.i2s!=nullptr) delete i2s;
        //Hardware.i2s = new RemoteI2S(s);  
        
    }
    
    // start with the default udp stream. 
    void begin() {
        if (default_stream == nullptr){
            default_stream = new WiFiUDPStream();
            handShake(default_stream);
            IPAddress ip = default_stream->remoteIP();
            int port = default_stream->remotePort();
            default_stream->setTarget(ip,port);
            default_stream->write((const uint8_t*)"OK",2);
            default_stream->flush();
        }
    }
    
    void stop(){
        cleanup();
        if (Hardware.i2c!=nullptr) delete Hardware.i2c;
        if (Hardware.spi!=nullptr) delete Hardware.spi;
        if (Hardware.gpio!=nullptr) delete Hardware.gpio;
    }
    
    protected:
        WiFiUDPStream *default_stream;
        int port;
    
        void cleanup(){
            if (default_stream!=nullptr){
                delete default_stream;
                default_stream = nullptr;
            }            
        }
    
        void handShake(Stream* s) {
            while(true){
                Logger.log(Info, "HardwareSetup","waiting for device...");
                try {
                    // we wait for the Arduino to send us the Arduino-Emulator string
                    if (s->available()>=16){
                        char buffer[30];
                        int len = s->readBytes(buffer, 18);
                        buffer[len]=0;
                        if (strncmp(buffer, "Arduino-Emulator", 16)){
                            Logger.log(Info, "WiFiUDPStream", "device found!");
                            break;
                        } else {
                            Logger.log(Info, "WiFiUDPStream", "unknown command", buffer);                            
                        }
                    }    
                    delay(10000);
                } catch (const std::exception& e) { 
                    Logger.log(Error,"WiFiUDPStream", e.what());
                }
            }
        }

};

extern HardwareSetupImpl HardwareSetup;

}

#endif