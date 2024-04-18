#pragma once

#include  <defines.h>
#include  <gametypes.h>

typedef struct GameState GameState;

struct GameState{
  f32 deltaTime;
};

i8    gameInitalize (Game* game);
i8    gameUpdate    (Game* game , f32 deltaTime);
i8    gameRender    (Game* game , f32 deltaTime);
void  gameOnResize  (Game* game , u32 width , u32 height);
