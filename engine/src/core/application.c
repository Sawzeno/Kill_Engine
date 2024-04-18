#include  "application.h"
#include  "gametypes.h"
#include  "defines.h"

#include  "platform/platform.h"
#include "renderer/rendererbackend.h"
#include  "renderer/rendererfrontend.h"

#include  "core/events.h"
#include  "core/kmemory.h"
#include  "core/events.h"
#include  "core/input.h"
#include  "core/clock.h"

#define   LIMIT_FRAMES true
#define   TARGET_FPS  240
typedef struct  applicationState  applicationState;

u8  applicationOnEvent(u16 code , void* sender , void* listener , eventContext context);
u8  applicationOnKey  (u16 code , void* sender , void* lsitener , eventContext context);

struct applicationState{
  Game* game;
  platformState  platform;
  i8    isRunning;
  i8    isSuspended;
  i16   width;
  i16   height;
  Clock clock;
  f64   lastTime;
};

static i8 initalized  = false;
static applicationState appState;

i8  applicationRun(){

  UINFO("APPLICATION STARTED SUCCESFULLY")
  UINFO(getMemoryUsage());  

  clockStart  (&appState.clock);
  clockUpdate (&appState.clock);

  //"accumulate time  ,until it exceded targetFramesPerSecond , afteer that reset frameCount"
  f64 runningTime     = 0;
  //this is to see how many frame are rendered in a second 
  u16 frameCount      = 0;
  //tagert frames per second
  f64 targetFrametime = 1.0f/TARGET_FPS;

  //time since start of the application
  f64 currentTime     = 0;
  //time elaspsed since last frame
  f64 delta           = 0;
  //time at the start of that frame 
  f64 frameStartTime  = 0;
  //time are the end  of that frame
  f64 frameEndTime    = 0;
  //time took for this frame to update and render
  f64 frameElapsedTime= 0;
  //remianing time at the end of frame
  f64 remainingTime   = 0;
  //remianing milliSeconds
  f64 remainingMs     = 0;

  while (appState.isRunning) {
    if (platformPumpMessages(&(appState.platform)) == false) {
      //flag shutdown
      appState.isRunning  = false;
    } //while game state is not suspended 
    if(appState.isSuspended == false){
      //calculate time at start of frame
      clockUpdate(&appState.clock);
      currentTime     = appState.clock.elapsed;
      delta           = currentTime - appState.lastTime;
      frameStartTime  = platformGetAbsoluteTime();



      //call the games update function
      if(appState.game->update(appState.game , (f32)delta) == false){
        UFATAL("Game Update Failed , Shutting Down!");
        appState.isRunning  = false;
        break;
      }

      //call the games render function
      if(appState.game->render(appState.game , (f32)delta) == false){
        UFATAL("Game Render Failed , Shutting Down!");
        appState.isRunning  = false;
        break;
      }

      //TEST SLAP
      renderPacket  packet;
      packet.deltaTime  = delta;
      rendererDrawFrame(&packet);

      //calculate time at end of frame
      frameEndTime      = platformGetAbsoluteTime();
      frameElapsedTime  = frameEndTime    - frameStartTime;
      runningTime      += frameElapsedTime; 
      remainingTime     = targetFrametime - frameElapsedTime;
      //give back time to OS if its remaining;
      if(remainingTime > 0 ){
        remainingMs= remainingTime * 1000;
        if(remainingMs > 0 && LIMIT_FRAMES){
          UTRACE("remainingMs : %f" , remainingMs);
          platformSleep(remainingMs - 1);
        }
        frameCount++;
      }
      // NOTE: Input update/state copying should always be handled
      // after any input should be recorded; I.E. before this line.
      // As a safety, input is the last thing to be updated before
      // this frame ends.
      inputUpdate(delta);
      appState.lastTime = currentTime;
    }
  }

  //if for some reason above loop exits without flag being set
  //imma make sure it defineltely happens here
  appState.isRunning  = false;

  eventUnregister (EVENT_CODE_APPLICATION_QUIT , NULL ,  applicationOnEvent);
  eventUnregister (EVENT_CODE_KEY_PRESSED      , NULL ,  applicationOnKey  );
  eventUnregister (EVENT_CODE_KEY_RELEASED     , NULL ,  applicationOnKey  );
  shutdownInput   ();
  shutdownEvents  ();
  shutdownRenderer();
  shutdownPlatform(&(appState.platform));
  return true;
}

i8  applicationCreate(Game* game){
  if(initalized){
    UERROR("application create called more than once");
    return false;
  }

  appState.game = game;

  UINFO("INITIALIZING ENGINE INPUT")
  initializeInput();

  appState.isRunning    = true;
  appState.isSuspended  = false;

  UINFO("INITIALIZING ENGINE EVENTS");
  if(initializeEvents() == false){
    UERROR("Event System initialization failed !!");
    return false;
  } 

  eventRegister(EVENT_CODE_APPLICATION_QUIT , NULL  , applicationOnEvent);
  eventRegister(EVENT_CODE_KEY_PRESSED      , NULL  , applicationOnKey  );
  eventRegister(EVENT_CODE_KEY_RELEASED     , NULL  , applicationOnKey  );
  UINFO("INITIALZING ENGINE WINDOWING");
  if (!startPlatform(&(appState.platform),
                     game->appConfig.name,
                     game->appConfig.posX,
                     game->appConfig.posY,
                     game->appConfig.width,
                     game->appConfig.height)) {
    UERROR("FAILED TO LAAUNCHING WINDOWING SYSTEM ON THIS PLATFORM");
    return false;
  }

  UINFO("INITIALIZING ENGINE RENDERER")
  if(initializeRenderer(game->appConfig.name,&appState.platform) == false){
    UFATAL("FAILED TO INITIALZE RENDERER")
  }
  //initialize the game 
  UINFO("INITIALZING GAME VARIABLES")
  if(appState.game->initialize(appState.game) == false){
    UFATAL("FAILED TO INITIALIZE GAME !")
    return false;
  }
  //WHY THE FUCK IS THIS IN CERATE ??
  appState.game->onResize(appState.game , appState.width , appState.height);

  initalized  = true;
  return true;
}


u8  applicationOnEvent(u16 code , void* sender, void* listener , eventContext context){
  switch (code) {
    case EVENT_CODE_APPLICATION_QUIT: {
      UINFO("EVENT_CODE_APPLICATION_QUIT recieved, shutting down.");
      appState.isRunning = false;
      return true;
    }
  }

  return false;
}

u8 applicationOnKey(u16 code , void* sender, void* listener , eventContext context){
  if(code == EVENT_CODE_KEY_PRESSED){
    u16 keyCode = context.data.u16[0];
    if(keyCode  == KEY_ESCAPE){
      eventContext  data  = {};
      eventFire(EVENT_CODE_APPLICATION_QUIT , 0 , data);
      return true;
    }else{
      UINFO("key pressed : %c ", keyCode)
    }
  }else if (code  ==  EVENT_CODE_KEY_RELEASED){
    u16 keyCode = context.data.u16[0];
    UINFO("key released  : %c" , keyCode)
  }
  return false;
}

