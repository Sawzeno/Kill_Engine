#pragma once

#include  "defines.h"

typedef struct Game Game;
typedef struct ApplicationConfig ApplicationConfig;

struct ApplicationConfig{
  i16   posX;
  i16   posY;
  i16   width;
  i16   height;
  char* name;
};


bool  applicationRun                ();
bool  applicationCreate             (Game* game);
void  applicationShutdown           ();
void  applicationGetFrameBufferSize (u32* width, u32* height);
void  applicationGetName            (char* name);

