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
    1, S(1) ssize_t _sys(H(2, 1, ssize_t arg1), H(2, 1, ssize_t call),
                         H(2, 1, ssize_t arg2), H(2, 1, ssize_t arg3)) {
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
    3,
    S(1) void putl(const char *const restrict string) {
      i32 length = 0;
      while (string[length]) {
        _sys(H(2, 2, stdout), H(2, 2, 1), H(2, 2, (ssize_t)(&string[length])),
             H(2, 2, 1));
        length++;
      }
    }

    S(1) void puts(const char *const restrict string) {
      putl(string);
      putl("\n");
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
    3, // Non-standard, gets but a word instead of a line
    S(1) bool getl(char *restrict string) {
      while (true) {
        const int result = _sys(H(2, 3, stdin), H(2, 3, 0),
                                H(2, 3, (ssize_t)string), H(2, 3, 1));

    // Assume stdin never closes on mini build
#ifdef FULL
        if (result < 1) {
          exit_now();
        }
#endif

        const char ch = *string;
        if (G(4, ch == ' ') || G(4, ch == '\n')) {
          *string = 0;
          return ch != '\n';
        }

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
  _sys(H(2, 5, 1), H(2, 5, 228), H(2, 5, (ssize_t)&ts), H(2, 5, 0));
  return ts.tv_sec * 1000 + ts.tv_nsec / 1000000;
}

#ifdef ASSERTS
#define assert(condition)                                                      \
  if (!(condition)) {                                                          \
    printf("Assert failed on line %i: ", __LINE__);                            \
    puts(#condition);                                                          \
    _sys(H(2, 6, 1), H(2, 6, 60), H(2, 6, 0), H(2, 6, 0));                     \
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
  G(5, u8 from; u8 to;)
  G(5, u8 takes_piece;)
  G(5, u8 promo;)
} Move;

typedef struct [[nodiscard]] {
  G(6, u64 ep;)
  G(6, u64 pieces[7];)
  G(6, u64 colour[2];)
  G(7, bool castling[4];)
  G(7, bool flipped;)
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
            u64 shift(H(11, 1, const u64 bb), H(11, 1, const i32 shift),
                      H(11, 1, const u64 mask)) {
              return shift > 0 ? bb << shift & mask : bb >> -shift & mask;
            })

G(
    10, [[nodiscard]] S(1)
            i32 count(const u64 bb) { return __builtin_popcountll(bb); })

G(
    12, [[nodiscard]] S(1)
            u64 east(const u64 bb) { return bb << 1 & ~0x101010101010101ull; })

G(12, [[nodiscard]] S(1) u64 south(const u64 bb) { return bb >> 8; })

G(12, [[nodiscard]] S(1) u64 north(const u64 bb) { return bb << 8; })

G(
    12, [[nodiscard]] S(1)
            u64 west(const u64 bb) { return bb >> 1 & ~0x8080808080808080ull; })

G(13, [[nodiscard]] S(1) u64 se(const u64 bb) { return east(south(bb)); })

G(13, [[nodiscard]] S(1) u64 sw(const u64 bb) { return west(south(bb)); })

G(
    13, [[nodiscard]] S(1) u64 nw(const u64 bb) {
      return shift(H(11, 2, bb), H(11, 2, 7), H(11, 2, ~0x8080808080808080ull));
      // return west(north(bb));
    })

G(13, [[nodiscard]] S(1) u64 ne(const u64 bb) { return east(north(bb)); })

G(
    14,
    [[nodiscard]] S(1)
        u64 ray(H(15, 1, const u64 bb), H(15, 1, const u64 mask),
                H(15, 1, const u64 blockers), H(15, 1, const i32 shift_by)) {
          u64 result = shift(H(11, 3, bb), H(11, 3, shift_by), H(11, 3, mask));
          for (i32 i = 0; i < 6; i++) {
            result |= shift(H(11, 4, result & ~blockers), H(11, 4, shift_by),
                            H(11, 4, mask));
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

G(14, S(0) u64 diag_mask[64];)

G(
    17, [[nodiscard]] S(1) u64 king(const u64 bb) {
      return G(18, bb << 8) |
             G(18, (bb << 1 | bb << 9 | bb >> 7) & ~0x101010101010101ull) |
             G(18, bb >> 8) |
             G(18, (bb >> 1 | bb >> 9 | bb << 7) & ~0x8080808080808080ull);
    })

G(
    17, [[nodiscard]] S(1)
            u64 rook(H(19, 1, const u64 blockers), H(19, 1, const u64 bb)) {
              assert(count(bb) == 1);
              return xattack(H(16, 2, bb ^ 0x101010101010101ULL << lsb(bb) % 8),
                             H(16, 2, bb), H(16, 2, blockers)) |
                     ray(H(15, 2, bb), H(15, 2, ~0x101010101010101ull),
                         H(15, 2, blockers), H(15, 2, 1)) // East
                     | ray(H(15, 3, bb), H(15, 3, ~0x8080808080808080ull),
                           H(15, 3, blockers), H(15, 3, -1)); // West
            })

G(
    17, [[nodiscard]] S(1) u64 knight(const u64 bb) {
      return G(20, (bb << 17 | bb >> 15) & ~0x101010101010101ull) |
             G(20, (bb << 15 | bb >> 17) & ~0x8080808080808080ull) |
             G(20, (bb << 10 | bb >> 6) & 0xFCFCFCFCFCFCFCFCull) |
             G(20, (bb << 6 | bb >> 10) & 0x3F3F3F3F3F3F3F3Full);
    })

G(
    17, [[nodiscard]] S(0)
            u64 bishop(H(21, 1, const u64 blockers), H(21, 1, const u64 bb)) {
              assert(count(bb) == 1);
              const i32 sq = lsb(bb);
              return xattack(H(16, 3, diag_mask[sq]), H(16, 3, bb),
                             H(16, 3, blockers)) |
                     xattack(H(16, 4, flip_bb(diag_mask[sq ^ 56])),
                             H(16, 4, bb), H(16, 4, blockers));
            })

G(
    22, S(1) void swapbool(G(23, bool *const restrict rhs),
                           G(23, bool *const restrict lhs)) {
      const bool temp = *lhs;
      *lhs = *rhs;
      *rhs = temp;
    })

G(
    22, [[nodiscard]] S(0)
            i32 piece_on(H(24, 1, const Position *const restrict pos),
                         H(24, 1, const i32 sq)) {
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
    22,
    S(1) void swapu32(G(25, u32 *const lhs), G(25, u32 *const rhs)) {
      const u32 temp = *lhs;
      *lhs = *rhs;
      *rhs = temp;
    }

    S(1) void swapmoves(G(26, Move *const lhs), G(26, Move *const rhs)) {
      swapu32(G(27, (u32 *)lhs), G(27, (u32 *)rhs));
    })

G(
    22, S(1) void move_str(H(23, 1, char *restrict str),
                           H(23, 1, const Move *restrict move),
                           H(23, 1, const i32 flip)) {
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
        G(28, str[i * 2] = 'a' + (&move->from)[i] % 8;)
        G(28, str[i * 2 + 1] = '1' + ((&move->from)[i] / 8 ^ 7 * flip);)
      }

      G(29, str[4] = "\0\0nbrq"[move->promo];)
      G(29, str[5] = '\0';)
    })

G(
    22, S(1) void swapu64(G(30, u64 *const rhs), G(30, u64 *const lhs)) {
      const u64 temp = *lhs;
      *lhs = *rhs;
      *rhs = temp;
    })

G(
    22, [[nodiscard]] S(1) bool move_equal(G(31, Move *const lhs),
                                           G(31, Move *const rhs)) {
      return *(u32 *)lhs == *(u32 *)rhs;
    })

G(
    32, [[nodiscard]] S(1) u64 get_mobility(H(33, 1, const Position *pos),
                                            H(33, 1, const i32 sq),
                                            H(33, 1, const i32 piece)) {
      u64 moves = 0;
      const u64 bb = 1ULL << sq;
      G(34, if (piece == King) { moves = king(bb); })
      else G(34, if (piece == Knight) { moves = knight(bb); }) else {
        const u64 blockers = pos->colour[0] | pos->colour[1];
        G(
            35, if (G(36, piece == Rook) || G(36, piece == Queen)) {
              moves |= rook(H(19, 2, blockers), H(19, 2, bb));
            })
        G(
            35, if (G(37, piece == Bishop) || G(37, piece == Queen)) {
              moves |= bishop(H(21, 2, blockers), H(21, 2, bb));
            })
      }
      return moves;
    })

G(
    32, S(0) void flip_pos(Position *const restrict pos) {
      G(
          38, for (i32 i = 0; i < 2; i++) {
            swapbool(G(39, &pos->castling[i + 2]), G(39, &pos->castling[i]));
          })

      G(38, swapu64(G(40, &pos->colour[1]), G(40, &pos->colour[0]));)
      G(
          38, // Hack to flip the first 10 bitboards in Position.
              // Technically UB but works in GCC 14.2
          u64 *pos_ptr = (u64 *)pos;
          for (i32 i = 0; i < 10; i++) { pos_ptr[i] = flip_bb(pos_ptr[i]); })
      G(38, pos->flipped ^= 1;)
    })

G(
    32, [[nodiscard]] S(1)
            i32 is_attacked(H(41, 1, const Position *const restrict pos),
                            H(41, 1, const u64 bb), H(41, 1, const i32 them)) {
              assert(count(bb) == 1);
              const u64 theirs = pos->colour[them];
              const u64 pawns = theirs & pos->pieces[Pawn];
              if ((them ? sw(pawns) | se(pawns) : nw(pawns) | ne(pawns)) & bb) {
                return true;
              }
              const u64 blockers = pos->colour[0] | pos->colour[1];
              return G(42, bishop(H(21, 3, blockers), H(21, 3, bb)) & theirs &
                               (pos->pieces[Bishop] | pos->pieces[Queen])) ||
                     G(42, knight(bb) & theirs & pos->pieces[Knight]) ||
                     G(42, king(bb) & theirs & pos->pieces[King]) ||
                     G(42, rook(H(19, 3, blockers), H(19, 3, bb)) & theirs &
                               (pos->pieces[Rook] | pos->pieces[Queen]));
            })

G(
    43, S(1) Move *generate_pawn_moves(H(44, 1, const Position *const pos),
                                       H(44, 1, Move *restrict movelist),
                                       H(44, 1, u64 to_mask),
                                       H(44, 1, const i32 offset)) {
      while (to_mask) {
        const u8 to = lsb(to_mask);
        to_mask &= to_mask - 1;
        const u8 from = to + offset;
        assert(from >= 0);
        assert(from < 64);
        assert(to >= 0);
        assert(to < 64);
        assert(piece_on(H(24, 2, pos), H(24, 2, from)) == Pawn);
        const u8 takes = piece_on(H(24, 3, pos), H(24, 3, to));
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

G(
    43, S(1) Move *generate_piece_moves(H(45, 1, const Position *restrict pos),
                                        H(45, 1, Move *restrict movelist),
                                        H(45, 1, const u64 to_mask)) {
      for (i32 piece = Knight; piece <= King; piece++) {
        assert(piece == Knight || piece == Bishop || piece == Rook ||
               piece == Queen || piece == King);
        u64 copy = pos->colour[0] & pos->pieces[piece];
        while (copy) {
          const u8 from = lsb(copy);
          assert(from >= 0);
          assert(from < 64);
          G(46, copy &= copy - 1;)

          G(46, u64 moves = G(47, get_mobility(H(33, 2, pos), H(33, 2, from),
                                               H(33, 2, piece))) &
                            G(47, to_mask);)

          while (moves) {
            const u8 to = lsb(moves);
            assert(to >= 0);
            assert(to < 64);

            G(48, *movelist++ = ((Move){
                      .from = from,
                      .to = to,
                      .promo = None,
                      .takes_piece = piece_on(H(24, 4, pos), H(24, 4, to))});)
            G(48, moves &= moves - 1;)
          }
        }
      }

      return movelist;
    })

G(
    43, S(0) i32 makemove(H(49, 1, Position *const restrict pos),
                          H(49, 1, const Move *const restrict move)) {
      assert(move->from >= 0);
      assert(move->from < 64);
      assert(move->to >= 0);
      assert(move->to < 64);
      assert(move->from != move->to);
      assert(move->promo == None || move->promo == Knight ||
             move->promo == Bishop || move->promo == Rook ||
             move->promo == Queen);
      assert(move->takes_piece != King);
      assert(move->takes_piece == piece_on(H(24, 5, pos), H(24, 5, move->to)));

      G(50, const u64 from = 1ull << move->from;)
      G(50, const u64 to = 1ull << move->to;)
      G(51, const u64 mask = from | to;)
      G(51, const i32 piece = piece_on(H(24, 6, pos), H(24, 6, move->from));
        assert(piece != None);)

      G(52, pos->pieces[piece] ^= mask;)

      G(
          52, // Captures
          if (move->takes_piece != None) {
            G(53, pos->pieces[move->takes_piece] ^= to;)
            G(53, pos->colour[1] ^= to;)
          })

      // Move the piece
      G(
          52, // Castling
          if (piece == King) {
            const u64 bb = move->to - move->from == 2   ? 0xa0
                           : move->from - move->to == 2 ? 0x9
                                                        : 0;
            G(54, pos->pieces[Rook] ^= bb;)
            G(54, pos->colour[0] ^= bb;)
          })
      G(52, pos->colour[0] ^= mask;)

      // En passant
      if (G(55, piece == Pawn) && G(55, to == pos->ep)) {
        G(56, pos->colour[1] ^= to >> 8;)
        G(56, pos->pieces[Pawn] ^= to >> 8;)
      }
      pos->ep = 0;

      G(
          57, // Pawn double move
          if (G(58, move->to - move->from == 16) && G(58, piece == Pawn)) {
            pos->ep = to >> 8;
          })

      G(
          41, // Promotions
          if (move->promo != None) {
            G(59, pos->pieces[move->promo] ^= to;)
            G(59, pos->pieces[Pawn] ^= to;)
          })

      G(57, // Update castling permissions
        G(60, pos->castling[0] &= !(mask & 0x90ull);)
            G(60, pos->castling[3] &= !(mask & 0x1100000000000000ull);)
                G(60, pos->castling[2] &= !(mask & 0x9000000000000000ull);)
                    G(60, pos->castling[1] &= !(mask & 0x11ull);))

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
      return !is_attacked(H(41, 2, pos),
                          H(41, 2, pos->colour[1] & pos->pieces[King]),
                          H(41, 2, false));
    })

enum { max_moves = 218 };

[[nodiscard]] S(1) i32 movegen(H(61, 1, const Position *const restrict pos),
                               H(61, 1, Move *restrict movelist),
                               H(61, 1, const i32 only_captures)) {

  G(62, const u64 to_mask = only_captures ? pos->colour[1] : ~pos->colour[0];)
  G(62, const Move *start = movelist;)
  G(62, const u64 all = pos->colour[0] | pos->colour[1];)
  if (!only_captures) {
    movelist = generate_pawn_moves(
        H(44, 2, pos), H(44, 2, movelist),
        H(44, 2,
          north(north(pos->colour[0] & pos->pieces[Pawn] & 0xFF00) & ~all) &
              ~all),
        H(44, 2, -16));
  }
  movelist = generate_pawn_moves(
      H(44, 3, pos), H(44, 3, movelist),
      H(44, 3,
        north(pos->colour[0] & pos->pieces[Pawn]) & ~all &
            (only_captures ? 0xFF00000000000000ull : ~0ull)),
      H(44, 3, -8));
  movelist = generate_pawn_moves(
      H(44, 4, pos), H(44, 4, movelist),
      H(44, 4,
        nw(pos->colour[0] & pos->pieces[Pawn]) & (pos->colour[1] | pos->ep)),
      H(44, 4, -7));
  movelist = generate_pawn_moves(
      H(44, 5, pos), H(44, 5, movelist),
      H(44, 5,
        ne(pos->colour[0] & pos->pieces[Pawn]) & (pos->colour[1] | pos->ep)),
      H(44, 5, -9));
  if (G(63, !only_captures) && G(63, pos->castling[0]) &&
      G(63, !(all & 0x60ull)) &&
      G(64, !is_attacked(H(41, 3, pos), H(41, 3, 1ULL << 5), H(41, 3, true))) &&
      G(64, !is_attacked(H(41, 4, pos), H(41, 4, 1ULL << 4), H(41, 4, true)))) {
    *movelist++ =
        (Move){.from = 4, .to = 6, .promo = None, .takes_piece = None};
  }
  if (G(65, !only_captures) && G(65, pos->castling[1]) &&
      G(65, !(all & 0xEull)) &&
      G(66, !is_attacked(H(41, 5, pos), H(41, 5, 1ULL << 4), H(41, 5, true))) &&
      G(66, !is_attacked(H(41, 6, pos), H(41, 6, 1ULL << 3), H(41, 6, true)))) {
    *movelist++ =
        (Move){.from = 4, .to = 2, .promo = None, .takes_piece = None};
  }
  movelist = generate_piece_moves(H(45, 2, pos), H(45, 2, movelist),
                                  H(45, 2, to_mask));

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
    if (!makemove(H(49, 2, &npos), H(49, 2, &moves[i]))) {
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
    H(68, 1, i8 passed_blocked_pawns[4];) H(68, 1, i8 mobilities[5];)
        H(68, 1, i8 passed_pawns[4];) H(68, 1, i8 king_attacks[5];)
            H(68, 1, i8 tempo;))
  H(67, 1,
    H(69, 1, i8 bishop_pair;) H(69, 1, i8 open_files[6];)
        H(69, 1, i8 pst_file[64];) H(69, 1, i8 pst_rank[64];))
} EvalParams;

typedef struct [[nodiscard]] __attribute__((packed)) {
  i32 material[6];
  H(67, 2,
    H(68, 2, i32 passed_blocked_pawns[4];) H(68, 2, i32 mobilities[5];)
        H(68, 2, i32 passed_pawns[4];) H(68, 2, i32 king_attacks[5];)
            H(68, 2, i32 tempo;))
  H(67, 2,
    H(69, 2, i32 bishop_pair;) H(69, 2, i32 open_files[6];)
        H(69, 2, i32 pst_file[64];) H(69, 2, i32 pst_rank[64];))
} EvalParamsMerged;

G(
    70, [[nodiscard]] S(1) i32 combine_eval_param(H(71, 1, const i32 mg_val),
                                                  H(71, 1, const i32 eg_val)) {
      return G(72, (eg_val << 16)) + G(72, mg_val);
    })

G(70, S(1) const EvalParams mg = ((EvalParams){
          .material = {68, 295, 298, 406, 898, 0},
          .pst_rank =
              {
                  0,   -10, -10, -8,  3,  26, 94,  0,   // Pawn
                  -23, -12, 1,   14,  25, 43, 25,  -73, // Knight
                  -9,  6,   13,  13,  16, 16, -3,  -54, // Bishop
                  -1,  -13, -19, -21, 1,  21, 15,  16,  // Rook
                  16,  17,  9,   -2,  -6, -2, -23, -9,  // Queen
                  -9,  -6,  -31, -35, 1,  60, 72,  80,  // King
              },
          .pst_file =
              {
                  -21, -10, -9, 0,   7,   22,  22, -10, // Pawn
                  -24, -10, 0,  14,  11,  11,  3,  -4,  // Knight
                  -10, 3,   5,  2,   5,   -3,  3,  -5,  // Bishop
                  -8,  -7,  2,  11,  13,  3,   -3, -10, // Rook
                  -10, -7,  -2, 2,   2,   1,   7,  7,   // Queen
                  -16, 25,  -2, -48, -18, -37, 20, 0,   // King
              },
          .mobilities = {7, 6, 2, 3, -9},
          .king_attacks = {0, 15, 20, 14, 0},
          .open_files = {23, -10, -10, 20, -3, -31},
          .passed_pawns = {-9, 12, 36, 94},
          .passed_blocked_pawns = {2, 10, 10, -30},
          .bishop_pair = 24,
          .tempo = 16});)

G(70, S(1) const EvalParams eg = ((EvalParams){
          .material = {71, 302, 297, 542, 992, 0},
          .pst_rank =
              {
                  0,   -4,  -6,  -5, 0,  15, 77, 0,   // Pawn
                  -33, -18, -4,  18, 24, 9,  1,  3,   // Knight
                  -11, -10, -1,  2,  6,  4,  3,  9,   // Bishop
                  -18, -19, -12, 4,  12, 11, 15, 7,   // Rook
                  -60, -45, -20, 9,  29, 30, 38, 18,  // Queen
                  -39, -1,  9,   19, 25, 21, 8,  -47, // King
              },
          .pst_file =
              {
                  9,   12, -1, -9, -4, -3, 2,  -6,  // Pawn
                  -18, -3, 8,  14, 14, 6,  -1, -19, // Knight
                  -6,  -1, 0,  3,  5,  4,  1,  -6,  // Bishop
                  1,   4,  4,  -1, -5, 0,  0,  -3,  // Rook
                  -19, -5, 3,  7,  12, 10, -2, -7,  // Queen
                  -25, 1,  13, 24, 19, 21, 1,  -32, // King
              },
          .mobilities = {2, 4, 3, 1, -4},
          .king_attacks = {0, -3, -6, 7, 0},
          .open_files = {28, -4, 6, 9, 27, 8},
          .passed_pawns = {25, 47, 84, 77},
          .passed_blocked_pawns = {-33, -57, -95, -101},
          .bishop_pair = 53,
          .tempo = 8});)

G(70, S(0) EvalParamsMerged eval_params;)

S(1) void init() {
  // INIT DIAGONAL MASKS
  G(
      49, for (i32 sq = 0; sq < 64; sq++) {
        const u64 bb = 1ULL << sq;
        diag_mask[sq] =
            G(73, ray(H(15, 4, bb), H(15, 4, ~0x101010101010101ull),
                      H(15, 4, 0), H(15, 4, 9))) | // Northeast
            G(73, ray(H(15, 5, bb), H(15, 5, ~0x8080808080808080ull),
                      H(15, 5, 0), H(15, 5, -9))); // Southwest
      })

  G(
      49, // MERGE NON-MATERIAL VALUES
      for (i32 i = 0; i < sizeof(mg) - sizeof(mg.material); i++) {
        // Technically writes past end of array
        // But since the structs are packed, it works
        const i32 offset = sizeof(mg.material);
        ((i32 *)&eval_params)[offset / sizeof(*mg.material) + i] =
            combine_eval_param(H(71, 2, ((i8 *)&mg)[offset + i]),
                               H(71, 2, ((i8 *)&eg)[offset + i]));
      })
  G(
      49, // MERGE MATERIAL VALUES
      for (i32 i = 0; i < sizeof(mg.material) / sizeof(i16); i++) {
        eval_params.material[i] = combine_eval_param(H(71, 3, mg.material[i]),
                                                     H(71, 3, eg.material[i]));
      })
}

G(74, __attribute__((aligned(8))) S(1)
          const i16 max_material[] = {0, 71, 302, 298, 542, 992};)
G(74,
  __attribute__((aligned(8))) S(1) const i8 phases[] = {0, 0, 1, 1, 2, 4, 0};)

S(1) i32 eval(Position *const restrict pos) {
  G(75, i32 score = eval_params.tempo;)
  G(75, i32 phase = 0;)

  for (i32 c = 0; c < 2; c++) {

    G(76,
      const u64 own_pawns = G(77, pos->pieces[Pawn]) & G(77, pos->colour[0]);)

    G(
        76, // BISHOP PAIR
        if (count(G(78, pos->pieces[Bishop]) & G(78, pos->colour[0])) > 1) {
          score += eval_params.bishop_pair;
        })
    G(76, const u64 opp_king_zone = king(pos->colour[1] & pos->pieces[King]);)
    G(76,
      const u64 opp_pawns = G(79, pos->pieces[Pawn]) & G(79, pos->colour[1]);
      const u64 attacked_by_pawns = G(80, se(opp_pawns)) | G(80, sw(opp_pawns));
      const u64 no_passers = G(81, attacked_by_pawns) | G(81, opp_pawns);)

    for (i32 p = Pawn; p <= King; p++) {
      u64 copy = G(82, pos->colour[0]) & G(82, pos->pieces[p]);
      while (copy) {
        const i32 sq = lsb(copy);
        G(83, phase += phases[p];)
        G(83, copy &= copy - 1;)
        G(83, const int file = sq & 7;)
        G(83, const int rank = sq >> 3;)

        G(44, // SPLIT PIECE-SQUARE TABLES FOR FILE
          score += eval_params.pst_file[(p - 1) * 8 + file];)
        G(44, // SPLIT PIECE-SQUARE TABLES FOR RANK
          score += eval_params.pst_rank[(p - 1) * 8 + rank];)

        G(
            44, // PASSED PAWNS
            if (G(84, p == Pawn) &&
                G(84,
                  !(G(85, (0x101010101010101ULL << sq)) & G(85, no_passers))) &&
                G(84, rank > 2)) {
              G(
                  86, if (north(1ULL << sq) & pos->colour[1]) {
                    score += eval_params.passed_blocked_pawns[rank - 3];
                  })

              G(86, score += eval_params.passed_pawns[rank - 3];)
            })

        G(
            44, // OPEN FILES / DOUBLED PAWNS
            if ((G(87, own_pawns) & G(87, north(0x101010101010101ULL << sq))) ==
                0) { score += eval_params.open_files[p - 1]; })

        G(
            44, if (p > Pawn) {
              const u64 mobility =
                  get_mobility(H(33, 3, pos), H(33, 3, sq), H(33, 3, p));

              G(88, // KING ATTACKS
                score += G(89, count(G(90, opp_king_zone) & G(90, mobility))) *
                         G(89, eval_params.king_attacks[p - 2]);)

              G(88, // MOBILITY
                score +=
                G(91, count(G(92, ~attacked_by_pawns) & G(92, mobility) &
                            G(92, ~pos->colour[0]))) *
                G(91, eval_params.mobilities[p - 2]);)
            })

        G(44, // MATERIAL
          score += eval_params.material[p - 1];)
      }
    }

    G(45, score = -score;)
    G(45, flip_pos(pos);)
  }

  return ((short)score * phase + ((score + 0x8000) >> 16) * (24 - phase)) / 24;
}

typedef struct [[nodiscard]] {
  G(68, i32 static_eval;)
  G(68, Move best_move;)
  G(68, i32 num_moves;)
  G(68, u64 position_hash;)
  G(68, Move killer;)
  G(68, Move moves[max_moves];)
} SearchStack;

typedef struct [[nodiscard]] __attribute__((packed)) {
  G(93, Move move;)
  G(93, i16 score;)
  G(93, u16 partial_hash;)
  G(93, u8 flag;)
  G(93, i8 depth;)
} TTEntry;
_Static_assert(sizeof(TTEntry) == 10);

enum { tt_length = 64 * 1024 * 1024 / sizeof(TTEntry) };
enum { Upper = 0, Lower = 1, Exact = 2 };
enum { max_ply = 96 };
enum { mate = 30000, inf = 32000 };

G(94, S(1) i32 move_history[2][6][64][64];)
G(94, S(1) TTEntry tt[tt_length];)
G(94, S(0) size_t start_time;)
G(94, S(0) size_t max_time;)

#if defined(__x86_64__) || defined(_M_X64)
typedef long long __attribute__((__vector_size__(16))) i128;

[[nodiscard]] __attribute__((target("aes"))) S(1) u64
    get_hash(const Position *const pos) {
  i128 hash = {0};

  // USE 16 BYTE POSITION SEGMENTS AS KEYS FOR AES
  const u8 *const data = (const u8 *)pos;
  for (i32 i = 0; i < 5; i++) {
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
  for (i32 i = 0; i < 5; ++i) {
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
i16 search(H(95, 1, Position *const restrict pos), H(95, 1, const i32 ply),
           H(95, 1, i32 alpha), H(95, 1, i32 depth),
           H(96, 1, const bool do_null),
#ifdef FULL
           u64 *nodes,
#endif
           H(96, 1, const i32 beta), H(96, 1, const i32 pos_history_count),
           H(96, 1, SearchStack *restrict stack)) {
  assert(alpha < beta);
  assert(ply >= 0);

  const bool in_check =
      is_attacked(H(41, 7, pos), H(41, 7, pos->colour[0] & pos->pieces[King]),
                  H(41, 7, true));

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
  for (i32 i = G(97, ply) + G(97, pos_history_count);
       G(98, do_null) && G(98, i > 0); i -= 2) {
    if (tt_hash == stack[i].position_hash) {
      return 0;
    }
  }

  // TT PROBING
  G(99, stack[ply].best_move = (Move){0};)
  G(99, TTEntry *tt_entry = &tt[tt_hash % tt_length];)
  G(99, const u16 tt_hash_partial = tt_hash / tt_length;)
  if (tt_entry->partial_hash == tt_hash_partial) {
    stack[ply].best_move = tt_entry->move;

    // TT PRUNING
    if (G(100, alpha == beta - 1) &&
        G(100, tt_entry->flag != tt_entry->score <= alpha) &&
        G(100, tt_entry->depth >= depth)) {
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
  if (G(101, tt_entry->partial_hash == tt_hash_partial) &&
      G(101, tt_entry->flag != static_eval > tt_entry->score)) {
    static_eval = tt_entry->score;
  }

  // QUIESCENCE
  if (G(102, static_eval > alpha) && G(102, in_qsearch)) {
    if (static_eval >= beta) {
      return static_eval;
    }
    alpha = static_eval;
  }

  if (G(103, !in_check) && G(103, alpha == beta - 1)) {
    if (G(104, !in_qsearch) && G(104, depth < 8)) {

      G(105, // RAZORING
        in_qsearch = static_eval + 131 * depth <= alpha;)

      G(105, {
        // REVERSE FUTILITY PRUNING
        if (static_eval - 47 * depth >= beta) {
          return static_eval;
        }
      })
    }

    // NULL MOVE PRUNING
    if (G(106, do_null) && G(106, static_eval >= beta) && G(106, depth > 2)) {
      Position npos = *pos;
      flip_pos(&npos);
      npos.ep = 0;
      const i32 score = -search(
          H(95, 2, &npos), H(95, 2, ply + 1), H(95, 2, -beta),
          H(95, 2, depth - 3 - depth / 4), H(96, 2, false),
#ifdef FULL
          nodes,
#endif
          H(96, 2, -alpha), H(96, 2, pos_history_count), H(96, 2, stack));
      if (score >= beta) {
        return score;
      }
    }
  }

  G(95, u8 tt_flag = Upper;)
  G(95, stack[pos_history_count + ply + 2].position_hash = tt_hash;)
  G(95, stack[ply].num_moves = movegen(
            H(61, 3, pos), H(61, 3, stack[ply].moves), H(61, 3, in_qsearch));)
  G(95, i32 best_score = in_qsearch ? static_eval : -inf;)
  G(95, i32 moves_evaluated = 0;)
  G(95, i32 quiets_evaluated = 0;)

  for (i32 move_index = 0; move_index < stack[ply].num_moves; move_index++) {
    i32 move_score = ~0x1010101LL; // Ends up as large negative

    // MOVE ORDERING
    for (i32 order_index = move_index; order_index < stack[ply].num_moves;
         order_index++) {
      assert(
          stack[ply].moves[order_index].takes_piece ==
          piece_on(H(24, 7, pos), H(24, 7, stack[ply].moves[order_index].to)));
      const i32 order_move_score =
          G(96, // KILLER MOVE
            move_equal(G(107, &stack[ply].killer),
                       G(107, &stack[ply].moves[order_index])) *
                915) +
          G(96, // PREVIOUS BEST MOVE FIRST
            (move_equal(G(108, &stack[ply].best_move),
                        G(108, &stack[ply].moves[order_index]))
             << 30)) +
          G(96, // MOST VALUABLE VICTIM
            stack[ply].moves[order_index].takes_piece * 921) +
          G(96, // HISTORY HEURISTIC
            move_history[pos->flipped]
                        [stack[ply].moves[order_index].takes_piece]
                        [stack[ply].moves[order_index].from]
                        [stack[ply].moves[order_index].to]);
      if (order_move_score > move_score) {
        G(109, swapmoves(G(110, &stack[ply].moves[move_index]),
                         G(110, &stack[ply].moves[order_index]));)
        G(109, move_score = order_move_score;)
      }
    }

    // FORWARD FUTILITY PRUNING / DELTA PRUNING
    if (G(111, !in_check) &&
        G(111,
          G(112, max_material[stack[ply].moves[move_index].promo]) +
                  G(112, static_eval + 128 * depth) +
                  G(112,
                    max_material[stack[ply].moves[move_index].takes_piece]) <
              alpha) &&
        G(111, moves_evaluated) && G(111, depth < 8)) {
      break;
    }

    Position npos = *pos;
#ifdef FULL
    (*nodes)++;
#endif
    if (!makemove(H(49, 3, &npos), H(49, 3, &stack[ply].moves[move_index]))) {
      continue;
    }

    // PRINCIPAL VARIATION SEARCH
    i32 low = moves_evaluated == 0 ? -beta : -alpha - 1;
    moves_evaluated++;

    // LATE MOVE REDCUCTION
    i32 reduction = G(113, depth > 1) && G(113, moves_evaluated > 6)
                        ? G(114, 1) + G(114, (alpha == beta - 1)) +
                              G(114, !improving) + G(114, moves_evaluated / 11)
                        : 1;

    i32 score;
    while (true) {
      score = -search(H(95, 3, &npos), H(95, 3, ply + 1), H(95, 3, low),
                      H(95, 3, depth - reduction), H(96, 3, true),
#ifdef FULL
                      nodes,
#endif
                      H(96, 3, -alpha), H(96, 3, pos_history_count),
                      H(96, 3, stack));

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
    }

    if (score > alpha) {
      G(115, stack[ply].best_move = stack[ply].moves[move_index];)
      G(115, alpha = score;)
      G(115, tt_flag = Exact;)
      if (score >= beta) {
        tt_flag = Lower;
        assert(stack[ply].best_move.takes_piece ==
               piece_on(H(24, 8, pos), H(24, 8, stack[ply].best_move.to)));
        G(
            116, if (stack[ply].best_move.takes_piece == None) {
              stack[ply].killer = stack[ply].best_move;
            })
        G(116, const i32 bonus = depth * depth;
          G(117,
            i32 *const this_hist =
                &move_history[pos->flipped][stack[ply].best_move.takes_piece]
                             [stack[ply].best_move.from]
                             [stack[ply].best_move.to];

            *this_hist += bonus - bonus * *this_hist / 1024;)
              G(
                  117, for (i32 prev_index = 0; prev_index < move_index;
                            prev_index++) {
                    const Move prev = stack[ply].moves[prev_index];
                    i32 *const prev_hist =
                        &move_history[pos->flipped][prev.takes_piece][prev.from]
                                     [prev.to];
                    *prev_hist -= bonus + bonus * *prev_hist / 1024;
                  }))
        break;
      }
    }

    if (stack[ply].moves[move_index].takes_piece == None) {
      quiets_evaluated++;
    }

    // LATE MOVE PRUNING
    if (G(118, !in_check) && G(118, alpha == beta - 1) &&
        G(118, quiets_evaluated > 1 + depth * depth >> !improving)) {
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
    H(119, 1, Position *const restrict pos),
    H(119, 1, SearchStack *restrict stack),
    H(119, 1, const i32 pos_history_count)) {
  start_time = get_time();
#ifdef FULL
  for (i32 depth = 1; depth < maxdepth; depth++) {
#else
  for (i32 depth = 1; depth < max_ply; depth++) {
#endif
    i32 score =
        search(H(95, 4, pos), H(95, 4, 0), H(95, 4, -inf), H(95, 4, depth),
               H(96, 4, false),
#ifdef FULL
               nodes,
#endif
               H(96, 4, inf), H(96, 4, pos_history_count), H(96, 4, stack));
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
    move_str(H(23, 2, move_name), H(23, 2, &stack[0].best_move),
             H(23, 2, pos->flipped));
    puts(move_name);
#endif

    if (elapsed > max_time / 16) {
      break;
    }
  }
  char move_name[8];
  move_str(H(23, 3, move_name), H(23, 3, &stack[0].best_move),
           H(23, 3, pos->flipped));
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
      i32 piece = piece_on(H(24, 9, &npos), H(24, 9, sq));
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
  iteratively_deepen(20, &nodes, H(119, 2, &pos), H(119, 2, stack),
                     H(119, 2, pos_history_count));
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

  G(120, // #ifdef LOWSTACK
         //  SearchStack *stack = malloc(sizeof(SearchStack) * 1024);
         // #else
    SearchStack stack[1024];
    // #endif
  )
  G(120, Position pos;)
  G(120, char line[4096];)
  G(120, __builtin_memset(move_history, 0, sizeof(move_history));)
  G(120, i32 pos_history_count;)
  G(120, init();)

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
      iteratively_deepen(max_ply, &nodes, H(119, 3, &pos), H(119, 3, stack),
                         H(119, 3, pos_history_count));
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
    G(121, if (line[0] == 'i') { puts("readyok"); })
    else G(121, if (line[0] == 'q') { exit_now(); }) else if (line[0] == 'p') {
      G(122, pos = start_pos;)
      G(122, pos_history_count = 0;)
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
          move_str(H(23, 4, move_name), H(23, 4, &stack[0].moves[i]),
                   H(23, 4, pos.flipped));
          assert(move_string_equal(line, move_name) ==
                 !strcmp(line, move_name));
          if (move_string_equal(G(123, line), G(123, move_name))) {
            stack[pos_history_count].position_hash = get_hash(&pos);
            pos_history_count++;
            if (stack[0].moves[i].takes_piece != None) {
              pos_history_count = 0;
            }
            makemove(H(49, 4, &pos), H(49, 4, &stack[0].moves[i]));
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
      iteratively_deepen(max_ply, &nodes, H(119, 4, &pos), H(119, 4, stack),
                         H(119, 4, pos_history_count));
#else
      for (i32 i = 0; i < (pos.flipped ? 4 : 2); i++) {
        getl(line);
        max_time = atoi(line) / 2;
      }
      iteratively_deepen(H(119, 5, &pos), H(119, 5, stack),
                         H(119, 5, pos_history_count));
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
