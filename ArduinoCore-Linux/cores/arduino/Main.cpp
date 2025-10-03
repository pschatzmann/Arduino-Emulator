#include "Arduino.h"
#include "HardwareSetup.h"

/**
 * @brief arduino main - this might be needed so we generate the function if DEFINE_MAIN is defined 
 * 
 */

__attribute__((weak)) void setup() {}
__attribute__((weak)) void loop() {}

void hardwareSetup(){
#if !defined(SKIP_HARDWARE_SETUP)
#  if (defined(USE_RPI))
    RPI.begin();
#  else
#    if defined(USE_REMOTE)
        Remote.begin();
#    endif
#  endif
#endif
}

__attribute__((weak)) int main () { 
    hardwareSetup();
    setup();
    while(true){
        loop();
    }
}	

