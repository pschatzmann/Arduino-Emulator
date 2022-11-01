#include "Arduino.h"

/**
 * @brief arduino main - this might be needed so we generate the function if DEFINE_MAIN is defined 
 * 
 */

__weak void setup() {}
__weak void loop() {}

__weak int main () { 
    setup();
    while(true){
        loop();
    }
}	

