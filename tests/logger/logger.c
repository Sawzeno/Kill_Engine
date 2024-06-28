#include "core/logger.h"
#include "../testmanager.h"

void loggerRegisterTests();

int main(void){
  testManagerInit();   
  loggerRegisterTests();
  UTEST("STARTING LINEAR ALLOCATOR TESTS !");
  testManagerRunTests();
  return 0;
}

u8 loggertesttest(){
  UINFO("LOGGER TEST !");
  return true;
}

void loggerRegisterTests(){
  testManagerRegisterTest(loggertesttest,"LOGGER TEST INIT!");
}
