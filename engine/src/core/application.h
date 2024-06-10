#pragma once

#include  "defines.h"

typedef struct Game Game;
typedef struct  ApplicationConfig ApplicationConfig;

struct ApplicationConfig{
  i16   posX;
  i16   posY;
  i16   width;
  i16   height;
  char* name;
};

i8  applicationGetFrameBufferSize(u32* width, u32* height);
i8  applicationCreate(Game* game);
i8  applicationRun();
