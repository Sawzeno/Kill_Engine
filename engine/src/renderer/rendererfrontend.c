#include  "rendererfrontend.h"

#include  "rendererbackend.h"
#include  "core/kmemory.h"
#include  "defines.h"

static rendererBackend* backend = 0;

u8  initializeRenderer(const char *applicationName, platformState *platState){

  backend = kallocate(sizeof(rendererBackend), MEMORY_TAG_RENDERER);
  if(backend  ==  NULL){
    UFATAL("could not allocate memory for backend")
    return false;
  }

  backend->platState    = platState;
  backend->initilaize   = rendererBackendInitialize;
  backend->shutdown     = rendererBackendShutdown;
  backend->endFrame     = rendererBackendEndFrame;
  backend->beginFrame   = rendererBackendBeginFrame;
  backend->resized      = rendererBackendResized;

  if(!(backend->initilaize(applicationName , platState))){
    UFATAL("FAILED TO INITIALIZE RENDERER BACKEND!!")
    return false;
  }

  return true;
}

u8  shutdownRenderer(){
  backend->platState  = 0;
  backend->resized    = 0;
  backend->endFrame   = 0;
  backend->shutdown   = 0;
  backend->initilaize = 0;
  backend->beginFrame = 0;

  kfree(backend , sizeof(rendererBackend) , MEMORY_TAG_RENDERER);

  UINFO("RENDERER SUBSYTEM SHUTDOWN");
  return true;
}
u8 rendererBeginFrame(f32 deltaTime){
  return backend->beginFrame( deltaTime);
}

u8 rendererEndFrame(f32 deltaTime){
  u8 result = backend->endFrame( deltaTime);
  backend->frameNumber++;
  return result;
}

u8  rendererDrawFrame(renderPacket* packet){
  UTRACE("RENDERER DRAW FRAME CALLED")
  if(rendererBeginFrame(packet->deltaTime)){
    u8 result = rendererEndFrame(packet->deltaTime);
    if(result ==  false){
      UERROR("rendererEndFrame Failed !")
      return false;
    }
  }

  return true;
}
