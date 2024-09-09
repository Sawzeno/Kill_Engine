#include "../testmanager.h"
#include "core/kstring.h"
u8 test1(){
  
  char str[] =  "            hello this is mario                                                                ";
  UTEST(kstrtrim(str));
  return true;
}

u8 test2(){
  char str[] = "hello earth";

  EXPECTED_AS(1, kstrindex(str, 'e'));
  EXPECTED_AS(-1, kstrindex(str, 'z'));

  return true;
}

u8 test3(){
  char* fmt = "assets/materials/%s.%s";
  char path[512] = {0};

 kstrfmt(path, fmt, "test", "kill");
  UINFO(path);
  return true;
}

int main(void){
  testManagerInit();   

 testManagerRegisterTest(test1, "kstrtrim");
 testManagerRegisterTest(test2, "kstrindex");
 testManagerRegisterTest(test3, "kstrfmt");
  
  UTEST("STARTING KSTRING TESTS !");
  testManagerRunTests();
  return 0;
}


