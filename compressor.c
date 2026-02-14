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
#define MAX_STREAM 32
#define LOG2_NPV 4
#define NPV (1 << LOG2_NPV)
#define PKG_BITS (NPV * 4)

enum { CT_INSTANT = 0, CT_FAST, CT_SLOW, CT_VERYSLOW };
static const char *const CT_NAMES[] = {"INSTANT", "FAST", "SLOW", "VERYSLOW"};

typedef struct {
  unsigned char weight;
  unsigned char mask;
} Model;

typedef struct {
  Model models[MAX_MODELS];
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
  int arena_offset;
  int map_offset;
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
} CtxEntry;

typedef struct {
  unsigned int hash;
  unsigned char prob[2];
  unsigned char used;
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
  ModelPred *model_info;
  CompactPredBlock *block_arena;
  int *map_arena;
  int length;
  PredBlock *accum;
  unsigned *accum_sizes;
  int64_t cost;
  int base_prob;
  float log_scale;
} Evaluator;

typedef struct {
  ModelPred models[256];
  CompactPredBlock *block_arena;
  int *map_arena;
  int arena_blocks;
  int arena_maps;
  Evaluator *eval;
} CompState;

static inline v4f v4f_splat(float x) { return (v4f){x, x, x, x}; }
static inline v4u v4u_splat(uint32_t x) { return (v4u){x, x, x, x}; }

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

static CtxEntry *ctx_table_probe(CtxEntry *table, unsigned int table_size,
                                 unsigned char mask, const unsigned char *data,
                                 int bit_pos) {
  const unsigned char *dp = &data[bit_pos / 8];
  unsigned char bn = (unsigned char)(bit_pos & 7);
  for (unsigned int h = ctx_hash(data, bit_pos, mask);; h++) {
    CtxEntry *e = &table[h % table_size];
    if (e->data_ptr == NULL) {
      e->mask = mask;
      e->bitnum = bn;
      e->data_ptr = dp;
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
                                CompactPredBlock *out_blocks,
                                int *out_offsets) {
  int max_blocks = (total_bits + PKG_BITS - 1) / PKG_BITS;
  int num_blocks = 0;
  memset(ht, 0, ht_size * sizeof(CtxEntry));

  for (int bi = 0; bi < max_blocks; bi++) {
    int base = bi * PKG_BITS;
    int needs_commit = 0;
    for (int off = 0; off < PKG_BITS; off++) {
      float rp = 0, tp = 0;
      if (base + off < total_bits) {
        int bit = get_bit(data, base + off);
        CtxEntry *e = ctx_table_probe(ht, ht_size, mask, data, base + off);
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

static void eval_setup(Evaluator *ev, ModelPred *models,
                       CompactPredBlock *block_arena, int *map_arena,
                       int length, int base_prob, float log_scale) {
  ev->model_info = models;
  ev->block_arena = block_arena;
  ev->map_arena = map_arena;
  ev->length = length;
  ev->base_prob = base_prob;
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

static int64_t eval_adjust(const Evaluator *ev, int model_idx, int delta) {
  const ModelPred *mp = &ev->model_info[model_idx];
  const int nblocks = mp->num_blocks;
  const CompactPredBlock *mblocks = &ev->block_arena[mp->arena_offset];
  const int *mmap = &ev->map_arena[mp->map_offset];

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
    cs->models[m].arena_offset = cursor;
    cs->models[m].map_offset = cursor;
    cs->models[m].num_blocks =
        compute_single_model(dp, total_bits, (unsigned char)m, ht, ht_size,
                             &cs->block_arena[cursor], &cs->map_arena[cursor]);
    cursor += cs->models[m].num_blocks;
  }
  free(ht);
  free(padded);

  cs->arena_blocks = cursor;
  cs->arena_maps = cursor;

  eval_setup(eval, cs->models, cs->block_arena, cs->map_arena, total_bits,
             base_prob, log_scale);
  return cs;
}

static void state_destroy(CompState *cs) {
  free(cs->block_arena);
  free(cs->map_arena);
  free(cs);
}

static int state_get_size(const CompState *cs) {
  return (int)(cs->eval->cost / (TPREC / BIT_PREC));
}

static int state_set_models(CompState *cs, const ModelSet *ml) {
  eval_evaluate(cs->eval, ml);
  return state_get_size(cs);
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

  unsigned int ext_masks[MAX_STREAM];
  unsigned char ctx_masks[MAX_STREAM];
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
  unsigned int slot = hash & mask;
  for (;;) {
    HashEntry *e = &table[slot];
    if (!e->used) {
      e->hash = hash;
      e->used = 1;
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

static void encode_from_stream(ArithCoder *ac, const HashBitStream *hb,
                               HashEntry *ht, int base_prob) {
  int num = hb->num_weights;
  int total_bits = (num == 0) ? hb->bits_len : (int)hb->hashes_len / num;
  unsigned int tmask = hb->table_size - 1;
  memset(ht, 0, hb->table_size * sizeof(HashEntry));

  HashEntry *matched[MAX_STREAM];
  int hpos = 0;
  for (int bp = 0; bp < total_bits; bp++) {
    int bit = hb->bits[bp];
    unsigned int probs[2] = {(unsigned)base_prob, (unsigned)base_prob};
    for (int m = 0; m < num; m++)
      matched[m] =
          hash_probe(ht, tmask, hb->hashes[hpos++], hb->weights[m], probs);
    arith_encode(ac, probs[1], probs[0], 1 - bit);
    for (int m = 0; m < num; m++)
      counter_update(matched[m]->prob, bit);
  }
}

static int compress_4k(const unsigned char *data, int size, unsigned char *out,
                       int max_out, const ModelSet *ml, int base_prob) {
  unsigned char ctx[MAX_CTX] = {};
  HashBitStream hb = compute_hash_stream(data, size, ctx, ml, 1, 1);
  HashEntry *ht = (HashEntry *)calloc(hb.table_size, sizeof(HashEntry));

  memset(out, 0, max_out);
  ArithCoder ac;
  arith_init(&ac, out);
  encode_from_stream(&ac, &hb, ht, base_prob);
  int total = arith_finish(&ac);

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

  unsigned char ctx_masks[MAX_STREAM];
  for (int i = 0; i < num; i++)
    ctx_masks[i] = cdata[7 + i];

  unsigned int ext_masks[MAX_STREAM];
  int weights[MAX_STREAM];
  decode_weight_mask(stored_wmask, num, ctx_masks, weights, ext_masks);

  const unsigned char *comp = cdata + 7 + num;
  unsigned char *buf = (unsigned char *)calloc(data_bytes + MAX_CTX, 1);
  unsigned char *dp = buf + MAX_CTX;
  unsigned int tsize = next_pow2(bitlen * num);
  HashEntry *ht = (HashEntry *)calloc(tsize, sizeof(HashEntry));
  unsigned int tmask = tsize - 1;

  unsigned int range = 0x80000000u, low = 0, value = 0;
  int cpos = 0;
  for (int i = 0; i < 31; i++)
    value = (value << 1) | get_compressed_bit(comp, cpos++);

  for (int bp = 0; bp < bitlen; bp++) {
    unsigned int probs[2] = {(unsigned)base_prob, (unsigned)base_prob};
    HashEntry *matched[MAX_STREAM];

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
  free(buf);
  free(ht);
  return data_bytes;
}

static unsigned int approximate_weights(CompState *cs, ModelSet *ml) {
  for (int i = 0; i < ml->num_models; i++)
    ml->models[i].weight = __builtin_popcount(ml->models[i].mask);
  return state_set_models(cs, ml);
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
      unsigned int trial = state_set_models(cs, &cand);
      if (trial < best) {
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

static unsigned int try_weights(CompState *cs, ModelSet *ml, int mode) {
  return (mode == CT_SLOW || mode == CT_VERYSLOW) ? optimize_weights(cs, ml)
                                                  : approximate_weights(cs, ml);
}

static int model_set_cmp(const void *a, const void *b) {
  int sa = ((const ModelSet *)a)->size;
  int sb = ((const ModelSet *)b)->size;
  return (sa > sb) - (sa < sb);
}

static ModelSet search_best_models(const unsigned char *data, int size,
                                   const unsigned char ctx[MAX_CTX], int mode,
                                   int base_prob, int *out_size) {
  const int beam = (mode == CT_VERYSLOW) ? 3 : 1;
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

  sets[0].size = state_get_size(cs) | EFLAG;
  for (int s = 1; s < beam; s++)
    sets[s].size = INT_MAX;

  for (int mi = 0; mi <= 255; mi++) {
    int mask = rev_masks[mi];

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
        *next = *cur;
        next->models[cur->num_models].mask = (unsigned char)mask;
        next->models[cur->num_models].weight = 0;
        next->num_models = cur->num_models + 1;

        int old_sz = cur->size & ~EFLAG;
        int new_sz = try_weights(cs, next, mode);

        if (new_sz < old_sz || mode == CT_VERYSLOW) {
          int best_sz = new_sz;

          for (int m = next->num_models - 2; m >= 0; m--) {
            Model removed = next->models[m];
            next->num_models--;
            next->models[m] = next->models[next->num_models];
            int trial = try_weights(cs, next, mode);
            if (trial < best_sz) {
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
    printf("\rCalculating models... %d/%d", mi + 1, 256);
    fflush(stdout);
  }

  assert((sets[0].size & EFLAG) != 0);
  sets[0].size &= ~EFLAG;
  qsort(sets, nsets, sizeof(ModelSet), model_set_cmp);

  ModelSet best = sets[0];
  int final_sz = optimize_weights(cs, &best);
  if (out_size)
    *out_size = final_sz;

  state_destroy(cs);
  eval_destroy(&eval);
  free(sets);
  return best;
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

static void print_usage(const char *prog) {
  printf("Usage: %s [options] <input_file>\n\nOptions:\n", prog);
  printf("  -o <file>    Output file (default: <input>.paq or <input>.bin)\n");
  printf("  -d           Decompress mode\n");
  printf("  -m <mode>    Compression mode: instant, fast, slow, veryslow "
         "(default: slow)\n");
  printf("  -b <n>       Base probability (default: %d)\n", DEFAULT_BPROB);
  printf("  -h           Show this help\n");
}

int main(int argc, char *argv[]) {
  const char *output_file = NULL;
  int mode = CT_SLOW;
  int base_prob = DEFAULT_BPROB;
  int decompress = 0;

  int opt;
  while ((opt = getopt(argc, argv, "o:m:b:dh")) != -1) {
    switch (opt) {
    case 'o':
      output_file = optarg;
      break;
    case 'd':
      decompress = 1;
      break;
    case 'm':
      if (!strcmp(optarg, "instant"))
        mode = CT_INSTANT;
      else if (!strcmp(optarg, "fast"))
        mode = CT_FAST;
      else if (!strcmp(optarg, "slow"))
        mode = CT_SLOW;
      else if (!strcmp(optarg, "veryslow"))
        mode = CT_VERYSLOW;
      else {
        fprintf(stderr, "Unknown mode: %s\n", optarg);
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

  static char default_out[512];

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

    int bitlen = data[0] | (data[1] << 8);
    int out_sz = (bitlen - 1) / 8;
    printf("Bitlength:   %d (%d data bytes)\n", bitlen, out_sz);
    printf("Models:      %d\n", data[6]);

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

  printf("Input:       %s (%d bytes)\n", input_file, data_size);
  printf("Mode:        %s\n", CT_NAMES[mode]);
  printf("Base prob:   %d\n", base_prob);

  unsigned char ctx[MAX_CTX] = {};
  int est_size = 0;
  ModelSet ml =
      search_best_models(data, data_size, ctx, mode, base_prob, &est_size);

  printf("\nEstimated:   %.3f bytes\n", est_size / (float)(BIT_PREC * 8));
  printf("Models:      ");
  model_set_print(&ml, stdout);

  int max_out = data_size + 1024;
  unsigned char *out_buf = (unsigned char *)malloc(max_out);
  int comp_bits =
      compress_4k(data, data_size, out_buf, max_out, &ml, base_prob);
  int comp_bytes = (comp_bits + 7) / 8;
  printf("Compressed:  %d bytes %d bits (%.2f%%)\n", comp_bytes, comp_bits,
         100.0f * comp_bytes / data_size);

  unsigned char ordered_masks[MAX_MODELS];
  unsigned int wmask = encode_weight_mask(&ml, ordered_masks, 1);

  int bitlen = data_size * 8 + 1;
  if (bitlen > 65535) {
    fprintf(stderr, "Input too large for 16-bit bitlength (%d bits)\n", bitlen);
    free(out_buf);
    free(data);
    return 1;
  }

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

  printf("Output:      %s (%d bytes, weightmask %08X)\n", output_file,
         7 + ml.num_models + comp_bytes, wmask);

  free(out_buf);
  free(data);
  return 0;
}
