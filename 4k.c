#pragma region libc shims

#ifdef _MSC_VER
#define __attribute__(...)
#endif

#if defined(_WIN64) || defined(__x86_64__) || defined(__ppc64__) ||            \
    defined(__aarch64__)
#define ARCH64 1
#define size_t unsigned long long
#define ssize_t long long
#else
#define ARCH32 1
#define size_t unsigned int
#define ssize_t int
#endif

#define u64 unsigned long long
#define i32 int
#define i16 short
#define i8 char
#define u8 unsigned char
#define bool char

#define NULL ((void *)0)
#define false 0
#define true 1

enum [[nodiscard]] {
  stdin = 0,
  stdout = 1,
  stderr = 2,
};

static ssize_t _sys(ssize_t call, ssize_t arg1, ssize_t arg2, ssize_t arg3) {
  ssize_t ret;
#if ARCH64
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

static void exit() {
#if ARCH32
  _sys(1, 0, 0, 0);
#else
  _sys(60, 0, 0, 0);
#endif
}

static void* malloc(size_t size)
{
  ssize_t call = 9;     // syscall number for mmap
  ssize_t addr = 0;     // addr = NULL
  ssize_t prot = 0x03;  // PROT_READ | PROT_WRITE
  ssize_t flags = 0x22; // MAP_PRIVATE | MAP_ANONYMOUS
  ssize_t fd = -1;      // no file descriptor
  ssize_t offset = 0;

  ssize_t ret;
  asm volatile(
    "mov %5, %%r10\n\t"
    "mov %6, %%r8\n\t"
    "mov %7, %%r9\n\t"
    "syscall"
    : "=a"(ret)
    : "a"(call),
    "D"(addr),
    "S"(size),
    "d"(prot),
    "r"(flags),
    "r"(fd),
    "r"(offset)
    : "r10", "r8", "r9", "rcx", "r11", "memory"
    );

  return (void*)ret;
}

[[nodiscard]] static i32 strlen(const char *const string) {
  i32 length = 0;
  while (string[length]) {
    length++;
  }
  return length;
}

static void puts(const char *const string) {
#if ARCH64
  _sys(1, stdout, (ssize_t)string, strlen(string));
#else
  _sys(4, stdout, (ssize_t)string, strlen(string));
#endif
}

// Non-standard, gets a word instead of a line
static bool gets(char *string) {
  while (true) {
#if ARCH64
    const int result = _sys(0, stdin, (ssize_t)string, 1);
#else
    const int result = _sys(3, stdin, (ssize_t)string, 1);
#endif

    // Assume stdin never closes on mini build
#if FULL
    if (result < 1) {
      exit();
    }
#endif

    if (*string == '\n') {
      *string = 0;
      return false;
    }

    if (*string == ' ') {
      *string = 0;
      return true;
    }

    string++;
  }
}

[[nodiscard]] static bool strcmp(const char *lhs, const char *rhs) {
  while (*lhs || *rhs) {
    if (*lhs != *rhs) {
      return true;
    }
    lhs++;
    rhs++;
  }
  return false;
}

[[nodiscard]] static size_t stoi(const char *string) {
  size_t result = 0;
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

static void _printf(const char *format, const size_t *args) {
  int value;
  char buffer[16], *string;

  while (true) {
    if (!*format) {
      break;
    }
    if (*format != '%') {
#if ARCH64
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
      puts((char *)*args);
      break;
    case 'i':
      value = *args;
      if (value < 0) {
        puts("-");
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
      puts(string);
      break;
    }
    args++;
  }
}

#if ASSERTS
#define assert(condition)                                                      \
  if (!(condition)) {                                                          \
    printf("Assert failed on line %i: ", __LINE__);                            \
    puts(#condition "\n");                                                     \
    _sys(60, 1, 0, 0);                                                         \
  }
#else
#define assert(condition)
#endif

#pragma endregion

#pragma region base

typedef struct [[nodiscard]] {
  ssize_t tv_sec;  // seconds
  ssize_t tv_nsec; // nanoseconds
} timespec;

enum [[nodiscard]] { None, Pawn, Knight, Bishop, Rook, Queen, King };
enum [[nodiscard]] { Upper, Lower, Exact };

typedef struct [[nodiscard]] __attribute__((aligned(8))) {
  u8 from;
  u8 to;
  u8 promo;
} Move;

typedef struct [[nodiscard]] {
  u64 colour[2];
  u64 pieces[7];
  u64 ep;
  bool castling[4];
  bool flipped;
} Position;

typedef struct [[nodiscard]] {
  u64 key;
  Move move;
  u8 flag;
  i16 score;
  i16 depth;
} TTEntry;

[[nodiscard]] size_t get_time() {
  timespec ts;
#if ARCH64
  _sys(228, 1, (ssize_t)&ts, 0);
#else
  _sys(265, 1, (ssize_t)&ts, 0);
#endif
  return ts.tv_sec * 1000 + ts.tv_nsec / 1000000;
}

[[nodiscard]] static u64 flip_bb(const u64 bb) { return __builtin_bswap64(bb); }

#if ARCH32
#pragma GCC push_options
#pragma GCC optimize("O3")
#endif
static i32 lsb(u64 bb) { return __builtin_ctzll(bb); }
#if ARCH32
#pragma GCC pop_options
#endif

[[nodiscard]] static i32 count(const u64 bb) {
  return __builtin_popcountll(bb);
}

[[nodiscard]] static u64 west(const u64 bb) {
  return bb >> 1 & ~0x8080808080808080ull;
}

[[nodiscard]] static u64 east(const u64 bb) {
  return bb << 1 & ~0x101010101010101ull;
}

[[nodiscard]] static u64 north(const u64 bb) { return bb << 8; }

[[nodiscard]] static u64 south(const u64 bb) { return bb >> 8; }

[[nodiscard]] static u64 nw(const u64 bb) { return north(west(bb)); }

[[nodiscard]] static u64 ne(const u64 bb) { return north(east(bb)); }

[[nodiscard]] static u64 sw(const u64 bb) { return south(west(bb)); }

[[nodiscard]] static u64 se(const u64 bb) { return south(east(bb)); }

[[nodiscard]] static u64 shift(const u64 bb, const i32 shift, const u64 mask) {
  return shift > 0 ? bb << shift & mask : bb >> -shift & mask;
}

[[nodiscard]] static u64 ray(const i32 sq, const u64 blockers,
                             const i32 shift_by, const u64 mask) {
  assert(sq >= 0);
  assert(sq < 64);
  u64 result = shift(1ull << sq, shift_by, mask);
  for (i32 i = 0; i < 6; i++) {
    result |= shift(result & ~blockers, shift_by, mask);
  }
  return result;
}

[[nodiscard]] static u64 bishop(const i32 sq, const u64 blockers) {
  assert(sq >= 0);
  assert(sq < 64);
  return ray(sq, blockers, 7, ~0x8080808080808080ull) |  // Northwest
         ray(sq, blockers, 9, ~0x101010101010101ull) |   // Northeast
         ray(sq, blockers, -9, ~0x8080808080808080ull) | // Southwest
         ray(sq, blockers, -7, ~0x101010101010101ull);   // Southeast
}

[[nodiscard]] static u64 rook(const i32 sq, const u64 blockers) {
  assert(sq >= 0);
  assert(sq < 64);
  return ray(sq, blockers, 8, ~0x0ull) |                 // North
         ray(sq, blockers, -1, ~0x8080808080808080ull) | // West
         ray(sq, blockers, -8, ~0x0ull) |                // South
         ray(sq, blockers, 1, ~0x101010101010101ull);    // East
}

[[nodiscard]] static u64 knight(const i32 sq, const u64 blockers) {
  (void)blockers;
  assert(sq >= 0);
  assert(sq < 64);
  const u64 bb = 1ull << sq;
  return (bb << 15 | bb >> 17) & ~0x8080808080808080ull |
         (bb << 17 | bb >> 15) & ~0x101010101010101ull |
         (bb << 10 | bb >> 6) & 0xFCFCFCFCFCFCFCFCull |
         (bb << 6 | bb >> 10) & 0x3F3F3F3F3F3F3F3Full;
}

[[nodiscard]] static u64 king(const i32 sq, const u64 blockers) {
  (void)blockers;
  assert(sq >= 0);
  assert(sq < 64);
  const u64 bb = 1ull << sq;
  return bb << 8 | bb >> 8 |
         (bb >> 1 | bb >> 9 | bb << 7) & ~0x8080808080808080ull |
         (bb << 1 | bb << 9 | bb >> 7) & ~0x101010101010101ull;
}

static void move_str(char *str, const Move *move, const i32 flip) {
  assert(move->from >= 0);
  assert(move->from < 64);
  assert(move->to >= 0);
  assert(move->to < 64);
  assert(move->from != move->to);
  assert(move->promo == None || move->promo == Knight ||
         move->promo == Bishop || move->promo == Rook || move->promo == Queen);

  // Hack to save bytes, technically UB but works on GCC 14.2
  for (i32 i = 0; i < 2; i++) {
    str[i * 2] = 'a' + (&move->from)[i] % 8;
    str[i * 2 + 1] = '1' + ((&move->from)[i] / 8 ^ 7 * flip);
  }

  str[4] = "\0\0nbrq"[move->promo];
  str[5] = '\0';
}

[[nodiscard]] static i32 piece_on(const Position *const pos, const i32 sq) {
  assert(sq >= 0);
  assert(sq < 64);
  for (i32 i = Pawn; i <= King; ++i) {
    if (pos->pieces[i] & 1ull << sq) {
      return i;
    }
  }
  return None;
}

static void swapu64(u64 *const lhs, u64 *const rhs) {
  const u64 temp = *lhs;
  *lhs = *rhs;
  *rhs = temp;
}

static void swapbool(bool *const lhs, bool *const rhs) {
  const bool temp = *lhs;
  *lhs = *rhs;
  *rhs = temp;
}

static void flip_pos(Position *const pos) {
  pos->flipped ^= 1;
  pos->colour[0] = flip_bb(pos->colour[0]);
  pos->colour[1] = flip_bb(pos->colour[1]);
  swapu64(&pos->colour[0], &pos->colour[1]);
  swapbool(&pos->castling[0], &pos->castling[2]);
  swapbool(&pos->castling[1], &pos->castling[3]);
  for (i32 i = Pawn; i <= King; ++i) {
    pos->pieces[i] = flip_bb(pos->pieces[i]);
  }

  pos->ep = flip_bb(pos->ep);
}

[[nodiscard]] static i32 is_attacked(const Position *const pos, const i32 sq,
                                     const i32 them) {
  assert(sq >= 0);
  assert(sq < 64);
  const u64 bb = 1ull << sq;
  const u64 pawns = pos->colour[them] & pos->pieces[Pawn];
  const u64 pawn_attacks = them ? sw(pawns) | se(pawns) : nw(pawns) | ne(pawns);
  return pawn_attacks & bb ||
         pos->colour[them] & pos->pieces[Knight] & knight(sq, 0) ||
         bishop(sq, pos->colour[0] | pos->colour[1]) & pos->colour[them] &
             (pos->pieces[Bishop] | pos->pieces[Queen]) ||
         rook(sq, pos->colour[0] | pos->colour[1]) & pos->colour[them] &
             (pos->pieces[Rook] | pos->pieces[Queen]) ||
         king(sq, pos->colour[0] | pos->colour[1]) & pos->colour[them] &
             pos->pieces[King];
}

static i32 makemove(Position *const pos, const Move *const move) {
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

  const i32 piece = piece_on(pos, move->from);
  assert(piece != None);
  const i32 captured = piece_on(pos, move->to);
  assert(captured != King);

  // Move the piece
  pos->colour[0] ^= mask;
  pos->pieces[piece] ^= mask;

  // Captures
  if (captured != None) {
    pos->colour[1] ^= to;
    pos->pieces[captured] ^= to;
  }

  // En passant
  if (piece == Pawn && to == pos->ep) {
    pos->colour[1] ^= to >> 8;
    pos->pieces[Pawn] ^= to >> 8;
  }

  pos->ep = 0;

  // Pawn double move
  if (piece == Pawn && move->to - move->from == 16)
    pos->ep = to >> 8;

  // Castling
  if (piece == King) {
    const u64 bb = move->to - move->from == 2   ? 0xa0
                   : move->from - move->to == 2 ? 0x9
                                                : 0;
    pos->colour[0] ^= bb;
    pos->pieces[Rook] ^= bb;
  }

  // Promotions
  if (piece == Pawn && move->to > 55) {
    pos->pieces[Pawn] ^= to;
    pos->pieces[move->promo] ^= to;
  }

  // Update castling permissions
  pos->castling[0] &= !(mask & 0x90ull);
  pos->castling[1] &= !(mask & 0x11ull);
  pos->castling[2] &= !(mask & 0x9000000000000000ull);
  pos->castling[3] &= !(mask & 0x1100000000000000ull);

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
  return !is_attacked(pos, lsb(pos->colour[1] & pos->pieces[King]), false);
}

static void generate_pawn_moves(
#if ASSERTS
    const Position *const pos,
#endif
    Move *const movelist, i32 *const num_moves, u64 to_mask, const i32 offset

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
    if (to > 55) {
      for (u8 piece = Queen; piece >= Knight; piece--) {
        movelist[(*num_moves)++] = (Move){from, to, piece};
      }
    } else
      movelist[(*num_moves)++] = (Move){from, to, None};
  }
}

static void generate_piece_moves(Move *const movelist, i32 *num_moves,
                                 const Position *pos, const i32 piece,
                                 const u64 to_mask,
                                 u64 (*f)(const i32, const u64)) {
  assert(piece == Knight || piece == Bishop || piece == Rook ||
         piece == Queen || piece == King);
  u64 copy = pos->colour[0] & pos->pieces[piece];
  while (copy) {
    const u8 fr = lsb(copy);
    assert(fr >= 0);
    assert(fr < 64);
    copy &= copy - 1;
    u64 moves = f(fr, pos->colour[0] | pos->colour[1]) & to_mask;
    while (moves) {
      const u8 to = lsb(moves);
      assert(to >= 0);
      assert(to < 64);
      moves &= moves - 1;
      movelist[(*num_moves)++] = (Move){fr, to, None};
      assert(*num_moves < 256);
    }
  }
}

[[nodiscard]] static i32 movegen(const Position *const pos,
                                 Move *const movelist,
                                 const i32 only_captures) {
  i32 num_moves = 0;
  const u64 all = pos->colour[0] | pos->colour[1];
  const u64 to_mask = only_captures ? pos->colour[1] : ~pos->colour[0];
  const u64 pawns = pos->colour[0] & pos->pieces[Pawn];
  generate_pawn_moves(
#if ASSERTS
      pos,
#endif
      movelist, &num_moves,
      north(pawns) & ~all & (only_captures ? 0xFF00000000000000ull : ~0ull),
      -8);
  if (!only_captures) {
    generate_pawn_moves(
#if ASSERTS
        pos,
#endif
        movelist, &num_moves, north(north(pawns & 0xFF00) & ~all) & ~all, -16);
  }
  generate_pawn_moves(
#if ASSERTS
      pos,
#endif
      movelist, &num_moves, nw(pawns) & (pos->colour[1] | pos->ep), -7);
  generate_pawn_moves(
#if ASSERTS
      pos,
#endif
      movelist, &num_moves, ne(pawns) & (pos->colour[1] | pos->ep), -9

  );
  generate_piece_moves(movelist, &num_moves, pos, Knight, to_mask, knight);
  generate_piece_moves(movelist, &num_moves, pos, Bishop, to_mask, bishop);
  generate_piece_moves(movelist, &num_moves, pos, Rook, to_mask, rook);
  generate_piece_moves(movelist, &num_moves, pos, Queen, to_mask, rook);
  generate_piece_moves(movelist, &num_moves, pos, Queen, to_mask, bishop);
  generate_piece_moves(movelist, &num_moves, pos, King, to_mask, king);
  if (!only_captures && pos->castling[0] && !(all & 0x60ull) &&
      !is_attacked(pos, 4, true) && !is_attacked(pos, 5, true)) {
    movelist[num_moves++] = (Move){4, 6, None};
  }
  if (!only_captures && pos->castling[1] && !(all & 0xEull) &&
      !is_attacked(pos, 4, true) && !is_attacked(pos, 3, true)) {
    movelist[num_moves++] = (Move){4, 2, None};
  }

  assert(num_moves < 256);
  return num_moves;
}

#pragma endregion

#pragma region engine

[[nodiscard]] static u64 perft(const Position *const pos, const i32 depth) {
  if (depth == 0) {
    return 1;
  }

  u64 nodes = 0;
  Move moves[256];
  const i32 num_moves = movegen(pos, moves, false);

  for (i32 i = 0; i < num_moves; ++i) {
    Position npos = *pos;

    // Check move legality
    if (!makemove(&npos, &moves[i])) {
      continue;
    }

    nodes += perft(&npos, depth - 1);
  }

  return nodes;
}

u64* keys;
#define num_tt_entries (1024ULL * 1024)
TTEntry* transposition_table;

[[nodiscard]] static u64 get_hash(const Position* pos) {
  u64 hash = pos->flipped;

  // Pieces
  for(i32 c = 0; c < 2; c++) {
    for (i32 p = Pawn; p <= King; p++) {
      u64 copy = pos->colour[c] & pos->pieces[p];
      while (copy) {
        const i32 sq = lsb(copy);
        copy &= copy - 1;
        hash ^= keys[c * 6 * 64 + p * 64 + sq];
      }
    }
  }

  // En passant square
  if (pos->ep) {
    hash ^= keys[14 * 64 + lsb(pos->ep)];
  }

  // Castling permissions
  hash ^= keys[15 * 64 + pos->castling[0] + pos->castling[1] * 2 + pos->castling[2] * 4 + pos->castling[3] * 8];

  return hash;
}

__attribute__((aligned(8))) static const i16 material[] = {0,   127,  373, 406,
                                                           633, 1220, 0};
__attribute__((aligned(8))) static const i8 pst_rank[] = {
    0,   -7,  -8,  -8, -1, 24, 79, 0,   // Pawn
    -20, -11, -1,  8,  16, 18, 5,  -16, // Knight
    -15, -3,  3,   6,  8,  9,  1,  -8,  // Bishop
    -11, -15, -13, -5, 6,  12, 15, 12,  // Rook
    -15, -9,  -6,  -2, 5,  11, 5,  11,  // Queen
    -12, -8,  -4,  3,  11, 14, 8,  -10, // King
};
__attribute__((aligned(8))) static const i8 pst_file[] = {
    -2,  2,  -3, -1, -1, 1,  8,  -4,  // Pawn
    -17, -5, 4,  9,  9,  8,  1,  -9,  // Knight
    -8,  0,  2,  3,  4,  1,  3,  -4,  // Bishop
    -4,  1,  4,  5,  4,  2,  -1, -10, // Rook
    -13, -6, 1,  3,  3,  4,  4,  4,   // Queen
    -8,  2,  0,  -1, -2, -2, 4,  -6,  // King
};

static i32 eval(Position *const pos) {
  i32 score = 16;
  for (i32 c = 0; c < 2; c++) {
    for (i32 p = Pawn; p <= King; p++) {
      u64 copy = pos->colour[0] & pos->pieces[p];
      while (copy) {
        const i32 sq = lsb(copy);
        copy &= copy - 1;

        const int rank = sq >> 3;
        const int file = sq & 7;

        score += material[p];

        score += pst_rank[(p - 1) * 8 + rank] * 2;
        score += pst_file[(p - 1) * 8 + file] * 2;
      }
    }

    score = -score;
    flip_pos(pos);
  }
  return score;
}

enum { inf = 32000, mate = 30000 };
static size_t start_time;
static size_t total_time;

static i32 search(Position *const pos, const i32 ply, i32 depth, i32 alpha,
                  const i32 beta,
#if FULL
                  u64 *nodes,
#endif
                  u64 history[64][64], Move best_moves[128]) {
  assert(alpha < beta);
  assert(ply >= 0);

  const bool in_check =
      is_attacked(pos, lsb(pos->colour[0] & pos->pieces[King]), true);
  if (in_check) {
    depth++;
  }

  if (ply > 127) {
    return alpha;
  }

  //i32 in_qsearch = depth <= 0;
  const u64 tt_key = get_hash(pos);

  //if (ply > 0 && !in_qsearch) {
  //  // Repetition detection
  //  for (const u64 old_hash : hash_history)
  //    if (old_hash == tt_key)
  //      return 0;
  //}

  // TT Probing
  TTEntry *const tt_entry = &transposition_table[tt_key % num_tt_entries];
  Move tt_move;
  if (tt_entry->key == tt_key) {
    tt_move = tt_entry->move;
    if (alpha == beta - 1 && tt_entry->depth >= depth && tt_entry->flag != tt_entry->score < beta)
      // If tt_entry.score < beta, tt_entry.flag cannot be Lower (ie must be Upper or Exact).
      // Otherwise, tt_entry.flag cannot be Upper (ie must be Lower or Exact).
      return tt_entry->score;
  }

  const bool in_qsearch = depth <= 0;
  const i32 static_eval = eval(pos);

  if (in_qsearch && static_eval > alpha) {
    if (static_eval >= beta) {
      return static_eval;
    }
    alpha = static_eval;
  }

  if (!in_qsearch && alpha == beta - 1 && !in_check &&
      static_eval - 128 * depth >= beta) {
    return static_eval;
  }

  Move moves[256];
  const i32 num_moves = movegen(pos, moves, in_qsearch);
  i32 moves_evaluated = 0;
  i32 best_score = in_qsearch ? static_eval : -inf;
  best_moves[ply] = tt_move;
  u8 tt_flag = Upper;

  for (i32 move_index = 0; move_index < num_moves; move_index++) {
    u64 move_score = 0;

    for (i32 order_index = move_index; order_index < num_moves; order_index++) {
      const u64 order_move_score =
          ((u64)(*(u64 *)&tt_move == *(u64 *)&moves[order_index])
           << 60) +
          ((u64)piece_on(pos, moves[order_index].to) << 50) +
          history[moves[order_index].from][moves[order_index].to];
      if (order_move_score > move_score) {
        move_score = order_move_score;

        swapu64((u64 *)&moves[move_index], (u64 *)&moves[order_index]);
      }
    }

    Position npos = *pos;
#if FULL
    (*nodes)++;
#endif
    if (!makemove(&npos, &moves[move_index])) {
      continue;
    }

    i32 low = moves_evaluated == 0 ? -beta : -alpha - 1;
    i32 reduction = 1 + (depth > 3 && moves_evaluated > 5);

    i32 score;
    while (true) {
      score = -search(&npos, ply + 1, depth - reduction, low, -alpha,
#if FULL
                      nodes,
#endif
                      history, best_moves);

      if (score <= alpha || (low == -beta && reduction == 1)) {
        break;
      }

      low = -beta;
      reduction = 1;
    }

    if(depth > 4 && get_time() - start_time > total_time / 4) {
      return 0;
    }

    moves_evaluated++;

    if(score > best_score) {
      best_score = score;
    }

    if (score > alpha) {
      best_moves[ply] = moves[move_index];
      tt_flag = Exact;
      alpha = score;

      if (score >= beta) {
        tt_flag = Lower;
        if (piece_on(pos, moves[move_index].to) == None) {
          history[moves[move_index].from][moves[move_index].to] +=
              depth * depth;
        }
        break;
      }
    }
  }

  if (moves_evaluated == 0 && !in_qsearch) {
    if (in_check) {
      return -mate;
    }

    return 0;
  }

  *tt_entry = (TTEntry){
    .key = tt_key,
    .move = best_moves[ply],
    .flag = tt_flag,
    .score = (i16)best_score,
    .depth = (i16)(!in_qsearch * depth)
  };

  return alpha; 
}
// #define FULL true

static void iteratively_deepen(Position *const pos) {
  start_time = get_time();
  Move best_moves[128];
  u64 history[64][64] = {0};
  u64 nodes = 0;
  for (i32 depth = 1; depth < 128; depth++) {
    i32 score = search(pos, 0, depth, -inf, inf,
#if FULL
                       &nodes,
#endif
                       history, best_moves);
    size_t elapsed = get_time() - start_time;

#if FULL
    char info_move_name[6];
    move_str(info_move_name, &best_moves[0], pos->flipped);
    printf("info depth %i score cp %i time %i nodes %i", depth, score, elapsed,
           nodes);
    if (elapsed > 0) {
      const u64 nps = nodes * 1000 / elapsed;
      printf(" nps %i", nps);
    }

    puts(" pv ");
    puts(info_move_name);
    puts("\n");
#endif

    if (elapsed > total_time / 64) {
      break;
    }
  }
  char move_name[6];
  move_str(move_name, &best_moves[0], pos->flipped);
  puts("bestmove ");
  puts(move_name);
  puts("\n");
}

// https://vigna.di.unimi.it/ftp/papers/xorshift.pdf
u64 rng_state = 1070372;
[[nodiscard]] static u64 rng() {
  rng_state ^= rng_state >> 12;
  rng_state ^= rng_state << 25;
  rng_state ^= rng_state >> 27;
  return rng_state * 2685821657736338717LL;
}

void _start() {
  char line[1024];
  Position pos;
  Move moves[256];
  i32 num_moves;

  keys = malloc(1024 * sizeof(u64));
  for(i32 i = 0; i < 1024; i++) {
    keys[i] = rng();
  }

  transposition_table = malloc(1024LL * 1024 * sizeof(TTEntry));

#if FULL
  pos = (Position){.castling = {true, true, true, true},
                   .colour = {0xFFFFull, 0xFFFF000000000000ull},
                   .pieces = {0, 0xFF00000000FF00ull, 0x4200000000000042ull,
                              0x2400000000000024ull, 0x8100000000000081ull,
                              0x800000000000008ull, 0x1000000000000010ull},
                   .ep = 0};
#endif

#if !FULL
  // Assume first input is "uci"
  gets(line);
  puts("uciok\n");
#endif

  // UCI loop
  while (true) {
    gets(line);
#if FULL
    if (!strcmp(line, "uci")) {
      puts("id name 4k.c\nid author Gediminas Masaitis\nuciok\n");
    } else if (!strcmp(line, "gi")) {
      total_time = 99999999999;
      iteratively_deepen(&pos);
    } else if (!strcmp(line, "perft")) {
      char depth_str[4];
      gets(depth_str);
      const i32 depth = stoi(depth_str);
      const u64 start = get_time();
      const u64 nodes = perft(&pos, depth);
      const u64 end = get_time();
      const i32 elapsed = end - start;
      const u64 nps = elapsed ? 1000 * nodes / elapsed : 0;
      printf("info depth %i nodes %i time %i nps %i \n", depth, nodes,
             end - start, nps);
    } else if (!strcmp(line, "quit")) {
      break;
    }
#endif
    if (line[0] == 'i') {
      puts("readyok\n");
    } else if (line[0] == 'p') {
      pos = (Position){.castling = {true, true, true, true},
                       .colour = {0xFFFFull, 0xFFFF000000000000ull},
                       .pieces = {0, 0xFF00000000FF00ull, 0x4200000000000042ull,
                                  0x2400000000000024ull, 0x8100000000000081ull,
                                  0x800000000000008ull, 0x1000000000000010ull},
                       .ep = 0};

      while (true) {
        const bool line_continue = gets(line);
        num_moves = movegen(&pos, moves, false);
        for (i32 i = 0; i < num_moves; i++) {
          char move_name[6];
          move_str(move_name, &moves[i], pos.flipped);
          if (!strcmp(line, move_name)) {
            makemove(&pos, &moves[i]);
            break;
          }
        }
        if (!line_continue) {
          break;
        }
      }
    } else if (line[0] == 'g') {
#if FULL
      while (true) {
        gets(line);
        if (!pos.flipped && !strcmp(line, "wtime")) {
          gets(line);
          total_time = stoi(line);
          break;
        } else if (pos.flipped && !strcmp(line, "btime")) {
          gets(line);
          total_time = stoi(line);
          break;
        } else if (!strcmp(line, "movetime")) {
          total_time = 40000; // Assume Lichess bot
          break;
        }
      }
#else
      for (i32 i = 0; i < (pos.flipped ? 4 : 2); i++) {
        gets(line);
        total_time = stoi(line);
      }
#endif
      iteratively_deepen(&pos);
    }
  }

  exit();
}

#pragma endregion
