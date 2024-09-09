#include "../testmanager.h"

u8 test(){
  UTEST("ADD TEST!");
  return true;
}

int main(void){
  testManagerInit();   

  testManagerRegisterTest(test, "info about test");
  
  UTEST("STARTING LINEAR ALLOCATOR TESTS !");
  testManagerRunTests();
  return 0;
}


