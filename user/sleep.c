#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"
#include "kernel/param.h"

// sleep
// 

int
main(int argc, char *argv[])
{
  if(argc != 2 || argv[1] == 0){
    fprintf(2,"usage: sleep seconds\n");
    exit(1);
  }
  

  int sec = atoi(argv[1]);
  // printf("Sleep for %d secs.\n", sec);
  sleep(sec);
  
  exit(0);
}
