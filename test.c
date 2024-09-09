#include  "stdio.h"
#include  "stdlib.h"

#define   SIZE 10240
int main(void){

  FILE* f = fopen("assets/textures/scylla.png", "r");
  if(!f){
    perror("fopen");
    return EXIT_FAILURE;
  }
  
  unsigned char buffer[SIZE];
  size_t ret = fread(buffer, sizeof(*buffer), SIZE, f);
  if(ret != SIZE){
    printf("fread failed");
  }
  printf("%s", buffer);
  fclose(f);

  return 0;
}
