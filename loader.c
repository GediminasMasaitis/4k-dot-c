const unsigned char payload_compressed[] = {
#embed "./build/4kc.lz4-noheader"
};

__attribute__((section(".payload"), used))
char payload_decompressed[4096];

//#include <stdio.h>

static const unsigned char * read_length(const unsigned char* compressed, unsigned int* length) {
  if (*length == 0x0F) {
    for (;;) {
      *length += *compressed++;
      if (compressed[-1] != 0xFF) {
        break;
      }
    }
  }
  return compressed;
}

static void unlz4(unsigned char* decompressed, const unsigned char* compressed)
{
  unsigned char history[64 * 1024];
  int history_index = 0;

  while (compressed < payload_compressed + sizeof(payload_compressed) - 4) {
    const unsigned char token = *compressed++;
    unsigned int length1 = token >> 4;
    compressed = read_length(compressed, &length1);
    for (int i = 0; i < length1; i++) {
      *decompressed++ = history[history_index++] = *compressed++;
      history_index %= sizeof(history);
    }

    if (compressed == payload_compressed + sizeof(payload_compressed) - 4) {
      break;
    }

    unsigned int delta = *compressed++;
    delta |= *compressed++ << 8;

    unsigned int length2 = token & 0x0F;
    compressed = read_length(compressed, &length2);

    unsigned int history_index2 = history_index - delta;
    for (int i = 0; i < length2 + 4; i++) {
      history_index2 %= sizeof(history);
      *decompressed++ = history[history_index++] = history[history_index2++];
      history_index %= sizeof(history);
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
