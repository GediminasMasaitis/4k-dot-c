// 4k.c - 4KB ELF chess engine by Gediminas Masaitis

// Minify thing thang




#pragma region libc shims

#ifdef _MSC_VER
#define __attribute__(...)
#endif

#if defined(_WIN64) || defined(__x86_64__) || defined(__ppc64__) || defined(__aarch64__)
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

// Types for aliasing-safe type punning (exempt from TBAA)
typedef u32 __attribute__((may_alias)) u32a;
typedef u64 __attribute__((may_alias)) u64a;

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

 static void exit_now() {
      asm volatile("syscall" : : "a"(60));
      __builtin_unreachable();
    }

 [[nodiscard]] static u32 atoi(const char *restrict string) {
      u32 result = 0;
      while (*string)
        result = result * 10 + *string++ - '0';
      return result;
    }


    typedef struct [[nodiscard]] {
      ssize_t tv_sec;  // seconds
      ssize_t tv_nsec; // nanoseconds
    } timespec;

    [[nodiscard]] static u64 get_time() {
      timespec ts;
      ssize_t ret; // Unused
      asm volatile("syscall" : "=a"(ret) : "0"(228), "D"(1), "S"(&ts) : "rcx", "r11", "memory");
      return  ts.tv_nsec +   ts.tv_sec *  1000 * 1000 * 1000ULL;
    }


     void putl(const char *const restrict string) {
      i32 length = 0;
      while (string[length]) {
        ssize_t ret;
        asm volatile("syscall" : "=a"(ret) : "0"(1), "D"(stdout), "S"(&string[length]), "d"(1) : "rcx", "r11", "memory");
        length++;
      }
    }

    static void puts(const char *const restrict string) {
      putl(string);
      putl("\n");
    }

 [[nodiscard]] static bool strcmp(const char *restrict lhs, const char *restrict rhs) {
      while (*lhs || *rhs) {
        if (*lhs != *rhs) {
          return true;
        }
        lhs++;
        rhs++;
      }
      return false;
    }

 // Non-standard, gets but a word instead of a line
     bool getl(char *restrict string) {
      while (true) {
        ssize_t result;
        asm volatile("syscall" : "=a"(result) : "0"(0), "D"(stdin), "S"(string), "d"(1) : "rcx", "r11", "memory");

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
    }

#ifdef FULL
static void *mmap_anon(size_t length) {
  void *p;
  register u64 r10 asm("r10") = 0x22;  // MAP_PRIVATE | MAP_ANONYMOUS
  register u64 r8 asm("r8") = (u64)-1; // fd
  register u64 r9 asm("r9") = 0;       // offset
  asm volatile("syscall" : "=a"(p) : "0"(9), "D"((void *)0), "S"(length), "d"(3), "r"(r10), "r"(r8), "r"(r9) : "rcx", "r11", "memory");
  return p;
}

static void munmap_sys(void *addr, size_t length) {
  ssize_t ret;
  asm volatile("syscall" : "=a"(ret) : "0"(11), "D"(addr), "S"(length) : "rcx", "r11", "memory");
}

static void *malloc(size_t size) {
  size_t total = size + 16;
  size_t *base = mmap_anon(total);
  base[0] = total;
  return (char *)base + 16;
}

static void free(void *ptr) {
  if (!ptr)
    return;
  size_t *base = (size_t *)((char *)ptr - 16);
  munmap_sys(base, *base);
}

static void *calloc(size_t n, size_t sz) { return malloc(n * sz); }

static void *memset(void *dst, int c, size_t n) {
  void *ret = dst;
  asm volatile("rep stosb" : "+D"(dst), "+c"(n) : "a"((unsigned char)c) : "memory");
  return ret;
}

static void *memcpy(void *dst, const void *src, size_t n) {
  void *ret = dst;
  asm volatile("rep movsb" : "+D"(dst), "+S"(src), "+c"(n) : : "memory");
  return ret;
}

static void print_u64(u64 val) {
  char buf[24];
  i32 i = 23;
  buf[i--] = 0;
  if (!val)
    buf[i--] = '0';
  else
    while (val) {
      buf[i--] = '0' + val % 10;
      val /= 10;
    }
  putl(&buf[i + 1]);
}

static void print_i32(i32 val) {
  if (val < 0) {
    putl("-");
    val = -val;
  }
  print_u64((u64)(u32)val);
}

static void printf(const char *fmt, ...) {
  __builtin_va_list ap;
  __builtin_va_start(ap, fmt);
  char ch[2] = {0, 0};
  while (*fmt) {
    if (*fmt != '%') {
      ch[0] = *fmt++;
      putl(ch);
      continue;
    }
    fmt++;
    if (*fmt == 'i' || *fmt == 'd') {
      print_i32(__builtin_va_arg(ap, i32));
      fmt++;
    } else if (fmt[0] == 'l' && fmt[1] == 'l' && fmt[2] == 'u') {
      print_u64(__builtin_va_arg(ap, u64));
      fmt += 3;
    } else {
      ch[0] = '%';
      putl(ch);
      ch[0] = *fmt++;
      putl(ch);
    }
  }
  __builtin_va_end(ap);
}
#endif

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

static bool getl(char *restrict string) {
  while (true) {

    const i32 c = getchar();

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
   u8 from; u8 to;
   u8 takes_piece;
   u8 promo;
} Move;

typedef struct [[nodiscard]] {
   u64 pieces[7];
   u64 ep;
   u64 colour[2];
   u8 padding[11];
   union {
        bool castling[4];
        u32 castling32;
      };
   bool flipped;
} Position;

#ifdef ASSERTS
#define assert(condition)                                                                                                                                      \
  if (!(condition)) {                                                                                                                                          \
    printf("Assert failed on line %i: ", __LINE__);                                                                                                            \
    puts(#condition);                                                                                                                                          \
    exit_now();                                                                                                                                                \
  }
#else
#define assert(condition)
#endif

 [[nodiscard]] static bool move_string_equal( const char *restrict rhs,  const char *restrict lhs) {
      return ( *(const u64a *)rhs ^  *(const u64a *)lhs) << 24 == 0;
    }

 [[nodiscard]] static u64 flip_bb(const u64 bb) { return __builtin_bswap64(bb); }
 [[nodiscard]] static i32 lsb(u64 bb) { return __builtin_ctzll(bb); }

 [[nodiscard]] static u64 shift( const i32 shift,  const u64 mask,  const u64 bb) {
      return shift > 0 ?  bb << shift &  mask :  bb >> -shift &  mask;
    }

 [[nodiscard]] static i64 count(const u64 bb) { return __builtin_popcountll(bb); }

 [[nodiscard]] static u64 west(const u64 bb) { return  bb >> 1 &  ~0x8080808080808080ull; }

 [[nodiscard]] static u64 north(const u64 bb) { return bb << 8; }

 [[nodiscard]] static u64 south(const u64 bb) { return bb >> 8; }

 [[nodiscard]] static u64 east(const u64 bb) { return  bb << 1 &  ~0x101010101010101ull; }

 [[nodiscard]] static u64 southeast(const u64 bb) {
      return  shift( -7,  ~0x101010101010101ull,  bb);
      return   east( south(bb));
    }

 [[nodiscard]] static u64 northeast(const u64 bb) {
      return   east( north(bb));
      return  shift( 9,  ~0x101010101010101ull,  bb);
    }

 [[nodiscard]] static u64 northwest(const u64 bb) {
      return  shift( 7,  ~0x8080808080808080ull,  bb);
      return   west( north(bb));
    }

 [[nodiscard]] static u64 southwest(const u64 bb) {
      return   west( south(bb));
      return  shift( -9,  ~0x8080808080808080ull,  bb);
    }

 static u64 diag_mask[64];

 [[nodiscard]] static
            u64 ray( const u64 blockers,  const u64 mask,  const u64 bb,  const i32 shift_by) {
              u64 result = shift( shift_by,  mask,  bb);
              for (i64 i = 5; i >= 0; i--) {
                result |= shift( shift_by,  mask,  result & ~blockers);
              }
              return result;
            }

 [[nodiscard]]  u64 xattack( const u64 blockers,  const u64 bb,  const u64 dir_mask) {
      return  dir_mask &
              ( ( blockers &  dir_mask) - bb ^  flip_bb(flip_bb( blockers &  dir_mask) - flip_bb(bb)));
    }

 [[nodiscard]]  u64 king(const u64 bb) {
      const u64 vertical =  north(bb) |  south(bb);
      const u64 vertical_inclusive =  bb |  vertical;
      return  vertical |  east(vertical_inclusive) |  west(vertical_inclusive);
    }

 [[nodiscard]]  u64 knight(const u64 bb) {
       const u64 east_bb = east(bb);
       const u64 west_bb = west(bb);
       const u64 horizontal1 =  west_bb |  east_bb;
       const u64 horizontal2 =  east(east_bb) |  west(west_bb);
      return  horizontal2 << 8 |  horizontal2 >> 8 |  horizontal1 >> 16 |  horizontal1 << 16;
    }

 [[nodiscard]]  u64 rook( const u64 blockers,  const u64 bb) {
      assert(count(bb) == 1);
      return  // West
               ray( blockers,  ~0x8080808080808080ull,  bb,  -1) |
              // East
               ray( blockers,  ~0x101010101010101ull,  bb,  1) |
              xattack( blockers,  bb,  bb ^ 0x101010101010101ULL << lsb(bb) % 8);
    }

 [[nodiscard]]  u64 bishop( const u64 blockers,  const u64 bb) {
      assert(count(bb) == 1);
      const i32 sq = lsb(bb);
      return  xattack( blockers,  bb,  diag_mask[sq]) |
              xattack( blockers,  bb,  flip_bb(diag_mask[ sq ^  56]));
    }


    static void swapu32( u32a *const rhs,  u32a *const lhs) {
      const u32 temp = *lhs;
      *lhs = *rhs;
      *rhs = temp;
    }

    static void swapmoves( Move *const rhs,  Move *const lhs) { swapu32( (u32a *)rhs,  (u32a *)lhs); }

 static void move_str( const i32 flip,  const Move *restrict move,
                            char *restrict str) {
      assert(move->from >= 0);
      assert(move->from < 64);
      assert(move->to >= 0);
      assert(move->to < 64);
      assert(move->from != move->to);
      assert(move->promo == None || move->promo == Knight || move->promo == Bishop || move->promo == Rook || move->promo == Queen);

       str[4] = "\0\0nbrq"[move->promo];

       // Hack to save bytes, technically UB but works on GCC 14.2
          for (i32 i = 0; i < 2; i++) {
             str[i * 2 + 1] = '1' + ((&move->from)[i] / 8 ^ 7 * flip);
             str[i * 2] = 'a' + (&move->from)[i] % 8;
          }
       str[5] = '\0';
    }

 [[nodiscard]] static bool move_equal( Move *const rhs,  Move *const lhs) { return  *(u32a *)lhs ==  *(u32a *)rhs; }

 [[nodiscard]]  i32
            piece_on( const Position *const restrict pos,  const i32 sq) {
              assert(sq >= 0);
              assert(sq < 64);
              for (u32 i = Pawn; i <= King; ++i) {
                if ( pos->pieces[i] &  1ull << sq) {
                  return i;
                }
              }
              return None;
            }

 [[nodiscard]] static i32 is_attacked( const Position *const restrict pos,  const u64 bb) {
      assert(count(bb) == 1);
      const u64 theirs = pos->colour[1];
       const u64 pawns = theirs & pos->pieces[Pawn];
       const u64 blockers = theirs | pos->colour[0];
      return   ( southwest(pawns) |  southeast(pawns)) &  bb ||
               rook( blockers,  bb) &  (pos->pieces[Rook] | pos->pieces[Queen]) &  theirs ||
               bishop( blockers,  bb) &  theirs &  (pos->pieces[Bishop] | pos->pieces[Queen]) ||
               pos->pieces[King] &  king(bb) &  theirs ||   pos->pieces[Knight] &  knight(bb) &  theirs;
    }

 [[nodiscard]] static u64 get_mobility( const Position *pos,  const i64 piece,
                                             const i64 sq) {
      u64 moves = 0;
      const u64 bb = 1ULL << sq;
       if (piece == King) { moves = king(bb); }
      else  if (piece == Knight) { moves = knight(bb); } else {
        const u64 blockers =  pos->colour[1] |  pos->colour[0];
         if (piece != Bishop) { moves |= rook( blockers,  bb); }
         if (piece != Rook) { moves |= bishop( blockers,  bb); }
      }
      return moves;
    }

  void flip_pos(Position *const restrict pos) {
       // Hack to flip the first 10 bitboards in Position.
          u64a *pos_ptr = (u64a *)pos;
          for (i32 i = 0; i < 10; i++) { pos_ptr[i] = flip_bb(pos_ptr[i]); }
       pos->colour[0] ^= pos->colour[1]; pos->colour[1] ^= pos->colour[0]; pos->colour[0] ^= pos->colour[1];

       pos->castling32 =  (pos->castling32 >> 16) |  (pos->castling32 << 16);
       pos->flipped ^= 1;
    }

 i32 find_in_check(const Position *restrict pos) {
  return is_attacked( pos,   pos->colour[0] &  pos->pieces[King]);
}

 static Move *generate_piece_moves( const u64 to_mask,  Move *restrict movelist,  const Position *restrict pos) {
      for (u32 piece = Knight; piece <= King; piece++) {
        assert(piece == Knight || piece == Bishop || piece == Rook || piece == Queen || piece == King);
        u64 copy =  pos->colour[0] &  pos->pieces[piece];
        while (copy) {
          const i8 from = lsb(copy);
          assert(from >= 0);
          assert(from < 64);
           u64 moves =  to_mask &  get_mobility( pos,  piece,  from);

           copy &= copy - 1;

          while (moves) {
            const u8 to = lsb(moves);
            assert(to >= 0);
            assert(to < 64);

             moves &= moves - 1;
             *movelist++ = ((Move){.from = from, .to = to, .promo = None, .takes_piece = piece_on( pos,  to)});
          }
        }
      }

      return movelist;
    }

  u32 makemove( Position *const restrict pos,  const Move *const restrict move) {
      assert(move->from >= 0);
      assert(move->from < 64);
      assert(move->to >= 0);
      assert(move->to < 64);
      assert(move->from != move->to);
      assert(move->promo == None || move->promo == Knight || move->promo == Bishop || move->promo == Rook || move->promo == Queen);
      assert(move->takes_piece != King);
      assert(move->takes_piece == piece_on( pos,  move->to));

       const u64 from = 1ull << move->from;
       const u64 to = 1ull << move->to;
       const i32 piece = piece_on( pos,  move->from); assert(piece != None);
       const u64 mask =  from |  to;
       const u64 south_to = south(to);

       // Captures
          if (move->takes_piece != None) {
             pos->colour[1] ^= to;
             pos->pieces[move->takes_piece] ^= to;
          }

       // Castling
          if (piece == King) {
            const u64 bb = move->to - move->from == 2 ? 0xa0 : move->from - move->to == 2 ? 0x9 : 0;
             pos->pieces[Rook] ^= bb;
             pos->colour[0] ^= bb;
          }

      // Move the piece
       pos->colour[0] ^= mask;
       pos->pieces[piece] ^= mask;

      // En passant
      if ( piece == Pawn &&  to == pos->ep) {
         pos->pieces[Pawn] ^= south_to;
         pos->colour[1] ^= south_to;
      }
      pos->ep = 0;

       // Pawn double move
          if ( move->to - move->from == 16 &&  piece == Pawn) { pos->ep = south_to; }

       // Promotions
          if (move->promo != None) {
             pos->pieces[Pawn] ^= to;
             pos->pieces[move->promo] ^= to;
          }

       // Update castling permissions
        const u64 oppMask = mask >> 56;
         pos->castling[1] &= !(mask & 0x11);  pos->castling[3] &= !(oppMask & 0x11);  pos->castling[2] &= !(oppMask & 0x90);
             pos->castling[0] &= !(mask & 0x90);

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
    }

  Move *generate_pawn_moves( u64 to_mask,  const Position *const pos,  Move *restrict movelist,
                                        const i64 offset) {
      while (to_mask) {
        const u8 to = lsb(to_mask);
        to_mask &= to_mask - 1;
        const u8 from =  to +  offset;
        assert(from >= 0);
        assert(from < 64);
        assert(to >= 0);
        assert(to < 64);
        assert(piece_on( pos,  from) == Pawn);
        const u8 takes = piece_on( pos,  to);
        if (to > 55) {
          for (i64 piece = Queen; piece >= Knight; piece--) {
            *movelist++ = ((Move){.from = from, .to = to, .promo = piece, .takes_piece = takes});
          }
        } else {
          *movelist++ = ((Move){.from = from, .to = to, .promo = None, .takes_piece = takes});
        }
      }

      return movelist;
    }

enum { max_moves = 218 };

[[nodiscard]]  i32
    movegen( const Position *const restrict pos,  const i64 only_captures,
             Move *restrict movelist) {

   const u64 all =  pos->colour[1] |  pos->colour[0];
   const Move *start = movelist;
   const u64 to_mask = only_captures ? pos->colour[1] : ~pos->colour[0];
   // PAWN PROMOTIONS
      if (!only_captures) {
        movelist = generate_pawn_moves(
              north( north( pos->colour[0] &  pos->pieces[Pawn] &  0xFF00) &  ~all) &  ~all,
             pos,  movelist,  -16);
      }
   // PAWN EAST CAPTURES
    movelist = generate_pawn_moves(
          northeast( pos->colour[0] &  pos->pieces[Pawn]) &  ( pos->colour[1] |  pos->ep),  pos,
         movelist,  -9);
   // PAWN DOUBLE MOVES
    movelist = generate_pawn_moves(
         north( pos->colour[0] &  pos->pieces[Pawn]) &  ~all &  (only_captures ? 0xFF00000000000000ull : ~0ull),
         pos,  movelist,  -8);
   // LONG CASTLE
      if ( !only_captures &&  !( all &  0xEull) &&  pos->castling[1] &&
           !is_attacked( pos,  1ULL << 3) &&
           !is_attacked( pos,  1ULL << 4)) { *movelist++ = (Move){.from = 4, .to = 2, .promo = None, .takes_piece = None}; }
   // SHORT CASTLE
      if ( !only_captures &&  !( all &  0x60ull) &&  pos->castling[0] &&
           !is_attacked( pos,  1ULL << 5) &&
           !is_attacked( pos,  1ULL << 4)) { *movelist++ = (Move){.from = 4, .to = 6, .promo = None, .takes_piece = None}; }
   // PAWN WEST CAPTURES
    movelist = generate_pawn_moves(
          northwest( pos->colour[0] &  pos->pieces[Pawn]) &  ( pos->colour[1] |  pos->ep),  pos,
         movelist,  -7);
  movelist = generate_piece_moves( to_mask,  movelist,  pos);

  const i32 num_moves = movelist - start;
  assert(num_moves < max_moves);
  return num_moves;
}

#pragma endregion

#pragma region engine

[[nodiscard]] static u64 perft(const Position *const restrict pos, const i32 depth) {
  if (depth == 0) {
    return 1;
  }

  u64 nodes = 0;
  Move moves[max_moves];
  const i32 num_moves = movegen( pos,  false,  moves);

  for (i32 i = 0; i < num_moves; ++i) {
    Position npos = *pos;

    // Check move legality
    if (!makemove( &npos,  &moves[i])) {
      continue;
    }

    nodes += perft(&npos, depth - 1);
  }

  return nodes;
}

static bool get_fen(Position *restrict pos, char *restrict fen) {
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
  bool more = getl(fen);
  p = fen;
  if (*p != '-') {
    const i32 file = p[0] - 'a';
    const i32 rank = p[1] - '1';
    pos->ep = 1ull << (rank * 8 + file);
  }

  if (black_to_move) {
    flip_pos(pos);
  }
  return more;
}

typedef struct [[nodiscard]] __attribute__((packed)) {
  i16 material[6];
  
     i8 king_shield[2];  i8 piece_threats[2];  i8 pawn_threat[5];  i8 passed_king_distance[2];
         i8 bishop_pawns[2];
  
     i8 open_files[12];  i8 mobilities[5];  i8 passed_blocked_pawns[6];  i8 pst_file[48];
         i8 pawn_attacked_penalty[2];  i8 tempo;
  
     i8 passed_pawns[6];  i8 bishop_pair;  i8 phalanx_pawn[6];  i8 protected_pawn[6];  i8 pst_rank[48];
         i8 king_attacks[5];
} EvalParams;

typedef struct [[nodiscard]] __attribute__((packed)) {
  i32 material[6];
  
     i32 king_shield[2];  i32 piece_threats[2];  i32 pawn_threat[5];  i32 passed_king_distance[2];
         i32 bishop_pawns[2];
  
     i32 open_files[12];  i32 mobilities[5];  i32 passed_blocked_pawns[6];  i32 pst_file[48];
         i32 pawn_attacked_penalty[2];  i32 tempo;
  
     i32 passed_pawns[6];  i32 bishop_pair;  i32 phalanx_pawn[6];  i32 protected_pawn[6];  i32 pst_rank[48];
         i32 king_attacks[5];
} EvalParamsMerged;

typedef struct [[nodiscard]] __attribute__((packed)) {
   i8 phases[6];
   EvalParams eg;
   EvalParams mg;
} EvalParamsInitial;

  EvalParamsMerged eval_params;

 // EVAL PARAMETERS
    __attribute__((aligned(8))) static const EvalParamsInitial initial_params = {
.phases = {0, 0, 1, 1, 2, 4},
.mg = {.material = {0, 68, 292, 330, 383, 841},
.pst_rank = {0, -15, -18, -4, 6, 30, 126, 0, // Pawn
-20, -5, 9, 23, 31, 35, 10, -84, // Knight
-8, 11, 16, 19, 17, 16, -11, -59, // Bishop
7, -2, -8, -11, 4, 13, -2, 0, // Rook
14, 17, 9, 2, -3, -3, -21, -14, // Queen
-8, 1, -10, -23, -6, 38, 44, 45, // King
},
.pst_file = {-7, -11, -8, 1, 6, 12, 12, -6, // Pawn
-30, -11, -1, 12, 14, 14, 7, -5, // Knight
-14, 0, 3, 1, 4, -1, 10, -4, // Bishop
-12, -7, -2, 7, 13, 8, 6, -12, // Rook
-15, -8, -4, 0, 3, 4, 13, 6, // Queen
-10, 16, -11, -28, -28, -11, 11, 2, // King
},
.mobilities = {6, 5, 2, 3, -2},
.king_attacks = {0, 13, 19, 14, 0},
.pawn_threat = {-16, -8, -8, -8, -1},
.open_files = {8, -11, -11, 17, -2, -22, 8, -9, -10, 33, -11, -57},
.passed_pawns = {-9, -2, 9, 41, 76, 126},
.passed_blocked_pawns = {-1, -5, -4, 3, 3, -48},
.passed_king_distance = {-1, -2},
.protected_pawn = {0, 21, 12, 19, 32, 127},
.phalanx_pawn = {7, 12, 20, 37, 127, 127},
.bishop_pair = 27,
.bishop_pawns = {-5, -5},
.king_shield = {29, 18},
.pawn_attacked_penalty = {-57, -114},
.piece_threats = {26, 24, 17, -1, -23},
.tempo = 30,
},
.eg = {.material = {0, 86, 407, 444, 740, 1370},
.pst_rank = {0, 0, -7, -6, -1, 14, 94, 0, // Pawn
-29, -17, -7, 15, 21, 7, 4, 7, // Knight
-12, -11, -1, 3, 6, 2, 5, 7, // Bishop
-17, -18, -10, 4, 10, 9, 17, 5, // Rook
-53, -38, -11, 13, 27, 23, 27, 12, // Queen
-36, -3, 4, 18, 31, 31, 19, -39, // King
},
.pst_file = {2, 7, -1, -7, -1, 1, 1, -3, // Pawn
-19, -5, 8, 17, 17, 4, -2, -21, // Knight
-9, -1, 4, 10, 11, 4, -5, -15, // Bishop
1, 5, 5, -3, -6, 0, 0, -3, // Rook
-21, -7, 4, 8, 15, 13, -2, -9, // Queen
-30, -1, 11, 17, 19, 14, 4, -30, // King
},
.mobilities = {5, 3, 4, 2, -4},
.king_attacks = {0, -1, -6, 9, 0},
.pawn_threat = {-3, -5, -17, -12, -7},
.open_files = {7, 3, 18, 9, 25, 8, 23, -7, 3, 12, 40, 7},
.passed_pawns = {14, 3, 20, 47, 101, 94},
.passed_blocked_pawns = {-12, -3, -13, -30, -68, -70},
.passed_king_distance = {-5, 9},
.protected_pawn = {0, 16, 15, 19, 46, 22},
.phalanx_pawn = {5, 15, 26, 82, 127, 127},
.bishop_pair = 57,
.bishop_pawns = {-11, -1},
.king_shield = {-10, -7},
.pawn_attacked_penalty = {-26, -128},
.piece_threats = {34, 37, 24, 28, 31},
.tempo = 18,
} };

 [[nodiscard]] static i64 combine_eval_param( const i32 mg_val,  const i64 eg_val) {
      return  mg_val +  (eg_val << 16);
    }

 i32 eval(Position *const restrict pos) {
   i32 score = eval_params.tempo;
   i32 phase = 0;

  for (i32 c = 0; c < 2; c++) {

     // BISHOP PAIR
        if (count( pos->pieces[Bishop] &  pos->colour[0]) > 1) { score += eval_params.bishop_pair; }

     u64 pawns[2]; for (i8 i = 0; i < 2; i++) {
          pawns[i] =  pos->colour[i] &  pos->pieces[Pawn];
        } const u64 attacked_by_pawns =  southwest(pawns[1]) |  southeast(pawns[1]);
         const u64 no_passers =  pawns[1] |  attacked_by_pawns;
     const u64 opp_king_zone = king( pos->pieces[King] &  pos->colour[1]);
    for (i32 p = Pawn; p <= King; p++) {
      u64 copy =  pos->colour[0] &  pos->pieces[p];
      while (copy) {
        const i64 sq = lsb(copy);
         const u64 piece_bb = 1ULL << sq;
         const i32 file =  sq &  7;
         phase += initial_params.phases[p];
         copy &= copy - 1;
         const i32 rank = sq >> 3;
         const u64 in_front = 0x101010101010101ULL << sq;
          // SPLIT PIECE-SQUARE TABLES FOR RANK
                score += eval_params.pst_rank[  (p - 1) *  8 +  rank];

                   // SPLIT PIECE-SQUARE TABLES FOR FILE
                    score += eval_params.pst_file[  (p - 1) *  8 +  file];

          // PROTECTED PAWNS
                  if ( p == Pawn &&   piece_bb &  ( northwest(pawns[0]) |  northeast(pawns[0]))) {
                    score += eval_params.protected_pawn[rank - 1];
                  }

                   // PHALANX PAWNS
                      if ( p == Pawn &&   piece_bb &  west(pawns[0])) { score += eval_params.phalanx_pawn[rank - 1]; }

         // PASSED PAWNS
            if ( p == Pawn &&  !( in_front &  no_passers)) {
               if ( north(piece_bb) &  pos->colour[1]) { score += eval_params.passed_blocked_pawns[rank - 1]; }

               score += eval_params.passed_pawns[rank - 1];

              // PASSED PAWN KING DISTANCE
              for (i32 i = 0; i < 2; i++) {
                const i64 king_sq = lsb( pos->colour[i] &  pos->pieces[King]);
                 const i32 rank_distance = __builtin_abs(king_sq / 8 -  1 -  rank);
                 const i32 file_distance = __builtin_abs(king_sq % 8 - file);
                score +=
                     (rank_distance > file_distance ? rank_distance : file_distance) *  (rank - 1) *  eval_params.passed_king_distance[i];
              }
            }

         // OPEN FILES / DOUBLED PAWNS
            if (( north(in_front) &  pawns[0]) == 0) {
              score += eval_params.open_files[  !( in_front &  pawns[1]) *  6 +  -1 +  p];
            }

         if (p > Pawn) {
               // PAWN PUSH THREATS
                  if ( in_front &  ~piece_bb &  attacked_by_pawns) { score += eval_params.pawn_threat[p - 2]; }

               // KING SHIELD
                  if (  p ==  King &&   piece_bb &  0xC3D7) {
                    const u64 shield = file < 3 ? 0x700 : 0xE000;
                     score +=  count( shield &  pawns[0]) *  eval_params.king_shield[0];
                     score +=  count( north(shield) &  pawns[0]) *  eval_params.king_shield[1];
                  }

               // PIECES ATTACKED BY PAWNS
                  if ( piece_bb &  no_passers) { score += eval_params.pawn_attacked_penalty[c]; }

               // BISHOP COLOUR PAWNS
                  if ( p ==  Bishop) {
                    u64 mask = 0xAA55AA55AA55AA55ULL;
                    if (!( piece_bb &  mask)) {
                      mask = ~mask;
                    }
                    for (u8 i = 0; i < 2; i++) {
                      score +=  eval_params.bishop_pawns[i] *  count( pawns[i] &  mask);
                    }
                  }

               const u64 mobility = get_mobility( pos,  p,  sq);

                 // PIECE THREATS
                    if (p < Queen) {
                      score +=  count( mobility &  pos->colour[1] & ~( pos->pieces[Pawn] |  attacked_by_pawns)) *
                                eval_params.piece_threats[ p ==  Rook];
                    }

                     // MOBILITY
                      score +=  count( ~pos->colour[0] &  mobility &  ~attacked_by_pawns) *  eval_params.mobilities[p - 2];

                         // KING ATTACKS
                          score +=  count( mobility &  opp_king_zone) *  eval_params.king_attacks[p - 2];
            }

         // MATERIAL
          score += eval_params.material[p];
      }
    }

     score = -score;
     flip_pos(pos);
  }

  const i16 stronger_side_pawns_missing = 8 - count( pos->colour[score < 0] &  pos->pieces[Pawn]);
  return (  (i16)score *  phase +
            ((score + 0x8000) >> 16) *  (128 - stronger_side_pawns_missing * stronger_side_pawns_missing) / 128 * (24 - phase)) /
         24;
}

#ifdef FULL
u64 tt_length = 1 << 23; // 80MB
#else
enum : u64 { tt_length = 1ULL << 23 }; // 80MB
// enum : u64 { tt_length = 1ULL << 27 }; // 1.25GB
// enum : u64 { tt_length = 1ULL << 29 }; // 5GB
// enum : u64 { tt_length = 1ULL << 31 }; // 20GB
#endif
enum { Upper = 0, Lower = 1, Exact = 2 };
enum { max_ply = 96 };
enum { mate = 31744, inf = 32256 };
#ifdef NOSTDLIB
enum { thread_count = 1 };
#else
static i32 thread_count = 1;
#endif
enum { thread_stack_size = 1024 * 1024 };
enum { corrhist_size = 65536 };

typedef struct [[nodiscard]] {
   Move best_move;
   Move prev_move;
   u64 position_hash;
   i32 num_moves;
   Move killer;
   i32 static_eval;
} SearchStack;

typedef struct [[nodiscard]] __attribute__((packed)) {
   i8 depth;
   Move move;
   i16 static_eval;
   u8 flag;
   u16 partial_hash;
   i16 score;
} TTEntry;
_Static_assert(sizeof(TTEntry) == 12);

typedef struct [[nodiscard]] {
#ifdef FULL
  i32 thread_id;
  u64 nodes;
#endif
   Position pos;
   u64 max_time;
   Move counter_moves[2][64][64];
   SearchStack stack[1024];
   i32 corrhist[corrhist_size];
   i32 move_history[2][6][64][64];
} ThreadData;

typedef struct __attribute__((aligned(16))) ThreadHeadStruct {
  void (*entry)(struct ThreadHeadStruct *);
  ThreadData data;
} ThreadHead;

#ifdef FULL
static ThreadData *main_data;
static TTEntry *tt;
static u64 pv_hist[256];
static i32 pv_hist_len;
#ifdef NOSTDLIB
__attribute__((aligned(4096))) u8 thread_stacks[thread_count][thread_stack_size];
#endif
#else
__attribute__((aligned(4096))) u8 thread_stacks[thread_count][thread_stack_size];
__attribute__((section(".bss.zztt")))  TTEntry tt[tt_length];
#endif
 static u64 start_time;
 static volatile bool stop;

#if defined(__x86_64__) || defined(_M_X64)
typedef long long __attribute__((__vector_size__(16))) i128;

[[nodiscard]] __attribute__((target("aes"))) static u64 get_hash(const Position *const pos) {
  i128 hash = {0};

  // USE 16 BYTE POSITION SEGMENTS AS KEYS FOR AES
  const u8 *const data = (const u8 *)pos;
  for (u8 i = 0; i < 6; i++) {
    i128 key;
    __builtin_memcpy(&key, data +  i *  16, 16);
    hash = __builtin_ia32_aesenc128(hash, key);
  }

  // FINAL ROUND FOR BIT MIXING
  hash = __builtin_ia32_aesenc128(hash, hash);

  // USE FIRST 64 BITS AS POSITION HASH
  return hash[0];
}
#elif defined(__aarch64__)

#include <arm_neon.h>

#ifdef __clang__
[[nodiscard]] __attribute__((target("+aes"))) u64
#else
[[nodiscard]] __attribute__((target("arch=armv8-a+crypto"))) u64
#endif
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

[[nodiscard]] static u64 get_material_hash(const Position *const pos) {
  u64 hash = 0;
  for (i32 c = 0; c < 2; c++) {
    for (i32 p = Pawn; p <= Queen; p++) {
      hash =  count( pos->pieces[p] &  pos->colour[c]) +   hash *  9;
    }
  }
  return hash;
}

static
void get_piece_hashes( const Position *const pos,  u64 hashes[4]) {
  for (u64 p = Pawn; p <= Queen; p++) {
    hashes[p / 2] ^= ( pos->pieces[p] *  0x9E3779B97F4A7C15ULL) >> 48;
  }
}

static
i32
search(
#ifdef FULL
    u64 *nodes,
#endif
      ThreadData *data,  const i32 beta,  Position *const pos,  i32 depth,
      const i32 ply,  i32 alpha,  const bool do_null) {
  assert(alpha < beta);
  assert(ply >= 0);

  SearchStack *const stack = data->stack;
  SearchStack *const ss = stack + ply;
   i32(*const move_history)[6][64][64] = data->move_history;
   Move counter = data->counter_moves[pos->flipped][ss[1].prev_move.from][ss[1].prev_move.to];

   // IN-CHECK EXTENSION
    const bool in_check = find_in_check(pos);
    depth += in_check;

   // FULL REPETITION DETECTION
    const u64 tt_hash = get_hash(pos);
  bool in_qsearch = depth <= 0;
  for (i32 i =  ply;  i >= 0 &&  do_null; i -= 2) {
    if ( tt_hash ==  stack[i].position_hash) {
      return 0;
    }
  }

  // TT PROBING
   ss->best_move = (Move){0};
   TTEntry *tt_entry = &tt[tt_hash % tt_length];
   const u16 tt_hash_partial = tt_hash / tt_length;
  const bool tt_hit =  tt_entry->partial_hash ==  tt_hash_partial;
  if (tt_hit) {
    ss->best_move = tt_entry->move;

    // TT PRUNING
    if (  tt_entry->flag !=  tt_entry->score <= alpha &&  tt_entry->depth >= depth &&   alpha ==  beta - 1) {
      return tt_entry->score;
    }
  } else if (depth > 3) {

    // INTERNAL ITERATIVE REDUCTION
    depth--;
  }

  // STATIC EVAL WITH CORRECTION HISTORY
  u64 corr_hashes[6] = {0};
   i32 * corr_entries[6];
   get_piece_hashes( pos,  corr_hashes);
   corr_hashes[3] = get_material_hash(pos);
   const i32 raw_eval = tt_hit ? tt_entry->static_eval : eval(pos); i32 static_eval = raw_eval;
    assert(static_eval < mate); assert(static_eval > -mate);
   corr_hashes[4] =  ss[1].prev_move.from |  ss[1].prev_move.to << 8;
   corr_hashes[5] =  ( ss->prev_move.from |  ss->prev_move.to << 8) +  16384;
  for (i64 i = 0; i < 6; i++) {
    corr_entries[i] = &data->corrhist[corr_hashes[i] % corrhist_size];
    static_eval += *corr_entries[i] / 256;
    assert(static_eval < mate);
    assert(static_eval > -mate);
  }

  ss->static_eval = static_eval;
  const i8 improving = ply > 1 && static_eval > ss[-2].static_eval;
  if ( tt_hit &&   tt_entry->flag !=  static_eval > tt_entry->score) {
    static_eval = tt_entry->score;
  }

  // QUIESCENCE
  if ( static_eval > alpha &&  in_qsearch) {
    if (static_eval >= beta) {
      return static_eval;
    }
    alpha = static_eval;
  }

  if ( !in_check &&   alpha ==  beta - 1) {
    if ( depth < 10 &&  !in_qsearch) {

       {
        // REVERSE FUTILITY PRUNING
        if (static_eval -  32 *  (depth - improving) >= beta) {
          return static_eval;
        }
      }

       // RAZORING
        in_qsearch =  static_eval +   83 *  depth <= alpha;
    }

    // NULL MOVE PRUNING
    if ( depth > 2 &&  do_null &&  static_eval >= beta) {
      Position npos = *pos;
       flip_pos(&npos);
       npos.ep = 0;
      const i64 score = -search(
#ifdef FULL
          nodes,
#endif
            data,  -alpha,  &npos,  depth -  depth / 4 -  4,
            ply + 1,  -beta,  false);
      if (score >= beta) {
        return score;
      }
    }
  }

   Move moves[max_moves]; ss->num_moves = movegen( pos,  in_qsearch,  moves);
   i32 best_score = in_qsearch ? static_eval : -inf;
    ss[ 2].position_hash = tt_hash;
   u8 tt_flag = Upper;
   i32 quiets_evaluated = 0;
   i32 moves_evaluated = 0;

  for (i32 move_index = 0; move_index < ss->num_moves; move_index++) {
    // MOVE ORDERING
     i32 move_score = ~0x1010101LL;
     i32 best_index = 0;
    for (i64 order_index = move_index; order_index < ss->num_moves; order_index++) {
      assert(moves[order_index].takes_piece == piece_on( pos,  moves[order_index].to));
      const i64 order_move_score =  // HISTORY HEURISTIC
                                                move_history[pos->flipped][moves[order_index].takes_piece][moves[order_index].from][moves[order_index].to] +
                                               // MOST VALUABLE VICTIM
                                                 moves[order_index].takes_piece *  545 +
                                               // PREVIOUS BEST MOVE FIRST
                                                (move_equal( &ss->best_move,  &moves[order_index]) << 30) +
                                               // KILLER MOVE
                                                 move_equal( &moves[order_index],  &ss->killer) *  730 +
                                               // COUNTER MOVE
                                                 move_equal( &moves[order_index],  &counter) *  600;
      if (order_move_score > move_score) {
         best_index = order_index;
         move_score = order_move_score;
      }
    }

    swapmoves( &moves[move_index],  &moves[best_index]);

     // FORWARD FUTILITY PRUNING / DELTA PRUNING
        if ( depth < 5 &&
               176 *  depth +  static_eval +  initial_params.eg.material[moves[move_index].promo] +
                            initial_params.eg.material[moves[move_index].takes_piece] <
                       alpha &&
             !in_check &&  moves_evaluated) { break; }

     // MOVE SCORE PRUNING
        if ( moves_evaluated &&  move_score <  -175 *  depth) { break; }

    Position npos = *pos;
#ifdef FULL
    (*nodes)++;
#endif
    if (!makemove( &npos,  &moves[move_index])) {
      continue;
    }

      ss[ 2].prev_move = moves[move_index];

    // PRINCIPAL VARIATION SEARCH
    i32 low = moves_evaluated == 0 ? -beta : -alpha - 1;
    moves_evaluated++;

    // LATE MOVE REDUCTION
    i64
    reduction =  depth > 3 &&  move_score <= 0 ?  depth / 12 +  !improving +  ( alpha ==  beta - 1) +
                                                                    moves_evaluated / 11 +  (move_score / -334)
                                                             : 0;

    i64 score;
    while (true) {
      score = -search(
#ifdef FULL
          nodes,
#endif
            data,  -alpha,  &npos,  depth -  reduction -  1,
            ply + 1,  low,  true);

      // EARLY EXITS
      if ( stop ||  (depth > 4 && get_time() - start_time > data->max_time)) {
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
         tt_flag = Exact;
         ss->best_move = moves[move_index];
         alpha = score;
        if (score >= beta) {
          assert(ss->best_move.takes_piece == piece_on( pos,  ss->best_move.to));
           tt_flag = Lower;
           if (ss->best_move.takes_piece == None) { ss->killer = ss->best_move; }
           // COUNTER MOVE UPDATE
              if (ss->best_move.takes_piece == None) { data->counter_moves[pos->flipped][ss[1].prev_move.from][ss[1].prev_move.to] = ss->best_move; }
           if (!in_qsearch) {
                const i32 bonus = depth * depth;
                 i32 *const this_hist = &move_history[pos->flipped][ss->best_move.takes_piece][ss->best_move.from][ss->best_move.to];

                  *this_hist += bonus -  bonus *  *this_hist / 1024;
                 for (i32 prev_index = 0; prev_index < move_index; prev_index++) {
                      const Move prev = moves[prev_index];
                      i32 *const prev_hist = &move_history[pos->flipped][prev.takes_piece][prev.from][prev.to];
                      *prev_hist -= bonus +  bonus *  *prev_hist / 1024;
                    }
              }
          break;
        }
      }
    }

    if (moves[move_index].takes_piece == None) {
      quiets_evaluated++;
    }

    // LATE MOVE PRUNING
    if ( quiets_evaluated > ( 1 +  depth * depth) >> !improving &&   alpha ==  beta - 1 &&  !in_check) {
      break;
    }
  }

  // MATE / STALEMATE DETECTION
  if ( best_score ==  -inf) {
    return  (ply - mate) *  in_check;
  }

   // UPDATE CORRECTION HISTORY
      if (  tt_flag !=  (best_score < ss->static_eval) &&   ss->best_move.takes_piece ==  None) {
         i32 dd = depth * depth; if (dd > 78) { dd = 78; }
         i32 target = best_score - ss->static_eval;  if (target < -176) { target = -176; }  if (target > 176) { target = 176; }

        for (i64 i = 0; i < 6; i++) {
          *corr_entries[i] = (  target *  256 *  dd +   *corr_entries[i] *  (484 - dd)) / 484;
        }
      }

   // UPDATE TRANSPOSITION TABLE
        *tt_entry =
            (TTEntry){.partial_hash = tt_hash_partial, .move = ss->best_move, .score = best_score, .static_eval = raw_eval, .depth = depth, .flag = tt_flag};

  return best_score;
}

static void init() {
   // INIT DIAGONAL MASKS
      for (i32 sq = 0; sq < 64; sq++) {
        const u64 bb = 1ULL << sq;
         u64 sw_bb = southwest(bb);
         u64 ne_bb = northeast(bb);
        for (i64 i = 6; i > 0; i--) {
           sw_bb |= southwest(sw_bb);
           ne_bb |= northeast(ne_bb);
        }
        diag_mask[sq] =  sw_bb |  ne_bb;
      }
   // MERGE EVAL PARAMS
      for (i64 i = 0; i < sizeof(EvalParamsMerged) / sizeof(i32); i++) {
        ((i32 *)&eval_params)[i] = combine_eval_param( i < 6 ? initial_params.mg.material[i] : ((i8 *)&initial_params.mg)[6 + i],
                                                       i < 6 ? initial_params.eg.material[i] : ((i8 *)&initial_params.eg)[6 + i]);
      }
}

#ifdef FULL
static void print_info(const Position *pos, const i32 depth, const i32 alpha, const i32 beta, const i32 score, const u64 nodes, const Move pv_move,
                       const u64 max_time) {
  // Do not print unfinished iteration scores
  u64 elapsed = get_time() - start_time;
  if (elapsed > max_time) {
    puts("info string hard time limit exceeded");
    return;
  }

  // Only use bound on failed search
  i32 print_score;
  if (score <= alpha) {
    print_score = alpha;
  } else if (score >= beta) {
    print_score = beta;
  } else {
    print_score = score;
  }

  // A bound outside the legal score range carries no information
  if (print_score > mate || print_score < -mate) {
    return;
  }

  printf("info depth %i score ", depth);

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
    move_str( pos->flipped,  &pv_move,  move_name);
    putl(move_name);

    Position cur_pos = *pos;
    if (makemove( &cur_pos,  &pv_move)) {
      u64 seen[max_ply];
      i32 seen_count = 0;
      seen[seen_count++] = get_hash(pos);
      i32 halfmoves = pv_hist_len + 1;
      while (seen_count < max_ply && halfmoves < 100) {
        const u64 hash = get_hash(&cur_pos);

        bool repeat = false;
        for (i32 i = 0; i < pv_hist_len; i++) {
          if (pv_hist[i] == hash) {
            repeat = true;
          }
        }
        for (i32 i = 0; i < seen_count; i++) {
          if (seen[i] == hash) {
            repeat = true;
          }
        }
        if (repeat) {
          break;
        }

        const TTEntry *const entry = &tt[hash % tt_length];
        if (entry->partial_hash != (u16)(hash / tt_length) || entry->flag != Exact) {
          break;
        }

        Move move = entry->move;
        Move moves[max_moves];
        const i32 num_moves = movegen( &cur_pos,  false,  moves);
        i32 move_index = 0;
        while (move_index < num_moves && !move_equal(&move, &moves[move_index])) {
          move_index++;
        }
        if (move_index == num_moves) {
          break;
        }
        Position next_pos = cur_pos;
        if (!makemove( &next_pos,  &move)) {
          break;
        }

        putl(" ");
        move_str( cur_pos.flipped,  &move,  move_name);
        putl(move_name);
        seen[seen_count++] = hash;
        cur_pos = next_pos;
        halfmoves++;
      }
    }
  }

  putl("\n");
}
#endif

static
void iteratively_deepen(
#ifdef FULL
    i32 maxdepth,
#endif
    ThreadData *data) {
  i32 score = -inf;
#ifdef FULL
  for (i32 depth = 1; depth < maxdepth; depth++) {
#else
  for (i32 depth = 1; depth < max_ply; depth++) {
#endif
    // ASPIRATION WINDOWS
     i64 window = 12;
     size_t elapsed;
    while (true) {
       const i32 alpha = score - window;
       const i32 beta =  score +  window;
      score = search(
#ifdef FULL
          &data->nodes,
#endif
            data,  beta,  &data->pos,  depth,   0,  alpha,  false);
#ifdef FULL
      if (data->thread_id == 0) {
        print_info(&data->pos, depth, alpha, beta, score, data->nodes, data->stack[0].best_move, data->max_time);
      }
#endif
      elapsed = get_time() - start_time;
       window *= 2;
       if ( elapsed > data->max_time ||  ( score > alpha &&  score < beta)) { break; }
    }

    if ( stop ||  elapsed > data->max_time / 10) {
      break;
    }
  }
}

static void *entry_full(void *param) {
  ThreadData *data = param;
  iteratively_deepen(
#ifdef FULL
      max_ply,
#endif
      data);
  return NULL;
}

static void entry_mini(ThreadHead *head) {
  iteratively_deepen(
#ifdef FULL
      max_ply,
#endif
      &head->data);
  exit_now();
}

#ifdef NOSTDLIB
__attribute__((naked)) static long newthread(ThreadHead *head) {
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

static
void run_smp() {
  start_time = get_time();
#ifndef NOSTDLIB
  main_data->nodes = 0;
  pthread_t helpers[thread_count - 1];
  ThreadData *helper_data[thread_count - 1];

  for (i32 i = 0; i < thread_count - 1; i++) {
    helper_data[i] = malloc(sizeof(ThreadData));
    __builtin_memset(helper_data[i], 0, sizeof(ThreadData));
    helper_data[i]->pos = main_data->pos;
    helper_data[i]->max_time = -1LL;
    helper_data[i]->thread_id = i + 1;
    pthread_create(&helpers[i], NULL, entry_full, helper_data[i]);
  }

  iteratively_deepen(max_ply, main_data);
  stop = true;

  for (i32 i = 0; i < thread_count - 1; i++) {
    pthread_join(helpers[i], NULL);
    free(helper_data[i]);
  }
#else
#ifdef FULL
  main_data->nodes = 0;
#else
  ThreadData *main_data = (ThreadData *)&thread_stacks[0][0];
#endif

  for (i32 i = 1; i < thread_count; i++) {
    ThreadHead *helper_head = (ThreadHead *)&thread_stacks[i][thread_stack_size - sizeof(ThreadHead)];
     helper_head->data.max_time = -1LL;
     helper_head->data.pos = main_data->pos;
#ifdef FULL
    helper_head->data.thread_id = i;
#endif
    helper_head->entry = entry_mini;
    newthread(helper_head);
  }

  iteratively_deepen(
#ifdef FULL
      max_ply,
#endif
      main_data);
  stop = true;

  for (i32 i = 0; i < thread_count - 1; i++) {
    // TODO: sync ?
  }
#endif

  char move_name[8];
  move_str( main_data->pos.flipped,  &main_data->stack[0].best_move,  move_name);
  putl("bestmove ");
  puts(move_name);
}

#if defined(FULL) && !defined(NOSTDLIB)
static pthread_t bg_thread;
static bool bg_running = false;
static void *bg_entry(void *unused) {
  (void)unused;
  run_smp();
  return NULL;
}
static void bg_stop(void) {
  if (bg_running) {
    stop = true;
    pthread_join(bg_thread, NULL);
    bg_running = false;
  }
}
#endif

#ifdef FULL
static void display_pos(Position *const pos) {
  Position npos = *pos;
  if (npos.flipped) {
    flip_pos(&npos);
  }
  for (i32 rank = 7; rank >= 0; rank--) {
    for (i32 file = 0; file < 8; file++) {
      i32 sq = rank * 8 + file;
      u64 bb = 1ULL << sq;
      i32 piece = piece_on( &npos,  sq);
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



Position start_pos = (Position){
    .ep = 0,
    .colour = {0xFFFFull, 0xFFFF000000000000ull},
    .pieces = {0, 0xFF00000000FF00ull, 0x4200000000000042ull, 0x2400000000000024ull, 0x8100000000000081ull, 0x800000000000008ull, 0x1000000000000010ull},
    .castling = {true, true, true, true}};

#ifdef FULL
static void bench() {
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
static void run() {
#endif
#ifndef NOSTDLIB
  setvbuf(stdout, NULL, _IONBF, 0);
#endif

   char line[4096];
   init();
#ifdef FULL
  main_data = calloc(1, sizeof(ThreadData));
  tt = malloc(tt_length * sizeof(TTEntry));
  __builtin_memset(tt, 0, tt_length * sizeof(TTEntry));
  main_data->pos = start_pos;
#else
  ThreadData *main_data = (ThreadData *)&thread_stacks[0][0];
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
      puts("option name Hash type spin default 80 min 1 max 65536");
      puts("option name Threads type spin default 1 min 1 max 256");
      puts("uciok");
    } else if (!strcmp(line, "setoption")) {
#if defined(FULL) && !defined(NOSTDLIB)
      bg_stop();
#endif
      getl(line); // "name"
      getl(line); // option name
      if (!strcmp(line, "Hash")) {
        getl(line); // "value"
        getl(line); // MB
        free(tt);
        const u64 mb = atoi(line);
        tt_length = mb * 1024 * 1024 / sizeof(TTEntry);
        tt = malloc(tt_length * sizeof(TTEntry));
        __builtin_memset(tt, 0, tt_length * sizeof(TTEntry));
      } else if (!strcmp(line, "Threads")) {
        getl(line); // "value"
        getl(line);
#ifdef NOSTDLIB
        if (atoi(line) > 1) {
          puts("info string Threads > 1 not supported in this build");
          exit_now();
        }
#else
        thread_count = atoi(line);
#endif
      }
    } else if (!strcmp(line, "ucinewgame")) {
#if defined(FULL) && !defined(NOSTDLIB)
      bg_stop();
#endif
      __builtin_memset(main_data, 0, sizeof(ThreadData));
      __builtin_memset(tt, 0, tt_length * sizeof(TTEntry));
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
      printf("info depth %i nodes %llu time %llu nps %llu \n", depth, nodes, elapsed, nps);
    }
#endif
     if ( line[0] ==  'q') { exit_now(); }
    else  if ( line[0] ==  'i') { puts("readyok"); }
#if defined(FULL) && !defined(NOSTDLIB)
    else  if (line[0] == 's') { bg_stop(); }
#endif
    else  if ( line[0] ==  'p') {
#if defined(FULL) && !defined(NOSTDLIB)
      bg_stop();
#endif
       main_data->pos = start_pos;
#ifdef FULL
        pv_hist_len = 0;
#endif
      while (true) {
        bool line_continue = getl(line);

#ifdef FULL
        if (!strcmp(line, "fen")) {
          getl(line);
          line_continue = get_fen(&main_data->pos, line);
        }
        else
#endif
        {
          Move moves[max_moves];
          const u8 num_moves =
            movegen( &main_data->pos,  false,
               moves);
          for (i32 i = 0; i < num_moves; i++) {
            char move_name[8];
            move_str( main_data->pos.flipped,
               &moves[i],
               move_name);
            assert(move_string_equal(line, move_name) ==
              !strcmp(line, move_name));
            if (move_string_equal( move_name,  line)) {
#ifdef FULL
              if (moves[i].takes_piece != None ||
                piece_on( &main_data->pos,
                   moves[i].from) == Pawn) {
                pv_hist_len = 0;
              }
              else if (pv_hist_len < 256) {
                pv_hist[pv_hist_len++] = get_hash(&main_data->pos);
              }
#endif
              makemove( &main_data->pos,  &moves[i]);
              break;
            }
          }
        }
        if (!line_continue) {
          break;
        }
      }
    } else  if ( line[0] ==  'g') {
#if defined(FULL) && !defined(NOSTDLIB)
      bg_stop();
#endif
      stop = false;
#ifdef FULL
      bool infinite = false;
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
          main_data->max_time =
            20ULL * 1000 * 1000 * 1000; // Assume Lichess bot
          break;
        }
        else if (!strcmp(line, "infinite")) {
#ifndef NOSTDLIB
          main_data->max_time = -1LL;
#else
          main_data->max_time = 20ULL * 1000 * 1000 * 1000;
#endif
          infinite = true;
          break;
        }
      }
#ifndef NOSTDLIB
      if (infinite) {
        pthread_create(&bg_thread, NULL, bg_entry, NULL);
        bg_running = true;
      }
      else
        run_smp();
#else
      (void)infinite;
      run_smp();
#endif
#else
      for (i64 i = 2 << main_data->pos.flipped; i > 0; i--) {
        getl(line);
        main_data->max_time = (u64)atoi(line) << 19; // Roughly /2 time
      }
      run_smp();
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
    tt = malloc(tt_length * sizeof(TTEntry));
    __builtin_memset(tt, 0, tt_length * sizeof(TTEntry));
    bench();
    exit_now();
  }
#endif
  run();
}
#endif

#pragma endregion
