#pragma once

#include  "defines.h"

typedef struct platformState platformState;

struct platformState{
  void* internalState;
};

u8    startPlatform       (platformState* platState,
                           const char* appliactionName,
                           i32 x,
                           i32 y,
                           i32 width,
                           i32 height);

void  shutdownPlatform    (platformState* platState);
u8    platformPumpMessages(platformState* platState);
void  platformSleep       (u64 ms);
f64   platformGetAbsoluteTime();

