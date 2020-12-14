#pragma once

#include "WiFiClient.h"

/**
 * @brief We provide the WiFi class to simulate the Arduino WIFI. In in Linux
 * we can expect that networking is already active. So we dont need to do anything
 * 
 */

namespace arduino {

typedef enum {
	WL_NO_SHIELD = 255,
        WL_IDLE_STATUS = 0,
        WL_NO_SSID_AVAIL,
        WL_SCAN_COMPLETED,
        WL_CONNECTED,
        WL_CONNECT_FAILED,
        WL_CONNECTION_LOST,
        WL_DISCONNECTED
} wl_status_t;
    
    
class WifiMock {
  public:
    virtual void begin(const char*name, const char* pwd){
        // nothing to be done
    }
    
    // we assume the network to be available on a desktop or host machine
    wl_status_t status() {
        return WL_CONNECTED;
    }
    
    IPAddress &localIP(){
        SocketImpl sock;
        adress.fromString(sock.getIPAddress());
        return adress;
    }
    
  protected:
    IPAddress adress;
    

};
    
extern WifiMock WiFi;

}

