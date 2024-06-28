#include  "defines.h"
#include  "core/logger.h"
#include  "containers/darray.h"

int main(void){
  const char* s1  = "HELLO1";
  const char* s2  = "HELLO2";

  const char** strings  = DARRAY_CREATE(const char*);
  DARRAY_PUSH(strings, s1);

  u64 len = DARRAY_LENGTH(strings);
  for(int i = 0 ; i < len ; ++i){
    UDEBUG("%s" , strings[0]);
  }
  return 0;
}
