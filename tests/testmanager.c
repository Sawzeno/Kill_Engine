#include "testmanager.h"
#include  "containers/darray.h"

#include  <stdio.h>
#include  <stdarg.h>

#include  <unistd.h>
#include  <errno.h>
#include  <execinfo.h>
#include  <dlfcn.h>

#include  <string.h>
#include  <time.h>

#define PREFIX_LEN 24
#define BUFFER_LEN 256
#define SUFFIX_LEN 6
#define UBUFF_CHECK(result, limit)                    \
if(result >= limit){                                  \
  fprintf(stderr, "result size exceeded limit\n");    \
  return result;                                      \
}else if( result < 0){                                \
  fprintf(stderr, "error in %s\n", __FUNCTION__);     \
  return result;                                      \
}

typedef struct TestEntry{
  PFN_TEST func;
  char* desc;
}TestEntry;

static TestEntry* tests;

void testManagerInit(){
  tests  = DARRAY_CREATE(TestEntry);
}

void testManagerRegisterTest(u8(*PFN_TEST)(), char* desc){
  TestEntry e;
  e.func  = PFN_TEST;
  e.desc  = desc;
  DARRAY_PUSH(tests, e);
}

void testManagerRunTests(){
  u32 passed  = 0;
  u32 failed  = 0;
  u32 skipped = 0;

  u32 count = DARRAY_LENGTH(tests);
  INITCLOCK;

  for(u32 i = 0; i < count ; ++i){
    UTEST("RUNNING TEST %d of %d", i + 1, count);
    u8 result = tests[i].func();
    if(result == true){
      ++passed;
    }else if(result == BYPASS){
      UWARN("[SKIPPED] : %s", tests[i].desc);
      ++skipped;
    }else{
      UERROR("[FAILED] : %s", tests[i].desc);
      ++failed;
    }
    WATCH(tests[i].desc);
  }
  UTEST("BYPASSED : %d, PASSED : %d, FAILED : %d", skipped, passed, failed);
}

u16   
testLog(const char* message , ...){
  char outmsg[PREFIX_LEN + BUFFER_LEN];
  u16   prefixlen = snprintf  (outmsg,
                               PREFIX_LEN,"%s%s%2s","\x1b[96m","[TEST] ","");
  UBUFF_CHECK(prefixlen, PREFIX_LEN);
  va_list args;
  va_start(args , message);
  u16   bufferlen = vsnprintf (outmsg + prefixlen,
                               BUFFER_LEN, message , args);
  va_end(args);
  UBUFF_CHECK(bufferlen, BUFFER_LEN);

  UBUFF_CHECK(bufferlen, BUFFER_LEN);
  u16   suffixlen    = snprintf  (outmsg + prefixlen + bufferlen,
                                  SUFFIX_LEN, "%s", "\x1b[0m\n");
  UBUFF_CHECK(suffixlen, SUFFIX_LEN);
  u16 totallen = prefixlen + bufferlen + suffixlen;
  if(write(STDOUT_FILENO, outmsg,totallen ) < 0){
    fprintf(stderr, "ULog failed with %s\n", strerror(errno));
  }
  return bufferlen ;
}

void elapsed(struct timespec* start, const char* func){
  struct timespec end1;
  clock_gettime(CLOCK_MONOTONIC, &end1);

  f64 elapsed = (end1.tv_sec - start->tv_sec) + (end1.tv_nsec - start->tv_nsec)/1e9;
  UTEST("%-30s : %.9f",func,elapsed);
  struct timespec end2;
  clock_gettime(CLOCK_MONOTONIC, &end2);
  *start  = end2;
}
