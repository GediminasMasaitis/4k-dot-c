#define _GNU_SOURCE
#include <unistd.h>
#include <sys/syscall.h>
#include <sys/mman.h>
#include <stdio.h>

#include "./build/4kc.h"

int main() {
  int fd = syscall(SYS_memfd_create, "4kc", MFD_CLOEXEC);
  write(fd, __build_4kc, __build_4kc_len);

  char path[64] = {0};
  sprintf(path, "/proc/self/fd/%d", fd);
  char* const null_args[] = { NULL };
  syscall(SYS_execve, path, null_args, null_args);

  // If syscall returns, it failed
  perror("syscall execve failed");
  return 1;
}
