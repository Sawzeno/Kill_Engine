#pragma once

#include    "defines.h"
#include    "math/mathtypes.h"

#define     TEXTURE_NAME_MAX_LEN  512 
#define     MATERIAL_NAME_MAX_LEN 512 

typedef struct Texture      Texture;
typedef struct TextureMap   TextureMap;
typedef struct Material     Material;

typedef enum{
  TEXTURE_USE_UNKNOWN     = 0,
  TEXTURE_USE_MAP_DIFFUSE = 1,
}TEXTURE_USE;

struct Texture{
  char name[TEXTURE_NAME_MAX_LEN];
  void* internalData;
  u32 id;
  u32 width;
  u32 height;
  u32 channelCount;
  u32 hasTransparency;
  u32 generation;
};

struct TextureMap{
  Texture* texture;
  TEXTURE_USE use;
};

struct Material{
  char name[MATERIAL_NAME_MAX_LEN];
  u32 id;
  u32 generation;
  u32 internalId;
  Vec4  diffuseColor;
  TextureMap diffuseMap;
};

// Function to print the texture information
void printTextureInfo(const struct Texture* texture, const char* str);

