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

#define MIN_LEVEL 1
#define MAX_LEVEL 3
#define DEFAULT_LEVEL 2

static int verbose = 0; /* 0=off, 1=verbose, 2=very verbose */

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
  int level;
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

  double entropy;    /* Shannon H0 in bits/byte */
  float *byte_costs; /* cost per data byte during encoding */
  int num_data_bytes;

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
  void *p = NULL;
  if (posix_memalign(&p, 32, size) != 0)
    return NULL;
  return p;
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
  free(ev->accum);
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
  free(cs->block_arena);
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
  if (stats && num_data_bytes > 0) {
    byte_costs = (float *)calloc(num_data_bytes, sizeof(float));
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
          model_bits_saved[m] +=
              fast_log2f((float)after_correct / after_total) -
              fast_log2f((float)before_correct / before_total);
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
    stats->num_data_bytes = num_data_bytes;
    stats->valid = 1;
  } else {
    free(byte_costs);
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

static unsigned int approximate_weights(CompState *cs, ModelSet *ml) {
  for (int i = 0; i < ml->num_models; i++)
    ml->models[i].weight = __builtin_popcount(ml->models[i].mask);
  eval_evaluate(cs->eval, ml);
  return eval_get_size(cs->eval);
}

static unsigned int optimize_weights(CompState *cs, ModelSet *ml) {
  ModelSet cand = *ml;
  int idx = ml->num_models - 1;
  int dir = 1;
  int last_improved = idx;
  unsigned int best = approximate_weights(cs, ml);
  if (ml->num_models == 0)
    return best;

  do {
    cand = *ml;
    cand.models[idx].weight += dir;
    int improved = 0;
    if (cand.models[idx].weight <= MAX_WEIGHT &&
        cand.models[idx].weight != 255) {
      eval_evaluate(cs->eval, &cand);
      unsigned int trial = eval_get_size(cs->eval);
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

static unsigned int try_weights(CompState *cs, ModelSet *ml, int level) {
  return (level >= 2) ? optimize_weights(cs, ml) : approximate_weights(cs, ml);
}

static int model_set_cmp(const void *a, const void *b) {
  int sa = ((const ModelSet *)a)->size;
  int sb = ((const ModelSet *)b)->size;
  return (sa > sb) - (sa < sb);
}

static ModelSet search_best_models(const unsigned char *data, int size,
                                   const unsigned char ctx[MAX_CTX], int level,
                                   int base_prob, int *out_size,
                                   const ModelSet *seed, CompStats *stats) {
  const int beam = (level >= 3) ? 3 : 1;
  const int EFLAG = INT_MIN;
  const int nsets = beam * 2;
  ModelSet *sets = (ModelSet *)calloc(nsets, sizeof(ModelSet));
  Evaluator eval = {0};
  CompState *cs = state_new(data, size, base_prob, &eval, ctx);

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
    eval_evaluate(cs->eval, &sets[0]);
    sets[0].size = eval_get_size(cs->eval) | EFLAG;
  } else {
    sets[0].size = eval_get_size(cs->eval) | EFLAG;
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
        int new_sz = try_weights(cs, next, level);

        if (new_sz < old_sz || level >= 3) {
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
            int trial = try_weights(cs, next, level);
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
  int final_sz = optimize_weights(cs, &best);
  if (out_size)
    *out_size = final_sz;
  if (verbose) {
    char setbuf[512] = "";
    model_set_sprint(&best, setbuf, sizeof(setbuf));
    printf("  Final: %2d models, est %.1f bytes [%s]\n", best.num_models,
           final_sz / (float)(BIT_PREC * 8), setbuf);
  }

  state_destroy(cs);
  eval_destroy(&eval);
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

  fprintf(f,
          "<!DOCTYPE html>\n<html lang=\"en\"><head><meta charset=\"UTF-8\">\n"
          "<title>Compression Report &ndash; %s</title>\n<style>\n"
          "*{margin:0;padding:0;box-sizing:border-box}\n"
          "body{font-family:'Segoe UI',system-ui,sans-serif;background:#fff;"
          "color:#1f2328;padding:32px;max-width:960px}\n"
          "h1{font-size:22px;font-weight:600;margin-bottom:6px}\n"
          ".sub{color:#656d76;font-size:13px;margin-bottom:28px}\n"
          ".sec{margin-bottom:36px}\n"
          ".sec h2{font-size:15px;font-weight:600;margin-bottom:4px}\n"
          ".sec .d{font-size:12px;color:#656d76;margin-bottom:12px}\n"
          "table{border-collapse:collapse;width:100%%;font-size:12.5px;"
          "font-variant-numeric:tabular-nums}\n"
          "th{background:#f6f8fa;color:#656d76;font-weight:600;"
          "text-transform:uppercase;font-size:10.5px;letter-spacing:.5px;"
          "padding:8px 12px;text-align:left;border-bottom:2px solid #d0d7de}\n"
          "th.r{text-align:right}\n"
          "td{padding:6px 12px;border-bottom:1px solid #d0d7de}\n"
          "td.r{text-align:right;font-family:'JetBrains Mono','Cascadia Code',"
          "'Consolas',monospace;font-size:12px}\n"
          "td.n{font-weight:500}\n"
          "tr:hover td{background:#f6f8fa}\n"
          ".B{color:#1a7f37;font-weight:700}\n"
          ".G{color:#0969da}\n"
          ".M{color:#9a6700}\n"
          ".D{color:#bc4c00}\n"
          ".W{color:#cf222e}\n"
          ".kvtbl{max-width:420px}\n"
          ".kvtbl td:first-child{color:#656d76;font-size:12px}\n"
          ".bar-cell{position:relative}\n"
          ".bar{position:absolute;left:0;top:0;bottom:0;opacity:.12;border-"
          "radius:2px}\n"
          ".bar-g{background:#1a7f37}.bar-b{background:#0969da}"
          ".bar-y{background:#9a6700}.bar-o{background:#bc4c00}"
          ".bar-r{background:#cf222e}\n"
          ".bar-label{position:relative;z-index:1}\n"
          "</style></head><body>\n",
          s->input_file);

  fprintf(f, "<h1>Compression Report</h1>\n");
  fprintf(f,
          "<p class=\"sub\">%s &middot; context-mixing arithmetic coder</p>\n",
          s->input_file);

  /* ── Summary ── */
  fprintf(f, "<div class=\"sec\"><h2>Summary</h2>\n"
             "<p class=\"d\">Compression parameters and results.</p>\n"
             "<table class=\"kvtbl\"><tr><th>Parameter</th><th "
             "class=\"r\">Value</th></tr>\n");
  fprintf(f,
          "<tr><td class=\"n\">Input file</td>"
          "<td class=\"r\">%s</td></tr>\n",
          s->input_file);
  fprintf(f,
          "<tr><td class=\"n\">Input size</td>"
          "<td class=\"r\">%d bytes</td></tr>\n",
          s->input_size);
  fprintf(f,
          "<tr><td class=\"n\">Compressed size</td>"
          "<td class=\"r\">%d bytes</td></tr>\n",
          s->total_bytes);
  fprintf(f,
          "<tr><td class=\"n\">Ratio</td>"
          "<td class=\"r %s\">%.2f%%</td></tr>\n",
          ratio < 50    ? "B"
          : ratio < 75  ? "G"
          : ratio < 100 ? "M"
                        : "W",
          ratio);
  fprintf(f,
          "<tr><td class=\"n\">Compressed bits</td>"
          "<td class=\"r\">%d</td></tr>\n",
          s->compressed_bits);
  fprintf(f,
          "<tr><td class=\"n\">Header</td>"
          "<td class=\"r\">%d bytes</td></tr>\n",
          s->header_bytes);
  fprintf(f,
          "<tr><td class=\"n\">Level</td>"
          "<td class=\"r\">%d</td></tr>\n",
          s->level);
  fprintf(f,
          "<tr><td class=\"n\">Base probability</td>"
          "<td class=\"r\">%d</td></tr>\n",
          s->base_prob);
  fprintf(f,
          "<tr><td class=\"n\">Estimated (pre-encode)</td>"
          "<td class=\"r\">%.3f bytes</td></tr>\n",
          s->estimated_bytes);
  float actual_bpb =
      s->input_size > 0 ? (float)s->total_bytes * 8.0f / s->input_size : 0;
  float est_delta = (float)s->total_bytes - s->estimated_bytes;
  float est_delta_pct =
      s->estimated_bytes > 0 ? 100.0f * est_delta / s->estimated_bytes : 0;
  fprintf(f,
          "<tr><td class=\"n\">Estimator delta</td>"
          "<td class=\"r %s\">%+.1f bytes (%+.1f%%)</td></tr>\n",
          (est_delta < 2 && est_delta > -2)   ? "G"
          : (est_delta < 5 && est_delta > -5) ? "M"
                                              : "D",
          est_delta, est_delta_pct);
  fprintf(f,
          "<tr><td class=\"n\">Shannon H\xe2\x82\x80</td>"
          "<td class=\"r\">%.3f bits/byte</td></tr>\n",
          s->entropy);
  fprintf(f,
          "<tr><td class=\"n\">Actual (incl. header)</td>"
          "<td class=\"r %s\">%.3f bits/byte</td></tr>\n",
          actual_bpb < s->entropy         ? "B"
          : actual_bpb < s->entropy * 1.1 ? "G"
          : actual_bpb < s->entropy * 1.3 ? "M"
                                          : "D",
          actual_bpb);
  float context_gain = (float)s->entropy - actual_bpb;
  fprintf(f,
          "<tr><td class=\"n\">Context gain over H\xe2\x82\x80</td>"
          "<td class=\"r %s\">%+.3f bits/byte</td></tr>\n",
          context_gain > 0 ? "B" : "W", context_gain);
  fprintf(f,
          "<tr><td class=\"n\">Models</td>"
          "<td class=\"r\">%s</td></tr>\n",
          s->model_string);
  fprintf(f,
          "<tr><td class=\"n\">Prediction cost</td>"
          "<td class=\"r\">%.1f bits (%.1f bytes)</td></tr>\n",
          s->total_cost, s->total_cost / 8.0);
  fprintf(f,
          "<tr><td class=\"n\">Arith coder min range</td>"
          "<td class=\"r\">0x%08X</td></tr>\n",
          s->min_range);
  fprintf(f, "</table></div>\n");

  /* ── Cost Over File Position ── */
  if (s->byte_costs && s->num_data_bytes > 1) {
    int nb = s->num_data_bytes;
    /* compute rolling average with adaptive window */
    int win = nb / 64;
    if (win < 4)
      win = 4;
    if (win > 64)
      win = 64;
    int npts = nb - win + 1;
    if (npts < 2)
      npts = 2;
    float *rolling = (float *)malloc(npts * sizeof(float));
    float rmin = 1e9f, rmax = -1e9f;
    for (int i = 0; i < npts; i++) {
      float sum = 0;
      int end = i + win;
      if (end > nb)
        end = nb;
      for (int j = i; j < end; j++)
        sum += s->byte_costs[j];
      rolling[i] = sum / (end - i);
      if (rolling[i] < rmin)
        rmin = rolling[i];
      if (rolling[i] > rmax)
        rmax = rolling[i];
    }
    if (rmax <= rmin)
      rmax = rmin + 1;

    int svg_w = 820, svg_h = 140;
    int pad_l = 40, pad_r = 10, pad_t = 10, pad_b = 25;
    int plot_w = svg_w - pad_l - pad_r;
    int plot_h = svg_h - pad_t - pad_b;

    fprintf(f,
            "<div class=\"sec\"><h2>Cost Over File Position</h2>\n"
            "<p class=\"d\">Rolling average encoding cost (bits/byte) across "
            "the input. "
            "Window = %d bytes. Lower is better. "
            "H\xe2\x82\x80 = %.2f shown as reference.</p>\n",
            win, s->entropy);
    fprintf(f,
            "<svg width=\"%d\" height=\"%d\" "
            "style=\"font-family:'Segoe UI',system-ui,sans-serif;display:block;"
            "margin-bottom:8px\">\n",
            svg_w, svg_h);

    /* background */
    fprintf(f,
            "<rect x=\"%d\" y=\"%d\" width=\"%d\" height=\"%d\" "
            "fill=\"#f6f8fa\" rx=\"3\"/>\n",
            pad_l, pad_t, plot_w, plot_h);

    /* y-axis gridlines + labels */
    int nyticks = 4;
    for (int i = 0; i <= nyticks; i++) {
      float val = rmin + (rmax - rmin) * (nyticks - i) / nyticks;
      int y = pad_t + plot_h * i / nyticks;
      fprintf(f,
              "<line x1=\"%d\" y1=\"%d\" x2=\"%d\" y2=\"%d\" "
              "stroke=\"#d0d7de\" stroke-width=\"0.5\"/>\n",
              pad_l, y, pad_l + plot_w, y);
      fprintf(f,
              "<text x=\"%d\" y=\"%d\" text-anchor=\"end\" "
              "font-size=\"9\" fill=\"#656d76\">%.1f</text>\n",
              pad_l - 4, y + 3, val);
    }

    /* H0 reference line */
    if (s->entropy >= rmin && s->entropy <= rmax) {
      int h0y = pad_t + (int)(plot_h * (rmax - s->entropy) / (rmax - rmin));
      fprintf(
          f,
          "<line x1=\"%d\" y1=\"%d\" x2=\"%d\" y2=\"%d\" "
          "stroke=\"#cf222e\" stroke-width=\"1\" stroke-dasharray=\"4,3\"/>\n",
          pad_l, h0y, pad_l + plot_w, h0y);
      fprintf(f,
              "<text x=\"%d\" y=\"%d\" font-size=\"9\" "
              "fill=\"#cf222e\">H\xe2\x82\x80</text>\n",
              pad_l + plot_w + 2, h0y + 3);
    }

    /* 8.0 reference line (uncompressible) */
    if (8.0 >= rmin && 8.0 <= rmax) {
      int y8 = pad_t + (int)(plot_h * (rmax - 8.0) / (rmax - rmin));
      fprintf(f,
              "<line x1=\"%d\" y1=\"%d\" x2=\"%d\" y2=\"%d\" "
              "stroke=\"#656d76\" stroke-width=\"0.5\" "
              "stroke-dasharray=\"2,3\"/>\n",
              pad_l, y8, pad_l + plot_w, y8);
      fprintf(f,
              "<text x=\"%d\" y=\"%d\" font-size=\"8\" "
              "fill=\"#656d76\">8.0</text>\n",
              pad_l + plot_w + 2, y8 + 3);
    }

    /* area fill */
    fprintf(f, "<path d=\"M%d,%d ", pad_l, pad_t + plot_h);
    for (int i = 0; i < npts; i++) {
      int x = pad_l + (int)((long)i * plot_w / (npts - 1));
      int y = pad_t + (int)(plot_h * (rmax - rolling[i]) / (rmax - rmin));
      fprintf(f, "L%d,%d ", x, y);
    }
    fprintf(f, "L%d,%d Z\" fill=\"#0969da\" opacity=\"0.08\"/>\n",
            pad_l + plot_w, pad_t + plot_h);

    /* line */
    fprintf(f, "<path d=\"");
    for (int i = 0; i < npts; i++) {
      int x = pad_l + (int)((long)i * plot_w / (npts - 1));
      int y = pad_t + (int)(plot_h * (rmax - rolling[i]) / (rmax - rmin));
      fprintf(f, "%c%d,%d ", i == 0 ? 'M' : 'L', x, y);
    }
    fprintf(f, "\" fill=\"none\" stroke=\"#0969da\" stroke-width=\"1.5\"/>\n");

    /* x-axis labels */
    int nxticks = 4;
    for (int i = 0; i <= nxticks; i++) {
      int x = pad_l + plot_w * i / nxticks;
      int byte_pos = (int)((long)i * (nb - 1) / nxticks);
      fprintf(f,
              "<text x=\"%d\" y=\"%d\" text-anchor=\"middle\" "
              "font-size=\"9\" fill=\"#656d76\">%d</text>\n",
              x, svg_h - 3, byte_pos);
    }

    fprintf(f, "</svg></div>\n");
    free(rolling);
  }

  /* ── Search Trajectory (log-log) ── */
  if (s->search_best && s->search_len > 0) {
    int npts = s->search_len;
    float smin = 1e9f, smax = -1e9f;
    for (int i = 0; i < npts; i++) {
      if (s->search_best[i] < smin)
        smin = s->search_best[i];
      if (s->search_best[i] > smax)
        smax = s->search_best[i];
    }
    for (int e = 0; e < s->search_nevents; e++) {
      float v = s->search_events[e].est_bytes;
      if (v < smin)
        smin = v;
      if (v > smax)
        smax = v;
    }
    if (smin < 1)
      smin = 1;
    if (smax <= smin)
      smax = smin * 2;

    /* log2 ranges with margin — x uses 1..npts, y uses data range */
    float xlog_max = fast_log2f((float)npts);
    float xlog_min = 0; /* log2(1) = 0 */
    float xlog_range = xlog_max - xlog_min;
    if (xlog_range < 1)
      xlog_range = 1;

    float ylog_min = fast_log2f(smin) - 0.1f;
    float ylog_max = fast_log2f(smax) + 0.1f;
    float ylog_range = ylog_max - ylog_min;
    if (ylog_range < 0.5f)
      ylog_range = 0.5f;

    int svg_w = 820, svg_h = 180;
    int pad_l = 50, pad_r = 10, pad_t = 14, pad_b = 28;
    int plot_w = svg_w - pad_l - pad_r;
    int plot_h = svg_h - pad_t - pad_b;

#define LOGX(idx)                                                              \
  (pad_l +                                                                     \
   (int)(plot_w * (fast_log2f((float)(idx) + 1) - xlog_min) / xlog_range))
#define LOGY(val)                                                              \
  (pad_t +                                                                     \
   (int)(plot_h * (ylog_max - fast_log2f((val) > 0.5f ? (val) : 0.5f)) /       \
         ylog_range))
#define CLAMP(v, lo, hi) ((v) < (lo) ? (lo) : ((v) > (hi) ? (hi) : (v)))

    fprintf(f, "<div class=\"sec\"><h2>Model Search Trajectory</h2>\n"
               "<p class=\"d\">Best estimated size (log-log) as the search "
               "iterates over "
               "256 context masks (bit-reversed order). "
               "Dots mark accepted model additions "
               "(<span style=\"color:#1a7f37\">\xe2\x97\x8f</span>) "
               "and removals "
               "(<span style=\"color:#cf222e\">\xe2\x97\x8f</span>).</p>\n");
    fprintf(f,
            "<svg width=\"%d\" height=\"%d\" "
            "style=\"font-family:'Segoe UI',system-ui,sans-serif;display:block;"
            "margin-bottom:8px\">\n",
            svg_w, svg_h);

    /* background */
    fprintf(f,
            "<rect x=\"%d\" y=\"%d\" width=\"%d\" height=\"%d\" "
            "fill=\"#f6f8fa\" rx=\"3\"/>\n",
            pad_l, pad_t, plot_w, plot_h);

    /* y-axis gridlines (log scale, nice values) */
    {
      float nice_ticks[64];
      int nticks = 0;
      float base = 1;
      while (base * 10 < smin)
        base *= 10;
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
                "stroke=\"#d0d7de\" stroke-width=\"0.5\"/>\n",
                pad_l, y, pad_l + plot_w, y);
        fprintf(f,
                "<text x=\"%d\" y=\"%d\" text-anchor=\"end\" "
                "font-size=\"9\" fill=\"#656d76\">%g</text>\n",
                pad_l - 4, y + 3, nice_ticks[i]);
      }
    }

    /* x-axis gridlines (log scale) */
    {
      int xticks[] = {1, 2, 4, 8, 16, 32, 64, 128, 256};
      for (int i = 0; i < 9; i++) {
        if (xticks[i] > npts)
          break;
        int x = CLAMP(LOGX(xticks[i] - 1), pad_l, pad_l + plot_w);
        fprintf(f,
                "<line x1=\"%d\" y1=\"%d\" x2=\"%d\" y2=\"%d\" "
                "stroke=\"#d0d7de\" stroke-width=\"0.5\"/>\n",
                x, pad_t, x, pad_t + plot_h);
        fprintf(f,
                "<text x=\"%d\" y=\"%d\" text-anchor=\"middle\" "
                "font-size=\"9\" fill=\"#656d76\">%d</text>\n",
                x, svg_h - 5, xticks[i] - 1);
      }
    }

    /* step line — area fill */
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
    fprintf(f, "L%d,%d Z\" fill=\"#0969da\" opacity=\"0.06\"/>\n",
            pad_l + plot_w, pad_t + plot_h);

    /* step line — stroke */
    fprintf(f, "<path d=\"");
    for (int i = 0; i < npts; i++) {
      int x = CLAMP(LOGX(i), pad_l, pad_l + plot_w);
      int y = CLAMP(LOGY(s->search_best[i]), pad_t, pad_t + plot_h);
      if (i == 0) {
        fprintf(f, "M%d,%d ", x, y);
      } else {
        int py = CLAMP(LOGY(s->search_best[i - 1]), pad_t, pad_t + plot_h);
        fprintf(f, "L%d,%d L%d,%d ", x, py, x, y);
      }
    }
    fprintf(f, "\" fill=\"none\" stroke=\"#0969da\" stroke-width=\"1.5\"/>\n");

    /* event dots */
    for (int e = 0; e < s->search_nevents; e++) {
      int mi = s->search_events[e].mask_idx;
      if (mi >= npts)
        continue;
      int x = CLAMP(LOGX(mi), pad_l, pad_l + plot_w);
      float est = s->search_events[e].est_bytes;
      int y = CLAMP(LOGY(est), pad_t, pad_t + plot_h);
      const char *col = s->search_events[e].is_removal ? "#cf222e" : "#1a7f37";
      fprintf(f,
              "<circle cx=\"%d\" cy=\"%d\" r=\"3\" fill=\"%s\" opacity=\"0.7\">"
              "<title>%s %02X (%d models, %.1f bytes)</title></circle>\n",
              x, y, col, s->search_events[e].is_removal ? "Remove" : "Add",
              s->search_events[e].mask, s->search_events[e].num_models, est);
    }

#undef LOGX
#undef LOGY
#undef CLAMP

    /* axis labels */
    fprintf(f,
            "<text x=\"%d\" y=\"%d\" text-anchor=\"middle\" "
            "font-size=\"10\" fill=\"#656d76\">mask index (log)</text>\n",
            pad_l + plot_w / 2, svg_h + 1);
    fprintf(f,
            "<text x=\"12\" y=\"%d\" text-anchor=\"middle\" "
            "font-size=\"10\" fill=\"#656d76\" "
            "transform=\"rotate(-90,12,%d)\">est. bytes (log)</text>\n",
            pad_t + plot_h / 2, pad_t + plot_h / 2);

    fprintf(f, "</svg></div>\n");
  }

  /* ── Per-Model Statistics ── */
  /* sort by bits saved descending */
  int order[MAX_SEARCH];
  for (int m = 0; m < s->num_models; m++)
    order[m] = m;
  for (int i = 0; i < s->num_models - 1; i++)
    for (int j = i + 1; j < s->num_models; j++)
      if (s->model_bits_saved[order[i]] < s->model_bits_saved[order[j]]) {
        int tmp = order[i];
        order[i] = order[j];
        order[j] = tmp;
      }

  double max_saved = 0;
  for (int m = 0; m < s->num_models; m++) {
    double v = s->model_bits_saved[m]; /* positive = model helped */
    if (v > max_saved)
      max_saved = v;
    if (-v > max_saved)
      max_saved = -v;
  }

  fprintf(f, "<div class=\"sec\"><h2>Per-Model Statistics</h2>\n"
             "<p class=\"d\">Sorted by contribution. Mask selects preceding "
             "context bytes; "
             "weight controls blend strength. Positive bits saved = model "
             "helped.</p>\n"
             "<table><tr><th>#</th><th>Mask</th><th class=\"r\">Weight</th>"
             "<th class=\"r\">Hits</th><th class=\"r\">Hit %%</th>"
             "<th class=\"r\">Unique Ctx</th>"
             "<th class=\"r\">Bits Saved</th>"
             "<th class=\"r\">Bytes Saved</th></tr>\n");

  double total_saved = 0;
  for (int i = 0; i < s->num_models; i++) {
    int m = order[i];
    unsigned int tot = s->model_hits[m] + s->model_misses[m];
    double pct = tot ? 100.0 * s->model_hits[m] / tot : 0.0;
    double bits = s->model_bits_saved[m]; /* positive = helped */
    total_saved += bits;
    double abs_bits = bits < 0 ? -bits : bits;
    double bar_w = max_saved > 0 ? 100.0 * abs_bits / max_saved : 0;
    const char *cls = bits > max_saved * 0.3   ? "B"
                      : bits > max_saved * 0.1 ? "G"
                      : bits > 0               ? "M"
                                               : "W";
    const char *bcls = bits > 0 ? "bar-g" : "bar-r";
    fprintf(f,
            "<tr><td class=\"r\">%d</td><td class=\"n\">%02X</td>"
            "<td class=\"r\">%d</td><td class=\"r\">%u</td>"
            "<td class=\"r\">%.1f</td><td class=\"r\">%u</td>"
            "<td class=\"r bar-cell\">"
            "<div class=\"bar %s\" style=\"width:%.0f%%\"></div>"
            "<span class=\"bar-label %s\">%.1f</span></td>"
            "<td class=\"r\">%.1f</td></tr>\n",
            m, s->model_masks[m], s->model_weights[m], s->model_hits[m], pct,
            s->model_misses[m], bcls, bar_w, cls, bits, bits / 8.0);
  }
  fprintf(f,
          "<tr style=\"border-top:2px solid #d0d7de\">"
          "<td colspan=\"6\" class=\"n\">Total</td>"
          "<td class=\"r B\">%.1f</td><td class=\"r B\">%.1f</td></tr>\n",
          total_saved, total_saved / 8.0);
  fprintf(f, "</table></div>\n");

  /* ── Prediction Confidence ── */
  unsigned int max_conf = 0;
  for (int i = 0; i < 11; i++)
    if (s->conf_hist[i] > max_conf)
      max_conf = s->conf_hist[i];

  const char *conf_labels[] = {"&lt;50%", "50-55%", "55-60%", "60-65%",
                               "65-70%",  "70-75%", "75-80%", "80-85%",
                               "85-90%",  "90-95%", "95-100%"};

  fprintf(f, "<div class=\"sec\"><h2>Prediction Confidence</h2>\n"
             "<p class=\"d\">Distribution of prediction confidence when "
             "encoding each bit. "
             "Higher confidence means better prediction.</p>\n"
             "<table style=\"max-width:520px\">"
             "<tr><th>Confidence</th><th class=\"r\">Bits</th>"
             "<th class=\"r\">%%</th><th>Distribution</th></tr>\n");

  for (int i = 0; i < 11; i++) {
    if (s->conf_hist[i] == 0)
      continue;
    double pct = 100.0 * s->conf_hist[i] / s->total_bits;
    double bar_w = max_conf > 0 ? 100.0 * s->conf_hist[i] / max_conf : 0;
    const char *cls = i >= 8   ? "B"
                      : i >= 5 ? "G"
                      : i >= 2 ? "M"
                      : i >= 1 ? "D"
                               : "W";
    const char *bcls = i >= 8   ? "bar-g"
                       : i >= 5 ? "bar-b"
                       : i >= 2 ? "bar-y"
                       : i >= 1 ? "bar-o"
                                : "bar-r";
    fprintf(f,
            "<tr><td class=\"n\">%s</td>"
            "<td class=\"r\">%u</td><td class=\"r\">%.1f</td>"
            "<td class=\"bar-cell\">"
            "<div class=\"bar %s\" style=\"width:%.0f%%;opacity:.25\"></div>"
            "<span class=\"bar-label %s\">&nbsp;</span></td></tr>\n",
            conf_labels[i], s->conf_hist[i], pct, bcls, bar_w, cls);
  }
  fprintf(f, "</table></div>\n");

  /* ── Byte Position Analysis ── */
  double max_bpc = 0;
  for (int i = 0; i < 8; i++) {
    if (s->bytepos_count[i] > 0) {
      double c = s->bytepos_cost[i] / s->bytepos_count[i];
      if (c > max_bpc)
        max_bpc = c;
    }
  }

  fprintf(
      f,
      "<div class=\"sec\"><h2>Byte Position Analysis</h2>\n"
      "<p class=\"d\">Average encoding cost per bit position within each byte. "
      "Lower is better.</p>\n"
      "<table style=\"max-width:520px\">"
      "<tr><th>Bit Position</th><th class=\"r\">Count</th>"
      "<th class=\"r\">Avg Cost</th><th>Cost</th></tr>\n");

  for (int i = 0; i < 8; i++) {
    if (s->bytepos_count[i] == 0)
      continue;
    double avg = s->bytepos_cost[i] / s->bytepos_count[i];
    double bar_w = max_bpc > 0 ? 100.0 * avg / max_bpc : 0;
    const char *cls = avg < max_bpc * 0.4   ? "B"
                      : avg < max_bpc * 0.6 ? "G"
                      : avg < max_bpc * 0.8 ? "M"
                                            : "D";
    const char *bcls = avg < max_bpc * 0.4   ? "bar-g"
                       : avg < max_bpc * 0.6 ? "bar-b"
                       : avg < max_bpc * 0.8 ? "bar-y"
                                             : "bar-o";
    fprintf(f,
            "<tr><td class=\"n\">Bit %d</td>"
            "<td class=\"r\">%u</td>"
            "<td class=\"r %s\">%.3f bits</td>"
            "<td class=\"bar-cell\">"
            "<div class=\"bar %s\" style=\"width:%.0f%%;opacity:.25\"></div>"
            "<span class=\"bar-label\">&nbsp;</span></td></tr>\n",
            i, s->bytepos_count[i], cls, avg, bcls, bar_w);
  }
  fprintf(f, "</table></div>\n");

  /* ── Hash Table Statistics ── */
  double load = s->ht_size ? 100.0 * s->ht_occupied / s->ht_size : 0;
  unsigned int total_sat =
      s->sat_lopsided + s->sat_strong + s->sat_balanced + s->sat_mixed;

  fprintf(
      f,
      "<div class=\"sec\"><h2>Hash Table</h2>\n"
      "<p class=\"d\">Context hash table occupancy and probe statistics.</p>\n"
      "<table class=\"kvtbl\">"
      "<tr><th>Metric</th><th class=\"r\">Value</th></tr>\n");
  fprintf(f,
          "<tr><td class=\"n\">Table size</td>"
          "<td class=\"r\">%u slots</td></tr>\n",
          s->ht_size);
  fprintf(f,
          "<tr><td class=\"n\">Occupied</td>"
          "<td class=\"r %s\">%u (%.1f%%)</td></tr>\n",
          load < 50   ? "G"
          : load < 75 ? "M"
                      : "D",
          s->ht_occupied, load);
  fprintf(f,
          "<tr><td class=\"n\">Max probe chain</td>"
          "<td class=\"r\">%u</td></tr>\n",
          s->ht_max_chain);
  fprintf(f,
          "<tr><td class=\"n\">Avg displacement</td>"
          "<td class=\"r\">%.2f</td></tr>\n",
          s->ht_avg_displacement);
  fprintf(f, "</table></div>\n");

  /* ── Counter Saturation ── */
  fprintf(
      f,
      "<div class=\"sec\"><h2>Counter Saturation</h2>\n"
      "<p class=\"d\">Distribution of probability counter balance across hash "
      "table entries.</p>\n"
      "<table style=\"max-width:520px\">"
      "<tr><th>Category</th><th class=\"r\">Count</th>"
      "<th class=\"r\">%%</th><th>Distribution</th></tr>\n");

  struct {
    const char *name;
    unsigned int count;
    const char *cls;
    const char *bcls;
  } cats[] = {
      {"Lopsided (one side = 0)", s->sat_lopsided, "B", "bar-g"},
      {"Strong (&gt;4:1)", s->sat_strong, "G", "bar-b"},
      {"Balanced (&lt;2:1)", s->sat_balanced, "M", "bar-y"},
      {"Mixed (2:1 to 4:1)", s->sat_mixed, "D", "bar-o"},
  };
  unsigned int max_sat = 0;
  for (int i = 0; i < 4; i++)
    if (cats[i].count > max_sat)
      max_sat = cats[i].count;

  for (int i = 0; i < 4; i++) {
    if (cats[i].count == 0)
      continue;
    double pct = total_sat ? 100.0 * cats[i].count / total_sat : 0;
    double bar_w = max_sat ? 100.0 * cats[i].count / max_sat : 0;
    fprintf(f,
            "<tr><td class=\"n\">%s</td>"
            "<td class=\"r\">%u</td><td class=\"r\">%.1f</td>"
            "<td class=\"bar-cell\">"
            "<div class=\"bar %s\" style=\"width:%.0f%%;opacity:.25\"></div>"
            "<span class=\"bar-label\">&nbsp;</span></td></tr>\n",
            cats[i].name, cats[i].count, pct, cats[i].bcls, bar_w);
  }
  fprintf(f, "</table></div>\n");

  fprintf(f, "</body></html>\n");
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
  printf("  -1/-2/-3     Compression level (default: -%d)\n", DEFAULT_LEVEL);
  printf("  -m <models>  Use explicit models, skip search (e.g. \"00:1 80:2 "
         "C0:3\")\n");
  printf("  -w           Optimize weights on explicit models from -m\n");
  printf("  -b <n>       Base probability (default: %d)\n", DEFAULT_BPROB);
  printf("  -H <file>    Write HTML stats report to file\n");
  printf("  -v           Verbose output (use -vv for very verbose)\n");
  printf("  -p <n>       Max search passes (default: 1)\n");
  printf("  -h           Show this help\n");
}

int main(int argc, char *argv[]) {
  const char *output_file = NULL;
  int level = DEFAULT_LEVEL;
  int base_prob = DEFAULT_BPROB;
  int decompress = 0;
  int max_passes = 1;
  ModelSet explicit_models = {0};
  int have_explicit_models = 0;
  int optimize_explicit_weights = 0;

  int opt;
  while ((opt = getopt(argc, argv, "o:m:b:p:H:123dwhv")) != -1) {
    switch (opt) {
    case 'o':
      output_file = optarg;
      break;
    case 'd':
      decompress = 1;
      break;
    case '1':
      level = 1;
      break;
    case '2':
      level = 2;
      break;
    case '3':
      level = 3;
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
  printf("Level:       %d\n", level);
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
      CompState *cs = state_new(data, data_size, base_prob, &eval, ctx);
      est_size = optimize_weights(cs, &ml);
      state_destroy(cs);
      eval_destroy(&eval);
      printf("Optimized:   ");
      model_set_print(&ml, stdout);
      printf("Estimated:   %.3f bytes\n", est_size / (float)(BIT_PREC * 8));
    } else {
      printf("Skipping search, using %d explicit models\n", ml.num_models);
    }
  } else {
    ml = search_best_models(data, data_size, ctx, level, base_prob, &est_size,
                            NULL, html_output ? &cstats : NULL);

    for (int pass = 2; pass <= max_passes; pass++) {
      int prev_size = est_size;
      if (verbose)
        printf("\n  Pass %d (seeded with %d models):\n", pass, ml.num_models);
      ml = search_best_models(data, data_size, ctx, level, base_prob, &est_size,
                              &ml, NULL);
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
    cstats.level = level;
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

    write_html_report(html_output, &cstats);
    printf("HTML report: %s\n", html_output);
    free(cstats.byte_costs);
    free(cstats.search_best);
    free(cstats.search_events);
  }

  free(out_buf);
  free(data);
  return 0;
}
