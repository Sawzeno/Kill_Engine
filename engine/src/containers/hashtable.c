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
hashtableCreate  (u64 elementSize, u32 elementCount, void* memory, bool isPointerType, Hashtable* outHashtable){
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

bool
hashtableSet     (Hashtable* hashtable, const char* name, void*  value){
  if(!hashtable || !name || !value){
    KERROR("hashtableSet requires hashtable, name and value to exist");
    return false;
  }
  if(hashtable->isPointerType){
    KERROR("hashtableSet not to be used with ptr types, use hashtableSetPtr method");
    return false;
  }
  if(hashtable->elementCount < 1 || hashtable->elementSize < 1){
    KERROR("elementCount or elementSize cannot be less than zero");
    return false;
  }
  u64 hash  = hashName(name, hashtable->elementCount);
  kcopyMemory(hashtable->memory + (hashtable->elementSize * hash), value, hashtable->elementSize, __FUNCTION__);
  // UDEBUG("value : %"PRIu64"",*(u64*)(hashtable->memory + (hashtable->elementSize * hash)));
  return true;
}

bool
hashtableGet     (Hashtable* hashtable, const char* name, void*  outValue){
  if(!hashtable || !name || !outValue){
    KERROR("hashtableSet requires hashtable, name and outValue to exist");
    return false;
  }
  if(hashtable->isPointerType){
    KERROR("hashtableGet not to be used with ptr types, use hashtableGetPtr method");
    return false;
  }
  if(hashtable->elementCount < 1 || hashtable->elementSize < 1){
    KERROR("elementCount or elementSize cannot be less than zero");
    return false;
  }

  u64 hash  = hashName(name, hashtable->elementCount);
  kcopyMemory(outValue, hashtable->memory + (hashtable->elementSize * hash), hashtable->elementSize, __FUNCTION__);
  // UDEBUG("outvalue : %"PRIu64"",*(u64*)(hashtable->memory + (hashtable->elementSize * hash)));
  return true;
}

bool
hashtableSetPtr  (Hashtable* hashtable, const char* name, void** value){
  if(!hashtable || !name){
    KERROR("hashtableSet requires hashtable, name and to exist");
    return false;
  }
  if(!hashtable->isPointerType){
    KERROR("hashtableSetPtr not to be used with non ptr types, use hashtableSet method");
    return false;
  }
  if(hashtable->elementCount < 1 || hashtable->elementSize < 1){
    KERROR("elementCount or elementSize cannot be less than zero");
    return false;
  }
  u64 hash  = hashName(name, hashtable->elementCount);
  ((void**)hashtable->memory)[hash] = value ? *value : 0;
  return true;
}

bool
hashtableGetPtr  (Hashtable* hashtable, const char* name, void** outValue){
  if(!hashtable || !name || !outValue){
    KERROR("hashtableSet requires hashtable, name and outValue to exist");
    return false;
  }
  if(!hashtable->isPointerType){
    KERROR("hashtableGetPtr not to be used with non-ptr types, use hashtableGet method");
    return false;
  }
  if(hashtable->elementCount < 1 || hashtable->elementSize < 1){
    KERROR("elementCount or elementSize cannot be less than zero");
    return false;
  }
  u64 hash  = hashName(name, hashtable->elementCount);
  *outValue = ((void**)hashtable->memory)[hash];
  return *outValue != 0;
}

// fills the entries in the hashtable with the given value
bool
hashtableFill    (Hashtable* hashtable, void* value){
  if(!hashtable || ! value){
    KERROR("hashtableFill requires a valid pointer to a hashtable a fill value");
    return false;
  }
  if(hashtable->isPointerType){
    KERROR("hashtable fill not be used with hashtable of ptr types");
    return false;
  }
  if(hashtable->elementCount < 1 || hashtable->elementSize < 1){
    KERROR("elementCount or elementSize cannot be less than zero");
    return false;
  }
  for( u32 i = 0; i < hashtable->elementCount; ++i){
    kcopyMemory(hashtable->memory + (hashtable->elementSize * i), value, hashtable->elementSize, __FUNCTION__);
  }
  return true;
}
