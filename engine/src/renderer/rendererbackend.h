#pragma once

#include  "defines.h"
#include  "renderertypes.h"


typedef struct  RendererBackend RendererBackend;
typedef struct  RenderPacket    RenderPacket;

struct RendererBackend{
  u32   applicationWidth;
  u32   applicationHeight;
  char* applicationName;
  u64   frameNumber;
 };

struct RenderPacket{
  f32 deltaTime;
};

bool  rendererBackendInitialize (RendererBackend* backend);

bool  rendererBackendBeginFrame (f32 deltaTime);

bool  rendererBackendEndFrame   (f32 deltaTime);

bool  rendererUpdateGlobalState (Mat4 projection, Mat4 view, Vec3 viewPosition, Vec4 ambientColor, i32 mode);

bool  rendererBackendResized    (u16 width, u16 height);

bool  rendererBackendShutdown   ();

bool  updateObject(GeometryRenderData data);

bool  createTexture(const char* name,
                    bool autoRelease,
                    i32 width,
                    i32 height,
                    i32 channelCount,
                    const u8* pixels,
                    bool hasTransparency,
                    Texture* outTexture);

void  destroyTexture(Texture* texture);
