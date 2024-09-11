#pragma once

#include "defines.h"

typedef struct Hashtable Hashtable;

struct Hashtable{
  u64   elementSize;
  void* memory;
  u32   elementCount;
  b32  isPointerType;
};

void hashtableCreate  (u64 elementSize, u32 elementCount, void* memory, b32 isPointerType, Hashtable* outHashtable);

void hashtableDestroy (Hashtable* hashtable);

b32 hashtableSet     (Hashtable* hashtable, const char* name, void*  value);

b32 hashtableGet     (Hashtable* hashtable, const char* name, void*  outValue);

b32 hashtableSetPtr  (Hashtable* hashtable, const char* name, void** value);

b32 hashtableGetPtr  (Hashtable* hashtable, const char* name, void** outValue);

// fills the entries in the hashtable with the given value
b32 hashtableFill    (Hashtable* hashtable, void* value);
