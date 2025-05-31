// Minify thing thang
#define G(a, b) b
#define H(a, b, c) c

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

static ssize_t _sys(ssize_t call, ssize_t arg1, ssize_t arg2, ssize_t arg3) {
  ssize_t ret;
#ifdef ARCH64
  asm volatile("syscall"
    : "=a"(ret)
    : "a"(call), "D"(arg1), "S"(arg2), "d"(arg3)
    : "rcx", "r11", "memory");
#else
  asm volatile("int $0x80"
    : "=a"(ret)
    : "a"(call), "b"(arg1), "c"(arg2), "d"(arg3)
    : "memory");
#endif
  return ret;
}

static void exit_now() {
#ifdef ARCH32
  asm volatile("movl $1, %eax\n\t"
    "int $0x80");
#else
  asm volatile("movl $60, %eax\n\t"
    "syscall");
#endif
}

[[nodiscard]] static i32 strlen(const char* const restrict string) {
  i32 length = 0;
  while (string[length]) {
    length++;
  }
  return length;
}

static void putl(const char* const restrict string) {
  i32 length = 0;
  while (string[length]) {
#ifdef ARCH64
    _sys(1, stdout, (ssize_t)(&string[length]), 1);
#else
    _sys(4, stdout, (ssize_t)(&string[length]), 1);
#endif
    length++;
  }
}

static void puts(const char* const restrict string) {
  putl(string);
  putl("\n");
}

// Non-standard, gets but a word instead of a line
static bool getl(char* restrict string) {
  while (true) {
#ifdef ARCH64
    const int result = _sys(0, stdin, (ssize_t)string, 1);
#else
    const int result = _sys(3, stdin, (ssize_t)string, 1);
#endif

    // Assume stdin never closes on mini build
#ifdef FULL
    if (result < 1) {
      exit_now();
    }
#endif

    const char ch = *string;
    if (G(1, ch == '\n') || G(1, ch == ' ')) {
      *string = 0;
      return ch != '\n';
    }

    string++;
  }
}

[[nodiscard]] static bool strcmp(const char* restrict lhs,
  const char* restrict rhs) {
  while (*lhs || *rhs) {
    if (*lhs != *rhs) {
      return true;
    }
    G(2, lhs++;)
      G(2, rhs++;)
  }
  return false;
}

[[nodiscard]] static u32 atoi(const char* restrict string) {
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
}

#define printf(format, ...) _printf(format, (size_t[]){__VA_ARGS__})

static void _printf(const char* format, const size_t* args) {
  long long value;
  char buffer[16], * string;

  while (true) {
    if (!*format) {
      break;
    }
    if (*format != '%') {
#ifdef ARCH64
      _sys(1, stdout, (ssize_t)format, 1);
#else
      _sys(4, stdout, (ssize_t)format, 1);
#endif
      format++;
      continue;
    }

    format++;
    switch (*format++) {
    case 's':
      putl((char*)*args);
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
#ifdef ARCH64
  _sys(228, 1, (ssize_t)&ts, 0);
#else
  _sys(265, 1, (ssize_t)&ts, 0);
#endif
  return ts.tv_sec * 1000 + ts.tv_nsec / 1000000;
}

#ifdef ASSERTS
#define assert(condition)                                                      \
  if (!(condition)) {                                                          \
    printf("Assert failed on line %i: ", __LINE__);                            \
    puts(#condition);                                                          \
    _sys(60, 1, 0, 0);                                                         \
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

static bool getl(char* restrict string) {
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

static void putl(const char* const restrict string) {
  fputs(string, stdout);
  fflush(stdout);
}

#endif

#pragma endregion

#pragma region base

enum [[nodiscard]] { None, Pawn, Knight, Bishop, Rook, Queen, King };

typedef struct [[nodiscard]] {
  G(3, u8 promo;)
    G(3, u8 takes_piece;)
    G(3, u8 from; u8 to;)
  } Move;

typedef struct [[nodiscard]] {
  G(4, u64 ep;)
    G(4, u64 pieces[7];)
    G(4, u64 colour[2];)
    G(5, bool castling[4];)
    G(5, bool flipped;)
  } Position;

[[nodiscard]] static bool move_string_equal(const char* restrict lhs,
  const char* restrict rhs) {
  return (*(const u64*)lhs ^ *(const u64*)rhs) << 24 == 0;
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

[[nodiscard]] static i32 count(const u64 bb) {
  return __builtin_popcountll(bb);
}

[[nodiscard]] static u64 shift(const u64 bb, const i32 shift, const u64 mask) {
  return shift > 0 ? bb << shift & mask : bb >> -shift & mask;
}

[[nodiscard]] static u64 west(const u64 bb) {
  return bb >> 1 & ~0x8080808080808080ull;
}

[[nodiscard]] static u64 east(const u64 bb) {
  return bb << 1 & ~0x101010101010101ull;
}

[[nodiscard]] static u64 north(const u64 bb) { return bb << 8; }

[[nodiscard]] static u64 south(const u64 bb) { return bb >> 8; }

[[nodiscard]] static u64 nw(const u64 bb) {
  return shift(bb, 7, ~0x8080808080808080ull);
  // return west(north(bb));
}

[[nodiscard]] static u64 ne(const u64 bb) { return east(north(bb)); }

[[nodiscard]] static u64 sw(const u64 bb) { return west(south(bb)); }

[[nodiscard]] static u64 se(const u64 bb) { return east(south(bb)); }

[[nodiscard]] u64 xattack(const u64 bb, const u64 blockers,
  const u64 dir_mask) {
  return dir_mask & ((blockers & dir_mask) - bb ^
    flip_bb(flip_bb(blockers & dir_mask) - flip_bb(bb)));
}

[[nodiscard]] static u64 ray(const u64 bb, const u64 blockers,
  const i32 shift_by, const u64 mask) {
  u64 result = shift(bb, shift_by, mask);
  for (i32 i = 0; i < 6; i++) {
    result |= shift(result & ~blockers, shift_by, mask);
  }
  return result;
}

static u64 diag_mask[64];

[[nodiscard]] static u64 bishop(const u64 bb, const u64 blockers) {
  assert(count(bb) == 1);
  const i32 sq = lsb(bb);
  return xattack(bb, blockers, diag_mask[sq]) |
    xattack(bb, blockers, flip_bb(diag_mask[sq ^ 56]));
}

[[nodiscard]] static u64 rook(const u64 bb, const u64 blockers) {
  assert(count(bb) == 1);
  return xattack(bb, blockers, bb ^ 0x101010101010101ULL << lsb(bb) % 8) |
    ray(bb, blockers, 1, ~0x101010101010101ull)      // East
    | ray(bb, blockers, -1, ~0x8080808080808080ull); // West
}

[[nodiscard]] static u64 knight(const u64 bb) {
  return (bb << 15 | bb >> 17) & ~0x8080808080808080ull |
    (bb << 17 | bb >> 15) & ~0x101010101010101ull |
    (bb << 10 | bb >> 6) & 0xFCFCFCFCFCFCFCFCull |
    (bb << 6 | bb >> 10) & 0x3F3F3F3F3F3F3F3Full;
}

[[nodiscard]] static u64 king(const u64 bb) {
  return bb << 8 | bb >> 8 |
    (bb >> 1 | bb >> 9 | bb << 7) & ~0x8080808080808080ull |
    (bb << 1 | bb << 9 | bb >> 7) & ~0x101010101010101ull;
}

static void move_str(H(6, 1, char* restrict str), H(6, 1, const Move* restrict move),
  H(6, 1, const i32 flip)) {
  assert(move->from >= 0);
  assert(move->from < 64);
  assert(move->to >= 0);
  assert(move->to < 64);
  assert(move->from != move->to);
  assert(move->promo == None || move->promo == Knight ||
    move->promo == Bishop || move->promo == Rook || move->promo == Queen);

  // Hack to save bytes, technically UB but works on GCC 14.2
  for (i32 i = 0; i < 2; i++) {
    G(7, str[i * 2] = 'a' + (&move->from)[i] % 8;)
      G(7, str[i * 2 + 1] = '1' + ((&move->from)[i] / 8 ^ 7 * flip);)
  }

  G(8, str[4] = "\0\0nbrq"[move->promo];)
    G(8, str[5] = '\0';)
}

[[nodiscard]] static i32 piece_on(const Position* const restrict pos,
  const i32 sq) {
  assert(sq >= 0);
  assert(sq < 64);
  for (i32 i = Pawn; i <= King; ++i) {
    if (pos->pieces[i] & 1ull << sq) {
      return i;
    }
  }
  return None;
}

static void swapu64(u64* const lhs, u64* const rhs) {
  const u64 temp = *lhs;
  *lhs = *rhs;
  *rhs = temp;
}

static void swapu32(u32* const lhs, u32* const rhs) {
  const u32 temp = *lhs;
  *lhs = *rhs;
  *rhs = temp;
}

static void swapmoves(Move* const lhs, Move* const rhs) {
  swapu32((u32*)lhs, (u32*)rhs);
}

static void swapbool(bool* const restrict lhs, bool* const restrict rhs) {
  const bool temp = *lhs;
  *lhs = *rhs;
  *rhs = temp;
}

[[nodiscard]] static bool move_equal(Move* const lhs, Move* const rhs) {
  return *(u32*)lhs == *(u32*)rhs;
}

static void flip_pos(Position* const restrict pos) {
  // Hack to flip the first 10 bitboards in Position.
  // Technically UB but works in GCC 14.2
  u64* pos_ptr = (u64*)pos;
  for (i32 i = 0; i < 10; i++) {
    pos_ptr[i] = flip_bb(pos_ptr[i]);
  }

  pos->flipped ^= 1;
  for (i32 i = 0; i < 2; i++) {
    swapbool(&pos->castling[i], &pos->castling[i + 2]);
  }
  swapu64(&pos->colour[0], &pos->colour[1]);
}

[[nodiscard]] static u64 get_mobility(const i32 sq, const i32 piece,
  const Position* pos) {
  u64 moves = 0;
  const u64 bb = 1ULL << sq;
  if (piece == Knight) {
    moves = knight(bb);
  }
  else if (piece == King) {
    moves = king(bb);
  }
  else {
    const u64 blockers = pos->colour[0] | pos->colour[1];
    if (piece == Rook || piece == Queen) {
      moves |= rook(bb, blockers);
    }
    if (piece == Bishop || piece == Queen) {
      moves |= bishop(bb, blockers);
    }
  }
  return moves;
}

[[nodiscard]] static i32 is_attacked(H(9, 1,const Position* const restrict pos),
  H(9, 1, const u64 bb), H(9, 1, const i32 them)) {
  assert(count(bb) == 1);
  const u64 theirs = pos->colour[them];
  const u64 pawns = theirs & pos->pieces[Pawn];
  if ((them ? sw(pawns) | se(pawns) : nw(pawns) | ne(pawns)) & bb) {
    return true;
  }
  const u64 blockers = pos->colour[0] | pos->colour[1];
  return G(10, rook(bb, blockers) & theirs &
    (pos->pieces[Rook] | pos->pieces[Queen])) ||
    G(10, knight(bb) & theirs & pos->pieces[Knight]) ||
    G(10, king(bb) & theirs & pos->pieces[King]) ||
    G(10, bishop(bb, blockers) & theirs &
      (pos->pieces[Bishop] | pos->pieces[Queen]));
}

i32 makemove(H(11, 1, Position* const restrict pos), H(11, 1, const Move* const restrict move)) {
  assert(move->from >= 0);
  assert(move->from < 64);
  assert(move->to >= 0);
  assert(move->to < 64);
  assert(move->from != move->to);
  assert(move->promo == None || move->promo == Knight ||
    move->promo == Bishop || move->promo == Rook || move->promo == Queen);

  const u64 from = 1ull << move->from;
  const u64 to = 1ull << move->to;
  const u64 mask = from | to;

  assert(move->takes_piece != King);
  assert(move->takes_piece == piece_on(pos, move->to));
  const i32 piece = piece_on(pos, move->from);
  assert(piece != None);

  G(12, // Castling
    if (piece == King) {
      const u64 bb = move->to - move->from == 2 ? 0xa0
        : move->from - move->to == 2 ? 0x9
        : 0;
      pos->colour[0] ^= bb;
      pos->pieces[Rook] ^= bb;
    })

    G(12, pos->colour[0] ^= mask;)

    // Move the piece
    G(12, pos->pieces[piece] ^= mask;)
    G(12, // Captures
      if (move->takes_piece != None) {
        pos->colour[1] ^= to;
        pos->pieces[move->takes_piece] ^= to;
      })

    // En passant
    if (piece == Pawn && to == pos->ep) {
      pos->colour[1] ^= to >> 8;
      pos->pieces[Pawn] ^= to >> 8;
    }
  pos->ep = 0;

  G(13, // Pawn double move
    if (piece == Pawn && move->to - move->from == 16) { pos->ep = to >> 8; })

    G(14, 
    // Promotions
    if (move->promo != None) {
    pos->pieces[Pawn] ^= to;
    pos->pieces[move->promo] ^= to;
  })

    G(13, // Update castling permissions
      pos->castling[2] &= !(mask & 0x9000000000000000ull);
  pos->castling[3] &= !(mask & 0x1100000000000000ull);
  pos->castling[0] &= !(mask & 0x90ull);
  pos->castling[1] &= !(mask & 0x11ull);)

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
  return !is_attacked(H(9, 2, pos), H(9, 2, pos->colour[1] & pos->pieces[King]), H(9, 2, false));
}

static Move* generate_pawn_moves(const Position* const pos,
  Move* restrict movelist, u64 to_mask,
  const i32 offset

) {
  while (to_mask) {
    const u8 to = lsb(to_mask);
    to_mask &= to_mask - 1;
    const u8 from = to + offset;
    assert(from >= 0);
    assert(from < 64);
    assert(to >= 0);
    assert(to < 64);
    assert(piece_on(pos, from) == Pawn);
    const u8 takes = piece_on(pos, to);
    if (to > 55) {
      for (u8 piece = Queen; piece >= Knight; piece--) {
        *movelist++ = (Move){
            .from = from, .to = to, .promo = piece, .takes_piece = takes };
      }
    }
    else {
      *movelist++ =
        (Move){ .from = from, .to = to, .promo = None, .takes_piece = takes };
    }
  }

  return movelist;
}

static Move* generate_piece_moves(H(15, 1, Move* restrict movelist),
  H(15, 1, const Position* restrict pos),
  H(15, 1, const u64 to_mask)) {
  for (i32 piece = Knight; piece <= King; piece++) {
    assert(piece == Knight || piece == Bishop || piece == Rook ||
      piece == Queen || piece == King);
    u64 copy = pos->colour[0] & pos->pieces[piece];
    while (copy) {
      const u8 from = lsb(copy);
      assert(from >= 0);
      assert(from < 64);
      copy &= copy - 1;

      u64 moves = get_mobility(from, piece, pos);
      moves &= to_mask;

      while (moves) {
        const u8 to = lsb(moves);
        assert(to >= 0);
        assert(to < 64);
        moves &= moves - 1;
        *movelist++ = (Move){ .from = from,
                             .to = to,
                             .promo = None,
                             .takes_piece = piece_on(pos, to) };
      }
    }
  }

  return movelist;
}

enum { max_moves = 218 };

[[nodiscard]] static i32 movegen(const Position* const restrict pos,
  Move* restrict movelist,
  const i32 only_captures) {

  const Move* start = movelist;
  const u64 all = pos->colour[0] | pos->colour[1];
  const u64 to_mask = only_captures ? pos->colour[1] : ~pos->colour[0];
  if (!only_captures) {
    movelist = generate_pawn_moves(
      pos, movelist,
      north(north(pos->colour[0] & pos->pieces[Pawn] & 0xFF00) & ~all) & ~all,
      -16);
  }
  movelist =
    generate_pawn_moves(pos, movelist,
      north(pos->colour[0] & pos->pieces[Pawn]) & ~all &
      (only_captures ? 0xFF00000000000000ull : ~0ull),
      -8);
  movelist = generate_pawn_moves(
    pos, movelist,
    nw(pos->colour[0] & pos->pieces[Pawn]) & (pos->colour[1] | pos->ep), -7);
  movelist = generate_pawn_moves(
    pos, movelist,
    ne(pos->colour[0] & pos->pieces[Pawn]) & (pos->colour[1] | pos->ep), -9);
  if (!only_captures && pos->castling[0] && !(all & 0x60ull) &&
    !is_attacked(H(9, 3, pos), H(9, 3, 1ULL << 4), H(9, 3, true)) &&
    !is_attacked(H(9, 4, pos), H(9, 4, 1ULL << 5), H(9, 4, true))) {
    *movelist++ =
      (Move){ .from = 4, .to = 6, .promo = None, .takes_piece = None };
  }
  if (!only_captures && pos->castling[1] && !(all & 0xEull) &&
    !is_attacked(H(9, 5, pos), H(9, 5, 1ULL << 4), H(9, 5, true)) &&
    !is_attacked(H(9, 6, pos), H(9, 6, 1ULL << 3), H(9, 6, true))) {
    *movelist++ =
      (Move){ .from = 4, .to = 2, .promo = None, .takes_piece = None };
  }
  movelist = generate_piece_moves(H(15, 2, movelist), H(15, 2, pos), H(15, 2, to_mask));

  const i32 num_moves = movelist - start;
  assert(num_moves < max_moves);
  return num_moves;
}

#pragma endregion

#pragma region engine

[[nodiscard]] static u64 perft(const Position* const restrict pos,
  const i32 depth) {
  if (depth == 0) {
    return 1;
  }

  u64 nodes = 0;
  Move moves[max_moves];
  const i32 num_moves = movegen(pos, moves, false);

  for (i32 i = 0; i < num_moves; ++i) {
    Position npos = *pos;

    // Check move legality
    if (!makemove(H(11, 2, &npos), H(11, 2, &moves[i]))) {
      continue;
    }

    nodes += perft(&npos, depth - 1);
  }

  return nodes;
}

static void get_fen(Position* restrict pos, char* restrict fen) {
  __builtin_memset(pos, 0, sizeof(Position));
  const char* p = fen;

  // PIECES
  i32 sq = 56;
  while (*p && *p != ' ') {
    const char c = *p;
    if (c == '/') {
      sq -= 16;
    }
    else if (c >= '1' && c <= '8') {
      sq += c - '0';
    }
    else {
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
  H(16, 1, i16 material[6];)
  H(16, 1, i8 pst_rank[64];)
  H(16, 1, i8 pst_file[64];)
    H(16, 1, i8 mobilities[4];)
    H(17, 1, i8 king_attacks[4];)
    H(17, 1, i8 open_files[6];)
    H(17, 1, i8 bishop_pair;)
    H(17, 1, i8 tempo;)
} EvalParams;

typedef struct [[nodiscard]] __attribute__((packed)) {
  H(16, 2, i32 material[6];)
  H(16, 2, i32 pst_rank[64];)
  H(16, 2, i32 pst_file[64];)
    H(16, 2, i32 mobilities[4];)
    H(17, 2, i32 king_attacks[4];)
    H(17, 2, i32 open_files[6];)
    H(17, 2, i32 bishop_pair;)
    H(17, 2, i32 tempo;)
} EvalParamsMerged;

G(18, static const EvalParams mg = ((EvalParams) {
  .material = { 72, 318, 299, 401, 896, 0 },
    .pst_rank =
  {
      0,   -16, -11, -10, 4,  33, 127, 0,   // Pawn
      -35, -21, 3,   15,  31, 56, 27,  -76, // Knight
      -11, 2,   11,  11,  16, 22, 0,   -51, // Bishop
      -2,  -13, -19, -21, -2, 19, 18,  19,  // Rook
      13,  15,  8,   -4,  -8, -1, -19, -3,  // Queen
      -16, -8,  -28, -25, 11, 71, 86,  100  // King
  },
    .pst_file =
  {
      -23, -9, -8, 3,   9,   22,  21, -14, // Pawn
      -31, -9, 6,  17,  14,  14,  3,  -14, // Knight
      -11, 3,  6,  1,   5,   -2,  4,  -6,  // Bishop
      -6,  -7, 2,  10,  12,  2,   -4, -9,  // Rook
      -9,  -6, -1, 1,   2,   0,   7,  7,   // Queen
      -22, 25, -2, -46, -17, -35, 19, -4   // King
  },
    .mobilities = { 4, 2, 2, -11 },
    .king_attacks = { 14, 19, 15, 0 },
    .open_files = { 23, -14, -10, 19, -3, -31 },
    .bishop_pair = 24,
    .tempo = 16
});)

G(18, static const EvalParams eg = ((EvalParams) {
  .material = { 88, 304, 283, 541, 978, 0 },
    .pst_rank =
  {
      0,   -10, -15, -15, -4, 45, 115, 0,  // Pawn
      -35, -15, 0,   20,  24, 9,  -1,  -2, // Knight
      -7,  -6,  -1,  0,   4,  1,  1,   8,  // Bishop
      -15, -17, -13, 2,   9,  10, 16,  9,  // Rook
      -56, -42, -20, 9,   28, 29, 38,  15, // Queen
      -31, -2,  11,  21,  26, 21, 7,   -37 // King
  },
    .pst_file =
  {
      12,  13, 0,  -6, -6, -6, -1, -7,  // Pawn
      -22, -4, 11, 17, 16, 7,  -3, -22, // Knight
      -3,  0,  -1, 2,  3,  2,  2,  -4,  // Bishop
      1,   4,  5,  0,  -5, 0,  -1, -4,  // Rook
      -18, -4, 3,  9,  12, 10, -4, -9,  // Queen
      -15, -2, 9,  20, 15, 18, 0,  -23  // King
  },
    .mobilities = { 5, 2, 1, 0 },
    .king_attacks = { -3, -6, 5, 0 },
    .open_files = { 32, -1, 11, 15, 28, 10 },
    .bishop_pair = 53,
    .tempo = 8
});)

static EvalParamsMerged eval_params;

static i32 combine_eval_param(const i32 mg_val, const i32 eg_val) {
  return (eg_val << 16) + mg_val;
}

static void init() {
  // INIT DIAGONAL MASKS
  G(19, for (i32 sq = 0; sq < 64; sq++) {
    const u64 bb = 1ULL << sq;
    diag_mask[sq] = ray(bb, 0, 9, ~0x101010101010101ull) |  // Northeast
      ray(bb, 0, -9, ~0x8080808080808080ull); // Southwest
  })

    G(19, // MERGE MATERIAL VALUES
      for (i32 i = 0; i < sizeof(mg.material) / sizeof(i16); i++) {
        eval_params.material[i] =
          combine_eval_param(mg.material[i], eg.material[i]);
      })
    G(19, // MERGE NON-MATERIAL VALUES
      for (i32 i = 0; i < sizeof(mg.pst_rank) + sizeof(mg.pst_file) +
        sizeof(mg.mobilities) + sizeof(mg.king_attacks) +
        sizeof(mg.open_files) + sizeof(mg.bishop_pair) +
        sizeof(mg.tempo);
        i++) {
        // Technically writes past end of array but since the structs are packed
        // it works
        eval_params.pst_rank[i] =
          combine_eval_param(mg.pst_rank[i], eg.pst_rank[i]);
      })
}

__attribute__((aligned(8))) static const i16 max_material[] = { 0,   88,  318,
                                                               299, 541, 978 };
__attribute__((aligned(8))) static const i8 phases[] = { 0, 0, 1, 1, 2, 4, 0 };

static i32 eval(Position* const restrict pos) {
  G(20, i32 score = eval_params.tempo;)
    G(20, i32 phase = 0;)

    for (i32 c = 0; c < 2; c++) {

      G(21, const u64 opp_king_zone = king(pos->colour[1] & pos->pieces[King]);)

        G(21, const u64 own_pawns = pos->colour[0] & pos->pieces[Pawn];)
        G(21, // BISHOP PAIR
          if (count(pos->colour[0] & pos->pieces[Bishop]) > 1) {
            score += eval_params.bishop_pair;
          })

        for (i32 p = Pawn; p <= King; p++) {
          u64 copy = pos->colour[0] & pos->pieces[p];
          while (copy) {
            phase += phases[p];

            const i32 sq = lsb(copy);
            copy &= copy - 1;

            G(22, const int rank = sq >> 3;)
              G(22, const int file = sq & 7;)

              // OPEN FILES / DOUBLED PAWNS
              G(23, // MATERIAL
                score += eval_params.material[p - 1];)

              // SPLIT PIECE-SQUARE TABLES
              G(23, score += eval_params.pst_file[(p - 1) * 8 + file];)
              G(23, score += eval_params.pst_rank[(p - 1) * 8 + rank];)

              G(23, if ((north(0x101010101010101ULL << sq) & own_pawns) == 0) {
              score += eval_params.open_files[p - 1];
            })

              G(23, if (p > Knight) {
              // MOBILITY
              const u64 mobility = get_mobility(sq, p, pos);
              score += eval_params.mobilities[p - 3] *
                count(mobility & ~pos->colour[0]);

              // KING ATTACKS
              score += eval_params.king_attacks[p - 3] *
                count(mobility & opp_king_zone);
            })
          }
        }

      G(24, score = -score;)
        G(24, flip_pos(pos);)
    }

  return ((short)score * phase + ((score + 0x8000) >> 16) * (24 - phase)) / 24;
}

typedef struct [[nodiscard]] {
  G(25, Move killer;)
    G(25, i32 static_eval;)
    G(25, Move best_move;)
    G(25, i32 num_moves;)
    G(25, Move moves[max_moves];)
    G(25, u64 position_hash;)
  } SearchStack;

typedef struct [[nodiscard]] __attribute__((packed)) {
  G(26, Move move;)
    G(26, u16 partial_hash;)
    G(26, u8 flag;)
    G(26, i8 depth;)
    G(26, i16 score;)
} TTEntry;
_Static_assert(sizeof(TTEntry) == 10);

enum { tt_length = 64 * 1024 * 1024 / sizeof(TTEntry) };
enum { Upper = 0, Lower = 1, Exact = 2 };
enum { max_ply = 96 };
enum { mate = 30000, inf = 32000 };

G(27, static i32 move_history[2][6][64][64];)
G(27, static TTEntry tt[tt_length];)
G(27, static size_t start_time;)
G(27, static size_t max_time;)

#if defined(__x86_64__) || defined(_M_X64)
typedef long long __attribute__((__vector_size__(16))) i128;

[[nodiscard]] __attribute__((target("aes"))) u64
get_hash(const Position* const pos) {
  i128 hash = { 0 };

  // USE 16 BYTE POSITION SEGMENTS AS KEYS FOR AES
  const u8* const data = (const u8*)pos;
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
  get_hash(const Position* const pos) {
  uint8x16_t hash = vdupq_n_u8(0);

  // USE 16 BYTE POSITION SEGMENTS AS KEYS FOR AES
  const u8* const data = (const u8*)pos;
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

  static i16 search(H(28, 1, Position* const restrict pos), H(28, 1, const i32 ply), H(28, 1, i32 depth),
    H(28, 1, i32 alpha), H(29, 1, const i32 beta),
#ifdef FULL
    u64* nodes,
#endif
    H(29, 1, SearchStack* restrict stack), H(29, 1, const i32 pos_history_count),
    H(29, 1, const bool do_null)) {
  assert(alpha < beta);
  assert(ply >= 0);

  const bool in_check =
    is_attacked(H(9, 7, pos), H(9, 7, pos->colour[0] & pos->pieces[King]), H(9, 7, true));

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
  for (i32 i = pos_history_count + ply; i > 0 && do_null; i -= 2) {
    if (tt_hash == stack[i].position_hash) {
      return 0;
    }
  }

  // TT PROBING
  G(30, TTEntry * tt_entry = &tt[tt_hash % tt_length];)
    G(30, const u16 tt_hash_partial = tt_hash / tt_length;)
    G(30, stack[ply].best_move = (Move){ 0 };)
    if (tt_entry->partial_hash == tt_hash_partial) {
      stack[ply].best_move = tt_entry->move;

      // TT PRUNING
      if (G(31, alpha == beta - 1) &&
        G(31, tt_entry->flag != tt_entry->score <= alpha) &&
        G(31, tt_entry->depth >= depth)) {
        return tt_entry->score;
      }
    }
    else if (depth > 3) {

      // INTERNAL ITERATIVE REDUCTION
      depth--;
    }

  // STATIC EVAL WITH ADJUSTMENT FROM TT
  i32 static_eval = eval(pos);
  stack[ply].static_eval = static_eval;
  const bool improving = ply > 1 && static_eval > stack[ply - 2].static_eval;
  if (G(32, tt_entry->partial_hash == tt_hash_partial) &&
    G(32, tt_entry->flag != static_eval > tt_entry->score)) {
    static_eval = tt_entry->score;
  }

  // QUIESCENCE
  if (G(33, in_qsearch) && G(33, static_eval > alpha)) {
    if (static_eval >= beta) {
      return static_eval;
    }
    alpha = static_eval;
  }

  if (G(34, !in_check) && G(34, alpha == beta - 1)) {
    if (G(35, depth < 8) && G(35, !in_qsearch)) {

      G(36, {
        // REVERSE FUTILITY PRUNING
        if (static_eval - 47 * depth >= beta) {
          return static_eval;
        }
        })

        G(36, // RAZORING
          in_qsearch = static_eval + 131 * depth <= alpha;)
    }

    // NULL MOVE PRUNING
    if (G(37, do_null) && G(37, depth > 2) && G(37, static_eval >= beta)) {
      Position npos = *pos;
      flip_pos(&npos);
      npos.ep = 0;
      const i32 score =
        -search(H(28, 2, &npos), H(28, 2, ply + 1), H(28, 2, depth - 3 - depth / 4), H(28, 2, -beta), H(29, 2, -alpha),
#ifdef FULL
          nodes,
#endif
          H(29, 2, stack), H(29, 2, pos_history_count), H(29, 2, false));
      if (score >= beta) {
        return score;
      }
    }
  }

  G(38, i32 moves_evaluated = 0;)
    G(38, i32 quiets_evaluated = 0;)
    G(38, u8 tt_flag = Upper;)
    G(38, stack[pos_history_count + ply + 2].position_hash = tt_hash;)
    G(38, i32 best_score = in_qsearch ? static_eval : -inf;)
    G(38, stack[ply].num_moves = movegen(pos, stack[ply].moves, in_qsearch);)

    for (i32 move_index = 0; move_index < stack[ply].num_moves; move_index++) {
      i32 move_score = ~0x1010101LL; // Ends up as large negative

      // MOVE ORDERING
      for (i32 order_index = move_index; order_index < stack[ply].num_moves;
        order_index++) {
        assert(stack[ply].moves[order_index].takes_piece ==
          piece_on(pos, stack[ply].moves[order_index].to));
        const i32 order_move_score =
          G(39, // KILLER MOVE
            move_equal(&stack[ply].killer, &stack[ply].moves[order_index]) *
            915) +
          G(39, // PREVIOUS BEST MOVE FIRST
            (move_equal(&stack[ply].best_move, &stack[ply].moves[order_index])
              << 30)) +
          G(39, // MOST VALUABLE VICTIM
            stack[ply].moves[order_index].takes_piece * 921) +
          G(39, // HISTORY HEURISTIC
            move_history[pos->flipped]
            [stack[ply].moves[order_index].takes_piece]
            [stack[ply].moves[order_index].from]
            [stack[ply].moves[order_index].to]);
        if (order_move_score > move_score) {
          G(40, move_score = order_move_score;)
            G(40, swapmoves(&stack[ply].moves[move_index],
              &stack[ply].moves[order_index]);)
        }
      }

      // FORWARD FUTILITY PRUNING / DELTA PRUNING
      if (G(41, depth < 8) &&
        G(41, static_eval + 128 * depth +
          max_material[stack[ply].moves[move_index].takes_piece] +
          max_material[stack[ply].moves[move_index].promo] <
          alpha) &&
        G(41, moves_evaluated) && G(41, !in_check)) {
        break;
      }

      Position npos = *pos;
#ifdef FULL
      (*nodes)++;
#endif
      if (!makemove(H(11, 3, &npos), H(11, 3, &stack[ply].moves[move_index]))) {
        continue;
      }

      // PRINCIPAL VARIATION SEARCH
      i32 low = moves_evaluated == 0 ? -beta : -alpha - 1;
      moves_evaluated++;

      // LATE MOVE REDCUCTION
      i32 reduction =
        G(42, depth > 1) && G(42, moves_evaluated > 6)
        ? 1 + (alpha == beta - 1) + moves_evaluated / 11 + !improving
        : 1;

      i32 score;
      while (true) {
        score = -search(H(28, 3, &npos), H(28, 3, ply + 1), H(28, 3, depth - reduction), H(28, 3, low), H(29, 3, -alpha),
#ifdef FULL
          nodes,
#endif
          H(29, 3, stack), H(29, 3, pos_history_count), H(29, 3, true));

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
        stack[ply].best_move = stack[ply].moves[move_index];
        alpha = score;
        tt_flag = Exact;
        if (score >= beta) {
          tt_flag = Lower;
          assert(stack[ply].best_move.takes_piece ==
            piece_on(pos, stack[ply].best_move.to));
          G(43, if (stack[ply].best_move.takes_piece == None) {
            stack[ply].killer = stack[ply].best_move;
          })
            G(43,
              i32* const this_hist =
              &move_history[pos->flipped][stack[ply].best_move.takes_piece]
              [stack[ply].best_move.from]
              [stack[ply].best_move.to];
          const i32 bonus = depth * depth;
          *this_hist += bonus - bonus * *this_hist / 1024;
          for (i32 prev_index = 0; prev_index < move_index; prev_index++) {
            const Move prev = stack[ply].moves[prev_index];
            i32* const prev_hist =
              &move_history[pos->flipped][prev.takes_piece][prev.from]
              [prev.to];
            *prev_hist -= bonus + bonus * *prev_hist / 1024;
          })
            break;
        }
      }

      if (stack[ply].moves[move_index].takes_piece == None) {
        quiets_evaluated++;
      }

      // LATE MOVE PRUNING
      if (G(44, !in_check) && G(44, alpha == beta - 1) &&
        G(44, quiets_evaluated > 1 + depth * depth >> !improving)) {
        break;
      }
    }

  // MATE / STALEMATE DETECTION
  if (best_score == -inf) {
    return (ply - mate) * in_check;
  }

  *tt_entry = (TTEntry){ .partial_hash = tt_hash_partial,
                        .move = stack[ply].best_move,
                        .score = best_score,
                        .depth = depth,
                        .flag = tt_flag };

  return best_score;
}

static void iteratively_deepen(
#ifdef FULL
  i32 maxdepth, u64* nodes,
#endif
  H(45, 1, Position* const restrict pos), H(45, 1, SearchStack* restrict stack),
  H(45, 1, const i32 pos_history_count)) {
  start_time = get_time();
#ifdef FULL
  for (i32 depth = 1; depth < maxdepth; depth++) {
#else
  for (i32 depth = 1; depth < max_ply; depth++) {
#endif
    i32 score = search(H(28, 4, pos), H(28, 4, 0), H(28, 4, depth), H(28, 4, -inf), H(29, 4, inf),
#ifdef FULL
      nodes,
#endif
      H(29, 4, stack), H(29, 4, pos_history_count), H(29, 4, false));
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
    move_str(H(6, 2, move_name), H(6, 2, &stack[0].best_move), H(6, 2, pos->flipped));
    puts(move_name);
#endif

    if (elapsed > max_time / 16) {
      break;
    }
  }
  char move_name[8];
  move_str(H(6, 3, move_name), H(6, 3, &stack[0].best_move), H(6, 3, pos->flipped));
  putl("bestmove ");
  puts(move_name);
  }

static void display_pos(Position* const pos) {
  Position npos = *pos;
  if (npos.flipped) {
    flip_pos(&npos);
  }
  for (i32 rank = 7; rank >= 0; rank--) {
    for (i32 file = 0; file < 8; file++) {
      i32 sq = rank * 8 + file;
      u64 bb = 1ULL << sq;
      i32 piece = piece_on(&npos, sq);
      if (bb & npos.colour[0]) {
        if (piece == Pawn) {
          putl("P");
        }
        else if (piece == Knight) {
          putl("N");
        }
        else if (piece == Bishop) {
          putl("B");
        }
        else if (piece == Rook) {
          putl("R");
        }
        else if (piece == Queen) {
          putl("Q");
        }
        else if (piece == King) {
          putl("K");
        }
      }
      else if (bb & npos.colour[1]) {
        if (piece == Pawn) {
          putl("p");
        }
        else if (piece == Knight) {
          putl("n");
        }
        else if (piece == Bishop) {
          putl("b");
        }
        else if (piece == Rook) {
          putl("r");
        }
        else if (piece == Queen) {
          putl("q");
        }
        else if (piece == King) {
          putl("k");
        }
      }
      else {
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
(Position){ .ep = 0,
           .colour = {0xFFFFull, 0xFFFF000000000000ull},
           .pieces = {0, 0xFF00000000FF00ull, 0x4200000000000042ull,
                      0x2400000000000024ull, 0x8100000000000081ull,
                      0x800000000000008ull, 0x1000000000000010ull},
           .castling = {true, true, true, true} };

#ifdef FULL
static void bench() {
  Position pos;
  i32 pos_history_count = 0;
#ifdef LOWSTACK
  SearchStack* stack = malloc(sizeof(SearchStack) * 1024);
#else
  SearchStack stack[1024];
#endif
  __builtin_memset(move_history, 0, sizeof(move_history));
  pos = start_pos;
  max_time = 99999999999;
  u64 nodes = 0;
  const u64 start = get_time();
  iteratively_deepen(20, &nodes, H(45, 2, &pos), H(45, 2, stack), H(45, 2, pos_history_count));
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

  G(46, char line[4096];)
    G(46, Position pos;)
    G(46, i32 pos_history_count;)
    G(46, // #ifdef LOWSTACK
      //  SearchStack *stack = malloc(sizeof(SearchStack) * 1024);
      // #else
      SearchStack stack[1024];
  // #endif
    )
    G(46, __builtin_memset(move_history, 0, sizeof(move_history));)
    G(46, init();)

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
    }
    else if (!strcmp(line, "ucinewgame")) {
      __builtin_memset(tt, 0, sizeof(tt));
      __builtin_memset(move_history, 0, sizeof(move_history));
    }
    else if (!strcmp(line, "bench")) {
      bench();
    }
    else if (!strcmp(line, "gi")) {
      max_time = 99999999999;
      iteratively_deepen(max_ply, &nodes, H(45, 3, &pos), H(45, 3, stack), H(45, 3, pos_history_count));
    }
    else if (!strcmp(line, "d")) {
      display_pos(&pos);
    }
    else if (!strcmp(line, "perft")) {
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
    if (line[0] == 'q') {
      exit_now();
    }
    else if (line[0] == 'i') {
      puts("readyok");
    }
    else if (line[0] == 'p') {
      G(47, pos = start_pos;)
        G(47, pos_history_count = 0;)
        while (true) {
          const bool line_continue = getl(line);

#if FULL
          if (!strcmp(line, "fen")) {
            getl(line);
            get_fen(&pos, line);
          }
#endif

          const i32 num_moves = movegen(&pos, stack[0].moves, false);
          for (i32 i = 0; i < num_moves; i++) {
            char move_name[8];
            move_str(H(6, 4, move_name), H(6, 4, &stack[0].moves[i]), H(6, 4, pos.flipped));
            assert(move_string_equal(line, move_name) ==
              !strcmp(line, move_name));
            if (move_string_equal(line, move_name)) {
              stack[pos_history_count].position_hash = get_hash(&pos);
              pos_history_count++;
              if (stack[0].moves[i].takes_piece != None) {
                pos_history_count = 0;
              }
              makemove(H(11, 4, &pos), H(11, 4, &stack[0].moves[i]));
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
      iteratively_deepen(max_ply, &nodes, H(45, 4, &pos), H(45, 4, stack), H(45, 4, pos_history_count));
#else
      for (i32 i = 0; i < (pos.flipped ? 4 : 2); i++) {
        getl(line);
        max_time = atoi(line) / 2;
      }
      iteratively_deepen(H(45, 5, &pos), H(45, 5, stack), H(45, 5, pos_history_count));
#endif
    }
  }
}

#if !defined(NOSTDLIB) || defined(FULL)
#ifdef NOSTDLIB
__attribute__((naked)) void _start() {
#ifdef FULL
  register long* stack asm("rsp");
  int argc = (int)*stack;
  char** argv = (char**)(stack + 1);
#endif
#else
int main(int argc, char** argv) {
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
