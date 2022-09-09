#include "Common.h"
extern "C" {
  #include "stdlib.h"
}

/* C++ prototypes */
long map(long x, long in_min, long in_max, long out_min, long out_max)
{
  return (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
}

// void randomSeed(unsigned long seed)
// {
//   if (seed != 0) {
//     srandom(seed);
//   }
// }

// long random(long howbig)
// {
//   if (howbig == 0) {
//     return 0;
//   }
//   return random() % howbig;
// }

// long random(long howsmall, long howbig)
// {
//   if (howsmall >= howbig) {
//     return howsmall;
//   }
//   long diff = howbig - howsmall;
//   return random(diff) + howsmall;
// }


uint16_t makeWord(uint16_t w) { return w; }
uint16_t makeWord(uint8_t h, uint8_t l) { return (h << 8) | l; }