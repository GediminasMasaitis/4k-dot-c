// Contains a modified version of the original aPLib decompressor
// Below is the original copyright notice

/*
 * aPLib compression library  -  the smaller the better :)
 *
 * C depacker
 *
 * Copyright (c) 1998-2014 Joergen Ibsen
 * All Rights Reserved
 *
 * http://www.ibsensoftware.com/
 */

#pragma once

struct State {
  const unsigned char *source;
  //unsigned char *destination;
  unsigned char tag;
  unsigned char bitcount;
};

static unsigned int get_bit(struct State *restrict state) {
  if (!state->bitcount--) {
    state->tag = *state->source++;
    state->bitcount = 7;
  }

  const unsigned int bit = state->tag >> 7;
  state->tag <<= 1;
  return bit;
}

static unsigned int get_gamma(struct State *restrict state) {
  unsigned int result = 1;
  do {
    result = (result << 1) + get_bit(state);
  } while (get_bit(state));
  return result;
}

static void decompress_aplib(unsigned char *restrict destination,
                             const unsigned char* restrict source) {
  struct State state;
  state.source = source;
  state.bitcount = 0;

  unsigned int offset;
  unsigned int length;
  unsigned int last_offset = -1;
  unsigned int last_was_match = 0;

  *destination++ = *state.source++;

  while (1) {
    if (get_bit(&state)) {
      if (get_bit(&state)) {
        if (get_bit(&state)) {
          offset = 0;

          for (int i = 4; i; i--) {
            offset = (offset << 1) + get_bit(&state);
          }

          if (offset) {
            *destination = *(destination - offset);
            destination++;
          } else {
            *destination++ = 0x00;
          }

          last_was_match = 0;
        } else {
          offset = *state.source++;

          length = 2 + (offset & 0x0001);

          offset >>= 1;

          if (offset) {
            for (; length; length--) {
              *destination = *(destination - offset);
              destination++;
            }
          } else {
            break;
          }

          last_offset = offset;
          last_was_match = 1;
        }
      } else {
        offset = get_gamma(&state);

        if (last_was_match == 0 && offset == 2) {
          offset = last_offset;

          length = get_gamma(&state);

          for (; length; length--) {
            *destination = *(destination - offset);
            destination++;
          }
        } else {
          if (last_was_match == 0) {
            offset -= 3;
          } else {
            offset -= 2;
          }

          offset <<= 8;
          offset += *state.source++;

          length = get_gamma(&state);
          if (offset >= 1280) {
            length++;
          }
          if (offset < 128) {
            length += 2;
          }

          for (; length; length--) {
            *destination = *(destination - offset);
            destination++;
          }

          last_offset = offset;
        }

        last_was_match = 1;
      }
    } else {
      *destination++ = *state.source++;
      last_was_match = 0;
    }
  }
}
