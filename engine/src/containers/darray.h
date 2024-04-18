#pragma once

#include "defines.h"

/*
                                    Memory layout
                u64 capacity  = number elements that can be held
                u64 length    = number of elements currently contained
                u64 stride    = size of each element in bytes
                void* elements
*/

enum {
  DARRAY_CAPACITY,
  DARRAY_LENGTH,
  DARRAY_STRIDE,
  DARRAY_FIELD_LENGTH
};

void* _darrayCreate   (u64 length, u64 stride);
u8    _darrayDestroy  (void* array);

u64   _darrayFieldGet (void* array, u64 field);
u8    _darrayFieldSet (void* array, u64 field, u64 value);

void* _darrayResize   (void* array);

void* _darrayPush     (void* array, const void* valueptr);
u8    _darrayPop      (void* array, void* dest);

void* _darrayPopAt    (void* array, u64 index, void* dest);
void* _darrayInsertAt (void* array, u64 index, void* valueptr);

#define DARRAY_DEFAULT_CAPACITY 1
#define DARRAY_RESIZE_FACTOR    2

#define darrayCreate(type) \
  _darrayCreate(DARRAY_DEFAULT_CAPACITY, sizeof(type))

#define darrayReserve(type, capacity) \
_darrayCreate(capacity, sizeof(type))

#define darrayDestroy(array) \
    _darrayDestroy(array);  \

#define darrayPush(array, value) \
  typeof(value) temp = value;       \
  array = _darrayPush(array, &temp); \


#define darrayPop(array, valuePtr) \
_darrayPop(array, valuePtr)

#define darrayInsertAt(array, index, value)         \
{                                               \
  typeof(value) temp = value;                 \
  array = _darrayInsertAt(array, index, &temp);\
}

#define darrayPopAt(array, index, valuePtr) \
_darrayPopAt(array, index, valuePtr)

#define darrayClear(array) \
_darrayFieldSet(array, DARRAY_LENGTH, 0)

#define darrayLengthSet(array, value) \
_darrayFieldSet(array, DARRAY_LENGTH, value)

#define darrayCapacity(array) \
_darrayFieldGet(array, DARRAY_CAPACITY)

#define darrayLength(array) \
_darrayFieldGet(array, DARRAY_LENGTH)

#define darrayStride(array) \
_darrayFieldGet(array , DARRAY_STRIDE)
