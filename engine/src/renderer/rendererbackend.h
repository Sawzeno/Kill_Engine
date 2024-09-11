#pragma once

#include  "defines.h"
#include  "math/mathtypes.h"
#include  "renderer/renderertypes.h"
#include  "resource/resourcetypes.h"


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

b32   rendererBackendInitialize (RendererBackend* backend);

b32   rendererBackendBeginFrame (f32 deltaTime);

b32   rendererBackendEndFrame   (f32 deltaTime);

b32   rendererUpdateGlobalState (Mat4 projection, Mat4 view, Vec3 viewPosition, Vec4 ambientColor, i32 mode);

b32   rendererBackendResized    (u16 width, u16 height);

b32   rendererBackendShutdown   ();

b32   rendererUpdateObject      (GeomteryRenderData data);

b32   rendererCreateTexture     (Texture* texture, const u8* pixels);

void  rendererDestroyTexture    (Texture* texture);

b32   rendererCreateMaterial    (Material* material);

void  rendererDestroyMaterial   (Material* material);
