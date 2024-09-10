#pragma once

#include "defines.h"

typedef struct Hashtable Hashtable;

struct Hashtable{
  u64   elementSize;
  void* memory;
  u32   elementCount;
  b8  isPointerType;
};

void hashtableCreate  (u64 elementSize, u32 elementCount, void* memory, b8 isPointerType, Hashtable* outHashtable);

void hashtableDestroy (Hashtable* hashtable);

b8 hashtableSet     (Hashtable* hashtable, const char* name, void*  value);

b8 hashtableGet     (Hashtable* hashtable, const char* name, void*  outValue);

b8 hashtableSetPtr  (Hashtable* hashtable, const char* name, void** value);

b8 hashtableGetPtr  (Hashtable* hashtable, const char* name, void** outValue);

// fills the entries in the hashtable with the given value
b8 hashtableFill    (Hashtable* hashtable, void* value);
