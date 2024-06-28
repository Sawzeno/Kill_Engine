#include  "memory/linearallocator.h"
#include  "core/logger.h"
#include  "../testmanager.h"

void  linearAllocatorRegisterTests();
int main(void){
  testManagerInit();   
  linearAllocatorRegisterTests();
  UTEST("STARTING LINEAR ALLOCATOR TESTS !");
  testManagerRunTests();
  return 0;
}

u8 CreateAndDestroy(){
  LinearAllocator alloc;
  linearAllocatorCreate(sizeof(u64), 0, &alloc);
  NOT_EXPECTED_AS(0, alloc.memory);
  EXPECTED_AS(sizeof(u64), alloc.totalSize);
  EXPECTED_AS(0, alloc.allocated);

  linearAllocatorDestroy(&alloc);
  EXPECTED_AS(0, alloc.memory);
  EXPECTED_AS(0, alloc.totalSize);
  EXPECTED_AS(0, alloc.allocated);

  return true;
}

u8 SingleAllocation(){
  u64 maxallocs = 1;
  LinearAllocator alloc;
  linearAllocatorCreate(sizeof(u64) * 1, NULL, &alloc);

  void* block = linearAllocatorAllocate(&alloc, sizeof(u64));
  EXPECTED_AS(sizeof(u64) , alloc.allocated);
  NOT_EXPECTED_AS(block, NULL);
  linearAllocatorDestroy(&alloc);
  return true;

}
u8 MultiAllocation(){
  u64 maxAllocs = 1024;
  LinearAllocator alloc;
  linearAllocatorCreate(sizeof(u64) * maxAllocs, NULL, &alloc);

  void* block;
  for( u64 i = 0; i < maxAllocs; ++i){
    block = linearAllocatorAllocate(&alloc, sizeof(u64));
    NOT_EXPECTED_AS(0, block);
    EXPECTED_AS(sizeof(u64) * (i+1) , alloc.allocated);
  }
  linearAllocatorDestroy(&alloc);
  return true;
}

u8 overAllocate(){
  LinearAllocator alloc;
  u8 maxallocs = 4;
  linearAllocatorCreate(maxallocs * sizeof(u64), NULL, &alloc);
  for( u8 i = 0; i < maxallocs; ++i){
    linearAllocatorAllocate(&alloc, sizeof(u64));
  }

  EXPECTED_AS(maxallocs* sizeof(u64), alloc.allocated);
  UWARN("FOLLOWING ERROR IS INTENTIONAL !");
  EXPECTED_AS(NULL,linearAllocatorAllocate(&alloc, sizeof(u64)));
  EXPECTED_AS(maxallocs* sizeof(u64), alloc.allocated);
  linearAllocatorDestroy(&alloc);
  return true;
}

u8 freeAll(){
  u64 maxallocs = 1024;
  LinearAllocator alloc;

  linearAllocatorCreate(maxallocs * sizeof(u64), NULL, &alloc);
  for( u64 i = 0; i < maxallocs; ++i){
    linearAllocatorAllocate(&alloc, sizeof(u64));
  }

  linearAllocatorFreeAll(&alloc);
  EXPECTED_AS(0, alloc.allocated);
  return true;
}

void linearAllocatorRegisterTests(){
  testManagerRegisterTest(CreateAndDestroy, "CREATE AND DESTROY");
  testManagerRegisterTest(SingleAllocation,"SINGLE ALLOCATION TESTING");
  testManagerRegisterTest(MultiAllocation,"MULTI ALLOCATION TESTING");
  testManagerRegisterTest(overAllocate,"OVER ALLOCATION TESTING");
  testManagerRegisterTest(freeAll,"FREE ALL TESTING");

  
}
