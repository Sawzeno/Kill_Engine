#include  "linearallocator.h"
#include  "defines.h"
#include  "core/kmemory.h"

void linearAllocatorCreate(u64 totalSize, void* memory, LinearAllocator* outAllocator){
  if(outAllocator){
    outAllocator->totalSize = totalSize;
    outAllocator->allocated = 0;
    outAllocator->ownsMemory= memory == 0;
    if(memory){
      outAllocator->memory  = memory;
    }else{
      outAllocator->memory  = kallocate(totalSize, MEMORY_TAG_LINEAR_ALLOCATOR);
    }
  }
}

void linearAllocatorDestroy(LinearAllocator* allocator){
  if(allocator){
    allocator->allocated   = 0;
    if(allocator->ownsMemory && allocator->memory){
      kfree(allocator->memory, allocator->totalSize, MEMORY_TAG_LINEAR_ALLOCATOR);
    }
    allocator->memory    = 0;
    allocator->totalSize = 0;
    allocator->ownsMemory=false;
  }
}

void* linearAllocatorAllocate(LinearAllocator* allocator, u64 size){
  if(allocator && allocator->memory){
    if(allocator->allocated + size > allocator->totalSize){
      u64 remaining = allocator->totalSize - allocator->allocated;
      UFATAL("linear allocator tried to allcoate %llu bytes, only %llu remaining", size, remaining);
      return NULL;
    }
    void* block = allocator->memory + allocator->allocated;
    allocator->allocated  += size;
    return block;
  }

  UERROR("LINEAR ALLOCATOR FAILED , PROBABLY NOT INITALIZED");
  return NULL;
}

void linearAllocatorFreeAll(LinearAllocator* allocator){
  if(allocator && allocator->memory){
    allocator->allocated= 0;
    kzeroMemory(allocator->memory, allocator->totalSize);
  }
}
