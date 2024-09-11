#pragma once

#include  "defines.h"

typedef struct Game Game;
typedef struct ApplicationConfig ApplicationConfig;

struct ApplicationConfig{
  char* name;
  i64   posX;
  i64   posY;
  u64   width;
  u64   height;
};


b32   applicationCreate             (Game* game);
b32   applicationRun                ();
void  applicationShutdown           ();
void  applicationGetFrameBufferSize (u32* width, u32* height);
void  applicationGetName            (char* name);

