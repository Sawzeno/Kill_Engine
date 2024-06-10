#pragma once

#include  "defines.h"

typedef struct PlatformState PlatformState;

struct PlatformState{
  void* internalState;
};

u8    startPlatform       (PlatformState* platState,
                           const char* appliactionName,
                           i32 x,
                           i32 y,
                           i32 width,
                           i32 height);

void  shutdownPlatform    (PlatformState* platState);
u8    platformPumpMessages(PlatformState* platState);
void  platformSleep       (u64 ms);
f64   platformGetAbsoluteTime();

