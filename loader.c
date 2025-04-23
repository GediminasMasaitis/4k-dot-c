const char compressed[] = {
#embed "./build/4kc"
};

__attribute__((section(".payload"), used))
char decompressed[4096];

void _start() {
  //__asm__ volatile (
  //  "jmp 4200827"
  //);

  __builtin_memcpy(decompressed, compressed, sizeof(compressed));

  ((void (*)(void))(0x400d5d))();
}
