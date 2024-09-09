#include "core/filesystem.h"
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

//
// b8    loadConfigurationFile (){
//   TRACEFUNCTION;
//
//   MaterialConfig config;
//   const char* path  = "/home/gon/Developer/Kill_Engine/assets/materials/test.kill";
//
//   FileHandle  f;
//   if(!KFileOpen(path, FILE_MODE_READ, false, &f)){
//     KERROR("FAILED TO OPEN FILE '%s'", path);
//     return false;
//   }
//   char* linebuf = calloc(512, sizeof(char));
//   i64 linelen = 0;
//   u64 lineNum = 1;
//   while(KFileReadLine(&f, &linebuf, 512)){
//     char* trimmed = kstrtrim(linebuf);
//     linelen = kstrlen(trimmed);
//
//     //skip blank lines and comments
//     if(linelen < 1 || trimmed[0] == '#'){
//       lineNum++;
//       continue;
//     }
//     //spilt into var/value
//     i32 equalIndex  = kstrindex(trimmed, '=');
//     if(equalIndex == -1){
//       KWARN("potential formatting issue found in file '%s' : '=' not found, skipping line %zu", path, lineNum);
//       lineNum++;
//       continue;
//     }
//     char rawVarName[64]=  {0};
//     ksubstr(rawVarName, trimmed, 0, equalIndex);
//     char* trimmedVarName  = kstrtrim(rawVarName);
//
//     char rawValue[446];
//     kzeroMemory(rawValue, sizeof(char) * 446);
//     ksubstr(rawValue, trimmed, equalIndex + 1, -1);
//     char* trimmedValue  = kstrtrim(rawValue);
//
//     //process the value
//     if(kstrequal(trimmedVarName, "version")){
//       //TODO
//     }else if(kstrequali(trimmedVarName, "name")){
//       kstrncpy(config.name,trimmedValue, 512);
//     }else if(kstrequali(trimmedVarName, "diffuseMapName")){
//       kstrncpy(config.diffuseMapName, trimmedValue, 512);
//     }else if(kstrequali(trimmedVarName, "diffuseColor")){
//       if(!kstrtovec4(trimmedValue, &config.diffuseColor)){
//         KWARN("Error parsing diffuseColor in file '%s', suing defualt of white instead", path);
//         config.diffuseColor = vec4One();
//       }
//     }
//     // TODO more fields
//     // clear the line buffer
//     kzeroMemory(linebuf, sizeof(char) * 512);
//     lineNum++;
//   }
//
//   Vec4* v = &config.diffuseColor;
//   UINFO("name: %s", config.name);
//   UINFO("diffuseMapName: %s", config.diffuseMapName);
//   UINFO("autoRelease : %d", config.autoRelease);
//   UINFO("x: %f, y: %f, y: %f, z: %f", v->x, v->y, v->z, v->w);
//   free(linebuf);
//   KFileClose(&f);
//   return true;
// }


int main(void){
  testManagerInit();   

  testManagerRegisterTest(openFile,"KfileOpen");
  testManagerRegisterTest(readFile,"KFileReadFull");
  testManagerRegisterTest(readLine,"KFileReadLine");
  testManagerRegisterTest(closeFile,"KFileClose");
  // testManagerRegisterTest(loadConfigurationFile,"loadConfigurationFile");

  testManagerRunTests();
  return 0;
}


