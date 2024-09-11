#pragma once

#include  "defines.h"

typedef struct PlatformState PlatformState;

struct PlatformState{
  void* internalState;
};


b32   startPlatform           (const char* appliactionName,i32 x,i32 y,i32 width,i32 height);
b32   platformIntialize      (u64* memoryRequirement, void* state);
b32   platformPumpMessages    ();

void  platformShutdown        ();
void  platformSleep           (u64 ms);

f64   platformGetAbsoluteTime ();
