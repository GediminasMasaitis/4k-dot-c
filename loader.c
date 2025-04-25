const char compressed[] = {
#embed "./build/4kc"
};

__attribute__((section(".payload"), used))
char decompressed[4096];

void _start() {
  __builtin_memcpy(decompressed, compressed, sizeof(compressed));

  //__asm__ volatile (
  //  "jmp 0x400c45"
  //);

  ((void (*)(void))(0x400c45))();
}
