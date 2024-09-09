#pragma once

#include "defines.h"

typedef union   Vec2  Vec2;
typedef struct  Vec3  Vec3;
typedef union   Vec4  Vec4;
typedef         Vec4  Quat;
typedef union   Mat4  Mat4;
typedef struct  Vertex3D Vertex3D;

union Mat4{
  f32 data[16];
};


union Vec2{
  // an array of x , y
  f32 elements[2];
  struct{
    union{
      //first elem
      f32 x, r, s, u;
    };
    union{
      //second elem
      f32 y, g, t, v;
    };
  };
};

struct Vec3{
  union{
    // an array of x, y, z
    f32 elements[3];
    struct{
      union{
        f32 x, r, s, u;
      };
      union{
        f32 y, g, t, v;
      };
      union{
        f32 z, b, p, w;
      };
    };
  };
};

union Vec4{
  f32 elements[4];
  union{
    struct{
      union{
        f32 x, r, s;
      };
      union{
        f32 y, g, t;
      };
      union{
        f32 z, b, p;
      };
      union{
        f32 w, a, q;
      };
    };
  };
};

struct Vertex3D{
  Vec3 position;
  Vec2 texcoord;
};
