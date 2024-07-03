#include  "game.h"

#include  "core/kmemory.h"
#include  "core/logger.h"
#include  "core/input.h"  

//initise the function pontiers
//for setting up the game config
//and running the update method 
//will be implimented by the game

i8    gameInitialize (Game* game){
  UREPORT(TODO);
  return true;
}
static u64 prevAllocCount = 0;
i8    gameUpdate    (Game* game , f32 deltaTime){

  const u64 allocCount  = getMemoryAllocCount();
  prevAllocCount  = getMemoryAllocCount();

  testInput('M');
  if(inputIsKeyUp('M') && inputWasKeyDown('M')){
    UFATAL("allocations : %llu, (this frame : %llu)",allocCount,prevAllocCount- allocCount);
  }
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

