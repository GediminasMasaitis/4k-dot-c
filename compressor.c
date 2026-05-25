// Derived from Crinkler by Rune L. H. Stubbe and Aske Simon Christensen
// Original: https://github.com/runestubbe/Crinkler (zlib license)
// Rewritten and ported to Linux ELF x86-64

#include <assert.h>
#include <getopt.h>
#include <limits.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#if defined(_WIN32)
#include <malloc.h>
#endif

typedef float v4f __attribute__((vector_size(16)));
typedef uint32_t v4u __attribute__((vector_size(16)));
typedef int32_t v4i __attribute__((vector_size(16)));
typedef uint16_t v8u16 __attribute__((vector_size(16)));

#define TPREC_BITS 12
#define TPREC (1 << TPREC_BITS)
#define HMUL 111
#define MAX_CTX 8
#define DEFAULT_BPROB 10
#define BIT_PREC 256
#define MAX_MODELS 256
#define MAX_SEARCH 21
#define MAX_WEIGHT 9
#define NPV 16
#define PKG_BITS (NPV * 4)

#define DEFAULT_BEAM 1

static int verbose = 0; /* 0=off, 1=verbose, 2=very verbose */
static int extreme = 0; /* use real compression instead of estimator */

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
  unsigned int hash;
  unsigned char prob[2];
} HashEntry;

typedef struct {
  unsigned int *hashes;
  unsigned int hashes_len, hashes_cap;
  unsigned char *bits;
  int bits_len, bits_cap;
  int *weights;
  int num_weights;
  unsigned int table_size;
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

  int compressed_bits;
  int header_bytes;
  int total_bytes;
  float estimated_bytes;

  int num_models;
  unsigned char model_masks[MAX_SEARCH];
  int model_weights[MAX_SEARCH];
  char model_string[512];

  unsigned int model_hits[MAX_SEARCH];
  unsigned int model_misses[MAX_SEARCH];
  double model_bits_saved[MAX_SEARCH];

  unsigned int conf_hist[11];
  int total_bits;

  unsigned int bytepos_count[8];
  double bytepos_cost[8];
  double total_cost;

  unsigned int min_range;

  unsigned int ht_occupied;
  unsigned int ht_size;
  unsigned int ht_max_chain;
  double ht_avg_displacement;

  unsigned int sat_lopsided;
  unsigned int sat_strong;
  unsigned int sat_balanced;
  unsigned int sat_mixed;

  double entropy; /* Shannon H0 in bits/byte */
  unsigned int byte_freq[256];
  float *byte_costs; /* cost per data byte during encoding */
  float *byte_model_contrib; /* [num_data_bytes * num_models] per-byte per-model */
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

  int valid;
} CompStats;

static const char *html_output = NULL;

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
  h ^= byte;
  h *= HMUL;
  h = (h & 0xFFFFFF00u) | ((h + byte) & 0xFF);
  return h - 1;
}

static unsigned int ctx_hash_initial(unsigned int mask) {
  unsigned char cmask = (unsigned char)mask;
  unsigned int h = mask;
  h = h * HMUL - 1;
  while (cmask) {
    if (cmask & 0x80)
      h = h * HMUL - 1;
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
  out.table_size = next_pow2(total_bits * num);

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

static HashEntry *hash_probe(HashEntry *table, unsigned int mask,
                             unsigned int hash, int weight_shift,
                             unsigned int probs[2]) {
  unsigned int slot = (hash + 1) & mask;
  for (;;) {
    HashEntry *e = &table[slot];
    if (!e->hash) {
      e->hash = hash;
      return e;
    }
    if (e->hash == hash) {
      unsigned int shift =
          (1 - (((e->prob[0] + 255) & (e->prob[1] + 255)) >> 8)) * 2 +
          weight_shift;
      probs[0] += (unsigned)e->prob[0] << shift;
      probs[1] += (unsigned)e->prob[1] << shift;
      return e;
    }
    slot = (slot + 1) & mask;
  }
}

static void print_hash_table_stats(const HashEntry *ht,
                                   unsigned int table_size) {
  unsigned int occupied = 0;
  unsigned int max_chain = 0;
  unsigned long total_displacement = 0;
  unsigned int mask = table_size - 1;

  for (unsigned int i = 0; i < table_size; i++) {
    if (ht[i].hash) {
      occupied++;
      unsigned int ideal = (ht[i].hash + 1) & mask;
      unsigned int displacement = (i - ideal) & mask;
      if (displacement > max_chain)
        max_chain = displacement;
      total_displacement += displacement;
    }
  }

  printf("  Hash table: %u / %u slots used (%.1f%% load)\n", occupied,
         table_size, 100.0 * occupied / table_size);
  printf("  Probe chains: max %u, avg %.2f displacement\n", max_chain,
         occupied ? (double)total_displacement / occupied : 0.0);
}

static void encode_from_stream(ArithCoder *ac, const HashBitStream *hb,
                               HashEntry *ht, int base_prob, const ModelSet *ml,
                               CompStats *stats) {
  int num = hb->num_weights;
  int total_bits = (num == 0) ? hb->bits_len : (int)hb->hashes_len / num;
  unsigned int tmask = hb->table_size - 1;
  memset(ht, 0, hb->table_size * sizeof(HashEntry));
  int collect = verbose || (stats != NULL);

  unsigned int model_hits[MAX_SEARCH] = {};
  unsigned int model_misses[MAX_SEARCH] = {};
  unsigned int min_range = 0xFFFFFFFFu;
  double model_bits_saved[MAX_SEARCH] = {};
  unsigned int conf_hist[11] = {};
  unsigned int bytepos_count[8] = {};
  double bytepos_cost[8] = {};
  double total_cost = 0;

  HashEntry *matched[MAX_SEARCH];
  int hpos = 0;
  /* per-byte cost tracking for position curve */
  int num_data_bytes = (total_bits > 1) ? (total_bits - 1 + 7) / 8 : 0;
  float *byte_costs = NULL;
  float *byte_model_contrib = NULL;
  if (stats && num_data_bytes > 0) {
    byte_costs = (float *)calloc(num_data_bytes, sizeof(float));
    byte_model_contrib = (float *)calloc(
        (size_t)num_data_bytes * num, sizeof(float));
  }
  for (int bp = 0; bp < total_bits; bp++) {
    int bit = hb->bits[bp];
    unsigned int probs[2] = {(unsigned)base_prob, (unsigned)base_prob};
    for (int m = 0; m < num; m++) {
      unsigned int p0_before = probs[0], p1_before = probs[1];
      matched[m] =
          hash_probe(ht, tmask, hb->hashes[hpos++], hb->weights[m], probs);
      if (collect) {
        if (matched[m]->prob[0] || matched[m]->prob[1]) {
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
        }
      }
    }
    if (collect) {
      float conf = (float)probs[bit] / (probs[0] + probs[1]);
      int bucket = (conf < 0.5f) ? 0 : (int)((conf - 0.5f) * 20) + 1;
      if (bucket > 10)
        bucket = 10;
      conf_hist[bucket]++;

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
    }
    arith_encode(ac, probs[1], probs[0], 1 - bit);
    if (collect && ac->range < min_range)
      min_range = ac->range;
    for (int m = 0; m < num; m++)
      counter_update(matched[m]->prob, bit);
  }

  if (verbose) {
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

    unsigned int sat_lopsided = 0, sat_strong = 0, sat_balanced = 0,
                 sat_other = 0;
    unsigned int ht_occupied = 0;
    for (unsigned int i = 0; i < hb->table_size; i++) {
      if (ht[i].hash) {
        ht_occupied++;
        unsigned int p0 = ht[i].prob[0], p1 = ht[i].prob[1];
        if (p0 == 0 || p1 == 0)
          sat_lopsided++;
        else if (p0 > p1 * 4 || p1 > p0 * 4)
          sat_strong++;
        else if (p0 <= p1 * 2 && p1 <= p0 * 2)
          sat_balanced++;
        else
          sat_other++;
      }
    }
    printf("  Counter saturation (%u entries):\n", ht_occupied);
    printf("    Lopsided (one side=0): %u (%.1f%%)\n", sat_lopsided,
           ht_occupied ? 100.0 * sat_lopsided / ht_occupied : 0.0);
    printf("    Strong (>4:1):         %u (%.1f%%)\n", sat_strong,
           ht_occupied ? 100.0 * sat_strong / ht_occupied : 0.0);
    printf("    Balanced (<2:1):       %u (%.1f%%)\n", sat_balanced,
           ht_occupied ? 100.0 * sat_balanced / ht_occupied : 0.0);
    printf("    Mixed (2:1 to 4:1):    %u (%.1f%%)\n", sat_other,
           ht_occupied ? 100.0 * sat_other / ht_occupied : 0.0);
  }

  if (stats) {
    int snum = num < MAX_SEARCH ? num : MAX_SEARCH;
    stats->num_models = snum;
    stats->total_bits = total_bits;
    stats->total_cost = total_cost;
    stats->min_range = min_range;
    memcpy(stats->conf_hist, conf_hist, sizeof(conf_hist));
    memcpy(stats->bytepos_count, bytepos_count, sizeof(bytepos_count));
    memcpy(stats->bytepos_cost, bytepos_cost, sizeof(bytepos_cost));
    for (int m = 0; m < snum; m++) {
      stats->model_masks[m] = ml->models[m].mask;
      stats->model_weights[m] = ml->models[m].weight;
      stats->model_hits[m] = model_hits[m];
      stats->model_misses[m] = model_misses[m];
      stats->model_bits_saved[m] = model_bits_saved[m];
    }
    /* hash table + saturation stats */
    stats->ht_size = hb->table_size;
    stats->ht_occupied = 0;
    stats->ht_max_chain = 0;
    stats->sat_lopsided = 0;
    stats->sat_strong = 0;
    stats->sat_balanced = 0;
    stats->sat_mixed = 0;
    unsigned long total_disp = 0;
    unsigned int htmask = hb->table_size - 1;
    for (unsigned int i = 0; i < hb->table_size; i++) {
      if (ht[i].hash) {
        stats->ht_occupied++;
        unsigned int ideal = (ht[i].hash + 1) & htmask;
        unsigned int disp = (i - ideal) & htmask;
        if (disp > stats->ht_max_chain)
          stats->ht_max_chain = disp;
        total_disp += disp;
        unsigned int p0 = ht[i].prob[0], p1 = ht[i].prob[1];
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
    stats->ht_avg_displacement =
        stats->ht_occupied ? (double)total_disp / stats->ht_occupied : 0.0;
    stats->byte_costs = byte_costs;
    stats->byte_model_contrib = byte_model_contrib;
    stats->num_data_bytes = num_data_bytes;
    stats->valid = 1;
  } else {
    free(byte_costs);
    free(byte_model_contrib);
  }
}

static int compress_4k(const unsigned char *data, int size, unsigned char *out,
                       const ModelSet *ml, int base_prob, CompStats *stats) {
  unsigned char ctx[MAX_CTX] = {};
  HashBitStream hb = compute_hash_stream(data, size, ctx, ml, 1, 1);
  if (verbose) {
    printf("  Max models: %d\n", MAX_SEARCH);
    printf("  Hash table size: %u entries (%u bytes)\n", hb.table_size,
           (unsigned)(hb.table_size * sizeof(HashEntry)));
  }
  HashEntry *ht = (HashEntry *)calloc(hb.table_size, sizeof(HashEntry));
  ArithCoder ac;
  arith_init(&ac, out);
  encode_from_stream(&ac, &hb, ht, base_prob, ml, stats);
  int total = arith_finish(&ac);

  if (verbose)
    print_hash_table_stats(ht, hb.table_size);

  free(ht);
  hbs_free(&hb);
  return total;
}

static unsigned int real_compress_size(const unsigned char *data, int size,
                                       const ModelSet *ml, int base_prob) {
  int max_out = size + 1024;
  unsigned char *out = (unsigned char *)calloc(max_out, 1);
  int comp_bits = compress_4k(data, size, out, ml, base_prob, NULL);
  free(out);
  int header_bits = (7 + ml->num_models) * 8;
  return (unsigned int)(header_bits + comp_bits) * BIT_PREC;
}

static int decompress_4k(const unsigned char *cdata, unsigned char *out,
                         int base_prob) {
  uint16_t bl16;
  memcpy(&bl16, cdata, 2);
  int bitlen = bl16;
  unsigned int stored_wmask;
  memcpy(&stored_wmask, cdata + 2, 4);
  int num = cdata[6];
  int data_bits = bitlen - 1;
  int data_bytes = data_bits / 8;

  unsigned char ctx_masks[MAX_SEARCH];
  for (int i = 0; i < num; i++)
    ctx_masks[i] = cdata[7 + i];

  unsigned int ext_masks[MAX_SEARCH];
  int weights[MAX_SEARCH];
  decode_weight_mask(stored_wmask, num, ctx_masks, weights, ext_masks);

  const unsigned char *comp = cdata + 7 + num;
  unsigned char *buf = (unsigned char *)calloc(data_bytes + MAX_CTX, 1);
  unsigned char *dp = buf + MAX_CTX;
  unsigned int tsize = next_pow2(bitlen * num);
  if (verbose) {
    printf("  Max models: %d\n", MAX_SEARCH);
    printf("  Hash table size: %u entries (%u bytes)\n", tsize,
           (unsigned)(tsize * sizeof(HashEntry)));
  }
  HashEntry *ht = (HashEntry *)calloc(tsize, sizeof(HashEntry));
  unsigned int tmask = tsize - 1;

  unsigned int range = 0x80000000u, low = 0, value = 0;
  int cpos = 0;
  for (int i = 0; i < 31; i++)
    value = (value << 1) | get_compressed_bit(comp, cpos++);

  for (int bp = 0; bp < bitlen; bp++) {
    unsigned int probs[2] = {(unsigned)base_prob, (unsigned)base_prob};
    HashEntry *matched[MAX_SEARCH];

    for (int m = 0; m < num; m++) {
      unsigned int h = (bp == 0) ? ctx_hash_initial(ext_masks[m])
                                 : ctx_hash(dp, bp - 1, ext_masks[m]);
      matched[m] = hash_probe(ht, tmask, h, weights[m], probs);
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

    for (int m = 0; m < num; m++)
      counter_update(matched[m]->prob, bit);

    if (bp > 0 && bit) {
      int dbp = bp - 1;
      dp[dbp >> 3] |= 1 << (7 - (dbp & 7));
    }
  }

  memcpy(out, dp, data_bytes);
  if (verbose)
    print_hash_table_stats(ht, tsize);
  free(buf);
  free(ht);
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

  /* ── HTML head + CSS ── */
  fprintf(f,
    "<!DOCTYPE html>\n<html lang=\"en\"><head><meta charset=\"UTF-8\">\n"
    "<meta name=\"viewport\" content=\"width=device-width,initial-scale=1\">\n"
    "<title>Compression Report &ndash; %s</title>\n"
    "<link rel=\"preconnect\" href=\"https://fonts.googleapis.com\">\n"
    "<link href=\"https://fonts.googleapis.com/css2?"
    "family=DM+Sans:ital,wght@0,400;0,500;0,600;0,700;1,400"
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
    "}\n"
    "*{margin:0;padding:0;box-sizing:border-box}\n"
    "html{scroll-behavior:smooth}\n"
    ".card[id]{scroll-margin-top:20px}\n"
    "body{font-family:var(--sans);background:var(--bg);color:var(--fg);"
    "line-height:1.6;-webkit-font-smoothing:antialiased}\n"
    ".wrap{padding:24px 28px 64px 28px;margin-left:200px;"
    "max-width:calc(100vw - 200px)}\n"
    "\n"
    "/* ── Sidebar ── */\n"
    ".sidebar{position:fixed;top:0;left:0;width:184px;height:100vh;"
    "background:var(--bg2);border-right:1px solid var(--bdr);"
    "padding:20px 0;overflow-y:auto;z-index:100;"
    "display:flex;flex-direction:column}\n"
    ".sidebar .sb-title{font-size:10px;font-weight:600;color:var(--fg3);"
    "text-transform:uppercase;letter-spacing:1.2px;padding:0 16px;"
    "margin-bottom:12px}\n"
    ".sidebar a{display:block;padding:6px 16px 6px 14px;font-size:12px;"
    "color:var(--fg3);text-decoration:none;border-left:2px solid transparent;"
    "transition:color .15s,border-color .15s,background .15s;"
    "line-height:1.4}\n"
    ".sidebar a:hover{color:var(--fg2);background:rgba(255,255,255,.02)}\n"
    ".sidebar a.active{color:var(--acc);border-left-color:var(--acc);"
    "background:rgba(34,211,238,.04)}\n"
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
    "color:var(--acc);line-height:1}\n"
    ".hero-pct .lbl{font-size:11px;color:var(--fg3);text-transform:uppercase;"
    "letter-spacing:1.5px;margin-top:4px}\n"
    ".hero-info h1{font-size:20px;font-weight:600;color:var(--fg);"
    "margin-bottom:4px}\n"
    ".hero-info .sub{font-size:13px;color:var(--fg3)}\n"
    ".hero-stats{display:flex;gap:32px;margin-top:18px}\n"
    ".hero-stat .val{font-family:var(--mono);font-size:20px;font-weight:600;"
    "line-height:1}\n"
    ".hero-stat .lbl{font-size:11px;color:var(--fg3);margin-top:3px;"
    "text-transform:uppercase;letter-spacing:.5px}\n"
    "\n"
    "/* ── Cards ── */\n"
    ".grid{display:grid;grid-template-columns:repeat(auto-fit,minmax(440px,1fr));"
    "gap:20px;margin-top:28px}\n"
    ".grid .full{grid-column:1/-1}\n"
    ".card{background:var(--bg2);border:1px solid var(--bdr);"
    "border-radius:10px;padding:22px 24px;overflow:hidden}\n"
    ".card h2{font-size:14px;font-weight:600;color:var(--fg);"
    "margin-bottom:2px;display:flex;align-items:center;gap:8px}\n"
    ""
    ".card .desc{font-size:11.5px;color:var(--fg3);margin-bottom:16px}\n"
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
    "tr:hover td{background:rgba(255,255,255,.02)}\n"
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
    ".slider-row button{font-size:10px;padding:3px 9px;border:1px solid var(--bdr2);"
    "border-radius:4px;background:var(--bg3);color:var(--fg2);cursor:pointer;"
    "font-family:var(--sans)}\n"
    ".slider-row button:hover{background:var(--bg4);border-color:var(--fg3)}\n"
    "\n"
    "/* ── Inspect panel ── */\n"
    ".cd-panel{display:none;margin-top:14px;padding:14px 18px;"
    "background:var(--bg3);border:1px solid var(--bdr2);border-radius:8px;"
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
    ".scrub-wrap{position:relative}\n"
    ".scrub-line{stroke:#fff;stroke-width:0.6;opacity:0;"
    "pointer-events:none;transition:opacity .1s}\n"
    ".hover-tip{position:absolute;display:none;background:var(--bg);"
    "border:1px solid var(--bdr2);border-radius:4px;padding:6px 10px;"
    "font-size:11px;font-family:var(--mono);pointer-events:none;z-index:10;"
    "white-space:nowrap;box-shadow:0 4px 12px rgba(0,0,0,.4);"
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
    "border-radius:4px;cursor:pointer;font-family:var(--sans);"
    "display:flex;align-items:center;justify-content:center;gap:6px;"
    "transition:background .12s,color .12s,border-color .12s}\n"
    ".theme-toggle:hover{background:var(--bg4);color:var(--fg);"
    "border-color:var(--bdr2)}\n"
    "</style></head><body>\n"
    "<nav class=\"sidebar\" id=\"sidebar\">\n"
    "<div class=\"sb-title\">Sections</div>\n"
    "<a href=\"#sec-params\">Parameters</a>\n"
    "<a href=\"#sec-output\">Output Breakdown</a>\n"
    "<a href=\"#sec-mctrib\">Model Contribution</a>\n"
    "<a href=\"#sec-bytefreq\">Byte Frequency</a>\n"
    "<a href=\"#sec-bigram\">Byte Bigrams</a>\n"
    "<a href=\"#sec-cmap\">Compressibility Map</a>\n"
    "<a href=\"#sec-attr\">Model Attribution</a>\n"
    "<a href=\"#sec-dominance\">Model Dominance</a>\n"
    "<a href=\"#sec-hurt\">Model Hurt</a>\n"
    "<a href=\"#sec-net\">Model Net</a>\n"
    "<a href=\"#sec-surprises\">Top Surprises</a>\n"
    "<a href=\"#sec-cost\">Cost Over Position</a>\n"
    "<a href=\"#sec-search\">Search Trajectory</a>\n"
    "<a href=\"#sec-models\">Model Statistics</a>\n"
    "<a href=\"#sec-conf\">Pred. Confidence</a>\n"
    "<a href=\"#sec-bytepos\">Byte Position</a>\n"
    "<a href=\"#sec-hash\">Hash Table</a>\n"
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
    "      +'\" stroke=\"rgba(255,255,255,.2)\" stroke-width=\"0.5\"/>';\n"
    "  }\n"
    "  s += '<path d=\"'+area+'\" fill=\"'+color+'\" fill-opacity=\".25\"/>';\n"
    "  s += '<path d=\"'+line+'\" fill=\"none\" stroke=\"'+color+'\" stroke-width=\"1\"/>';\n"
    "  if(hi>=0&&hi<n){\n"
    "    s += '<circle cx=\"'+xOf(hi).toFixed(1)+'\" '\n"
    "      +'cy=\"'+yOf(values[hi]).toFixed(1)+'\" r=\"2.5\" fill=\"#fff\"/>';\n"
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
  fprintf(f,
    "<tr><td>Arith coder min range</td>"
    "<td>0x%08X</td></tr>\n", s->min_range);
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
      fprintf(f,
        "<div style=\"display:flex;align-items:center;gap:10px;"
        "color:var(--fg3);font-size:11px\">\n"
        "<span style=\"width:10px;flex-shrink:0\"></span>\n"
        "<span style=\"flex:1\">+%d more</span>\n"
        "<span style=\"font-family:var(--mono)\">%.0f B</span>\n"
        "<span style=\"font-family:var(--mono);min-width:42px;"
        "text-align:right\">%.1f%%</span>\n"
        "</div>\n",
        rest_count, rest_bits / 8.0, rest_pct);
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
        int cr, cg, cb;
        if (freq == 0) {
          cr = 26; cg = 30; cb = 43; /* var(--bg3) */
        } else {
          /* dark teal to bright cyan */
          cr = (int)(13 + (34 - 13) * intensity);
          cg = (int)(40 + (211 - 40) * intensity);
          cb = (int)(60 + (238 - 60) * intensity);
        }
        char label[8] = "";
        if (byte_val >= 0x20 && byte_val <= 0x7E && byte_val != '<' &&
            byte_val != '>' && byte_val != '&' && byte_val != '"')
          snprintf(label, sizeof(label), "%c", byte_val);

        fprintf(f,
          "<rect x=\"%d\" y=\"%d\" width=\"%d\" height=\"%d\" "
          "rx=\"2\" fill=\"rgb(%d,%d,%d)\" data-b=\"%d\" "
          "style=\"cursor:pointer\"/>\n",
          x, y, cell, cell, cr, cg, cb, byte_val);

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
      "<p class=\"desc\">256&times;256 matrix of byte pair frequencies. "
      "Row = preceding byte, column = following byte. "
      "Intensity = log frequency. Click to inspect.</p>\n");

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
      "border:1px solid var(--bdr)\"></canvas>\n"
      /* crosshair lines + hover tooltip, positioned over canvas */
      "<div id=\"bigram-v\" style=\"position:absolute;width:1px;"
      "background:rgba(255,255,255,.5);pointer-events:none;display:none\"></div>\n"
      "<div id=\"bigram-h\" style=\"position:absolute;height:1px;"
      "background:rgba(255,255,255,.5);pointer-events:none;display:none\"></div>\n"
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
      "for(var r=0;r<256;r++){\n"
      "  for(var c=0;c<256;c++){\n"
      "    var v=BG[r*256+c];\n"
      "    var t=0;\n"
      "    if(v>0) t=Math.min((Math.log2(v)+1)/(logMax+1),1);\n"
      "    if(v>0&&t<0.1) t=0.1;\n"
      /* 2x2 pixel per cell */
      "    var cr,cg,cb;\n"
      "    if(v===0){cr=26;cg=30;cb=43;}\n"
      "    else{cr=13+(34-13)*t|0;cg=40+(211-40)*t|0;cb=60+(238-60)*t|0;}\n"
      "    for(var dy=0;dy<2;dy++)for(var dx=0;dx<2;dx++){\n"
      "      var px=((r*2+dy)*512+(c*2+dx))*4;\n"
      "      d[px]=cr;d[px+1]=cg;d[px+2]=cb;d[px+3]=255;\n"
      "    }\n"
      "  }\n"
      "}\n"
      "ctx.putImageData(img,0,0);\n"
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
    else { cols = 128; cell = 4; }
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
    else { cols = 128; cell = 4; }
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
      "      +(isBase?'background:rgba(255,255,255,.08);color:var(--fg)':'')+'\">'\n"
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
      "    +'stroke=\"rgba(255,255,255,.2)\" stroke-width=\"0.5\"/>';\n"
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
      "      +(isBase?'background:rgba(255,255,255,.08);color:var(--fg)':'')+'\">'\n"
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
      "    +'stroke=\"rgba(255,255,255,.2)\" stroke-width=\"0.5\"/>';\n"
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
      "      +(isBase?'background:rgba(255,255,255,.08);color:var(--fg)':'')+'\">'\n"
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
      "    +'\" stroke=\"rgba(255,255,255,.25)\" stroke-width=\"1\"/>';\n"
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
      "#search-svg circle:hover{stroke:#fff;stroke-width:1.5}</style>\n");
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

  /* ── Hash Table ── */
  {
    double load = s->ht_size ? 100.0 * s->ht_occupied / s->ht_size : 0;

    fprintf(f,
      "<div class=\"card\" id=\"sec-hash\">\n"
      "<h2>Hash Table</h2>\n"
      "<p class=\"desc\">Context hash table occupancy and probe stats.</p>\n"
      "<table class=\"kv\">\n");
    fprintf(f, "<tr><td>Table size</td><td>%u slots</td></tr>\n", s->ht_size);
    fprintf(f,
      "<tr><td>Occupied</td><td class=\"%s\">%u (%.1f%%)</td></tr>\n",
      load < 50 ? "c-grn" : load < 75 ? "c-ylw" : "c-orn",
      s->ht_occupied, load);
    fprintf(f,
      "<tr><td>Max probe chain</td><td>%u</td></tr>\n", s->ht_max_chain);
    fprintf(f,
      "<tr><td>Avg displacement</td><td>%.2f</td></tr>\n",
      s->ht_avg_displacement);
    fprintf(f, "</table></div>\n\n");
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
  fprintf(f,
    "<div style=\"margin-top:32px;padding-top:16px;border-top:1px solid var(--bdr);"
    "font-size:11px;color:var(--fg3);text-align:center\">"
    "Generated by context-mixing arithmetic compressor"
    "</div>\n");

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
    "  btn.addEventListener('click',function(){\n"
    "    var cur=document.body.classList.contains('light')?'light':'dark';\n"
    "    var nxt=cur==='light'?'dark':'light';\n"
    "    apply(nxt);\n"
    "    try { localStorage.setItem('report-theme',nxt); } catch(e){}\n"
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
  printf("  -H <file>    Write HTML stats report to file\n");
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

  int opt;
#ifdef _WIN32
  /* MinGW getopt stops at first non-option; permute argv so options
     can appear in any order (matches GNU getopt's default) */
  {
    const char *os = "o:m:b:p:k:H:sdwehv";
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
  while ((opt = getopt(argc, argv, "o:m:b:p:k:H:sdwehv")) != -1) {
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
    case 'H':
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

    if (data_size < 7) {
      fprintf(stderr, "Input too small to be a valid .paq file\n");
      free(data);
      return 1;
    }

    int bitlen = data[0] | (data[1] << 8);
    int num_models = data[6];

    if (bitlen < 1 || 7 + num_models > data_size) {
      fprintf(stderr, "Corrupt header (bitlen=%d, models=%d, filesize=%d)\n",
              bitlen, num_models, data_size);
      free(data);
      return 1;
    }

    int out_sz = (bitlen - 1) / 8;
    printf("Bitlength:   %d (%d data bytes)\n", bitlen, out_sz);
    printf("Models:      %d\n", num_models);

    unsigned char *out_data = (unsigned char *)calloc(out_sz + 16, 1);
    int decoded = decompress_4k(data, out_data, base_prob);

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
  if (bitlen > 65535) {
    fprintf(stderr, "Input too large for 16-bit bitlength (%d bits)\n", bitlen);
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

  int max_out = data_size + 1024;
  unsigned char *out_buf = (unsigned char *)calloc(max_out, 1);
  int comp_bits = compress_4k(data, data_size, out_buf, &ml, base_prob,
                              html_output ? &cstats : NULL);
  int comp_bytes = (comp_bits + 7) / 8;

  unsigned char ordered_masks[MAX_SEARCH];
  unsigned int wmask = encode_weight_mask(&ml, ordered_masks, 1);

  int header_bytes = 7 + ml.num_models;
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

  unsigned char header[7];
  uint16_t bl16 = (uint16_t)bitlen;
  memcpy(header, &bl16, 2);
  memcpy(header + 2, &wmask, 4);
  header[6] = ml.num_models;

  FILE *fout = fopen(output_file, "wb");
  if (!fout) {
    fprintf(stderr, "Failed to open output '%s'\n", output_file);
    free(out_buf);
    free(data);
    return 1;
  }
  fwrite(header, 1, 7, fout);
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
    cstats.compressed_bits = comp_bits;
    cstats.header_bytes = header_bytes;
    cstats.total_bytes = total_bytes;
    cstats.estimated_bytes = est_size / (float)(BIT_PREC * 8);
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
    free(cstats.search_best);
    free(cstats.search_events);
  }

  free(out_buf);
  free(data);
  return 0;
}
