#pragma once

#include  "../platform/platform.h"
#include  "defines.h"
#include  <vulkan/vulkan.h>
#include  <vulkan/vulkan_core.h>

typedef struct  rendererBackend rendererBackend;
typedef struct  renderPacket    renderPacket;     
typedef struct  vulkanContext   vulkanContext;

struct rendererBackend{
  platformState* platState;
  u64 frameNumber;
  u8  (*initilaize) (const char* applicationName , platformState* platState);
  u8  (*shutdown)   ();
  u8  (*resized)    ();

  u8  (*beginFrame) (f32 deltaTime);
  u8  (*endFrame)   (f32 deltaTime);
};

struct renderPacket{
  f32 deltaTime;
};

struct vulkanContext{
  VkInstance instance;
  VkAllocationCallbacks* allocator;
};


u8  rendererBackendInitialize(const char* applicationName , platformState* platState);

u8  rendererBackendBeginFrame(f32 deltaTime);

u8  rendererBackendEndFrame(f32 deltaTime);

u8  rendererBackendResized();

u8  rendererBackendShutdown();
