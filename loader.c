const unsigned char payload_compressed[] = {
#embed "./build/4kc.lz4-noheader"
};

__attribute__((section(".payload"), used))
unsigned char payload_decompressed[4096 * 2];

static const unsigned char * read_length(const unsigned char* restrict compressed, unsigned int* restrict length) {
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

static void decompress_lz4(unsigned char* restrict decompressed, const unsigned char* restrict compressed) {
  const unsigned char* compressed_end = compressed + sizeof(payload_compressed);

  //while (compressed < compressed_end) {
  while (1) { // Sketchy, restore previous condition if it fails
    const unsigned char token = *compressed++;

    unsigned int literal_length = token >> 4;
    compressed = read_length(compressed, &literal_length);
    __builtin_memcpy(decompressed, compressed, literal_length);
    decompressed += literal_length;
    compressed += literal_length;

    if (compressed >= compressed_end) {
      break;
    }

    const unsigned int delta = compressed[0] | (compressed[1] << 8);
    compressed += 2;
    unsigned int match_len = token & 0x0F;
    compressed = read_length(compressed, &match_len);
    match_len += 4;
    const unsigned char* match = decompressed - delta;
    for (int i = 0; i < match_len; ++i) {
      decompressed[i] = match[i];
    }
    decompressed += match_len;
  }
}

void _start() {
  decompress_lz4(payload_decompressed, payload_compressed);

  __asm__ volatile (
    "jmp " PAYLOAD_START
  );

  __builtin_unreachable();
}
