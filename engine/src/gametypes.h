#pragma once

#include  "core/application.h"

typedef struct Game Game;

//represents the basic game state
struct Game{
  ApplicationConfig appConfig;
  
  i8    (*initialize) (Game* game);
  i8    (*update)     (Game* game , f32 deltaTime);
  i8    (*render)     (Game* game , f32 deltaTime);
  void  (*onResize)   (Game* game , u32 width , u32 height);
  // game specific state create and managed by the game
  void* state;
  // application state
  void* applicationState;
};
