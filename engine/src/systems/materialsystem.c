#include  "materialsystem.h"
#include  "containers/hashtable.h"
#include "core/filesystem.h"
#include "core/kmemory.h"
#include "core/kstring.h"
#include "core/logger.h"
#include "defines.h"
#include "math/kmath.h"
#include "renderer/rendererbackend.h"
#include "systems/texturesystem.h"


typedef struct MaterialSystemState  MaterialSystemState;
typedef struct MaterialReference    MaterialReference;

struct MaterialSystemState{
  MaterialSystemConfig  config;
  Material              defualtMaterial;
  Material*             registeredMaterials;
  Hashtable             registeredMaterialTable;
};

struct MaterialReference{
  u64 referenceCount;
  u32 handle;
  b32  autoRelease;
};


b32   loadConfigFile  (MaterialConfig* config, const char* path);
b32   loadMaterial    (Material* mat, MaterialConfig config);
void  destroyMaterial (Material* mat);
b32   createDefaultMaterial ();


static MaterialSystemState* state;

b32
materialSystemInitailize(u64* memReq, void* stateptr, MaterialSystemConfig config)
{
  TRACEFUNCTION;
  if(config.maxMaterialCount ==  0){
    KFATAL("MAX MATERIAL COUNT MUST BE > 0");
    return 0;
  }

  u64 stateReq  = sizeof(MaterialSystemState);
  u64 arrayReq  = sizeof(Material) * config.maxMaterialCount;
  u64 hashtReq  = sizeof(MaterialReference) * config.maxMaterialCount;
  *memReq = stateReq + arrayReq + hashtReq;
  if(stateptr == NULL) return true;
  state  = stateptr;
  state->config    =config;

  void* arrayBlock  = stateptr + stateReq;
  state->registeredMaterials  = arrayBlock;

  void* hashtBlock  = arrayBlock + arrayReq;
  hashtableCreate(sizeof(MaterialReference), config.maxMaterialCount, hashtBlock, false, &state->registeredMaterialTable);

  MaterialReference invalidRef = {0};
  invalidRef.handle         = INVALID_ID;
  invalidRef.autoRelease    = false;
  invalidRef.referenceCount = 0;
  hashtableFill(&state->registeredMaterialTable, &invalidRef);

  u64 count = state->config.maxMaterialCount;
  for(u64 i = 0; i < count; ++i){
    state->registeredMaterials[i].id          = INVALID_ID;
    state->registeredMaterials[i].internalId  = INVALID_ID;
    state->registeredMaterials[i].generation  = INVALID_ID;
  }

  if(createDefaultMaterial() == FALSE) return FALSE;
  KINFO("INITIALIZED MATERIAL SYSTEM SUCCESFULLY!");

  return TRUE;
}

void 
materialSystemShutdown()
{
  TRACEFUNCTION;
  if(state == NULL) return ;
  UINFO("TEXTURE SYSTEM SHUTDOWN");
  {
    Material* mat  =  NULL;
    if(state == NULL) return;
    for(u64 i = 0; i < state->config.maxMaterialCount; ++i){
      mat = &state->registeredMaterials[i];
      if(mat->generation  != INVALID_ID){
        rendererDestroyMaterial(mat);
      }
    }
    destroyMaterial(&state->defualtMaterial);
  }
  state = NULL;
}

Material*
materialSystemAcquireResources   (const char* name)
{
  TRACEFUNCTION;

  if(state == NULL || name == NULL) return NULL;
  u8 result = FALSE;
  MaterialConfig config;
  char* fmt = "/home/gon/Developer/Kill_Engine/assets/materials/%s";
  char path[MAX_STR_LEN];
  if(kstrfmt(path, fmt, name) < 0) return NULL;
  UINFO("LOADING MATERIAL AT PATH '%s'", path);

  result = loadConfigFile(&config, path);
  KCHECK_B32(result, "FAILED TO LOAD MATERIAL CONFIG FILE");

  UINFO("LOADED MATERIAL AT PATH '%s' SUCCESFULLY", path);
  return materialSystemAcquireFromConfig(config);
}
void      materialSystemRelease   (const char* name);

Material* 
materialSystemAcquireFromConfig(MaterialConfig config)
{
  TRACEFUNCTION;
  if(state == NULL) return NULL;
  u8 result = FALSE;
  if(kstrequali(config.name, DEFAULT_MATERIAL_NAME))
  {
    KWARN("USE materialSystemGetDefaultMaterial FOR TEXTURE '%s'", DEFAULT_MATERIAL_NAME);
    return &state->defualtMaterial;
  }
  MaterialReference ref  = {0};
  result = hashtableGet(&state->registeredMaterialTable, config.name, &ref);
  KCHECK_B32(result, "FAILED TO GET %s FROM registeredMaterialTable",config.name);

  Material* mat  =  NULL;
  u64 count = state->config.maxMaterialCount;

  ref.referenceCount++;
  if(ref.handle == INVALID_ID) // NEW TEXTURE
  {
    for(u64 i = 0 ; i < count; ++i)
    {
      if(state->registeredMaterials[i].id == INVALID_ID){
        ref.handle  = i;
        mat = &state->registeredMaterials[i];
        break;
      }
    }
    if(mat == NULL || ref.handle  == INVALID_ID)
    {
      KERROR("TEXTURE SYSTEM CANNOT HOLD ANYMORE TEXTURES!");
      return NULL;
    }
    result = loadMaterial(mat, config);
    KCHECK_B32(result,"FAILED TO LOAD TEXTURE");

    mat->id = ref.handle;
  }
  KTRACE("referenceCount of %s INCREASED TO: %"PRIu64"",config.name, ref.referenceCount);

  result = hashtableSet(&state->registeredMaterialTable, config.name, &ref);
  KCHECK_B32(result, "FAILED TO SET %s TO registeredTextureTable", config.name);

  KINFO("SUCCESFULLY ACQUIRED TEXTURE '%s'", config.name);
  return &state->registeredMaterials[ref.handle];
}

void      
materialSystemReleaseResources(const char* name)
{
  TRACEFUNCTION;
  b32 result = FALSE;
  if(name == NULL || state == NULL) return;
  if(kstrequali(name, DEFAULT_MATERIAL_NAME))
  {
    KWARN("CANNOT FREE DEFAULT MATERIAL");
    return;
  }
  MaterialReference ref = {0};
  result = hashtableGet(&state->registeredMaterialTable, name, &ref); 
  KCHECK_VOID(result, "CANNOT RELEASE %s, FAILED TO GET  FROM registeredMaterialTable");
  if(ref.referenceCount == 0 && ref.autoRelease == TRUE)
  {
    KWARN("TRIED TO RELEASE A NON EXISTENT MATERIAL '%s'", name);
    return;
  }
  ref.referenceCount--;
  if(ref.referenceCount == 0 && ref.autoRelease)
  {
    Material* mat  = &state->registeredMaterials[ref.handle];
    destroyMaterial(mat);
    ref.handle  = INVALID_ID;
    ref.autoRelease = FALSE;
    KTRACE("RELEASED & UNLOADED MATERIAL '%s'",name);
  }
  KTRACE("RELEASED MATERIAL '%s'| REF: (%"PRIu64")| AR : (%d)|",name,ref.referenceCount, ref.autoRelease);
  result = hashtableSet(&state->registeredMaterialTable, name, &ref);
  KCHECK_VOID(result, "FAILED TO SET %s TO registeredMaterialTable", name)
}


b32    
loadMaterial    (Material* mat, MaterialConfig config)
{
  MEMZERO(mat);
  u8 result = FALSE;
  if(state == NULL || mat == NULL) return FALSE;
  kstrncpy(mat->name, config.name, MATERIAL_NAME_MAX_LEN);

  mat->diffuseColor = config.diffuseColor;

  if(kstrlen(config.diffuseMapName) > 0){
    mat->diffuseMap.use = TEXTURE_USE_MAP_DIFFUSE;
    mat->diffuseMap.texture = textureSystemAcquire(config.diffuseMapName, config.autoRelease);
    if(mat->diffuseMap.texture == NULL)
    {
      KERROR("FAILED TO ACQUIRE TEXTURE '%s' FOR MATERIAL '%s' , SETTING TO DEFAULT",config.diffuseMapName, config.name);
      mat->diffuseMap.texture = textureSystemGetDefaultTexture();
    }
  }else{
    mat->diffuseMap.use = TEXTURE_USE_UNKNOWN;
    mat->diffuseMap.texture = NULL;
  }

  result = rendererCreateMaterial(mat);
  KCHECK_B32(result, "FAILED TO CREATE MATERIAL '%s'", mat->name);

  return TRUE;
}

void  
destroyMaterial (Material* mat)
{
  rendererDestroyMaterial(mat);
  MEMZERO(&mat->name);
  MEMZERO(mat);
  mat->id         = INVALID_ID;
  mat->internalId = INVALID_ID;
  mat->generation = INVALID_ID;
}

b32    
createDefaultMaterial ()
{
  TRACEFUNCTION;
  MEMZERO(&state->defualtMaterial);
  kstrncpy(state->defualtMaterial.name, DEFAULT_MATERIAL_NAME, MATERIAL_NAME_MAX_LEN);
  state->defualtMaterial.diffuseMap.texture = textureSystemGetDefaultTexture();
  state->defualtMaterial.diffuseMap.use = TEXTURE_USE_MAP_DIFFUSE;
  state->defualtMaterial.diffuseColor = vec4One();
  state->defualtMaterial.internalId = INVALID_ID;
  state->defualtMaterial.id = INVALID_ID;

  b32 result = rendererCreateMaterial(&state->defualtMaterial);
  KCHECK_B32(result, "FAILED TO CREATE DEFAULT MATERIAL, EXITING APPLICATION");
  KINFO("SUCCESFULLY CREATED DEFAULT MATERIAL");
  return TRUE;
}

b32    
loadConfigFile (MaterialConfig* config, const char* path)
{
  TRACEFUNCTION;

  FileHandle  f;
  if(KFileOpen(path, FILE_MODE_READ, false, &f) != KFILE_RET_SUCCESS)
  {
    KERROR("FAILED TO OPEN FILE '%s'", path);
    return FALSE;
  }
  //TODO CHANGE THIS 
  char* buffer    = calloc(1024, sizeof(char));
  char* line      = buffer;
  char* varname   = line + 512;
  char* varvalue  = varname + 448 ;
  i64 linelen = 0;
  u64 lineNum = 1;
  while(KFileReadLine(&f, &line, 512) == KFILE_RET_SUCCESS)
  {
    char* trimmed = kstrtrim(line);
    linelen = kstrlen(trimmed);
    if(linelen < 1 || trimmed[0] == '#')    //skip blank lines and comments
    {
      lineNum++;
      continue;
    }
    i32 equalIndex  = kstrindex(trimmed, '=');    //spilt into var/value
    if(equalIndex < 0)
    {
      KWARN("potential formatting issue found in file '%s' : '=' not found, skipping line %zu", path, lineNum);
      lineNum++;
      continue;
    }
    ksubstr(varname, trimmed, 0, equalIndex);  // get value name
    char* trimmedVarName  = kstrtrim(varname);

    ksubstr(varvalue, trimmed, equalIndex + 1, -1);// gat value
    char* trimmedValue  = kstrtrim(varvalue);

    if(kstrequali(trimmedVarName, "name"))
    {
      kstrncpy(config->name,trimmedValue, 512);
    }else if(kstrequali(trimmedVarName, "diffuse_map_name"))
    {
      kstrncpy(config->diffuseMapName, trimmedValue, 512);
    }else if(kstrequali(trimmedVarName, "diffuse_color"))
    {
      if(!kstrtovec4(trimmedValue, &config->diffuseColor))
      {
        KWARN("Error parsing diffuseColor in file '%s', suing defualt of white instead", path);
        config->diffuseColor = vec4One();
      }
    }
    kzeroMemory(buffer, sizeof(char) * 1024);
    lineNum++;
  }

  Vec4* v = &config->diffuseColor;
  UINFO("name: %s", config->name);
  UINFO("diffuseMapName: %s", config->diffuseMapName);
  UINFO("x: %f, y: %f, y: %f, z: %f", v->x, v->y, v->z, v->w);
  free(buffer);
  KFileClose(&f);
  return TRUE;
}

