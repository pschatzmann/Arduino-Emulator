#ifndef WIFI_H
#define WIFI_H

/**
 * @brief We provide the WiFi class to simulate the Arduino WIFI. In in Linux
 * we can expect that networking is already active. So we dont need to do anything
 * 
 */

namespace arduino {

class WifiMock {
  public:
    virtual void begin(const char*name, const char* pwd){
        // nothing to be done
    }
    
    // we assume the network to be available on a desktop or host machine
    int status() {
        return WL_CONNECTED;
    }
};

WifiMock WiFi;

}

#endif