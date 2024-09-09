#include  "rendererfrontend.h"
#include "core/kmemory.h"
#include  "rendererbackend.h"

#include  "defines.h"
#include  "core/logger.h"
#include  "math/kmath.h"
#include  "core/application.h"

typedef struct RendererSystemState{
  RendererBackend backend;
  Mat4            pojection;
  Mat4            view;
  f32             nearClip;
  f32             farClip;

  Texture         defaultTex;
}RendererSystemState;;

bool    rendererBeginFrame(f32 deltaTime);
bool    rendererEndFrame(f32 deltaTime);
static  RendererSystemState*  statePtr;

bool
initializeRenderer(u64* requiredMemory, void* state)
{
  TRACEFUNCTION;
  KDEBUG("requiredMemory : %"PRIu32" state : %p",*requiredMemory,state);
  *requiredMemory = sizeof(requiredMemory);
  if(state  ==  NULL){
    return true;
  }
  statePtr  = state;

  applicationGetName(statePtr->backend.applicationName);
  applicationGetFrameBufferSize (&statePtr->backend.applicationWidth,
                                 &statePtr->backend.applicationHeight);

  if(!rendererBackendInitialize(&statePtr->backend)){
    KFATAL("FAILED TO INITIALIZE RENDERER BACKEND!!");
    return false;
  }
  statePtr->nearClip  = 0.1f;
  statePtr->farClip   = 1000.0f;
  statePtr->pojection = mat4Prespective(degToRad(45.0),
                                                      1440/1080.0f,
                                                      statePtr->nearClip,
                                                      statePtr->farClip);
  statePtr->view      = mat4Translation((Vec3){0,0,-30});
  statePtr->view      = mat4Inverse(statePtr->view);

  UTRACE("CREATING DEFAULT TEX DATA");

  const u32 texdim    =256;
  const u32 bpp       = 4;
  const u32 pixelcount= texdim * texdim;

  u8 pixels[pixelcount * bpp];
  ksetMemory(pixels, 255, sizeof(u8) * pixelcount * bpp);

  for( u64 y = 0; y < texdim; ++y){
    for( u64 x = 0; x < texdim; ++x){
      u64 i = (y * texdim) + x;
      u64 ip=  i * bpp;
      if( (y + x) % 2 == 0){
        pixels[ip + 0]  = 0;
        pixels[ip + 1]  = 0;
      }
    }
  }

  rendererCreateTexture("defalut", false, texdim, texdim, bpp, pixels, false, &statePtr->defaultTex);
  return true;
}

bool
rendererDrawFrame(RenderPacket* packet)
{
  UTRACE("%s called", __FUNCTION__);
  if(rendererBeginFrame(packet->deltaTime)){

    rendererUpdateGlobalState(statePtr->pojection, statePtr->view, vec3Zero(), vec4One(), 0);

    Mat4 model = mat4Translation((Vec3){0,0,0});
    // static f32 angle   = 0.1f;
    // angle      += 0.1f;
    // Quat rot    = quatFromAxisAngle(vec3Forward(), angle, false);
    // Mat4 model  = quatToRotationMatrix(rot, vec3Zero());
    GeomteryRenderData data = {0};
    data.objectId = 0;
    data.model  = model;
    data.textures[0] = &statePtr->defaultTex;
    updateObject(data);

    return rendererEndFrame(packet->deltaTime);
  }else{
    return false;
  }
}

bool
rendererBeginFrame(f32 deltaTime)
{
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
    statePtr->backend.frameNumber++;
    return true;
  }else{
    UERROR("RENDERER END FRAME FAILED !");
    return false;
  }
}

void rendererCreateTexture(const char *name, bool autoRelease, i32 width, u32 height, i32 channelCount, const u8 *pixels, u32 hasTransparency, Texture *tex){
  rendererBackendCreateTexture(name, autoRelease, width, height, channelCount, pixels, hasTransparency, tex);
}

void rendererDestroyTexture( Texture* tex){
  rendererBackendDestroyTexture(tex);
}


void
shutdownRenderer()
{
  KTRACE("RENDERER SUBSYTEM SHUTDOWN");
  rendererDestroyTexture(&statePtr->defaultTex);
  rendererBackendShutdown();
}

void
rendererSetView(Mat4 view){
  statePtr->view  = view;
}

bool
rendererResize(u16 width , u16 height)
{
  UTRACE("RENDRER RESIZE CALLED");
  if(statePtr != NULL){
    statePtr->backend.applicationWidth  = width;
    statePtr->backend.applicationHeight = height;
    statePtr->pojection = mat4Prespective(degToRad(45.0),
                                                      1440/1080.0f,
                                                      statePtr->nearClip,
                                                      statePtr->farClip);
    rendererBackendResized(width, height);
  }else{
    UFATAL("RESIZE CALLED WHEN NO BACKEND EXISTS!!, %i, %i",width,height);
  }
  return true;
}
