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
  DARRAY_FILED_CAPACITY,
  DARRAY_FILED_LENGTH,
  DARRAY_FILED_STRIDE,
  DARRAY_FILED_FIELD_LENGTH
};

void* _darrayCreate   (u64 length, u64 stride);
bool  _darrayDestroy  (void* darray);

u64   _darrayFieldGet (void* darray, u64 field);
bool  _darrayFieldSet (void* darray, u64 field, u64 value);

void* _darrayResize   (void* darray);

void* _darrayPush     (void* darray, const void* valueptr);
bool  _darrayPop      (void* darray, void* dest);

void* _darrayPopAt    (void* darray, u64 index, void* dest);
void* _darrayInsertAt (void* darray, u64 index, void* valueptr);

#define DARRAY_FILED_DEFAULT_CAPACITY 1
#define DARRAY_FILED_RESIZE_FACTOR    2

#if LOG_DEBUG_ENABLED == 1

#define DARRAY_CREATE(type) \
_darrayCreate(DARRAY_FILED_DEFAULT_CAPACITY, sizeof(type));\
MINFO("func : %s",__FUNCTION__)

#define DARRAY_RESERVE(type, capacity) \
_darrayCreate(capacity, sizeof(type));\
MINFO("func : %s",__FUNCTION__)

#define DARRAY_DESTROY(array) \
_darrayDestroy(array);  \
MINFO("func : %s",__FUNCTION__)

#define DARRAY_PUSH(array, value) \
{                                 \
  typeof(value) temp = value;       \
  array = _darrayPush(array, &temp); \
  MINFO("func : %s",__FUNCTION__);\
}

#define DARRAY_POP(array, valuePtr) \
_darrayPop(array, valuePtr);\
MINFO("func : %s",__FUNCTION__)

#define DARRAY_INSERTAT(array, index, value)         \
{                                               \
  typeof(value) temp = value;                 \
  array = _darrayInsertAt(array, index, &temp);\
  MINFO("func : %s",__FUNCTION__);\
}

#define DARRAY_POPAT(array, index, valuePtr) \
_darrayPopAt(array, index, valuePtr);\
MINFO("func : %s",__FUNCTION__)

#define DARRAY_CLEAR(array) \
_darrayFieldSet(array, DARRAY_FILED_LENGTH, 0);\
MINFO("func : %s",__FUNCTION__)

#define DARRAY_LENGTHSET(array, value) \
_darrayFieldSet(array, DARRAY_FILED_LENGTH, value);\
MINFO("func : %s",__FUNCTION__)

#define DARRAY_CAPACITY(array) \
_darrayFieldGet(array, DARRAY_FILED_CAPACITY)\

#define DARRAY_LENGTH(array) \
_darrayFieldGet(array, DARRAY_FILED_LENGTH)\

#define DARRAY_STRIDE(array) \
_darrayFieldGet(array , DARRAY_FILED_STRIDE)\

#else
#define DARRAY_CREATE(type) \
_darrayCreate(DARRAY_FILED_DEFAULT_CAPACITY, sizeof(type))

#define DARRAY_RESERVE(type, capacity) \
_darrayCreate(capacity, sizeof(type))

#define DARRAY_DESTROY(array) \
_darrayDestroy(array);  \

#define DARRAY_PUSH(array, value) \
{                                 \
  typeof(value) temp = value;       \
  array = _darrayPush(array, &temp); \
}                                   \

#define DARRAY_POP(array, valuePtr) \
_darrayPop(array, valuePtr)

#define DARRAY_INSERTAT(array, index, value)         \
{                                               \
  typeof(value) temp = value;                 \
  array = _darrayInsertAt(array, index, &temp);\
}

#define DARRAY_POPAT(array, index, valuePtr) \
_darrayPopAt(array, index, valuePtr)

#define DARRAY_CLEAR(array) \
_darrayFieldSet(array, DARRAY_FILED_LENGTH, 0)

#define DARRAY_LENGTHSET(array, value) \
_darrayFieldSet(array, DARRAY_FILED_LENGTH, value)

#define DARRAY_CAPACITYSET(array, value) \
_darrayFieldSet(array, DARRAY_FILED_CAPACITY, value)

#define DARRAY_CAPACITY(array) \
_darrayFieldGet(array, DARRAY_FILED_CAPACITY)

#define DARRAY_LENGTH(array) \
_darrayFieldGet(array, DARRAY_FILED_LENGTH)

#define DARRAY_STRIDE(array) \
_darrayFieldGet(array , DARRAY_FILED_STRIDE)

#endif


