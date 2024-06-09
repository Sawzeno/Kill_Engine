#include  "defines.h"
#include  "containers/darray.h"

int main(void){
  const char* s1  = "HELLO1";
  const char* s2  = "HELLO2";

  const char** strings  = darrayCreate(const char*);
  darrayPush(strings, s1);

  u64 len = darrayLength(strings);
  for(int i = 0 ; i < len ; ++i){
    UDEBUG("%s" , strings[0]);
  }
  return 0;
}
