#pragma once

#include  "math/mathtypes.h"
#include  "resource/resourcetypes.h"

typedef struct GlobalUniformObject GlobalUniformObject;
typedef struct MaterialUniformObject  MaterialUniformObject;
typedef struct GeomteryRenderData  GeomteryRenderData;

struct GlobalUniformObject{
  Mat4 projection;        //64 bytes
  Mat4 view;
  Mat4 reserved0; //padded
  Mat4 reserved1; //padded
};

struct MaterialUniformObject{
  Vec4 diffuseColor;    // 16 bytes
  Vec4 reserved0;
  Vec4 reserved1;
  Vec4 reserved2;
};

struct GeomteryRenderData{
  Mat4 model;
  Material* material;
};
