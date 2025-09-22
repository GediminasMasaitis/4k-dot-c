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
                   : "a"(call), "D"(arg1), "S"(arg2), "d"(arg3)
                   : "rcx", "r11", "memory");
      return ret;
    })

G(
    1, S(1) void exit_now() {
      asm volatile("movl $60, %eax\n\t"
                   "syscall");
    })

G(
    1, [[nodiscard]] S(1) i32 strlen(const char *const restrict string) {
      i32 length = 0;
      while (string[length]) {
        length++;
      }
      return length;
    })

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

[[nodiscard]] S(1) bool strcmp(const char *restrict lhs,
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
  _sys(H(2, 5, 1), H(2, 5, 0), H(2, 5, (ssize_t)&ts), H(2, 5, 228));
  return ts.tv_sec * 1000 + ts.tv_nsec / 1000000;
}

#ifdef ASSERTS
#define assert(condition)                                                      \
  if (!(condition)) {                                                          \
    printf("Assert failed on line %i: ", __LINE__);                            \
    puts(#condition);                                                          \
    _sys(H(2, 6, 1), H(2, 6, 0), H(2, 6, 0), H(2, 6, 60));                     \
  }
#else
#define assert(condition)
#endif

#else
#include <assert.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

[[nodiscard]] S(1) size_t get_time() {
  struct timespec ts;
  clock_gettime(CLOCK_MONOTONIC, &ts);
  return ts.tv_sec * 1000 + ts.tv_nsec / 1000000;
}

S(1) void exit_now() { exit(0); }

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

S(1) void putl(const char *const restrict string) {
  fputs(string, stdout);
  fflush(stdout);
}

#endif

#pragma endregion

#pragma region base

enum [[nodiscard]] { None, Pawn, Knight, Bishop, Rook, Queen, King };

typedef struct [[nodiscard]] {
  G(5, u8 promo;)
  G(5, u8 takes_piece;)
  G(5, u8 from; u8 to;)
} Move;

typedef struct [[nodiscard]] {
  G(6, u64 ep;)
  G(6, u64 pieces[7];)
  G(6, u64 colour[2];)
  G(7, bool castling[4];)
  G(7, bool flipped;)
  G(7, u8 padding[11];)
} Position;

[[nodiscard]] S(1) bool move_string_equal(G(8, const char *restrict lhs),
                                          G(8, const char *restrict rhs)) {
  return (G(9, *(const u64 *)rhs) ^ G(9, *(const u64 *)lhs)) << 24 == 0;
}

[[nodiscard]] S(1) u64 flip_bb(const u64 bb) { return __builtin_bswap64(bb); }

#ifdef ARCH32
#pragma GCC push_options
#pragma GCC optimize("O3")
#endif
S(1) i32 lsb(u64 bb) { return __builtin_ctzll(bb); }
#ifdef ARCH32
#pragma GCC pop_options
#endif

G(
    10, [[nodiscard]] S(1)
            u64 shift(H(11, 1, const i32 shift), H(11, 1, const u64 mask),
                      H(11, 1, const u64 bb)) {
              return shift > 0 ? bb << shift & mask : bb >> -shift & mask;
            })

G(
    10, [[nodiscard]] S(1)
            i32 count(const u64 bb) { return __builtin_popcountll(bb); })

G(
    12, [[nodiscard]] S(1)
            u64 west(const u64 bb) { return bb >> 1 & ~0x8080808080808080ull; })

G(12, [[nodiscard]] S(1) u64 north(const u64 bb) { return bb << 8; })

G(12, [[nodiscard]] S(1) u64 south(const u64 bb) { return bb >> 8; })

G(
    12, [[nodiscard]] S(1)
            u64 east(const u64 bb) { return bb << 1 & ~0x101010101010101ull; })

G(13, [[nodiscard]] S(1) u64 se(const u64 bb) { return east(south(bb)); })

G(13, [[nodiscard]] S(1) u64 ne(const u64 bb) { return east(north(bb)); })

G(
    13, [[nodiscard]] S(1) u64 nw(const u64 bb) {
      return shift(H(11, 2, 7), H(11, 2, ~0x8080808080808080ull), H(11, 2, bb));
      // return west(north(bb));
    })

G(13, [[nodiscard]] S(1) u64 sw(const u64 bb) { return west(south(bb)); })

G(14, S(1) u64 diag_mask[64];)

G(
    14, [[nodiscard]] S(1)
            u64 ray(H(15, 1, const u64 blockers), H(15, 1, const u64 mask),
                    H(15, 1, const u64 bb), H(15, 1, const i32 shift_by)) {
              u64 result =
                  shift(H(11, 3, shift_by), H(11, 3, mask), H(11, 3, bb));
              for (i32 i = 0; i < 6; i++) {
                result |= shift(H(11, 4, shift_by), H(11, 4, mask),
                                H(11, 4, result & ~blockers));
              }
              return result;
            })

G(
    14, [[nodiscard]] S(0) u64 xattack(H(16, 1, const u64 dir_mask),
                                       H(16, 1, const u64 bb),
                                       H(16, 1, const u64 blockers)) {
      return dir_mask & ((blockers & dir_mask) - bb ^
                         flip_bb(flip_bb(blockers & dir_mask) - flip_bb(bb)));
    })

G(
    17, [[nodiscard]] S(0)
            u64 bishop(H(18, 1, const u64 blockers), H(18, 1, const u64 bb)) {
              assert(count(bb) == 1);
              const i32 sq = lsb(bb);
              return xattack(H(16, 2, diag_mask[sq]), H(16, 2, bb),
                             H(16, 2, blockers)) |
                     xattack(H(16, 3, flip_bb(diag_mask[sq ^ 56])),
                             H(16, 3, bb), H(16, 3, blockers));
            })

G(
    17, [[nodiscard]] S(1) u64 king(const u64 bb) {
      return G(19, bb >> 8) | G(19, bb << 8) |
             G(19, (bb << 1 | bb << 9 | bb >> 7) & ~0x101010101010101ull) |
             G(19, (bb >> 1 | bb >> 9 | bb << 7) & ~0x8080808080808080ull);
    })

G(
    17, [[nodiscard]] S(1) u64 knight(const u64 bb) {
      return G(20, (bb << 17 | bb >> 15) & ~0x101010101010101ull) |
             G(20, (bb << 15 | bb >> 17) & ~0x8080808080808080ull) |
             G(20, (bb << 6 | bb >> 10) & 0x3F3F3F3F3F3F3F3Full) |
             G(20, (bb << 10 | bb >> 6) & 0xFCFCFCFCFCFCFCFCull);
    })

G(
    17, [[nodiscard]] S(1)
            u64 rook(H(21, 1, const u64 blockers), H(21, 1, const u64 bb)) {
              assert(count(bb) == 1);
              return xattack(H(16, 4, bb ^ 0x101010101010101ULL << lsb(bb) % 8),
                             H(16, 4, bb), H(16, 4, blockers)) |
                     ray(H(15, 2, blockers), H(15, 2, ~0x101010101010101ull),
                         H(15, 2, bb), H(15, 2, 1)) // East
                     | ray(H(15, 3, blockers), H(15, 3, ~0x8080808080808080ull),
                           H(15, 3, bb), H(15, 3, -1)); // West
            })

G(
    22,
    S(1) void swapu32(G(23, u32 *const rhs), G(23, u32 *const lhs)) {
      const u32 temp = *lhs;
      *lhs = *rhs;
      *rhs = temp;
    }

    S(1) void swapmoves(G(24, Move *const lhs), G(24, Move *const rhs)) {
      swapu32(G(25, (u32 *)lhs), G(25, (u32 *)rhs));
    })

G(
    22, [[nodiscard]] S(1) bool move_equal(G(26, Move *const rhs),
                                           G(26, Move *const lhs)) {
      return *(u32 *)lhs == *(u32 *)rhs;
    })

G(
    22, S(1) void swapu64(G(27, u64 *const lhs), G(27, u64 *const rhs)) {
      const u64 temp = *lhs;
      *lhs = *rhs;
      *rhs = temp;
    })

G(
    22, S(1) void move_str(H(28, 1, char *restrict str),
                           H(28, 1, const Move *restrict move),
                           H(28, 1, const i32 flip)) {
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
        G(29, str[i * 2] = 'a' + (&move->from)[i] % 8;)
        G(29, str[i * 2 + 1] = '1' + ((&move->from)[i] / 8 ^ 7 * flip);)
      }

      G(30, str[5] = '\0';)
      G(30, str[4] = "\0\0nbrq"[move->promo];)
    })

G(
    22, S(1) void swapbool(G(28, bool *const restrict rhs),
                           G(28, bool *const restrict lhs)) {
      const bool temp = *lhs;
      *lhs = *rhs;
      *rhs = temp;
    })

G(
    22, [[nodiscard]] S(1)
            i32 piece_on(H(31, 1, const Position *const restrict pos),
                         H(31, 1, const i32 sq)) {
              assert(sq >= 0);
              assert(sq < 64);
              for (i32 i = Pawn; i <= King; ++i) {
                if (pos->pieces[i] & 1ull << sq) {
                  return i;
                }
              }
              return None;
            })

G(
    32, [[nodiscard]] S(1)
            i32 is_attacked(H(33, 1, const Position *const restrict pos),
                            H(33, 1, const i32 them), H(33, 1, const u64 bb)) {
              assert(count(bb) == 1);
              const u64 theirs = pos->colour[them];
              const u64 pawns = theirs & pos->pieces[Pawn];
              if ((them ? sw(pawns) | se(pawns) : nw(pawns) | ne(pawns)) & bb) {
                return true;
              }
              const u64 blockers = pos->colour[0] | pos->colour[1];
              return G(34, bishop(H(18, 2, blockers), H(18, 2, bb)) & theirs &
                               (pos->pieces[Bishop] | pos->pieces[Queen])) ||
                     G(34, knight(bb) & theirs & pos->pieces[Knight]) ||
                     G(34, king(bb) & theirs & pos->pieces[King]) ||
                     G(34, rook(H(21, 2, blockers), H(21, 2, bb)) & theirs &
                               (pos->pieces[Rook] | pos->pieces[Queen]));
            })

G(
    32, S(1) void flip_pos(Position *const restrict pos) {
      G(
          35, for (i32 i = 0; i < 2; i++) {
            swapbool(G(36, &pos->castling[i + 2]), G(36, &pos->castling[i]));
          })

      G(35, swapu64(G(37, &pos->colour[0]), G(37, &pos->colour[1]));)
      G(
          35, // Hack to flip the first 10 bitboards in Position.
              // Technically UB but works in GCC 14.2
          u64 *pos_ptr = (u64 *)pos;
          for (i32 i = 0; i < 10; i++) { pos_ptr[i] = flip_bb(pos_ptr[i]); })
      G(35, pos->flipped ^= 1;)
    })

G(
    32, [[nodiscard]] S(1)
            u64 get_mobility(H(38, 1, const i32 piece), H(38, 1, const i32 sq),
                             H(38, 1, const Position *pos)) {
              u64 moves = 0;
              const u64 bb = 1ULL << sq;
              G(39, if (piece == King) { moves = king(bb); })
              else G(39, if (piece == Knight) { moves = knight(bb); }) else {
                const u64 blockers = pos->colour[0] | pos->colour[1];
                G(
                    40, if (G(41, piece == Queen) || G(41, piece == Bishop)) {
                      moves |= bishop(H(18, 3, blockers), H(18, 3, bb));
                    })
                G(
                    40, if (G(42, piece == Queen) || G(42, piece == Rook)) {
                      moves |= rook(H(21, 3, blockers), H(21, 3, bb));
                    })
              }
              return moves;
            })

G(
    43, S(0) i32 makemove(H(44, 1, Position *const restrict pos),
                          H(44, 1, const Move *const restrict move)) {
      assert(move->from >= 0);
      assert(move->from < 64);
      assert(move->to >= 0);
      assert(move->to < 64);
      assert(move->from != move->to);
      assert(move->promo == None || move->promo == Knight ||
             move->promo == Bishop || move->promo == Rook ||
             move->promo == Queen);
      assert(move->takes_piece != King);
      assert(move->takes_piece == piece_on(H(31, 2, pos), H(31, 2, move->to)));

      G(45, const u64 from = 1ull << move->from;)
      G(45, const u64 to = 1ull << move->to;)
      G(46, const i32 piece = piece_on(H(31, 3, pos), H(31, 3, move->from));
        assert(piece != None);)
      G(46, const u64 mask = from | to;)

      G(
          47, // Castling
          if (piece == King) {
            const u64 bb = move->to - move->from == 2   ? 0xa0
                           : move->from - move->to == 2 ? 0x9
                                                        : 0;
            G(48, pos->colour[0] ^= bb;)
            G(48, pos->pieces[Rook] ^= bb;)
          })

      G(47, pos->colour[0] ^= mask;)

      // Move the piece
      G(47, pos->pieces[piece] ^= mask;)
      G(
          47, // Captures
          if (move->takes_piece != None) {
            G(49, pos->colour[1] ^= to;)
            G(49, pos->pieces[move->takes_piece] ^= to;)
          })

      // En passant
      if (G(50, piece == Pawn) && G(50, to == pos->ep)) {
        G(51, pos->colour[1] ^= to >> 8;)
        G(51, pos->pieces[Pawn] ^= to >> 8;)
      }
      pos->ep = 0;

      G(
          52, // Pawn double move
          if (G(53, move->to - move->from == 16) && G(53, piece == Pawn)) {
            pos->ep = to >> 8;
          })

      G(
          33, // Promotions
          if (move->promo != None) {
            G(54, pos->pieces[Pawn] ^= to;)
            G(54, pos->pieces[move->promo] ^= to;)
          })

      G(52, // Update castling permissions
        G(55, pos->castling[1] &= !(mask & 0x11ull);)
            G(55, pos->castling[3] &= !(mask & 0x1100000000000000ull);)
                G(55, pos->castling[2] &= !(mask & 0x9000000000000000ull);)
                    G(55, pos->castling[0] &= !(mask & 0x90ull);))

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
      return !is_attacked(H(33, 2, pos), H(33, 2, false),
                          H(33, 2, pos->colour[1] & pos->pieces[King]));
    })

G(
    43,
    S(1) Move *generate_piece_moves(H(56, 1, const u64 to_mask),
                                    H(56, 1, Move *restrict movelist),
                                    H(56, 1, const Position *restrict pos)) {
      for (i32 piece = Knight; piece <= King; piece++) {
        assert(piece == Knight || piece == Bishop || piece == Rook ||
               piece == Queen || piece == King);
        u64 copy = pos->colour[0] & pos->pieces[piece];
        while (copy) {
          const u8 from = lsb(copy);
          assert(from >= 0);
          assert(from < 64);
          G(57, copy &= copy - 1;)

          G(57, u64 moves = G(58, to_mask) &
                            G(58, get_mobility(H(38, 2, piece), H(38, 2, from),
                                               H(38, 2, pos)));)

          while (moves) {
            const u8 to = lsb(moves);
            assert(to >= 0);
            assert(to < 64);

            G(59, *movelist++ = ((Move){
                      .from = from,
                      .to = to,
                      .promo = None,
                      .takes_piece = piece_on(H(31, 4, pos), H(31, 4, to))});)
            G(59, moves &= moves - 1;)
          }
        }
      }

      return movelist;
    })

G(
    43, S(0) Move *generate_pawn_moves(H(60, 1, const Position *const pos),
                                       H(60, 1, Move *restrict movelist),
                                       H(60, 1, u64 to_mask),
                                       H(60, 1, const i32 offset)) {
      while (to_mask) {
        const u8 to = lsb(to_mask);
        to_mask &= to_mask - 1;
        const u8 from = to + offset;
        assert(from >= 0);
        assert(from < 64);
        assert(to >= 0);
        assert(to < 64);
        assert(piece_on(H(31, 5, pos), H(31, 5, from)) == Pawn);
        const u8 takes = piece_on(H(31, 6, pos), H(31, 6, to));
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

[[nodiscard]] S(1) i32 movegen(H(61, 1, const Position *const restrict pos),
                               H(61, 1, Move *restrict movelist),
                               H(61, 1, const i32 only_captures)) {

  G(62, const u64 all = pos->colour[0] | pos->colour[1];)
  G(62, const Move *start = movelist;)
  G(62, const u64 to_mask = only_captures ? pos->colour[1] : ~pos->colour[0];)
  if (!only_captures) {
    movelist = generate_pawn_moves(
        H(60, 2, pos), H(60, 2, movelist),
        H(60, 2,
          north(north(pos->colour[0] & pos->pieces[Pawn] & 0xFF00) & ~all) &
              ~all),
        H(60, 2, -16));
  }
  movelist = generate_pawn_moves(
      H(60, 3, pos), H(60, 3, movelist),
      H(60, 3,
        north(pos->colour[0] & pos->pieces[Pawn]) & ~all &
            (only_captures ? 0xFF00000000000000ull : ~0ull)),
      H(60, 3, -8));
  movelist = generate_pawn_moves(
      H(60, 4, pos), H(60, 4, movelist),
      H(60, 4,
        nw(pos->colour[0] & pos->pieces[Pawn]) & (pos->colour[1] | pos->ep)),
      H(60, 4, -7));
  movelist = generate_pawn_moves(
      H(60, 5, pos), H(60, 5, movelist),
      H(60, 5,
        ne(pos->colour[0] & pos->pieces[Pawn]) & (pos->colour[1] | pos->ep)),
      H(60, 5, -9));
  if (G(63, !only_captures) && G(63, pos->castling[0]) &&
      G(63, !(all & 0x60ull)) &&
      G(64, !is_attacked(H(33, 3, pos), H(33, 3, true), H(33, 3, 1ULL << 5))) &&
      G(64, !is_attacked(H(33, 4, pos), H(33, 4, true), H(33, 4, 1ULL << 4)))) {
    *movelist++ =
        (Move){.from = 4, .to = 6, .promo = None, .takes_piece = None};
  }
  if (G(65, !only_captures) && G(65, pos->castling[1]) &&
      G(65, !(all & 0xEull)) &&
      G(66, !is_attacked(H(33, 5, pos), H(33, 5, true), H(33, 5, 1ULL << 3))) &&
      G(66, !is_attacked(H(33, 6, pos), H(33, 6, true), H(33, 6, 1ULL << 4)))) {
    *movelist++ =
        (Move){.from = 4, .to = 2, .promo = None, .takes_piece = None};
  }
  movelist = generate_piece_moves(H(56, 2, to_mask), H(56, 2, movelist),
                                  H(56, 2, pos));

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
      movegen(H(61, 2, pos), H(61, 2, moves), H(61, 2, false));

  for (i32 i = 0; i < num_moves; ++i) {
    Position npos = *pos;

    // Check move legality
    if (!makemove(H(44, 2, &npos), H(44, 2, &moves[i]))) {
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
  i16 material[6];
  H(67, 1,
    H(68, 1, i8 passed_blocked_pawns[6];) H(68, 1, i8 tempo;)
        H(68, 1, i8 mobilities[5];) H(68, 1, i8 passed_pawns[6];)
            H(68, 1, i8 king_attacks[5];))
  H(67, 1,
    H(69, 1, i8 bishop_pair;) H(69, 1, i8 open_files[6];)
        H(69, 1, u8 pawn_attacked_penalty[2];) H(69, 1, i8 pst_file[64];)
            H(69, 1, i8 pst_rank[64];))
} EvalParams;

typedef struct [[nodiscard]] __attribute__((packed)) {
  i32 material[6];
  H(67, 2,
    H(68, 2, i32 passed_blocked_pawns[6];) H(68, 2, i32 tempo;)
        H(68, 2, i32 mobilities[5];) H(68, 2, i32 passed_pawns[6];)
            H(68, 2, i32 king_attacks[5];))
  H(67, 2,
    H(69, 2, i32 bishop_pair;) H(69, 2, i32 open_files[6];)
        H(69, 2, i32 pawn_attacked_penalty[2];) H(69, 2, i32 pst_file[64];)
            H(69, 2, i32 pst_rank[64];))

} EvalParamsMerged;

G(70, S(1) EvalParamsMerged eval_params;)

G(70, S(1) const EvalParams mg = ((EvalParams){
          .material = {71, 294, 298, 405, 899, 0},
          .pst_rank =
              {
                  0,   -10, -9,  -8,  2,  25, 92,  0,   // Pawn
                  -22, -11, 1,   14,  25, 43, 25,  -74, // Knight
                  -8,  6,   14,  13,  16, 16, -3,  -54, // Bishop
                  -1,  -13, -19, -21, 1,  21, 15,  16,  // Rook
                  16,  17,  9,   -2,  -6, -2, -24, -9,  // Queen
                  -9,  -6,  -31, -35, 0,  59, 71,  78,  // King
              },
          .pst_file =
              {
                  -19, -9,  -10, -1,  6,   21,  22, -10, // Pawn
                  -24, -10, 0,   14,  11,  11,  3,  -4,  // Knight
                  -10, 3,   5,   2,   5,   -3,  3,  -5,  // Bishop
                  -8,  -7,  2,   11,  12,  3,   -3, -10, // Rook
                  -10, -7,  -2,  2,   2,   1,   7,  7,   // Queen
                  -16, 24,  -3,  -49, -19, -37, 19, 0,   // King
              },
          .mobilities = {7, 6, 2, 3, -9},
          .king_attacks = {0, 15, 19, 14, 0},
          .open_files = {24, -10, -10, 21, -3, -31},
          .passed_pawns = {-17, -20, -12, 10, 34, 92},
          .passed_blocked_pawns = {5, -2, 3, 11, 11, -30},
          .bishop_pair = 25,
          .pawn_attacked_penalty = {-16, -128},
          .tempo = 17});)

G(
    70, [[nodiscard]] S(1) i32 combine_eval_param(H(71, 1, const i32 mg_val),
                                                  H(71, 1, const i32 eg_val)) {
      return G(72, mg_val) + G(72, (eg_val << 16));
    })

G(70, S(1) const EvalParams eg = ((EvalParams){
          .material = {71, 302, 297, 542, 991, 0},
          .pst_rank =
              {
                  0,   -4,  -6,  -5, 0,  15, 77, 0,   // Pawn
                  -33, -18, -4,  18, 23, 9,  1,  3,   // Knight
                  -11, -10, -1,  2,  5,  3,  3,  9,   // Bishop
                  -18, -19, -12, 5,  12, 10, 15, 7,   // Rook
                  -60, -45, -20, 10, 29, 30, 39, 19,  // Queen
                  -39, -1,  9,   19, 24, 21, 8,  -47, // King
              },
          .pst_file =
              {
                  9,   12, -1, -9, -4, -3, 2,  -6,  // Pawn
                  -18, -3, 8,  14, 14, 5,  -1, -19, // Knight
                  -6,  -1, 0,  3,  5,  4,  1,  -6,  // Bishop
                  1,   4,  4,  -1, -5, 0,  0,  -3,  // Rook
                  -19, -5, 3,  7,  12, 10, -1, -7,  // Queen
                  -25, 1,  13, 25, 19, 21, 1,  -32, // King
              },
          .mobilities = {2, 4, 3, 1, -4},
          .king_attacks = {0, -3, -6, 7, 0},
          .open_files = {28, -4, 6, 9, 27, 8},
          .passed_pawns = {0, 4, 25, 46, 84, 77},
          .passed_blocked_pawns = {-13, -13, -33, -57, -95, -102},
          .bishop_pair = 53,
          .pawn_attacked_penalty = {-10, -128},
          .tempo = 7});)

S(1) void init() {
  // INIT DIAGONAL MASKS
  G(
      44, for (i32 sq = 0; sq < 64; sq++) {
        const u64 bb = 1ULL << sq;
        diag_mask[sq] = G(73, ray(H(15, 4, 0), H(15, 4, ~0x8080808080808080ull),
                                  H(15, 4, bb), H(15, 4, -9))) | // Northeast
                        G(73, ray(H(15, 5, 0), H(15, 5, ~0x101010101010101ull),
                                  H(15, 5, bb), H(15, 5, 9))); // Southwest
      })

  G(
      44, // MERGE MATERIAL VALUES
      for (i32 i = 0; i < sizeof(mg.material) / sizeof(i16); i++) {
        eval_params.material[i] = combine_eval_param(H(71, 2, mg.material[i]),
                                                     H(71, 2, eg.material[i]));
      })
  G(
      44, // MERGE NON-MATERIAL VALUES
      for (i32 i = 0; i < sizeof(mg) - sizeof(mg.material); i++) {
        // Technically writes past end of array
        // But since the structs are packed, it works
        const i32 offset = sizeof(mg.material);
        ((i32 *)&eval_params)[offset / sizeof(*mg.material) + i] =
            combine_eval_param(H(71, 3, ((i8 *)&mg)[offset + i]),
                               H(71, 3, ((i8 *)&eg)[offset + i]));
      })
}

G(74, __attribute__((aligned(8))) S(1)
          const i16 max_material[] = {0, 71, 302, 298, 542, 991};)
G(74,
  __attribute__((aligned(8))) S(1) const i8 phases[] = {0, 0, 1, 1, 2, 4, 0};)

S(1) i32 eval(Position *const restrict pos) {
  G(75, i32 score = eval_params.tempo;)
  G(75, i32 phase = 0;)

  for (i32 c = 0; c < 2; c++) {

    G(76, const u64 opp_king_zone = king(pos->colour[1] & pos->pieces[King]);)

    G(76,
      const u64 own_pawns = G(77, pos->pieces[Pawn]) & G(77, pos->colour[0]);)
    G(76,
      const u64 opp_pawns = G(78, pos->colour[1]) & G(78, pos->pieces[Pawn]);
      const u64 attacked_by_pawns = G(79, se(opp_pawns)) | G(79, sw(opp_pawns));
      const u64 no_passers = G(80, opp_pawns) | G(80, attacked_by_pawns);)
    G(
        76, // BISHOP PAIR
        if (count(G(81, pos->pieces[Bishop]) & G(81, pos->colour[0])) > 1) {
          score += eval_params.bishop_pair;
        })

    for (i32 p = Pawn; p <= King; p++) {
      u64 copy = G(82, pos->colour[0]) & G(82, pos->pieces[p]);
      while (copy) {
        const i32 sq = lsb(copy);
        G(83, const int file = sq & 7;)
        G(83, const int rank = sq >> 3;)
        G(83, phase += phases[p];)
        G(83, copy &= copy - 1;)

        G(
            60, // OPEN FILES / DOUBLED PAWNS
            if ((G(84, north(0x101010101010101ULL << sq)) & G(84, own_pawns)) ==
                0) { score += eval_params.open_files[p - 1]; })

        G(60, // SPLIT PIECE-SQUARE TABLES FOR FILE
          score += eval_params.pst_file[(p - 1) * 8 + file];)
        G(60, // SPLIT PIECE-SQUARE TABLES FOR RANK
          score += eval_params.pst_rank[(p - 1) * 8 + rank];)

        G(60, // MATERIAL
          score += eval_params.material[p - 1];)

        G(
            60, if (p > Pawn) {
              G(
                  85, // PIECES ATTACKED BY PAWNS
                  if (1ULL << sq & no_passers) {
                    score += eval_params.pawn_attacked_penalty[c];
                  })

              G(85, const u64 mobility =
                        get_mobility(H(38, 3, p), H(38, 3, sq), H(38, 3, pos));

                G(86, // MOBILITY
                  score +=
                  G(87, count(G(88, mobility) & G(88, ~attacked_by_pawns) &
                              G(88, ~pos->colour[0]))) *
                  G(87, eval_params.mobilities[p - 2]);)

                    G(86, // KING ATTACKS
                      score +=
                      G(89, count(G(90, mobility) & G(90, opp_king_zone))) *
                      G(89, eval_params.king_attacks[p - 2]);))
            })

        G(
            60, // PASSED PAWNS
            if (G(91,
                  !(G(92, (0x101010101010101ULL << sq)) & G(92, no_passers))) &&
                G(91, p == Pawn)) {
              G(
                  93, if (north(1ULL << sq) & pos->colour[1]) {
                    score += eval_params.passed_blocked_pawns[rank - 1];
                  })

              G(93, score += eval_params.passed_pawns[rank - 1];)
            })
      }
    }

    G(56, score = -score;)
    G(56, flip_pos(pos);)
  }

  return ((short)score * phase + ((score + 0x8000) >> 16) * (24 - phase)) / 24;
}

typedef struct [[nodiscard]] {
  G(68, Move best_move;)
  G(68, i32 static_eval;)
  G(68, i32 num_moves;)
  G(68, Move killer;)
  G(68, Move moves[max_moves];)
  G(68, u64 position_hash;)
} SearchStack;

typedef struct [[nodiscard]] __attribute__((packed)) {
  G(94, i16 score;)
  G(94, Move move;)
  G(94, u16 partial_hash;)
  G(94, u8 flag;)
  G(94, i8 depth;)
} TTEntry;
_Static_assert(sizeof(TTEntry) == 10);

enum { tt_length = 64 * 1024 * 1024 / sizeof(TTEntry) };
enum { Upper = 0, Lower = 1, Exact = 2 };
enum { max_ply = 96 };
enum { mate = 30000, inf = 32000 };

G(95, S(1) i32 move_history[2][6][64][64];)
G(95, S(1) TTEntry tt[tt_length];)
G(95, S(0) size_t start_time;)
G(95, S(1) size_t max_time;)

#if defined(__x86_64__) || defined(_M_X64)
typedef long long __attribute__((__vector_size__(16))) i128;

[[nodiscard]] __attribute__((target("aes"))) S(1) u64
    get_hash(const Position *const pos) {
  i128 hash = {0};

  // USE 16 BYTE POSITION SEGMENTS AS KEYS FOR AES
  const u8 *const data = (const u8 *)pos;
  for (i32 i = 0; i < 6; i++) {
    i128 key;
    __builtin_memcpy(&key, data + i * 16, 16);
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
i16 search(H(96, 1, Position *const restrict pos), H(96, 1, i32 alpha),
           H(96, 1, const i32 ply), H(96, 1, i32 depth),
           H(97, 1, SearchStack *restrict stack),
#ifdef FULL
           u64 *nodes,
#endif
           H(97, 1, const bool do_null), H(97, 1, const i32 pos_history_count),
           H(97, 1, const i32 beta)) {
  assert(alpha < beta);
  assert(ply >= 0);

  const bool in_check =
      is_attacked(H(33, 7, pos), H(33, 7, true),
                  H(33, 7, pos->colour[0] & pos->pieces[King]));

  // IN-CHECK EXTENSION
  if (in_check) {
    depth++;
  }

  // EARLY EXITS
  if (depth > 4 && get_time() - start_time > max_time) {
    return alpha;
  }

  const u64 tt_hash = get_hash(pos);

  // FULL REPETITION DETECTION
  bool in_qsearch = depth <= 0;
  for (i32 i = G(98, ply) + G(98, pos_history_count);
       G(99, i > 0) && G(99, do_null); i -= 2) {
    if (tt_hash == stack[i].position_hash) {
      return 0;
    }
  }

  // TT PROBING
  G(100, stack[ply].best_move = (Move){0};)
  G(100, TTEntry *tt_entry = &tt[tt_hash % tt_length];)
  G(100, const u16 tt_hash_partial = tt_hash / tt_length;)
  if (tt_entry->partial_hash == tt_hash_partial) {
    stack[ply].best_move = tt_entry->move;

    // TT PRUNING
    if (G(101, alpha == beta - 1) &&
        G(101, tt_entry->flag != tt_entry->score <= alpha) &&
        G(101, tt_entry->depth >= depth)) {
      return tt_entry->score;
    }
  } else if (depth > 3) {

    // INTERNAL ITERATIVE REDUCTION
    depth--;
  }

  // STATIC EVAL WITH ADJUSTMENT FROM TT
  i32 static_eval = eval(pos);
  stack[ply].static_eval = static_eval;
  const bool improving = ply > 1 && static_eval > stack[ply - 2].static_eval;
  if (G(102, tt_entry->partial_hash == tt_hash_partial) &&
      G(102, tt_entry->flag != static_eval > tt_entry->score)) {
    static_eval = tt_entry->score;
  }

  // QUIESCENCE
  if (G(103, static_eval > alpha) && G(103, in_qsearch)) {
    if (static_eval >= beta) {
      return static_eval;
    }
    alpha = static_eval;
  }

  if (G(104, !in_check) && G(104, alpha == beta - 1)) {
    if (G(105, depth < 8) && G(105, !in_qsearch)) {

      G(106, {
        // REVERSE FUTILITY PRUNING
        if (static_eval - 52 * depth >= beta) {
          return static_eval;
        }
      })

      G(106, // RAZORING
        in_qsearch = static_eval + 123 * depth <= alpha;)
    }

    // NULL MOVE PRUNING
    if (G(107, depth > 2) && G(107, static_eval >= beta) && G(107, do_null)) {
      Position npos = *pos;
      flip_pos(&npos);
      npos.ep = 0;
      const i32 score = -search(
          H(96, 2, &npos), H(96, 2, -beta), H(96, 2, ply + 1),
          H(96, 2, depth - 3 - depth / 4), H(97, 2, stack),
#ifdef FULL
          nodes,
#endif
          H(97, 2, false), H(97, 2, pos_history_count), H(97, 2, -alpha));
      if (score >= beta) {
        return score;
      }
    }
  }

  G(96, i32 moves_evaluated = 0;)
  G(96, i32 quiets_evaluated = 0;)
  G(96, stack[pos_history_count + ply + 2].position_hash = tt_hash;)
  G(96, i32 best_score = in_qsearch ? static_eval : -inf;)
  G(96, stack[ply].num_moves = movegen(
            H(61, 3, pos), H(61, 3, stack[ply].moves), H(61, 3, in_qsearch));)
  G(96, u8 tt_flag = Upper;)

  for (i32 move_index = 0; move_index < stack[ply].num_moves; move_index++) {
    // MOVE ORDERING
    G(108, i32 move_score = ~0x1010101LL;)
    G(108, i32 best_index = 0;)
    for (i32 order_index = move_index; order_index < stack[ply].num_moves;
         order_index++) {
      assert(
          stack[ply].moves[order_index].takes_piece ==
          piece_on(H(31, 7, pos), H(31, 7, stack[ply].moves[order_index].to)));
      const i32 order_move_score =
          G(97, // KILLER MOVE
            move_equal(G(109, &stack[ply].moves[order_index]),
                       G(109, &stack[ply].killer)) *
                861) +
          G(97, // PREVIOUS BEST MOVE FIRST
            (move_equal(G(110, &stack[ply].best_move),
                        G(110, &stack[ply].moves[order_index]))
             << 30)) +
          G(97, // MOST VALUABLE VICTIM
            stack[ply].moves[order_index].takes_piece * 737) +
          G(97, // HISTORY HEURISTIC
            move_history[pos->flipped]
                        [stack[ply].moves[order_index].takes_piece]
                        [stack[ply].moves[order_index].from]
                        [stack[ply].moves[order_index].to]);
      if (order_move_score > move_score) {
        G(111, best_index = order_index;)
        G(111, move_score = order_move_score;)
      }
    }

    swapmoves(G(112, &stack[ply].moves[best_index]),
              G(112, &stack[ply].moves[move_index]));

    // FORWARD FUTILITY PRUNING / DELTA PRUNING
    if (G(113, depth < 8) &&
        G(113,
          G(114, static_eval + 136 * depth) +
                  G(114,
                    max_material[stack[ply].moves[move_index].takes_piece]) +
                  G(114, max_material[stack[ply].moves[move_index].promo]) <
              alpha) &&
        G(113, moves_evaluated) && G(113, !in_check)) {
      break;
    }

    Position npos = *pos;
#ifdef FULL
    (*nodes)++;
#endif
    if (!makemove(H(44, 3, &npos), H(44, 3, &stack[ply].moves[move_index]))) {
      continue;
    }

    // PRINCIPAL VARIATION SEARCH
    i32 low = moves_evaluated == 0 ? -beta : -alpha - 1;
    moves_evaluated++;

    // LATE MOVE REDUCTION
    i32 reduction = G(115, depth > 1) && G(115, moves_evaluated > 6)
                        ? G(116, (alpha == beta - 1)) + G(116, 1) +
                              G(116, moves_evaluated / 11) + G(116, !improving)
                        : 1;

    i32 score;
    while (true) {
      score = -search(H(96, 3, &npos), H(96, 3, low), H(96, 3, ply + 1),
                      H(96, 3, depth - reduction), H(97, 3, stack),
#ifdef FULL
                      nodes,
#endif
                      H(97, 3, true), H(97, 3, pos_history_count),
                      H(97, 3, -alpha));

      if (score > alpha) {
        if (reduction != 1) {
          reduction = 1;
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
        G(117, tt_flag = Exact;)
        G(117, stack[ply].best_move = stack[ply].moves[move_index];)
        G(117, alpha = score;)
        if (score >= beta) {
          assert(stack[ply].best_move.takes_piece ==
                 piece_on(H(31, 8, pos), H(31, 8, stack[ply].best_move.to)));
          G(118, tt_flag = Lower;)
          G(
              118, if (stack[ply].best_move.takes_piece == None) {
                stack[ply].killer = stack[ply].best_move;
              })
          G(118, const i32 bonus = depth * depth;
            G(119,
              i32 *const this_hist =
                  &move_history[pos->flipped][stack[ply].best_move.takes_piece]
                               [stack[ply].best_move.from]
                               [stack[ply].best_move.to];

              *this_hist += bonus - bonus * *this_hist / 1024;)
                G(
                    119, for (i32 prev_index = 0; prev_index < move_index;
                              prev_index++) {
                      const Move prev = stack[ply].moves[prev_index];
                      i32 *const prev_hist =
                          &move_history[pos->flipped][prev.takes_piece]
                                       [prev.from][prev.to];
                      *prev_hist -= bonus + bonus * *prev_hist / 1024;
                    }))
          break;
        }
      }
    }

    if (stack[ply].moves[move_index].takes_piece == None) {
      quiets_evaluated++;
    }

    // LATE MOVE PRUNING
    if (G(120, !in_check) &&
        G(120, quiets_evaluated > 1 + depth * depth >> !improving) &&
        G(120, alpha == beta - 1)) {
      break;
    }
  }

  // MATE / STALEMATE DETECTION
  if (best_score == -inf) {
    return (ply - mate) * in_check;
  }

  *tt_entry = (TTEntry){.partial_hash = tt_hash_partial,
                        .move = stack[ply].best_move,
                        .score = best_score,
                        .depth = depth,
                        .flag = tt_flag};

  return best_score;
}

S(1)
void iteratively_deepen(
#ifdef FULL
    i32 maxdepth, u64 *nodes,
#endif
    H(121, 1, Position *const restrict pos),
    H(121, 1, SearchStack *restrict stack),
    H(121, 1, const i32 pos_history_count)) {
  start_time = get_time();
#ifdef FULL
  for (i32 depth = 1; depth < maxdepth; depth++) {
#else
  for (i32 depth = 1; depth < max_ply; depth++) {
#endif
    i32 score =
        search(H(96, 4, pos), H(96, 4, -inf), H(96, 4, 0), H(96, 4, depth),
               H(97, 4, stack),
#ifdef FULL
               nodes,
#endif
               H(97, 4, false), H(97, 4, pos_history_count), H(97, 4, inf));
    size_t elapsed = get_time() - start_time;

#ifdef FULL
    printf("info depth %i score cp %i time %i nodes %i", depth, score, elapsed,
           *nodes);
    if (elapsed > 0) {
      const u64 nps = *nodes * 1000 / elapsed;
      printf(" nps %i", nps);
    }

    putl(" pv ");
    char move_name[8];
    move_str(H(28, 2, move_name), H(28, 2, &stack[0].best_move),
             H(28, 2, pos->flipped));
    puts(move_name);
#endif

    if (elapsed > max_time / 16) {
      break;
    }
  }
  char move_name[8];
  move_str(H(28, 3, move_name), H(28, 3, &stack[0].best_move),
           H(28, 3, pos->flipped));
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
      i32 piece = piece_on(H(31, 9, &npos), H(31, 9, sq));
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
  iteratively_deepen(20, &nodes, H(121, 2, &pos), H(121, 2, stack),
                     H(121, 2, pos_history_count));
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

  G(122, char line[4096];)
  G(122, Position pos;)
  G(122, i32 pos_history_count;)
  G(122, // #ifdef LOWSTACK
         //  SearchStack *stack = malloc(sizeof(SearchStack) * 1024);
         // #else
    SearchStack stack[1024];
    // #endif
  )
  G(122, __builtin_memset(move_history, 0, sizeof(move_history));)
  G(122, init();)

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
      iteratively_deepen(max_ply, &nodes, H(121, 3, &pos), H(121, 3, stack),
                         H(121, 3, pos_history_count));
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
    G(123, if (line[0] == 'q') { exit_now(); })
    else G(
        123,
        if (line[0] == 'i') { puts("readyok"); }) else if (line[0] == 'p') {
      G(124, pos_history_count = 0;)
      G(124, pos = start_pos;)
      while (true) {
        const bool line_continue = getl(line);

#if FULL
        if (!strcmp(line, "fen")) {
          getl(line);
          get_fen(&pos, line);
        }
#endif

        const i32 num_moves =
            movegen(H(61, 4, &pos), H(61, 4, stack[0].moves), H(61, 4, false));
        for (i32 i = 0; i < num_moves; i++) {
          char move_name[8];
          move_str(H(28, 4, move_name), H(28, 4, &stack[0].moves[i]),
                   H(28, 4, pos.flipped));
          assert(move_string_equal(line, move_name) ==
                 !strcmp(line, move_name));
          if (move_string_equal(G(125, line), G(125, move_name))) {
            stack[pos_history_count].position_hash = get_hash(&pos);
            pos_history_count++;
            if (stack[0].moves[i].takes_piece != None) {
              pos_history_count = 0;
            }
            makemove(H(44, 4, &pos), H(44, 4, &stack[0].moves[i]));
            break;
          }
        }
        if (!line_continue) {
          break;
        }
      }
    }
    else if (line[0] == 'g') {
#ifdef FULL
      while (true) {
        getl(line);
        if (!pos.flipped && !strcmp(line, "wtime")) {
          getl(line);
          max_time = atoi(line) / 2;
          break;
        } else if (pos.flipped && !strcmp(line, "btime")) {
          getl(line);
          max_time = atoi(line) / 2;
          break;
        } else if (!strcmp(line, "movetime")) {
          max_time = 20000; // Assume Lichess bot
          break;
        }
      }
      iteratively_deepen(max_ply, &nodes, H(121, 4, &pos), H(121, 4, stack),
                         H(121, 4, pos_history_count));
#else
      for (i32 i = 0; i < (pos.flipped ? 4 : 2); i++) {
        getl(line);
        max_time = atoi(line) / 2;
      }
      iteratively_deepen(H(121, 5, &pos), H(121, 5, stack),
                         H(121, 5, pos_history_count));
#endif
    }
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
