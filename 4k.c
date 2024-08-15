#pragma region libc shims

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
#define u8 unsigned char
#define bool char

#define NULL ((void *)0)
#define false 0
#define true 1

enum {
  stdin = 0,
  stdout = 1,
  stderr = 2,
};

ssize_t _sys(ssize_t call, ssize_t arg1, ssize_t arg2, ssize_t arg3) {
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

static void *memcpy(void *dest, const void *src, size_t n) {
  char *d = dest;
  const char *s = src;

  for (size_t i = 0; i < n; i++) {
    d[i] = s[i];
  }

  return dest;
}

int abs(int x) { return (x < 0) ? -x : x; }

static int strlen(char *string) {
  int length = 0;
  while (string[length]) {
    length++;
  }
  return length;
}

static void puts(char *string) {
#if ARCH64
  _sys(1, stdout, (ssize_t)string, strlen(string));
#else
  _sys(4, stdout, (ssize_t)string, strlen(string));
#endif
}

static bool getw(char *string) {
  while (true) {
#if ARCH64
    int result = _sys(0, stdin, (ssize_t)string, 1);
#else
    int result = _sys(3, stdin, (ssize_t)string, 1);
#endif
    if (result < 1) {
      // EXIT
      _sys(1, 0, 0, 0);
    }
    if (*string == '\n') {
      *string = 0;
      return false;
    } else if (*string == ' ') {
      *string = 0;
      return true;
    }
    string++;
  }
}

static int strcmp(char *lhs, char *rhs) {
  while (*lhs || *rhs) {
    if (*lhs != *rhs) {
      return 1;
    }
    lhs++;
    rhs++;
  }
  return 0;
}

static size_t stoi(char *string) {
  size_t result = 0;
  while (true) {
    if (!*string) {
      return result;
    }
    if (*string < '0' || *string > '9') {
      return -1;
    }
    result *= 10;
    result += *string - '0';
    string++;
  }
}

#define printf(format, ...) _printf(format, (size_t[]){__VA_ARGS__})

static void _printf(char *format, size_t *args) {
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

#pragma endregion

#pragma region base

typedef struct {
  ssize_t tv_sec;  // seconds
  ssize_t tv_nsec; // nanoseconds
} timespec;

size_t get_time() {
  timespec ts;
#if ARCH64
  _sys(228, 1, (ssize_t)&ts, 0);
#else
  _sys(265, 1, (ssize_t)&ts, 0);
#endif
  return ts.tv_sec * 1000 + ts.tv_nsec / 1000000;
}

enum { Pawn, Knight, Bishop, Rook, Queen, King, None };

typedef struct {
  u8 from;
  u8 to;
  u8 promo;
} Move;

typedef struct {
  bool castling[4];
  u64 colour[2];
  u64 pieces[6];
  u64 ep;
  bool flipped;
} Position;

static u64 flip_bb(u64 bb) {
  // return __builtin_bswap64(bb);
  return ((bb & 0x00000000000000FFULL) << 56) |
         ((bb & 0x000000000000FF00ULL) << 40) |
         ((bb & 0x0000000000FF0000ULL) << 24) |
         ((bb & 0x00000000FF000000ULL) << 8) |
         ((bb & 0x000000FF00000000ULL) >> 8) |
         ((bb & 0x0000FF0000000000ULL) >> 24) |
         ((bb & 0x00FF000000000000ULL) >> 40) |
         ((bb & 0xFF00000000000000ULL) >> 56);
}

static i32 lsb(u64 bb) {
  // return __builtin_ctzll(bb);
  if (bb == 0) {
    return 64;
  }

  i32 index = 0;
  while ((bb & 1) == 0) {
    bb >>= 1;
    index++;
  }
  return index;
}

static i32 count(u64 bb) {
  i32 count = 0;
  while (bb) {
    bb &= bb - 1;
    count++;
  }
  return count;
}

static u64 west(const u64 bb) { return bb >> 1 & ~0x8080808080808080ull; }

static u64 east(const u64 bb) { return bb << 1 & ~0x101010101010101ull; }

static u64 north(const u64 bb) { return bb << 8; }

static u64 south(const u64 bb) { return bb >> 8; }

static u64 nw(const u64 bb) { return north(west(bb)); }

static u64 ne(const u64 bb) { return north(east(bb)); }

static u64 sw(const u64 bb) { return south(west(bb)); }

static u64 se(const u64 bb) { return south(east(bb)); }

static u64 ray(const i32 sq, const u64 blockers, u64 (*f)(u64)) {
  u64 mask = f(1ull << sq);
  mask |= f(mask & ~blockers);
  mask |= f(mask & ~blockers);
  mask |= f(mask & ~blockers);
  mask |= f(mask & ~blockers);
  mask |= f(mask & ~blockers);
  mask |= f(mask & ~blockers);
  return mask;
}

static u64 bishop(const i32 sq, const u64 blockers) {
  return ray(sq, blockers, nw) | ray(sq, blockers, ne) | ray(sq, blockers, sw) |
         ray(sq, blockers, se);
}

static u64 rook(const i32 sq, const u64 blockers) {
  return ray(sq, blockers, north) | ray(sq, blockers, east) |
         ray(sq, blockers, south) | ray(sq, blockers, west);
}

static u64 knight(const i32 sq, const u64 blockers) {
  (void)blockers;
  const u64 bb = 1ull << sq;
  return (bb << 15 | bb >> 17) & ~0x8080808080808080ull |
         (bb << 17 | bb >> 15) & ~0x101010101010101ull |
         (bb << 10 | bb >> 6) & 0xFCFCFCFCFCFCFCFCull |
         (bb << 6 | bb >> 10) & 0x3F3F3F3F3F3F3F3Full;
}

static u64 king(const i32 sq, const u64 blockers) {
  (void)blockers;
  const u64 bb = 1ull << sq;
  return bb << 8 | bb >> 8 |
         (bb >> 1 | bb >> 9 | bb << 7) & ~0x8080808080808080ull |
         (bb << 1 | bb << 9 | bb >> 7) & ~0x101010101010101ull;
}

static void move_str(char *str, const Move *move, const i32 flip) {
  str[0] = 'a' + move->from % 8;
  str[1] = '1' + (move->from / 8 ^ 7 * flip);
  str[2] = 'a' + move->to % 8;
  str[3] = '1' + (move->to / 8 ^ 7 * flip);
  if (move->promo != None) {
    str[4] = "nbrq"[move->promo - Knight];
    str[5] = '\0';
  } else {
    str[4] = '\0';
  }
}

static i32 piece_on(const Position *pos, const i32 sq) {
  for (i32 i = Pawn; i < None; ++i)
    if (pos->pieces[i] & 1ull << sq)
      return i;
  return None;
}

static void swapu64(u64 *lhs, u64 *rhs) {
  u64 temp = *lhs;
  *lhs = *rhs;
  *rhs = temp;
}

static void swapbool(bool *lhs, bool *rhs) {
  bool temp = *lhs;
  *lhs = *rhs;
  *rhs = temp;
}

static void flip_pos(Position *pos) {
  pos->flipped ^= 1;
  pos->colour[0] = flip_bb(pos->colour[0]);
  pos->colour[1] = flip_bb(pos->colour[1]);
  swapu64(&pos->colour[0], &pos->colour[1]);
  swapbool(&pos->castling[0], &pos->castling[2]);
  swapbool(&pos->castling[1], &pos->castling[3]);
  for (i32 i = Pawn; i < None; ++i) {
    pos->pieces[i] = flip_bb(pos->pieces[i]);
  }

  pos->ep = flip_bb(pos->ep);
}

static i32 is_attacked(const Position *pos, const i32 sq, const i32 them) {
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

static i32 makemove(Position *pos, const Move *move) {
  const u64 from = 1ull << move->from;
  const u64 to = 1ull << move->to;
  const u64 mask = from | to;

  const i32 piece = piece_on(pos, move->from);
  const i32 captured = piece_on(pos, move->to);

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

  // Return move legality
  return !is_attacked(pos, lsb(pos->colour[1] & pos->pieces[King]), false);
}

static void generate_pawn_moves(Move *const movelist, i32 *num_moves,
                                u64 to_mask, const i32 offset) {
  while (to_mask) {
    const u8 to = lsb(to_mask);
    to_mask &= to_mask - 1;
    const u8 from = to + offset;
    if (to > 55) {
      movelist[(*num_moves)++] = (Move){from, to, Queen};
      movelist[(*num_moves)++] = (Move){from, to, Rook};
      movelist[(*num_moves)++] = (Move){from, to, Bishop};
      movelist[(*num_moves)++] = (Move){from, to, Knight};
    } else
      movelist[(*num_moves)++] = (Move){from, to, None};
  }
}

static void generate_piece_moves(Move *const movelist, i32 *num_moves,
                                 const Position *pos, const i32 piece,
                                 const u64 to_mask,
                                 u64 (*f)(const i32, const u64)) {
  u64 copy = pos->colour[0] & pos->pieces[piece];
  while (copy) {
    const u8 fr = lsb(copy);
    copy &= copy - 1;
    u64 moves = f(fr, pos->colour[0] | pos->colour[1]) & to_mask;
    while (moves) {
      const u8 to = lsb(moves);
      moves &= moves - 1;
      movelist[(*num_moves)++] = (Move){fr, to, None};
    }
  }
}

static i32 movegen(const Position *pos, Move *const movelist,
                   const i32 only_captures) {
  i32 num_moves = 0;
  const u64 all = pos->colour[0] | pos->colour[1];
  const u64 to_mask = only_captures ? pos->colour[1] : ~pos->colour[0];
  const u64 pawns = pos->colour[0] & pos->pieces[Pawn];
  generate_pawn_moves(movelist, &num_moves,
                      north(pawns) & ~all &
                          (only_captures ? 0xFF00000000000000ull : ~0ull),
                      -8);
  if (!only_captures)
    generate_pawn_moves(movelist, &num_moves,
                        north(north(pawns & 0xFF00) & ~all) & ~all, -16);
  generate_pawn_moves(movelist, &num_moves,
                      nw(pawns) & (pos->colour[1] | pos->ep), -7);
  generate_pawn_moves(movelist, &num_moves,
                      ne(pawns) & (pos->colour[1] | pos->ep), -9);
  generate_piece_moves(movelist, &num_moves, pos, Knight, to_mask, knight);
  generate_piece_moves(movelist, &num_moves, pos, Bishop, to_mask, bishop);
  generate_piece_moves(movelist, &num_moves, pos, Rook, to_mask, rook);
  generate_piece_moves(movelist, &num_moves, pos, Queen, to_mask, rook);
  generate_piece_moves(movelist, &num_moves, pos, Queen, to_mask, bishop);
  generate_piece_moves(movelist, &num_moves, pos, King, to_mask, king);
  if (!only_captures && pos->castling[0] && !(all & 0x60ull) &&
      !is_attacked(pos, 4, true) && !is_attacked(pos, 5, true))
    movelist[num_moves++] = (Move){4, 6, None};
  if (!only_captures && pos->castling[1] && !(all & 0xEull) &&
      !is_attacked(pos, 4, true) && !is_attacked(pos, 3, true))
    movelist[num_moves++] = (Move){4, 2, None};
  return num_moves;
}

#pragma endregion

#pragma region engine

static u64 perft(const Position *pos, const i32 depth) {
  if (depth == 0) {
    return 1;
  }

  u64 nodes = 0;
  Move moves[256];
  const i32 num_moves = movegen(pos, moves, false);

  for (i32 i = 0; i < num_moves; ++i) {
    Position npos;
    memcpy(&npos, pos, sizeof(Position));

    // Check move legality
    if (!makemove(&npos, &moves[i])) {
      continue;
    }

    nodes += perft(&npos, depth - 1);
  }

  return nodes;
}

static i32 eval(Position *pos) {
  const i32 material[] = {100, 339, 372, 582, 1180, 0, 0};
  const i32 centralities[] = {2, 20, 16, 1, 3, 11};
  i32 score = 0;
  for (i32 c = 0; c < 2; c++) {
    for (i32 p = 0; p < 6; p++) {
      u64 copy = pos->colour[0] & pos->pieces[p];
      while (copy) {
        const i32 sq = lsb(copy);
        copy &= copy - 1;

        const int rank = sq >> 3;
        const int file = sq & 7;

        // score += count(pos->colour[0] & pos->pieces[p]) * material[p];
        score += material[p];

        const int centrality =
            (7 - abs(7 - rank - file) - abs(rank - file)) / 2;
        score += centrality * centralities[p];
      }
    }

    score = -score;
    flip_pos(pos);
  }
  return score;
}

enum { inf = 32000, mate = 30000 };

static i32 search(Position *pos, i32 depth, i32 alpha, i32 beta,
                  Move *best_move) {
  if (depth == 0) {
    return eval(pos);
  }

  Move moves[256];
  i32 num_moves = movegen(pos, moves, false);
  i32 best_score = -inf;
  i32 moves_evaluated = 0;
  for (i32 move_index = 0; move_index < num_moves; move_index++) {
    Position npos;
    memcpy(&npos, pos, sizeof(Position));
    if (!makemove(&npos, &moves[move_index])) {
      continue;
    }
    moves_evaluated++;

    Move child_best_move;
    i32 score = -search(&npos, depth - 1, -beta, -alpha, &child_best_move);
    if (score > best_score) {
      best_score = score;
      memcpy(best_move, &moves[move_index], sizeof(Move));

      if (score > alpha) {
        alpha = score;

        if (score >= beta) {
          break;
        }
      }
    }
  }

  if (moves_evaluated == 0) {
    if (is_attacked(pos, lsb(pos->colour[0] & pos->pieces[King]), true)) {
      return -mate;
    } else {
      return 0;
    }
  }

  return best_score;
}

static void iteratively_deepen(Position *pos, size_t total_time) {
  size_t start_time = get_time();
  Move best_move;
  for (i32 depth = 1; depth < 128; depth++) {
    i32 score = search(pos, depth, -inf, inf, &best_move);
    size_t elapsed = get_time() - start_time;

#if FULL
    char info_move_name[256];
    move_str(info_move_name, &best_move, pos->flipped);
    printf("info depth %i score %i time %i pv ", depth, score, elapsed);
    puts(info_move_name);
    puts("\n");
#endif

    if (elapsed > total_time / 64) {
      break;
    }
  }
  char move_name[256];
  move_str(move_name, &best_move, pos->flipped);
  puts("bestmove ");
  puts(move_name);
  puts("\n");
}

void _start() {
  Position pos;
  Move moves[256];
  i32 num_moves;

#if FULL
  pos = (Position){.castling = {true, true, true, true},
                   .colour = {0xFFFFull, 0xFFFF000000000000ull},
                   .pieces = {0xFF00000000FF00ull, 0x4200000000000042ull,
                              0x2400000000000024ull, 0x8100000000000081ull,
                              0x800000000000008ull, 0x1000000000000010ull},
                   .ep = 0};
#endif

  // UCI loop
  while (true) {
    char move_name[256];
    char line[256];
    getw(line);
    if (!strcmp(line, "uci")) {
      puts("id name 4k.c\nid author Gediminas Masaitis\nuciok\n");
    } else if (!strcmp(line, "isready")) {
      puts("readyok\n");
    } else if (!strcmp(line, "position")) {
      pos = (Position){.castling = {true, true, true, true},
                       .colour = {0xFFFFull, 0xFFFF000000000000ull},
                       .pieces = {0xFF00000000FF00ull, 0x4200000000000042ull,
                                  0x2400000000000024ull, 0x8100000000000081ull,
                                  0x800000000000008ull, 0x1000000000000010ull},
                       .ep = 0};

      while (true) {
        bool line_continue = getw(line);
        num_moves = movegen(&pos, moves, false);
        for (i32 i = 0; i < num_moves; i++) {
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
    } else if (!strcmp(line, "go")) {
      getw(line); // wtime
      getw(line); // wtime value
      if (pos.flipped) {
        getw(line); // winc
        getw(line); // winc value
        getw(line); // btime
        getw(line); // btime value
      }
      size_t total_time = stoi(line);
      iteratively_deepen(&pos, total_time);
    }
#if FULL
    // go infinite
    else if (!strcmp(line, "gi")) {
      iteratively_deepen(&pos, 99999999999);
    }
#endif
#if FULL
    else if (!strcmp(line, "perft")) {
      char depth_str[4];
      getw(depth_str);
      const i32 depth = stoi(depth_str);
      const u64 start = get_time();
      const u64 nodes = perft(&pos, depth);
      const u64 end = get_time();
      const i32 elapsed = end - start;
      const u64 nps = elapsed ? 1000 * nodes / elapsed : 0;
      printf("info depth %i nodes %i time %i nps %i \n", depth, nodes,
             end - start);
    }
#endif
  }

  // Exit
  _sys(1, 0, 0, 0);
}

#pragma endregion
