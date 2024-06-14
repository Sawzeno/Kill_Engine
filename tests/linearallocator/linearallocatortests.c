#include "memory/linearallocator.h"
#include  "../testmanager.h"
#include "linearallocatortests.h"

int main(void){
  testManagerInit();   

  linearAllocatorRegisterTests();
  UTEST("STARTING TESTS !");
  testManagerRunTests();
  return 0;
}
