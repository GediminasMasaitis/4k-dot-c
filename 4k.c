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
    1, S(1) ssize_t _sys(H(2, 1, ssize_t arg1), H(2, 1, ssize_t arg3),
                         H(2, 1, ssize_t arg2), H(2, 1, ssize_t call)) {
      ssize_t ret;
      asm volatile("syscall"
                   : "=a"(ret)
                   : "0"(call), "D"(arg1), "S"(arg2), "d"(arg3)
                   : "rcx", "r11", "memory");
      return ret;
    })

G(
    1, S(1) void exit_now() {
      asm volatile("syscall" : : "a"(60));
      __builtin_unreachable();
    })

G(
    3, // Non-standard, gets but a word instead of a line
    S(1) bool getl(char *restrict string) {
      while (true) {
        const int result = _sys(H(2, 2, stdin), H(2, 2, 1),
                                H(2, 2, (ssize_t)string), H(2, 2, 0));

    // Assume stdin never closes on mini build
#ifdef FULL
        if (result < 1) {
          exit_now();
        }
#endif

        const char ch = *string;
        if (G(4, ch == '\n') || G(4, ch == ' ')) {
          *string = 0;
          return ch != '\n';
        }

        string++;
      }
    })

G(
    3,
    S(1) void putl(const char *const restrict string) {
      i32 length = 0;
      while (string[length]) {
        _sys(H(2, 3, stdout), H(2, 3, 1), H(2, 3, (ssize_t)(&string[length])),
             H(2, 3, 1));
        length++;
      }
    }

    S(1) void puts(const char *const restrict string) {
      putl(string);
      putl("\n");
    })

[[nodiscard]] static bool strcmp(const char *restrict lhs,
                                 const char *restrict rhs) {
  while (*lhs || *rhs) {
    if (*lhs != *rhs) {
      return true;
    }
    lhs++;
    rhs++;
  }
  return false;
}

G(
    3, [[nodiscard]] S(1) u32 atoi(const char *restrict string) {
      // Will break if reads a value over 4294967295
      // This works out to be just over 49 days

      u32 result = 0;
      while (true) {
        if (!*string) {
          return result;
        }
        result *= 10;
        result += *string - '0';
        string++;
      }
    })

#define printf(format, ...) _printf(format, (size_t[]){__VA_ARGS__})

S(1) void _printf(const char *format, const size_t *args) {
  long long value;
  char buffer[16], *string;

  while (true) {
    if (!*format) {
      break;
    }
    if (*format != '%') {
      _sys(H(2, 4, stdout), H(2, 4, 1), H(2, 4, (ssize_t)format), H(2, 4, 1));
      format++;
      continue;
    }

    format++;
    switch (*format++) {
    case 's':
      putl((char *)*args);
      break;
    case 'i':
      value = *args;
      if (value < 0) {
        putl("-");
        value *= -1;
      }
      string = buffer + sizeof buffer - 1;
      *string-- = 0;
      for (;;) {
        *string = '0' + value % 10;
        value /= 10;
        if (!value) {
          break;
        }
        string--;
      }
      putl(string);
      break;
    }
    args++;
  }
}

typedef struct [[nodiscard]] {
  ssize_t tv_sec;  // seconds
  ssize_t tv_nsec; // nanoseconds
} timespec;

[[nodiscard]] S(1) size_t get_time() {
  timespec ts;
  ssize_t ret; // Unused
  asm volatile("syscall"
               : "=a"(ret)
               : "0"(228), "D"(1), "S"(&ts)
               : "rcx", "r11", "memory");
  return G(5, ts.tv_nsec / 1000000) + G(5, G(6, ts.tv_sec) * G(6, 1000));
}

#else
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

[[nodiscard]] static size_t get_time() {
  struct timespec ts;
  clock_gettime(CLOCK_MONOTONIC, &ts);
  return ts.tv_sec * 1000 + ts.tv_nsec / 1000000;
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
  G(7, u8 promo;)
  G(7, u8 takes_piece;)
  G(7, u8 from; u8 to;)
} Move;

typedef struct [[nodiscard]] {
  G(8, u64 ep;)
  G(8, u64 pieces[7];)
  G(8, u64 colour[2];)
  G(9, bool flipped;)
  G(9, bool castling[4];)
  G(9, u8 padding[11];)
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
    10,
    [[nodiscard]] S(1) bool move_string_equal(G(11, const char *restrict lhs),
                                              G(11, const char *restrict rhs)) {
      return (G(12, *(const u64 *)rhs) ^ G(12, *(const u64 *)lhs)) << 24 == 0;
    })

G(
    10, [[nodiscard]] S(1)
            u64 flip_bb(const u64 bb) { return __builtin_bswap64(bb); })
G(10, [[nodiscard]] S(1) i32 lsb(u64 bb) { return __builtin_ctzll(bb); })

G(
    10, [[nodiscard]] S(1)
            u64 shift(H(13, 1, const i32 shift), H(13, 1, const u64 mask),
                      H(13, 1, const u64 bb)) {
              return shift > 0 ? G(14, bb << shift) & G(14, mask)
                               : G(15, bb >> -shift) & G(15, mask);
            })

G(
    10, [[nodiscard]] S(1)
            i32 count(const u64 bb) { return __builtin_popcountll(bb); })

G(
    16, [[nodiscard]] S(1) u64 west(const u64 bb) {
      return G(17, bb >> 1) & G(17, ~0x8080808080808080ull);
    })

G(16, [[nodiscard]] S(1) u64 north(const u64 bb) { return bb << 8; })

G(16, [[nodiscard]] S(1) u64 south(const u64 bb) { return bb >> 8; })

G(
    16, [[nodiscard]] S(1) u64 east(const u64 bb) {
      return G(18, bb << 1) & G(18, ~0x101010101010101ull);
    })

G(
    19, [[nodiscard]] S(1)
            u64 se(const u64 bb) { return G(20, east)(G(20, south)(bb)); })

G(
    19, [[nodiscard]] S(1)
            u64 ne(const u64 bb) { return G(21, east)(G(21, north)(bb)); })

G(
    19, [[nodiscard]] S(1) u64 nw(const u64 bb) {
      return shift(H(13, 2, 7), H(13, 2, ~0x8080808080808080ull), H(13, 2, bb));
      // return west(north(bb));
    })

G(
    19, [[nodiscard]] S(1)
            u64 sw(const u64 bb) { return G(22, west)(G(22, south)(bb)); })

G(23, S(0) u64 diag_mask[64];)

G(
    23, [[nodiscard]] S(1)
            u64 ray(H(24, 1, const u64 blockers), H(24, 1, const u64 mask),
                    H(24, 1, const u64 bb), H(24, 1, const i32 shift_by)) {
              u64 result =
                  shift(H(13, 3, shift_by), H(13, 3, mask), H(13, 3, bb));
              for (i32 i = 0; i < 6; i++) {
                result |= shift(H(13, 4, shift_by), H(13, 4, mask),
                                H(13, 4, result & ~blockers));
              }
              return result;
            })

G(
    23, [[nodiscard]] S(0) u64 xattack(H(25, 1, const u64 dir_mask),
                                       H(25, 1, const u64 bb),
                                       H(25, 1, const u64 blockers)) {
      return G(26, dir_mask) &
             G(26, (G(27, (G(28, blockers) & G(28, dir_mask)) - bb) ^
                    G(27, flip_bb(flip_bb(G(29, blockers) & G(29, dir_mask)) -
                                  flip_bb(bb)))));
    })

G(
    30, [[nodiscard]] S(0) u64 bishop(H(31, 1, const u64 blockers),
                                      H(31, 1, const u64 bb)) {
      assert(count(bb) == 1);
      const i32 sq = lsb(bb);
      return G(32, xattack(H(25, 2, diag_mask[sq]), H(25, 2, bb),
                           H(25, 2, blockers))) |
             G(32, xattack(H(25, 3, flip_bb(diag_mask[G(33, sq) ^ G(33, 56)])),
                           H(25, 3, bb), H(25, 3, blockers)));
    })

G(
    30, [[nodiscard]] S(1) u64 rook(H(34, 1, const u64 blockers),
                                    H(34, 1, const u64 bb)) {
      assert(count(bb) == 1);
      return G(35, xattack(H(25, 4, bb ^ 0x101010101010101ULL << lsb(bb) % 8),
                           H(25, 4, bb), H(25, 4, blockers))) |
             G(35, // East
               ray(H(24, 2, blockers), H(24, 2, ~0x101010101010101ull),
                   H(24, 2, bb), H(24, 2, 1))) |
             G(35, // West
               ray(H(24, 3, blockers), H(24, 3, ~0x8080808080808080ull),
                   H(24, 3, bb), H(24, 3, -1)));
    })

G(
    30, [[nodiscard]] S(1) u64 knight(const u64 bb) {
      return G(36, G(37, (G(38, bb << 17) | G(38, bb >> 15))) &
                       G(37, ~0x101010101010101ull)) |
             G(36, G(39, (G(40, bb << 15 | G(40, bb >> 17)))) &
                       G(39, ~0x8080808080808080ull)) |
             G(36, G(41, (G(42, bb << 6) | G(42, bb >> 10))) &
                       G(41, 0x3F3F3F3F3F3F3F3Full)) |
             G(36, G(43, (G(44, bb >> 6) | G(44, bb << 10))) &
                       G(43, 0xFCFCFCFCFCFCFCFCull));
    })

G(
    30, [[nodiscard]] S(1) u64 king(const u64 bb) {
      return G(45, bb << 8) | G(45, bb >> 8) |
             G(45, G(46, (G(47, bb << 9) | G(47, bb >> 7) | G(47, bb << 1))) &
                       G(46, ~0x101010101010101ull)) |
             G(45, G(48, (G(49, bb << 7) | G(49, bb >> 9) | G(49, bb >> 1))) &
                       G(48, ~0x8080808080808080ull));
    })

G(
    50,
    S(1) void swapu32(G(51, u32 *const lhs), G(51, u32 *const rhs)) {
      const u32 temp = *lhs;
      *lhs = *rhs;
      *rhs = temp;
    }

    S(1) void swapmoves(G(52, Move *const lhs), G(52, Move *const rhs)) {
      swapu32(G(53, (u32 *)lhs), G(53, (u32 *)rhs));
    })

G(
    50, [[nodiscard]] S(1) bool move_equal(G(54, Move *const rhs),
                                           G(54, Move *const lhs)) {
      return G(55, *(u32 *)lhs) == G(55, *(u32 *)rhs);
    })

G(
    50, S(1) void swapu64(G(56, u64 *const rhs), G(56, u64 *const lhs)) {
      const u64 temp = *lhs;
      *lhs = *rhs;
      *rhs = temp;
    })

G(
    50, S(1) void move_str(H(57, 1, char *restrict str),
                           H(57, 1, const Move *restrict move),
                           H(57, 1, const i32 flip)) {
      assert(move->from >= 0);
      assert(move->from < 64);
      assert(move->to >= 0);
      assert(move->to < 64);
      assert(move->from != move->to);
      assert(move->promo == None || move->promo == Knight ||
             move->promo == Bishop || move->promo == Rook ||
             move->promo == Queen);

      // Hack to save bytes, technically UB but works on GCC 14.2
      for (i32 i = 0; i < 2; i++) {
        G(58, str[i * 2] = 'a' + (&move->from)[i] % 8;)
        G(58, str[i * 2 + 1] = '1' + ((&move->from)[i] / 8 ^ 7 * flip);)
      }

      G(59, str[5] = '\0';)
      G(59, str[4] = "\0\0nbrq"[move->promo];)
    })

G(
    50, S(1) void swapbool(G(57, bool *const restrict rhs),
                           G(57, bool *const restrict lhs)) {
      const bool temp = *lhs;
      *lhs = *rhs;
      *rhs = temp;
    })

G(
    50, [[nodiscard]] S(1)
            i32 piece_on(H(60, 1, const Position *const restrict pos),
                         H(60, 1, const i32 sq)) {
              assert(sq >= 0);
              assert(sq < 64);
              for (i32 i = Pawn; i <= King; ++i) {
                if (G(61, pos->pieces[i]) & G(61, 1ull << sq)) {
                  return i;
                }
              }
              return None;
            })

G(
    62,
    [[nodiscard]] S(1) i32 is_attacked(H(63, 1,
                                         const Position *const restrict pos),
                                       H(63, 1, const u64 bb)) {
      assert(count(bb) == 1);
      const u64 theirs = pos->colour[1];
      G(64, const u64 pawns = theirs & pos->pieces[Pawn];)
      G(64, const u64 blockers = theirs | pos->colour[0];)
      return G(65, G(66, (G(67, sw(pawns)) | G(67, se(pawns)))) & G(66, bb)) ||
             G(65, G(68, bishop(H(31, 2, blockers), H(31, 2, bb))) &
                       G(68, theirs) &
                       G(68, (pos->pieces[Bishop] | pos->pieces[Queen]))) ||
             G(65,
               G(69, king(bb)) & G(69, theirs) & G(69, pos->pieces[King])) ||
             G(65, G(70, knight(bb)) & G(70, theirs) &
                       G(70, pos->pieces[Knight])) ||
             G(65, G(71, (pos->pieces[Rook] | pos->pieces[Queen])) &
                       G(71, rook(H(34, 2, blockers), H(34, 2, bb))) &
                       G(71, theirs));
    })

G(
    62, S(0) void flip_pos(Position *const restrict pos) {
      G(72, swapu64(G(73, &pos->colour[0]), G(73, &pos->colour[1]));)

      G(
          72, // Hack to flip the first 10 bitboards in Position.
              // Technically UB but works in GCC 14.2
          u64 *pos_ptr = (u64 *)pos;
          for (i32 i = 0; i < 10; i++) { pos_ptr[i] = flip_bb(pos_ptr[i]); })
      G(72, pos->flipped ^= 1;)
      G(
          72, for (i32 i = 0; i < 2; i++) {
            swapbool(G(74, &pos->castling[i + 2]), G(74, &pos->castling[i]));
          })
    })

G(
    62, [[nodiscard]] S(1) u64 get_mobility(H(75, 1, const Position *pos),
                                            H(75, 1, const i32 sq),
                                            H(75, 1, const i32 piece)) {
      u64 moves = 0;
      const u64 bb = 1ULL << sq;
      G(76, if (piece == King) { moves = king(bb); })
      else G(76, if (piece == Knight) { moves = knight(bb); }) else {
        const u64 blockers = G(77, pos->colour[1]) | G(77, pos->colour[0]);
        G(
            78, if (G(79, piece == Queen) || G(79, piece == Bishop)) {
              moves |= bishop(H(31, 3, blockers), H(31, 3, bb));
            })
        G(
            78, if (G(80, piece == Queen) || G(80, piece == Rook)) {
              moves |= rook(H(34, 3, blockers), H(34, 3, bb));
            })
      }
      return moves;
    })

S(0) i32 find_in_check(const Position *restrict pos) {
  return is_attacked(
      H(63, 2, pos),
      H(63, 2, G(81, pos->colour[0]) & G(81, pos->pieces[King])));
}

G(
    82, S(0) i32 makemove(H(83, 1, Position *const restrict pos),
                          H(83, 1, const Move *const restrict move)) {
      assert(move->from >= 0);
      assert(move->from < 64);
      assert(move->to >= 0);
      assert(move->to < 64);
      assert(move->from != move->to);
      assert(move->promo == None || move->promo == Knight ||
             move->promo == Bishop || move->promo == Rook ||
             move->promo == Queen);
      assert(move->takes_piece != King);
      assert(move->takes_piece == piece_on(H(60, 2, pos), H(60, 2, move->to)));

      G(84, const u64 from = 1ull << move->from;)
      G(84, const u64 to = 1ull << move->to;)
      G(85, const i32 piece = piece_on(H(60, 3, pos), H(60, 3, move->from));
        assert(piece != None);)
      G(85, const u64 mask = G(86, from) | G(86, to);)

      G(
          87, // Castling
          if (piece == King) {
            const u64 bb = move->to - move->from == 2   ? 0xa0
                           : move->from - move->to == 2 ? 0x9
                                                        : 0;
            G(88, pos->pieces[Rook] ^= bb;)
            G(88, pos->colour[0] ^= bb;)
          })

      G(87, pos->colour[0] ^= mask;)

      // Move the piece
      G(87, pos->pieces[piece] ^= mask;)
      G(
          87, // Captures
          if (move->takes_piece != None) {
            G(89, pos->colour[1] ^= to;)
            G(89, pos->pieces[move->takes_piece] ^= to;)
          })

      // En passant
      if (G(90, piece == Pawn) && G(90, to == pos->ep)) {
        G(91, pos->colour[1] ^= to >> 8;)
        G(91, pos->pieces[Pawn] ^= to >> 8;)
      }
      pos->ep = 0;

      G(
          92, // Pawn double move
          if (G(93, move->to - move->from == 16) && G(93, piece == Pawn)) {
            pos->ep = to >> 8;
          })

      G(
          92, // Promotions
          if (move->promo != None) {
            G(94, pos->pieces[Pawn] ^= to;)
            G(94, pos->pieces[move->promo] ^= to;)
          })

      G(92, // Update castling permissions
        G(95, pos->castling[1] &= !(mask & 0x11ull);)
            G(95, pos->castling[3] &= !(mask & 0x1100000000000000ull);)
                G(95, pos->castling[2] &= !(mask & 0x9000000000000000ull);)
                    G(95, pos->castling[0] &= !(mask & 0x90ull);))

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
    82,
    S(1) Move *generate_piece_moves(H(96, 1, const u64 to_mask),
                                    H(96, 1, Move *restrict movelist),
                                    H(96, 1, const Position *restrict pos)) {
      for (i32 piece = Knight; piece <= King; piece++) {
        assert(piece == Knight || piece == Bishop || piece == Rook ||
               piece == Queen || piece == King);
        u64 copy = G(97, pos->colour[0]) & G(97, pos->pieces[piece]);
        while (copy) {
          const u8 from = lsb(copy);
          assert(from >= 0);
          assert(from < 64);
          G(98, copy &= copy - 1;)

          G(98, u64 moves = G(99, to_mask) &
                            G(99, get_mobility(H(75, 2, pos), H(75, 2, from),
                                               H(75, 2, piece)));)

          while (moves) {
            const u8 to = lsb(moves);
            assert(to >= 0);
            assert(to < 64);

            G(100, *movelist++ = ((Move){
                       .from = from,
                       .to = to,
                       .promo = None,
                       .takes_piece = piece_on(H(60, 4, pos), H(60, 4, to))});)
            G(100, moves &= moves - 1;)
          }
        }
      }

      return movelist;
    })

G(
    82, S(0) Move *generate_pawn_moves(H(101, 1, const Position *const pos),
                                       H(101, 1, Move *restrict movelist),
                                       H(101, 1, u64 to_mask),
                                       H(101, 1, const i32 offset)) {
      while (to_mask) {
        const u8 to = lsb(to_mask);
        to_mask &= to_mask - 1;
        const u8 from = G(102, to) + G(102, offset);
        assert(from >= 0);
        assert(from < 64);
        assert(to >= 0);
        assert(to < 64);
        assert(piece_on(H(60, 5, pos), H(60, 5, from)) == Pawn);
        const u8 takes = piece_on(H(60, 6, pos), H(60, 6, to));
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

[[nodiscard]] S(1) i32 movegen(H(103, 1, const Position *const restrict pos),
                               H(103, 1, Move *restrict movelist),
                               H(103, 1, const i32 only_captures)) {

  G(104, const u64 all = G(105, pos->colour[0]) | G(105, pos->colour[1]);)
  G(104, const Move *start = movelist;)
  G(104, const u64 to_mask = only_captures ? pos->colour[1] : ~pos->colour[0];)
  G(
      106, // PAWN PROMOTIONS
      if (!only_captures) {
        movelist = generate_pawn_moves(
            H(101, 2, pos), H(101, 2, movelist),
            H(101, 2,
              G(107, north(G(108, north(G(109, pos->colour[0]) &
                                        G(109, pos->pieces[Pawn]) &
                                        G(109, 0xFF00))) &
                           G(108, ~all))) &
                  G(107, ~all)),
            H(101, 2, -16));
      })
  G(106, // PAWN DOUBLE MOVES
    movelist = generate_pawn_moves(
        H(101, 3, pos), H(101, 3, movelist),
        H(101, 3,
          north(G(110, G(111, pos->colour[0]) & G(111, pos->pieces[Pawn]))) &
              G(110, (only_captures ? 0xFF00000000000000ull : ~0ull)) &
              G(110, ~all)),
        H(101, 3, -8));)
  G(106, // PAWN WEST CAPTURES
    movelist = generate_pawn_moves(
        H(101, 4, pos), H(101, 4, movelist),
        H(101, 4,
          G(112, nw(G(113, pos->colour[0]) & G(113, pos->pieces[Pawn]))) &
              G(112, (G(114, pos->colour[1]) | G(114, pos->ep)))),
        H(101, 4, -7));)
  G(106, // PAWN EAST CAPTURES
    movelist = generate_pawn_moves(
        H(101, 5, pos), H(101, 5, movelist),
        H(101, 5,
          G(115, ne(G(116, pos->colour[0]) & G(116, pos->pieces[Pawn]))) &
              G(115, (G(117, pos->colour[1]) | G(117, pos->ep)))),
        H(101, 5, -9));)
  G(
      106, // LONG CASTLE
      if (G(118, !only_captures) && G(118, pos->castling[0]) &&
          G(118, !(G(119, all) & G(119, 0x60ull))) &&
          G(120, !is_attacked(H(63, 3, pos), H(63, 3, 1ULL << 5))) &&
          G(120, !is_attacked(H(63, 4, pos), H(63, 4, 1ULL << 4)))) {
        *movelist++ =
            (Move){.from = 4, .to = 6, .promo = None, .takes_piece = None};
      })
  G(
      106, // SHORT CASTLE
      if (G(121, !only_captures) && G(121, pos->castling[1]) &&
          G(121, !(G(122, all) & G(122, 0xEull))) &&
          G(123, !is_attacked(H(63, 6, pos), H(63, 6, 1ULL << 4))) &&
          G(123, !is_attacked(H(63, 5, pos), H(63, 5, 1ULL << 3)))) {
        *movelist++ =
            (Move){.from = 4, .to = 2, .promo = None, .takes_piece = None};
      })
  movelist = generate_piece_moves(H(96, 2, to_mask), H(96, 2, movelist),
                                  H(96, 2, pos));

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
      movegen(H(103, 2, pos), H(103, 2, moves), H(103, 2, false));

  for (i32 i = 0; i < num_moves; ++i) {
    Position npos = *pos;

    // Check move legality
    if (!makemove(H(83, 2, &npos), H(83, 2, &moves[i]))) {
      continue;
    }

    nodes += perft(&npos, depth - 1);
  }

  return nodes;
}

static void get_fen(Position *restrict pos, char *restrict fen) {
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
  getl(fen);
  p = fen;
  if (*p != '-') {
    const i32 file = p[0] - 'a';
    const i32 rank = p[1] - '1';
    pos->ep = 1ull << (rank * 8 + file);
  }

  if (black_to_move) {
    flip_pos(pos);
  }
}

typedef struct [[nodiscard]] __attribute__((packed)) {
  i16 material[7];
  H(124, 1,
    H(125, 1, u8 pawn_attacked_penalty[2];) H(125, 1, i8 mobilities[5];)
        H(125, 1, i8 tempo;) H(125, 1, i8 open_files[6];)
            H(125, 1, i8 pst_file[48];))
  H(124, 1,
    H(126, 1, i8 passed_blocked_pawns[6];) H(126, 1, i8 bishop_pair;)
        H(126, 1, i8 protected_pawn;) H(126, 1, i8 pst_rank[48];)
            H(126, 1, i8 passed_pawns[6];) H(126, 1, i8 king_attacks[5];))
} EvalParams;

typedef struct [[nodiscard]] __attribute__((packed)) {
  i32 material[7];
  H(124, 2,
    H(125, 2, i32 pawn_attacked_penalty[2];) H(125, 2, i32 mobilities[5];)
        H(125, 2, i32 tempo;) H(125, 2, i32 open_files[6];)
            H(125, 2, i32 pst_file[48];))
  H(124, 2,
    H(125, 2, i32 passed_blocked_pawns[6];) H(126, 2, i32 bishop_pair;)
        H(125, 2, i32 protected_pawn;) H(126, 2, i32 pst_rank[48];)
            H(125, 2, i32 passed_pawns[6];) H(126, 2, i32 king_attacks[5];))

} EvalParamsMerged;

typedef struct [[nodiscard]] __attribute__((packed)) {
  G(127, i8 phases[7];)
  G(127, EvalParams eg;)
  G(127, EvalParams mg;)
} EvalParamsInitial;

G(128, S(0) EvalParamsMerged eval_params;)

G(128,
  __attribute__((aligned(8))) S(1)
      const EvalParamsInitial initial_params = {.phases = {0, 0, 1, 1, 2, 4, 0},
                                                .mg = {.material = {0, 67, 267,
                                                                    273, 360,
                                                                    774, 0},
                                                       .pst_rank =
                                                           {
                                                               0,   -8,  -15,
                                                               -9,  4,   28,
                                                               92,  0, // Pawn
                                                               -22, -11, 2,
                                                               14,  25,  44,
                                                               24,  -76, // Knight
                                                               -9,  6,   13,
                                                               13,  16,  17,
                                                               -2,  -54, // Bishop
                                                               1,   -11, -16,
                                                               -19, 1,   21,
                                                               10,  14, // Rook
                                                               15,  17,  9,
                                                               -2,  -6,  0,
                                                               -23, -10, // Queen
                                                               -3,  -1,  -32,
                                                               -42, -11, 40,
                                                               49,  46, // King
                                                           },
                                                       .pst_file =
                                                           {
                                                               -17, -11, -9,
                                                               0,   5,   20,
                                                               20,  -7, // Pawn
                                                               -25, -10, 0,
                                                               14,  11,  11,
                                                               4,   -4, // Knight
                                                               -11, 3,   5,
                                                               3,   6,   -3,
                                                               3,   -6, // Bishop
                                                               -9,  -7,  1,
                                                               11,  13,  1,
                                                               -2,  -9, // Rook
                                                               -10, -7,  -2,
                                                               1,   1,   0,
                                                               8,   8, // Queen
                                                               -17, 24,  -4,
                                                               -54, -20, -38,
                                                               21,  2, // King
                                                           },
                                                       .mobilities = {6, 5, 2,
                                                                      3, -10},
                                                       .king_attacks = {0, 15,
                                                                        21, 13,
                                                                        0},
                                                       .open_files = {18, -9,
                                                                      -9, 20,
                                                                      -3, -31},
                                                       .passed_pawns =
                                                           {-14, -16, -11, 9,
                                                            30, 92},
                                                       .passed_blocked_pawns =
                                                           {4, -1, 5, 13, 15,
                                                            -32},
                                                       .protected_pawn = 10,
                                                       .bishop_pair = 24,
                                                       .pawn_attacked_penalty =
                                                           {-16, -128},
                                                       .tempo = 17},
                                                .eg = {.material = {0, 85, 399,
                                                                    395, 710,
                                                                    1346, 0},
                                                       .pst_rank =
                                                           {
                                                               0,   -4,  -9,
                                                               -7,  0,   20,
                                                               99,  0, // Pawn
                                                               -38, -22, -5,
                                                               22,  28,  10,
                                                               1,   3, // Knight
                                                               -14, -12, -2,
                                                               3,   7,   4,
                                                               3,   10, // Bishop
                                                               -24, -24, -15,
                                                               4,   15,  14,
                                                               21,  9, // Rook
                                                               -66, -50, -23,
                                                               11,  34,  33,
                                                               41,  20, // Queen
                                                               -50, -7,  8,
                                                               23,  34,  31,
                                                               16,  -50, // King
                                                           },
                                                       .pst_file =
                                                           {
                                                               11,  12,  0,
                                                               -11, -4,  -3,
                                                               0,   -6, // Pawn
                                                               -22, -4,  9,
                                                               17,  17,  7,
                                                               -2,  -22, // Knight
                                                               -9,  -1,  0,
                                                               5,   6,   5,
                                                               1,   -8, // Bishop
                                                               2,   5,   5,
                                                               -1,  -5,  1,
                                                               0,   -6, // Rook
                                                               -23, -6,  4,
                                                               10,  16,  12,
                                                               -2,  -12, // Queen
                                                               -30, 1,   17,
                                                               31,  23,  25,
                                                               0,   -40, // King
                                                           },
                                                       .mobilities = {2, 5, 4,
                                                                      1, -4},
                                                       .king_attacks = {0, -4,
                                                                        -7, 10,
                                                                        0},
                                                       .open_files = {27, -4, 7,
                                                                      10, 28,
                                                                      9},
                                                       .passed_pawns =
                                                           {6, 13, 37, 64, 110,
                                                            99},
                                                       .passed_blocked_pawns =
                                                           {-13, -15, -40, -70,
                                                            -119, -124},
                                                       .protected_pawn = 12,
                                                       .bishop_pair = 63,
                                                       .pawn_attacked_penalty =
                                                           {-10, -128},
                                                       .tempo = 7}};)

G(
    128,
    [[nodiscard]] S(1) i32 combine_eval_param(H(129, 1, const i32 mg_val),
                                              H(129, 1, const i32 eg_val)) {
      return G(130, mg_val) + G(130, (eg_val << 16));
    })

S(1) i32 eval(Position *const restrict pos) {
  G(131, i32 score = eval_params.tempo;)
  G(131, i32 phase = 0;)

  for (i32 c = 0; c < 2; c++) {

    G(132,
      const u64 opp_pawns = G(134, pos->pieces[Pawn]) & G(134, pos->colour[1]);
      const u64 attacked_by_pawns =
          G(135, se(opp_pawns)) | G(135, sw(opp_pawns));
      G(999,
        const u64 no_passers = G(136, opp_pawns) | G(136, attacked_by_pawns);)
          G(999, score -= eval_params.protected_pawn *
                          count(opp_pawns & attacked_by_pawns);))

    G(132,
      const u64 own_pawns = G(137, pos->pieces[Pawn]) & G(137, pos->colour[0]);)
    G(132, const u64 opp_king_zone =
               king(G(138, pos->colour[1]) & G(138, pos->pieces[King]));)
    G(
        132, // BISHOP PAIR
        if (count(G(133, pos->pieces[Bishop]) & G(133, pos->colour[0])) > 1) {
          score += eval_params.bishop_pair;
        })

    for (i32 p = Pawn; p <= King; p++) {
      u64 copy = G(139, pos->colour[0]) & G(139, pos->pieces[p]);
      while (copy) {
        const i32 sq = lsb(copy);
        G(140, const int file = G(141, sq) & G(141, 7);)
        G(140, phase += initial_params.phases[p];)
        G(140, const int rank = sq >> 3;)
        G(140, copy &= copy - 1;)

        G(
            101, // OPEN FILES / DOUBLED PAWNS
            if ((G(142, north(0x101010101010101ULL << sq)) &
                 G(142, own_pawns)) == 0) {
              score += eval_params.open_files[p - 1];
            })

        G(101, // SPLIT PIECE-SQUARE TABLES FOR FILE
          score +=
          eval_params
              .pst_file[G(143, G(144, (p - 1)) * G(144, 8)) + G(143, file)];)
        G(101, // SPLIT PIECE-SQUARE TABLES FOR RANK
          score +=
          eval_params
              .pst_rank[G(143, G(145, (p - 1)) * G(145, 8)) + G(143, rank)];)

        G(101, // MATERIAL
          score += eval_params.material[p];)

        G(
            101, if (p > Pawn) {
              G(
                  146, // PIECES ATTACKED BY PAWNS
                  if (G(147, 1ULL << sq) & G(147, no_passers)) {
                    score += eval_params.pawn_attacked_penalty[c];
                  })

              G(146, const u64 mobility =
                         G(148, get_mobility(H(75, 3, pos), H(75, 3, sq),
                                             H(75, 3, p))) &
                         G(148, G(149, ~pos->colour[0]));

                G(150, // MOBILITY
                  score +=
                  G(151, count(G(149, mobility) & G(149, ~attacked_by_pawns))) *
                  G(151, eval_params.mobilities[p - 2]);)

                    G(150, // KING ATTACKS
                      score +=
                      G(152, count(G(153, mobility) & G(153, opp_king_zone))) *
                      G(152, eval_params.king_attacks[p - 2]);))
            })

        G(
            101, // PASSED PAWNS
            if (G(154, !(G(155, (0x101010101010101ULL << sq)) &
                         G(155, no_passers))) &&
                G(154, p == Pawn)) {
              G(
                  156, if (G(157, north(1ULL << sq)) & G(157, pos->colour[1])) {
                    score += eval_params.passed_blocked_pawns[rank - 1];
                  })

              G(156, score += eval_params.passed_pawns[rank - 1];)
            })
      }
    }

    G(96, score = -score;)
    G(96, flip_pos(pos);)
  }

  const i32 stronger_side_pawns_missing =
      8 - count(G(158, pos->colour[score < 0]) & G(158, pos->pieces[Pawn]));
  return (G(159, (i16)score) * G(159, phase) +
          G(160,
            (128 - stronger_side_pawns_missing * stronger_side_pawns_missing)) *
              G(160, ((score + 0x8000) >> 16)) / 128 * (24 - phase)) /
         24;
}

typedef struct [[nodiscard]] {
  G(125, i32 static_eval;)
  G(125, i32 num_moves;)
  G(125, u64 position_hash;)
  G(125, Move best_move;)
  G(125, Move killer;)
  G(125, Move moves[max_moves];)
} SearchStack;

typedef struct [[nodiscard]] __attribute__((packed)) {
  G(162, i8 depth;)
  G(162, u16 partial_hash;)
  G(162, i16 score;)
  G(162, Move move;)
  G(162, u8 flag;)
} TTEntry;
_Static_assert(sizeof(TTEntry) == 10);

enum { tt_length = 64 * 1024 * 1024 / sizeof(TTEntry) };
enum { Upper = 0, Lower = 1, Exact = 2 };
enum { max_ply = 96 };
enum { mate = 31744, inf = 32256 };

G(163, S(1) i32 move_history[2][6][64][64];)
G(163, S(0) size_t start_time;)
G(163, S(0) size_t max_time;)
G(163, S(1) TTEntry tt[tt_length];)

#if defined(__x86_64__) || defined(_M_X64)
typedef long long __attribute__((__vector_size__(16))) i128;

[[nodiscard]] __attribute__((target("aes"))) S(1) u64
    get_hash(const Position *const pos) {
  i128 hash = {0};

  // USE 16 BYTE POSITION SEGMENTS AS KEYS FOR AES
  const u8 *const data = (const u8 *)pos;
  for (i32 i = 0; i < 6; i++) {
    i128 key;
    __builtin_memcpy(&key, data + G(164, i) * G(164, 16), 16);
    hash = __builtin_ia32_aesenc128(hash, key);
  }

  // FINAL ROUND FOR BIT MIXING
  hash = __builtin_ia32_aesenc128(hash, hash);

  // USE FIRST 64 BITS AS POSITION HASH
  return hash[0];
}
#elif defined(__aarch64__)

#include <arm_neon.h>

[[nodiscard]] __attribute__((target("+aes"))) u64
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

S(1)
i32 search(H(165, 1, const i32 beta), H(165, 1, i32 alpha),
           H(165, 1, i32 depth), H(165, 1, const bool do_null),
           H(165, 1, SearchStack *restrict stack),
#ifdef FULL
           u64 *nodes,
#endif
           H(166, 1, Position *const restrict pos),
           H(166, 1, const i32 pos_history_count), H(166, 1, const i32 ply)) {
  assert(alpha < beta);
  assert(ply >= 0);

  // IN-CHECK EXTENSION
  const bool in_check = find_in_check(pos);
  depth += in_check;

  // FULL REPETITION DETECTION
  const u64 tt_hash = get_hash(pos);
  bool in_qsearch = depth <= 0;
  for (i32 i = G(167, ply) + G(167, pos_history_count);
       G(168, i > 0) && G(168, do_null); i -= 2) {
    if (G(169, tt_hash) == G(169, stack[i].position_hash)) {
      return 0;
    }
  }

  // TT PROBING
  G(170, stack[ply].best_move = (Move){0};)
  G(170, TTEntry *tt_entry = &tt[tt_hash % tt_length];)
  G(170, const u16 tt_hash_partial = tt_hash / tt_length;)
  if (G(171, tt_entry->partial_hash) == G(171, tt_hash_partial)) {
    stack[ply].best_move = tt_entry->move;

    // TT PRUNING
    if (G(172, G(173, tt_entry->flag) != G(173, tt_entry->score <= alpha)) &&
        G(172, tt_entry->depth >= depth) &&
        G(172, G(174, alpha) == G(174, beta - 1))) {
      return tt_entry->score;
    }
  } else if (depth > 3) {

    // INTERNAL ITERATIVE REDUCTION
    depth--;
  }

  // STATIC EVAL WITH ADJUSTMENT FROM TT
  i32 static_eval = eval(pos);
  assert(static_eval < mate);
  assert(static_eval > -mate);

  stack[ply].static_eval = static_eval;
  const bool improving = ply > 1 && static_eval > stack[ply - 2].static_eval;
  if (G(175, G(176, tt_entry->partial_hash) == G(176, tt_hash_partial)) &&
      G(175, G(177, tt_entry->flag) != G(177, static_eval) > tt_entry->score)) {
    static_eval = tt_entry->score;
  }

  // QUIESCENCE
  if (G(178, static_eval > alpha) && G(178, in_qsearch)) {
    if (static_eval >= beta) {
      return static_eval;
    }
    alpha = static_eval;
  }

  if (G(179, !in_check) && G(179, G(180, alpha) == G(180, beta - 1))) {
    if (G(181, depth < 8) && G(181, !in_qsearch)) {

      G(182, {
        // REVERSE FUTILITY PRUNING
        if (static_eval - G(183, 56) * G(183, (depth - improving)) >= beta) {
          return static_eval;
        }
      })

      G(182, // RAZORING
        in_qsearch =
            G(184, static_eval) + G(184, G(185, 122) * G(185, depth)) <= alpha;)
    }

    // NULL MOVE PRUNING
    if (G(186, depth > 2) && G(186, static_eval >= beta) && G(186, do_null)) {
      Position npos = *pos;
      G(187, flip_pos(&npos);)
      G(187, npos.ep = 0;)
      const i32 score = -search(
          H(165, 2, -alpha), H(165, 2, -beta),
          H(165, 2, depth - G(188, 4) - G(188, depth / 4)), H(165, 2, false),
          H(165, 2, stack),
#ifdef FULL
          nodes,
#endif
          H(166, 2, &npos), H(166, 2, pos_history_count), H(166, 2, ply + 1));
      if (score >= beta) {
        return score;
      }
    }
  }

  G(189, i32 moves_evaluated = 0;)
  G(189, i32 quiets_evaluated = 0;)
  G(189,
    stack[ply].num_moves = movegen(H(103, 3, pos), H(103, 3, stack[ply].moves),
                                   H(103, 3, in_qsearch));)
  G(189, u8 tt_flag = Upper;)
  G(189, i32 best_score = in_qsearch ? static_eval : -inf;)
  G(189,
    stack[G(190, pos_history_count) + G(190, ply) + G(190, 2)].position_hash =
        tt_hash;)

  for (i32 move_index = 0; move_index < stack[ply].num_moves; move_index++) {
    // MOVE ORDERING
    G(191, i32 move_score = ~0x1010101LL;)
    G(191, i32 best_index = 0;)
    for (i32 order_index = move_index; order_index < stack[ply].num_moves;
         order_index++) {
      assert(
          stack[ply].moves[order_index].takes_piece ==
          piece_on(H(60, 7, pos), H(60, 7, stack[ply].moves[order_index].to)));
      const i32 order_move_score =
          G(165, // PREVIOUS BEST MOVE FIRST
            (move_equal(G(192, &stack[ply].best_move),
                        G(192, &stack[ply].moves[order_index]))
             << 30)) +
          G(165, // KILLER MOVE
            G(193, move_equal(G(194, &stack[ply].moves[order_index]),
                              G(194, &stack[ply].killer))) *
                G(193, 836)) +
          G(165, // MOST VALUABLE VICTIM
            G(195, stack[ply].moves[order_index].takes_piece) * G(195, 712)) +
          G(165, // HISTORY HEURISTIC
            move_history[pos->flipped]
                        [stack[ply].moves[order_index].takes_piece]
                        [stack[ply].moves[order_index].from]
                        [stack[ply].moves[order_index].to]);
      if (order_move_score > move_score) {
        G(196, best_index = order_index;)
        G(196, move_score = order_move_score;)
      }
    }

    swapmoves(G(197, &stack[ply].moves[move_index]),
              G(197, &stack[ply].moves[best_index]));

    // FORWARD FUTILITY PRUNING / DELTA PRUNING
    if (G(198, depth < 8) &&
        G(198,
          G(199, static_eval) + G(199, G(200, 142) * G(200, depth)) +
                  G(199, initial_params.eg
                             .material[stack[ply].moves[move_index].promo]) +
                  G(199,
                    initial_params.eg
                        .material[stack[ply].moves[move_index].takes_piece]) <
              alpha) &&
        G(198, !in_check) && G(198, moves_evaluated)) {
      break;
    }

    Position npos = *pos;
#ifdef FULL
    (*nodes)++;
#endif
    if (!makemove(H(83, 3, &npos), H(83, 3, &stack[ply].moves[move_index]))) {
      continue;
    }

    // PRINCIPAL VARIATION SEARCH
    i32 low = moves_evaluated == 0 ? -beta : -alpha - 1;
    moves_evaluated++;

    // LATE MOVE REDUCTION
    i32 reduction = G(201, depth > 3) && G(201, move_score <= 0)
                        ? G(202, moves_evaluated / 10) +
                              G(202, (G(203, alpha) == G(203, beta - 1))) +
                              G(202, (move_score < -256)) + G(202, !improving)
                        : 0;

    i32 score;
    while (true) {
      score = -search(H(165, 3, -alpha), H(165, 3, low),
                      H(165, 3, depth - G(204, 1) - G(204, reduction)),
                      H(165, 3, true), H(165, 3, stack),
#ifdef FULL
                      nodes,
#endif
                      H(166, 3, &npos), H(166, 3, pos_history_count),
                      H(166, 3, ply + 1));

      // EARLY EXITS
      if (depth > 4 && get_time() - start_time > max_time) {
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
        G(205, tt_flag = Exact;)
        G(205, stack[ply].best_move = stack[ply].moves[move_index];)
        G(205, alpha = score;)
        if (score >= beta) {
          assert(stack[ply].best_move.takes_piece ==
                 piece_on(H(60, 8, pos), H(60, 8, stack[ply].best_move.to)));
          G(206, tt_flag = Lower;)
          G(
              206, if (stack[ply].best_move.takes_piece == None) {
                stack[ply].killer = stack[ply].best_move;
              })
          G(
              206, if (!in_qsearch) {
                const i32 bonus = depth * depth;
                G(207, i32 *const this_hist =
                           &move_history[pos->flipped]
                                        [stack[ply].best_move.takes_piece]
                                        [stack[ply].best_move.from]
                                        [stack[ply].best_move.to];

                  *this_hist +=
                  bonus - G(208, bonus) * G(208, *this_hist) / 1024;)
                G(
                    207, for (i32 prev_index = 0; prev_index < move_index;
                              prev_index++) {
                      const Move prev = stack[ply].moves[prev_index];
                      i32 *const prev_hist =
                          &move_history[pos->flipped][prev.takes_piece]
                                       [prev.from][prev.to];
                      *prev_hist -=
                          bonus + G(209, bonus) * G(209, *prev_hist) / 1024;
                    })
              })
          break;
        }
      }
    }

    if (stack[ply].moves[move_index].takes_piece == None) {
      quiets_evaluated++;
    }

    // LATE MOVE PRUNING
    if (G(210, !in_check) && G(210, G(211, alpha) == G(211, beta - 1)) &&
        G(210, quiets_evaluated > (G(212, 1) + G(212, depth * depth)) >>
                   !improving)) {
      break;
    }
  }

  // MATE / STALEMATE DETECTION
  if (G(213, best_score) == G(213, -inf)) {
    return G(214, (ply - mate)) * G(214, in_check);
  }

  *tt_entry = (TTEntry){.partial_hash = tt_hash_partial,
                        .move = stack[ply].best_move,
                        .score = best_score,
                        .depth = depth,
                        .flag = tt_flag};

  return best_score;
}

S(1) void init() {
  G(
      83, // INIT DIAGONAL MASKS
      for (i32 sq = 0; sq < 64; sq++) {
        const u64 bb = 1ULL << sq;
        diag_mask[sq] =
            G(215, ray(H(24, 5, 0), H(24, 5, ~0x101010101010101ull),
                       H(24, 5, bb), H(24, 5, 9))) | // Northeast
            G(215, ray(H(24, 4, 0), H(24, 4, ~0x8080808080808080ull),
                       H(24, 4, bb), H(24, 4, -9))); // Southwest
      })
  G(
      83, // MERGE MATERIAL VALUES
      for (i32 i = 0; i < sizeof(initial_params.mg.material) / sizeof(i16);
           i++) {
        eval_params.material[i] =
            combine_eval_param(H(129, 2, initial_params.mg.material[i]),
                               H(129, 2, initial_params.eg.material[i]));
      })

  G(83, // CLEAR HISTORY
    __builtin_memset(move_history, 0, sizeof(move_history));)
  G(
      83, // MERGE NON-MATERIAL VALUES
      for (i32 i = 0;
           i < sizeof(initial_params.mg) - sizeof(initial_params.mg.material);
           i++) {
        // Technically writes past end of array
        // But since the structs are packed, it works
        const i32 offset = sizeof(initial_params.mg.material);
        ((i32 *)&eval_params)[G(216,
                                offset / sizeof(*initial_params.mg.material)) +
                              G(216, i)] =
            combine_eval_param(
                H(129, 3,
                  ((i8 *)&initial_params.mg)[G(217, offset) + G(217, i)]),
                H(129, 3,
                  ((i8 *)&initial_params.eg)[G(218, offset) + G(218, i)]));
      })
}

static void print_info(const Position *pos, const i32 depth, const i32 alpha,
                       const i32 beta, const i32 score, const u64 nodes,
                       const Move pv_move) {
  // Do not print unfinished iteration scores
  size_t elapsed = get_time() - start_time;
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

  printf("time %i nodes %i", elapsed, nodes);

  // Only print nps if >1 ms elapsed
  if (elapsed > 0) {
    const u64 nps = nodes * 1000 / elapsed;
    printf(" nps %i", nps);
  }

  // Only print pv move if within window
  if (score > alpha && score < beta) {
    putl(" pv ");
    char move_name[8];
    move_str(H(57, 2, move_name), H(57, 2, &pv_move), H(57, 2, pos->flipped));
    putl(move_name);
  }

  putl("\n");
}

S(1)
void iteratively_deepen(
#ifdef FULL
    i32 maxdepth, u64 *nodes,
#endif
    H(219, 1, Position *const restrict pos),
    H(219, 1, SearchStack *restrict stack),
    H(219, 1, const i32 pos_history_count)) {
  G(220, start_time = get_time();)
  G(220, i32 score = 0;)
#ifdef FULL
  for (i32 depth = 1; depth < maxdepth; depth++) {
#else
  for (i32 depth = 1; depth < max_ply; depth++) {
#endif
    // ASPIRATION WINDOWS
    G(221, i32 window = 15;)
    G(221, size_t elapsed;)
    while (true) {
      G(222, const i32 alpha = score - window;)
      G(222, const i32 beta = G(223, score) + G(223, window);)
      score =
          search(H(165, 4, beta), H(165, 4, alpha), H(165, 4, depth),
                 H(165, 4, false), H(165, 4, stack),
#ifdef FULL
                 nodes,
#endif
                 H(166, 4, pos), H(166, 4, pos_history_count), H(166, 4, 0));
#ifdef FULL
      print_info(pos, depth, alpha, beta, score, *nodes, stack[0].best_move);
#endif
      elapsed = get_time() - start_time;
      G(
          224, if (G(225, (G(226, score > alpha) && G(226, score < beta))) ||
                   G(225, elapsed > max_time)) { break; })
      G(224, window *= 2;)
    }

    if (elapsed > max_time / 16) {
      break;
    }
  }
  char move_name[8];
  move_str(H(57, 3, move_name), H(57, 3, &stack[0].best_move),
           H(57, 3, pos->flipped));
  putl("bestmove ");
  puts(move_name);
}

S(1) void display_pos(Position *const pos) {
  Position npos = *pos;
  if (npos.flipped) {
    flip_pos(&npos);
  }
  for (i32 rank = 7; rank >= 0; rank--) {
    for (i32 file = 0; file < 8; file++) {
      i32 sq = rank * 8 + file;
      u64 bb = 1ULL << sq;
      i32 piece = piece_on(H(60, 9, &npos), H(60, 9, sq));
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
  Position pos;
  i32 pos_history_count = 0;
#ifdef LOWSTACK
  SearchStack *stack = malloc(sizeof(SearchStack) * 1024);
#else
  SearchStack stack[1024];
#endif
  __builtin_memset(move_history, 0, sizeof(move_history));
  pos = start_pos;
  max_time = 99999999999;
  u64 nodes = 0;
  const u64 start = get_time();
  iteratively_deepen(23, &nodes, H(219, 2, &pos), H(219, 2, stack),
                     H(219, 2, pos_history_count));
  const u64 end = get_time();
  const i32 elapsed = end - start;
  const u64 nps = elapsed ? 1000 * nodes / elapsed : 0;
  printf("%i nodes %i nps\n", nodes, nps);
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

  G(227, char line[4096];)
  G(227, Position pos;)
  G(227, i32 pos_history_count;)
  G(227, // #ifdef LOWSTACK
         //  SearchStack *stack = malloc(sizeof(SearchStack) * 1024);
         // #else
    SearchStack stack[1024];
    // #endif
  )
  G(227, init();)

#ifdef FULL
  pos = start_pos;
  pos_history_count = 0;
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
    u64 nodes = 0;
    if (!strcmp(line, "uci")) {
      puts("id name 4k.c");
      puts("id author Gediminas Masaitis");
      puts("");
      puts("option name Hash type spin default 1 min 1 max 1");
      puts("option name Threads type spin default 1 min 1 max 1");
      puts("uciok");
    } else if (!strcmp(line, "ucinewgame")) {
      __builtin_memset(tt, 0, sizeof(tt));
      __builtin_memset(move_history, 0, sizeof(move_history));
    } else if (!strcmp(line, "bench")) {
      bench();
    } else if (!strcmp(line, "gi")) {
      max_time = 99999999999;
      iteratively_deepen(max_ply, &nodes, H(219, 3, &pos), H(219, 3, stack),
                         H(219, 3, pos_history_count));
    } else if (!strcmp(line, "d")) {
      display_pos(&pos);
    } else if (!strcmp(line, "perft")) {
      char depth_str[4];
      getl(depth_str);
      const i32 depth = atoi(depth_str);
      const u64 start = get_time();
      nodes = perft(&pos, depth);
      const u64 end = get_time();
      const u64 elapsed = end - start;
      const u64 nps = elapsed ? 1000 * nodes / elapsed : 0;
      printf("info depth %i nodes %i time %i nps %i \n", depth, nodes, elapsed,
             nps);
    }
#endif
    G(228, if (G(229, line[0]) == G(229, 'q')) { exit_now(); })
    else G(228, if (G(230, line[0]) == G(230, 'p')) {
      G(231, pos_history_count = 0;)
        G(231, pos = start_pos;)
        while (true) {
          const bool line_continue = getl(line);

#if FULL
          if (!strcmp(line, "fen")) {
            getl(line);
            get_fen(&pos, line);
          }
#endif

          const i32 num_moves =
            movegen(H(103, 4, &pos), H(103, 4, stack[0].moves), H(103, 4, false));
          for (i32 i = 0; i < num_moves; i++) {
            char move_name[8];
            move_str(H(57, 4, move_name), H(57, 4, &stack[0].moves[i]),
              H(57, 4, pos.flipped));
            assert(move_string_equal(line, move_name) ==
              !strcmp(line, move_name));
            if (move_string_equal(G(232, line), G(232, move_name))) {
              stack[pos_history_count].position_hash = get_hash(&pos);
              pos_history_count++;
              if (stack[0].moves[i].takes_piece != None) {
                pos_history_count = 0;
              }
              makemove(H(83, 4, &pos), H(83, 4, &stack[0].moves[i]));
              break;
            }
          }
          if (!line_continue) {
            break;
          }
        }
    })
    else G(228, if (G(233, line[0]) == G(233, 'g')) {
#ifdef FULL
      while (true) {
        getl(line);
        if (!pos.flipped && !strcmp(line, "wtime")) {
          getl(line);
          max_time = atoi(line) / 2;
          break;
        }
        else if (pos.flipped && !strcmp(line, "btime")) {
          getl(line);
          max_time = atoi(line) / 2;
          break;
        }
        else if (!strcmp(line, "movetime")) {
          max_time = 20000; // Assume Lichess bot
          break;
        }
      }
      iteratively_deepen(max_ply, &nodes, H(219, 4, &pos), H(219, 4, stack),
        H(219, 4, pos_history_count));
#else
      for (i32 i = 0; i < (pos.flipped ? 4 : 2); i++) {
        getl(line);
        max_time = atoi(line) / 2;
      }
      iteratively_deepen(H(219, 5, &pos), H(219, 5, stack),
        H(219, 5, pos_history_count));
#endif
    })
    else G(228, if (G(234, line[0]) == G(234, 'i')) { puts("readyok"); })
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
    bench();
    exit_now();
  }
#endif
  run();
}
#endif

#pragma endregion
