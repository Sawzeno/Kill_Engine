#pragma once

#include  "defines.h"

typedef struct Texture Texture;

struct Texture{

  void* internalData;
  u32 id;
  u32 width;
  u32 height;
  u32 channelCount;
  u32 hasTransparency;
  u32 generation;
};

// Function to print the texture information
void printTextureInfo(const struct Texture* texture, const char* str);

