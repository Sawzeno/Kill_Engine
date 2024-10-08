#pragma once

#include  "defines.h"

typedef struct EventContext EventContext;

struct EventContext{

  //128 bytes
  union{
    i64 i64[2];
    u64 u64[2];
    f64 f63[2];

    i32 i32[4];
    u32 u32[4];
    f32 f32[4];

    i16 i16[8];
    u16 u16[8];

    i8  i8[16];
    b32  u8[16];
  }data;
};

typedef enum SystemEventCode{
  //Shuts down the application on the next frame
  EVENT_CODE_APPLICATION_QUIT = 0x01,
  //Keyboard Key Pressed
  EVENT_CODE_KEY_PRESSED      = 0x02,
  //Keyboard Key Released
  EVENT_CODE_KEY_RELEASED     = 0x03,
  //Mouse Button Pressed
  EVENT_CODE_BUTTON_PRESSED   = 0x04,
  //Mouse Button Released
  EVENT_CODE_BUTTON_RELEASED  = 0x05,
  //Mouse Moved
  EVENT_CODE_MOUSE_MOVED      = 0x06,
  //Mouse Wheel Moved
  EVENT_CODE_MOUSE_WHEEL      = 0x07,
  //Change Resolution
  EVENT_CODE_RESIZED          = 0x08,
  //Max events  = 255
  EVENT_CODE_DEBUG0           = 0x10,
  EVENT_CODE_DEBUG1           = 0x11,
  EVENT_CODE_DEBUG2           = 0x12,
  EVENT_CODE_DEBUG3           = 0x13,
  EVENT_CODE_DEBUG4           = 0x14,

  MAX_EVENT_CODE              = 0xFF
}systemEventCode;

typedef b32(*pfnOnEvent)(u16 code, void* sender, void* listener, EventContext data);

b32   eventsSystemInitialize(u64* memoryRequirement, void* state);
void  eventsSystemShutdown  ();

b32   eventRegister   (u16 code, void* listener, pfnOnEvent onEvent);
b32   eventUnregister (u16 code, void* listener, pfnOnEvent onEvent);
b32   eventFire       (u16 code, void* sender  , EventContext context);

