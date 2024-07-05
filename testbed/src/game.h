#pragma once

#include  <defines.h>
#include  <gametypes.h>
#include  "math/mathtypes.h"

typedef struct GameState GameState;

struct GameState{
  f32   deltaTime;
  Mat4  view;
  Vec3  cameraPosition;
  Vec3  cameraEuler; // pitch, yaw , roll
  bool  cameraViewDirty;
};

i8    gameInitialize(Game* game);
i8    gameUpdate    (Game* game , f32 deltaTime);
i8    gameRender    (Game* game , f32 deltaTime);
void  gameOnResize  (Game* game , u32 width , u32 height);
