#pragma once

#include  <stdio.h>
#include  <stdlib.h>
#include  <stdint.h>
#include  <signal.h>
#include  <stdbool.h>

#define LOG_WARN_ENABLED  1
#define LOG_INFO_ENABLED  1
#define LOG_TRACE_ENABLED 0
#define LOG_DEBUG_ENABLED 1

#define UASSERTIONS_ENABLED

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

typedef enum log_level{
  LOG_FATAL = 0,
  LOG_ERROR = 1,
  LOG_WARN  = 2,
  LOG_INFO  = 3,
  LOG_DEBUG = 4,
  LOG_TRACE = 5,
}log_level;

typedef struct  Unode Unode;
typedef struct  Ulist Ulist;  
typedef struct  Ubuff Ubuff;


#define UFATAL(fmt, ...)  Ulog(LOG_FATAL, fmt, ##__VA_ARGS__); 
#define UERROR(fmt, ...)  Ulog(LOG_ERROR, fmt, ##__VA_ARGS__);

#if LOG_WARN_ENABLED == 1
#define UWARN(fmt, ...)   Ulog(LOG_WARN, fmt, ##__VA_ARGS__);
#else
#define UWARN(fmt, ...)
#endif

#if LOG_INFO_ENABLED == 1
#define UINFO(fmt, ...)   Ulog(LOG_INFO, fmt, ##__VA_ARGS__);
#else
#define UINFO(fmt, ...)
#endif

#if LOG_DEBUG_ENABLED == 1
#define UDEBUG(fmt, ...)  Ulog(LOG_DEBUG, fmt, ##__VA_ARGS__);
#else
#define UDEBUG(fmt, ...)
#endif

#if LOG_TRACE_ENABLED == 1
#define UTRACE(fmt, ...)  Ulog(LOG_TRACE, fmt, ##__VA_ARGS__);
#else
#define UTRACE(fmt, ...)
#endif

#ifdef UASSERTIONS_ENABLED
#define UASSERT(expr) if(expr){}else Ulog(LOG_FATAL , "Assertion Failure \n'%s' \nfunc : %s \nfile : %s \nline : %d",#expr,__FUNCTION__,__FILE__,__LINE__); raise(SIGILL);
#else
#define UASSERT(expr)
#endif

#define UREPORT(LEVEL , X) Ulog(LEVEL , "%20s\nfunc  : %29s \nfile  : %29s \nline  : %20d", #X , __FUNCTION__,__FILE__,__LINE__);


#define MEMERR(X) do { if(X == NULL) { UERROR("%s allocation failed", #X); exit(EXIT_FAILURE); } } while(0);

#define ISNULL(X) if(X == NULL) { UWARN("%s is passed as NULL , returning 0 \nfunc : %s \nfile : %s \nline : %d", #X , __FUNCTION__,__FILE__,__LINE__) return 0; };

void    initializeLogging ();
void    shutdownLogging   ();

void    Ulog      (log_level level , const char* message , ...);
void    Uwrite    (const char *format , ...); 
void    writeint  (i64 integer);

