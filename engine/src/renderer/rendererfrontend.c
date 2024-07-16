#include  "rendererfrontend.h"
#include  "rendererbackend.h"

#include  "core/logger.h"
#include  "math/kmath.h"
#include  "core/application.h"

typedef struct RendererSystemState{
  RendererBackend backend;
  Mat4            pojection;
  Mat4            view;
  f32             nearClip;
  f32             farClip;
  Texture         defaultTexture;
}RendererSystemState;;

bool    rendererBeginFrame(f32 deltaTime);
bool    rendererEndFrame(f32 deltaTime);
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

  applicationGetName(rendererSystemStatePtr->backend.applicationName);
  applicationGetFrameBufferSize (&rendererSystemStatePtr->backend.applicationWidth,
                                 &rendererSystemStatePtr->backend.applicationHeight);

  if(!rendererBackendInitialize(&rendererSystemStatePtr->backend)){
    KFATAL("FAILED TO INITIALIZE RENDERER BACKEND!!");
    return false;
  }
  rendererSystemStatePtr->nearClip  = 0.1f;
  rendererSystemStatePtr->farClip   = 1000.0f;
  rendererSystemStatePtr->pojection = mat4Prespective(degToRad(45.0),
                                                      1440/1080.0f,
                                                      rendererSystemStatePtr->nearClip,
                                                      rendererSystemStatePtr->farClip);
  rendererSystemStatePtr->view      = mat4Translation((Vec3){0,0,-30});
  rendererSystemStatePtr->view      = mat4Inverse(rendererSystemStatePtr->view);

  // create default texrure , a 256 X 256 white black chess board pattern 
  KTRACE("CREATING DEFEAULT TEXTURE");
  const u32 texDimension  = 256;
  const u32 channels      = 4;
  const u32 pixelCount    = texDimension * texDimension;
  u8  pixels[pixelCount * channels] = {0};
  u8  s = 2;
  for(u64  y = 0; y < texDimension; ++y){
    for(u64 x = 0; x < texDimension; ++x){
      u64 index = x + (y * texDimension);
      u64 channelIndex = index * channels;
      if(y%s){
        if(x%s){
          pixels[channelIndex + 0]  = 0;
          pixels[channelIndex + 1]  = 0;
        }
      }else{
        if(!(x%s)){
          pixels[channelIndex + 0]  = 0;
          pixels[channelIndex + 1]  = 0;
        }

      }
    }
  }

  rendererCreateTexture("default",
                        false,
                        texDimension,
                        texDimension,
                        4,
                        pixels,
                        false,
                        &rendererSystemStatePtr->defaultTexture);
  return true;
}

bool
rendererDrawFrame(RenderPacket* packet)
{
  UTRACE("%s called", __FUNCTION__);
  if(rendererBeginFrame(packet->deltaTime)){

    rendererUpdateGlobalState(rendererSystemStatePtr->pojection, rendererSystemStatePtr->view, vec3Zero(), vec4One(), 0);

    static f32 angle   = 0.1f;
    angle      += 0.1f;
    Quat rot    = quatFromAxisAngle(vec3Forward(), angle, false);
    Mat4 model  = quatToRotationMatrix(rot, vec3Zero());
    GeometryRenderData data = {};
    data.objectId = 0;
    data.model  = model;
    data.textures[0]  = &rendererSystemStatePtr->defaultTexture;
    updateObject(data);

    return rendererEndFrame(packet->deltaTime);
  }else{
    return false;
  }
}

void
rendererSetView(Mat4 view){
  rendererSystemStatePtr->view  = view;
}


bool  rendererCreateTexture(const char* name,bool autoRelease,i32 width,i32 height,i32 channelCount,const u8* pixels,bool hasTransparency,Texture* outTexture)
{
  bool result = createTexture(name,autoRelease,width,height,channelCount,pixels,hasTransparency,outTexture);
  if(result){
    KINFO("SUCCESFULLY CREATED TEXUTRE");
    return true;
  }else{
    UINFO("FAILED TO CREATE TETURE");
    return false;
  }
}

void  rendererDestroyTexture(Texture* texture){
  destroyTexture(texture);
}

bool
rendererResize(u16 width , u16 height)
{
  UTRACE("RENDRER RESIZE CALLED");
  if(rendererSystemStatePtr != NULL){
    rendererSystemStatePtr->backend.applicationWidth  = width;
    rendererSystemStatePtr->backend.applicationHeight = height;
    rendererSystemStatePtr->pojection = mat4Prespective(degToRad(45.0),
                                                        1440/1080.0f,
                                                        rendererSystemStatePtr->nearClip,
                                                        rendererSystemStatePtr->farClip);
    rendererBackendResized(width, height);
  }else{
    UFATAL("RESIZE CALLED WHEN NO BACKEND EXISTS!!, %i, %i",width,height);
  }
  return true;
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
    rendererSystemStatePtr->backend.frameNumber++;
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
    rendererDestroyTexture(&rendererSystemStatePtr->defaultTexture);
  rendererBackendShutdown();
}

