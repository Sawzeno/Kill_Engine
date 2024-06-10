#include  "rendererfrontend.h"

#include  "rendererbackend.h"
#include  "core/kmemory.h"
#include  "defines.h"

static RendererBackend* backend = 0;

u8  initializeRenderer(const char *applicationName, PlatformState *platState){

  backend = kallocate(sizeof(RendererBackend), MEMORY_TAG_RENDERER);


  backend->platState    = platState;
  backend->initilaize   = rendererBackendInitialize;
  backend->shutdown     = rendererBackendShutdown;
  backend->endFrame     = rendererBackendEndFrame;
  backend->beginFrame   = rendererBackendBeginFrame;
  backend->resized      = rendererBackendResized;

  if(!(backend->initilaize(applicationName , platState))){
    UFATAL("FAILED TO INITIALIZE RENDERER BACKEND!!");
    return false;
  }
  return true;
}

u8  shutdownRenderer(){

  backend->shutdown();
  backend->platState  = 0;
  backend->resized    = 0;
  backend->endFrame   = 0;
  backend->shutdown   = 0;
  backend->initilaize = 0;
  backend->beginFrame = 0;

  kfree(backend , sizeof(RendererBackend) , MEMORY_TAG_RENDERER);
  UINFO("RENDERER SUBSYTEM SHUTDOWN");
  return true;
}
u8 rendererBeginFrame(f32 deltaTime){
  return backend->beginFrame( deltaTime);
}

u8 rendererEndFrame(f32 deltaTime){
  u8 result = backend->endFrame( deltaTime);
  return result;
}

u8  rendererDrawFrame(RenderPacket* packet){
  if(rendererBeginFrame(packet->deltaTime)){
    UTRACE("RENDERER BEGIN FRAME SUCCESFULL");
    if(rendererEndFrame(packet->deltaTime)){
      UTRACE("RENDERER END FRAME SUCCESFULL");
      backend->frameNumber++;
      return true;
    }else{
      UERROR("rendererEndFrame Failed !");
      return false;
    }
  }else{
    UERROR("rendererBeginFrame Failed !");
    return false;
  }
}


u8  rendererOnResized(u16 width , u16 height){
  UTRACE("RENDRER RESIZE CALLED");
  if(backend){
    backend->resized(width, height);
  }else{
    UFATAL("RESIZE CALLED WHEN NO BAKEND EXISTS!!, %i, %i",width,height);
  }
  return true;
}
