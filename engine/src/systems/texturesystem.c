#include  "texturesystem.h"
#include "defines.h"

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
  b8  autoRelease;
}TextureReference;

static TextureSystemState* state = NULL;

b8    createDefaultTextures();
void  destroyDefaultTexture();
b8    loadTexture(const char* texName, Texture* tex);

b8
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

  printTextureInfo(&state->defaultTexture, "AFTER TEX SYSTEM INIT");
  return TRUE;
}

b8
loadTexture(const char* texName, Texture* tex){
  TRACEFUNCTION;

  if(state == NULL) return FALSE;

  const char* fmt = "/home/gon/Developer/Kill_Engine/assets/textures/%s" ;
  const i32 requiredChannelCount  = STBI_rgb_alpha;
  stbi_set_flip_vertically_on_load(false);

  char path[1024];
  if(kstrfmt(path, fmt, texName) < 0) return false;
  UINFO("LOADING TEXTURE AT PATH '%s'", path);

  Texture temp;
  u8* data  = stbi_load(path, (i32*)&temp.width, (i32*)&temp.height, (i32*)&temp.channelCount, requiredChannelCount);

  if(!data){
    if(stbi_failure_reason()){
      UWARN("loadTexture failed to load file '%s' : %s", path, stbi_failure_reason());
    }
    return FALSE;
  }

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
  b8 result = rendererCreateTexture(texName, temp.width, temp.height, temp.channelCount, data, hasTransparency, &temp);
  if(result == FALSE){
    KFATAL("FAILED TO CREATE TEXTURE '%s'", texName);
    return FALSE;
  }

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

Texture*  
textureSystemAcquire(const char* name, b8 autoRelease)
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
  CHECK_B8(result, "FAILED TO GET %s FROM registeredTextureTable",name);

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
    result = loadTexture(name, tex);
    CHECK_B8(result,"FAILED TO LOAD TEXTURE");

    tex->id = ref.handle;
  }
  KTRACE("referenceCount of %s INCREASED TO: %"PRIu64"",name, ref.referenceCount);

  result = hashtableSet(&state->registeredTextureTable, name, &ref);
  CHECK_B8(result, "FAILED TO SET %s TO registeredTextureTable", name);

  KINFO("SUCCESFULLY ACQUIRED TEXTURE '%s'", name);
  return &state->registeredTextures[ref.handle];
}

void      
textureSystemRelease(const char* name)
{
  TRACEFUNCTION;
  if(name == NULL || state == NULL) return;
  if(kstrequali(name, DEFAULT_TEXTURE_NAME))
  {
    KWARN("CANNOT FREE DEFAULT TEXTURE");
    return;
  }
  TextureReference ref = {0};
  if(hashtableGet(&state->registeredTextureTable, name, &ref) == FALSE)
  {
    KERROR("FAILED TO GET %s registeredTextureTable");
    return;
  }
  if(ref.referenceCount == 0 && ref.autoRelease == TRUE)
  {
    KWARN("TRIED TO RELEASE A NON EXISTENT TEXTURE '%s'", name);
    return;
  }
  ref.referenceCount--;
  if(ref.referenceCount == 0 && ref.autoRelease){
    Texture* tex  = &state->registeredTextures[ref.handle];
    rendererDestroyTexture(tex);
    MEMZERO(tex);
    tex->id = INVALID_ID;
    tex->generation = INVALID_ID;
    ref.handle  = INVALID_ID;
    ref.autoRelease = FALSE;
    KTRACE("RELEASED & UNLOADED TEXTURE '%s'",name);
  }
  KTRACE("RELEASED TEXTURE '%s'| REF: (%"PRIu64")| AR : (%d)|",name,ref.referenceCount, ref.autoRelease);
  hashtableSet(&state->registeredTextureTable, name, &ref);
}

Texture*  textureSystemGetDefaultTexture(){
  if(state == NULL) return NULL;
  return &state->defaultTexture;
}

b8  
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

  bk1();
  b8 result = rendererCreateTexture("default", texdim, texdim, bpp, pixels, false, &state->defaultTexture);
  CHECK_B8(result, "COULD NOT CREATE DEFAULT TEXTURE, SHUTTING DOWN!!");

  state->defaultTexture.id  = INVALID_ID;
  state->defaultTexture.generation = INVALID_ID;
  return true;
}

void  
destroyDefaultTexture()
{
  if(state == NULL) return;
  rendererDestroyTexture(&state->defaultTexture);
}
void printTextureInfo(const Texture* texture,const char* str) {
  (void)texture;
  (void)str;
    // UINFO(str);
    // if (texture == NULL) {
    //     UERROR("Texture is NULL");
    //     return;
    // }
    // UINFO("Texture Info:");
    // UINFO("  ID: %u", texture->id);
    // UINFO("  Width: %u", texture->width);
    // UINFO("  Height: %u", texture->height);
    // UINFO("  Channel Count: %u", texture->channelCount);
    // UINFO("  Has Transparency: %s", texture->hasTransparency ? "Yes" : "No");
    // UINFO("  Generation: %u", texture->generation);
    // UINFO("  Internal Data Pointer: %p", texture->internalData);
}
