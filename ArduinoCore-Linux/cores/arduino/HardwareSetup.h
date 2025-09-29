#pragma once

#include "Hardware.h"
#include "RemoteI2C.h"
#include "RemoteSPI.h"
#include "RemoteGPIO.h"
#include "WiFiUdpStream.h"
#include <exception>

//#include "RemoteI2S.h"


namespace arduino {

/**
 * Class which is used to configure the actual Hardware APIs
 */

class HardwareSetupRemoteClass {
  public:
    // as a default we use udp
    HardwareSetupRemoteClass(int port=7000){
        this->port = port;
        default_stream = new WiFiUDPStream();
        default_stream->begin(port);
    }
    
    ~HardwareSetupRemoteClass(){
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
                Logger.info( "HardwareSetup","waiting for device...");
                try {
                    // we wait for the Arduino to send us the Arduino-Emulator string
                    if (s->available()>=16){
                        char buffer[30];
                        int len = s->readBytes(buffer, 18);
                        buffer[len]=0;
                        if (strncmp(buffer, "Arduino-Emulator", 16)){
                            Logger.info( "WiFiUDPStream", "device found!");
                            break;
                        } else {
                            Logger.info( "WiFiUDPStream", "unknown command", buffer);                            
                        }
                    }    
                    delay(10000);
                } catch (const std::exception &ex) { 
                    Logger.error("WiFiUDPStream", ex.what());
                }
            }
        }

};

extern HardwareSetupRemoteClass HardwareSetup;

}

