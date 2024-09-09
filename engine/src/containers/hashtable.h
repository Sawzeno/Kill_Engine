#pragma once

#include "defines.h"

typedef struct Hashtable Hashtable;

struct Hashtable{
  u64   elementSize;
  void* memory;
  u32   elementCount;
  bool  isPointerType;
};

void hashtableCreate  (u64 elementSize, u32 elementCount, void* memory, bool isPointerType, Hashtable* outHashtable);

void hashtableDestroy (Hashtable* hashtable);

bool hashtableSet     (Hashtable* hashtable, const char* name, void*  value);

bool hashtableGet     (Hashtable* hashtable, const char* name, void*  outValue);

bool hashtableSetPtr  (Hashtable* hashtable, const char* name, void** value);

bool hashtableGetPtr  (Hashtable* hashtable, const char* name, void** outValue);

// fills the entries in the hashtable with the given value
bool hashtableFill    (Hashtable* hashtable, void* value);
