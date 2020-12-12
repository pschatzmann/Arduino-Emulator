#ifndef WIFIUDPSTREAM_H
#define WIFIUDPSTREAM_H

#include <string.h>
#include <thread>         
#include "ArduinoAPI.h"
#include "WiFiUdp.h"
#include "IPAddress.h"
#include "ArduinoLogger.h"

namespace arduino {

/**
 * Single Target UDP Stream which sends the data on flush
 */

class WiFiUDPStream : public WiFiUDP {
  public:
      // unknown target -> we wait for a hallo until we get one
      WiFiUDPStream() {
          Logger.log(Debug,"WiFiUDPStream");
      }
    
      ~WiFiUDPStream() {
          Logger.log(Debug,"~WiFiUDPStream");
      }   
    
      // known target
      WiFiUDPStream(IPAddress targetAdress, int port){
          setTarget(targetAdress,port);
      }
      
      void flush() {
          WiFiUDP::flush();
          endPacket();
          if (!targetDefined()){
              target_adress = remoteIP();
          }
          beginPacket(target_adress, port);
      }
          
      void stop(){
          endPacket();
          WiFiUDP::stop();
          active = false;
      }
    
      bool targetDefined() {
          return ((uint32_t)target_adress)!=0;
      }
    
      void setTarget(IPAddress targetAdress, int port){
          this->target_adress = targetAdress;
          this->port = port;
          beginPacket(targetAdress, port);          
      }
    
      size_t readBytes(uint8_t* values, size_t len){
          if (this->available()==0){
            // we need to receive the next packet
            this->parsePacket();
            Logger.log(Debug,"parsePacket");  
          } else {
            Logger.log(Debug,"no parsePacket()");
          }
          size_t result = read(values, len);
          
          char msg[50];
          sprintf(msg, "->len %ld",result);
          Logger.log(Debug,msg);  
          
          
          return result;
      }

      bool isActive() {
          return active;
      }
  
  protected:
      bool active;
      IPAddress target_adress;
      int port;
    
};
    
// Define a global function which will be used to start a thread 
  

}
#endif

