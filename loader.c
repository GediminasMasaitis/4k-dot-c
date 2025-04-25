const char compressed[] = {
#embed "./build/4kc.lz4"
, 0
};

__attribute__((section(".payload"), used))
char decompressed[4096];

static unsigned char* unlz4(unsigned char* decompressed, const unsigned char* compressed)
{
  unsigned char has_size;
  unsigned char flags;
  unsigned char history[64 * 1024];
  unsigned int position;
  unsigned int block_size;
  unsigned char token;
  unsigned int length;
  unsigned int delta;
  unsigned int position0;

  compressed += 4;

  flags = *compressed++;
  has_size = flags & 8;

  compressed += 2;
  if (has_size) compressed += 8;
  position = 0;

  for (;;) {

    block_size = *compressed++;
    block_size |= (unsigned int)*compressed++ << 8;
    block_size |= (unsigned int)*compressed++ << 16;
    block_size |= (unsigned int)*compressed++ << 24;

    if (block_size == 0) break;

    if (block_size & 0x80000000) {

      block_size &= 0x7FFFFFFF;
      while (block_size > 0) {
        *decompressed++ = history[position++] = *compressed++;
        position %= sizeof history;
        block_size--;
      }

      continue;
    }

    while (block_size > 0) {

      token = *compressed++;
      block_size--;

      length = token >> 4;
      if (length == 0x0F) {
        for (;;) {
          length += *compressed++;
          block_size--;
          if (compressed[-1] != 0xFF) break;
        }
      }

      while (length > 0) {
        length--;
        *decompressed++ = history[position++] = *compressed++;
        position %= sizeof history;
        block_size--;
      }

      if (block_size == 0) break;

      delta = *compressed++;
      delta |= *compressed++ << 8;
      block_size -= 2;

      length = token & 0x0F;
      if (length == 0x0F) {
        for (;;) {
          length += *compressed++;
          block_size--;
          if (compressed[-1] != 0xFF) break;
        }
      }

      length += 4;
      position0 = position - delta;
      while (length > 0) {
        length--;
        position0 %= sizeof history;
        *decompressed++ = history[position++] = history[position0++];
        position %= sizeof history;
      }
    }
  }

  return decompressed;
}

void _start() {
  //__builtin_memcpy(decompressed, compressed, sizeof(compressed));
  unlz4(decompressed, compressed);

  //__asm__ volatile (
  //  "jmp 0x400c45"
  //);

  ((void (*)(void))(0x400c45))();
}
