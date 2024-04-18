#pragma once

#include  "core/application.h"

typedef struct Game Game;

//represents the basic game state
struct Game{
  applicationConfig appConfig;
  
  i8    (*initialize) (Game* game);
  i8    (*update)     (Game* game , f32 deltaTime);
  i8    (*render)     (Game* game , f32 deltaTime);
  void  (*onResize)   (Game* game , u32 width , u32 height);
  void* state;
};
