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

G(1, S(1) ssize_t _sys(H(2, 1, ssize_t arg1), H(2, 1, ssize_t arg3),
                         H(2, 1, ssize_t arg2), H(2, 1, ssize_t call)) {
      ssize_t ret;
      asm volatile("syscall"
                   : "=a"(ret)
                   : "0"(call), "D"(arg1), "S"(arg2), "d"(arg3)
                   : "rcx", "r11", "memory");
      return ret;
    })

G(1, S(1) void exit_now() {
      asm volatile("syscall" : : "a"(60));
      __builtin_unreachable();
    })

G(3, // Non-standard, gets but a word instead of a line
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

G(3, [[nodiscard]] S(1) u32 atoi(const char *restrict string) {
      u32 result = 0;
      while (*string)
        result = result * 10 + *string++ - '0';
      return result;
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

G(3, S(1) void putl(const char *const restrict string) {
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
  return G(5, ts.tv_nsec) + G(5, G(6, ts.tv_sec) * G(6, 1000 * 1000 * 1000ULL));
}

#else
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <pthread.h>

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
  G(7, u8 takes_piece;)
  G(7, u8 promo;)
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

G(10, [[nodiscard]] S(1) bool move_string_equal(G(11, const char *restrict rhs),
                                              G(11, const char *restrict lhs)) {
      return (G(12, *(const u64 *)rhs) ^ G(12, *(const u64 *)lhs)) << 24 == 0;
    })

G(10, [[nodiscard]] S(1)
            u64 flip_bb(const u64 bb) { return __builtin_bswap64(bb); })
G(10, [[nodiscard]] S(1) i32 lsb(u64 bb) { return __builtin_ctzll(bb); })

G(10, [[nodiscard]] S(1)
            u64 shift(H(13, 1, const i32 shift), H(13, 1, const u64 mask),
                      H(13, 1, const u64 bb)) {
              return shift > 0 ? G(14, bb << shift) & G(14, mask)
                               : G(15, bb >> -shift) & G(15, mask);
            })

G(10, [[nodiscard]] S(1)
            i32 count(const u64 bb) { return __builtin_popcountll(bb); })

G(16, [[nodiscard]] S(1) u64 west(const u64 bb) {
      return G(17, bb >> 1) & G(17, ~0x8080808080808080ull);
    })

G(16, [[nodiscard]] S(1) u64 north(const u64 bb) { return bb << 8; })

G(16, [[nodiscard]] S(1) u64 south(const u64 bb) { return bb >> 8; })

G(16, [[nodiscard]] S(1) u64 east(const u64 bb) {
      return G(18, bb << 1) & G(18, ~0x101010101010101ull);
    })

G(19, [[nodiscard]] S(1) u64 southeast(const u64 bb) {
      return G(20, shift(H(13, 2, -7), H(13, 2, ~0x101010101010101ull),
                         H(13, 2, bb)));
      return G(20, G(21, east)(G(21, south)(bb)));
    })

G(19, [[nodiscard]] S(1) u64 northeast(const u64 bb) {
      return G(22, G(23, east)(G(23, north)(bb)));
      return G(22, shift(H(13, 3, 9), H(13, 3, ~0x101010101010101ull),
                         H(13, 3, bb)));
    })

G(19, [[nodiscard]] S(1) u64 northwest(const u64 bb) {
      return G(24, shift(H(13, 4, 7), H(13, 4, ~0x8080808080808080ull),
                         H(13, 4, bb)));
      return G(24, G(25, west)(G(25, north)(bb)));
    })

G(19, [[nodiscard]] S(1) u64 southwest(const u64 bb) {
      return G(26, G(27, west)(G(27, south)(bb)));
      return G(26, shift(H(13, 5, -9), H(13, 5, ~0x8080808080808080ull),
                         H(13, 5, bb)));
    })

G(28, S(0) u64 diag_mask[64];)

G(28, [[nodiscard]] S(1)
            u64 ray(H(29, 1, const u64 blockers), H(29, 1, const u64 mask),
                    H(29, 1, const u64 bb), H(29, 1, const i32 shift_by)) {
              u64 result =
                  shift(H(13, 6, shift_by), H(13, 6, mask), H(13, 6, bb));
              for (i32 i = 5; i >= 0; i--) {
                result |= shift(H(13, 7, shift_by), H(13, 7, mask),
                                H(13, 7, result & ~blockers));
              }
              return result;
            })

G(28, [[nodiscard]] S(0) u64 xattack(H(30, 1, const u64 dir_mask),
                                       H(30, 1, const u64 bb),
                                       H(30, 1, const u64 blockers)) {
      return G(31, dir_mask) &
             G(31, (G(32, (G(33, blockers) & G(33, dir_mask)) - bb) ^
                    G(32, flip_bb(flip_bb(G(34, blockers) & G(34, dir_mask)) -
                                  flip_bb(bb)))));
    })

G(35, [[nodiscard]] S(0) u64 bishop(H(36, 1, const u64 blockers),
                                      H(36, 1, const u64 bb)) {
      assert(count(bb) == 1);
      const i32 sq = lsb(bb);
      return G(37, xattack(H(30, 2, diag_mask[sq]), H(30, 2, bb),
                           H(30, 2, blockers))) |
             G(37, xattack(H(30, 3, flip_bb(diag_mask[G(38, sq) ^ G(38, 56)])),
                           H(30, 3, bb), H(30, 3, blockers)));
    })

G(35, [[nodiscard]] S(1) u64 rook(H(39, 1, const u64 blockers),
                                    H(39, 1, const u64 bb)) {
      assert(count(bb) == 1);
      return G(40, xattack(H(30, 4, bb ^ 0x101010101010101ULL << lsb(bb) % 8),
                           H(30, 4, bb), H(30, 4, blockers))) |
             G(40, // East
               ray(H(29, 2, blockers), H(29, 2, ~0x101010101010101ull),
                   H(29, 2, bb), H(29, 2, 1))) |
             G(40, // West
               ray(H(29, 3, blockers), H(29, 3, ~0x8080808080808080ull),
                   H(29, 3, bb), H(29, 3, -1)));
    })

G(35, [[nodiscard]] S(1) u64 knight(const u64 bb) {
      G(41, const u64 east_bb = east(bb);)
      G(41, const u64 west_bb = west(bb);)
      G(42, const u64 horizontal1 = G(43, west_bb) | G(43, east_bb);)
      G(42, const u64 horizontal2 = G(44, east(east_bb)) | G(44, west(west_bb));)
      return G(45, horizontal1 << 16) | G(45, horizontal1 >> 16) |
             G(45, horizontal2 >> 8) | G(45, horizontal2 << 8);
    })

G(35, [[nodiscard]] S(0) u64 king(const u64 bb) {
      const u64 vertical = G(46, south(bb)) | G(46, north(bb));
      const u64 vertical_inclusive = G(47, bb) | G(47, vertical);
      return G(48, vertical) | G(48, east(vertical_inclusive)) |
             G(48, west(vertical_inclusive));
    })

G(49, S(1) void swapu32(G(50, u32 *const rhs), G(50, u32 *const lhs)) {
      const u32 temp = *lhs;
      *lhs = *rhs;
      *rhs = temp;
    }

    S(1) void swapmoves(G(51, Move *const lhs), G(51, Move *const rhs)) {
      swapu32(G(52, (u32 *)lhs), G(52, (u32 *)rhs));
    })

G(49, [[nodiscard]] S(1) bool move_equal(G(53, Move *const rhs),
                                           G(53, Move *const lhs)) {
      return G(54, *(u32 *)lhs) == G(54, *(u32 *)rhs);
    })

G(49, S(1) void move_str(H(57, 1, char *restrict str),
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

      G(58, str[5] = '\0';)

      G(58, str[4] = "\0\0nbrq"[move->promo];)
      G(58, // Hack to save bytes, technically UB but works on GCC 14.2
          for (i32 i = 0; i < 2; i++) {
            G(59, str[i * 2] = 'a' + (&move->from)[i] % 8;)
            G(59, str[i * 2 + 1] = '1' + ((&move->from)[i] / 8 ^ 7 * flip);)
          })
    })

G(49, [[nodiscard]] S(1)
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

G(60, [[nodiscard]] S(1) i32 is_attacked(H(61, 1, const Position *const restrict pos),
                                       H(61, 1, const u64 bb)) {
      assert(count(bb) == 1);
      const u64 theirs = pos->colour[1];
      G(62, const u64 pawns = theirs & pos->pieces[Pawn];)
      G(62, const u64 blockers = theirs | pos->colour[0];)
      return G(63, G(64, (G(65, southwest(pawns)) | G(65, southeast(pawns)))) & G(64, bb)) ||
             G(63, G(66, bishop(H(36, 2, blockers), H(36, 2, bb))) &
                       G(66, theirs) &
                       G(66, (pos->pieces[Bishop] | pos->pieces[Queen]))) ||
             G(63, G(67, (pos->pieces[Rook] | pos->pieces[Queen])) &
                       G(67, theirs) &
                       G(67, rook(H(39, 2, blockers), H(39, 2, bb)))) ||
             G(63, G(68, king(bb)) & G(68, theirs) & G(68, pos->pieces[King])) ||
             G(63, G(69, knight(bb)) & G(69, theirs) & G(69, pos->pieces[Knight]));
    })

G(60, S(0) void flip_pos(Position *const restrict pos) {
      G(70, // Hack to flip the first 10 bitboards in Position.
              // Technically UB but works in GCC 14.2
          u64 *pos_ptr = (u64 *)pos;
          for (i32 i = 0; i < 10; i++) { pos_ptr[i] = flip_bb(pos_ptr[i]); })
      G(70, pos->colour[0] ^= pos->colour[1]; pos->colour[1] ^= pos->colour[0];
        pos->colour[0] ^= pos->colour[1];)

      G(70, u32 *c = (u32 *)pos->castling;
        *c = G(71, (*c >> 16)) | G(71, (*c << 16));)
      G(70, pos->flipped ^= 1;)
    })

G(60, [[nodiscard]] S(1) u64 get_mobility(H(72, 1, const Position *pos),
                                            H(72, 1, const i32 sq),
                                            H(72, 1, const i32 piece)) {
      u64 moves = 0;
      const u64 bb = 1ULL << sq;
      G(73, if (piece == Knight) { moves = knight(bb); })
      else G(73, if (piece == King) { moves = king(bb); }) else {
        const u64 blockers = G(74, pos->colour[1]) | G(74, pos->colour[0]);
        G(75, if (G(76, piece == Queen) || G(76, piece == Bishop)) {
              moves |= bishop(H(36, 3, blockers), H(36, 3, bb));
            })
        G(75, if (G(77, piece == Queen) || G(77, piece == Rook)) {
              moves |= rook(H(39, 3, blockers), H(39, 3, bb));
            })
      }
      return moves;
    })

S(0) i32 find_in_check(const Position *restrict pos) {
  return is_attacked(
      H(61, 2, pos),
      H(61, 2, G(78, pos->colour[0]) & G(78, pos->pieces[King])));
}

G(79, S(1) Move *generate_piece_moves(H(80, 1, const u64 to_mask),
                                    H(80, 1, Move *restrict movelist),
                                    H(80, 1, const Position *restrict pos)) {
      for (i32 piece = Knight; piece <= King; piece++) {
        assert(piece == Knight || piece == Bishop || piece == Rook ||
               piece == Queen || piece == King);
        u64 copy = G(81, pos->colour[0]) & G(81, pos->pieces[piece]);
        while (copy) {
          const u8 from = lsb(copy);
          assert(from >= 0);
          assert(from < 64);
          G(82, copy &= copy - 1;)

          G(82, u64 moves = G(83, to_mask) &
                            G(83, get_mobility(H(72, 2, pos), H(72, 2, from),
                                               H(72, 2, piece)));)

          while (moves) {
            const u8 to = lsb(moves);
            assert(to >= 0);
            assert(to < 64);

            G(84, moves &= moves - 1;)
            G(84, *movelist++ = ((Move){
                      .from = from,
                      .to = to,
                      .promo = None,
                      .takes_piece = piece_on(H(55, 2, pos), H(55, 2, to))});)
          }
        }
      }

      return movelist;
    })

G(79, S(0) i32 makemove(H(85, 1, Position *const restrict pos),
                          H(85, 1, const Move *const restrict move)) {
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

      G(86, const u64 to = 1ull << move->to;)
      G(86, const u64 from = 1ull << move->from;)
      G(87, const i32 piece = piece_on(H(55, 4, pos), H(55, 4, move->from));
        assert(piece != None);)
      G(87, const u64 mask = G(88, from) | G(88, to);)

      G(89, // Castling
          if (piece == King) {
            const u64 bb = move->to - move->from == 2   ? 0xa0
                           : move->from - move->to == 2 ? 0x9
                                                        : 0;
            G(90, pos->pieces[Rook] ^= bb;)
            G(90, pos->colour[0] ^= bb;)
          })

      G(89, pos->colour[0] ^= mask;)

      // Move the piece
      G(89, pos->pieces[piece] ^= mask;)
      G(89, // Captures
          if (move->takes_piece != None) {
            G(91, pos->colour[1] ^= to;)
            G(91, pos->pieces[move->takes_piece] ^= to;)
          })

      // En passant
      if (G(92, piece == Pawn) && G(92, to == pos->ep)) {
        G(93, pos->colour[1] ^= to >> 8;)
        G(93, pos->pieces[Pawn] ^= to >> 8;)
      }
      pos->ep = 0;

      G(94, // Pawn double move
          if (G(95, move->to - move->from == 16) && G(95, piece == Pawn)) {
            pos->ep = to >> 8;
          })

      G(94, // Promotions
          if (move->promo != None) {
            G(96, pos->pieces[Pawn] ^= to;)
            G(96, pos->pieces[move->promo] ^= to;)
          })

      G(94, // Update castling permissions
        const u64 oppMask = mask >> 56;
        G(97, pos->castling[3] &= !(oppMask & 0x11);)
            G(97, pos->castling[1] &= !(mask & 0x11);)
                G(97, pos->castling[0] &= !(mask & 0x90);)
                    G(97, pos->castling[2] &= !(oppMask & 0x90);))

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

G(79, S(0) Move *generate_pawn_moves(H(98, 1, const i32 offset),
                                       H(98, 1, Move *restrict movelist),
                                       H(98, 1, u64 to_mask),
                                       H(98, 1, const Position *const pos)) {
      while (to_mask) {
        const u8 to = lsb(to_mask);
        to_mask &= to_mask - 1;
        const u8 from = G(99, to) + G(99, offset);
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

[[nodiscard]] S(1) i32 movegen(H(100, 1, const Position *const restrict pos),
                               H(100, 1, Move *restrict movelist),
                               H(100, 1, const i32 only_captures)) {

  G(101, const u64 all = G(102, pos->colour[0]) | G(102, pos->colour[1]);)
  G(101, const Move *start = movelist;)
  G(101, const u64 to_mask = only_captures ? pos->colour[1] : ~pos->colour[0];)
  G(103, // PAWN PROMOTIONS
      if (!only_captures) {
        movelist = generate_pawn_moves(
            H(98, 2, -16), H(98, 2, movelist),
            H(98, 2, G(104, north(G(105, north(G(106, pos->colour[0]) &
                                        G(106, pos->pieces[Pawn]) &
                                        G(106, 0xFF00))) &
                           G(105, ~all))) &
                  G(104, ~all)),
            H(98, 2, pos));
      })
  G(103, // PAWN DOUBLE MOVES
    movelist = generate_pawn_moves(
        H(98, 3, -8), H(98, 3, movelist),
        H(98, 3, north(G(107, G(108, pos->colour[0]) & G(108, pos->pieces[Pawn]))) &
              G(107, ~all) &
              G(107, (only_captures ? 0xFF00000000000000ull : ~0ull))),
        H(98, 3, pos));)
  G(103, // PAWN WEST CAPTURES
    movelist = generate_pawn_moves(
        H(98, 4, -7), H(98, 4, movelist),
        H(98, 4, G(109, northwest(G(110, pos->colour[0]) & G(110, pos->pieces[Pawn]))) &
              G(109, (G(111, pos->colour[1]) | G(111, pos->ep)))),
        H(98, 4, pos));)
  G(103, // PAWN EAST CAPTURES
    movelist = generate_pawn_moves(
        H(98, 5, -9), H(98, 5, movelist),
        H(98, 5, G(112, northeast(G(113, pos->colour[0]) & G(113, pos->pieces[Pawn]))) &
              G(112, (G(114, pos->colour[1]) | G(114, pos->ep)))),
        H(98, 5, pos));)
  G(103, // LONG CASTLE
      if (G(115, !only_captures) && G(115, pos->castling[0]) &&
          G(115, !(G(116, all) & G(116, 0x60ull))) &&
          G(117, !is_attacked(H(61, 3, pos), H(61, 3, 1ULL << 4))) &&
          G(117, !is_attacked(H(61, 4, pos), H(61, 4, 1ULL << 5)))) {
        *movelist++ =
            (Move){.from = 4, .to = 6, .promo = None, .takes_piece = None};
      })
  G(103, // SHORT CASTLE
      if (G(118, !only_captures) && G(118, pos->castling[1]) &&
          G(118, !(G(119, all) & G(119, 0xEull))) &&
          G(120, !is_attacked(H(61, 5, pos), H(61, 5, 1ULL << 4))) &&
          G(120, !is_attacked(H(61, 6, pos), H(61, 6, 1ULL << 3)))) {
        *movelist++ =
            (Move){.from = 4, .to = 2, .promo = None, .takes_piece = None};
      })
  movelist = generate_piece_moves(H(80, 2, to_mask), H(80, 2, movelist),
                                  H(80, 2, pos));

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
      movegen(H(100, 2, pos), H(100, 2, moves), H(100, 2, false));

  for (i32 i = 0; i < num_moves; ++i) {
    Position npos = *pos;

    // Check move legality
    if (!makemove(H(85, 2, &npos), H(85, 2, &moves[i]))) {
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
  H(121, 1, H(122, 1, i8 mobilities[5];) H(122, 1, i8 open_files[6];)
        H(122, 1, i8 tempo;) H(122, 1, i8 pst_file[48];)
            H(122, 1, i8 passed_blocked_pawns[6];)
                H(122, 1, i8 pawn_attacked_penalty[2];))
  H(121, 1, H(123, 1, i8 king_attacks[5];) H(123, 1, i8 pst_rank[48];)
        H(123, 1, i8 passed_pawns[6];) H(123, 1, i8 bishop_pair;)
            H(123, 1, i8 protected_pawn;) H(123, 1, i8 phalanx_pawn;))
} EvalParams;

typedef struct [[nodiscard]] __attribute__((packed)) {
  i32 material[6];
  H(121, 2, H(122, 2, i32 mobilities[5];) H(122, 2, i32 open_files[6];)
        H(122, 2, i32 tempo;) H(122, 2, i32 pst_file[48];)
            H(122, 2, i32 passed_blocked_pawns[6];)
                H(122, 2, i32 pawn_attacked_penalty[2];))
  H(121, 2, H(123, 2, i32 king_attacks[5];) H(123, 2, i32 pst_rank[48];)
        H(123, 2, i32 passed_pawns[6];) H(123, 2, i32 bishop_pair;)
            H(123, 2, i32 protected_pawn;) H(123, 2, i32 phalanx_pawn;))

} EvalParamsMerged;

typedef struct [[nodiscard]] __attribute__((packed)) {
  i8 phases[6];
  G(124, EvalParams eg;)
  G(124, EvalParams mg;)
} EvalParamsInitial;

G(125, S(0) EvalParamsMerged eval_params;)

G(125, __attribute__((aligned(8))) S(1)
      const EvalParamsInitial initial_params = {.phases = {0, 0, 1, 1, 2, 4},
                                                .mg = {.material = {0, 65, 270,
                                                                    273, 362,
                                                                    780},
                                                       .pst_rank =
                                                           {
                                                               0,   -11, -16,
                                                               -8,  6,   29,
                                                               94,  0, // Pawn
                                                               -21, -12, 2,
                                                               15,  26,  44,
                                                               23,  -78, // Knight
                                                               -7,  7,   13,
                                                               14,  15,  16,
                                                               -3,  -55, // Bishop
                                                               1,   -11, -16,
                                                               -18, 1,   20,
                                                               10,  13, // Rook
                                                               16,  17,  10,
                                                               -1,  -6,  -1,
                                                               -24, -11, // Queen
                                                               -1,  -1,  -32,
                                                               -42, -12, 39,
                                                               47,  47, // King
                                                           },
                                                       .pst_file =
                                                           {
                                                               -15, -13, -9,
                                                               0,   5,   20,
                                                               17,  -5, // Pawn
                                                               -25, -11, 0,
                                                               14,  12,  11,
                                                               3,   -4, // Knight
                                                               -12, 2,   4,
                                                               2,   5,   -2,
                                                               6,   -4, // Bishop
                                                               -9,  -7,  1,
                                                               10,  13,  1,
                                                               -1,  -9, // Rook
                                                               -10, -7,  -2,
                                                               1,   2,   0,
                                                               8,   8, // Queen
                                                               -15, 24,  -5,
                                                               -55, -21, -39,
                                                               21,  3, // King
                                                           },
                                                       .mobilities = {6, 6, 2,
                                                                      3, -9},
                                                       .king_attacks = {0, 15,
                                                                        21, 13,
                                                                        0},
                                                       .open_files = {13, -9,
                                                                      -9, 20,
                                                                      -3, -30},
                                                       .passed_pawns = {-9, -11,
                                                                        -7, 11,
                                                                        34, 94},
                                                       .passed_blocked_pawns =
                                                           {6, 0, 5, 13, 16,
                                                            -32},
                                                       .protected_pawn = 17,
                                                       .phalanx_pawn = 11,
                                                       .bishop_pair = 24,
                                                       .pawn_attacked_penalty =
                                                           {-16, -128},
                                                       .tempo = 17},
                                                .eg = {.material = {0, 84, 401,
                                                                    397, 711,
                                                                    1349},
                                                       .pst_rank =
                                                           {
                                                               0,   -5,  -8,
                                                               -7,  0,   20,
                                                               101, 0, // Pawn
                                                               -38, -21, -5,
                                                               22,  28,  10,
                                                               1,   4, // Knight
                                                               -14, -13, -2,
                                                               3,   7,   5,
                                                               3,   10, // Bishop
                                                               -24, -24, -15,
                                                               4,   15,  14,
                                                               21,  9, // Rook
                                                               -66, -51, -23,
                                                               10,  34,  34,
                                                               42,  21, // Queen
                                                               -50, -6,  8,
                                                               23,  34,  31,
                                                               16,  -50, // King
                                                           },
                                                       .pst_file =
                                                           {
                                                               12,  12,  0,
                                                               -10, -4,  -3,
                                                               -2,  -5, // Pawn
                                                               -22, -4,  10,
                                                               17,  17,  7,
                                                               -2,  -22, // Knight
                                                               -9,  -1,  1,
                                                               5,   7,   5,
                                                               0,   -8, // Bishop
                                                               2,   5,   5,
                                                               -1,  -5,  1,
                                                               0,   -6, // Rook
                                                               -23, -6,  4,
                                                               11,  15,  12,
                                                               -2,  -12, // Queen
                                                               -30, 1,   17,
                                                               31,  23,  25,
                                                               1,   -39, // King
                                                           },
                                                       .mobilities = {3, 5, 4,
                                                                      1, -4},
                                                       .king_attacks = {0, -4,
                                                                        -7, 10,
                                                                        0},
                                                       .open_files = {23, -5, 7,
                                                                      10, 27,
                                                                      10},
                                                       .passed_pawns =
                                                           {10, 16, 41, 68, 115,
                                                            101},
                                                       .passed_blocked_pawns =
                                                           {-12, -14, -39, -69,
                                                            -119, -124},
                                                       .protected_pawn = 16,
                                                       .phalanx_pawn = 14,
                                                       .bishop_pair = 63,
                                                       .pawn_attacked_penalty =
                                                           {-10, -128},
                                                       .tempo = 7}};)

G(125, [[nodiscard]] S(1) i32 combine_eval_param(H(126, 1, const i32 mg_val),
                                              H(126, 1, const i32 eg_val)) {
      return G(127, mg_val) + G(127, (eg_val << 16));
    })

S(0) i32 eval(Position *const restrict pos) {
  G(128, i32 score = eval_params.tempo;)
  G(128, i32 phase = 0;)

  for (i32 c = 0; c < 2; c++) {

    G(129, const u64 opp_king_zone =
               king(G(130, pos->colour[1]) & G(130, pos->pieces[King]));)

    G(129, const u64 own_pawns = G(131, pos->pieces[Pawn]) & G(131, pos->colour[0]);)
    G(129, const u64 opp_pawns = G(132, pos->pieces[Pawn]) & G(132, pos->colour[1]);
      const u64 attacked_by_pawns =
          G(133, southeast(opp_pawns)) | G(133, southwest(opp_pawns));
      G(134, // PROTECTED PAWNS
            score -=
            G(136, eval_params.protected_pawn) *
            G(136, count(G(137, opp_pawns) & G(137, attacked_by_pawns)));)
          G(134, const u64 no_passers = G(135, opp_pawns) | G(135, attacked_by_pawns);)
              G(134, // PHALANX PAWNS
                score -=
                G(138, eval_params.phalanx_pawn) *
                G(138, count(G(139, opp_pawns) & G(139, west(opp_pawns))));))
    G(129, // BISHOP PAIR
        if (count(G(140, pos->pieces[Bishop]) & G(140, pos->colour[0])) > 1) {
          score += eval_params.bishop_pair;
        })

    for (i32 p = Pawn; p <= King; p++) {
      u64 copy = G(141, pos->colour[0]) & G(141, pos->pieces[p]);
      while (copy) {
        const i32 sq = lsb(copy);
        G(142, copy &= copy - 1;)
        G(142, phase += initial_params.phases[p];)
        G(142, const i32 rank = sq >> 3;)
        G(142, const i32 file = G(143, sq) & G(143, 7);)

        G(98, // PASSED PAWNS
            if (G(145, p == Pawn) &&
                G(145, !(G(146, (0x101010101010101ULL << sq)) &
                         G(146, no_passers)))) {
              G(147, if (G(148, north(1ULL << sq)) & G(148, pos->colour[1])) {
                    score += eval_params.passed_blocked_pawns[rank - 1];
                  })

              G(147, score += eval_params.passed_pawns[rank - 1];)
            })

        G(98, // MATERIAL
          score += eval_params.material[p];)
        G(98, // SPLIT PIECE-SQUARE TABLES FOR FILE
          score +=
          eval_params
              .pst_file[G(151, G(152, (p - 1)) * G(152, 8)) + G(151, file)];)

        G(98, // OPEN FILES / DOUBLED PAWNS
            if ((G(144, north(0x101010101010101ULL << sq)) &
                 G(144, own_pawns)) == 0) {
              score += eval_params.open_files[p - 1];
            })

        G(98, // SPLIT PIECE-SQUARE TABLES FOR RANK
          score +=
          eval_params
              .pst_rank[G(149, G(150, (p - 1)) * G(150, 8)) + G(149, rank)];)

        G(98, if (p > Pawn) {
              G(153, // PIECES ATTACKED BY PAWNS
                  if (G(154, 1ULL << sq) & G(154, no_passers)) {
                    score += eval_params.pawn_attacked_penalty[c];
                  })

              G(153, const u64 mobility =
                         G(155, get_mobility(H(72, 3, pos), H(72, 3, sq),
                                             H(72, 3, p))) &
                         G(155, ~attacked_by_pawns);

                G(156, // MOBILITY
                  score +=
                  G(157, count(G(158, ~pos->colour[0]) & G(158, mobility))) *
                  G(157, eval_params.mobilities[p - 2]);)

                    G(156, // KING ATTACKS
                      score +=
                      G(159, count(G(160, mobility) & G(160, opp_king_zone))) *
                      G(159, eval_params.king_attacks[p - 2]);))
            })
      }
    }

    G(80, score = -score;)
    G(80, flip_pos(pos);)
  }

  const i32 stronger_side_pawns_missing =
      8 - count(G(161, pos->colour[score < 0]) & G(161, pos->pieces[Pawn]));
  return (G(162, (i16)score) * G(162, phase) +
          G(163, ((score + 0x8000) >> 16)) *
              G(163, (128 - stronger_side_pawns_missing *
                                stronger_side_pawns_missing)) /
              128 * (24 - phase)) /
         24;
}

typedef struct [[nodiscard]] {
  G(122, Move best_move;)
  G(122, Move killer;)
  G(122, i32 static_eval;)
  G(122, i32 num_moves;)
  G(122, u64 position_hash;)
} SearchStack;

typedef struct [[nodiscard]] __attribute__((packed)) {
  G(164, i8 depth;)
  G(164, i16 score;)
  G(164, u8 flag;)
  G(164, u16 partial_hash;)
  G(164, Move move;)
} TTEntry;
_Static_assert(sizeof(TTEntry) == 10);

enum { tt_length = 1 << 23 }; // 80MB
enum { Upper = 0, Lower = 1, Exact = 2 };
enum { max_ply = 96 };
enum { mate = 31744, inf = 32256 };

G(165, S(1) TTEntry tt[tt_length];)
G(165, S(1) u64 start_time;)
G(165, S(1) volatile bool stop;)

#if defined(__x86_64__) || defined(_M_X64)
typedef long long __attribute__((__vector_size__(16))) i128;

[[nodiscard]] __attribute__((target("aes"))) S(1) u64
    get_hash(const Position *const pos) {
  i128 hash = {0};

  // USE 16 BYTE POSITION SEGMENTS AS KEYS FOR AES
  const u8 *const data = (const u8 *)pos;
  for (i32 i = 0; i < 6; i++) {
    i128 key;
    __builtin_memcpy(&key, data + G(166, i) * G(166, 16), 16);
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
i32 search(H(167, 1, const bool do_null), H(167, 1, const i32 beta),
           H(167, 1, i32 depth), H(167, 1, SearchStack *restrict stack),
           H(167, 1, i32 alpha),
#ifdef FULL
           u64 *nodes,
#endif
           H(168, 1, Position *const restrict pos),
           H(168, 1, const u64 max_time),
           H(168, 1, const i32 pos_history_count), H(168, 1, i32 move_history[2][6][64][64]),
           H(168, 1, const i32 ply)) {
  assert(alpha < beta);
  assert(ply >= 0);

  // IN-CHECK EXTENSION
  const bool in_check = find_in_check(pos);
  depth += in_check;

  // FULL REPETITION DETECTION
  const u64 tt_hash = get_hash(pos);
  bool in_qsearch = depth <= 0;
  for (i32 i = G(169, ply) + G(169, pos_history_count);
       G(170, i >= 0) && G(170, do_null); i -= 2) {
    if (G(171, tt_hash) == G(171, stack[i].position_hash)) {
      return 0;
    }
  }

  // TT PROBING
  G(172, TTEntry *tt_entry = &tt[tt_hash % tt_length];)
  G(172, const u16 tt_hash_partial = tt_hash / tt_length;)
  G(172, stack[ply].best_move = (Move){0};)
  if (G(173, tt_entry->partial_hash) == G(173, tt_hash_partial)) {
    stack[ply].best_move = tt_entry->move;

    // TT PRUNING
    if (G(174, tt_entry->depth >= depth) &&
        G(174, G(175, tt_entry->flag) != G(175, tt_entry->score <= alpha)) &&
        G(174, G(176, alpha) == G(176, beta - 1))) {
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
  if (G(177, G(178, tt_entry->partial_hash) == G(178, tt_hash_partial)) &&
      G(177, G(179, tt_entry->flag) != G(179, static_eval) > tt_entry->score)) {
    static_eval = tt_entry->score;
  }

  // QUIESCENCE
  if (G(180, static_eval > alpha) && G(180, in_qsearch)) {
    if (static_eval >= beta) {
      return static_eval;
    }
    alpha = static_eval;
  }

  if (G(181, !in_check) && G(181, G(182, alpha) == G(182, beta - 1))) {
    if (G(183, depth < 8) && G(183, !in_qsearch)) {

      G(184, {
        // REVERSE FUTILITY PRUNING
        if (static_eval - G(185, 56) * G(185, (depth - improving)) >= beta) {
          return static_eval;
        }
      })

      G(184, // RAZORING
        in_qsearch =
            G(186, static_eval) + G(186, G(187, 122) * G(187, depth)) <= alpha;)
    }

    // NULL MOVE PRUNING
    if (G(188, depth > 2) && G(188, static_eval >= beta) && G(188, do_null)) {
      Position npos = *pos;
      G(189, flip_pos(&npos);)
      G(189, npos.ep = 0;)
      const i32 score = -search(
          H(167, 2, false), H(167, 2, -alpha),
          H(167, 2, depth - G(190, 4) - G(190, depth / 4)), H(167, 2, stack),
          H(167, 2, -beta),
#ifdef FULL
                  nodes,
#endif
                  H(168, 2, &npos), H(168, 2, max_time),
                  H(168, 2, pos_history_count), H(168, 2, move_history), H(168, 2, ply + 1));
      if (score >= beta) {
        return score;
      }
    }
  }

  G(191, Move moves[max_moves];
    stack[ply].num_moves =
        movegen(H(100, 3, pos), H(100, 3, moves), H(100, 3, in_qsearch));)
  G(191, i32 best_score = in_qsearch ? static_eval : -inf;)
  G(191, u8 tt_flag = Upper;)
  G(191, stack[G(192, pos_history_count) + G(192, ply) + G(192, 2)].position_hash =
        tt_hash;)
  G(191, i32 moves_evaluated = 0;)
  G(191, i32 quiets_evaluated = 0;)

  for (i32 move_index = 0; move_index < stack[ply].num_moves; move_index++) {
    // MOVE ORDERING
    G(193, i32 best_index = 0;)
    G(193, i32 move_score = ~0x1010101LL;)
    for (i32 order_index = move_index; order_index < stack[ply].num_moves;
         order_index++) {
      assert(
          stack[ply].moves[order_index].takes_piece ==
          piece_on(H(55, 7, pos), H(55, 7, stack[ply].moves[order_index].to)));
      const i32 order_move_score =
          G(167, // KILLER MOVE
            G(194, move_equal(G(195, &moves[order_index]),
                              G(195, &stack[ply].killer))) *
                G(194, 836)) +
          G(167, // PREVIOUS BEST MOVE FIRST
            (move_equal(G(196, &stack[ply].best_move),
                        G(196, &moves[order_index]))
             << 30)) +
          G(167, // MOST VALUABLE VICTIM
            G(197, moves[order_index].takes_piece) * G(197, 712)) +
          G(167, // HISTORY HEURISTIC
            move_history[pos->flipped][moves[order_index].takes_piece]
                        [moves[order_index].from][moves[order_index].to]);
      if (order_move_score > move_score) {
        G(198, best_index = order_index;)
        G(198, move_score = order_move_score;)
      }
    }

    swapmoves(G(199, &moves[move_index]), G(199, &moves[best_index]));

    G(200, // MOVE SCORE PRUNING
        if (G(204, move_score < G(205, -128) * G(205, depth)) &&
            G(204, moves_evaluated)) { break; })

    G(200, // FORWARD FUTILITY PRUNING / DELTA PRUNING
        if (G(201, depth < 8) &&
            G(201, G(202, static_eval) + G(202, G(203, 142) * G(203, depth)) +
                      G(202, initial_params.eg
                                 .material[moves[move_index].takes_piece]) +
                      G(202, initial_params.eg.material[moves[move_index].promo]) <
                  alpha) &&
            G(201, !in_check) && G(201, moves_evaluated)) { break; })

    Position npos = *pos;
#ifdef FULL
    (*nodes)++;
#endif
    if (!makemove(H(85, 3, &npos), H(85, 3, &moves[move_index]))) {
      continue;
    }

    // PRINCIPAL VARIATION SEARCH
    i32 low = moves_evaluated == 0 ? -beta : -alpha - 1;
    moves_evaluated++;

    // LATE MOVE REDUCTION
    i32 reduction = G(206, depth > 3) && G(206, move_score <= 0)
                        ? G(207, (move_score < -256)) +
                              G(207, (G(208, alpha) == G(208, beta - 1))) +
                              G(207, moves_evaluated / 10) + G(207, !improving)
                        : 0;

    i32 score;
    while (true) {
      score = -search(H(167, 3, true), H(167, 3, -alpha),
                      H(167, 3, depth - G(209, 1) - G(209, reduction)),
                      H(167, 3, stack), H(167, 3, low),
#ifdef FULL
                      nodes,
#endif
                      H(168, 3, &npos), H(168, 3, max_time),
                      H(168, 3, pos_history_count), H(168, 3, move_history), H(168, 3, ply + 1));

      // EARLY EXITS
      if (stop || (depth > 4 && get_time() - start_time > max_time)) {
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
        G(210, tt_flag = Exact;)
        G(210, stack[ply].best_move = moves[move_index];)
        G(210, alpha = score;)
        if (score >= beta) {
          assert(stack[ply].best_move.takes_piece ==
                 piece_on(H(55, 8, pos), H(55, 8, stack[ply].best_move.to)));
          G(211, tt_flag = Lower;)
          G(211, if (stack[ply].best_move.takes_piece == None) {
                stack[ply].killer = stack[ply].best_move;
              })
          G(211, if (!in_qsearch) {
                const i32 bonus = depth * depth;
                G(212, i32 *const this_hist =
                           &move_history[pos->flipped]
                                        [stack[ply].best_move.takes_piece]
                                        [stack[ply].best_move.from]
                                        [stack[ply].best_move.to];

                  *this_hist +=
                  bonus - G(213, bonus) * G(213, *this_hist) / 1024;)
                G(212, for (i32 prev_index = 0; prev_index < move_index;
                              prev_index++) {
                      const Move prev = moves[prev_index];
                      i32 *const prev_hist =
                          &move_history[pos->flipped][prev.takes_piece]
                                       [prev.from][prev.to];
                      *prev_hist -=
                          bonus + G(214, bonus) * G(214, *prev_hist) / 1024;
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
    if (G(215, G(216, alpha) == G(216, beta - 1)) &&
        G(215, quiets_evaluated > (G(217, 1) + G(217, depth * depth)) >>
                   !improving) &&
        G(215, !in_check)) {
      break;
    }
  }

  // MATE / STALEMATE DETECTION
  if (G(218, best_score) == G(218, -inf)) {
    return G(219, (ply - mate)) * G(219, in_check);
  }

  *tt_entry = (TTEntry){.partial_hash = tt_hash_partial,
                        .move = stack[ply].best_move,
                        .score = best_score,
                        .depth = depth,
                        .flag = tt_flag};

  return best_score;
}

S(1) void init() {
  G(85, // INIT DIAGONAL MASKS
      for (i32 sq = 0; sq < 64; sq++) {
        const u64 bb = 1ULL << sq;
        G(220, u64 sw_bb = southwest(bb);)
        G(220, u64 ne_bb = northeast(bb);)
        for (i32 i = 6; i > 0; i--) {
          G(221, sw_bb |= southwest(sw_bb);)
          G(221, ne_bb |= northeast(ne_bb);)
        }
        diag_mask[sq] = G(222, sw_bb) | G(222, ne_bb);
      })
  G(85, // MERGE EVAL PARAMS
      for (i32 i = 0; i < sizeof(EvalParamsMerged) / sizeof(i32); i++) {
        ((i32 *)&eval_params)[i] =
            combine_eval_param(H(126, 2, i < 6 ? initial_params.mg.material[i]
                                       : ((i8 *)&initial_params.mg)[6 + i]),
                               H(126, 2, i < 6 ? initial_params.eg.material[i]
                                       : ((i8 *)&initial_params.eg)[6 + i]));
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
    move_str(H(57, 2, move_name), H(57, 2, &pv_move), H(57, 2, pos->flipped));
    putl(move_name);
  }

  putl("\n");
}
#endif

S(1)
void iteratively_deepen(
#ifdef FULL
    i32 maxdepth, u64 *nodes, i32 thread_id,
#endif
    H(223, 1, const u64 max_time),
    H(223, 1, const i32 pos_history_count),
    H(223, 1, i32 move_history[2][6][64][64]),
  H(223, 1, Position *const restrict pos), H(223, 1, SearchStack *restrict stack)) {
  i32 score = 0;
#ifdef FULL
  for (i32 depth = 1; depth < maxdepth; depth++) {
#else
  for (i32 depth = 1; depth < max_ply; depth++) {
#endif
    // ASPIRATION WINDOWS
    G(225, i32 window = 15;)
    G(225, size_t elapsed;)
    while (true) {
      G(226, const i32 beta = G(227, score) + G(227, window);)
      G(226, const i32 alpha = score - window;)
      score =
          search(H(167, 4, false), H(167, 4, beta), H(167, 4, depth),
                 H(167, 4, stack), H(167, 4, alpha),
#ifdef FULL
                     nodes,
#endif
                     H(168, 4, pos), H(168, 4, max_time), H(168, 4, pos_history_count),
                     H(168, 4, move_history), H(168, 4, 0));
#ifdef FULL
      if (thread_id == 0) {
        print_info(pos, depth, alpha, beta, score, *nodes, stack[0].best_move,
                   max_time);
      }
#endif
      elapsed = get_time() - start_time;
      G(228, window *= 2;)
      G(228, if (G(229, elapsed > max_time) ||
                   G(229, (G(230, score > alpha) && G(230, score < beta)))) {
            break;
          })
    }

    if (stop || elapsed > max_time / 16) {
      break;
    }
  }
}

typedef struct __attribute__((aligned(16))) ThreadDataStruct {
  void (*entry)(struct ThreadDataStruct*);
#ifdef FULL
  i32 thread_id;
  u64 nodes;
#endif
  G(998, Position pos;)
  G(998, SearchStack stack[1024];)
  G(998, i32 pos_history_count;)
  G(998, i32 move_history[2][6][64][64];)
  G(998, u64 max_time;)
} ThreadData;

S(1) void *thread_fun(void *param) {
  ThreadData *data = param;
  iteratively_deepen(
#ifdef FULL
    max_ply, &data->nodes, data->thread_id,
#endif
    H(223, 2, data->max_time), H(223, 2, data->pos_history_count),
    H(223, 2, data->move_history),
    H(223, 2, &data->pos), H(223, 2, data->stack));
  return NULL;
}

S(1) void threadentry(ThreadData* data) {

  iteratively_deepen(
#ifdef FULL
  max_ply, &data->nodes, data->thread_id,
#endif
    H(223, 3, data->max_time), H(223, 3, data->pos_history_count),
    H(223, 3, data->move_history),
    H(223, 3, &data->pos), H(223, 3, data->stack));

  exit_now();
}

__attribute__((naked)) S(1) long newthread(ThreadData* stack)
{
  asm volatile (
    "mov  rsi, rdi\n"         // arg2 = stack
    "mov  edi, 0x50f00\n"     // arg1 = clone flags
    "mov  eax, 56\n"          // SYS_clone
    "syscall\n"
    "mov  rdi, rsp\n"         // entry point argument
    "ret\n"
    : : : "rax", "rcx", "rsi", "rdi", "r11", "memory"
    );

}

enum { thread_count = 4 };
enum { thread_stack_size = 8 * 1024 * 1024 };

_Static_assert(sizeof(ThreadData) < thread_stack_size);

__attribute__((aligned(4096))) u8 thread_stacks[thread_count][thread_stack_size];

S(1)
void run_smp(const u64 max_time) {
  start_time = get_time();
#ifdef FULL
  pthread_t helpers[thread_count - 1];
  u64 nodes = 0;
#endif

  ThreadData* main_data = (ThreadData*)&thread_stacks[0][0];

  for (i32 i = 1; i < thread_count; i++) {
    ThreadData* helper_data = (ThreadData*)&thread_stacks[i][0];
    G(999,
      // * pos_history_count?
      __builtin_memcpy(helper_data->stack, main_data->stack, sizeof(SearchStack) * 1024);)
    G(999, helper_data->pos = main_data->pos;)
    G(999, helper_data->pos_history_count = main_data->pos_history_count;)
    G(999, helper_data->max_time = -1LL;)
#ifdef FULL
    helper_data->thread_id = i;
    pthread_create(&helpers[i - 1], NULL, thread_fun, helper_data);
#else
    helper_data->entry = threadentry;
    newthread(helper_data);
#endif
  }

  iteratively_deepen(
#ifdef FULL
    max_ply, &nodes, 0,
#endif
    H(223, 4, max_time), H(223, 4, main_data->pos_history_count), H(223, 4, main_data->move_history), H(223, 4, &main_data->pos), H(223, 4, main_data->stack));
  stop = true;

  for (i32 i = 0; i < thread_count - 1; i++) {
#ifdef FULL
    pthread_join(helpers[i], NULL);
#else
    // TODO: sync ?
#endif
  }

  char move_name[8];
  move_str(H(57, 3, move_name), H(57, 3, &main_data->stack[0].best_move), H(57, 3, main_data->pos.flipped));
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
  Position pos;
  i32 pos_history_count = 0;
  i32 move_history[2][6][64][64];
  SearchStack stack[1024];
  __builtin_memset(move_history, 0, sizeof(move_history));
  pos = start_pos;
  u64 nodes = 0;
  stop = false;
  const u64 start = get_time();
  start_time = start;
  iteratively_deepen(23, &nodes, 0, H(223, 5, -1LL), H(223, 5, pos_history_count, H(223, 5, move_history), H(223, 5, &pos)),
                     H(223, 5, stack));
  const u64 end = get_time();
  const u64 elapsed = end - start;
  const u64 nps = elapsed ? nodes * 1000 * 1000 * 1000U / elapsed : 0;
  printf("%llu nodes %llu nps\n", nodes, nps);
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

  G(231, char line[4096];)
  G(231, init();)
  G(231, __builtin_memset(thread_stacks, 0, sizeof(thread_stacks));)
  G(231, ThreadData* main_data = (ThreadData*)&thread_stacks[0][0];)

#ifdef FULL
  main_data->pos = start_pos;
  main_data->pos_history_count = 0;
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
      run_smp(-1LL);
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
    G(232, if (G(234, line[0]) == G(234, 'g')) {
      stop = false;
      u64 max_time;
#ifdef FULL
      while (true) {
        getl(line);
        if (!main_data->pos.flipped && !strcmp(line, "wtime")) {
          getl(line);
          max_time = (u64)atoi(line) << 19; // Roughly /2 time
          break;
        }
        else if (main_data->pos.flipped && !strcmp(line, "btime")) {
          getl(line);
          max_time = (u64)atoi(line) << 19; // Roughly /2 time
          break;
        }
        else if (!strcmp(line, "movetime")) {
          max_time = 20ULL * 1000 * 1000 * 1000; // Assume Lichess bot
          break;
        }
      }
      run_smp(max_time);
#else
      for (i32 i = 2 << main_data->pos.flipped; i > 0; i--) {
        getl(line);
        max_time = (u64)atoi(line) << 19; // Roughly /2 time
      }
      start_time = get_time();
      run_smp(max_time);
#endif
    })
    else G(232, if (G(233, line[0]) == G(233, 'i')) { puts("readyok"); })
    else G(232, if (G(236, line[0]) == G(236, 'p')) {
      G(237, main_data->pos_history_count = 0;)
        G(237, main_data->pos = start_pos;)
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
            movegen(H(100, 4, &main_data->pos), H(100, 4, moves), H(100, 4, false));
          for (i32 i = 0; i < num_moves; i++) {
            char move_name[8];
            move_str(H(57, 4, move_name), H(57, 4, &moves[i]),
              H(57, 4, main_data->pos.flipped));
            assert(move_string_equal(line, move_name) ==
              !strcmp(line, move_name));
            if (move_string_equal(G(238, move_name), G(238, line))) {
              main_data->stack[main_data->pos_history_count].position_hash = get_hash(&main_data->pos);
              main_data->pos_history_count++;
              if (moves[i].takes_piece) {
                main_data->pos_history_count = 0;
              }
              makemove(H(85, 4, &main_data->pos), H(85, 4, &moves[i]));
              break;
            }
          }
          if (!line_continue) {
            break;
          }
        }
    })
    else G(232, if (G(235, line[0]) == G(235, 'q')) { exit_now(); })
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
