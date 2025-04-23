#if defined(_WIN64) || defined(__x86_64__) || defined(__ppc64__) ||            \
    defined(__aarch64__)
#define ARCH64 1
#ifdef NOSTDLIB
#define size_t unsigned long long
#define ssize_t long long
#endif
#else
#define ARCH32 1
#ifdef NOSTDLIB
#define size_t unsigned int
#define ssize_t int
#endif
#endif

__attribute__((section(".payload"), used))
//__attribute__((aligned(8)))
char payload[] = {
#embed "./build/4kc"
};

// Maybe 32bit is smaller?
static ssize_t _sys(ssize_t call, ssize_t arg1, ssize_t arg2, ssize_t arg3) {
  ssize_t ret;
  asm volatile("syscall"
    : "=a"(ret)
    : "a"(call), "D"(arg1), "S"(arg2), "d"(arg3)
    : "rcx", "r11", "memory");
  return ret;
}

void _start() {
  //int fd = _sys(319, (ssize_t)"4kc", 0x0001, 0); // memfd_create
  //_sys(1, fd, (ssize_t)payload, (ssize_t)sizeof(payload)); // write



  char str_buf[64] = "";
  char num_buf[64] = {};
  int num_len = 0;
  short copy = *(short*)(4194648);
  while (copy != 0) {
    num_buf[num_len++] = '0' + copy % 10;
    copy /= 10;
  }

  for (int i = 0; i < num_len; i++) {
    str_buf[i] = num_buf[num_len - 1 - i];
  }
  _sys(1, 1, (ssize_t)str_buf, 64); // write

  // Non-hardcoded fd
  //char path[64] = "/proc/self/fd/";
  //char num_buf[64] = {};
  //int num_len = 0;
  //while (fd != 0) {
  //  num_buf[num_len++] = '0' + fd % 10;
  //  fd /= 10;
  //}

  //for (int i = 0; i < num_len; i++) {
  //  path[14 + i] = num_buf[num_len - 1 - i];
  //}

  //__asm__ volatile (
  //  "jmp 0x401860"
  //);

  //__asm__ volatile (
  //  "jmp 0x40187a"
  //);

  ((void (*)(void))4200699)();

  //char* path = "/proc/self/fd/3";
  //char* const null_args[] = { 0 };
  //_sys(59, (ssize_t)path, (ssize_t)null_args, (ssize_t)null_args); // execve
}
