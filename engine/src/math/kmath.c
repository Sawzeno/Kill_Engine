#include "kmath.h"
#include  "math.h"
#include  "../core/platform.h"

#include "defines.h"
#include <math.h>
#include <stdlib.h>

static u8 randSeeded = false;

f32 ksin(f32 x){
  return sinf(x);
}
f32 kcos(f32 x){
  return cosf(x);
}
f32 ktan(f32 x){
  return tanf(x);
}
f32 kacos(f32 x){
  return acosf(x);
}
f32 ksqrt(f32 x){
  return sqrtf(x);
}
f32 kabs(f32 x){
  return fabsf(x);
}


i32 krandom() {
  if (!randSeeded) {
    srand((u32)platformGetAbsoluteTime());
    randSeeded = true;
  }
  return rand();
}

i32 krandomInRange(i32 min, i32 max) {
  if (!randSeeded) {
    srand((u32)platformGetAbsoluteTime());
    randSeeded = true;
  }
  return (rand() % (max - min + 1)) + min;
}

f32 fkrandom() {
  return (float)krandom() / (f32)RAND_MAX;
}


