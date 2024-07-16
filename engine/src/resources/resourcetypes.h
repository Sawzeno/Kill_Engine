#pragma once
#include  "math/mathtypes.h"

typedef struct Texture Texture;

struct Texture{
  u32   id;
  u32   width;
  u32   height;
  bool  channelCount;
  bool  hasTransparency;
  u32   generation;
  void* internalData;
};

