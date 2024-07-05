#include  "clock.h"

#include  "defines.h"
#include  "platform/platform.h"

u8  clockStart(Clock *clock){
  clock->start  = platformGetAbsoluteTime();
  clock->elapsed= 0;
  return true;
}

u8  clockUpdate(Clock *clock){
  if(clock->start != 0){
    clock->elapsed  = clock->start  - platformGetAbsoluteTime();
  }
  return true;
}

u8 clockStop(Clock* clock){
  clock->start = 0;
  return true;
}
