#include "core/filesystem.h"
#include "core/kstring.h"
#include "math/kmath.h"
#include "systems/materialsystem.h"
#include "testmanager.h"
#include "core/kmemory.h"

void filesystemRegisterTests();

u8 openFile(){
  u8** buffer = {0};
  FileHandle  fh  = {0};
  bool result = KFileOpen("/home/gon/Developer/Kill_Engine/tests/filesystem/test.txt",FILE_MODE_READ,false,&fh);
  EXPECTED_AS(KFILE_RET_SUCCESS,result);

  UTEST("%s",fh.path);
  UTEST("%d",fh.size);

  return true;
}

u8  closeFile(){
  FileHandle  fh  = {0};
  KFileOpen("/home/gon/Developer/Kill_Engine/tests/filesystem/test.txt",FILE_MODE_READ,false,&fh);
  KFileClose(&fh);
  return true;
}

u8 readFile(){
  FileHandle  fh  = {0};
  bool result = KFileOpen("/home/gon/Developer/Kill_Engine/tests/filesystem/test.txt",FILE_MODE_READ,false,&fh);

  u8* buffer = kallocate(fh.size * sizeof(u8),MEMORY_TAG_ARRAY);  
  KFILE_RET r = KFileReadAllBytes(&fh, &buffer);

  EXPECTED_AS(KFILE_RET_SUCCESS, r);
  return true;
}

u8  readLine(){
  FileHandle  fh  = {0};
  KFileOpen("/home/gon/Developer/Kill_Engine/tests/filesystem/test.txt",FILE_MODE_READ,false,&fh);

  char* buffer;
  buffer  = kallocate( fh.size * sizeof(u8),MEMORY_TAG_ARRAY);  
  KFileReadLine(&fh, &buffer, 64);
  UTEST("%s", buffer);

  return true;
}


u8 loadConfigurationFile (){

  MaterialConfig config;
  const char* path  = "/home/gon/Developer/Kill_Engine/assets/materials/test.kill";

  FileHandle  f;
  if(KFileOpen(path, FILE_MODE_READ, false, &f) != KFILE_RET_SUCCESS)
  {
    UTEST("FAILED TO OPEN FILE '%s'", path);
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
      kstrncpy(config.name,trimmedValue, 512);
    }else if(kstrequali(trimmedVarName, "diffuse_map_name"))
    {
      kstrncpy(config.diffuseMapName, trimmedValue, 512);
    }else if(kstrequali(trimmedVarName, "diffuse_color"))
    {
      if(!kstrtovec4(trimmedValue, &config.diffuseColor))
      {
        KWARN("Error parsing diffuseColor in file '%s', suing defualt of white instead", path);
        config.diffuseColor = vec4One();
      }
    }
    kzeroMemory(buffer, sizeof(char) * 1024);
    lineNum++;
  }

  Vec4* v = &config.diffuseColor;
  UINFO("name: %s", config.name);
  UINFO("diffuseMapName: %s", config.diffuseMapName);
  UINFO("autoRelease : %d", config.autoRelease);
  UINFO("x: %f, y: %f, y: %f, z: %f", v->x, v->y, v->z, v->w);
  free(buffer);
  KFileClose(&f);

  return TRUE;
}


int main(void){
  testManagerInit();   

  // testManagerRegisterTest(openFile,"KfileOpen");
  // testManagerRegisterTest(readFile,"KFileReadFull");
  // testManagerRegisterTest(readLine,"KFileReadLine");
  // testManagerRegisterTest(closeFile,"KFileClose");

  testManagerRegisterTest(loadConfigurationFile,"loadConfigurationFile");
  testManagerRunTests();
  return 0;
}


