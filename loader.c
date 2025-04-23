__attribute__((section(".payload"), used))
//__attribute__((aligned(8)))
char payload[] = {
#embed "./build/4kc"
};

void _start() {
  //__asm__ volatile (
  //  "jmp 4200827"
  //);

  ((void (*)(void))(0x400dd5))();
}
