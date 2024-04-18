#include  "defines.h"

#include  <stdio.h>
#include  <stdarg.h>
#include  <stdbool.h>
#include  <unistd.h>


void  initializeLogging(){
  UREPORT(LOG_WARN, "TODO")
}

void  shutdownLogging(){
  UREPORT(LOG_WARN, "TODO")
}

void  Ulog(log_level level , const char* message , ...){
  const char* levels[6]  = {"[FATAL]: ","[ERROR]: ","[WARN]:  ","[INFO]:  ","[DEBUG]: ","[TRACE]: "};
  char out_message[1024];
  int  prefix_len = sprintf(out_message, "%s", levels[level]);
  va_list args;
  va_start(args , message);
  vsnprintf(out_message + prefix_len, 1024 - prefix_len, message , args);
  va_end(args);

  printf("%s\n",out_message);
}

void  Uwrite(const char *message, ...) {
  va_list args;
  va_start(args, message);
  char buffer[1024];
  u64 size  = vsnprintf(buffer,1024, message, args);
  va_end(args);

  write(STDOUT_FILENO, buffer, size); 
}

void writeint(i64 integer) {
  bool isNegative = false;
  if (integer < 0) {
    isNegative = true;
    integer = -integer;
  }

  u64 _int = (u64)integer;

  u8 string[24];
  u8 count = 0;
  u8 rem = 0;

  do {
    rem = (u8)(_int % 10);
    string[count] = rem + '0';
    _int /= 10;
    ++count;
  } while (_int != 0);

  if (isNegative) {
    putchar('-');
  }

  for (size_t i = count; i > 0; --i) {
    putchar(string[i - 1]);
  }
  putchar('\n');
}

