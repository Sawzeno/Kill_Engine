#pragma once

#include  "defines.h"
#include  "math/mathtypes.h"
#include  "renderer/renderertypes.h"
#include  "resource/resourcetypes.h"


typedef struct  RendererBackend RendererBackend;
typedef struct  RenderPacket    RenderPacket;

struct RendererBackend{
  Texture*  defaultDiffuse;
  u32   applicationWidth;
  u32   applicationHeight;
  char* applicationName;
  u64   frameNumber;
 };

struct RenderPacket{
  f32 deltaTime;
};

bool rendererBackendInitialize (RendererBackend* backend);

bool rendererBackendBeginFrame (f32 deltaTime);

bool rendererBackendEndFrame   (f32 deltaTime);

bool rendererUpdateGlobalState (Mat4 projection, Mat4 view, Vec3 viewPosition, Vec4 ambientColor, i32 mode);

bool rendererBackendResized    (u16 width, u16 height);

bool rendererBackendShutdown   ();

bool updateObject              (GeomteryRenderData data);

void rendererCreateTexture(const char* name, bool autoRelease, i32 width, u32 height, i32 channelCount, const u8* pixels, u32 hasTransparency, Texture* tex);

void rendererDestroyTexture( Texture* tex);
