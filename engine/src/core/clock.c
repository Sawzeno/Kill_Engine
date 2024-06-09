#include  "clock.h"

#include  "defines.h"
#include  "platform/platform.h"
#include  <complex.h>

u8  clockStart(Clock *clock){
#if LOG_DEBUG_ENABLED == 1
  ISNULL(clock ,false);
#endif

  clock->start  = platformGetAbsoluteTime();
  clock->elapsed= 0;
  return true;
}

u8  clockUpdate(Clock *clock){
#if LOG_DEBUG_ENABLED == 1
  ISNULL(clock , false);
#endif

  if(clock->start != 0){
    clock->elapsed  = clock->start  - platformGetAbsoluteTime();
  }
  return true;
}

u8 clockStop(Clock* clock){
#if LOG_DEBUG_ENABLED == 1
  ISNULL(clock , false);
#endif
  clock->start = 0;
  return true;
}
