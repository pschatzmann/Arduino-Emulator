#ifndef WIFIUDPSTREAM_H
#define WIFIUDPSTREAM_H

#include "WiFiUDP.h"
#include "IPAddress.h"

namespace arduino {

/**
 * Single Target UDP Stream which sends the data on flush
 */

class WiFiUDPStream : public WiFiUDP {
  public:
      WiFiUDPStream(IPAddress targetAdress, int port){
          this->target_adress = targetAdress;
          this->port = port;
          beginPacket(targetAdress, port);
      }
      
      void flush() {
          WiFiUDP::flush();
          endPacket();
          beginPacket(target_adress, port);
      }
      
      void stop(){
          endPacket();
          WiFiUDP::stop();      
      }
  
  protected:
      IPAddress target_adress;
      int port;


};

}
#endif

