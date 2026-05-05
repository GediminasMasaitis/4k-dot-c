// Throwaway: mixing-quality tests for hash functions used in 4k.c.
//
//   MODE = PAWN      ->  pawn corrhist hash variants (A-E)
//   MODE = MATERIAL  ->  material corrhist hash variants (Current + A-E
//                        applied to the *9+count accumulator)
//   MODE = MAIN      ->  main get_hash variants (F = with final round,
//                        G = without)
//
// All modes use the same framework: walk a chain of "semi-realistic"
// position states with walk_step() and measure both static distribution and
// avalanche behaviour for each variant in one pass.
//
// Edit MODE below and rebuild.

#include <math.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <wmmintrin.h>

#define PAWN 0
#define MATERIAL 1
#define MAIN 2
#define MODE MATERIAL

typedef uint8_t u8;
typedef uint32_t u32;
typedef uint64_t u64;
typedef long long __attribute__((__vector_size__(16))) i128;

#define BUCKETS 16384
#define BUCKET_BITS 14
#define SAMPLES 100000000
#define WALK_LEN 100

// ---------------------------------------------------------------- RNG -----

static u64 rng_state = 0x123456789ABCDEFULL;
static inline u64 xs64(void) {
  u64 x = rng_state;
  x ^= x << 13;
  x ^= x >> 7;
  x ^= x << 17;
  rng_state = x;
  return x;
}

// ------------------------------------------------------------- Position ---

enum { None, Pawn, Knight, Bishop, Rook, Queen, King };

typedef struct {
  u64 ep;
  u64 pieces[7];
  u64 colour[2];
  u8 castling[4];
  u8 flipped;
  u8 padding[11];
} Position;
_Static_assert(sizeof(Position) == 96, "Position must be 96 bytes for AES");

static void place_pieces(Position *pos, int side, int piece, u64 *occupied,
                         int n, int sq_min, int sq_count) {
  for (int i = 0; i < n; i++) {
    for (;;) {
      int sq = sq_min + (int)(xs64() % (u32)sq_count);
      u64 bit = 1ULL << sq;
      if (!(*occupied & bit)) {
        pos->pieces[piece] |= bit;
        pos->colour[side] |= bit;
        *occupied |= bit;
        break;
      }
    }
  }
}

static Position gen_position(void) {
  Position pos = {0};
  u64 occupied = 0;
  for (int side = 0; side < 2; side++) {
    place_pieces(&pos, side, King,   &occupied, 1,                  0,  64);
    place_pieces(&pos, side, Pawn,   &occupied, (int)(xs64() % 9),  8,  48);
    place_pieces(&pos, side, Knight, &occupied, (int)(xs64() % 3),  0,  64);
    place_pieces(&pos, side, Bishop, &occupied, (int)(xs64() % 3),  0,  64);
    place_pieces(&pos, side, Rook,   &occupied, (int)(xs64() % 3),  0,  64);
    place_pieces(&pos, side, Queen,  &occupied, (int)(xs64() % 2),  0,  64);
  }
  for (int i = 0; i < 4; i++) pos.castling[i] = (u8)(xs64() & 1);
  pos.flipped = (u8)(xs64() & 1);
  if ((xs64() & 0xF) == 0) {
    int file = (int)(xs64() & 7);
    int rank = (xs64() & 1) ? 5 : 2;
    pos.ep = 1ULL << (rank * 8 + file);
  }
  return pos;
}

// One step of "semi-realistic" position evolution: flip one bit in a random
// piece bitboard plus the same bit in one colour bitboard, toggle side-to-
// move, and occasionally drop a castling bit or toggle the ep target. Does
// not follow chess rules.
static void walk_step(Position *pos) {
  int p = Pawn + (int)(xs64() % 6);
  // Pawns may only sit on ranks 2-7 (bits 8..55) in a legal chess position.
  int sq = (p == Pawn) ? 8 + (int)(xs64() % 48) : (int)(xs64() & 63);
  int side = (int)(xs64() & 1);
  u64 bit = 1ULL << sq;
  pos->pieces[p] ^= bit;
  pos->colour[side] ^= bit;
  pos->flipped ^= 1;
  if ((xs64() & 0x1F) == 0) pos->castling[xs64() & 3] = 0;
  if ((xs64() & 0x7) == 0) {
    if (pos->ep) {
      pos->ep = 0;
    } else {
      int file = (int)(xs64() & 7);
      int rank = (xs64() & 1) ? 5 : 2;
      pos->ep = 1ULL << (rank * 8 + file);
    }
  }
}

// ----------------------------------------------------- pawn hash variants -

static inline u64 pawn_A(const Position *pos) {
  u64 p = pos->pieces[Pawn];
  return (p * 0x9E3779B97F4A7C15ULL) & (BUCKETS - 1);
}

static inline u64 pawn_B(const Position *pos) {
  u64 p = pos->pieces[Pawn];
  return ((p ^ (p >> 32)) * 2654435761u) & (BUCKETS - 1);
}

static inline u64 pawn_C(const Position *pos) {
  u64 p = pos->pieces[Pawn];
  return (p * 0x9E3779B97F4A7C15ULL) >> (64 - BUCKET_BITS);
}

__attribute__((target("aes"))) static u64 pawn_D(const Position *pos) {
  u64 p = pos->pieces[Pawn];
  i128 state = {0};
  i128 key = {(long long)p, 0};
  state = __builtin_ia32_aesenc128(state, key);
  return (u64)state[0] & (BUCKETS - 1);
}

__attribute__((target("aes"))) static u64 pawn_E(const Position *pos) {
  u64 p = pos->pieces[Pawn];
  i128 state = {0};
  i128 key = {(long long)p, 0};
  state = __builtin_ia32_aesenc128(state, key);
  state = __builtin_ia32_aesenc128(state, state);
  return (u64)state[0] & (BUCKETS - 1);
}

// ------------------------------------------------- material hash variants -

// The current 4k.c material accumulator: Horner-style on per-(color,piece)
// popcounts using radix 9. Result is a small u64 that uniquely encodes the
// material distribution when each count stays < 9.
static inline u64 calc_material_hash(const Position *pos) {
  u64 hash = 0;
  for (int c = 0; c < 2; c++) {
    for (int p = Pawn; p <= Queen; p++) {
      hash = hash * 9 +
             (u64)__builtin_popcountll(pos->pieces[p] & pos->colour[c]);
    }
  }
  return hash;
}

static inline u64 mat_current(const Position *pos) {
  return calc_material_hash(pos) & (BUCKETS - 1);
}

static inline u64 mat_A(const Position *pos) {
  u64 m = calc_material_hash(pos);
  return (m * 0x9E3779B97F4A7C15ULL) & (BUCKETS - 1);
}

static inline u64 mat_B(const Position *pos) {
  u64 m = calc_material_hash(pos);
  return ((m ^ (m >> 32)) * 2654435761u) & (BUCKETS - 1);
}

static inline u64 mat_C(const Position *pos) {
  u64 m = calc_material_hash(pos);
  return (m * 0x9E3779B97F4A7C15ULL) >> (64 - BUCKET_BITS);
}

__attribute__((target("aes"))) static u64 mat_D(const Position *pos) {
  u64 m = calc_material_hash(pos);
  i128 state = {0};
  i128 key = {(long long)m, 0};
  state = __builtin_ia32_aesenc128(state, key);
  return (u64)state[0] & (BUCKETS - 1);
}

__attribute__((target("aes"))) static u64 mat_E(const Position *pos) {
  u64 m = calc_material_hash(pos);
  i128 state = {0};
  i128 key = {(long long)m, 0};
  state = __builtin_ia32_aesenc128(state, key);
  state = __builtin_ia32_aesenc128(state, state);
  return (u64)state[0] & (BUCKETS - 1);
}

// ----------------------------------------------------- main hash variants -

__attribute__((target("aes"))) static u64 hash_main(const Position *pos,
                                                    int do_final_round) {
  i128 hash = {0};
  const u8 *data = (const u8 *)pos;
  for (int i = 0; i < 6; i++) {
    i128 key;
    __builtin_memcpy(&key, data + i * 16, 16);
    hash = __builtin_ia32_aesenc128(hash, key);
  }
  if (do_final_round) hash = __builtin_ia32_aesenc128(hash, hash);
  return (u64)hash[0];
}

static u64 main_F(const Position *pos) { return hash_main(pos, 1); }
static u64 main_G(const Position *pos) { return hash_main(pos, 0); }

// ------------------------------------------------------ stats accumulators

// Distribution: chi-squared on multiple 14-bit windows + per-bit balance.
// For PAWN, only slice 0 (low 14 bits) is meaningful; others always read 0.
#define NUM_SLICES 4
static const int slice_shifts[NUM_SLICES] = {0, 16, 32, 50};

typedef struct {
  u32 *counts[NUM_SLICES];
  long bit_ones[64];
  long n;
} dist_t;

static dist_t dist_create(void) {
  dist_t d = {0};
  for (int s = 0; s < NUM_SLICES; s++)
    d.counts[s] = calloc(BUCKETS, sizeof(u32));
  return d;
}

static inline void dist_update(dist_t *d, u64 h) {
  for (int s = 0; s < NUM_SLICES; s++) {
    u32 idx = (u32)((h >> slice_shifts[s]) & (BUCKETS - 1));
    d->counts[s][idx]++;
  }
  for (int b = 0; b < 64; b++) d->bit_ones[b] += (h >> b) & 1;
  d->n++;
}

static void dist_free(dist_t *d) {
  for (int s = 0; s < NUM_SLICES; s++) free(d->counts[s]);
}

// num_slices_to_print: 1 for PAWN (only low slice meaningful), 4 for MAIN.
// output_bits: 14 for PAWN, 64 for MAIN — limits per-bit balance reporting.
static void dist_print(const dist_t *d, int num_slices_to_print,
                       int output_bits) {
  double expected = (double)d->n / BUCKETS;
  for (int s = 0; s < num_slices_to_print; s++) {
    int distinct = 0;
    u32 max_count = 0;
    double chi2 = 0.0;
    for (int i = 0; i < BUCKETS; i++) {
      if (d->counts[s][i]) distinct++;
      if (d->counts[s][i] > max_count) max_count = d->counts[s][i];
      double diff = d->counts[s][i] - expected;
      chi2 += diff * diff / expected;
    }
    if (num_slices_to_print == 1) {
      printf("  distribution:       distinct %d/%d  max %u (%.3fx)  chi2 %.0f"
             "  (uniform-expected ~%d)\n",
             distinct, BUCKETS, max_count, (double)max_count / expected, chi2,
             BUCKETS - 1);
    } else {
      printf("  slice bits %2d-%2d:   distinct %5d/%d  max %u (%.3fx)  chi2 %.0f\n",
             slice_shifts[s], slice_shifts[s] + BUCKET_BITS - 1, distinct,
             BUCKETS, max_count, (double)max_count / expected, chi2);
    }
  }
  double total_e = 0, min_e = 1.0;
  int min_bit = 0;
  for (int b = 0; b < output_bits; b++) {
    double p1 = (double)d->bit_ones[b] / d->n;
    double p0 = 1.0 - p1;
    double e = 0;
    if (p1 > 0) e -= p1 * log2(p1);
    if (p0 > 0) e -= p0 * log2(p0);
    total_e += e;
    if (e < min_e) {
      min_e = e;
      min_bit = b;
    }
  }
  printf("  per-bit entropy:    avg %.6f / 1.0   min %.6f at bit %d\n",
         total_e / output_bits, min_e, min_bit);
}

// Avalanche: HW between consecutive output hashes.
typedef struct {
  long bit_flips[64];
  long hw_hist[65];
  long n;
} aval_t;

static inline void aval_update(aval_t *a, u64 prev, u64 cur) {
  u64 x = prev ^ cur;
  a->hw_hist[__builtin_popcountll(x)]++;
  for (int b = 0; b < 64; b++) a->bit_flips[b] += (x >> b) & 1;
  a->n++;
}

static void aval_print(const aval_t *a, int output_bits) {
  if (a->n == 0) {
    printf("  avalanche:          (no input-changing steps recorded)\n");
    return;
  }
  long total = 0;
  for (int hw = 0; hw <= 64; hw++) total += (long)hw * a->hw_hist[hw];
  double mean_hw = (double)total / a->n;
  double var = 0;
  for (int hw = 0; hw <= 64; hw++) {
    double dh = hw - mean_hw;
    var += a->hw_hist[hw] * dh * dh;
  }
  var /= a->n;
  double ideal_mean = output_bits / 2.0;
  double ideal_sd = sqrt(output_bits * 0.25);

  double min_rate = 1.0, max_rate = 0.0, sum_dev = 0;
  int min_bit = 0, max_bit = 0;
  for (int b = 0; b < output_bits; b++) {
    double rate = (double)a->bit_flips[b] / a->n;
    if (rate < min_rate) {
      min_rate = rate;
      min_bit = b;
    }
    if (rate > max_rate) {
      max_rate = rate;
      max_bit = b;
    }
    sum_dev += fabs(rate - 0.5);
  }

  printf("  avalanche samples:  %ld\n", a->n);
  printf("  output HW per step: mean %.4f  stddev %.4f  (ideal %.2f / %.2f)\n",
         mean_hw, sqrt(var), ideal_mean, ideal_sd);
  printf("  per-bit flip rate:  min %.4f (bit %d), max %.4f (bit %d)\n",
         min_rate, min_bit, max_rate, max_bit);
  printf("  avg |rate - 0.5|:   %.6f  (ideal 0)\n", sum_dev / output_bits);
  printf("  HW histogram (>=0.5%%):");
  for (int hw = 0; hw <= output_bits; hw++) {
    double pct = 100.0 * a->hw_hist[hw] / a->n;
    if (pct >= 0.5) printf(" %d:%.1f%%", hw, pct);
  }
  printf("\n");
}

// ----------------------------------------------------------------- main ---

// Per-mode predicate for "did the relevant input to this hash family change?"
// Used to filter avalanche updates (HW=0 from no-op steps would just add
// noise to the histogram).
#if MODE == PAWN
static inline u64 input_key(const Position *pos) { return pos->pieces[Pawn]; }
#elif MODE == MATERIAL
static inline u64 input_key(const Position *pos) {
  return calc_material_hash(pos);
}
#elif MODE == MAIN
// MAIN: walk_step almost always changes some byte of the position, so just
// always count. Pick any cheap key — the comparison is unused.
static inline u64 input_key(const Position *pos) { (void)pos; return 0; }
#endif

int main(void) {
#if MODE == PAWN
  printf("MODE PAWN: pawn corrhist hash variants on position walks\n");
#define NUM_VARIANTS 5
  static const struct {
    const char *name;
    u64 (*fn)(const Position *);
  } variants[NUM_VARIANTS] = {
      {"A: p * 0x9E3779B97F4A7C15 (low 14 bits)", pawn_A},
      {"B: (p ^ p>>32) * 2654435761 (low 14 bits)", pawn_B},
      {"C: p * 0x9E3779B97F4A7C15 >> 50 (high 14 bits)", pawn_C},
      {"D: aesenc(0, {p,0}) (low 14 bits)", pawn_D},
      {"E: aesenc(aesenc(0, {p,0}), self) (low 14 bits)", pawn_E},
  };
  const int output_bits = BUCKET_BITS;
  const int num_slices_to_print = 1;
#elif MODE == MATERIAL
  printf("MODE MATERIAL: material corrhist hash variants on position walks\n");
#define NUM_VARIANTS 6
  static const struct {
    const char *name;
    u64 (*fn)(const Position *);
  } variants[NUM_VARIANTS] = {
      {"Current: m & 0x3FFF (no mixing, low 14 bits of accumulator)",
       mat_current},
      {"A: m * 0x9E3779B97F4A7C15 (low 14 bits)", mat_A},
      {"B: (m ^ m>>32) * 2654435761 (low 14 bits)", mat_B},
      {"C: m * 0x9E3779B97F4A7C15 >> 50 (high 14 bits)", mat_C},
      {"D: aesenc(0, {m,0}) (low 14 bits)", mat_D},
      {"E: aesenc(aesenc(0, {m,0}), self) (low 14 bits)", mat_E},
  };
  const int output_bits = BUCKET_BITS;
  const int num_slices_to_print = 1;
#elif MODE == MAIN
  printf("MODE MAIN: main get_hash variants on position walks\n");
#define NUM_VARIANTS 2
  static const struct {
    const char *name;
    u64 (*fn)(const Position *);
  } variants[NUM_VARIANTS] = {
      {"F: full get_hash (6 keyed rounds + final aesenc(h,h))", main_F},
      {"G: get_hash without final mixing round", main_G},
  };
  const int output_bits = 64;
  const int num_slices_to_print = NUM_SLICES;
#else
#error "Unknown MODE"
#endif

  int num_walks = SAMPLES / WALK_LEN;
  printf("Walks: %d  Steps per walk: %d  Total steps: %d\n\n", num_walks,
         WALK_LEN, num_walks * WALK_LEN);

  dist_t dists[NUM_VARIANTS];
  aval_t avals[NUM_VARIANTS] = {0};
  u64 prevs[NUM_VARIANTS];
  for (int v = 0; v < NUM_VARIANTS; v++) dists[v] = dist_create();

  for (int w = 0; w < num_walks; w++) {
    Position pos = gen_position();
    u64 prev_key = input_key(&pos);
    for (int v = 0; v < NUM_VARIANTS; v++) {
      prevs[v] = variants[v].fn(&pos);
      dist_update(&dists[v], prevs[v]);
    }
    for (int step = 0; step < WALK_LEN; step++) {
      walk_step(&pos);
      u64 new_key = input_key(&pos);
#if MODE == MAIN
      const int count_aval = 1;
#else
      int count_aval = (new_key != prev_key);
#endif
      prev_key = new_key;
      for (int v = 0; v < NUM_VARIANTS; v++) {
        u64 h = variants[v].fn(&pos);
        dist_update(&dists[v], h);
        if (count_aval) aval_update(&avals[v], prevs[v], h);
        prevs[v] = h;
      }
    }
  }

  for (int v = 0; v < NUM_VARIANTS; v++) {
    printf("=== %s ===\n", variants[v].name);
    dist_print(&dists[v], num_slices_to_print, output_bits);
    aval_print(&avals[v], output_bits);
    printf("\n");
    dist_free(&dists[v]);
  }
  return 0;
}
