#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"
#include "kernel/fs.h"
#include "kernel/param.h"

// Regex matcher (match/matchhere/matchstar) logic borrowed from user/grep.c
int matchhere(char *, char *);
int matchstar(int, char *, char *);

int
match(char *re, char *text)
{
  if (re[0] == '^')
    return matchhere(re + 1, text);
  do {
    if (matchhere(re, text))
      return 1;
  } while (*text++ != '\0');
  return 0;
}

int
matchhere(char *re, char *text)
{
  if (re[0] == '\0')
    return 1;
  if (re[1] == '*')
    return matchstar(re[0], re + 2, text);
  if (re[0] == '$' && re[1] == '\0')
    return *text == '\0';
  if (*text != '\0' && (re[0] == '.' || re[0] == *text))
    return matchhere(re + 1, text + 1);
  return 0;
}
int
matchstar(int c, char *re, char *text)
{
  do {
    if (matchhere(re, text))
      return 1;
  } while (*text != '\0' && (*text++ == c || c == '.'));
  return 0;
}
char*
fmtname(char *path)
{
  static char buf[DIRSIZ+1];
  char *p;

  for(p=path+strlen(path); p >= path && *p != '/'; p--);
  p++;
  if(strlen(p) >= DIRSIZ)
    return p;
  memmove(buf, p, strlen(p));
  buf[strlen(p)] = 0;
  return buf;
}

void
handle_match(char *path, int execflag, char *execargs[], int execargc)
{
  if (execflag) {
    char *argv[MAXARG];
    for (int i = 0; i < execargc; i++)
      argv[i] = execargs[i];
    argv[execargc] = path;
    argv[execargc + 1] = 0;

    if (fork() == 0) {
      exec(argv[0], argv);
      fprintf(2, "exec %s failed\n", argv[0]);
      exit(1);
    } else {
      wait(0);
    }
  } else {
    printf("%s\n", path);
  }
}

void
rfind(char *path, char *target, int execflag, char *execargs[], int execargc)
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
    if(match(target, fmtname(path))){
      handle_match(path, execflag, execargs, execargc);
    }
    break;

  case T_DIR:
    if(match(target, fmtname(path))){
      handle_match(path, execflag, execargs, execargc);
    }

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
      if(strcmp(de.name, ".") == 0 || strcmp(de.name, "..") == 0)
        continue;
      memmove(p, de.name, DIRSIZ);
      p[DIRSIZ] = 0;
      rfind(buf, target, execflag, execargs, execargc);
    }
    break;
  }
  close(fd);
}

int
main(int argc, char *argv[])
{
  if(argc < 3){
    fprintf(2, "usage: find <path> <filename> [-exec cmd args]\n");
    exit(1);
  }

  int execflag = 0;
  char *execargs[MAXARG];
  int execargc = 0;

  for(int i=3; i<argc; i++){
    if(strcmp(argv[i], "-exec") == 0){
      execflag = 1;
      for(int j=i+1; j<argc; j++){
        execargs[execargc++] = argv[j];
      }
      break;
    }
  }

  rfind(argv[1], argv[2], execflag, execargs, execargc);
  exit(0);
}
