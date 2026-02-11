/*
 * comp.c — Single-file Crinkler-style context-mixing compressor
 * Converted from C++ multi-file source to pure C11 + pthreads + SSE2
 * Compile: gcc -O2 -msse2 -std=gnu11 -fno-strict-aliasing -o comp comp.c -lpthread -lm
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <limits.h>
#include <assert.h>
#include <getopt.h>
#include <math.h>
#include <pthread.h>
#include <unistd.h>
#include <emmintrin.h>

/* ── Helpers ─────────────────────────────────────────────────── */
static inline unsigned int bsr(unsigned int v) {
    return v ? 31 - __builtin_clz(v) : 0;
}
static inline void* aligned_alloc16(size_t sz) {
    void* p = NULL;
    posix_memalign(&p, 16, sz);
    return p;
}

/* ── Constants ───────────────────────────────────────────────── */
#define TABLE_BIT_PRECISION_BITS 12
#define TABLE_BIT_PRECISION      (1 << TABLE_BIT_PRECISION_BITS)
#define HMUL                     111
#define MAX_CTX                  8
#define DEFAULT_BASEPROB         10
#define BIT_PRECISION            256
#define MAX_MODELS_N             256
#define MAX_N_SEARCH             21
#define MAX_MODEL_WT             9
#define MAX_N_STREAM             32
#define LOG2_NPV                 4
#define NPV                      (1 << LOG2_NPV)
#define PKG_SIZE                 (NPV * 4)

enum { CT_INSTANT=0, CT_FAST, CT_SLOW, CT_VERYSLOW };

/* ── Parallel primitives (pthreads) ──────────────────────────── */
#define PAR_THRESHOLD 32

typedef struct { void (*fn)(int,void*); void* ctx; int next; int end; pthread_mutex_t mtx; } ParJob;

static int g_nthreads;
static pthread_t* g_threads;
static ParJob g_job;
static pthread_mutex_t g_start_mtx = PTHREAD_MUTEX_INITIALIZER;
static pthread_cond_t  g_start_cv  = PTHREAD_COND_INITIALIZER;
static pthread_cond_t  g_done_cv   = PTHREAD_COND_INITIALIZER;
static int g_generation = 0;
static int g_active = 0;
static int g_shutdown = 0;

static void par_execute(void) {
    for (;;) {
        int item;
        pthread_mutex_lock(&g_job.mtx);
        item = g_job.next++;
        pthread_mutex_unlock(&g_job.mtx);
        if (item >= g_job.end) break;
        g_job.fn(item, g_job.ctx);
        pthread_mutex_lock(&g_start_mtx);
        g_active--;
        if (g_active == 0) pthread_cond_signal(&g_done_cv);
        pthread_mutex_unlock(&g_start_mtx);
    }
}

static void* par_worker(void* arg) {
    (void)arg;
    int last_gen = 0;
    for (;;) {
        pthread_mutex_lock(&g_start_mtx);
        while (!g_shutdown && g_generation == last_gen)
            pthread_cond_wait(&g_start_cv, &g_start_mtx);
        if (g_shutdown) { pthread_mutex_unlock(&g_start_mtx); return NULL; }
        last_gen = g_generation;
        pthread_mutex_unlock(&g_start_mtx);
        par_execute();
    }
}

static void par_init(void) {
    g_nthreads = (int)sysconf(_SC_NPROCESSORS_ONLN);
    if (g_nthreads <= 0) g_nthreads = 4;
    if (g_nthreads > 1) g_nthreads--;
    pthread_mutex_init(&g_job.mtx, NULL);
    g_threads = (pthread_t*)malloc(g_nthreads * sizeof(pthread_t));
    for (int i = 0; i < g_nthreads; i++)
        pthread_create(&g_threads[i], NULL, par_worker, NULL);
}

static void par_shutdown(void) {
    pthread_mutex_lock(&g_start_mtx);
    g_shutdown = 1;
    pthread_cond_broadcast(&g_start_cv);
    pthread_mutex_unlock(&g_start_mtx);
    for (int i = 0; i < g_nthreads; i++)
        pthread_join(g_threads[i], NULL);
    free(g_threads);
}

static void parallel_for(int begin, int end, void (*fn)(int,void*), void* ctx) {
    int count = end - begin;
    if (count <= 0) return;
    if (count < PAR_THRESHOLD) {
        for (int i = begin; i < end; i++) fn(i, ctx);
        return;
    }
    pthread_mutex_lock(&g_job.mtx);
    g_job.fn = fn; g_job.ctx = ctx; g_job.next = begin; g_job.end = end;
    pthread_mutex_unlock(&g_job.mtx);

    pthread_mutex_lock(&g_start_mtx);
    g_active = count;
    g_generation++;
    pthread_cond_broadcast(&g_start_cv);
    pthread_mutex_unlock(&g_start_mtx);

    par_execute();

    pthread_mutex_lock(&g_start_mtx);
    while (g_active > 0) pthread_cond_wait(&g_done_cv, &g_start_mtx);
    pthread_mutex_unlock(&g_start_mtx);
}

/* Thread-local accumulator */
#define MAX_ACCUM_THREADS 128
typedef struct { int64_t vals[MAX_ACCUM_THREADS]; } Combinable;
static void comb_init(Combinable* c) { memset(c, 0, sizeof(*c)); }
static int64_t* comb_local(Combinable* c) {
    pthread_t self = pthread_self();
    /* Simple: use thread id hashed to slot */
    unsigned slot = (unsigned)(uintptr_t)self % MAX_ACCUM_THREADS;
    return &c->vals[slot];
}
static int64_t comb_sum(Combinable* c) {
    int64_t s = 0;
    for (int i = 0; i < MAX_ACCUM_THREADS; i++) s += c->vals[i];
    return s;
}

/* ── Log Table (computed at init) ──────────────────────────────── */
#define LOG_TABLE_SIZE (2 * TABLE_BIT_PRECISION)
static int LogTable[LOG_TABLE_SIZE];

static void InitLogTable(void) {
    LogTable[0] = 0;
    for (int i = 1; i < LOG_TABLE_SIZE; i++) {
        double v = log2((double)i / TABLE_BIT_PRECISION) * TABLE_BIT_PRECISION;
        LogTable[i] = (int)(v + 0.5);  /* C-style truncation rounding */
    }
}

/* ── Structs ─────────────────────────────────────────────────── */
typedef struct { void* dest_ptr; unsigned int dest_bit; unsigned int interval_size; unsigned int interval_min; } AritState;

typedef struct { unsigned short boosted_counters[2]; unsigned short next_state[2]; } CounterState;

typedef struct { unsigned char weight; unsigned char mask; } Model;

typedef struct {
    Model models[MAX_MODELS_N];
    int nmodels;
    int size;
} ModelList4k;

typedef struct { unsigned char prob[2]; } Weights;

typedef struct { __m128i prob[NPV]; } CompactPackage;
typedef struct { __m128 prob[NPV][2]; } Package;

typedef struct { int numPackages; CompactPackage* packages; int* packageOffsets; } ModelPredictions;

typedef struct {
    unsigned int hashes_cap, hashes_len;
    unsigned int* hashes;
    int bits_cap, bits_len;
    unsigned char* bits;   /* stored as 0/1 bytes */
    int* weights;
    int nweights;
    unsigned int tinyhashsize;
} HashBits;

typedef struct { unsigned int hash; unsigned char prob[2]; unsigned char used; } TinyHashEntry;

/* ── Arit coder ──────────────────────────────────────────────── */
static inline int AritSize2(int right_prob, int wrong_prob) {
    assert(right_prob > 0 && wrong_prob > 0);
    unsigned int right_len, total_len;
    int total_prob = right_prob + wrong_prob;
    if (total_prob < TABLE_BIT_PRECISION)
        return LogTable[total_prob] - LogTable[right_prob];
    right_len = bsr(right_prob);
    total_len = bsr(total_prob);
    right_len = right_len > 12 ? right_len - 12 : 0;
    total_len = total_len > 12 ? total_len - 12 : 0;
    return LogTable[total_prob >> total_len] - LogTable[right_prob >> right_len] + ((total_len - right_len) << 12);
}

static void AritCodeInit(AritState *s, void *dest) {
    s->dest_ptr = dest; s->dest_bit = (unsigned)-1;
    s->interval_size = 0x80000000; s->interval_min = 0;
}

static inline void PutBit(unsigned char* d, int db) {
    unsigned int msk, v;
    do {
        --db; if (db < 0) return;
        msk = 1u << (db & 7);
        v = d[db >> 3];
        d[db >> 3] = v ^ msk;
    } while (v & msk);
}

static void AritCode(AritState *s, unsigned int zp, unsigned int op, int bit) {
    unsigned char* d = (unsigned char*)s->dest_ptr;
    unsigned int db = s->dest_bit, imin = s->interval_min, isz = s->interval_size;
    unsigned int tp = zp + op;
    unsigned int thr = (uint64_t)isz * zp / tp;
    if (bit) {
        unsigned int old = imin;
        imin += thr;
        if (imin < old) PutBit(d, db);
        isz -= thr;
    } else {
        isz = thr;
    }
    while (isz < 0x80000000) {
        db++;
        if (imin & 0x80000000) PutBit(d, db);
        imin <<= 1; isz <<= 1;
    }
    s->dest_bit = db; s->interval_min = imin; s->interval_size = isz;
}

static int AritCodeEnd(AritState *s) {
    unsigned char* d = (unsigned char*)s->dest_ptr;
    unsigned int db = s->dest_bit, imin = s->interval_min, isz = s->interval_size;
    if (imin > 0) {
        if (imin + isz - 1 >= imin) db++;
        PutBit(d, db);
    }
    return db;
}

/* ── Counter States ──────────────────────────────────────────── */
static CounterState unsat_states[1471];
static CounterState sat_states[1470];
static int cs_map[256][256];

static int CSVisit(CounterState* st, int* ns, int ms, unsigned char c0, unsigned char c1, int nb, int sat) {
    if (nb == 0) { if (!sat || c0 < 255) c0++; if (c1 > 1) c1 >>= 1; }
    else         { if (!sat || c1 < 255) c1++; if (c0 > 1) c0 >>= 1; }
    if (cs_map[c1][c0] != -1) return cs_map[c1][c0];
    int boost = (c0 == 0 || c1 == 0) ? 2 : 0;
    int si = (*ns);
    cs_map[c1][c0] = si;
    assert(*ns < ms);
    CounterState* e = &st[(*ns)++];
    e->boosted_counters[0] = c0 << boost;
    e->boosted_counters[1] = c1 << boost;
    st[si].next_state[0] = CSVisit(st, ns, ms, c0, c1, 0, sat);
    st[si].next_state[1] = CSVisit(st, ns, ms, c0, c1, 1, sat);
    return si;
}

static void GenCounterStates(CounterState* st, int ms, int sat) {
    memset(cs_map, -1, sizeof(cs_map));
    cs_map[0][1] = 0; cs_map[1][0] = 1;
    int ns = 0;
    assert(ns < ms); CounterState* s0 = &st[ns++]; s0->boosted_counters[0] = 1<<2; s0->boosted_counters[1] = 0;
    assert(ns < ms); CounterState* s1 = &st[ns++]; s1->boosted_counters[0] = 0;    s1->boosted_counters[1] = 1<<2;
    st[0].next_state[0] = CSVisit(st, &ns, ms, 1, 0, 0, sat);
    st[0].next_state[1] = CSVisit(st, &ns, ms, 1, 0, 1, sat);
    st[1].next_state[0] = CSVisit(st, &ns, ms, 0, 1, 0, sat);
    st[1].next_state[1] = CSVisit(st, &ns, ms, 0, 1, 1, sat);
    assert(ns == ms);
}

static void InitCounterStates(void) {
    GenCounterStates(unsat_states, 1471, 0);
    GenCounterStates(sat_states, 1470, 1);
}

/* ── Model Hash ──────────────────────────────────────────────── */
static inline int GetBit(const unsigned char *d, int bp) { return (d[bp>>3] >> (7-(bp&7))) & 1; }

static unsigned int ModelHashStart(unsigned int mask, int hm) {
    unsigned char dl = (unsigned char)mask;
    unsigned int h = mask;
    h = h * hm - 1;
    while (dl) { if (dl & 0x80) h = h * hm - 1; dl += dl; }
    return h;
}

static unsigned int ModelHash(const unsigned char* d, int bp, unsigned int mask, int hm) {
    unsigned char dl = (unsigned char)mask;
    const unsigned char* p = d + (bp >> 3);
    unsigned int h = mask;
    unsigned char cb = (0x100 | *p) >> ((~bp & 7) + 1);
    h ^= cb; h *= hm; h = (h & 0xFFFFFF00) | ((h + cb) & 0xFF); h--;
    while (dl) {
        p--;
        if (dl & 0x80) { cb = *p; h ^= cb; h *= hm; h = (h & 0xFFFFFF00)|((h+cb)&0xFF); h--; }
        dl += dl;
    }
    return h;
}

/* ── ModelList4k ─────────────────────────────────────────────── */
static ModelList4k ML_copy(const ModelList4k* src) { return *src; }

static int Parity(int n) { return __builtin_parity(n & 0xFF); }

static unsigned int ML_GetMaskList(const ModelList4k* ml, unsigned char* masks, int terminate) {
    unsigned int wm = 0;
    int nm = ml->nmodels, biti = 31;
    for (int w = 0; nm; w++) {
        for (int m = 0; m < ml->nmodels; m++) {
            if (ml->models[m].weight == w) {
                if (masks) *masks++ = ml->models[m].mask;
                nm--; biti--;
            }
        }
        wm |= (1u << biti); biti--;
    }
    while (biti >= 0) { wm |= 1u << biti; biti--; }
    return wm & (unsigned)(-2 + (terminate ^ Parity(wm)));
}

static void ML_Print(const ModelList4k* ml, FILE* f) {
    for (int m = 0; m < ml->nmodels; m++)
        fprintf(f, "%s%02X:%d", m ? " " : "", ml->models[m].mask, ml->models[m].weight);
    fprintf(f, "\n");
}

/* ── Weights update ──────────────────────────────────────────── */
static void UpdateWeights(Weights* w, int bit, int saturate) {
    if (!saturate || w->prob[bit] < 255) w->prob[bit] += 1;
    if (w->prob[!bit] > 1) w->prob[!bit] >>= 1;
}

/* ── Utility ─────────────────────────────────────────────────── */
static int NextPowerOf2(int v) { v--; v|=v>>1; v|=v>>2; v|=v>>4; v|=v>>8; v|=v>>16; return v+1; }
static int PreviousPrime(int n) {
    for (;;) { n = (n-2)|1; int ok=1; for(int i=3;i*i<n;i+=2) if(n/i*i==n){ok=0;break;} if(ok) return n; }
}
static const char* CTName(int ct) {
    switch(ct) { case CT_INSTANT: return "INSTANT"; case CT_FAST: return "FAST"; case CT_SLOW: return "SLOW"; case CT_VERYSLOW: return "VERYSLOW"; }
    return "UNKNOWN";
}

/* ── CompressionStateEvaluator ───────────────────────────────── */
typedef struct {
    int         weights[256];
    ModelPredictions* models;
    int         length;
    Package*    packages;
    unsigned*   packageSizes;
    int64_t     compressedSize;
    int         baseprob;
    float       logScale;
} CSEval;

static void CSEval_init(CSEval* e) { memset(e, 0, sizeof(*e)); }

static void CSEval_destroy(CSEval* e) {
    free(e->packages); free(e->packageSizes);
}

static int CSEval_Init(CSEval* e, ModelPredictions* models, int length, int baseprob, float ls) {
    e->length = length; e->models = models; e->baseprob = baseprob; e->logScale = ls;
    int np = (length + PKG_SIZE - 1) / PKG_SIZE;
    e->packages = (Package*)aligned_alloc16(np * sizeof(Package));
    e->packageSizes = (unsigned*)malloc(np * sizeof(unsigned));
    for (int i = 0; i < np; i++) {
        for (int j = 0; j < NPV; j++) {
            e->packages[i].prob[j][0] = _mm_set1_ps(baseprob * ls);
            if (i * PKG_SIZE + j * 4 < length)
                e->packages[i].prob[j][1] = _mm_set1_ps(baseprob * 2 * ls);
            else
                e->packages[i].prob[j][1] = _mm_set1_ps(baseprob * ls);
        }
        int remain = length - i * PKG_SIZE;
        if (remain > PKG_SIZE) remain = PKG_SIZE;
        e->packageSizes[i] = remain * TABLE_BIT_PRECISION;
    }
    e->compressedSize = (int64_t)length << TABLE_BIT_PRECISION_BITS;
    return 1;
}

/* ChangeWeight context for parallel_for */
typedef struct {
    CSEval* eval; int modelIndex; int diffw;
    Combinable comb;
    int PPJOB; int numPackages;
} CWCtx;

static void CW_job(int job, void* vctx) {
    CWCtx* c = (CWCtx*)vctx;
    CSEval* e = c->eval;
    int mi = c->modelIndex;
    int pidxbase = job * c->PPJOB;

    __m128 vdw = _mm_set1_ps(c->diffw * e->logScale);
    __m128i vz = _mm_setzero_si128();
    __m128 vone = _mm_set1_ps(1.0f);

    /* poly3 log2 approx coefficients */
    __m128 vc0 = _mm_set1_ps(1.42286530448213f);
    __m128 vc1 = _mm_set1_ps(-0.58208536795165f);
    __m128 vc2 = _mm_set1_ps(0.15922006346951f);
    __m128 vbps = _mm_set1_ps((float)TABLE_BIT_PRECISION);
    __m128i vmm = _mm_set1_epi32(0x7fffff);

    Package* sp = e->packages;
    CompactPackage* mp = e->models[mi].packages;
    unsigned* ps = e->packageSizes;
    int* po = e->models[mi].packageOffsets;

    int64_t ds2 = 0;
    for (int i = 0; i < c->PPJOB && pidxbase + i < c->numPackages; i++) {
        int pi = pidxbase + i;
        int poff = po[pi];
        Package* sum = &sp[poff];
        CompactPackage* mdl = &mp[pi];

        __m128 vpr = vone, vpt = vone;

#define DO(I) { \
    __m128 sr = sum->prob[I][0], st = sum->prob[I][1]; \
    __m128i pk = mdl->prob[I]; \
    sr = _mm_add_ps(sr, _mm_mul_ps(_mm_castsi128_ps(_mm_unpacklo_epi16(vz, pk)), vdw)); \
    st = _mm_add_ps(st, _mm_mul_ps(_mm_castsi128_ps(_mm_unpackhi_epi16(vz, pk)), vdw)); \
    sum->prob[I][0] = sr; sum->prob[I][1] = st; \
    vpr = _mm_mul_ps(vpr, sr); vpt = _mm_mul_ps(vpt, st); }

        DO(0)DO(1)DO(2)DO(3)DO(4)DO(5)DO(6)DO(7)
        DO(8)DO(9)DO(10)DO(11)DO(12)DO(13)DO(14)DO(15)
#undef DO

        __m128i ire = _mm_srli_epi32(_mm_castps_si128(vpr), 23);
        __m128i ite = _mm_srli_epi32(_mm_castps_si128(vpt), 23);
        __m128 rl = _mm_castsi128_ps(_mm_or_si128(_mm_and_si128(_mm_castps_si128(vpr), vmm), _mm_castps_si128(vone)));
        __m128 tl = _mm_castsi128_ps(_mm_or_si128(_mm_and_si128(_mm_castps_si128(vpt), vmm), _mm_castps_si128(vone)));

        rl = _mm_sub_ps(rl, vone);
        rl = _mm_mul_ps(_mm_add_ps(_mm_mul_ps(_mm_add_ps(_mm_mul_ps(vc2, rl), vc1), rl), vc0), rl);
        tl = _mm_sub_ps(tl, vone);
        tl = _mm_mul_ps(_mm_add_ps(_mm_mul_ps(_mm_add_ps(_mm_mul_ps(vc2, tl), vc1), tl), vc0), tl);

        __m128i vfrac = _mm_cvtps_epi32(_mm_mul_ps(_mm_sub_ps(tl, rl), vbps));
        __m128i vns = _mm_add_epi32(_mm_slli_epi32(_mm_sub_epi32(ite, ire), TABLE_BIT_PRECISION_BITS), vfrac);
        vns = _mm_add_epi32(vns, _mm_shuffle_epi32(vns, _MM_SHUFFLE(1,0,3,2)));
        vns = _mm_add_epi32(vns, _mm_shuffle_epi32(vns, _MM_SHUFFLE(2,3,0,1)));
        int nsz = _mm_cvtsi128_si32(vns);

        int osz = ps[poff];
        ps[poff] = nsz;
        ds2 += nsz - osz;
    }
    *comb_local(&c->comb) += ds2;
}

static int64_t CSEval_ChangeWeight(CSEval* e, int mi, int dw) {
    int np = e->models[mi].numPackages;
    int PPJ = 64;
    int nj = (np + PPJ - 1) / PPJ;
    CWCtx ctx;
    ctx.eval = e; ctx.modelIndex = mi; ctx.diffw = dw;
    ctx.PPJOB = PPJ; ctx.numPackages = np;
    comb_init(&ctx.comb);
    parallel_for(0, nj, CW_job, &ctx);
    return comb_sum(&ctx.comb);
}

static int64_t CSEval_Evaluate(CSEval* e, const ModelList4k* ml) {
    int nw[MAX_MODELS_N] = {};
    for (int i = 0; i < ml->nmodels; i++)
        nw[ml->models[i].mask] = 1 << ml->models[i].weight;
    for (int i = 0; i < MAX_MODELS_N; i++) {
        if (nw[i] != e->weights[i]) {
            int64_t ds = CSEval_ChangeWeight(e, i, nw[i] - e->weights[i]);
            if (e->weights[i] == 0) e->compressedSize += 8 * TABLE_BIT_PRECISION;
            else if (nw[i] == 0) e->compressedSize -= 8 * TABLE_BIT_PRECISION;
            e->weights[i] = nw[i];
            e->compressedSize += ds;
        }
    }
    return e->compressedSize;
}

/* ── CompressionState ────────────────────────────────────────── */
typedef struct { int size; int saturate; ModelPredictions models[256]; int64_t csz; CSEval* eval; float logScale; } CState;

typedef struct { unsigned char mask; unsigned char bitnum; Weights w; const unsigned char* datapos; } HashEntry;

static HashEntry* FindEntry(HashEntry* t, unsigned hs, unsigned char mask, const unsigned char* d, int bp) {
    const unsigned char* dp = &d[bp/8];
    unsigned char bn = (unsigned char)(bp & 7);
    for (unsigned h = ModelHash(d, bp, mask, HMUL) ;; h++) {
        HashEntry* e = &t[h % hs];
        if (e->datapos == NULL) { e->mask = mask; e->bitnum = bn; e->datapos = dp; return e; }
        if (e->mask == mask && e->bitnum == bn && (dp[0] & (0xFF00 >> bn)) == (e->datapos[0] & (0xFF00 >> bn))) {
            int ok = 1;
            for (int i = 0; i < 8; i++)
                if (((mask >> i) & 1) && dp[i-8] != e->datapos[i-8]) { ok = 0; break; }
            if (ok) return e;
        }
    }
}

static ModelPredictions CS_ApplyModel(const unsigned char* data, int bl, unsigned char mask, int saturate) {
    int hs = PreviousPrime(bl * 2);
    int maxp = (bl + PKG_SIZE - 1) / PKG_SIZE;
    int np = 0;
    CompactPackage* pkgs = (CompactPackage*)aligned_alloc16(maxp * sizeof(CompactPackage));
    int* poff = (int*)malloc(maxp * sizeof(int));
    HashEntry* ht = (HashEntry*)calloc(hs, sizeof(HashEntry));

    for (int idx = 0; idx < maxp; idx++) {
        int bpb = idx * PKG_SIZE;
        int needs_commit = 0;
        for (int bpo = 0; bpo < PKG_SIZE; bpo++) {
            float pr = 0, pt = 0;
            if (bpb + bpo < bl) {
                int bit = GetBit(data, bpb + bpo);
                HashEntry* e = FindEntry(ht, hs, mask, data, bpb + bpo);
                int boost = (e->w.prob[0]==0||e->w.prob[1]==0) ? 2 : 0;
                if (e->w.prob[0] || e->w.prob[1]) needs_commit = 1;
                pr = (float)(e->w.prob[bit] << boost);
                pt = (float)((e->w.prob[0]+e->w.prob[1]) << boost);
                UpdateWeights(&e->w, bit, saturate);
            }
            uint16_t* p16 = (uint16_t*)&pkgs[np].prob[bpo >> 2];
            p16[bpo & 3]       = *(int*)&pr >> 16;
            p16[4 + (bpo & 3)] = *(int*)&pt >> 16;
        }
        poff[np] = idx;
        if (needs_commit) np++;
    }
    free(ht);
    ModelPredictions mp = { np, pkgs, poff };
    return mp;
}

typedef struct { const unsigned char* data; int bl; int saturate; ModelPredictions* models; } ApplyCtx;
static void ApplyJob(int mask, void* vc) {
    ApplyCtx* c = (ApplyCtx*)vc;
    c->models[mask] = CS_ApplyModel(c->data, c->bl, (unsigned char)mask, c->saturate);
}

static CState* CState_new(const unsigned char* data, int size, int baseprob, int saturate, CSEval* eval, const unsigned char* context) {
    CState* cs = (CState*)calloc(1, sizeof(CState));
    cs->size = size * 8; cs->saturate = saturate; cs->eval = eval;
    unsigned char* d2 = (unsigned char*)malloc(size + MAX_CTX);
    memcpy(d2, context, MAX_CTX);
    memcpy(d2 + MAX_CTX, data, size);
    assert(baseprob >= 9);
    cs->logScale = 1.0f / 2048.0f;

    ApplyCtx ac = { d2 + MAX_CTX, cs->size, saturate, cs->models };
    parallel_for(0, 256, ApplyJob, &ac);
    free(d2);

    CSEval_Init(eval, cs->models, size * 8, baseprob, cs->logScale);
    cs->csz = (int64_t)TABLE_BIT_PRECISION * size * 8;
    return cs;
}

static void CState_destroy(CState* cs) {
    for (int i = 0; i < 256; i++) { free(cs->models[i].packages); free(cs->models[i].packageOffsets); }
    free(cs);
}

static int CState_SetModels(CState* cs, const ModelList4k* ml) {
    cs->csz = CSEval_Evaluate(cs->eval, ml);
    return (int)(cs->csz / (TABLE_BIT_PRECISION / BIT_PRECISION));
}

static int CState_GetCompressedSize(CState* cs) { return (int)(cs->csz / (TABLE_BIT_PRECISION / BIT_PRECISION)); }

/* ── HashBits / ComputeHashBits ──────────────────────────────── */
static void HB_init(HashBits* h) { memset(h, 0, sizeof(*h)); }
static void HB_free(HashBits* h) { free(h->hashes); free(h->bits); free(h->weights); }

static void HB_push_hash(HashBits* h, unsigned int v) {
    if (h->hashes_len >= h->hashes_cap) {
        h->hashes_cap = h->hashes_cap ? h->hashes_cap * 2 : 4096;
        h->hashes = (unsigned*)realloc(h->hashes, h->hashes_cap * sizeof(unsigned));
    }
    h->hashes[h->hashes_len++] = v;
}
static void HB_push_bit(HashBits* h, int v) {
    if (h->bits_len >= h->bits_cap) {
        h->bits_cap = h->bits_cap ? h->bits_cap * 2 : 4096;
        h->bits = (unsigned char*)realloc(h->bits, h->bits_cap);
    }
    h->bits[h->bits_len++] = (unsigned char)v;
}

static HashBits ComputeHashBits(const unsigned char* d, int size, unsigned char* context, const ModelList4k* models, int first, int finish) {
    int bl = first + size * 8;
    int length = bl * models->nmodels;
    HashBits out; HB_init(&out);
    out.nweights = models->nmodels;
    out.weights = (int*)malloc(models->nmodels * sizeof(int));
    out.tinyhashsize = NextPowerOf2(length);

    unsigned char* databuf = (unsigned char*)malloc(size + MAX_CTX);
    unsigned char* data = databuf + MAX_CTX;
    memcpy(databuf, context, MAX_CTX);
    memcpy(data, d, size);

    unsigned int wmasks[MAX_N_STREAM];
    unsigned char masks[MAX_N_STREAM];
    int nm = models->nmodels;
    unsigned int w = ML_GetMaskList(models, masks, finish);
    int v = 0;
    for (int n = 0; n < nm; n++) {
        while (w & 0x80000000) { w <<= 1; v++; }
        w <<= 1;
        out.weights[n] = v;
        wmasks[n] = (unsigned int)masks[n] | (w & 0xFFFFFF00);
    }

    if (first) {
        for (int m = 0; m < nm; m++)
            HB_push_hash(&out, ModelHashStart(wmasks[m], HMUL));
        HB_push_bit(&out, 1);
    }

    for (int bp = 0; bp < size * 8; bp++) {
        int bit = GetBit(data, bp);
        for (int m = 0; m < nm; m++)
            HB_push_hash(&out, ModelHash(data, bp, wmasks[m], HMUL));
        HB_push_bit(&out, bit);
    }

    { int s = size < MAX_CTX ? size : MAX_CTX;
      if (s > 0) memcpy(context + MAX_CTX - s, data + size - s, s); }
    free(databuf);
    return out;
}

/* ── CompressFromHashBits ────────────────────────────────────── */
static void CompressFromHashBits(AritState* as, const HashBits* hb, TinyHashEntry* ht, int baseprob, int hashsize, int saturate) {
    int length = (int)hb->hashes_len;
    int nm = hb->nweights;
    int bl = (nm == 0) ? hb->bits_len : length / nm;
    hashsize /= 2;
    uint32_t hs = 0; { int h = hashsize; while (h > (1u << hs)) hs++; }
    uint32_t rcp = (uint32_t)(((1ull << (hs + 31)) + hashsize - 1) / hashsize);
    uint32_t rsh = hs - 1u + 32u;

    unsigned int ths = hb->tinyhashsize;
    memset(ht, 0, ths * sizeof(TinyHashEntry));
    TinyHashEntry* he_arr[MAX_N_STREAM];

    int hp = 0;
    for (int bp = 0; bp < bl; bp++) {
        int bit = hb->bits[bp];
        unsigned int probs[2] = { (unsigned)baseprob, (unsigned)baseprob };
        for (int m = 0; m < nm; m++) {
            uint32_t h = hb->hashes[hp++];
            unsigned int hash = h - (uint32_t)(((uint64_t)h * rcp) >> rsh) * hashsize;
            unsigned int th = hash & (ths - 1);
            TinyHashEntry* he = &ht[th];
            while (1) {
                if (he->used == 0) { he->hash = hash; he->used = 1; he_arr[m] = he; break; }
                else if (he->hash == hash) {
                    he_arr[m] = he;
                    int fac = hb->weights[m];
                    unsigned int shift = (1 - (((he->prob[0]+255)&(he->prob[1]+255)) >> 8)) * 2 + fac;
                    probs[0] += (unsigned)he->prob[0] << shift;
                    probs[1] += (unsigned)he->prob[1] << shift;
                    break;
                } else { th++; if (th >= ths) th = 0; he = &ht[th]; }
            }
        }
        AritCode(as, probs[1], probs[0], 1 - bit);
        for (int m = 0; m < nm; m++)
            UpdateWeights((Weights*)he_arr[m]->prob, bit, saturate);
    }
}

/* ── Compress4k ──────────────────────────────────────────────── */
static int Compress4k(const unsigned char* data, int dataSize, unsigned char* outData, int maxOut, ModelList4k* ml, int saturate, int baseprob, int hashsize) {
    unsigned char context[MAX_CTX] = {};
    HashBits hb = ComputeHashBits(data, dataSize, context, ml, 1, 1);
    TinyHashEntry* ht = (TinyHashEntry*)calloc(hb.tinyhashsize, sizeof(TinyHashEntry));

    memset(outData, 0, maxOut);
    AritState as;
    AritCodeInit(&as, outData);
    CompressFromHashBits(&as, &hb, ht, baseprob, hashsize, saturate);
    int ret = (AritCodeEnd(&as) + 7) / 8;

    free(ht);
    HB_free(&hb);
    return ret;
}

/* ── Weight optimization ─────────────────────────────────────── */
static unsigned int ApproximateWeights(CState* cs, ModelList4k* ml) {
    for (int i = 0; i < ml->nmodels; i++) {
        unsigned char w = 0;
        for (int b = 0; b < 8; b++) if (ml->models[i].mask & (1 << b)) w++;
        ml->models[i].weight = w;
    }
    return CState_SetModels(cs, ml);
}

static unsigned int OptimizeWeights(CState* cs, ModelList4k* ml) {
    ModelList4k nm = ML_copy(ml);
    int idx = ml->nmodels - 1, dir = 1, lastidx = idx;
    unsigned int best = ApproximateWeights(cs, ml);
    if (ml->nmodels == 0) return best;

    do {
        int skip = 0;
        for (int i = 0; i < ml->nmodels; i++) {
            nm.models[i] = ml->models[i];
            if (i == idx) {
                nm.models[i].weight += dir;
                if (nm.models[i].weight > MAX_MODEL_WT) { nm.models[i].weight = MAX_MODEL_WT; skip = 1; }
                if (nm.models[i].weight == 255) { nm.models[i].weight = 0; skip = 1; }
            }
        }
        nm.nmodels = ml->nmodels;
        unsigned int sz = 0;
        if (!skip) sz = CState_SetModels(cs, &nm);
        if (!skip && sz < best) {
            best = sz;
            for (int i = 0; i < ml->nmodels; i++) ml->models[i] = nm.models[i];
            lastidx = idx;
        } else {
            if (dir == 1 && ml->models[idx].weight > 0) { dir = -1; }
            else { dir = 1; idx--; if (idx < 0) idx = ml->nmodels - 1; if (idx == lastidx) break; }
        }
    } while (1);
    return best;
}

static unsigned int TryWeights(CState* cs, ModelList4k* ml, int ct) {
    if (ct == CT_SLOW || ct == CT_VERYSLOW) return OptimizeWeights(cs, ml);
    return ApproximateWeights(cs, ml);
}

/* ── ApproximateModels4k ─────────────────────────────────────── */
typedef void ProgressCB(void*, int, int);

static ModelList4k ApproximateModels4k(const unsigned char* data, int sz, const unsigned char ctx[MAX_CTX],
    int ct, int saturate, int baseprob, int* outSz, ProgressCB* cb, void* cbud)
{
    int width = (ct == CT_VERYSLOW) ? 3 : 1;
    int EFLAG = INT_MIN;

    int nsets = width * 2;
    ModelList4k* ms = (ModelList4k*)calloc(nsets, sizeof(ModelList4k));
    CSEval eval; CSEval_init(&eval);
    CState* cs = CState_new(data, sz, baseprob, saturate, &eval, ctx);

    unsigned char masks[256];
    for (int m = 0; m <= 255; m++) {
        int v = m;
        v=((v&0x0f)<<4)|((v&0xf0)>>4);
        v=((v&0x33)<<2)|((v&0xcc)>>2);
        v=((v&0x55)<<1)|((v&0xaa)>>1);
        masks[m] = (unsigned char)v;
    }

    ms[0].size = CState_GetCompressedSize(cs) | EFLAG;
    for (int s = 1; s < width; s++) ms[s].size = INT_MAX;

    for (int mi = 0; mi <= 255; mi++) {
        int mask = masks[mi];

        for (int s = 0; s < width; s++) {
            ModelList4k* cur = &ms[s];
            ModelList4k* nxt = &ms[width + s];
            nxt->size = INT_MAX;
            if (cur->size == INT_MAX) continue;

            int used = 0;
            for (int m = 0; m < cur->nmodels; m++)
                if (cur->models[m].mask == mask) used = 1;

            if (!used && cur->nmodels < (int)MAX_N_SEARCH) {
                *nxt = ML_copy(cur);
                nxt->models[cur->nmodels].mask = (unsigned char)mask;
                nxt->models[cur->nmodels].weight = 0;
                nxt->nmodels = cur->nmodels + 1;

                int old_sz = cur->size & ~EFLAG;
                int new_sz = TryWeights(cs, nxt, ct);

                if (new_sz < old_sz || ct == CT_VERYSLOW) {
                    int bsz = new_sz;
                    for (int m = nxt->nmodels - 2; m >= 0; m--) {
                        Model rmod = nxt->models[m];
                        nxt->nmodels--;
                        nxt->models[m] = nxt->models[nxt->nmodels];
                        int tsz = TryWeights(cs, nxt, ct);
                        if (tsz < bsz) { bsz = tsz; }
                        else { nxt->models[m] = rmod; nxt->nmodels++; }
                    }
                    nxt->size = bsz;
                    if ((cur->size & EFLAG) && new_sz < old_sz) {
                        cur->size &= ~EFLAG;
                        nxt->size |= EFLAG;
                    }
                } else {
                    nxt->size = INT_MAX;
                }
            }
        }

        /* Sort by size */
        for (int i = 0; i < nsets - 1; i++)
            for (int j = i + 1; j < nsets; j++)
                if (ms[j].size < ms[i].size) { ModelList4k t = ms[i]; ms[i] = ms[j]; ms[j] = t; }

        if (cb) cb(cbud, mi + 1, 256);
    }

    assert((ms[0].size & EFLAG) != 0);
    ms[0].size &= ~EFLAG;
    for (int i = 0; i < nsets - 1; i++)
        for (int j = i + 1; j < nsets; j++)
            if (ms[j].size < ms[i].size) { ModelList4k t = ms[i]; ms[i] = ms[j]; ms[j] = t; }

    ModelList4k best = ML_copy(&ms[0]);
    int sz2 = OptimizeWeights(cs, &best);
    if (outSz) *outSz = sz2;

    CState_destroy(cs);
    CSEval_destroy(&eval);
    free(ms);
    return best;
}

/* ── main ────────────────────────────────────────────────────── */
static void ProgressUpdate(void* ud, int val, int mx) {
    (void)ud;
    printf("\rCalculating models... %d/%d", val, mx);
    fflush(stdout);
}

static void PrintUsage(const char* a0) {
    printf("Usage: %s [options] <input_file>\n\nOptions:\n", a0);
    printf("  -o <file>    Output file (default: <input>.compressed)\n");
    printf("  -m <mode>    Compression mode: instant, fast, slow, veryslow (default: slow)\n");
    printf("  -b <n>       Base probability (default: %d)\n", DEFAULT_BASEPROB);
    printf("  -h           Show this help\n");
}

int main(int argc, char* argv[]) {
    const char* outFile = NULL;
    int ct = CT_SLOW;
    int saturate = 0;
    int baseprob = DEFAULT_BASEPROB;

    int opt;
    while ((opt = getopt(argc, argv, "o:m:b:h")) != -1) {
        switch (opt) {
        case 'o': outFile = optarg; break;
        case 'm':
            if (!strcmp(optarg,"instant"))       ct = CT_INSTANT;
            else if (!strcmp(optarg,"fast"))      ct = CT_FAST;
            else if (!strcmp(optarg,"slow"))      ct = CT_SLOW;
            else if (!strcmp(optarg,"veryslow"))  ct = CT_VERYSLOW;
            else { fprintf(stderr, "Unknown mode: %s\n", optarg); return 1; }
            break;
        case 'b':
            baseprob = atoi(optarg);
            if (baseprob < 9) { fprintf(stderr, "Base prob must be >= 9\n"); return 1; }
            break;
        case 'h': PrintUsage(argv[0]); return 0;
        default:  PrintUsage(argv[0]); return 1;
        }
    }
    if (optind >= argc) { fprintf(stderr, "Error: no input file specified\n"); PrintUsage(argv[0]); return 1; }

    const char* inFile = argv[optind];
    char defOut[512];
    if (!outFile) { snprintf(defOut, sizeof(defOut), "%s.compressed", inFile); outFile = defOut; }

    FILE* f = fopen(inFile, "rb");
    if (!f) { fprintf(stderr, "Failed to open '%s'\n", inFile); return 1; }
    fseek(f, 0, SEEK_END);
    int dataSize = (int)ftell(f);
    fseek(f, 0, SEEK_SET);
    unsigned char* data = (unsigned char*)malloc(dataSize);
    fread(data, 1, dataSize, f);
    fclose(f);

    par_init();
    InitLogTable();
    InitCounterStates();

    printf("Input:       %s (%d bytes)\n", inFile, dataSize);
    printf("Mode:        %s\n", CTName(ct));
    printf("Saturate:    %s\n", saturate ? "yes" : "no");
    printf("Base prob:   %d\n", baseprob);

    unsigned char context[MAX_CTX] = {};
    int estimatedSize = 0;
    ModelList4k ml = ApproximateModels4k(data, dataSize, context, ct, saturate, baseprob, &estimatedSize, ProgressUpdate, NULL);

    printf("\nEstimated:   %.3f bytes\n", estimatedSize / (float)(BIT_PRECISION * 8));
    printf("Models:      "); ML_Print(&ml, stdout);

    int maxOut = dataSize + 1024;
    unsigned char* out = (unsigned char*)malloc(maxOut);
    int hashsize = 1024 * 1024 * 1024;
    int compSz = Compress4k(data, dataSize, out, maxOut, &ml, saturate, baseprob, hashsize);

    printf("Compressed:  %d bytes (%.2f%%)\n", compSz, 100.0f * compSz / dataSize);

    unsigned char sortedMasks[MAX_MODELS_N];
    unsigned int weightmask = ML_GetMaskList(&ml, sortedMasks, 1);
    int nm = ml.nmodels;
    int hashbits = bsr(hashsize);

    unsigned char hdr[12];
    hdr[0]=dataSize; hdr[1]=dataSize>>8; hdr[2]=dataSize>>16; hdr[3]=dataSize>>24;
    hdr[4]=weightmask; hdr[5]=weightmask>>8; hdr[6]=weightmask>>16; hdr[7]=weightmask>>24;
    hdr[8]=nm; hdr[9]=baseprob; hdr[10]=hashbits; hdr[11]=saturate?1:0;

    FILE* wf = fopen(outFile, "wb");
    if (!wf) { fprintf(stderr, "Failed to open output '%s'\n", outFile); free(out); free(data); return 1; }
    fwrite(hdr, 1, 12, wf);
    fwrite(sortedMasks, 1, nm, wf);
    fwrite(out, 1, compSz, wf);
    fclose(wf);

    printf("Output:      %s (%d bytes, weightmask %08X)\n", outFile, 12 + nm + compSz, weightmask);

    free(out); free(data);
    par_shutdown();
    return 0;
}
