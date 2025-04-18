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
  char num_buf[64] = {};
  int num_len = 0;
  while (fd != 0) {
    num_buf[num_len++] = '0' + fd % 10;
    fd /= 10;
  }

  for (int i = 0; i < num_len; i++) {
    path[14 + i] = num_buf[num_len - 1 - i];
  }

  char* const null_args[] = { NULL };
  syscall(59, path, null_args, null_args);

  return 1;
}
