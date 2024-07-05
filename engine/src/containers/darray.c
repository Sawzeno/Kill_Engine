#include  "darray.h"

#include  "core/kmemory.h"
#include  "core/logger.h"

void*
_darrayCreate(u64 cap , u64 stride)
{
  TRACEMEMORY;
  MDEBUG("cap  : %"PRIu64" stride : %"PRIu64"");
  u64 headersize  = DARRAY_FILED_FIELD_LENGTH * sizeof(u64);
  u64 arraysize   = cap * stride;

  u64* darray   = kallocate(arraysize + headersize , MEMORY_TAG_DARRAY);

  darray[DARRAY_FILED_CAPACITY] = cap;
  darray[DARRAY_FILED_LENGTH]   = 0;
  darray[DARRAY_FILED_STRIDE]   = stride;

  u64* arr  = darray + DARRAY_FILED_FIELD_LENGTH;
  MDEBUG("CREATED: %p --> %p len  :%"PRIu64" cap : %"PRIu64" stride : %"PRIu64"",darray,arr,DARRAY_LENGTH(arr),DARRAY_CAPACITY(arr),DARRAY_STRIDE(arr));
  return (void*)(arr);
}

void*
_darrayResize(void* darray)
{
  TRACEMEMORY;
  MDEBUG("darray  : %p",darray);
  ISNULL(darray ,NULL);

  u64 oldlen    = DARRAY_LENGTH  (darray);
  u64 oldcap    = DARRAY_CAPACITY(darray);
  u64 oldstride = DARRAY_STRIDE  (darray);
  u64 newcap    = oldcap * DARRAY_FILED_RESIZE_FACTOR;
  u64* oldhead  = (u64*)darray - DARRAY_FILED_FIELD_LENGTH;
  //ALLOCATE
  u64 headersize    = DARRAY_FILED_FIELD_LENGTH * sizeof(u64);
  u64 newarraysize  = newcap * oldstride;
  u64* newhead    = kallocate(headersize + newarraysize, MEMORY_TAG_DARRAY);
  //COPY HEADER
  MDEBUG("oldhead contains [0]: %"PRIu64" [1] :%"PRIu64" [2] :%"PRIu64"", *((u64*)oldhead),*((u64*)oldhead+1),*((u64*)oldhead+2));
  kcopyMemory(newhead, oldhead, headersize, __FUNCTION__);
  newhead[DARRAY_FILED_CAPACITY]= newcap;
  newhead[DARRAY_FILED_LENGTH]  = oldlen;
  newhead[DARRAY_FILED_STRIDE]  = oldstride;
  MDEBUG("newhead contains [0]: %"PRIu64" [1] :%"PRIu64" [2] :%"PRIu64"", *((u64*)newhead),*((u64*)newhead+1),*((u64*)newhead+2));
  //COPY BODY
  u64 newbody = (u64)(newhead);
  newbody+= (headersize);
  u64 oldbody = (u64)(darray);
  kcopyMemory((void*)newbody, (void*)oldbody, oldlen * oldstride, __FUNCTION__);

  MDEBUG("RESIZE RETURNED : %p", (u64*)oldhead +DARRAY_FILED_FIELD_LENGTH);
  return (void*)newbody;
}

void*
_darrayPush(void* darray , const void* valueptr)
{
  TRACEMEMORY;
  MDEBUG("darray : %p valueptr : %p",darray,valueptr);
  ISNULL(darray , NULL);
  u64 length    =   DARRAY_LENGTH  (darray);
  u64 stride    =   DARRAY_STRIDE  (darray);
  u64 cap       =   DARRAY_CAPACITY(darray);
  if (length >= cap) {
    darray = _darrayResize(darray);
  }
  u64 addr = (u64)darray;
  addr += (length * stride );
  kcopyMemory((void*)addr, valueptr, stride, __FUNCTION__);
  _darrayFieldSet(darray, DARRAY_FILED_LENGTH, ++length);

  length    = DARRAY_LENGTH  (darray);
  stride    = DARRAY_STRIDE  (darray);
  cap       = DARRAY_CAPACITY(darray);

  MDEBUG("PUSHED len  :%"PRIu64" cap : %"PRIu64" stride : %"PRIu64"",length,cap,stride);
  return darray;
}

bool  
_darrayPop(void* darray, void* dest)
{
  TRACEMEMORY;
  MDEBUG("darray : %p  dest  : %p",darray,dest);
  ISNULL(darray, false);

  u64 length = DARRAY_LENGTH(darray);
  u64 stride = DARRAY_STRIDE(darray);
  if (length == 0) {
    KERROR("Pop called on empty darray!");
    return false;
  }

  u64 addr = (u64)darray + ((length - 1) * stride);
  kcopyMemory(dest, (void*)addr, stride, __FUNCTION__);
  _darrayFieldSet(darray, DARRAY_FILED_LENGTH, length - 1);

  MDEBUG("POPPED: len: %" PRIu64 " cap: %" PRIu64 " stride: %" PRIu64 "", DARRAY_LENGTH(darray), DARRAY_CAPACITY(darray), stride);
  return true;
}

void*
_darrayPopAt(void* darray, u64 index, void* dest)
{
  TRACEMEMORY;
  MDEBUG("darray : %p  index : %"PRIu64" dest  : %p",darray,index,dest);
  ISNULL(darray, NULL);

  u64 length = DARRAY_LENGTH(darray);
  u64 stride = DARRAY_STRIDE(darray);
  if (index >= length) {
    KERROR("Index outside the bounds of this darray! Length: %" PRIu64 ", index: %" PRIu64 "", length, index);
    return darray;
  }

  u64 addr = (u64)darray + (index * stride);
  kcopyMemory(dest, (void*)addr, stride, __FUNCTION__);

  if (index != length - 1) {
    kcopyMemory(
      (void*)(addr),
      (void*)(addr + stride),
      stride * (length - index - 1),
      __FUNCTION__
    );
  }

  _darrayFieldSet(darray, DARRAY_FILED_LENGTH, length - 1);
  MDEBUG("POPPED AT: len: %" PRIu64 " cap: %" PRIu64 " stride: %" PRIu64 "", DARRAY_LENGTH(darray), DARRAY_CAPACITY(darray), stride);
  return darray;
}


void*
_darrayInsertAt(void* darray, u64 index, void* valueptr)
{
  TRACEMEMORY;
  MDEBUG("darray : %p  index : %"PRIu64" valueptr  : %p",darray,index,valueptr);
  ISNULL(darray , NULL);
  u64 length    = DARRAY_LENGTH  (darray);
  u64 stride    = DARRAY_STRIDE  (darray);
  u64 capacity  = DARRAY_CAPACITY(darray);
  if (index >= length) {
    UERROR("Index outside the bounds of this darray! Length: %i, index: %index", length, index);
    return darray;
  }
  if (length >= capacity) {
    darray = _darrayResize(darray);
  }
  u64 addr = (u64)darray;

  // If not on the last element, copy the rest outward.
  if (index != length - 1) {
    kcopyMemory(
      (void*)(addr + ((index + 1) * stride)),
      (void*)(addr + (index * stride)),
      stride * (length - index), __FUNCTION__);
  }

  // Set the value at the index
  kcopyMemory((void*)(addr + (index * stride)), valueptr, stride, __FUNCTION__);

  _darrayFieldSet(darray, DARRAY_FILED_LENGTH, length + 1);
  MDEBUG("INSERTED AT: len: %" PRIu64 " cap: %" PRIu64 " stride: %" PRIu64 "", DARRAY_LENGTH(darray), DARRAY_CAPACITY(darray), stride);
  return darray;
}

bool
_darrayDestroy(void* darray)
{
  ISNULL(darray , false);
  u64*  header  = (u64*)darray - DARRAY_FILED_FIELD_LENGTH; 
  free(header);
  MDEBUG("DESTROYED : %p --> %p",header,darray);
  return true;
}

u64
_darrayFieldGet(void* darray, u64 field)
{
  ISNULL(darray , 0);
  u64* header = (u64*)darray - DARRAY_FILED_FIELD_LENGTH;
  return header[field];
}

bool
_darrayFieldSet(void* darray, u64 field, u64 value)
{
  ISNULL(darray , false);
  u64* header = (u64*)darray - DARRAY_FILED_FIELD_LENGTH;
  header[field] = value;
  return true;
}



