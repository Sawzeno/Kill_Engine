#pragma once

#include "defines.h"

typedef struct  LinearAllocator LinearAllocator;

struct LinearAllocator{
  void* memory;
  u64   totalSize;
  u64   allocated;
  u64   ownsMemory;
};

void linearAllocatorCreate(u64 totalSize, void* memory, LinearAllocator* outAllocator);
void linearAllocatorDestroy(LinearAllocator* allocator);

void* linearAllocatorAllocate(LinearAllocator* allocator, u64 size);
void linearAllocatorFreeAll(LinearAllocator* allocator);
