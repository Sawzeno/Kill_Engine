#include  "gametypes.h"

#include  "core/logger.h"

extern bool createGame(Game* game);

int main(void){

  //Request the game instance of the application
  Game game;
  // defined in engine/gametypes.h
  if(createGame(&game) == false){
    UFATAL("Could not initialize the appliaction itself!");
    return -1;
  }

  //Ensure all function pointers exist
  if(!game.render || !game.update || !game.initialize || !game.onResize){
    UFATAL("Games's function pointers not initialized !");
    return -2;
  }

  //Initializes the config , assigns all the func pointers 
  if(applicationCreate(&game) == false){
    UINFO("Application failed to create !");
    return 1;
  }

  if(applicationRun() ==  false){
    UFATAL("Application did not quit gracefully");
    return 2;
  }

  return 0;
}
