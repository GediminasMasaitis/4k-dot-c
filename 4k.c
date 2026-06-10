// Minify thing thang
#define G(a, b...) b
#define H(a, b, c...) c
#define STATIC_0
#define STATIC_1 static
#define S(id) STATIC_##id

#pragma region libc shims

#ifdef _MSC_VER
#define __attribute__(...)
#endif

#if defined(_WIN64) || defined(__x86_64__) || defined(__ppc64__) ||            \
    defined(__aarch64__)
#define ARCH64 1
#ifdef NOSTDLIB
#define size_t unsigned long long
#define ssize_t long long
#endif
#else
#define ARCH32 1
#ifdef NOSTDLIB
#define size_t unsigned int
#define ssize_t int
#endif
#endif

#define i64 long long
#define u64 unsigned long long
#define i32 int
#define u32 unsigned
#define i16 short
#define u16 unsigned short
#define i8 signed char
#define u8 unsigned char

#ifdef NOSTDLIB

#if __GNUC__ < 13
typedef _Bool bool;
#define true 1
#define false 0
#endif

#define NULL ((void *)0)

enum [[nodiscard]] {
  stdin = 0,
  stdout = 1,
  stderr = 2,
};

G(
    1, S(1) void exit_now() {
      asm volatile("syscall" : : "a"(60));
      __builtin_unreachable();
    })

G(
    1, [[nodiscard]] S(1) u32 atoi(const char *restrict string) {
      u32 result = 0;
      while (*string)
        result = result * 10 + *string++ - '0';
      return result;
    })

G(
    1, // Non-standard, gets but a word instead of a line
    S(0) bool getl(char *restrict string) {
      while (true) {
        ssize_t result;
        asm volatile("syscall"
                     : "=a"(result)
                     : "0"(0), "D"(stdin), "S"(string), "d"(1)
                     : "rcx", "r11", "memory");

    // Assume stdin never closes on mini build
#ifdef FULL
        if (result < 1) {
          exit_now();
        }
#endif

        const char ch = *string;
        if (ch <= ' ') {
          *string = 0;
          return ch != '\n';
        }

        string++;
      }
    })

G(
    1,
    S(0) void putl(const char *const restrict string) {
      i32 length = 0;
      while (string[length]) {
        ssize_t ret;
        asm volatile("syscall"
                     : "=a"(ret)
                     : "0"(1), "D"(stdout), "S"(&string[length]), "d"(1)
                     : "rcx", "r11", "memory");
        length++;
      }
    }

    S(1) void puts(const char *const restrict string) {
      putl(string);
      putl("\n");
    })

G(
    1, [[nodiscard]] static bool strcmp(const char *restrict lhs,
                                        const char *restrict rhs) {
      while (*lhs || *rhs) {
        if (*lhs != *rhs) {
          return true;
        }
        lhs++;
        rhs++;
      }
      return false;
    })

G(
    1,
    typedef struct [[nodiscard]] {
      ssize_t tv_sec;  // seconds
      ssize_t tv_nsec; // nanoseconds
    } timespec;

    [[nodiscard]] S(0) u64 get_time() {
      timespec ts;
      ssize_t ret; // Unused
      asm volatile("syscall"
                   : "=a"(ret)
                   : "0"(228), "D"(1), "S"(&ts)
                   : "rcx", "r11", "memory");
      return G(2, ts.tv_nsec) +
             G(2, G(3, ts.tv_sec) * G(3, 1000 * 1000 * 1000ULL));
    })

#ifdef FULL
static void *mmap_anon(size_t length) {
  void *p;
  register u64 r10 asm("r10") = 0x22;  // MAP_PRIVATE | MAP_ANONYMOUS
  register u64 r8 asm("r8") = (u64)-1; // fd
  register u64 r9 asm("r9") = 0;       // offset
  asm volatile("syscall"
               : "=a"(p)
               : "0"(9), "D"((void *)0), "S"(length), "d"(3), "r"(r10), "r"(r8),
                 "r"(r9)
               : "rcx", "r11", "memory");
  return p;
}

static void munmap_sys(void *addr, size_t length) {
  ssize_t ret;
  asm volatile("syscall"
               : "=a"(ret)
               : "0"(11), "D"(addr), "S"(length)
               : "rcx", "r11", "memory");
}

static void *malloc(size_t size) {
  size_t total = size + 16;
  size_t *base = mmap_anon(total);
  base[0] = total;
  return (char *)base + 16;
}

static void free(void *ptr) {
  if (!ptr)
    return;
  size_t *base = (size_t *)((char *)ptr - 16);
  munmap_sys(base, *base);
}

static void *calloc(size_t n, size_t sz) { return malloc(n * sz); }

static void *memset(void *dst, int c, size_t n) {
  void *ret = dst;
  asm volatile("rep stosb"
               : "+D"(dst), "+c"(n)
               : "a"((unsigned char)c)
               : "memory");
  return ret;
}

static void *memcpy(void *dst, const void *src, size_t n) {
  void *ret = dst;
  asm volatile("rep movsb" : "+D"(dst), "+S"(src), "+c"(n) : : "memory");
  return ret;
}

static void print_u64(u64 val) {
  char buf[24];
  i32 i = 23;
  buf[i--] = 0;
  if (!val)
    buf[i--] = '0';
  else
    while (val) {
      buf[i--] = '0' + val % 10;
      val /= 10;
    }
  putl(&buf[i + 1]);
}

static void print_i32(i32 val) {
  if (val < 0) {
    putl("-");
    val = -val;
  }
  print_u64((u64)(u32)val);
}

static void printf(const char *fmt, ...) {
  __builtin_va_list ap;
  __builtin_va_start(ap, fmt);
  char ch[2] = {0, 0};
  while (*fmt) {
    if (*fmt != '%') {
      ch[0] = *fmt++;
      putl(ch);
      continue;
    }
    fmt++;
    if (*fmt == 'i' || *fmt == 'd') {
      print_i32(__builtin_va_arg(ap, i32));
      fmt++;
    } else if (fmt[0] == 'l' && fmt[1] == 'l' && fmt[2] == 'u') {
      print_u64(__builtin_va_arg(ap, u64));
      fmt += 3;
    } else {
      ch[0] = '%';
      putl(ch);
      ch[0] = *fmt++;
      putl(ch);
    }
  }
  __builtin_va_end(ap);
}
#endif

#else
#include <pthread.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

[[nodiscard]] static u64 get_time() {
  struct timespec ts;
  clock_gettime(CLOCK_MONOTONIC, &ts);
  return ts.tv_sec * 1000 * 1000 * 1000ULL + ts.tv_nsec;
}

static void exit_now() { exit(0); }

S(1) bool getl(char *restrict string) {
  while (true) {

    const char c = getchar();

    if (c == EOF) {
      exit_now();
    }

    if (c == '\n') {
      *string = 0;
      return false;
    }

    if (c == ' ') {
      *string = 0;
      return true;
    }

    *string = c;
    string++;
  }
}

static void putl(const char *const restrict string) {
  fputs(string, stdout);
  fflush(stdout);
}

#endif

#pragma endregion

#pragma region base

enum [[nodiscard]] { None, Pawn, Knight, Bishop, Rook, Queen, King };

typedef struct [[nodiscard]] {
  G(4, u8 from; u8 to;)
  G(4, u8 takes_piece;)
  G(4, u8 promo;)
} Move;

typedef struct [[nodiscard]] {
  G(5, u64 ep;)
  G(5, u64 pieces[7];)
  G(5, u64 colour[2];)
  G(6, bool castling[4];)
  G(6, u8 padding[11];)
  G(6, bool flipped;)
} Position;

#ifdef ASSERTS
#define assert(condition)                                                      \
  if (!(condition)) {                                                          \
    printf("Assert failed on line %i: ", __LINE__);                            \
    puts(#condition);                                                          \
    exit_now();                                                                \
  }
#else
#define assert(condition)
#endif

G(
    7,
    [[nodiscard]] S(1) bool move_string_equal(G(8, const char *restrict rhs),
                                              G(8, const char *restrict lhs)) {
      return (G(9, *(const u64 *)rhs) ^ G(9, *(const u64 *)lhs)) << 24 == 0;
    })

G(
    7, [[nodiscard]] S(1)
           u64 flip_bb(const u64 bb) { return __builtin_bswap64(bb); })
G(7, [[nodiscard]] S(1) i32 lsb(u64 bb) { return __builtin_ctzll(bb); })

G(
    7, [[nodiscard]] S(1)
           u64 shift(H(10, 1, const i32 shift), H(10, 1, const u64 mask),
                     H(10, 1, const u64 bb)) {
             return shift > 0 ? G(11, bb << shift) & G(11, mask)
                              : G(12, bb >> -shift) & G(12, mask);
           })

G(
    7, [[nodiscard]] S(1)
           i32 count(const u64 bb) { return __builtin_popcountll(bb); })

G(
    13, [[nodiscard]] S(1) u64 west(const u64 bb) {
      return G(14, bb >> 1) & G(14, ~0x8080808080808080ull);
    })

G(13, [[nodiscard]] S(1) u64 north(const u64 bb) { return bb << 8; })

G(13, [[nodiscard]] S(1) u64 south(const u64 bb) { return bb >> 8; })

G(
    13, [[nodiscard]] S(1) u64 east(const u64 bb) {
      return G(15, bb << 1) & G(15, ~0x101010101010101ull);
    })

G(
    16, [[nodiscard]] S(1) u64 southeast(const u64 bb) {
      return G(17, G(18, east)(G(18, south)(bb)));
      return G(17, shift(H(10, 2, -7), H(10, 2, ~0x101010101010101ull),
                         H(10, 2, bb)));
    })

G(
    16, [[nodiscard]] S(1) u64 northeast(const u64 bb) {
      return G(19, G(20, east)(G(20, north)(bb)));
      return G(19, shift(H(10, 3, 9), H(10, 3, ~0x101010101010101ull),
                         H(10, 3, bb)));
    })

G(
    16, [[nodiscard]] S(1) u64 northwest(const u64 bb) {
      return G(21, shift(H(10, 4, 7), H(10, 4, ~0x8080808080808080ull),
                         H(10, 4, bb)));
      return G(21, G(22, west)(G(22, north)(bb)));
    })

G(
    16, [[nodiscard]] S(1) u64 southwest(const u64 bb) {
      return G(23, G(24, west)(G(24, south)(bb)));
      return G(23, shift(H(10, 5, -9), H(10, 5, ~0x8080808080808080ull),
                         H(10, 5, bb)));
    })

G(25, S(1) u64 diag_mask[64];)

G(
    25, [[nodiscard]] S(1)
            u64 ray(H(26, 1, const u64 blockers), H(26, 1, const u64 mask),
                    H(26, 1, const u64 bb), H(26, 1, const i32 shift_by)) {
              u64 result =
                  shift(H(10, 6, shift_by), H(10, 6, mask), H(10, 6, bb));
              for (i32 i = 5; i >= 0; i--) {
                result |= shift(H(10, 7, shift_by), H(10, 7, mask),
                                H(10, 7, result & ~blockers));
              }
              return result;
            })

G(
    25, [[nodiscard]] S(0) u64 xattack(H(27, 1, const u64 blockers),
                                       H(27, 1, const u64 bb),
                                       H(27, 1, const u64 dir_mask)) {
      return G(28, dir_mask) &
             G(28, (G(29, (G(30, blockers) & G(30, dir_mask)) - bb) ^
                    G(29, flip_bb(flip_bb(G(31, blockers) & G(31, dir_mask)) -
                                  flip_bb(bb)))));
    })

G(
    32, [[nodiscard]] S(0) u64 king(const u64 bb) {
      const u64 vertical = G(33, north(bb)) | G(33, south(bb));
      const u64 vertical_inclusive = G(34, bb) | G(34, vertical);
      return G(35, vertical) | G(35, east(vertical_inclusive)) |
             G(35, west(vertical_inclusive));
    })

G(
    32, [[nodiscard]] S(0) u64 knight(const u64 bb) {
      G(36, const u64 east_bb = east(bb);)
      G(36, const u64 west_bb = west(bb);)
      G(37, const u64 horizontal1 = G(38, west_bb) | G(38, east_bb);)
      G(37,
        const u64 horizontal2 = G(39, east(east_bb)) | G(39, west(west_bb));)
      return G(40, horizontal2 << 8) | G(40, horizontal2 >> 8) |
             G(40, horizontal1 >> 16) | G(40, horizontal1 << 16);
    })

G(
    32, [[nodiscard]] S(0) u64 rook(H(41, 1, const u64 blockers),
                                    H(41, 1, const u64 bb)) {
      assert(count(bb) == 1);
      return G(42, // West
               ray(H(26, 2, blockers), H(26, 2, ~0x8080808080808080ull),
                   H(26, 2, bb), H(26, 2, -1))) |
             G(42, // East
               ray(H(26, 3, blockers), H(26, 3, ~0x101010101010101ull),
                   H(26, 3, bb), H(26, 3, 1))) |
             G(42, xattack(H(27, 2, blockers), H(27, 2, bb),
                           H(27, 2, bb ^ 0x101010101010101ULL << lsb(bb) % 8)));
    })

G(
    32, [[nodiscard]] S(0) u64 bishop(H(43, 1, const u64 blockers),
                                      H(43, 1, const u64 bb)) {
      assert(count(bb) == 1);
      const i32 sq = lsb(bb);
      return G(44, xattack(H(27, 3, blockers), H(27, 3, bb),
                           H(27, 3, diag_mask[sq]))) |
             G(44,
               xattack(H(27, 4, blockers), H(27, 4, bb),
                       H(27, 4, flip_bb(diag_mask[G(45, sq) ^ G(45, 56)]))));
    })

G(
    46,
    S(1) void swapu32(G(47, u32 *const rhs), G(47, u32 *const lhs)) {
      const u32 temp = *lhs;
      *lhs = *rhs;
      *rhs = temp;
    }

    S(1) void swapmoves(G(48, Move *const rhs), G(48, Move *const lhs)) {
      swapu32(G(49, (u32 *)lhs), G(49, (u32 *)rhs));
    })

G(
    46, S(1) void move_str(H(50, 1, const i32 flip),
                           H(50, 1, const Move *restrict move),
                           H(50, 1, char *restrict str)) {
      assert(move->from >= 0);
      assert(move->from < 64);
      assert(move->to >= 0);
      assert(move->to < 64);
      assert(move->from != move->to);
      assert(move->promo == None || move->promo == Knight ||
             move->promo == Bishop || move->promo == Rook ||
             move->promo == Queen);

      G(51, str[4] = "\0\0nbrq"[move->promo];)

      G(51, str[5] = '\0';)
      G(
          51, // Hack to save bytes, technically UB but works on GCC 14.2
          for (i32 i = 0; i < 2; i++) {
            G(52, str[i * 2 + 1] = '1' + ((&move->from)[i] / 8 ^ 7 * flip);)
            G(52, str[i * 2] = 'a' + (&move->from)[i] % 8;)
          })
    })

G(
    46, [[nodiscard]] S(1) bool move_equal(G(53, Move *const rhs),
                                           G(53, Move *const lhs)) {
      return G(54, *(u32 *)lhs) == G(54, *(u32 *)rhs);
    })

G(
    46, [[nodiscard]] S(0)
            i32 piece_on(H(55, 1, const Position *const restrict pos),
                         H(55, 1, const i32 sq)) {
              assert(sq >= 0);
              assert(sq < 64);
              for (i32 i = Pawn; i <= King; ++i) {
                if (G(56, pos->pieces[i]) & G(56, 1ull << sq)) {
                  return i;
                }
              }
              return None;
            })

G(
    57,
    [[nodiscard]] S(0) i32 is_attacked(H(58, 1,
                                         const Position *const restrict pos),
                                       H(58, 1, const u64 bb)) {
      assert(count(bb) == 1);
      const u64 theirs = pos->colour[1];
      G(59, const u64 pawns = theirs & pos->pieces[Pawn];)
      G(59, const u64 blockers = theirs | pos->colour[0];)
      return G(60, G(61, (G(62, southwest(pawns)) | G(62, southeast(pawns)))) &
                       G(61, bb)) ||
             G(60, G(63, rook(H(41, 2, blockers), H(41, 2, bb))) &
                       G(63, (pos->pieces[Rook] | pos->pieces[Queen])) &
                       G(63, theirs)) ||
             G(60,
               G(64, pos->pieces[King]) & G(64, theirs) & G(64, king(bb))) ||
             G(60, G(65, bishop(H(43, 2, blockers), H(43, 2, bb))) &
                       G(65, theirs) &
                       G(65, (pos->pieces[Bishop] | pos->pieces[Queen]))) ||
             G(60,
               G(66, pos->pieces[Knight]) & G(66, knight(bb)) & G(66, theirs));
    })

G(
    57, S(0) void flip_pos(Position *const restrict pos) {
      G(67, pos->flipped ^= 1;)
      G(
          67, // Hack to flip the first 10 bitboards in Position.
          // Technically UB but works in GCC 14.2
          u64 *pos_ptr = (u64 *)pos;
          for (i32 i = 0; i < 10; i++) { pos_ptr[i] = flip_bb(pos_ptr[i]); })

      G(67, pos->colour[0] ^= pos->colour[1]; pos->colour[1] ^= pos->colour[0];
        pos->colour[0] ^= pos->colour[1];)
      G(67, u32 *c = (u32 *)pos->castling;
        *c = G(68, (*c >> 16)) | G(68, (*c << 16));)
    })

G(
    57, [[nodiscard]] S(0) u64 get_mobility(H(69, 1, const Position *pos),
                                            H(69, 1, const i32 piece),
                                            H(69, 1, const i32 sq)) {
      u64 moves = 0;
      const u64 bb = 1ULL << sq;
      G(70, if (piece == Knight) { moves = knight(bb); })
      else G(70, if (piece == King) { moves = king(bb); }) else {
        const u64 blockers = G(71, pos->colour[0]) | G(71, pos->colour[1]);
        G(
            72, if (piece != Bishop) {
              moves |= rook(H(41, 3, blockers), H(41, 3, bb));
            })
        G(
            72, if (piece != Rook) {
              moves |= bishop(H(43, 3, blockers), H(43, 3, bb));
            })
      }
      return moves;
    })

S(0) i32 find_in_check(const Position *restrict pos) {
  return is_attacked(
      H(58, 2, pos),
      H(58, 2, G(73, pos->colour[0]) & G(73, pos->pieces[King])));
}

G(
    74,
    S(1) Move *generate_piece_moves(H(75, 1, const u64 to_mask),
                                    H(75, 1, Move *restrict movelist),
                                    H(75, 1, const Position *restrict pos)) {
      for (i32 piece = Knight; piece <= King; piece++) {
        assert(piece == Knight || piece == Bishop || piece == Rook ||
               piece == Queen || piece == King);
        u64 copy = G(76, pos->colour[0]) & G(76, pos->pieces[piece]);
        while (copy) {
          const u8 from = lsb(copy);
          assert(from >= 0);
          assert(from < 64);
          G(77, copy &= copy - 1;)

          G(77, u64 moves = G(78, to_mask) &
                            G(78, get_mobility(H(69, 2, pos), H(69, 2, piece),
                                               H(69, 2, from)));)

          while (moves) {
            const u8 to = lsb(moves);
            assert(to >= 0);
            assert(to < 64);

            G(79, moves &= moves - 1;)
            G(79, *movelist++ = ((Move){
                      .from = from,
                      .to = to,
                      .promo = None,
                      .takes_piece = piece_on(H(55, 2, pos), H(55, 2, to))});)
          }
        }
      }

      return movelist;
    })

G(
    74, S(0) i32 makemove(H(80, 1, Position *const restrict pos),
                          H(80, 1, const Move *const restrict move)) {
      assert(move->from >= 0);
      assert(move->from < 64);
      assert(move->to >= 0);
      assert(move->to < 64);
      assert(move->from != move->to);
      assert(move->promo == None || move->promo == Knight ||
             move->promo == Bishop || move->promo == Rook ||
             move->promo == Queen);
      assert(move->takes_piece != King);
      assert(move->takes_piece == piece_on(H(55, 3, pos), H(55, 3, move->to)));

      G(81, const u64 to = 1ull << move->to;)
      G(81, const u64 from = 1ull << move->from;)
      G(82, const i32 piece = piece_on(H(55, 4, pos), H(55, 4, move->from));
        assert(piece != None);)
      G(82, const u64 mask = G(83, from) | G(83, to);)

      G(
          84, // Castling
          if (piece == King) {
            const u64 bb = move->to - move->from == 2   ? 0xa0
                           : move->from - move->to == 2 ? 0x9
                                                        : 0;
            G(85, pos->pieces[Rook] ^= bb;)
            G(85, pos->colour[0] ^= bb;)
          })

      G(84, pos->colour[0] ^= mask;)

      // Move the piece
      G(
          84, // Captures
          if (move->takes_piece != None) {
            G(86, pos->colour[1] ^= to;)
            G(86, pos->pieces[move->takes_piece] ^= to;)
          })
      G(84, pos->pieces[piece] ^= mask;)

      // En passant
      if (G(87, piece == Pawn) && G(87, to == pos->ep)) {
        G(88, pos->pieces[Pawn] ^= to >> 8;)
        G(88, pos->colour[1] ^= to >> 8;)
      }
      pos->ep = 0;

      G(
          89, // Pawn double move
          if (G(90, move->to - move->from == 16) && G(90, piece == Pawn)) {
            pos->ep = to >> 8;
          })

      G(
          89, // Promotions
          if (move->promo != None) {
            G(91, pos->pieces[Pawn] ^= to;)
            G(91, pos->pieces[move->promo] ^= to;)
          })

      G(89, // Update castling permissions
        const u64 oppMask = mask >> 56;
        G(92, pos->castling[3] &= !(oppMask & 0x11);)
            G(92, pos->castling[1] &= !(mask & 0x11);)
                G(92, pos->castling[0] &= !(mask & 0x90);)
                    G(92, pos->castling[2] &= !(oppMask & 0x90);))

      if (find_in_check(pos)) {
        return false;
      }

      flip_pos(pos);

      assert(!(pos->colour[0] & pos->colour[1]));
      assert(!(pos->pieces[Pawn] & pos->pieces[Knight]));
      assert(!(pos->pieces[Pawn] & pos->pieces[Bishop]));
      assert(!(pos->pieces[Pawn] & pos->pieces[Rook]));
      assert(!(pos->pieces[Pawn] & pos->pieces[Queen]));
      assert(!(pos->pieces[Pawn] & pos->pieces[King]));
      assert(!(pos->pieces[Knight] & pos->pieces[Bishop]));
      assert(!(pos->pieces[Knight] & pos->pieces[Rook]));
      assert(!(pos->pieces[Knight] & pos->pieces[Queen]));
      assert(!(pos->pieces[Knight] & pos->pieces[King]));
      assert(!(pos->pieces[Bishop] & pos->pieces[Rook]));
      assert(!(pos->pieces[Bishop] & pos->pieces[Queen]));
      assert(!(pos->pieces[Bishop] & pos->pieces[King]));
      assert(!(pos->pieces[Rook] & pos->pieces[Queen]));
      assert(!(pos->pieces[Rook] & pos->pieces[King]));
      assert(!(pos->pieces[Queen] & pos->pieces[King]));

      // Return move legality
      return true;
    })

G(
    74, S(0) Move *generate_pawn_moves(H(93, 1, const i32 offset),
                                       H(93, 1, u64 to_mask),
                                       H(93, 1, Move *restrict movelist),
                                       H(93, 1, const Position *const pos)) {
      while (to_mask) {
        const u8 to = lsb(to_mask);
        to_mask &= to_mask - 1;
        const u8 from = G(94, to) + G(94, offset);
        assert(from >= 0);
        assert(from < 64);
        assert(to >= 0);
        assert(to < 64);
        assert(piece_on(H(55, 5, pos), H(55, 5, from)) == Pawn);
        const u8 takes = piece_on(H(55, 6, pos), H(55, 6, to));
        if (to > 55) {
          for (u8 piece = Queen; piece >= Knight; piece--) {
            *movelist++ = ((Move){
                .from = from, .to = to, .promo = piece, .takes_piece = takes});
          }
        } else {
          *movelist++ = ((Move){
              .from = from, .to = to, .promo = None, .takes_piece = takes});
        }
      }

      return movelist;
    })

enum { max_moves = 218 };

[[nodiscard]] S(1) i32 movegen(H(95, 1, const Position *const restrict pos),
                               H(95, 1, const i32 only_captures),
                               H(95, 1, Move *restrict movelist)) {

  G(96, const u64 all = G(97, pos->colour[0]) | G(97, pos->colour[1]);)
  G(96, const Move *start = movelist;)
  G(96, const u64 to_mask = only_captures ? pos->colour[1] : ~pos->colour[0];)
  G(
      98, // PAWN PROMOTIONS
      if (!only_captures) {
        movelist = generate_pawn_moves(
            H(93, 2, -16),
            H(93, 2,
              G(99, north(G(100,
                            north(G(101, pos->colour[0]) &
                                  G(101, pos->pieces[Pawn]) & G(101, 0xFF00))) &
                          G(100, ~all))) &
                  G(99, ~all)),
            H(93, 2, movelist), H(93, 2, pos));
      })
  G(98, // PAWN DOUBLE MOVES
    movelist = generate_pawn_moves(
        H(93, 3, -8),
        H(93, 3,
          north(G(102, G(103, pos->colour[0]) & G(103, pos->pieces[Pawn]))) &
              G(102, ~all) &
              G(102, (only_captures ? 0xFF00000000000000ull : ~0ull))),
        H(93, 3, movelist), H(93, 3, pos));)
  G(98, // PAWN EAST CAPTURES
    movelist = generate_pawn_moves(
        H(93, 4, -9),
        H(93, 4,
          G(104,
            northeast(G(105, pos->colour[0]) & G(105, pos->pieces[Pawn]))) &
              G(104, (G(106, pos->colour[1]) | G(106, pos->ep)))),
        H(93, 4, movelist), H(93, 4, pos));)
  G(98, // PAWN WEST CAPTURES
    movelist = generate_pawn_moves(
        H(93, 5, -7),
        H(93, 5,
          G(107,
            northwest(G(108, pos->colour[0]) & G(108, pos->pieces[Pawn]))) &
              G(107, (G(109, pos->colour[1]) | G(109, pos->ep)))),
        H(93, 5, movelist), H(93, 5, pos));)
  G(
      98, // LONG CASTLE
      if (G(110, !only_captures) && G(110, !(G(111, all) & G(111, 0xEull))) &&
          G(110, pos->castling[1]) &&
          G(112, !is_attacked(H(58, 3, pos), H(58, 3, 1ULL << 3))) &&
          G(112, !is_attacked(H(58, 4, pos), H(58, 4, 1ULL << 4)))) {
        *movelist++ =
            (Move){.from = 4, .to = 2, .promo = None, .takes_piece = None};
      })
  G(
      98, // SHORT CASTLE
      if (G(113, !only_captures) && G(113, !(G(114, all) & G(114, 0x60ull))) &&
          G(113, pos->castling[0]) &&
          G(115, !is_attacked(H(58, 5, pos), H(58, 5, 1ULL << 5))) &&
          G(115, !is_attacked(H(58, 6, pos), H(58, 6, 1ULL << 4)))) {
        *movelist++ =
            (Move){.from = 4, .to = 6, .promo = None, .takes_piece = None};
      })
  movelist = generate_piece_moves(H(75, 2, to_mask), H(75, 2, movelist),
                                  H(75, 2, pos));

  const i32 num_moves = movelist - start;
  assert(num_moves < max_moves);
  return num_moves;
}

#pragma endregion

#pragma region engine

[[nodiscard]] S(1) u64
    perft(const Position *const restrict pos, const i32 depth) {
  if (depth == 0) {
    return 1;
  }

  u64 nodes = 0;
  Move moves[max_moves];
  const i32 num_moves =
      movegen(H(95, 2, pos), H(95, 2, false), H(95, 2, moves));

  for (i32 i = 0; i < num_moves; ++i) {
    Position npos = *pos;

    // Check move legality
    if (!makemove(H(80, 2, &npos), H(80, 2, &moves[i]))) {
      continue;
    }

    nodes += perft(&npos, depth - 1);
  }

  return nodes;
}

static bool get_fen(Position *restrict pos, char *restrict fen) {
  __builtin_memset(pos, 0, sizeof(Position));
  const char *p = fen;

  // PIECES
  i32 sq = 56;
  while (*p && *p != ' ') {
    const char c = *p;
    if (c == '/') {
      sq -= 16;
    } else if (c >= '1' && c <= '8') {
      sq += c - '0';
    } else {
      const bool side = c >= 'a' && c <= 'z';
      const char lowercase = c | 32;
      i32 piece;
      switch (lowercase) {
      case 'p':
        piece = Pawn;
        break;
      case 'n':
        piece = Knight;
        break;
      case 'b':
        piece = Bishop;
        break;
      case 'r':
        piece = Rook;
        break;
      case 'q':
        piece = Queen;
        break;
      case 'k':
        piece = King;
        break;
      default:
        piece = None;
        break;
      }
      pos->colour[side] |= 1ull << sq;
      pos->pieces[piece] |= 1ull << sq;
      sq++;
    }
    p++;
  }

  // SIDE TO MOVE
  getl(fen);
  p = fen;
  const bool black_to_move = *p == 'b';

  // CASTLING
  getl(fen);
  p = fen;
  if (*p != '-') {
    while (*p && *p != ' ') {
      switch (*p) {
      case 'K':
        pos->castling[0] = true;
        break;
      case 'Q':
        pos->castling[1] = true;
        break;
      case 'k':
        pos->castling[2] = true;
        break;
      case 'q':
        pos->castling[3] = true;
        break;
      default:
        break;
      }
      p++;
    }
  }

  // EN PASSANT
  bool more = getl(fen);
  p = fen;
  if (*p != '-') {
    const i32 file = p[0] - 'a';
    const i32 rank = p[1] - '1';
    pos->ep = 1ull << (rank * 8 + file);
  }

  if (black_to_move) {
    flip_pos(pos);
  }
  return more;
}

typedef struct [[nodiscard]] __attribute__((packed)) {
  i16 material[6];
  H(116, 1,
    H(117, 1, i8 pawn_threat[5];) H(117, 1, i8 king_shield[2];)
        H(117, 1, i8 bishop_pawns[2];))
  H(116, 1,
    H(118, 1, i8 passed_pawns[6];) H(118, 1, i8 pst_rank[48];)
        H(118, 1, i8 bishop_pair;) H(118, 1, i8 king_attacks[5];)
            H(118, 1, i8 phalanx_pawn;) H(118, 1, i8 protected_pawn;))
  H(116, 1,
    H(119, 1, i8 mobilities[5];) H(119, 1, i8 pst_file[48];)
        H(119, 1, i8 passed_blocked_pawns[6];) H(119, 1, i8 open_files[12];)
            H(119, 1, i8 tempo;) H(119, 1, i8 pawn_attacked_penalty[2];))
} EvalParams;

typedef struct [[nodiscard]] __attribute__((packed)) {
  i32 material[6];
  H(116, 2,
    H(117, 2, i32 pawn_threat[5];) H(117, 2, i32 king_shield[2];)
        H(117, 2, i32 bishop_pawns[2];))
  H(116, 2,
    H(118, 2, i32 passed_pawns[6];) H(118, 2, i32 pst_rank[48];)
        H(118, 2, i32 bishop_pair;) H(118, 2, i32 king_attacks[5];)
            H(118, 2, i32 phalanx_pawn;) H(118, 2, i32 protected_pawn;))
  H(116, 2,
    H(119, 2, i32 mobilities[5];) H(119, 2, i32 pst_file[48];)
        H(119, 2, i32 passed_blocked_pawns[6];) H(119, 2, i32 open_files[12];)
            H(119, 2, i32 tempo;) H(119, 2, i32 pawn_attacked_penalty[2];))
} EvalParamsMerged;

typedef struct [[nodiscard]] __attribute__((packed)) {
  i8 phases[6];
  G(120, EvalParams eg;)
  G(120, EvalParams mg;)
} EvalParamsInitial;

G(121, S(0) EvalParamsMerged eval_params;)

G(121, // EVAL PARAMETERS
  __attribute__((aligned(8))) S(1)
      const EvalParamsInitial initial_params = {
          .phases = {0, 0, 1, 1, 2, 4},
          .mg = {.material = {0, 67, 292, 331, 383, 841},
                 .pst_rank =
                     {
                         0,   -15, -17, -5,  7,  29, 95,  0,   // Pawn
                         -19, -6,  7,   21,  30, 40, 18,  -90, // Knight
                         -8,  9,   15,  17,  18, 18, -4,  -65, // Bishop
                         5,   -5,  -10, -13, 2,  13, 1,   6,   // Rook
                         16,  17,  9,   0,   -4, -3, -20, -14, // Queen
                         -12, -7,  -16, -20, 11, 63, 70,  87,  // King
                     },
                 .pst_file =
                     {
                         -10, -12, -6,  3,   8,   13,  12, -7,  // Pawn
                         -30, -11, -1,  12,  14,  13,  7,  -5,  // Knight
                         -14, 0,   3,   0,   4,   -1,  11, -3,  // Bishop
                         -12, -8,  -2,  7,   12,  8,   6,  -11, // Rook
                         -14, -8,  -4,  0,   3,   4,   13, 6,   // Queen
                         -12, 12,  -13, -27, -28, -10, 11, 4,   // King
                     },
                 .mobilities = {7, 5, 2, 3, -2},
                 .king_attacks = {0, 13, 19, 14, 0},
                 .pawn_threat = {-18, -8, -7, -8, -1},
                 .open_files = {8, -10, -10, 17, -2, -24, 8, -12, -14, 33, -13,
                                -57},
                 .passed_pawns = {-7, -11, -9, 11, 32, 95},
                 .passed_blocked_pawns = {0, -1, 5, 14, 16, -38},
                 .protected_pawn = 16,
                 .phalanx_pawn = 9,
                 .bishop_pair = 27,
                 .bishop_pawns = {-5, -5},
                 .king_shield = {29, 20},
                 .pawn_attacked_penalty = {-56, -108},
                 .tempo = 28},
          .eg = {.material = {0, 88, 391, 431, 722, 1343},
                 .pst_rank =
                     {
                         0,   -5,  -9,  -7, 1,  20, 104, 0,   // Pawn
                         -32, -20, -7,  17, 22, 8,  3,   10,  // Knight
                         -15, -14, -1,  5,  8,  4,  4,   10,  // Bishop
                         -17, -19, -12, 5,  11, 10, 18,  5,   // Rook
                         -59, -43, -14, 13, 30, 26, 31,  16,  // Queen
                         -42, -1,  7,   20, 30, 28, 14,  -56, // King
                     },
                 .pst_file =
                     {
                         11,  11, -1, -11, -5, -1, 0,  -4,  // Pawn
                         -19, -4, 7,  16,  16, 5,  -1, -19, // Knight
                         -11, 0,  5,  10,  11, 5,  -3, -15, // Bishop
                         1,   6,  6,  -3,  -5, 1,  0,  -5,  // Rook
                         -23, -7, 4,  8,   15, 15, 0,  -10, // Queen
                         -33, 4,  18, 25,  26, 16, 3,  -40, // King
                     },
                 .mobilities = {5, 3, 4, 2, -6},
                 .king_attacks = {0, -3, -7, 7, 0},
                 .pawn_threat = {-2, -5, -18, -14, -9},
                 .open_files = {20, 1, 13, 6, 22, 14, 23, -8, 2, 12, 43, 9},
                 .passed_pawns = {9, 15, 40, 67, 116, 104},
                 .passed_blocked_pawns = {-9, -8, -32, -63, -112, -118},
                 .protected_pawn = 16,
                 .phalanx_pawn = 14,
                 .bishop_pair = 62,
                 .bishop_pawns = {-12, -1},
                 .king_shield = {-12, -7},
                 .pawn_attacked_penalty = {-25, -128},
                 .tempo = 15}};)

G(
    121,
    [[nodiscard]] S(1) i32 combine_eval_param(H(122, 1, const i32 mg_val),
                                              H(122, 1, const i32 eg_val)) {
      return G(123, mg_val) + G(123, (eg_val << 16));
    })

S(0) i32 eval(Position *const restrict pos) {
  G(124, i32 score = eval_params.tempo;)
  G(124, i32 phase = 0;)

  for (i32 c = 0; c < 2; c++) {

    G(125, const u64 opp_king_zone =
               king(G(126, pos->colour[1]) & G(126, pos->pieces[King]));)

    G(
        125, u64 pawns[2]; for (i32 i = 0; i < 2; i++) {
          pawns[i] = G(127, pos->colour[i]) & G(127, pos->pieces[Pawn]);
        } const u64 attacked_by_pawns = G(128, southwest(pawns[1])) |
                                        G(128, southeast(pawns[1]));
        G(129,
          const u64 no_passers = G(132, pawns[1]) | G(132, attacked_by_pawns);)
            G(129, // PHALANX PAWNS
              score -=
              G(133, eval_params.phalanx_pawn) *
              G(133, count(G(134, pawns[1]) & G(134, west(pawns[1]))));)
                G(129, // PROTECTED PAWNS
                  score -=
                  G(130, eval_params.protected_pawn) *
                  G(130, count(G(131, pawns[1]) & G(131, attacked_by_pawns)));))
    G(
        125, // BISHOP PAIR
        if (count(G(135, pos->pieces[Bishop]) & G(135, pos->colour[0])) > 1) {
          score += eval_params.bishop_pair;
        })
    for (i32 p = Pawn; p <= King; p++) {
      u64 copy = G(136, pos->colour[0]) & G(136, pos->pieces[p]);
      while (copy) {
        const i32 sq = lsb(copy);
        G(137, const i32 rank = sq >> 3;)
        G(137, copy &= copy - 1;)
        G(137, phase += initial_params.phases[p];)
        G(137, const u64 piece_bb = 1ULL << sq;)
        G(137, const i32 file = G(138, sq) & G(138, 7);)
        G(137, const u64 in_front = 0x101010101010101ULL << sq;)
        G(93, // MATERIAL
          score += eval_params.material[p];)

        G(
            93, // PASSED PAWNS
            if (G(139, p == Pawn) &&
                G(139, !(G(140, in_front) & G(140, no_passers)))) {
              G(141, score += eval_params.passed_pawns[rank - 1];)

              G(
                  141, if (G(142, north(piece_bb)) & G(142, pos->colour[1])) {
                    score += eval_params.passed_blocked_pawns[rank - 1];
                  })
            })
        G(93, // SPLIT PIECE-SQUARE TABLES FOR RANK
          score +=
          eval_params
              .pst_rank[G(143, G(144, (p - 1)) * G(144, 8)) + G(143, rank)];)

        G(
            93, // OPEN FILES / DOUBLED PAWNS
            if ((G(145, north(in_front)) & G(145, pawns[0])) == 0) {
              score +=
                  eval_params.open_files[G(146, G(147, !(G(148, in_front) &
                                                         G(148, pawns[1]))) *
                                                    G(147, 6)) +
                                         G(146, -1) + G(146, p)];
            })

        G(93, // SPLIT PIECE-SQUARE TABLES FOR FILE
          score +=
          eval_params
              .pst_file[G(149, G(150, (p - 1)) * G(150, 8)) + G(149, file)];)

        G(
            93, if (p > Pawn) {
              G(
                  151, // PAWN PUSH THREATS
                  if (G(152, in_front) & G(152, ~piece_bb) &
                      G(152, attacked_by_pawns)) {
                    score += eval_params.pawn_threat[p - 2];
                  })

              G(
                  151, // KING SHIELD
                  if (G(153, G(154, p) == G(154, King)) &&
                      G(153, G(155, piece_bb) & G(155, 0xC3D7))) {
                    const u64 shield = file < 3 ? 0x700 : 0xE000;
                    G(156, score += G(157, count(G(158, north(shield)) &
                                                 G(158, pawns[0]))) *
                                    G(157, eval_params.king_shield[1]);)
                    G(156, score +=
                           G(159, count(G(160, shield) & G(160, pawns[0]))) *
                           G(159, eval_params.king_shield[0]);)
                  })

              G(
                  151, // PIECES ATTACKED BY PAWNS
                  if (G(161, piece_bb) & G(161, no_passers)) {
                    score += eval_params.pawn_attacked_penalty[c];
                  })

              G(
                  151, // BISHOP COLOUR PAWNS
                  if (G(162, p) == G(162, Bishop)) {
                    u64 mask = 0xAA55AA55AA55AA55ULL;
                    if (!(G(163, piece_bb) & G(163, mask))) {
                      mask = ~mask;
                    }
                    for (i32 i = 0; i < 2; i++) {
                      score += G(164, eval_params.bishop_pawns[i]) *
                               G(164, count(G(165, pawns[i]) & G(165, mask)));
                    }
                  })

              G(151, const u64 mobility =
                         get_mobility(H(69, 3, pos), H(69, 3, p), H(69, 3, sq));

                G(166, // MOBILITY
                  score +=
                  G(167, count(G(168, ~pos->colour[0]) & G(168, mobility) &
                               G(168, ~attacked_by_pawns))) *
                  G(167, eval_params.mobilities[p - 2]);)

                    G(166, // KING ATTACKS
                      score +=
                      G(169, count(G(170, mobility) & G(170, opp_king_zone))) *
                      G(169, eval_params.king_attacks[p - 2]);))
            })
      }
    }

    G(75, score = -score;)
    G(75, flip_pos(pos);)
  }

  const i32 stronger_side_pawns_missing =
      8 - count(G(171, pos->colour[score < 0]) & G(171, pos->pieces[Pawn]));
  return (G(172, (i16)score) * G(172, phase) +
          G(173, ((score + 0x8000) >> 16)) *
              G(173, (128 - stronger_side_pawns_missing *
                                stronger_side_pawns_missing)) /
              128 * (24 - phase)) /
         24;
}

#ifdef FULL
u64 tt_length = 1 << 23; // 80MB
#else
enum : u64 { tt_length = 1ULL << 23 }; // 80MB
// enum : u64 { tt_length = 1ULL << 27 }; // 1.25GB
// enum : u64 { tt_length = 1ULL << 29 }; // 5GB
// enum : u64 { tt_length = 1ULL << 31 }; // 20GB
#endif
enum { Upper = 0, Lower = 1, Exact = 2 };
enum { max_ply = 96 };
enum { mate = 31744, inf = 32256 };
#ifdef NOSTDLIB
enum { thread_count = 4 };
#else
static i32 thread_count = 1;
#endif
enum { thread_stack_size = 1024 * 1024 };
enum { corrhist_size = 16384 };

typedef struct [[nodiscard]] {
  G(119, Move best_move;)
  G(119, u64 position_hash;)
  G(119, Move killer;)
  G(119, i32 num_moves;)
  G(119, i32 static_eval;)
} SearchStack;

typedef struct [[nodiscard]] __attribute__((packed)) {
  G(174, i16 score;)
  G(174, Move move;)
  G(174, u8 flag;)
  G(174, i8 depth;)
  G(174, u16 partial_hash;)
} TTEntry;
_Static_assert(sizeof(TTEntry) == 10);

typedef struct [[nodiscard]] {
#ifdef FULL
  i32 thread_id;
  u64 nodes;
#endif
  G(175, Position pos;)
  G(175, u64 max_time;)
  G(175, SearchStack stack[1024];)
  G(175, i32 corrhist[2][corrhist_size];)
  G(175, i32 move_history[2][6][64][64];)
} ThreadData;

typedef struct __attribute__((aligned(16))) ThreadHeadStruct {
  void (*entry)(struct ThreadHeadStruct *);
  ThreadData data;
} ThreadHead;

#ifdef FULL
static ThreadData *main_data;
static TTEntry *tt;
static u64 pv_hist[256];
static i32 pv_hist_len;
#ifdef NOSTDLIB
__attribute__((
    aligned(4096))) u8 thread_stacks[thread_count][thread_stack_size];
#endif
#else
__attribute__((
    aligned(4096))) u8 thread_stacks[thread_count][thread_stack_size];
__attribute__((section(".bss.zztt"))) S(0) TTEntry tt[tt_length];
#endif
G(176, S(1) u64 start_time;)
G(176, S(1) volatile bool stop;)

#if defined(__x86_64__) || defined(_M_X64)
typedef long long __attribute__((__vector_size__(16))) i128;

[[nodiscard]] __attribute__((target("aes"))) S(1) u64
    get_hash(const Position *const pos) {
  i128 hash = {0};

  // USE 16 BYTE POSITION SEGMENTS AS KEYS FOR AES
  const u8 *const data = (const u8 *)pos;
  for (i32 i = 0; i < 6; i++) {
    i128 key;
    __builtin_memcpy(&key, data + G(177, i) * G(177, 16), 16);
    hash = __builtin_ia32_aesenc128(hash, key);
  }

  // FINAL ROUND FOR BIT MIXING
  hash = __builtin_ia32_aesenc128(hash, hash);

  // USE FIRST 64 BITS AS POSITION HASH
  return hash[0];
}
#elif defined(__aarch64__)

#include <arm_neon.h>

#ifdef __clang__
[[nodiscard]] __attribute__((target("+aes"))) u64
#else
[[nodiscard]] __attribute__((target("arch=armv8-a+crypto"))) u64
#endif
get_hash(const Position *const pos) {
  uint8x16_t hash = vdupq_n_u8(0);

  // USE 16 BYTE POSITION SEGMENTS AS KEYS FOR AES
  const u8 *const data = (const u8 *)pos;
  for (i32 i = 0; i < 6; ++i) {
    uint8x16_t key;
    memcpy(&key, data + i * 16, 16);

    hash = vaesmcq_u8(vaeseq_u8(hash, vdupq_n_u8(0)));
    hash = veorq_u8(hash, key);
  }

  // FINAL ROUND FOR BIT MIXING
  uint8x16_t key = hash;
  hash = vaesmcq_u8(vaeseq_u8(hash, vdupq_n_u8(0)));
  hash = veorq_u8(hash, key);

  // USE FIRST 64 BITS AS POSITION HASH
  u64 result;
  memcpy(&result, &hash, sizeof(result));
  return result;
}

#else
#error "Unsupported architecture: get_hash only for x86_64 and aarch64"
#endif

[[nodiscard]] S(1) u64 get_material_hash(const Position *const pos) {
  u64 hash = 0;
  for (i32 c = 0; c < 2; c++) {
    for (i32 p = Pawn; p <= Queen; p++) {
      hash = G(178, G(179, hash) * G(179, 9)) +
             G(178, count(G(180, pos->pieces[p]) & G(180, pos->colour[c])));
    }
  }
  return hash;
}

[[nodiscard]] S(1) u64 get_pawn_hash(const Position *const pos) {
  return (G(181, pos->pieces[Pawn]) * G(181, 0x9E3779B97F4A7C15ULL)) >> 50;
}

S(1)
i32 search(
#ifdef FULL
    u64 *nodes,
#endif
    H(182, 1, H(183, 1, Position *const pos), H(183, 1, const i32 beta),
      H(183, 1, i32 depth), H(183, 1, ThreadData *data)),
    H(182, 1, H(184, 1, const i32 ply), H(184, 1, const bool do_null),
      H(184, 1, i32 alpha))) {
  assert(alpha < beta);
  assert(ply >= 0);

  SearchStack *const stack = data->stack;
  i32(*const move_history)[6][64][64] = data->move_history;

  // IN-CHECK EXTENSION
  const bool in_check = find_in_check(pos);
  depth += in_check;

  // FULL REPETITION DETECTION
  const u64 tt_hash = get_hash(pos);
  bool in_qsearch = depth <= 0;
  for (i32 i = G(185, ply); G(186, i >= 0) && G(186, do_null); i -= 2) {
    if (G(187, tt_hash) == G(187, stack[i].position_hash)) {
      return 0;
    }
  }

  // TT PROBING
  G(188, TTEntry *tt_entry = &tt[tt_hash % tt_length];)
  G(188, const u16 tt_hash_partial = tt_hash / tt_length;)
  G(188, stack[ply].best_move = (Move){0};)
  if (G(189, tt_entry->partial_hash) == G(189, tt_hash_partial)) {
    stack[ply].best_move = tt_entry->move;

    // TT PRUNING
    if (G(190, tt_entry->depth >= depth) &&
        G(190, G(191, tt_entry->flag) != G(191, tt_entry->score <= alpha)) &&
        G(190, G(192, alpha) == G(192, beta - 1))) {
      return tt_entry->score;
    }
  } else if (depth > 3) {

    // INTERNAL ITERATIVE REDUCTION
    depth--;
  }

  // STATIC EVAL WITH CORRECTION HISTORY
  G(193, i32 static_eval = eval(pos); assert(static_eval < mate);
    assert(static_eval > -mate);)
  G(193, const u64 corr_hashes[2] = {G(194, get_material_hash(pos)),
                                     G(194, get_pawn_hash(pos))};)
  G(193, i32 * corr_entries[2];)
  for (i32 i = 0; i < 2; i++) {
    corr_entries[i] =
        &data->corrhist[pos->flipped][corr_hashes[i] % corrhist_size];
    static_eval += *corr_entries[i] / 256;
    assert(static_eval < mate);
    assert(static_eval > -mate);
  }

  stack[ply].static_eval = static_eval;
  const bool improving = ply > 1 && static_eval > stack[ply - 2].static_eval;
  if (G(195, G(196, tt_entry->partial_hash) == G(196, tt_hash_partial)) &&
      G(195, G(197, tt_entry->flag) != G(197, static_eval) > tt_entry->score)) {
    static_eval = tt_entry->score;
  }

  // QUIESCENCE
  if (G(198, static_eval > alpha) && G(198, in_qsearch)) {
    if (static_eval >= beta) {
      return static_eval;
    }
    alpha = static_eval;
  }

  if (G(199, !in_check) && G(199, G(200, alpha) == G(200, beta - 1))) {
    if (G(201, depth < 9) && G(201, !in_qsearch)) {

      G(202, {
        // REVERSE FUTILITY PRUNING
        if (static_eval - G(203, 57) * G(203, (depth - improving)) >= beta) {
          return static_eval;
        }
      })

      G(202, // RAZORING
        in_qsearch =
            G(204, static_eval) + G(204, G(205, 115) * G(205, depth)) <= alpha;)
    }

    // NULL MOVE PRUNING
    if (G(206, depth > 2) && G(206, do_null) && G(206, static_eval >= beta)) {
      Position npos = *pos;
      G(207, flip_pos(&npos);)
      G(207, npos.ep = 0;)
      const i32 score = -search(
#ifdef FULL
          nodes,
#endif
          H(182, 2, H(183, 2, &npos), H(183, 2, -alpha),
            H(183, 2, depth - G(208, depth / 4) - G(208, 4)), H(183, 2, data)),
          H(182, 2, H(184, 2, ply + 1), H(184, 2, false), H(184, 2, -beta)));
      if (score >= beta) {
        return score;
      }
    }
  }

  G(209, Move moves[max_moves];
    stack[ply].num_moves =
        movegen(H(95, 3, pos), H(95, 3, in_qsearch), H(95, 3, moves));)
  G(209, i32 best_score = in_qsearch ? static_eval : -inf;)
  G(209, stack[G(210, ply) + G(210, 2)].position_hash = tt_hash;)
  G(209, u8 tt_flag = Upper;)
  G(209, i32 quiets_evaluated = 0;)
  G(209, i32 moves_evaluated = 0;)

  for (i32 move_index = 0; move_index < stack[ply].num_moves; move_index++) {
    // MOVE ORDERING
    G(211, i32 move_score = ~0x1010101LL;)
    G(211, i32 best_index = 0;)
    for (i32 order_index = move_index; order_index < stack[ply].num_moves;
         order_index++) {
      assert(moves[order_index].takes_piece ==
             piece_on(H(55, 7, pos), H(55, 7, moves[order_index].to)));
      const i32 order_move_score =
          G(183, // KILLER MOVE
            G(212, move_equal(G(213, &moves[order_index]),
                              G(213, &stack[ply].killer))) *
                G(212, 819)) +
          G(183, // HISTORY HEURISTIC
            move_history[pos->flipped][moves[order_index].takes_piece]
                        [moves[order_index].from][moves[order_index].to]) +
          G(183, // PREVIOUS BEST MOVE FIRST
            (move_equal(G(214, &stack[ply].best_move),
                        G(214, &moves[order_index]))
             << 30)) +
          G(183, // MOST VALUABLE VICTIM
            G(215, moves[order_index].takes_piece) * G(215, 622));
      if (order_move_score > move_score) {
        G(216, best_index = order_index;)
        G(216, move_score = order_move_score;)
      }
    }

    swapmoves(G(217, &moves[best_index]), G(217, &moves[move_index]));

    G(
        218, // MOVE SCORE PRUNING
        if (G(222, moves_evaluated) &&
            G(222, move_score < G(223, -125) * G(223, depth))) { break; })

    G(
        218, // FORWARD FUTILITY PRUNING / DELTA PRUNING
        if (G(219, depth < 8) &&
            G(219,
              G(220, static_eval) + G(220, G(221, 154) * G(221, depth)) +
                      G(220,
                        initial_params.eg.material[moves[move_index].promo]) +
                      G(220, initial_params.eg
                                 .material[moves[move_index].takes_piece]) <
                  alpha) &&
            G(219, moves_evaluated) && G(219, !in_check)) { break; })

    Position npos = *pos;
#ifdef FULL
    (*nodes)++;
#endif
    if (!makemove(H(80, 3, &npos), H(80, 3, &moves[move_index]))) {
      continue;
    }

    // PRINCIPAL VARIATION SEARCH
    i32 low = moves_evaluated == 0 ? -beta : -alpha - 1;
    moves_evaluated++;

    // LATE MOVE REDUCTION
    i32 reduction = G(224, depth > 3) && G(224, move_score <= 0)
                        ? G(225, !improving) + G(225, (move_score / -358)) +
                              G(225, (G(226, alpha) == G(226, beta - 1))) +
                              G(225, moves_evaluated / 9)
                        : 0;

    i32 score;
    while (true) {
      score = -search(
#ifdef FULL
          nodes,
#endif
          H(182, 3, H(183, 3, &npos), H(183, 3, -alpha),
            H(183, 3, depth - G(227, 1) - G(227, reduction)), H(183, 3, data)),
          H(182, 3, H(184, 3, ply + 1), H(184, 3, true), H(184, 3, low)));

      // EARLY EXITS
      if (stop || (depth > 4 && get_time() - start_time > data->max_time)) {
        return best_score;
      }

      if (score > alpha) {
        if (reduction != 0) {
          reduction = 0;
          continue;
        }

        if (low != -beta) {
          low = -beta;
          continue;
        }
      }
      break;
    }

    if (score > best_score) {
      best_score = score;

      if (score > alpha) {
        G(228, tt_flag = Exact;)
        G(228, stack[ply].best_move = moves[move_index];)
        G(228, alpha = score;)
        if (score >= beta) {
          assert(stack[ply].best_move.takes_piece ==
                 piece_on(H(55, 8, pos), H(55, 8, stack[ply].best_move.to)));
          G(229, tt_flag = Lower;)
          G(
              229, if (stack[ply].best_move.takes_piece == None) {
                stack[ply].killer = stack[ply].best_move;
              })
          G(
              229, if (!in_qsearch) {
                const i32 bonus = depth * depth;
                G(230, i32 *const this_hist =
                           &move_history[pos->flipped]
                                        [stack[ply].best_move.takes_piece]
                                        [stack[ply].best_move.from]
                                        [stack[ply].best_move.to];

                  *this_hist +=
                  bonus - G(231, bonus) * G(231, *this_hist) / 1024;)
                G(
                    230, for (i32 prev_index = 0; prev_index < move_index;
                              prev_index++) {
                      const Move prev = moves[prev_index];
                      i32 *const prev_hist =
                          &move_history[pos->flipped][prev.takes_piece]
                                       [prev.from][prev.to];
                      *prev_hist -=
                          bonus + G(232, bonus) * G(232, *prev_hist) / 1024;
                    })
              })
          break;
        }
      }
    }

    if (moves[move_index].takes_piece == None) {
      quiets_evaluated++;
    }

    // LATE MOVE PRUNING
    if (G(233, !in_check) && G(233, G(234, alpha) == G(234, beta - 1)) &&
        G(233, quiets_evaluated > (G(235, 1) + G(235, depth * depth)) >>
                   !improving)) {
      break;
    }
  }

  // MATE / STALEMATE DETECTION
  if (G(236, best_score) == G(236, -inf)) {
    return G(237, (ply - mate)) * G(237, in_check);
  }

  G(
      238, // UPDATE CORRECTION HISTORY
      if (G(239,
            G(240, tt_flag) != G(240, (best_score < stack[ply].static_eval))) &&
          G(239, G(241, stack[ply].best_move.takes_piece) == G(241, None))) {
        G(242, i32 dd = depth * depth; if (dd > 64) { dd = 64; })
        G(242, i32 target = best_score - stack[ply].static_eval; G(
              243, if (target < -96) { target = -96; })
              G(243, if (target > 96) { target = 96; }))

        for (i32 i = 0; i < 2; i++) {
          *corr_entries[i] =
              (G(244, G(245, *corr_entries[i]) * G(245, (600 - dd))) +
               G(244, G(246, target) * G(246, 256) * G(246, dd))) /
              600;
        }
      })

  G(238, // UPDATE TRANSPOSITION TABLE
        *tt_entry = (TTEntry){.partial_hash = tt_hash_partial,
                              .move = stack[ply].best_move,
                              .score = best_score,
                              .depth = depth,
                              .flag = tt_flag};)

  return best_score;
}

S(1) void init() {
  G(
      80, // MERGE EVAL PARAMS
      for (i32 i = 0; i < sizeof(EvalParamsMerged) / sizeof(i32); i++) {
        ((i32 *)&eval_params)[i] =
            combine_eval_param(H(122, 2,
                                 i < 6 ? initial_params.mg.material[i]
                                       : ((i8 *)&initial_params.mg)[6 + i]),
                               H(122, 2,
                                 i < 6 ? initial_params.eg.material[i]
                                       : ((i8 *)&initial_params.eg)[6 + i]));
      })
  G(
      80, // INIT DIAGONAL MASKS
      for (i32 sq = 0; sq < 64; sq++) {
        const u64 bb = 1ULL << sq;
        G(247, u64 sw_bb = southwest(bb);)
        G(247, u64 ne_bb = northeast(bb);)
        for (i32 i = 6; i > 0; i--) {
          G(248, sw_bb |= southwest(sw_bb);)
          G(248, ne_bb |= northeast(ne_bb);)
        }
        diag_mask[sq] = G(249, sw_bb) | G(249, ne_bb);
      })
}

#ifdef FULL
static void print_info(const Position *pos, const i32 depth, const i32 alpha,
                       const i32 beta, const i32 score, const u64 nodes,
                       const Move pv_move, const u64 max_time) {
  // Do not print unfinished iteration scores
  u64 elapsed = get_time() - start_time;
  if (elapsed > max_time) {
    puts("info string hard time limit exceeded");
    return;
  }

  printf("info depth %i score ", depth);

  // Only use bound on failed search
  i32 print_score;
  if (score <= alpha) {
    print_score = alpha;
  } else if (score >= beta) {
    print_score = beta;
  } else {
    print_score = score;
  }

  // Handle mate scores
  const i32 abs_score = print_score > 0 ? print_score : -print_score;
  if (abs_score > mate - 1024 && print_score <= mate) {
    const i32 abs_dist_plies = mate - abs_score;
    const i32 abs_dist_moves = (abs_dist_plies + 1) / 2;
    const i32 dist_moves = print_score > 0 ? abs_dist_moves : -abs_dist_moves;
    printf("mate %i ", dist_moves);
  } else {
    printf("cp %i ", print_score);
  }

  // Handle bounds
  if (score <= alpha) {
    putl("upperbound ");
  } else if (score >= beta) {
    putl("lowerbound ");
  }

  printf("time %llu nodes %llu", elapsed / (1000 * 1000), nodes);

  // Only print nps if >=1 ns elapsed
  if (elapsed > 0) {
    const u64 nps = nodes * 1000 * 1000 * 1000 / elapsed;
    printf(" nps %llu", nps);
  }

  // Only print pv move if within window
  if (score > alpha && score < beta) {
    putl(" pv ");
    char move_name[8];
    move_str(H(50, 2, pos->flipped), H(50, 2, &pv_move), H(50, 2, move_name));
    putl(move_name);

    Position cur_pos = *pos;
    if (makemove(&cur_pos, &pv_move)) {
      u64 seen[max_ply];
      i32 seen_count = 0;
      seen[seen_count++] = get_hash(pos);
      i32 halfmoves = pv_hist_len + 1;
      while (seen_count < max_ply && halfmoves < 100) {
        const u64 hash = get_hash(&cur_pos);

        bool repeat = false;
        for (i32 i = 0; i < pv_hist_len; i++) {
          if (pv_hist[i] == hash) {
            repeat = true;
          }
        }
        for (i32 i = 0; i < seen_count; i++) {
          if (seen[i] == hash) {
            repeat = true;
          }
        }
        if (repeat) {
          break;
        }

        const TTEntry *const entry = &tt[hash % tt_length];
        if (entry->partial_hash != (u16)(hash / tt_length) ||
            entry->flag != Exact) {
          break;
        }

        Move move = entry->move;
        Move moves[max_moves];
        const i32 num_moves = movegen(&cur_pos, false, moves);
        i32 move_index = 0;
        while (move_index < num_moves &&
               !move_equal(&move, &moves[move_index])) {
          move_index++;
        }
        if (move_index == num_moves) {
          break;
        }
        Position next_pos = cur_pos;
        if (!makemove(&next_pos, &move)) {
          break;
        }

        putl(" ");
        move_str(cur_pos.flipped, &move, move_name);
        putl(move_name);
        seen[seen_count++] = hash;
        cur_pos = next_pos;
        halfmoves++;
      }
    }
  }

  putl("\n");
}
#endif

S(1)
void iteratively_deepen(
#ifdef FULL
    i32 maxdepth,
#endif
    ThreadData *data) {
  i32 score = 0;
#ifdef FULL
  for (i32 depth = 1; depth < maxdepth; depth++) {
#else
  for (i32 depth = 1; depth < max_ply; depth++) {
#endif
    // ASPIRATION WINDOWS
    G(250, i32 window = 16;)
    G(250, size_t elapsed;)
    while (true) {
      G(251, const i32 alpha = score - window;)
      G(251, const i32 beta = G(252, score) + G(252, window);)
      score = search(
#ifdef FULL
          &data->nodes,
#endif
          H(182, 4, H(183, 4, &data->pos), H(183, 4, beta), H(183, 4, depth),
            H(183, 4, data)),
          H(182, 4, H(184, 4, 0), H(184, 4, false), H(184, 4, alpha)));
#ifdef FULL
      if (data->thread_id == 0) {
        print_info(&data->pos, depth, alpha, beta, score, data->nodes,
                   data->stack[0].best_move, data->max_time);
      }
#endif
      elapsed = get_time() - start_time;
      G(253, window *= 2;)
      G(
          253, if (G(254, elapsed > data->max_time) ||
                   G(254, (G(255, score > alpha) && G(255, score < beta)))) {
            break;
          })
    }

    if (stop || elapsed > data->max_time / 14) {
      break;
    }
  }
}

static void *entry_full(void *param) {
  ThreadData *data = param;
  iteratively_deepen(
#ifdef FULL
      max_ply,
#endif
      data);
  return NULL;
}

S(1) void entry_mini(ThreadHead *head) {
  iteratively_deepen(
#ifdef FULL
      max_ply,
#endif
      &head->data);
  exit_now();
}

#ifdef NOSTDLIB
__attribute__((naked)) S(0) long newthread(ThreadHead *head) {
  __asm__ volatile("mov  rsi, rdi\n"     // arg2 = stack
                   "mov  edi, 0x50f00\n" // arg1 = clone flags
                   "mov  eax, 56\n"      // SYS_clone
                   "syscall\n"
                   "mov  rdi, rsp\n" // entry point argument
                   "ret\n"
                   :
                   :
                   : "rax", "rcx", "rsi", "rdi", "r11", "memory");
}
#endif

_Static_assert(sizeof(ThreadData) < thread_stack_size);

S(1)
void run_smp() {
  start_time = get_time();
#ifndef NOSTDLIB
  main_data->nodes = 0;
  pthread_t helpers[thread_count - 1];
  ThreadData *helper_data[thread_count - 1];

  for (i32 i = 0; i < thread_count - 1; i++) {
    helper_data[i] = malloc(sizeof(ThreadData));
    __builtin_memset(helper_data[i], 0, sizeof(ThreadData));
    helper_data[i]->pos = main_data->pos;
    helper_data[i]->max_time = -1LL;
    helper_data[i]->thread_id = i + 1;
    pthread_create(&helpers[i], NULL, entry_full, helper_data[i]);
  }

  iteratively_deepen(max_ply, main_data);
  stop = true;

  for (i32 i = 0; i < thread_count - 1; i++) {
    pthread_join(helpers[i], NULL);
    free(helper_data[i]);
  }
#else
#ifdef FULL
  main_data->nodes = 0;
#else
  ThreadData *main_data = (ThreadData *)&thread_stacks[0][0];
#endif

  for (i32 i = 1; i < thread_count; i++) {
    ThreadHead *helper_head =
        (ThreadHead *)&thread_stacks[i][thread_stack_size - sizeof(ThreadHead)];
    G(256, helper_head->data.max_time = -1LL;)
    G(256, helper_head->data.pos = main_data->pos;)
#ifdef FULL
    helper_head->data.thread_id = i;
#endif
    helper_head->entry = entry_mini;
    newthread(helper_head);
  }

  iteratively_deepen(
#ifdef FULL
      max_ply,
#endif
      main_data);
  stop = true;

  for (i32 i = 0; i < thread_count - 1; i++) {
    // TODO: sync ?
  }
#endif

  char move_name[8];
  move_str(H(50, 3, main_data->pos.flipped),
           H(50, 3, &main_data->stack[0].best_move), H(50, 3, move_name));
  putl("bestmove ");
  puts(move_name);
}

#if defined(FULL) && !defined(NOSTDLIB)
static pthread_t bg_thread;
static bool bg_running = false;
static void *bg_entry(void *unused) {
  (void)unused;
  run_smp();
  return NULL;
}
static void bg_stop(void) {
  if (bg_running) {
    stop = true;
    pthread_join(bg_thread, NULL);
    bg_running = false;
  }
}
#endif

#ifdef FULL
S(1) void display_pos(Position *const pos) {
  Position npos = *pos;
  if (npos.flipped) {
    flip_pos(&npos);
  }
  for (i32 rank = 7; rank >= 0; rank--) {
    for (i32 file = 0; file < 8; file++) {
      i32 sq = rank * 8 + file;
      u64 bb = 1ULL << sq;
      i32 piece = piece_on(H(55, 9, &npos), H(55, 9, sq));
      if (bb & npos.colour[0]) {
        if (piece == Pawn) {
          putl("P");
        } else if (piece == Knight) {
          putl("N");
        } else if (piece == Bishop) {
          putl("B");
        } else if (piece == Rook) {
          putl("R");
        } else if (piece == Queen) {
          putl("Q");
        } else if (piece == King) {
          putl("K");
        }
      } else if (bb & npos.colour[1]) {
        if (piece == Pawn) {
          putl("p");
        } else if (piece == Knight) {
          putl("n");
        } else if (piece == Bishop) {
          putl("b");
        } else if (piece == Rook) {
          putl("r");
        } else if (piece == Queen) {
          putl("q");
        } else if (piece == King) {
          putl("k");
        }
      } else {
        putl(".");
      }
    }
    putl("\n");
  }
  putl("\nTurn: ");
  putl(pos->flipped ? "Black" : "White");
  putl("\nCastling: ");
  if (npos.castling[0]) {
    putl("K");
  }
  if (npos.castling[1]) {
    putl("Q");
  }
  if (npos.castling[2]) {
    putl("k");
  }
  if (npos.castling[3]) {
    putl("q");
  }
  printf("\nEn passant: %d", lsb(npos.ep));
  printf("\nHash: %llu", get_hash(&npos));
  putl("\nEval: ");
  i32 score = eval(pos);
  if (pos->flipped) {
    score = -score;
  }
  printf("%d\n", score);
}
#endif

S(1)
const Position start_pos =
    (Position){.ep = 0,
               .colour = {0xFFFFull, 0xFFFF000000000000ull},
               .pieces = {0, 0xFF00000000FF00ull, 0x4200000000000042ull,
                          0x2400000000000024ull, 0x8100000000000081ull,
                          0x800000000000008ull, 0x1000000000000010ull},
               .castling = {true, true, true, true}};

#ifdef FULL
S(1) void bench() {
  stop = false;
  ThreadData data = {
      .thread_id = 0,
      .nodes = 0,
      .max_time = -1LL,
      .pos = start_pos,
  };
  const u64 start = get_time();
  start_time = start;
  iteratively_deepen(23, &data);
  const u64 end = get_time();
  const u64 elapsed = end - start;
  const u64 nps = elapsed ? data.nodes * 1000 * 1000 * 1000U / elapsed : 0;
  printf("%llu nodes %llu nps\n", data.nodes, nps);
}
#endif

#if !defined(FULL) && defined(NOSTDLIB)
void _start() {
#else
S(1) void run() {
#endif
#ifndef NOSTDLIB
  setvbuf(stdout, NULL, _IONBF, 0);
#endif

  G(257, char line[4096];)
  G(257, init();)
#ifdef FULL
  main_data = calloc(1, sizeof(ThreadData));
  tt = malloc(tt_length * sizeof(TTEntry));
  __builtin_memset(tt, 0, tt_length * sizeof(TTEntry));
  main_data->pos = start_pos;
#else
  ThreadData *main_data = (ThreadData *)&thread_stacks[0][0];
#endif

#ifndef FULL
  // Assume first input is "uci"
  getl(line);
  puts("uciok");
#endif

  // UCI loop
  while (true) {
    getl(line);
#ifdef FULL
    if (!strcmp(line, "uci")) {
      puts("id name 4k.c");
      puts("id author Gediminas Masaitis");
      puts("");
      puts("option name Hash type spin default 80 min 1 max 65536");
      puts("option name Threads type spin default 1 min 1 max 256");
      puts("uciok");
    } else if (!strcmp(line, "setoption")) {
#if defined(FULL) && !defined(NOSTDLIB)
      bg_stop();
#endif
      getl(line); // "name"
      getl(line); // option name
      if (!strcmp(line, "Hash")) {
        getl(line); // "value"
        getl(line); // MB
        free(tt);
        const u64 mb = atoi(line);
        tt_length = mb * 1024 * 1024 / sizeof(TTEntry);
        tt = malloc(tt_length * sizeof(TTEntry));
        __builtin_memset(tt, 0, tt_length * sizeof(TTEntry));
      } else if (!strcmp(line, "Threads")) {
        getl(line); // "value"
        getl(line);
#ifdef NOSTDLIB
        if (atoi(line) > 1) {
          puts("info string Threads > 1 not supported in this build");
          exit_now();
        }
#else
        thread_count = atoi(line);
#endif
      }
    } else if (!strcmp(line, "ucinewgame")) {
#if defined(FULL) && !defined(NOSTDLIB)
      bg_stop();
#endif
      __builtin_memset(main_data, 0, sizeof(ThreadData));
      __builtin_memset(tt, 0, tt_length * sizeof(TTEntry));
    } else if (!strcmp(line, "bench")) {
      bench();
    } else if (!strcmp(line, "gi")) {
      stop = false;
      start_time = get_time();
      main_data->max_time = -1LL;
      run_smp();
    } else if (!strcmp(line, "d")) {
      display_pos(&main_data->pos);
    } else if (!strcmp(line, "perft")) {
      char depth_str[4];
      getl(depth_str);
      const i32 depth = atoi(depth_str);
      const u64 start = get_time();
      const u64 nodes = perft(&main_data->pos, depth);
      const u64 end = get_time();
      const u64 elapsed = end - start;
      const u64 nps = elapsed ? nodes * 1000 * 1000 * 1000 / elapsed : 0;
      printf("info depth %i nodes %llu time %llu nps %llu \n", depth, nodes,
             elapsed, nps);
    }
#endif
    G(
        258, if (G(260, line[0]) == G(260, 'g')) {
#if defined(FULL) && !defined(NOSTDLIB)
          bg_stop();
#endif
          stop = false;
#ifdef FULL
          bool infinite = false;
          while (true) {
            getl(line);
            if (!main_data->pos.flipped && !strcmp(line, "wtime")) {
              getl(line);
              main_data->max_time = (u64)atoi(line) << 19; // Roughly /2 time
              break;
            } else if (main_data->pos.flipped && !strcmp(line, "btime")) {
              getl(line);
              main_data->max_time = (u64)atoi(line) << 19; // Roughly /2 time
              break;
            } else if (!strcmp(line, "movetime")) {
              main_data->max_time =
                  20ULL * 1000 * 1000 * 1000; // Assume Lichess bot
              break;
            } else if (!strcmp(line, "infinite")) {
#ifndef NOSTDLIB
              main_data->max_time = -1LL;
#else
          main_data->max_time = 20ULL * 1000 * 1000 * 1000;
#endif
              infinite = true;
              break;
            }
          }
#ifndef NOSTDLIB
          if (infinite) {
            pthread_create(&bg_thread, NULL, bg_entry, NULL);
            bg_running = true;
          } else
            run_smp();
#else
      (void)infinite;
      run_smp();
#endif
#else
      for (i32 i = 2 << main_data->pos.flipped; i > 0; i--) {
        getl(line);
        main_data->max_time = (u64)atoi(line) << 19; // Roughly /2 time
      }
      start_time = get_time();
      run_smp();
#endif
        })
    else G(
        258,
        if (G(262, line[0]) == G(262, 'p')) {
#if defined(FULL) && !defined(NOSTDLIB)
          bg_stop();
#endif
          G(263, main_data->pos = start_pos;)
#ifdef FULL
          pv_hist_len = 0;
#endif
          while (true) {
            bool line_continue = getl(line);

#ifdef FULL
            if (!strcmp(line, "fen")) {
              getl(line);
              line_continue = get_fen(&main_data->pos, line);
            } else
#endif
            {
              Move moves[max_moves];
              const i32 num_moves = movegen(H(95, 4, &main_data->pos),
                                            H(95, 4, false), H(95, 4, moves));
              for (i32 i = 0; i < num_moves; i++) {
                char move_name[8];
                move_str(H(50, 4, main_data->pos.flipped), H(50, 4, &moves[i]),
                         H(50, 4, move_name));
                assert(move_string_equal(line, move_name) ==
                       !strcmp(line, move_name));
                if (move_string_equal(G(264, move_name), G(264, line))) {
#ifdef FULL
                  if (moves[i].takes_piece != None ||
                      piece_on(&main_data->pos, moves[i].from) == Pawn) {
                    pv_hist_len = 0;
                  } else if (pv_hist_len < 256) {
                    pv_hist[pv_hist_len++] = get_hash(&main_data->pos);
                  }
#endif
                  makemove(H(80, 4, &main_data->pos), H(80, 4, &moves[i]));
                  break;
                }
              }
            }
            if (!line_continue) {
              break;
            }
          }
        })
#if defined(FULL) && !defined(NOSTDLIB)
        else G(
            258,
            if (line[0] == 's') { bg_stop(); })
#endif
        else G(
            258, if (G(261, line[0]) == G(261, 'i')) {
              puts("readyok");
            }) else G(258, if (G(259, line[0]) == G(259, 'q')) { exit_now(); })
  }
}

#if !defined(NOSTDLIB) || defined(FULL)
#ifdef NOSTDLIB
__attribute__((naked)) void _start() {
#ifdef FULL
  register long *stack asm("rsp");
  int argc = (int)*stack;
  char **argv = (char **)(stack + 1);
#endif
#else
int main(int argc, char **argv) {
#endif
#ifdef FULL
  if (argc > 1 && !strcmp(argv[1], "bench")) {
    init();
    tt = malloc(tt_length * sizeof(TTEntry));
    __builtin_memset(tt, 0, tt_length * sizeof(TTEntry));
    bench();
    exit_now();
  }
#endif
  run();
}
#endif

#pragma endregion
