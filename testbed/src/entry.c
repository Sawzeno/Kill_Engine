#include  <entry.h>

#include  "game.h"
#include  "core/kmemory.h"

i8 creategame(Game* outGame){

  //Config
  outGame->appConfig.posX   = 100;
  outGame->appConfig.posY   = 100;
  outGame->appConfig.width  = 1280;
  outGame->appConfig.height = 720;
  outGame->appConfig.name   = "KILL ENGINE";
  
  //Funcrtion Pointers
  outGame->initialize = gameInitalize;
  outGame->update     = gameUpdate;
  outGame->render     = gameRender;
  outGame->onResize   = gameOnResize;

  outGame->state      = kallocate(sizeof(GameState), MEMORY_TAG_APPLICATION);
  return true;
}
