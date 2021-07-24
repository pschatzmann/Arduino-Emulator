#include "Arduino.h"

/**
 * @brief arduino main - this might be needed so we generate the function if DEFINE_MAIN is defined 
 * 
 */

#ifdef DEFINE_MAIN
int main () { 
    setup();
    while(true){
        loop();
    }
}	
#endif