#pragma once

#include  "platform/platform.h"
#include  "defines.h"
#include  <vulkan/vulkan.h>
#include  <vulkan/vulkan_core.h>

typedef struct  RendererBackend RendererBackend;
typedef struct  RenderPacket    RenderPacket;

struct RendererBackend{
  PlatformState* platState;
  u64 frameNumber;
  u8  (*initilaize) (const char* applicationName , PlatformState* platState);
  u8  (*beginFrame) (f32 deltaTime);
  u8  (*endFrame)   (f32 deltaTime);
  u8  (*resized)    (u16 width, u16 height);
  u8  (*shutdown)   ();
};

struct RenderPacket{
  f32 deltaTime;
};

u8  rendererBackendInitialize (const char* applicationName , PlatformState* platState);

u8  rendererBackendBeginFrame (f32 deltaTime);

u8  rendererBackendEndFrame   (f32 deltaTime);

u8  rendererBackendResized    (u16 width, u16 height);

u8  rendererBackendShutdown   ();
