// Derived from Crinkler by Rune L. H. Stubbe and Aske Simon Christensen
// Original: https://github.com/runestubbe/Crinkler (zlib license)
// Rewritten and ported to Linux ELF x86-64

#include <assert.h>
#include <getopt.h>
#include <limits.h>
#include <nmmintrin.h> /* _mm_crc32_u8 (SSE4.2) */
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#if defined(_WIN32)
#include <malloc.h>
#endif

typedef float v4f __attribute__((vector_size(16)));
typedef uint32_t v4u __attribute__((vector_size(16)));
typedef int32_t v4i __attribute__((vector_size(16)));
typedef uint16_t v8u16 __attribute__((vector_size(16)));

#define TPREC_BITS 12
#define TPREC (1 << TPREC_BITS)
#define MAX_CTX 8
#define DEFAULT_BPROB 10
#define BIT_PREC 256
#define MAX_MODELS 256
#define MAX_SEARCH 21
#define MAX_WEIGHT 9
#define NPV 16
#define PKG_BITS (NPV * 4)

#define DEFAULT_BEAM 1

static int verbose = 0;        /* 0=off, 1=verbose, 2=very verbose */
static int extreme = 0;        /* use real compression instead of estimator */
static int direct_bits = 24;   /* direct-mapped table of 2^direct_bits 2-byte
                                  slots (Crinkler-style, lossy); loader
                                  DIRECT_BITS   must match. Override with -H. */
static int timing_reps = 1;    /* repeat the encode pass for stable timing */
static double g_encode_ms = 0; /* last encode pass time, ms/iteration */
static int large_field = 0;

static int hdr_bitlen_bytes(void) { return large_field ? 4 : 2; }
static int hdr_base_bytes(void) { return large_field ? 8 : 6; }

/* The model count is not stored in the header; recover it by replaying the
   loader's wmask consumption: bits MSB-first, a 0 bit stores a model, a 1 bit
   bumps the weight, and the field ends when the register hits zero. */
static int wmask_num_models(unsigned int w) {
  int num = 0;
  for (;;) {
    unsigned int bit = w & 0x80000000u;
    w <<= 1;
    if (!w)
      break;
    if (!bit)
      num++;
  }
  return num;
}

typedef struct {
  unsigned char weight;
  unsigned char mask;
} Model;

typedef struct {
  Model models[MAX_SEARCH];
  int num_models;
  int size;
} ModelSet;

typedef struct {
  uint16_t prob[NPV][8];
} CompactPredBlock;

typedef struct {
  v4f prob[NPV][2];
} PredBlock;

typedef struct {
  int num_blocks;
  int offset;
} ModelPred;

typedef struct {
  unsigned char *dest;
  unsigned int bit_pos;
  unsigned int range;
  unsigned int low;
} ArithCoder;

typedef struct {
  unsigned char mask;
  unsigned char bitnum;
  unsigned char prob[2];
  const unsigned char *data_ptr;
  unsigned int generation;
} CtxEntry;

typedef struct {
  unsigned int *hashes;
  unsigned int hashes_len, hashes_cap;
  unsigned char *bits;
  int bits_len, bits_cap;
  int *weights;
  int num_weights;
} HashBitStream;

typedef struct {
  int active_weights[MAX_MODELS];
  const struct CompState *cs;
  PredBlock *accum;
  unsigned *accum_sizes;
  int64_t cost;
  float log_scale;
} Evaluator;

typedef struct CompState {
  ModelPred models[MAX_MODELS];
  CompactPredBlock *block_arena;
  int *map_arena;
  Evaluator *eval;
} CompState;

typedef struct {
  char input_file[512];
  int input_size;
  int beam;
  int simple;
  int extreme;
  int base_prob;
  int large_field; /* -L: 32-bit header bitlength field for inputs >~8KB */

  int compressed_bits;
  int header_bytes;
  int total_bytes;
  float estimated_bytes;
  double search_ms; /* model search wall time */

  /* actual emitted header, for the anatomy card */
  unsigned int hdr_wmask;
  unsigned char hdr_masks[MAX_SEARCH]; /* header order: ascending weight */
  int hdr_bitlen;

  int num_models;
  unsigned char model_masks[MAX_SEARCH];
  int model_weights[MAX_SEARCH];
  char model_string[512];

  unsigned int model_hits[MAX_SEARCH];
  unsigned int model_misses[MAX_SEARCH];
  double model_bits_saved[MAX_SEARCH];

  unsigned int conf_hist[11];
  /* calibration: predicted P(bit=1) in 5% buckets vs actual outcomes */
  unsigned int calib_count[20];
  unsigned int calib_ones[20];
  double calib_psum[20];
  int total_bits;

  unsigned int bytepos_count[8];
  double bytepos_cost[8];
  double total_cost;

  unsigned int min_range;

  unsigned int ht_occupied;
  unsigned int ht_size;
  /* direct-mapped table occupancy growth: fresh-slot activations sampled
     every occ_interval bits during the stats pass */
  unsigned int occ_samples[257];
  int occ_nsamples;
  int occ_interval;

  unsigned int sat_lopsided;
  unsigned int sat_strong;
  unsigned int sat_balanced;
  unsigned int sat_mixed;

  double entropy; /* Shannon H0 in bits/byte */
  unsigned int byte_freq[256];
  float *byte_costs; /* cost per data byte during encoding */
  float *byte_model_contrib; /* [num_data_bytes * num_models] per-byte per-model */
  float *bit_costs; /* [num_data_bytes * 8] per-bit cost, MSB-first */
  int num_data_bytes;
  const unsigned char
      *input_data; /* pointer to original input (valid during report write) */

  /* search trajectory log */
  float *search_best; /* best estimate after each mask (256 entries) */
  int search_len;
  struct {
    int mask_idx;       /* which of the 256 iterations */
    unsigned char mask; /* context mask value */
    int num_models;     /* model count after this event */
    float est_bytes;    /* estimated size */
    int is_removal;     /* 0 = addition, 1 = removal */
  } *search_events;
  int search_nevents, search_events_cap;

  /* per-mask search outcome: 0=not tried, 1=rejected, 2=accepted then
     removed, 3=in final set; delta = est. size change (bytes) at trial */
  unsigned char mask_outcome[256];
  float mask_delta[256];

  int valid;
} CompStats;

static const char *html_output = NULL;
static char g_cmdline[1024]; /* original argv, for report provenance */

static void fputs_html(const char *str, FILE *f) {
  for (; *str; str++) {
    switch (*str) {
    case '&':
      fputs("&amp;", f);
      break;
    case '<':
      fputs("&lt;", f);
      break;
    case '>':
      fputs("&gt;", f);
      break;
    case '"':
      fputs("&quot;", f);
      break;
    default:
      fputc(*str, f);
    }
  }
}

static inline v4f v4f_splat(float x) { return (v4f){x, x, x, x}; }
static inline v4u v4u_splat(uint32_t x) { return (v4u){x, x, x, x}; }

static inline float fast_log2f(float x) {
  union {
    float f;
    uint32_t u;
  } bits = {x};
  int exp = (int)(bits.u >> 23) - 127;
  bits.u = (bits.u & 0x7FFFFFu) | 0x3F800000u;
  float m = bits.f - 1.0f;
  return exp + m * (1.42286530448213f +
                    m * (-0.58208536795165f + m * 0.15922006346951f));
}

/* sqrt without libm: exponent-halving first guess + Newton refinement */
static double sqrt_pos(double x) {
  if (x <= 0)
    return 0;
  union {
    double d;
    uint64_t u;
  } b = {x};
  b.u = (b.u >> 1) + 0x1FF8000000000000ULL;
  double g = b.d;
  for (int i = 0; i < 6; i++)
    g = 0.5 * (g + x / g);
  return g;
}

static inline void *alloc_aligned(size_t size) {
  /* aligned_alloc requires size to be a multiple of alignment */
  size_t rounded = (size + 31) & ~(size_t)31;
#if defined(_WIN32)
  return _aligned_malloc(rounded, 32);
#else
  return aligned_alloc(32, rounded);
#endif
}

static inline void free_aligned(void *p) {
#if defined(_WIN32)
  _aligned_free(p);
#else
  free(p);
#endif
}

static int next_pow2(int v) {
  v--;
  v |= v >> 1;
  v |= v >> 2;
  v |= v >> 4;
  v |= v >> 8;
  v |= v >> 16;
  return v + 1;
}

static int prev_prime(int n) {
  for (;;) {
    n = (n - 2) | 1;
    int ok = 1;
    for (int d = 3; d * d < n; d += 2) {
      if (n % d == 0) {
        ok = 0;
        break;
      }
    }
    if (ok)
      return n;
  }
}

static double mono_sec(void) {
  struct timespec ts;
  clock_gettime(CLOCK_MONOTONIC, &ts);
  return (double)ts.tv_sec + (double)ts.tv_nsec * 1e-9;
}

static inline void counter_update(unsigned char prob[2], int bit) {
  prob[bit] += 1;
  if (prob[!bit] > 1)
    prob[!bit] >>= 1;
}

static unsigned char *alloc_padded(const unsigned char *ctx,
                                   const unsigned char *data, int size) {
  unsigned char *buf = (unsigned char *)malloc(size + MAX_CTX);
  memcpy(buf, ctx, MAX_CTX);
  memcpy(buf + MAX_CTX, data, size);
  return buf;
}

#define DYNPUSH(arr, len, cap, val, elem_sz)                                   \
  do {                                                                         \
    if ((len) >= (cap)) {                                                      \
      (cap) = (cap) ? (cap) * 2 : 4096;                                        \
      (arr) = realloc((arr), (cap) * (elem_sz));                               \
    }                                                                          \
    (arr)[(len)++] = (val);                                                    \
  } while (0)

static inline unsigned int hash_mix(unsigned int h, unsigned char byte) {
  return _mm_crc32_u8(h, byte);
}

static unsigned int ctx_hash_initial(unsigned int mask) {
  unsigned char cmask = (unsigned char)mask;
  unsigned int h = mask;
  h = _mm_crc32_u8(h, 0);
  while (cmask) {
    if (cmask & 0x80)
      h = _mm_crc32_u8(h, 0);
    cmask += cmask;
  }
  return h;
}

static unsigned int ctx_hash(const unsigned char *data, int bit_pos,
                             unsigned int mask) {
  unsigned char cmask = (unsigned char)mask;
  const unsigned char *ptr = data + (bit_pos >> 3);
  unsigned int h = mask;
  unsigned char cur = (0x100 | *ptr) >> ((~bit_pos & 7) + 1);
  h = hash_mix(h, cur);
  while (cmask) {
    ptr--;
    if (cmask & 0x80) {
      cur = *ptr;
      h = hash_mix(h, cur);
    }
    cmask += cmask;
  }
  return h;
}

static inline int get_bit(const unsigned char *data, int pos) {
  return (data[pos >> 3] >> (7 - (pos & 7))) & 1;
}

static inline int get_compressed_bit(const unsigned char *data, int pos) {
  return (data[pos >> 3] >> (pos & 7)) & 1;
}

static void arith_init(ArithCoder *ac, void *dest) {
  ac->dest = (unsigned char *)dest;
  ac->bit_pos = (unsigned)-1;
  ac->range = 0x80000000u;
  ac->low = 0;
}

static void propagate_carry(unsigned char *dest, int pos) {
  while (--pos >= 0) {
    unsigned int mask = 1u << (pos & 7);
    unsigned int byte = dest[pos >> 3];
    dest[pos >> 3] = byte ^ mask;
    if (!(byte & mask))
      break;
  }
}

static void arith_encode(ArithCoder *ac, unsigned int p0, unsigned int p1,
                         int bit) {
  unsigned int total = p0 + p1;
  unsigned int thresh = (uint64_t)ac->range * p0 / total;
  if (bit) {
    unsigned int old_low = ac->low;
    ac->low += thresh;
    if (ac->low < old_low)
      propagate_carry(ac->dest, ac->bit_pos);
    ac->range -= thresh;
  } else {
    ac->range = thresh;
  }
  while (ac->range < 0x80000000u) {
    ac->bit_pos++;
    if (ac->low & 0x80000000u)
      propagate_carry(ac->dest, ac->bit_pos);
    ac->low <<= 1;
    ac->range <<= 1;
  }
}

static int arith_finish(const ArithCoder *ac) {
  unsigned int pos = ac->bit_pos;
  if (ac->low > 0) {
    if (ac->low + ac->range - 1 >= ac->low)
      pos++;
    propagate_carry(ac->dest, pos);
  }
  return pos;
}

static unsigned int encode_weight_mask(const ModelSet *ml, unsigned char *masks,
                                       int terminate) {
  unsigned int wmask = 0;
  int remaining = ml->num_models;
  int bit = 31;
  for (int w = 0; remaining; w++) {
    for (int m = 0; m < ml->num_models; m++) {
      if (ml->models[m].weight == w) {
        if (masks)
          *masks++ = ml->models[m].mask;
        remaining--;
        bit--;
      }
    }
    wmask |= 1u << bit;
    bit--;
  }
  while (bit >= 0) {
    wmask |= 1u << bit;
    bit--;
  }
  int parity = __builtin_parity(wmask & 0xFF);
  return wmask & (unsigned)(-2 + (terminate ^ parity));
}

static void decode_weight_mask(unsigned int wmask, int num_models,
                               const unsigned char *ctx_masks, int *weights,
                               unsigned int *ext_masks) {
  int wval = 0;
  for (int n = 0; n < num_models; n++) {
    while (wmask & 0x80000000u) {
      wmask <<= 1;
      wval++;
    }
    wmask <<= 1;
    weights[n] = wval;
    ext_masks[n] = (unsigned int)ctx_masks[n] | (wmask & 0xFFFFFF00u);
  }
}

static void model_set_print(const ModelSet *ml, FILE *f) {
  for (int m = 0; m < ml->num_models; m++)
    fprintf(f, "%s%02X:%d", m ? " " : "", ml->models[m].mask,
            ml->models[m].weight);
  fprintf(f, "\n");
}

static void model_set_sprint(const ModelSet *ml, char *buf, int bufsize) {
  int pos = 0;
  for (int m = 0; m < ml->num_models && pos < bufsize - 6; m++)
    pos += snprintf(buf + pos, bufsize - pos, "%s%02X:%d", m ? " " : "",
                    ml->models[m].mask, ml->models[m].weight);
}

static CtxEntry *ctx_table_probe(CtxEntry *table, unsigned int table_size,
                                 unsigned char mask, const unsigned char *data,
                                 int bit_pos, unsigned int generation) {
  const unsigned char *dp = &data[bit_pos / 8];
  unsigned char bn = (unsigned char)(bit_pos & 7);
  for (unsigned int h = ctx_hash(data, bit_pos, mask);; h++) {
    CtxEntry *e = &table[h % table_size];
    if (e->generation != generation) {
      e->mask = mask;
      e->bitnum = bn;
      e->data_ptr = dp;
      e->prob[0] = 0;
      e->prob[1] = 0;
      e->generation = generation;
      return e;
    }
    if (e->mask == mask && e->bitnum == bn &&
        (dp[0] & (0xFF00 >> bn)) == (e->data_ptr[0] & (0xFF00 >> bn))) {
      int match = 1;
      for (int i = 0; i < 8; i++) {
        if (((mask >> i) & 1) && dp[i - 8] != e->data_ptr[i - 8]) {
          match = 0;
          break;
        }
      }
      if (match)
        return e;
    }
  }
}

static int compute_single_model(const unsigned char *data, int total_bits,
                                unsigned char mask, CtxEntry *ht, int ht_size,
                                CompactPredBlock *out_blocks, int *out_offsets,
                                unsigned int generation) {
  int max_blocks = (total_bits + PKG_BITS - 1) / PKG_BITS;
  int num_blocks = 0;

  for (int bi = 0; bi < max_blocks; bi++) {
    int base = bi * PKG_BITS;
    int needs_commit = 0;
    for (int off = 0; off < PKG_BITS; off++) {
      float rp = 0, tp = 0;
      if (base + off < total_bits) {
        int bit = get_bit(data, base + off);
        CtxEntry *e =
            ctx_table_probe(ht, ht_size, mask, data, base + off, generation);
        int boost = (e->prob[0] == 0 || e->prob[1] == 0) ? 2 : 0;
        if (e->prob[0] || e->prob[1])
          needs_commit = 1;
        rp = (float)(e->prob[bit] << boost);
        tp = (float)((e->prob[0] + e->prob[1]) << boost);
        counter_update(e->prob, bit);
      }
      uint16_t *packed = (uint16_t *)&out_blocks[num_blocks].prob[off >> 2];
      int ri, ti;
      memcpy(&ri, &rp, sizeof(int));
      memcpy(&ti, &tp, sizeof(int));
      packed[off & 3] = (uint16_t)(ri >> 16);
      packed[4 + (off & 3)] = (uint16_t)(ti >> 16);
    }
    out_offsets[num_blocks] = bi;
    if (needs_commit)
      num_blocks++;
  }

  return num_blocks;
}

static void eval_setup(Evaluator *ev, const CompState *cs, int length,
                       int base_prob, float log_scale) {
  ev->cs = cs;
  ev->log_scale = log_scale;

  int nblocks = (length + PKG_BITS - 1) / PKG_BITS;
  ev->accum = (PredBlock *)alloc_aligned(nblocks * sizeof(PredBlock));
  ev->accum_sizes = (unsigned *)malloc(nblocks * sizeof(unsigned));

  float bv = base_prob * log_scale;
  float tv = base_prob * 2 * log_scale;
  v4f base_v = v4f_splat(bv);
  v4f total_v = v4f_splat(tv);

  for (int i = 0; i < nblocks; i++) {
    for (int j = 0; j < NPV; j++) {
      ev->accum[i].prob[j][0] = base_v;
      ev->accum[i].prob[j][1] =
          (i * PKG_BITS + j * 4 < length) ? total_v : base_v;
    }
    int remain = length - i * PKG_BITS;
    if (remain > PKG_BITS)
      remain = PKG_BITS;
    ev->accum_sizes[i] = remain * TPREC;
  }
  ev->cost = (int64_t)length << TPREC_BITS;
}

static void eval_destroy(const Evaluator *ev) {
  free_aligned(ev->accum);
  free(ev->accum_sizes);
}

static inline int eval_get_size(const Evaluator *ev) {
  return (int)(ev->cost / (TPREC / BIT_PREC));
}

static int64_t eval_adjust(const Evaluator *ev, int model_idx, int delta) {
  const ModelPred *mp = &ev->cs->models[model_idx];
  const int nblocks = mp->num_blocks;
  const CompactPredBlock *mblocks = &ev->cs->block_arena[mp->offset];
  const int *mmap = &ev->cs->map_arena[mp->offset];

  float dw = delta * ev->log_scale;
  const v4f vdw = v4f_splat(dw);
  const v4f vone = v4f_splat(1);

  static const float C0 = 1.42286530448213f;
  static const float C1 = -0.58208536795165f;
  static const float C2 = 0.15922006346951f;
  const v4f vc0 = v4f_splat(C0);
  const v4f vc1 = v4f_splat(C1);
  const v4f vc2 = v4f_splat(C2);
  const v4f vprec = v4f_splat(TPREC);
  const v4u vmant = v4u_splat(0x7FFFFFu);
  const v4u vexp1 = v4u_splat(0x3F800000u);

  PredBlock *accum = ev->accum;
  unsigned *sizes = ev->accum_sizes;
  int64_t delta_cost = 0;

  for (int bi = 0; bi < nblocks; bi++) {
    const int off = mmap[bi];
    PredBlock *sum = &accum[off];
    const CompactPredBlock *mb = &mblocks[bi];

    v4f prod_r = vone, prod_t = vone;
    const v8u16 vzero16 = {};

#pragma GCC unroll 16
    for (int I = 0; I < NPV; I++) {
      v8u16 packed;
      memcpy(&packed, mb->prob[I], 16);
      v8u16 ri =
          __builtin_shufflevector(vzero16, packed, 0, 8, 1, 9, 2, 10, 3, 11);
      v8u16 ti =
          __builtin_shufflevector(vzero16, packed, 4, 12, 5, 13, 6, 14, 7, 15);
      v4f fr;
      memcpy(&fr, &ri, 16);
      v4f ft;
      memcpy(&ft, &ti, 16);

      v4f sr = sum->prob[I][0] + fr * vdw;
      v4f st = sum->prob[I][1] + ft * vdw;
      sum->prob[I][0] = sr;
      sum->prob[I][1] = st;
      prod_r *= sr;
      prod_t *= st;
    }

    v4u rb = (v4u)prod_r, tb = (v4u)prod_t;
    v4i rexp = (v4i)(rb >> 23), texp = (v4i)(tb >> 23);
    v4f rmf = (v4f)((rb & vmant) | vexp1);
    v4f tmf = (v4f)((tb & vmant) | vexp1);
    v4f rx = rmf - vone, tx = tmf - vone;
    v4f rlog = rx * (vc0 + rx * (vc1 + rx * vc2));
    v4f tlog = tx * (vc0 + tx * (vc1 + tx * vc2));
    v4f frac_f = (tlog - rlog) * vprec;
    v4i exp_diff = (texp - rexp) << TPREC_BITS;
    v4u sign = (v4u)frac_f & v4u_splat(0x80000000u);
    v4f bias = (v4f)(sign | v4u_splat(0x3F000000u));
    v4i frac_i = __builtin_convertvector(frac_f + bias, v4i);
    v4i sz = exp_diff + frac_i;

    int new_sz = sz[0] + sz[1] + sz[2] + sz[3];
    int old_sz = sizes[off];
    sizes[off] = new_sz;
    delta_cost += new_sz - old_sz;
  }
  return delta_cost;
}

static int64_t eval_evaluate(Evaluator *ev, const ModelSet *ml) {
  int new_w[MAX_MODELS] = {};
  for (int i = 0; i < ml->num_models; i++)
    new_w[ml->models[i].mask] = 1 << ml->models[i].weight;

  for (int i = 0; i < MAX_MODELS; i++) {
    if (new_w[i] != ev->active_weights[i]) {
      int64_t d = eval_adjust(ev, i, new_w[i] - ev->active_weights[i]);
      if (ev->active_weights[i] == 0)
        ev->cost += 8 * TPREC;
      else if (new_w[i] == 0)
        ev->cost -= 8 * TPREC;
      ev->active_weights[i] = new_w[i];
      ev->cost += d;
    }
  }
  return ev->cost;
}

static CompState *state_new(const unsigned char *data, int size, int base_prob,
                            Evaluator *eval, const unsigned char *ctx) {
  CompState *cs = (CompState *)calloc(1, sizeof(CompState));
  cs->eval = eval;
  int total_bits = size * 8;
  float log_scale = 1.0f / 2048.0f;

  unsigned char *padded = alloc_padded(ctx, data, size);
  unsigned char *dp = padded + MAX_CTX;
  assert(base_prob >= 9);

  int max_blocks_per_model = (total_bits + PKG_BITS - 1) / PKG_BITS;
  int arena_capacity = 256 * max_blocks_per_model;
  cs->block_arena = (CompactPredBlock *)alloc_aligned(arena_capacity *
                                                      sizeof(CompactPredBlock));
  cs->map_arena = (int *)malloc(arena_capacity * sizeof(int));

  int ht_size = prev_prime(total_bits * 2);
  CtxEntry *ht = (CtxEntry *)calloc(ht_size, sizeof(CtxEntry));

  int cursor = 0;
  for (int m = 0; m < 256; m++) {
    cs->models[m].offset = cursor;
    cs->models[m].num_blocks = compute_single_model(
        dp, total_bits, (unsigned char)m, ht, ht_size, &cs->block_arena[cursor],
        &cs->map_arena[cursor], m + 1);
    cursor += cs->models[m].num_blocks;
  }
  free(ht);
  free(padded);

  eval_setup(eval, cs, total_bits, base_prob, log_scale);
  return cs;
}

static void state_destroy(CompState *cs) {
  free_aligned(cs->block_arena);
  free(cs->map_arena);
  free(cs);
}

static void hbs_init(HashBitStream *hb) { memset(hb, 0, sizeof(*hb)); }

static void hbs_free(const HashBitStream *hb) {
  free(hb->hashes);
  free(hb->bits);
  free(hb->weights);
}

static inline void hbs_push_hash(HashBitStream *hb, unsigned int v) {
  DYNPUSH(hb->hashes, hb->hashes_len, hb->hashes_cap, v, sizeof(unsigned int));
}

static inline void hbs_push_bit(HashBitStream *hb, int v) {
  DYNPUSH(hb->bits, hb->bits_len, hb->bits_cap, (unsigned char)v,
          sizeof(unsigned char));
}

static HashBitStream compute_hash_stream(const unsigned char *data, int size,
                                         unsigned char *ctx, const ModelSet *ml,
                                         int first, int finish) {
  int total_bits = first + size * 8;
  int num = ml->num_models;
  HashBitStream out;
  hbs_init(&out);
  out.num_weights = num;
  out.weights = (int *)malloc(num * sizeof(int));

  unsigned char *padded = alloc_padded(ctx, data, size);
  unsigned char *dp = padded + MAX_CTX;

  unsigned int ext_masks[MAX_SEARCH];
  unsigned char ctx_masks[MAX_SEARCH];
  unsigned int wmask = encode_weight_mask(ml, ctx_masks, finish);
  decode_weight_mask(wmask, num, ctx_masks, out.weights, ext_masks);

  if (first) {
    for (int m = 0; m < num; m++)
      hbs_push_hash(&out, ctx_hash_initial(ext_masks[m]));
    hbs_push_bit(&out, 1);
  }

  for (int bp = 0; bp < size * 8; bp++) {
    int bit = get_bit(dp, bp);
    for (int m = 0; m < num; m++)
      hbs_push_hash(&out, ctx_hash(dp, bp, ext_masks[m]));
    hbs_push_bit(&out, bit);
  }

  int copy = size < MAX_CTX ? size : MAX_CTX;
  if (copy > 0)
    memcpy(ctx + MAX_CTX - copy, dp + size - copy, copy);
  free(padded);
  return out;
}

/* Direct-mapped, lossy, no-probing encoder (original Crinkler scheme).
   Table is 2 bytes per slot (prob[0], prob[1]); slot = hash & dmask. Two
   different contexts that map to the same slot silently share a counter.
   When collect is set (verbose output or an HTML report was requested) the
   encoder also gathers per-model / per-bit statistics into the local
   accumulators and, if stats != NULL, copies them out. The timing loop in
   compress_4k runs with collect = 0 so the measurements stay clean. */
static void encode_from_stream_direct(ArithCoder *ac, const HashBitStream *hb,
                                      unsigned char *dt, unsigned int dmask,
                                      int base_prob, const ModelSet *ml,
                                      CompStats *stats, int collect) {
  int num = hb->num_weights;
  int total_bits = (num == 0) ? hb->bits_len : (int)hb->hashes_len / num;

  unsigned int model_hits[MAX_SEARCH] = {};
  unsigned int model_misses[MAX_SEARCH] = {};
  unsigned int min_range = 0xFFFFFFFFu;
  double model_bits_saved[MAX_SEARCH] = {};
  unsigned int conf_hist[11] = {};
  unsigned int calib_count[20] = {};
  unsigned int calib_ones[20] = {};
  double calib_psum[20] = {};
  unsigned int bytepos_count[8] = {};
  double bytepos_cost[8] = {};
  double total_cost = 0;

  unsigned char *matched[MAX_SEARCH];
  int hpos = 0;
  /* per-byte cost tracking for position curve */
  int num_data_bytes = (total_bits > 1) ? (total_bits - 1 + 7) / 8 : 0;
  float *byte_costs = NULL;
  float *byte_model_contrib = NULL;
  float *bit_costs = NULL;
  if (stats && num_data_bytes > 0) {
    byte_costs = (float *)calloc(num_data_bytes, sizeof(float));
    byte_model_contrib = (float *)calloc(
        (size_t)num_data_bytes * num, sizeof(float));
    bit_costs = (float *)calloc((size_t)num_data_bytes * 8, sizeof(float));
  }
  /* table occupancy growth: fresh-slot activations, sampled periodically */
  unsigned int occ_run = 0;
  unsigned int occ_samps[257];
  int occ_n = 0;
  int occ_interval = total_bits / 256 + 1;
  for (int bp = 0; bp < total_bits; bp++) {
    int bit = hb->bits[bp];
    unsigned int probs[2] = {(unsigned)base_prob, (unsigned)base_prob};
    for (int m = 0; m < num; m++) {
      unsigned int h = hb->hashes[hpos++];
      unsigned char *e = &dt[2u * (h >> (32 - direct_bits))];
      unsigned int p0_before = probs[0], p1_before = probs[1];
      unsigned int shift =
          (1 - (((e[0] + 255) & (e[1] + 255)) >> 8)) * 2 + hb->weights[m];
      probs[0] += (unsigned)e[0] << shift;
      probs[1] += (unsigned)e[1] << shift;
      matched[m] = e;
      if (collect) {
        /* A non-empty slot means this context (or a colliding one) has been
           seen before and is actually contributing a prediction. */
        if (e[0] || e[1]) {
          model_hits[m]++;
          unsigned int before_correct = bit ? p1_before : p0_before;
          unsigned int before_total = p0_before + p1_before;
          unsigned int after_correct = probs[bit];
          unsigned int after_total = probs[0] + probs[1];
          float delta =
              fast_log2f((float)after_correct / after_total) -
              fast_log2f((float)before_correct / before_total);
          model_bits_saved[m] += delta;
          if (byte_model_contrib && bp > 0) {
            int byte_idx = (bp - 1) / 8;
            if (byte_idx < num_data_bytes)
              byte_model_contrib[byte_idx * num + m] += delta;
          }
        } else {
          model_misses[m]++;
          occ_run++; /* empty slot: about to be occupied by this bit's update */
        }
      }
    }
    if (collect) {
      float conf = (float)probs[bit] / (probs[0] + probs[1]);
      int bucket = (conf < 0.5f) ? 0 : (int)((conf - 0.5f) * 20) + 1;
      if (bucket > 10)
        bucket = 10;
      conf_hist[bucket]++;

      /* calibration: predicted P(bit=1) vs outcome, 5% buckets */
      double p1f = (double)probs[1] / (probs[0] + probs[1]);
      int cb = (int)(p1f * 20);
      if (cb > 19)
        cb = 19;
      calib_count[cb]++;
      calib_ones[cb] += (unsigned)bit;
      calib_psum[cb] += p1f;

      float bit_cost = -fast_log2f((float)probs[bit] / (probs[0] + probs[1]));
      total_cost += bit_cost;
      int bpos = bp & 7;
      bytepos_count[bpos]++;
      bytepos_cost[bpos] += bit_cost;
      if (byte_costs && bp > 0) {
        int byte_idx = (bp - 1) / 8;
        if (byte_idx < num_data_bytes)
          byte_costs[byte_idx] += bit_cost;
      }
      if (bit_costs && bp > 0) {
        int byte_idx = (bp - 1) / 8;
        if (byte_idx < num_data_bytes)
          bit_costs[byte_idx * 8 + ((bp - 1) & 7)] = bit_cost;
      }
      if ((bp + 1) % occ_interval == 0 && occ_n < 256)
        occ_samps[occ_n++] = occ_run;
    }
    arith_encode(ac, probs[1], probs[0], 1 - bit);
    if (collect && ac->range < min_range)
      min_range = ac->range;
    for (int m = 0; m < num; m++) {
      unsigned char *e = matched[m];
      e[bit] += 1;
      if (e[1 - bit] > 1)
        e[1 - bit] >>= 1;
    }
  }

  if (collect && verbose) {
    printf("  Arithmetic coder min range: 0x%08X (%.1f effective bits)\n",
           min_range, 31.0 - __builtin_clz(min_range));

    printf("  Prediction confidence:\n");
    const char *labels[] = {"   <50%%", " 50-55%%", " 55-60%%", " 60-65%%",
                            " 65-70%%", " 70-75%%", " 75-80%%", " 80-85%%",
                            " 85-90%%", " 90-95%%", "95-100%%"};
    for (int i = 0; i < 11; i++) {
      if (conf_hist[i] > 0)
        printf("    %s: %5u bits (%5.1f%%)\n", labels[i], conf_hist[i],
               100.0 * conf_hist[i] / total_bits);
    }
    printf("  Total prediction cost: %.1f bits (%.1f bytes)\n", total_cost,
           total_cost / 8.0);

    printf("  Byte position analysis:\n");
    for (int i = 0; i < 8; i++) {
      if (bytepos_count[i] > 0)
        printf("    Bit %d: %5u bits, avg cost %.3f bits/bit\n", i,
               bytepos_count[i], bytepos_cost[i] / bytepos_count[i]);
    }

    printf("  Per-model stats:\n");
    double total_saved = 0;
    for (int m = 0; m < num; m++) {
      unsigned int total = model_hits[m] + model_misses[m];
      printf("    Model %2d (mask %02X, w%d): %5u hits (%5.1f%%), "
             "%5u unique ctx, %8.1f bits (%6.1f bytes)\n",
             m, ml->models[m].mask, ml->models[m].weight, model_hits[m],
             total ? 100.0 * model_hits[m] / total : 0.0, model_misses[m],
             -model_bits_saved[m], -model_bits_saved[m] / 8.0);
      total_saved += model_bits_saved[m];
    }
    printf("    Total model contribution: %.1f bits (%.1f bytes)\n",
           -total_saved, -total_saved / 8.0);

    int order[MAX_SEARCH];
    for (int m = 0; m < num; m++)
      order[m] = m;
    for (int i = 0; i < num - 1; i++)
      for (int j = i + 1; j < num; j++)
        if (model_bits_saved[order[i]] < model_bits_saved[order[j]]) {
          int tmp = order[i];
          order[i] = order[j];
          order[j] = tmp;
        }
    printf("  Per-model stats (by bits saved):\n");
    for (int i = 0; i < num; i++) {
      int m = order[i];
      unsigned int total = model_hits[m] + model_misses[m];
      printf("    Model %2d (mask %02X, w%d): %5u hits (%5.1f%%), "
             "%5u unique ctx, %8.1f bits (%6.1f bytes)\n",
             m, ml->models[m].mask, ml->models[m].weight, model_hits[m],
             total ? 100.0 * model_hits[m] / total : 0.0, model_misses[m],
             -model_bits_saved[m], -model_bits_saved[m] / 8.0);
    }
  }

  if (stats) {
    int snum = num < MAX_SEARCH ? num : MAX_SEARCH;
    stats->num_models = snum;
    stats->total_bits = total_bits;
    stats->total_cost = total_cost;
    stats->min_range = min_range;
    memcpy(stats->conf_hist, conf_hist, sizeof(conf_hist));
    memcpy(stats->calib_count, calib_count, sizeof(calib_count));
    memcpy(stats->calib_ones, calib_ones, sizeof(calib_ones));
    memcpy(stats->calib_psum, calib_psum, sizeof(calib_psum));
    memcpy(stats->bytepos_count, bytepos_count, sizeof(bytepos_count));
    memcpy(stats->bytepos_cost, bytepos_cost, sizeof(bytepos_cost));
    for (int m = 0; m < snum; m++) {
      stats->model_masks[m] = ml->models[m].mask;
      stats->model_weights[m] = ml->models[m].weight;
      stats->model_hits[m] = model_hits[m];
      stats->model_misses[m] = model_misses[m];
      stats->model_bits_saved[m] = model_bits_saved[m];
    }
    /* Direct-mapped table occupancy + counter saturation (no probing). */
    stats->ht_size = dmask + 1;
    stats->ht_occupied = 0;
    stats->sat_lopsided = 0;
    stats->sat_strong = 0;
    stats->sat_balanced = 0;
    stats->sat_mixed = 0;
    for (unsigned int i = 0; i <= dmask; i++) {
      unsigned char *e = &dt[2u * i];
      unsigned int p0 = e[0], p1 = e[1];
      if (p0 || p1) {
        stats->ht_occupied++;
        if (p0 == 0 || p1 == 0)
          stats->sat_lopsided++;
        else if (p0 > p1 * 4 || p1 > p0 * 4)
          stats->sat_strong++;
        else if (p0 <= p1 * 2 && p1 <= p0 * 2)
          stats->sat_balanced++;
        else
          stats->sat_mixed++;
      }
    }
    stats->byte_costs = byte_costs;
    stats->byte_model_contrib = byte_model_contrib;
    stats->bit_costs = bit_costs;
    if (occ_n < 257)
      occ_samps[occ_n++] = occ_run; /* final value */
    memcpy(stats->occ_samples, occ_samps, occ_n * sizeof(unsigned int));
    stats->occ_nsamples = occ_n;
    stats->occ_interval = occ_interval;
    stats->num_data_bytes = num_data_bytes;
    stats->valid = 1;
  } else {
    free(byte_costs);
    free(byte_model_contrib);
    free(bit_costs);
  }
}

static int compress_4k(const unsigned char *data, int size, unsigned char *out,
                       const ModelSet *ml, int base_prob, CompStats *stats) {
  unsigned char ctx[MAX_CTX] = {};
  HashBitStream hb = compute_hash_stream(data, size, ctx, ml, 1, 1);
  int reps = timing_reps < 1 ? 1 : timing_reps;
  ArithCoder ac;
  int total = 0;

  unsigned int dmask = (1u << direct_bits) - 1u;
  size_t dbytes = ((size_t)dmask + 1) * 2;
  unsigned char *dt = (unsigned char *)calloc(dbytes, 1);
  if (!dt) {
    fprintf(stderr, "calloc(%zu) failed for direct-mapped table\n", dbytes);
    exit(1);
  }
  if (verbose)
    printf("  Direct-mapped table: %u slots (%zu bytes, 2 B/slot)\n",
           dmask + 1, dbytes);

  /* Timed passes: encode only, no statistics, for stable timing. */
  double t0 = mono_sec();
  for (int r = 0; r < reps; r++) {
    if (r > 0)
      memset(dt, 0, dbytes); /* re-zero only for subsequent timing reps */
    memset(out, 0, (size_t)size + 1024); /* coder XORs into dest */
    arith_init(&ac, out);
    encode_from_stream_direct(&ac, &hb, dt, dmask, base_prob, ml, NULL, 0);
  }
  g_encode_ms = (mono_sec() - t0) / reps * 1000.0;
  total = arith_finish(&ac);

  /* One extra, untimed pass to gather verbose / HTML statistics. */
  if (verbose || stats) {
    memset(dt, 0, dbytes);
    memset(out, 0, (size_t)size + 1024);
    arith_init(&ac, out);
    encode_from_stream_direct(&ac, &hb, dt, dmask, base_prob, ml, stats, 1);
    total = arith_finish(&ac);
  }
  free(dt);

  hbs_free(&hb);
  return total;
}

static unsigned int real_compress_size(const unsigned char *data, int size,
                                       const ModelSet *ml, int base_prob) {
  int max_out = size + 1024;
  unsigned char *out = (unsigned char *)calloc(max_out, 1);
  int comp_bits = compress_4k(data, size, out, ml, base_prob, NULL);
  free(out);
  int header_bits = (hdr_base_bytes() + ml->num_models) * 8;
  return (unsigned int)(header_bits + comp_bits) * BIT_PREC;
}

/* Inverse of encode_from_stream_direct: same direct-mapped lossy table. */
static int decompress_4k_direct(const unsigned char *cdata, unsigned char *out,
                                int base_prob) {
  int bitlen = 0;
  memcpy(&bitlen, cdata, hdr_bitlen_bytes()); /* 2 or 4 little-endian bytes */
  unsigned int stored_wmask;
  memcpy(&stored_wmask, cdata + hdr_bitlen_bytes(),
         4); /* wmask follows bitlen */
  int num = wmask_num_models(stored_wmask);
  int data_bytes = (bitlen - 1) / 8;

  unsigned char ctx_masks[MAX_SEARCH];
  for (int i = 0; i < num; i++)
    ctx_masks[i] = cdata[hdr_base_bytes() + i];
  unsigned int ext_masks[MAX_SEARCH];
  int weights[MAX_SEARCH];
  decode_weight_mask(stored_wmask, num, ctx_masks, weights, ext_masks);

  const unsigned char *comp = cdata + hdr_base_bytes() + num;
  unsigned char *buf = (unsigned char *)calloc(data_bytes + MAX_CTX, 1);
  unsigned char *dp = buf + MAX_CTX;

  unsigned int dmask = (1u << direct_bits) - 1u;
  unsigned char *dt = (unsigned char *)calloc((size_t)dmask + 1, 2);
  if (!dt) {
    fprintf(stderr, "calloc failed for direct-mapped table\n");
    exit(1);
  }

  unsigned int range = 0x80000000u, low = 0, value = 0;
  int cpos = 0;
  for (int i = 0; i < 31; i++)
    value = (value << 1) | get_compressed_bit(comp, cpos++);

  for (int bp = 0; bp < bitlen; bp++) {
    unsigned int probs[2] = {(unsigned)base_prob, (unsigned)base_prob};
    unsigned char *matched[MAX_SEARCH];
    for (int m = 0; m < num; m++) {
      unsigned int h = (bp == 0) ? ctx_hash_initial(ext_masks[m])
                                 : ctx_hash(dp, bp - 1, ext_masks[m]);
      unsigned char *e = &dt[2u * (h >> (32 - direct_bits))];
      unsigned int shift =
          (1 - (((e[0] + 255) & (e[1] + 255)) >> 8)) * 2 + weights[m];
      probs[0] += (unsigned)e[0] << shift;
      probs[1] += (unsigned)e[1] << shift;
      matched[m] = e;
    }

    unsigned int total = probs[0] + probs[1];
    unsigned int thresh = (uint64_t)range * probs[1] / total;
    unsigned int diff = value - low;
    int bit;
    if (diff < thresh) {
      range = thresh;
      bit = 1;
    } else {
      low += thresh;
      range -= thresh;
      bit = 0;
    }
    while (!(range & 0x80000000u)) {
      low <<= 1;
      range <<= 1;
      value = (value << 1) | get_compressed_bit(comp, cpos++);
    }
    for (int m = 0; m < num; m++) {
      unsigned char *e = matched[m];
      e[bit] += 1;
      if (e[1 - bit] > 1)
        e[1 - bit] >>= 1;
    }
    if (bp > 0 && bit) {
      int dbp = bp - 1;
      dp[dbp >> 3] |= 1 << (7 - (dbp & 7));
    }
  }

  memcpy(out, dp, data_bytes);
  free(buf);
  free(dt);
  return data_bytes;
}

static unsigned int approximate_weights(CompState *cs, ModelSet *ml,
                                        const unsigned char *data, int size,
                                        int base_prob) {
  for (int i = 0; i < ml->num_models; i++)
    ml->models[i].weight = __builtin_popcount(ml->models[i].mask);
  if (extreme)
    return real_compress_size(data, size, ml, base_prob);
  eval_evaluate(cs->eval, ml);
  return eval_get_size(cs->eval);
}

static unsigned int optimize_weights(CompState *cs, ModelSet *ml,
                                     const unsigned char *data, int size,
                                     int base_prob) {
  ModelSet cand = *ml;
  int idx = ml->num_models - 1;
  int dir = 1;
  int last_improved = idx;
  unsigned int best = approximate_weights(cs, ml, data, size, base_prob);
  if (ml->num_models == 0)
    return best;

  do {
    cand = *ml;
    cand.models[idx].weight += dir;
    int improved = 0;
    if (cand.models[idx].weight <= MAX_WEIGHT &&
        cand.models[idx].weight != 255) {
      unsigned int trial;
      if (extreme) {
        trial = real_compress_size(data, size, &cand, base_prob);
      } else {
        eval_evaluate(cs->eval, &cand);
        trial = eval_get_size(cs->eval);
      }
      if (trial < best) {
        if (verbose >= 2)
          printf("    weight %02X: %d->%d, est %.1f -> %.1f bytes\n",
                 ml->models[idx].mask, ml->models[idx].weight,
                 cand.models[idx].weight, best / (float)(BIT_PREC * 8),
                 trial / (float)(BIT_PREC * 8));
        best = trial;
        *ml = cand;
        last_improved = idx;
        improved = 1;
      }
    }
    if (!improved) {
      if (dir == 1 && ml->models[idx].weight > 0) {
        dir = -1;
      } else {
        dir = 1;
        idx--;
        if (idx < 0)
          idx = ml->num_models - 1;
        if (idx == last_improved)
          break;
      }
    }
  } while (1);
  return best;
}

static unsigned int try_weights(CompState *cs, ModelSet *ml, int simple,
                                const unsigned char *data, int size,
                                int base_prob) {
  return simple ? approximate_weights(cs, ml, data, size, base_prob)
                : optimize_weights(cs, ml, data, size, base_prob);
}

static int model_set_cmp(const void *a, const void *b) {
  int sa = ((const ModelSet *)a)->size;
  int sb = ((const ModelSet *)b)->size;
  return (sa > sb) - (sa < sb);
}

static ModelSet search_best_models(const unsigned char *data, int size,
                                   const unsigned char ctx[MAX_CTX], int beam,
                                   int simple, int base_prob, int *out_size,
                                   const ModelSet *seed, CompStats *stats) {
  const int EFLAG = INT_MIN;
  const int nsets = beam * 2;
  ModelSet *sets = (ModelSet *)calloc(nsets, sizeof(ModelSet));
  Evaluator eval = {0};
  CompState *cs = extreme ? NULL : state_new(data, size, base_prob, &eval, ctx);

  unsigned char rev_masks[256];
  for (int m = 0; m <= 255; m++) {
    int v = m;
    v = ((v & 0x0F) << 4) | ((v & 0xF0) >> 4);
    v = ((v & 0x33) << 2) | ((v & 0xCC) >> 2);
    v = ((v & 0x55) << 1) | ((v & 0xAA) >> 1);
    rev_masks[m] = (unsigned char)v;
  }

  if (seed && seed->num_models > 0) {
    sets[0] = *seed;
    if (extreme) {
      sets[0].size =
          real_compress_size(data, size, &sets[0], base_prob) | EFLAG;
    } else {
      eval_evaluate(cs->eval, &sets[0]);
      sets[0].size = eval_get_size(cs->eval) | EFLAG;
    }
  } else {
    if (extreme) {
      ModelSet empty = {0};
      sets[0].size = real_compress_size(data, size, &empty, base_prob) | EFLAG;
    } else {
      sets[0].size = eval_get_size(cs->eval) | EFLAG;
    }
  }
  for (int s = 1; s < beam; s++)
    sets[s].size = INT_MAX;

  int masks_tried = 0, masks_accepted = 0;

  /* search trajectory logging */
  int log_search = (stats != NULL);
  if (log_search) {
    stats->search_best = (float *)malloc(256 * sizeof(float));
    stats->search_len = 0;
    stats->search_events = NULL;
    stats->search_nevents = 0;
    stats->search_events_cap = 0;
    memset(stats->mask_outcome, 0, sizeof(stats->mask_outcome));
    memset(stats->mask_delta, 0, sizeof(stats->mask_delta));
  }

  for (int mi = 0; mi <= 255; mi++) {
    int mask = rev_masks[mi];
    int mask_helped = 0;

    for (int s = 0; s < beam; s++) {
      ModelSet *cur = &sets[s];
      ModelSet *next = &sets[beam + s];
      next->size = INT_MAX;
      if (cur->size == INT_MAX)
        continue;

      int used = 0;
      for (int m = 0; m < cur->num_models; m++) {
        if (cur->models[m].mask == mask) {
          used = 1;
          break;
        }
      }

      if (!used && cur->num_models < (int)MAX_SEARCH) {
        masks_tried++;
        *next = *cur;
        next->models[cur->num_models].mask = (unsigned char)mask;
        next->models[cur->num_models].weight = 0;
        next->num_models = cur->num_models + 1;

        int old_sz = cur->size & ~EFLAG;
        if (verbose >= 2)
          printf("    -- try adding %02X:\n", mask);
        int new_sz = try_weights(cs, next, simple, data, size, base_prob);

        /* per-mask outcome for the mask grid (slot-0 lineage only) */
        if (log_search && s == 0 && new_sz < INT_MAX) {
          float d = (new_sz - old_sz) / (float)(BIT_PREC * 8);
          if (new_sz < old_sz) {
            stats->mask_outcome[mask] = 2;
            stats->mask_delta[mask] = d;
          } else if (stats->mask_outcome[mask] == 0) {
            stats->mask_outcome[mask] = 1;
            stats->mask_delta[mask] = d;
          }
        }

        if (new_sz < old_sz || beam > 1) {
          int best_sz = new_sz;

          if (verbose && new_sz < INT_MAX) {
            char setbuf[512] = "";
            model_set_sprint(next, setbuf, sizeof(setbuf));
            printf("  +mask %02X -> %2d models, est %.1f bytes [%s]\n", mask,
                   next->num_models, new_sz / (float)(BIT_PREC * 8), setbuf);
            fflush(stdout);
            mask_helped = 1;
          }
          if (log_search && new_sz < INT_MAX && s == 0) {
            if (stats->search_nevents >= stats->search_events_cap) {
              stats->search_events_cap =
                  stats->search_events_cap ? stats->search_events_cap * 2 : 64;
              stats->search_events = realloc(
                  stats->search_events,
                  stats->search_events_cap * sizeof(stats->search_events[0]));
            }
            int idx = stats->search_nevents++;
            stats->search_events[idx].mask_idx = mi;
            stats->search_events[idx].mask = (unsigned char)mask;
            stats->search_events[idx].num_models = next->num_models;
            stats->search_events[idx].est_bytes =
                new_sz / (float)(BIT_PREC * 8);
            stats->search_events[idx].is_removal = 0;
          }

          for (int m = next->num_models - 2; m >= 0; m--) {
            Model removed = next->models[m];
            next->num_models--;
            next->models[m] = next->models[next->num_models];
            if (verbose >= 2)
              printf("    -- try removing %02X:\n", removed.mask);
            int trial = try_weights(cs, next, simple, data, size, base_prob);
            if (trial < best_sz) {
              if (verbose) {
                char setbuf[512] = "";
                model_set_sprint(next, setbuf, sizeof(setbuf));
                printf("  -mask %02X -> %2d models, est %.1f bytes [%s]\n",
                       removed.mask, next->num_models,
                       trial / (float)(BIT_PREC * 8), setbuf);
                fflush(stdout);
              }
              if (log_search && s == 0) {
                if (stats->search_nevents >= stats->search_events_cap) {
                  stats->search_events_cap = stats->search_events_cap
                                                 ? stats->search_events_cap * 2
                                                 : 64;
                  stats->search_events =
                      realloc(stats->search_events,
                              stats->search_events_cap *
                                  sizeof(stats->search_events[0]));
                }
                int idx = stats->search_nevents++;
                stats->search_events[idx].mask_idx = mi;
                stats->search_events[idx].mask = removed.mask;
                stats->search_events[idx].num_models = next->num_models;
                stats->search_events[idx].est_bytes =
                    trial / (float)(BIT_PREC * 8);
                stats->search_events[idx].is_removal = 1;
              }
              best_sz = trial;
            } else {
              next->models[m] = removed;
              next->num_models++;
            }
          }
          next->size = best_sz;
          if ((cur->size & EFLAG) && new_sz < old_sz) {
            cur->size &= ~EFLAG;
            next->size |= EFLAG;
          }
        } else {
          next->size = INT_MAX;
        }
      }
    }

    qsort(sets, nsets, sizeof(ModelSet), model_set_cmp);
    if (mask_helped)
      masks_accepted++;
    if (log_search) {
      int best_raw = sets[0].size & ~EFLAG;
      stats->search_best[mi] = best_raw / (float)(BIT_PREC * 8);
      stats->search_len = mi + 1;
    }
    if (!verbose) {
      printf("\rCalculating models... %d/%d", mi + 1, 256);
      fflush(stdout);
    }
  }

  assert((sets[0].size & EFLAG) != 0);
  sets[0].size &= ~EFLAG;
  qsort(sets, nsets, sizeof(ModelSet), model_set_cmp);

  ModelSet best = sets[0];

  if (verbose) {
    printf("\n  Search: %d masks tried, %d accepted, %d rejected\n",
           masks_tried, masks_accepted, masks_tried - masks_accepted);
    printf("  Final weight optimization:\n");
  }
  int final_sz = optimize_weights(cs, &best, data, size, base_prob);
  if (log_search)
    for (int m = 0; m < best.num_models; m++)
      stats->mask_outcome[best.models[m].mask] = 3;
  if (out_size)
    *out_size = final_sz;
  if (verbose) {
    char setbuf[512] = "";
    model_set_sprint(&best, setbuf, sizeof(setbuf));
    printf("  Final: %2d models, est %.1f bytes [%s]\n", best.num_models,
           final_sz / (float)(BIT_PREC * 8), setbuf);
  }

  if (!extreme) {
    state_destroy(cs);
    eval_destroy(&eval);
  }
  free(sets);
  return best;
}

static void write_html_report(const char *path, const CompStats *s) {
  FILE *f = fopen(path, "w");
  if (!f) {
    fprintf(stderr, "Failed to open HTML output '%s'\n", path);
    return;
  }

  double ratio = 100.0 * s->total_bytes / (double)s->input_size;
  double savings = 100.0 - ratio;
  float actual_bpb =
      s->input_size > 0 ? (float)s->total_bytes * 8.0f / s->input_size : 0;
  float est_delta = (float)s->total_bytes - s->estimated_bytes;
  float est_delta_pct =
      s->estimated_bytes > 0 ? 100.0f * est_delta / s->estimated_bytes : 0;
  float context_gain = (float)s->entropy - actual_bpb;

  /* Order-1 conditional entropy H1 = H(next byte | previous byte), from
     bigram counts. Shows how much of the win order-1 modeling explains. */
  double h1 = -1;
  if (s->input_data && s->input_size > 1) {
    unsigned int *bg = (unsigned int *)calloc(65536, sizeof(unsigned int));
    unsigned int row_tot[256] = {0};
    for (int i = 0; i + 1 < s->input_size; i++) {
      bg[s->input_data[i] * 256 + s->input_data[i + 1]]++;
      row_tot[s->input_data[i]]++;
    }
    double pairs = (double)(s->input_size - 1);
    h1 = 0;
    for (int r = 0; r < 256; r++) {
      if (!row_tot[r])
        continue;
      for (int c = 0; c < 256; c++) {
        unsigned int v = bg[r * 256 + c];
        if (v)
          h1 -= (v / pairs) * fast_log2f((float)v / row_tot[r]);
      }
    }
    free(bg);
  }

  /* input fingerprint (CRC32-C) for run-to-run provenance */
  unsigned int input_crc = 0;
  if (s->input_data) {
    unsigned int c = 0xFFFFFFFFu;
    for (int i = 0; i < s->input_size; i++)
      c = _mm_crc32_u8(c, s->input_data[i]);
    input_crc = c ^ 0xFFFFFFFFu;
  }

  char timebuf[64] = "";
  {
    time_t now = time(NULL);
    struct tm *tmv = localtime(&now);
    if (tmv)
      strftime(timebuf, sizeof(timebuf), "%Y-%m-%d %H:%M:%S", tmv);
  }

  /* ── HTML head + CSS ── */
  fprintf(f,
    "<!DOCTYPE html>\n"
    "<!--\n"
    "    #####    ###    #####\n"
    "    ##  ##  ## ##  ##  ##\n"
    "    #####  ####### ##  ##\n"
    "    ##     ##   ## ##  ##\n"
    "    ##     ##   ##  #####\n"
    "                        ##\n"
    "    you found the source. we like you.\n"
    "    69 64 64 71 64\n"
    "-->\n"
    "<html lang=\"en\"><head><meta charset=\"UTF-8\">\n"
    "<meta name=\"viewport\" content=\"width=device-width,initial-scale=1\">\n"
    "<title>Compression Report &ndash; %s</title>\n"
    "<link rel=\"preconnect\" href=\"https://fonts.googleapis.com\">\n"
    "<link href=\"https://fonts.googleapis.com/css2?"
    "family=DM+Sans:ital,wght@0,400;0,500;0,600;0,700;1,400"
    "&family=Press+Start+2P"
    "&family=JetBrains+Mono:wght@400;500;600&display=swap\" rel=\"stylesheet\">\n"
    "<style>\n"
    ":root{\n"
    "  --bg:#0c0e14;--bg2:#13161f;--bg3:#1a1e2b;--bg4:#242938;\n"
    "  --bdr:#2a2f3f;--bdr2:#353b4f;\n"
    "  --fg:#e8eaf0;--fg2:#a0a6b8;--fg3:#6b7186;\n"
    "  --acc:#22d3ee;--acc2:#06b6d4;--acc3:#0891b2;\n"
    "  --grn:#34d399;--grn2:#10b981;\n"
    "  --red:#f87171;--red2:#ef4444;\n"
    "  --ylw:#fbbf24;--ylw2:#f59e0b;\n"
    "  --orn:#fb923c;--blu:#60a5fa;\n"
    "  --mono:'JetBrains Mono',ui-monospace,'Cascadia Code','Consolas',monospace;\n"
    "  --sans:'DM Sans',system-ui,-apple-system,sans-serif;\n"
    "  --pix:'Press Start 2P',monospace;\n"
    "  --pnk:#ff3fd8;--yel:#ffe33f;\n"
    "  --shadow:0 1px 2px rgba(0,0,0,.25),0 14px 34px -18px rgba(0,0,0,.55);\n"
    "}\n"
    "body.light{\n"
    "  --bg:#f6f7fa;--bg2:#ffffff;--bg3:#eef1f5;--bg4:#e0e4ec;\n"
    "  --bdr:#d6dae3;--bdr2:#b8bdca;\n"
    "  --fg:#11141c;--fg2:#3b4156;--fg3:#7a8094;\n"
    "  --acc:#0891b2;--acc2:#06b6d4;--acc3:#0e7490;\n"
    "  --grn:#059669;--grn2:#10b981;\n"
    "  --red:#dc2626;--red2:#ef4444;\n"
    "  --ylw:#d97706;--ylw2:#f59e0b;\n"
    "  --orn:#ea580c;--blu:#2563eb;\n"
    "  --shadow:0 1px 2px rgba(15,23,42,.05),0 14px 34px -18px rgba(15,23,42,.16);\n"
    "}\n"
    "*{margin:0;padding:0;box-sizing:border-box}\n"
    "html{scroll-behavior:smooth}\n"
    ".card[id]{scroll-margin-top:20px}\n"
    "body{font-family:var(--sans);color:var(--fg);line-height:1.6;"
    "-webkit-font-smoothing:antialiased;background:"
    "radial-gradient(1100px 700px at 85%% -5%%,rgba(34,211,238,.06),transparent 60%%),"
    "radial-gradient(900px 700px at -5%% 25%%,rgba(96,165,250,.05),transparent 60%%),"
    "var(--bg);background-attachment:fixed}\n"
    "::selection{background:rgba(34,211,238,.28)}\n"
    "::-webkit-scrollbar{width:11px;height:11px}\n"
    "::-webkit-scrollbar-thumb{background:var(--bg4);border-radius:6px;"
    "border:3px solid var(--bg)}\n"
    "::-webkit-scrollbar-thumb:hover{background:var(--bdr2)}\n"
    "::-webkit-scrollbar-track{background:transparent}\n"
    ".wrap{padding:24px 28px 64px 28px;margin-left:200px;"
    "max-width:calc(100vw - 200px)}\n"
    "\n"
    "/* ── Sidebar ── */\n"
    ".sidebar{position:fixed;top:0;left:0;width:184px;height:100vh;"
    "background:var(--bg2);border-right:1px solid var(--bdr);"
    "padding:18px 0 12px;overflow-y:auto;z-index:100;"
    "display:flex;flex-direction:column;scrollbar-width:thin}\n"
    ".sb-brand{display:flex;align-items:center;gap:8px;padding:2px 16px 14px;"
    "border-bottom:1px solid var(--bdr);margin-bottom:8px}\n"
    ".sb-brand .dot{width:9px;height:9px;border-radius:50%%;flex-shrink:0;"
    "background:linear-gradient(135deg,var(--acc),var(--blu));"
    "box-shadow:0 0 10px rgba(34,211,238,.6)}\n"
    ".sb-brand span{font-size:12px;font-weight:700;color:var(--fg);"
    "letter-spacing:.4px}\n"
    ".sidebar .sb-title{font-size:9.5px;font-weight:700;color:var(--fg3);"
    "text-transform:uppercase;letter-spacing:1.4px;padding:0 16px;"
    "margin:12px 0 4px}\n"
    ".sidebar a{display:block;padding:5px 16px 5px 14px;font-size:12px;"
    "color:var(--fg3);text-decoration:none;border-left:2px solid transparent;"
    "border-radius:0 6px 6px 0;"
    "transition:color .15s,border-color .15s,background .15s,padding .15s;"
    "line-height:1.4}\n"
    ".sidebar a:hover{color:var(--fg);background:rgba(127,140,170,.08);"
    "padding-left:18px}\n"
    ".sidebar a.active{color:var(--acc);border-left-color:var(--acc);"
    "background:linear-gradient(90deg,rgba(34,211,238,.10),transparent)}\n"
    "@media(max-width:900px){.sidebar{display:none}.wrap{margin-left:0}}\n"
    "\n"
    "/* ── Hero ── */\n"
    ".hero{display:flex;align-items:center;gap:40px;padding:40px 0 36px;"
    "border-bottom:1px solid var(--bdr)}\n"
    ".hero-ring{position:relative;flex-shrink:0}\n"
    ".hero-ring svg{display:block}\n"
    ".hero-pct{position:absolute;inset:0;display:flex;flex-direction:column;"
    "align-items:center;justify-content:center}\n"
    ".hero-pct .big{font-size:32px;font-weight:700;letter-spacing:-1px;"
    "color:var(--acc);line-height:1;text-shadow:0 0 26px}\n"
    ".hero-pct .lbl{font-size:11px;color:var(--fg3);text-transform:uppercase;"
    "letter-spacing:1.5px;margin-top:4px}\n"
    ".hero-info h1{font-size:20px;font-weight:600;color:var(--fg);"
    "margin-bottom:4px}\n"
    ".hero-info .sub{font-size:13px;color:var(--fg3)}\n"
    ".hero-stats{display:flex;gap:0;margin-top:18px}\n"
    ".hero-stat{padding:0 26px;border-left:1px solid var(--bdr)}\n"
    ".hero-stat:first-child{padding-left:0;border-left:none}\n"
    ".hero-stat .val{font-family:var(--mono);font-size:20px;font-weight:600;"
    "line-height:1}\n"
    ".hero-stat .lbl{font-size:11px;color:var(--fg3);margin-top:3px;"
    "text-transform:uppercase;letter-spacing:.5px}\n"
    "\n"
    "/* ── Cards ── */\n"
    ".grid{display:grid;grid-template-columns:repeat(auto-fit,minmax(440px,1fr));"
    "gap:20px;margin-top:28px}\n"
    ".grid .full{grid-column:1/-1}\n"
    ".card{background:linear-gradient(180deg,rgba(255,255,255,.014),"
    "transparent 46%%),var(--bg2);border:1px solid var(--bdr);"
    "border-radius:14px;padding:22px 24px;overflow:hidden;"
    "box-shadow:var(--shadow);transition:border-color .25s}\n"
    ".card:hover{border-color:var(--bdr2)}\n"
    ".card h2{font-size:14px;font-weight:600;color:var(--fg);"
    "margin-bottom:2px;display:flex;align-items:center;gap:9px;"
    "letter-spacing:.2px}\n"
    ".card h2::before{content:'';width:3px;height:15px;border-radius:2px;"
    "flex-shrink:0;background:linear-gradient(180deg,var(--acc),var(--blu))}\n"
    ".card .desc{font-size:11.5px;color:var(--fg3);margin-bottom:16px;"
    "line-height:1.55}\n"
    "\n"
    "/* ── Tables ── */\n"
    "table{border-collapse:collapse;width:100%%;font-size:12.5px;"
    "font-variant-numeric:tabular-nums}\n"
    "th{color:var(--fg3);font-weight:600;text-transform:uppercase;"
    "font-size:10px;letter-spacing:.6px;padding:7px 10px;text-align:left;"
    "border-bottom:1px solid var(--bdr2)}\n"
    "th.r{text-align:right}\n"
    "td{padding:6px 10px;border-bottom:1px solid var(--bdr);color:var(--fg2)}\n"
    "td.r{text-align:right;font-family:var(--mono);font-size:11.5px}\n"
    "td.n{font-weight:500;color:var(--fg)}\n"
    "tr:hover td{background:rgba(127,140,170,.07)}\n"
    ".kv td:first-child{color:var(--fg3);font-size:11.5px;white-space:nowrap}\n"
    ".kv td:last-child{font-family:var(--mono);font-size:11.5px;"
    "text-align:right;color:var(--fg)}\n"
    "\n"
    "/* ── Colors ── */\n"
    ".c-acc{color:var(--acc)}.c-grn{color:var(--grn)}.c-red{color:var(--red)}\n"
    ".c-ylw{color:var(--ylw)}.c-orn{color:var(--orn)}.c-blu{color:var(--blu)}\n"
    ".c-fg2{color:var(--fg2)}\n"
    "\n"
    "/* ── Bar cells ── */\n"
    ".bar-cell{position:relative}\n"
    ".bar{position:absolute;left:0;top:0;bottom:0;border-radius:2px}\n"
    ".bar-acc{background:var(--acc)}.bar-grn{background:var(--grn)}\n"
    ".bar-blu{background:var(--blu)}.bar-ylw{background:var(--ylw)}\n"
    ".bar-orn{background:var(--orn)}.bar-red{background:var(--red)}\n"
    ".bar-label{position:relative;z-index:1}\n"
    "\n"
    "/* ── Anim ── */\n"
    "@keyframes fadeUp{from{opacity:0;transform:translateY(12px)}"
    "to{opacity:1;transform:none}}\n"
    ".card{animation:fadeUp .4s ease both}\n"
    ".card:nth-child(1){animation-delay:.05s}\n"
    ".card:nth-child(2){animation-delay:.1s}\n"
    ".card:nth-child(3){animation-delay:.15s}\n"
    ".card:nth-child(4){animation-delay:.2s}\n"
    ".card:nth-child(5){animation-delay:.25s}\n"
    ".card:nth-child(6){animation-delay:.3s}\n"
    ".card:nth-child(7){animation-delay:.35s}\n"
    ".card:nth-child(8){animation-delay:.4s}\n"
    ".card:nth-child(9){animation-delay:.45s}\n"
    ".card:nth-child(10){animation-delay:.5s}\n"
    "\n"
    "/* ── Misc ── */\n"
    "svg text{font-family:var(--sans)}\n"
    ".slider-row{display:flex;align-items:center;gap:10px;margin:6px 0 10px;"
    "font-size:12px;color:var(--fg3)}\n"
    ".slider-row input[type=range]{width:180px;accent-color:var(--acc)}\n"
    ".slider-row button{font-size:10px;padding:3px 10px;border:1px solid var(--bdr2);"
    "border-radius:6px;background:var(--bg3);color:var(--fg2);cursor:pointer;"
    "font-family:var(--sans);transition:background .12s,color .12s,"
    "border-color .12s}\n"
    ".slider-row button:hover{background:var(--bg4);border-color:var(--fg3);"
    "color:var(--fg)}\n"
    "\n"
    "/* ── Inspect panel ── */\n"
    ".cd-panel{display:none;margin-top:14px;padding:14px 18px;"
    "background:var(--bg3);border:1px solid var(--bdr2);border-radius:10px;"
    "font-size:12px;color:var(--fg2);animation:fadeUp .2s ease both}\n"
    ".cd-panel .cd-head{display:flex;align-items:baseline;gap:14px;"
    "margin-bottom:10px}\n"
    ".cd-panel .cd-byte{font-family:var(--mono);font-size:18px;"
    "font-weight:600;color:var(--fg)}\n"
    ".cd-panel .cd-sub{font-size:11px;color:var(--fg3)}\n"
    ".cd-panel .cd-cost{font-family:var(--mono);font-size:14px;"
    "font-weight:600}\n"
    ".cd-panel .cd-bar{display:flex;align-items:center;gap:8px;"
    "margin:3px 0;font-size:11px}\n"
    ".cd-panel .cd-bar-lbl{font-family:var(--mono);width:50px;"
    "color:var(--fg3);flex-shrink:0}\n"
    ".cd-panel .cd-bar-track{flex:1;height:14px;background:var(--bg);"
    "border-radius:3px;position:relative;overflow:hidden}\n"
    ".cd-panel .cd-bar-fill{height:100%%;border-radius:3px;"
    "position:absolute;left:0;top:0;transition:width .2s}\n"
    ".cd-panel .cd-bar-val{font-family:var(--mono);width:70px;"
    "text-align:right;flex-shrink:0}\n"
    ".cmap-sel{stroke:var(--fg);stroke-width:2}\n"
    ".hex-sel{outline:2px solid var(--fg);outline-offset:-1px}\n"
    ".scrub-wrap{position:relative}\n"
    ".scrub-line{stroke:var(--fg);stroke-width:0.6;opacity:0;"
    "pointer-events:none;transition:opacity .1s}\n"
    ".hover-tip{position:absolute;display:none;background:var(--bg);"
    "border:1px solid var(--bdr2);border-radius:8px;padding:7px 11px;"
    "font-size:11px;font-family:var(--mono);pointer-events:none;z-index:10;"
    "white-space:nowrap;box-shadow:0 10px 28px rgba(0,0,0,.45);"
    "color:var(--fg2);line-height:1.5}\n"
    ".hover-tip .tip-row{display:flex;gap:8px;justify-content:space-between;"
    "min-width:120px}\n"
    ".hover-tip .tip-sw{display:inline-block;width:8px;height:8px;"
    "border-radius:2px;margin-right:4px;vertical-align:middle}\n"
    ".pm-th{cursor:pointer;user-select:none;transition:color .12s}\n"
    ".pm-th:hover{color:var(--fg)}\n"
    ".pm-th .sort-arrow{font-size:9px;color:var(--fg3);margin-left:4px}\n"
    ".pm-th.active{color:var(--fg)}\n"
    ".pm-th.active .sort-arrow{color:var(--acc)}\n"
    ".theme-toggle{margin:auto 12px 12px;padding:6px 10px;font-size:11px;"
    "background:var(--bg3);color:var(--fg2);border:1px solid var(--bdr);"
    "border-radius:6px;cursor:pointer;font-family:var(--sans);"
    "display:flex;align-items:center;justify-content:center;gap:6px;"
    "transition:background .12s,color .12s,border-color .12s}\n"
    ".theme-toggle:hover{background:var(--bg4);color:var(--fg);"
    "border-color:var(--bdr2)}\n"
    ".mc-more{list-style:none;cursor:pointer;transition:color .12s}\n"
    ".mc-more::-webkit-details-marker{display:none}\n"
    ".mc-more:hover{color:var(--fg2)}\n"
    ".mc-more .mc-caret{transition:transform .15s}\n"
    "details[open]>.mc-more .mc-caret{transform:rotate(90deg)}\n"
    "\n"
    "/* ════ PARTY MODE (hidden; Konami code or 7 taps on the ring) ════ */\n"
    "#stars,#raster,#copper2,#glenz,.scrolltext{display:none}\n"
    "@keyframes copper{to{background-position:300%% 0}}\n"
    "@keyframes scrolltx{to{transform:translateX(-100%%)}}\n"
    "@keyframes wave{0%%,100%%{transform:translateY(-3px)}"
    "50%%{transform:translateY(3px)}}\n"
    "@keyframes stars{to{background-position:-520px 260px,-1120px 560px}}\n"
    "@keyframes blink{50%%{opacity:0}}\n"
    "@keyframes crackclr{0%%,100%%{color:var(--pnk)}33%%{color:var(--acc)}"
    "66%%{color:var(--yel)}}\n"
    "@keyframes ringpulse{0%%,100%%{filter:drop-shadow(0 0 6px "
    "rgba(34,211,238,.7))}50%%{filter:drop-shadow(0 0 16px "
    "rgba(255,63,216,.8))}}\n"
    "@keyframes rgbjitter{0%%,100%%{text-shadow:2px 2px 0 rgba(255,63,216,.55)}"
    "25%%{text-shadow:-2px 2px 0 rgba(34,211,238,.6)}"
    "50%%{text-shadow:2px -2px 0 rgba(255,227,63,.6)}"
    "75%%{text-shadow:-2px -2px 0 rgba(255,63,216,.6)}}\n"
    "body.party{\n"
    "  --bg:#000004;--bg2:#0b0b10;--bg3:#14141b;--bg4:#1f1f29;\n"
    "  --bdr:#26262f;--bdr2:#3a3a48;\n"
    "  --fg:#e8e8f0;--fg2:#9a9aad;--fg3:#63636f;\n"
    "  --shadow:5px 5px 0 rgba(0,0,0,.45);\n"
    "  --sans:'JetBrains Mono',ui-monospace,monospace;\n"
    "  background:var(--bg);line-height:1.65}\n"
    "body.party::before{content:'';position:fixed;top:0;left:0;right:0;"
    "height:4px;z-index:10000;background:linear-gradient(90deg,var(--pnk),"
    "var(--acc),var(--yel),var(--pnk));background-size:300%% 100%%;"
    "animation:copper 6s linear infinite}\n"
    "body.party::after{content:'';position:fixed;inset:0;"
    "pointer-events:none;z-index:9999;"
    "background:repeating-linear-gradient(0deg,rgba(0,0,0,.16) 0,"
    "rgba(0,0,0,.16) 1px,transparent 1px,transparent 3px)}\n"
    "body.party #stars{display:block;position:fixed;inset:0;z-index:-1;"
    "pointer-events:none;background-image:"
    "radial-gradient(rgba(255,255,255,.35) 1px,transparent 1.6px),"
    "radial-gradient(rgba(255,255,255,.18) 1px,transparent 1.6px);"
    "background-size:260px 260px,140px 140px;"
    "animation:stars 90s linear infinite}\n"
    "#moon{display:none}\n"
    "body.party #moon.lit{display:block;position:fixed;top:72px;right:96px;"
    "width:26px;height:26px;z-index:-1;pointer-events:none;"
    "border-radius:50%%;background:"
    "radial-gradient(circle at 62%% 40%%,rgba(0,0,0,.12) 0 12%%,"
    "transparent 14%%),"
    "radial-gradient(circle at 40%% 65%%,rgba(0,0,0,.10) 0 9%%,"
    "transparent 11%%),"
    "radial-gradient(circle at 35%% 35%%,#fff7d6,#ffe9a8 60%%,#e8c97a);"
    "box-shadow:0 0 18px rgba(255,233,168,.45)}\n"
    "body.party .scrolltext{display:block;overflow:hidden;"
    "white-space:nowrap;margin-top:30px;border-top:1px solid var(--bdr);"
    "border-bottom:1px solid var(--bdr);padding:9px 0;cursor:pointer;"
    "font-family:var(--pix);font-size:9px;color:var(--acc)}\n"
    "body.party .scrolltext span{display:inline-block;padding-left:100%%;"
    "animation:scrolltx 32s linear infinite;"
    "text-shadow:2px 2px 0 rgba(255,63,216,.5)}\n"
    "body.party .scrolltext i{display:inline-block;font-style:normal;"
    "animation:wave 1.4s ease-in-out infinite}\n"
    "body.party ::selection{background:rgba(255,63,216,.35)}\n"
    "body.party ::-webkit-scrollbar-thumb{border-radius:0}\n"
    "body.party .card{background:var(--bg2);border-radius:0;"
    "position:relative;padding-top:24px}\n"
    "body.party .card::before{content:'';position:absolute;top:0;left:0;"
    "right:0;height:3px;background:linear-gradient(90deg,var(--pnk),"
    "var(--acc),var(--yel),var(--pnk));background-size:300%% 100%%;"
    "animation:copper 8s linear infinite;opacity:.85}\n"
    "body.party .card:hover{border-color:var(--pnk)}\n"
    "body.party .card h2{font-family:var(--pix);font-size:10px;"
    "font-weight:400;text-transform:uppercase;letter-spacing:1px;"
    "line-height:1.6;margin-bottom:6px;gap:10px;"
    "text-shadow:2px 2px 0 rgba(255,63,216,.55)}\n"
    "body.party .card:hover h2{animation:rgbjitter .35s steps(1) infinite}\n"
    "body.party .card h2::before{width:8px;height:8px;border-radius:0;"
    "background:var(--acc);box-shadow:2px 2px 0 var(--pnk)}\n"
    "body.party .hero-info h1{font-family:var(--pix);font-size:14px;"
    "font-weight:400;text-transform:uppercase;letter-spacing:1px;"
    "line-height:1.5;margin-bottom:8px;"
    "text-shadow:2px 2px 0 rgba(255,63,216,.55)}\n"
    "body.party .hero-info h1::after{content:'\\2588';margin-left:8px;"
    "color:var(--acc);animation:blink 1.1s steps(2) infinite}\n"
    "body.party .hero-info::after{content:"
    "var(--stamp,'*** FUCK YOU SHANNON PAQ BEGS TO DIFFER ***');"
    "display:block;margin-top:12px;font-family:var(--pix);font-size:7px;"
    "letter-spacing:1px;animation:crackclr 4s linear infinite}\n"
    "body.party .hero-ring svg{animation:ringpulse 2.6s ease-in-out infinite}\n"
    "body.party .hero-pct .lbl,body.party .hero-stat .lbl{"
    "font-family:var(--pix);font-size:7px;letter-spacing:1px}\n"
    "body.party .hero-stats .hero-stat:last-child{cursor:help}\n"
    "body.party .hero-stats::after{content:'[ M = MUTE ]';display:flex;"
    "align-items:center;margin-left:28px;font-family:var(--pix);"
    "font-size:7px;letter-spacing:1px;color:var(--fg3);"
    "animation:blink 2.4s steps(2) infinite}\n"
    "body.party .sb-brand .dot{border-radius:0;background:var(--acc);"
    "box-shadow:3px 3px 0 var(--pnk)}\n"
    "body.party .sb-brand span:not(.dot){font-family:var(--pix);"
    "font-size:8px;font-weight:400;letter-spacing:1px;"
    "text-transform:uppercase;"
    "text-shadow:1px 1px 0 rgba(255,63,216,.6)}\n"
    "body.party .sidebar .sb-title{font-family:var(--pix);font-size:7px;"
    "font-weight:400;letter-spacing:1px}\n"
    "body.party .sidebar a:hover{text-shadow:1px 0 var(--pnk),"
    "-1px 0 var(--acc);background:rgba(255,63,216,.07)}\n"
    "body.party .sidebar a.active{border-left-color:var(--pnk)}\n"
    "body.party .slider-row button{border-radius:0;text-transform:uppercase;"
    "letter-spacing:.5px;box-shadow:2px 2px 0 rgba(0,0,0,.5)}\n"
    "body.party .slider-row button:hover{border-color:var(--pnk)}\n"
    "body.party .slider-row button:active{transform:translate(2px,2px);"
    "box-shadow:none}\n"
    "body.party .theme-toggle{border-radius:0;text-transform:uppercase;"
    "letter-spacing:1px}\n"
    "body.party .hover-tip{border-radius:0;"
    "box-shadow:4px 4px 0 rgba(0,0,0,.55)}\n"
    "body.party .cd-panel{border-radius:0}\n"
    "body.party #raster{display:block;position:fixed;inset:0;z-index:-1;"
    "pointer-events:none;background-image:"
    "linear-gradient(180deg,transparent 0,rgba(255,63,216,.16) 50%%,"
    "transparent 100%%),"
    "linear-gradient(180deg,transparent 0,rgba(34,211,238,.13) 50%%,"
    "transparent 100%%),"
    "linear-gradient(180deg,transparent 0,rgba(255,227,63,.09) 50%%,"
    "transparent 100%%);"
    "background-size:100%% 180px,100%% 260px,100%% 340px;"
    "animation:raster 5s linear infinite}\n"
    "@keyframes raster{to{background-position:0 180px,0 -260px,0 340px}}\n"
    "body.party #copper2{display:block;position:fixed;bottom:0;left:0;right:0;"
    "height:4px;z-index:10000;background:linear-gradient(90deg,var(--pnk),"
    "var(--acc),var(--yel),var(--pnk));background-size:300%% 100%%;"
    "animation:copper 6s linear infinite reverse}\n"
    "body.party #glenz{display:block;position:fixed;right:36px;bottom:46px;"
    "width:70px;height:70px;z-index:9998;cursor:pointer;touch-action:none;"
    "transform-style:preserve-3d;animation:glenz 7s linear infinite}\n"
    "body.party #glenz.kick{animation-duration:1.1s}\n"
    "body.party #glenz.dead{animation:glenzfall 1.6s ease-in forwards}\n"
    "@keyframes glenzfall{to{transform:translateY(130vh) rotateX(720deg)}}\n"
    "body.party .hero-ring.docked{animation:ringdock .9s ease}\n"
    "@keyframes ringdock{50%%{transform:scale(1.12)}}\n"
    "@keyframes glenz{to{transform:rotateX(360deg) rotateY(720deg)}}\n"
    "body.party #glenz i{position:absolute;inset:0;"
    "background:rgba(255,63,216,.10);border:1px solid var(--acc)}\n"
    "body.party #glenz i:nth-child(even){background:rgba(34,211,238,.10);"
    "border-color:var(--pnk)}\n"
    "body.party #glenz i:nth-child(1){transform:translateZ(35px)}\n"
    "body.party #glenz i:nth-child(2){transform:rotateY(180deg) "
    "translateZ(35px)}\n"
    "body.party #glenz i:nth-child(3){transform:rotateY(90deg) "
    "translateZ(35px)}\n"
    "body.party #glenz i:nth-child(4){transform:rotateY(-90deg) "
    "translateZ(35px)}\n"
    "body.party #glenz i:nth-child(5){transform:rotateX(90deg) "
    "translateZ(35px)}\n"
    "body.party #glenz i:nth-child(6){transform:rotateX(-90deg) "
    "translateZ(35px)}\n"
    "body.party-boot{animation:crtboot .55s ease-out}\n"
    "@keyframes crtboot{0%%{transform:scaleY(.004);filter:brightness(6)}"
    "55%%{transform:scaleY(1.03);filter:brightness(1.6)}"
    "100%%{transform:none;filter:none}}\n"
    "#decrunch{position:fixed;inset:0;z-index:10002;background:#000;"
    "display:flex;flex-direction:column;align-items:center;"
    "justify-content:center;gap:16px;font-family:var(--pix);"
    "color:var(--acc);font-size:10px;letter-spacing:2px}\n"
    "#decrunch .bar{color:var(--pnk);font-size:13px;letter-spacing:1px;"
    "min-height:16px}\n"
    ".pspark{position:fixed;width:4px;height:4px;z-index:10001;"
    "pointer-events:none;animation:spark .5s ease-out forwards}\n"
    "@keyframes spark{to{transform:translateY(12px) scale(0);opacity:0}}\n"
    ".pburst{position:fixed;width:5px;height:5px;z-index:10001;"
    "pointer-events:none;animation:pburst .45s ease-out forwards}\n"
    "@keyframes pburst{to{transform:translate(var(--dx),var(--dy)) "
    "scale(0);opacity:0}}\n"
    "#guru{position:fixed;inset:0;z-index:10003;background:#000;"
    "cursor:pointer;display:flex;justify-content:center;"
    "padding-top:26px}\n"
    "#guru .box{border:5px solid #f00;padding:14px 44px;text-align:center;"
    "font-family:var(--mono);color:#f00;font-size:15px;line-height:1.9;"
    "height:fit-content;animation:gurublink 1.1s steps(2) infinite}\n"
    "@keyframes gurublink{50%%{border-color:transparent}}\n"
    "#party-toast{position:fixed;top:40%%;left:50%%;"
    "transform:translate(-50%%,-50%%);z-index:10001;pointer-events:none;"
    "font-family:var(--pix);font-size:14px;color:var(--acc);"
    "background:rgba(0,0,4,.92);border:2px solid var(--pnk);"
    "padding:18px 26px;letter-spacing:2px;"
    "text-shadow:2px 2px 0 rgba(255,63,216,.6)}\n"
    "</style></head><body>\n"
    "<div id=\"stars\"></div>\n"
    "<div id=\"moon\"></div>\n"
    "<div id=\"raster\"></div>\n"
    "<div id=\"copper2\"></div>\n"
    "<div id=\"glenz\"><i></i><i></i><i></i><i></i><i></i><i></i></div>\n"
    "<nav class=\"sidebar\" id=\"sidebar\">\n"
    "<div class=\"sb-brand\"><span class=\"dot\"></span>"
    "<span>Compression Report</span></div>\n"
    "<div class=\"sb-title\">Overview</div>\n"
    "<a href=\"#sec-params\">Parameters</a>\n"
    "<a href=\"#sec-output\">Output Breakdown</a>\n"
    "<a href=\"#sec-header\">Header Anatomy</a>\n"
    "<a href=\"#sec-mctrib\">Model Contribution</a>\n"
    "<div class=\"sb-title\">Input Data</div>\n"
    "<a href=\"#sec-bytefreq\">Byte Frequency</a>\n"
    "<a href=\"#sec-bigram\">Byte Bigrams</a>\n"
    "<a href=\"#sec-cmap\">Compressibility Map</a>\n"
    "<a href=\"#sec-costhist\">Cost Histogram</a>\n"
    "<a href=\"#sec-infoscatter\">Cost vs Entropy</a>\n"
    "<a href=\"#sec-learn\">Learning Curve</a>\n"
    "<a href=\"#sec-strings\">Strings</a>\n"
    "<a href=\"#sec-hex\">Hex View</a>\n"
    "<div class=\"sb-title\">Models</div>\n"
    "<a href=\"#sec-attr\">Model Attribution</a>\n"
    "<a href=\"#sec-dominance\">Model Dominance</a>\n"
    "<a href=\"#sec-hurt\">Model Hurt</a>\n"
    "<a href=\"#sec-net\">Model Net</a>\n"
    "<a href=\"#sec-depth\">Context Depth</a>\n"
    "<a href=\"#sec-corr\">Model Correlation</a>\n"
    "<a href=\"#sec-surprises\">Top Surprises</a>\n"
    "<div class=\"sb-title\">Position</div>\n"
    "<a href=\"#sec-cost\">Cost Over Position</a>\n"
    "<a href=\"#sec-cumcost\">Cumulative Cost</a>\n"
    "<a href=\"#sec-bitmap\">Bit Heatmap</a>\n"
    "<a href=\"#sec-repeat\">Repetition</a>\n"
    "<div class=\"sb-title\">Search</div>\n"
    "<a href=\"#sec-search\">Search Trajectory</a>\n"
    "<a href=\"#sec-maskgrid\">Mask Outcomes</a>\n"
    "<a href=\"#sec-models\">Model Statistics</a>\n"
    "<div class=\"sb-title\">Coder</div>\n"
    "<a href=\"#sec-conf\">Pred. Confidence</a>\n"
    "<a href=\"#sec-calib\">Calibration</a>\n"
    "<a href=\"#sec-bytepos\">Byte Position</a>\n"
    "<a href=\"#sec-hash\">Direct-Mapped Table</a>\n"
    "<a href=\"#sec-occ\">Table Growth</a>\n"
    "<a href=\"#sec-sat\">Counter Saturation</a>\n"
    "<button id=\"theme-toggle\" class=\"theme-toggle\" type=\"button\" "
    "title=\"Toggle theme\">"
    "<span id=\"theme-toggle-icon\">\xe2\x98\xbd</span>"
    "<span id=\"theme-toggle-label\">Light</span></button>\n"
    "</nav>\n"
    "<div class=\"wrap\">\n",
    s->input_file);

  /* shared sparkline helper used by hover tooltips across the report */
  fprintf(f, "%s",
    "<script>\n"
    "window.makeSparkline=function(values,opts){\n"
    "  opts=opts||{};\n"
    "  var W=opts.width||200,H=opts.height||40,P=2;\n"
    "  var color=opts.color||'#22d3ee';\n"
    "  var hi=(opts.highlightIdx!==undefined)?opts.highlightIdx:-1;\n"
    "  var n=values.length; if(n<2) return '';\n"
    "  var vmin=Infinity,vmax=-Infinity;\n"
    "  for(var i=0;i<n;i++){\n"
    "    if(values[i]<vmin) vmin=values[i];\n"
    "    if(values[i]>vmax) vmax=values[i];\n"
    "  }\n"
    "  var hasNeg=vmin<0&&vmax>0;\n"
    "  var midY=H/2, topY=2, botY=H-2;\n"
    "  function yOf(v){\n"
    "    if(hasNeg){\n"
    "      var amp=Math.max(Math.abs(vmin),Math.abs(vmax));\n"
    "      if(amp<0.001) return midY;\n"
    "      return midY-(v/amp)*(midY-2);\n"
    "    }\n"
    "    if(vmax<=vmin) return botY;\n"
    "    return botY-((v-vmin)/(vmax-vmin))*(botY-topY);\n"
    "  }\n"
    "  function xOf(i){return P+(i*(W-2*P)/(n>1?n-1:1));}\n"
    "  var base=hasNeg?midY:botY;\n"
    "  var area='M'+P+','+base;\n"
    "  for(var i=0;i<n;i++) area+=' L'+xOf(i).toFixed(1)+','+yOf(values[i]).toFixed(1);\n"
    "  area+=' L'+(W-P)+','+base+' Z';\n"
    "  var line='';\n"
    "  for(var i=0;i<n;i++) line+=(i?'L':'M')+xOf(i).toFixed(1)+','+yOf(values[i]).toFixed(1)+' ';\n"
    "  var s='<svg width=\"'+W+'\" height=\"'+H+'\" '\n"
    "    +'style=\"display:block;margin-top:6px\">'\n"
    "    +'<rect x=\"0\" y=\"0\" width=\"'+W+'\" height=\"'+H+'\" '\n"
    "    +'fill=\"var(--bg3)\" rx=\"3\"/>';\n"
    "  if(hasNeg){\n"
    "    s += '<line x1=\"0\" y1=\"'+midY+'\" x2=\"'+W+'\" y2=\"'+midY\n"
    "      +'\" stroke=\"var(--fg3)\" stroke-width=\"0.5\"/>';\n"
    "  }\n"
    "  s += '<path d=\"'+area+'\" fill=\"'+color+'\" fill-opacity=\".25\"/>';\n"
    "  s += '<path d=\"'+line+'\" fill=\"none\" stroke=\"'+color+'\" stroke-width=\"1\"/>';\n"
    "  if(hi>=0&&hi<n){\n"
    "    s += '<circle cx=\"'+xOf(hi).toFixed(1)+'\" '\n"
    "      +'cy=\"'+yOf(values[hi]).toFixed(1)+'\" r=\"2.5\" fill=\"var(--fg)\"/>';\n"
    "  }\n"
    "  if(opts.labelTop) s += '<text x=\"4\" y=\"9\" font-size=\"8\" '\n"
    "    +'fill=\"var(--fg3)\">'+opts.labelTop+'</text>';\n"
    "  if(opts.labelBot) s += '<text x=\"4\" y=\"'+(H-3)+'\" font-size=\"8\" '\n"
    "    +'fill=\"var(--fg3)\">'+opts.labelBot+'</text>';\n"
    "  if(opts.labelRight) s += '<text x=\"'+(W-4)+'\" y=\"'+(H-3)\n"
    "    +'\" text-anchor=\"end\" font-size=\"8\" fill=\"var(--fg3)\">'\n"
    "    +opts.labelRight+'</text>';\n"
    "  return s+'</svg>';\n"
    "};\n"
    "/* URL state: serialize active model/mode/sort to #hash so a particular\n"
    "   finding can be bookmarked or shared as a deep link */\n"
    "window.reportState={m:null,mode:null,sortCol:6,sortDir:-1};\n"
    "window.reportStateSuppress=false;\n"
    "window.serializeReportState=function(){\n"
    "  if(window.reportStateSuppress) return;\n"
    "  var s=window.reportState, parts=[];\n"
    "  if(s.m!==null&&s.m!==undefined){\n"
    "    parts.push('m='+s.m);\n"
    "    if(s.mode) parts.push('mode='+s.mode);\n"
    "  }\n"
    "  if(s.sortCol!==6||s.sortDir!==-1)\n"
    "    parts.push('sort='+s.sortCol+','+s.sortDir);\n"
    "  var h=parts.length?'#'+parts.join('&'):location.pathname+location.search;\n"
    "  try { history.replaceState(null,'',h); } catch(e) {}\n"
    "};\n"
    "window.parseReportState=function(){\n"
    "  var h=location.hash.replace(/^#/,'');\n"
    "  if(!h) return {};\n"
    "  var out={};\n"
    "  h.split('&').forEach(function(p){\n"
    "    var i=p.indexOf('='); if(i<0) return;\n"
    "    out[p.substring(0,i)]=p.substring(i+1);\n"
    "  });\n"
    "  return out;\n"
    "};\n"
    "</script>\n");

  /* ── Hero section with ring gauge ── */
  {
    /* ring gauge: SVG donut */
    int ring_r = 60, ring_stroke = 10;
    double circumf = 2.0 * 3.14159265 * ring_r;
    double fill_len = circumf * savings / 100.0;
    double gap_len = circumf - fill_len;
    const char *ring_color = savings > 60 ? "#22d3ee"
                           : savings > 40 ? "#34d399"
                           : savings > 20 ? "#fbbf24"
                                          : "#f87171";

    fprintf(f, "<div class=\"hero\">\n");
    fprintf(f,
      "<div class=\"hero-ring\">\n"
      "<svg width=\"%d\" height=\"%d\" viewBox=\"0 0 %d %d\">\n"
      "<circle cx=\"%d\" cy=\"%d\" r=\"%d\" fill=\"none\" "
      "stroke=\"var(--bg3)\" stroke-width=\"%d\"/>\n"
      "<circle cx=\"%d\" cy=\"%d\" r=\"%d\" fill=\"none\" "
      "stroke=\"%s\" stroke-width=\"%d\" "
      "stroke-dasharray=\"%.1f %.1f\" stroke-dashoffset=\"%.1f\" "
      "stroke-linecap=\"round\" "
      "style=\"transform:rotate(-90deg);transform-origin:center;"
      "filter:drop-shadow(0 0 8px %s40)\"/>\n"
      "</svg>\n"
      "<div class=\"hero-pct\">"
      "<span class=\"big\" style=\"color:%s\">%.0f%%</span>"
      "<span class=\"lbl\">saved</span></div>\n"
      "</div>\n",
      (ring_r + ring_stroke) * 2, (ring_r + ring_stroke) * 2,
      (ring_r + ring_stroke) * 2, (ring_r + ring_stroke) * 2,
      ring_r + ring_stroke, ring_r + ring_stroke, ring_r, ring_stroke,
      ring_r + ring_stroke, ring_r + ring_stroke, ring_r,
      ring_color, ring_stroke, fill_len, gap_len, circumf * 0.25,
      ring_color, ring_color, savings);

    fprintf(f,
      "<div class=\"hero-info\">\n"
      "<h1>Compression Report</h1>\n"
      "<p class=\"sub\">%s &middot; context-mixing arithmetic coder</p>\n"
      "<div class=\"hero-stats\">\n"
      "<div class=\"hero-stat\"><div class=\"val\">%d</div>"
      "<div class=\"lbl\">Input bytes</div></div>\n"
      "<div class=\"hero-stat\"><div class=\"val c-acc\">%d</div>"
      "<div class=\"lbl\">Output bytes</div></div>\n"
      "<div class=\"hero-stat\"><div class=\"val c-grn\">%.2f%%</div>"
      "<div class=\"lbl\">Ratio</div></div>\n"
      "<div class=\"hero-stat\"><div class=\"val\" style=\"color:%s\">%+.3f</div>"
      "<div class=\"lbl\">H\xe2\x82\x80 gain</div></div>\n"
      "</div></div></div>\n\n",
      s->input_file, s->input_size, s->total_bytes, ratio,
      context_gain > 0 ? "#34d399" : "#f87171", context_gain);
  }

  fprintf(f, "<div class=\"grid\">\n");

  /* ── Summary card ── */
  fprintf(f,
    "<div class=\"card\" id=\"sec-params\">\n"
    "<h2>Parameters</h2>\n"
    "<p class=\"desc\">Compression settings and entropy metrics.</p>\n"
    "<table class=\"kv\">\n");
  fprintf(f, "<tr><td>Search</td><td>beam %d%s%s</td></tr>\n",
    s->beam, s->simple ? " simple" : "", s->extreme ? " extreme" : "");
  fprintf(f, "<tr><td>Base probability</td><td>%d</td></tr>\n", s->base_prob);
  fprintf(f, "<tr><td>Models</td><td style=\"font-size:10.5px\">%s</td></tr>\n",
    s->model_string);
  fprintf(f, "<tr><td>Compressed bits</td><td>%d</td></tr>\n",
    s->compressed_bits);
  fprintf(f, "<tr><td>Header</td><td>%d bytes</td></tr>\n", s->header_bytes);
  fprintf(f, "<tr><td>Large mode (-L)</td><td class=\"%s\">%s</td></tr>\n",
    s->large_field ? "c-blu" : "c-grn",
    s->large_field ? "on (32-bit length field)"
                   : "off (16-bit length field)");
  fprintf(f, "<tr><td>Estimated (pre-encode)</td><td>%.3f bytes</td></tr>\n",
    s->estimated_bytes);
  fprintf(f,
    "<tr><td>Estimator delta</td>"
    "<td class=\"%s\">%+.1f bytes (%+.1f%%)</td></tr>\n",
    (est_delta < 2 && est_delta > -2) ? "c-grn"
    : (est_delta < 5 && est_delta > -5) ? "c-ylw" : "c-orn",
    est_delta, est_delta_pct);
  fprintf(f,
    "<tr><td>Shannon H\xe2\x82\x80</td>"
    "<td>%.3f bits/byte</td></tr>\n", s->entropy);
  if (h1 >= 0)
    fprintf(f,
      "<tr><td>Order-1 H\xe2\x82\x81</td>"
      "<td>%.3f bits/byte</td></tr>\n", h1);
  fprintf(f,
    "<tr><td>Actual (incl. header)</td>"
    "<td class=\"%s\">%.3f bits/byte</td></tr>\n",
    actual_bpb < s->entropy ? "c-grn"
    : actual_bpb < s->entropy * 1.1 ? "c-blu"
    : actual_bpb < s->entropy * 1.3 ? "c-ylw" : "c-orn",
    actual_bpb);
  fprintf(f,
    "<tr><td>Prediction cost</td>"
    "<td>%.1f bits (%.1f B)</td></tr>\n",
    s->total_cost, s->total_cost / 8.0);
  {
    /* actual emitted bits minus ideal (sum of -log2 p): arithmetic coder
       loss + termination. Should be a byte or two. */
    double ovh = s->compressed_bits - s->total_cost;
    fprintf(f,
      "<tr><td>Coder overhead</td>"
      "<td class=\"%s\">%+.1f bits (%.2f B)</td></tr>\n",
      ovh < 16 ? "c-grn" : ovh < 64 ? "c-ylw" : "c-orn",
      ovh, ovh / 8.0);
  }
  fprintf(f,
    "<tr><td>Arith coder min range</td>"
    "<td>0x%08X</td></tr>\n", s->min_range);
  fprintf(f, "<tr><td>Search time</td><td>%.1f ms</td></tr>\n", s->search_ms);
  if (g_encode_ms > 0)
    fprintf(f,
      "<tr><td>Encode time</td>"
      "<td>%.3f ms/pass (%.1f MB/s, %d rep%s)</td></tr>\n",
      g_encode_ms, s->input_size / g_encode_ms / 1000.0, timing_reps,
      timing_reps == 1 ? "" : "s");
  if (s->input_data)
    fprintf(f, "<tr><td>Input CRC32-C</td><td>0x%08X</td></tr>\n", input_crc);
  fprintf(f, "</table></div>\n\n");

  /* ── Output Breakdown card ── */
  {
    int payload_bytes = s->total_bytes - s->header_bytes;
    int padding_bits = payload_bytes * 8 - s->compressed_bits;
    float total = (float)s->total_bytes;
    float f_hdr = s->header_bytes / total * 100;
    float f_pay = (payload_bytes * 8 - padding_bits) / 8.0f / total * 100;
    float f_pad = padding_bits / 8.0f / total * 100;

    fprintf(f,
      "<div class=\"card\" id=\"sec-output\">\n"
      "<h2>Output Breakdown</h2>\n"
      "<p class=\"desc\">Where the compressed bytes go.</p>\n");

    struct { const char *label; float pct; const char *fill; int bytes; } segs[] = {
      {"Header", f_hdr, "#6b7186", s->header_bytes},
      {"Payload", f_pay, "#22d3ee",
       (int)((payload_bytes * 8 - padding_bits + 7) / 8)},
      {"Padding", f_pad, "#3b4156", (padding_bits + 7) / 8},
    };
    int nsegs = padding_bits > 0 ? 3 : 2;

    /* donut chart + side legend */
    fprintf(f, "<div style=\"display:flex;align-items:center;gap:24px;"
      "padding:8px 0 4px\">\n");

    int cx = 70, cy = 70, r = 52, sw = 18;
    double circumf = 2.0 * 3.14159265 * r;

    fprintf(f, "<svg width=\"140\" height=\"140\" viewBox=\"0 0 140 140\" "
      "style=\"flex-shrink:0\">\n");
    /* background ring */
    fprintf(f,
      "<circle cx=\"%d\" cy=\"%d\" r=\"%d\" fill=\"none\" "
      "stroke=\"var(--bg3)\" stroke-width=\"%d\"/>\n", cx, cy, r, sw);
    /* segments, rotated -90 so 0%% starts at top */
    fprintf(f, "<g transform=\"rotate(-90 %d %d)\">\n", cx, cy);
    double cum_pct = 0;
    for (int i = 0; i < nsegs; i++) {
      double arc = circumf * segs[i].pct / 100.0;
      double gap = circumf - arc;
      double offset = -circumf * cum_pct / 100.0;
      fprintf(f,
        "<circle cx=\"%d\" cy=\"%d\" r=\"%d\" fill=\"none\" "
        "stroke=\"%s\" stroke-width=\"%d\" "
        "stroke-dasharray=\"%.2f %.2f\" stroke-dashoffset=\"%.2f\"/>\n",
        cx, cy, r, segs[i].fill, sw, arc, gap, offset);
      cum_pct += segs[i].pct;
    }
    fprintf(f, "</g>\n");
    /* center label */
    fprintf(f,
      "<text x=\"%d\" y=\"%d\" text-anchor=\"middle\" "
      "font-size=\"22\" font-weight=\"600\" fill=\"var(--fg)\" "
      "font-family=\"var(--sans)\">%d</text>\n", cx, cy + 4, s->total_bytes);
    fprintf(f,
      "<text x=\"%d\" y=\"%d\" text-anchor=\"middle\" "
      "font-size=\"9\" fill=\"var(--fg3)\" "
      "letter-spacing=\"1.2px\">BYTES</text>\n", cx, cy + 20);
    fprintf(f, "</svg>\n");

    /* legend */
    fprintf(f, "<div style=\"flex:1;display:flex;flex-direction:column;"
      "gap:8px;font-size:12px;min-width:0\">\n");
    for (int i = 0; i < nsegs; i++) {
      fprintf(f,
        "<div style=\"display:flex;align-items:center;gap:10px\">\n"
        "<span style=\"display:inline-block;width:10px;height:10px;"
        "border-radius:2px;background:%s;flex-shrink:0\"></span>\n"
        "<span style=\"flex:1;color:var(--fg2)\">%s</span>\n"
        "<span style=\"font-family:var(--mono);color:var(--fg);"
        "font-weight:500\">%d B</span>\n"
        "<span style=\"font-family:var(--mono);color:var(--fg3);"
        "min-width:48px;text-align:right\">%.1f%%</span>\n"
        "</div>\n",
        segs[i].fill, segs[i].label, segs[i].bytes, segs[i].pct);
    }
    fprintf(f, "</div>\n"
      "</div>\n"
      "</div>\n\n");
  }

  /* ── Header Anatomy ── */
  if (s->num_models > 0) {
    int len_bytes = s->large_field ? 4 : 2;

    /* classify the 32 wmask bits by replaying the loader's walk:
       0 bit = store next mask at current weight, 1 bit = bump weight,
       field ends when the shifted register hits zero */
    int bit_kind[32];   /* 0=model, 1=bump, 2=fill/end */
    int bit_model[32];
    int bit_weight[32];
    int hdr_w[MAX_SEARCH] = {0}; /* weight of each header mask */
    {
      unsigned int w = s->hdr_wmask;
      int wval = 0, mi2 = 0, ended = 0;
      for (int i = 0; i < 32; i++) {
        unsigned int bit = w & 0x80000000u;
        w <<= 1;
        bit_model[i] = 0;
        bit_weight[i] = wval;
        if (ended || !w) {
          bit_kind[i] = 2;
          ended = 1;
        } else if (bit) {
          bit_kind[i] = 1;
          bit_weight[i] = ++wval;
        } else {
          bit_kind[i] = 0;
          bit_model[i] = mi2++;
          if (bit_model[i] < MAX_SEARCH)
            hdr_w[bit_model[i]] = wval;
        }
      }
    }

    fprintf(f, "%s",
      "<div class=\"card\" id=\"sec-header\" style=\"position:relative\">\n"
      "<h2>Header Anatomy</h2>\n"
      "<p class=\"desc\">The emitted .paq header, byte by byte: "
      "<span style=\"color:#60a5fa\">bitlength</span> &middot; "
      "<span style=\"color:#fbbf24\">weight mask</span> &middot; "
      "<span style=\"color:#22d3ee\">context masks</span> "
      "(stored by ascending weight).</p>\n");

    /* ── byte row ── */
    fprintf(f, "<div style=\"display:flex;flex-wrap:wrap;gap:3px;"
      "margin-bottom:14px;font-family:var(--mono)\">\n");
    for (int i = 0; i < len_bytes; i++)
      fprintf(f,
        "<div data-d=\"bitlength byte %d of %d (little-endian)\" "
        "style=\"width:32px;text-align:center;cursor:default;"
        "background:rgba(96,165,250,.14);border:1px solid #60a5fa;"
        "border-radius:3px;padding:2px 0\">"
        "<div style=\"font-size:11px;color:var(--fg)\">%02X</div>"
        "<div style=\"font-size:8px;color:var(--fg3)\">len</div></div>\n",
        i, len_bytes, (s->hdr_bitlen >> (8 * i)) & 0xFF);
    for (int i = 0; i < 4; i++)
      fprintf(f,
        "<div data-d=\"weight mask byte %d of 4 (little-endian)\" "
        "style=\"width:32px;text-align:center;cursor:default;"
        "background:rgba(251,191,36,.14);border:1px solid #fbbf24;"
        "border-radius:3px;padding:2px 0\">"
        "<div style=\"font-size:11px;color:var(--fg)\">%02X</div>"
        "<div style=\"font-size:8px;color:var(--fg3)\">wm</div></div>\n",
        i, (s->hdr_wmask >> (8 * i)) & 0xFF);
    for (int i = 0; i < s->num_models; i++)
      fprintf(f,
        "<div data-d=\"context mask %d (weight %d)\" "
        "style=\"width:32px;text-align:center;cursor:default;"
        "background:rgba(34,211,238,.14);border:1px solid #22d3ee;"
        "border-radius:3px;padding:2px 0\">"
        "<div style=\"font-size:11px;color:var(--fg)\">%02X</div>"
        "<div style=\"font-size:8px;color:var(--fg3)\">m%d</div></div>\n",
        i, hdr_w[i], s->hdr_masks[i], i);
    fprintf(f, "</div>\n");

    /* ── wmask bit strip ── */
    fprintf(f, "%s",
      "<div style=\"font-size:11px;color:var(--fg3);margin-bottom:6px\">"
      "Weight mask, MSB first: <span style=\"color:#22d3ee\">0 = store "
      "next mask</span> at current weight (digit = mask index), "
      "<span style=\"color:#fbbf24\">1 = bump weight</span>; field ends "
      "when the register empties.</div>\n"
      "<div id=\"ha-bits\" style=\"display:flex;flex-wrap:wrap;gap:2px;"
      "margin-bottom:14px;font-family:var(--mono)\">\n");
    for (int i = 0; i < 32; i++) {
      const char *bg, *fg, *ch;
      char chbuf[8];
      if (bit_kind[i] == 0) {
        bg = "#22d3ee"; fg = "#0c0e14";
        snprintf(chbuf, sizeof(chbuf), "%d", bit_model[i]);
        ch = chbuf;
      } else if (bit_kind[i] == 1) {
        bg = "#fbbf24"; fg = "#0c0e14"; ch = "+";
      } else {
        bg = "var(--bg4)"; fg = "var(--fg3)"; ch = "&middot;";
      }
      fprintf(f,
        "<div data-i=\"%d\" data-k=\"%d\" data-m=\"%d\" data-w=\"%d\" "
        "style=\"width:18px;height:18px;display:flex;align-items:center;"
        "justify-content:center;font-size:9px;border-radius:2px;"
        "cursor:default;background:%s;color:%s\">%s</div>\n",
        i, bit_kind[i], bit_model[i], bit_weight[i], bg, fg, ch);
    }
    fprintf(f, "</div>\n");

    /* ── decoded values ── */
    fprintf(f, "<table class=\"kv\">\n");
    fprintf(f,
      "<tr><td>Bitlength field</td><td>%d (%d data bits + 1 stop bit)"
      "</td></tr>\n",
      s->hdr_bitlen, s->hdr_bitlen - 1);
    fprintf(f, "<tr><td>Weight mask</td><td>0x%08X</td></tr>\n",
      s->hdr_wmask);
    fprintf(f,
      "<tr><td>Termination parity</td><td>low-byte parity = %d "
      "(1 = stream terminates)</td></tr>\n",
      __builtin_parity(s->hdr_wmask & 0xFF));
    fprintf(f,
      "<tr><td>Layout</td><td>%d len + 4 wmask + %d masks = %d bytes"
      "</td></tr>\n",
      len_bytes, s->num_models, s->header_bytes);
    fprintf(f, "</table>\n");

    fprintf(f, "<div id=\"ha-tip\" class=\"hover-tip\"></div>\n");

    /* hover tooltips for bytes + bits */
    fprintf(f, "<script>\n(function(){\nvar HAM=[");
    for (int i = 0; i < s->num_models; i++)
      fprintf(f, "%s\"%02X\"", i ? "," : "", s->hdr_masks[i]);
    fprintf(f, "];\n");
    fprintf(f, "%s",
      "var card=document.getElementById('sec-header');\n"
      "var tip=document.getElementById('ha-tip');\n"
      "function hide(){tip.style.display='none';}\n"
      "card.addEventListener('mousemove',function(e){\n"
      "  var el=e.target.closest('[data-d],[data-i]');\n"
      "  if(!el){hide();return;}\n"
      "  var txt;\n"
      "  if(el.hasAttribute('data-d')){\n"
      "    txt=el.getAttribute('data-d');\n"
      "  } else {\n"
      "    var i=+el.getAttribute('data-i');\n"
      "    var k=+el.getAttribute('data-k');\n"
      "    var m=+el.getAttribute('data-m');\n"
      "    var w=+el.getAttribute('data-w');\n"
      "    if(k===0) txt='bit '+i+': store mask '+(HAM[m]||m)\n"
      "      +' at weight '+w+' (model '+m+')';\n"
      "    else if(k===1) txt='bit '+i+': bump weight \\u2192 '+w;\n"
      "    else txt='bit '+i+': fill / end of field'\n"
      "      +(i>=24?'; low-byte parity encodes termination':'');\n"
      "  }\n"
      "  tip.innerHTML='<div class=\"tip-row\"><span style=\"color:var(--fg)\">'\n"
      "    +txt+'</span></div>';\n"
      "  var cr=card.getBoundingClientRect();\n"
      "  var tx=(e.clientX-cr.left)+14, ty=(e.clientY-cr.top)-34;\n"
      "  if(tx+240>cr.width) tx=(e.clientX-cr.left)-240;\n"
      "  if(ty<0) ty=(e.clientY-cr.top)+18;\n"
      "  tip.style.left=tx+'px'; tip.style.top=ty+'px';\n"
      "  tip.style.display='block';\n"
      "});\n"
      "card.addEventListener('mouseleave',hide);\n"
      "})();</script>\n");

    fprintf(f, "</div>\n\n");
  }

  /* ── Model Contribution donut ── */
  if (s->num_models > 0) {
    /* same palette as Attribution map for visual continuity */
    static const int mc_pal[][3] = {
      {34,211,238},{251,146,60},{167,139,250},{52,211,153},
      {251,191,36},{248,113,113},{96,165,250},{232,121,249},
      {163,230,53},{244,114,182},{45,212,191},{253,186,116},
      {134,239,172},{196,181,253},{252,211,77},{125,211,252},
      {249,168,212},{190,242,100},{253,164,175},{110,231,183},
      {217,70,239},
    };
    int mc_npal = (int)(sizeof(mc_pal) / sizeof(mc_pal[0]));

    /* sort by bits_saved desc */
    int mc_order[MAX_SEARCH];
    for (int m = 0; m < s->num_models; m++) mc_order[m] = m;
    for (int i = 0; i < s->num_models - 1; i++)
      for (int j = i + 1; j < s->num_models; j++)
        if (s->model_bits_saved[mc_order[i]] < s->model_bits_saved[mc_order[j]]) {
          int t = mc_order[i]; mc_order[i] = mc_order[j]; mc_order[j] = t;
        }

    double mc_total = 0;
    for (int m = 0; m < s->num_models; m++)
      if (s->model_bits_saved[m] > 0) mc_total += s->model_bits_saved[m];
    if (mc_total < 1) mc_total = 1;

    fprintf(f,
      "<div class=\"card\" id=\"sec-mctrib\">\n"
      "<h2>Model Contribution</h2>\n"
      "<p class=\"desc\">Each model's share of total bits saved.</p>\n");

    fprintf(f, "<div style=\"display:flex;align-items:center;gap:24px;"
      "padding:8px 0 4px\">\n");

    int cx = 70, cy = 70, r = 52, sw = 18;
    double circumf = 2.0 * 3.14159265 * r;

    fprintf(f, "<svg width=\"140\" height=\"140\" viewBox=\"0 0 140 140\" "
      "style=\"flex-shrink:0\">\n");
    fprintf(f,
      "<circle cx=\"%d\" cy=\"%d\" r=\"%d\" fill=\"none\" "
      "stroke=\"var(--bg3)\" stroke-width=\"%d\"/>\n", cx, cy, r, sw);
    fprintf(f, "<g transform=\"rotate(-90 %d %d)\">\n", cx, cy);

    double cum_pct = 0;
    for (int i = 0; i < s->num_models; i++) {
      int m = mc_order[i];
      double bits = s->model_bits_saved[m];
      if (bits <= 0) continue;
      double pct = 100.0 * bits / mc_total;
      double arc = circumf * pct / 100.0;
      double gap = circumf - arc;
      double offset = -circumf * cum_pct / 100.0;
      int pi = i % mc_npal;
      fprintf(f,
        "<circle cx=\"%d\" cy=\"%d\" r=\"%d\" fill=\"none\" "
        "stroke=\"rgb(%d,%d,%d)\" stroke-width=\"%d\" "
        "stroke-dasharray=\"%.2f %.2f\" stroke-dashoffset=\"%.2f\"/>\n",
        cx, cy, r, mc_pal[pi][0], mc_pal[pi][1], mc_pal[pi][2], sw,
        arc, gap, offset);
      cum_pct += pct;
    }
    fprintf(f, "</g>\n");

    /* center: total bytes saved */
    fprintf(f,
      "<text x=\"%d\" y=\"%d\" text-anchor=\"middle\" "
      "font-size=\"22\" font-weight=\"600\" fill=\"var(--fg)\" "
      "font-family=\"var(--sans)\">%.0f</text>\n",
      cx, cy + 4, mc_total / 8.0);
    fprintf(f,
      "<text x=\"%d\" y=\"%d\" text-anchor=\"middle\" "
      "font-size=\"8\" fill=\"var(--fg3)\" "
      "letter-spacing=\"1.2px\">BYTES SAVED</text>\n", cx, cy + 20);
    fprintf(f, "</svg>\n");

    /* legend (top N + grouped remainder) */
    int show_n = s->num_models < 6 ? s->num_models : 6;
    fprintf(f, "<div style=\"flex:1;display:flex;flex-direction:column;"
      "gap:6px;font-size:12px;min-width:0\">\n");
    for (int i = 0; i < show_n; i++) {
      int m = mc_order[i];
      double bits = s->model_bits_saved[m];
      double pct = 100.0 * bits / mc_total;
      int pi = i % mc_npal;
      const char *clr = bits <= 0 ? "var(--fg3)" : "var(--fg)";
      fprintf(f,
        "<div style=\"display:flex;align-items:center;gap:10px\">\n"
        "<span style=\"display:inline-block;width:10px;height:10px;"
        "border-radius:2px;background:rgb(%d,%d,%d);flex-shrink:0\"></span>\n"
        "<span style=\"flex:1;color:var(--fg2);font-family:var(--mono);"
        "font-size:11px\">%02X:%d</span>\n"
        "<span style=\"font-family:var(--mono);color:%s;font-weight:500;"
        "font-size:11px\">%.0f B</span>\n"
        "<span style=\"font-family:var(--mono);color:var(--fg3);"
        "min-width:42px;text-align:right;font-size:11px\">%.1f%%</span>\n"
        "</div>\n",
        mc_pal[pi][0], mc_pal[pi][1], mc_pal[pi][2],
        s->model_masks[m], s->model_weights[m],
        clr, bits / 8.0, pct);
    }
    if (s->num_models > show_n) {
      double rest_bits = 0;
      int rest_count = s->num_models - show_n;
      for (int i = show_n; i < s->num_models; i++) {
        double b = s->model_bits_saved[mc_order[i]];
        if (b > 0) rest_bits += b;
      }
      double rest_pct = 100.0 * rest_bits / mc_total;
      /* Collapsed summary row; click to reveal the individual hidden models. */
      fprintf(f,
        "<details>\n"
        "<summary class=\"mc-more\" style=\"display:flex;align-items:center;"
        "gap:10px;color:var(--fg3);font-size:11px\">\n"
        "<span class=\"mc-caret\" style=\"display:inline-block;width:10px;"
        "flex-shrink:0;text-align:center;font-size:9px\">&#9656;</span>\n"
        "<span style=\"flex:1\">+%d more</span>\n"
        "<span style=\"font-family:var(--mono)\">%.0f B</span>\n"
        "<span style=\"font-family:var(--mono);min-width:42px;"
        "text-align:right\">%.1f%%</span>\n"
        "</summary>\n"
        "<div style=\"display:flex;flex-direction:column;gap:6px;"
        "margin-top:6px\">\n",
        rest_count, rest_bits / 8.0, rest_pct);
      for (int i = show_n; i < s->num_models; i++) {
        int m = mc_order[i];
        double bits = s->model_bits_saved[m];
        double pct = 100.0 * bits / mc_total;
        int pi = i % mc_npal;
        const char *clr = bits <= 0 ? "var(--fg3)" : "var(--fg)";
        fprintf(f,
          "<div style=\"display:flex;align-items:center;gap:10px\">\n"
          "<span style=\"display:inline-block;width:10px;height:10px;"
          "border-radius:2px;background:rgb(%d,%d,%d);flex-shrink:0\"></span>\n"
          "<span style=\"flex:1;color:var(--fg2);font-family:var(--mono);"
          "font-size:11px\">%02X:%d</span>\n"
          "<span style=\"font-family:var(--mono);color:%s;font-weight:500;"
          "font-size:11px\">%.0f B</span>\n"
          "<span style=\"font-family:var(--mono);color:var(--fg3);"
          "min-width:42px;text-align:right;font-size:11px\">%.1f%%</span>\n"
          "</div>\n",
          mc_pal[pi][0], mc_pal[pi][1], mc_pal[pi][2],
          s->model_masks[m], s->model_weights[m],
          clr, bits / 8.0, pct);
      }
      fprintf(f, "</div>\n</details>\n");
    }
    fprintf(f, "</div>\n"
      "</div>\n"
      "</div>\n\n");
  }

  /* ── Byte Frequency Heatmap ── */
  {
    unsigned int fmax = 0;
    int nunique = 0;
    for (int i = 0; i < 256; i++) {
      if (s->byte_freq[i] > fmax) fmax = s->byte_freq[i];
      if (s->byte_freq[i] > 0) nunique++;
    }
    float log_fmax = fmax > 1 ? fast_log2f((float)fmax) : 1;

    int cell = 28, gap = 1;
    int grid = cell * 16 + gap * 15;
    int hdr = 18, row_lbl = 24;
    int svg_w = row_lbl + grid + 4;
    int svg_h = hdr + grid + 4;

    fprintf(f,
      "<div class=\"card full\" id=\"sec-bytefreq\">\n"
      "<h2>Byte Frequency</h2>\n"
      "<p class=\"desc\">16&times;16 grid of all 256 byte values. "
      "Intensity = log frequency. %d unique bytes, max count = %u.</p>\n",
      nunique, fmax);
    fprintf(f, "<div class=\"scrub-wrap\">\n");
    fprintf(f, "<div id=\"bfreq-tip\" class=\"hover-tip\"></div>\n");
    fprintf(f,
      "<svg id=\"bfreq-svg\" width=\"100%%\" viewBox=\"0 0 %d %d\" "
      "style=\"font-family:var(--mono);display:block\">\n",
      svg_w, svg_h);

    for (int c2 = 0; c2 < 16; c2++) {
      int x = row_lbl + c2 * (cell + gap) + cell / 2;
      fprintf(f,
        "<text x=\"%d\" y=\"%d\" text-anchor=\"middle\" "
        "font-size=\"8\" fill=\"var(--fg3)\">%X</text>\n", x, hdr - 5, c2);
    }

    for (int r = 0; r < 16; r++) {
      int y = hdr + r * (cell + gap);
      fprintf(f,
        "<text x=\"%d\" y=\"%d\" text-anchor=\"end\" "
        "font-size=\"8\" fill=\"var(--fg3)\">%X_</text>\n",
        row_lbl - 4, y + cell / 2 + 3, r);
      for (int c2 = 0; c2 < 16; c2++) {
        int byte_val = r * 16 + c2;
        unsigned int freq = s->byte_freq[byte_val];
        int x = row_lbl + c2 * (cell + gap);
        float intensity = 0;
        if (freq > 0 && fmax > 0) {
          intensity = (fast_log2f((float)freq) + 1) / (log_fmax + 1);
          if (intensity < 0.08f) intensity = 0.08f;
          if (intensity > 1.0f) intensity = 1.0f;
        }
        char fill[32];
        if (freq == 0) {
          snprintf(fill, sizeof(fill), "var(--bg3)");
        } else {
          /* dark teal to bright cyan */
          int cr = (int)(13 + (34 - 13) * intensity);
          int cg = (int)(40 + (211 - 40) * intensity);
          int cb = (int)(60 + (238 - 60) * intensity);
          snprintf(fill, sizeof(fill), "rgb(%d,%d,%d)", cr, cg, cb);
        }
        char label[8] = "";
        if (byte_val >= 0x20 && byte_val <= 0x7E && byte_val != '<' &&
            byte_val != '>' && byte_val != '&' && byte_val != '"')
          snprintf(label, sizeof(label), "%c", byte_val);

        fprintf(f,
          "<rect x=\"%d\" y=\"%d\" width=\"%d\" height=\"%d\" "
          "rx=\"2\" fill=\"%s\" data-b=\"%d\" "
          "style=\"cursor:pointer\"/>\n",
          x, y, cell, cell, fill, byte_val);

        if (label[0]) {
          int text_bright = intensity > 0.35f;
          fprintf(f,
            "<text x=\"%d\" y=\"%d\" text-anchor=\"middle\" "
            "font-size=\"9\" fill=\"%s\" pointer-events=\"none\">%s</text>\n",
            x + cell / 2, y + cell / 2 + 3,
            text_bright ? "#0c0e14" : "#6b7186", label);
        }
      }
    }
    fprintf(f, "</svg>\n");
    fprintf(f, "</div>\n"); /* close scrub-wrap */

    /* ── Detail panel ── */
    fprintf(f, "<div id=\"bfreq-detail\" class=\"cd-panel\"></div>\n");

    /* ── Emit frequency data + click handler ── */
    fprintf(f, "<script>\nvar BF=[");
    for (int i = 0; i < 256; i++)
      fprintf(f, "%s%u", i ? "," : "", s->byte_freq[i]);
    fprintf(f, "];\n");
    fprintf(f, "var BF_TOTAL=%d;\n", s->input_size);

    fprintf(f,
      "(function(){\n"
      "var panel=document.getElementById('bfreq-detail');\n"
      "var selRect=null;\n"
      "document.getElementById('bfreq-svg').addEventListener('click',function(e){\n"
      "  var r=e.target; if(r.tagName!=='rect') return;\n"
      "  var bv=r.getAttribute('data-b'); if(bv===null) return;\n"
      "  bv=parseInt(bv);\n"
      "  if(selRect) selRect.classList.remove('cmap-sel');\n"
      "  r.classList.add('cmap-sel'); selRect=r;\n"
      "  var freq=BF[bv];\n"
      "  var pct=BF_TOTAL>0?(100*freq/BF_TOTAL):0;\n"
      "  var ch=(bv>=0x20&&bv<=0x7E)?String.fromCharCode(bv):null;\n"
      "  var selfInfo=freq>0?-Math.log2(freq/BF_TOTAL):0;\n"
      "  var h='<div class=\"cd-head\">';\n"
      "  h+='<span class=\"cd-byte\">0x'+(bv<16?'0':'')+bv.toString(16).toUpperCase();\n"
      "  if(ch) h+=\" '\"+ch+\"'\";\n"
      "  h+='</span>';\n"
      "  h+='<span class=\"cd-sub\">decimal '+bv+'</span>';\n"
      "  h+='</div>';\n"
      /* stats table */
      "  h+='<div style=\"display:grid;grid-template-columns:1fr 1fr 1fr;gap:12px 20px;"
      "margin:10px 0;font-size:12px\">';\n"
      "  h+='<div><span style=\"color:var(--fg3)\">Count</span><br>'"
      "    +'<span style=\"font-family:var(--mono);font-weight:600;color:var(--acc)\">'+freq+'</span></div>';\n"
      "  h+='<div><span style=\"color:var(--fg3)\">Frequency</span><br>'"
      "    +'<span style=\"font-family:var(--mono);font-weight:600;color:var(--acc)\">'+pct.toFixed(2)+'%%</span></div>';\n"
      "  h+='<div><span style=\"color:var(--fg3)\">Self-information</span><br>'"
      "    +'<span style=\"font-family:var(--mono);font-weight:600;color:var(--acc)\">'+(freq>0?selfInfo.toFixed(2):'\\u221e')+' bits</span></div>';\n"
      "  h+='</div>';\n"
      /* avg encoding cost from BD */
      "  if(typeof BD!=='undefined' && BD.length>0){\n"
      "    var hex=(bv<16?'0':'')+bv.toString(16).toUpperCase();\n"
      "    var sum=0,cnt=0,offsets=[];\n"
      "    for(var i=0;i<BD.length;i++){\n"
      "      if(BD[i].h===hex){sum+=BD[i].c;cnt++;if(offsets.length<12)offsets.push(BD[i].o);}\n"
      "    }\n"
      "    if(cnt>0){\n"
      "      var avg=sum/cnt;\n"
      "      var avgClr=avg<3?'#34d399':avg<6?'#fbbf24':'#f87171';\n"
      "      h+='<div style=\"margin:6px 0;font-size:12px\">';\n"
      "      h+='<span style=\"color:var(--fg3)\">Avg encoding cost: </span>';\n"
      "      h+='<span style=\"font-family:var(--mono);font-weight:600;color:'+avgClr+'\">'+avg.toFixed(2)+' bits/byte</span>';\n"
      "      h+=' <span style=\"color:var(--fg3)\">('+selfInfo.toFixed(2)+' H\\u2080)</span>';\n"
      "      h+='</div>';\n"
      "      if(offsets.length>0){\n"
      "        h+='<div style=\"margin-top:6px;font-size:11px;color:var(--fg3)\">Offsets: ';\n"
      "        h+='<span style=\"font-family:var(--mono);color:var(--fg2)\">';\n"
      "        for(var j=0;j<offsets.length;j++){\n"
      "          if(j>0) h+=', ';\n"
      "          h+=offsets[j];\n"
      "        }\n"
      "        if(cnt>offsets.length) h+=', \\u2026 ('+(cnt-offsets.length)+' more)';\n"
      "        h+='</span></div>';\n"
      "      }\n"
      "    }\n"
      "  }\n"
      "  panel.innerHTML=h;\n"
      "  panel.style.display='block';\n"
      "});\n"
      "/* hover tooltip */\n"
      "var btip=document.getElementById('bfreq-tip');\n"
      "var bsvg=document.getElementById('bfreq-svg');\n"
      "function hideBtip(){btip.style.display='none';}\n"
      "bsvg.addEventListener('mousemove',function(e){\n"
      "  var r=e.target; if(r.tagName!=='rect'){hideBtip();return;}\n"
      "  var bv=r.getAttribute('data-b'); if(bv===null) return;\n"
      "  bv=parseInt(bv);\n"
      "  var freq=BF[bv];\n"
      "  var pct=BF_TOTAL>0?(100*freq/BF_TOTAL):0;\n"
      "  var ch=(bv>=0x20&&bv<=0x7E)?String.fromCharCode(bv):null;\n"
      "  var hex='0x'+(bv<16?'0':'')+bv.toString(16).toUpperCase();\n"
      "  var label=hex+(ch?\" '\"+ch+\"'\":'');\n"
      "  var selfInfo=freq>0?-Math.log2(freq/BF_TOTAL):0;\n"
      "  var h='<div class=\"tip-row\"><span style=\"color:var(--fg)\">'+label+'</span></div>'\n"
      "    +'<div style=\"border-top:1px solid var(--bdr);margin:4px 0 2px;padding-top:4px\"></div>'\n"
      "    +'<div class=\"tip-row\"><span style=\"color:var(--fg3)\">count</span>'\n"
      "    +'<span style=\"color:var(--acc);font-weight:600\">'+freq+'</span></div>'\n"
      "    +'<div class=\"tip-row\"><span style=\"color:var(--fg3)\">freq</span>'\n"
      "    +'<span>'+pct.toFixed(2)+'%</span></div>'\n"
      "    +'<div class=\"tip-row\"><span style=\"color:var(--fg3)\">H\\u2080</span>'\n"
      "    +'<span>'+(freq>0?selfInfo.toFixed(2)+' bits':'\\u221e')+'</span></div>';\n"
      "  btip.innerHTML=h;\n"
      "  var pr=bsvg.parentNode.getBoundingClientRect();\n"
      "  var tx=(e.clientX-pr.left)+14, ty=(e.clientY-pr.top)-60;\n"
      "  if(tx+180>pr.width) tx=(e.clientX-pr.left)-180;\n"
      "  if(ty<0) ty=(e.clientY-pr.top)+18;\n"
      "  btip.style.left=tx+'px'; btip.style.top=ty+'px';\n"
      "  btip.style.display='block';\n"
      "});\n"
      "bsvg.addEventListener('mouseleave',hideBtip);\n"
      "})();\n");

    fprintf(f, "</script>\n");
    fprintf(f, "</div>\n\n");
  }

  /* ── Byte Bigram Matrix ── */
  if (s->input_data && s->input_size > 1) {
    fprintf(f,
      "<div class=\"card full\" id=\"sec-bigram\">\n"
      "<h2>Byte Bigram Matrix</h2>\n"
      "<p class=\"desc\">256&times;256 matrix of byte pairs. "
      "Row = preceding byte, column = following byte. "
      "Cyan = log frequency; cost mode colors each transition by its "
      "average encoding cost. Click to inspect.</p>\n");
    fprintf(f, "%s",
      "<div class=\"slider-row\">"
      "<span>Color:</span>"
      "<button id=\"bg-mode-freq\" type=\"button\">frequency</button>"
      "<button id=\"bg-mode-cost\" type=\"button\">avg cost</button>"
      "</div>\n");

    /* axis labels above canvas */
    fprintf(f,
      "<div id=\"bigram-wrap\">\n");

    /* top axis */
    fprintf(f,
      "<div style=\"margin-left:44px;display:flex;margin-bottom:2px;"
      "font-family:var(--mono);font-size:8px;color:var(--fg3)\">");
    for (int c = 0; c < 16; c++)
      fprintf(f, "<span style=\"flex:1;text-align:center\">%X0</span>", c);
    fprintf(f, "</div>\n");

    /* canvas with row labels */
    fprintf(f,
      "<div style=\"display:flex;align-items:stretch;gap:4px;position:relative\">\n"
      "<div style=\"width:40px;flex-shrink:0;font-family:var(--mono);"
      "font-size:8px;color:var(--fg3);display:flex;flex-direction:column\">\n");
    for (int r = 0; r < 16; r++)
      fprintf(f, "<div style=\"flex:1;display:flex;align-items:center;"
              "justify-content:flex-end\">%X0</div>\n", r);
    fprintf(f, "</div>\n");

    fprintf(f,
      "<canvas id=\"bigram-cv\" width=\"512\" height=\"512\" "
      "style=\"flex:1;min-width:0;aspect-ratio:1;"
      "cursor:crosshair;image-rendering:pixelated;"
      "background:var(--bg3);border:1px solid var(--bdr)\"></canvas>\n"
      /* crosshair lines + hover tooltip, positioned over canvas */
      "<div id=\"bigram-v\" style=\"position:absolute;width:1px;"
      "background:var(--fg3);pointer-events:none;display:none\"></div>\n"
      "<div id=\"bigram-h\" style=\"position:absolute;height:1px;"
      "background:var(--fg3);pointer-events:none;display:none\"></div>\n"
      "<div id=\"bigram-tip\" class=\"hover-tip\"></div>\n"
      "</div>\n"
      "</div>\n");

    /* detail panel */
    fprintf(f, "<div id=\"bigram-detail\" class=\"cd-panel\"></div>\n");

    /* emit bigram counts as flat array */
    fprintf(f, "<script>\n");

    /* compute bigrams in C for efficiency */
    fprintf(f, "var BG=new Uint32Array(65536);\n");
    {
      unsigned int *bg = (unsigned int *)calloc(65536, sizeof(unsigned int));
      unsigned int bg_max = 0;
      unsigned int bg_total = 0;
      for (int i = 0; i + 1 < s->input_size; i++) {
        unsigned int idx = s->input_data[i] * 256 + s->input_data[i + 1];
        bg[idx]++;
        if (bg[idx] > bg_max) bg_max = bg[idx];
        bg_total++;
      }
      /* emit only nonzero entries for compactness */
      fprintf(f, "var BG_MAX=%u,BG_TOTAL=%u;\n", bg_max, bg_total);
      for (int i = 0; i < 65536; i++) {
        if (bg[i] > 0)
          fprintf(f, "BG[%d]=%u;\n", i, bg[i]);
      }
      free(bg);
    }

    /* render + interact */
    fprintf(f,
      "(function(){\n"
      "var cv=document.getElementById('bigram-cv');\n"
      "var ctx=cv.getContext('2d');\n"
      "var img=ctx.createImageData(512,512);\n"
      "var d=img.data;\n"
      "var logMax=BG_MAX>1?Math.log2(BG_MAX):1;\n"
      "var bgMode='freq';\n"
      "/* per-transition cost, built lazily from BD (defined later in the\n"
      "   document, but available by the time the user can click) */\n"
      "var BGC=null,BGN=null,BGC_MAX=0;\n"
      "function buildCost(){\n"
      "  if(BGC||typeof BD==='undefined'||BD.length<2) return;\n"
      "  BGC=new Float32Array(65536); BGN=new Uint32Array(65536);\n"
      "  for(var i=1;i<BD.length;i++){\n"
      "    var idx=parseInt(BD[i-1].h,16)*256+parseInt(BD[i].h,16);\n"
      "    BGC[idx]+=BD[i].c; BGN[idx]++;\n"
      "  }\n"
      "  for(var i=0;i<65536;i++){\n"
      "    if(BGN[i]){var a=BGC[i]/BGN[i]; if(a>BGC_MAX) BGC_MAX=a;}\n"
      "  }\n"
      "  if(BGC_MAX<=0) BGC_MAX=1;\n"
      "}\n"
      "function paint(){\n"
      "for(var r=0;r<256;r++){\n"
      "  for(var c=0;c<256;c++){\n"
      "    var i2=r*256+c;\n"
      "    /* 2x2 pixel per cell. Use alpha-based intensity so the canvas\n"
      "       stays theme-aware - empty cells are fully transparent and\n"
      "       the card background shows through */\n"
      "    var cr,cg,cb,ca;\n"
      "    if(bgMode==='cost'&&BGC){\n"
      "      var n=BGN[i2];\n"
      "      if(!n){cr=0;cg=0;cb=0;ca=0;}\n"
      "      else{\n"
      "        var t=BGC[i2]/n/BGC_MAX; if(t>1)t=1;\n"
      "        if(t<0.5){var u=t*2;cr=16+(180-16)*u;cg=185+(140-185)*u;"
      "cb=129+(40-129)*u;}\n"
      "        else{var u=(t-0.5)*2;cr=180+(248-180)*u;cg=140+(113-140)*u;"
      "cb=40+(113-40)*u;}\n"
      "        cr|=0;cg|=0;cb|=0;ca=255;\n"
      "      }\n"
      "    } else {\n"
      "      var v=BG[i2];\n"
      "      var t=0;\n"
      "      if(v>0) t=Math.min((Math.log2(v)+1)/(logMax+1),1);\n"
      "      if(v>0&&t<0.1) t=0.1;\n"
      "      if(v===0){cr=0;cg=0;cb=0;ca=0;}\n"
      "      else{cr=34;cg=211;cb=238;ca=Math.round(255*t);}\n"
      "    }\n"
      "    for(var dy=0;dy<2;dy++)for(var dx=0;dx<2;dx++){\n"
      "      var px=((r*2+dy)*512+(c*2+dx))*4;\n"
      "      d[px]=cr;d[px+1]=cg;d[px+2]=cb;d[px+3]=ca;\n"
      "    }\n"
      "  }\n"
      "}\n"
      "ctx.putImageData(img,0,0);\n"
      "}\n"
      "var bF=document.getElementById('bg-mode-freq');\n"
      "var bC=document.getElementById('bg-mode-cost');\n"
      "function styleBg(){\n"
      "  if(!bF) return;\n"
      "  bF.style.color=bgMode==='freq'?'var(--acc)':'';\n"
      "  bC.style.color=bgMode==='cost'?'var(--acc)':'';\n"
      "}\n"
      "if(bF){\n"
      "  bF.addEventListener('click',function(){bgMode='freq';styleBg();paint();});\n"
      "  bC.addEventListener('click',function(){\n"
      "    buildCost();\n"
      "    if(!BGC) return;\n"
      "    bgMode='cost';styleBg();paint();\n"
      "  });\n"
      "}\n"
      "styleBg();\n"
      "paint();\n"
      "\n"
      /* hover crosshair + click */
      "var panel=document.getElementById('bigram-detail');\n"
      "function fmtByte(b){\n"
      "  var h=(b<16?'0':'')+b.toString(16).toUpperCase();\n"
      "  var ch=(b>=0x20&&b<=0x7E)?\" '\"+String.fromCharCode(b)+\"'\":'';\n"
      "  return '0x'+h+ch;\n"
      "}\n"
      "cv.addEventListener('click',function(e){\n"
      "  var rect=cv.getBoundingClientRect();\n"
      "  var sx=(e.clientX-rect.left)/rect.width;\n"
      "  var sy=(e.clientY-rect.top)/rect.height;\n"
      "  var row=Math.floor(sy*256);\n"
      "  var col=Math.floor(sx*256);\n"
      "  if(row<0)row=0;if(row>255)row=255;\n"
      "  if(col<0)col=0;if(col>255)col=255;\n"
      "  var freq=BG[row*256+col];\n"
      "  var pct=BG_TOTAL>0?(100*freq/BG_TOTAL):0;\n"
      "  var h='<div class=\"cd-head\">';\n"
      "  h+='<span class=\"cd-byte\">'+fmtByte(row)+' \\u2192 '+fmtByte(col)+'</span>';\n"
      "  h+='</div>';\n"
      "  h+='<div style=\"display:grid;grid-template-columns:1fr 1fr 1fr;gap:12px 20px;"
      "margin:10px 0;font-size:12px\">';\n"
      "  h+='<div><span style=\"color:var(--fg3)\">Count</span><br>'"
      "    +'<span style=\"font-family:var(--mono);font-weight:600;color:var(--acc)\">'+freq+'</span></div>';\n"
      "  h+='<div><span style=\"color:var(--fg3)\">Of all bigrams</span><br>'"
      "    +'<span style=\"font-family:var(--mono);font-weight:600;color:var(--acc)\">'+pct.toFixed(3)+'%%</span></div>';\n"
      /* conditional probability P(col|row) */
      "  var rowTotal=0;\n"
      "  for(var c=0;c<256;c++) rowTotal+=BG[row*256+c];\n"
      "  var condProb=rowTotal>0?(100*freq/rowTotal):0;\n"
      "  h+='<div><span style=\"color:var(--fg3)\">P('+fmtByte(col).split(' ')[0]+' | '+fmtByte(row).split(' ')[0]+')</span><br>'"
      "    +'<span style=\"font-family:var(--mono);font-weight:600;color:var(--acc)\">'+condProb.toFixed(1)+'%%</span></div>';\n"
      "  h+='</div>';\n"
      /* top successors for this row byte */
      "  if(rowTotal>0){\n"
      "    var succs=[];\n"
      "    for(var c=0;c<256;c++){var f=BG[row*256+c];if(f>0)succs.push({b:c,f:f});}\n"
      "    succs.sort(function(a,b){return b.f-a.f;});\n"
      "    h+='<div style=\"margin-top:8px;font-size:11px\">';\n"
      "    h+='<span style=\"color:var(--fg3)\">Top successors of '+fmtByte(row)+': </span>';\n"
      "    var shown=Math.min(succs.length,8);\n"
      "    for(var i=0;i<shown;i++){\n"
      "      var s=succs[i];\n"
      "      var sp=100*s.f/rowTotal;\n"
      "      var isThis=s.b===col;\n"
      "      h+=(i>0?', ':'');\n"
      "      if(isThis) h+='<strong style=\"color:var(--acc)\">';\n"
      "      h+='<span style=\"font-family:var(--mono)\">'+fmtByte(s.b).split(' ')[0]+'</span>';\n"
      "      h+=' <span style=\"color:var(--fg3)\">('+sp.toFixed(1)+'%%)</span>';\n"
      "      if(isThis) h+='</strong>';\n"
      "    }\n"
      "    if(succs.length>shown) h+=', \\u2026';\n"
      "    h+='</div>';\n"
      "  }\n"
      /* top predecessors for this col byte */
      "  var colTotal=0;\n"
      "  for(var r=0;r<256;r++) colTotal+=BG[r*256+col];\n"
      "  if(colTotal>0){\n"
      "    var preds=[];\n"
      "    for(var r=0;r<256;r++){var f=BG[r*256+col];if(f>0)preds.push({b:r,f:f});}\n"
      "    preds.sort(function(a,b){return b.f-a.f;});\n"
      "    h+='<div style=\"margin-top:4px;font-size:11px\">';\n"
      "    h+='<span style=\"color:var(--fg3)\">Top predecessors of '+fmtByte(col)+': </span>';\n"
      "    var shown=Math.min(preds.length,8);\n"
      "    for(var i=0;i<shown;i++){\n"
      "      var p=preds[i];\n"
      "      var pp=100*p.f/colTotal;\n"
      "      var isThis=p.b===row;\n"
      "      h+=(i>0?', ':'');\n"
      "      if(isThis) h+='<strong style=\"color:var(--acc)\">';\n"
      "      h+='<span style=\"font-family:var(--mono)\">'+fmtByte(p.b).split(' ')[0]+'</span>';\n"
      "      h+=' <span style=\"color:var(--fg3)\">('+pp.toFixed(1)+'%%)</span>';\n"
      "      if(isThis) h+='</strong>';\n"
      "    }\n"
      "    if(preds.length>shown) h+=', \\u2026';\n"
      "    h+='</div>';\n"
      "  }\n"
      "  panel.innerHTML=h;\n"
      "  panel.style.display='block';\n"
      "  panel.scrollIntoView({behavior:'smooth',block:'nearest'});\n"
      "});\n"
      "/* hover crosshair + quick tooltip */\n"
      "var vLine=document.getElementById('bigram-v');\n"
      "var hLine=document.getElementById('bigram-h');\n"
      "var tip=document.getElementById('bigram-tip');\n"
      "function hideCross(){vLine.style.display='none';"
      "hLine.style.display='none';tip.style.display='none';}\n"
      "cv.addEventListener('mousemove',function(e){\n"
      "  var rect=cv.getBoundingClientRect();\n"
      "  var sx=(e.clientX-rect.left)/rect.width;\n"
      "  var sy=(e.clientY-rect.top)/rect.height;\n"
      "  if(sx<0||sx>1||sy<0||sy>1){hideCross();return;}\n"
      "  var row=Math.floor(sy*256), col=Math.floor(sx*256);\n"
      "  if(row<0)row=0;if(row>255)row=255;\n"
      "  if(col<0)col=0;if(col>255)col=255;\n"
      "  /* parent is positioning context */\n"
      "  var pr=cv.parentNode.getBoundingClientRect();\n"
      "  var cellW=rect.width/256, cellH=rect.height/256;\n"
      "  var cxLeft=rect.left-pr.left + (col+0.5)*cellW;\n"
      "  var cyTop =rect.top -pr.top  + (row+0.5)*cellH;\n"
      "  vLine.style.left=cxLeft+'px';\n"
      "  vLine.style.top=(rect.top-pr.top)+'px';\n"
      "  vLine.style.height=rect.height+'px';\n"
      "  vLine.style.display='block';\n"
      "  hLine.style.top=cyTop+'px';\n"
      "  hLine.style.left=(rect.left-pr.left)+'px';\n"
      "  hLine.style.width=rect.width+'px';\n"
      "  hLine.style.display='block';\n"
      "  var freq=BG[row*256+col];\n"
      "  var rowTotal=0;\n"
      "  for(var c=0;c<256;c++) rowTotal+=BG[row*256+c];\n"
      "  var cond=rowTotal>0?(100*freq/rowTotal):0;\n"
      "  var pct=BG_TOTAL>0?(100*freq/BG_TOTAL):0;\n"
      "  function fmt(b){var h=(b<16?'0':'')+b.toString(16).toUpperCase();\n"
      "    var ch=(b>=0x20&&b<=0x7E)?\" '\"+String.fromCharCode(b)+\"'\":'';\n"
      "    return '0x'+h+ch;}\n"
      "  var hh='<div class=\"tip-row\"><span style=\"color:var(--fg)\">'+fmt(row)\n"
      "    +' \\u2192 '+fmt(col)+'</span></div>'\n"
      "    +'<div style=\"border-top:1px solid var(--bdr);margin:4px 0 2px;padding-top:4px\"></div>'\n"
      "    +'<div class=\"tip-row\"><span style=\"color:var(--fg3)\">count</span>'\n"
      "    +'<span style=\"color:var(--acc);font-weight:600\">'+freq+'</span></div>'\n"
      "    +'<div class=\"tip-row\"><span style=\"color:var(--fg3)\">of all</span>'\n"
      "    +'<span>'+pct.toFixed(2)+'%</span></div>'\n"
      "    +'<div class=\"tip-row\"><span style=\"color:var(--fg3)\">P(col|row)</span>'\n"
      "    +'<span>'+cond.toFixed(1)+'%</span></div>';\n"
      "  if(BGC&&BGN[row*256+col]){\n"
      "    var ac2=BGC[row*256+col]/BGN[row*256+col];\n"
      "    var acClr=ac2<3?'#34d399':ac2<6?'#fbbf24':'#f87171';\n"
      "    hh+='<div class=\"tip-row\"><span style=\"color:var(--fg3)\">avg cost</span>'\n"
      "      +'<span style=\"color:'+acClr+'\">'+ac2.toFixed(2)+' bits</span></div>';\n"
      "  }\n"
      "  tip.innerHTML=hh;\n"
      "  var tx=(e.clientX-pr.left)+14, ty=(e.clientY-pr.top)-50;\n"
      "  if(tx+200>pr.width) tx=(e.clientX-pr.left)-200;\n"
      "  if(ty<0) ty=(e.clientY-pr.top)+18;\n"
      "  tip.style.left=tx+'px'; tip.style.top=ty+'px';\n"
      "  tip.style.display='block';\n"
      "});\n"
      "cv.addEventListener('mouseleave',hideCross);\n"
      "})();\n");

    fprintf(f, "</script>\n");
    fprintf(f, "</div>\n\n");
  }

  /* ── Compressibility Map ── */
  if (s->byte_costs && s->num_data_bytes > 0) {
    int nb = s->num_data_bytes;
    float cmin = 1e9f, cmax = -1e9f;
    for (int i = 0; i < nb; i++) {
      if (s->byte_costs[i] < cmin) cmin = s->byte_costs[i];
      if (s->byte_costs[i] > cmax) cmax = s->byte_costs[i];
    }
    if (cmax <= cmin) cmax = cmin + 1;

    int cols, cell;
    if (nb <= 64) { cols = nb < 16 ? nb : 16; cell = 16; }
    else if (nb <= 512)  { cols = 32; cell = 14; }
    else if (nb <= 2048) { cols = 64; cell = 10; }
    else if (nb <= 4096) { cols = 64; cell = 8; }
    else if (nb <= 8192) { cols = 96; cell = 6; }
    else { cols = 128; cell = 6; }
    int rows = (nb + cols - 1) / cols;
    int cg = cell >= 6 ? 1 : 0;
    int stride = cell + cg;
    int svg_w = cols * stride - cg + 2;
    int svg_h = rows * stride - cg + 2;

    fprintf(f,
      "<div class=\"card full\" id=\"sec-cmap\">\n"
      "<h2>Compressibility Map</h2>\n"
      "<p class=\"desc\">Each cell = one byte. "
      "<span style=\"color:#34d399\">\xe2\x96\x88</span> green = "
      "compressible, "
      "<span style=\"color:#f87171\">\xe2\x96\x88</span> red = "
      "hard to compress. "
      "%d bytes, %d&times;%d grid.</p>\n", nb, cols, rows);

    fprintf(f,
      "<div class=\"slider-row\">"
      "<span>Scale max:</span>"
      "<input type=\"range\" id=\"cmap-slider\" min=\"1\" max=\"%.0f\" "
      "value=\"%.0f\" step=\"0.5\">"
      "<span id=\"cmap-val\" style=\"font-family:var(--mono);"
      "min-width:70px\">%.1f bits</span>"
      "<button onclick=\"document.getElementById('cmap-slider').value=8;"
      "document.getElementById('cmap-slider').dispatchEvent("
      "new Event('input'))\">8.0 (H\xe2\x82\x80)</button>"
      "<button onclick=\"document.getElementById('cmap-slider').value=%.0f;"
      "document.getElementById('cmap-slider').dispatchEvent("
      "new Event('input'))\">Auto (%.1f)</button>"
      "</div>\n",
      cmax + 1, cmax, cmax, cmax, cmax);

    fprintf(f, "<div class=\"scrub-wrap\">\n");
    fprintf(f, "<div id=\"cmap-tip\" class=\"hover-tip\"></div>\n");
    fprintf(f,
      "<svg id=\"cmap-svg\" width=\"100%%\" viewBox=\"0 0 %d %d\" "
      "style=\"display:block\">\n", svg_w, svg_h);

    for (int i = 0; i < nb; i++) {
      int col = i % cols, row2 = i / cols;
      int x = 1 + col * stride, y = 1 + row2 * stride;
      float cost = s->byte_costs[i];
      float t = cost / cmax;
      if (t < 0) t = 0; if (t > 1) t = 1;
      int cr, ccg, cb;
      if (t < 0.5f) {
        float u = t * 2;
        cr = (int)(16 + (180 - 16) * u);
        ccg = (int)(185 + (140 - 185) * u);
        cb = (int)(129 + (40 - 129) * u);
      } else {
        float u = (t - 0.5f) * 2;
        cr = (int)(180 + (248 - 180) * u);
        ccg = (int)(140 + (113 - 140) * u);
        cb = (int)(40 + (113 - 40) * u);
      }
      unsigned char bval = s->input_data ? s->input_data[i] : 0;
      fprintf(f,
        "<rect x=\"%d\" y=\"%d\" width=\"%d\" height=\"%d\" "
        "fill=\"rgb(%d,%d,%d)\" data-c=\"%.2f\" data-i=\"%d\" "
        "style=\"cursor:pointer\"/>\n",
        x, y, cell, cell, cr, ccg, cb, cost, i);
      if (cell >= 6 && bval >= 0x20 && bval <= 0x7E && bval != '<'
          && bval != '>' && bval != '&' && bval != '"') {
        int bright = t < 0.6f;
        fprintf(f,
          "<text x=\"%d\" y=\"%d\" text-anchor=\"middle\" "
          "font-size=\"%d\" fill=\"%s\" pointer-events=\"none\" "
          "opacity=\".8\">%c</text>\n",
          x + cell / 2, y + cell / 2 + (cell >= 10 ? 3 : cell >= 8 ? 2 : 2),
          cell >= 10 ? 7 : cell >= 8 ? 6 : 5,
          bright ? "#0c0e14" : "rgba(255,255,255,.7)",
          bval);
      }
    }
    fprintf(f, "</svg>\n");
    fprintf(f, "</div>\n"); /* close scrub-wrap */

    /* ── Detail panel HTML ── */
    fprintf(f, "<div id=\"cmap-detail\" class=\"cd-panel\"></div>\n");

    /* ── Emit byte data as JS ── */
    fprintf(f, "<script>\n");
    fprintf(f, "var BD=[\n");
    for (int i = 0; i < nb; i++) {
      unsigned char bval = s->input_data ? s->input_data[i] : 0;
      float cost = s->byte_costs[i];
      fprintf(f, "{o:%d,h:'%02X',c:%.2f", i, bval, cost);
      /* printable ASCII character */
      if (bval >= 0x20 && bval <= 0x7E && bval != '\'' && bval != '\\')
        fprintf(f, ",ch:'%c'", bval);
      else if (bval == '\'')
        fprintf(f, ",ch:\"'\"");
      else if (bval == '\\')
        fprintf(f, ",ch:'\\\\'");
      else
        fprintf(f, ",ch:null");
      /* per-model contributions */
      if (s->byte_model_contrib) {
        fprintf(f, ",m:[");
        for (int m = 0; m < s->num_models; m++) {
          float v = s->byte_model_contrib[i * s->num_models + m];
          fprintf(f, "%s%.2f", m ? "," : "", v);
        }
        fprintf(f, "]");
      }
      fprintf(f, "}%s\n", i < nb - 1 ? "," : "");
    }
    fprintf(f, "];\n");

    /* model info */
    fprintf(f, "var MI=[");
    for (int m = 0; m < s->num_models; m++)
      fprintf(f, "%s{mask:'%02X',w:%d}", m ? "," : "",
              s->model_masks[m], s->model_weights[m]);
    fprintf(f, "];\n");

    /* slider logic */
    fprintf(f,
      "var sl=document.getElementById('cmap-slider');\n"
      "var lbl=document.getElementById('cmap-val');\n"
      "var rects=document.querySelectorAll('#cmap-svg rect[data-c]');\n"
      "sl.addEventListener('input',function(){\n"
      "  var mx=parseFloat(sl.value);\n"
      "  lbl.textContent=mx.toFixed(1)+' bits';\n"
      "  for(var i=0;i<rects.length;i++){\n"
      "    var c=parseFloat(rects[i].getAttribute('data-c'));\n"
      "    var t=c/mx; if(t<0)t=0; if(t>1)t=1;\n"
      "    var r,g,b;\n"
      "    if(t<0.5){var u=t*2;\n"
      "      r=16+(180-16)*u;g=185+(140-185)*u;b=129+(40-129)*u;\n"
      "    }else{var u=(t-0.5)*2;\n"
      "      r=180+(248-180)*u;g=140+(113-140)*u;b=40+(113-40)*u;\n"
      "    }\n"
      "    rects[i].setAttribute('fill',\n"
      "      'rgb('+Math.round(r)+','+Math.round(g)+','+Math.round(b)+')');\n"
      "  }\n"
      "});\n");

    /* click-to-inspect logic */
    fprintf(f,
      "var panel=document.getElementById('cmap-detail');\n"
      "var selRect=null;\n"
      "document.getElementById('cmap-svg').addEventListener('click',function(e){\n"
      "  var r=e.target; if(r.tagName!=='rect') return;\n"
      "  var idx=r.getAttribute('data-i'); if(idx===null) return;\n"
      "  idx=parseInt(idx); var d=BD[idx]; if(!d) return;\n"
      "  if(selRect) selRect.classList.remove('cmap-sel');\n"
      "  r.classList.add('cmap-sel'); selRect=r;\n"
      "  var ch=d.ch?\" '\"+ d.ch +\"'\":'';\n"
      "  var costClr=d.c<3?'#34d399':d.c<6?'#fbbf24':'#f87171';\n"
      "  var h='<div class=\"cd-head\">';\n"
      "  h+='<span class=\"cd-byte\">0x'+d.h+ch+'</span>';\n"
      "  h+='<span class=\"cd-sub\">offset '+d.o+'</span>';\n"
      "  h+='<span class=\"cd-cost\" style=\"color:'+costClr+'\">'+d.c.toFixed(2)+' bits</span>';\n"
      "  h+='</div>';\n"
      "  if(d.m && MI.length){\n"
      "    var mx=0;\n"
      "    for(var i=0;i<d.m.length;i++){var a=Math.abs(d.m[i]);if(a>mx)mx=a;}\n"
      "    if(mx<0.01)mx=1;\n"
      "    for(var i=0;i<d.m.length;i++){\n"
      "      var v=d.m[i]; /* positive = bits saved */\n"
      "      var pct=Math.min(Math.abs(v)/mx*100,100);\n"
      "      var clr=v>0.01?'#34d399':v<-0.01?'#f87171':'#353b4f';\n"
      "      var sign=v>0?'+':'';\n"
      "      h+='<div class=\"cd-bar\">';\n"
      "      h+='<span class=\"cd-bar-lbl\">'+MI[i].mask+':'+MI[i].w+'</span>';\n"
      "      h+='<span class=\"cd-bar-track\"><span class=\"cd-bar-fill\" style=\"width:'+pct.toFixed(0)+'%%;background:'+clr+'\"></span></span>';\n"
      "      h+='<span class=\"cd-bar-val\" style=\"color:'+clr+'\">'+sign+v.toFixed(2)+' bits</span>';\n"
      "      h+='</div>';\n"
      "    }\n"
      "  }\n"
      "  panel.innerHTML=h;\n"
      "  panel.style.display='block';\n"
      "  if(window.hexHighlight) window.hexHighlight(idx);\n"
      "});\n");

    /* cmap hover tooltip */
    fprintf(f, "%s",
      "var ctip=document.getElementById('cmap-tip');\n"
      "var csvg=document.getElementById('cmap-svg');\n"
      "function hideCtip(){ctip.style.display='none';}\n"
      "csvg.addEventListener('mousemove',function(e){\n"
      "  var r=e.target; if(r.tagName!=='rect'){hideCtip();return;}\n"
      "  var idx=r.getAttribute('data-i'); if(idx===null) return;\n"
      "  idx=parseInt(idx); var d=BD[idx]; if(!d) return;\n"
      "  var costClr=d.c<3?'#34d399':d.c<6?'#fbbf24':'#f87171';\n"
      "  var ch=d.ch?\" '\"+d.ch+\"'\":'';\n"
      "  var h='<div class=\"tip-row\"><span style=\"color:var(--fg3)\">byte</span>'\n"
      "    +'<span style=\"color:var(--fg)\">0x'+d.h+ch+'</span></div>'\n"
      "    +'<div class=\"tip-row\"><span style=\"color:var(--fg3)\">offset</span>'\n"
      "    +'<span>'+d.o+'</span></div>'\n"
      "    +'<div class=\"tip-row\"><span style=\"color:var(--fg3)\">cost</span>'\n"
      "    +'<span style=\"color:'+costClr+';font-weight:600\">'+d.c.toFixed(2)+' bits</span></div>';\n"
      "  if(window.makeSparkline){\n"
      "    var lo=Math.max(0,idx-32), hi=Math.min(BD.length-1,idx+32);\n"
      "    var vals=[];\n"
      "    for(var k=lo;k<=hi;k++) vals.push(BD[k]?BD[k].c:0);\n"
      "    var sp=window.makeSparkline(vals,{\n"
      "      color:'#22d3ee', highlightIdx:idx-lo,\n"
      "      labelTop:'cost', labelRight:lo+'\\u2026'+hi});\n"
      "    if(sp) h += '<div style=\"color:var(--fg3);font-size:10px;'\n"
      "      +'margin-top:6px\">context cost</div>'+sp;\n"
      "  }\n"
      "  ctip.innerHTML=h;\n"
      "  var pr=csvg.parentNode.getBoundingClientRect();\n"
      "  var tx=(e.clientX-pr.left)+14, ty=(e.clientY-pr.top)-50;\n"
      "  if(tx+180>pr.width) tx=(e.clientX-pr.left)-180;\n"
      "  if(ty<0) ty=(e.clientY-pr.top)+18;\n"
      "  ctip.style.left=tx+'px'; ctip.style.top=ty+'px';\n"
      "  ctip.style.display='block';\n"
      "});\n"
      "csvg.addEventListener('mouseleave',hideCtip);\n");

    fprintf(f, "</script>\n");
    fprintf(f, "</div>\n\n");
  }

  /* ── Byte Cost Histogram ── */
  if (s->byte_costs && s->num_data_bytes > 0) {
    fprintf(f,
      "<div class=\"card\" id=\"sec-costhist\" style=\"position:relative\">\n"
      "<h2>Byte Cost Distribution</h2>\n"
      "<p class=\"desc\">Histogram of per-byte encoding cost. "
      "Dashed lines: <span style=\"color:#34d399\">mean</span> and "
      "<span style=\"color:#f87171\">H\xe2\x82\x80</span>.</p>\n"
      "<div id=\"ch-chart\"></div>\n"
      "<div id=\"ch-tip\" class=\"hover-tip\"></div>\n");

    /* built from the BD array the Compressibility Map already embedded */
    fprintf(f, "<script>(function(){\n");
    fprintf(f, "var H0=%.4f;\n", s->entropy);
    fprintf(f, "%s",
      "if(typeof BD==='undefined'||!BD.length) return;\n"
      "var maxc=0,sum=0;\n"
      "for(var i=0;i<BD.length;i++){var c=BD[i].c;sum+=c;if(c>maxc)maxc=c;}\n"
      "var mean=sum/BD.length;\n"
      "var binW=0.25;\n"
      "while(maxc/binW>32) binW*=2;\n"
      "var nbins=Math.max(1,Math.ceil(maxc/binW));\n"
      "var bins=new Array(nbins);\n"
      "for(var i=0;i<nbins;i++) bins[i]=0;\n"
      "for(var i=0;i<BD.length;i++){\n"
      "  var b=Math.floor(BD[i].c/binW);\n"
      "  if(b<0)b=0; if(b>=nbins)b=nbins-1;\n"
      "  bins[b]++;\n"
      "}\n"
      "var maxN=1;\n"
      "for(var i=0;i<nbins;i++) if(bins[i]>maxN) maxN=bins[i];\n"
      "var W=460,H=190,PL=38,PR=8,PT=10,PB=26;\n"
      "var pw=W-PL-PR,ph=H-PT-PB;\n"
      "function xOf(b){return PL+b*pw/nbins;}\n"
      "function yOf(n){return PT+ph*(1-n/maxN);}\n"
      "function xVal(v){return PL+(v/(nbins*binW))*pw;}\n"
      "var s='<svg width=\"100%\" viewBox=\"0 0 '+W+' '+H+'\" style=\"display:block\">';\n"
      "s+='<rect x=\"'+PL+'\" y=\"'+PT+'\" width=\"'+pw+'\" height=\"'+ph\n"
      "  +'\" fill=\"var(--bg3)\" rx=\"4\"/>';\n"
      "for(var i=0;i<=3;i++){\n"
      "  var n=Math.round(maxN*(3-i)/3), y=(PT+ph*i/3)|0;\n"
      "  s+='<line x1=\"'+PL+'\" y1=\"'+y+'\" x2=\"'+(PL+pw)+'\" y2=\"'+y\n"
      "    +'\" stroke=\"var(--bdr)\" stroke-width=\"0.5\"/>'\n"
      "    +'<text x=\"'+(PL-5)+'\" y=\"'+(y+3)+'\" text-anchor=\"end\" '\n"
      "    +'font-size=\"9\" fill=\"var(--fg3)\">'+n+'</text>';\n"
      "}\n"
      "for(var b=0;b<nbins;b++){\n"
      "  if(!bins[b]) continue;\n"
      "  var x=xOf(b), bw=pw/nbins, y=yOf(bins[b]);\n"
      "  var mid=(b+0.5)*binW;\n"
      "  var clr=mid<3?'#34d399':mid<6?'#fbbf24':'#f87171';\n"
      "  s+='<rect x=\"'+(x+0.5).toFixed(1)+'\" y=\"'+y.toFixed(1)\n"
      "    +'\" width=\"'+(bw-1).toFixed(1)+'\" height=\"'+(PT+ph-y).toFixed(1)\n"
      "    +'\" fill=\"'+clr+'\" fill-opacity=\".65\" data-b=\"'+b+'\"/>';\n"
      "}\n"
      "/* mean + H0 markers */\n"
      "if(mean<=nbins*binW){\n"
      "  var xm=xVal(mean).toFixed(1);\n"
      "  s+='<line x1=\"'+xm+'\" y1=\"'+PT+'\" x2=\"'+xm+'\" y2=\"'+(PT+ph)\n"
      "    +'\" stroke=\"#34d399\" stroke-width=\"1\" stroke-dasharray=\"4,3\" opacity=\".8\"/>'\n"
      "    +'<text x=\"'+xm+'\" y=\"'+(PT+8)+'\" font-size=\"8\" fill=\"#34d399\" dx=\"3\">'\n"
      "    +mean.toFixed(2)+'</text>';\n"
      "}\n"
      "if(H0<=nbins*binW){\n"
      "  var xh=xVal(H0).toFixed(1);\n"
      "  s+='<line x1=\"'+xh+'\" y1=\"'+PT+'\" x2=\"'+xh+'\" y2=\"'+(PT+ph)\n"
      "    +'\" stroke=\"#f87171\" stroke-width=\"1\" stroke-dasharray=\"4,3\" opacity=\".8\"/>'\n"
      "    +'<text x=\"'+xh+'\" y=\"'+(PT+16)+'\" font-size=\"8\" fill=\"#f87171\" dx=\"3\">'\n"
      "    +'H\\u2080</text>';\n"
      "}\n"
      "var step=Math.max(1,Math.round(nbins/8));\n"
      "for(var b=0;b<=nbins;b+=step){\n"
      "  s+='<text x=\"'+xOf(b).toFixed(1)+'\" y=\"'+(H-6)+'\" text-anchor=\"middle\" '\n"
      "    +'font-size=\"9\" fill=\"var(--fg3)\">'+(b*binW).toFixed(binW<1?1:0)+'</text>';\n"
      "}\n"
      "s+='</svg>';\n"
      "var chart=document.getElementById('ch-chart');\n"
      "chart.innerHTML=s;\n"
      "var tip=document.getElementById('ch-tip');\n"
      "var card=document.getElementById('sec-costhist');\n"
      "chart.addEventListener('mousemove',function(e){\n"
      "  var t=e.target;\n"
      "  if(t.tagName!=='rect'||t.getAttribute('data-b')===null){\n"
      "    tip.style.display='none'; return;\n"
      "  }\n"
      "  var b=parseInt(t.getAttribute('data-b'));\n"
      "  var pct=100*bins[b]/BD.length;\n"
      "  tip.innerHTML='<div class=\"tip-row\"><span style=\"color:var(--fg3)\">cost</span>'\n"
      "    +'<span style=\"color:var(--fg)\">'+(b*binW).toFixed(2)+'\\u2013'\n"
      "    +((b+1)*binW).toFixed(2)+' bits</span></div>'\n"
      "    +'<div class=\"tip-row\"><span style=\"color:var(--fg3)\">bytes</span>'\n"
      "    +'<span style=\"color:var(--acc);font-weight:600\">'+bins[b]+'</span></div>'\n"
      "    +'<div class=\"tip-row\"><span style=\"color:var(--fg3)\">share</span>'\n"
      "    +'<span>'+pct.toFixed(1)+'%</span></div>';\n"
      "  var cr=card.getBoundingClientRect();\n"
      "  var tx=(e.clientX-cr.left)+14, ty=(e.clientY-cr.top)-40;\n"
      "  if(tx+170>cr.width) tx=(e.clientX-cr.left)-170;\n"
      "  if(ty<0) ty=(e.clientY-cr.top)+16;\n"
      "  tip.style.left=tx+'px'; tip.style.top=ty+'px';\n"
      "  tip.style.display='block';\n"
      "});\n"
      "chart.addEventListener('mouseleave',function(){tip.style.display='none';});\n"
      "})();</script>\n");

    fprintf(f, "</div>\n\n");
  }

  /* ── Cost vs Self-Information scatter ── */
  if (s->byte_costs && s->num_data_bytes > 0) {
    fprintf(f, "%s",
      "<div class=\"card\" id=\"sec-infoscatter\" style=\"position:relative\">\n"
      "<h2>Cost vs Self-Information</h2>\n"
      "<p class=\"desc\">One dot per byte value: order-0 self-information vs "
      "average actual cost. Dot size = frequency. "
      "<span style=\"color:#34d399\">Below the diagonal</span> = context "
      "models beat the histogram, "
      "<span style=\"color:#f87171\">above</span> = they do worse.</p>\n"
      "<div id=\"is-chart\"></div>\n"
      "<div id=\"is-tip\" class=\"hover-tip\"></div>\n");

    /* built from BD (per-byte costs) + BF (byte frequencies) */
    fprintf(f, "%s",
      "<script>(function(){\n"
      "if(typeof BD==='undefined'||typeof BF==='undefined'||!BD.length) return;\n"
      "var total=0;\n"
      "for(var v=0;v<256;v++) total+=BF[v];\n"
      "if(!total) return;\n"
      "var sum=new Float64Array(256), cnt=new Uint32Array(256);\n"
      "for(var i=0;i<BD.length;i++){\n"
      "  var v=parseInt(BD[i].h,16);\n"
      "  sum[v]+=BD[i].c; cnt[v]++;\n"
      "}\n"
      "var pts=[], maxv=1;\n"
      "for(var v=0;v<256;v++){\n"
      "  if(!cnt[v]) continue;\n"
      "  var si=-Math.log2(BF[v]/total);\n"
      "  var ac=sum[v]/cnt[v];\n"
      "  if(si>maxv) maxv=si;\n"
      "  if(ac>maxv) maxv=ac;\n"
      "  pts.push({v:v,si:si,ac:ac,n:cnt[v]});\n"
      "}\n"
      "maxv=Math.ceil(maxv);\n"
      "var W=460,H=340,PL=40,PR=10,PT=10,PB=32;\n"
      "var pw=W-PL-PR,ph=H-PT-PB;\n"
      "function X(x){return PL+x/maxv*pw;}\n"
      "function Y(y){return PT+ph-y/maxv*ph;}\n"
      "var s='<svg width=\"100%\" viewBox=\"0 0 '+W+' '+H\n"
      "  +'\" style=\"display:block;max-width:560px\">';\n"
      "s+='<rect x=\"'+PL+'\" y=\"'+PT+'\" width=\"'+pw+'\" height=\"'+ph\n"
      "  +'\" fill=\"var(--bg3)\" rx=\"4\"/>';\n"
      "var tick=maxv>8?2:1;\n"
      "for(var t2=0;t2<=maxv;t2+=tick){\n"
      "  var gx=X(t2), gy=Y(t2);\n"
      "  s+='<line x1=\"'+gx.toFixed(1)+'\" y1=\"'+PT+'\" x2=\"'+gx.toFixed(1)\n"
      "    +'\" y2=\"'+(PT+ph)+'\" stroke=\"var(--bdr)\" stroke-width=\"0.5\"/>'\n"
      "    +'<line x1=\"'+PL+'\" y1=\"'+gy.toFixed(1)+'\" x2=\"'+(PL+pw)\n"
      "    +'\" y2=\"'+gy.toFixed(1)+'\" stroke=\"var(--bdr)\" stroke-width=\"0.5\"/>'\n"
      "    +'<text x=\"'+gx.toFixed(1)+'\" y=\"'+(PT+ph+14)\n"
      "    +'\" text-anchor=\"middle\" font-size=\"9\" fill=\"var(--fg3)\">'+t2+'</text>'\n"
      "    +'<text x=\"'+(PL-5)+'\" y=\"'+(gy+3).toFixed(1)\n"
      "    +'\" text-anchor=\"end\" font-size=\"9\" fill=\"var(--fg3)\">'+t2+'</text>';\n"
      "}\n"
      "s+='<line x1=\"'+X(0).toFixed(1)+'\" y1=\"'+Y(0).toFixed(1)\n"
      "  +'\" x2=\"'+X(maxv).toFixed(1)+'\" y2=\"'+Y(maxv).toFixed(1)\n"
      "  +'\" stroke=\"var(--fg3)\" stroke-width=\"1\" stroke-dasharray=\"5,3\" opacity=\".6\"/>';\n"
      "for(var i=0;i<pts.length;i++){\n"
      "  var p=pts[i];\n"
      "  var r=1.5+Math.log2(p.n+1)*0.7; if(r>7)r=7;\n"
      "  var clr=p.ac<p.si-0.15?'#34d399':p.ac>p.si+0.15?'#f87171':'#60a5fa';\n"
      "  s+='<circle cx=\"'+X(p.si).toFixed(1)+'\" cy=\"'+Y(p.ac).toFixed(1)\n"
      "    +'\" r=\"'+r.toFixed(1)+'\" fill=\"'+clr+'\" fill-opacity=\".7\" '\n"
      "    +'data-i=\"'+i+'\" style=\"cursor:pointer\"/>';\n"
      "}\n"
      "s+='<text x=\"'+(PL+pw/2)+'\" y=\"'+(H-2)+'\" text-anchor=\"middle\" '\n"
      "  +'font-size=\"10\" fill=\"var(--fg3)\">self-information H\\u2080 (bits)</text>';\n"
      "s+='<text x=\"12\" y=\"'+(PT+ph/2)+'\" text-anchor=\"middle\" '\n"
      "  +'font-size=\"10\" fill=\"var(--fg3)\" transform=\"rotate(-90 12 '\n"
      "  +(PT+ph/2)+')\">avg cost (bits)</text>';\n"
      "s+='</svg>';\n"
      "var chart=document.getElementById('is-chart');\n"
      "chart.innerHTML=s;\n"
      "var tip=document.getElementById('is-tip');\n"
      "var card=document.getElementById('sec-infoscatter');\n"
      "function esc(c){return c==='&'?'&amp;':c==='<'?'&lt;':c==='>'?'&gt;':c;}\n"
      "chart.addEventListener('mousemove',function(e){\n"
      "  var t=e.target;\n"
      "  if(t.tagName!=='circle'){tip.style.display='none';return;}\n"
      "  var p=pts[+t.getAttribute('data-i')]; if(!p) return;\n"
      "  var hex='0x'+(p.v<16?'0':'')+p.v.toString(16).toUpperCase();\n"
      "  var ch=(p.v>=0x20&&p.v<=0x7E)?\" '\"+esc(String.fromCharCode(p.v))+\"'\":'';\n"
      "  var d=p.ac-p.si;\n"
      "  var dClr=d<-0.15?'#34d399':d>0.15?'#f87171':'var(--fg)';\n"
      "  tip.innerHTML='<div class=\"tip-row\"><span style=\"color:var(--fg)\">'\n"
      "    +hex+ch+'</span></div>'\n"
      "    +'<div style=\"border-top:1px solid var(--bdr);margin:4px 0 2px;'\n"
      "    +'padding-top:4px\"></div>'\n"
      "    +'<div class=\"tip-row\"><span style=\"color:var(--fg3)\">count</span>'\n"
      "    +'<span>'+p.n+'</span></div>'\n"
      "    +'<div class=\"tip-row\"><span style=\"color:var(--fg3)\">H\\u2080</span>'\n"
      "    +'<span>'+p.si.toFixed(2)+' bits</span></div>'\n"
      "    +'<div class=\"tip-row\"><span style=\"color:var(--fg3)\">actual</span>'\n"
      "    +'<span>'+p.ac.toFixed(2)+' bits</span></div>'\n"
      "    +'<div class=\"tip-row\"><span style=\"color:var(--fg3)\">\\u0394</span>'\n"
      "    +'<span style=\"color:'+dClr+';font-weight:600\">'+(d>=0?'+':'')\n"
      "    +d.toFixed(2)+'</span></div>';\n"
      "  var cr=card.getBoundingClientRect();\n"
      "  var tx=(e.clientX-cr.left)+14, ty=(e.clientY-cr.top)-60;\n"
      "  if(tx+170>cr.width) tx=(e.clientX-cr.left)-170;\n"
      "  if(ty<0) ty=(e.clientY-cr.top)+18;\n"
      "  tip.style.left=tx+'px'; tip.style.top=ty+'px';\n"
      "  tip.style.display='block';\n"
      "});\n"
      "chart.addEventListener('mouseleave',function(){tip.style.display='none';});\n"
      "})();</script>\n");

    fprintf(f, "</div>\n\n");
  }

  /* ── Learning Curve ── */
  if (s->byte_costs && s->num_data_bytes > 0) {
    fprintf(f, "%s",
      "<div class=\"card\" id=\"sec-learn\" style=\"position:relative\">\n"
      "<h2>Learning Curve</h2>\n"
      "<p class=\"desc\">Average cost by occurrence number of the byte "
      "value (log-binned): how fast the counters adapt. Healthy = steep "
      "drop, then flat. Dashed line = H\xe2\x82\x80.</p>\n"
      "<div id=\"lc-chart\"></div>\n"
      "<div id=\"lc-tip\" class=\"hover-tip\"></div>\n");

    fprintf(f, "<script>(function(){\n");
    fprintf(f, "var H0=%.4f;\n", s->entropy);
    fprintf(f, "%s",
      "if(typeof BD==='undefined'||!BD.length) return;\n"
      "var seen=new Uint32Array(256);\n"
      "var NB=15;\n"
      "var sum=new Float64Array(NB), cnt=new Uint32Array(NB);\n"
      "for(var i=0;i<BD.length;i++){\n"
      "  var v=parseInt(BD[i].h,16);\n"
      "  seen[v]++;\n"
      "  var b=Math.floor(Math.log2(seen[v]));\n"
      "  if(b>=NB) b=NB-1;\n"
      "  sum[b]+=BD[i].c; cnt[b]++;\n"
      "}\n"
      "var bins=[];\n"
      "for(var b=0;b<NB;b++)\n"
      "  if(cnt[b]) bins.push({b:b,avg:sum[b]/cnt[b],n:cnt[b]});\n"
      "if(bins.length<2) return;\n"
      "function lbl(b){\n"
      "  if(b===0) return '1';\n"
      "  if(b===NB-1) return '\\u2265'+Math.pow(2,b);\n"
      "  return Math.pow(2,b)+'\\u2013'+(Math.pow(2,b+1)-1);\n"
      "}\n"
      "var maxY=H0;\n"
      "for(var j=0;j<bins.length;j++) if(bins[j].avg>maxY) maxY=bins[j].avg;\n"
      "maxY*=1.1; if(maxY<=0) maxY=1;\n"
      "var W=460,H=210,PL=36,PR=10,PT=12,PB=42;\n"
      "var pw=W-PL-PR,ph=H-PT-PB;\n"
      "function Y(v){return PT+ph-v/maxY*ph;}\n"
      "var bw=pw/bins.length;\n"
      "var s='<svg width=\"100%\" viewBox=\"0 0 '+W+' '+H\n"
      "  +'\" style=\"display:block;max-width:560px\">';\n"
      "s+='<rect x=\"'+PL+'\" y=\"'+PT+'\" width=\"'+pw+'\" height=\"'+ph\n"
      "  +'\" fill=\"var(--bg3)\" rx=\"4\"/>';\n"
      "for(var i=0;i<=3;i++){\n"
      "  var v=maxY*(3-i)/3, y=(PT+ph*i/3)|0;\n"
      "  s+='<line x1=\"'+PL+'\" y1=\"'+y+'\" x2=\"'+(PL+pw)+'\" y2=\"'+y\n"
      "    +'\" stroke=\"var(--bdr)\" stroke-width=\"0.5\"/>'\n"
      "    +'<text x=\"'+(PL-4)+'\" y=\"'+(y+3)+'\" text-anchor=\"end\" '\n"
      "    +'font-size=\"9\" fill=\"var(--fg3)\">'+v.toFixed(1)+'</text>';\n"
      "}\n"
      "if(H0<=maxY){\n"
      "  var hy=Y(H0).toFixed(1);\n"
      "  s+='<line x1=\"'+PL+'\" y1=\"'+hy+'\" x2=\"'+(PL+pw)+'\" y2=\"'+hy\n"
      "    +'\" stroke=\"#f87171\" stroke-width=\"1\" stroke-dasharray=\"5,3\" '\n"
      "    +'opacity=\".6\"/>';\n"
      "}\n"
      "for(var j=0;j<bins.length;j++){\n"
      "  var bn=bins[j];\n"
      "  var x=PL+j*bw, y=Y(bn.avg);\n"
      "  var clr=bn.avg<3?'#34d399':bn.avg<6?'#fbbf24':'#f87171';\n"
      "  s+='<rect x=\"'+(x+2).toFixed(1)+'\" y=\"'+y.toFixed(1)\n"
      "    +'\" width=\"'+(bw-4).toFixed(1)+'\" height=\"'+(PT+ph-y).toFixed(1)\n"
      "    +'\" rx=\"2\" fill=\"'+clr+'\" fill-opacity=\".65\" data-j=\"'+j+'\"/>';\n"
      "  var lx=x+bw/2;\n"
      "  s+='<text x=\"'+lx.toFixed(1)+'\" y=\"'+(PT+ph+12)\n"
      "    +'\" text-anchor=\"end\" font-size=\"8\" fill=\"var(--fg3)\" '\n"
      "    +'transform=\"rotate(-40 '+lx.toFixed(1)+' '+(PT+ph+12)+')\">'\n"
      "    +lbl(bn.b)+'</text>';\n"
      "}\n"
      "s+='<text x=\"'+(PL+pw/2)+'\" y=\"'+(H-2)+'\" text-anchor=\"middle\" '\n"
      "  +'font-size=\"10\" fill=\"var(--fg3)\">occurrence of byte value</text>';\n"
      "s+='</svg>';\n"
      "var chart=document.getElementById('lc-chart');\n"
      "chart.innerHTML=s;\n"
      "var tip=document.getElementById('lc-tip');\n"
      "var card=document.getElementById('sec-learn');\n"
      "chart.addEventListener('mousemove',function(e){\n"
      "  var t=e.target;\n"
      "  if(t.tagName!=='rect'||t.getAttribute('data-j')===null){\n"
      "    tip.style.display='none'; return;\n"
      "  }\n"
      "  var bn=bins[+t.getAttribute('data-j')]; if(!bn) return;\n"
      "  var clr=bn.avg<3?'#34d399':bn.avg<6?'#fbbf24':'#f87171';\n"
      "  tip.innerHTML='<div class=\"tip-row\"><span style=\"color:var(--fg3)\">'\n"
      "    +'occurrence</span><span style=\"color:var(--fg)\">'+lbl(bn.b)+'</span></div>'\n"
      "    +'<div class=\"tip-row\"><span style=\"color:var(--fg3)\">bytes</span>'\n"
      "    +'<span>'+bn.n+'</span></div>'\n"
      "    +'<div class=\"tip-row\"><span style=\"color:var(--fg3)\">avg cost</span>'\n"
      "    +'<span style=\"color:'+clr+';font-weight:600\">'+bn.avg.toFixed(2)\n"
      "    +' bits</span></div>'\n"
      "    +'<div class=\"tip-row\"><span style=\"color:var(--fg3)\">vs H\\u2080</span>'\n"
      "    +'<span>'+(bn.avg-H0>=0?'+':'')+(bn.avg-H0).toFixed(2)+'</span></div>';\n"
      "  var cr=card.getBoundingClientRect();\n"
      "  var tx=(e.clientX-cr.left)+14, ty=(e.clientY-cr.top)-50;\n"
      "  if(tx+170>cr.width) tx=(e.clientX-cr.left)-170;\n"
      "  if(ty<0) ty=(e.clientY-cr.top)+18;\n"
      "  tip.style.left=tx+'px'; tip.style.top=ty+'px';\n"
      "  tip.style.display='block';\n"
      "});\n"
      "chart.addEventListener('mouseleave',function(){tip.style.display='none';});\n"
      "})();</script>\n");

    fprintf(f, "</div>\n\n");
  }

  /* ── Strings ── */
  if (s->byte_costs && s->num_data_bytes > 0) {
    fprintf(f, "%s",
      "<div class=\"card\" id=\"sec-strings\">\n"
      "<h2>Strings</h2>\n"
      "<p class=\"desc\">ASCII runs (&ge;4 chars) ranked by total encoding "
      "cost. <span style=\"color:var(--fg3)\">Click a row to locate it in "
      "the Hex View.</span></p>\n"
      "<table><thead><tr><th class=\"r\">Offset</th><th>String</th>"
      "<th class=\"r\">Len</th><th class=\"r\">Cost</th>"
      "<th class=\"r\">Bits/ch</th></tr></thead>"
      "<tbody id=\"st-body\"></tbody></table>\n"
      "<div id=\"st-note\" style=\"font-size:11px;color:var(--fg3);"
      "margin-top:6px\"></div>\n");

    fprintf(f, "%s",
      "<script>(function(){\n"
      "if(typeof BD==='undefined'||!BD.length) return;\n"
      "function esc(x){return x.replace(/&/g,'&amp;').replace(/</g,'&lt;')"
      ".replace(/>/g,'&gt;');}\n"
      "var runs=[];\n"
      "var start=-1,str='',cost=0;\n"
      "function flush(){\n"
      "  if(start>=0&&str.length>=4)\n"
      "    runs.push({o:start,s:str,c:cost,l:str.length});\n"
      "  start=-1;str='';cost=0;\n"
      "}\n"
      "for(var i=0;i<BD.length;i++){\n"
      "  var d=BD[i];\n"
      "  if(d.ch){\n"
      "    if(start<0) start=i;\n"
      "    str+=d.ch; cost+=d.c;\n"
      "  } else flush();\n"
      "}\n"
      "flush();\n"
      "if(!runs.length){\n"
      "  document.getElementById('sec-strings').style.display='none';\n"
      "  return;\n"
      "}\n"
      "runs.sort(function(a,b){return b.c-a.c;});\n"
      "var show=Math.min(runs.length,20);\n"
      "var h='';\n"
      "for(var i=0;i<show;i++){\n"
      "  var r=runs[i];\n"
      "  var disp=r.s.length>44?r.s.slice(0,42)+'\\u2026':r.s;\n"
      "  var bpc=r.c/r.l;\n"
      "  var clr=bpc<3?'#34d399':bpc<6?'#fbbf24':'#f87171';\n"
      "  h+='<tr class=\"st-row\" data-o=\"'+r.o+'\" style=\"cursor:pointer\">'\n"
      "    +'<td class=\"r\">'+r.o+'</td>'\n"
      "    +'<td style=\"font-family:var(--mono);font-size:11px;'\n"
      "    +'color:var(--fg)\">&quot;'+esc(disp)+'&quot;</td>'\n"
      "    +'<td class=\"r\">'+r.l+'</td>'\n"
      "    +'<td class=\"r\">'+(r.c/8).toFixed(1)+' B</td>'\n"
      "    +'<td class=\"r\" style=\"color:'+clr+';font-weight:600\">'\n"
      "    +bpc.toFixed(2)+'</td>'\n"
      "    +'</tr>';\n"
      "}\n"
      "document.getElementById('st-body').innerHTML=h;\n"
      "if(runs.length>show)\n"
      "  document.getElementById('st-note').textContent=\n"
      "    '+'+(runs.length-show)+' more strings not shown';\n"
      "document.getElementById('st-body').addEventListener('click',function(e){\n"
      "  var tr=e.target.closest('tr[data-o]'); if(!tr) return;\n"
      "  var o=parseInt(tr.getAttribute('data-o'));\n"
      "  if(window.hexHighlight){\n"
      "    window.hexHighlight(o);\n"
      "    var hx=document.getElementById('sec-hex');\n"
      "    if(hx) hx.scrollIntoView({behavior:'smooth',block:'start'});\n"
      "  }\n"
      "});\n"
      "})();</script>\n");

    fprintf(f, "</div>\n\n");
  }

  /* ── Hex View ── */
  if (s->byte_costs && s->num_data_bytes > 0 && s->input_data) {
    fprintf(f, "%s",
      "<div class=\"card full\" id=\"sec-hex\" style=\"position:relative\">\n"
      "<h2>Hex View</h2>\n"
      "<p class=\"desc\">Hex dump colored by per-byte encoding cost or by "
      "dominant model. "
      "<span style=\"color:var(--fg3)\">Click a byte to inspect it in the "
      "Attribution Map; selections elsewhere highlight here too.</span></p>\n"
      "<div class=\"slider-row\">"
      "<span>Color:</span>"
      "<button id=\"hex-mode-cost\" type=\"button\">cost</button>"
      "<button id=\"hex-mode-model\" type=\"button\">model</button>"
      "<span style=\"margin-left:14px\">Offset:</span>"
      "<input id=\"hex-jump\" type=\"text\" placeholder=\"0x0000\" "
      "style=\"width:80px;font-family:var(--mono);font-size:11px;"
      "background:var(--bg3);color:var(--fg);border:1px solid var(--bdr2);"
      "border-radius:4px;padding:3px 6px\">"
      "<button id=\"hex-go\" type=\"button\">Go</button>"
      "</div>\n"
      "<div id=\"hex-dump\" style=\"max-height:420px;overflow-y:auto;"
      "font-family:var(--mono);font-size:11px;line-height:1.6\"></div>\n"
      "<div id=\"hex-tip\" class=\"hover-tip\"></div>\n");

    /* built entirely from the BD array embedded by the Compressibility Map */
    fprintf(f, "%s",
      "<script>(function(){\n"
      "if(typeof BD==='undefined'||!BD.length) return;\n"
      "var N=Math.min(BD.length,65536);\n"
      "var dump=document.getElementById('hex-dump');\n"
      "var tip=document.getElementById('hex-tip');\n"
      "var card=document.getElementById('sec-hex');\n"
      "var mode='cost';\n"
      "var cmax=0;\n"
      "for(var i=0;i<N;i++) if(BD[i].c>cmax) cmax=BD[i].c;\n"
      "if(cmax<=0) cmax=1;\n"
      "var hasM=!!BD[0].m;\n"
      "var gmax=0.01;\n"
      "if(hasM){\n"
      "  for(var i=0;i<N;i++){var mm=BD[i].m;\n"
      "    for(var k=0;k<mm.length;k++) if(mm[k]>gmax) gmax=mm[k];}\n"
      "}\n"
      "function esc(c){return c==='&'?'&amp;':c==='<'?'&lt;':c==='>'?'&gt;':c;}\n"
      "function hex6(v){var h=v.toString(16).toUpperCase();"
      "while(h.length<6)h='0'+h;return h;}\n"
      "function costBg(c){\n"
      "  var t=c/cmax; if(t<0)t=0; if(t>1)t=1;\n"
      "  var r,g,b;\n"
      "  if(t<0.5){var u=t*2;r=16+(180-16)*u;g=185+(140-185)*u;b=129+(40-129)*u;}\n"
      "  else{var u=(t-0.5)*2;r=180+(248-180)*u;g=140+(113-140)*u;b=40+(113-40)*u;}\n"
      "  return {bg:'rgb('+Math.round(r)+','+Math.round(g)+','+Math.round(b)+')',\n"
      "          fg:t<0.6?'#0c0e14':'rgba(255,255,255,.85)'};\n"
      "}\n"
      "function bestModel(i){\n"
      "  if(!hasM) return -1;\n"
      "  var mm=BD[i].m,bi=-1,bv=0.001;\n"
      "  for(var k=0;k<mm.length;k++) if(mm[k]>bv){bv=mm[k];bi=k;}\n"
      "  return bi;\n"
      "}\n"
      "function modelBg(i){\n"
      "  var bi=bestModel(i);\n"
      "  if(bi<0||typeof ATTR_PAL==='undefined'||!ATTR_PAL[bi])\n"
      "    return {bg:'rgba(128,128,128,.10)',fg:'var(--fg2)'};\n"
      "  var pc=ATTR_PAL[bi];\n"
      "  var v=BD[i].m[bi], t=v/gmax; if(t>1)t=1;\n"
      "  t=1-(1-t)*(1-t);\n"
      "  var op=.15+.85*t;\n"
      "  var lum=(pc[0]*299+pc[1]*587+pc[2]*114)/1000*op;\n"
      "  return {bg:'rgba('+pc[0]+','+pc[1]+','+pc[2]+','+op.toFixed(2)+')',\n"
      "          fg:lum>120?'#0c0e14':'var(--fg)'};\n"
      "}\n"
      "var selIdx=-1;\n"
      "function render(){\n"
      "  var st=dump.scrollTop;\n"
      "  var out=[];\n"
      "  for(var r=0;r<N;r+=16){\n"
      "    var hx='',asc='';\n"
      "    for(var i=r;i<r+16;i++){\n"
      "      var pad=(i===r+8)?'margin-left:8px;':'';\n"
      "      if(i>=N){\n"
      "        hx+='<span style=\"width:20px;'+pad+'\"></span>';\n"
      "        asc+='<span style=\"width:9px\"></span>';\n"
      "        continue;\n"
      "      }\n"
      "      var c=mode==='cost'?costBg(BD[i].c):modelBg(i);\n"
      "      hx+='<span class=\"hx\" data-i=\"'+i+'\" style=\"width:20px;'\n"
      "        +'text-align:center;border-radius:2px;cursor:pointer;'+pad\n"
      "        +'background:'+c.bg+';color:'+c.fg+'\">'+BD[i].h+'</span>';\n"
      "      var ch=BD[i].ch?esc(BD[i].ch):'\\u00b7';\n"
      "      asc+='<span data-i=\"'+i+'\" style=\"width:9px;text-align:center;'\n"
      "        +'cursor:pointer;color:'+(BD[i].ch?'var(--fg2)':'var(--fg3)')\n"
      "        +'\">'+ch+'</span>';\n"
      "    }\n"
      "    out.push('<div style=\"display:flex;gap:14px;align-items:center\">'\n"
      "      +'<span style=\"color:var(--fg3);width:52px;flex-shrink:0;'\n"
      "      +'text-align:right\">'+hex6(r)+'</span>'\n"
      "      +'<span style=\"display:flex;gap:2px\">'+hx+'</span>'\n"
      "      +'<span style=\"display:flex\">'+asc+'</span></div>');\n"
      "  }\n"
      "  if(BD.length>N)\n"
      "    out.push('<div style=\"color:var(--fg3);padding:6px 0\">showing '\n"
      "      +'first '+N+' of '+BD.length+' bytes</div>');\n"
      "  dump.innerHTML=out.join('');\n"
      "  dump.scrollTop=st;\n"
      "  styleBtns();\n"
      "  if(selIdx>=0) markSel(selIdx,false);\n"
      "}\n"
      "var btnC=document.getElementById('hex-mode-cost');\n"
      "var btnM=document.getElementById('hex-mode-model');\n"
      "function styleBtns(){\n"
      "  btnC.style.color=mode==='cost'?'var(--acc)':'';\n"
      "  btnM.style.color=mode==='model'?'var(--acc)':'';\n"
      "}\n"
      "btnC.addEventListener('click',function(){mode='cost';render();});\n"
      "btnM.addEventListener('click',function(){mode='model';render();});\n"
      "if(!hasM) btnM.style.display='none';\n"
      "function markSel(idx,scroll){\n"
      "  var prev=dump.querySelectorAll('.hex-sel');\n"
      "  for(var i=0;i<prev.length;i++) prev[i].classList.remove('hex-sel');\n"
      "  selIdx=idx;\n"
      "  var els=dump.querySelectorAll('[data-i=\"'+idx+'\"]');\n"
      "  for(var i=0;i<els.length;i++) els[i].classList.add('hex-sel');\n"
      "  if(scroll&&els.length){\n"
      "    var row=els[0].parentNode.parentNode;\n"
      "    dump.scrollTop=row.offsetTop-dump.offsetTop-dump.clientHeight/2;\n"
      "  }\n"
      "}\n"
      "var suppressScroll=false;\n"
      "window.hexHighlight=function(idx,scroll){\n"
      "  idx=parseInt(idx);\n"
      "  if(isNaN(idx)||idx<0||idx>=N) return;\n"
      "  var sc=(scroll!==false)&&!suppressScroll;\n"
      "  suppressScroll=false;\n"
      "  markSel(idx,sc);\n"
      "};\n"
      "dump.addEventListener('click',function(e){\n"
      "  var el=e.target.closest('[data-i]'); if(!el) return;\n"
      "  var idx=parseInt(el.getAttribute('data-i'));\n"
      "  suppressScroll=true;\n"
      "  markSel(idx,false);\n"
      "  if(window.attrSelectByte) window.attrSelectByte(idx);\n"
      "  suppressScroll=false;\n"
      "});\n"
      "dump.addEventListener('mousemove',function(e){\n"
      "  var el=e.target.closest('[data-i]');\n"
      "  if(!el){tip.style.display='none';return;}\n"
      "  var idx=parseInt(el.getAttribute('data-i'));\n"
      "  var d=BD[idx]; if(!d){tip.style.display='none';return;}\n"
      "  var costClr=d.c<3?'#34d399':d.c<6?'#fbbf24':'#f87171';\n"
      "  var ch=d.ch?\" '\"+esc(d.ch)+\"'\":'';\n"
      "  var h='<div class=\"tip-row\"><span style=\"color:var(--fg3)\">byte</span>'\n"
      "    +'<span style=\"color:var(--fg)\">0x'+d.h+ch+'</span></div>'\n"
      "    +'<div class=\"tip-row\"><span style=\"color:var(--fg3)\">offset</span>'\n"
      "    +'<span>'+d.o+' (0x'+d.o.toString(16).toUpperCase()+')</span></div>'\n"
      "    +'<div class=\"tip-row\"><span style=\"color:var(--fg3)\">cost</span>'\n"
      "    +'<span style=\"color:'+costClr+';font-weight:600\">'+d.c.toFixed(2)\n"
      "    +' bits</span></div>';\n"
      "  var bi=bestModel(idx);\n"
      "  if(bi>=0&&typeof MI!=='undefined'&&MI[bi]){\n"
      "    var pc=(typeof ATTR_PAL!=='undefined'&&ATTR_PAL[bi])||[100,100,100];\n"
      "    h+='<div class=\"tip-row\"><span style=\"color:var(--fg3)\">best</span>'\n"
      "      +'<span><span class=\"tip-sw\" style=\"background:rgb('+pc[0]+','\n"
      "      +pc[1]+','+pc[2]+')\"></span>'+MI[bi].mask+':'+MI[bi].w\n"
      "      +' (+'+d.m[bi].toFixed(2)+')</span></div>';\n"
      "  }\n"
      "  tip.innerHTML=h;\n"
      "  var cr=card.getBoundingClientRect();\n"
      "  var tx=(e.clientX-cr.left)+14, ty=(e.clientY-cr.top)-50;\n"
      "  if(tx+200>cr.width) tx=(e.clientX-cr.left)-200;\n"
      "  if(ty<0) ty=(e.clientY-cr.top)+18;\n"
      "  tip.style.left=tx+'px'; tip.style.top=ty+'px';\n"
      "  tip.style.display='block';\n"
      "});\n"
      "dump.addEventListener('mouseleave',function(){tip.style.display='none';});\n"
      "function jump(){\n"
      "  var v=document.getElementById('hex-jump').value.trim();\n"
      "  if(!v) return;\n"
      "  var idx=v.slice(0,2).toLowerCase()==='0x'?parseInt(v,16):parseInt(v,10);\n"
      "  if(isNaN(idx)) return;\n"
      "  if(window.partyToast&&document.body.classList.contains('party')){\n"
      "    if(idx===1337) window.partyToast('ELITE.');\n"
      "    else if(idx===404) window.partyToast('OFFSET NOT FOUND');\n"
      "    else if(idx===42) window.partyToast('THE ANSWER');\n"
      "  }\n"
      "  if(idx<0) idx=0; if(idx>=N) idx=N-1;\n"
      "  markSel(idx,true);\n"
      "  if(window.attrSelectByte) window.attrSelectByte(idx);\n"
      "}\n"
      "document.getElementById('hex-go').addEventListener('click',jump);\n"
      "document.getElementById('hex-jump').addEventListener('keydown',\n"
      "  function(e){if(e.key==='Enter') jump();});\n"
      "render();\n"
      "})();</script>\n");

    fprintf(f, "</div>\n\n");
  }

  /* ── Model Attribution Map ── */
  if (s->byte_model_contrib && s->byte_costs && s->num_data_bytes > 0
      && s->num_models > 0) {
    int nb = s->num_data_bytes;
    int nm = s->num_models;
    /* reuse same grid sizing as compressibility map */
    int cols, cell;
    if (nb <= 64) { cols = nb < 16 ? nb : 16; cell = 16; }
    else if (nb <= 512)  { cols = 32; cell = 14; }
    else if (nb <= 2048) { cols = 64; cell = 10; }
    else if (nb <= 4096) { cols = 64; cell = 8; }
    else if (nb <= 8192) { cols = 96; cell = 6; }
    else { cols = 128; cell = 6; }
    int rows = (nb + cols - 1) / cols;
    int gap = 1;
    int stride = cell + gap;
    int cg = cell >= 6 ? 1 : 0;
    int svg_w = cols * stride - cg + 2;
    int svg_h = rows * stride - cg + 2;

    /* distinct color palette for up to MAX_SEARCH models */
    static const int palette[][3] = {
      {34,211,238},   /* cyan     */
      {251,146,60},   /* orange   */
      {167,139,250},  /* purple   */
      {52,211,153},   /* emerald  */
      {251,191,36},   /* amber    */
      {248,113,113},  /* red      */
      {96,165,250},   /* blue     */
      {232,121,249},  /* fuchsia  */
      {163,230,53},   /* lime     */
      {244,114,182},  /* pink     */
      {45,212,191},   /* teal     */
      {253,186,116},  /* peach    */
      {134,239,172},  /* mint     */
      {196,181,253},  /* lavender */
      {252,211,77},   /* gold     */
      {125,211,252},  /* sky      */
      {249,168,212},  /* rose     */
      {190,242,100},  /* chartreuse */
      {253,164,175},  /* salmon   */
      {110,231,183},  /* seafoam  */
      {217,70,239},   /* magenta  */
    };
    int npal = (int)(sizeof(palette) / sizeof(palette[0]));

    fprintf(f,
      "<div class=\"card full\" id=\"sec-attr\">\n"
      "<h2>Model Attribution Map</h2>\n"
      "<p class=\"desc\">Each cell = one byte. Color = model that saved the most bits. "
      "Brightness = strength of contribution. Gray = no model helped. "
      "%d bytes, %d&times;%d grid. "
      "<span style=\"color:var(--fg3)\">Click a cell or legend item to highlight "
      "where that model dominates; click the same again to flip to "
      "where it hurts most.</span></p>\n",
      nb, cols, rows);

    /* legend - clickable for highlight */
    fprintf(f, "<div id=\"attr-legend\" style=\"display:flex;flex-wrap:wrap;"
      "gap:6px 14px;margin-bottom:12px;font-size:11px;"
      "font-family:var(--mono)\">\n");
    for (int m = 0; m < nm && m < npal; m++) {
      fprintf(f, "<span class=\"attr-lg\" data-bm=\"%d\" "
        "style=\"display:inline-flex;align-items:center;gap:4px;"
        "cursor:pointer;padding:2px 6px;border-radius:3px\">"
        "<span style=\"display:inline-block;width:12px;height:12px;"
        "border-radius:2px;background:rgb(%d,%d,%d)\"></span>"
        "%02X:%d<span class=\"attr-lg-arr\"></span></span>\n",
        m,
        palette[m % npal][0], palette[m % npal][1], palette[m % npal][2],
        s->model_masks[m], s->model_weights[m]);
    }
    fprintf(f, "<span class=\"attr-lg\" data-bm=\"-1\" "
      "style=\"display:inline-flex;align-items:center;gap:4px;"
      "cursor:pointer;padding:2px 6px;border-radius:3px\">"
      "<span style=\"display:inline-block;width:12px;height:12px;"
      "border-radius:2px;background:rgb(40,44,60)\"></span>"
      "none<span class=\"attr-lg-arr\"></span></span>\n");
    fprintf(f, "<span id=\"attr-status\" style=\"align-self:center;"
      "font-style:italic;color:var(--fg3)\"></span>\n");
    fprintf(f, "</div>\n");

    fprintf(f, "<div class=\"scrub-wrap\">\n");
    fprintf(f, "<div id=\"attr-tip\" class=\"hover-tip\"></div>\n");
    fprintf(f,
      "<svg id=\"attr-svg\" width=\"100%%\" viewBox=\"0 0 %d %d\" "
      "style=\"display:block\">\n", svg_w, svg_h);

    /* find global max contribution for normalization */
    float global_max = 0.01f;
    for (int i = 0; i < nb; i++) {
      for (int m = 0; m < nm; m++) {
        float v = s->byte_model_contrib[i * nm + m];
        if (v > global_max) global_max = v;
      }
    }

    for (int i = 0; i < nb; i++) {
      int col = i % cols, row2 = i / cols;
      int x = 1 + col * stride, y = 1 + row2 * stride;

      /* find dominant model (most bits saved) and worst model (most bits lost) */
      int best_m = -1, worst_m = -1;
      float best_v = 0.0f, worst_v = 0.0f;
      for (int m = 0; m < nm; m++) {
        float v = s->byte_model_contrib[i * nm + m];
        if (v > best_v) { best_v = v; best_m = m; }
        if (v < worst_v) { worst_v = v; worst_m = m; }
      }

      int cr, cg, cb;
      float opa;
      if (best_m >= 0 && best_v > 0.001f) {
        /* use full palette color + opacity so the card background
           (light or dark) shows through and the theme stays coherent */
        float t = best_v / global_max;
        if (t > 1.0f) t = 1.0f;
        /* concave curve so weak contributions stay visible */
        t = 1.0f - (1.0f - t) * (1.0f - t);
        int pi = best_m % npal;
        cr = palette[pi][0];
        cg = palette[pi][1];
        cb = palette[pi][2];
        opa = 0.15f + 0.85f * t;
      } else {
        /* no model helped — faint neutral */
        cr = 128; cg = 128; cb = 128;
        opa = 0.10f;
      }

      unsigned char bval = s->input_data ? s->input_data[i] : 0;
      fprintf(f,
        "<rect x=\"%d\" y=\"%d\" width=\"%d\" height=\"%d\" "
        "fill=\"rgb(%d,%d,%d)\" fill-opacity=\"%.3f\" "
        "data-i=\"%d\" data-bm=\"%d\" data-wm=\"%d\" "
        "style=\"cursor:pointer;transition:opacity .15s\"/>\n",
        x, y, cell, cell, cr, cg, cb, opa, i, best_m, worst_m);
      if (cell >= 6 && bval >= 0x20 && bval <= 0x7E && bval != '<'
          && bval != '>' && bval != '&' && bval != '"') {
        int lum = (cr * 299 + cg * 587 + cb * 114) / 1000;
        fprintf(f,
          "<text x=\"%d\" y=\"%d\" text-anchor=\"middle\" "
          "font-size=\"%d\" fill=\"%s\" pointer-events=\"none\" "
          "opacity=\".8\">%c</text>\n",
          x + cell / 2, y + cell / 2 + (cell >= 10 ? 3 : cell >= 8 ? 2 : 2),
          cell >= 10 ? 7 : cell >= 8 ? 6 : 5,
          lum > 120 ? "#0c0e14" : "rgba(255,255,255,.7)",
          bval);
      }
    }
    fprintf(f, "</svg>\n");
    fprintf(f, "</div>\n"); /* close scrub-wrap */

    /* ── Highlight style + Detail panel ── */
    fprintf(f, "<style id=\"attr-hilite\"></style>\n");
    fprintf(f, "<div id=\"attr-detail\" class=\"cd-panel\"></div>\n");

    /* ── Emit palette as JS + click handler ── */
    fprintf(f, "<script>\n");
    fprintf(f, "var ATTR_PAL=[");
    for (int m = 0; m < nm && m < npal; m++)
      fprintf(f, "%s[%d,%d,%d]", m ? "," : "",
              palette[m % npal][0], palette[m % npal][1], palette[m % npal][2]);
    fprintf(f, "];\n");

    fprintf(f,
      "(function(){\n"
      "var panel=document.getElementById('attr-detail');\n"
      "var hilite=document.getElementById('attr-hilite');\n"
      "var legend=document.getElementById('attr-legend');\n"
      "var status=document.getElementById('attr-status');\n"
      "var selRect=null;\n"
      "var state=null;\n"
      "function setHilite(m){\n"
      "  m=String(m);\n"
      "  /* cycle: null -> best -> worst -> null when clicking same model */\n"
      "  if(state && state.m===m)\n"
      "    state = state.mode==='best' ? {m:m,mode:'worst'} : null;\n"
      "  else\n"
      "    state = {m:m,mode:'best'};\n"
      "  if(state===null){ hilite.textContent=''; }\n"
      "  else {\n"
      "    var attr=state.mode==='best'?'data-bm':'data-wm';\n"
      "    hilite.textContent='#attr-svg rect['+attr+']:not(['+attr\n"
      "      +'=\"'+state.m+'\"]){opacity:.15}';\n"
      "  }\n"
      "  var items=legend.querySelectorAll('.attr-lg');\n"
      "  for(var i=0;i<items.length;i++){\n"
      "    var v=items[i].getAttribute('data-bm');\n"
      "    var on=(state && v===state.m);\n"
      "    items[i].style.background=on\n"
      "      ?(state.mode==='best'?'rgba(52,211,153,.18)':'rgba(248,113,113,.18)')\n"
      "      :'';\n"
      "    var arr=items[i].querySelector('.attr-lg-arr');\n"
      "    if(arr) arr.textContent=on\n"
      "      ?(state.mode==='best'?' \\u2193':' \\u2191')\n"
      "      :'';\n"
      "  }\n"
      "  if(!state){ status.textContent=''; }\n"
      "  else {\n"
      "    var label;\n"
      "    if(state.m==='-1')\n"
      "      label = state.mode==='best' ? 'cells no model helped'\n"
      "                                  : 'cells no model hurt';\n"
      "    else {\n"
      "      var info=MI[+state.m];\n"
      "      var name=info?info.mask+':'+info.w:state.m;\n"
      "      label = name+(state.mode==='best'?' dominates':' hurts most');\n"
      "    }\n"
      "    status.textContent='\\u2192 '+label;\n"
      "    status.style.color=state.mode==='best'?'#34d399':'#f87171';\n"
      "  }\n"
      "  /* propagate to other linked sections (no-op if not present) */\n"
      "  var sm=state?state.m:null, mo=state?state.mode:null;\n"
      "  if(window.domSetActive) window.domSetActive(sm);\n"
      "  if(window.hurtSetActive) window.hurtSetActive(sm);\n"
      "  if(window.netSetActive) window.netSetActive(sm);\n"
      "  if(window.pmSetActive) window.pmSetActive(sm,mo);\n"
      "  /* persist to URL hash */\n"
      "  if(window.reportState){\n"
      "    window.reportState.m=sm; window.reportState.mode=mo;\n"
      "    if(window.serializeReportState) window.serializeReportState();\n"
      "  }\n"
      "}\n"
      "legend.addEventListener('click',function(e){\n"
      "  var el=e.target.closest('[data-bm]');\n"
      "  if(el) setHilite(el.getAttribute('data-bm'));\n"
      "});\n"
      "/* expose for cross-section triggers */\n"
      "window.attrSetHilite=setHilite;\n"
      "function selectByte(idx){\n"
      "  idx=parseInt(idx); var d=BD[idx]; if(!d) return;\n"
      "  var r=document.querySelector('#attr-svg rect[data-i=\"'+idx+'\"]');\n"
      "  if(!r) return;\n"
      "  if(selRect) selRect.classList.remove('cmap-sel');\n"
      "  r.classList.add('cmap-sel'); selRect=r;\n"
      "  setHilite(r.getAttribute('data-bm'));\n"
      "  if(window.hexHighlight) window.hexHighlight(idx);\n"
      "  var ch=d.ch?\" '\"+ d.ch +\"'\":'';\n"
      "  var costClr=d.c<3?'#34d399':d.c<6?'#fbbf24':'#f87171';\n"
      "  var h='<div class=\"cd-head\">';\n"
      "  h+='<span class=\"cd-byte\">0x'+d.h+ch+'</span>';\n"
      "  h+='<span class=\"cd-sub\">offset '+d.o+'</span>';\n"
      "  h+='<span class=\"cd-cost\" style=\"color:'+costClr+'\">'+d.c.toFixed(2)+' bits</span>';\n"
      "  h+='</div>';\n"
      "  if(d.m && MI.length){\n"
      "    var mx=0;\n"
      "    for(var i=0;i<d.m.length;i++){var a=Math.abs(d.m[i]);if(a>mx)mx=a;}\n"
      "    if(mx<0.01)mx=1;\n"
      "    for(var i=0;i<d.m.length;i++){\n"
      "      var v=d.m[i];\n"
      "      var pct=Math.min(Math.abs(v)/mx*100,100);\n"
      "      var pc=ATTR_PAL[i]||[100,100,100];\n"
      "      var clr='rgb('+pc[0]+','+pc[1]+','+pc[2]+')';\n"
      "      if(v<-0.01) clr='#f87171';\n"
      "      else if(v<0.01) clr='#353b4f';\n"
      "      var sign=v>0?'+':'';\n"
      "      h+='<div class=\"cd-bar\">';\n"
      "      h+='<span class=\"cd-bar-lbl\">'+MI[i].mask+':'+MI[i].w+'</span>';\n"
      "      h+='<span class=\"cd-bar-track\"><span class=\"cd-bar-fill\" style=\"width:'+pct.toFixed(0)+'%%;background:'+clr+'\"></span></span>';\n"
      "      h+='<span class=\"cd-bar-val\" style=\"color:'+clr+'\">'+sign+v.toFixed(2)+' bits</span>';\n"
      "      h+='</div>';\n"
      "    }\n"
      "  }\n"
      "  panel.innerHTML=h;\n"
      "  panel.style.display='block';\n"
      "}\n"
      "window.attrSelectByte=selectByte;\n"
      "document.getElementById('attr-svg').addEventListener('click',function(e){\n"
      "  var r=e.target; if(r.tagName!=='rect') return;\n"
      "  var idx=r.getAttribute('data-i'); if(idx===null) return;\n"
      "  selectByte(idx);\n"
      "});\n"
      "/* hover tooltip for quick preview without clicking */\n"
      "var atip=document.getElementById('attr-tip');\n"
      "var asvg=document.getElementById('attr-svg');\n"
      "function hideAtip(){atip.style.display='none';}\n"
      "asvg.addEventListener('mousemove',function(e){\n"
      "  var r=e.target; if(r.tagName!=='rect'){hideAtip();return;}\n"
      "  var idx=r.getAttribute('data-i'); if(idx===null) return;\n"
      "  idx=parseInt(idx); var d=BD[idx]; if(!d) return;\n"
      "  var bm=r.getAttribute('data-bm');\n"
      "  var costClr=d.c<3?'#34d399':d.c<6?'#fbbf24':'#f87171';\n"
      "  var ch=d.ch?\" '\"+d.ch+\"'\":'';\n"
      "  var h='<div class=\"tip-row\"><span style=\"color:var(--fg3)\">byte</span>'\n"
      "    +'<span style=\"color:var(--fg)\">0x'+d.h+ch+'</span></div>'\n"
      "    +'<div class=\"tip-row\"><span style=\"color:var(--fg3)\">offset</span>'\n"
      "    +'<span>'+d.o+'</span></div>'\n"
      "    +'<div class=\"tip-row\"><span style=\"color:var(--fg3)\">cost</span>'\n"
      "    +'<span style=\"color:'+costClr+';font-weight:600\">'+d.c.toFixed(2)+' bits</span></div>';\n"
      "  if(bm!==null && bm!=='-1'){\n"
      "    var mi=MI[+bm];\n"
      "    var pc=ATTR_PAL[+bm]||[100,100,100];\n"
      "    var mv=d.m?d.m[+bm]:0;\n"
      "    h+='<div class=\"tip-row\"><span style=\"color:var(--fg3)\">best</span>'\n"
      "      +'<span><span class=\"tip-sw\" style=\"background:rgb('+pc[0]+','+pc[1]+','+pc[2]+')\"></span>'\n"
      "      +(mi?mi.mask+':'+mi.w:bm)+' (+'+mv.toFixed(2)+')</span></div>';\n"
      "  }\n"
      "  /* cost window sparkline */\n"
      "  if(window.makeSparkline){\n"
      "    var lo=Math.max(0,idx-32), hi=Math.min(BD.length-1,idx+32);\n"
      "    var vals=[];\n"
      "    for(var k=lo;k<=hi;k++) vals.push(BD[k]?BD[k].c:0);\n"
      "    var sp=window.makeSparkline(vals,{\n"
      "      color:'#22d3ee', highlightIdx:idx-lo,\n"
      "      labelTop:'cost', labelRight:lo+'\\u2026'+hi});\n"
      "    if(sp) h += '<div style=\"color:var(--fg3);font-size:10px;'\n"
      "      +'margin-top:6px\">context cost</div>'+sp;\n"
      "  }\n"
      "  atip.innerHTML=h;\n"
      "  var pr=asvg.parentNode.getBoundingClientRect();\n"
      "  var tx=(e.clientX-pr.left)+14, ty=(e.clientY-pr.top)-60;\n"
      "  if(tx+220>pr.width) tx=(e.clientX-pr.left)-220;\n"
      "  if(ty<0) ty=(e.clientY-pr.top)+18;\n"
      "  atip.style.left=tx+'px'; atip.style.top=ty+'px';\n"
      "  atip.style.display='block';\n"
      "});\n"
      "asvg.addEventListener('mouseleave',hideAtip);\n"
      "})();\n");

    fprintf(f, "</script>\n");
    fprintf(f, "</div>\n\n");
  }

  /* ── Model Dominance Timeline ── */
  if (s->byte_model_contrib && s->byte_costs && s->num_data_bytes > 1
      && s->num_models > 0) {
    int nb = s->num_data_bytes;
    int nm = s->num_models;
    int win = nb / 64;
    if (win < 4) win = 4;
    if (win > 64) win = 64;
    int npts = nb - win + 1;
    if (npts < 2) npts = 2;

    /* compute rolling window contribution per model (positive only = bits saved) */
    float *mdata = (float *)calloc((size_t)npts * nm, sizeof(float));
    float smax = 0;

    for (int i = 0; i < npts; i++) {
      int end = i + win;
      if (end > nb) end = nb;
      int cnt = end - i;
      float row_total = 0;
      for (int m = 0; m < nm; m++) {
        float sum = 0;
        for (int j = i; j < end; j++) {
          float v = s->byte_model_contrib[j * nm + m];
          if (v > 0) sum += v;
        }
        mdata[i * nm + m] = sum / cnt; /* avg bits saved per byte in window */
        row_total += mdata[i * nm + m];
      }
      if (row_total > smax) smax = row_total;
    }
    if (smax < 0.01f) smax = 1.0f;

    /* palette - same as attribution map */
    static const int dom_pal[][3] = {
      {34,211,238},{251,146,60},{167,139,250},{52,211,153},
      {251,191,36},{248,113,113},{96,165,250},{232,121,249},
      {163,230,53},{244,114,182},{45,212,191},{253,186,116},
      {134,239,172},{196,181,253},{252,211,77},{125,211,252},
      {249,168,212},{190,242,100},{253,164,175},{110,231,183},
      {217,70,239},
    };
    int dom_npal = (int)(sizeof(dom_pal) / sizeof(dom_pal[0]));

    int svg_w = 960, svg_h = 200;
    int pad_l = 44, pad_r = 12, pad_t = 12, pad_b = 28;

    fprintf(f,
      "<div class=\"card full\" id=\"sec-dominance\">\n"
      "<h2>Model Dominance Timeline</h2>\n"
      "<p class=\"desc\">Stacked area: each model's contribution (bits saved/byte) "
      "over file position. Window = %d bytes. "
      "<span style=\"color:var(--fg3)\">Click a model in the legend or chart "
      "to rebase it to the bottom.</span></p>\n", win);

    fprintf(f, "<div id=\"dom-legend\" style=\"display:flex;flex-wrap:wrap;"
      "gap:6px 14px;margin-bottom:10px;font-size:11px;"
      "font-family:var(--mono)\"></div>\n");
    fprintf(f, "<div class=\"scrub-wrap\">\n");
    fprintf(f, "<div id=\"dom-chart\"></div>\n");
    fprintf(f, "<div id=\"dom-tip\" class=\"hover-tip\"></div>\n");
    fprintf(f, "</div>\n");

    /* Embed raw per-window data + render code. Embedding mdata rather than
       pre-stacked values lets JS re-stack on click. */
    fprintf(f, "<script>(function(){\n");
    fprintf(f, "var nm=%d,npts=%d,nb=%d,smax=%g;\n", nm, npts, nb, smax);
    fprintf(f, "var W=%d,H=%d,PL=%d,PR=%d,PT=%d,PB=%d;\n",
      svg_w, svg_h, pad_l, pad_r, pad_t, pad_b);
    fprintf(f, "var pw=W-PL-PR,ph=H-PT-PB;\n");

    fprintf(f, "var pal=[");
    for (int m = 0; m < nm; m++) {
      int pi = m % dom_npal;
      fprintf(f, "%s[%d,%d,%d]", m ? "," : "",
        dom_pal[pi][0], dom_pal[pi][1], dom_pal[pi][2]);
    }
    fprintf(f, "];\n");

    fprintf(f, "var labels=[");
    for (int m = 0; m < nm; m++) {
      fprintf(f, "%s\"%02X:%d\"", m ? "," : "",
        s->model_masks[m], s->model_weights[m]);
    }
    fprintf(f, "];\n");

    fprintf(f, "var mdata=[");
    for (int i = 0; i < npts; i++) {
      for (int m = 0; m < nm; m++) {
        fprintf(f, "%s%.4g", (i || m) ? "," : "", mdata[i * nm + m]);
      }
    }
    fprintf(f, "];\n");

    fprintf(f, "%s",
      "var order=[];for(var i=0;i<nm;i++)order.push(i);\n"
      "var legend=document.getElementById('dom-legend');\n"
      "var chart=document.getElementById('dom-chart');\n"
      "function path(d,fill,m){\n"
      "  return '<path d=\"'+d+'\" fill=\"'+fill+'\" fill-opacity=\".75\" '\n"
      "    +'data-m=\"'+m+'\" style=\"cursor:pointer\"/>';\n"
      "}\n"
      "function xCoord(i){return (PL+(i*pw/(npts>1?npts-1:1)))|0;}\n"
      "function yCoord(v){\n"
      "  var y=(PT+(ph*(1-v/smax)))|0;\n"
      "  if(y<PT)y=PT;if(y>PT+ph)y=PT+ph;return y;\n"
      "}\n"
      "function render(){\n"
      "  /* legend - always in palette order so colors stay put */\n"
      "  var lh='';\n"
      "  for(var m=0;m<nm;m++){\n"
      "    var isBase=order[0]===m;\n"
      "    lh += '<span data-m=\"'+m+'\" style=\"display:inline-flex;'\n"
      "      +'align-items:center;gap:4px;cursor:pointer;padding:2px 5px;'\n"
      "      +'border-radius:3px;'\n"
      "      +(isBase?'background:rgba(127,140,170,.18);color:var(--fg)':'')+'\">'\n"
      "      +'<span style=\"display:inline-block;width:10px;height:10px;'\n"
      "      +'border-radius:2px;background:rgb('+pal[m].join(',')+')\"></span>'\n"
      "      +labels[m]+(isBase?' \\u2193':'')+'</span>';\n"
      "  }\n"
      "  legend.innerHTML=lh;\n"
      "  /* stack along x using current order */\n"
      "  var stk=new Float32Array(npts*nm);\n"
      "  for(var i=0;i<npts;i++){\n"
      "    var c=0;\n"
      "    for(var k=0;k<nm;k++){c+=mdata[i*nm+order[k]];stk[i*nm+k]=c;}\n"
      "  }\n"
      "  var s='<svg width=\"100%\" viewBox=\"0 0 '+W+' '+H\n"
      "    +'\" style=\"display:block\">';\n"
      "  s += '<rect x=\"'+PL+'\" y=\"'+PT+'\" width=\"'+pw+'\" height=\"'+ph\n"
      "    +'\" fill=\"var(--bg3)\" rx=\"4\"/>';\n"
      "  /* y gridlines */\n"
      "  for(var i=0;i<=4;i++){\n"
      "    var val=smax*(4-i)/4, y=(PT+ph*i/4)|0;\n"
      "    s += '<line x1=\"'+PL+'\" y1=\"'+y+'\" x2=\"'+(PL+pw)+'\" y2=\"'+y\n"
      "      +'\" stroke=\"var(--bdr)\" stroke-width=\"0.5\"/>'\n"
      "      +'<text x=\"'+(PL-5)+'\" y=\"'+(y+3)+'\" text-anchor=\"end\" '\n"
      "      +'font-size=\"9\" fill=\"var(--fg3)\">'+val.toFixed(1)+'</text>';\n"
      "  }\n"
      "  /* x labels */\n"
      "  for(var i=0;i<=5;i++){\n"
      "    var off=(((nb-1)*i/5)|0), x=(PL+(pw*i/5))|0;\n"
      "    s += '<text x=\"'+x+'\" y=\"'+(PT+ph+16)+'\" text-anchor=\"middle\" '\n"
      "      +'font-size=\"9\" fill=\"var(--fg3)\">'+off+'</text>';\n"
      "  }\n"
      "  /* paths - top of stack first so lower bands aren't obscured */\n"
      "  for(var k=nm-1;k>=0;k--){\n"
      "    var d='';\n"
      "    for(var i=0;i<npts;i++)\n"
      "      d += (i?'L':'M')+xCoord(i)+','+yCoord(stk[i*nm+k])+' ';\n"
      "    for(var i=npts-1;i>=0;i--)\n"
      "      d += 'L'+xCoord(i)+','+yCoord(k>0?stk[i*nm+k-1]:0)+' ';\n"
      "    s += path(d+'Z','rgb('+pal[order[k]].join(',')+')',order[k]);\n"
      "  }\n"
      "  /* top line */\n"
      "  var dt='';\n"
      "  for(var i=0;i<npts;i++)\n"
      "    dt += (i?'L':'M')+xCoord(i)+','+yCoord(stk[i*nm+nm-1])+' ';\n"
      "  s += '<path d=\"'+dt+'\" fill=\"none\" '\n"
      "    +'stroke=\"var(--fg3)\" stroke-width=\"0.5\"/>';\n"
      "  /* scrubber line, hidden initially */\n"
      "  s += '<line id=\"dom-scrub\" class=\"scrub-line\" '\n"
      "    +'x1=\"0\" y1=\"'+PT+'\" x2=\"0\" y2=\"'+(PT+ph)+'\"/>';\n"
      "  s += '</svg>';\n"
      "  chart.innerHTML=s;\n"
      "}\n"
      "function rebase(m){\n"
      "  order=order.filter(function(x){return x!==m;});\n"
      "  order.unshift(m);\n"
      "  render();\n"
      "}\n"
      "/* observer hook: Attribution Map drives this on cross-section selection */\n"
      "window.domSetActive=function(m){\n"
      "  if(m===null||m===undefined||m==='-1'){\n"
      "    order=[]; for(var i=0;i<nm;i++) order.push(i);\n"
      "  } else {\n"
      "    var n=+m;\n"
      "    if(n>=0&&n<nm){\n"
      "      order=order.filter(function(x){return x!==n;});\n"
      "      order.unshift(n);\n"
      "    }\n"
      "  }\n"
      "  render();\n"
      "};\n"
      "function onClick(e){\n"
      "  var el=e.target.closest('[data-m]');\n"
      "  if(!el) return;\n"
      "  var m=el.getAttribute('data-m');\n"
      "  /* route through Attribution Map so the tri-state cycle stays consistent */\n"
      "  if(window.attrSetHilite) window.attrSetHilite(m);\n"
      "  else rebase(+m);\n"
      "}\n"
      "legend.addEventListener('click',onClick);\n"
      "chart.addEventListener('click',onClick);\n"
      "/* hover scrubber: vertical line + tooltip with per-model breakdown */\n"
      "var tip=document.getElementById('dom-tip');\n"
      "function hideTip(){\n"
      "  var l=document.getElementById('dom-scrub');\n"
      "  if(l) l.setAttribute('opacity','0');\n"
      "  tip.style.display='none';\n"
      "}\n"
      "/* hover legend swatch -> per-model sparkline */\n"
      "legend.addEventListener('mousemove',function(e){\n"
      "  var el=e.target.closest('[data-m]');\n"
      "  if(!el){hideTip();return;}\n"
      "  var m=parseInt(el.getAttribute('data-m'));\n"
      "  var vals=[];\n"
      "  for(var i=0;i<npts;i++) vals.push(mdata[i*nm+m]);\n"
      "  var mx=0; for(var i=0;i<npts;i++) if(vals[i]>mx) mx=vals[i];\n"
      "  var sp=window.makeSparkline?window.makeSparkline(vals,{\n"
      "    color:'rgb('+pal[m].join(',')+')',\n"
      "    labelTop:'+'+mx.toFixed(2), labelRight:nb+' B'}):'';\n"
      "  tip.innerHTML='<div class=\"tip-row\"><span style=\"color:var(--fg3)\">model</span>'\n"
      "    +'<span style=\"color:var(--fg)\">'+labels[m]+'</span></div>'\n"
      "    +'<div style=\"color:var(--fg3);font-size:10px;margin-top:4px\">'\n"
      "    +'gain per window</div>'+sp;\n"
      "  var wr=chart.parentNode.getBoundingClientRect();\n"
      "  var tx=(e.clientX-wr.left)+12, ty=(e.clientY-wr.top)+14;\n"
      "  if(tx+220>wr.width) tx=(e.clientX-wr.left)-220;\n"
      "  tip.style.left=tx+'px'; tip.style.top=ty+'px';\n"
      "  tip.style.display='block';\n"
      "});\n"
      "legend.addEventListener('mouseleave',hideTip);\n"
      "chart.addEventListener('mousemove',function(e){\n"
      "  var svg=chart.querySelector('svg'); if(!svg) return;\n"
      "  var r=svg.getBoundingClientRect();\n"
      "  var px=(e.clientX-r.left)/r.width*W;\n"
      "  if(px<PL||px>PL+pw){hideTip();return;}\n"
      "  var i=Math.round((px-PL)/pw*(npts-1));\n"
      "  if(i<0)i=0; if(i>=npts)i=npts-1;\n"
      "  var bytePos=Math.round(i*(nb-1)/(npts-1));\n"
      "  /* gather per-model values at this window, sorted desc */\n"
      "  var rows=[];\n"
      "  var total=0;\n"
      "  for(var m=0;m<nm;m++){\n"
      "    var v=mdata[i*nm+m];\n"
      "    if(v>0.001){rows.push({m:m,v:v}); total+=v;}\n"
      "  }\n"
      "  rows.sort(function(a,b){return b.v-a.v;});\n"
      "  var l=document.getElementById('dom-scrub');\n"
      "  if(l){\n"
      "    var x=PL+i*pw/(npts-1);\n"
      "    l.setAttribute('x1',x); l.setAttribute('x2',x);\n"
      "    l.setAttribute('opacity','0.5');\n"
      "  }\n"
      "  var h='<div class=\"tip-row\"><span style=\"color:var(--fg3)\">byte</span>'\n"
      "    +'<span style=\"color:var(--fg)\">~'+bytePos+'</span></div>'\n"
      "    +'<div class=\"tip-row\"><span style=\"color:var(--fg3)\">total</span>'\n"
      "    +'<span style=\"color:#34d399;font-weight:600\">'+total.toFixed(2)+' b/B</span></div>';\n"
      "  if(rows.length){h += '<div style=\"border-top:1px solid var(--bdr);'\n"
      "    +'margin:4px 0 2px;padding-top:4px\"></div>';}\n"
      "  var show=Math.min(rows.length,5);\n"
      "  for(var k=0;k<show;k++){\n"
      "    var rr=rows[k];\n"
      "    h+='<div class=\"tip-row\">'\n"
      "      +'<span><span class=\"tip-sw\" style=\"background:rgb('+pal[rr.m].join(',')+')\"></span>'\n"
      "      +labels[rr.m]+'</span>'\n"
      "      +'<span style=\"color:var(--fg)\">'+rr.v.toFixed(2)+'</span></div>';\n"
      "  }\n"
      "  if(rows.length>show) h+='<div class=\"tip-row\"><span style=\"color:var(--fg3)\">+'\n"
      "    +(rows.length-show)+' more</span></div>';\n"
      "  tip.innerHTML=h;\n"
      "  var wrap=chart.parentNode;\n"
      "  var wr=wrap.getBoundingClientRect();\n"
      "  var tx=(e.clientX-wr.left)+12, ty=(e.clientY-wr.top)-40;\n"
      "  if(tx+220>wr.width) tx=(e.clientX-wr.left)-220;\n"
      "  if(ty<0) ty=(e.clientY-wr.top)+16;\n"
      "  tip.style.left=tx+'px'; tip.style.top=ty+'px';\n"
      "  tip.style.display='block';\n"
      "});\n"
      "chart.addEventListener('mouseleave',hideTip);\n"
      "render();\n"
      "})();</script>\n");

    fprintf(f, "</div>\n\n");

    free(mdata);
  }

  /* ── Model Hurt Timeline ── */
  if (s->byte_model_contrib && s->byte_costs && s->num_data_bytes > 1
      && s->num_models > 0) {
    int nb = s->num_data_bytes;
    int nm = s->num_models;
    int win = nb / 64;
    if (win < 4) win = 4;
    if (win > 64) win = 64;
    int npts = nb - win + 1;
    if (npts < 2) npts = 2;

    /* compute rolling window penalty per model (negative only = bits lost) */
    float *mdata = (float *)calloc((size_t)npts * nm, sizeof(float));
    float smax = 0;

    for (int i = 0; i < npts; i++) {
      int end = i + win;
      if (end > nb) end = nb;
      int cnt = end - i;
      float row_total = 0;
      for (int m = 0; m < nm; m++) {
        float sum = 0;
        for (int j = i; j < end; j++) {
          float v = s->byte_model_contrib[j * nm + m];
          if (v < 0) sum += -v;
        }
        mdata[i * nm + m] = sum / cnt;
        row_total += mdata[i * nm + m];
      }
      if (row_total > smax) smax = row_total;
    }
    if (smax < 0.01f) smax = 1.0f;

    /* same palette as dominance to keep model identity consistent across charts */
    static const int hurt_pal[][3] = {
      {34,211,238},{251,146,60},{167,139,250},{52,211,153},
      {251,191,36},{248,113,113},{96,165,250},{232,121,249},
      {163,230,53},{244,114,182},{45,212,191},{253,186,116},
      {134,239,172},{196,181,253},{252,211,77},{125,211,252},
      {249,168,212},{190,242,100},{253,164,175},{110,231,183},
      {217,70,239},
    };
    int hurt_npal = (int)(sizeof(hurt_pal) / sizeof(hurt_pal[0]));

    int svg_w = 960, svg_h = 200;
    int pad_l = 44, pad_r = 12, pad_t = 12, pad_b = 28;

    fprintf(f,
      "<div class=\"card full\" id=\"sec-hurt\">\n"
      "<h2>Model Hurt Timeline</h2>\n"
      "<p class=\"desc\">Stacked area: each model's penalty (bits lost/byte) "
      "over file position. Window = %d bytes. "
      "<span style=\"color:var(--fg3)\">Click a model in the legend or chart "
      "to rebase it to the bottom.</span></p>\n", win);

    fprintf(f, "<div id=\"hurt-legend\" style=\"display:flex;flex-wrap:wrap;"
      "gap:6px 14px;margin-bottom:10px;font-size:11px;"
      "font-family:var(--mono)\"></div>\n");
    fprintf(f, "<div class=\"scrub-wrap\">\n");
    fprintf(f, "<div id=\"hurt-chart\"></div>\n");
    fprintf(f, "<div id=\"hurt-tip\" class=\"hover-tip\"></div>\n");
    fprintf(f, "</div>\n");

    fprintf(f, "<script>(function(){\n");
    fprintf(f, "var nm=%d,npts=%d,nb=%d,smax=%g;\n", nm, npts, nb, smax);
    fprintf(f, "var W=%d,H=%d,PL=%d,PR=%d,PT=%d,PB=%d;\n",
      svg_w, svg_h, pad_l, pad_r, pad_t, pad_b);
    fprintf(f, "var pw=W-PL-PR,ph=H-PT-PB;\n");

    fprintf(f, "var pal=[");
    for (int m = 0; m < nm; m++) {
      int pi = m % hurt_npal;
      fprintf(f, "%s[%d,%d,%d]", m ? "," : "",
        hurt_pal[pi][0], hurt_pal[pi][1], hurt_pal[pi][2]);
    }
    fprintf(f, "];\n");

    fprintf(f, "var labels=[");
    for (int m = 0; m < nm; m++) {
      fprintf(f, "%s\"%02X:%d\"", m ? "," : "",
        s->model_masks[m], s->model_weights[m]);
    }
    fprintf(f, "];\n");

    fprintf(f, "var mdata=[");
    for (int i = 0; i < npts; i++) {
      for (int m = 0; m < nm; m++) {
        fprintf(f, "%s%.4g", (i || m) ? "," : "", mdata[i * nm + m]);
      }
    }
    fprintf(f, "];\n");

    fprintf(f, "%s",
      "var order=[];for(var i=0;i<nm;i++)order.push(i);\n"
      "var legend=document.getElementById('hurt-legend');\n"
      "var chart=document.getElementById('hurt-chart');\n"
      "function path(d,fill,m){\n"
      "  return '<path d=\"'+d+'\" fill=\"'+fill+'\" fill-opacity=\".75\" '\n"
      "    +'data-m=\"'+m+'\" style=\"cursor:pointer\"/>';\n"
      "}\n"
      "function xCoord(i){return (PL+(i*pw/(npts>1?npts-1:1)))|0;}\n"
      "function yCoord(v){\n"
      "  var y=(PT+(ph*(1-v/smax)))|0;\n"
      "  if(y<PT)y=PT;if(y>PT+ph)y=PT+ph;return y;\n"
      "}\n"
      "function render(){\n"
      "  var lh='';\n"
      "  for(var m=0;m<nm;m++){\n"
      "    var isBase=order[0]===m;\n"
      "    lh += '<span data-m=\"'+m+'\" style=\"display:inline-flex;'\n"
      "      +'align-items:center;gap:4px;cursor:pointer;padding:2px 5px;'\n"
      "      +'border-radius:3px;'\n"
      "      +(isBase?'background:rgba(127,140,170,.18);color:var(--fg)':'')+'\">'\n"
      "      +'<span style=\"display:inline-block;width:10px;height:10px;'\n"
      "      +'border-radius:2px;background:rgb('+pal[m].join(',')+')\"></span>'\n"
      "      +labels[m]+(isBase?' \\u2193':'')+'</span>';\n"
      "  }\n"
      "  legend.innerHTML=lh;\n"
      "  var stk=new Float32Array(npts*nm);\n"
      "  for(var i=0;i<npts;i++){\n"
      "    var c=0;\n"
      "    for(var k=0;k<nm;k++){c+=mdata[i*nm+order[k]];stk[i*nm+k]=c;}\n"
      "  }\n"
      "  var s='<svg width=\"100%\" viewBox=\"0 0 '+W+' '+H\n"
      "    +'\" style=\"display:block\">';\n"
      "  s += '<rect x=\"'+PL+'\" y=\"'+PT+'\" width=\"'+pw+'\" height=\"'+ph\n"
      "    +'\" fill=\"var(--bg3)\" rx=\"4\"/>';\n"
      "  for(var i=0;i<=4;i++){\n"
      "    var val=smax*(4-i)/4, y=(PT+ph*i/4)|0;\n"
      "    s += '<line x1=\"'+PL+'\" y1=\"'+y+'\" x2=\"'+(PL+pw)+'\" y2=\"'+y\n"
      "      +'\" stroke=\"var(--bdr)\" stroke-width=\"0.5\"/>'\n"
      "      +'<text x=\"'+(PL-5)+'\" y=\"'+(y+3)+'\" text-anchor=\"end\" '\n"
      "      +'font-size=\"9\" fill=\"var(--fg3)\">'+val.toFixed(2)+'</text>';\n"
      "  }\n"
      "  for(var i=0;i<=5;i++){\n"
      "    var off=(((nb-1)*i/5)|0), x=(PL+(pw*i/5))|0;\n"
      "    s += '<text x=\"'+x+'\" y=\"'+(PT+ph+16)+'\" text-anchor=\"middle\" '\n"
      "      +'font-size=\"9\" fill=\"var(--fg3)\">'+off+'</text>';\n"
      "  }\n"
      "  for(var k=nm-1;k>=0;k--){\n"
      "    var d='';\n"
      "    for(var i=0;i<npts;i++)\n"
      "      d += (i?'L':'M')+xCoord(i)+','+yCoord(stk[i*nm+k])+' ';\n"
      "    for(var i=npts-1;i>=0;i--)\n"
      "      d += 'L'+xCoord(i)+','+yCoord(k>0?stk[i*nm+k-1]:0)+' ';\n"
      "    s += path(d+'Z','rgb('+pal[order[k]].join(',')+')',order[k]);\n"
      "  }\n"
      "  var dt='';\n"
      "  for(var i=0;i<npts;i++)\n"
      "    dt += (i?'L':'M')+xCoord(i)+','+yCoord(stk[i*nm+nm-1])+' ';\n"
      "  s += '<path d=\"'+dt+'\" fill=\"none\" '\n"
      "    +'stroke=\"var(--fg3)\" stroke-width=\"0.5\"/>';\n"
      "  s += '<line id=\"hurt-scrub\" class=\"scrub-line\" '\n"
      "    +'x1=\"0\" y1=\"'+PT+'\" x2=\"0\" y2=\"'+(PT+ph)+'\"/>';\n"
      "  s += '</svg>';\n"
      "  chart.innerHTML=s;\n"
      "}\n"
      "function rebase(m){\n"
      "  order=order.filter(function(x){return x!==m;});\n"
      "  order.unshift(m);\n"
      "  render();\n"
      "}\n"
      "window.hurtSetActive=function(m){\n"
      "  if(m===null||m===undefined||m==='-1'){\n"
      "    order=[]; for(var i=0;i<nm;i++) order.push(i);\n"
      "  } else {\n"
      "    var n=+m;\n"
      "    if(n>=0&&n<nm){\n"
      "      order=order.filter(function(x){return x!==n;});\n"
      "      order.unshift(n);\n"
      "    }\n"
      "  }\n"
      "  render();\n"
      "};\n"
      "function onClick(e){\n"
      "  var el=e.target.closest('[data-m]');\n"
      "  if(!el) return;\n"
      "  var m=el.getAttribute('data-m');\n"
      "  if(window.attrSetHilite) window.attrSetHilite(m);\n"
      "  else rebase(+m);\n"
      "}\n"
      "legend.addEventListener('click',onClick);\n"
      "chart.addEventListener('click',onClick);\n"
      "var tip=document.getElementById('hurt-tip');\n"
      "legend.addEventListener('mousemove',function(e){\n"
      "  var el=e.target.closest('[data-m]');\n"
      "  if(!el){tip.style.display='none';return;}\n"
      "  var m=parseInt(el.getAttribute('data-m'));\n"
      "  var vals=[];\n"
      "  for(var i=0;i<npts;i++) vals.push(mdata[i*nm+m]);\n"
      "  var mx=0; for(var i=0;i<npts;i++) if(vals[i]>mx) mx=vals[i];\n"
      "  var sp=window.makeSparkline?window.makeSparkline(vals,{\n"
      "    color:'rgb('+pal[m].join(',')+')',\n"
      "    labelTop:'+'+mx.toFixed(2), labelRight:nb+' B'}):'';\n"
      "  tip.innerHTML='<div class=\"tip-row\"><span style=\"color:var(--fg3)\">model</span>'\n"
      "    +'<span style=\"color:var(--fg)\">'+labels[m]+'</span></div>'\n"
      "    +'<div style=\"color:var(--fg3);font-size:10px;margin-top:4px\">'\n"
      "    +'loss per window</div>'+sp;\n"
      "  var wr=chart.parentNode.getBoundingClientRect();\n"
      "  var tx=(e.clientX-wr.left)+12, ty=(e.clientY-wr.top)+14;\n"
      "  if(tx+220>wr.width) tx=(e.clientX-wr.left)-220;\n"
      "  tip.style.left=tx+'px'; tip.style.top=ty+'px';\n"
      "  tip.style.display='block';\n"
      "});\n"
      "legend.addEventListener('mouseleave',function(){tip.style.display='none';});\n"
      "function hideTip(){\n"
      "  var l=document.getElementById('hurt-scrub');\n"
      "  if(l) l.setAttribute('opacity','0');\n"
      "  tip.style.display='none';\n"
      "}\n"
      "chart.addEventListener('mousemove',function(e){\n"
      "  var svg=chart.querySelector('svg'); if(!svg) return;\n"
      "  var r=svg.getBoundingClientRect();\n"
      "  var px=(e.clientX-r.left)/r.width*W;\n"
      "  if(px<PL||px>PL+pw){hideTip();return;}\n"
      "  var i=Math.round((px-PL)/pw*(npts-1));\n"
      "  if(i<0)i=0; if(i>=npts)i=npts-1;\n"
      "  var bytePos=Math.round(i*(nb-1)/(npts-1));\n"
      "  var rows=[],total=0;\n"
      "  for(var m=0;m<nm;m++){\n"
      "    var v=mdata[i*nm+m];\n"
      "    if(v>0.001){rows.push({m:m,v:v}); total+=v;}\n"
      "  }\n"
      "  rows.sort(function(a,b){return b.v-a.v;});\n"
      "  var l=document.getElementById('hurt-scrub');\n"
      "  if(l){\n"
      "    var x=PL+i*pw/(npts-1);\n"
      "    l.setAttribute('x1',x); l.setAttribute('x2',x);\n"
      "    l.setAttribute('opacity','0.5');\n"
      "  }\n"
      "  var h='<div class=\"tip-row\"><span style=\"color:var(--fg3)\">byte</span>'\n"
      "    +'<span style=\"color:var(--fg)\">~'+bytePos+'</span></div>'\n"
      "    +'<div class=\"tip-row\"><span style=\"color:var(--fg3)\">total</span>'\n"
      "    +'<span style=\"color:#f87171;font-weight:600\">'+total.toFixed(2)+' b/B</span></div>';\n"
      "  if(rows.length){h += '<div style=\"border-top:1px solid var(--bdr);'\n"
      "    +'margin:4px 0 2px;padding-top:4px\"></div>';}\n"
      "  var show=Math.min(rows.length,5);\n"
      "  for(var k=0;k<show;k++){\n"
      "    var rr=rows[k];\n"
      "    h+='<div class=\"tip-row\">'\n"
      "      +'<span><span class=\"tip-sw\" style=\"background:rgb('+pal[rr.m].join(',')+')\"></span>'\n"
      "      +labels[rr.m]+'</span>'\n"
      "      +'<span style=\"color:var(--fg)\">'+rr.v.toFixed(2)+'</span></div>';\n"
      "  }\n"
      "  if(rows.length>show) h+='<div class=\"tip-row\"><span style=\"color:var(--fg3)\">+'\n"
      "    +(rows.length-show)+' more</span></div>';\n"
      "  tip.innerHTML=h;\n"
      "  var wrap=chart.parentNode;\n"
      "  var wr=wrap.getBoundingClientRect();\n"
      "  var tx=(e.clientX-wr.left)+12, ty=(e.clientY-wr.top)-40;\n"
      "  if(tx+220>wr.width) tx=(e.clientX-wr.left)-220;\n"
      "  if(ty<0) ty=(e.clientY-wr.top)+16;\n"
      "  tip.style.left=tx+'px'; tip.style.top=ty+'px';\n"
      "  tip.style.display='block';\n"
      "});\n"
      "chart.addEventListener('mouseleave',hideTip);\n"
      "render();\n"
      "})();</script>\n");

    fprintf(f, "</div>\n\n");

    free(mdata);
  }

  /* ── Model Net Contribution Timeline ── */
  if (s->byte_model_contrib && s->byte_costs && s->num_data_bytes > 1
      && s->num_models > 0) {
    int nb = s->num_data_bytes;
    int nm = s->num_models;
    int win = nb / 64;
    if (win < 4) win = 4;
    if (win > 64) win = 64;
    int npts = nb - win + 1;
    if (npts < 2) npts = 2;

    /* signed per-window per-model net contribution */
    float *mdata = (float *)calloc((size_t)npts * nm, sizeof(float));
    float smax_pos = 0, smax_neg = 0;

    for (int i = 0; i < npts; i++) {
      int end = i + win;
      if (end > nb) end = nb;
      int cnt = end - i;
      float pos_sum = 0, neg_sum = 0;
      for (int m = 0; m < nm; m++) {
        float sum = 0;
        for (int j = i; j < end; j++) {
          sum += s->byte_model_contrib[j * nm + m];
        }
        mdata[i * nm + m] = sum / cnt;
        if (mdata[i * nm + m] > 0) pos_sum += mdata[i * nm + m];
        else                       neg_sum += -mdata[i * nm + m];
      }
      if (pos_sum > smax_pos) smax_pos = pos_sum;
      if (neg_sum > smax_neg) smax_neg = neg_sum;
    }
    if (smax_pos < 0.01f) smax_pos = 1.0f;
    if (smax_neg < 0.01f) smax_neg = 0.5f;

    static const int net_pal[][3] = {
      {34,211,238},{251,146,60},{167,139,250},{52,211,153},
      {251,191,36},{248,113,113},{96,165,250},{232,121,249},
      {163,230,53},{244,114,182},{45,212,191},{253,186,116},
      {134,239,172},{196,181,253},{252,211,77},{125,211,252},
      {249,168,212},{190,242,100},{253,164,175},{110,231,183},
      {217,70,239},
    };
    int net_npal = (int)(sizeof(net_pal) / sizeof(net_pal[0]));

    int svg_w = 960, svg_h = 240;
    int pad_l = 44, pad_r = 12, pad_t = 12, pad_b = 28;

    fprintf(f,
      "<div class=\"card full\" id=\"sec-net\">\n"
      "<h2>Model Net Contribution Timeline</h2>\n"
      "<p class=\"desc\">Net contribution per model per window (gain &minus; hurt). "
      "Above the 0-line stacks models that are net helpful; below stacks "
      "models that are net hurtful. Window = %d bytes. "
      "<span style=\"color:var(--fg3)\">Click a model to rebase.</span></p>\n",
      win);

    fprintf(f, "<div id=\"net-legend\" style=\"display:flex;flex-wrap:wrap;"
      "gap:6px 14px;margin-bottom:10px;font-size:11px;"
      "font-family:var(--mono)\"></div>\n");
    fprintf(f, "<div class=\"scrub-wrap\">\n");
    fprintf(f, "<div id=\"net-chart\"></div>\n");
    fprintf(f, "<div id=\"net-tip\" class=\"hover-tip\"></div>\n");
    fprintf(f, "</div>\n");

    fprintf(f, "<script>(function(){\n");
    fprintf(f, "var nm=%d,npts=%d,nb=%d,smaxP=%g,smaxN=%g;\n",
      nm, npts, nb, smax_pos, smax_neg);
    fprintf(f, "var W=%d,H=%d,PL=%d,PR=%d,PT=%d,PB=%d;\n",
      svg_w, svg_h, pad_l, pad_r, pad_t, pad_b);
    fprintf(f, "var pw=W-PL-PR,ph=H-PT-PB;\n"
      "var upH=ph*smaxP/(smaxP+smaxN), loH=ph-upH;\n"
      "var midY=PT+upH;\n");

    fprintf(f, "var pal=[");
    for (int m = 0; m < nm; m++) {
      int pi = m % net_npal;
      fprintf(f, "%s[%d,%d,%d]", m ? "," : "",
        net_pal[pi][0], net_pal[pi][1], net_pal[pi][2]);
    }
    fprintf(f, "];\n");

    fprintf(f, "var labels=[");
    for (int m = 0; m < nm; m++) {
      fprintf(f, "%s\"%02X:%d\"", m ? "," : "",
        s->model_masks[m], s->model_weights[m]);
    }
    fprintf(f, "];\n");

    fprintf(f, "var mdata=[");
    for (int i = 0; i < npts; i++) {
      for (int m = 0; m < nm; m++) {
        fprintf(f, "%s%.4g", (i || m) ? "," : "", mdata[i * nm + m]);
      }
    }
    fprintf(f, "];\n");

    fprintf(f, "%s",
      "var order=[];for(var i=0;i<nm;i++)order.push(i);\n"
      "var legend=document.getElementById('net-legend');\n"
      "var chart=document.getElementById('net-chart');\n"
      "function path(d,fill,m){\n"
      "  return '<path d=\"'+d+'\" fill=\"'+fill+'\" fill-opacity=\".75\" '\n"
      "    +'data-m=\"'+m+'\" style=\"cursor:pointer\"/>';\n"
      "}\n"
      "function xCoord(i){return (PL+(i*pw/(npts>1?npts-1:1)))|0;}\n"
      "function yUp(v){var y=(midY-(v/smaxP)*upH)|0;\n"
      "  if(y<PT)y=PT;if(y>midY)y=midY;return y;}\n"
      "function yLo(v){var y=(midY+(v/smaxN)*loH)|0;\n"
      "  if(y<midY)y=midY;if(y>PT+ph)y=PT+ph;return y;}\n"
      "function render(){\n"
      "  /* legend */\n"
      "  var lh='';\n"
      "  for(var m=0;m<nm;m++){\n"
      "    var isBase=order[0]===m;\n"
      "    lh += '<span data-m=\"'+m+'\" style=\"display:inline-flex;'\n"
      "      +'align-items:center;gap:4px;cursor:pointer;padding:2px 5px;'\n"
      "      +'border-radius:3px;'\n"
      "      +(isBase?'background:rgba(127,140,170,.18);color:var(--fg)':'')+'\">'\n"
      "      +'<span style=\"display:inline-block;width:10px;height:10px;'\n"
      "      +'border-radius:2px;background:rgb('+pal[m].join(',')+')\"></span>'\n"
      "      +labels[m]+(isBase?' \\u2193':'')+'</span>';\n"
      "  }\n"
      "  legend.innerHTML=lh;\n"
      "  /* per-window: build positive & negative stacks per model in order */\n"
      "  var stkP=new Float32Array(npts*nm), stkN=new Float32Array(npts*nm);\n"
      "  for(var i=0;i<npts;i++){\n"
      "    var cp=0,cn=0;\n"
      "    for(var k=0;k<nm;k++){\n"
      "      var v=mdata[i*nm+order[k]];\n"
      "      if(v>0){cp+=v;} else {cn+=-v;}\n"
      "      stkP[i*nm+k]=cp; stkN[i*nm+k]=cn;\n"
      "    }\n"
      "  }\n"
      "  var s='<svg width=\"100%\" viewBox=\"0 0 '+W+' '+H\n"
      "    +'\" style=\"display:block\">';\n"
      "  s += '<rect x=\"'+PL+'\" y=\"'+PT+'\" width=\"'+pw+'\" height=\"'+ph\n"
      "    +'\" fill=\"var(--bg3)\" rx=\"4\"/>';\n"
      "  /* y gridlines: ticks on both sides of midline */\n"
      "  for(var i=1;i<=2;i++){\n"
      "    var vp=smaxP*i/2, yp=yUp(vp);\n"
      "    s += '<line x1=\"'+PL+'\" y1=\"'+yp+'\" x2=\"'+(PL+pw)+'\" y2=\"'+yp\n"
      "      +'\" stroke=\"var(--bdr)\" stroke-width=\"0.5\"/>'\n"
      "      +'<text x=\"'+(PL-5)+'\" y=\"'+(yp+3)+'\" text-anchor=\"end\" '\n"
      "      +'font-size=\"9\" fill=\"var(--fg3)\">+'+vp.toFixed(1)+'</text>';\n"
      "    var vn=smaxN*i/2, yn=yLo(vn);\n"
      "    s += '<line x1=\"'+PL+'\" y1=\"'+yn+'\" x2=\"'+(PL+pw)+'\" y2=\"'+yn\n"
      "      +'\" stroke=\"var(--bdr)\" stroke-width=\"0.5\"/>'\n"
      "      +'<text x=\"'+(PL-5)+'\" y=\"'+(yn+3)+'\" text-anchor=\"end\" '\n"
      "      +'font-size=\"9\" fill=\"var(--fg3)\">\\u2212'+vn.toFixed(1)+'</text>';\n"
      "  }\n"
      "  /* zero line */\n"
      "  s += '<line x1=\"'+PL+'\" y1=\"'+midY+'\" x2=\"'+(PL+pw)+'\" y2=\"'+midY\n"
      "    +'\" stroke=\"var(--fg2)\" stroke-width=\"1\"/>';\n"
      "  s += '<text x=\"'+(PL-5)+'\" y=\"'+(midY+3)+'\" text-anchor=\"end\" '\n"
      "    +'font-size=\"9\" fill=\"var(--fg3)\">0</text>';\n"
      "  /* x labels */\n"
      "  for(var i=0;i<=5;i++){\n"
      "    var off=(((nb-1)*i/5)|0), x=(PL+(pw*i/5))|0;\n"
      "    s += '<text x=\"'+x+'\" y=\"'+(PT+ph+16)+'\" text-anchor=\"middle\" '\n"
      "      +'font-size=\"9\" fill=\"var(--fg3)\">'+off+'</text>';\n"
      "  }\n"
      "  /* paths: each model gets up to two bands (positive above, negative below) */\n"
      "  for(var k=nm-1;k>=0;k--){\n"
      "    var m=order[k];\n"
      "    /* positive band */\n"
      "    var dP='', hasP=false;\n"
      "    for(var i=0;i<npts;i++){\n"
      "      if(mdata[i*nm+m]>0){hasP=true; break;}\n"
      "    }\n"
      "    if(hasP){\n"
      "      for(var i=0;i<npts;i++)\n"
      "        dP += (i?'L':'M')+xCoord(i)+','+yUp(stkP[i*nm+k])+' ';\n"
      "      for(var i=npts-1;i>=0;i--)\n"
      "        dP += 'L'+xCoord(i)+','+yUp(k>0?stkP[i*nm+k-1]:0)+' ';\n"
      "      s += path(dP+'Z','rgb('+pal[m].join(',')+')',m);\n"
      "    }\n"
      "    /* negative band */\n"
      "    var dN='', hasN=false;\n"
      "    for(var i=0;i<npts;i++){\n"
      "      if(mdata[i*nm+m]<0){hasN=true; break;}\n"
      "    }\n"
      "    if(hasN){\n"
      "      for(var i=0;i<npts;i++)\n"
      "        dN += (i?'L':'M')+xCoord(i)+','+yLo(stkN[i*nm+k])+' ';\n"
      "      for(var i=npts-1;i>=0;i--)\n"
      "        dN += 'L'+xCoord(i)+','+yLo(k>0?stkN[i*nm+k-1]:0)+' ';\n"
      "      s += path(dN+'Z','rgb('+pal[m].join(',')+')',m);\n"
      "    }\n"
      "  }\n"
      "  /* scrubber line */\n"
      "  s += '<line id=\"net-scrub\" class=\"scrub-line\" '\n"
      "    +'x1=\"0\" y1=\"'+PT+'\" x2=\"0\" y2=\"'+(PT+ph)+'\"/>';\n"
      "  s += '</svg>';\n"
      "  chart.innerHTML=s;\n"
      "}\n"
      "function rebase(m){\n"
      "  order=order.filter(function(x){return x!==m;});\n"
      "  order.unshift(m);\n"
      "  render();\n"
      "}\n"
      "/* expose signed per-window data for cross-section sparklines */\n"
      "window.netMdata=mdata; window.netNm=nm; window.netNpts=npts;\n"
      "window.netNb=nb; window.netPal=pal;\n"
      "window.netSetActive=function(m){\n"
      "  if(m===null||m===undefined||m==='-1'){\n"
      "    order=[]; for(var i=0;i<nm;i++) order.push(i);\n"
      "  } else {\n"
      "    var n=+m;\n"
      "    if(n>=0&&n<nm){\n"
      "      order=order.filter(function(x){return x!==n;});\n"
      "      order.unshift(n);\n"
      "    }\n"
      "  }\n"
      "  render();\n"
      "};\n"
      "function onClick(e){\n"
      "  var el=e.target.closest('[data-m]');\n"
      "  if(!el) return;\n"
      "  var m=el.getAttribute('data-m');\n"
      "  if(window.attrSetHilite) window.attrSetHilite(m);\n"
      "  else rebase(+m);\n"
      "}\n"
      "legend.addEventListener('click',onClick);\n"
      "chart.addEventListener('click',onClick);\n"
      "var tip=document.getElementById('net-tip');\n"
      "legend.addEventListener('mousemove',function(e){\n"
      "  var el=e.target.closest('[data-m]');\n"
      "  if(!el){tip.style.display='none';return;}\n"
      "  var m=parseInt(el.getAttribute('data-m'));\n"
      "  var vals=[];\n"
      "  for(var i=0;i<npts;i++) vals.push(mdata[i*nm+m]);\n"
      "  var mxA=0; for(var i=0;i<npts;i++){var a=Math.abs(vals[i]); if(a>mxA) mxA=a;}\n"
      "  var sp=window.makeSparkline?window.makeSparkline(vals,{\n"
      "    color:'rgb('+pal[m].join(',')+')',\n"
      "    labelTop:'+'+mxA.toFixed(2),\n"
      "    labelBot:'\\u2212'+mxA.toFixed(2),\n"
      "    labelRight:nb+' B'}):'';\n"
      "  tip.innerHTML='<div class=\"tip-row\"><span style=\"color:var(--fg3)\">model</span>'\n"
      "    +'<span style=\"color:var(--fg)\">'+labels[m]+'</span></div>'\n"
      "    +'<div style=\"color:var(--fg3);font-size:10px;margin-top:4px\">'\n"
      "    +'net per window</div>'+sp;\n"
      "  var wr=chart.parentNode.getBoundingClientRect();\n"
      "  var tx=(e.clientX-wr.left)+12, ty=(e.clientY-wr.top)+14;\n"
      "  if(tx+220>wr.width) tx=(e.clientX-wr.left)-220;\n"
      "  tip.style.left=tx+'px'; tip.style.top=ty+'px';\n"
      "  tip.style.display='block';\n"
      "});\n"
      "legend.addEventListener('mouseleave',function(){tip.style.display='none';});\n"
      "function hideTip(){\n"
      "  var l=document.getElementById('net-scrub');\n"
      "  if(l) l.setAttribute('opacity','0');\n"
      "  tip.style.display='none';\n"
      "}\n"
      "chart.addEventListener('mousemove',function(e){\n"
      "  var svg=chart.querySelector('svg'); if(!svg) return;\n"
      "  var r=svg.getBoundingClientRect();\n"
      "  var px=(e.clientX-r.left)/r.width*W;\n"
      "  if(px<PL||px>PL+pw){hideTip();return;}\n"
      "  var i=Math.round((px-PL)/pw*(npts-1));\n"
      "  if(i<0)i=0; if(i>=npts)i=npts-1;\n"
      "  var bytePos=Math.round(i*(nb-1)/(npts-1));\n"
      "  var rowsP=[],rowsN=[],netTotal=0;\n"
      "  for(var m=0;m<nm;m++){\n"
      "    var v=mdata[i*nm+m]; netTotal+=v;\n"
      "    if(v>0.001) rowsP.push({m:m,v:v});\n"
      "    else if(v<-0.001) rowsN.push({m:m,v:v});\n"
      "  }\n"
      "  rowsP.sort(function(a,b){return b.v-a.v;});\n"
      "  rowsN.sort(function(a,b){return a.v-b.v;});\n"
      "  var l=document.getElementById('net-scrub');\n"
      "  if(l){\n"
      "    var x=PL+i*pw/(npts-1);\n"
      "    l.setAttribute('x1',x); l.setAttribute('x2',x);\n"
      "    l.setAttribute('opacity','0.5');\n"
      "  }\n"
      "  var netClr=netTotal>0?'#34d399':netTotal<0?'#f87171':'var(--fg2)';\n"
      "  var sign=netTotal>=0?'+':'';\n"
      "  var h='<div class=\"tip-row\"><span style=\"color:var(--fg3)\">byte</span>'\n"
      "    +'<span style=\"color:var(--fg)\">~'+bytePos+'</span></div>'\n"
      "    +'<div class=\"tip-row\"><span style=\"color:var(--fg3)\">net</span>'\n"
      "    +'<span style=\"color:'+netClr+';font-weight:600\">'+sign+netTotal.toFixed(2)+' b/B</span></div>';\n"
      "  function rowList(arr, prefix){\n"
      "    var out='';\n"
      "    var show=Math.min(arr.length,3);\n"
      "    for(var k=0;k<show;k++){\n"
      "      var rr=arr[k];\n"
      "      out+='<div class=\"tip-row\">'\n"
      "        +'<span><span class=\"tip-sw\" style=\"background:rgb('+pal[rr.m].join(',')+')\"></span>'\n"
      "        +labels[rr.m]+'</span>'\n"
      "        +'<span style=\"color:var(--fg)\">'+prefix+rr.v.toFixed(2)+'</span></div>';\n"
      "    }\n"
      "    return out;\n"
      "  }\n"
      "  if(rowsP.length){h+='<div style=\"border-top:1px solid var(--bdr);'\n"
      "    +'margin:4px 0 2px;padding-top:4px;color:#34d399;font-size:10px\">helps</div>';\n"
      "    h+=rowList(rowsP,'+');}\n"
      "  if(rowsN.length){h+='<div style=\"border-top:1px solid var(--bdr);'\n"
      "    +'margin:4px 0 2px;padding-top:4px;color:#f87171;font-size:10px\">hurts</div>';\n"
      "    h+=rowList(rowsN,'');}\n"
      "  tip.innerHTML=h;\n"
      "  var wrap=chart.parentNode;\n"
      "  var wr=wrap.getBoundingClientRect();\n"
      "  var tx=(e.clientX-wr.left)+12, ty=(e.clientY-wr.top)-40;\n"
      "  if(tx+220>wr.width) tx=(e.clientX-wr.left)-220;\n"
      "  if(ty<0) ty=(e.clientY-wr.top)+16;\n"
      "  tip.style.left=tx+'px'; tip.style.top=ty+'px';\n"
      "  tip.style.display='block';\n"
      "});\n"
      "chart.addEventListener('mouseleave',hideTip);\n"
      "render();\n"
      "})();</script>\n");

    fprintf(f, "</div>\n\n");

    free(mdata);
  }

  /* ── Context Depth Timeline ── */
  if (s->byte_model_contrib && s->num_data_bytes > 1 && s->num_models > 0) {
    fprintf(f, "%s",
      "<div class=\"card full\" id=\"sec-depth\">\n"
      "<h2>Context Depth Timeline</h2>\n"
      "<p class=\"desc\">Stacked area of bits saved per byte, grouped by the "
      "contributing model's context depth (mask popcount). Shows how deep "
      "the useful context is across regions of the file.</p>\n"
      "<div id=\"ctxd-legend\" style=\"display:flex;flex-wrap:wrap;"
      "gap:6px 14px;margin-bottom:10px;font-size:11px;"
      "font-family:var(--mono)\"></div>\n"
      "<div class=\"scrub-wrap\">\n"
      "<div id=\"ctxd-chart\"></div>\n"
      "<div id=\"ctxd-tip\" class=\"hover-tip\"></div>\n"
      "</div>\n");

    /* built from BD.m (per-byte contributions) + MI (masks) */
    fprintf(f, "%s",
      "<script>(function(){\n"
      "if(typeof BD==='undefined'||typeof MI==='undefined'"
      "||!BD.length||!BD[0].m) return;\n"
      "var nb=BD.length, nm=MI.length;\n"
      "function pop(x){var c=0;while(x){c+=x&1;x>>=1;}return c;}\n"
      "var depth=[];\n"
      "for(var m=0;m<nm;m++) depth.push(pop(parseInt(MI[m].mask,16)));\n"
      "var depths=[];\n"
      "for(var m=0;m<nm;m++) if(depths.indexOf(depth[m])<0) depths.push(depth[m]);\n"
      "depths.sort(function(a,b){return a-b;});\n"
      "var nd=depths.length;\n"
      "var gidx=depth.map(function(d){return depths.indexOf(d);});\n"
      "/* per-byte positive contribution per depth group */\n"
      "var pb=new Float32Array(nb*nd);\n"
      "for(var i=0;i<nb;i++){\n"
      "  var mm=BD[i].m;\n"
      "  for(var m=0;m<nm;m++){\n"
      "    var v=mm[m];\n"
      "    if(v>0) pb[i*nd+gidx[m]]+=v;\n"
      "  }\n"
      "}\n"
      "/* rolling window via prefix sums */\n"
      "var win=Math.max(4,Math.min(64,nb>>6));\n"
      "var npts=Math.max(2,nb-win+1);\n"
      "var pre=new Float64Array((nb+1)*nd);\n"
      "for(var i=0;i<nb;i++)\n"
      "  for(var d=0;d<nd;d++) pre[(i+1)*nd+d]=pre[i*nd+d]+pb[i*nd+d];\n"
      "var mdata=new Float32Array(npts*nd);\n"
      "var smax=0;\n"
      "for(var i=0;i<npts;i++){\n"
      "  var end=Math.min(i+win,nb), c=end-i, tot=0;\n"
      "  for(var d=0;d<nd;d++){\n"
      "    var v=(pre[end*nd+d]-pre[i*nd+d])/c;\n"
      "    mdata[i*nd+d]=v; tot+=v;\n"
      "  }\n"
      "  if(tot>smax) smax=tot;\n"
      "}\n"
      "if(smax<0.01) smax=1;\n"
      "/* sequential palette shallow (green) -> deep (purple) */\n"
      "var cols=[];\n"
      "for(var d=0;d<nd;d++){\n"
      "  var hue=nd>1?160+120*d/(nd-1):190;\n"
      "  cols.push('hsl('+hue+',70%,55%)');\n"
      "}\n"
      "var dcnt=[];\n"
      "for(var d=0;d<nd;d++){\n"
      "  var c=0;\n"
      "  for(var m=0;m<nm;m++) if(gidx[m]===d) c++;\n"
      "  dcnt.push(c);\n"
      "}\n"
      "var lh='';\n"
      "for(var d=0;d<nd;d++){\n"
      "  lh+='<span style=\"display:inline-flex;align-items:center;gap:4px;'\n"
      "    +'padding:2px 5px\">'\n"
      "    +'<span style=\"display:inline-block;width:10px;height:10px;'\n"
      "    +'border-radius:2px;background:'+cols[d]+'\"></span>'\n"
      "    +'depth '+depths[d]+' ('+dcnt[d]+' model'+(dcnt[d]===1?'':'s')+')</span>';\n"
      "}\n"
      "document.getElementById('ctxd-legend').innerHTML=lh;\n"
      "var W=960,H=200,PL=44,PR=12,PT=12,PB=28;\n"
      "var pw=W-PL-PR,ph=H-PT-PB;\n"
      "function xC(i){return (PL+i*pw/(npts>1?npts-1:1))|0;}\n"
      "function yC(v){\n"
      "  var y=(PT+ph*(1-v/smax))|0;\n"
      "  if(y<PT)y=PT;if(y>PT+ph)y=PT+ph;return y;\n"
      "}\n"
      "var stk=new Float32Array(npts*nd);\n"
      "for(var i=0;i<npts;i++){\n"
      "  var c=0;\n"
      "  for(var d=0;d<nd;d++){c+=mdata[i*nd+d];stk[i*nd+d]=c;}\n"
      "}\n"
      "var s='<svg width=\"100%\" viewBox=\"0 0 '+W+' '+H+'\" style=\"display:block\">';\n"
      "s+='<rect x=\"'+PL+'\" y=\"'+PT+'\" width=\"'+pw+'\" height=\"'+ph\n"
      "  +'\" fill=\"var(--bg3)\" rx=\"4\"/>';\n"
      "for(var i=0;i<=4;i++){\n"
      "  var val=smax*(4-i)/4, y=(PT+ph*i/4)|0;\n"
      "  s+='<line x1=\"'+PL+'\" y1=\"'+y+'\" x2=\"'+(PL+pw)+'\" y2=\"'+y\n"
      "    +'\" stroke=\"var(--bdr)\" stroke-width=\"0.5\"/>'\n"
      "    +'<text x=\"'+(PL-5)+'\" y=\"'+(y+3)+'\" text-anchor=\"end\" '\n"
      "    +'font-size=\"9\" fill=\"var(--fg3)\">'+val.toFixed(1)+'</text>';\n"
      "}\n"
      "for(var i=0;i<=5;i++){\n"
      "  var off=(((nb-1)*i/5)|0), x=(PL+(pw*i/5))|0;\n"
      "  s+='<text x=\"'+x+'\" y=\"'+(PT+ph+16)+'\" text-anchor=\"middle\" '\n"
      "    +'font-size=\"9\" fill=\"var(--fg3)\">'+off+'</text>';\n"
      "}\n"
      "for(var d=nd-1;d>=0;d--){\n"
      "  var p='';\n"
      "  for(var i=0;i<npts;i++) p+=(i?'L':'M')+xC(i)+','+yC(stk[i*nd+d])+' ';\n"
      "  for(var i=npts-1;i>=0;i--) p+='L'+xC(i)+','+yC(d>0?stk[i*nd+d-1]:0)+' ';\n"
      "  s+='<path d=\"'+p+'Z\" fill=\"'+cols[d]+'\" fill-opacity=\".75\"/>';\n"
      "}\n"
      "s+='<line id=\"ctxd-scrub\" class=\"scrub-line\" x1=\"0\" y1=\"'+PT\n"
      "  +'\" x2=\"0\" y2=\"'+(PT+ph)+'\"/>';\n"
      "s+='</svg>';\n"
      "var chart=document.getElementById('ctxd-chart');\n"
      "chart.innerHTML=s;\n"
      "var tip=document.getElementById('ctxd-tip');\n"
      "function hide(){\n"
      "  var l=document.getElementById('ctxd-scrub');\n"
      "  if(l) l.setAttribute('opacity','0');\n"
      "  tip.style.display='none';\n"
      "}\n"
      "chart.addEventListener('mousemove',function(e){\n"
      "  var svg=chart.querySelector('svg'); if(!svg) return;\n"
      "  var r=svg.getBoundingClientRect();\n"
      "  var px=(e.clientX-r.left)/r.width*W;\n"
      "  if(px<PL||px>PL+pw){hide();return;}\n"
      "  var i=Math.round((px-PL)/pw*(npts-1));\n"
      "  if(i<0)i=0; if(i>=npts)i=npts-1;\n"
      "  var bytePos=Math.round(i*(nb-1)/(npts-1));\n"
      "  var l=document.getElementById('ctxd-scrub');\n"
      "  if(l){\n"
      "    var x=PL+i*pw/(npts-1);\n"
      "    l.setAttribute('x1',x); l.setAttribute('x2',x);\n"
      "    l.setAttribute('opacity','0.5');\n"
      "  }\n"
      "  var tot=0;\n"
      "  for(var d=0;d<nd;d++) tot+=mdata[i*nd+d];\n"
      "  var h='<div class=\"tip-row\"><span style=\"color:var(--fg3)\">byte</span>'\n"
      "    +'<span style=\"color:var(--fg)\">~'+bytePos+'</span></div>'\n"
      "    +'<div class=\"tip-row\"><span style=\"color:var(--fg3)\">total</span>'\n"
      "    +'<span style=\"color:#34d399;font-weight:600\">'+tot.toFixed(2)+' b/B</span></div>'\n"
      "    +'<div style=\"border-top:1px solid var(--bdr);margin:4px 0 2px;'\n"
      "    +'padding-top:4px\"></div>';\n"
      "  for(var d=nd-1;d>=0;d--){\n"
      "    var v=mdata[i*nd+d];\n"
      "    if(v<0.001) continue;\n"
      "    h+='<div class=\"tip-row\">'\n"
      "      +'<span><span class=\"tip-sw\" style=\"background:'+cols[d]+'\"></span>'\n"
      "      +'depth '+depths[d]+'</span>'\n"
      "      +'<span style=\"color:var(--fg)\">'+v.toFixed(2)+'</span></div>';\n"
      "  }\n"
      "  tip.innerHTML=h;\n"
      "  var wr=chart.parentNode.getBoundingClientRect();\n"
      "  var tx=(e.clientX-wr.left)+12, ty=(e.clientY-wr.top)-40;\n"
      "  if(tx+200>wr.width) tx=(e.clientX-wr.left)-200;\n"
      "  if(ty<0) ty=(e.clientY-wr.top)+16;\n"
      "  tip.style.left=tx+'px'; tip.style.top=ty+'px';\n"
      "  tip.style.display='block';\n"
      "});\n"
      "chart.addEventListener('mouseleave',hide);\n"
      "})();</script>\n");

    fprintf(f, "</div>\n\n");
  }

  /* ── Model Correlation Matrix ── */
  if (s->byte_model_contrib && s->num_data_bytes > 1 && s->num_models >= 2) {
    int nb = s->num_data_bytes;
    int nm = s->num_models;

    /* Pearson correlation between per-byte contributions of model pairs */
    double mean[MAX_SEARCH] = {0};
    for (int m = 0; m < nm; m++) {
      for (int i = 0; i < nb; i++)
        mean[m] += s->byte_model_contrib[i * nm + m];
      mean[m] /= nb;
    }
    double *cov = (double *)calloc((size_t)nm * nm, sizeof(double));
    int *both_help = (int *)calloc((size_t)nm * nm, sizeof(int));
    int *both_hurt = (int *)calloc((size_t)nm * nm, sizeof(int));
    for (int i = 0; i < nb; i++) {
      const float *row = &s->byte_model_contrib[i * nm];
      for (int a = 0; a < nm; a++) {
        double da = row[a] - mean[a];
        for (int b = a; b < nm; b++) {
          cov[a * nm + b] += da * (row[b] - mean[b]);
          if (row[a] > 0.01f && row[b] > 0.01f)
            both_help[a * nm + b]++;
          if (row[a] < -0.01f && row[b] < -0.01f)
            both_hurt[a * nm + b]++;
        }
      }
    }
    float *corr = (float *)calloc((size_t)nm * nm, sizeof(float));
    for (int a = 0; a < nm; a++) {
      for (int b = a; b < nm; b++) {
        double denom = sqrt_pos(cov[a * nm + a]) * sqrt_pos(cov[b * nm + b]);
        float r = denom > 1e-12 ? (float)(cov[a * nm + b] / denom) : 0;
        if (r > 1) r = 1;
        if (r < -1) r = -1;
        corr[a * nm + b] = r;
        corr[b * nm + a] = r;
        both_help[b * nm + a] = both_help[a * nm + b];
        both_hurt[b * nm + a] = both_hurt[a * nm + b];
      }
    }

    int cell = 30, cgap = 1;
    int lbl = 44;
    int gridsz = nm * (cell + cgap) - cgap;
    int svg_w = lbl + gridsz + 2;
    int svg_h = lbl + gridsz + 2;

    fprintf(f,
      "<div class=\"card full\" id=\"sec-corr\" style=\"position:relative\">\n"
      "<h2>Model Correlation</h2>\n"
      "<p class=\"desc\">Pearson correlation of per-byte contributions. "
      "<span style=\"color:#22d3ee\">Cyan</span> = models help on the same "
      "bytes (redundant), <span style=\"color:#f87171\">red</span> = "
      "anti-correlated (complementary).</p>\n");
    fprintf(f, "<div class=\"scrub-wrap\">\n");
    fprintf(f, "<div id=\"corr-tip\" class=\"hover-tip\"></div>\n");
    fprintf(f,
      "<svg id=\"corr-svg\" width=\"100%%\" viewBox=\"0 0 %d %d\" "
      "style=\"font-family:var(--mono);display:block;margin:0 auto;"
      "max-width:%dpx\">\n",
      svg_w, svg_h, svg_w * 2);

    /* column labels, rotated */
    for (int b = 0; b < nm; b++) {
      int x = lbl + b * (cell + cgap) + cell / 2;
      fprintf(f,
        "<text x=\"%d\" y=\"%d\" text-anchor=\"start\" font-size=\"9\" "
        "fill=\"var(--fg3)\" transform=\"rotate(-60 %d %d)\">%02X:%d</text>\n",
        x, lbl - 6, x, lbl - 6, s->model_masks[b], s->model_weights[b]);
    }
    for (int a = 0; a < nm; a++) {
      int y = lbl + a * (cell + cgap);
      fprintf(f,
        "<text x=\"%d\" y=\"%d\" text-anchor=\"end\" font-size=\"9\" "
        "fill=\"var(--fg3)\">%02X:%d</text>\n",
        lbl - 5, y + cell / 2 + 3, s->model_masks[a], s->model_weights[a]);
      for (int b = 0; b < nm; b++) {
        int x = lbl + b * (cell + cgap);
        float r = corr[a * nm + b];
        float ar = r < 0 ? -r : r;
        const char *fill = r >= 0 ? "#22d3ee" : "#f87171";
        float op = a == b ? 0.9f : 0.05f + 0.85f * ar;
        fprintf(f,
          "<rect x=\"%d\" y=\"%d\" width=\"%d\" height=\"%d\" rx=\"2\" "
          "fill=\"%s\" fill-opacity=\"%.2f\" data-a=\"%d\" data-b=\"%d\" "
          "style=\"cursor:pointer\"/>\n",
          x, y, cell, cell, fill, op, a, b);
        if (a != b)
          fprintf(f,
            "<text x=\"%d\" y=\"%d\" text-anchor=\"middle\" font-size=\"8\" "
            "fill=\"%s\" pointer-events=\"none\">%.2f</text>\n",
            x + cell / 2, y + cell / 2 + 3,
            ar > 0.55f ? "#0c0e14" : "var(--fg2)", r);
      }
    }
    fprintf(f, "</svg>\n</div>\n");

    /* pair data + hover tooltip */
    fprintf(f, "<script>\n(function(){\n");
    fprintf(f, "var CN=%d;\n", nm);
    fprintf(f, "var CR=[");
    for (int i = 0; i < nm * nm; i++)
      fprintf(f, "%s%.3f", i ? "," : "", corr[i]);
    fprintf(f, "];\nvar CBH=[");
    for (int i = 0; i < nm * nm; i++)
      fprintf(f, "%s%d", i ? "," : "", both_help[i]);
    fprintf(f, "];\nvar CBT=[");
    for (int i = 0; i < nm * nm; i++)
      fprintf(f, "%s%d", i ? "," : "", both_hurt[i]);
    fprintf(f, "];\n");
    fprintf(f, "%s",
      "var svg=document.getElementById('corr-svg');\n"
      "var tip=document.getElementById('corr-tip');\n"
      "function hide(){tip.style.display='none';}\n"
      "svg.addEventListener('mousemove',function(e){\n"
      "  var t=e.target;\n"
      "  if(t.tagName!=='rect'||t.getAttribute('data-a')===null){hide();return;}\n"
      "  var a=+t.getAttribute('data-a'), b=+t.getAttribute('data-b');\n"
      "  var r=CR[a*CN+b];\n"
      "  var la=(typeof MI!=='undefined'&&MI[a])?MI[a].mask+':'+MI[a].w:a;\n"
      "  var lb=(typeof MI!=='undefined'&&MI[b])?MI[b].mask+':'+MI[b].w:b;\n"
      "  var rClr=r>0.5?'#22d3ee':r<-0.5?'#f87171':'var(--fg)';\n"
      "  var h='<div class=\"tip-row\"><span style=\"color:var(--fg)\">'+la\n"
      "    +' \\u00d7 '+lb+'</span></div>'\n"
      "    +'<div style=\"border-top:1px solid var(--bdr);margin:4px 0 2px;'\n"
      "    +'padding-top:4px\"></div>'\n"
      "    +'<div class=\"tip-row\"><span style=\"color:var(--fg3)\">r</span>'\n"
      "    +'<span style=\"color:'+rClr+';font-weight:600\">'+r.toFixed(3)+'</span></div>'\n"
      "    +'<div class=\"tip-row\"><span style=\"color:var(--fg3)\">both help</span>'\n"
      "    +'<span>'+CBH[a*CN+b]+' bytes</span></div>'\n"
      "    +'<div class=\"tip-row\"><span style=\"color:var(--fg3)\">both hurt</span>'\n"
      "    +'<span>'+CBT[a*CN+b]+' bytes</span></div>';\n"
      "  tip.innerHTML=h;\n"
      "  var pr=svg.parentNode.getBoundingClientRect();\n"
      "  var tx=(e.clientX-pr.left)+14, ty=(e.clientY-pr.top)-50;\n"
      "  if(tx+180>pr.width) tx=(e.clientX-pr.left)-180;\n"
      "  if(ty<0) ty=(e.clientY-pr.top)+18;\n"
      "  tip.style.left=tx+'px'; tip.style.top=ty+'px';\n"
      "  tip.style.display='block';\n"
      "});\n"
      "svg.addEventListener('mouseleave',hide);\n"
      "})();</script>\n");

    fprintf(f, "</div>\n\n");
    free(cov);
    free(both_help);
    free(both_hurt);
    free(corr);
  }

  /* ── Top Surprises ── */
  if (s->byte_costs && s->num_data_bytes > 0 && s->num_models > 0) {
    int nb = s->num_data_bytes;
    int topn = 20;
    if (topn > nb) topn = nb;

    /* sort indices by cost descending */
    int *idx = (int *)malloc(nb * sizeof(int));
    for (int i = 0; i < nb; i++) idx[i] = i;
    /* simple selection sort for top N */
    for (int i = 0; i < topn; i++) {
      int best = i;
      for (int j = i + 1; j < nb; j++)
        if (s->byte_costs[idx[j]] > s->byte_costs[idx[best]]) best = j;
      if (best != i) { int t = idx[i]; idx[i] = idx[best]; idx[best] = t; }
    }

    fprintf(f,
      "<div class=\"card full\" id=\"sec-surprises\" style=\"position:relative\">\n"
      "<h2>Top Surprises</h2>\n"
      "<p class=\"desc\">The %d most expensive bytes to encode &mdash; "
      "where the compressor was most surprised. "
      "<span style=\"color:var(--fg3)\">Hover for a quick per-model "
      "preview; click a row to inspect in the Attribution Map.</span></p>\n"
      "<div id=\"ts-tip\" class=\"hover-tip\"></div>\n", topn);

    fprintf(f, "<table><thead><tr>"
      "<th>Rank</th><th class=\"r\">Offset</th>"
      "<th>Byte</th><th class=\"r\">Cost (bits)</th>"
      "<th>Context</th>"
      "<th>Dominant Model</th>"
      "</tr></thead><tbody>\n");

    for (int i = 0; i < topn; i++) {
      int bi = idx[i];
      float cost = s->byte_costs[bi];
      unsigned char bval = s->input_data ? s->input_data[bi] : 0;

      /* color */
      const char *clr = cost >= 8.0f ? "#f87171" : cost >= 5.0f ? "#fb923c" :
                         cost >= 3.0f ? "#fbbf24" : "#34d399";

      /* byte display */
      char bytebuf[80];
      if (bval >= 0x20 && bval <= 0x7E && bval != '<' && bval != '>'
          && bval != '&' && bval != '"')
        snprintf(bytebuf, sizeof(bytebuf), "0x%02X <span style=\"color:var(--fg)\">('%c')</span>", bval, bval);
      else
        snprintf(bytebuf, sizeof(bytebuf), "0x%02X", bval);

      /* context: show a few bytes before and after */
      char ctxbuf[128] = "";
      {
        int cstart = bi - 3;
        if (cstart < 0) cstart = 0;
        int cend = bi + 4;
        if (cend > s->input_size) cend = s->input_size;
        int pos = 0;
        for (int j = cstart; j < cend && pos < 100; j++) {
          unsigned char cv = s->input_data[j];
          if (j == bi)
            pos += snprintf(ctxbuf + pos, sizeof(ctxbuf) - pos,
              "<span style=\"color:var(--red);font-weight:700\">%02X</span> ", cv);
          else
            pos += snprintf(ctxbuf + pos, sizeof(ctxbuf) - pos, "%02X ", cv);
        }
      }

      /* dominant model for this byte */
      char modelbuf[64] = "—";
      if (s->byte_model_contrib) {
        int best_m = -1;
        float best_v = 0;
        for (int m = 0; m < s->num_models; m++) {
          float v = s->byte_model_contrib[bi * s->num_models + m];
          if (v > best_v) { best_v = v; best_m = m; }
        }
        if (best_m >= 0)
          snprintf(modelbuf, sizeof(modelbuf), "%02X:%d <span style=\"color:var(--fg3)\">(%.2f bits)</span>",
                   s->model_masks[best_m], s->model_weights[best_m], best_v);
      }

      fprintf(f,
        "<tr class=\"ts-row\" data-i=\"%d\" style=\"cursor:pointer\">"
        "<td class=\"r\">%d</td><td class=\"r\">%d</td>"
        "<td style=\"font-family:var(--mono);font-size:11.5px\">%s</td>"
        "<td class=\"r\" style=\"color:%s;font-weight:600\">%.2f</td>"
        "<td style=\"font-family:var(--mono);font-size:10px;color:var(--fg3)\">%s</td>"
        "<td style=\"font-family:var(--mono);font-size:11.5px\">%s</td>"
        "</tr>\n",
        bi, i + 1, bi, bytebuf, clr, cost, ctxbuf, modelbuf);
    }

    fprintf(f, "</tbody></table>\n");

    /* Wire rows to Attribution Map's selectByte (set up earlier as
       window.attrSelectByte). Falls back to no-op if attribution missing. */
    fprintf(f, "%s",
      "<script>(function(){\n"
      "var rows=document.querySelectorAll('#sec-surprises tr.ts-row');\n"
      "var tip=document.getElementById('ts-tip');\n"
      "var card=document.getElementById('sec-surprises');\n"
      "function hideTip(){tip.style.display='none';}\n"
      "for(var i=0;i<rows.length;i++){\n"
      "  rows[i].addEventListener('click',function(){\n"
      "    var i=this.getAttribute('data-i');\n"
      "    if(window.attrSelectByte) window.attrSelectByte(i);\n"
      "    var attr=document.getElementById('sec-attr');\n"
      "    if(attr) attr.scrollIntoView({behavior:'smooth',block:'start'});\n"
      "  });\n"
      "  rows[i].addEventListener('mousemove',function(e){\n"
      "    if(typeof BD==='undefined') return;\n"
      "    var idx=parseInt(this.getAttribute('data-i'));\n"
      "    var d=BD[idx]; if(!d) return;\n"
      "    var costClr=d.c<3?'#34d399':d.c<6?'#fbbf24':'#f87171';\n"
      "    var ch=d.ch?\" '\"+d.ch+\"'\":'';\n"
      "    var h='<div class=\"tip-row\"><span style=\"color:var(--fg3)\">byte</span>'\n"
      "      +'<span style=\"color:var(--fg)\">0x'+d.h+ch+' @ '+d.o+'</span></div>'\n"
      "      +'<div class=\"tip-row\"><span style=\"color:var(--fg3)\">cost</span>'\n"
      "      +'<span style=\"color:'+costClr+';font-weight:600\">'+d.c.toFixed(2)+' bits</span></div>';\n"
      "    if(d.m && typeof MI!=='undefined' && typeof ATTR_PAL!=='undefined'){\n"
      "      var ms=[];\n"
      "      for(var k=0;k<d.m.length;k++) ms.push({m:k,v:d.m[k]});\n"
      "      ms.sort(function(a,b){return Math.abs(b.v)-Math.abs(a.v);});\n"
      "      h += '<div style=\"border-top:1px solid var(--bdr);'\n"
      "        +'margin:4px 0 2px;padding-top:4px;color:var(--fg3);font-size:10px\">models</div>';\n"
      "      var show=Math.min(ms.length,4);\n"
      "      for(var k=0;k<show;k++){\n"
      "        var mm=ms[k]; if(Math.abs(mm.v)<0.01) break;\n"
      "        var pc=ATTR_PAL[mm.m]||[100,100,100];\n"
      "        var clr=mm.v<0?'#f87171':'var(--fg)';\n"
      "        var sign=mm.v>=0?'+':'';\n"
      "        h += '<div class=\"tip-row\">'\n"
      "          +'<span><span class=\"tip-sw\" style=\"background:rgb('+pc[0]+','+pc[1]+','+pc[2]+')\"></span>'\n"
      "          +(MI[mm.m]?MI[mm.m].mask+':'+MI[mm.m].w:mm.m)+'</span>'\n"
      "          +'<span style=\"color:'+clr+'\">'+sign+mm.v.toFixed(2)+'</span></div>';\n"
      "      }\n"
      "    }\n"
      "    /* cost window sparkline (\\u00b132 bytes around this surprise) */\n"
      "    if(window.makeSparkline && typeof BD!=='undefined'){\n"
      "      var lo=Math.max(0,idx-32), hi2=Math.min(BD.length-1,idx+32);\n"
      "      var vals=[];\n"
      "      for(var k=lo;k<=hi2;k++) vals.push(BD[k]?BD[k].c:0);\n"
      "      var spark=window.makeSparkline(vals,{\n"
      "        color:'#22d3ee', highlightIdx:idx-lo,\n"
      "        labelTop:'cost', labelRight:lo+'\\u2026'+hi2\n"
      "      });\n"
      "      if(spark) h += '<div style=\"color:var(--fg3);font-size:10px;'\n"
      "        +'margin-top:6px\">context cost</div>'+spark;\n"
      "    }\n"
      "    tip.innerHTML=h;\n"
      "    var cr=card.getBoundingClientRect();\n"
      "    var tx=(e.clientX-cr.left)+14, ty=(e.clientY-cr.top)+14;\n"
      "    if(tx+220>cr.width) tx=(e.clientX-cr.left)-220;\n"
      "    tip.style.left=tx+'px'; tip.style.top=ty+'px';\n"
      "    tip.style.display='block';\n"
      "  });\n"
      "  rows[i].addEventListener('mouseleave',hideTip);\n"
      "}\n"
      "})();</script>\n");

    fprintf(f, "</div>\n\n");
    free(idx);
  }

  /* ── Cost Over File Position ── */
  if (s->byte_costs && s->num_data_bytes > 1) {
    int nb = s->num_data_bytes;
    int win = nb / 64;
    if (win < 4) win = 4;
    if (win > 64) win = 64;
    int npts = nb - win + 1;
    if (npts < 2) npts = 2;
    float *rolling = (float *)malloc(npts * sizeof(float));
    float rmin = 1e9f, rmax = -1e9f;
    for (int i = 0; i < npts; i++) {
      float sum = 0;
      int end = i + win;
      if (end > nb) end = nb;
      for (int j = i; j < end; j++) sum += s->byte_costs[j];
      rolling[i] = sum / (end - i);
      if (rolling[i] < rmin) rmin = rolling[i];
      if (rolling[i] > rmax) rmax = rolling[i];
    }
    if (rmax <= rmin) rmax = rmin + 1;

    int svg_w = 960, svg_h = 160;
    int pad_l = 44, pad_r = 12, pad_t = 12, pad_b = 28;
    int plot_w = svg_w - pad_l - pad_r;
    int plot_h = svg_h - pad_t - pad_b;

    fprintf(f,
      "<div class=\"card full\" id=\"sec-cost\">\n"
      "<h2>Cost Over File Position</h2>\n"
      "<p class=\"desc\">Rolling avg encoding cost (bits/byte). "
      "Window = %d bytes. H\xe2\x82\x80 = %.2f.</p>\n", win, s->entropy);
    fprintf(f, "<div class=\"scrub-wrap\">\n");
    fprintf(f,
      "<svg id=\"cost-svg\" width=\"100%%\" viewBox=\"0 0 %d %d\" "
      "style=\"display:block\">\n", svg_w, svg_h);

    /* bg */
    fprintf(f,
      "<rect x=\"%d\" y=\"%d\" width=\"%d\" height=\"%d\" "
      "fill=\"var(--bg3)\" rx=\"4\"/>\n", pad_l, pad_t, plot_w, plot_h);

    /* y gridlines */
    int nyticks = 4;
    for (int i = 0; i <= nyticks; i++) {
      float val = rmin + (rmax - rmin) * (nyticks - i) / nyticks;
      int y = pad_t + plot_h * i / nyticks;
      fprintf(f,
        "<line x1=\"%d\" y1=\"%d\" x2=\"%d\" y2=\"%d\" "
        "stroke=\"var(--bdr)\" stroke-width=\"0.5\"/>\n",
        pad_l, y, pad_l + plot_w, y);
      fprintf(f,
        "<text x=\"%d\" y=\"%d\" text-anchor=\"end\" "
        "font-size=\"9\" fill=\"var(--fg3)\">%.1f</text>\n",
        pad_l - 5, y + 3, val);
    }

    /* H0 line */
    if (s->entropy >= rmin && s->entropy <= rmax) {
      int h0y = pad_t + (int)(plot_h * (rmax - s->entropy) / (rmax - rmin));
      fprintf(f,
        "<line x1=\"%d\" y1=\"%d\" x2=\"%d\" y2=\"%d\" "
        "stroke=\"#f87171\" stroke-width=\"1\" stroke-dasharray=\"5,3\" "
        "opacity=\".6\"/>\n", pad_l, h0y, pad_l + plot_w, h0y);
      fprintf(f,
        "<text x=\"%d\" y=\"%d\" font-size=\"9\" fill=\"#f87171\" "
        "opacity=\".8\">H\xe2\x82\x80</text>\n",
        pad_l + plot_w + 3, h0y + 3);
    }
    /* 8.0 line */
    if (8.0 >= rmin && 8.0 <= rmax) {
      int y8 = pad_t + (int)(plot_h * (rmax - 8.0) / (rmax - rmin));
      fprintf(f,
        "<line x1=\"%d\" y1=\"%d\" x2=\"%d\" y2=\"%d\" "
        "stroke=\"var(--fg3)\" stroke-width=\"0.5\" stroke-dasharray=\"3,3\" "
        "opacity=\".4\"/>\n", pad_l, y8, pad_l + plot_w, y8);
    }

    /* area fill with gradient */
    fprintf(f,
      "<defs><linearGradient id=\"cfill\" x1=\"0\" y1=\"0\" x2=\"0\" y2=\"1\">"
      "<stop offset=\"0\" stop-color=\"#22d3ee\" stop-opacity=\".2\"/>"
      "<stop offset=\"1\" stop-color=\"#22d3ee\" stop-opacity=\".02\"/>"
      "</linearGradient></defs>\n");
    fprintf(f, "<path d=\"M%d,%d ", pad_l, pad_t + plot_h);
    for (int i = 0; i < npts; i++) {
      int x = pad_l + (int)((long)i * plot_w / (npts - 1));
      int y = pad_t + (int)(plot_h * (rmax - rolling[i]) / (rmax - rmin));
      fprintf(f, "L%d,%d ", x, y);
    }
    fprintf(f, "L%d,%d Z\" fill=\"url(#cfill)\"/>\n",
      pad_l + plot_w, pad_t + plot_h);

    /* line */
    fprintf(f, "<path d=\"");
    for (int i = 0; i < npts; i++) {
      int x = pad_l + (int)((long)i * plot_w / (npts - 1));
      int y = pad_t + (int)(plot_h * (rmax - rolling[i]) / (rmax - rmin));
      fprintf(f, "%c%d,%d ", i == 0 ? 'M' : 'L', x, y);
    }
    fprintf(f,
      "\" fill=\"none\" stroke=\"#22d3ee\" stroke-width=\"1.5\" "
      "stroke-linejoin=\"round\"/>\n");

    /* x-axis */
    int nxticks = 5;
    for (int i = 0; i <= nxticks; i++) {
      int x = pad_l + plot_w * i / nxticks;
      int byte_pos = (int)((long)i * (nb - 1) / nxticks);
      fprintf(f,
        "<text x=\"%d\" y=\"%d\" text-anchor=\"middle\" "
        "font-size=\"9\" fill=\"var(--fg3)\">%d</text>\n",
        x, svg_h - 4, byte_pos);
    }
    /* scrubber line, hidden until hover */
    fprintf(f,
      "<line id=\"cost-scrub\" class=\"scrub-line\" "
      "x1=\"0\" y1=\"%d\" x2=\"0\" y2=\"%d\"/>\n",
      pad_t, pad_t + plot_h);
    fprintf(f, "</svg>\n");
    fprintf(f, "<div id=\"cost-tip\" class=\"hover-tip\"></div>\n");

    /* embed rolling data + scrubber JS */
    fprintf(f, "<script>(function(){\n");
    fprintf(f, "var R=[");
    for (int i = 0; i < npts; i++)
      fprintf(f, "%s%.3g", i ? "," : "", rolling[i]);
    fprintf(f, "];\n");
    fprintf(f, "var nb=%d,npts=%d,W=%d,PL=%d,pw=%d;\n",
      nb, npts, svg_w, pad_l, plot_w);

    fprintf(f, "%s",
      "var svg=document.getElementById('cost-svg');\n"
      "var scrub=document.getElementById('cost-scrub');\n"
      "var tip=document.getElementById('cost-tip');\n"
      "var wrap=svg.parentNode;\n"
      "function hide(){scrub.setAttribute('opacity','0');tip.style.display='none';}\n"
      "svg.addEventListener('mousemove',function(e){\n"
      "  var r=svg.getBoundingClientRect();\n"
      "  var px=(e.clientX-r.left)/r.width*W;\n"
      "  if(px<PL||px>PL+pw){hide();return;}\n"
      "  var idx=Math.round((px-PL)/pw*(npts-1));\n"
      "  if(idx<0)idx=0; if(idx>=npts)idx=npts-1;\n"
      "  var bytePos=Math.round(idx*(nb-1)/(npts-1));\n"
      "  var val=R[idx];\n"
      "  var x=PL+idx*pw/(npts-1);\n"
      "  scrub.setAttribute('x1',x); scrub.setAttribute('x2',x);\n"
      "  scrub.setAttribute('opacity','0.5');\n"
      "  var clr=val<3?'#34d399':val<6?'#fbbf24':'#f87171';\n"
      "  tip.innerHTML='<div class=\"tip-row\"><span style=\"color:var(--fg3)\">byte</span>'\n"
      "    +'<span style=\"color:var(--fg)\">~'+bytePos+'</span></div>'\n"
      "    +'<div class=\"tip-row\"><span style=\"color:var(--fg3)\">cost</span>'\n"
      "    +'<span style=\"color:'+clr+';font-weight:600\">'+val.toFixed(2)+' bits</span></div>';\n"
      "  var wr=wrap.getBoundingClientRect();\n"
      "  var tx=(e.clientX-wr.left)+12, ty=(e.clientY-wr.top)-40;\n"
      "  if(tx+200>wr.width) tx=(e.clientX-wr.left)-200;\n"
      "  if(ty<0) ty=(e.clientY-wr.top)+16;\n"
      "  tip.style.left=tx+'px'; tip.style.top=ty+'px';\n"
      "  tip.style.display='block';\n"
      "});\n"
      "svg.addEventListener('mouseleave',hide);\n"
      "})();</script>\n");

    fprintf(f, "</div></div>\n\n");
    free(rolling);
  }

  /* ── Cumulative Cost ── */
  if (s->byte_costs && s->num_data_bytes > 1) {
    fprintf(f, "%s",
      "<div class=\"card full\" id=\"sec-cumcost\">\n"
      "<h2>Cumulative Cost</h2>\n"
      "<p class=\"desc\">Running compressed size over input position: "
      "<span style=\"color:#22d3ee\">actual</span>, "
      "<span style=\"color:var(--fg3)\">average slope</span>, "
      "<span style=\"color:#f87171\">order-0 (H\xe2\x82\x80)</span>. "
      "Steep spans are where the budget goes.</p>\n"
      "<div class=\"scrub-wrap\">\n"
      "<div id=\"cc-chart\"></div>\n"
      "<div id=\"cc-tip\" class=\"hover-tip\"></div>\n"
      "</div>\n");

    fprintf(f, "<script>(function(){\n");
    fprintf(f, "var H0=%.4f;\n", s->entropy);
    fprintf(f, "%s",
      "if(typeof BD==='undefined'||BD.length<2) return;\n"
      "var n=BD.length;\n"
      "var cum=new Float64Array(n), t=0;\n"
      "for(var i=0;i<n;i++){t+=BD[i].c;cum[i]=t;}\n"
      "var totalB=t/8, h0B=H0*n/8;\n"
      "var maxY=Math.max(totalB,h0B)*1.02;\n"
      "if(maxY<=0) return;\n"
      "var W=960,H=200,PL=48,PR=12,PT=12,PB=28;\n"
      "var pw=W-PL-PR,ph=H-PT-PB;\n"
      "function X(i){return PL+i*pw/(n-1);}\n"
      "function Y(b){return PT+ph-b/maxY*ph;}\n"
      "var s='<svg width=\"100%\" viewBox=\"0 0 '+W+' '+H+'\" style=\"display:block\">';\n"
      "s+='<rect x=\"'+PL+'\" y=\"'+PT+'\" width=\"'+pw+'\" height=\"'+ph\n"
      "  +'\" fill=\"var(--bg3)\" rx=\"4\"/>';\n"
      "for(var i=0;i<=4;i++){\n"
      "  var val=maxY*(4-i)/4, y=(PT+ph*i/4)|0;\n"
      "  s+='<line x1=\"'+PL+'\" y1=\"'+y+'\" x2=\"'+(PL+pw)+'\" y2=\"'+y\n"
      "    +'\" stroke=\"var(--bdr)\" stroke-width=\"0.5\"/>'\n"
      "    +'<text x=\"'+(PL-5)+'\" y=\"'+(y+3)+'\" text-anchor=\"end\" '\n"
      "    +'font-size=\"9\" fill=\"var(--fg3)\">'+val.toFixed(0)+' B</text>';\n"
      "}\n"
      "for(var i=0;i<=5;i++){\n"
      "  var off=(((n-1)*i/5)|0), x=(PL+(pw*i/5))|0;\n"
      "  s+='<text x=\"'+x+'\" y=\"'+(PT+ph+16)+'\" text-anchor=\"middle\" '\n"
      "    +'font-size=\"9\" fill=\"var(--fg3)\">'+off+'</text>';\n"
      "}\n"
      "/* H0 reference (straight by construction) and average chord */\n"
      "s+='<line x1=\"'+X(0).toFixed(1)+'\" y1=\"'+Y(H0/8).toFixed(1)\n"
      "  +'\" x2=\"'+X(n-1).toFixed(1)+'\" y2=\"'+Y(h0B).toFixed(1)\n"
      "  +'\" stroke=\"#f87171\" stroke-width=\"1\" stroke-dasharray=\"5,3\" opacity=\".6\"/>';\n"
      "s+='<line x1=\"'+X(0).toFixed(1)+'\" y1=\"'+Y(0).toFixed(1)\n"
      "  +'\" x2=\"'+X(n-1).toFixed(1)+'\" y2=\"'+Y(totalB).toFixed(1)\n"
      "  +'\" stroke=\"var(--fg3)\" stroke-width=\"1\" stroke-dasharray=\"3,3\" opacity=\".5\"/>';\n"
      "var step=Math.max(1,Math.floor(n/960));\n"
      "var d='M'+X(0).toFixed(1)+','+Y(cum[0]/8).toFixed(1);\n"
      "for(var i=step;i<n;i+=step)\n"
      "  d+=' L'+X(i).toFixed(1)+','+Y(cum[i]/8).toFixed(1);\n"
      "d+=' L'+X(n-1).toFixed(1)+','+Y(totalB).toFixed(1);\n"
      "s+='<path d=\"'+d+' L'+X(n-1).toFixed(1)+','+(PT+ph)+' L'+X(0).toFixed(1)\n"
      "  +','+(PT+ph)+' Z\" fill=\"#22d3ee\" fill-opacity=\".08\"/>';\n"
      "s+='<path d=\"'+d+'\" fill=\"none\" stroke=\"#22d3ee\" stroke-width=\"1.5\" '\n"
      "  +'stroke-linejoin=\"round\"/>';\n"
      "s+='<line id=\"cc-scrub\" class=\"scrub-line\" x1=\"0\" y1=\"'+PT\n"
      "  +'\" x2=\"0\" y2=\"'+(PT+ph)+'\"/>';\n"
      "s+='</svg>';\n"
      "var chart=document.getElementById('cc-chart');\n"
      "chart.innerHTML=s;\n"
      "var tip=document.getElementById('cc-tip');\n"
      "function hide(){\n"
      "  var l=document.getElementById('cc-scrub');\n"
      "  if(l) l.setAttribute('opacity','0');\n"
      "  tip.style.display='none';\n"
      "}\n"
      "chart.addEventListener('mousemove',function(e){\n"
      "  var svg=chart.querySelector('svg'); if(!svg) return;\n"
      "  var r=svg.getBoundingClientRect();\n"
      "  var px=(e.clientX-r.left)/r.width*W;\n"
      "  if(px<PL||px>PL+pw){hide();return;}\n"
      "  var i=Math.round((px-PL)/pw*(n-1));\n"
      "  if(i<0)i=0; if(i>=n)i=n-1;\n"
      "  var cb=cum[i]/8;\n"
      "  var avgB=totalB*i/(n-1);\n"
      "  var h0i=H0*(i+1)/8;\n"
      "  var dAvg=cb-avgB, dH0=cb-h0i;\n"
      "  var l=document.getElementById('cc-scrub');\n"
      "  if(l){\n"
      "    var x=PL+i*pw/(n-1);\n"
      "    l.setAttribute('x1',x); l.setAttribute('x2',x);\n"
      "    l.setAttribute('opacity','0.5');\n"
      "  }\n"
      "  tip.innerHTML='<div class=\"tip-row\"><span style=\"color:var(--fg3)\">byte</span>'\n"
      "    +'<span style=\"color:var(--fg)\">'+i+'</span></div>'\n"
      "    +'<div class=\"tip-row\"><span style=\"color:var(--fg3)\">so far</span>'\n"
      "    +'<span style=\"color:#22d3ee;font-weight:600\">'+cb.toFixed(1)+' B ('\n"
      "    +(100*cb/totalB).toFixed(1)+'%)</span></div>'\n"
      "    +'<div class=\"tip-row\"><span style=\"color:var(--fg3)\">vs avg</span>'\n"
      "    +'<span style=\"color:'+(dAvg<0?'#34d399':'#f87171')+'\">'\n"
      "    +(dAvg>=0?'+':'')+dAvg.toFixed(1)+' B</span></div>'\n"
      "    +'<div class=\"tip-row\"><span style=\"color:var(--fg3)\">vs H\\u2080</span>'\n"
      "    +'<span style=\"color:'+(dH0<0?'#34d399':'#f87171')+'\">'\n"
      "    +(dH0>=0?'+':'')+dH0.toFixed(1)+' B</span></div>';\n"
      "  var wr=chart.parentNode.getBoundingClientRect();\n"
      "  var tx=(e.clientX-wr.left)+12, ty=(e.clientY-wr.top)-40;\n"
      "  if(tx+200>wr.width) tx=(e.clientX-wr.left)-200;\n"
      "  if(ty<0) ty=(e.clientY-wr.top)+16;\n"
      "  tip.style.left=tx+'px'; tip.style.top=ty+'px';\n"
      "  tip.style.display='block';\n"
      "});\n"
      "chart.addEventListener('mouseleave',hide);\n"
      "})();</script>\n");

    fprintf(f, "</div>\n\n");
  }

  /* ── Bit-Position Heatmap ── */
  if (s->bit_costs && s->num_data_bytes > 1) {
    int nb = s->num_data_bytes;
    fprintf(f, "%s",
      "<div class=\"card full\" id=\"sec-bitmap\" style=\"position:relative\">\n"
      "<h2>Bit-Position Heatmap</h2>\n"
      "<p class=\"desc\">Cost of each bit position over file position: "
      "x = byte offset, y = bit (MSB top). The Byte Position card shows the "
      "file-wide average; this shows where each bit is cheap or "
      "expensive.</p>\n"
      "<div style=\"display:flex;gap:6px;align-items:stretch\">\n"
      "<div style=\"width:26px;flex-shrink:0;display:flex;"
      "flex-direction:column;font-family:var(--mono);font-size:9px;"
      "color:var(--fg3)\">"
      "<div style=\"flex:1;display:flex;align-items:center;"
      "justify-content:flex-end\">b7</div>"
      "<div style=\"flex:1\"></div><div style=\"flex:1\"></div>"
      "<div style=\"flex:1\"></div><div style=\"flex:1\"></div>"
      "<div style=\"flex:1\"></div><div style=\"flex:1\"></div>"
      "<div style=\"flex:1;display:flex;align-items:center;"
      "justify-content:flex-end\">b0</div>"
      "</div>\n"
      "<canvas id=\"bh-cv\" style=\"flex:1;min-width:0;height:96px;"
      "image-rendering:pixelated;background:var(--bg3);"
      "border:1px solid var(--bdr);cursor:crosshair\"></canvas>\n"
      "</div>\n"
      "<div id=\"bh-tip\" class=\"hover-tip\"></div>\n");

    fprintf(f, "<script>(function(){\n");
    fprintf(f, "var nb=%d;\n", nb);
    fprintf(f, "var BB=[");
    for (int i = 0; i < nb * 8; i++)
      fprintf(f, "%s%.3g", i ? "," : "", s->bit_costs[i]);
    fprintf(f, "];\n");
    fprintf(f, "%s",
      "var cv=document.getElementById('bh-cv');\n"
      "var tip=document.getElementById('bh-tip');\n"
      "var card=document.getElementById('sec-bitmap');\n"
      "var agg=Math.ceil(nb/2048);\n"
      "var cw=Math.ceil(nb/agg);\n"
      "cv.width=cw; cv.height=8;\n"
      "var ctx=cv.getContext('2d');\n"
      "var img=ctx.createImageData(cw,8);\n"
      "var cmax=0;\n"
      "for(var i=0;i<BB.length;i++) if(BB[i]>cmax) cmax=BB[i];\n"
      "if(cmax<=0) cmax=1;\n"
      "function cellAvg(x,b){\n"
      "  var sum=0,c=0;\n"
      "  for(var k=x*agg;k<Math.min((x+1)*agg,nb);k++){sum+=BB[k*8+b];c++;}\n"
      "  return c?sum/c:0;\n"
      "}\n"
      "for(var x=0;x<cw;x++){\n"
      "  for(var b=0;b<8;b++){\n"
      "    var t=cellAvg(x,b)/cmax; if(t>1)t=1;\n"
      "    var r,g,bl;\n"
      "    if(t<0.5){var u=t*2;r=16+(180-16)*u;g=185+(140-185)*u;bl=129+(40-129)*u;}\n"
      "    else{var u=(t-0.5)*2;r=180+(248-180)*u;g=140+(113-140)*u;bl=40+(113-40)*u;}\n"
      "    var px=(b*cw+x)*4;\n"
      "    img.data[px]=r|0;img.data[px+1]=g|0;img.data[px+2]=bl|0;img.data[px+3]=255;\n"
      "  }\n"
      "}\n"
      "ctx.putImageData(img,0,0);\n"
      "cv.addEventListener('mousemove',function(e){\n"
      "  var r=cv.getBoundingClientRect();\n"
      "  var x=Math.floor((e.clientX-r.left)/r.width*cw);\n"
      "  var b=Math.floor((e.clientY-r.top)/r.height*8);\n"
      "  if(x<0)x=0; if(x>=cw)x=cw-1;\n"
      "  if(b<0)b=0; if(b>7)b=7;\n"
      "  var lo=x*agg, hi=Math.min((x+1)*agg,nb)-1;\n"
      "  var v=cellAvg(x,b);\n"
      "  var clr=v<3?'#34d399':v<6?'#fbbf24':'#f87171';\n"
      "  var h='<div class=\"tip-row\"><span style=\"color:var(--fg3)\">byte</span>'\n"
      "    +'<span style=\"color:var(--fg)\">'+(lo===hi?lo:lo+'\\u2013'+hi)+'</span></div>'\n"
      "    +'<div class=\"tip-row\"><span style=\"color:var(--fg3)\">bit</span>'\n"
      "    +'<span>'+(7-b)+(b===0?' (MSB)':b===7?' (LSB)':'')+'</span></div>'\n"
      "    +'<div class=\"tip-row\"><span style=\"color:var(--fg3)\">'\n"
      "    +(lo===hi?'cost':'avg cost')+'</span>'\n"
      "    +'<span style=\"color:'+clr+';font-weight:600\">'+v.toFixed(2)+' bits</span></div>';\n"
      "  if(lo===hi&&typeof BD!=='undefined'&&BD[lo]){\n"
      "    var d=BD[lo];\n"
      "    var ch=d.ch?\" '\"+(d.ch==='<'?'&lt;':d.ch==='>'?'&gt;':d.ch==='&'?'&amp;':d.ch)+\"'\":'';\n"
      "    h+='<div class=\"tip-row\"><span style=\"color:var(--fg3)\">value</span>'\n"
      "      +'<span>0x'+d.h+ch+'</span></div>';\n"
      "  }\n"
      "  tip.innerHTML=h;\n"
      "  var cr=card.getBoundingClientRect();\n"
      "  var tx=(e.clientX-cr.left)+14, ty=(e.clientY-cr.top)-60;\n"
      "  if(tx+180>cr.width) tx=(e.clientX-cr.left)-180;\n"
      "  if(ty<0) ty=(e.clientY-cr.top)+18;\n"
      "  tip.style.left=tx+'px'; tip.style.top=ty+'px';\n"
      "  tip.style.display='block';\n"
      "});\n"
      "cv.addEventListener('mouseleave',function(){tip.style.display='none';});\n"
      "})();</script>\n");

    fprintf(f, "</div>\n\n");
  }

  /* ── Repetition Structure ── */
  if (s->input_data && s->input_size > 4) {
    int n = s->input_size;
    const unsigned char *d = s->input_data;

    /* longest backward match per position (LZ77-style): hash chains over
       4-byte prefixes, capped depth/length so degenerate inputs stay fast */
    enum { REP_MAXM = 4096, REP_DEPTH = 64 };
    int *mlen = (int *)calloc(n, sizeof(int));
    int *mdist = (int *)calloc(n, sizeof(int));
    int hbits = 12;
    while ((1 << hbits) < n * 2 && hbits < 20)
      hbits++;
    int hsize = 1 << hbits;
    int *head = (int *)malloc(hsize * sizeof(int));
    int *prevp = (int *)malloc(n * sizeof(int));
    memset(head, -1, hsize * sizeof(int));

    for (int i = 0; i + 4 <= n; i++) {
      unsigned int key;
      memcpy(&key, d + i, 4);
      unsigned int h = (key * 2654435761u) >> (32 - hbits);
      int best = 0, bestj = -1;
      int maxl = n - i;
      if (maxl > REP_MAXM)
        maxl = REP_MAXM;
      int depth = 0;
      for (int j = head[h]; j >= 0 && depth < REP_DEPTH;
           j = prevp[j], depth++) {
        if (d[j + best] != d[i + best])
          continue;
        int l = 0;
        while (l < maxl && d[j + l] == d[i + l])
          l++;
        if (l > best) {
          best = l;
          bestj = j;
          if (best >= maxl)
            break;
        }
      }
      if (best >= 4) {
        mlen[i] = best;
        mdist[i] = i - bestj;
      }
      prevp[i] = head[h];
      head[h] = i;
    }
    free(head);
    free(prevp);

    int matched = 0, max_len = 0;
    double len_sum = 0;
    for (int i = 0; i < n; i++) {
      if (mlen[i] >= 4) {
        matched++;
        len_sum += mlen[i];
        if (mlen[i] > max_len)
          max_len = mlen[i];
      }
    }

    if (matched > 0) {
      fprintf(f,
        "<div class=\"card full\" id=\"sec-repeat\">\n"
        "<h2>Repetition Structure</h2>\n"
        "<p class=\"desc\">Longest backward match at each position "
        "(<span style=\"color:#a78bfa\">purple area</span>, log scale, "
        "&ge;4 bytes) with rolling cost overlaid "
        "(<span style=\"color:#22d3ee\">cyan</span>, right axis). "
        "%.1f%% of positions start a match; mean %.0f, max %d bytes. "
        "Long matches should be cheap &mdash; expensive repeated regions "
        "mean the models miss the redundancy. "
        "<span style=\"color:var(--fg3)\">Click to jump to the match "
        "source in the Hex View.</span></p>\n",
        100.0 * matched / n, len_sum / matched, max_len);
      fprintf(f, "<div class=\"scrub-wrap\">\n");
      fprintf(f, "<div id=\"rep-chart\"></div>\n");
      fprintf(f, "<div id=\"rep-tip\" class=\"hover-tip\"></div>\n");
      fprintf(f, "</div>\n");

      fprintf(f, "<script>(function(){\n");
      fprintf(f, "var ML=[");
      for (int i = 0; i < n; i++)
        fprintf(f, "%s%d", i ? "," : "", mlen[i]);
      fprintf(f, "];\nvar MD=[");
      for (int i = 0; i < n; i++)
        fprintf(f, "%s%d", i ? "," : "", mdist[i]);
      fprintf(f, "];\n");
      fprintf(f, "%s",
        "var n=ML.length; if(n<2) return;\n"
        "var maxL=0;\n"
        "for(var i=0;i<n;i++) if(ML[i]>maxL) maxL=ML[i];\n"
        "var lden=Math.log2(1+maxL); if(lden<=0) lden=1;\n"
        "var W=960,H=220,PL=48,PR=48,PT=12,PB=28;\n"
        "var pw=W-PL-PR,ph=H-PT-PB;\n"
        "function X(i){return PL+i*pw/(n-1);}\n"
        "function yL(l){return PT+ph-(Math.log2(1+l)/lden)*ph;}\n"
        "/* rolling cost overlay from BD */\n"
        "var haveC=(typeof BD!=='undefined')&&BD.length>=2;\n"
        "var R=null,cmax=0,cn=0;\n"
        "if(haveC){\n"
        "  cn=Math.min(n,BD.length);\n"
        "  var win=Math.max(4,Math.min(64,cn>>6));\n"
        "  var pre=new Float64Array(cn+1);\n"
        "  for(var i=0;i<cn;i++) pre[i+1]=pre[i]+BD[i].c;\n"
        "  R=new Float32Array(cn);\n"
        "  for(var i=0;i<cn;i++){\n"
        "    var a=Math.max(0,i-(win>>1)), b=Math.min(cn,a+win);\n"
        "    R[i]=(pre[b]-pre[a])/(b-a);\n"
        "    if(R[i]>cmax) cmax=R[i];\n"
        "  }\n"
        "  if(cmax<=0) haveC=false;\n"
        "}\n"
        "function yC(v){return PT+ph-(v/cmax)*ph;}\n"
        "var s='<svg width=\"100%\" viewBox=\"0 0 '+W+' '+H+'\" style=\"display:block\">';\n"
        "s+='<rect x=\"'+PL+'\" y=\"'+PT+'\" width=\"'+pw+'\" height=\"'+ph\n"
        "  +'\" fill=\"var(--bg3)\" rx=\"4\"/>';\n"
        "/* left axis: match length, powers of 4 */\n"
        "var ticks=[4,16,64,256,1024,4096];\n"
        "for(var k=0;k<ticks.length;k++){\n"
        "  var t=ticks[k]; if(t>maxL) break;\n"
        "  var y=yL(t).toFixed(1);\n"
        "  s+='<line x1=\"'+PL+'\" y1=\"'+y+'\" x2=\"'+(PL+pw)+'\" y2=\"'+y\n"
        "    +'\" stroke=\"var(--bdr)\" stroke-width=\"0.5\"/>'\n"
        "    +'<text x=\"'+(PL-4)+'\" y=\"'+(+y+3)+'\" text-anchor=\"end\" '\n"
        "    +'font-size=\"9\" fill=\"#a78bfa\">'+t+'</text>';\n"
        "}\n"
        "/* right axis: cost */\n"
        "if(haveC){\n"
        "  for(var k=1;k<=3;k++){\n"
        "    var v=cmax*k/3, y=yC(v).toFixed(1);\n"
        "    s+='<text x=\"'+(PL+pw+4)+'\" y=\"'+(+y+3)+'\" '\n"
        "      +'text-anchor=\"start\" font-size=\"9\" fill=\"#22d3ee\">'\n"
        "      +v.toFixed(1)+'</text>';\n"
        "  }\n"
        "}\n"
        "/* x labels */\n"
        "for(var k=0;k<=5;k++){\n"
        "  var off=(((n-1)*k/5)|0), x=(PL+(pw*k/5))|0;\n"
        "  s+='<text x=\"'+x+'\" y=\"'+(PT+ph+16)+'\" text-anchor=\"middle\" '\n"
        "    +'font-size=\"9\" fill=\"var(--fg3)\">'+off+'</text>';\n"
        "}\n"
        "/* match-length area: per-slot maxima so spikes survive downsampling */\n"
        "var slots=Math.min(n,960);\n"
        "var per=n/slots;\n"
        "var d2='M'+PL+','+(PT+ph);\n"
        "for(var s2=0;s2<slots;s2++){\n"
        "  var lo=Math.floor(s2*per), hi=Math.min(n,Math.ceil((s2+1)*per));\n"
        "  var mx=0;\n"
        "  for(var k=lo;k<hi;k++) if(ML[k]>mx) mx=ML[k];\n"
        "  var x=PL+(s2+0.5)*pw/slots;\n"
        "  d2+=' L'+x.toFixed(1)+','+yL(mx).toFixed(1);\n"
        "}\n"
        "d2+=' L'+(PL+pw)+','+(PT+ph)+' Z';\n"
        "s+='<path d=\"'+d2+'\" fill=\"#a78bfa\" fill-opacity=\".45\" '\n"
        "  +'stroke=\"#a78bfa\" stroke-width=\"0.5\"/>';\n"
        "if(haveC){\n"
        "  var dc='';\n"
        "  for(var s2=0;s2<slots;s2++){\n"
        "    var i=Math.min(cn-1,Math.floor((s2+0.5)*per));\n"
        "    dc+=(s2?'L':'M')+(PL+(s2+0.5)*pw/slots).toFixed(1)+','\n"
        "      +yC(R[i]).toFixed(1)+' ';\n"
        "  }\n"
        "  s+='<path d=\"'+dc+'\" fill=\"none\" stroke=\"#22d3ee\" '\n"
        "    +'stroke-width=\"1.2\"/>';\n"
        "}\n"
        "s+='<line id=\"rep-scrub\" class=\"scrub-line\" x1=\"0\" y1=\"'+PT\n"
        "  +'\" x2=\"0\" y2=\"'+(PT+ph)+'\"/>';\n"
        "s+='</svg>';\n"
        "var chart=document.getElementById('rep-chart');\n"
        "chart.innerHTML=s;\n"
        "var tip=document.getElementById('rep-tip');\n"
        "function hide(){\n"
        "  var l=document.getElementById('rep-scrub');\n"
        "  if(l) l.setAttribute('opacity','0');\n"
        "  tip.style.display='none';\n"
        "}\n"
        "function idxFromEvent(e){\n"
        "  var svg=chart.querySelector('svg'); if(!svg) return -1;\n"
        "  var r=svg.getBoundingClientRect();\n"
        "  var px=(e.clientX-r.left)/r.width*W;\n"
        "  if(px<PL||px>PL+pw) return -1;\n"
        "  var i=Math.round((px-PL)/pw*(n-1));\n"
        "  if(i<0)i=0; if(i>=n)i=n-1;\n"
        "  return i;\n"
        "}\n"
        "chart.addEventListener('mousemove',function(e){\n"
        "  var i=idxFromEvent(e);\n"
        "  if(i<0){hide();return;}\n"
        "  var l=document.getElementById('rep-scrub');\n"
        "  if(l){\n"
        "    var x=PL+i*pw/(n-1);\n"
        "    l.setAttribute('x1',x); l.setAttribute('x2',x);\n"
        "    l.setAttribute('opacity','0.5');\n"
        "  }\n"
        "  var h='<div class=\"tip-row\"><span style=\"color:var(--fg3)\">byte</span>'\n"
        "    +'<span style=\"color:var(--fg)\">'+i+'</span></div>';\n"
        "  if(ML[i]>=4){\n"
        "    h+='<div class=\"tip-row\"><span style=\"color:var(--fg3)\">match</span>'\n"
        "      +'<span style=\"color:#a78bfa;font-weight:600\">'+ML[i]+' B</span></div>'\n"
        "      +'<div class=\"tip-row\"><span style=\"color:var(--fg3)\">source</span>'\n"
        "      +'<span>'+(i-MD[i])+' (dist '+MD[i]+')</span></div>';\n"
        "  } else {\n"
        "    h+='<div class=\"tip-row\"><span style=\"color:var(--fg3)\">match</span>'\n"
        "      +'<span>none \\u22654</span></div>';\n"
        "  }\n"
        "  if(haveC&&i<cn){\n"
        "    var c=BD[i].c;\n"
        "    var clr=c<3?'#34d399':c<6?'#fbbf24':'#f87171';\n"
        "    h+='<div class=\"tip-row\"><span style=\"color:var(--fg3)\">cost</span>'\n"
        "      +'<span style=\"color:'+clr+';font-weight:600\">'+c.toFixed(2)\n"
        "      +' bits</span></div>'\n"
        "      +'<div class=\"tip-row\"><span style=\"color:var(--fg3)\">rolling</span>'\n"
        "      +'<span>'+R[i].toFixed(2)+' b/B</span></div>';\n"
        "  }\n"
        "  tip.innerHTML=h;\n"
        "  var wr=chart.parentNode.getBoundingClientRect();\n"
        "  var tx=(e.clientX-wr.left)+12, ty=(e.clientY-wr.top)-50;\n"
        "  if(tx+200>wr.width) tx=(e.clientX-wr.left)-200;\n"
        "  if(ty<0) ty=(e.clientY-wr.top)+16;\n"
        "  tip.style.left=tx+'px'; tip.style.top=ty+'px';\n"
        "  tip.style.display='block';\n"
        "});\n"
        "chart.addEventListener('mouseleave',hide);\n"
        "chart.addEventListener('click',function(e){\n"
        "  var i=idxFromEvent(e);\n"
        "  if(i<0||!window.hexHighlight) return;\n"
        "  var target=ML[i]>=4?(i-MD[i]):i;\n"
        "  window.hexHighlight(target);\n"
        "  var hx=document.getElementById('sec-hex');\n"
        "  if(hx) hx.scrollIntoView({behavior:'smooth',block:'start'});\n"
        "});\n"
        "})();</script>\n");

      fprintf(f, "</div>\n\n");
    }
    free(mlen);
    free(mdist);
  }

  /* ── Search Trajectory ── */
  if (s->search_best && s->search_len > 0) {
    int npts = s->search_len;
    float smin = 1e9f, smax = -1e9f;
    for (int i = 0; i < npts; i++) {
      if (s->search_best[i] < smin) smin = s->search_best[i];
      if (s->search_best[i] > smax) smax = s->search_best[i];
    }
    for (int e = 0; e < s->search_nevents; e++) {
      float v = s->search_events[e].est_bytes;
      if (v < smin) smin = v;
      if (v > smax) smax = v;
    }
    if (smin < 1) smin = 1;
    if (smax <= smin) smax = smin * 2;

    float xlog_max = fast_log2f((float)npts);
    float xlog_min = 0;
    float xlog_range = xlog_max - xlog_min;
    if (xlog_range < 1) xlog_range = 1;
    float ylog_min = fast_log2f(smin) - 0.1f;
    float ylog_max = fast_log2f(smax) + 0.1f;
    float ylog_range = ylog_max - ylog_min;
    if (ylog_range < 0.5f) ylog_range = 0.5f;

    int svg_w = 960, svg_h = 200;
    int pad_l = 54, pad_r = 12, pad_t = 14, pad_b = 30;
    int plot_w = svg_w - pad_l - pad_r;
    int plot_h = svg_h - pad_t - pad_b;

#define LOGX(idx) \
  (pad_l + (int)(plot_w * (fast_log2f((float)(idx) + 1) - xlog_min) / xlog_range))
#define LOGY(val) \
  (pad_t + (int)(plot_h * (ylog_max - fast_log2f((val) > 0.5f ? (val) : 0.5f)) / ylog_range))
#define CLAMP(v, lo, hi) ((v) < (lo) ? (lo) : ((v) > (hi) ? (hi) : (v)))

    fprintf(f,
      "<div class=\"card full\" id=\"sec-search\">\n"
      "<h2>Model Search Trajectory</h2>\n"
      "<p class=\"desc\">Best estimated size (log-log) over 256 context masks. "
      "<span style=\"color:#34d399\">\xe2\x97\x8f</span> addition, "
      "<span style=\"color:#f87171\">\xe2\x97\x8f</span> removal. "
      "<span style=\"color:var(--fg3)\">Click any dot for details.</span></p>\n");
    fprintf(f,
      "<style>#search-svg circle{transition:stroke-width .12s}"
      "#search-svg circle:hover{stroke:var(--fg);stroke-width:1.5}</style>\n");
    fprintf(f, "<div class=\"scrub-wrap\">\n");
    fprintf(f,
      "<svg id=\"search-svg\" width=\"100%%\" viewBox=\"0 0 %d %d\" "
      "style=\"display:block\">\n",
      svg_w, svg_h);

    fprintf(f,
      "<rect x=\"%d\" y=\"%d\" width=\"%d\" height=\"%d\" "
      "fill=\"var(--bg3)\" rx=\"4\"/>\n", pad_l, pad_t, plot_w, plot_h);

    /* y gridlines */
    {
      float nice_ticks[64]; int nticks = 0;
      float base = 1;
      while (base * 10 < smin) base *= 10;
      while (base * 0.1f < smax * 2 && nticks < 60) {
        float muls[] = {1, 1.2f, 1.5f, 2, 2.5f, 3, 4, 5, 6, 7, 8};
        for (int mi2 = 0; mi2 < 11 && nticks < 60; mi2++) {
          float val = base * muls[mi2];
          if (val >= smin * 0.95f && val <= smax * 1.05f)
            nice_ticks[nticks++] = val;
        }
        base *= 10;
      }
      for (int i = 0; i < nticks; i++) {
        int y = CLAMP(LOGY(nice_ticks[i]), pad_t, pad_t + plot_h);
        fprintf(f,
          "<line x1=\"%d\" y1=\"%d\" x2=\"%d\" y2=\"%d\" "
          "stroke=\"var(--bdr)\" stroke-width=\"0.5\"/>\n",
          pad_l, y, pad_l + plot_w, y);
        fprintf(f,
          "<text x=\"%d\" y=\"%d\" text-anchor=\"end\" "
          "font-size=\"9\" fill=\"var(--fg3)\">%g</text>\n",
          pad_l - 5, y + 3, nice_ticks[i]);
      }
    }

    /* x gridlines */
    {
      int xticks[] = {1, 2, 4, 8, 16, 32, 64, 128, 256};
      for (int i = 0; i < 9; i++) {
        if (xticks[i] > npts) break;
        int x = CLAMP(LOGX(xticks[i] - 1), pad_l, pad_l + plot_w);
        fprintf(f,
          "<line x1=\"%d\" y1=\"%d\" x2=\"%d\" y2=\"%d\" "
          "stroke=\"var(--bdr)\" stroke-width=\"0.5\"/>\n",
          x, pad_t, x, pad_t + plot_h);
        fprintf(f,
          "<text x=\"%d\" y=\"%d\" text-anchor=\"middle\" "
          "font-size=\"9\" fill=\"var(--fg3)\">%d</text>\n",
          x, svg_h - 6, xticks[i] - 1);
      }
    }

    /* area fill */
    fprintf(f,
      "<defs><linearGradient id=\"sfill\" x1=\"0\" y1=\"0\" x2=\"0\" y2=\"1\">"
      "<stop offset=\"0\" stop-color=\"#60a5fa\" stop-opacity=\".15\"/>"
      "<stop offset=\"1\" stop-color=\"#60a5fa\" stop-opacity=\".01\"/>"
      "</linearGradient></defs>\n");
    fprintf(f, "<path d=\"M%d,%d ", pad_l, pad_t + plot_h);
    for (int i = 0; i < npts; i++) {
      int x = CLAMP(LOGX(i), pad_l, pad_l + plot_w);
      int y = CLAMP(LOGY(s->search_best[i]), pad_t, pad_t + plot_h);
      if (i > 0) {
        int py = CLAMP(LOGY(s->search_best[i - 1]), pad_t, pad_t + plot_h);
        fprintf(f, "L%d,%d ", x, py);
      }
      fprintf(f, "L%d,%d ", x, y);
    }
    fprintf(f, "L%d,%d Z\" fill=\"url(#sfill)\"/>\n",
      pad_l + plot_w, pad_t + plot_h);

    /* step stroke */
    fprintf(f, "<path d=\"");
    for (int i = 0; i < npts; i++) {
      int x = CLAMP(LOGX(i), pad_l, pad_l + plot_w);
      int y = CLAMP(LOGY(s->search_best[i]), pad_t, pad_t + plot_h);
      if (i == 0) { fprintf(f, "M%d,%d ", x, y); }
      else {
        int py = CLAMP(LOGY(s->search_best[i - 1]), pad_t, pad_t + plot_h);
        fprintf(f, "L%d,%d L%d,%d ", x, py, x, y);
      }
    }
    fprintf(f,
      "\" fill=\"none\" stroke=\"#60a5fa\" stroke-width=\"1.5\"/>\n");

    /* event dots */
    for (int e = 0; e < s->search_nevents; e++) {
      int mi = s->search_events[e].mask_idx;
      if (mi >= npts) continue;
      int x = CLAMP(LOGX(mi), pad_l, pad_l + plot_w);
      float est = s->search_events[e].est_bytes;
      int y = CLAMP(LOGY(est), pad_t, pad_t + plot_h);
      const char *col = s->search_events[e].is_removal ? "#f87171" : "#34d399";
      fprintf(f,
        "<circle cx=\"%d\" cy=\"%d\" r=\"3.5\" fill=\"%s\" opacity=\".8\" "
        "data-e=\"%d\" style=\"cursor:pointer\"/>\n",
        x, y, col, e);
    }

#undef LOGX
#undef LOGY
#undef CLAMP

    /* axis labels */
    fprintf(f,
      "<text x=\"%d\" y=\"%d\" text-anchor=\"middle\" "
      "font-size=\"10\" fill=\"var(--fg3)\">mask index (log)</text>\n",
      pad_l + plot_w / 2, svg_h);
    fprintf(f,
      "<text x=\"14\" y=\"%d\" text-anchor=\"middle\" "
      "font-size=\"10\" fill=\"var(--fg3)\" "
      "transform=\"rotate(-90,14,%d)\">est. bytes (log)</text>\n",
      pad_t + plot_h / 2, pad_t + plot_h / 2);
    /* scrubber line */
    fprintf(f,
      "<line id=\"search-scrub\" class=\"scrub-line\" "
      "x1=\"0\" y1=\"%d\" x2=\"0\" y2=\"%d\"/>\n",
      pad_t, pad_t + plot_h);
    fprintf(f, "</svg>\n");
    fprintf(f, "<div id=\"search-tip\" class=\"hover-tip\"></div>\n");
    fprintf(f, "</div>\n");

    /* ── Detail panel ── */
    fprintf(f, "<div id=\"search-detail\" class=\"cd-panel\"></div>\n");

    /* ── Embed event data + click handler ── */
    fprintf(f, "<script>\nvar SE=[");
    for (int e = 0; e < s->search_nevents; e++) {
      fprintf(f, "%s{i:%d,m:%d,n:%d,b:%.4g,r:%d}", e ? "," : "",
        s->search_events[e].mask_idx,
        s->search_events[e].mask,
        s->search_events[e].num_models,
        s->search_events[e].est_bytes,
        s->search_events[e].is_removal);
    }
    fprintf(f, "];\n");
    fprintf(f, "var SB=[");
    for (int i = 0; i < s->search_len; i++) {
      fprintf(f, "%s%.4g", i ? "," : "", s->search_best[i]);
    }
    fprintf(f, "];\n");

    fprintf(f, "%s",
      "(function(){\n"
      "var svg=document.getElementById('search-svg');\n"
      "var panel=document.getElementById('search-detail');\n"
      "var sel=null;\n"
      "svg.addEventListener('click',function(ev){\n"
      "  var t=ev.target; if(t.tagName!=='circle') return;\n"
      "  var ei=t.getAttribute('data-e'); if(ei===null) return;\n"
      "  ei=+ei; var e=SE[ei]; if(!e) return;\n"
      "  if(sel) sel.classList.remove('cmap-sel');\n"
      "  t.classList.add('cmap-sel'); sel=t;\n"
      "  var hex='0x'+(e.m<16?'0':'')+e.m.toString(16).toUpperCase();\n"
      "  var before=(e.i>0&&e.i-1<SB.length)?SB[e.i-1]:null;\n"
      "  var delta=before!==null?e.b-before:null;\n"
      "  var dClr=delta===null?'var(--fg3)'\n"
      "    :delta<0?'#34d399':delta>0?'#f87171':'var(--fg3)';\n"
      "  var opClr=e.r?'#f87171':'#34d399';\n"
      "  var opLabel=e.r?'Remove':'Add';\n"
      "  /* mask bit visualization, matches Per-Model Statistics */\n"
      "  var maskBits='';\n"
      "  for(var b=7;b>=0;b--){\n"
      "    var on=(e.m>>b)&1;\n"
      "    maskBits+='<rect x=\"'+((7-b)*9+1)+'\" y=\"1\" width=\"8\" '\n"
      "      +'height=\"10\" rx=\"1\" fill=\"'+(on?'#22d3ee':'var(--bg3)')\n"
      "      +'\" stroke=\"var(--bdr)\" stroke-width=\"0.5\"/>';\n"
      "  }\n"
      "  var h='<div class=\"cd-head\">';\n"
      "  h+='<span class=\"cd-byte\" style=\"color:'+opClr+'\">'+opLabel\n"
      "    +' '+hex+'</span>';\n"
      "  h+='<svg width=\"82\" height=\"12\" '\n"
      "    +'style=\"vertical-align:middle\">'+maskBits+'</svg>';\n"
      "  h+='<span class=\"cd-sub\">iteration '+e.i+' / 255 \\u00b7 '\n"
      "    +'event '+(ei+1)+' / '+SE.length+'</span>';\n"
      "  h+='</div>';\n"
      "  h+='<div style=\"display:grid;grid-template-columns:1fr 1fr 1fr;'\n"
      "    +'gap:12px 20px;margin:10px 0;font-size:12px\">';\n"
      "  h+='<div><span style=\"color:var(--fg3)\">Models after</span><br>'\n"
      "    +'<span style=\"font-family:var(--mono);font-weight:600;'\n"
      "    +'color:var(--fg)\">'+e.n+'</span></div>';\n"
      "  h+='<div><span style=\"color:var(--fg3)\">Est. size</span><br>'\n"
      "    +'<span style=\"font-family:var(--mono);font-weight:600;'\n"
      "    +'color:var(--fg)\">'+e.b.toFixed(2)+' B</span></div>';\n"
      "  if(delta!==null){\n"
      "    h+='<div><span style=\"color:var(--fg3)\">\\u0394 vs prev iter</span><br>'\n"
      "      +'<span style=\"font-family:var(--mono);font-weight:600;'\n"
      "      +'color:'+dClr+'\">'+(delta>=0?'+':'')+delta.toFixed(2)+' B</span></div>';\n"
      "  }\n"
      "  h+='</div>';\n"
      "  panel.innerHTML=h;\n"
      "  panel.style.display='block';\n"
      "});\n"
      "/* hover scrubber - log-x mapping back to iteration */\n");
    fprintf(f,
      "var SW=%d,SPL=%d,SPW=%d,SPT=%d,SPH=%d;\n"
      "var XLM=%g,XLR=%g;\n",
      svg_w, pad_l, plot_w, pad_t, plot_h, xlog_min, xlog_range);
    fprintf(f, "%s",
      "var stip=document.getElementById('search-tip');\n"
      "var ssvg=document.getElementById('search-svg');\n"
      "function hideStip(){\n"
      "  var l=document.getElementById('search-scrub');\n"
      "  if(l) l.setAttribute('opacity','0');\n"
      "  stip.style.display='none';\n"
      "}\n"
      "ssvg.addEventListener('mousemove',function(e){\n"
      "  var r=ssvg.getBoundingClientRect();\n"
      "  var px=(e.clientX-r.left)/r.width*SW;\n"
      "  if(px<SPL||px>SPL+SPW){hideStip();return;}\n"
      "  /* invert: idx = 2^(((x-PL)/pw)*XLR + XLM) - 1 */\n"
      "  var logIdx=((px-SPL)/SPW)*XLR+XLM;\n"
      "  var idx=Math.round(Math.pow(2,logIdx)-1);\n"
      "  if(idx<0)idx=0; if(idx>=SB.length)idx=SB.length-1;\n"
      "  var best=SB[idx];\n"
      "  var l=document.getElementById('search-scrub');\n"
      "  if(l){l.setAttribute('x1',px); l.setAttribute('x2',px);\n"
      "        l.setAttribute('opacity','0.5');}\n"
      "  /* gather events at exactly this iter */\n"
      "  var evs=[];\n"
      "  for(var k=0;k<SE.length;k++) if(SE[k].i===idx) evs.push(SE[k]);\n"
      "  var h='<div class=\"tip-row\"><span style=\"color:var(--fg3)\">iter</span>'\n"
      "    +'<span style=\"color:var(--fg)\">'+idx+' / '+(SB.length-1)+'</span></div>'\n"
      "    +'<div class=\"tip-row\"><span style=\"color:var(--fg3)\">best</span>'\n"
      "    +'<span style=\"color:var(--fg);font-weight:600\">'+best.toFixed(1)+' B</span></div>';\n"
      "  if(evs.length){\n"
      "    h += '<div style=\"border-top:1px solid var(--bdr);'\n"
      "      +'margin:4px 0 2px;padding-top:4px;color:var(--fg3);font-size:10px\">'\n"
      "      +(evs.length===1?'event':evs.length+' events')+'</div>';\n"
      "    var show=Math.min(evs.length,4);\n"
      "    for(var k=0;k<show;k++){\n"
      "      var ev=evs[k];\n"
      "      var hex='0x'+(ev.m<16?'0':'')+ev.m.toString(16).toUpperCase();\n"
      "      var opClr=ev.r?'#f87171':'#34d399';\n"
      "      var op=ev.r?'-':'+';\n"
      "      h += '<div class=\"tip-row\">'\n"
      "        +'<span style=\"color:'+opClr+';font-weight:600\">'+op+hex+'</span>'\n"
      "        +'<span style=\"color:var(--fg2)\">'+ev.n+'m \\u00b7 '+ev.b.toFixed(1)+' B</span></div>';\n"
      "    }\n"
      "    if(evs.length>show)\n"
      "      h+='<div class=\"tip-row\"><span style=\"color:var(--fg3)\">+'\n"
      "        +(evs.length-show)+' more</span></div>';\n"
      "    /* sparkline of best size around this iteration */\n"
      "    if(window.makeSparkline){\n"
      "      var lo=Math.max(0,idx-10), hi=Math.min(SB.length-1,idx+10);\n"
      "      var vals=[];\n"
      "      for(var k=lo;k<=hi;k++) vals.push(SB[k]);\n"
      "      var sp=window.makeSparkline(vals,{\n"
      "        color:'#60a5fa', highlightIdx:idx-lo,\n"
      "        labelTop:Math.max.apply(Math,vals).toFixed(0)+' B',\n"
      "        labelRight:'iter '+lo+'-'+hi});\n"
      "      if(sp) h += '<div style=\"color:var(--fg3);font-size:10px;'\n"
      "        +'margin-top:4px\">best ~10 iters</div>'+sp;\n"
      "    }\n"
      "  }\n"
      "  stip.innerHTML=h;\n"
      "  var wrap=ssvg.parentNode;\n"
      "  var wr=wrap.getBoundingClientRect();\n"
      "  var tx=(e.clientX-wr.left)+12, ty=(e.clientY-wr.top)-40;\n"
      "  if(tx+180>wr.width) tx=(e.clientX-wr.left)-180;\n"
      "  if(ty<0) ty=(e.clientY-wr.top)+16;\n"
      "  stip.style.left=tx+'px'; stip.style.top=ty+'px';\n"
      "  stip.style.display='block';\n"
      "});\n"
      "ssvg.addEventListener('mouseleave',hideStip);\n"
      "})();\n"
      "</script>\n");

    fprintf(f, "</div>\n\n");
  }

  /* ── Mask Outcome Grid ── */
  if (s->search_best && s->search_len > 0) {
    int counts[4] = {0, 0, 0, 0};
    float dmax_rej = 0;
    for (int m = 0; m < 256; m++) {
      counts[s->mask_outcome[m] & 3]++;
      if (s->mask_outcome[m] == 1 && s->mask_delta[m] > dmax_rej)
        dmax_rej = s->mask_delta[m];
    }
    if (dmax_rej <= 0)
      dmax_rej = 1;

    int cell = 28, gap = 1;
    int grid = cell * 16 + gap * 15;
    int hdr = 18, row_lbl = 24;
    int svg_w = row_lbl + grid + 4;
    int svg_h = hdr + grid + 4;

    fprintf(f,
      "<div class=\"card full\" id=\"sec-maskgrid\">\n"
      "<h2>Mask Search Outcomes</h2>\n"
      "<p class=\"desc\">All 256 context masks by first-pass search outcome: "
      "%d in final set, %d accepted then removed, %d rejected, %d not tried. "
      "Rejected cells brighten the closer the mask came to helping; "
      "final-set cells show the model weight.</p>\n",
      counts[3], counts[2], counts[1], counts[0]);

    /* legend */
    {
      struct { const char *clr; const char *lbl; } lg[] = {
        {"#22d3ee", "final set"},
        {"#fbbf24", "accepted, later removed"},
        {"#f87171", "rejected"},
        {"var(--bg4)", "not tried"},
      };
      fprintf(f, "<div style=\"display:flex;flex-wrap:wrap;gap:8px 16px;"
        "margin-bottom:10px;font-size:11px;color:var(--fg2)\">\n");
      for (int i = 0; i < 4; i++)
        fprintf(f,
          "<span style=\"display:inline-flex;align-items:center;gap:5px\">"
          "<span style=\"display:inline-block;width:10px;height:10px;"
          "border-radius:2px;background:%s\"></span>%s</span>\n",
          lg[i].clr, lg[i].lbl);
      fprintf(f, "</div>\n");
    }

    fprintf(f, "<div class=\"scrub-wrap\">\n");
    fprintf(f, "<div id=\"mg-tip\" class=\"hover-tip\"></div>\n");
    fprintf(f,
      "<svg id=\"mg-svg\" width=\"100%%\" viewBox=\"0 0 %d %d\" "
      "style=\"font-family:var(--mono);display:block\">\n", svg_w, svg_h);

    for (int c = 0; c < 16; c++) {
      int x = row_lbl + c * (cell + gap) + cell / 2;
      fprintf(f,
        "<text x=\"%d\" y=\"%d\" text-anchor=\"middle\" font-size=\"8\" "
        "fill=\"var(--fg3)\">%X</text>\n", x, hdr - 5, c);
    }
    for (int r = 0; r < 16; r++) {
      int y = hdr + r * (cell + gap);
      fprintf(f,
        "<text x=\"%d\" y=\"%d\" text-anchor=\"end\" font-size=\"8\" "
        "fill=\"var(--fg3)\">%X_</text>\n", row_lbl - 4, y + cell / 2 + 3, r);
      for (int c = 0; c < 16; c++) {
        int m = r * 16 + c;
        int x = row_lbl + c * (cell + gap);
        int oc = s->mask_outcome[m];
        char fill[40];
        if (oc == 3)
          snprintf(fill, sizeof(fill), "#22d3ee");
        else if (oc == 2)
          snprintf(fill, sizeof(fill), "#fbbf24");
        else if (oc == 1) {
          float t = 1.0f - s->mask_delta[m] / dmax_rej;
          if (t < 0) t = 0;
          if (t > 1) t = 1;
          snprintf(fill, sizeof(fill), "rgba(248,113,113,%.2f)",
                   0.08f + 0.55f * t);
        } else
          snprintf(fill, sizeof(fill), "var(--bg4)");
        fprintf(f,
          "<rect x=\"%d\" y=\"%d\" width=\"%d\" height=\"%d\" rx=\"2\" "
          "fill=\"%s\" data-m=\"%d\" style=\"cursor:pointer\"/>\n",
          x, y, cell, cell, fill, m);
        if (oc == 3) {
          int w = 0;
          for (int k = 0; k < s->num_models; k++)
            if (s->model_masks[k] == m) { w = s->model_weights[k]; break; }
          fprintf(f,
            "<text x=\"%d\" y=\"%d\" text-anchor=\"middle\" font-size=\"10\" "
            "font-weight=\"600\" fill=\"#0c0e14\" "
            "pointer-events=\"none\">%d</text>\n",
            x + cell / 2, y + cell / 2 + 3, w);
        }
      }
    }
    fprintf(f, "</svg>\n</div>\n");

    /* outcome data + hover tooltip */
    fprintf(f, "<script>\n(function(){\nvar MO=[");
    for (int m = 0; m < 256; m++)
      fprintf(f, "%s%d", m ? "," : "", s->mask_outcome[m]);
    fprintf(f, "];\nvar MDL=[");
    for (int m = 0; m < 256; m++)
      fprintf(f, "%s%.3g", m ? "," : "", s->mask_delta[m]);
    fprintf(f, "];\n");
    fprintf(f, "%s",
      "var svg=document.getElementById('mg-svg');\n"
      "var tip=document.getElementById('mg-tip');\n"
      "var NAMES=['not tried','rejected','accepted, later removed',"
      "'in final set'];\n"
      "var CLRS=['var(--fg3)','#f87171','#fbbf24','#22d3ee'];\n"
      "function hide(){tip.style.display='none';}\n"
      "svg.addEventListener('mousemove',function(e){\n"
      "  var t=e.target;\n"
      "  if(t.tagName!=='rect'||t.getAttribute('data-m')===null){hide();return;}\n"
      "  var m=+t.getAttribute('data-m');\n"
      "  var hex='0x'+(m<16?'0':'')+m.toString(16).toUpperCase();\n"
      "  var bits='';\n"
      "  for(var b=7;b>=0;b--){\n"
      "    var on=(m>>b)&1;\n"
      "    bits+='<rect x=\"'+((7-b)*9)+'\" y=\"0\" width=\"8\" height=\"10\" '\n"
      "      +'rx=\"1\" fill=\"'+(on?'#22d3ee':'var(--bg3)')\n"
      "      +'\" stroke=\"var(--bdr)\" stroke-width=\"0.5\"/>';\n"
      "  }\n"
      "  var oc=MO[m];\n"
      "  var h='<div class=\"tip-row\"><span style=\"color:var(--fg)\">'+hex+'</span>'\n"
      "    +'<span><svg width=\"72\" height=\"10\">'+bits+'</svg></span></div>'\n"
      "    +'<div class=\"tip-row\"><span style=\"color:var(--fg3)\">outcome</span>'\n"
      "    +'<span style=\"color:'+CLRS[oc]+';font-weight:600\">'+NAMES[oc]\n"
      "    +'</span></div>';\n"
      "  if(oc>0){\n"
      "    var d=MDL[m];\n"
      "    h+='<div class=\"tip-row\"><span style=\"color:var(--fg3)\">'\n"
      "      +'\\u0394 when tried</span>'\n"
      "      +'<span style=\"color:'+(d<0?'#34d399':'#f87171')+'\">'\n"
      "      +(d>=0?'+':'')+d.toFixed(2)+' B</span></div>';\n"
      "  }\n"
      "  tip.innerHTML=h;\n"
      "  var pr=svg.parentNode.getBoundingClientRect();\n"
      "  var tx=(e.clientX-pr.left)+14, ty=(e.clientY-pr.top)-50;\n"
      "  if(tx+190>pr.width) tx=(e.clientX-pr.left)-190;\n"
      "  if(ty<0) ty=(e.clientY-pr.top)+18;\n"
      "  tip.style.left=tx+'px'; tip.style.top=ty+'px';\n"
      "  tip.style.display='block';\n"
      "});\n"
      "svg.addEventListener('mouseleave',hide);\n"
      "})();</script>\n");

    fprintf(f, "</div>\n\n");
  }

  /* ── Per-Model Statistics ── */
  {
    int order[MAX_SEARCH];
    for (int m = 0; m < s->num_models; m++) order[m] = m;
    for (int i = 0; i < s->num_models - 1; i++)
      for (int j = i + 1; j < s->num_models; j++)
        if (s->model_bits_saved[order[i]] < s->model_bits_saved[order[j]]) {
          int tmp = order[i]; order[i] = order[j]; order[j] = tmp;
        }

    double max_saved = 0;
    for (int m = 0; m < s->num_models; m++) {
      double v = s->model_bits_saved[m];
      if (v > max_saved) max_saved = v;
      if (-v > max_saved) max_saved = -v;
    }

    fprintf(f,
      "<div class=\"card full\" id=\"sec-models\" style=\"position:relative\">\n"
      "<h2>Per-Model Statistics</h2>\n"
      "<p class=\"desc\">Sorted by contribution. Positive bits saved = model helped. "
      "<span style=\"color:var(--fg3)\">Hover for a per-window sparkline; "
      "click a row to highlight that model in the Attribution Map "
      "(click again to flip to where it hurts).</span></p>\n"
      "<div id=\"pm-tip\" class=\"hover-tip\"></div>\n"
      "<table><tr>"
      "<th class=\"pm-th\" data-col=\"0\">#</th>"
      "<th class=\"pm-th\" data-col=\"1\">Mask</th>"
      "<th class=\"pm-th r\" data-col=\"2\">Weight</th>"
      "<th class=\"pm-th r\" data-col=\"3\">Hits</th>"
      "<th class=\"pm-th r\" data-col=\"4\">Hit %%</th>"
      "<th class=\"pm-th r\" data-col=\"5\">Unique Ctx</th>"
      "<th class=\"pm-th r\" data-col=\"6\">Bits Saved</th>"
      "<th class=\"pm-th r\" data-col=\"7\">Bytes Saved</th>"
      "</tr>\n");

    double total_saved = 0;
    for (int i = 0; i < s->num_models; i++) {
      int m = order[i];
      unsigned int tot = s->model_hits[m] + s->model_misses[m];
      double pct = tot ? 100.0 * s->model_hits[m] / tot : 0.0;
      double bits = s->model_bits_saved[m];
      total_saved += bits;
      double abs_bits = bits < 0 ? -bits : bits;
      double bar_w = max_saved > 0 ? 100.0 * abs_bits / max_saved : 0;
      const char *cls = bits > max_saved * 0.3 ? "c-grn"
                      : bits > max_saved * 0.1 ? "c-acc"
                      : bits > 0 ? "c-ylw" : "c-red";
      const char *bcls = bits > 0 ? "bar-grn" : "bar-red";

      fprintf(f,
        "<tr class=\"pm-row\" data-bm=\"%d\" style=\"cursor:pointer\">"
        "<td class=\"r\" data-v=\"%d\">%d</td>"
        "<td class=\"n\" data-v=\"%d\" style=\"white-space:nowrap\">%02X <svg "
        "width=\"66\" height=\"12\" "
        "style=\"vertical-align:middle;display:inline-block\">",
        m, m, m, s->model_masks[m], s->model_masks[m]);
      for (int b = 7; b >= 0; b--) {
        int on = (s->model_masks[m] >> b) & 1;
        int bx = (7 - b) * 8 + 1;
        fprintf(f,
          "<rect x=\"%d\" y=\"1\" width=\"7\" height=\"10\" rx=\"1\" "
          "fill=\"%s\" stroke=\"%s\" stroke-width=\"0.5\"/>",
          bx, on ? "#22d3ee" : "var(--bg3)", "var(--bdr)");
      }
      fprintf(f,
        "</svg></td>"
        "<td class=\"r\" data-v=\"%d\">%d</td>"
        "<td class=\"r\" data-v=\"%u\">%u</td>"
        "<td class=\"r\" data-v=\"%.4f\">%.1f</td>"
        "<td class=\"r\" data-v=\"%u\">%u</td>"
        "<td class=\"r bar-cell\" data-v=\"%.4f\">"
        "<div class=\"bar %s\" style=\"width:%.0f%%;opacity:.18\"></div>"
        "<span class=\"bar-label %s\">%.1f</span></td>"
        "<td class=\"r\" data-v=\"%.4f\">%.1f</td></tr>\n",
        s->model_weights[m], s->model_weights[m],
        s->model_hits[m], s->model_hits[m],
        pct, pct,
        s->model_misses[m], s->model_misses[m],
        bits, bcls, bar_w, cls, bits,
        bits / 8.0, bits / 8.0);
    }
    fprintf(f,
      "<tr style=\"border-top:2px solid var(--bdr2)\">"
      "<td colspan=\"6\" class=\"n\">Total</td>"
      "<td class=\"r c-grn\" style=\"font-weight:600\">%.1f</td>"
      "<td class=\"r c-grn\" style=\"font-weight:600\">%.1f</td></tr>\n",
      total_saved, total_saved / 8.0);
    fprintf(f, "</table>\n");

    /* Wire rows to the Attribution Map's setHilite (set up earlier on
       window.attrSetHilite). Also register pmSetActive so Attribution Map
       can drive the row highlight when the user clicks elsewhere.
       Also wire sortable column headers. */
    fprintf(f, "%s",
      "<script>(function(){\n"
      "var rows=Array.prototype.slice.call("
      "document.querySelectorAll('#sec-models tr.pm-row'));\n"
      "var pmTip=document.getElementById('pm-tip');\n"
      "var pmCard=document.getElementById('sec-models');\n"
      "function hidePmTip(){if(pmTip) pmTip.style.display='none';}\n"
      "/* build per-model sparkline via shared helper */\n"
      "function sparkline(m){\n"
      "  if(!window.netMdata || !window.makeSparkline) return '';\n"
      "  var nm=window.netNm, np=window.netNpts, nb=window.netNb;\n"
      "  var vals=[]; var vmax=0.001;\n"
      "  for(var i=0;i<np;i++){\n"
      "    var v=window.netMdata[i*nm+m]; vals.push(v);\n"
      "    var a=Math.abs(v); if(a>vmax) vmax=a;\n"
      "  }\n"
      "  var pc=(window.netPal&&window.netPal[m])||[100,100,100];\n"
      "  return window.makeSparkline(vals,{\n"
      "    color:'rgb('+pc[0]+','+pc[1]+','+pc[2]+')',\n"
      "    labelTop:'+'+vmax.toFixed(2),\n"
      "    labelBot:'\\u2212'+vmax.toFixed(2),\n"
      "    labelRight:nb+' B'});\n"
      "}\n"
      "for(var i=0;i<rows.length;i++){\n"
      "  rows[i].addEventListener('click',function(){\n"
      "    var m=this.getAttribute('data-bm');\n"
      "    if(window.attrSetHilite) window.attrSetHilite(m);\n"
      "    var attr=document.getElementById('sec-attr');\n"
      "    if(attr) attr.scrollIntoView({behavior:'smooth',block:'start'});\n"
      "  });\n"
      "  rows[i].addEventListener('mousemove',function(e){\n"
      "    var m=parseInt(this.getAttribute('data-bm'));\n"
      "    var maskCell=this.children[1];\n"
      "    var label=maskCell?maskCell.textContent.trim().split(/\\s+/)[0]:'';\n"
      "    var spark=sparkline(m);\n"
      "    var h='<div class=\"tip-row\"><span style=\"color:var(--fg3)\">model</span>'\n"
      "      +'<span style=\"color:var(--fg)\">#'+m+(label?' (mask '+label+')':'')+'</span></div>';\n"
      "    if(spark){h += '<div style=\"color:var(--fg3);font-size:10px;'\n"
      "      +'margin-top:4px\">net contribution per window</div>'+spark;}\n"
      "    else {h += '<div class=\"tip-row\"><span style=\"color:var(--fg3)\">'\n"
      "      +'(sparkline data unavailable)</span></div>';}\n"
      "    pmTip.innerHTML=h;\n"
      "    var cr=pmCard.getBoundingClientRect();\n"
      "    var tx=(e.clientX-cr.left)+14, ty=(e.clientY-cr.top)+14;\n"
      "    if(tx+220>cr.width) tx=(e.clientX-cr.left)-220;\n"
      "    pmTip.style.left=tx+'px'; pmTip.style.top=ty+'px';\n"
      "    pmTip.style.display='block';\n"
      "  });\n"
      "  rows[i].addEventListener('mouseleave',hidePmTip);\n"
      "}\n"
      "/* observer hook: highlight the active model's row */\n"
      "window.pmSetActive=function(m,mode){\n"
      "  for(var i=0;i<rows.length;i++){\n"
      "    var rm=rows[i].getAttribute('data-bm');\n"
      "    var on=(m!==null&&m!==undefined&&rm===m);\n"
      "    rows[i].style.background=on\n"
      "      ?(mode==='best'?'rgba(52,211,153,.10)':'rgba(248,113,113,.10)')\n"
      "      :'';\n"
      "  }\n"
      "};\n"
      "/* sortable columns */\n"
      "var ths=document.querySelectorAll('#sec-models .pm-th');\n"
      "/* rows share a parent (auto-created tbody); use it for insertBefore */\n"
      "var rowParent=rows.length?rows[0].parentNode:null;\n"
      "var allRows=rowParent?rowParent.querySelectorAll('tr'):[];\n"
      "var totalRow=allRows.length?allRows[allRows.length-1]:null;\n"
      "if(totalRow && totalRow.classList.contains('pm-row')) totalRow=null;\n"
      "for(var i=0;i<ths.length;i++){\n"
      "  var sp=document.createElement('span');\n"
      "  sp.className='sort-arrow'; sp.textContent='\\u2195';\n"
      "  ths[i].appendChild(sp);\n"
      "}\n"
      "var sortCol=6, sortDir=-1; /* bits saved desc by default (matches C) */\n"
      "function sortBy(col,dir){\n"
      "  sortCol=col; sortDir=dir;\n"
      "  rows.sort(function(a,b){\n"
      "    var va=parseFloat(a.children[col].getAttribute('data-v'));\n"
      "    var vb=parseFloat(b.children[col].getAttribute('data-v'));\n"
      "    return (va-vb)*dir;\n"
      "  });\n"
      "  var frag=document.createDocumentFragment();\n"
      "  rows.forEach(function(r){frag.appendChild(r);});\n"
      "  if(totalRow) rowParent.insertBefore(frag,totalRow);\n"
      "  else rowParent.appendChild(frag);\n"
      "  for(var i=0;i<ths.length;i++){\n"
      "    var c=parseInt(ths[i].getAttribute('data-col'));\n"
      "    var ar=ths[i].querySelector('.sort-arrow');\n"
      "    if(c===col){\n"
      "      ths[i].classList.add('active');\n"
      "      if(ar) ar.textContent=dir>0?'\\u2191':'\\u2193';\n"
      "    } else {\n"
      "      ths[i].classList.remove('active');\n"
      "      if(ar) ar.textContent='\\u2195';\n"
      "    }\n"
      "  }\n"
      "  if(window.reportState){\n"
      "    window.reportState.sortCol=col; window.reportState.sortDir=dir;\n"
      "    if(window.serializeReportState) window.serializeReportState();\n"
      "  }\n"
      "}\n"
      "window.pmSortBy=sortBy;\n"
      "for(var i=0;i<ths.length;i++){\n"
      "  (function(th){\n"
      "    th.addEventListener('click',function(){\n"
      "      var col=parseInt(th.getAttribute('data-col'));\n"
      "      if(col===sortCol) sortDir=-sortDir;\n"
      "      else { sortCol=col; sortDir=(col===0||col===1)?1:-1; }\n"
      "      sortBy(sortCol,sortDir);\n"
      "    });\n"
      "  })(ths[i]);\n"
      "}\n"
      "sortBy(sortCol,sortDir);\n"
      "})();</script>\n");

    fprintf(f, "</div>\n\n");
  }

  /* ── Prediction Confidence ── */
  {
    unsigned int max_conf = 0;
    for (int i = 0; i < 11; i++)
      if (s->conf_hist[i] > max_conf) max_conf = s->conf_hist[i];

    const char *conf_labels[] = {"&lt;50%%", "50-55%%", "55-60%%", "60-65%%",
      "65-70%%", "70-75%%", "75-80%%", "80-85%%", "85-90%%", "90-95%%", "95-100%%"};

    fprintf(f,
      "<div class=\"card\" id=\"sec-conf\">\n"
      "<h2>Prediction Confidence</h2>\n"
      "<p class=\"desc\">Confidence distribution when encoding each bit.</p>\n"
      "<table><tr><th>Range</th><th class=\"r\">Bits</th>"
      "<th class=\"r\">%%</th><th>Distribution</th></tr>\n");

    for (int i = 0; i < 11; i++) {
      if (s->conf_hist[i] == 0) continue;
      double pct = 100.0 * s->conf_hist[i] / s->total_bits;
      double bar_w = max_conf > 0 ? 100.0 * s->conf_hist[i] / max_conf : 0;
      const char *bcls = i >= 8 ? "bar-grn"
                       : i >= 5 ? "bar-acc"
                       : i >= 2 ? "bar-ylw"
                       : i >= 1 ? "bar-orn" : "bar-red";
      fprintf(f,
        "<tr><td class=\"n\">%s</td>"
        "<td class=\"r\">%u</td><td class=\"r\">%.1f</td>"
        "<td class=\"bar-cell\">"
        "<div class=\"bar %s\" style=\"width:%.0f%%;opacity:.25\"></div>"
        "<span class=\"bar-label\">&nbsp;</span></td></tr>\n",
        conf_labels[i], s->conf_hist[i], pct, bcls, bar_w);
    }
    fprintf(f, "</table></div>\n\n");
  }

  /* ── Calibration ── */
  {
    unsigned int total_cal = 0;
    for (int i = 0; i < 20; i++)
      total_cal += s->calib_count[i];
    if (total_cal > 0) {
      double ece = 0;
      for (int i = 0; i < 20; i++) {
        if (!s->calib_count[i])
          continue;
        double pred = s->calib_psum[i] / s->calib_count[i];
        double obs = (double)s->calib_ones[i] / s->calib_count[i];
        ece += (double)s->calib_count[i] / total_cal *
               (pred > obs ? pred - obs : obs - pred);
      }

      int W = 320, Hh = 320;
      int pl = 38, pr = 10, pt = 10, pb = 34;
      int pw = W - pl - pr, ph = Hh - pt - pb;

      fprintf(f,
        "<div class=\"card\" id=\"sec-calib\" style=\"position:relative\">\n"
        "<h2>Calibration</h2>\n"
        "<p class=\"desc\">Predicted P(bit=1) vs observed rate, 5%% buckets "
        "over %u bits. Points on the diagonal = honest probabilities. "
        "ECE = %.4f (lower is better).</p>\n",
        total_cal, ece);
      fprintf(f, "<div class=\"scrub-wrap\">\n");
      fprintf(f, "<div id=\"calib-tip\" class=\"hover-tip\"></div>\n");
      fprintf(f,
        "<svg id=\"calib-svg\" width=\"100%%\" viewBox=\"0 0 %d %d\" "
        "style=\"display:block;max-width:420px;margin:0 auto\">\n", W, Hh);
      fprintf(f,
        "<rect x=\"%d\" y=\"%d\" width=\"%d\" height=\"%d\" "
        "fill=\"var(--bg3)\" rx=\"4\"/>\n", pl, pt, pw, ph);

      /* gridlines at 0, .25, .5, .75, 1 on both axes */
      for (int i = 0; i <= 4; i++) {
        double v = i / 4.0;
        int x = pl + (int)(v * pw);
        int y = pt + ph - (int)(v * ph);
        fprintf(f,
          "<line x1=\"%d\" y1=\"%d\" x2=\"%d\" y2=\"%d\" "
          "stroke=\"var(--bdr)\" stroke-width=\"0.5\"/>\n",
          x, pt, x, pt + ph);
        fprintf(f,
          "<line x1=\"%d\" y1=\"%d\" x2=\"%d\" y2=\"%d\" "
          "stroke=\"var(--bdr)\" stroke-width=\"0.5\"/>\n",
          pl, y, pl + pw, y);
        fprintf(f,
          "<text x=\"%d\" y=\"%d\" text-anchor=\"middle\" font-size=\"9\" "
          "fill=\"var(--fg3)\">%.2f</text>\n", x, pt + ph + 13, v);
        fprintf(f,
          "<text x=\"%d\" y=\"%d\" text-anchor=\"end\" font-size=\"9\" "
          "fill=\"var(--fg3)\">%.2f</text>\n", pl - 4, y + 3, v);
      }
      /* ideal diagonal */
      fprintf(f,
        "<line x1=\"%d\" y1=\"%d\" x2=\"%d\" y2=\"%d\" "
        "stroke=\"var(--fg3)\" stroke-width=\"1\" stroke-dasharray=\"5,3\" "
        "opacity=\".6\"/>\n", pl, pt + ph, pl + pw, pt);

      /* one dot per bucket: x = mean predicted, y = observed */
      for (int i = 0; i < 20; i++) {
        if (!s->calib_count[i])
          continue;
        double pred = s->calib_psum[i] / s->calib_count[i];
        double obs = (double)s->calib_ones[i] / s->calib_count[i];
        double dev = pred > obs ? pred - obs : obs - pred;
        const char *clr = dev < 0.02 ? "#34d399"
                        : dev < 0.06 ? "#fbbf24" : "#f87171";
        float r = 2.0f + fast_log2f((float)s->calib_count[i] + 1) * 0.5f;
        if (r > 7) r = 7;
        fprintf(f,
          "<circle cx=\"%.1f\" cy=\"%.1f\" r=\"%.1f\" fill=\"%s\" "
          "fill-opacity=\".8\" data-cb=\"%d\" style=\"cursor:pointer\"/>\n",
          pl + pred * pw, pt + ph - obs * ph, r, clr, i);
      }

      fprintf(f,
        "<text x=\"%d\" y=\"%d\" text-anchor=\"middle\" font-size=\"10\" "
        "fill=\"var(--fg3)\">predicted P(1)</text>\n",
        pl + pw / 2, Hh - 4);
      fprintf(f,
        "<text x=\"12\" y=\"%d\" text-anchor=\"middle\" font-size=\"10\" "
        "fill=\"var(--fg3)\" transform=\"rotate(-90 12 %d)\">"
        "observed P(1)</text>\n",
        pt + ph / 2, pt + ph / 2);
      fprintf(f, "</svg>\n</div>\n");

      /* bucket data + hover tooltip */
      fprintf(f, "<script>\n(function(){\nvar CAL=[");
      for (int i = 0; i < 20; i++) {
        if (s->calib_count[i])
          fprintf(f, "%s[%.4f,%.4f,%u]", i ? "," : "",
            s->calib_psum[i] / s->calib_count[i],
            (double)s->calib_ones[i] / s->calib_count[i],
            s->calib_count[i]);
        else
          fprintf(f, "%s0", i ? "," : "");
      }
      fprintf(f, "];\n");
      fprintf(f, "%s",
        "var svg=document.getElementById('calib-svg');\n"
        "var tip=document.getElementById('calib-tip');\n"
        "function hide(){tip.style.display='none';}\n"
        "svg.addEventListener('mousemove',function(e){\n"
        "  var t=e.target;\n"
        "  if(t.tagName!=='circle'){hide();return;}\n"
        "  var b=+t.getAttribute('data-cb');\n"
        "  var c=CAL[b]; if(!c){hide();return;}\n"
        "  var dev=c[0]-c[1];\n"
        "  var dClr=Math.abs(dev)<0.02?'#34d399':Math.abs(dev)<0.06?'#fbbf24':'#f87171';\n"
        "  tip.innerHTML='<div class=\"tip-row\"><span style=\"color:var(--fg)\">'\n"
        "    +(b*5)+'\\u2013'+((b+1)*5)+'% bucket</span></div>'\n"
        "    +'<div style=\"border-top:1px solid var(--bdr);margin:4px 0 2px;'\n"
        "    +'padding-top:4px\"></div>'\n"
        "    +'<div class=\"tip-row\"><span style=\"color:var(--fg3)\">bits</span>'\n"
        "    +'<span>'+c[2]+'</span></div>'\n"
        "    +'<div class=\"tip-row\"><span style=\"color:var(--fg3)\">predicted</span>'\n"
        "    +'<span>'+c[0].toFixed(3)+'</span></div>'\n"
        "    +'<div class=\"tip-row\"><span style=\"color:var(--fg3)\">observed</span>'\n"
        "    +'<span>'+c[1].toFixed(3)+'</span></div>'\n"
        "    +'<div class=\"tip-row\"><span style=\"color:var(--fg3)\">deviation</span>'\n"
        "    +'<span style=\"color:'+dClr+';font-weight:600\">'\n"
        "    +(dev>=0?'+':'')+dev.toFixed(3)+'</span></div>';\n"
        "  var pr=svg.parentNode.getBoundingClientRect();\n"
        "  var tx=(e.clientX-pr.left)+14, ty=(e.clientY-pr.top)-60;\n"
        "  if(tx+180>pr.width) tx=(e.clientX-pr.left)-180;\n"
        "  if(ty<0) ty=(e.clientY-pr.top)+18;\n"
        "  tip.style.left=tx+'px'; tip.style.top=ty+'px';\n"
        "  tip.style.display='block';\n"
        "});\n"
        "svg.addEventListener('mouseleave',hide);\n"
        "})();</script>\n");

      fprintf(f, "</div>\n\n");
    }
  }

  /* ── Byte Position Analysis ── */
  {
    double max_bpc = 0;
    for (int i = 0; i < 8; i++) {
      if (s->bytepos_count[i] > 0) {
        double c2 = s->bytepos_cost[i] / s->bytepos_count[i];
        if (c2 > max_bpc) max_bpc = c2;
      }
    }

    fprintf(f,
      "<div class=\"card\" id=\"sec-bytepos\">\n"
      "<h2>Byte Position Analysis</h2>\n"
      "<p class=\"desc\">Average encoding cost per bit position within each byte.</p>\n"
      "<table><tr><th>Bit</th><th class=\"r\">Count</th>"
      "<th class=\"r\">Avg Cost</th><th>Cost</th></tr>\n");

    for (int i = 0; i < 8; i++) {
      if (s->bytepos_count[i] == 0) continue;
      double avg = s->bytepos_cost[i] / s->bytepos_count[i];
      double bar_w = max_bpc > 0 ? 100.0 * avg / max_bpc : 0;
      const char *cls = avg < max_bpc * 0.4 ? "c-grn"
                      : avg < max_bpc * 0.6 ? "c-blu"
                      : avg < max_bpc * 0.8 ? "c-ylw" : "c-orn";
      const char *bcls = avg < max_bpc * 0.4 ? "bar-grn"
                       : avg < max_bpc * 0.6 ? "bar-acc"
                       : avg < max_bpc * 0.8 ? "bar-ylw" : "bar-orn";
      fprintf(f,
        "<tr><td class=\"n\">Bit %d</td>"
        "<td class=\"r\">%u</td>"
        "<td class=\"r %s\">%.3f</td>"
        "<td class=\"bar-cell\">"
        "<div class=\"bar %s\" style=\"width:%.0f%%;opacity:.2\"></div>"
        "<span class=\"bar-label\">&nbsp;</span></td></tr>\n",
        i, s->bytepos_count[i], cls, avg, bcls, bar_w);
    }
    fprintf(f, "</table></div>\n\n");
  }

  /* ── Direct-Mapped Table ── */
  {
    double load = s->ht_size ? 100.0 * s->ht_occupied / s->ht_size : 0;
    double tsz = (double)s->ht_size * 2.0; /* 2 bytes per slot */
    const char *tunit = "B";
    if (tsz >= 1024) { tsz /= 1024; tunit = "KiB"; }
    if (tsz >= 1024) { tsz /= 1024; tunit = "MiB"; }
    if (tsz >= 1024) { tsz /= 1024; tunit = "GiB"; }

    fprintf(f,
      "<div class=\"card\" id=\"sec-hash\">\n"
      "<h2>Direct-Mapped Table</h2>\n"
      "<p class=\"desc\">Lossy direct-mapped counter table (slot = hash &amp; mask). "
      "Distinct contexts that map to the same slot silently share a counter, so a "
      "higher load means more collisions.</p>\n"
      "<table class=\"kv\">\n");
    fprintf(f,
      "<tr><td>Table size</td><td>%u slots (%.1f %s, 2 B/slot)</td></tr>\n",
      s->ht_size, tsz, tunit);
    fprintf(f,
      "<tr><td>Occupied</td><td class=\"%s\">%u (%.1f%%)</td></tr>\n",
      load < 50 ? "c-grn" : load < 75 ? "c-ylw" : "c-orn",
      s->ht_occupied, load);
    fprintf(f, "</table></div>\n\n");
  }

  /* ── Table Occupancy Growth ── */
  if (s->occ_nsamples > 1) {
    fprintf(f, "%s",
      "<div class=\"card\" id=\"sec-occ\">\n"
      "<h2>Table Occupancy Growth</h2>\n"
      "<p class=\"desc\">Fresh slots claimed in the direct-mapped table as "
      "encoding progresses. A straight line means mostly-new contexts; "
      "flattening means contexts repeat (or collide).</p>\n"
      "<div class=\"scrub-wrap\">\n"
      "<div id=\"occ-chart\"></div>\n"
      "<div id=\"occ-tip\" class=\"hover-tip\"></div>\n"
      "</div>\n");

    fprintf(f, "<script>(function(){\n");
    fprintf(f, "var OI=%d,TB=%d,HT=%u;\n",
      s->occ_interval, s->total_bits, s->ht_size);
    fprintf(f, "var OS=[");
    for (int i = 0; i < s->occ_nsamples; i++)
      fprintf(f, "%s%u", i ? "," : "", s->occ_samples[i]);
    fprintf(f, "];\n");
    fprintf(f, "%s",
      "var n=OS.length;\n"
      "function bitsAt(k){return k<n-1?(k+1)*OI:TB;}\n"
      "var maxY=OS[n-1]*1.05;\n"
      "if(maxY<=0) maxY=1;\n"
      "var W=460,H=190,PL=52,PR=10,PT=10,PB=28;\n"
      "var pw=W-PL-PR,ph=H-PT-PB;\n"
      "function X(k){return PL+bitsAt(k)/TB*pw;}\n"
      "function Y(v){return PT+ph-v/maxY*ph;}\n"
      "var s='<svg width=\"100%\" viewBox=\"0 0 '+W+' '+H\n"
      "  +'\" style=\"display:block;max-width:560px\">';\n"
      "s+='<rect x=\"'+PL+'\" y=\"'+PT+'\" width=\"'+pw+'\" height=\"'+ph\n"
      "  +'\" fill=\"var(--bg3)\" rx=\"4\"/>';\n"
      "for(var i=0;i<=3;i++){\n"
      "  var v=maxY*(3-i)/3, y=(PT+ph*i/3)|0;\n"
      "  s+='<line x1=\"'+PL+'\" y1=\"'+y+'\" x2=\"'+(PL+pw)+'\" y2=\"'+y\n"
      "    +'\" stroke=\"var(--bdr)\" stroke-width=\"0.5\"/>'\n"
      "    +'<text x=\"'+(PL-4)+'\" y=\"'+(y+3)+'\" text-anchor=\"end\" '\n"
      "    +'font-size=\"9\" fill=\"var(--fg3)\">'+Math.round(v)+'</text>';\n"
      "}\n"
      "for(var i=0;i<=4;i++){\n"
      "  var bits=TB*i/4, x=(PL+pw*i/4)|0;\n"
      "  s+='<text x=\"'+x+'\" y=\"'+(PT+ph+15)+'\" text-anchor=\"middle\" '\n"
      "    +'font-size=\"9\" fill=\"var(--fg3)\">'+Math.round(bits/8)+' B</text>';\n"
      "}\n"
      "var d='M'+PL+','+Y(0).toFixed(1);\n"
      "for(var k=0;k<n;k++) d+=' L'+X(k).toFixed(1)+','+Y(OS[k]).toFixed(1);\n"
      "s+='<path d=\"'+d+' L'+(PL+pw)+','+(PT+ph)+' L'+PL+','+(PT+ph)\n"
      "  +' Z\" fill=\"#22d3ee\" fill-opacity=\".08\"/>';\n"
      "s+='<path d=\"'+d+'\" fill=\"none\" stroke=\"#22d3ee\" '\n"
      "  +'stroke-width=\"1.5\" stroke-linejoin=\"round\"/>';\n"
      "s+='<line id=\"occ-scrub\" class=\"scrub-line\" x1=\"0\" y1=\"'+PT\n"
      "  +'\" x2=\"0\" y2=\"'+(PT+ph)+'\"/>';\n"
      "s+='</svg>';\n"
      "var chart=document.getElementById('occ-chart');\n"
      "chart.innerHTML=s;\n"
      "var tip=document.getElementById('occ-tip');\n"
      "function hide(){\n"
      "  var l=document.getElementById('occ-scrub');\n"
      "  if(l) l.setAttribute('opacity','0');\n"
      "  tip.style.display='none';\n"
      "}\n"
      "chart.addEventListener('mousemove',function(e){\n"
      "  var svg=chart.querySelector('svg'); if(!svg) return;\n"
      "  var r=svg.getBoundingClientRect();\n"
      "  var px=(e.clientX-r.left)/r.width*W;\n"
      "  if(px<PL||px>PL+pw){hide();return;}\n"
      "  var bits=(px-PL)/pw*TB;\n"
      "  var k=Math.round(bits/OI)-1;\n"
      "  if(k<0)k=0; if(k>=n)k=n-1;\n"
      "  var occ=OS[k];\n"
      "  var prev=k>0?OS[k-1]:0;\n"
      "  var span=k>0?bitsAt(k)-bitsAt(k-1):bitsAt(0);\n"
      "  var rate=span>0?(occ-prev)/span:0;\n"
      "  var l=document.getElementById('occ-scrub');\n"
      "  if(l){\n"
      "    var x=X(k);\n"
      "    l.setAttribute('x1',x); l.setAttribute('x2',x);\n"
      "    l.setAttribute('opacity','0.5');\n"
      "  }\n"
      "  tip.innerHTML='<div class=\"tip-row\"><span style=\"color:var(--fg3)\">at</span>'\n"
      "    +'<span style=\"color:var(--fg)\">'+Math.round(bitsAt(k)/8)+' B in</span></div>'\n"
      "    +'<div class=\"tip-row\"><span style=\"color:var(--fg3)\">slots</span>'\n"
      "    +'<span style=\"color:#22d3ee;font-weight:600\">'+occ+'</span></div>'\n"
      "    +'<div class=\"tip-row\"><span style=\"color:var(--fg3)\">of table</span>'\n"
      "    +'<span>'+(100*occ/HT).toFixed(2)+'%</span></div>'\n"
      "    +'<div class=\"tip-row\"><span style=\"color:var(--fg3)\">new/bit</span>'\n"
      "    +'<span>'+rate.toFixed(2)+'</span></div>';\n"
      "  var wr=chart.parentNode.getBoundingClientRect();\n"
      "  var tx=(e.clientX-wr.left)+12, ty=(e.clientY-wr.top)-40;\n"
      "  if(tx+170>wr.width) tx=(e.clientX-wr.left)-170;\n"
      "  if(ty<0) ty=(e.clientY-wr.top)+16;\n"
      "  tip.style.left=tx+'px'; tip.style.top=ty+'px';\n"
      "  tip.style.display='block';\n"
      "});\n"
      "chart.addEventListener('mouseleave',hide);\n"
      "})();</script>\n");

    fprintf(f, "</div>\n\n");
  }

  /* ── Counter Saturation ── */
  {
    unsigned int total_sat =
      s->sat_lopsided + s->sat_strong + s->sat_balanced + s->sat_mixed;

    fprintf(f,
      "<div class=\"card\" id=\"sec-sat\">\n"
      "<h2>Counter Saturation</h2>\n"
      "<p class=\"desc\">Probability counter balance distribution.</p>\n"
      "<table><tr><th>Category</th><th class=\"r\">Count</th>"
      "<th class=\"r\">%%</th><th>Distribution</th></tr>\n");

    struct { const char *name; unsigned int count; const char *bcls; } cats[] = {
      {"Lopsided (one=0)", s->sat_lopsided, "bar-grn"},
      {"Strong (&gt;4:1)", s->sat_strong, "bar-acc"},
      {"Balanced (&lt;2:1)", s->sat_balanced, "bar-ylw"},
      {"Mixed (2:1\xe2\x80\x93" "4:1)", s->sat_mixed, "bar-orn"},
    };
    unsigned int max_sat = 0;
    for (int i = 0; i < 4; i++)
      if (cats[i].count > max_sat) max_sat = cats[i].count;

    for (int i = 0; i < 4; i++) {
      if (cats[i].count == 0) continue;
      double pct = total_sat ? 100.0 * cats[i].count / total_sat : 0;
      double bar_w = max_sat ? 100.0 * cats[i].count / max_sat : 0;
      fprintf(f,
        "<tr><td class=\"n\">%s</td>"
        "<td class=\"r\">%u</td><td class=\"r\">%.1f</td>"
        "<td class=\"bar-cell\">"
        "<div class=\"bar %s\" style=\"width:%.0f%%;opacity:.2\"></div>"
        "<span class=\"bar-label\">&nbsp;</span></td></tr>\n",
        cats[i].name, cats[i].count, pct, cats[i].bcls, bar_w);
    }
    fprintf(f, "</table></div>\n\n");
  }

  fprintf(f, "</div><!-- grid -->\n");

  /* footer */
  {
    /* party-mode sine-wave scroller: one <i> per char, phase-shifted so the
       marquee undulates. Hidden unless body.party is set. */
    static const char scrolltxt[] =
      "*** PAQ REPORT *** A CONTEXT-MIXING CRUNCHER *** SIZE IS EVERYTHING "
      "*** GREETINGS TO CRINKLER . FARBRAUSCH . CONSPIRACY . MERCURY . "
      "LOGICOMA . TBC *** KEEP IT UNDER 4K *** ";
    fprintf(f, "<div class=\"scrolltext\"><span>");
    for (int i = 0; scrolltxt[i]; i++) {
      if (scrolltxt[i] == ' ')
        fprintf(f, "<i style=\"animation-delay:-%.2fs\">&nbsp;</i>",
                i * 0.07);
      else
        fprintf(f, "<i style=\"animation-delay:-%.2fs\">%c</i>",
                i * 0.07, scrolltxt[i]);
    }
    fprintf(f, "</span></div>\n");
  }

  fprintf(f,
    "<div style=\"margin-top:32px;padding-top:16px;border-top:1px solid var(--bdr);"
    "font-size:11px;color:var(--fg3);text-align:center\">"
    "Generated by context-mixing arithmetic compressor");
  if (timebuf[0])
    fprintf(f, " &middot; %s", timebuf);
  if (g_cmdline[0]) {
    fprintf(f, " &middot; <span style=\"font-family:var(--mono)\">");
    fputs_html(g_cmdline, f);
    fprintf(f, "</span>");
  }
  fprintf(f, "</div>\n");

  fprintf(f, "</div><!-- wrap -->\n"
    "<script>\n"
    "(function(){\n"
    "  var links=document.querySelectorAll('#sidebar a');\n"
    "  var sections=[];\n"
    "  links.forEach(function(a){\n"
    "    var id=a.getAttribute('href').slice(1);\n"
    "    var el=document.getElementById(id);\n"
    "    if(el) sections.push({el:el,link:a});\n"
    "  });\n"
    "  function update(){\n"
    "    var scrollY=window.scrollY||window.pageYOffset;\n"
    "    var current=null;\n"
    "    for(var i=0;i<sections.length;i++){\n"
    "      if(sections[i].el.offsetTop<=scrollY+80) current=i;\n"
    "    }\n"
    "    links.forEach(function(a){a.classList.remove('active')});\n"
    "    if(current!==null) sections[current].link.classList.add('active');\n"
    "  }\n"
    "  window.addEventListener('scroll',update,{passive:true});\n"
    "  update();\n"
    "})();\n"
    "/* theme toggle: applies + persists light/dark in localStorage */\n"
    "(function(){\n"
    "  var btn=document.getElementById('theme-toggle');\n"
    "  var icon=document.getElementById('theme-toggle-icon');\n"
    "  var lbl=document.getElementById('theme-toggle-label');\n"
    "  function apply(theme){\n"
    "    if(theme==='light'){\n"
    "      document.body.classList.add('light');\n"
    "      icon.textContent='\\u2600';\n"
    "      lbl.textContent='Dark';\n"
    "    } else {\n"
    "      document.body.classList.remove('light');\n"
    "      icon.textContent='\\u263d';\n"
    "      lbl.textContent='Light';\n"
    "    }\n"
    "  }\n"
    "  var saved='dark';\n"
    "  try { saved=localStorage.getItem('report-theme')||'dark'; } catch(e){}\n"
    "  apply(saved);\n"
    "  var spamN=0,spamT=0;\n"
    "  btn.addEventListener('click',function(){\n"
    "    var cur=document.body.classList.contains('light')?'light':'dark';\n"
    "    var nxt=cur==='light'?'dark':'light';\n"
    "    apply(nxt);\n"
    "    try { localStorage.setItem('report-theme',nxt); } catch(e){}\n"
    "    var now=Date.now();\n"
    "    if(now-spamT>4000) spamN=0;\n"
    "    spamT=now; spamN++;\n"
    "    if(spamN>=6&&window.partyToast\n"
    "       &&document.body.classList.contains('party')){\n"
    "      window.partyToast('PICK A SIDE');\n"
    "      spamN=0;\n"
    "    }\n"
    "  });\n"
    "})();\n"
    "/* restore state from URL hash, then bind keyboard shortcuts */\n"
    "(function(){\n"
    "  var s=window.parseReportState();\n"
    "  window.reportStateSuppress=true;\n"
    "  try {\n"
    "    if(s.sort && window.pmSortBy){\n"
    "      var sp=s.sort.split(',');\n"
    "      window.pmSortBy(parseInt(sp[0]),parseInt(sp[1]));\n"
    "    }\n"
    "    if(s.m!==undefined && window.attrSetHilite){\n"
    "      window.attrSetHilite(s.m);\n"
    "      if(s.mode==='worst') window.attrSetHilite(s.m);\n"
    "    }\n"
    "  } catch(e) { console.error('state restore failed:', e); }\n"
    "  window.reportStateSuppress=false;\n"
    "  /* keyboard shortcuts */\n"
    "  document.addEventListener('keydown',function(e){\n"
    "    var t=e.target;\n"
    "    if(t.tagName==='INPUT'||t.tagName==='TEXTAREA') return;\n"
    "    if(e.ctrlKey||e.altKey||e.metaKey) return;\n"
    "    if(e.key==='Escape'){\n"
    "      if(window.attrSetHilite && window.reportState.m!==null){\n"
    "        var m=window.reportState.m;\n"
    "        if(window.reportState.mode==='best'){\n"
    "          window.attrSetHilite(m); window.attrSetHilite(m);\n"
    "        } else if(window.reportState.mode==='worst'){\n"
    "          window.attrSetHilite(m);\n"
    "        }\n"
    "      }\n"
    "      e.preventDefault();\n"
    "    } else if(e.key==='['||e.key===']'){\n"
    "      if(!window.attrSetHilite||typeof MI==='undefined') return;\n"
    "      var dir=e.key===']'?1:-1, n=MI.length;\n"
    "      var cur=window.reportState.m;\n"
    "      var nxt=(cur===null||cur==='-1')\n"
    "        ?(dir>0?0:n-1)\n"
    "        :((parseInt(cur)+dir+n)%%n);\n"
    "      /* if the cycle would land on same model in cycle, advance again */\n"
    "      if(String(nxt)===String(cur)){\n"
    "        /* same model: clear first then set */\n"
    "        if(window.reportState.mode==='best'){\n"
    "          window.attrSetHilite(cur); window.attrSetHilite(cur);\n"
    "        }\n"
    "      }\n"
    "      window.attrSetHilite(String(nxt));\n"
    "      e.preventDefault();\n"
    "    } else if(e.key>='0'&&e.key<='9'){\n"
    "      if(!window.attrSetHilite||typeof MI==='undefined') return;\n"
    "      var n=parseInt(e.key);\n"
    "      if(n<MI.length){\n"
    "        /* set to (n, best): need to clear any existing state first */\n"
    "        if(window.reportState.m!==null && window.reportState.m!==String(n)){\n"
    "          window.attrSetHilite(String(n));\n"
    "        } else if(window.reportState.m===null){\n"
    "          window.attrSetHilite(String(n));\n"
    "        }\n"
    "        e.preventDefault();\n"
    "      }\n"
    "    }\n"
    "  });\n"
    "})();\n"
    "/* party mode: Konami code (keyboard) or 7 quick taps on the ring */\n"
    "(function(){\n"
    "  var brand=document.querySelector('.sb-brand span:not(.dot)');\n"
    "  var brandOrig=brand?brand.textContent:'';\n"
    "  var toast=null;\n"
    "  function showToast(msg){\n"
    "    if(toast) toast.remove();\n"
    "    toast=document.createElement('div');\n"
    "    toast.id='party-toast';\n"
    "    toast.textContent=msg;\n"
    "    document.body.appendChild(toast);\n"
    "    setTimeout(function(){if(toast){toast.remove();toast=null;}},1600);\n"
    "  }\n"
    "  /* chiptune: square-wave arpeggio over Am F C G, WebAudio */\n"
    "  var actx=null,gainM=null,seqTimer=null,muted=false,mstep=0,nextT=0;\n"
    "  var CHORDS=[[57,60,64],[53,57,60],[48,52,55],[55,59,62]];\n"
    "  function mf(n){return 440*Math.pow(2,(n-69)/12);}\n"
    "  function schedNote(step,t){\n"
    "    var ch=CHORDS[(step>>4)&3];\n"
    "    var o=actx.createOscillator(),g=actx.createGain();\n"
    "    o.type='square';o.frequency.value=mf(ch[step%%3]+24);\n"
    "    g.gain.setValueAtTime(.04,t);\n"
    "    g.gain.exponentialRampToValueAtTime(.001,t+.12);\n"
    "    o.connect(g);g.connect(gainM);o.start(t);o.stop(t+.13);\n"
    "    if(step%%4===0){\n"
    "      var b=actx.createOscillator(),bg=actx.createGain();\n"
    "      b.type='square';b.frequency.value=mf(ch[0]-12);\n"
    "      bg.gain.setValueAtTime(.05,t);\n"
    "      bg.gain.exponentialRampToValueAtTime(.001,t+.22);\n"
    "      b.connect(bg);bg.connect(gainM);b.start(t);b.stop(t+.23);\n"
    "    }\n"
    "  }\n"
    "  /* lookahead scheduler: notes are queued ~250ms ahead on the audio\n"
    "     clock, so main-thread jank (scrolling) cannot shift their timing */\n"
    "  function tick(){\n"
    "    if(!actx) return;\n"
    "    var now=actx.currentTime;\n"
    "    if(nextT<now) nextT=now+.05; /* resync after a long stall */\n"
    "    while(nextT<now+.25){\n"
    "      if(!muted) schedNote(mstep,nextT);\n"
    "      mstep++;\n"
    "      nextT+=.125;\n"
    "    }\n"
    "  }\n"
    "  function ensureAudio(){\n"
    "    try{\n"
    "      if(!actx){\n"
    "        actx=new (window.AudioContext||window.webkitAudioContext)();\n"
    "        gainM=actx.createGain();gainM.gain.value=.5;\n"
    "        gainM.connect(actx.destination);\n"
    "      }\n"
    "      if(actx.state==='suspended') actx.resume();\n"
    "    }catch(e){}\n"
    "  }\n"
    "  function blip(fr,g,dur){\n"
    "    if(!actx||muted) return;\n"
    "    try{\n"
    "      var t=actx.currentTime;\n"
    "      var o=actx.createOscillator(),og=actx.createGain();\n"
    "      o.type='square';o.frequency.value=fr;\n"
    "      og.gain.setValueAtTime(g,t);\n"
    "      og.gain.exponentialRampToValueAtTime(.001,t+dur);\n"
    "      o.connect(og);og.connect(gainM);o.start(t);o.stop(t+dur+.01);\n"
    "    }catch(e){}\n"
    "  }\n"
    "  function startMusic(){\n"
    "    ensureAudio();\n"
    "    if(!actx) return;\n"
    "    mstep=0;\n"
    "    nextT=actx.currentTime+.06;\n"
    "    if(!seqTimer) seqTimer=setInterval(tick,40);\n"
    "  }\n"
    "  function stopMusic(){\n"
    "    if(seqTimer){clearInterval(seqTimer);seqTimer=null;}\n"
    "    if(actx){try{actx.suspend();}catch(e){}}\n"
    "  }\n"
    "  var titleOrig=document.title,titleTimer=null;\n"
    "  function startTitle(){\n"
    "    if(titleTimer) return;\n"
    "    var fl=0;\n"
    "    titleTimer=setInterval(function(){\n"
    "      fl^=1;\n"
    "      document.title=fl?'*** PARTY MODE ***':'\\u259A\\u259E PAQ REPORT';\n"
    "    },700);\n"
    "  }\n"
    "  function stopTitle(){\n"
    "    if(titleTimer){clearInterval(titleTimer);titleTimer=null;}\n"
    "    document.title=titleOrig;\n"
    "  }\n"
    "  /* decode-scramble: headings dissolve into glyphs, resolve left to "
    "right */\n"
    "  /* the stamp is a slot machine: the classic half the time, else a\n"
    "     random pull from the rejected-candidates archive */\n"
    "  var STAMPS=[\n"
    "    '*** FUCK YOU SHANNON PAQ BEGS TO DIFFER ***',\n"
    "    '*** ENTROPY IS A SUGGESTION ***',\n"
    "    '*** SORRY, SHANNON ***',\n"
    "    '*** ILLEGAL ENTROPY DENSITY ***',\n"
    "    '*** BITS BEG FOR MERCY ***',\n"
    "    '*** KOLMOGOROV APPROVED ***',\n"
    "    '*** ANOTHER BYTE BITES THE DUST ***'];\n"
    "  var scrTimer=null,scrSaved=null;\n"
    "  function scrambleRestore(){\n"
    "    if(scrTimer){clearInterval(scrTimer);scrTimer=null;}\n"
    "    if(scrSaved){\n"
    "      scrSaved.forEach(function(p){p[0].textContent=p[1];});\n"
    "      scrSaved=null;\n"
    "    }\n"
    "  }\n"
    "  function scrambleIn(){\n"
    "    scrambleRestore();\n"
    "    var nodes=[].slice.call(document.querySelectorAll('.card h2'));\n"
    "    var h1=document.querySelector('.hero-info h1');\n"
    "    if(h1) nodes.unshift(h1);\n"
    "    scrSaved=nodes.map(function(n){return [n,n.textContent];});\n"
    "    var GLYPH='#$&@!01<>*+=?';\n"
    "    var step=0,steps=14;\n"
    "    scrTimer=setInterval(function(){\n"
    "      step++;\n"
    "      scrSaved.forEach(function(pair){\n"
    "        var orig=pair[1];\n"
    "        var reveal=Math.floor(orig.length*step/steps);\n"
    "        var out=orig.slice(0,reveal);\n"
    "        for(var j=reveal;j<orig.length;j++)\n"
    "          out+=orig[j]===' '?' ':GLYPH[(Math.random()*GLYPH.length)|0];\n"
    "        pair[0].textContent=out;\n"
    "      });\n"
    "      if(step>=steps) scrambleRestore();\n"
    "    },45);\n"
    "  }\n"
    "  /* fake cracktro loader shown before entering party */\n"
    "  var loading=false;\n"
    "  function decrunch(cb){\n"
    "    loading=true;\n"
    "    ensureAudio();\n"
    "    var ov=document.createElement('div');\n"
    "    ov.id='decrunch';\n"
    "    ov.innerHTML='LOADING PART 2 OF 1<div class=\"bar\"></div>';\n"
    "    document.body.appendChild(ov);\n"
    "    var bar=ov.querySelector('.bar');\n"
    "    var i=0;\n"
    "    var iv=setInterval(function(){\n"
    "      i++;\n"
    "      bar.textContent=new Array(i+1).join('\\u2588');\n"
    "      blip(180+i*55,.03,.05);\n"
    "      if(i>=16){\n"
    "        clearInterval(iv);\n"
    "        setTimeout(function(){ov.remove();loading=false;cb();},180);\n"
    "      }\n"
    "    },55);\n"
    "  }\n"
    "  function applyParty(on,silent,msg){\n"
    "    document.body.classList.toggle('party',on);\n"
    "    if(brand) brand.textContent=on?'paq report':brandOrig;\n"
    "    document.body.classList.add('party-boot');\n"
    "    setTimeout(function(){document.body.classList.remove('party-boot');},"
    "600);\n"
    "    if(on){\n"
    "      var st=Math.random()<.5?STAMPS[0]\n"
    "        :STAMPS[1+((Math.random()*(STAMPS.length-1))|0)];\n"
    "      document.body.style.setProperty('--stamp','\"'+st+'\"');\n"
    "      startMusic();startTitle();scrambleIn();\n"
    "    }\n"
    "    else{stopMusic();stopTitle();scrambleRestore();}\n"
    "    if(!silent){\n"
    "      if(on&&msg) showToast(msg);\n"
    "      else if(!on) showToast('PARTY OVER');\n"
    "    }\n"
    "  }\n"
    "  function setParty(on,silent,msg){\n"
    "    if(loading) return;\n"
    "    if(on&&!silent) decrunch(function(){applyParty(on,silent,msg);});\n"
    "    else applyParty(on,silent,msg);\n"
    "  }\n"
    "  /* party toys: cursor sparkle trail + click pixel burst */\n"
    "  var COLS=['#ff3fd8','#22d3ee','#ffe33f'];\n"
    "  var lastSpark=0;\n"
    "  document.addEventListener('mousemove',function(e){\n"
    "    if(!document.body.classList.contains('party')) return;\n"
    "    var now=Date.now();\n"
    "    if(now-lastSpark<40) return;\n"
    "    lastSpark=now;\n"
    "    var s=document.createElement('span');\n"
    "    s.className='pspark';\n"
    "    s.style.background=COLS[(Math.random()*3)|0];\n"
    "    s.style.left=(e.clientX+((Math.random()*10)|0)-5)+'px';\n"
    "    s.style.top=(e.clientY+((Math.random()*10)|0)-5)+'px';\n"
    "    document.body.appendChild(s);\n"
    "    setTimeout(function(){s.remove();},520);\n"
    "  });\n"
    "  document.addEventListener('click',function(e){\n"
    "    if(!document.body.classList.contains('party')) return;\n"
    "    for(var j=0;j<8;j++){\n"
    "      var s=document.createElement('span');\n"
    "      s.className='pburst';\n"
    "      s.style.background=COLS[j%%3];\n"
    "      s.style.left=e.clientX+'px';\n"
    "      s.style.top=e.clientY+'px';\n"
    "      var a=j*.785+Math.random()*.5;\n"
    "      s.style.setProperty('--dx',(Math.cos(a)*36).toFixed(0)+'px');\n"
    "      s.style.setProperty('--dy',(Math.sin(a)*36).toFixed(0)+'px');\n"
    "      document.body.appendChild(s);\n"
    "      (function(el){setTimeout(function(){el.remove();},480);})(s);\n"
    "    }\n"
    "    blip(380+Math.random()*420,.03,.06);\n"
    "  });\n"
    "  var seq=['ArrowUp','ArrowUp','ArrowDown','ArrowDown',"
    "'ArrowLeft','ArrowRight','ArrowLeft','ArrowRight','b','a'];\n"
    "  var pos=0,buf='';\n"
    "  document.addEventListener('keydown',function(e){\n"
    "    var t=e.target;\n"
    "    if(t.tagName==='INPUT'||t.tagName==='TEXTAREA') return;\n"
    "    var k=e.key.length===1?e.key.toLowerCase():e.key;\n"
    "    if(k==='m'&&document.body.classList.contains('party')){\n"
    "      muted=!muted;showToast(muted?'MUSIC OFF':'MUSIC ON');return;\n"
    "    }\n"
    "    if(e.key.length===1){\n"
    "      buf=(buf+k).slice(-5);\n"
    "      if(buf==='iddqd'){\n"
    "        buf='';\n"
    "        setParty(!document.body.classList.contains('party'),false,\n"
    "          '*** DEGREELESSNESS MODE ***');\n"
    "        return;\n"
    "      }\n"
    "      if(document.body.classList.contains('party')){\n"
    "        if(buf==='xyzzy'){buf='';showToast('NOTHING HAPPENS');return;}\n"
    "        if(buf==='idkfa'){buf='';showToast('AMMO NOT INCLUDED');return;}\n"
    "        if(buf.slice(-4)==='e2e4'){buf='';\n"
    "          showToast('BEST BY TEST');return;}\n"
    "        if(buf==='uciok'){buf='';showToast('READYOK');return;}\n"
    "        if(buf.slice(-4)==='guru'){buf='';showGuru();return;}\n"
    "        if(buf.slice(-4)==='sudo'){buf='';\n"
    "          showToast('THIS INCIDENT WILL BE REPORTED');return;}\n"
    "        if(buf.slice(-3)==='man'){buf='';\n"
    "          showToast('FIGURE IT OUT');return;}\n"
    "      }\n"
    "    }\n"
    "    pos=(k===seq[pos])?pos+1:(k===seq[0]?1:0);\n"
    "    if(pos===seq.length){\n"
    "      pos=0;\n"
    "      setParty(!document.body.classList.contains('party'));\n"
    "    }\n"
    "  });\n"
    "  window.partyToast=showToast;\n"
    "  /* the amiga remembers */\n"
    "  function showGuru(){\n"
    "    if(document.getElementById('guru')) return;\n"
    "    var g=document.createElement('div');\n"
    "    g.id='guru';\n"
    "    g.innerHTML='<div class=\"box\">Software Failure.&nbsp;&nbsp;"
    "Press left mouse button to continue.<br>"
    "Guru Meditation #00504151.00000004</div>';\n"
    "    g.addEventListener('click',function(){g.remove();});\n"
    "    document.body.appendChild(g);\n"
    "  }\n"
    "  /* select the man's name and see */\n"
    "  var shFired=false,shT=null;\n"
    "  document.addEventListener('selectionchange',function(){\n"
    "    if(shT) clearTimeout(shT);\n"
    "    shT=setTimeout(function(){\n"
    "      if(!document.body.classList.contains('party')){\n"
    "        shFired=false;return;\n"
    "      }\n"
    "      var s='';\n"
    "      try{s=String(window.getSelection());}catch(e){}\n"
    "      if(s.trim().toLowerCase()==='shannon'){\n"
    "        if(!shFired){shFired=true;showToast('HE CAN\\'T HEAR YOU');}\n"
    "      } else shFired=false;\n"
    "    },350);\n"
    "  });\n"
    "  /* for those who open the console */\n"
    "  try{\n"
    "    console.log('%%c PAQ %%c the old gods answer to iddqd',\n"
    "      'background:#ff3fd8;color:#000;font-family:monospace;'\n"
    "      +'font-weight:bold;padding:2px 6px',\n"
    "      'color:#22d3ee;font-family:monospace');\n"
    "  }catch(e){}\n"
    "  /* for those who click the numbers */\n"
    "  /* shown only when the actual sky agrees */\n"
    "  var moon=document.getElementById('moon');\n"
    "  if(moon){\n"
    "    var syn=29.530588853;\n"
    "    var ph=((Date.now()/864e5-10962.76)%%syn+syn)%%syn;\n"
    "    if(Math.abs(ph-14.765)<0.75) moon.classList.add('lit');\n"
    "  }\n"
    "  var h0stat=document.querySelector('.hero-stats .hero-stat:last-child');\n"
    "  if(h0stat){\n"
    "    h0stat.addEventListener('click',function(){\n"
    "      if(!document.body.classList.contains('party')) return;\n"
    "      showToast('ORDER-0 IS FOR COWARDS');\n"
    "    });\n"
    "  }\n"
    "  /* party-only toys: kick the cube, reverse the scroller */\n"
    "  var glz=document.getElementById('glenz'),glzN=0,feedN=0;\n"
    "  if(glz){\n"
    "    /* the cube can be carried; it does not mind */\n"
    "    var dragOn=false,dragged=false,gdx=0,gdy=0,gsx=0,gsy=0;\n"
    "    glz.addEventListener('pointerdown',function(e){\n"
    "      if(!document.body.classList.contains('party')) return;\n"
    "      if(glz.classList.contains('dead')) return;\n"
    "      dragOn=true;gsx=e.clientX;gsy=e.clientY;\n"
    "      var r=glz.getBoundingClientRect();\n"
    "      gdx=e.clientX-r.left;gdy=e.clientY-r.top;\n"
    "      e.preventDefault();\n"
    "    });\n"
    "    document.addEventListener('pointermove',function(e){\n"
    "      if(!dragOn) return;\n"
    "      if(Math.abs(e.clientX-gsx)+Math.abs(e.clientY-gsy)>4)\n"
    "        dragged=true;\n"
    "      if(!dragged) return;\n"
    "      glz.style.left=(e.clientX-gdx)+'px';\n"
    "      glz.style.top=(e.clientY-gdy)+'px';\n"
    "      glz.style.right='auto';glz.style.bottom='auto';\n"
    "    });\n"
    "    function cubeHits(el){\n"
    "      if(!el) return false;\n"
    "      var a=el.getBoundingClientRect();\n"
    "      var c=glz.getBoundingClientRect();\n"
    "      return !(c.right<a.left||c.left>a.right||\n"
    "               c.bottom<a.top||c.top>a.bottom);\n"
    "    }\n"
    "    document.addEventListener('pointerup',function(){\n"
    "      if(dragOn&&dragged&&document.body.classList.contains('party')){\n"
    "        if(ring&&cubeHits(ring)){\n"
    "          showToast('SQUARING THE CIRCLE');\n"
    "          ring.classList.add('docked');\n"
    "          setTimeout(function(){ring.classList.remove('docked');},1000);\n"
    "          blip(520,.05,.2);\n"
    "        } else if(cubeHits(document.getElementById('bfreq-svg'))){\n"
    "          if(feedN>=8){\n"
    "            showToast('NO THANK YOU');\n"
    "          } else {\n"
    "            var bf=document.getElementById('bfreq-svg');\n"
    "            var cells=bf.querySelectorAll('rect[data-b]');\n"
    "            if(cells.length){\n"
    "              feedN++;\n"
    "              var c2=cells[(Math.random()*cells.length)|0];\n"
    "              var bv=+c2.getAttribute('data-b');\n"
    "              c2.style.stroke='#ffe33f';\n"
    "              c2.style.strokeWidth='3';\n"
    "              setTimeout(function(){\n"
    "                c2.style.stroke='';c2.style.strokeWidth='';\n"
    "              },700);\n"
    "              if(feedN===8){\n"
    "                showToast('THE CUBE IS FULL');\n"
    "                glz.style.width='82px';\n"
    "                glz.style.height='82px';\n"
    "              } else {\n"
    "                showToast('YUM. 0x'+(bv<16?'0':'')\n"
    "                  +bv.toString(16).toUpperCase());\n"
    "              }\n"
    "              blip(70,.06,.25);\n"
    "            }\n"
    "          }\n"
    "        }\n"
    "      }\n"
    "      dragOn=false;\n"
    "      setTimeout(function(){dragged=false;},0);\n"
    "    });\n"
    "    glz.addEventListener('click',function(){\n"
    "      if(!document.body.classList.contains('party')) return;\n"
    "      if(dragged) return;\n"
    "      if(glz.classList.contains('dead')) return;\n"
    "      glzN++;\n"
    "      if(glzN>=100){\n"
    "        glzN=0;\n"
    "        glz.classList.remove('kick');\n"
    "        glz.classList.add('dead');\n"
    "        showToast('YOU MONSTER');\n"
    "        blip(90,.05,.4);\n"
    "        setTimeout(function(){glz.classList.remove('dead');},10000);\n"
    "        return;\n"
    "      }\n"
    "      glz.classList.add('kick');\n"
    "      setTimeout(function(){glz.classList.remove('kick');},1500);\n"
    "      blip(700+Math.random()*300,.04,.08);\n"
    "      if(glzN===10) showToast('THE CUBE IS PLEASED');\n"
    "    });\n"
    "  }\n"
    "  var scr=document.querySelector('.scrolltext');\n"
    "  if(scr){\n"
    "    scr.addEventListener('click',function(){\n"
    "      var sp=scr.querySelector('span');\n"
    "      if(!sp) return;\n"
    "      sp.style.animationDirection=\n"
    "        sp.style.animationDirection==='reverse'?'':'reverse';\n"
    "      blip(240,.04,.09);\n"
    "    });\n"
    "    /* three full loops, actually watched */\n"
    "    var spEl=scr.querySelector('span');\n"
    "    var scrVis=false,scrLoops=0,scrDone=false;\n"
    "    try{\n"
    "      new IntersectionObserver(function(en){\n"
    "        scrVis=en[0].isIntersecting;\n"
    "        if(!scrVis) scrLoops=0;\n"
    "      },{threshold:.9}).observe(scr);\n"
    "    }catch(e){}\n"
    "    if(spEl) spEl.addEventListener('animationiteration',function(e){\n"
    "      if(e.animationName!=='scrolltx') return;\n"
    "      if(!document.body.classList.contains('party')) return;\n"
    "      if(!scrVis){scrLoops=0;return;}\n"
    "      scrLoops++;\n"
    "      if(scrLoops>=3&&!scrDone){\n"
    "        scrDone=true;\n"
    "        showToast('YOU ACTUALLY READ IT ALL');\n"
    "      }\n"
    "    });\n"
    "  }\n"
    "  var ring=document.querySelector('.hero-ring');\n"
    "  if(ring){\n"
    "    var taps=0,last=0;\n"
    "    ring.addEventListener('click',function(){\n"
    "      var now=Date.now();\n"
    "      if(now-last>900) taps=0;\n"
    "      last=now; taps++;\n"
    "      if(taps>=7){\n"
    "        taps=0;\n"
    "        setParty(!document.body.classList.contains('party'));\n"
    "      }\n"
    "    });\n"
    "  }\n"
    "})();\n"
    "</script>\n"
    "</body></html>\n");
  fclose(f);
}

static unsigned char *read_file(const char *path, int *out_size) {
  FILE *f = fopen(path, "rb");
  if (!f)
    return NULL;
  fseek(f, 0, SEEK_END);
  int sz = (int)ftell(f);
  fseek(f, 0, SEEK_SET);
  unsigned char *buf = (unsigned char *)malloc(sz);
  if ((int)fread(buf, 1, sz, f) != sz) {
    free(buf);
    fclose(f);
    return NULL;
  }
  fclose(f);
  *out_size = sz;
  return buf;
}

static int write_file(const char *path, const void *data, int size) {
  FILE *f = fopen(path, "wb");
  if (!f)
    return 0;
  fwrite(data, 1, size, f);
  fclose(f);
  return 1;
}

static int parse_models(const char *str, ModelSet *ml) {
  ml->num_models = 0;
  ml->size = 0;
  const char *p = str;
  while (*p && ml->num_models < MAX_SEARCH) {
    while (*p == ' ' || *p == ',')
      p++;
    if (!*p)
      break;
    unsigned int mask;
    unsigned int weight;
    int consumed = 0;
    if (sscanf(p, "%x:%u%n", &mask, &weight, &consumed) < 2)
      return -1;
    if (mask > 255 || weight > MAX_WEIGHT)
      return -1;
    ml->models[ml->num_models].mask = (unsigned char)mask;
    ml->models[ml->num_models].weight = (unsigned char)weight;
    ml->num_models++;
    p += consumed;
  }
  return ml->num_models;
}

static void print_usage(const char *prog) {
  printf("Usage: %s [options] <input_file>\n\nOptions:\n", prog);
  printf("  -o <file>    Output file (default: <input>.paq or <input>.bin)\n");
  printf("  -d           Decompress mode\n");
  printf("  -k <n>       Search beam width (default: %d). >1 also accepts\n",
         DEFAULT_BEAM);
  printf("               non-improving mask additions during search\n");
  printf(
      "  -s           Simple search: skip per-candidate weight optimization\n");
  printf("  -m <models>  Use explicit models, skip search (e.g. \"00:1 80:2 "
         "C0:3\")\n");
  printf("  -w           Optimize weights on explicit models from -m\n");
  printf("  -b <n>       Base probability (default: %d)\n", DEFAULT_BPROB);
  printf("  -e           Extreme: use real compression during search\n");
  printf("  -H <bits>    Direct-mapped table size = 2^bits 2-byte slots\n");
  printf("               (default 24); loader DIRECT_BITS must match\n");
  printf(
      "  -L           Large mode: 32-bit header bitlength field for inputs\n");
  printf(
      "               >~8KB. Not stored in file; pass -L to both compress\n");
  printf("               and decompress\n");
  printf("  -R <n>       Repeat encode pass n times for stable timing\n");
  printf("  -r <file>    Write HTML stats report to file\n");
  printf("  -v           Verbose output (use -vv for very verbose)\n");
  printf("  -p <n>       Max search passes (default: 1)\n");
  printf("  -h           Show this help\n");
}

int main(int argc, char *argv[]) {
  const char *output_file = NULL;
  int beam = DEFAULT_BEAM;
  int simple = 0;
  int base_prob = DEFAULT_BPROB;
  int decompress = 0;
  int max_passes = 1;
  ModelSet explicit_models = {0};
  int have_explicit_models = 0;
  int optimize_explicit_weights = 0;

  /* capture the original command line before getopt/permutation mangle it */
  {
    int pos = 0;
    g_cmdline[0] = '\0';
    for (int i = 0; i < argc; i++) {
      int n = snprintf(g_cmdline + pos, sizeof(g_cmdline) - pos, "%s%s",
                       i ? " " : "", argv[i]);
      if (n < 0 || pos + n >= (int)sizeof(g_cmdline))
        break;
      pos += n;
    }
  }

  int opt;
#ifdef _WIN32
  /* MinGW getopt stops at first non-option; permute argv so options
     can appear in any order (matches GNU getopt's default) */
  {
    const char *os = "o:m:b:p:k:H:R:r:sdwehvL";
    char **opts = (char **)malloc(sizeof(char *) * argc);
    char **pos = (char **)malloc(sizeof(char *) * argc);
    int n_opts = 0, n_pos = 0;
    for (int i = 1; i < argc; i++) {
      if (argv[i][0] == '-' && argv[i][1] != '\0') {
        opts[n_opts++] = argv[i];
        char c = argv[i][1];
        const char *p = strchr(os, c);
        /* take next argv as value only if option needs one AND value
           isn't already joined like -Ofoo */
        if (p && p[1] == ':' && argv[i][2] == '\0' && i + 1 < argc)
          opts[n_opts++] = argv[++i];
      } else {
        pos[n_pos++] = argv[i];
      }
    }
    int j = 1;
    for (int i = 0; i < n_opts; i++) argv[j++] = opts[i];
    for (int i = 0; i < n_pos; i++) argv[j++] = pos[i];
    free(opts);
    free(pos);
  }
#endif
  while ((opt = getopt(argc, argv, "o:m:b:p:k:H:R:r:sdwehvL")) != -1) {
    switch (opt) {
    case 'o':
      output_file = optarg;
      break;
    case 'd':
      decompress = 1;
      break;
    case 'k':
      beam = atoi(optarg);
      if (beam < 1) {
        fprintf(stderr, "Beam width must be >= 1\n");
        return 1;
      }
      break;
    case 's':
      simple = 1;
      break;
    case 'r':
      html_output = optarg;
      break;
    case 'm': {
      int n = parse_models(optarg, &explicit_models);
      if (n < 1) {
        fprintf(stderr, "Invalid model string: '%s'\n", optarg);
        fprintf(stderr,
                "Expected format: \"00:1 80:2 C0:3\" (hex_mask:weight)\n");
        return 1;
      }
      have_explicit_models = 1;
      break;
    }
    case 'w':
      optimize_explicit_weights = 1;
      break;
    case 'e':
      extreme = 1;
      break;
    case 'H':
      direct_bits = atoi(optarg);
      if (direct_bits < 1 || direct_bits > 30) {
        fprintf(stderr,
                "Direct-map bits must be 1..30 (table = 2^bits slots)\n");
        return 1;
      }
      break;
    case 'L':
      large_field = 1;
      break;
    case 'R':
      timing_reps = atoi(optarg);
      if (timing_reps < 1) {
        fprintf(stderr, "Reps must be >= 1\n");
        return 1;
      }
      break;
    case 'b':
      base_prob = atoi(optarg);
      if (base_prob < 9) {
        fprintf(stderr, "Base prob must be >= 9\n");
        return 1;
      }
      break;
    case 'h':
      print_usage(argv[0]);
      return 0;
    case 'v':
      verbose++;
      break;
    case 'p':
      max_passes = atoi(optarg);
      if (max_passes < 1) {
        fprintf(stderr, "Passes must be >= 1\n");
        return 1;
      }
      break;
    default:
      print_usage(argv[0]);
      return 1;
    }
  }
  if (optind >= argc) {
    fprintf(stderr, "Error: no input file specified\n");
    print_usage(argv[0]);
    return 1;
  }

  const char *input_file = argv[optind];
  int data_size;
  unsigned char *data = read_file(input_file, &data_size);
  if (!data) {
    fprintf(stderr, "Failed to read '%s'\n", input_file);
    return 1;
  }

  char default_out[512];

  if (decompress) {
    if (!output_file) {
      int len = strlen(input_file);
      if (len > 4 && !strcmp(input_file + len - 4, ".paq"))
        snprintf(default_out, sizeof(default_out), "%.*s", len - 4, input_file);
      else
        snprintf(default_out, sizeof(default_out), "%s.bin", input_file);
      output_file = default_out;
    }

    printf("Input:       %s (%d bytes)\n", input_file, data_size);
    printf("Base prob:   %d\n", base_prob);

    if (data_size < hdr_base_bytes()) {
      fprintf(stderr, "Input too small to be a valid .paq file\n");
      free(data);
      return 1;
    }

    int bitlen = 0;
    memcpy(&bitlen, data, hdr_bitlen_bytes()); /* 2 or 4 little-endian bytes */
    unsigned int wm;
    memcpy(&wm, data + hdr_bitlen_bytes(), 4);
    int num_models = wmask_num_models(wm);

    if (bitlen < 1 || hdr_base_bytes() + num_models > data_size) {
      fprintf(stderr, "Corrupt header (bitlen=%d, models=%d, filesize=%d)\n",
              bitlen, num_models, data_size);
      free(data);
      return 1;
    }

    int out_sz = (bitlen - 1) / 8;
    printf("Bitlength:   %d (%d data bytes)\n", bitlen, out_sz);
    printf("Models:      %d\n", num_models);

    unsigned char *out_data = (unsigned char *)calloc(out_sz + 16, 1);
    int decoded = decompress_4k_direct(data, out_data, base_prob);

    if (!write_file(output_file, out_data, decoded)) {
      fprintf(stderr, "Failed to open output '%s'\n", output_file);
      free(out_data);
      free(data);
      return 1;
    }
    printf("Output:      %s (%d bytes)\n", output_file, decoded);
    free(out_data);
    free(data);
    return 0;
  }

  if (!output_file) {
    snprintf(default_out, sizeof(default_out), "%s.paq", input_file);
    output_file = default_out;
  }

  int bitlen = data_size * 8 + 1;
  if (!large_field && bitlen > 65535) {
    fprintf(stderr,
            "Input too large for 16-bit bitlength (%d bits); pass -L for a "
            "32-bit field\n",
            bitlen);
    free(data);
    return 1;
  }

  printf("Input:       %s (%d bytes)\n", input_file, data_size);
  printf("Search:      beam %d%s%s\n", beam, simple ? " simple" : "",
         extreme ? " extreme" : "");
  printf("Base prob:   %d\n", base_prob);
  if (max_passes > 1 && !have_explicit_models)
    printf("Max passes:  %d\n", max_passes);

  unsigned char ctx[MAX_CTX] = {};
  int est_size = 0;
  ModelSet ml;
  double t_search = mono_sec();
  CompStats cstats = {0};

  if (have_explicit_models) {
    ml = explicit_models;
    printf("Models:      ");
    model_set_print(&ml, stdout);
    if (optimize_explicit_weights) {
      printf("Optimizing weights for %d explicit models...\n", ml.num_models);
      Evaluator eval = {0};
      CompState *cs =
          extreme ? NULL : state_new(data, data_size, base_prob, &eval, ctx);
      est_size = optimize_weights(cs, &ml, data, data_size, base_prob);
      if (!extreme) {
        state_destroy(cs);
        eval_destroy(&eval);
      }
      printf("Optimized:   ");
      model_set_print(&ml, stdout);
      printf("Estimated:   %.3f bytes\n", est_size / (float)(BIT_PREC * 8));
    } else {
      printf("Skipping search, using %d explicit models\n", ml.num_models);
    }
  } else {
    ml = search_best_models(data, data_size, ctx, beam, simple, base_prob,
                            &est_size, NULL,
                            html_output ? &cstats : NULL);

    for (int pass = 2; pass <= max_passes; pass++) {
      int prev_size = est_size;
      if (verbose)
        printf("\n  Pass %d (seeded with %d models):\n", pass, ml.num_models);
      ml = search_best_models(data, data_size, ctx, beam, simple, base_prob,
                              &est_size, &ml, NULL);
      if (est_size >= prev_size) {
        if (verbose)
          printf("  No improvement, stopping.\n");
        break;
      }
      if (verbose)
        printf("  Pass %d improved: %.1f -> %.1f bytes\n", pass,
               prev_size / (float)(BIT_PREC * 8),
               est_size / (float)(BIT_PREC * 8));
    }

    printf("\nEstimated:   %.3f bytes\n", est_size / (float)(BIT_PREC * 8));
    printf("Models:      ");
    model_set_print(&ml, stdout);
  }

  double search_ms = (mono_sec() - t_search) * 1000.0;

  int max_out = data_size + 1024;
  unsigned char *out_buf = (unsigned char *)calloc(max_out, 1);
  int comp_bits = compress_4k(data, data_size, out_buf, &ml, base_prob,
                              html_output ? &cstats : NULL);
  int comp_bytes = (comp_bits + 7) / 8;

  unsigned char ordered_masks[MAX_SEARCH];
  unsigned int wmask = encode_weight_mask(&ml, ordered_masks, 1);

  int header_bytes = hdr_base_bytes() + ml.num_models;
  int total_bytes = header_bytes + comp_bytes;
  int total_bits = header_bytes * 8 + comp_bits;

  if (verbose) {
    printf("Raw:         %d bytes %d bits (%.2f%%)\n", comp_bytes, comp_bits,
           100.0f * comp_bytes / data_size);
    printf("Weightmask:  %08X\n", wmask);
    printf("Padding:     %d bits\n", total_bytes * 8 - total_bits);
  }
  printf("Compressed:  %d bytes %d bits (%.2f%%)\n", total_bytes, total_bits,
         100.0f * total_bytes / data_size);
  printf("Table:       direct-mapped (lossy), 2^%d slots\n", direct_bits);
  printf("Search time: %.1f ms\n", search_ms);
  printf("Encode time: %.3f ms/iter (reps=%d)\n", g_encode_ms, timing_reps);

  unsigned char header[8];
  unsigned int bl = (unsigned int)bitlen;
  memcpy(header, &bl, hdr_bitlen_bytes());        /* bitlen: 2 or 4 bytes */
  memcpy(header + hdr_bitlen_bytes(), &wmask, 4); /* wmask follows bitlen */

  FILE *fout = fopen(output_file, "wb");
  if (!fout) {
    fprintf(stderr, "Failed to open output '%s'\n", output_file);
    free(out_buf);
    free(data);
    return 1;
  }
  fwrite(header, 1, hdr_base_bytes(), fout);
  fwrite(ordered_masks, 1, ml.num_models, fout);
  fwrite(out_buf, 1, comp_bytes, fout);
  fclose(fout);

  printf("Output:      %s (%d bytes)\n", output_file, total_bytes);

  if (html_output && cstats.valid) {
    snprintf(cstats.input_file, sizeof(cstats.input_file), "%s", input_file);
    cstats.input_size = data_size;
    cstats.beam = beam;
    cstats.simple = simple;
    cstats.extreme = extreme;
    cstats.base_prob = base_prob;
    cstats.large_field = large_field;
    cstats.compressed_bits = comp_bits;
    cstats.header_bytes = header_bytes;
    cstats.total_bytes = total_bytes;
    cstats.estimated_bytes = est_size / (float)(BIT_PREC * 8);
    cstats.search_ms = search_ms;
    cstats.hdr_wmask = wmask;
    memcpy(cstats.hdr_masks, ordered_masks, ml.num_models);
    cstats.hdr_bitlen = bitlen;
    model_set_sprint(&ml, cstats.model_string, sizeof(cstats.model_string));

    /* compute Shannon H0 entropy */
    unsigned int freq[256] = {};
    for (int i = 0; i < data_size; i++)
      freq[data[i]]++;
    double h0 = 0;
    for (int i = 0; i < 256; i++) {
      if (freq[i] > 0) {
        double p = (double)freq[i] / data_size;
        h0 -= p * fast_log2f((float)p);
      }
    }
    cstats.entropy = h0;
    memcpy(cstats.byte_freq, freq, sizeof(freq));

    cstats.input_data = data;
    write_html_report(html_output, &cstats);
    printf("HTML report: %s\n", html_output);
    free(cstats.byte_costs);
    free(cstats.byte_model_contrib);
    free(cstats.bit_costs);
    free(cstats.search_best);
    free(cstats.search_events);
  }

  free(out_buf);
  free(data);
  return 0;
}
