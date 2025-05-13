const unsigned char payload_compressed[] = {
#embed "./build/4kc.ap"
};

__attribute__((section(".payload"),
               used)) unsigned char payload_decompressed[4096 * 2];

//#include "aplib.h"
void aP_depack_asm(const void* source, void* destination);

void _start() {
  //decompress_aplib(payload_decompressed, payload_compressed);
  aP_depack_asm(payload_compressed, payload_decompressed);

  __asm__ volatile("jmp " PAYLOAD_START);

  __builtin_unreachable();
}
