#pragma once

#include  "defines.h"
#include "resource/resourcetypes.h"

#define DEFAULT_MATERIAL_NAME "default"

typedef struct MaterialSystemConfig MaterialSystemConfig;
typedef struct MaterialConfig       MaterialConfig;

struct MaterialSystemConfig{
  u64 maxMaterialCount;
};

struct MaterialConfig{
  char  name[MATERIAL_NAME_MAX_LEN];
  char  diffuseMapName[TEXTURE_NAME_MAX_LEN];
  Vec4  diffuseColor;
  b32    autoRelease;
};

b32       materialSystemInitailize(u64* memReq, void* stateptr, MaterialSystemConfig config);
void      materialSystemShutdown  ();

Material* materialSystemAcquireResources   (const char* name);
void      materialSystemReleaseResources   (const char* name);
Material* materialSystemAcquireFromConfig(MaterialConfig config);
