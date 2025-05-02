#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sched.h>
#include <sys/mman.h>
#include <sys/syscall.h>

enum { thread_count = 4 };
enum { thread_stack_size = 8 * 1024 * 1024 };

typedef struct {
  char bytes[thread_stack_size];
} thread_stack __attribute__((aligned(4096)));

thread_stack thread_stacks[thread_count];

struct __attribute((aligned(16))) stack_head {
  void (*entry)(struct stack_head*);
  int thread_id;
  // ...
};

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
    //fflush(stdout);
    sleep(1);
  }

  syscall(SYS_exit, 0);
}

int main() {
  setvbuf(stdout, NULL, _IONBF, 0);

  for (int i = 0; i < thread_count; i++)
  {
    struct stack_head* stack = (struct stack_head*)(((char*)&thread_stacks[i+1]) - 4096);
    //struct stack_head* stack = newstack(stack_size);
    stack->entry = threadentry;
    stack->thread_id = i;
    newthread(stack);
    printf("created %d\n", i);
  }

  while (true)
  {
    getc(stdin);
    printf("Hello from main\n");
    //fflush(stdout);
  }

  //getchar();  // Wait for input
  return 0;
}
