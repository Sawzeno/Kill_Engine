#include  "game.h"

#include  <defines.h>
#include  <core/input.h>
#include  <unistd.h>

//initise the function pontiers
//for setting up the game config
//and running the update method 
//will be implimented by the game

i8    gameInitalize (Game* game){
  UREPORT(TODO);
  return true;
}
i8    gameUpdate    (Game* game , f32 deltaTime){
  UTRACE("GAME UPDATE CALLED");

  return true;
}
i8    gameRender    (Game* game , f32 deltaTime){
  UTRACE("GAME RENDER CALLED");
  return true;
}
void  gameOnResize  (Game* game , u32 width , u32 height){
  UTRACE("GAME RESIZE CALLED");
}

