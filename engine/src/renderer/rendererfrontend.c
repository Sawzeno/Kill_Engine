#include  "rendererfrontend.h"
#include  "rendererbackend.h"
#include "resource/resourcetypes.h"
#include  "systems/texturesystem.h"

#include  "defines.h"
#include  "math/kmath.h"
#include  "core/logger.h"
#include  "core/application.h"
#include  "core/events.h"


typedef struct RendererSystemState{
  RendererBackend backend;
  Mat4            pojection;
  Mat4            view;
  f32             nearClip;
  f32             farClip;

  Texture*        t;
}RendererSystemState;;

bool    rendererBeginFrame(f32 deltaTime);
bool    rendererEndFrame  (f32 deltaTime);
u8      eventOnDebug      (u16 code, void* sender, void* listener, EventContext data);
static  RendererSystemState*  state;



bool
rendererDrawFrame(RenderPacket* packet)
{
  TRACEFUNCTION;
  if(rendererBeginFrame(packet->deltaTime)){

    rendererUpdateGlobalState(state->pojection, state->view, vec3Zero(), vec4One(), 0);
    Mat4 model = mat4Translation((Vec3){0,0,0});
    // static f32 angle   = 0.1f;
    // angle      += 0.1f;
    // Quat rot    = quatFromAxisAngle(vec3Forward(), angle, false);
    // Mat4 model  = quatToRotationMatrix(rot, vec3Zero());
    GeomteryRenderData data = {0};
    data.objectId = 0;
    data.model  = model;

    if(!state->t){
      UINFO("SETTIG DEFAULT TEXTURE TO TEX ARRAY");
      state->t = textureSystemGetDefaultTexture();
    }
    data.textures[0] = state->t;
    updateObject(data);

    return rendererEndFrame(packet->deltaTime);
  }else{
    return false;
  }
}
u8 eventOnDebug(u16 code, void* sender, void* listener, EventContext data){
  static i8 choice  = 3;
  const char* names[3] = {
    "bismarck.png",
    "scylla.png",
    "u410.png",
  };

  const char* oldname = names[choice];
  choice++;
  choice %= 3;
  state->t = textureSystemAcquire(names[choice], true);
  textureSystemRelease(oldname);

  return true;
}

bool
initializeRenderer(u64* requiredMemory, void* statePtr)
{
  TRACEFUNCTION;
  KDEBUG("requiredMemory : %"PRIu32" state : %p",*requiredMemory,statePtr);
  *requiredMemory = sizeof(requiredMemory);
  if(statePtr  ==  NULL){
    return true;
  }
  state  = statePtr;

  eventRegister(EVENT_CODE_DEBUG0, state, eventOnDebug);

  applicationGetName(state->backend.applicationName);
  applicationGetFrameBufferSize (&state->backend.applicationWidth,
                                 &state->backend.applicationHeight);

  if(!rendererBackendInitialize(&state->backend)){
    KFATAL("FAILED TO INITIALIZE RENDERER BACKEND!!");
    return false;
  }
  state->nearClip  = 0.1f;
  state->farClip   = 1000.0f;
  state->pojection = mat4Prespective(degToRad(45.0),
                                     1440/1080.0f,
                                     state->nearClip,
                                     state->farClip);
  state->view      = mat4Translation((Vec3){0,0,-30});
  state->view      = mat4Inverse(state->view);
  state->t = NULL;

  printTextureInfo(state->t, "FRONTEND TEX AT START");

  return true;
}


bool
rendererBeginFrame(f32 deltaTime)
{
  KINFO("-------------BEGIN FRAME---------------");
  UINFO("-------------BEGIN FRAME---------------");
  printTextureInfo(state->t, "FRONTEND TEX AT BEGIN FRAME");
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
  KINFO("----------------END FRAME---------------");
  UINFO("----------------END FRAME---------------");
  // printTextureInfo(state->t, "FRONTEND TEX AT END FRAME");
  if(rendererBackendEndFrame( deltaTime)){
    UINFO("RENDERER END FRAME SUCCESFULL");
    state->backend.frameNumber++;
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
  eventUnregister(EVENT_CODE_DEBUG0, state, eventOnDebug);
  rendererBackendShutdown();
}

void
rendererSetView(Mat4 view){
  state->view  = view;
}

bool
rendererResize(u16 width , u16 height)
{
  TRACEFUNCTION;
  if(state != NULL){
    state->backend.applicationWidth  = width;
    state->backend.applicationHeight = height;
    state->pojection = mat4Prespective(degToRad(45.0),
                                       1440/1080.0f,
                                       state->nearClip,
                                       state->farClip);
    rendererBackendResized(width, height);
  }else{
    UFATAL("RESIZE CALLED WHEN NO BACKEND EXISTS!!, %i, %i",width,height);
  }
  return true;
}

