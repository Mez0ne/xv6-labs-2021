#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"
#include "kernel/param.h"

// primes
//

int runner_cnt = 0;

// pipe for each runner
int pipe_r[20];
int pipe_w[20];

void runner(int id) {

  if(id>0){
    close(pipe_r[id-1]);
    pipe_r[id-1]=-1;
  }
  
  int p = 0;

  if(read(pipe_r[id], &p, 4) == -1){
    printf("%d, ",pipe_r[id]);
    fprintf(2,"primes: runner[%d] read failed\n", id);
    exit(1);
  } else if(p==0){
    fprintf(2,"primes: p==0 something wrong!\n");
    exit(1);
  }

  printf("prime %d\n", p);

  while (1) {
    int n;
    
    if (read(pipe_r[id], &n, 4) == 0) {
      // mission finished
      close(pipe_r[id]);
      pipe_r[id]=-1;

      if (pipe_w[id] != -1) {
        close(pipe_w[id]);
        pipe_w[id]=-1;
      }


      // wait for right runner destroy
      wait((int*)0);
      exit(0);
    }
    // n is not prime, drop it
    if (n % p == 0) {
      continue;
    }
    if (runner_cnt <= id + 1) {
      // create a new runner

      int new_pipe[2];
      if (pipe(new_pipe)) {
        fprintf(2, "primes: create pipe failed\n");
        exit(1);
      }

      pipe_r[id + 1] = new_pipe[0];
      pipe_w[id] = new_pipe[1];
      runner_cnt += 1;

      if (!fork()) {
        close(new_pipe[1]);
        pipe_w[id]=-1;
        runner(id + 1);
      } else {
        close(new_pipe[0]);
        pipe_r[id+1]=-1;
      }
    }

    // send the number to right runner
    write(pipe_w[id], &n, 4);
  }
}


int main(int argc, char *argv[]) {
  close(0);

  for (int i = 0; i < sizeof(pipe_r) / sizeof(int); i++) {
    pipe_r[i] = -1;
    pipe_w[i] = -1;
  }

  int pi[2];
  if (pipe(pi)) {
    fprintf(2, "primes: create pipe error!\n");
    exit(1);
  }

  pipe_r[0] = pi[0];

  // feed numbers to sieve
  for (int i = 2; i <= 35; i++) {
    int cur = i;
    write(pi[1], &cur, 4);
    
  }
  close(pi[1]);

  if (fork()) {
    // feeder: wait for childs
    close(pi[0]);
    wait(0);
  } else {
    // child

    runner_cnt += 1;

    // create first runner
    runner(0);

    exit(0);
  }
  exit(0);
}
