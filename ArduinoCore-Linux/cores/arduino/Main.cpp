#include "Arduino.h"

/**
 * @brief arduino main - this might be needed so we generate the function if DEFINE_MAIN is defined 
 * 
 */

__attribute__((weak)) void setup() {}
__attribute__((weak)) void loop() {}

__attribute__((weak)) int main () { 
    setup();
    while(true){
        loop();
    }
}	

