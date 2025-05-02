#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sched.h>
#include <sys/mman.h>
#include <sys/syscall.h>

enum { stack_size = 1024 * 1024};

typedef struct {
  char bytes[stack_size];
} thread_stack __attribute__((aligned(4096)));

thread_stack thread_stacks[2];

struct __attribute((aligned(16))) stack_head {
  void (*entry)(struct stack_head*);
  int thread_id;
  // ...
};

long syscall6(long n, long a, long b, long c, long d, long e, long f)
{
  register long ret;
  register long r10 asm("r10") = d;
  register long r8  asm("r8") = e;
  register long r9  asm("r9") = f;
  __asm volatile (
  "syscall"
    : "=a"(ret)
    : "a"(n), "D"(a), "S"(b), "d"(c), "r"(r10), "r"(r8), "r"(r9)
    : "rcx", "r11", "memory"
    );
  return ret;
}


static struct stack_head* newstack(long size)
{
  unsigned long p = syscall6(9, 0, size, 3, 0x22, -1, 0);
  long count = size / sizeof(struct stack_head);
  return (struct stack_head*)p + count - 1;
}


__attribute((naked))
static long newthread(struct stack_head* stack)
{
  __asm volatile (
  "mov  %%rdi, %%rsi\n"     // arg2 = stack
    "mov  $0x50f00, %%edi\n"  // arg1 = clone flags
    "mov  $56, %%eax\n"       // SYS_clone
    "syscall\n"
    "mov  %%rsp, %%rdi\n"     // entry point argument
    "ret\n"
    : : : "rax", "rcx", "rsi", "rdi", "r11", "memory"
    );
}

static void threadentry(struct stack_head* stack)
{
  //while (true)
  for (int i = 0; i < 5; i++)
  {
    printf("Hello %d from thread %d\n", i, stack->thread_id);
    fflush(stdout);
    sleep(1);
  }

  syscall(SYS_exit, 0);
}

int main() {

  for (int i = 0; i < 1; i++)
  {
    struct stack_head* stack = (struct stack_head*)(((char*)&thread_stacks[i+1]) - 4096);
    stack->entry = threadentry;
    stack->thread_id = i;
    newthread(stack);
  }

  while (true)
  {
    getc(stdin);
    printf("Hello from main\n");
  }

  //getchar();  // Wait for input
  return 0;
}
