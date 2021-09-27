#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"
#include "kernel/fs.h"


// find


char cur_dir[DIRSIZ+1];
char parent_dir[DIRSIZ+1];

char target_name[DIRSIZ + 1];


char*
fmtname(char *path)
{
  static char buf[DIRSIZ+1];
  char *p;

  // Find first character after last slash.
  for(p=path+strlen(path); p >= path && *p != '/'; p--)
    ;
  p++;

  // Return blank-padded name.
  if(strlen(p) >= DIRSIZ)
    return p;
  memmove(buf, p, strlen(p));
  memset(buf+strlen(p), ' ', DIRSIZ-strlen(p));
  return buf;
}


void
find(char *path)
{
  char buf[512], *p;
  int fd;
  struct dirent de;
  struct stat st;

  if((fd = open(path, 0)) < 0){
    fprintf(2, "find: cannot open %s\n", path);
    return;
  }

  if(fstat(fd, &st) < 0){
    fprintf(2, "find: cannot stat %s\n", path);
    close(fd);
    return;
  }

  switch(st.type){
  case T_FILE:
    if(strcmp(fmtname(path), target_name)==0){
      printf("%s\n", (path));
    }
    break;

  case T_DIR:
    if(strlen(path) + 1 + DIRSIZ + 1 > sizeof buf){
      printf("find: path too long\n");
      break;
    }
    strcpy(buf, path);
    p = buf+strlen(buf);
    *p++ = '/';
    while(read(fd, &de, sizeof(de)) == sizeof(de)){
      if(de.inum == 0)
        continue;
      memmove(p, de.name, DIRSIZ);
      p[DIRSIZ] = 0;
      if(stat(buf, &st) < 0){
        printf("find: cannot stat %s\n", buf);
        continue;
      }

      if(strcmp(fmtname(buf), target_name)==0){
        printf("%s\n", buf);
      }
      if(st.type == T_DIR && strcmp(cur_dir, fmtname(de.name) ) && 
        strcmp(parent_dir ,fmtname(de.name) )){
        find(buf);
      }
    }
    break;
  }
  close(fd);
}

int
main(int argc, char *argv[])
{

  strcpy(cur_dir,fmtname("."));
  strcpy(parent_dir,fmtname(".."));
  if(argc < 2){
    find(".");
  } else {
    strcpy(target_name, fmtname( argv[2]) );
    find(argv[1]);
  }

  exit(0);
}