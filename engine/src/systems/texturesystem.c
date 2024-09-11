#include  "texturesystem.h"
#include "defines.h"
#include "resource/resourcetypes.h"

#define STB_IMAGE_IMPLEMENTATION
#define STBI_FAILURE_USERMSG
#include  "stb_image.h"

#include  "core/kmemory.h"
#include  "core/kstring.h"
#include  "core/logger.h"
#include  "renderer/rendererbackend.h"
#include  "containers/hashtable.h"

typedef struct TextureSystemState{
  TextureSystemConfig config;
  Texture             defaultTexture;
  Texture*            registeredTextures;
  Hashtable           registeredTextureTable;
}TextureSystemState;

typedef struct TextureReference{
  u64 referenceCount;
  u32 handle;
  b32  autoRelease;
}TextureReference;

static TextureSystemState* state = NULL;

b32    createDefaultTextures ();
void  destroyDefaultTexture ();
b32    loadTexture           (Texture* tex, const char* name);
void  destroyTexture        (Texture* tex);

b32
textureSystemInitialize(u64* memReq, void* stateptr, TextureSystemConfig config)
{
  TRACEFUNCTION;
  if(config.maxTextureCount ==  0){
    KFATAL("MAX TEXTURE COUNT MUST BE > 0");
    return 0;
  }

  u64 stateReq  = sizeof(TextureSystemState);
  u64 arrayReq  = sizeof(Texture) * config.maxTextureCount;
  u64 hashtReq  = sizeof(TextureReference) * config.maxTextureCount;
  *memReq = stateReq + arrayReq + hashtReq;
  if(stateptr == NULL) return true;
  state  = stateptr;
  state->config    =config;

  void* arrayBlock  = stateptr + stateReq;
  state->registeredTextures  = arrayBlock;

  void* hashtBlock  = arrayBlock + arrayReq;
  hashtableCreate(sizeof(TextureReference), config.maxTextureCount, hashtBlock, false, &state->registeredTextureTable);

  TextureReference invalidRef = {0};
  invalidRef.handle         = INVALID_ID;
  invalidRef.autoRelease    = false;
  invalidRef.referenceCount = 0;
  hashtableFill(&state->registeredTextureTable, &invalidRef);

  u64 count = state->config.maxTextureCount;
  for(u64 i = 0; i < count; ++i){
    state->registeredTextures[i].id  = INVALID_ID;
    state->registeredTextures[i].generation  = INVALID_ID;
  }

  if(createDefaultTextures() == FALSE) return FALSE;
  KINFO("INITIALIZED TEXTURE SYSTEM SUCCESFULLY!");

  return TRUE;
}

Texture*  
textureSystemAcquire(const char* name, b32 autoRelease)
{
  TRACEFUNCTION;
  if(name == NULL || state == NULL) return NULL;

  u8 result = FALSE;
  KTRACE("ACQUIRING TEXTURE '%s'", name);
  if(kstrequali(name, DEFAULT_TEXTURE_NAME))
  {
    KWARN("USE textureSystemGetDefaultTexture FOR TEXTURE '%s'", DEFAULT_TEXTURE_NAME);
    return &state->defaultTexture;
  }
  TextureReference ref  = {0};
  result = hashtableGet(&state->registeredTextureTable, name, &ref);
  KCHECK_B32(result, "FAILED TO GET %s FROM registeredTextureTable",name);

  Texture* tex  =  NULL;
  u64 count = state->config.maxTextureCount;

  if(ref.referenceCount  == 0)  ref.autoRelease = autoRelease;
  ref.referenceCount++;

  if(ref.handle == INVALID_ID) // NEW TEXTURE
  {
    for(u64 i = 0 ; i < count; ++i)
    {
      if(state->registeredTextures[i].id == INVALID_ID){
        ref.handle  = i;
        tex = &state->registeredTextures[i];
        break;
      }
    }
    if(tex == NULL || ref.handle  == INVALID_ID)
    {
      KERROR("TEXTURE SYSTEM CANNOT HOLD ANYMORE TEXTURES!");
      return NULL;
    }
    result = loadTexture(tex, name);
    KCHECK_B32(result,"FAILED TO LOAD TEXTURE");

    tex->id = ref.handle;
  }
  KTRACE("referenceCount of %s INCREASED TO: %"PRIu64"",name, ref.referenceCount);

  result = hashtableSet(&state->registeredTextureTable, name, &ref);
  KCHECK_B32(result, "FAILED TO SET %s TO registeredTextureTable", name);

  KINFO("SUCCESFULLY ACQUIRED TEXTURE '%s'", name);
  return &state->registeredTextures[ref.handle];
}

b32
loadTexture(Texture* tex, const char* name){
  TRACEFUNCTION;

  if(state == NULL) return FALSE;

  const char* fmt = "/home/gon/Developer/Kill_Engine/assets/textures/%s" ;
  const i32 requiredChannelCount  = STBI_rgb_alpha;
  stbi_set_flip_vertically_on_load(false);

  char path[MAX_STR_LEN];
  if(kstrfmt(path, fmt, name) < 0) return false;
  UINFO("LOADING TEXTURE AT PATH '%s'", path);

  Texture temp;
  u8* data  = NULL;
  KTRACE("LODING IMAGE DATA");
  {
    data  = stbi_load(path, (i32*)&temp.width, (i32*)&temp.height, (i32*)&temp.channelCount, requiredChannelCount);
    if(data == NULL){
      if(stbi_failure_reason()){
        UWARN("FAILED TO LOAD IMAGE DATA '%s' : %s", path, stbi_failure_reason());
        stbi__err(0,0);
      }
      return FALSE;
    }
  }
  KDEBUG("SUCCESFULLY LOADED IMAGE DATA");
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

  kstrncpy(temp.name, name, MATERIAL_NAME_MAX_LEN);
  temp.generation   = INVALID_ID;               
  temp.hasTransparency  = hasTransparency;
  temp.channelCount = requiredChannelCount;

  b32 result = rendererCreateTexture(&temp, data);
  KCHECK_B32(result,"FAILED TO CREATE TEXTURE '%s'", name);

  Texture old = *tex; *tex  = temp;
  rendererDestroyTexture(&old);

  if(currentGeneration  ==  INVALID_ID){
    tex->generation = 0;
  }else{
    tex->generation = currentGeneration + 1;
  }

  stbi_image_free(data);

  UINFO("LOADED TEXTURE AT PATH '%s' SUCCESFULLY", path);
  return TRUE;
}

void      
textureSystemRelease(const char* name)
{
  TRACEFUNCTION;
  b32 result = FALSE;
  if(name == NULL || state == NULL) return;
  if(kstrequali(name, DEFAULT_TEXTURE_NAME))
  {
    KWARN("CANNOT FREE DEFAULT TEXTURE");
    return;
  }
  TextureReference ref = {0};
  result = hashtableGet(&state->registeredTextureTable, name, &ref);
  KCHECK_VOID(result, "CANNOT RELEASE %s FAILED TO GET registeredTextureTable");
  if(ref.referenceCount == 0 && ref.autoRelease == TRUE)
  {
    KWARN("TRIED TO RELEASE A NON EXISTENT TEXTURE '%s'", name);
    return;
  }
  char nameCopy[TEXTURE_NAME_MAX_LEN];
  kstrcpy(nameCopy, name);
  ref.referenceCount--;
  if(ref.referenceCount == 0 && ref.autoRelease)
  {
    Texture* tex  = &state->registeredTextures[ref.handle];
    destroyTexture(tex);
    ref.handle  = INVALID_ID;
    ref.autoRelease = FALSE;
    KTRACE("RELEASED & UNLOADED TEXTURE '%s'",name);
  }
  KTRACE("RELEASED TEXTURE '%s'| REF: (%"PRIu64")| AR : (%d)|",nameCopy,ref.referenceCount, ref.autoRelease);

  result = hashtableSet(&state->registeredTextureTable, nameCopy, &ref);
  KCHECK_VOID(result, "FAILED TO SET %s TO registeredTextureTable", name);
}

Texture*  textureSystemGetDefaultTexture(){
  if(state == NULL) return NULL;
  return &state->defaultTexture;
}

b32  
createDefaultTextures()
{
  TRACEFUNCTION;
  UINFO("CREATING DEFAULT TEXTURES");
  if(state == NULL) return 0;

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

  kstrcpy(state->defaultTexture.name, DEFAULT_TEXTURE_NAME);
  state->defaultTexture.width           = texdim;
  state->defaultTexture.height          = texdim;
  state->defaultTexture.channelCount    = 4;
  state->defaultTexture.generation      = INVALID_ID;
  state->defaultTexture.hasTransparency = FALSE;
  b32 result = rendererCreateTexture(&state->defaultTexture, pixels);
  KCHECK_B32(result, "COULD NOT CREATE DEFAULT TEXTURE, SHUTTING DOWN!!");

  state->defaultTexture.id  = INVALID_ID;
  state->defaultTexture.generation = INVALID_ID;
  return true;
}

void  
destroyDefaultTexture()
{
  if(state == NULL) return;
  destroyTexture(&state->defaultTexture);
}

void destroyTexture(Texture* tex){
  rendererDestroyTexture(tex);
  MEMZERO(&tex->name);
  MEMZERO(tex);
  tex->id         = INVALID_ID;
  tex->generation = INVALID_ID;
}

void textureSystemShutdown(){
  TRACEFUNCTION;
  UINFO("TEXTURE SYSTEM SHUTDOWN");
  {
    Texture* tex  =  NULL;
    if(state == NULL) return;
    for(u64 i = 0; i < state->config.maxTextureCount; ++i){
      tex = &state->registeredTextures[i];
      if(tex->generation  != INVALID_ID){
        rendererDestroyTexture(tex);
      }
    }
  }
  destroyDefaultTexture();
  state = NULL;
}

