#include  "rendererfrontend.h"
#include  "rendererbackend.h"

#include  "defines.h"
#include  "core/logger.h"
#include  "math/kmath.h"
#include  "core/application.h"

typedef struct RendererSystemState{
  RendererBackend backend;
}RendererSystemState;;

static  RendererSystemState*  rendererSystemStatePtr;

bool
initializeRenderer(u64* requiredMemory, void* state)
{
  TRACEFUNCTION;
  KDEBUG("requiredMemory : %"PRIu32" state : %p",*requiredMemory,state);
  *requiredMemory = sizeof(requiredMemory);
  if(state  ==  NULL){
    return true;
  }
  rendererSystemStatePtr  = state;
  u32 width, height;
  char* applicationName;
  applicationGetName(applicationName);
  applicationGetFrameBufferSize (&width, &height);

  if(!rendererBackendInitialize(width, height, applicationName)){
    KFATAL("FAILED TO INITIALIZE RENDERER BACKEND!!");
    return false;
  }
  return true;
}

void
shutdownRenderer()
{
  KTRACE("RENDERER SUBSYTEM SHUTDOWN");
  rendererBackendShutdown();
}

bool rendererBeginFrame(f32 deltaTime){
  UTRACE("%s called", __FUNCTION__);
  KINFO("-------------BEGIN FRAME---------------");
  if(rendererBackendBeginFrame( deltaTime)){
    UINFO("RENDERER BEGIN FRAME SUCCESFULL");
    return true;
  }else{
    UERROR("RENDERER BEGIN FRAME FAILED");
    return false;
  }
}

bool
rendererEndFrame(f32 deltaTime)
{
  UTRACE("%s called", __FUNCTION__);
  KINFO("----------------END FRAME---------------");
  if(rendererBackendEndFrame( deltaTime)){
    UINFO("RENDERER END FRAME SUCCESFULL");
    rendererSystemStatePtr->backend.frameNumber++;
    return true;
  }else{
    UERROR("RENDERER END FRAME FAILED !");
    return false;
  }
}

bool
rendererDrawFrame(RenderPacket* packet)
{
  UTRACE("%s called", __FUNCTION__);
  if(rendererBeginFrame(packet->deltaTime)){

    static f32 z    = -10.0f;
    z -= 0.01f;             
    Mat4 projection = mat4Prespective(degToRad(45.0), 1440/1080.0f, 0.1f, 1000.0f);
    Mat4 view       = mat4Translation((Vec3){0,0,z});
    rendererUpdateGlobalState(projection, view, vec3Zero(), vec4One(), 0);

    static f32 angle   = 0.1f;
    angle      += 0.1f;
    Quat rot    = quatFromAxisAngle(vec3Forward(), angle, false);
    Mat4 model  = quatToRotationMatrix(rot, vec3Zero());
    updateObject(model);

    return rendererEndFrame(packet->deltaTime);
  }else{
    return false;
  }
}


bool
rendererOnResized(u16 width , u16 height)
{
  UTRACE("RENDRER RESIZE CALLED");
  if(rendererSystemStatePtr != NULL){
    rendererBackendResized(width, height);
  }else{
    UFATAL("RESIZE CALLED WHEN NO BACKEND EXISTS!!, %i, %i",width,height);
  }
  return true;
}
