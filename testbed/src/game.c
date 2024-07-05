#include  "game.h"

#include  "core/kmemory.h"
#include  "core/logger.h"
#include  "core/input.h"  

#include "defines.h"
#include "math/kmath.h"
#include  "renderer/rendererfrontend.h"

void recalcaulateViewMatrix(GameState* state);
void cameraYaw(GameState* state, f32 amount);
void cameraPitch(GameState* state, f32 amount);

i8    gameInitialize (Game* game){
  TRACEFUNCTION;
  GameState* state  = (GameState*)game->state;

  state->cameraPosition = (Vec3){0,0,30.0};
  state->cameraEuler    = vec3Zero();

  state->view = mat4Translation(state->cameraPosition);
  state->view = mat4Inverse(state->view);
  state->cameraViewDirty  = true;
  UINFO("GAME INITIALIZED SUCCESSEFULY");
  return true;
}

i8    gameUpdate    (Game* game , f32 deltaTime){
  GameState* state  = (GameState*)game->state;

  if(inputIsKeyDown(KEY_UP)){
    UWARN("UP");
    cameraPitch(state, 1.0f * deltaTime);
  }

  if(inputIsKeyDown(KEY_LEFT)){
    UWARN("A");
    cameraYaw(state, 1.0f * deltaTime);
  }

  if(inputIsKeyDown(KEY_DOWN)){
    UWARN("DOWN");
    cameraPitch(state, -1.0f * deltaTime);
  }

  if(inputIsKeyDown(KEY_RIGHT)){
    UWARN("D");
    cameraYaw(state, -1.0f * deltaTime);
  }


  f32 tempMoveSpeed = 10.0f;
  Vec3  velocity    = vec3Zero();
  if(inputIsKeyDown('W')){
    Vec3 forward    = mat4Forward(state->view);
    velocity        = vec3Add(velocity, forward);
  }
  if(inputIsKeyDown('S')){
    Vec3 backward   = mat4Backward(state->view);
    velocity        = vec3Add(velocity, backward);
  }
  if(inputIsKeyDown('Q')){
    Vec3 left       = mat4Left(state->view);
    velocity        = vec3Add(velocity, left);
  }
  if(inputIsKeyDown('E')){
    Vec3 right      = mat4Right(state->view);
    velocity        = vec3Add(velocity, right);
  }
  if(inputIsKeyDown(KEY_SPACE)){
    velocity.y     += 1.0f;
  }
  if(inputIsKeyDown('X')){
    velocity.y     -= 1.0f;
  }
  Vec3 z  = vec3Zero();
  if(!vec3Compare(z, velocity, 0.002f)){
    // be sure to normalize velocity befire applyong speed
    vec3Normalize(&velocity);
    state->cameraPosition.x +=  velocity.x * tempMoveSpeed * deltaTime ;
    state->cameraPosition.y +=  velocity.y * tempMoveSpeed * deltaTime ;
    state->cameraPosition.z +=  velocity.z * tempMoveSpeed * deltaTime ;
    state->cameraViewDirty   =  true;
  }
  recalcaulateViewMatrix(state);
  rendererSetView(state->view);

  return true;
}

void recalcaulateViewMatrix(GameState* state){
  if(state->cameraViewDirty){
    Mat4 rotation     = mat4EulerXYZ(state->cameraEuler.x, state->cameraEuler.y, state->cameraEuler.z);
    Mat4 translation  = mat4Translation(state->cameraPosition);

    state->view = mat4Mul(rotation, translation);
    state->view = mat4Inverse(state->view);
    state->cameraViewDirty  = false;
  }
}

void cameraYaw(GameState* state, f32 amount){
  state->cameraEuler.y  +=  amount;
  state->cameraViewDirty =  true;
}

void cameraPitch(GameState* state, f32 amount){
  TRACEFUNCTION;
  state->cameraEuler.x  +=  amount;
  f32 limit = degToRad(89.0f);
  state->cameraEuler.x  = UCLAMP(state->cameraEuler.x, -limit, limit);
  state->cameraViewDirty =  true;
}

i8    gameRender    (Game* game , f32 deltaTime){
  return true;
}

void  gameOnResize  (Game* game , u32 width , u32 height){
  UTRACE("GAME ON RESIZE CALLED");
}

