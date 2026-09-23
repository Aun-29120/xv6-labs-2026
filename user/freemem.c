#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"

int
main(void)
{
  int bytes = (int)freemem();
  printf("free memory: %d bytes (%d KB)\n", bytes, bytes / 1024);
  exit(0);
}