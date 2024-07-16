#pragma once

#include  "resources/resourcetypes.h"

typedef struct GlobalUniformObject  GlobalUniformObject;
typedef struct LocalUniformObject   LocalUniformObject;
typedef struct GeometryRenderData   GeometryRenderData;

 // this gets uploaded once per frame
struct GlobalUniformObject{
  Mat4 projection;
  Mat4 view;
  Mat4 reserved0; //padded
  Mat4 reserved1; //padded
};

// uspposed to be object specific uniform buffer object , this is what gonna be fed into a uniform bufer per object
// this gets uploaded once per object per frame
struct LocalUniformObject{
  Vec4 diffuseColor;
  Vec4 reserved0;
  Vec4 reserved1;
  Vec4 reserved2;
};

struct GeometryRenderData{
  u32 objectId;
  Mat4 model;
  Texture* textures[16];
};
