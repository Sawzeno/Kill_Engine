#include  "rendererfrontend.h"
#include  "core/logger.h"

#include  "rendererbackend.h"
#include  "defines.h"


typedef struct RendererSystemState{
  RendererBackend backend;
}RendererSystemState;;

static  RendererSystemState*  rendererSystemStatePtr;

u8  initializeRenderer(u64* requiredMemory, void* state){
TRACEFUNCTION;
KDEBUG("requiredMemory : %"PRIu32" state : %p",*requiredMemory,state);
  *requiredMemory = sizeof(requiredMemory);
  if(state  ==  NULL){
    return true;
  }
  rendererSystemStatePtr  = state;
  
  rendererSystemStatePtr->backend.initilaize   = rendererBackendInitialize;
  rendererSystemStatePtr->backend.shutdown     = rendererBackendShutdown;
  rendererSystemStatePtr->backend.endFrame     = rendererBackendEndFrame;
  rendererSystemStatePtr->backend.beginFrame   = rendererBackendBeginFrame;
  rendererSystemStatePtr->backend.resized      = rendererBackendResized;

  if(!(rendererSystemStatePtr->backend.initilaize())){
    KFATAL("FAILED TO INITIALIZE RENDERER BACKEND!!");
    return false;
  }
  return true;
}

u8  shutdownRenderer(){
  KTRACE("RENDERER SUBSYTEM SHUTDOWN");
  rendererSystemStatePtr->backend.shutdown();
  rendererSystemStatePtr->backend.resized    = 0;
  rendererSystemStatePtr->backend.endFrame   = 0;
  rendererSystemStatePtr->backend.shutdown   = 0;
  rendererSystemStatePtr->backend.initilaize = 0;
  rendererSystemStatePtr->backend.beginFrame = 0;

  return true;
}
u8 rendererBeginFrame(f32 deltaTime){
  UTRACE("%s called", __FUNCTION__);
  return rendererSystemStatePtr->backend.beginFrame( deltaTime);
}

u8 rendererEndFrame(f32 deltaTime){
  UTRACE("%s called", __FUNCTION__);
  u8 result = rendererSystemStatePtr->backend.endFrame( deltaTime);
  return result;
}

u8  rendererDrawFrame(RenderPacket* packet){
  UTRACE("%s called", __FUNCTION__);
  if(rendererBeginFrame(packet->deltaTime)){
    UINFO("RENDERER BEGIN FRAME SUCCESFULL");
    if(rendererEndFrame(packet->deltaTime)){
      UINFO("RENDERER END FRAME SUCCESFULL");
      rendererSystemStatePtr->backend.frameNumber++;
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
  if(rendererSystemStatePtr != NULL){
    rendererSystemStatePtr->backend.resized(width, height);
  }else{
    UFATAL("RESIZE CALLED WHEN NO BACKEND EXISTS!!, %i, %i",width,height);
  }
  return true;
}
