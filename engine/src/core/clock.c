#include  "clock.h"

#include  "defines.h"
#include  "core/platform.h"

b32  clockStart(Clock *clock){
  clock->start  = platformGetAbsoluteTime();
  clock->elapsed= 0;
  return TRUE;
}

b32  clockUpdate(Clock *clock){
  if(clock->start != 0){
    clock->elapsed  = clock->start  - platformGetAbsoluteTime();
  }
  return TRUE;
}

b32 clockStop(Clock* clock){
  clock->start = 0;
  return TRUE;
}
