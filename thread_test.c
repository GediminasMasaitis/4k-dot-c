#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sched.h>
#include <sys/mman.h>
#include <sys/syscall.h>

#define STACK_SIZE (1 << 16)

static int thread_func(void* arg) {
  printf("Hello from thread!\n");
  fflush(stdout);
  while (true)
  {
    sleep(1);
    printf("Hello from thread\n");
    fflush(stdout);
  }

  syscall(SYS_exit, 0);  // or just return 0;
  return 0;
}

int main() {
  // Allocate memory for stack
  void* stack = mmap(NULL, STACK_SIZE, PROT_READ | PROT_WRITE,
    MAP_PRIVATE | MAP_ANONYMOUS | MAP_STACK, -1, 0);

  void* stack_top = (char*)stack + STACK_SIZE;

  pid_t tid = clone(thread_func, stack_top,
    CLONE_VM | CLONE_FS | CLONE_FILES |
    CLONE_SIGHAND | CLONE_THREAD | CLONE_SYSVSEM,
    NULL);

  while (true)
  {
    sleep(2);
    printf("Hello from main\n");
  }

  //getchar();  // Wait for input
  return 0;
}
