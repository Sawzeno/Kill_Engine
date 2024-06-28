#include "platform/filesystem.h"
#include "../testmanager.h"
#include "core/kmemory.h"
#include "core/logger.h"

void filesystemRegisterTests();

int main(void){
  testManagerInit();   
  filesystemRegisterTests();
  UTEST("STARTING FILE SYSTEM TESTS !");
  testManagerRunTests();
  return 0;
}

u8 openfile(){
  u8** buffer = {0};
  FileHandle  fh  = {0};
  bool result = KFileOpen("/home/gon/Developer/Kill_Engine/tests/filesystem/test.txt",FILE_MODE_READ,false,&fh);
  UTEST("%s",fh.path);
  UTEST("%d",fh.size);
  EXPECTED_AS(true,result); 
  
  return true;
}

u8 readfile(){
  FileHandle  fh  = {0};
  KFileOpen("/home/gon/Developer/Kill_Engine/tests/filesystem/test.txt",FILE_MODE_READ,false,&fh);
  u8** buffer;
  buffer  = kallocate(5 * sizeof(u8*),MEMORY_TAG_ARRAY);  
  bool result = KFileReadAllBytes(&fh, &buffer[0]);
  UTEST("%s",*buffer);
  return true;
}

u8  readline(){
  FileHandle  fh  = {0};
  KFileOpen("/home/gon/Developer/Kill_Engine/tests/filesystem/test.txt",FILE_MODE_READ,false,&fh);
  char** buffer;
  u64 bytesread = {0};
  buffer  = kallocate(5 * sizeof(u8*),MEMORY_TAG_ARRAY);  
  bool result = KFileReadLine(&fh, &buffer[0], 100);
  EXPECTED_AS(result, true);
  Uwrite(1024,"%s",buffer[0]);
  result = KFileReadLine(&fh, &buffer[1], 1024);
  EXPECTED_AS(result, true);
  Uwrite(1024,"%s",buffer[1]);
  return true;
}
u8  fileClose(){
  FileHandle  fh  = {0};
  KFileOpen("/home/gon/Developer/Kill_Engine/tests/filesystem/test.txt",FILE_MODE_READ,false,&fh);
  KFileClose(&fh);
  return true;
}

void filesystemRegisterTests(){
  testManagerRegisterTest(openfile,"KfileOpen");
  testManagerRegisterTest(readfile,"KFileReadFull");
  testManagerRegisterTest(readline,"KFileReadLine");
  testManagerRegisterTest(fileClose,"KFileClose");
}
