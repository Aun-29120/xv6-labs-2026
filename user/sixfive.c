#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"
#include "kernel/fcntl.h"

char separators[] = " \r\t\n,./";

int
is_separator(char c) {
  return strchr(separators, c) != 0;
}

void
process_fd(int fd) {
  char buf[1];
  char numbuf[32];
  int nlen = 0;

  while (read(fd, buf, 1) == 1) {
    char c = buf[0];
    if (c >= '0' && c <= '9') {
      if (nlen < sizeof(numbuf) - 1)
        numbuf[nlen++] = c;
    } else {
      if (nlen > 0) {
        numbuf[nlen] = '\0';
        int num = atoi(numbuf);
        if (num % 5 == 0 || num % 6 == 0)
          printf("%d\n", num);
        nlen = 0;
      }
    }
  }

  if (nlen > 0) {
    numbuf[nlen] = '\0';
    int num = atoi(numbuf);
    if (num % 5 == 0 || num % 6 == 0)
      printf("%d\n", num);
  }
}

void
process_file(char *filename) {
  int fd = open(filename, O_RDONLY);
  if (fd < 0) {
    fprintf(2, "sixfive: cannot open %s\n", filename);
    return;
  }
  process_fd(fd);
  close(fd);
}

int
main(int argc, char *argv[]) {
  if (argc < 2) {
    process_fd(0);   // no args: read from stdin
    exit(0);
  }
  for (int i = 1; i < argc; i++) {
    process_file(argv[i]);
  }
  exit(0);
}
