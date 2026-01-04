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
    3, [[nodiscard]] S(1) u32 atoi(const char *restrict string) {
      u32 result = 0;
      while (*string)
        result = result * 10 + *string++ - '0';
      return result;
    })

G(
    3, // Non-standard, gets but a word instead of a line
    S(1) bool getl(char *restrict string) {
      while (true) {
        const int result = _sys(H(2, 3, stdin), H(2, 3, 1),
                                H(2, 3, (ssize_t)string), H(2, 3, 0));

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

typedef struct [[nodiscard]] {
  ssize_t tv_sec;  // seconds
  ssize_t tv_nsec; // nanoseconds
} timespec;

[[nodiscard]] S(1) u64 get_time() {
  timespec ts;
  ssize_t ret; // Unused
  asm volatile("syscall"
               : "=a"(ret)
               : "0"(228), "D"(1), "S"(&ts)
               : "rcx", "r11", "memory");
  return G(4, ts.tv_nsec) + G(4, G(5, ts.tv_sec) * G(5, 1000 * 1000 * 1000ULL));
}

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
  G(6, u8 from; u8 to;)
  G(6, u8 promo;)
  G(6, u8 takes_piece;)
} Move;

typedef struct [[nodiscard]] {
  G(7, u64 ep;)
  G(7, u64 pieces[7];)
  G(7, u64 colour[2];)
  G(8, bool flipped;)
  G(8, bool castling[4];)
  G(8, u8 padding[11];)
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
    9,
    [[nodiscard]] S(1) bool move_string_equal(G(10, const char *restrict rhs),
                                              G(10, const char *restrict lhs)) {
      return (G(11, *(const u64 *)rhs) ^ G(11, *(const u64 *)lhs)) << 24 == 0;
    })

G(
    9, [[nodiscard]] S(1)
           u64 flip_bb(const u64 bb) { return __builtin_bswap64(bb); })
G(9, [[nodiscard]] S(1) i32 lsb(u64 bb) { return __builtin_ctzll(bb); })

G(
    9, [[nodiscard]] S(1)
           u64 shift(H(12, 1, const i32 shift), H(12, 1, const u64 mask),
                     H(12, 1, const u64 bb)) {
             return shift > 0 ? G(13, bb << shift) & G(13, mask)
                              : G(14, bb >> -shift) & G(14, mask);
           })

G(
    9, [[nodiscard]] S(1)
           i32 count(const u64 bb) { return __builtin_popcountll(bb); })

G(
    15, [[nodiscard]] S(1) u64 west(const u64 bb) {
      return G(16, bb >> 1) & G(16, ~0x8080808080808080ull);
    })

G(15, [[nodiscard]] S(1) u64 north(const u64 bb) { return bb << 8; })

G(15, [[nodiscard]] S(1) u64 south(const u64 bb) { return bb >> 8; })

G(
    15, [[nodiscard]] S(1) u64 east(const u64 bb) {
      return G(17, bb << 1) & G(17, ~0x101010101010101ull);
    })

G(
    18, [[nodiscard]] S(1) u64 southeast(const u64 bb) {
      return G(19, G(20, east)(G(20, south)(bb)));
      return G(19, shift(H(12, 2, -7), H(12, 2, ~0x101010101010101ull),
                         H(12, 2, bb)));
    })

G(
    18, [[nodiscard]] S(1) u64 northeast(const u64 bb) {
      return G(21, G(22, east)(G(22, north)(bb)));
      return G(21, shift(H(12, 3, 9), H(12, 3, ~0x101010101010101ull),
                         H(12, 3, bb)));
    })

G(
    18, [[nodiscard]] S(1) u64 northwest(const u64 bb) {
      return G(23, shift(H(12, 4, 7), H(12, 4, ~0x8080808080808080ull),
                         H(12, 4, bb)));
      return G(23, G(24, west)(G(24, north)(bb)));
    })

G(
    18, [[nodiscard]] S(1) u64 southwest(const u64 bb) {
      return G(25, G(26, west)(G(26, south)(bb)));
      return G(25, shift(H(12, 5, -9), H(12, 5, ~0x8080808080808080ull),
                         H(12, 5, bb)));
    })

G(27, S(0) u64 diag_mask[64];)

G(
    27, [[nodiscard]] S(1)
            u64 ray(H(28, 1, const u64 blockers), H(28, 1, const u64 mask),
                    H(28, 1, const u64 bb), H(28, 1, const i32 shift_by)) {
              u64 result =
                  shift(H(12, 6, shift_by), H(12, 6, mask), H(12, 6, bb));
              for (i32 i = 5; i >= 0; i--) {
                result |= shift(H(12, 7, shift_by), H(12, 7, mask),
                                H(12, 7, result & ~blockers));
              }
              return result;
            })

G(
    27, [[nodiscard]] S(0) u64 xattack(H(29, 1, const u64 dir_mask),
                                       H(29, 1, const u64 bb),
                                       H(29, 1, const u64 blockers)) {
      return G(30, dir_mask) &
             G(30, (G(31, (G(32, blockers) & G(32, dir_mask)) - bb) ^
                    G(31, flip_bb(flip_bb(G(33, blockers) & G(33, dir_mask)) -
                                  flip_bb(bb)))));
    })

G(
    34, [[nodiscard]] S(0) u64 bishop(H(35, 1, const u64 blockers),
                                      H(35, 1, const u64 bb)) {
      assert(count(bb) == 1);
      const i32 sq = lsb(bb);
      return G(36, xattack(H(29, 2, diag_mask[sq]), H(29, 2, bb),
                           H(29, 2, blockers))) |
             G(36, xattack(H(29, 3, flip_bb(diag_mask[G(37, sq) ^ G(37, 56)])),
                           H(29, 3, bb), H(29, 3, blockers)));
    })

G(
    34, [[nodiscard]] S(1) u64 rook(H(38, 1, const u64 blockers),
                                    H(38, 1, const u64 bb)) {
      assert(count(bb) == 1);
      return G(39, xattack(H(29, 4, bb ^ 0x101010101010101ULL << lsb(bb) % 8),
                           H(29, 4, bb), H(29, 4, blockers))) |
             G(39, // East
               ray(H(28, 2, blockers), H(28, 2, ~0x101010101010101ull),
                   H(28, 2, bb), H(28, 2, 1))) |
             G(39, // West
               ray(H(28, 3, blockers), H(28, 3, ~0x8080808080808080ull),
                   H(28, 3, bb), H(28, 3, -1)));
    })

G(
    34, [[nodiscard]] S(0) u64 knight(const u64 bb) {
      G(40, const u64 east_bb = east(bb);)
      G(40, const u64 west_bb = west(bb);)
      G(41, const u64 horizontal1 = G(42, west_bb) | G(42, east_bb);)
      G(41,
        const u64 horizontal2 = G(43, east(east_bb)) | G(43, west(west_bb));)
      return G(44, horizontal1 << 16) | G(44, horizontal1 >> 16) |
             G(44, horizontal2 >> 8) | G(44, horizontal2 << 8);
    })

G(
    34, [[nodiscard]] S(0) u64 king(const u64 bb) {
      const u64 vertical = G(45, south(bb)) | G(45, north(bb));
      const u64 vertical_inclusive = G(46, bb) | G(46, vertical);
      return G(47, vertical) | G(47, east(vertical_inclusive)) |
             G(47, west(vertical_inclusive));
    })

G(
    48,
    S(1) void swapu32(G(49, u32 *const rhs), G(49, u32 *const lhs)) {
      const u32 temp = *lhs;
      *lhs = *rhs;
      *rhs = temp;
    }

    S(1) void swapmoves(G(50, Move *const lhs), G(50, Move *const rhs)) {
      swapu32(G(51, (u32 *)lhs), G(51, (u32 *)rhs));
    })

G(
    48, [[nodiscard]] S(1) bool move_equal(G(52, Move *const rhs),
                                           G(52, Move *const lhs)) {
      return G(53, *(u32 *)lhs) == G(53, *(u32 *)rhs);
    })

G(
    48, S(1) void move_str(H(54, 1, char *restrict str),
                           H(54, 1, const Move *restrict move),
                           H(54, 1, const i32 flip)) {
      assert(move->from >= 0);
      assert(move->from < 64);
      assert(move->to >= 0);
      assert(move->to < 64);
      assert(move->from != move->to);
      assert(move->promo == None || move->promo == Knight ||
             move->promo == Bishop || move->promo == Rook ||
             move->promo == Queen);

      G(55, str[5] = '\0';)

      G(55, str[4] = "\0\0nbrq"[move->promo];)
      G(
          55, // Hack to save bytes, technically UB but works on GCC 14.2
          for (i32 i = 0; i < 2; i++) {
            G(56, str[i * 2] = 'a' + (&move->from)[i] % 8;)
            G(56, str[i * 2 + 1] = '1' + ((&move->from)[i] / 8 ^ 7 * flip);)
          })
    })

G(
    48, [[nodiscard]] S(1)
            i32 piece_on(H(57, 1, const Position *const restrict pos),
                         H(57, 1, const i32 sq)) {
              assert(sq >= 0);
              assert(sq < 64);
              for (i32 i = Pawn; i <= King; ++i) {
                if (G(58, pos->pieces[i]) & G(58, 1ull << sq)) {
                  return i;
                }
              }
              return None;
            })

G(
    59,
    [[nodiscard]] S(1) i32 is_attacked(H(60, 1,
                                         const Position *const restrict pos),
                                       H(60, 1, const u64 bb)) {
      assert(count(bb) == 1);
      const u64 theirs = pos->colour[1];
      G(61, const u64 pawns = theirs & pos->pieces[Pawn];)
      G(61, const u64 blockers = theirs | pos->colour[0];)
      return G(62, G(63, (G(64, southwest(pawns)) | G(64, southeast(pawns)))) &
                       G(63, bb)) ||
             G(62, G(65, bishop(H(35, 2, blockers), H(35, 2, bb))) &
                       G(65, theirs) &
                       G(65, (pos->pieces[Bishop] | pos->pieces[Queen]))) ||
             G(62,
               G(66, king(bb)) & G(66, theirs) & G(66, pos->pieces[King])) ||
             G(62, G(67, knight(bb)) & G(67, theirs) &
                       G(67, pos->pieces[Knight])) ||
             G(62, G(68, rook(H(38, 2, blockers), H(38, 2, bb))) &
                       G(68, (pos->pieces[Rook] | pos->pieces[Queen])) &
                       G(68, theirs));
    })

G(
    59, S(0) void flip_pos(Position *const restrict pos) {
      G(
          69, // Hack to flip the first 10 bitboards in Position.
              // Technically UB but works in GCC 14.2
          u64 *pos_ptr = (u64 *)pos;
          for (i32 i = 0; i < 10; i++) { pos_ptr[i] = flip_bb(pos_ptr[i]); })
      G(69, pos->colour[0] ^= pos->colour[1]; pos->colour[1] ^= pos->colour[0];
        pos->colour[0] ^= pos->colour[1];)

      G(69, u32 *c = (u32 *)pos->castling;
        *c = G(70, (*c >> 16)) | G(70, (*c << 16));)
      G(69, pos->flipped ^= 1;)
    })

G(
    59, [[nodiscard]] S(1) u64 get_mobility(H(71, 1, const Position *pos),
                                            H(71, 1, const i32 sq),
                                            H(71, 1, const i32 piece)) {
      u64 moves = 0;
      const u64 bb = 1ULL << sq;
      G(72, if (piece == Knight) { moves = knight(bb); })
      else G(72, if (piece == King) { moves = king(bb); }) else {
        const u64 blockers = G(73, pos->colour[1]) | G(73, pos->colour[0]);
        G(
            74, if (piece != Rook) {
              moves |= bishop(H(35, 3, blockers), H(35, 3, bb));
            })
        G(
            74, if (piece != Bishop) {
              moves |= rook(H(38, 3, blockers), H(38, 3, bb));
            })
      }
      return moves;
    })

S(0) i32 find_in_check(const Position *restrict pos) {
  return is_attacked(
      H(60, 2, pos),
      H(60, 2, G(75, pos->colour[0]) & G(75, pos->pieces[King])));
}

G(
    76,
    S(1) Move *generate_piece_moves(H(77, 1, const u64 to_mask),
                                    H(77, 1, Move *restrict movelist),
                                    H(77, 1, const Position *restrict pos)) {
      for (i32 piece = Knight; piece <= King; piece++) {
        assert(piece == Knight || piece == Bishop || piece == Rook ||
               piece == Queen || piece == King);
        u64 copy = G(78, pos->colour[0]) & G(78, pos->pieces[piece]);
        while (copy) {
          const u8 from = lsb(copy);
          assert(from >= 0);
          assert(from < 64);
          G(79, copy &= copy - 1;)

          G(79, u64 moves = G(80, to_mask) &
                            G(80, get_mobility(H(71, 2, pos), H(71, 2, from),
                                               H(71, 2, piece)));)

          while (moves) {
            const u8 to = lsb(moves);
            assert(to >= 0);
            assert(to < 64);

            G(81, moves &= moves - 1;)
            G(81, *movelist++ = ((Move){
                      .from = from,
                      .to = to,
                      .promo = None,
                      .takes_piece = piece_on(H(57, 2, pos), H(57, 2, to))});)
          }
        }
      }

      return movelist;
    })

G(
    76, S(0) i32 makemove(H(82, 1, Position *const restrict pos),
                          H(82, 1, const Move *const restrict move)) {
      assert(move->from >= 0);
      assert(move->from < 64);
      assert(move->to >= 0);
      assert(move->to < 64);
      assert(move->from != move->to);
      assert(move->promo == None || move->promo == Knight ||
             move->promo == Bishop || move->promo == Rook ||
             move->promo == Queen);
      assert(move->takes_piece != King);
      assert(move->takes_piece == piece_on(H(57, 3, pos), H(57, 3, move->to)));

      G(83, const u64 from = 1ull << move->from;)
      G(83, const u64 to = 1ull << move->to;)
      G(84, const i32 piece = piece_on(H(57, 4, pos), H(57, 4, move->from));
        assert(piece != None);)
      G(84, const u64 mask = G(85, from) | G(85, to);)

      G(86, pos->pieces[piece] ^= mask;)

      G(
          86, // Castling
          if (piece == King) {
            const u64 bb = move->to - move->from == 2   ? 0xa0
                           : move->from - move->to == 2 ? 0x9
                                                        : 0;
            G(87, pos->pieces[Rook] ^= bb;)
            G(87, pos->colour[0] ^= bb;)
          })

      // Move the piece
      G(86, pos->colour[0] ^= mask;)
      G(
          86, // Captures
          if (move->takes_piece != None) {
            G(88, pos->colour[1] ^= to;)
            G(88, pos->pieces[move->takes_piece] ^= to;)
          })

      // En passant
      if (G(89, piece == Pawn) && G(89, to == pos->ep)) {
        G(90, pos->colour[1] ^= to >> 8;)
        G(90, pos->pieces[Pawn] ^= to >> 8;)
      }
      pos->ep = 0;

      G(
          91, // Pawn double move
          if (G(92, move->to - move->from == 16) && G(92, piece == Pawn)) {
            pos->ep = to >> 8;
          })

      G(
          91, // Promotions
          if (move->promo != None) {
            G(93, pos->pieces[Pawn] ^= to;)
            G(93, pos->pieces[move->promo] ^= to;)
          })

      G(91, // Update castling permissions
        const u64 oppMask = mask >> 56;
        G(94, pos->castling[3] &= !(oppMask & 0x11);)
            G(94, pos->castling[1] &= !(mask & 0x11);)
                G(94, pos->castling[0] &= !(mask & 0x90);)
                    G(94, pos->castling[2] &= !(oppMask & 0x90);))

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
    76, S(0) Move *generate_pawn_moves(H(95, 1, const i32 offset),
                                       H(95, 1, Move *restrict movelist),
                                       H(95, 1, u64 to_mask),
                                       H(95, 1, const Position *const pos)) {
      while (to_mask) {
        const u8 to = lsb(to_mask);
        to_mask &= to_mask - 1;
        const u8 from = G(96, to) + G(96, offset);
        assert(from >= 0);
        assert(from < 64);
        assert(to >= 0);
        assert(to < 64);
        assert(piece_on(H(57, 5, pos), H(57, 5, from)) == Pawn);
        const u8 takes = piece_on(H(57, 6, pos), H(57, 6, to));
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

[[nodiscard]] S(1) i32 movegen(H(97, 1, const Position *const restrict pos),
                               H(97, 1, Move *restrict movelist),
                               H(97, 1, const i32 only_captures)) {

  G(98, const u64 all = G(99, pos->colour[0]) | G(99, pos->colour[1]);)
  G(98, const Move *start = movelist;)
  G(98, const u64 to_mask = only_captures ? pos->colour[1] : ~pos->colour[0];)
  G(
      100, // PAWN PROMOTIONS
      if (!only_captures) {
        movelist = generate_pawn_moves(
            H(95, 2, -16), H(95, 2, movelist),
            H(95, 2,
              G(101, north(G(102, north(G(103, pos->colour[0]) &
                                        G(103, pos->pieces[Pawn]) &
                                        G(103, 0xFF00))) &
                           G(102, ~all))) &
                  G(101, ~all)),
            H(95, 2, pos));
      })
  G(100, // PAWN DOUBLE MOVES
    movelist = generate_pawn_moves(
        H(95, 3, -8), H(95, 3, movelist),
        H(95, 3,
          north(G(104, G(105, pos->colour[0]) & G(105, pos->pieces[Pawn]))) &
              G(104, ~all) &
              G(104, (only_captures ? 0xFF00000000000000ull : ~0ull))),
        H(95, 3, pos));)
  G(100, // PAWN WEST CAPTURES
    movelist = generate_pawn_moves(
        H(95, 4, -7), H(95, 4, movelist),
        H(95, 4,
          G(106,
            northwest(G(107, pos->colour[0]) & G(107, pos->pieces[Pawn]))) &
              G(106, (G(108, pos->colour[1]) | G(108, pos->ep)))),
        H(95, 4, pos));)
  G(100, // PAWN EAST CAPTURES
    movelist = generate_pawn_moves(
        H(95, 5, -9), H(95, 5, movelist),
        H(95, 5,
          G(109,
            northeast(G(110, pos->colour[0]) & G(110, pos->pieces[Pawn]))) &
              G(109, (G(111, pos->colour[1]) | G(111, pos->ep)))),
        H(95, 5, pos));)
  G(
      100, // SHORT CASTLE
      if (G(115, !only_captures) && G(115, pos->castling[1]) &&
          G(115, !(G(116, all) & G(116, 0xEull))) &&
          G(117, !is_attacked(H(60, 5, pos), H(60, 5, 1ULL << 4))) &&
          G(117, !is_attacked(H(60, 6, pos), H(60, 6, 1ULL << 3)))) {
        *movelist++ =
            (Move){.from = 4, .to = 2, .promo = None, .takes_piece = None};
      })
  G(
      100, // LONG CASTLE
      if (G(112, !only_captures) && G(112, pos->castling[0]) &&
          G(112, !(G(113, all) & G(113, 0x60ull))) &&
          G(114, !is_attacked(H(60, 3, pos), H(60, 3, 1ULL << 4))) &&
          G(114, !is_attacked(H(60, 4, pos), H(60, 4, 1ULL << 5)))) {
        *movelist++ =
            (Move){.from = 4, .to = 6, .promo = None, .takes_piece = None};
      })
  movelist = generate_piece_moves(H(77, 2, to_mask), H(77, 2, movelist),
                                  H(77, 2, pos));

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
      movegen(H(97, 2, pos), H(97, 2, moves), H(97, 2, false));

  for (i32 i = 0; i < num_moves; ++i) {
    Position npos = *pos;

    // Check move legality
    if (!makemove(H(82, 2, &npos), H(82, 2, &moves[i]))) {
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
  H(118, 1,
    H(119, 1, i8 pawn_attacked_penalty[2];) H(119, 1, i8 tempo;)
        H(119, 1, i8 open_files[6];) H(119, 1, i8 pst_file[48];)
            H(119, 1, i8 passed_blocked_pawns[6];) H(119, 1, i8 mobilities[5];))
  H(118, 1,
    H(120, 1, i8 king_attacks[5];) H(120, 1, i8 pst_rank[48];)
        H(120, 1, i8 passed_pawns[6];) H(120, 1, i8 bishop_pair;)
            H(120, 1, i8 protected_pawn;) H(120, 1, i8 phalanx_pawn;))
} EvalParams;

typedef struct [[nodiscard]] __attribute__((packed)) {
  i32 material[6];
  H(118, 2,
    H(119, 2, i32 pawn_attacked_penalty[2];) H(119, 2, i32 tempo;)
        H(119, 2, i32 open_files[6];) H(119, 2, i32 pst_file[48];)
            H(119, 2, i32 passed_blocked_pawns[6];)
                H(119, 2, i32 mobilities[5];))
  H(118, 2,
    H(120, 2, i32 king_attacks[5];) H(120, 2, i32 pst_rank[48];)
        H(120, 2, i32 passed_pawns[6];) H(120, 2, i32 bishop_pair;)
            H(120, 2, i32 protected_pawn;) H(120, 2, i32 phalanx_pawn;))

} EvalParamsMerged;

typedef struct [[nodiscard]] __attribute__((packed)) {
  i8 phases[6];
  G(121, EvalParams mg;)
  G(121, EvalParams eg;)
} EvalParamsInitial;

G(122, S(0) EvalParamsMerged eval_params;)

G(122,
  __attribute__((aligned(8))) S(1)
      const EvalParamsInitial initial_params = {.phases = {0, 0, 1, 1, 2, 4},
                                                .mg = {.material = {0, 67, 289,
                                                                    272, 358,
                                                                    783},
                                                       .pst_rank =
                                                           {
                                                               0,   -15, -15,
                                                               -7,  9,   28,
                                                               88,  0, // Pawn
                                                               -31, -19, 4,
                                                               16,  32,  55,
                                                               24,  -81, // Knight
                                                               -9,  4,   11,
                                                               11,  15,  21,
                                                               -2,  -52, // Bishop
                                                               1,   -11, -16,
                                                               -20, -2,  17,
                                                               13,  17, // Rook
                                                               13,  16,  9,
                                                               -2,  -7,  0,
                                                               -20, -7, // Queen
                                                               -8,  -1,  -27,
                                                               -33, -4,  43,
                                                               50,  46, // King
                                                           },
                                                       .pst_file =
                                                           {
                                                               -17, -12, -9,
                                                               1,   6,   20,
                                                               17,  -7, // Pawn
                                                               -31, -10, 5,
                                                               16,  14,  15,
                                                               3,   -12, // Knight
                                                               -11, 2,   4,
                                                               0,   4,   -1,
                                                               7,   -4, // Bishop
                                                               -6,  -7,  1,
                                                               9,   12,  1,
                                                               -2,  -8, // Rook
                                                               -9,  -6,  -1,
                                                               1,   2,   0,
                                                               7,   6, // Queen
                                                               -17, 24,  -5,
                                                               -54, -21, -38,
                                                               20,  1, // King
                                                           },
                                                       .mobilities = {0, 4, 2,
                                                                      1, -10},
                                                       .king_attacks = {0, 14,
                                                                        19, 14,
                                                                        0},
                                                       .open_files = {13, -11,
                                                                      -8, 20,
                                                                      -2, -29},
                                                       .passed_pawns = {-6, -10,
                                                                        -6, 10,
                                                                        37, 88},
                                                       .passed_blocked_pawns =
                                                           {7, -1, 3, 13, 18,
                                                            -25},
                                                       .protected_pawn = 17,
                                                       .phalanx_pawn = 12,
                                                       .bishop_pair = 25,
                                                       .pawn_attacked_penalty =
                                                           {-16, -128},
                                                       .tempo = 17},
                                                .eg = {.material = {0, 84, 407,
                                                                    394, 716,
                                                                    1339},
                                                       .pst_rank =
                                                           {
                                                               0,   -2,  -8,
                                                               -7,  -2,  19,
                                                               100, 0, // Pawn
                                                               -41, -19, -3,
                                                               22,  28,  10,
                                                               0,   2, // Knight
                                                               -13, -12, -3,
                                                               2,   8,   4,
                                                               3,   11, // Bishop
                                                               -20, -22, -16,
                                                               2,   12,  12,
                                                               21,  11, // Rook
                                                               -61, -48, -24,
                                                               7,   32,  30,
                                                               43,  22, // Queen
                                                               -45, -9,  9,
                                                               25,  34,  28,
                                                               11,  -44, // King
                                                           },
                                                       .pst_file =
                                                           {
                                                               13,  12,  0,
                                                               -10, -4,  -3,
                                                               -2,  -5, // Pawn
                                                               -25, -5,  11,
                                                               19,  19,  9,
                                                               -4,  -25, // Knight
                                                               -7,  0,   0,
                                                               4,   6,   4,
                                                               1,   -8, // Bishop
                                                               1,   5,   6,
                                                               -1,  -6,  1,
                                                               -1,  -6, // Rook
                                                               -20, -5,  3,
                                                               10,  14,  12,
                                                               -4,  -11, // Queen
                                                               -24, -2,  14,
                                                               28,  21,  24,
                                                               0,   -33, // King
                                                           },
                                                       .mobilities = {2, 5, 4,
                                                                      2, 0},
                                                       .king_attacks = {0, -3,
                                                                        -6, 8,
                                                                        0},
                                                       .open_files = {24, -5, 7,
                                                                      9, 25, 9},
                                                       .passed_pawns =
                                                           {10, 16, 40, 67, 113,
                                                            100},
                                                       .passed_blocked_pawns =
                                                           {-12, -14, -40, -71,
                                                            -121, -126},
                                                       .protected_pawn = 16,
                                                       .phalanx_pawn = 14,
                                                       .bishop_pair = 62,
                                                       .pawn_attacked_penalty =
                                                           {-10, -128},
                                                       .tempo = 7}};)

G(
    122,
    [[nodiscard]] S(1) i32 combine_eval_param(H(123, 1, const i32 eg_val),
                                              H(123, 1, const i32 mg_val)) {
      return G(124, mg_val) + G(124, (eg_val << 16));
    })

S(0) i32 eval(Position *const restrict pos) {
  G(125, i32 score = eval_params.tempo;)
  G(125, i32 phase = 0;)

  for (i32 c = 0; c < 2; c++) {

    G(126, const u64 opp_king_zone =
               king(G(127, pos->colour[1]) & G(127, pos->pieces[King]));)

    G(126,
      const u64 own_pawns = G(128, pos->colour[0]) & G(128, pos->pieces[Pawn]);)
    G(126,
      const u64 opp_pawns = G(129, pos->pieces[Pawn]) & G(129, pos->colour[1]);
      const u64 attacked_by_pawns =
          G(130, southwest(opp_pawns)) | G(130, southeast(opp_pawns));
      G(131, // PROTECTED PAWNS
        score -= G(133, eval_params.protected_pawn) *
                 G(133, count(G(134, opp_pawns) & G(134, attacked_by_pawns)));)
          G(131, const u64 no_passers =
                     G(132, opp_pawns) | G(132, attacked_by_pawns);)
              G(131, // PHALANX PAWNS
                score -=
                G(135, eval_params.phalanx_pawn) *
                G(135, count(G(136, opp_pawns) & G(136, west(opp_pawns))));))
    G(
        126, // BISHOP PAIR
        if (count(G(137, pos->pieces[Bishop]) & G(137, pos->colour[0])) > 1) {
          score += eval_params.bishop_pair;
        })

    for (i32 p = Pawn; p <= King; p++) {
      u64 copy = G(138, pos->colour[0]) & G(138, pos->pieces[p]);
      while (copy) {
        const i32 sq = lsb(copy);
        G(139, const i32 rank = sq >> 3;)
        G(139, const i32 file = G(140, sq) & G(140, 7);)
        G(139, phase += initial_params.phases[p];)
        G(139, const u64 in_front = 0x101010101010101ULL << sq;)
        G(139, copy &= copy - 1;)
        G(139, const u64 piece_bb = 1ULL << sq;)
        G(
            95, // OPEN FILES / DOUBLED PAWNS
            if ((G(147, north(in_front)) & G(147, own_pawns)) == 0) {
              score += eval_params.open_files[p - 1];
            })

        G(95, // MATERIAL
          score += eval_params.material[p];)
        G(
            95, // PASSED PAWNS
            if (G(141, p == Pawn) &&
                G(141, !(G(142, in_front) & G(142, no_passers)))) {
              G(
                  143, if (G(144, north(piece_bb)) & G(144, pos->colour[1])) {
                    score += eval_params.passed_blocked_pawns[rank - 1];
                  })

              G(143, score += eval_params.passed_pawns[rank - 1];)
            })

        G(95, // SPLIT PIECE-SQUARE TABLES FOR RANK
          score +=
          eval_params
              .pst_rank[G(148, G(149, (p - 1)) * G(149, 8)) + G(148, rank)];)

        G(95, // SPLIT PIECE-SQUARE TABLES FOR FILE
          score +=
          eval_params
              .pst_file[G(145, G(146, (p - 1)) * G(146, 8)) + G(145, file)];)

        G(
            95, if (p > Pawn) {
              G(
                  150, // PIECES ATTACKED BY PAWNS
                  if (G(151, piece_bb) & G(151, no_passers)) {
                    score += eval_params.pawn_attacked_penalty[c];
                  })

              G(150, const u64 mobility =
                         get_mobility(H(71, 3, pos), H(71, 3, sq), H(71, 3, p));

                G(153, // MOBILITY
                  score +=
                  G(154, count(G(155, ~pos->colour[0]) & G(155, mobility) &
                               G(155, ~attacked_by_pawns))) *
                  G(154, eval_params.mobilities[p - 2]);)

                    G(153, // KING ATTACKS
                      score +=
                      G(156, count(G(157, mobility) & G(157, opp_king_zone))) *
                      G(156, eval_params.king_attacks[p - 2]);))
            })
      }
    }

    G(77, flip_pos(pos);)
    G(77, score = -score;)
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
  G(119, Move best_move;)
  G(119, u64 position_hash;)
  G(119, i32 num_moves;)
  G(119, Move killer;)
  G(119, i32 static_eval;)
} SearchStack;

typedef struct [[nodiscard]] __attribute__((packed)) {
  G(161, i16 score;)
  G(161, i8 depth;)
  G(161, u8 flag;)
  G(161, Move move;)
  G(161, u16 partial_hash;)
} TTEntry;
_Static_assert(sizeof(TTEntry) == 10);

typedef struct __attribute__((aligned(16))) ThreadDataStruct {
  void (*entry)(struct ThreadDataStruct *);
#ifdef FULL
  i32 thread_id;
  u64 nodes;
#endif
  G(162, u64 max_time;)
  G(162, Position pos;)
  G(162, SearchStack stack[1024];)
  G(162, i32 move_history[2][6][64][64];)
} ThreadData;

enum { tt_length = 1 << 23 }; // 80MB
enum { Upper = 0, Lower = 1, Exact = 2 };
enum { max_ply = 96 };
enum { mate = 31744, inf = 32256 };
enum { thread_count = 1 };
enum { thread_stack_size = 1024 * 1024 };

G(163, S(1) u64 start_time;)
G(163, S(1) TTEntry tt[tt_length];)
G(163, __attribute__((aligned(4096))) u8
           thread_stacks[thread_count][thread_stack_size];)
G(163, S(1) volatile bool stop;)

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
i32 search(
#ifdef FULL
    u64 *nodes,
#endif
    H(165, 1, Position *const pos), H(165, 1, const i32 beta),
    H(165, 1, const i32 ply), H(165, 1, i32 depth), H(165, 1, ThreadData *data),
    H(165, 1, const bool do_null), H(165, 1, i32 alpha)) {
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
  for (i32 i = G(166, ply); G(167, i >= 0) && G(167, do_null); i -= 2) {
    if (G(168, tt_hash) == G(168, stack[i].position_hash)) {
      return 0;
    }
  }

  // TT PROBING
  G(169, TTEntry *tt_entry = &tt[tt_hash % tt_length];)
  G(169, const u16 tt_hash_partial = tt_hash / tt_length;)
  G(169, stack[ply].best_move = (Move){0};)
  if (G(170, tt_entry->partial_hash) == G(170, tt_hash_partial)) {
    stack[ply].best_move = tt_entry->move;

    // TT PRUNING
    if (G(171, tt_entry->depth >= depth) &&
        G(171, G(172, tt_entry->flag) != G(172, tt_entry->score <= alpha)) &&
        G(171, G(173, alpha) == G(173, beta - 1))) {
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
  if (G(174, G(175, tt_entry->partial_hash) == G(175, tt_hash_partial)) &&
      G(174, G(176, tt_entry->flag) != G(176, static_eval) > tt_entry->score)) {
    static_eval = tt_entry->score;
  }

  // QUIESCENCE
  if (G(177, static_eval > alpha) && G(177, in_qsearch)) {
    if (static_eval >= beta) {
      return static_eval;
    }
    alpha = static_eval;
  }

  if (G(178, !in_check) && G(178, G(179, alpha) == G(179, beta - 1))) {
    if (G(180, !in_qsearch) && G(180, depth < 9)) {

      G(181, {
        // REVERSE FUTILITY PRUNING
        if (static_eval - G(182, 61) * G(182, (depth - improving)) >= beta) {
          return static_eval;
        }
      })

      G(181, // RAZORING
        in_qsearch =
            G(183, static_eval) + G(183, G(184, 117) * G(184, depth)) <= alpha;)
    }

    // NULL MOVE PRUNING
    if (G(185, depth > 2) && G(185, static_eval >= beta) && G(185, do_null)) {
      Position npos = *pos;
      G(186, flip_pos(&npos);)
      G(186, npos.ep = 0;)
      const i32 score = -search(
#ifdef FULL
          nodes,
#endif
          H(165, 2, &npos), H(165, 2, -alpha), H(165, 2, ply + 1),
          H(165, 2, depth - G(187, 4) - G(187, depth / 4)), H(165, 2, data),
          H(165, 2, false), H(165, 2, -beta));
      if (score >= beta) {
        return score;
      }
    }
  }

  G(188, Move moves[max_moves];
    stack[ply].num_moves =
        movegen(H(97, 3, pos), H(97, 3, moves), H(97, 3, in_qsearch));)
  G(188, stack[G(189, ply) + G(189, 2)].position_hash = tt_hash;)
  G(188, i32 best_score = in_qsearch ? static_eval : -inf;)
  G(188, u8 tt_flag = Upper;)
  G(188, i32 quiets_evaluated = 0;)
  G(188, i32 moves_evaluated = 0;)

  for (i32 move_index = 0; move_index < stack[ply].num_moves; move_index++) {
    // MOVE ORDERING
    G(190, i32 move_score = ~0x1010101LL;)
    G(190, i32 best_index = 0;)
    for (i32 order_index = move_index; order_index < stack[ply].num_moves;
         order_index++) {
      assert(
          stack[ply].moves[order_index].takes_piece ==
          piece_on(H(57, 7, pos), H(57, 7, stack[ply].moves[order_index].to)));
      const i32 order_move_score =
          G(165, // PREVIOUS BEST MOVE FIRST
            (move_equal(G(191, &stack[ply].best_move),
                        G(191, &moves[order_index]))
             << 30)) +
          G(165, // MOST VALUABLE VICTIM
            G(192, moves[order_index].takes_piece) * G(192, 663)) +
          G(165, // KILLER MOVE
            G(193, move_equal(G(194, &moves[order_index]),
                              G(194, &stack[ply].killer))) *
                G(193, 829)) +
          G(165, // HISTORY HEURISTIC
            move_history[pos->flipped][moves[order_index].takes_piece]
                        [moves[order_index].from][moves[order_index].to]);
      if (order_move_score > move_score) {
        G(195, best_index = order_index;)
        G(195, move_score = order_move_score;)
      }
    }

    swapmoves(G(196, &moves[move_index]), G(196, &moves[best_index]));

    G(
        197, // MOVE SCORE PRUNING
        if (G(198, moves_evaluated) &&
            G(198, move_score < G(199, -125) * G(199, depth))) { break; })

    G(
        197, // FORWARD FUTILITY PRUNING / DELTA PRUNING
        if (G(200, depth < 8) &&
            G(200,
              G(201, static_eval) + G(201, G(202, 146) * G(202, depth)) +
                      G(201,
                        initial_params.eg.material[moves[move_index].promo]) +
                      G(201, initial_params.eg
                                 .material[moves[move_index].takes_piece]) <
                  alpha) &&
            G(200, moves_evaluated) && G(200, !in_check)) { break; })

    Position npos = *pos;
#ifdef FULL
    (*nodes)++;
#endif
    if (!makemove(H(82, 3, &npos), H(82, 3, &moves[move_index]))) {
      continue;
    }

    // PRINCIPAL VARIATION SEARCH
    i32 low = moves_evaluated == 0 ? -beta : -alpha - 1;
    moves_evaluated++;

    // LATE MOVE REDUCTION
    i32 reduction = G(203, depth > 3) && G(203, move_score <= 0)
                        ? G(204, (move_score < -263)) +
                              G(204, moves_evaluated / 9) + G(204, !improving) +
                              G(204, (G(205, alpha) == G(205, beta - 1)))
                        : 0;

    i32 score;
    while (true) {
      score = -search(
#ifdef FULL
          nodes,
#endif
          H(165, 3, &npos), H(165, 3, -alpha), H(165, 3, ply + 1),
          H(165, 3, depth - G(206, 1) - G(206, reduction)), H(165, 3, data),
          H(165, 3, true), H(165, 3, low));

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
        G(207, tt_flag = Exact;)
        G(207, stack[ply].best_move = moves[move_index];)
        G(207, alpha = score;)
        if (score >= beta) {
          assert(stack[ply].best_move.takes_piece ==
                 piece_on(H(57, 8, pos), H(57, 8, stack[ply].best_move.to)));
          G(208, tt_flag = Lower;)
          G(
              208, if (stack[ply].best_move.takes_piece == None) {
                stack[ply].killer = stack[ply].best_move;
              })
          G(
              208, if (!in_qsearch) {
                const i32 bonus = depth * depth;
                G(209, i32 *const this_hist =
                           &move_history[pos->flipped]
                                        [stack[ply].best_move.takes_piece]
                                        [stack[ply].best_move.from]
                                        [stack[ply].best_move.to];

                  *this_hist +=
                  bonus - G(210, bonus) * G(210, *this_hist) / 1024;)
                G(
                    209, for (i32 prev_index = 0; prev_index < move_index;
                              prev_index++) {
                      const Move prev = moves[prev_index];
                      i32 *const prev_hist =
                          &move_history[pos->flipped][prev.takes_piece]
                                       [prev.from][prev.to];
                      *prev_hist -=
                          bonus + G(211, bonus) * G(211, *prev_hist) / 1024;
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
    if (G(212, !in_check) && G(212, G(213, alpha) == G(213, beta - 1)) &&
        G(212, quiets_evaluated > (G(214, 1) + G(214, depth * depth)) >>
                   !improving)) {
      break;
    }
  }

  // MATE / STALEMATE DETECTION
  if (G(215, best_score) == G(215, -inf)) {
    return G(216, (ply - mate)) * G(216, in_check);
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
      82, // INIT DIAGONAL MASKS
      for (i32 sq = 0; sq < 64; sq++) {
        const u64 bb = 1ULL << sq;
        G(217, u64 sw_bb = southwest(bb);)
        G(217, u64 ne_bb = northeast(bb);)
        for (i32 i = 6; i > 0; i--) {
          G(218, ne_bb |= northeast(ne_bb);)
          G(218, sw_bb |= southwest(sw_bb);)
        }
        diag_mask[sq] = G(219, sw_bb) | G(219, ne_bb);
      })
  G(
      82, // MERGE EVAL PARAMS
      for (i32 i = 0; i < sizeof(EvalParamsMerged) / sizeof(i32); i++) {
        ((i32 *)&eval_params)[i] =
            combine_eval_param(H(123, 2,
                                 i < 6 ? initial_params.eg.material[i]
                                       : ((i8 *)&initial_params.eg)[6 + i]),
                               H(123, 2,
                                 i < 6 ? initial_params.mg.material[i]
                                       : ((i8 *)&initial_params.mg)[6 + i]));
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
    move_str(H(54, 2, move_name), H(54, 2, &pv_move), H(54, 2, pos->flipped));
    putl(move_name);
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
    G(220, i32 window = 15;)
    G(220, size_t elapsed;)
    while (true) {
      G(221, const i32 alpha = score - window;)
      G(221, const i32 beta = G(222, score) + G(222, window);)
      score = search(
#ifdef FULL
          &data->nodes,
#endif
          H(165, 4, &data->pos), H(165, 4, beta), H(165, 4, 0),
          H(165, 4, depth), H(165, 4, data), H(165, 4, false),
          H(165, 4, alpha));
#ifdef FULL
      if (data->thread_id == 0) {
        print_info(&data->pos, depth, alpha, beta, score, data->nodes,
                   data->stack[0].best_move, data->max_time);
      }
#endif
      elapsed = get_time() - start_time;
      G(223, window *= 2;)
      G(
          223, if (G(224, elapsed > data->max_time) ||
                   G(224, (G(225, score < beta) && G(225, score > alpha)))) {
            break;
          })
    }

    if (stop || elapsed > data->max_time / 16) {
      break;
    }
  }
}

S(1) void *entry_full(void *param) {
  ThreadData *data = param;
  iteratively_deepen(
#ifdef FULL
      max_ply,
#endif
      data);
  return NULL;
}

S(1) void entry_mini(ThreadData *data) {
  iteratively_deepen(
#ifdef FULL
      max_ply,
#endif
      data);
  exit_now();
}

#ifndef FULL
__attribute__((naked)) S(1) long newthread(ThreadData *stack) {
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
#ifdef FULL
  pthread_t helpers[thread_count - 1];
  u64 nodes = 0;
#endif

  ThreadData *main_data = (ThreadData *)&thread_stacks[0][0];

  for (i32 i = 1; i < thread_count; i++) {
    ThreadData *helper_data =
        (ThreadData *)&thread_stacks[i][thread_stack_size - sizeof(ThreadData)];
    G(226, helper_data->pos = main_data->pos;)
    G(226, helper_data->max_time = -1LL;)
#ifdef FULL
    helper_data->thread_id = i;
    pthread_create(&helpers[i - 1], NULL, entry_full, helper_data);
#else
    helper_data->entry = entry_mini;
    newthread(helper_data);
#endif
  }

  iteratively_deepen(
#ifdef FULL
      max_ply,
#endif
      main_data);
  stop = true;

  for (i32 i = 0; i < thread_count - 1; i++) {
#ifdef FULL
    pthread_join(helpers[i], NULL);
#else
    // TODO: sync ?
#endif
  }

  char move_name[8];
  move_str(H(54, 3, move_name), H(54, 3, &main_data->stack[0].best_move),
           H(54, 3, main_data->pos.flipped));
  putl("bestmove ");
  puts(move_name);
}

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
      i32 piece = piece_on(H(57, 9, &npos), H(57, 9, sq));
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
  Position pos;
  i32 move_history[2][6][64][64];
  SearchStack stack[1024];
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

  G(227, char line[4096];)
  G(227, init();)
  G(227, __builtin_memset(thread_stacks, 0, sizeof(thread_stacks));)
  G(227, ThreadData *main_data = (ThreadData *)&thread_stacks[0][0];)

#ifdef FULL
  main_data->pos = start_pos;
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
      puts("option name Hash type spin default 1 min 1 max 1");
      puts("option name Threads type spin default 4 min 1 max 4");
      puts("uciok");
    } else if (!strcmp(line, "ucinewgame")) {
      __builtin_memset(thread_stacks, 0, sizeof(thread_stacks));
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
    G(228, if (G(229, line[0]) == G(229, 'q')) { exit_now(); })
    else G(228, if (G(230, line[0]) == G(230, 'i')) { puts("readyok"); })
    else G(228, if (G(231, line[0]) == G(231, 'g')) {
      stop = false;
#ifdef FULL
      while (true) {
        getl(line);
        if (!main_data->pos.flipped && !strcmp(line, "wtime")) {
          getl(line);
          main_data->max_time = (u64)atoi(line) << 19; // Roughly /2 time
          break;
        }
        else if (main_data->pos.flipped && !strcmp(line, "btime")) {
          getl(line);
          main_data->max_time = (u64)atoi(line) << 19; // Roughly /2 time
          break;
        }
        else if (!strcmp(line, "movetime")) {
          main_data->max_time = 20ULL * 1000 * 1000 * 1000; // Assume Lichess bot
          break;
        }
      }
      run_smp();
#else
      for (i32 i = 2 << main_data->pos.flipped; i > 0; i--) {
        getl(line);
        main_data->max_time = (u64)atoi(line) << 19; // Roughly /2 time
      }
      start_time = get_time();
      run_smp();
#endif
    })
    else G(228, if (G(232, line[0]) == G(232, 'p')) {
      G(233, main_data->pos = start_pos;)
        while (true) {
          const bool line_continue = getl(line);

#if FULL
          if (!strcmp(line, "fen")) {
            getl(line);
            get_fen(&main_data->pos, line);
          }
#endif
          Move moves[max_moves];
          const i32 num_moves =
            movegen(H(97, 4, &main_data->pos), H(97, 4, moves), H(97, 4, false));
          for (i32 i = 0; i < num_moves; i++) {
            char move_name[8];
            move_str(H(54, 4, move_name), H(54, 4, &moves[i]),
              H(54, 4, main_data->pos.flipped));
            assert(move_string_equal(line, move_name) ==
              !strcmp(line, move_name));
            if (move_string_equal(G(234, move_name), G(234, line))) {
              makemove(H(82, 4, &main_data->pos), H(82, 4, &moves[i]));
              break;
            }
          }
          if (!line_continue) {
            break;
          }
        }
    })
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
