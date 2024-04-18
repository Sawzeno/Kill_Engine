#include  "darray.h"
#include  "core/kmemory.h"
#include  "defines.h"

#include  <stdlib.h>

void* _darrayCreate(u64 len , u64 stride){
  u64 headersize  = DARRAY_FIELD_LENGTH * sizeof(u64);
  u64 arraysize   = len * stride;
  u64* newarray   = kallocate(arraysize + headersize , MEMORY_TAG_DARRAY);
  MEMERR(newarray)

  newarray[DARRAY_CAPACITY] = len;
  newarray[DARRAY_LENGTH]   = 0;
  newarray[DARRAY_STRIDE]   = stride;

  return (void*)(newarray + DARRAY_FIELD_LENGTH);
}

u8    _darrayDestroy(void* array){
  ISNULL(array)
  u64*  header  = (u64*)array - DARRAY_FIELD_LENGTH; 
  free(header);
  return true;
}

u64   _darrayFieldGet(void* array, u64 field) {
  ISNULL(array)
  u64* header = (u64*)array - DARRAY_FIELD_LENGTH;
  return header[field];
}

u8    _darrayFieldSet(void* array, u64 field, u64 value) {
  ISNULL(array)
  u64* header = (u64*)array - DARRAY_FIELD_LENGTH;
  header[field] = value;
  return true;
}

void* _darrayResize(void* array){
  ISNULL(array)
  u64 length  = darrayLength(array);
  u64 stride  = darrayStride(array);

  void* temp  = _darrayCreate(DARRAY_RESIZE_FACTOR * darrayCapacity(array), darrayStride(array));
  kcopyMemory(temp, array, length);
  _darrayFieldSet(temp, DARRAY_LENGTH , length);
  _darrayDestroy(array);

  return temp;
}

void* _darrayPush(void* array , const void* valueptr){
  ISNULL(array)
  u64 length = darrayLength(array);
  u64 stride = darrayStride(array);
  if (length >= darrayCapacity(array)) {
    array = _darrayResize(array);
  }

  u64 addr = (u64)array;
  addr += (length * stride);
  kcopyMemory((void*)addr, valueptr, stride);
  _darrayFieldSet(array, DARRAY_LENGTH, length + 1);
  return array;
}

u8   _darrayPop(void* array, void* dest) {
  ISNULL(array)
  u64 length = darrayLength(array);
  u64 stride = darrayStride(array);
  u64 addr = (u64)array;
  addr += ((length - 1) * stride);
  kcopyMemory(dest, (void*)addr, stride);
  _darrayFieldSet(array, DARRAY_LENGTH, length - 1);
  return true;
}

void* _darrayPopAt(void* array, u64 index, void* dest) {
  ISNULL(array)
  u64 length = darrayLength(array);
  u64 stride = darrayStride(array);
  if (index >= length) {
    UERROR("Index outside the bounds of this array! Length: %i, index: %index", length, index);
    return array;
  }

  u64 addr = (u64)array;
  kcopyMemory(dest, (void*)(addr + (index * stride)), stride);

  // If not on the last element, snip out the entry and copy the rest inward.
  if (index != length - 1) {
    kcopyMemory(
      (void*)(addr + (index * stride)),
      (void*)(addr + ((index + 1) * stride)),
      stride * (length - index));
  }

  _darrayFieldSet(array, DARRAY_LENGTH, length - 1);
  return array;
}

void* _darrayInsertAt(void* array, u64 index, void* valueptr) {
  ISNULL(array)
  u64 length = darrayLength(array);
  u64 stride = darrayStride(array);
  if (index >= length) {
    UERROR("Index outside the bounds of this array! Length: %i, index: %index", length, index);
    return array;
  }
  if (length >= darrayCapacity(array)) {
    array = _darrayResize(array);
  }
  u64 addr = (u64)array;

  // If not on the last element, copy the rest outward.
  if (index != length - 1) {
    kcopyMemory(
      (void*)(addr + ((index + 1) * stride)),
      (void*)(addr + (index * stride)),
      stride * (length - index));
  }

  // Set the value at the index
  kcopyMemory((void*)(addr + (index * stride)), valueptr, stride);

  _darrayFieldSet(array, DARRAY_LENGTH, length + 1);
  return array;
}
