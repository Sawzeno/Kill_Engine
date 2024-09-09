#include "kstring.h"
#include "logger.h"
#include "kmemory.h"

#include <ctype.h>
#include <stdio.h>
#include <string.h>
#include <stdarg.h>


#define MAX_STR_LEN 1024
i64   kstrlen   (const char* str){

  if(str == NULL ) return -1;
  return strlen (str);
}

char* kstrdup   (const char* str){

  if(str == NULL ) return NULL;
  return strdup(str);
}

bool  kstrequal (const char* str0, const char* str1){

  if(str0 == NULL || str1 == NULL) return false;
  return (strcmp(str0, str1) == 0) ?  true :  false;
}

bool  kstrequali(const char* str0, const char* str1){

  if(str0 == NULL || str1 == NULL) return false;
  return (strcasecmp(str0, str1) == 0)? true : false;
}

i64 kstrfmt(char* dst, const char* fmt, ...){
  if(dst == NULL || fmt == NULL) return -1;

  va_list args;
  va_start(args, fmt);
  int written = vsnprintf(dst, MAX_STR_LEN, fmt, args); 
  va_end(args);

  if(written < 0) return -2; // encoding error has occured;
  if(written >= MAX_STR_LEN) return -3; // output was truncated;

  return (i64)written;
}

i64   kstrfmtv  (char* dst, const char* fmt, void* va_list){
  if(dst == NULL || fmt == NULL) return -1;
  UERROR("%s not implimented", __FUNCTION__);
  return -4;
}

char* kstrcpy   (char* dst,const  char* src){

  if(dst == NULL || src == NULL) return NULL;
  return strcpy(dst, src);
}

char* kstrncpy  (char* dst,const  char* src, u64 n){

  if(dst == NULL || src == NULL) return NULL;
  return strncpy(dst, src, n);
}

char* ltrim(char* str){
  if( str == NULL) return NULL;
  while(isspace(*str)) ++str;
  return str;
}

char* rtrim(char* str){

  u64 len = kstrlen(str);
  if(len ==  0) return NULL;
  char* end = str + len  -1;
  int i = 0;
  while(end >= str && isspace((unsigned char)*end)){
    --end;
  }
  (*(end + 1)) = '\0';

  return str;
}

char* kstrtrim(char* str){
  return rtrim(ltrim(str));
}

i64   kstrindex(const char* str, char c){
  if( str == NULL) return -2;
  u64 len = kstrlen(str);
  for( u64 i = 0; i < len; ++i){
    if(c == str[i]) return i;
  }
  return -1;
}


void ksubstr(char* dst, const char* src, i32 start, i32 len) {
  if (dst == NULL || src == NULL) return;

  u64 srclen = strlen(src);

  // Handle cases where start is beyond the end of src
  if ((u64)start >= srclen) {
    dst[0] = '\0';
    return;
  }

  // Adjust len if negative
  if (len < 0) {
    len = srclen - start;
  }

  // Ensure len does not exceed the remaining length of src
  if ((u64)start + (u64)len > srclen) {
    len = srclen - start;
  }

  // Copy the substring
  for (u64 i = 0; i < (u64)len; ++i) {
    dst[i] = src[start + i];
  }

  // Null-terminate the destination
  dst[len] = '\0';
}



bool  kstrtovec4(char* str,Vec4* v){
  if( str == NULL) return false;

  kzeroMemory(v, sizeof(Vec4)); 
  i32 result = sscanf(str, "%f %f %f %f",&v->x,&v->y,&v->z,&v->w);
  return result != -1;
}

bool  kstrtovec3(char* str,Vec3* v){
  if( str == NULL) return false;

  kzeroMemory(v, sizeof(Vec3)); 
  i32 result = sscanf(str, "%f %f %f",&v->x,&v->y,&v->z);
  return result != -1;
}

bool  kstrtovec2(char* str,Vec2* v){
  if( str == NULL) return false;

  kzeroMemory(v, sizeof(Vec2)); 
  i32 result = sscanf(str, "%f %f",&v->x,&v->y);
  return result != -1;
}

bool  kstrtof32 (char* str, f32* f){
  if( str == NULL) return false;
  *f = 0;
  i32 result = sscanf(str, "%f", f);
  return result != -1;
}

bool  kstrtof64 (char* str, f64* f){
  if( str == NULL) return false;
  *f = 0;
  i32 result = sscanf(str, "%lf", f);
  return result != -1;
}

bool  kstrtoi64 (char* str, i64* i){
  if( str == NULL) return false;
  *i = 0;
  i32 result = sscanf(str, "%"SCNi64"", i);
  return result != -1;
}
bool  kstrtoi32 (char* str, i32* i){
  if( str == NULL) return false;
  *i = 0;
  i32 result = sscanf(str, "%"SCNi32"", i);
  return result != -1;
}
bool  kstrtoi16 (char* str, i16* i){
  if( str == NULL) return false;
  *i = 0;
  i32 result = sscanf(str, "%"SCNi16"", i);
  return result != -1;
}
bool  kstrtoi8  (char* str , i8* i){
  if( str == NULL) return false;
  *i = 0;
  i32 result = sscanf(str, "%"SCNi8"", i);
  return result != -1;
}
bool  kstrtou64 (char* str,u64*u){
  if( str == NULL) return false;
  *u = 0;
  i32 result = sscanf(str, "%"SCNu64"",u);
  return result != -1;
}
bool  kstrtou32 (char* str,u32*u){
  if( str == NULL) return false;
  *u = 0;
  i32 result = sscanf(str, "%"SCNu32"",u);
  return result != -1;
}
bool  kstrtou16 (char* str,u16*u){
  if( str == NULL) return false;
  *u = 0;
  i32 result = sscanf(str, "%"SCNu16"",u);
  return result != -1;
}
bool  kstrtou8  (char* str ,u8*u){
  if( str == NULL) return false;
  *u = 0;
  i32 result = sscanf(str, "%"SCNu8"",u);
  return result != -1;
}

bool  kstrtobool(char* str,bool* b){
  if( str == NULL) return false;
  return kstrequal(str, "1") || kstrequali(str, "true") ;
}

