#include  "application.h"
#include  "core/kmemory.h"
#include  "core/logger.h"
#include  "core/errors.h"
#include  "memory/linearallocator.h"
#include  "gametypes.h"

#include  "platform/platform.h"
#include  "renderer/rendererfrontend.h"

#include  "core/events.h"
#include  "core/events.h"
#include  "core/input.h"
#include  "core/clock.h"

#define   LIMITFRAMES false
#define   TARGETFPS  60
typedef struct  ApplicationState  ApplicationState;

u8  applicationOnEvent  (u16 code , void* sender , void* listener , EventContext context);
u8  applicationOnKey    (u16 code , void* sender , void* lsitener , EventContext context);
u8  applicationOnResize (u16 code , void* sender , void* listener , EventContext context);

struct ApplicationState{
  Game* game;
  bool  isRunning;
  bool  isSuspended;
  i16   width;
  i16   height;
  Clock clock;
  f64   lastTime;

  LinearAllocator systemsAllocator;

  u64   memorySystemMemoryRequirement;
  void* memorySystemState;

  u64   loggingSystemMemoryRequirement;
  void* loggingSystemState;

  u64   inputSystemMemoryRequirement;
  void* inputSystemState;

  u64   eventSystemMemoryRequirement;
  void* eventSystemState;

  u64   platformSystemMenoryRequirement;
  void* platformSystemState;

  u64   rendererSystemMemoryRequirement;
  void* rendererSystemState;
};

static bool initalized  = false;
static ApplicationState* appState;

bool  applicationCreate(Game* game){
  UINFO("CREATING APPLICATION");
  if(game->applicationState){
    UERROR("application create called more than once");
    return false;
  }

  game->applicationState= calloc(1, sizeof(ApplicationState));
  if(game->applicationState ==  NULL){
    UFATAL("COULD NOT ALLOCATE MEMORY FOR game.applicationState in applicationCreate");
    return false;
  }
  appState  = game->applicationState;
  appState->game = game;
  appState->isSuspended  = false;
  appState->width        = game->appConfig.width;
  appState->height       = game->appConfig.height;

  //---------------------------------STARTUP CODE---------------------------------
  //requires calloc as other memory functions use the logging subsytem
  //so initialize the memory and logging system with calloc
  UINFO("INITIALZING APPLICATION MEMORY");
  initializeMemory(&appState->memorySystemMemoryRequirement, NULL);

  appState->memorySystemState = calloc(1, appState->memorySystemMemoryRequirement);
  ISNULL(appState->memorySystemState, false);
  if(!initializeMemory(&appState->memorySystemMemoryRequirement, appState->memorySystemState)){
    UFATAL("failed to initialize memory subsytem, shutting down");
  }

  UINFO("INITIALZING ENGINE LOGGER");
  initializeLogging(&appState->loggingSystemMemoryRequirement, NULL);
  appState->loggingSystemState= calloc(1, appState->loggingSystemMemoryRequirement);

  ISNULL(appState->loggingSystemState, false);
  if(!initializeLogging(&appState->loggingSystemMemoryRequirement, appState->loggingSystemState)){
    UFATAL("failed to initialze logging system, shutting down");
    return false;
  }

  
  //-------------------------------------------------------------------------------

  KINFO("ALLOCATING MEMMORY FOR SUBSYSTEMS");
  u64 systemsAllocatorTotalSize = 1024 * 1024;
  linearAllocatorCreate(systemsAllocatorTotalSize, NULL, &appState->systemsAllocator);

  KINFO("INITIALIZING ENGINE INPUT");
  initializeInput(&appState->inputSystemMemoryRequirement, NULL);
  appState->inputSystemState  = linearAllocatorAllocate(&appState->systemsAllocator, appState->inputSystemMemoryRequirement);
  if(!initializeInput(&appState->inputSystemMemoryRequirement, appState->inputSystemState)){
    KFATAL("failed to initialize input sysytem, shutting down");
    return false;
  }

  KINFO("INITIALIZING ENGINE EVENTS");
  initializeEvents(&appState->eventSystemMemoryRequirement, NULL);
  appState->eventSystemState  = linearAllocatorAllocate(&appState->systemsAllocator, appState->eventSystemMemoryRequirement);
  if(!initializeEvents(&appState->eventSystemMemoryRequirement, appState->eventSystemState)){
    KFATAL("failed to initialize input subsystem, shutting down");
    return false;
  }

  eventRegister(EVENT_CODE_APPLICATION_QUIT , NULL  , applicationOnEvent);
  eventRegister(EVENT_CODE_KEY_PRESSED      , NULL  , applicationOnKey  );
  eventRegister(EVENT_CODE_KEY_RELEASED     , NULL  , applicationOnKey  );
  eventRegister(EVENT_CODE_RESIZED          , NULL  , applicationOnResize);

  KINFO("INITIALZING ENGINE PLATFORM");
  initializePlatform(&appState->platformSystemMenoryRequirement, NULL);
  appState->platformSystemState = linearAllocatorAllocate(&appState->systemsAllocator, appState->platformSystemMenoryRequirement);
  if(!initializePlatform(&appState->platformSystemMenoryRequirement, appState->platformSystemState)){
    KFATAL("failed to initialize platform subsystem, shutting down!");
  }
  if (!startPlatform(game->appConfig.name,
                     game->appConfig.posX,
                     game->appConfig.posY,
                     game->appConfig.width,
                     game->appConfig.height)) {
    KERROR("FAILED TO SET WINDOWING SYSTEM ON THIS PLATFORM");
    return false;
  }

  KINFO("INITIALIZING ENGINE RENDERER");
  initializeRenderer(&appState->rendererSystemMemoryRequirement, NULL);
  appState->rendererSystemState = linearAllocatorAllocate(&appState->systemsAllocator, appState->rendererSystemMemoryRequirement);
  if(!initializeRenderer(&appState->rendererSystemMemoryRequirement, appState->rendererSystemState)){
    KFATAL("failed to initialize renderer subsystem, shutting down!");
    return false;
  }

  //initialize the game 
  KINFO("INITIALZING GAME VARIABLES");
  if(appState->game->initialize(appState->game) == false){
    KFATAL("FAILED TO INITIALIZE GAME !");
    return false;
  }
    
  initErrors();
  UINFO("GAME ENGINE INITIALIZED");
  initalized  = true;
  return true;
}

bool  applicationRun(){
  TRACEFUNCTION;
  KINFO("APPLICATION STARTED SUCCESSFULY");
  appState->isRunning    = true;
  Uwrite(1024, getMemoryUsage());  

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
    if (platformPumpMessages() == false) {
      //flag shutdown
      appState->isRunning  = false;
    } //while game state is not suspended 
    if(appState->isSuspended == false){
      //calculate time at start of frame
      clockUpdate(&appState->clock);
      currentTime     = appState->clock.elapsed;
      delta           = currentTime - appState->lastTime;
      frameStartTime  = platformGetAbsoluteTime();

      //call the games update function
      if(appState->game->update(appState->game , (f32)delta) == false){
        UFATAL("Game Update Failed , Shutting Down!");
        KFATAL("Game Update Failed , Shutting Down!");
        appState->isRunning  = false;
        break;
      }
      //call the games render function
      if(appState->game->render(appState->game , (f32)delta) == false){
        UFATAL("Game Render Failed , Shutting Down!");
        KFATAL("Game Render Failed , Shutting Down!");
        appState->isRunning  = false;
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
      appState->lastTime = currentTime;
    }
  }

  //if for some reason above loop exits without flag being set
  //imma make sure it defineltely happens here
  appState->isRunning  = false;

  UINFO("UNREGISTERING EVENTS");
  eventUnregister (EVENT_CODE_APPLICATION_QUIT , NULL ,  applicationOnEvent);
  eventUnregister (EVENT_CODE_KEY_PRESSED      , NULL ,  applicationOnKey  );
  eventUnregister (EVENT_CODE_KEY_RELEASED     , NULL ,  applicationOnKey  );
  eventUnregister (EVENT_CODE_RESIZED          , NULL ,  applicationOnResize);

  applicationShutdown();
  return true;
}

void applicationShutdown(){
  UINFO("SHUTING DOWN SUBSYSTEMS");
  shutdownRenderer();
  shutdownInput   ();
  shutdownEvents  ();
  shutdownPlatform();
  shutdownMemory  ();
  shutdownLogging ();
}

u8  applicationOnEvent(u16 code , void* sender, void* listener , EventContext context){
  switch (code) {
    case EVENT_CODE_APPLICATION_QUIT: {
      UTRACE("EVENT_CODEAPPLICATIONQUIT recieved, shutting down.");
      KTRACE("EVENT_CODEAPPLICATIONQUIT recieved, shutting down.");
      appState->isRunning = false;
      return true;
    }
  }

  return false;
}

u8 applicationOnKey(u16 code , void* sender, void* listener , EventContext context){
  TRACEEVENT;
  if(code == EVENT_CODE_KEY_PRESSED){
    u16 keyCode = context.data.u16[0];
    if(keyCode  == KEY_ESCAPE){
      EventContext  data  = {};
      EINFO("APPLICATION QUIT EVENT");
      eventFire(EVENT_CODE_APPLICATION_QUIT , 0 , data);
      return true;
    }
  }else if (code  ==  EVENT_CODE_KEY_RELEASED){
    u16 keyCode = context.data.u16[0];
  }
  return false;
}


u8  applicationOnResize  (u16 code , void* sender , void* listener , EventContext context){
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
        appState->isSuspended = true;
        return true;
      } else {
        if (appState->isSuspended) {
          UWARN("Window restored, resuming application.");
          appState->isSuspended = false;
        }
        rendererResize(width, height);
        appState->game->onResize(appState->game, width, height);
      }
    }

  }    
  // Event purposely not handled to allow other listeners to get this.
  return false;
}

void  applicationGetFrameBufferSize(u32* width, u32* height){
  *width  = appState->width;
  *height = appState->height;
}

void  applicationGetName            (char* name){
  name  = appState->game->appConfig.name;
}

