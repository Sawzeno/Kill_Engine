#include  "defines.h"
#include  "logger.h"

#include  <stdio.h>
#include  <stdlib.h>
#include  <stdarg.h>

#include  <unistd.h>
#include  <errno.h>
#include  <string.h>
#include  <time.h>

#define LOG_LEVELS 6
#define PREFIX_LEN 24
#define BUFFER_LEN 992
#define SUFFIX_LEN 8

#define UBUFF_CHECK(result, limit)                    \
if(result >= limit){                                  \
  fprintf(stderr, "result size exceeded limit\n");    \
  return result;                                      \
}else if( result < 0){                                \
  fprintf(stderr, "error in %s\n", __FUNCTION__);     \
  return result;                                      \
}

typedef struct LoggerSystemState{
  FILE* engineLogFile;
  FILE* memoryLogFile;
  FILE* eventsLogFile;
  struct timespec clockStart;
}LoggerSystemState;

static const char* logLevels[LOG_LEVELS]  = 
  { "[FATAL]",
    "[ERROR]",
    "[WARN] ",
    "[INFO] ",
    "[DEBUG]",
    "[TRACE]",
  };

static const char* logLevelColors[LOG_LEVELS + 1] =
  {
    "\x1b[105m",
    "\x1b[41m",
    "\x1b[93m",
    "\x1b[90m",
    "\x1b[92m",
    "\x1b[97m",
    "\x1b[0m\n"
  };

static LoggerSystemState* loggingSystemStatePtr;

bool createLogFiles(void);
FILE* createLogFile(const char* filename);

bool  initializeLogging (u64* memoryRequirement, void* state){
  *memoryRequirement = sizeof(LoggerSystemState);
  if(state  == NULL){
    UDEBUG("LOGGING SUBSYSTEM : STATE PASSED AS NULL");
    return true;
  }

  loggingSystemStatePtr  = state;
  if(LOG_FILES_ENABLED == 1){
    if(!createLogFiles()){
      UERROR("COULD NOT CREATE LOG FILES!");
      return false;
    }

    struct timespec start;
    clock_gettime(CLOCK_MONOTONIC, &loggingSystemStatePtr->clockStart);

    KFATAL("TEST MESSAGE!");
    KERROR("TEST MESSAGE!"); 
    KWARN ("TEST MESSAGE!"); 
    KINFO ("TEST MESSAGE!"); 
    KTRACE("TEST MESSAGE!"); 
    KDEBUG("TEST MESSAGE!"); 

    MINFO ("TEST MESSAGE!"); 
    MDEBUG("TEST MESSAGE!"); 
    MTRACE("TEST MESSAGE!"); 

    EINFO ("TEST MESSAGE!"); 
    EDEBUG("TEST MESSAGE!"); 
    ETRACE("TEST MESSAGE!"); 

  }else{
    UWARN("LOG_FILES_DISBALED");
  }

  UINFO("LOGGING SUBSYSTEM INITITALIZED!");
  KINFO("LOGGING SUBSYSTEM INITITALIZED!");
  return true;
}

void shutdownLogging(){
  KINFO("LOGGING SYSTEM SHUTDOWN");
  if(LOG_FILES_ENABLED == 1){
    fclose(loggingSystemStatePtr->engineLogFile);
    fclose(loggingSystemStatePtr->memoryLogFile);
    fclose(loggingSystemStatePtr->eventsLogFile);
  }
  loggingSystemStatePtr  = NULL;
}

u16   
consoleLog(LOG_LEVEL level , const char* message , ...){
  char outmsg[PREFIX_LEN + BUFFER_LEN + SUFFIX_LEN];
  u16   prefixlen = snprintf  (outmsg,
                               PREFIX_LEN,"%s%s%2s",logLevelColors[level], logLevels[level],"");
  UBUFF_CHECK(prefixlen, PREFIX_LEN);
  va_list args;
  va_start(args , message);
  u16   bufferlen    = vsnprintf (outmsg + prefixlen,
                                  BUFFER_LEN, message , args);
  va_end(args);
  UBUFF_CHECK(bufferlen, BUFFER_LEN);
  u16   suffixlen    = snprintf  (outmsg + prefixlen + bufferlen,
                                  SUFFIX_LEN, "%s", logLevelColors[LOG_LEVELS]);
  UBUFF_CHECK(suffixlen, SUFFIX_LEN);
  u16 totallen = prefixlen + bufferlen + suffixlen;
  if(write(STDOUT_FILENO, outmsg,totallen ) < 0){
    fprintf(stderr, "ULog failed with %s\n", strerror(errno));
  }
  return bufferlen ;
}

u16   
fileLog(LOG_TYPE type, LOG_LEVEL level, const char* message, ...){
  struct timespec end;
  char outmsg[PREFIX_LEN + BUFFER_LEN + SUFFIX_LEN];

  u16   prefixlen = 0;
  if(level == LOG_INFO || level == LOG_TRACE || level == LOG_DEBUG)
  {
    clock_gettime(CLOCK_MONOTONIC, &end);
    f64 elapsed = (end.tv_sec - loggingSystemStatePtr->clockStart.tv_sec) + (end.tv_nsec - loggingSystemStatePtr->clockStart.tv_nsec)/1e9;
    prefixlen = snprintf  (outmsg,PREFIX_LEN,"[%.9f]%2s",elapsed,"");
  }else{
    prefixlen = snprintf  (outmsg,PREFIX_LEN,"%s%2s",logLevels[level],"");
  }

  UBUFF_CHECK(prefixlen, PREFIX_LEN);
  va_list args;
  va_start(args , message);
  u16   bufferlen = vsnprintf (outmsg + prefixlen,BUFFER_LEN, message , args);
  va_end(args);

  u64 totallen  = bufferlen + prefixlen;
  outmsg[totallen] = '\n';
  outmsg[totallen + 1] = '\0';
  FILE* targetfile  = {0};
  if(type == LOG_ENGINE){
    targetfile  = loggingSystemStatePtr->engineLogFile;
  }else if(type == LOG_MEMORY){
    targetfile  = loggingSystemStatePtr->memoryLogFile;
  }else if(type == LOG_EVENT){
    targetfile  = loggingSystemStatePtr->eventsLogFile;
  }
  if(fputs(outmsg, targetfile) == EOF){
    UERROR("could not write to %d due to %s",type,strerror(errno));
    return bufferlen;
  }; 
  clock_settime(CLOCK_MONOTONIC, &loggingSystemStatePtr->clockStart);
  return bufferlen ;
}

u16
Uwrite(u16 limit, const char *message, ...)
{
  va_list args;

  va_start(args, message);
  char buffer[limit];
  u64 size  = vsnprintf(buffer,limit, message, args);
  UBUFF_CHECK(size, limit);
  va_end(args);

  if(write(STDOUT_FILENO, buffer, size) < 0){
    fprintf(stderr," Uwrite FAILED WITH %s\n", strerror(errno));
    return 0;
  }; 
  return size;
}


bool createLogFiles(void){
  FILE* file;
  file = createLogFile("engine.logs");
  ISNULL(file, false);
  loggingSystemStatePtr->engineLogFile  = file;

  file = createLogFile("memory.logs");
  ISNULL(file, false);
  loggingSystemStatePtr->memoryLogFile  = file;

  file = createLogFile("events.logs");
  ISNULL(file, false);
  loggingSystemStatePtr->eventsLogFile  = file;

  return true;
}

#define RETURN_DEFER(value) do { result = (value); goto defer; } while(0)
FILE* createLogFile(const char* filename){
  bool result = false; 
  FILE* file  = NULL;
  file  = fopen(filename, "w");
  if(file == NULL)  RETURN_DEFER(false);
  result= true;

  UDEBUG("SUCCESFULLY CREATED LOG FILE : %s", filename);
defer:
  if(!result){
    UERROR("FAILED TO CREATE LOG FILE %s : %s", filename, strerror(errno));
    if(file) fclose(file);
    return NULL;
  }
  return file;
}

void bk1(){
  UINFO("BK1");
}
void bk2(){
  UINFO("BK2");
}
void bk3(){
  UINFO("BK3");
}
void bk4(){
  UINFO("BK4");
}
