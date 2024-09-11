#pragma once

#include "defines.h"
#include "math/mathtypes.h"

#define MAX_STR_LEN 1024

i64   kstrlen   (const char* str);

char* kstrdup   (const char* str);

b32  kstrequal (const char* str0, const char* str1);

b32  kstrequali(const char* str0, const char* str1);

i64   kstrfmt   (char* dst, const char* fmt, ...);

i64   kstrfmtv  (char* dst, const char* fmt, void* va_list);

char* kstrcpy   (char* dst, const char* src);

char* kstrncpy  ( char* dst,const char* src, u64 n);

char* kstrtrim  (char* str);

i64   kstrindex (const char* str, char c);

void  ksubstr   (char* dst, const char* src, i32 start, i32 length);

b32  kstrtovec4(char* str,Vec4* v);
b32  kstrtovec3(char* str,Vec3* v);
b32  kstrtovec2(char* str,Vec2* v);

b32  kstrtof32 (char* str, f32* f);
b32  kstrtof64 (char* str, f64* f);

b32  kstrtoi64 (char* str, i64* i);
b32  kstrtoi32 (char* str, i32* i);
b32  kstrtoi16 (char* str, i16* i);
b32  kstrtoi8  (char* str , i8* i);

b32  kstrtou64 (char* str, u64* u);
b32  kstrtou32 (char* str, u32* u);
b32  kstrtou16 (char* str, u16* u);
b32  kstrtou8  (char* str , u8* u);

b32  kstrtob32(char* str,b32* b);
