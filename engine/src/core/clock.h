#pragma once

#include "defines.h"

// this is for calculating delta time
typedef struct Clock Clock;

struct Clock{
  f64 start;
  f64 elapsed;
};

u8  clockStart  (Clock* clock);
u8  clockUpdate (Clock* clock);
u8  clockStop   (Clock* clock);
