#pragma once

#include  "defines.h"
#include  "math/mathtypes.h"


typedef struct  RendererBackend RendererBackend;
typedef struct  RenderPacket    RenderPacket;

struct RendererBackend{
  u64 frameNumber;
 };

struct RenderPacket{
  f32 deltaTime;
};

bool  rendererBackendInitialize (u32 width, u32 height, const char* applicationName);

bool  rendererBackendBeginFrame (f32 deltaTime);

bool  rendererBackendEndFrame   (f32 deltaTime);

bool  rendererUpdateGlobalState (Mat4 projection, Mat4 view, Vec3 viewPosition, Vec4 ambientColor, i32 mode);

bool  rendererBackendResized    (u16 width, u16 height);

bool  rendererBackendShutdown   ();

bool  updateObject(Mat4 model);
