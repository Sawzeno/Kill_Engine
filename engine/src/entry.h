#include  "defines.h"
#include  "core/kmemory.h"
#include  "core/application.h"
#include  "gametypes.h"


extern i8 creategame(Game* game);

int main(void){

  UINFO("INITIALZING ENGINE LOGGER")
  initializeLogging();
  UINFO("INITIALZING ENGINE MEMORY")
  initializeMemory();

  //Request the game instance of the application
  UINFO("INITIALIZING GAME APPLICATION ...")
  Game game;
  if(creategame(&game) == false){
    UFATAL("Could not initialize the appliaction itself!");
    return -1;
  }

  //Ensure all function pointers exist
  if(!game.render || !game.update || !game.initialize || !game.onResize){
    UFATAL("Games's function pointers not initialized !");
    return -2;
  }

  //Initializes the config , assigns all the func pointers 
  UINFO("LAAUNCHING GAME...")
  if(applicationCreate(&game) == false){
    UINFO("Application failed to create !");
    return 1;
  }

  //contains the game loop
  if(applicationRun() ==  false){
    UINFO("Application did not quit gracefully");
    return 2;
  }

  shutdownMemory();
  shutdownLogging();
  return 0;
}
