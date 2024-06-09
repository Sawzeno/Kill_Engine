#pragma once

#include  "platform/platform.h"
#include  "defines.h"
#include  <vulkan/vulkan.h>
#include  <vulkan/vulkan_core.h>

typedef struct  rendererBackend rendererBackend;
typedef struct  renderPacket    renderPacket;

struct rendererBackend{
  platformState* platState;
  u64 frameNumber;
  u8  (*initilaize) (const char* applicationName , platformState* platState);
  u8  (*beginFrame) (f32 deltaTime);
  u8  (*endFrame)   (f32 deltaTime);
  u8  (*resized)    (u16 width, u16 height);
  u8  (*shutdown)   ();
};

struct renderPacket{
  f32 deltaTime;
};

u8  rendererBackendInitialize (const char* applicationName , platformState* platState);

u8  rendererBackendBeginFrame (f32 deltaTime);

u8  rendererBackendEndFrame   (f32 deltaTime);

u8  rendererBackendResized    (u16 width, u16 height);

u8  rendererBackendShutdown   ();
