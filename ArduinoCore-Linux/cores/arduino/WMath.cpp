// WMath.cpp
#include "api/Common.h"
extern "C" {
  #include <stdlib.h>
}

long random(long howbig) {
  if (howbig == 0) {
    return 0;
  }
  return (long)(rand() % howbig);
}

long random(long howsmall, long howbig) {
  if (howsmall >= howbig) {
    return howsmall;
  }
  long diff = howbig - howsmall;
  return random(diff) + howsmall;
}

void randomSeed(unsigned long seed) {
  if (seed != 0) {
    srand(seed);
  }
}
