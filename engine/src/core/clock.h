#pragma once

#include "defines.h"

// this is for calculating delta time
typedef struct Clock Clock;

struct Clock{
  f64 start;
  f64 elapsed;
};

b32  clockStart  (Clock* clock);
b32  clockUpdate (Clock* clock);
b32  clockStop   (Clock* clock);
