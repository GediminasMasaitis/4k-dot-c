#define NULL ((void *)0)
#define false 0
#define true 1

enum {
  stdin = 0,
  stdout = 1,
  stderr = 2,
};

int _sys(int call, int arg1, int arg2, int arg3) {
  int ret;
  asm volatile("int $0x80"
               : "=a"(ret)
               : "a"(call), "b"(arg1), "c"(arg2), "d"(arg3));
  return ret;
}

static int strlen(char *string) {
  int length = 0;
  while (string[length]) {
    length++;
  }
  return length;
}

static void puts(char *string) { _sys(4, stdout, (int)string, strlen(string)); }

static char *gets(char *string0) {
  char *string;
  string = string0;
  while (true) {
    int result = _sys(3, stdin, (int)string, 1);
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
