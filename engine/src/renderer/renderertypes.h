#pragma once

#include  "math/mathtypes.h"
#include  "resource/resourcetypes.h"

typedef struct GlobalUniformObject GlobalUniformObject;
typedef struct LocalUniformObject  LocalUniformObject;
typedef struct GeomteryRenderData  GeomteryRenderData;

struct GlobalUniformObject{
  Mat4 projection;        //64 bytes
  Mat4 view;
  Mat4 reserved0; //padded
  Mat4 reserved1; //padded
};

struct LocalUniformObject{
  Vec4 diffuseColor;    // 16 bytes
  Vec4 reserved0;
  Vec4 reserved1;
  Vec4 reserved2;
};

struct GeomteryRenderData{
  u32 objectId;
  Mat4 model;
  Texture* textures[16];
};
