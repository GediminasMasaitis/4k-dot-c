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

static void puts(char *string) { _sys(4, 1, (int)string, strlen(string)); }

void _start() {
  puts("Hello World!\n");
  _sys(1, 0, 0, 0);
}
