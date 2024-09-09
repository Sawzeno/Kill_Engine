#pragma once

#include "defines.h"
#include "math/mathtypes.h"

i64   kstrlen   (const char* str);

char* kstrdup   (const char* str);

bool  kstrequal (const char* str0, const char* str1);

bool  kstrequali(const char* str0, const char* str1);

i64   kstrfmt   (char* dst, const char* fmt, ...);

i64   kstrfmtv  (char* dst, const char* fmt, void* va_list);

char* kstrcpy   (char* dst, const char* src);

char* kstrncpy  ( char* dst,const char* src, u64 n);

char* kstrtrim  (char* str);

i64   kstrindex (const char* str, char c);

void  ksubstr   (char* dst, const char* src, i32 start, i32 length);

bool  kstrtovec4(char* str,Vec4* v);
bool  kstrtovec3(char* str,Vec3* v);
bool  kstrtovec2(char* str,Vec2* v);

bool  kstrtof32 (char* str, f32* f);
bool  kstrtof64 (char* str, f64* f);

bool  kstrtoi64 (char* str, i64* i);
bool  kstrtoi32 (char* str, i32* i);
bool  kstrtoi16 (char* str, i16* i);
bool  kstrtoi8  (char* str , i8* i);

bool  kstrtou64 (char* str, u64* u);
bool  kstrtou32 (char* str, u32* u);
bool  kstrtou16 (char* str, u16* u);
bool  kstrtou8  (char* str , u8* u);

bool  kstrtobool(char* str,bool* b);
