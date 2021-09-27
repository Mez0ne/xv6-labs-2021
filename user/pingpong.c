#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"
#include "kernel/param.h"

// pingpong
// 

int
main(int argc, char *argv[])
{
  int p[2];
  if(pipe(p)){
    fprintf(2,"create pipe failed\n");
    exit(1);
  }

  int pid=fork();

  char buf[1];
  buf[0]=0x30;

  if(pid==0){ // child
    
    // read ping from parent
    read(p[0],buf,1);
    printf("%d: received ping\n",getpid());

    // pong to parent
    write(p[1],buf,1);
    
    exit(0);

  } else { // parent
    // ping to child
    write(p[1],buf,1);

    // wait child exit
    wait((int*)0);

    // read pong from child
    read(p[0],buf,1);
    
    printf("%d: received pong\n",getpid());
  }
  exit(0);
}
