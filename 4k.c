// Minify thing thang
#define G(a, b...) b
#define H(a, b, c...) c

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
    1, static ssize_t _sys(H(2, 1, ssize_t arg2), H(2, 1, ssize_t call),
                           H(2, 1, ssize_t arg1), H(2, 1, ssize_t arg3)) {
      ssize_t ret;
      asm volatile("syscall"
                   : "=a"(ret)
                   : "a"(call), "D"(arg1), "S"(arg2), "d"(arg3)
                   : "rcx", "r11", "memory");
      return ret;
    })

G(
    1, [[nodiscard]] static i32 strlen(const char *const restrict string) {
      i32 length = 0;
      while (string[length]) {
        length++;
      }
      return length;
    })

G(
    1, static void exit_now() {
      asm volatile("movl $60, %eax\n\t"
                   "syscall");
    })

G(
    3,
    static void putl(const char *const restrict string) {
      i32 length = 0;
      while (string[length]) {
        _sys(H(2, 2, (ssize_t)(&string[length])), H(2, 2, 1), H(2, 2, stdout),
             H(2, 2, 1));
        length++;
      }
    }

    static void puts(const char *const restrict string) {
      putl(string);
      putl("\n");
    })

G(
    3, [[nodiscard]] static u32 atoi(const char *restrict string) {
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
    3, // Non-standard, gets but a word instead of a line
    static bool getl(char *restrict string) {
      while (true) {
        const int result = _sys(H(2, 3, (ssize_t)string), H(2, 3, 0),
                                H(2, 3, stdin), H(2, 3, 1));

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

#define printf(format, ...) _printf(format, (size_t[]){__VA_ARGS__})

static void _printf(const char *format, const size_t *args) {
  long long value;
  char buffer[16], *string;

  while (true) {
    if (!*format) {
      break;
    }
    if (*format != '%') {
      _sys(H(2, 4, (ssize_t)format), H(2, 4, 1), H(2, 4, stdout), H(2, 4, 1));
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

[[nodiscard]] static size_t get_time() {
  timespec ts;
  _sys(H(2, 5, (ssize_t)&ts), H(2, 5, 228), H(2, 5, 1), H(2, 5, 0));
  return ts.tv_sec * 1000 + ts.tv_nsec / 1000000;
}

#ifdef ASSERTS
#define assert(condition)                                                      \
  if (!(condition)) {                                                          \
    printf("Assert failed on line %i: ", __LINE__);                            \
    puts(#condition);                                                          \
    _sys(H(2, 6, 0), H(2, 6, 60), H(2, 6, 1), H(2, 6, 0));                     \
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

[[nodiscard]] static size_t get_time() {
  struct timespec ts;
  clock_gettime(CLOCK_MONOTONIC, &ts);
  return ts.tv_sec * 1000 + ts.tv_nsec / 1000000;
}

static void exit_now() { exit(0); }

static bool getl(char *restrict string) {
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
  G(5, u8 promo;)
  G(5, u8 from; u8 to;)
  G(5, u8 takes_piece;)
} Move;

typedef struct [[nodiscard]] {
  G(6, u64 ep;)
  G(6, u64 pieces[7];)
  G(6, u64 colour[2];)
  G(7, bool castling[4];)
  G(7, bool flipped;)
} Position;

[[nodiscard]] static bool move_string_equal(G(8, const char *restrict lhs),
                                            G(8, const char *restrict rhs)) {
  return (G(9, *(const u64 *)rhs) ^ G(9, *(const u64 *)lhs)) << 24 == 0;
}

[[nodiscard]] static u64 flip_bb(const u64 bb) { return __builtin_bswap64(bb); }

#ifdef ARCH32
#pragma GCC push_options
#pragma GCC optimize("O3")
#endif
static i32 lsb(u64 bb) { return __builtin_ctzll(bb); }
#ifdef ARCH32
#pragma GCC pop_options
#endif

G(
    10, [[nodiscard]] static u64 shift(H(11, 1, const u64 bb),
                                       H(11, 1, const u64 mask),
                                       H(11, 1, const i32 shift)) {
      return shift > 0 ? bb << shift & mask : bb >> -shift & mask;
    })

G(
    10, [[nodiscard]] static i32 count(const u64 bb) {
      return __builtin_popcountll(bb);
    })

G(12, [[nodiscard]] static u64 south(const u64 bb) { return bb >> 8; })

G(12, [[nodiscard]] static u64 north(const u64 bb) { return bb << 8; })

G(
    12, [[nodiscard]] static u64 east(const u64 bb) {
      return bb << 1 & ~0x101010101010101ull;
    })

G(
    12, [[nodiscard]] static u64 west(const u64 bb) {
      return bb >> 1 & ~0x8080808080808080ull;
    })

G(13, [[nodiscard]] static u64 ne(const u64 bb) { return east(north(bb)); })

G(
    13, [[nodiscard]] static u64 nw(const u64 bb) {
      return shift(H(11, 2, bb), H(11, 2, ~0x8080808080808080ull), H(11, 2, 7));
      // return west(north(bb));
    })

G(13, [[nodiscard]] static u64 sw(const u64 bb) { return west(south(bb)); })

G(13, [[nodiscard]] static u64 se(const u64 bb) { return east(south(bb)); })

G(
    14, [[nodiscard]] static u64 ray(H(15, 1, const u64 mask),
                                     H(15, 1, const u64 bb),
                                     H(15, 1, const i32 shift_by),
                                     H(15, 1, const u64 blockers)) {
      u64 result = shift(H(11, 3, bb), H(11, 3, mask), H(11, 3, shift_by));
      for (i32 i = 0; i < 6; i++) {
        result |= shift(H(11, 4, result & ~blockers), H(11, 4, mask),
                        H(11, 4, shift_by));
      }
      return result;
    })

G(14, static u64 diag_mask[64];)

G(
    14, [[nodiscard]] u64 xattack(H(16, 1, const u64 dir_mask),
                                  H(16, 1, const u64 bb),
                                  H(16, 1, const u64 blockers)) {
      return dir_mask & ((blockers & dir_mask) - bb ^
                         flip_bb(flip_bb(blockers & dir_mask) - flip_bb(bb)));
    })

G(
    17, [[nodiscard]] static u64 king(const u64 bb) {
      return G(18, (bb << 1 | bb << 9 | bb >> 7) & ~0x101010101010101ull) |
             G(18, bb >> 8) |
             G(18, (bb >> 1 | bb >> 9 | bb << 7) & ~0x8080808080808080ull) |
             G(18, bb << 8);
    })

G(
    17, [[nodiscard]] static u64 rook(H(19, 1, const u64 blockers),
                                      H(19, 1, const u64 bb)) {
      assert(count(bb) == 1);
      return xattack(H(16, 2, bb ^ 0x101010101010101ULL << lsb(bb) % 8),
                     H(16, 2, bb), H(16, 2, blockers)) |
             ray(H(15, 2, ~0x101010101010101ull), H(15, 2, bb), H(15, 2, 1),
                 H(15, 2, blockers)) // East
             | ray(H(15, 3, ~0x8080808080808080ull), H(15, 3, bb), H(15, 3, -1),
                   H(15, 3, blockers)); // West
    })

G(
    17, [[nodiscard]] static u64 bishop(H(20, 1, const u64 blockers),
                                        H(20, 1, const u64 bb)) {
      assert(count(bb) == 1);
      const i32 sq = lsb(bb);
      return xattack(H(16, 3, diag_mask[sq]), H(16, 3, bb),
                     H(16, 3, blockers)) |
             xattack(H(16, 4, flip_bb(diag_mask[sq ^ 56])), H(16, 4, bb),
                     H(16, 4, blockers));
    })

G(
    17, [[nodiscard]] static u64 knight(const u64 bb) {
      return G(21, (bb << 17 | bb >> 15) & ~0x101010101010101ull) |
             G(21, (bb << 15 | bb >> 17) & ~0x8080808080808080ull) |
             G(21, (bb << 6 | bb >> 10) & 0x3F3F3F3F3F3F3F3Full) |
             G(21, (bb << 10 | bb >> 6) & 0xFCFCFCFCFCFCFCFCull);
    })

G(
    22, static void swapu64(G(23, u64 *const rhs), G(23, u64 *const lhs)) {
      const u64 temp = *lhs;
      *lhs = *rhs;
      *rhs = temp;
    })

G(
    22, static void move_str(H(24, 1, const i32 flip),
                             H(24, 1, const Move *restrict move),
                             H(24, 1, char *restrict str)) {
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
        G(25, str[i * 2] = 'a' + (&move->from)[i] % 8;)
        G(25, str[i * 2 + 1] = '1' + ((&move->from)[i] / 8 ^ 7 * flip);)
      }

      G(26, str[5] = '\0';)
      G(26, str[4] = "\0\0nbrq"[move->promo];)
    })

G(
    22, [[nodiscard]] static i32 piece_on(H(27, 1,
                                            const Position *const restrict pos),
                                          H(27, 1, const i32 sq)) {
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
    22, static void swapbool(G(24, bool *const restrict lhs),
                             G(24, bool *const restrict rhs)) {
      const bool temp = *lhs;
      *lhs = *rhs;
      *rhs = temp;
    })

G(
    22, [[nodiscard]] static bool move_equal(G(28, Move *const rhs),
                                             G(28, Move *const lhs)) {
      return *(u32 *)lhs == *(u32 *)rhs;
    })

G(
    22,
    static void swapu32(G(29, u32 *const lhs), G(29, u32 *const rhs)) {
      const u32 temp = *lhs;
      *lhs = *rhs;
      *rhs = temp;
    }

    static void swapmoves(G(30, Move *const lhs), G(30, Move *const rhs)) {
      swapu32(G(31, (u32 *)rhs), G(31, (u32 *)lhs));
    })

G(
    32, [[nodiscard]] static i32 is_attacked(
            H(33, 1, const Position *const restrict pos),
            H(33, 1, const u64 bb), H(33, 1, const i32 them)) {
      assert(count(bb) == 1);
      const u64 theirs = pos->colour[them];
      const u64 pawns = theirs & pos->pieces[Pawn];
      if ((them ? sw(pawns) | se(pawns) : nw(pawns) | ne(pawns)) & bb) {
        return true;
      }
      const u64 blockers = pos->colour[0] | pos->colour[1];
      return G(34, bishop(H(20, 2, blockers), H(20, 2, bb)) & theirs &
                       (pos->pieces[Bishop] | pos->pieces[Queen])) ||
             G(34, king(bb) & theirs & pos->pieces[King]) ||
             G(34, knight(bb) & theirs & pos->pieces[Knight]) ||
             G(34, rook(H(19, 2, blockers), H(19, 2, bb)) & theirs &
                       (pos->pieces[Rook] | pos->pieces[Queen]));
    })

G(
    32, [[nodiscard]] static u64 get_mobility(H(35, 1, const Position *pos),
                                              H(35, 1, const i32 sq),
                                              H(35, 1, const i32 piece)) {
      u64 moves = 0;
      const u64 bb = 1ULL << sq;
      if (piece == Knight) {
        moves = knight(bb);
      } else if (piece == King) {
        moves = king(bb);
      } else {
        const u64 blockers = pos->colour[0] | pos->colour[1];
        if (piece == Rook || piece == Queen) {
          moves |= rook(H(19, 3, blockers), H(19, 3, bb));
        }
        if (piece == Bishop || piece == Queen) {
          moves |= bishop(H(20, 3, blockers), H(20, 3, bb));
        }
      }
      return moves;
    })

G(
    32, static void flip_pos(Position *const restrict pos) {
      G(36, pos->flipped ^= 1;)

      G(
          36, // Hack to flip the first 10 bitboards in Position.
              // Technically UB but works in GCC 14.2
          u64 *pos_ptr = (u64 *)pos;
          for (i32 i = 0; i < 10; i++) { pos_ptr[i] = flip_bb(pos_ptr[i]); })
      G(
          36, for (i32 i = 0; i < 2; i++) {
            swapbool(G(37, &pos->castling[i + 2]), G(37, &pos->castling[i]));
          })
      G(36, swapu64(G(38, &pos->colour[0]), G(38, &pos->colour[1]));)
    })

G(
    39,
    static Move *generate_piece_moves(H(40, 1, const u64 to_mask),
                                      H(40, 1, const Position *restrict pos),
                                      H(40, 1, Move *restrict movelist)) {
      for (i32 piece = Knight; piece <= King; piece++) {
        assert(piece == Knight || piece == Bishop || piece == Rook ||
               piece == Queen || piece == King);
        u64 copy = pos->colour[0] & pos->pieces[piece];
        while (copy) {
          const u8 from = lsb(copy);
          assert(from >= 0);
          assert(from < 64);
          G(41, u64 moves = G(42, to_mask) &
                            G(42, get_mobility(H(35, 2, pos), H(35, 2, from),
                                               H(35, 2, piece)));)

          G(41, copy &= copy - 1;)

          while (moves) {
            const u8 to = lsb(moves);
            assert(to >= 0);
            assert(to < 64);

            G(43, moves &= moves - 1;)
            G(43, *movelist++ = ((Move){
                      .from = from,
                      .to = to,
                      .promo = None,
                      .takes_piece = piece_on(H(27, 2, pos), H(27, 2, to))});)
          }
        }
      }

      return movelist;
    })

G(
    39, static Move *generate_pawn_moves(H(44, 1, const Position *const pos),
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
        assert(piece_on(H(27, 3, pos), H(27, 3, from)) == Pawn);
        const u8 takes = piece_on(H(27, 4, pos), H(27, 4, to));
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
    39, i32 makemove(H(45, 1, Position *const restrict pos),
                     H(45, 1, const Move *const restrict move)) {
      assert(move->from >= 0);
      assert(move->from < 64);
      assert(move->to >= 0);
      assert(move->to < 64);
      assert(move->from != move->to);
      assert(move->promo == None || move->promo == Knight ||
             move->promo == Bishop || move->promo == Rook ||
             move->promo == Queen);
      assert(move->takes_piece != King);
      assert(move->takes_piece == piece_on(H(27, 5, pos), H(27, 5, move->to)));

      G(46, const u64 from = 1ull << move->from;)
      G(46, const u64 to = 1ull << move->to;)
      G(47, const i32 piece = piece_on(H(27, 6, pos), H(27, 6, move->from));
        assert(piece != None);)
      G(47, const u64 mask = from | to;)

      G(
          48, // Castling
          if (piece == King) {
            const u64 bb = move->to - move->from == 2   ? 0xa0
                           : move->from - move->to == 2 ? 0x9
                                                        : 0;
            G(49, pos->colour[0] ^= bb;)
            G(49, pos->pieces[Rook] ^= bb;)
          })

      G(48, pos->colour[0] ^= mask;)

      // Move the piece
      G(48, pos->pieces[piece] ^= mask;)
      G(
          48, // Captures
          if (move->takes_piece != None) {
            G(50, pos->colour[1] ^= to;)
            G(50, pos->pieces[move->takes_piece] ^= to;)
          })

      // En passant
      if (G(51, piece == Pawn) && G(51, to == pos->ep)) {
        G(52, pos->colour[1] ^= to >> 8;)
        G(52, pos->pieces[Pawn] ^= to >> 8;)
      }
      pos->ep = 0;

      G(
          53, // Pawn double move
          if (G(54, move->to - move->from == 16) && G(54, piece == Pawn)) {
            pos->ep = to >> 8;
          })

      G(
          33, // Promotions
          if (move->promo != None) {
            G(55, pos->pieces[Pawn] ^= to;)
            G(55, pos->pieces[move->promo] ^= to;)
          })

      G(53, // Update castling permissions
        G(56, pos->castling[3] &= !(mask & 0x1100000000000000ull);)
            G(56, pos->castling[1] &= !(mask & 0x11ull);)
                G(56, pos->castling[2] &= !(mask & 0x9000000000000000ull);)
                    G(56, pos->castling[0] &= !(mask & 0x90ull);))

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
      return !is_attacked(H(33, 2, pos),
                          H(33, 2, pos->colour[1] & pos->pieces[King]),
                          H(33, 2, false));
    })

enum { max_moves = 218 };

[[nodiscard]] static i32 movegen(H(57, 1, const Position *const restrict pos),
                                 H(57, 1, Move *restrict movelist),
                                 H(57, 1, const i32 only_captures)) {

  G(58, const Move *start = movelist;)
  G(58, const u64 to_mask = only_captures ? pos->colour[1] : ~pos->colour[0];)
  G(58, const u64 all = pos->colour[0] | pos->colour[1];)
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
  if (G(59, !only_captures) && G(59, pos->castling[0]) &&
      G(59, !(all & 0x60ull)) &&
      G(60, !is_attacked(H(33, 3, pos), H(33, 3, 1ULL << 5), H(33, 3, true))) &&
      G(60, !is_attacked(H(33, 4, pos), H(33, 4, 1ULL << 4), H(33, 4, true)))) {
    *movelist++ =
        (Move){.from = 4, .to = 6, .promo = None, .takes_piece = None};
  }
  if (G(61, !(all & 0xEull)) && G(61, pos->castling[1]) &&
      G(61, !only_captures) &&
      G(62, !is_attacked(H(33, 5, pos), H(33, 5, 1ULL << 4), H(33, 5, true))) &&
      G(62, !is_attacked(H(33, 6, pos), H(33, 6, 1ULL << 3), H(33, 6, true)))) {
    *movelist++ =
        (Move){.from = 4, .to = 2, .promo = None, .takes_piece = None};
  }
  movelist = generate_piece_moves(H(40, 2, to_mask), H(40, 2, pos),
                                  H(40, 2, movelist));

  const i32 num_moves = movelist - start;
  assert(num_moves < max_moves);
  return num_moves;
}

#pragma endregion

#pragma region engine

[[nodiscard]] static u64 perft(const Position *const restrict pos,
                               const i32 depth) {
  if (depth == 0) {
    return 1;
  }

  u64 nodes = 0;
  Move moves[max_moves];
  const i32 num_moves =
      movegen(H(57, 2, pos), H(57, 2, moves), H(57, 2, false));

  for (i32 i = 0; i < num_moves; ++i) {
    Position npos = *pos;

    // Check move legality
    if (!makemove(H(45, 2, &npos), H(45, 2, &moves[i]))) {
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
  H(63, 1,
    H(64, 1, i8 passed_blocked_pawns[4];) H(64, 1, i8 mobilities[4];)
        H(64, 1, i8 passed_pawns[4];) H(64, 1, i8 tempo;)
            H(64, 1, i8 king_attacks[4];))
  H(63, 1,
    H(65, 1, i8 bishop_pair;) H(65, 1, i8 open_files[6];)
        H(65, 1, i8 pst_file[64];) H(65, 1, i8 pst_rank[64];))
} EvalParams;

typedef struct [[nodiscard]] __attribute__((packed)) {
  i32 material[6];
  H(63, 2,
    H(64, 2, i32 passed_blocked_pawns[4];) H(64, 2, i32 mobilities[4];)
        H(64, 2, i32 passed_pawns[4];) H(64, 2, i32 tempo;)
            H(64, 2, i32 king_attacks[4];))
  H(63, 2,
    H(65, 2, i32 bishop_pair;) H(65, 2, i32 open_files[6];)
        H(65, 2, i32 pst_file[64];) H(65, 2, i32 pst_rank[64];))
} EvalParamsMerged;

G(66, static const EvalParams eg = ((EvalParams){
          .material = {71, 311, 298, 543, 996, 0},
          .pst_rank =
              {
                  0,   -4,  -6,  -5, 0,  15, 77, 0,   // Pawn
                  -38, -18, -1,  20, 26, 10, 1,  0,   // Knight
                  -12, -10, -1,  2,  6,  3,  3,  8,   // Bishop
                  -18, -19, -12, 5,  12, 11, 15, 7,   // Rook
                  -60, -45, -20, 10, 30, 30, 38, 18,  // Queen
                  -39, -1,  9,   18, 25, 21, 8,  -47, // King
              },
          .pst_file =
              {
                  9,   12, -1, -10, -4, -3, 2,  -6,  // Pawn
                  -22, -4, 11, 17,  17, 8,  -3, -23, // Knight
                  -6,  -1, 0,  4,   5,  4,  1,  -6,  // Bishop
                  1,   4,  4,  -1,  -4, 0,  0,  -4,  // Rook
                  -19, -5, 2,  8,   13, 10, -2, -8,  // Queen
                  -26, 1,  13, 24,  18, 20, 1,  -33, // King
              },
          .mobilities = {4, 3, 1, -4},
          .king_attacks = {-3, -6, 7, 0},
          .open_files = {28, -4, 6, 9, 27, 8},
          .passed_pawns = {25, 47, 84, 77},
          .passed_blocked_pawns = {-32, -56, -95, -102},
          .bishop_pair = 53,
          .tempo = 8});)

G(66, static const EvalParams mg = ((EvalParams){
          .material = {69, 317, 297, 403, 892, 0},
          .pst_rank =
              {
                  0,   -11, -9,  -9,  3,  26, 94,  0,   // Pawn
                  -32, -18, 2,   15,  31, 55, 25,  -78, // Knight
                  -8,  5,   14,  13,  16, 17, -3,  -54, // Bishop
                  -1,  -13, -19, -21, 1,  21, 15,  17,  // Rook
                  16,  16,  9,   -2,  -6, -2, -23, -8,  // Queen
                  -7,  -5,  -30, -33, 2,  61, 73,  81,  // King
              },
          .pst_file =
              {
                  -22, -10, -9, 0,   7,   23,  22, -11, // Pawn
                  -31, -9,  5,  16,  14,  15,  3,  -13, // Knight
                  -11, 3,   6,  2,   5,   -3,  3,  -5,  // Bishop
                  -7,  -7,  2,  10,  11,  3,   -3, -9,  // Rook
                  -10, -7,  -1, 1,   1,   0,   8,  8,   // Queen
                  -15, 24,  -1, -48, -17, -36, 19, 1,   // King
              },
          .mobilities = {6, 2, 3, -8},
          .king_attacks = {14, 19, 14, 0},
          .open_files = {23, -11, -9, 21, -3, -31},
          .passed_pawns = {-9, 12, 36, 94},
          .passed_blocked_pawns = {0, 9, 10, -29},
          .bishop_pair = 25,
          .tempo = 16});)

G(
    66,
    [[nodiscard]] static i32 combine_eval_param(H(67, 1, const i32 mg_val),
                                                H(67, 1, const i32 eg_val)) {
      return G(68, (eg_val << 16)) + G(68, mg_val);
    })

G(66, static EvalParamsMerged eval_params;)

static void init() {
  // INIT DIAGONAL MASKS
  G(
      45, for (i32 sq = 0; sq < 64; sq++) {
        const u64 bb = 1ULL << sq;
        diag_mask[sq] =
            G(69, ray(H(15, 4, ~0x101010101010101ull), H(15, 4, bb),
                      H(15, 4, 9), H(15, 4, 0))) | // Northeast
            G(69, ray(H(15, 5, ~0x8080808080808080ull), H(15, 5, bb),
                      H(15, 5, -9), H(15, 5, 0))); // Southwest
      })

  G(
      45, // MERGE NON-MATERIAL VALUES
      for (i32 i = 0; i < sizeof(mg) - sizeof(mg.material); i++) {
        // Technically writes past end of array
        // But since the structs are packed, it works
        const i32 offset = sizeof(mg.material);
        ((i32 *)&eval_params)[offset / sizeof(*mg.material) + i] =
            combine_eval_param(H(67, 2, ((i8 *)&mg)[offset + i]),
                               H(67, 2, ((i8 *)&eg)[offset + i]));
      })
  G(
      45, // MERGE MATERIAL VALUES
      for (i32 i = 0; i < sizeof(mg.material) / sizeof(i16); i++) {
        eval_params.material[i] = combine_eval_param(H(67, 3, mg.material[i]),
                                                     H(67, 3, eg.material[i]));
      })
}

G(70, __attribute__((aligned(8))) static const i16 max_material[] = {
          0, 71, 317, 298, 543, 996};)
G(70,
  __attribute__((aligned(8))) static const i8 phases[] = {0, 0, 1, 1, 2, 4, 0};)

static i32 eval(Position *const restrict pos) {
  G(71, i32 phase = 0;)
  G(71, i32 score = eval_params.tempo;)

  for (i32 c = 0; c < 2; c++) {

    G(
        72, // BISHOP PAIR
        if (count(G(73, pos->colour[0]) & G(73, pos->pieces[Bishop])) > 1) {
          score += eval_params.bishop_pair;
        })

    G(72, const u64 opp_king_zone = king(pos->colour[1] & pos->pieces[King]);)
    G(72,
      const u64 opp_pawns = G(74, pos->pieces[Pawn]) & G(74, pos->colour[1]);
      const u64 attacked_by_pawns = G(75, se(opp_pawns)) | G(75, sw(opp_pawns));
      const u64 no_passers = G(76, attacked_by_pawns) | G(76, opp_pawns);)
    G(72,
      const u64 own_pawns = G(77, pos->colour[0]) & G(77, pos->pieces[Pawn]);)

    for (i32 p = Pawn; p <= King; p++) {
      u64 copy = G(78, pos->colour[0]) & G(78, pos->pieces[p]);
      while (copy) {
        const i32 sq = lsb(copy);
        G(79, phase += phases[p];)
        G(79, const int file = sq & 7;)
        G(79, copy &= copy - 1;)
        G(79, const int rank = sq >> 3;)

        G(44, // SPLIT PIECE-SQUARE TABLES FOR FILE
          score += eval_params.pst_file[(p - 1) * 8 + file];)
        G(44, // SPLIT PIECE-SQUARE TABLES FOR RANK
          score += eval_params.pst_rank[(p - 1) * 8 + rank];)

        G(
            44, // PASSED PAWNS
            if (G(80, p == Pawn) &&
                G(80,
                  !(G(81, (0x101010101010101ULL << sq)) & G(81, no_passers))) &&
                G(80, rank > 2)) {
              G(
                  82, if (north(1ULL << sq) & pos->colour[1]) {
                    score += eval_params.passed_blocked_pawns[rank - 3];
                  })

              G(82, score += eval_params.passed_pawns[rank - 3];)
            })

        G(
            44, // OPEN FILES / DOUBLED PAWNS
            if ((G(83, own_pawns) & G(83, north(0x101010101010101ULL << sq))) ==
                0) { score += eval_params.open_files[p - 1]; })

        G(
            44, if (p > Knight) {
              const u64 mobility =
                  get_mobility(H(35, 3, pos), H(35, 3, sq), H(35, 3, p));

              G(84, // MOBILITY
                score +=
                G(85, count(G(86, mobility) & G(86, ~attacked_by_pawns) &
                            G(86, ~pos->colour[0]))) *
                G(85, eval_params.mobilities[p - 3]);)

              G(84, // KING ATTACKS
                score += G(87, eval_params.king_attacks[p - 3]) *
                         G(87, count(G(88, mobility) & G(88, opp_king_zone)));)
            })

        G(44, // MATERIAL
          score += eval_params.material[p - 1];)
      }
    }

    G(40, flip_pos(pos);)
    G(40, score = -score;)
  }

  return ((short)score * phase + ((score + 0x8000) >> 16) * (24 - phase)) / 24;
}

typedef struct [[nodiscard]] {
  G(64, Move best_move;)
  G(64, i32 static_eval;)
  G(64, Move killer;)
  G(64, u64 position_hash;)
  G(64, i32 num_moves;)
  G(64, Move moves[max_moves];)
} SearchStack;

typedef struct [[nodiscard]] __attribute__((packed)) {
  G(89, i16 score;)
  G(89, Move move;)
  G(89, u16 partial_hash;)
  G(89, i8 depth;)
  G(89, u8 flag;)
} TTEntry;
_Static_assert(sizeof(TTEntry) == 10);

enum { tt_length = 64 * 1024 * 1024 / sizeof(TTEntry) };
enum { Upper = 0, Lower = 1, Exact = 2 };
enum { max_ply = 96 };
enum { mate = 30000, inf = 32000 };

G(90, static i32 move_history[2][6][64][64];)
G(90, static TTEntry tt[tt_length];)
G(90, static size_t max_time;)
G(90, static size_t start_time;)

#if defined(__x86_64__) || defined(_M_X64)
typedef long long __attribute__((__vector_size__(16))) i128;

[[nodiscard]] __attribute__((target("aes"))) u64
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

static i16 search(H(91, 1, Position *const restrict pos),
                  H(91, 1, const i32 ply), H(91, 1, i32 alpha),
                  H(91, 1, i32 depth), H(92, 1, const bool do_null),
#ifdef FULL
                  u64 *nodes,
#endif
                  H(92, 1, const i32 beta),
                  H(92, 1, const i32 pos_history_count),
                  H(92, 1, SearchStack *restrict stack)) {
  assert(alpha < beta);
  assert(ply >= 0);

  const bool in_check =
      is_attacked(H(33, 7, pos), H(33, 7, pos->colour[0] & pos->pieces[King]),
                  H(33, 7, true));

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
  for (i32 i = G(93, pos_history_count) + G(93, ply);
       G(94, i > 0) && G(94, do_null); i -= 2) {
    if (tt_hash == stack[i].position_hash) {
      return 0;
    }
  }

  // TT PROBING
  G(95, stack[ply].best_move = (Move){0};)
  G(95, TTEntry *tt_entry = &tt[tt_hash % tt_length];)
  G(95, const u16 tt_hash_partial = tt_hash / tt_length;)
  if (tt_entry->partial_hash == tt_hash_partial) {
    stack[ply].best_move = tt_entry->move;

    // TT PRUNING
    if (G(96, alpha == beta - 1) &&
        G(96, tt_entry->flag != tt_entry->score <= alpha) &&
        G(96, tt_entry->depth >= depth)) {
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
  if (G(97, tt_entry->partial_hash == tt_hash_partial) &&
      G(97, tt_entry->flag != static_eval > tt_entry->score)) {
    static_eval = tt_entry->score;
  }

  // QUIESCENCE
  if (G(98, static_eval > alpha) && G(98, in_qsearch)) {
    if (static_eval >= beta) {
      return static_eval;
    }
    alpha = static_eval;
  }

  if (G(99, !in_check) && G(99, alpha == beta - 1)) {
    if (G(100, !in_qsearch) && G(100, depth < 8)) {

      G(101, {
        // REVERSE FUTILITY PRUNING
        if (static_eval - 47 * depth >= beta) {
          return static_eval;
        }
      })

      G(101, // RAZORING
        in_qsearch = static_eval + 131 * depth <= alpha;)
    }

    // NULL MOVE PRUNING
    if (G(102, static_eval >= beta) && G(102, do_null) && G(102, depth > 2)) {
      Position npos = *pos;
      flip_pos(&npos);
      npos.ep = 0;
      const i32 score = -search(
          H(91, 2, &npos), H(91, 2, ply + 1), H(91, 2, -beta),
          H(91, 2, depth - 3 - depth / 4), H(92, 2, false),
#ifdef FULL
          nodes,
#endif
          H(92, 2, -alpha), H(92, 2, pos_history_count), H(92, 2, stack));
      if (score >= beta) {
        return score;
      }
    }
  }

  G(91, stack[ply].num_moves = movegen(
            H(57, 3, pos), H(57, 3, stack[ply].moves), H(57, 3, in_qsearch));)
  G(91, i32 moves_evaluated = 0;)
  G(91, i32 best_score = in_qsearch ? static_eval : -inf;)
  G(91, stack[pos_history_count + ply + 2].position_hash = tt_hash;)
  G(91, i32 quiets_evaluated = 0;)
  G(91, u8 tt_flag = Upper;)

  for (i32 move_index = 0; move_index < stack[ply].num_moves; move_index++) {
    i32 move_score = ~0x1010101LL; // Ends up as large negative

    // MOVE ORDERING
    for (i32 order_index = move_index; order_index < stack[ply].num_moves;
         order_index++) {
      assert(
          stack[ply].moves[order_index].takes_piece ==
          piece_on(H(27, 7, pos), H(27, 7, stack[ply].moves[order_index].to)));
      const i32 order_move_score =
          G(92, // MOST VALUABLE VICTIM
            stack[ply].moves[order_index].takes_piece * 921) +
          G(92, // KILLER MOVE
            move_equal(G(103, &stack[ply].moves[order_index]),
                       G(103, &stack[ply].killer)) *
                915) +
          G(92, // PREVIOUS BEST MOVE FIRST
            (move_equal(G(104, &stack[ply].best_move),
                        G(104, &stack[ply].moves[order_index]))
             << 30)) +
          G(92, // HISTORY HEURISTIC
            move_history[pos->flipped]
                        [stack[ply].moves[order_index].takes_piece]
                        [stack[ply].moves[order_index].from]
                        [stack[ply].moves[order_index].to]);
      if (order_move_score > move_score) {
        G(105, swapmoves(G(106, &stack[ply].moves[move_index]),
                         G(106, &stack[ply].moves[order_index]));)
        G(105, move_score = order_move_score;)
      }
    }

    // FORWARD FUTILITY PRUNING / DELTA PRUNING
    if (G(107, depth < 8) &&
        G(107,
          G(108, static_eval + 128 * depth) +
                  G(108, max_material[stack[ply].moves[move_index].promo]) +
                  G(108,
                    max_material[stack[ply].moves[move_index].takes_piece]) <
              alpha) &&
        G(107, moves_evaluated) && G(107, !in_check)) {
      break;
    }

    Position npos = *pos;
#ifdef FULL
    (*nodes)++;
#endif
    if (!makemove(H(45, 3, &npos), H(45, 3, &stack[ply].moves[move_index]))) {
      continue;
    }

    // PRINCIPAL VARIATION SEARCH
    i32 low = moves_evaluated == 0 ? -beta : -alpha - 1;
    moves_evaluated++;

    // LATE MOVE REDCUCTION
    i32 reduction = G(109, depth > 1) && G(109, moves_evaluated > 6)
                        ? G(110, (alpha == beta - 1)) + G(110, 1) +
                              G(110, !improving) + G(110, moves_evaluated / 11)
                        : 1;

    i32 score;
    while (true) {
      score = -search(H(91, 3, &npos), H(91, 3, ply + 1), H(91, 3, low),
                      H(91, 3, depth - reduction), H(92, 3, true),
#ifdef FULL
                      nodes,
#endif
                      H(92, 3, -alpha), H(92, 3, pos_history_count),
                      H(92, 3, stack));

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
      G(111, stack[ply].best_move = stack[ply].moves[move_index];)
      G(111, alpha = score;)
      G(111, tt_flag = Exact;)
      if (score >= beta) {
        tt_flag = Lower;
        assert(stack[ply].best_move.takes_piece ==
               piece_on(H(27, 8, pos), H(27, 8, stack[ply].best_move.to)));
        G(112, const i32 bonus = depth * depth;
          G(113,
            i32 *const this_hist =
                &move_history[pos->flipped][stack[ply].best_move.takes_piece]
                             [stack[ply].best_move.from]
                             [stack[ply].best_move.to];

            *this_hist += bonus - bonus * *this_hist / 1024;)
              G(
                  113, for (i32 prev_index = 0; prev_index < move_index;
                            prev_index++) {
                    const Move prev = stack[ply].moves[prev_index];
                    i32 *const prev_hist =
                        &move_history[pos->flipped][prev.takes_piece][prev.from]
                                     [prev.to];
                    *prev_hist -= bonus + bonus * *prev_hist / 1024;
                  }))
        G(
            112, if (stack[ply].best_move.takes_piece == None) {
              stack[ply].killer = stack[ply].best_move;
            })
        break;
      }
    }

    if (stack[ply].moves[move_index].takes_piece == None) {
      quiets_evaluated++;
    }

    // LATE MOVE PRUNING
    if (G(114, alpha == beta - 1) &&
        G(114, quiets_evaluated > 1 + depth * depth >> !improving) &&
        G(114, !in_check)) {
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

static void iteratively_deepen(
#ifdef FULL
    i32 maxdepth, u64 *nodes,
#endif
    H(115, 1, Position *const restrict pos),
    H(115, 1, SearchStack *restrict stack),
    H(115, 1, const i32 pos_history_count)) {
  start_time = get_time();
#ifdef FULL
  for (i32 depth = 1; depth < maxdepth; depth++) {
#else
  for (i32 depth = 1; depth < max_ply; depth++) {
#endif
    i32 score =
        search(H(91, 4, pos), H(91, 4, 0), H(91, 4, -inf), H(91, 4, depth),
               H(92, 4, false),
#ifdef FULL
               nodes,
#endif
               H(92, 4, inf), H(92, 4, pos_history_count), H(92, 4, stack));
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
    move_str(H(24, 2, pos->flipped), H(24, 2, &stack[0].best_move),
             H(24, 2, move_name));
    puts(move_name);
#endif

    if (elapsed > max_time / 16) {
      break;
    }
  }
  char move_name[8];
  move_str(H(24, 3, pos->flipped), H(24, 3, &stack[0].best_move),
           H(24, 3, move_name));
  putl("bestmove ");
  puts(move_name);
}

static void display_pos(Position *const pos) {
  Position npos = *pos;
  if (npos.flipped) {
    flip_pos(&npos);
  }
  for (i32 rank = 7; rank >= 0; rank--) {
    for (i32 file = 0; file < 8; file++) {
      i32 sq = rank * 8 + file;
      u64 bb = 1ULL << sq;
      i32 piece = piece_on(H(27, 9, &npos), H(27, 9, sq));
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

static const Position start_pos =
    (Position){.ep = 0,
               .colour = {0xFFFFull, 0xFFFF000000000000ull},
               .pieces = {0, 0xFF00000000FF00ull, 0x4200000000000042ull,
                          0x2400000000000024ull, 0x8100000000000081ull,
                          0x800000000000008ull, 0x1000000000000010ull},
               .castling = {true, true, true, true}};

#ifdef FULL
static void bench() {
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
  iteratively_deepen(20, &nodes, H(115, 2, &pos), H(115, 2, stack),
                     H(115, 2, pos_history_count));
  const u64 end = get_time();
  const i32 elapsed = end - start;
  const u64 nps = elapsed ? 1000 * nodes / elapsed : 0;
  printf("%i nodes %i nps\n", nodes, nps);
}
#endif

#if !defined(FULL) && defined(NOSTDLIB)
void _start() {
#else
static void run() {
#endif
#ifndef NOSTDLIB
  setvbuf(stdout, NULL, _IONBF, 0);
#endif

  G(116, // #ifdef LOWSTACK
         //  SearchStack *stack = malloc(sizeof(SearchStack) * 1024);
         // #else
    SearchStack stack[1024];
    // #endif
  )
  G(116, Position pos;)
  G(116, __builtin_memset(move_history, 0, sizeof(move_history));)
  G(116, init();)
  G(116, i32 pos_history_count;)
  G(116, char line[4096];)

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
      iteratively_deepen(max_ply, &nodes, H(115, 3, &pos), H(115, 3, stack),
                         H(115, 3, pos_history_count));
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
    G(117, if (line[0] == 'i') { puts("readyok"); })
    else G(117, if (line[0] == 'q') { exit_now(); }) else if (line[0] == 'p') {
      G(118, pos = start_pos;)
      G(118, pos_history_count = 0;)
      while (true) {
        const bool line_continue = getl(line);

#if FULL
        if (!strcmp(line, "fen")) {
          getl(line);
          get_fen(&pos, line);
        }
#endif

        const i32 num_moves =
            movegen(H(57, 4, &pos), H(57, 4, stack[0].moves), H(57, 4, false));
        for (i32 i = 0; i < num_moves; i++) {
          char move_name[8];
          move_str(H(24, 4, pos.flipped), H(24, 4, &stack[0].moves[i]),
                   H(24, 4, move_name));
          assert(move_string_equal(line, move_name) ==
                 !strcmp(line, move_name));
          if (move_string_equal(G(119, line), G(119, move_name))) {
            stack[pos_history_count].position_hash = get_hash(&pos);
            pos_history_count++;
            if (stack[0].moves[i].takes_piece != None) {
              pos_history_count = 0;
            }
            makemove(H(45, 4, &pos), H(45, 4, &stack[0].moves[i]));
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
      iteratively_deepen(max_ply, &nodes, H(115, 4, &pos), H(115, 4, stack),
                         H(115, 4, pos_history_count));
#else
      for (i32 i = 0; i < (pos.flipped ? 4 : 2); i++) {
        getl(line);
        max_time = atoi(line) / 2;
      }
      iteratively_deepen(H(115, 5, &pos), H(115, 5, stack),
                         H(115, 5, pos_history_count));
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
