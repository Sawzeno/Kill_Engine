#pragma once

#include "defines.h"
#include "math/mathtypes.h"

#define MAX_STR_LEN 1024

i64   kstrlen   (const char* str);

char* kstrdup   (const char* str);

b8  kstrequal (const char* str0, const char* str1);

b8  kstrequali(const char* str0, const char* str1);

i64   kstrfmt   (char* dst, const char* fmt, ...);

i64   kstrfmtv  (char* dst, const char* fmt, void* va_list);

char* kstrcpy   (char* dst, const char* src);

char* kstrncpy  ( char* dst,const char* src, u64 n);

char* kstrtrim  (char* str);

i64   kstrindex (const char* str, char c);

void  ksubstr   (char* dst, const char* src, i32 start, i32 length);

b8  kstrtovec4(char* str,Vec4* v);
b8  kstrtovec3(char* str,Vec3* v);
b8  kstrtovec2(char* str,Vec2* v);

b8  kstrtof32 (char* str, f32* f);
b8  kstrtof64 (char* str, f64* f);

b8  kstrtoi64 (char* str, i64* i);
b8  kstrtoi32 (char* str, i32* i);
b8  kstrtoi16 (char* str, i16* i);
b8  kstrtoi8  (char* str , i8* i);

b8  kstrtou64 (char* str, u64* u);
b8  kstrtou32 (char* str, u32* u);
b8  kstrtou16 (char* str, u16* u);
b8  kstrtou8  (char* str , u8* u);

b8  kstrtob8(char* str,b8* b);
