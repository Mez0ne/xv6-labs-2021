#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"
#include "kernel/param.h"

// xargs
//

// getline without \r \n
char *getline(char *buf, int max) {
  int i, cc;
  char c;

  for (i = 0; i + 1 < max;) {
    cc = read(0, &c, 1);
    if (cc < 1)
      break;
    buf[i++] = c;
    if (c == '\n' || c == '\r') {
      buf[i - 1] = '\0';
      break;
    }
  }
  return buf;
}

int max_argc = MAXARG;
int main(int argc, char *argv[]) {

  char *cmd = 0;
  char **nargv = malloc(sizeof(char *) * (max_argc));
  if (nargv == 0) {
    fprintf(2, "xargs: malloc failed\n");
    exit(1);
  }

  memset(nargv, 0, sizeof(char *) * (max_argc));

  // copy the args
  for (int i = 2; i < argc; i++) {
    nargv[i - 1] = argv[i];
  }

  if (argc == 1) {
    cmd = "echo";
    nargv[0] = "echo";
  } else {
    cmd = argv[1];
    nargv[0] = argv[1];
  }

  while (1) {
    int nargc = argc - 1;

    char *buf = malloc(128);
    memset(buf, 0, 128);
    getline(buf, 128);
    if (buf[0] == 0) {
      break;
    }

    nargv[nargc] = buf;
    nargc++;
    if (nargc > MAXARG) {
      fprintf(2, "xargs: too many args");
    } else if (fork()) {
      wait((int *)0);
    } else {
      // child
      exec(cmd, nargv);
    }

    free(buf);
  }

  exit(0);
}