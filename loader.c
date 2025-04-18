#define _GNU_SOURCE
#include <unistd.h>
#include <sys/syscall.h>
#include <sys/mman.h>
#include <stdio.h>

//#include "./build/4kc.h"

char payload[] = {
#embed "./build/4kc"
};

int main() {
  int fd = syscall(319, "4kc", 0x0001);
  write(fd, payload, sizeof(payload));

  char path[64] = "/proc/self/fd/";
  printf("/proc/self/fd/%d\n", fd);
  sprintf(path, "/proc/self/fd/%d", fd);
  char* const null_args[] = { NULL };
  syscall(59, path, null_args, null_args);

  return 1;
}
