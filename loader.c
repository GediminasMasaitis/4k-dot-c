const unsigned char payload_compressed[] = {
#embed "./build/4kc.lz4-noheader"
};

__attribute__((section(".payload"), used))
char payload_decompressed[4096];

//#include <stdio.h>

static void unlz4(unsigned char* decompressed, const unsigned char* compressed)
{
  unsigned char history[64 * 1024];
  unsigned int position = 0;

  while (compressed < payload_compressed + sizeof(payload_compressed) - 4) {

    const unsigned char token = *compressed++;
    unsigned int length = token >> 4;

    if (length == 0x0F) {
      for (;;) {
        length += *compressed++;
        if (compressed[-1] != 0xFF) {
          break;
        }
      }
    }

    while (length > 0) {
      length--;
      *decompressed++ = history[position++] = *compressed++;
      position %= sizeof history;
    }

    if (compressed == payload_compressed + sizeof(payload_compressed) - 4) {
      break;
    }

    unsigned int delta = *compressed++;
    delta |= *compressed++ << 8;

    length = token & 0x0F;
    if (length == 0x0F) {
      for (;;) {
        length += *compressed++;
        if (compressed[-1] != 0xFF) {
          break;
        }
      }
    }

    length += 4;
    unsigned int position0 = position - delta;
    while (length > 0) {
      length--;
      position0 %= sizeof history;
      *decompressed++ = history[position++] = history[position0++];
      position %= sizeof history;
    }
  }
}

void _start() {
  //__builtin_memcpy(decompressed, compressed, sizeof(compressed));
  unlz4(payload_decompressed, payload_compressed);

  //__asm__ volatile (
  //  "jmp 0x400c45"
  //);

  ((void (*)(void))(0x400c45))();
}
