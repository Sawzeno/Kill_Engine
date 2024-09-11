#pragma once

#include  "defines.h"
#include  "resource/resourcetypes.h"

#define DEFAULT_TEXTURE_NAME  "default"

typedef struct TextureSystemConfig TextureSystemConfig;

struct TextureSystemConfig{
  u32 maxTextureCount;
};

b32       textureSystemInitialize(u64* memReq, void* state, TextureSystemConfig config);
void      textureSystemShutdown();

Texture*  textureSystemGetDefaultTexture();

Texture*  textureSystemAcquire(const char* name, b32 autoRelease);
void      textureSystemRelease(const char* name);

