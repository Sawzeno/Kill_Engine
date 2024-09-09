#include  "rendererfrontend.h"
#include "core/kmemory.h"
#include  "rendererbackend.h"

#include  "defines.h"
#include  "core/logger.h"
#include  "math/kmath.h"
#include  "core/application.h"

// TEMP
#include  "core/kstring.h"
#include  "core/events.h"
#define STB_IMAGE_IMPLEMENTATION
// #define STBI_FAILURE_USERMSG
#include  "stb_image.h"

typedef struct RendererSystemState{
  RendererBackend backend;
  Mat4            pojection;
  Mat4            view;
  f32             nearClip;
  f32             farClip;

  Texture         defaultTex;
  Texture         test;
}RendererSystemState;;

bool    rendererBeginFrame(f32 deltaTime);
bool    rendererEndFrame  (f32 deltaTime);
void    createTexture     (Texture* tex);
bool    loadTexture       (const char* texName, Texture* tex);
u8      eventOnDebug      (u16 code, void* sender, void* listener, EventContext data);
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
  statePtr->backend.defaultDiffuse  = &statePtr->defaultTex;

  eventRegister(EVENT_CODE_DEBUG0, statePtr, eventOnDebug);

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

  rendererCreateTexture("default", false, texdim, texdim, bpp, pixels, false, &statePtr->defaultTex);
  statePtr->defaultTex.generation = INVALID_ID;

  createTexture(&statePtr->test);
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
    data.textures[0] = &statePtr->test;
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

void
shutdownRenderer()
{
  KTRACE("RENDERER SUBSYTEM SHUTDOWN");
  eventUnregister(EVENT_CODE_DEBUG0, statePtr, eventOnDebug);
  rendererDestroyTexture(&statePtr->defaultTex);
  rendererDestroyTexture(&statePtr->test);
  rendererBackendShutdown();
}

void
rendererSetView(Mat4 view){
  statePtr->view  = view;
}

bool
rendererResize(u16 width , u16 height)
{
  TRACEFUNCTION;
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

void createTexture(Texture* tex){
  MEMZERO(tex);
  tex->generation = INVALID_ID;
}

bool loadTexture(const char* texName, Texture* tex){
  const char* fmt = "/home/gon/Developer/Kill_Engine/assets/textures/%s" ;

  const i32 requiredChannelCount  = STBI_rgb_alpha;
  stbi_set_flip_vertically_on_load(false);

  char path[1024];
  if(kstrfmt(path, fmt, texName) < 0) return false;
  UINFO("LOADING TEXTURE '%s'", path);

  Texture temp;
  u8* data  = stbi_load(path, (i32*)&temp.width, (i32*)&temp.height, (i32*)&temp.channelCount, requiredChannelCount);

  if(!data){
    if(stbi_failure_reason()){
      UWARN("loadTexture failed to load file '%s' : %s", path, stbi_failure_reason());
    }
    return false;
  }

  UINFO("LOADED DATA SUCCESFULLY!");
  u32 currentGeneration = tex->generation;
  tex->generation = INVALID_ID;

  u64 totalSize = temp.width * temp.height * requiredChannelCount;

  bool hasTransparency = false;
  for( u64 i = 0; i < totalSize; i += requiredChannelCount){
    u8 a = data[i+3];
    if(a < 255){
      hasTransparency = true;
      break;
    }
  }

  rendererCreateTexture(texName, true, temp.width, temp.height, temp.channelCount, data, hasTransparency, &temp);

  Texture old = *tex;
  *tex  = temp;
  rendererDestroyTexture(&old);

  if(currentGeneration  ==  INVALID_ID){
    tex->generation = 0;
  }else{
    tex->generation = currentGeneration + 1;
  }
  stbi_image_free(data);
  return true;
}

u8 eventOnDebug(u16 code, void* sender, void* listener, EventContext data){
  const char* names[3] = {
    "bismarck.png",
    "scylla.png",
    "u410.png",
  };

  static i8 choice  = 2;
  choice++;
  choice %= 3;

  return (u8)loadTexture(names[choice], &statePtr->test);
}
