#pragma once

#include  "renderer/renderertypes.h"
#include  "defines.h"

#define DEFAULT_TEXTURE_NAME  "default"

typedef struct TextureSystemConfig TextureSystemConfig;

struct TextureSystemConfig{
  u32 maxTextureCount;
};

b8        textureSystemInitialize(u64* memReq, void* state, TextureSystemConfig config);
void      textureSystemShutdown();

Texture*  textureSystemGetDefaultTexture();

Texture*  textureSystemAcquire(const char* name, b8 autoRelease);
void      textureSystemRelease(const char* name);

