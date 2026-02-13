#include <assert.h>
#include <getopt.h>
#include <limits.h>
#include <math.h>
#include <pthread.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

typedef float v4f __attribute__((vector_size(16)));
typedef uint32_t v4u __attribute__((vector_size(16)));
typedef int32_t v4i __attribute__((vector_size(16)));
typedef uint16_t v8u16 __attribute__((vector_size(16)));

static inline unsigned int bsr(unsigned int value) {
  return value ? 31 - __builtin_clz(value) : 0;
}

static inline void *aligned_alloc32(size_t size) {
  void *ptr = NULL;
  posix_memalign(&ptr, 32, size);
  return ptr;
}

#define TABLE_BIT_PRECISION_BITS 12
#define TABLE_BIT_PRECISION (1 << TABLE_BIT_PRECISION_BITS)
#define HMUL 111
#define MAX_CTX 8
#define DEFAULT_BASEPROB 10
#define BIT_PRECISION 256
#define MAX_MODELS_N 256
#define MAX_N_SEARCH 21
#define MAX_MODEL_WT 9
#define MAX_N_STREAM 32
#define LOG2_NPV 4
#define NPV (1 << LOG2_NPV)
#define PKG_SIZE (NPV * 4)

enum { CT_INSTANT = 0, CT_FAST, CT_SLOW, CT_VERYSLOW };

#define PAR_THRESHOLD 32

typedef struct {
  void (*fn)(int, void *);
  void *ctx;
  int next;
  int end;
  pthread_mutex_t mtx;
} ParJob;

static int g_nthreads;
static pthread_t *g_threads;
static ParJob g_job;
static pthread_mutex_t g_start_mtx = PTHREAD_MUTEX_INITIALIZER;
static pthread_cond_t g_start_cv = PTHREAD_COND_INITIALIZER;
static pthread_cond_t g_done_cv = PTHREAD_COND_INITIALIZER;
static int g_generation = 0;
static int g_active = 0;
static int g_shutdown = 0;

static void par_execute(void) {
  for (;;) {
    pthread_mutex_lock(&g_job.mtx);
    int item = g_job.next++;
    pthread_mutex_unlock(&g_job.mtx);
    if (item >= g_job.end) {
      break;
    }
    g_job.fn(item, g_job.ctx);
    pthread_mutex_lock(&g_start_mtx);
    g_active--;
    if (g_active == 0) {
      pthread_cond_signal(&g_done_cv);
    }
    pthread_mutex_unlock(&g_start_mtx);
  }
}

static void *par_worker(void *arg) {
  (void)arg;
  int last_gen = 0;
  for (;;) {
    pthread_mutex_lock(&g_start_mtx);
    while (!g_shutdown && g_generation == last_gen) {
      pthread_cond_wait(&g_start_cv, &g_start_mtx);
    }
    if (g_shutdown) {
      pthread_mutex_unlock(&g_start_mtx);
      return NULL;
    }
    last_gen = g_generation;
    pthread_mutex_unlock(&g_start_mtx);
    par_execute();
  }
}

static void par_init(void) {
  g_nthreads = (int)sysconf(_SC_NPROCESSORS_ONLN);
  if (g_nthreads <= 0) {
    g_nthreads = 4;
  }
  if (g_nthreads > 1) {
    g_nthreads--;
  }
  pthread_mutex_init(&g_job.mtx, NULL);
  g_threads = (pthread_t *)malloc(g_nthreads * sizeof(pthread_t));
  for (int i = 0; i < g_nthreads; i++) {
    pthread_create(&g_threads[i], NULL, par_worker, NULL);
  }
}

static void par_shutdown(void) {
  pthread_mutex_lock(&g_start_mtx);
  g_shutdown = 1;
  pthread_cond_broadcast(&g_start_cv);
  pthread_mutex_unlock(&g_start_mtx);
  for (int i = 0; i < g_nthreads; i++) {
    pthread_join(g_threads[i], NULL);
  }
  free(g_threads);
}

static void parallel_for(const int begin, const int end,
                         void (*fn)(int, void *), void *ctx) {
  const int count = end - begin;
  if (count <= 0) {
    return;
  }
  if (count < PAR_THRESHOLD) {
    for (int i = begin; i < end; i++) {
      fn(i, ctx);
    }
    return;
  }
  pthread_mutex_lock(&g_job.mtx);
  g_job.fn = fn;
  g_job.ctx = ctx;
  g_job.next = begin;
  g_job.end = end;
  pthread_mutex_unlock(&g_job.mtx);

  pthread_mutex_lock(&g_start_mtx);
  g_active = count;
  g_generation++;
  pthread_cond_broadcast(&g_start_cv);
  pthread_mutex_unlock(&g_start_mtx);

  par_execute();

  pthread_mutex_lock(&g_start_mtx);
  while (g_active > 0) {
    pthread_cond_wait(&g_done_cv, &g_start_mtx);
  }
  pthread_mutex_unlock(&g_start_mtx);
}

#define MAX_ACCUM_THREADS 128

typedef struct {
  int64_t vals[MAX_ACCUM_THREADS];
} Combinable;

static void comb_init(Combinable *comb) { memset(comb, 0, sizeof(*comb)); }

static int64_t *comb_local(Combinable *comb) {
  const unsigned slot = (unsigned)(uintptr_t)pthread_self() % MAX_ACCUM_THREADS;
  return &comb->vals[slot];
}

static int64_t comb_sum(const Combinable *comb) {
  int64_t total = 0;
  for (int i = 0; i < MAX_ACCUM_THREADS; i++) {
    total += comb->vals[i];
  }
  return total;
}

#define LOG_TABLE_SIZE (2 * TABLE_BIT_PRECISION)
static int LogTable[LOG_TABLE_SIZE];

static void InitLogTable(void) {
  LogTable[0] = 0;
  for (int i = 1; i < LOG_TABLE_SIZE; i++) {
    double val = log2((double)i / TABLE_BIT_PRECISION) * TABLE_BIT_PRECISION;
    LogTable[i] = (int)(val + 0.5);
  }
}

typedef struct {
  void *dest_ptr;
  unsigned int dest_bit;
  unsigned int interval_size;
  unsigned int interval_min;
} AritState;

typedef struct {
  unsigned short boosted_counters[2];
  unsigned short next_state[2];
} CounterState;

typedef struct {
  unsigned char weight;
  unsigned char mask;
} Model;

typedef struct {
  Model models[MAX_MODELS_N];
  int nmodels;
  int size;
} ModelList4k;

typedef struct {
  unsigned char prob[2];
} Weights;

typedef struct {
  uint16_t prob[NPV][8];
} CompactPackage;

typedef struct {
  v4f prob[NPV][2];
} Package;

typedef struct {
  int num_packages;
  CompactPackage *packages;
  int *package_offsets;
} ModelPredictions;

typedef struct {
  unsigned int hashes_cap;
  unsigned int hashes_len;
  unsigned int *hashes;
  int bits_cap;
  int bits_len;
  unsigned char *bits;
  int *weights;
  int num_weights;
  unsigned int tiny_hash_size;
} HashBits;

typedef struct {
  unsigned int hash;
  unsigned char prob[2];
  unsigned char used;
} TinyHashEntry;

static inline int AritSize2(int right_prob, int wrong_prob) {
  assert(right_prob > 0 && wrong_prob > 0);
  const int total_prob = right_prob + wrong_prob;
  if (total_prob < TABLE_BIT_PRECISION) {
    return LogTable[total_prob] - LogTable[right_prob];
  }
  unsigned int right_shift = bsr(right_prob);
  unsigned int total_shift = bsr(total_prob);
  right_shift = right_shift > 12 ? right_shift - 12 : 0;
  total_shift = total_shift > 12 ? total_shift - 12 : 0;
  return LogTable[total_prob >> total_shift] -
         LogTable[right_prob >> right_shift] +
         ((total_shift - right_shift) << 12);
}

static void AritCodeInit(AritState *state, void *dest) {
  state->dest_ptr = dest;
  state->dest_bit = (unsigned)-1;
  state->interval_size = 0x80000000;
  state->interval_min = 0;
}

static void PutBit(unsigned char *dest, int bit_pos) {
  do {
    --bit_pos;
    if (bit_pos < 0) {
      return;
    }
    const unsigned int mask = 1u << (bit_pos & 7);
    const unsigned int byte_val = dest[bit_pos >> 3];
    dest[bit_pos >> 3] = byte_val ^ mask;
    if (!(byte_val & mask)) {
      break;
    }
  } while (1);
}

static void AritCode(AritState *state, const unsigned int zero_prob,
                     const unsigned int one_prob, const int bit) {
  unsigned char *dest = (unsigned char *)state->dest_ptr;
  unsigned int bit_pos = state->dest_bit;
  unsigned int interval_min = state->interval_min;
  unsigned int interval_size = state->interval_size;
  const unsigned int total_prob = zero_prob + one_prob;
  const unsigned int threshold =
      (uint64_t)interval_size * zero_prob / total_prob;
  if (bit) {
    const unsigned int old_min = interval_min;
    interval_min += threshold;
    if (interval_min < old_min) {
      PutBit(dest, bit_pos);
    }
    interval_size -= threshold;
  } else {
    interval_size = threshold;
  }
  while (interval_size < 0x80000000) {
    bit_pos++;
    if (interval_min & 0x80000000) {
      PutBit(dest, bit_pos);
    }
    interval_min <<= 1;
    interval_size <<= 1;
  }
  state->dest_bit = bit_pos;
  state->interval_min = interval_min;
  state->interval_size = interval_size;
}

static int AritCodeEnd(AritState *state) {
  unsigned char *dest = (unsigned char *)state->dest_ptr;
  unsigned int bit_pos = state->dest_bit;
  const unsigned int interval_min = state->interval_min;
  const unsigned int interval_size = state->interval_size;
  if (interval_min > 0) {
    if (interval_min + interval_size - 1 >= interval_min) {
      bit_pos++;
    }
    PutBit(dest, bit_pos);
  }
  return bit_pos;
}

static CounterState unsat_states[1471];
static int counter_state_map[256][256];

static int CSVisit(CounterState *states, int *num_states, const int max_states,
                   unsigned char count0, unsigned char count1,
                   int const new_bit) {
  if (new_bit == 0) {
    count0++;
    if (count1 > 1) {
      count1 >>= 1;
    }
  } else {
    count1++;
    if (count0 > 1) {
      count0 >>= 1;
    }
  }

  if (counter_state_map[count1][count0] != -1) {
    return counter_state_map[count1][count0];
  }

  const int boost = (count0 == 0 || count1 == 0) ? 2 : 0;
  const int state_idx = (*num_states);
  counter_state_map[count1][count0] = state_idx;
  assert(*num_states < max_states);
  CounterState *entry = &states[(*num_states)++];
  entry->boosted_counters[0] = count0 << boost;
  entry->boosted_counters[1] = count1 << boost;
  states[state_idx].next_state[0] =
      CSVisit(states, num_states, max_states, count0, count1, 0);
  states[state_idx].next_state[1] =
      CSVisit(states, num_states, max_states, count0, count1, 1);
  return state_idx;
}

static void GenCounterStates(CounterState *states, const int max_states) {
  memset(counter_state_map, -1, sizeof(counter_state_map));
  counter_state_map[0][1] = 0;
  counter_state_map[1][0] = 1;
  int num_states = 0;

  assert(num_states < max_states);
  CounterState *first_zero = &states[num_states++];
  first_zero->boosted_counters[0] = 1 << 2;
  first_zero->boosted_counters[1] = 0;

  assert(num_states < max_states);
  CounterState *first_one = &states[num_states++];
  first_one->boosted_counters[0] = 0;
  first_one->boosted_counters[1] = 1 << 2;

  states[0].next_state[0] = CSVisit(states, &num_states, max_states, 1, 0, 0);
  states[0].next_state[1] = CSVisit(states, &num_states, max_states, 1, 0, 1);
  states[1].next_state[0] = CSVisit(states, &num_states, max_states, 0, 1, 0);
  states[1].next_state[1] = CSVisit(states, &num_states, max_states, 0, 1, 1);
  assert(num_states == max_states);
}

static void InitCounterStates(void) { GenCounterStates(unsat_states, 1471); }

static inline int GetBit(const unsigned char *data, const int bit_pos) {
  return (data[bit_pos >> 3] >> (7 - (bit_pos & 7))) & 1;
}

static unsigned int ModelHashStart(const unsigned int mask,
                                   const int hash_mul) {
  unsigned char data_len = (unsigned char)mask;
  unsigned int hash = mask;
  hash = hash * hash_mul - 1;
  while (data_len) {
    if (data_len & 0x80) {
      hash = hash * hash_mul - 1;
    }
    data_len += data_len;
  }
  return hash;
}

static unsigned int ModelHash(const unsigned char *data, const int bit_pos,
                              const unsigned int mask, const int hash_mul) {
  unsigned char data_len = (unsigned char)mask;
  const unsigned char *ptr = data + (bit_pos >> 3);
  unsigned int hash = mask;
  unsigned char cur_byte = (0x100 | *ptr) >> ((~bit_pos & 7) + 1);
  hash ^= cur_byte;
  hash *= hash_mul;
  hash = (hash & 0xFFFFFF00) | ((hash + cur_byte) & 0xFF);
  hash--;
  while (data_len) {
    ptr--;
    if (data_len & 0x80) {
      cur_byte = *ptr;
      hash ^= cur_byte;
      hash *= hash_mul;
      hash = (hash & 0xFFFFFF00) | ((hash + cur_byte) & 0xFF);
      hash--;
    }
    data_len += data_len;
  }
  return hash;
}

static ModelList4k ML_copy(const ModelList4k *src) { return *src; }

static unsigned int ML_GetMaskList(const ModelList4k *ml, unsigned char *masks,
                                   const int terminate) {
  unsigned int weightmask = 0;
  int remaining = ml->nmodels;
  int bit_idx = 31;
  for (int weight = 0; remaining; weight++) {
    for (int m = 0; m < ml->nmodels; m++) {
      if (ml->models[m].weight == weight) {
        if (masks) {
          *masks++ = ml->models[m].mask;
        }
        remaining--;
        bit_idx--;
      }
    }
    weightmask |= (1u << bit_idx);
    bit_idx--;
  }
  while (bit_idx >= 0) {
    weightmask |= 1u << bit_idx;
    bit_idx--;
  }
  int parity = __builtin_parity(weightmask & 0xFF);
  return weightmask & (unsigned)(-2 + (terminate ^ parity));
}

static void ML_Print(const ModelList4k *ml, FILE *file) {
  for (int m = 0; m < ml->nmodels; m++) {
    fprintf(file, "%s%02X:%d", m ? " " : "", ml->models[m].mask,
            ml->models[m].weight);
  }
  fprintf(file, "\n");
}

static void UpdateWeights(Weights *weights, const int bit) {
  weights->prob[bit] += 1;
  if (weights->prob[!bit] > 1) {
    weights->prob[!bit] >>= 1;
  }
}

static int NextPowerOf2(int value) {
  value--;
  value |= value >> 1;
  value |= value >> 2;
  value |= value >> 4;
  value |= value >> 8;
  value |= value >> 16;
  return value + 1;
}

static int PreviousPrime(int n) {
  for (;;) {
    n = (n - 2) | 1;
    int is_prime = 1;
    for (int divisor = 3; divisor * divisor < n; divisor += 2) {
      if (n / divisor * divisor == n) {
        is_prime = 0;
        break;
      }
    }
    if (is_prime) {
      return n;
    }
  }
}

static const char *CTName(const int compression_type) {
  switch (compression_type) {
  case CT_INSTANT:
    return "INSTANT";
  case CT_FAST:
    return "FAST";
  case CT_SLOW:
    return "SLOW";
  case CT_VERYSLOW:
    return "VERYSLOW";
  }
  return "UNKNOWN";
}

typedef struct {
  int weights[256];
  ModelPredictions *models;
  int length;
  Package *packages;
  unsigned *package_sizes;
  int64_t compressed_size;
  int baseprob;
  float log_scale;
} CSEval;

static void CSEval_init(CSEval *eval) { memset(eval, 0, sizeof(*eval)); }

static void CSEval_destroy(const CSEval *eval) {
  free(eval->packages);
  free(eval->package_sizes);
}

static void CSEval_Init(CSEval *eval, ModelPredictions *models,
                        const int length, const int baseprob,
                        const float log_scale) {
  eval->length = length;
  eval->models = models;
  eval->baseprob = baseprob;
  eval->log_scale = log_scale;
  int num_packages = (length + PKG_SIZE - 1) / PKG_SIZE;
  eval->packages = (Package *)aligned_alloc32(num_packages * sizeof(Package));
  eval->package_sizes = (unsigned *)malloc(num_packages * sizeof(unsigned));
  float bv = baseprob * log_scale;
  float tv = baseprob * 2 * log_scale;
  v4f base_v = {bv, bv, bv, bv};
  v4f total_active = {tv, tv, tv, tv};
  for (int i = 0; i < num_packages; i++) {
    for (int j = 0; j < NPV; j++) {
      eval->packages[i].prob[j][0] = base_v;
      eval->packages[i].prob[j][1] =
          (i * PKG_SIZE + j * 4 < length) ? total_active : base_v;
    }
    int remain = length - i * PKG_SIZE;
    if (remain > PKG_SIZE) {
      remain = PKG_SIZE;
    }
    eval->package_sizes[i] = remain * TABLE_BIT_PRECISION;
  }
  eval->compressed_size = (int64_t)length << TABLE_BIT_PRECISION_BITS;
}

typedef struct {
  CSEval *eval;
  int model_index;
  int delta_weight;
  Combinable accumulator;
  int packages_per_job;
  int num_packages;
} ChangeWeightCtx;

static void ChangeWeight_job(const int job_idx, void *vctx) {
  ChangeWeightCtx *ctx = (ChangeWeightCtx *)vctx;
  const CSEval *eval = ctx->eval;
  const int model_idx = ctx->model_index;
  const int pkg_base = job_idx * ctx->packages_per_job;

  float dw = ctx->delta_weight * eval->log_scale;
  const v4f vdw = {dw, dw, dw, dw};
  const v4f vone = {1, 1, 1, 1};

  static const float LOG2_C0 = 1.42286530448213f;
  static const float LOG2_C1 = -0.58208536795165f;
  static const float LOG2_C2 = 0.15922006346951f;
  const v4f vc0 = {LOG2_C0, LOG2_C0, LOG2_C0, LOG2_C0};
  const v4f vc1 = {LOG2_C1, LOG2_C1, LOG2_C1, LOG2_C1};
  const v4f vc2 = {LOG2_C2, LOG2_C2, LOG2_C2, LOG2_C2};
  const v4f vprec = {TABLE_BIT_PRECISION, TABLE_BIT_PRECISION,
                     TABLE_BIT_PRECISION, TABLE_BIT_PRECISION};
  const v4u vmant = {0x7fffff, 0x7fffff, 0x7fffff, 0x7fffff};
  const v4u vexp1 = {0x3f800000, 0x3f800000, 0x3f800000, 0x3f800000};

  Package *sum_packages = eval->packages;
  CompactPackage *model_packages = eval->models[model_idx].packages;
  unsigned *pkg_sizes = eval->package_sizes;
  const int *pkg_offsets = eval->models[model_idx].package_offsets;

  int64_t delta_size = 0;
  for (int i = 0; i < ctx->packages_per_job && pkg_base + i < ctx->num_packages;
       i++) {
    const int pkg_idx = pkg_base + i;
    const int offset = pkg_offsets[pkg_idx];
    Package *sum = &sum_packages[offset];
    const CompactPackage *model = &model_packages[pkg_idx];

    v4f prod_right = vone;
    v4f prod_total = vone;

    const v8u16 vzero16 = {};
#pragma GCC unroll 16
    for (int I = 0; I < NPV; I++) {
      v8u16 packed;
      memcpy(&packed, model->prob[I], 16);
      v8u16 right_i =
          __builtin_shufflevector(vzero16, packed, 0, 8, 1, 9, 2, 10, 3, 11);
      v8u16 total_i =
          __builtin_shufflevector(vzero16, packed, 4, 12, 5, 13, 6, 14, 7, 15);
      v4f fmr;
      memcpy(&fmr, &right_i, 16);
      v4f fmt;
      memcpy(&fmt, &total_i, 16);

      v4f sr = sum->prob[I][0];
      v4f st = sum->prob[I][1];
      sr += fmr * vdw;
      st += fmt * vdw;
      sum->prob[I][0] = sr;
      sum->prob[I][1] = st;

      prod_right *= sr;
      prod_total *= st;
    }

    v4u rb = (v4u)prod_right;
    v4u tb = (v4u)prod_total;

    v4i right_exp = (v4i)(rb >> 23);
    v4i total_exp = (v4i)(tb >> 23);

    v4f rmf = (v4f)((rb & vmant) | vexp1);
    v4f tmf = (v4f)((tb & vmant) | vexp1);

    v4f rx = rmf - vone;
    v4f tx = tmf - vone;
    v4f right_log = rx * (vc0 + rx * (vc1 + rx * vc2));
    v4f total_log = tx * (vc0 + tx * (vc1 + tx * vc2));

    v4f frac_f = (total_log - right_log) * vprec;
    v4i exp_diff = (total_exp - right_exp) << TABLE_BIT_PRECISION_BITS;
    v4u sign =
        (v4u)frac_f & (v4u){0x80000000u, 0x80000000u, 0x80000000u, 0x80000000u};
    v4f bias =
        (v4f)(sign | (v4u){0x3F000000u, 0x3F000000u, 0x3F000000u, 0x3F000000u});
    v4i frac_i = __builtin_convertvector(frac_f + bias, v4i);
    v4i sizes = exp_diff + frac_i;
    int new_pkg_size = sizes[0] + sizes[1] + sizes[2] + sizes[3];

    int old_pkg_size = pkg_sizes[offset];
    pkg_sizes[offset] = new_pkg_size;
    delta_size += new_pkg_size - old_pkg_size;
  }
  *comb_local(&ctx->accumulator) += delta_size;
}

static int64_t CSEval_ChangeWeight(CSEval *eval, const int model_idx,
                                   const int delta_weight) {
  const int num_packages = eval->models[model_idx].num_packages;
  const int pkgs_per_job = 64;
  const int num_jobs = (num_packages + pkgs_per_job - 1) / pkgs_per_job;
  ChangeWeightCtx ctx;
  ctx.eval = eval;
  ctx.model_index = model_idx;
  ctx.delta_weight = delta_weight;
  ctx.packages_per_job = pkgs_per_job;
  ctx.num_packages = num_packages;
  comb_init(&ctx.accumulator);
  parallel_for(0, num_jobs, ChangeWeight_job, &ctx);
  return comb_sum(&ctx.accumulator);
}

static int64_t CSEval_Evaluate(CSEval *eval, const ModelList4k *ml) {
  int new_weights[MAX_MODELS_N] = {};
  for (int i = 0; i < ml->nmodels; i++) {
    new_weights[ml->models[i].mask] = 1 << ml->models[i].weight;
  }
  for (int i = 0; i < MAX_MODELS_N; i++) {
    if (new_weights[i] != eval->weights[i]) {
      const int64_t delta =
          CSEval_ChangeWeight(eval, i, new_weights[i] - eval->weights[i]);
      if (eval->weights[i] == 0) {
        eval->compressed_size += 8 * TABLE_BIT_PRECISION;
      } else if (new_weights[i] == 0) {
        eval->compressed_size -= 8 * TABLE_BIT_PRECISION;
      }
      eval->weights[i] = new_weights[i];
      eval->compressed_size += delta;
    }
  }
  return eval->compressed_size;
}

typedef struct {
  int size;
  ModelPredictions models[256];
  int64_t compressed_size;
  CSEval *eval;
  float log_scale;
} CState;

typedef struct {
  unsigned char mask;
  unsigned char bitnum;
  Weights weights;
  const unsigned char *datapos;
} HashEntry;

static HashEntry *FindEntry(HashEntry *table, const unsigned table_size,
                            const unsigned char mask, const unsigned char *data,
                            const int bit_pos) {
  const unsigned char *data_ptr = &data[bit_pos / 8];
  const unsigned char bit_num = (unsigned char)(bit_pos & 7);
  for (unsigned hash = ModelHash(data, bit_pos, mask, HMUL);; hash++) {
    HashEntry *entry = &table[hash % table_size];
    if (entry->datapos == NULL) {
      entry->mask = mask;
      entry->bitnum = bit_num;
      entry->datapos = data_ptr;
      return entry;
    }
    if (entry->mask == mask && entry->bitnum == bit_num &&
        (data_ptr[0] & (0xFF00 >> bit_num)) ==
            (entry->datapos[0] & (0xFF00 >> bit_num))) {
      int match = 1;
      for (int i = 0; i < 8; i++) {
        if (((mask >> i) & 1) && data_ptr[i - 8] != entry->datapos[i - 8]) {
          match = 0;
          break;
        }
      }
      if (match) {
        return entry;
      }
    }
  }
}

static ModelPredictions CS_ApplyModel(const unsigned char *data,
                                      const int bit_length,
                                      const unsigned char mask) {
  int table_size = PreviousPrime(bit_length * 2);
  int max_packages = (bit_length + PKG_SIZE - 1) / PKG_SIZE;
  int num_packages = 0;
  CompactPackage *packages =
      (CompactPackage *)aligned_alloc32(max_packages * sizeof(CompactPackage));
  int *offsets = (int *)malloc(max_packages * sizeof(int));
  HashEntry *hash_table = (HashEntry *)calloc(table_size, sizeof(HashEntry));

  for (int pkg_idx = 0; pkg_idx < max_packages; pkg_idx++) {
    int bit_base = pkg_idx * PKG_SIZE;
    int needs_commit = 0;
    for (int bit_offset = 0; bit_offset < PKG_SIZE; bit_offset++) {
      float right_prob = 0;
      float total_prob = 0;
      if (bit_base + bit_offset < bit_length) {
        int bit = GetBit(data, bit_base + bit_offset);
        HashEntry *entry = FindEntry(hash_table, table_size, mask, data,
                                     bit_base + bit_offset);
        int boost = (entry->weights.prob[0] == 0 || entry->weights.prob[1] == 0)
                        ? 2
                        : 0;
        if (entry->weights.prob[0] || entry->weights.prob[1]) {
          needs_commit = 1;
        }
        right_prob = (float)(entry->weights.prob[bit] << boost);
        total_prob =
            (float)((entry->weights.prob[0] + entry->weights.prob[1]) << boost);
        UpdateWeights(&entry->weights, bit);
      }
      uint16_t *packed =
          (uint16_t *)&packages[num_packages].prob[bit_offset >> 2];
      packed[bit_offset & 3] = *(int *)&right_prob >> 16;
      packed[4 + (bit_offset & 3)] = *(int *)&total_prob >> 16;
    }
    offsets[num_packages] = pkg_idx;
    if (needs_commit) {
      num_packages++;
    }
  }
  free(hash_table);
  const ModelPredictions result = {num_packages, packages, offsets};
  return result;
}

typedef struct {
  const unsigned char *data;
  int bit_length;
  ModelPredictions *models;
} ApplyModelCtx;

static void ApplyModelJob(const int mask, void *vctx) {
  const ApplyModelCtx *ctx = (ApplyModelCtx *)vctx;
  ctx->models[mask] =
      CS_ApplyModel(ctx->data, ctx->bit_length, (unsigned char)mask);
}

static CState *CState_new(const unsigned char *data, const int size,
                          const int baseprob, CSEval *eval,
                          const unsigned char *context) {
  CState *cs = (CState *)calloc(1, sizeof(CState));
  cs->size = size * 8;
  cs->eval = eval;
  unsigned char *padded_data = (unsigned char *)malloc(size + MAX_CTX);
  memcpy(padded_data, context, MAX_CTX);
  memcpy(padded_data + MAX_CTX, data, size);
  assert(baseprob >= 9);
  cs->log_scale = 1.0f / 2048.0f;

  ApplyModelCtx apply_ctx = {padded_data + MAX_CTX, cs->size, cs->models};
  parallel_for(0, 256, ApplyModelJob, &apply_ctx);
  free(padded_data);

  CSEval_Init(eval, cs->models, size * 8, baseprob, cs->log_scale);
  cs->compressed_size = (int64_t)TABLE_BIT_PRECISION * size * 8;
  return cs;
}

static void CState_destroy(CState *cs) {
  for (int i = 0; i < 256; i++) {
    free(cs->models[i].packages);
    free(cs->models[i].package_offsets);
  }
  free(cs);
}

static int CState_SetModels(CState *cs, const ModelList4k *ml) {
  cs->compressed_size = CSEval_Evaluate(cs->eval, ml);
  return (int)(cs->compressed_size / (TABLE_BIT_PRECISION / BIT_PRECISION));
}

static int CState_GetCompressedSize(const CState *cs) {
  return (int)(cs->compressed_size / (TABLE_BIT_PRECISION / BIT_PRECISION));
}

static void HB_init(HashBits *hb) { memset(hb, 0, sizeof(*hb)); }

static void HB_free(const HashBits *hb) {
  free(hb->hashes);
  free(hb->bits);
  free(hb->weights);
}

static void HB_push_hash(HashBits *hb, const unsigned int value) {
  if (hb->hashes_len >= hb->hashes_cap) {
    hb->hashes_cap = hb->hashes_cap ? hb->hashes_cap * 2 : 4096;
    hb->hashes =
        (unsigned *)realloc(hb->hashes, hb->hashes_cap * sizeof(unsigned));
  }
  hb->hashes[hb->hashes_len++] = value;
}

static void HB_push_bit(HashBits *hb, const int value) {
  if (hb->bits_len >= hb->bits_cap) {
    hb->bits_cap = hb->bits_cap ? hb->bits_cap * 2 : 4096;
    hb->bits = (unsigned char *)realloc(hb->bits, hb->bits_cap);
  }
  hb->bits[hb->bits_len++] = (unsigned char)value;
}

static HashBits ComputeHashBits(const unsigned char *data, const int size,
                                unsigned char *context,
                                const ModelList4k *models, const int first,
                                const int finish) {
  const int bit_length = first + size * 8;
  const int total_length = bit_length * models->nmodels;
  HashBits out;
  HB_init(&out);
  out.num_weights = models->nmodels;
  out.weights = (int *)malloc(models->nmodels * sizeof(int));
  out.tiny_hash_size = NextPowerOf2(total_length);

  unsigned char *databuf = (unsigned char *)malloc(size + MAX_CTX);
  unsigned char *padded_data = databuf + MAX_CTX;
  memcpy(databuf, context, MAX_CTX);
  memcpy(padded_data, data, size);

  unsigned int wide_masks[MAX_N_STREAM];
  unsigned char model_masks[MAX_N_STREAM];
  const int num_models = models->nmodels;
  unsigned int weightmask = ML_GetMaskList(models, model_masks, finish);
  int weight_val = 0;
  for (int n = 0; n < num_models; n++) {
    while (weightmask & 0x80000000) {
      weightmask <<= 1;
      weight_val++;
    }
    weightmask <<= 1;
    out.weights[n] = weight_val;
    wide_masks[n] = (unsigned int)model_masks[n] | (weightmask & 0xFFFFFF00);
  }

  if (first) {
    for (int m = 0; m < num_models; m++) {
      HB_push_hash(&out, ModelHashStart(wide_masks[m], HMUL));
    }
    HB_push_bit(&out, 1);
  }

  for (int bp = 0; bp < size * 8; bp++) {
    int bit = GetBit(padded_data, bp);
    for (int m = 0; m < num_models; m++) {
      HB_push_hash(&out, ModelHash(padded_data, bp, wide_masks[m], HMUL));
    }
    HB_push_bit(&out, bit);
  }

  int copy_size = size < MAX_CTX ? size : MAX_CTX;
  if (copy_size > 0) {
    memcpy(context + MAX_CTX - copy_size, padded_data + size - copy_size,
           copy_size);
  }
  free(databuf);
  return out;
}

static void CompressFromHashBits(AritState *arit_state, const HashBits *hb,
                                 TinyHashEntry *hash_table, const int baseprob,
                                 int hashsize) {
  const int total_hashes = (int)hb->hashes_len;
  const int num_models = hb->num_weights;
  const int bit_length =
      (num_models == 0) ? hb->bits_len : total_hashes / num_models;
  hashsize /= 2;
  uint32_t hash_shift = 0;
  {
    int h = hashsize;
    while (h > (1u << hash_shift)) {
      hash_shift++;
    }
  }
  uint32_t hash_reciprocal =
      (uint32_t)(((1ull << (hash_shift + 31)) + hashsize - 1) / hashsize);
  uint32_t reciprocal_shift = hash_shift - 1u + 32u;

  unsigned int tiny_size = hb->tiny_hash_size;
  memset(hash_table, 0, tiny_size * sizeof(TinyHashEntry));
  TinyHashEntry *matched_entries[MAX_N_STREAM];

  int hash_pos = 0;
  for (int bp = 0; bp < bit_length; bp++) {
    const int bit = hb->bits[bp];
    unsigned int probs[2] = {(unsigned)baseprob, (unsigned)baseprob};
    for (int m = 0; m < num_models; m++) {
      uint32_t hash_val = hb->hashes[hash_pos++];
      const unsigned int reduced_hash =
          hash_val - (uint32_t)(((uint64_t)hash_val * hash_reciprocal) >>
                                reciprocal_shift) *
                         hashsize;
      unsigned int slot = reduced_hash & (tiny_size - 1);
      TinyHashEntry *entry = &hash_table[slot];
      while (1) {
        if (entry->used == 0) {
          entry->hash = reduced_hash;
          entry->used = 1;
          matched_entries[m] = entry;
          break;
        }
        if (entry->hash == reduced_hash) {
          matched_entries[m] = entry;
          const int weight_factor = hb->weights[m];
          unsigned int shift =
              (1 - (((entry->prob[0] + 255) & (entry->prob[1] + 255)) >> 8)) *
                  2 +
              weight_factor;
          probs[0] += (unsigned)entry->prob[0] << shift;
          probs[1] += (unsigned)entry->prob[1] << shift;
          break;
        }

        slot++;
        if (slot >= tiny_size) {
          slot = 0;
        }
        entry = &hash_table[slot];
      }
    }
    AritCode(arit_state, probs[1], probs[0], 1 - bit);
    for (int m = 0; m < num_models; m++) {
      UpdateWeights((Weights *)matched_entries[m]->prob, bit);
    }
  }
}

static int Compress4k(const unsigned char *data, const int data_size,
                      unsigned char *out_data, const int max_out,
                      const ModelList4k *ml, const int baseprob,
                      const int hashsize) {
  unsigned char context[MAX_CTX] = {};
  HashBits hb = ComputeHashBits(data, data_size, context, ml, 1, 1);
  TinyHashEntry *hash_table =
      (TinyHashEntry *)calloc(hb.tiny_hash_size, sizeof(TinyHashEntry));

  memset(out_data, 0, max_out);
  AritState arit_state;
  AritCodeInit(&arit_state, out_data);
  CompressFromHashBits(&arit_state, &hb, hash_table, baseprob, hashsize);
  const int compressed_bits = AritCodeEnd(&arit_state);

  free(hash_table);
  HB_free(&hb);
  return compressed_bits;
}

static unsigned int ApproximateWeights(CState *cs, ModelList4k *ml) {
  for (int i = 0; i < ml->nmodels; i++) {
    ml->models[i].weight = __builtin_popcount(ml->models[i].mask);
  }
  return CState_SetModels(cs, ml);
}

static unsigned int OptimizeWeights(CState *cs, ModelList4k *ml) {
  ModelList4k candidate = ML_copy(ml);
  int model_idx = ml->nmodels - 1;
  int direction = 1;
  int last_improved_idx = model_idx;
  unsigned int best_size = ApproximateWeights(cs, ml);
  if (ml->nmodels == 0) {
    return best_size;
  }

  do {
    int skip = 0;
    for (int i = 0; i < ml->nmodels; i++) {
      candidate.models[i] = ml->models[i];
      if (i == model_idx) {
        candidate.models[i].weight += direction;
        if (candidate.models[i].weight > MAX_MODEL_WT) {
          candidate.models[i].weight = MAX_MODEL_WT;
          skip = 1;
        }
        if (candidate.models[i].weight == 255) {
          candidate.models[i].weight = 0;
          skip = 1;
        }
      }
    }
    candidate.nmodels = ml->nmodels;
    unsigned int candidate_size = 0;
    if (!skip) {
      candidate_size = CState_SetModels(cs, &candidate);
    }
    if (!skip && candidate_size < best_size) {
      best_size = candidate_size;
      for (int i = 0; i < ml->nmodels; i++) {
        ml->models[i] = candidate.models[i];
      }
      last_improved_idx = model_idx;
    } else {
      if (direction == 1 && ml->models[model_idx].weight > 0) {
        direction = -1;
      } else {
        direction = 1;
        model_idx--;
        if (model_idx < 0) {
          model_idx = ml->nmodels - 1;
        }
        if (model_idx == last_improved_idx) {
          break;
        }
      }
    }
  } while (1);
  return best_size;
}

static unsigned int TryWeights(CState *cs, ModelList4k *ml,
                               const int compression_type) {
  if (compression_type == CT_SLOW || compression_type == CT_VERYSLOW) {
    return OptimizeWeights(cs, ml);
  }
  return ApproximateWeights(cs, ml);
}

typedef void ProgressCB(void *, int, int);

static ModelList4k
ApproximateModels4k(const unsigned char *data, const int size,
                    const unsigned char context[MAX_CTX],
                    const int compression_type, const int baseprob,
                    int *out_size, ProgressCB *callback, void *callback_data) {
  const int beam_width = (compression_type == CT_VERYSLOW) ? 3 : 1;
  const int EFLAG = INT_MIN;
  const int num_sets = beam_width * 2;
  ModelList4k *model_sets =
      (ModelList4k *)calloc(num_sets, sizeof(ModelList4k));
  CSEval eval;
  CSEval_init(&eval);
  CState *cs = CState_new(data, size, baseprob, &eval, context);

  unsigned char reversed_masks[256];
  for (int m = 0; m <= 255; m++) {
    int v = m;
    v = ((v & 0x0f) << 4) | ((v & 0xf0) >> 4);
    v = ((v & 0x33) << 2) | ((v & 0xcc) >> 2);
    v = ((v & 0x55) << 1) | ((v & 0xaa) >> 1);
    reversed_masks[m] = (unsigned char)v;
  }

  model_sets[0].size = CState_GetCompressedSize(cs) | EFLAG;
  for (int s = 1; s < beam_width; s++) {
    model_sets[s].size = INT_MAX;
  }

  for (int mask_idx = 0; mask_idx <= 255; mask_idx++) {
    const int mask = reversed_masks[mask_idx];

    for (int s = 0; s < beam_width; s++) {
      ModelList4k *current = &model_sets[s];
      ModelList4k *next = &model_sets[beam_width + s];
      next->size = INT_MAX;
      if (current->size == INT_MAX) {
        continue;
      }

      int already_used = 0;
      for (int m = 0; m < current->nmodels; m++) {
        if (current->models[m].mask == mask) {
          already_used = 1;
        }
      }

      if (!already_used && current->nmodels < (int)MAX_N_SEARCH) {
        *next = ML_copy(current);
        next->models[current->nmodels].mask = (unsigned char)mask;
        next->models[current->nmodels].weight = 0;
        next->nmodels = current->nmodels + 1;

        const int old_size = current->size & ~EFLAG;
        const int new_size = TryWeights(cs, next, compression_type);

        if (new_size < old_size || compression_type == CT_VERYSLOW) {
          int best_size = new_size;
          for (int m = next->nmodels - 2; m >= 0; m--) {
            const Model removed_model = next->models[m];
            next->nmodels--;
            next->models[m] = next->models[next->nmodels];
            const int trial_size = TryWeights(cs, next, compression_type);
            if (trial_size < best_size) {
              best_size = trial_size;
            } else {
              next->models[m] = removed_model;
              next->nmodels++;
            }
          }
          next->size = best_size;
          if ((current->size & EFLAG) && new_size < old_size) {
            current->size &= ~EFLAG;
            next->size |= EFLAG;
          }
        } else {
          next->size = INT_MAX;
        }
      }
    }

    for (int i = 0; i < num_sets - 1; i++) {
      for (int j = i + 1; j < num_sets; j++) {
        if (model_sets[j].size < model_sets[i].size) {
          const ModelList4k temp = model_sets[i];
          model_sets[i] = model_sets[j];
          model_sets[j] = temp;
        }
      }
    }

    if (callback) {
      callback(callback_data, mask_idx + 1, 256);
    }
  }

  assert((model_sets[0].size & EFLAG) != 0);
  model_sets[0].size &= ~EFLAG;
  for (int i = 0; i < num_sets - 1; i++) {
    for (int j = i + 1; j < num_sets; j++) {
      if (model_sets[j].size < model_sets[i].size) {
        const ModelList4k temp = model_sets[i];
        model_sets[i] = model_sets[j];
        model_sets[j] = temp;
      }
    }
  }

  ModelList4k best = ML_copy(&model_sets[0]);
  const int final_size = OptimizeWeights(cs, &best);
  if (out_size) {
    *out_size = final_size;
  }

  CState_destroy(cs);
  CSEval_destroy(&eval);
  free(model_sets);
  return best;
}

static void ProgressUpdate(void *userdata, const int current, const int total) {
  (void)userdata;
  printf("\rCalculating models... %d/%d", current, total);
  fflush(stdout);
}

static void PrintUsage(const char *program) {
  printf("Usage: %s [options] <input_file>\n\nOptions:\n", program);
  printf("  -o <file>    Output file (default: <input>.paq)\n");
  printf("  -m <mode>    Compression mode: instant, fast, slow, veryslow "
         "(default: slow)\n");
  printf("  -b <n>       Base probability (default: %d)\n", DEFAULT_BASEPROB);
  printf("  -h           Show this help\n");
}

int main(int argc, char *argv[]) {
  const char *output_file = NULL;
  int compression_type = CT_SLOW;
  int baseprob = DEFAULT_BASEPROB;

  int opt;
  while ((opt = getopt(argc, argv, "o:m:b:h")) != -1) {
    switch (opt) {
    case 'o':
      output_file = optarg;
      break;
    case 'm':
      if (!strcmp(optarg, "instant")) {
        compression_type = CT_INSTANT;
      } else if (!strcmp(optarg, "fast")) {
        compression_type = CT_FAST;
      } else if (!strcmp(optarg, "slow")) {
        compression_type = CT_SLOW;
      } else if (!strcmp(optarg, "veryslow")) {
        compression_type = CT_VERYSLOW;
      } else {
        fprintf(stderr, "Unknown mode: %s\n", optarg);
        return 1;
      }
      break;
    case 'b':
      baseprob = atoi(optarg);
      if (baseprob < 9) {
        fprintf(stderr, "Base prob must be >= 9\n");
        return 1;
      }
      break;
    case 'h':
      PrintUsage(argv[0]);
      return 0;
    default:
      PrintUsage(argv[0]);
      return 1;
    }
  }
  if (optind >= argc) {
    fprintf(stderr, "Error: no input file specified\n");
    PrintUsage(argv[0]);
    return 1;
  }

  const char *input_file = argv[optind];
  if (!output_file) {
    char default_output[512];
    snprintf(default_output, sizeof(default_output), "%s.paq", input_file);
    output_file = default_output;
  }

  FILE *file = fopen(input_file, "rb");
  if (!file) {
    fprintf(stderr, "Failed to open '%s'\n", input_file);
    return 1;
  }
  fseek(file, 0, SEEK_END);
  int data_size = (int)ftell(file);
  fseek(file, 0, SEEK_SET);
  unsigned char *data = (unsigned char *)malloc(data_size);
  fread(data, 1, data_size, file);
  fclose(file);

  par_init();
  InitLogTable();
  InitCounterStates();

  printf("Input:       %s (%d bytes)\n", input_file, data_size);
  printf("Mode:        %s\n", CTName(compression_type));
  printf("Base prob:   %d\n", baseprob);

  unsigned char context[MAX_CTX] = {};
  int estimated_size = 0;
  ModelList4k ml =
      ApproximateModels4k(data, data_size, context, compression_type, baseprob,
                          &estimated_size, ProgressUpdate, NULL);

  printf("\nEstimated:   %.3f bytes\n",
         estimated_size / (float)(BIT_PRECISION * 8));
  printf("Models:      ");
  ML_Print(&ml, stdout);

  int max_output = data_size + 1024;
  unsigned char *output_buf = (unsigned char *)malloc(max_output);
  int hashsize = 1024 * 1024 * 1024;
  int compressed_bits = Compress4k(data, data_size, output_buf, max_output, &ml,
                                   baseprob, hashsize);
  int compressed_size = (compressed_bits + 7) / 8;

  printf("Compressed:  %d bytes %d bits (%.2f%%)\n", compressed_size,
         compressed_bits, 100.0f * compressed_size / data_size);

  unsigned char sorted_masks[MAX_MODELS_N];
  unsigned int weightmask = ML_GetMaskList(&ml, sorted_masks, 1);
  int num_models = ml.nmodels;
  int hashbits = bsr(hashsize);

  unsigned char header[11];
  header[0] = data_size;
  header[1] = data_size >> 8;
  header[2] = data_size >> 16;
  header[3] = data_size >> 24;
  header[4] = weightmask;
  header[5] = weightmask >> 8;
  header[6] = weightmask >> 16;
  header[7] = weightmask >> 24;
  header[8] = num_models;
  header[9] = baseprob;
  header[10] = hashbits;

  FILE *out_file = fopen(output_file, "wb");
  if (!out_file) {
    fprintf(stderr, "Failed to open output '%s'\n", output_file);
    free(output_buf);
    free(data);
    return 1;
  }

  fwrite(header, 1, 11, out_file);
  fwrite(sorted_masks, 1, num_models, out_file);
  fwrite(output_buf, 1, compressed_size, out_file);
  fclose(out_file);

  printf("Output:      %s (%d bytes, weightmask %08X)\n", output_file,
         11 + num_models + compressed_size, weightmask);

  free(output_buf);
  free(data);
  par_shutdown();
  return 0;
}
