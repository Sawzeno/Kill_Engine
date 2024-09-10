#include  "hashtable.h"

#include  "core/kmemory.h"
#include  "core/logger.h"

u64 hashName(const char* name, u32 elementCount){
  static const u64 multiplier = 97;
  unsigned const char* us;
  u64 hash  = 0;
  for(us = (unsigned const char*)name; *us; us++){
    hash = hash * multiplier + *us;
  }
  hash %= elementCount;
  // UDEBUG("hash : %"PRIu64"",hash);
  return hash;
}

void
hashtableCreate  (u64 elementSize, u32 elementCount, void* memory, b8 isPointerType, Hashtable* outHashtable){
  if(!memory || !outHashtable){
    KERROR("hashTableCreate requires a valid pointer to memory and outHashtable");
  }
  if(!elementCount || !elementSize){
    KERROR("hashtableCreate requires a valid elementSize and elementCount");
  }

  // TODO : do this using an allocator
  outHashtable->isPointerType =  isPointerType;
  outHashtable->elementSize   = elementSize;
  outHashtable->elementCount  = elementCount;
  outHashtable->memory        = memory;
  kzeroMemory(outHashtable->memory, elementSize * elementCount);
}

void
hashtableDestroy (Hashtable* hashtable){
  if(hashtable){
    kzeroMemory(hashtable, sizeof(Hashtable));
  }else{
    KERROR("invlaid hashtable passed to destroy");
  }
}

b8
hashtableSet     (Hashtable* hashtable, const char* name, void*  value){
  if(!hashtable || !name || !value){
    KERROR("hashtableSet requires hashtable, name and value to exist");
    return FALSE;
  }
  if(hashtable->isPointerType){
    KERROR("hashtableSet not to be used with ptr types, use hashtableSetPtr method");
    return FALSE;
  }
  if(hashtable->elementCount < 1 || hashtable->elementSize < 1){
    KERROR("elementCount or elementSize cannot be less than zero");
    return FALSE;
  }
  u64 hash  = hashName(name, hashtable->elementCount);
  kcopyMemory(hashtable->memory + (hashtable->elementSize * hash), value, hashtable->elementSize, __FUNCTION__);
  return TRUE;
}

b8
hashtableGet     (Hashtable* hashtable, const char* name, void*  outValue){
  if(!hashtable || !name || !outValue){
    KERROR("hashtableSet requires hashtable, name and outValue to exist");
    return FALSE;
  }
  if(hashtable->isPointerType){
    KERROR("hashtableGet not to be used with ptr types, use hashtableGetPtr method");
    return FALSE;
  }
  if(hashtable->elementCount < 1 || hashtable->elementSize < 1){
    KERROR("elementCount or elementSize cannot be less than zero");
    return FALSE;
  }

  u64 hash  = hashName(name, hashtable->elementCount);
  kcopyMemory(outValue, hashtable->memory + (hashtable->elementSize * hash), hashtable->elementSize, __FUNCTION__);
  return TRUE;
}

b8
hashtableSetPtr  (Hashtable* hashtable, const char* name, void** value){
  if(!hashtable || !name){
    KERROR("hashtableSet requires hashtable, name and to exist");
    return FALSE;
  }
  if(!hashtable->isPointerType){
    KERROR("hashtableSetPtr not to be used with non ptr types, use hashtableSet method");
    return FALSE;
  }
  if(hashtable->elementCount < 1 || hashtable->elementSize < 1){
    KERROR("elementCount or elementSize cannot be less than zero");
    return FALSE;
  }
  u64 hash  = hashName(name, hashtable->elementCount);
  ((void**)hashtable->memory)[hash] = value ? *value : 0;
  return TRUE;
}

b8
hashtableGetPtr  (Hashtable* hashtable, const char* name, void** outValue){
  if(!hashtable || !name || !outValue){
    KERROR("hashtableSet requires hashtable, name and outValue to exist");
    return FALSE;
  }
  if(!hashtable->isPointerType){
    KERROR("hashtableGetPtr not to be used with non-ptr types, use hashtableGet method");
    return FALSE;
  }
  if(hashtable->elementCount < 1 || hashtable->elementSize < 1){
    KERROR("elementCount or elementSize cannot be less than zero");
    return FALSE;
  }
  u64 hash  = hashName(name, hashtable->elementCount);
  *outValue = ((void**)hashtable->memory)[hash];
  return *outValue != 0;
}

// fills the entries in the hashtable with the given value
b8
hashtableFill    (Hashtable* hashtable, void* value){
  if(!hashtable || ! value){
    KERROR("hashtableFill requires a valid pointer to a hashtable a fill value");
    return FALSE;
  }
  if(hashtable->isPointerType){
    KERROR("hashtable fill not be used with hashtable of ptr types");
    return FALSE;
  }
  if(hashtable->elementCount < 1 || hashtable->elementSize < 1){
    KERROR("elementCount or elementSize cannot be less than zero");
    return FALSE;
  }
  for( u32 i = 0; i < hashtable->elementCount; ++i){
    kcopyMemory(hashtable->memory + (hashtable->elementSize * i), value, hashtable->elementSize, __FUNCTION__);
  }
 return TRUE;
}
