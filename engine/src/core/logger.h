#pragma once

#include "defines.h"
#include  <time.h>
#include  <error.h>
#include  <time.h>
#include  <string.h>
#include  <stdlib.h>

typedef enum LOG_LEVEL{
  LOG_FATAL = 0,
  LOG_ERROR = 1,
  LOG_WARN  = 2,
  LOG_INFO  = 3,
  LOG_DEBUG = 4,
  LOG_TRACE = 5,
}LOG_LEVEL;

typedef enum LOG_TYPE{
  LOG_ENGINE,
  LOG_MEMORY,
  LOG_EVENT,
}LOG_TYPE;

#ifndef LOG_FILES_ENABLED
#define LOG_FILES_ENABLED 1
#endif

#ifndef LOG_WARN_ENABLED
#define LOG_WARN_ENABLED 1
#endif

#ifndef LOG_INFO_ENABLED
#define LOG_INFO_ENABLED  1
#endif

#ifndef LOG_TRACE_ENABLED
#define LOG_TRACE_ENABLED 1
#endif

#ifndef LOG_DEBUG_ENABLED
#define LOG_DEBUG_ENABLED 1
#endif

#define UREPORT(X) consoleLog(LOG_WARN, "%s  func : %s   file : %s line : %d", #X, __FUNCTION__, __FILE__, __LINE__);

#define UFATAL(fmt, ...) consoleLog   (LOG_FATAL, fmt, ##__VA_ARGS__)
#define UERROR(fmt, ...) consoleLog   (LOG_ERROR, fmt, ##__VA_ARGS__)

#if LOG_WARN_ENABLED == 1
#define UWARN(fmt, ...)  consoleLog   (LOG_WARN , fmt, ##__VA_ARGS__)
#else 
#define UWARN(fmt, ...)
#endif

#if LOG_INFO_ENABLED == 1
#define UINFO(fmt, ...)  consoleLog   (LOG_INFO , fmt, ##__VA_ARGS__) 
#else
#define UINFO(fmt, ...)
#endif

#if LOG_TRACE_ENABLED == 1
#define UTRACE(fmt, ...)  consoleLog  (LOG_TRACE , fmt, ##__VA_ARGS__) 
#else
#define UTRACE(fmt, ...)
#endif

#if LOG_DEBUG_ENABLED == 1
#define UDEBUG(fmt, ...)  consoleLog  (LOG_DEBUG , fmt, ##__VA_ARGS__) 
#else
#define UDEBUG(fmt, ...)
#endif

#if LOG_WARN_ENABLED == 1 && LOG_FILES_ENABLED == 1
#define KFATAL(fmt, ...) fileLog    (LOG_ENGINE,LOG_FATAL, fmt, ##__VA_ARGS__)
#define KERROR(fmt, ...) fileLog    (LOG_ENGINE,LOG_ERROR, fmt, ##__VA_ARGS__)
#else
#define KFATAL(fmt, ...)
#define KERROR(fmt, ...)
#endif


#if LOG_WARN_ENABLED == 1 && LOG_FILES_ENABLED == 1
#define KWARN(fmt, ...)  fileLog    (LOG_ENGINE,LOG_WARN , fmt, ##__VA_ARGS__)
#else
#define KWARN(fmt, ...)
#endif

#if LOG_INFO_ENABLED == 1 && LOG_FILES_ENABLED == 1
#define KINFO(fmt, ...)  fileLog    (LOG_ENGINE , LOG_INFO , fmt, ##__VA_ARGS__) 
#define MINFO(fmt, ...)  fileLog    (LOG_MEMORY , LOG_INFO , fmt, ##__VA_ARGS__) 
#define EINFO(fmt, ...)  fileLog    (LOG_EVENT  , LOG_INFO , fmt, ##__VA_ARGS__) 
#else
#define KINFO(fmt, ...)
#define MINFO(fmt, ...)
#define EINFO(fmt, ...)
#endif

#if LOG_TRACE_ENABLED == 1 && LOG_FILES_ENABLED == 1

#define KTRACE(fmt, ...)  fileLog    (LOG_ENGINE , LOG_TRACE , fmt, ##__VA_ARGS__) 
#define MTRACE(fmt, ...)  fileLog    (LOG_MEMORY , LOG_TRACE , fmt, ##__VA_ARGS__) 
#define ETRACE(fmt, ...)  fileLog    (LOG_EVENT  , LOG_TRACE , fmt, ##__VA_ARGS__) 
#else
#define KTRACE(fmt, ...)
#define MTRACE(fmt, ...)
#define ETRACE(fmt, ...)
#endif

#if LOG_DEBUG_ENABLED == 1 && LOG_FILES_ENABLED == 1
#define KDEBUG(fmt, ...)  fileLog    (LOG_ENGINE , LOG_DEBUG , fmt, ##__VA_ARGS__) 
#define MDEBUG(fmt, ...)  fileLog    (LOG_MEMORY , LOG_DEBUG , fmt, ##__VA_ARGS__) 
#define EDEBUG(fmt, ...)  fileLog    (LOG_EVENT  , LOG_DEBUG , fmt, ##__VA_ARGS__) 
#else
#define KDEBUG(fmt, ...)
#define MDEBUG(fmt, ...)
#define EDEBUG(fmt, ...)
#endif

#define UASSERTIONS_ENABLED

#ifdef UASSERTIONS_ENABLED // expression has to be true otherwise it returns
#define UASSERT(expr, ret, info) if (!(expr)) { UREPORT(expr); UERROR(info);return ret; }
#else
#define UASSERT(expr)
#endif

#define MEMERR(X) if (X == NULL) { UFATAL("%s allocation failed due to %s at  %s", #X,strerror(errno), __FUNCTION__); exit(EXIT_FAILURE); }
#define ISNULL(X, Y) if (X == NULL) { UWARN("%s is NULL", #X); return Y; }
#define UWRITE(fmt, ...) Uwrite(1024, fmt, ##__VA_ARGS__)

#define TRACEFUNCTION KTRACE("%s called",__FUNCTION__)
#define TRACEMEMORY   MTRACE("%s called",__FUNCTION__)
#define TRACEEVENT    ETRACE("%s called",__FUNCTION__)

typedef struct Uinfo Uinfo;

struct  Uinfo{
  const char* str;
  bool        e;
};

// Initializes logging system , call twice once with state = 0 , to get memory size , and then to pass allocated memory to the state;

bool    initializeLogging (u64* memoryRequirement, void* state);
void    shutdownLogging   ();
u16     consoleLog        (               LOG_LEVEL level , const char* message , ...);
u16     fileLog           (LOG_TYPE type, LOG_LEVEL level , const char* message , ...);
u16     Uwrite            (u16 limit, const char *format , ...); 
void    sendSignal        (int sig, const char* str, bool e);
