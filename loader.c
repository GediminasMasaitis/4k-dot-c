__attribute__((aligned(1))) const unsigned char payload_compressed[] = {
#embed "./build/4kc.ap"
};

__attribute__((section(".payload"),
               used)) unsigned char payload_decompressed[4096 * 2];

// #include "aplib.h"
void decompress_aplib(void *destination, const void *source);

void _start() {
  decompress_aplib(payload_decompressed, payload_compressed);

  __asm__ volatile("jmp " PAYLOAD_START);

  __builtin_unreachable();
}
