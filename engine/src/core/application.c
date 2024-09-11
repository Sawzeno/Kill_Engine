#include  "application.h"
#include  "kmemory.h"
#include  "logger.h"
#include  "errors.h"
#include  "gametypes.h"
#include  "platform.h"

#include  "memory/linearallocator.h"
#include  "renderer/rendererfrontend.h"

#include  "core/events.h"
#include  "core/events.h"
#include  "core/input.h"
#include  "core/clock.h"

#include "systems/materialsystem.h"
#include  "systems/texturesystem.h"

#define   LIMITFRAMES TRUE
#define   TARGETFPS  15
typedef struct  ApplicationState  ApplicationState;

b32  applicationOnEvent  (u16 code , void* sender , void* listener , EventContext context);
b32  applicationOnKey    (u16 code , void* sender , void* lsitener , EventContext context);
b32  applicationOnResize (u16 code , void* sender , void* listener , EventContext context);

struct ApplicationState{
  LinearAllocator systemsAllocator;

  Clock clock;

  b32   isRunning;
  b32   isSuspended;
  u32   width;
  u32   height;

  Game* game;

  u64   memorySystemReq;
  void* memorySystemState;

  u64   loggingSystemReq;
  void* loggingSystemState;

  u64   inputSystemReq;
  void* inputSystemState;

  u64   eventSystemReq;
  void* eventSystemState;

  u64   platformSystemReq;
  void* platformSystemState;

  u64   rendererSystemReq;
  void* rendererSystemState;

  u64   textureSystemReq;
  void* textureSystemState;

  u64   materialSystemReq;
  void* materialSystemState;

  f64   lastTime;
};

static b32 initalized  = FALSE;
static ApplicationState* appState;

b32  applicationCreate(Game* game){
  UINFO("CREATING APPLICATION");
  if(game->applicationState){
    UERROR("application create called more than once");
    return FALSE;
  }

  game->applicationState= calloc(1, sizeof(ApplicationState));
  if(game->applicationState ==  NULL){
    UFATAL("COULD NOT ALLOCATE MEMORY FOR game.applicationState in applicationCreate");
    return FALSE;
  }
  appState  = game->applicationState;
  appState->game = game;
  appState->isSuspended  = FALSE;
  appState->width        = game->appConfig.width;
  appState->height       = game->appConfig.height;

  //---------------------------------STARTUP CODE---------------------------------
  //requires calloc as other memory functions use the logging subsytem
  //so initialize the memory and logging system with calloc
  UINFO("INITIALZING APPLICATION MEMORY");
  memorySystemInitialize(&appState->memorySystemReq, NULL);

  appState->memorySystemState = calloc(1, appState->memorySystemReq);
  ISNULL(appState->memorySystemState, FALSE);
  if(!memorySystemInitialize(&appState->memorySystemReq, appState->memorySystemState)){
    UFATAL("failed to initialize memory subsytem, shutting down");
  }

  UINFO("INITIALZING ENGINE LOGGER");
  loggingSystemInitialize(&appState->loggingSystemReq, NULL);
  appState->loggingSystemState= calloc(1, appState->loggingSystemReq);

  ISNULL(appState->loggingSystemState, FALSE);
  if(!loggingSystemInitialize(&appState->loggingSystemReq, appState->loggingSystemState)){
    UFATAL("failed to initialze logging system, shutting down");
    return FALSE;
  }

  
  //-------------------------------------------------------------------------------

  KINFO("ALLOCATING MEMMORY FOR SUBSYSTEMS");
  u64 systemsAllocatorTotalSize = 1024 * 1024 * 24;
  linearAllocatorCreate(systemsAllocatorTotalSize, NULL, &appState->systemsAllocator);

  KINFO("INITIALIZING ENGINE INPUT");
  inputSystemInitialize(&appState->inputSystemReq, NULL);
  appState->inputSystemState  = linearAllocatorAllocate(&appState->systemsAllocator, appState->inputSystemReq);
  if(!inputSystemInitialize(&appState->inputSystemReq, appState->inputSystemState)){
    KFATAL("failed to initialize input sysytem, shutting down");
    return FALSE;
  }

  KINFO("INITIALIZING ENGINE EVENTS");
  eventsSystemInitialize(&appState->eventSystemReq, NULL);
  appState->eventSystemState  = linearAllocatorAllocate(&appState->systemsAllocator, appState->eventSystemReq);
  if(!eventsSystemInitialize(&appState->eventSystemReq, appState->eventSystemState)){
    KFATAL("failed to initialize input subsystem, shutting down");
    return FALSE;
  }

  eventRegister(EVENT_CODE_APPLICATION_QUIT , NULL  , applicationOnEvent);
  eventRegister(EVENT_CODE_KEY_PRESSED      , NULL  , applicationOnKey  );
  eventRegister(EVENT_CODE_KEY_RELEASED     , NULL  , applicationOnKey  );
  eventRegister(EVENT_CODE_RESIZED          , NULL  , applicationOnResize);

  KINFO("INITIALZING ENGINE PLATFORM");
  platformIntialize(&appState->platformSystemReq, NULL);
  appState->platformSystemState = linearAllocatorAllocate(&appState->systemsAllocator, appState->platformSystemReq);
  if(!platformIntialize(&appState->platformSystemReq, appState->platformSystemState)){
    KFATAL("failed to initialize platform subsystem, shutting down!");
  }
  if (!startPlatform(game->appConfig.name,
                     game->appConfig.posX,
                     game->appConfig.posY,
                     game->appConfig.width,
                     game->appConfig.height)) {
    KERROR("FAILED TO SET WINDOWING SYSTEM ON THIS PLATFORM");
    return FALSE;
  }
  

  KINFO("INITIALIZING ENGINE RENDERER");
  rendererInitalize(&appState->rendererSystemReq, NULL);
  appState->rendererSystemState = linearAllocatorAllocate(&appState->systemsAllocator, appState->rendererSystemReq);
  if(!rendererInitalize(&appState->rendererSystemReq, appState->rendererSystemState)){
    KFATAL("failed to initialize renderer subsystem, shutting down!");
    return FALSE;
  }

  KINFO("INITIALIZING ENGINE TEXTURE SYSTEM");

  appState->textureSystemReq  = 0;
  TextureSystemConfig textureSysConfig = {.maxTextureCount  = 64};
  textureSystemInitialize(&appState->textureSystemReq, NULL, textureSysConfig);
  appState->textureSystemState  = linearAllocatorAllocate(&appState->systemsAllocator, appState->textureSystemReq);
  if(!textureSystemInitialize(&appState->textureSystemReq, appState->textureSystemState, textureSysConfig))
  {
    KFATAL("FAILED TO INITIALIZE ENGINE TEXTURE SYSTEM");
    return FALSE;
  }

  appState->materialSystemReq = 0;
  MaterialSystemConfig materialSysConfig  = {.maxMaterialCount  = 64};
  materialSystemInitailize(&appState->materialSystemReq, NULL, materialSysConfig);
  appState->materialSystemState  = linearAllocatorAllocate(&appState->systemsAllocator, appState->materialSystemReq);
  if(!materialSystemInitailize(&appState->materialSystemReq, appState->materialSystemState, materialSysConfig))
  {
    KFATAL("FAILED TO INITIALIZE ENGINE MATERIAL SYSTEM");
    return FALSE;
  }

  KINFO("INITIALZING GAME VARIABLES");
  if(appState->game->initialize(appState->game) == FALSE){
    KFATAL("FAILED TO INITIALIZE GAME !");
    return FALSE;
  }
    
  initErrors();
  UINFO("GAME ENGINE INITIALIZED");
  initalized  = TRUE;
  return TRUE;
}

b32  applicationRun(){
  TRACEFUNCTION;
  KINFO("APPLICATION STARTED SUCCESSFULY");
  appState->isRunning    = TRUE;
  // Uwrite(1024, getMemoryUsage());  

  clockStart  (&appState->clock);
  clockUpdate (&appState->clock);

  //"accumulate time  ,until it exceded targetFramesPerSecond , afteer that reset frameCount"
  f64 runningTime     = 0;
  //this is to see how many frame are rendered in a second 
  u16 frameCount      = 0;
  //tagert frames per second
  f64 targetFrametime = 1.0f/TARGETFPS;

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

  while (appState->isRunning) {
    if (platformPumpMessages() == FALSE) {
      //flag shutdown
      appState->isRunning  = FALSE;
    } //while game state is not suspended 
    if(appState->isSuspended == FALSE){
      //calculate time at start of frame
      clockUpdate(&appState->clock);
      currentTime     = appState->clock.elapsed;
      delta           = currentTime - appState->lastTime;
      frameStartTime  = platformGetAbsoluteTime();

      //call the games update function
      if(appState->game->update(appState->game , (f32)delta) == FALSE){
        UFATAL("Game Update Failed , Shutting Down!");
        KFATAL("Game Update Failed , Shutting Down!");
        appState->isRunning  = FALSE;
        break;
      }
      //call the games render function
      if(appState->game->render(appState->game , (f32)delta) == FALSE){
        UFATAL("Game Render Failed , Shutting Down!");
        KFATAL("Game Render Failed , Shutting Down!");
        appState->isRunning  = FALSE;
        break;
      }
      //TEST SLAP
      RenderPacket  packet;
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
        if(remainingMs > 0 && LIMITFRAMES){
          platformSleep(remainingMs - 1);
        }
        frameCount++;
      }
      // NOTE: Input update/state copying should always be handled
      // after any input should be recorded; I.E. before this line.
      // As a safety, input is the last thing to be updated before
      // this frame ends.
      inputUpdate(delta);
      appState->lastTime = currentTime;
    }
  }

  //if for some reason above loop exits without flag being set
  //imma make sure it defineltely happens here
  appState->isRunning  = FALSE;

  UINFO("UNREGISTERING EVENTS");
  eventUnregister (EVENT_CODE_APPLICATION_QUIT , NULL ,  applicationOnEvent);
  eventUnregister (EVENT_CODE_KEY_PRESSED      , NULL ,  applicationOnKey  );
  eventUnregister (EVENT_CODE_KEY_RELEASED     , NULL ,  applicationOnKey  );
  eventUnregister (EVENT_CODE_RESIZED          , NULL ,  applicationOnResize);

  applicationShutdown();
  return TRUE;
}

void applicationShutdown(){
  UINFO("SHUTING DOWN SUBSYSTEMS");

  //-----------------render---------------
  textureSystemShutdown();
  materialSystemShutdown();
  rendererShutdown();

  //-----------------platform---------------
  inputSystemShutdown   ();
  eventsSystemShutdown  ();
  platformShutdown();

  //-----------------idk---------------
  memorySystemShutdown  ();
  loggingSystemShutdown ();
}

b32  applicationOnEvent(u16 code , void* sender, void* listener , EventContext context){
  switch (code) {
    case EVENT_CODE_APPLICATION_QUIT: {
      UTRACE("EVENT_CODEAPPLICATIONQUIT recieved, shutting down.");
      KTRACE("EVENT_CODEAPPLICATIONQUIT recieved, shutting down.");
      appState->isRunning = FALSE;
      return TRUE;
    }
  }

  return FALSE;
}

b32 applicationOnKey(u16 code , void* sender, void* listener , EventContext context){
  TRACEEVENT;
  if(code == EVENT_CODE_KEY_PRESSED){
    u16 keyCode = context.data.u16[0];
    if(keyCode  == KEY_ESCAPE){
      EventContext  data  = {};
      EINFO("APPLICATION QUIT EVENT");
      eventFire(EVENT_CODE_APPLICATION_QUIT , 0 , data);
      return TRUE;
    }
  }else if (code  ==  EVENT_CODE_KEY_RELEASED){
    u16 keyCode = context.data.u16[0];
  }
  return FALSE;
}


b32  applicationOnResize  (u16 code , void* sender , void* listener , EventContext context){
  TRACEEVENT;
  KDEBUG("code  : %"PRIu16" sender : %p listener : %p context : %p");
  u16 width = context.data.u16[0];
  u16 height = context.data.u16[1];


  if (code == EVENT_CODE_RESIZED) {
    u16 width = context.data.u16[0];
    u16 height = context.data.u16[1];

    // Check if different. If so, trigger a resize event.
    if (width != appState->width || height != appState->height) {
      appState->width = width;
      appState->height = height;

      UWARN("Window resized to: %i, %i", width, height);

      // Handle minimization
      if (width == 0 || height == 0) {
        UWARN("Window minimized, suspending application.");
        appState->isSuspended = TRUE;
        return TRUE;
      } else {
        if (appState->isSuspended) {
          UWARN("Window restored, resuming application.");
          appState->isSuspended = FALSE;
        }
        rendererResize(width, height);
        appState->game->onResize(appState->game, width, height);
      }
    }
  }    
  // Event purposely not handled to allow other listeners to get this.
  return FALSE;
}

void  applicationGetFrameBufferSize(u32* width, u32* height){
  *width  = appState->width;
  *height = appState->height;
}

void  applicationGetName            (char* name){
  name  = appState->game->appConfig.name;
}

