#include  "rendererfrontend.h"
#include "core/kstring.h"
#include  "rendererbackend.h"
#include "resource/resourcetypes.h"
#include "systems/materialsystem.h"
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

  Material*       t;
}RendererSystemState;;

b32   rendererBeginFrame(f32 deltaTime);
b32   rendererEndFrame  (f32 deltaTime);
b32   eventOnDebug      (u16 code, void* sender, void* listener, EventContext data);
static  RendererSystemState*  state;

bool
rendererDrawFrame(RenderPacket* packet)
{
  TRACEFUNCTION;
  if(rendererBeginFrame(packet->deltaTime) == FALSE) return false;

  rendererUpdateGlobalState(state->pojection, state->view, vec3Zero(), vec4One(), 0);
  Mat4 model = mat4Translation((Vec3){0,0,0});
  // static f32 angle   = 0.1f;
  // angle      += 0.1f;
  // Quat rot    = quatFromAxisAngle(vec3Forward(), angle, FALSE);
  // Mat4 model  = quatToRotationMatrix(rot, vec3Zero());
  GeomteryRenderData data = {0};
  data.model  = model;
  if(state->t == NULL)
  {
    UINFO("LOADING MATERIAL test.kill");
    state->t  = materialSystemAcquireResources("test.kill"); 
    if(state->t == NULL)
    {
      UINFO("COULD NOT LOAD test.kill CREATING A DEFAULT MATERIAL");
      MaterialConfig config;
      kstrncpy(config.name, "test.kill", MATERIAL_NAME_MAX_LEN);
      kstrncpy(config.diffuseMapName, DEFAULT_MATERIAL_NAME, TEXTURE_NAME_MAX_LEN);
      config.autoRelease  = FALSE;
      config.diffuseColor = vec4One();
      state->t  = materialSystemAcquireFromConfig(config);
      if(state->t == NULL)
      {
        UINFO("COULD NOT EVEN LOAD DEFAULT MATERIAL");
        return FALSE;
      }
    }
  }
  data.material = state->t;
  rendererUpdateObject(data);

  return rendererEndFrame(packet->deltaTime);
}
b32 
eventOnDebug(u16 code, void* sender, void* listener, EventContext data)
{
  static i8 choice  = 3;
  const char* names[3] = {
    "bismarck.png",
    "scylla.png",
    "u410.png",
  };

  const char* oldname = names[0];
  choice++;
  choice %= 3;

  state->t->diffuseMap.texture  = textureSystemAcquire(names[choice], TRUE);
  if(state->t->diffuseMap.texture == NULL){
    UWARN("FAILED TO ACQUIRE TEXTURE , USING DEFAULT");
    state->t->diffuseMap.texture  = textureSystemGetDefaultTexture();
  }
  textureSystemRelease(oldname);

  return TRUE;
}

bool
rendererInitalize(u64* requiredMemory, void* statePtr)
{
  TRACEFUNCTION;
  KDEBUG("requiredMemory : %"PRIu32" state : %p",*requiredMemory,statePtr);
  *requiredMemory = sizeof(requiredMemory);
  if(statePtr  ==  NULL){
    return TRUE;
  }
  state  = statePtr;

  eventRegister(EVENT_CODE_DEBUG0, state, eventOnDebug);

  applicationGetName(state->backend.applicationName);
  applicationGetFrameBufferSize (&state->backend.applicationWidth,
                                 &state->backend.applicationHeight);

  if(!rendererBackendInitialize(&state->backend)){
    KFATAL("FAILED TO INITIALIZE RENDERER BACKEND!!");
    return FALSE;
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

  return TRUE;
}

b32
rendererBeginFrame(f32 deltaTime)
{
  KINFO("-------------BEGIN FRAME---------------");
  UINFO("-------------BEGIN FRAME---------------");
  if(rendererBackendBeginFrame( deltaTime)){
    UINFO("RENDERER BEGIN FRAME SUCCESFULL");
    return TRUE;
  }else{
    UERROR("RENDERER BEGIN FRAME FAILED");
    return FALSE;
  }
}

b32
rendererEndFrame(f32 deltaTime)
{
  KINFO("----------------END FRAME---------------");
  UINFO("----------------END FRAME---------------");
  if(rendererBackendEndFrame( deltaTime)){
    UINFO("RENDERER END FRAME SUCCESFULL");
    state->backend.frameNumber++;
    return TRUE;
  }else{
    UERROR("RENDERER END FRAME FAILED !");
    return FALSE;
  }
}

void
rendererShutdown()
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
  return TRUE;
}

