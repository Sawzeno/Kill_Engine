#pragma once

#include  "stdlib.h"
#include  <stdbool.h>
#include  <inttypes.h>

typedef uint64_t  u64;
typedef uint32_t  u32;
typedef uint16_t  u16;
typedef uint8_t   u8;

typedef int64_t   i64;
typedef int32_t   i32;
typedef int16_t   i16;
typedef int8_t    i8;

typedef float     f32;
typedef double    f64;

typedef int32_t   b32;

#define INVALID_ID (4294967295U)
#define FALSE (i32)0
#define TRUE  (i32)1

#define KCLAMP(value, min, max) (value <= min) ? min : (value >= max) ? max : value

#define KCHECK_B32(result, message, ...)\
  if (result == FALSE){                 \
  UERROR(message,##__VA_ARGS__);        \
  return 0;                         \
}
#define KCHECK_VOID(result, message, ...) \
  if (result == FALSE){                   \
  UERROR(message,##__VA_ARGS__);          \
  return;                                 \
}
