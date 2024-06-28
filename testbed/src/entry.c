#include  <entry.h>

#include  "game.h"
#include  "core/logger.h"

i8 createGame(Game* outGame){

  UINFO("INITIALIZING GAME APPLICATION ...");
  //Config
  outGame->appConfig.posX   = 100;
  outGame->appConfig.posY   = 100;
  outGame->appConfig.width  = 1280;
  outGame->appConfig.height = 1440;
  outGame->appConfig.name   = "KILL ENGINE";
  
  //Funcrtion Pointers
  outGame->initialize = gameInitialize;
  outGame->update     = gameUpdate;
  outGame->render     = gameRender;
  outGame->onResize   = gameOnResize;

  outGame->state      = calloc(1, sizeof(GameState));
  MEMERR(outGame->state);
  outGame->applicationState = NULL;
  return true;
}
