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
        if (G(4, ch == '\n') || G(4, ch == ' ')) {
          *string = 0;
          return ch != '\n';
        }

        string++;
      }
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
    19, [[nodiscard]] S(1) u64 se(const u64 bb) {
      return G(20, G(21, east)(G(21, south)(bb)));
      return G(20, shift(H(13, 2, -7), H(13, 2, ~0x101010101010101ull),
                         H(13, 2, bb)));
    })

G(
    19, [[nodiscard]] S(1) u64 ne(const u64 bb) {
      return G(22, G(23, east)(G(23, north)(bb)));
      return G(22, shift(H(13, 3, 9), H(13, 3, ~0x101010101010101ull),
                         H(13, 3, bb)));
    })

G(
    19, [[nodiscard]] S(1) u64 nw(const u64 bb) {
      return G(24, shift(H(13, 4, 7), H(13, 4, ~0x8080808080808080ull),
                         H(13, 4, bb)));
      return G(24, G(25, west)(G(25, north)(bb)));
    })

G(
    19, [[nodiscard]] S(1) u64 sw(const u64 bb) {
      return G(26, G(27, west)(G(27, south)(bb)));
      return G(26, shift(H(13, 5, -9), H(13, 5, ~0x8080808080808080ull),
                         H(13, 5, bb)));
    })

G(28, S(0) u64 diag_mask[64];)

G(
    28, [[nodiscard]] S(1)
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

G(
    28, [[nodiscard]] S(0) u64 xattack(H(30, 1, const u64 dir_mask),
                                       H(30, 1, const u64 bb),
                                       H(30, 1, const u64 blockers)) {
      return G(31, dir_mask) &
             G(31, (G(32, (G(33, blockers) & G(33, dir_mask)) - bb) ^
                    G(32, flip_bb(flip_bb(G(34, blockers) & G(34, dir_mask)) -
                                  flip_bb(bb)))));
    })

G(
    35, [[nodiscard]] S(0) u64 bishop(H(36, 1, const u64 blockers),
                                      H(36, 1, const u64 bb)) {
      assert(count(bb) == 1);
      const i32 sq = lsb(bb);
      return G(37, xattack(H(30, 2, diag_mask[sq]), H(30, 2, bb),
                           H(30, 2, blockers))) |
             G(37, xattack(H(30, 3, flip_bb(diag_mask[G(38, sq) ^ G(38, 56)])),
                           H(30, 3, bb), H(30, 3, blockers)));
    })

G(
    35, [[nodiscard]] S(1) u64 rook(H(39, 1, const u64 blockers),
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

G(
    35, [[nodiscard]] S(1) u64 knight(const u64 bb) {
      return G(41, G(42, (G(43, bb << 6) | G(43, bb >> 10))) &
                       G(42, 0x3F3F3F3F3F3F3F3Full)) |
             G(41, G(44, (G(45, bb >> 6) | G(45, bb << 10))) &
                       G(44, 0xFCFCFCFCFCFCFCFCull)) |
             G(41, G(46, (G(47, bb << 17) | G(47, bb >> 15))) &
                       G(46, ~0x101010101010101ull)) |
             G(41, G(48, (G(49, bb << 15 | G(49, bb >> 17)))) &
                       G(48, ~0x8080808080808080ull));
    })

G(
    35, [[nodiscard]] S(1) u64 king(const u64 bb) {
      return G(50, bb << 8) | G(50, bb >> 8) |
             G(50, G(51, (G(52, bb << 9) | G(52, bb >> 7) | G(52, bb << 1))) &
                       G(51, ~0x101010101010101ull)) |
             G(50, G(53, (G(54, bb << 7) | G(54, bb >> 9) | G(54, bb >> 1))) &
                       G(53, ~0x8080808080808080ull));
    })

G(
    55,
    S(1) void swapu32(G(56, u32 *const rhs), G(56, u32 *const lhs)) {
      const u32 temp = *lhs;
      *lhs = *rhs;
      *rhs = temp;
    }

    S(1) void swapmoves(G(57, Move *const lhs), G(57, Move *const rhs)) {
      swapu32(G(58, (u32 *)lhs), G(58, (u32 *)rhs));
    })

G(
    55, [[nodiscard]] S(1) bool move_equal(G(59, Move *const rhs),
                                           G(59, Move *const lhs)) {
      return G(60, *(u32 *)lhs) == G(60, *(u32 *)rhs);
    })

G(
    55, S(1) void swapu64(G(61, u64 *const rhs), G(61, u64 *const lhs)) {
      const u64 temp = *lhs;
      *lhs = *rhs;
      *rhs = temp;
    })

G(
    55, S(1) void move_str(H(64, 1, char *restrict str),
                           H(64, 1, const Move *restrict move),
                           H(64, 1, const i32 flip)) {
      assert(move->from >= 0);
      assert(move->from < 64);
      assert(move->to >= 0);
      assert(move->to < 64);
      assert(move->from != move->to);
      assert(move->promo == None || move->promo == Knight ||
             move->promo == Bishop || move->promo == Rook ||
             move->promo == Queen);

      G(65, str[5] = '\0';)

      G(65, str[4] = "\0\0nbrq"[move->promo];)
      G(
          65, // Hack to save bytes, technically UB but works on GCC 14.2
          for (i32 i = 1; i >= 0; i--) {
            G(66, str[i * 2] = 'a' + (&move->from)[i] % 8;)
            G(66, str[i * 2 + 1] = '1' + ((&move->from)[i] / 8 ^ 7 * flip);)
          })
    })

G(
    55, [[nodiscard]] S(1)
            i32 piece_on(H(62, 1, const Position *const restrict pos),
                         H(62, 1, const i32 sq)) {
              assert(sq >= 0);
              assert(sq < 64);
              for (i32 i = Pawn; i <= King; ++i) {
                if (G(63, pos->pieces[i]) & G(63, 1ull << sq)) {
                  return i;
                }
              }
              return None;
            })

G(
    55, S(1) void swapu16(G(64, u16 *const restrict lhs),
                          G(64, u16 *const restrict rhs)) {
      const u16 temp = *lhs;
      *lhs = *rhs;
      *rhs = temp;
    })

G(
    67,
    [[nodiscard]] S(1) i32 is_attacked(H(68, 1,
                                         const Position *const restrict pos),
                                       H(68, 1, const u64 bb)) {
      assert(count(bb) == 1);
      const u64 theirs = pos->colour[1];
      G(69, const u64 pawns = theirs & pos->pieces[Pawn];)
      G(69, const u64 blockers = theirs | pos->colour[0];)
      return G(70, G(71, (G(72, sw(pawns)) | G(72, se(pawns)))) & G(71, bb)) ||
             G(70, G(73, bishop(H(36, 2, blockers), H(36, 2, bb))) &
                       G(73, theirs) &
                       G(73, (pos->pieces[Bishop] | pos->pieces[Queen]))) ||
             G(70,
               G(74, king(bb)) & G(74, theirs) & G(74, pos->pieces[King])) ||
             G(70, G(75, knight(bb)) & G(75, theirs) &
                       G(75, pos->pieces[Knight])) ||
             G(70, G(76, (pos->pieces[Rook] | pos->pieces[Queen])) &
                       G(76, rook(H(39, 2, blockers), H(39, 2, bb))) &
                       G(76, theirs));
    })

G(
    67, S(0) void flip_pos(Position *const restrict pos) {
      G(77, swapu64(G(78, &pos->colour[0]), G(78, &pos->colour[1]));)

      G(
          77, // Hack to flip the first 10 bitboards in Position.
              // Technically UB but works in GCC 14.2
          u64 *pos_ptr = (u64 *)pos;
          for (i32 i = 0; i < 10; i++) { pos_ptr[i] = flip_bb(pos_ptr[i]); })
      G(77, // Swap castling index 0 with 2, 1 with 3
        swapu16(G(79, &((u16 *)pos->castling)[0]),
                G(79, &((u16 *)pos->castling)[1]));)
      G(77, pos->flipped ^= 1;)
    })

G(
    67, [[nodiscard]] S(1) u64 get_mobility(H(80, 1, const Position *pos),
                                            H(80, 1, const i32 sq),
                                            H(80, 1, const i32 piece)) {
      u64 moves = 0;
      const u64 bb = 1ULL << sq;
      G(81, if (piece == King) { moves = king(bb); })
      else G(81, if (piece == Knight) { moves = knight(bb); }) else {
        const u64 blockers = G(82, pos->colour[1]) | G(82, pos->colour[0]);
        G(
            83, if (G(84, piece == Queen) || G(84, piece == Bishop)) {
              moves |= bishop(H(36, 3, blockers), H(36, 3, bb));
            })
        G(
            83, if (G(85, piece == Queen) || G(85, piece == Rook)) {
              moves |= rook(H(39, 3, blockers), H(39, 3, bb));
            })
      }
      return moves;
    })

S(0) i32 find_in_check(const Position *restrict pos) {
  return is_attacked(
      H(68, 2, pos),
      H(68, 2, G(86, pos->colour[0]) & G(86, pos->pieces[King])));
}

G(
    87,
    S(1) Move *generate_piece_moves(H(88, 1, const u64 to_mask),
                                    H(88, 1, Move *restrict movelist),
                                    H(88, 1, const Position *restrict pos)) {
      for (i32 piece = Knight; piece <= King; piece++) {
        assert(piece == Knight || piece == Bishop || piece == Rook ||
               piece == Queen || piece == King);
        u64 copy = G(89, pos->colour[0]) & G(89, pos->pieces[piece]);
        while (copy) {
          const u8 from = lsb(copy);
          assert(from >= 0);
          assert(from < 64);
          G(90, copy &= copy - 1;)

          G(90, u64 moves = G(91, to_mask) &
                            G(91, get_mobility(H(80, 2, pos), H(80, 2, from),
                                               H(80, 2, piece)));)

          while (moves) {
            const u8 to = lsb(moves);
            assert(to >= 0);
            assert(to < 64);

            G(92, moves &= moves - 1;)
            G(92, *movelist++ = ((Move){
                      .from = from,
                      .to = to,
                      .promo = None,
                      .takes_piece = piece_on(H(62, 2, pos), H(62, 2, to))});)
          }
        }
      }

      return movelist;
    })

G(
    87, S(0) i32 makemove(H(93, 1, Position *const restrict pos),
                          H(93, 1, const Move *const restrict move)) {
      assert(move->from >= 0);
      assert(move->from < 64);
      assert(move->to >= 0);
      assert(move->to < 64);
      assert(move->from != move->to);
      assert(move->promo == None || move->promo == Knight ||
             move->promo == Bishop || move->promo == Rook ||
             move->promo == Queen);
      assert(move->takes_piece != King);
      assert(move->takes_piece == piece_on(H(62, 3, pos), H(62, 3, move->to)));

      G(94, const u64 from = 1ull << move->from;)
      G(94, const u64 to = 1ull << move->to;)
      G(95, const i32 piece = piece_on(H(62, 4, pos), H(62, 4, move->from));
        assert(piece != None);)
      G(95, const u64 mask = G(96, from) | G(96, to);)

      G(
          97, // Castling
          if (piece == King) {
            const u64 bb = move->to - move->from == 2   ? 0xa0
                           : move->from - move->to == 2 ? 0x9
                                                        : 0;
            G(98, pos->pieces[Rook] ^= bb;)
            G(98, pos->colour[0] ^= bb;)
          })

      G(97, pos->colour[0] ^= mask;)

      // Move the piece
      G(97, pos->pieces[piece] ^= mask;)
      G(
          97, // Captures
          if (move->takes_piece != None) {
            G(99, pos->colour[1] ^= to;)
            G(99, pos->pieces[move->takes_piece] ^= to;)
          })

      // En passant
      if (G(100, piece == Pawn) && G(100, to == pos->ep)) {
        G(101, pos->colour[1] ^= to >> 8;)
        G(101, pos->pieces[Pawn] ^= to >> 8;)
      }
      pos->ep = 0;

      G(
          102, // Pawn double move
          if (G(103, move->to - move->from == 16) && G(103, piece == Pawn)) {
            pos->ep = to >> 8;
          })

      G(
          102, // Promotions
          if (move->promo != None) {
            G(104, pos->pieces[Pawn] ^= to;)
            G(104, pos->pieces[move->promo] ^= to;)
          })

      G(102, // Update castling permissions
        G(105, pos->castling[1] &= !(mask & 0x11ull);)
            G(105, pos->castling[3] &= !(mask & 0x1100000000000000ull);)
                G(105, pos->castling[2] &= !(mask & 0x9000000000000000ull);)
                    G(105, pos->castling[0] &= !(mask & 0x90ull);))

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
    87, S(0) Move *generate_pawn_moves(H(106, 1, const i32 offset),
                                       H(106, 1, Move *restrict movelist),
                                       H(106, 1, u64 to_mask),
                                       H(106, 1, const Position *const pos)) {
      while (to_mask) {
        const u8 to = lsb(to_mask);
        to_mask &= to_mask - 1;
        const u8 from = G(107, to) + G(107, offset);
        assert(from >= 0);
        assert(from < 64);
        assert(to >= 0);
        assert(to < 64);
        assert(piece_on(H(62, 5, pos), H(62, 5, from)) == Pawn);
        const u8 takes = piece_on(H(62, 6, pos), H(62, 6, to));
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

[[nodiscard]] S(1) i32 movegen(H(108, 1, const Position *const restrict pos),
                               H(108, 1, Move *restrict movelist),
                               H(108, 1, const i32 only_captures)) {

  G(109, const u64 all = G(110, pos->colour[0]) | G(110, pos->colour[1]);)
  G(109, const Move *start = movelist;)
  G(109, const u64 to_mask = only_captures ? pos->colour[1] : ~pos->colour[0];)
  G(
      111, // PAWN PROMOTIONS
      if (!only_captures) {
        movelist = generate_pawn_moves(
            H(106, 2, -16), H(106, 2, movelist),
            H(106, 2,
              G(112, north(G(113, north(G(114, pos->colour[0]) &
                                        G(114, pos->pieces[Pawn]) &
                                        G(114, 0xFF00))) &
                           G(113, ~all))) &
                  G(112, ~all)),
            H(106, 2, pos));
      })
  G(111, // PAWN DOUBLE MOVES
    movelist = generate_pawn_moves(
        H(106, 3, -8), H(106, 3, movelist),
        H(106, 3,
          north(G(115, G(116, pos->colour[0]) & G(116, pos->pieces[Pawn]))) &
              G(115, ~all) &
              G(115, (only_captures ? 0xFF00000000000000ull : ~0ull))),
        H(106, 3, pos));)
  G(111, // PAWN WEST CAPTURES
    movelist = generate_pawn_moves(
        H(106, 4, -7), H(106, 4, movelist),
        H(106, 4,
          G(117, nw(G(118, pos->colour[0]) & G(118, pos->pieces[Pawn]))) &
              G(117, (G(119, pos->colour[1]) | G(119, pos->ep)))),
        H(106, 4, pos));)
  G(111, // PAWN EAST CAPTURES
    movelist = generate_pawn_moves(
        H(106, 5, -9), H(106, 5, movelist),
        H(106, 5,
          G(120, ne(G(121, pos->colour[0]) & G(121, pos->pieces[Pawn]))) &
              G(120, (G(122, pos->colour[1]) | G(122, pos->ep)))),
        H(106, 5, pos));)
  G(
      111, // LONG CASTLE
      if (G(123, !only_captures) && G(123, pos->castling[0]) &&
          G(123, !(G(124, all) & G(124, 0x60ull))) &&
          G(125, !is_attacked(H(68, 3, pos), H(68, 3, 1ULL << 4))) &&
          G(125, !is_attacked(H(68, 4, pos), H(68, 4, 1ULL << 5)))) {
        *movelist++ =
            (Move){.from = 4, .to = 6, .promo = None, .takes_piece = None};
      })
  G(
      111, // SHORT CASTLE
      if (G(126, !only_captures) && G(126, pos->castling[1]) &&
          G(126, !(G(127, all) & G(127, 0xEull))) &&
          G(128, !is_attacked(H(68, 5, pos), H(68, 5, 1ULL << 4))) &&
          G(128, !is_attacked(H(68, 6, pos), H(68, 6, 1ULL << 3)))) {
        *movelist++ =
            (Move){.from = 4, .to = 2, .promo = None, .takes_piece = None};
      })
  movelist = generate_piece_moves(H(88, 2, to_mask), H(88, 2, movelist),
                                  H(88, 2, pos));

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
      movegen(H(108, 2, pos), H(108, 2, moves), H(108, 2, false));

  for (i32 i = 0; i < num_moves; ++i) {
    Position npos = *pos;

    // Check move legality
    if (!makemove(H(93, 2, &npos), H(93, 2, &moves[i]))) {
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
  H(129, 1,
    H(130, 1, i8 tempo;) H(130, 1, i8 pawn_attacked_penalty[2];)
        H(130, 1, i8 mobilities[5];) H(130, 1, i8 passed_blocked_pawns[6];)
            H(130, 1, i8 open_files[6];) H(130, 1, i8 pst_file[48];))
  H(129, 1,
    H(131, 1, i8 protected_pawn;) H(131, 1, i8 bishop_pair;)
        H(131, 1, i8 phalanx_pawn;) H(131, 1, i8 passed_pawns[6];)
            H(131, 1, i8 king_attacks[5];) H(131, 1, i8 pst_rank[48];))
} EvalParams;

typedef struct [[nodiscard]] __attribute__((packed)) {
  i32 material[6];
  H(129, 2,
    H(130, 2, i32 tempo;) H(130, 2, i32 pawn_attacked_penalty[2];)
        H(130, 2, i32 mobilities[5];) H(130, 2, i32 passed_blocked_pawns[6];)
            H(130, 2, i32 open_files[6];) H(130, 2, i32 pst_file[48];))
  H(129, 2,
    H(131, 2, i32 protected_pawn;) H(131, 2, i32 bishop_pair;)
        H(131, 2, i32 phalanx_pawn;) H(131, 2, i32 passed_pawns[6];)
            H(131, 2, i32 king_attacks[5];) H(131, 2, i32 pst_rank[48];))

} EvalParamsMerged;

typedef struct [[nodiscard]] __attribute__((packed)) {
  i8 phases[6];
  G(132, EvalParams mg;)
  G(132, EvalParams eg;)
} EvalParamsInitial;

G(133, S(0) EvalParamsMerged eval_params;)

G(133,
  __attribute__((aligned(8))) S(1)
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

G(
    133,
    [[nodiscard]] S(1) i32 combine_eval_param(H(134, 1, const i32 mg_val),
                                              H(134, 1, const i32 eg_val)) {
      return G(135, mg_val) + G(135, (eg_val << 16));
    })

S(1) i32 eval(Position *const restrict pos) {
  G(136, i32 score = eval_params.tempo;)
  G(136, i32 phase = 0;)

  for (i32 c = 0; c < 2; c++) {

    G(137, const u64 opp_king_zone =
               king(G(138, pos->colour[1]) & G(138, pos->pieces[King]));)

    G(137,
      const u64 own_pawns = G(139, pos->pieces[Pawn]) & G(139, pos->colour[0]);)
    G(137,
      const u64 opp_pawns = G(140, pos->pieces[Pawn]) & G(140, pos->colour[1]);
      const u64 attacked_by_pawns =
          G(141, se(opp_pawns)) | G(141, sw(opp_pawns));
      G(142,
        const u64 no_passers = G(143, opp_pawns) | G(143, attacked_by_pawns);)
          G(142, // PROTECTED PAWNS
            score -=
            G(144, eval_params.protected_pawn) *
            G(144, count(G(145, opp_pawns) & G(145, attacked_by_pawns)));)
              G(142, // PHALANX PAWNS
                score -=
                G(146, eval_params.phalanx_pawn) *
                G(146, count(G(147, opp_pawns) & G(147, west(opp_pawns))));))
    G(
        137, // BISHOP PAIR
        if (count(G(148, pos->pieces[Bishop]) & G(148, pos->colour[0])) > 1) {
          score += eval_params.bishop_pair;
        })

    for (i32 p = Pawn; p <= King; p++) {
      u64 copy = G(149, pos->colour[0]) & G(149, pos->pieces[p]);
      while (copy) {
        const i32 sq = lsb(copy);
        G(150, const i32 rank = sq >> 3;)
        G(150, copy &= copy - 1;)
        G(150, phase += initial_params.phases[p];)
        G(150, const i32 file = G(151, sq) & G(151, 7);)

        G(
            106, // OPEN FILES / DOUBLED PAWNS
            if ((G(152, north(0x101010101010101ULL << sq)) &
                 G(152, own_pawns)) == 0) {
              score += eval_params.open_files[p - 1];
            })

        G(
            106, // PASSED PAWNS
            if (G(153, p == Pawn) &&
                G(153, !(G(154, (0x101010101010101ULL << sq)) &
                         G(154, no_passers)))) {
              G(
                  155, if (G(156, north(1ULL << sq)) & G(156, pos->colour[1])) {
                    score += eval_params.passed_blocked_pawns[rank - 1];
                  })

              G(155, score += eval_params.passed_pawns[rank - 1];)
            })
        G(106, // SPLIT PIECE-SQUARE TABLES FOR RANK
          score +=
          eval_params
              .pst_rank[G(157, G(158, (p - 1)) * G(158, 8)) + G(157, rank)];)

        G(106, // SPLIT PIECE-SQUARE TABLES FOR FILE
          score +=
          eval_params
              .pst_file[G(159, G(160, (p - 1)) * G(160, 8)) + G(159, file)];)

        G(106, // MATERIAL
          score += eval_params.material[p];)

        G(
            106, if (p > Pawn) {
              G(
                  161, // PIECES ATTACKED BY PAWNS
                  if (G(162, 1ULL << sq) & G(162, no_passers)) {
                    score += eval_params.pawn_attacked_penalty[c];
                  })

              G(161, const u64 mobility =
                         G(163, get_mobility(H(80, 3, pos), H(80, 3, sq),
                                             H(80, 3, p))) &
                         G(163, ~attacked_by_pawns);

                G(164, // MOBILITY
                  score +=
                  G(165, count(G(166, ~pos->colour[0]) & G(166, mobility))) *
                  G(165, eval_params.mobilities[p - 2]);)

                    G(164, // KING ATTACKS
                      score +=
                      G(167, count(G(168, mobility) & G(168, opp_king_zone))) *
                      G(167, eval_params.king_attacks[p - 2]);))
            })
      }
    }

    G(88, flip_pos(pos);)
    G(88, score = -score;)
  }

  const i32 stronger_side_pawns_missing =
      8 - count(G(169, pos->colour[score < 0]) & G(169, pos->pieces[Pawn]));
  return (G(170, (i16)score) * G(170, phase) +
          G(171,
            (128 - stronger_side_pawns_missing * stronger_side_pawns_missing)) *
              G(171, ((score + 0x8000) >> 16)) / 128 * (24 - phase)) /
         24;
}

typedef struct [[nodiscard]] {
  G(130, Move best_move;)
  G(130, Move killer;)
  G(130, u64 position_hash;)
  G(130, i32 static_eval;)
  G(130, i32 num_moves;)
  G(130, Move moves[max_moves];)
} SearchStack;

typedef struct [[nodiscard]] __attribute__((packed)) {
  G(172, u16 partial_hash;)
  G(172, i16 score;)
  G(172, i8 depth;)
  G(172, Move move;)
  G(172, u8 flag;)
} TTEntry;
_Static_assert(sizeof(TTEntry) == 10);

enum { tt_length = 1 << 23 }; // 80MB
enum { Upper = 0, Lower = 1, Exact = 2 };
enum { max_ply = 96 };
enum { mate = 31744, inf = 32256 };

G(173, S(1) i32 move_history[2][6][64][64];)
G(173, S(1) TTEntry tt[tt_length];)
G(173, S(0) u64 start_time;)
G(173, S(0) u64 max_time;)

#if defined(__x86_64__) || defined(_M_X64)
typedef long long __attribute__((__vector_size__(16))) i128;

[[nodiscard]] __attribute__((target("aes"))) S(1) u64
    get_hash(const Position *const pos) {
  i128 hash = {0};

  // USE 16 BYTE POSITION SEGMENTS AS KEYS FOR AES
  const u8 *const data = (const u8 *)pos;
  for (i32 i = 0; i < 6; i++) {
    i128 key;
    __builtin_memcpy(&key, data + G(174, i) * G(174, 16), 16);
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
i32 search(H(175, 1, const i32 beta), H(175, 1, SearchStack *restrict stack),
           H(175, 1, const bool do_null), H(175, 1, i32 depth),
           H(175, 1, i32 alpha),
#ifdef FULL
           u64 *nodes,
#endif
           H(176, 1, Position *const restrict pos),
           H(176, 1, const i32 pos_history_count), H(176, 1, const i32 ply)) {
  assert(alpha < beta);
  assert(ply >= 0);

  // IN-CHECK EXTENSION
  const bool in_check = find_in_check(pos);
  depth += in_check;

  // FULL REPETITION DETECTION
  const u64 tt_hash = get_hash(pos);
  bool in_qsearch = depth <= 0;
  for (i32 i = G(177, ply) + G(177, pos_history_count);
       G(178, i > 0) && G(178, do_null); i -= 2) {
    if (G(179, tt_hash) == G(179, stack[i].position_hash)) {
      return 0;
    }
  }

  // TT PROBING
  G(180, stack[ply].best_move = (Move){0};)
  G(180, TTEntry *tt_entry = &tt[tt_hash % tt_length];)
  G(180, const u16 tt_hash_partial = tt_hash / tt_length;)
  if (G(181, tt_entry->partial_hash) == G(181, tt_hash_partial)) {
    stack[ply].best_move = tt_entry->move;

    // TT PRUNING
    if (G(182, tt_entry->depth >= depth) &&
        G(182, G(183, tt_entry->flag) != G(183, tt_entry->score <= alpha)) &&
        G(182, G(184, alpha) == G(184, beta - 1))) {
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
  if (G(185, G(186, tt_entry->partial_hash) == G(186, tt_hash_partial)) &&
      G(185, G(187, tt_entry->flag) != G(187, static_eval) > tt_entry->score)) {
    static_eval = tt_entry->score;
  }

  // QUIESCENCE
  if (G(188, static_eval > alpha) && G(188, in_qsearch)) {
    if (static_eval >= beta) {
      return static_eval;
    }
    alpha = static_eval;
  }

  if (G(189, !in_check) && G(189, G(190, alpha) == G(190, beta - 1))) {
    if (G(191, !in_qsearch) && G(191, depth < 8)) {

      G(192, {
        // REVERSE FUTILITY PRUNING
        if (static_eval - G(193, 56) * G(193, (depth - improving)) >= beta) {
          return static_eval;
        }
      })

      G(192, // RAZORING
        in_qsearch =
            G(194, static_eval) + G(194, G(195, 122) * G(195, depth)) <= alpha;)
    }

    // NULL MOVE PRUNING
    if (G(196, depth > 2) && G(196, static_eval >= beta) && G(196, do_null)) {
      Position npos = *pos;
      G(197, flip_pos(&npos);)
      G(197, npos.ep = 0;)
      const i32 score = -search(
          H(175, 2, -alpha), H(175, 2, stack), H(175, 2, false),
          H(175, 2, depth - G(198, 4) - G(198, depth / 4)), H(175, 2, -beta),
#ifdef FULL
          nodes,
#endif
          H(176, 2, &npos), H(176, 2, pos_history_count), H(176, 2, ply + 1));
      if (score >= beta) {
        return score;
      }
    }
  }

  G(199, i32 moves_evaluated = 0;)
  G(199, i32 quiets_evaluated = 0;)
  G(199,
    stack[ply].num_moves = movegen(H(108, 3, pos), H(108, 3, stack[ply].moves),
                                   H(108, 3, in_qsearch));)
  G(199, u8 tt_flag = Upper;)
  G(199, i32 best_score = in_qsearch ? static_eval : -inf;)
  G(199,
    stack[G(200, pos_history_count) + G(200, ply) + G(200, 2)].position_hash =
        tt_hash;)

  for (i32 move_index = 0; move_index < stack[ply].num_moves; move_index++) {
    // MOVE ORDERING
    G(201, i32 move_score = ~0x1010101LL;)
    G(201, i32 best_index = 0;)
    for (i32 order_index = move_index; order_index < stack[ply].num_moves;
         order_index++) {
      assert(
          stack[ply].moves[order_index].takes_piece ==
          piece_on(H(62, 7, pos), H(62, 7, stack[ply].moves[order_index].to)));
      const i32 order_move_score =
          G(175, // KILLER MOVE
            G(202, move_equal(G(203, &stack[ply].moves[order_index]),
                              G(203, &stack[ply].killer))) *
                G(202, 836)) +
          G(175, // PREVIOUS BEST MOVE FIRST
            (move_equal(G(204, &stack[ply].best_move),
                        G(204, &stack[ply].moves[order_index]))
             << 30)) +
          G(175, // MOST VALUABLE VICTIM
            G(205, stack[ply].moves[order_index].takes_piece) * G(205, 712)) +
          G(175, // HISTORY HEURISTIC
            move_history[pos->flipped]
                        [stack[ply].moves[order_index].takes_piece]
                        [stack[ply].moves[order_index].from]
                        [stack[ply].moves[order_index].to]);
      if (order_move_score > move_score) {
        G(206, best_index = order_index;)
        G(206, move_score = order_move_score;)
      }
    }

    swapmoves(G(207, &stack[ply].moves[move_index]),
              G(207, &stack[ply].moves[best_index]));

    G(
        208, // FORWARD FUTILITY PRUNING / DELTA PRUNING
        if (G(209, depth < 8) &&
            G(209,
              G(210, static_eval) + G(210, G(211, 142) * G(211, depth)) +
                      G(210, initial_params.eg.material
                                 [stack[ply].moves[move_index].takes_piece]) +
                      G(210,
                        initial_params.eg
                            .material[stack[ply].moves[move_index].promo]) <
                  alpha) &&
            G(209, !in_check) && G(209, moves_evaluated)) { break; })

    G(
        208, // MOVE SCORE PRUNING
        if (G(212, move_score < G(213, -128) * G(213, depth)) &&
            G(212, moves_evaluated)) { break; })

    Position npos = *pos;
#ifdef FULL
    (*nodes)++;
#endif
    if (!makemove(H(93, 3, &npos), H(93, 3, &stack[ply].moves[move_index]))) {
      continue;
    }

    // PRINCIPAL VARIATION SEARCH
    i32 low = moves_evaluated == 0 ? -beta : -alpha - 1;
    moves_evaluated++;

    // LATE MOVE REDUCTION
    i32 reduction = G(214, depth > 3) && G(214, move_score <= 0)
                        ? G(215, (-move_score / 256)) +
                              G(215, (G(216, alpha) == G(216, beta - 1))) +
                              G(215, !improving) + G(215, moves_evaluated / 10)
                        : 0;

    i32 score;
    while (true) {
      score = -search(
          H(175, 3, -alpha), H(175, 3, stack), H(175, 3, true),
          H(175, 3, depth - G(217, 1) - G(217, reduction)), H(175, 3, low),
#ifdef FULL
          nodes,
#endif
          H(176, 3, &npos), H(176, 3, pos_history_count), H(176, 3, ply + 1));

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
        G(218, tt_flag = Exact;)
        G(218, stack[ply].best_move = stack[ply].moves[move_index];)
        G(218, alpha = score;)
        if (score >= beta) {
          assert(stack[ply].best_move.takes_piece ==
                 piece_on(H(62, 8, pos), H(62, 8, stack[ply].best_move.to)));
          G(219, tt_flag = Lower;)
          G(
              219, if (stack[ply].best_move.takes_piece == None) {
                stack[ply].killer = stack[ply].best_move;
              })
          G(
              219, if (!in_qsearch) {
                const i32 bonus = depth * depth;
                G(220, i32 *const this_hist =
                           &move_history[pos->flipped]
                                        [stack[ply].best_move.takes_piece]
                                        [stack[ply].best_move.from]
                                        [stack[ply].best_move.to];

                  *this_hist +=
                  bonus - G(221, bonus) * G(221, *this_hist) / 1024;)
                G(
                    220, for (i32 prev_index = 0; prev_index < move_index;
                              prev_index++) {
                      const Move prev = stack[ply].moves[prev_index];
                      i32 *const prev_hist =
                          &move_history[pos->flipped][prev.takes_piece]
                                       [prev.from][prev.to];
                      *prev_hist -=
                          bonus + G(222, bonus) * G(222, *prev_hist) / 1024;
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
    if (G(223, G(224, alpha) == G(224, beta - 1)) &&
        G(223, quiets_evaluated > (G(225, 1) + G(225, depth * depth)) >>
                   !improving) &&
        G(223, !in_check)) {
      break;
    }
  }

  // MATE / STALEMATE DETECTION
  if (G(226, best_score) == G(226, -inf)) {
    return G(227, (ply - mate)) * G(227, in_check);
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
      93, // MERGE EVAL PARAMS
      for (i32 i = 0; i < sizeof(EvalParamsMerged) / sizeof(i32); i++) {
        ((i32 *)&eval_params)[i] =
            combine_eval_param(H(134, 2,
                                 i < 6 ? initial_params.mg.material[i]
                                       : ((i8 *)&initial_params.mg)[6 + i]),
                               H(134, 2,
                                 i < 6 ? initial_params.eg.material[i]
                                       : ((i8 *)&initial_params.eg)[6 + i]));
      })
  G(93, // CLEAR HISTORY
    __builtin_memset(move_history, 0, sizeof(move_history));)
  G(
      93, // INIT DIAGONAL MASKS
      for (i32 sq = 0; sq < 64; sq++) {
        const u64 bb = 1ULL << sq;
        diag_mask[sq] =
            G(228, ray(H(29, 4, 0), H(29, 4, ~0x8080808080808080ull),
                       H(29, 4, bb), H(29, 4, -9))) |
            G(228, ray(H(29, 5, 0), H(29, 5, ~0x101010101010101ull),
                       H(29, 5, bb), H(29, 5, 9)));
      })
}

#ifdef FULL
static void print_info(const Position *pos, const i32 depth, const i32 alpha,
                       const i32 beta, const i32 score, const u64 nodes,
                       const Move pv_move) {
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
    move_str(H(64, 2, move_name), H(64, 2, &pv_move), H(64, 2, pos->flipped));
    putl(move_name);
  }

  putl("\n");
}
#endif

S(1)
void iteratively_deepen(
#ifdef FULL
    i32 maxdepth, u64 *nodes,
#endif
    H(229, 1, Position *const restrict pos),
    H(229, 1, SearchStack *restrict stack),
    H(229, 1, const i32 pos_history_count)) {
  G(230, start_time = get_time();)
  G(230, i32 score = 0;)
#ifdef FULL
  for (i32 depth = 1; depth < maxdepth; depth++) {
#else
  for (i32 depth = 1; depth < max_ply; depth++) {
#endif
    // ASPIRATION WINDOWS
    G(231, i32 window = 15;)
    G(231, size_t elapsed;)
    while (true) {
      G(232, const i32 beta = G(233, score) + G(233, window);)
      G(232, const i32 alpha = score - window;)
      score =
          search(H(175, 4, beta), H(175, 4, stack), H(175, 4, false),
                 H(175, 4, depth), H(175, 4, alpha),
#ifdef FULL
                 nodes,
#endif
                 H(176, 4, pos), H(176, 4, pos_history_count), H(176, 4, 0));
#ifdef FULL
      print_info(pos, depth, alpha, beta, score, *nodes, stack[0].best_move);
#endif
      elapsed = get_time() - start_time;
      G(234, window *= 2;)
      G(
          234, if (G(235, elapsed > max_time) ||
                   G(235, (G(236, score > alpha) && G(236, score < beta)))) {
            break;
          })
    }

    if (elapsed > max_time / 16) {
      break;
    }
  }
  char move_name[8];
  move_str(H(64, 3, move_name), H(64, 3, &stack[0].best_move),
           H(64, 3, pos->flipped));
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
      i32 piece = piece_on(H(62, 9, &npos), H(62, 9, sq));
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
#ifdef LOWSTACK
  SearchStack *stack = malloc(sizeof(SearchStack) * 1024);
#else
  SearchStack stack[1024];
#endif
  __builtin_memset(move_history, 0, sizeof(move_history));
  pos = start_pos;
  max_time = -1ll;
  u64 nodes = 0;
  const u64 start = get_time();
  iteratively_deepen(23, &nodes, H(229, 2, &pos), H(229, 2, stack),
                     H(229, 2, pos_history_count));
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

  G(237, char line[4096];)
  G(237, Position pos;)
  G(237, i32 pos_history_count;)
  G(237, // #ifdef LOWSTACK
         //  SearchStack *stack = malloc(sizeof(SearchStack) * 1024);
         // #else
    SearchStack stack[1024];
    // #endif
  )
  G(237, init();)

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
      max_time = -1ll;
      iteratively_deepen(max_ply, &nodes, H(229, 3, &pos), H(229, 3, stack),
                         H(229, 3, pos_history_count));
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
      const u64 nps = elapsed ? nodes * 1000 * 1000 * 1000 / elapsed : 0;
      printf("info depth %i nodes %llu time %llu nps %llu \n", depth, nodes,
             elapsed, nps);
    }
#endif
    G(
        238, if (G(239, line[0]) == G(239, 'g')) {
#ifdef FULL
          while (true) {
            getl(line);
            if (!pos.flipped && !strcmp(line, "wtime")) {
              getl(line);
              max_time = (u64)atoi(line) << 19; // Roughly /2 time
              break;
            } else if (pos.flipped && !strcmp(line, "btime")) {
              getl(line);
              max_time = (u64)atoi(line) << 19; // Roughly /2 time
              break;
            } else if (!strcmp(line, "movetime")) {
              max_time = 20ULL * 1000 * 1000 * 1000; // Assume Lichess bot
              break;
            }
          }
          iteratively_deepen(max_ply, &nodes, H(229, 4, &pos), H(229, 4, stack),
                             H(229, 4, pos_history_count));
#else
      for (i32 i = 0; i < (pos.flipped ? 4 : 2); i++) {
        getl(line);
        max_time = (u64)atoi(line) << 19;// Roughly /2 time
      }
      iteratively_deepen(H(229, 5, &pos), H(229, 5, stack),
        H(229, 5, pos_history_count));
#endif
        })
    else G(238, if (G(240, line[0]) == G(240, 'i')) { puts("readyok"); })
    else G(238, if (G(241, line[0]) == G(241, 'q')) { exit_now(); })
    else G(238, if (G(242, line[0]) == G(242, 'p')) {
      G(243, pos_history_count = 0;)
        G(243, pos = start_pos;)
        while (true) {
          const bool line_continue = getl(line);

#if FULL
          if (!strcmp(line, "fen")) {
            getl(line);
            get_fen(&pos, line);
          }
#endif

          const i32 num_moves =
            movegen(H(108, 4, &pos), H(108, 4, stack[0].moves), H(108, 4, false));
          for (i32 i = 0; i < num_moves; i++) {
            char move_name[8];
            move_str(H(64, 4, move_name), H(64, 4, &stack[0].moves[i]),
              H(64, 4, pos.flipped));
            assert(move_string_equal(line, move_name) ==
              !strcmp(line, move_name));
            if (move_string_equal(G(244, move_name), G(244, line))) {
              stack[pos_history_count].position_hash = get_hash(&pos);
              pos_history_count++;
              if (stack[0].moves[i].takes_piece != None) {
                pos_history_count = 0;
              }
              makemove(H(93, 4, &pos), H(93, 4, &stack[0].moves[i]));
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
