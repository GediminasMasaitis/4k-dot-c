#if defined(_WIN64) || defined(__x86_64__) || defined(__ppc64__) ||            \
    defined(__aarch64__)
#define ARCH64 1
#define size_t unsigned long long
#define ssize_t long long
#else
#define ARCH32 1
#define size_t unsigned int
#define ssize_t int
#endif

#define NULL ((void *)0)
#define false 0
#define true 1

enum {
  stdin = 0,
  stdout = 1,
  stderr = 2,
};

ssize_t _sys(ssize_t call, ssize_t arg1, ssize_t arg2, ssize_t arg3) {
  ssize_t ret;
#if ARCH64
  asm volatile("syscall"
               : "=a"(ret)
               : "a"(call), "D"(arg1), "S"(arg2), "d"(arg3)
               : "rcx", "r11", "memory");
#else
  asm volatile("int $0x80"
               : "=a"(ret)
               : "a"(call), "b"(arg1), "c"(arg2), "d"(arg3)
               : "memory");
#endif
  return ret;
}

static int strlen(char *string) {
  int length = 0;
  while (string[length]) {
    length++;
  }
  return length;
}

static void puts(char *string) {
#if ARCH64
  _sys(1, stdout, (ssize_t)string, strlen(string));
#else
  _sys(4, stdout, (ssize_t)string, strlen(string));
#endif
}

static char *gets(char *string0) {
  char *string;
  string = string0;
  while (true) {
#if ARCH64
    int result = _sys(0, stdin, (ssize_t)string, 1);
#else
    int result = _sys(3, stdin, (ssize_t)string, 1);
#endif
    if (result < 1) {
      if (string == string0) {
        return NULL;
      }
      break;
    }
    if (*string == '\n') {
      break;
    }
    string++;
  }
  *string = 0;
  return string0;
}

static int strcmp(char *lhs, char *rhs) {
  while (*lhs || *rhs) {
    if (*lhs != *rhs) {
      return 1;
    }
    lhs++;
    rhs++;
  }
  return 0;
}

void _start() {
  char line[256];

  // UCI loop
  while (true) {
    gets(line);
    if (!strcmp(line, "uci")) {
      puts("id name 4k.c\nid author Gediminas Masaitis\n");
    } else if (!strcmp(line, "isready")) {
      puts("readyok\n");
    }
  }

  // Exit
  _sys(1, 0, 0, 0);
}
