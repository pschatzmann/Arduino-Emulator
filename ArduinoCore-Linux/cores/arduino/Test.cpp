#include "HardwareSetup.h"
#include "HardwareService.h"
#include "WiFi.h"
#include "WiFiClient.h"
#include "WiFiUdpStream.h"
#include "Serial.h"

namespace arduino {
/**
 * Sometimes it is hard to check the header files for syntax errors.
 * We use the header files here to see any compile errors.
 */
    
class Test {
    void testHardwareSetup() {
        HardwareSetup.end();
    }
    void testWiFiUDPStream() {
        auto str = new WiFiUDPStream();
        delete str;
    }
    
#if PROVIDE_SERIALLIB    
    void testSerialImpl() {
        auto str = new SerialImpl();
        delete str;
    }
#endif
};
    
}