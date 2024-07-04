#pragma once

#include  "math/mathtypes.h"

typedef struct GlobalUniformObject GlobalUniformObject;

struct GlobalUniformObject{
  Mat4 projection;
  Mat4 view;
  Mat4 reserved0; //padded
  Mat4 reserved1; //padded
};
