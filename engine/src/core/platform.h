#pragma once

#include  "defines.h"

typedef struct PlatformState PlatformState;

struct PlatformState{
  void* internalState;
};


u8    startPlatform       (const char* appliactionName,
                           i32 x,
                           i32 y,
                           i32 width,
                           i32 height);

bool  initializePlatform      (u64* memoryRequirement, void* state);
void  shutdownPlatform        ();
u8    platformPumpMessages    ();
void  platformSleep           (u64 ms);
f64   platformGetAbsoluteTime ();

void platformKeyRepeatOff();
