/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkJumper.h"
#include <string.h>

template <typename T, typename P>
static T unaligned_load(const P* p) {
    T v;
    memcpy(&v, p, sizeof(v));
    return v;
}

template <typename Dst, typename Src>
static Dst bit_cast(const Src& src) {
    static_assert(sizeof(Dst) == sizeof(Src), "");
    return unaligned_load<Dst>(&src);
}

// A couple functions for embedding constants directly into code,
// so that no .const or .literal4 section is created.
static inline int C(int x) {
#if defined(JUMPER) && defined(__x86_64__)
    // Move x-the-compile-time-constant as a literal into x-the-register.
    asm("mov %1, %0" : "=r"(x) : "i"(x));
#endif
    return x;
}
static inline float C(float f) {
    int x = C(unaligned_load<int>(&f));
    return unaligned_load<float>(&x);
}
static inline int   operator "" _i(unsigned long long int i) { return C(  (int)i); }
static inline float operator "" _f(           long double f) { return C((float)f); }

// Not all constants can be generated using C() or _i/_f.  We read the rest from this struct.
using K = const SkJumper_constants;

#if !defined(JUMPER)
    // This path should lead to portable code that can be compiled directly into Skia.
    // (All other paths are compiled offline by Clang into SkJumper_generated.h.)
    #include <math.h>

    using F   = float;
    using I32 =  int32_t;
    using U32 = uint32_t;
    using U16 = uint16_t;
    using U8  = uint8_t;

    static F   mad(F f, F m, F a)   { return f*m+a; }
    static F   min(F a, F b)        { return fminf(a,b); }
    static F   max(F a, F b)        { return fmaxf(a,b); }
    static F   abs_  (F v)          { return fabsf(v); }
    static F   floor_(F v)          { return floorf(v); }
    static F   rcp   (F v)          { return 1.0f / v; }
    static F   rsqrt (F v)          { return 1.0f / sqrtf(v); }
    static U32 round (F v, F scale) { return (uint32_t)lrintf(v*scale); }
    static U16 pack(U32 v)          { return (U16)v; }
    static U8  pack(U16 v)          { return  (U8)v; }

    static F if_then_else(I32 c, F t, F e) { return c ? t : e; }

    static F gather(const float* p, U32 ix) { return p[ix]; }

    #define WRAP(name) sk_##name

#elif defined(__aarch64__)
    #include <arm_neon.h>

    // Since we know we're using Clang, we can use its vector extensions.
    using F   = float    __attribute__((ext_vector_type(4)));
    using I32 =  int32_t __attribute__((ext_vector_type(4)));
    using U32 = uint32_t __attribute__((ext_vector_type(4)));
    using U16 = uint16_t __attribute__((ext_vector_type(4)));
    using U8  = uint8_t  __attribute__((ext_vector_type(4)));

    // We polyfill a few routines that Clang doesn't build into ext_vector_types.
    static F   mad(F f, F m, F a)                    { return vfmaq_f32(a,f,m);        }
    static F   min(F a, F b)                         { return vminq_f32(a,b);          }
    static F   max(F a, F b)                         { return vmaxq_f32(a,b);          }
    static F   abs_  (F v)                           { return vabsq_f32(v);            }
    static F   floor_(F v)                           { return vrndmq_f32(v);           }
    static F   rcp   (F v) { auto e = vrecpeq_f32 (v); return vrecpsq_f32 (v,e  ) * e; }
    static F   rsqrt (F v) { auto e = vrsqrteq_f32(v); return vrsqrtsq_f32(v,e*e) * e; }
    static U32 round (F v, F scale)                  { return vcvtnq_u32_f32(v*scale); }
    static U16 pack(U32 v)                           { return __builtin_convertvector(v, U16); }
    static U8  pack(U16 v)                           { return __builtin_convertvector(v,  U8); }

    static F if_then_else(I32 c, F t, F e) { return vbslq_f32((U32)c,t,e); }

    static F gather(const float* p, U32 ix) { return {p[ix[0]], p[ix[1]], p[ix[2]], p[ix[3]]}; }

    #define WRAP(name) sk_##name##_aarch64

#elif defined(__arm__)
    #if defined(__thumb2__) || !defined(__ARM_ARCH_7A__) || !defined(__ARM_VFPV4__)
        #error On ARMv7, compile with -march=armv7-a -mfpu=neon-vfp4, without -mthumb.
    #endif
    #include <arm_neon.h>

    // We can pass {s0-s15} as arguments under AAPCS-VFP.  We'll slice that as 8 d-registers.
    using F   = float    __attribute__((ext_vector_type(2)));
    using I32 =  int32_t __attribute__((ext_vector_type(2)));
    using U32 = uint32_t __attribute__((ext_vector_type(2)));
    using U16 = uint16_t __attribute__((ext_vector_type(2)));
    using U8  = uint8_t  __attribute__((ext_vector_type(2)));

    static F   mad(F f, F m, F a)                  { return vfma_f32(a,f,m);        }
    static F   min(F a, F b)                       { return vmin_f32(a,b);          }
    static F   max(F a, F b)                       { return vmax_f32(a,b);          }
    static F   abs_ (F v)                          { return vabs_f32(v);            }
    static F   rcp  (F v) { auto e = vrecpe_f32 (v); return vrecps_f32 (v,e  ) * e; }
    static F   rsqrt(F v) { auto e = vrsqrte_f32(v); return vrsqrts_f32(v,e*e) * e; }
    static U32 round(F v, F scale)                 { return vcvt_u32_f32(mad(v,scale,0.5f)); }
    static U16 pack(U32 v)                         { return __builtin_convertvector(v, U16); }
    static U8  pack(U16 v)                         { return __builtin_convertvector(v,  U8); }

    static F if_then_else(I32 c, F t, F e) { return vbsl_f32((U32)c,t,e); }

    static F floor_(F v) {
        F roundtrip = vcvt_f32_s32(vcvt_s32_f32(v));
        return roundtrip - if_then_else(roundtrip > v, 1.0_f, 0);
    }

    static F gather(const float* p, U32 ix) { return {p[ix[0]], p[ix[1]]}; }

    #define WRAP(name) sk_##name##_vfp4

#elif defined(__AVX__)
    #include <immintrin.h>

    // These are __m256 and __m256i, but friendlier and strongly-typed.
    using F   = float    __attribute__((ext_vector_type(8)));
    using I32 =  int32_t __attribute__((ext_vector_type(8)));
    using U32 = uint32_t __attribute__((ext_vector_type(8)));
    using U16 = uint16_t __attribute__((ext_vector_type(8)));
    using U8  = uint8_t  __attribute__((ext_vector_type(8)));

    static F mad(F f, F m, F a)  {
    #if defined(__FMA__)
        return _mm256_fmadd_ps(f,m,a);
    #else
        return f*m+a;
    #endif
    }

    static F   min(F a, F b)        { return _mm256_min_ps(a,b);    }
    static F   max(F a, F b)        { return _mm256_max_ps(a,b);    }
    static F   abs_  (F v)          { return _mm256_and_ps(v, 0-v); }
    static F   floor_(F v)          { return _mm256_floor_ps(v);    }
    static F   rcp   (F v)          { return _mm256_rcp_ps  (v);    }
    static F   rsqrt (F v)          { return _mm256_rsqrt_ps(v);    }
    static U32 round (F v, F scale) { return _mm256_cvtps_epi32(v*scale); }

    static U16 pack(U32 v) {
        return _mm_packus_epi32(_mm256_extractf128_si256(v, 0),
                                _mm256_extractf128_si256(v, 1));
    }
    static U8 pack(U16 v) {
        auto r = _mm_packus_epi16(v,v);
        return unaligned_load<U8>(&r);
    }

    static F if_then_else(I32 c, F t, F e) { return _mm256_blendv_ps(e,t,c); }

    static F gather(const float* p, U32 ix) {
    #if defined(__AVX2__)
        return _mm256_i32gather_ps(p, ix, 4);
    #else
        return { p[ix[0]], p[ix[1]], p[ix[2]], p[ix[3]],
                 p[ix[4]], p[ix[5]], p[ix[6]], p[ix[7]], };
    #endif
    }

    #if defined(__AVX2__) && defined(__F16C__) && defined(__FMA__)
        #define WRAP(name) sk_##name##_hsw
    #else
        #define WRAP(name) sk_##name##_avx
    #endif

#elif defined(__SSE2__)
    #include <immintrin.h>

    using F   = float    __attribute__((ext_vector_type(4)));
    using I32 =  int32_t __attribute__((ext_vector_type(4)));
    using U32 = uint32_t __attribute__((ext_vector_type(4)));
    using U16 = uint16_t __attribute__((ext_vector_type(4)));
    using U8  = uint8_t  __attribute__((ext_vector_type(4)));

    static F   mad(F f, F m, F a)  { return f*m+a;              }
    static F   min(F a, F b)       { return _mm_min_ps(a,b);    }
    static F   max(F a, F b)       { return _mm_max_ps(a,b);    }
    static F   abs_(F v)           { return _mm_and_ps(v, 0-v); }
    static F   rcp  (F v)          { return _mm_rcp_ps  (v);    }
    static F   rsqrt(F v)          { return _mm_rsqrt_ps(v);    }
    static U32 round(F v, F scale) { return _mm_cvtps_epi32(v*scale); }

    static U16 pack(U32 v) {
    #if defined(__SSE4_1__)
        auto p = _mm_packus_epi32(v,v);
    #else
        // Sign extend so that _mm_packs_epi32() does the pack we want.
        auto p = _mm_srai_epi32(_mm_slli_epi32(v, 16), 16);
        p = _mm_packs_epi32(p,p);
    #endif
        return unaligned_load<U16>(&p);  // We have two copies.  Return (the lower) one.
    }
    static U8 pack(U16 v) {
        __m128i r;
        memcpy(&r, &v, sizeof(v));
        r = _mm_packus_epi16(r,r);
        return unaligned_load<U8>(&r);
    }

    static F if_then_else(I32 c, F t, F e) {
        return _mm_or_ps(_mm_and_ps(c, t), _mm_andnot_ps(c, e));
    }

    static F floor_(F v) {
    #if defined(__SSE4_1__)
        return _mm_floor_ps(v);
    #else
        F roundtrip = _mm_cvtepi32_ps(_mm_cvttps_epi32(v));
        return roundtrip - if_then_else(roundtrip > v, 1.0_f, 0);
    #endif
    }

    static F gather(const float* p, U32 ix) { return {p[ix[0]], p[ix[1]], p[ix[2]], p[ix[3]]}; }

    #if defined(__SSE4_1__)
        #define WRAP(name) sk_##name##_sse41
    #else
        #define WRAP(name) sk_##name##_sse2
    #endif
#endif

static const size_t kStride = sizeof(F) / sizeof(float);

// We need to be a careful with casts.
// (F)x means cast x to float in the portable path, but bit_cast x to float in the others.
// These named casts and bit_cast() are always what they seem to be.
#if defined(JUMPER)
    static F   cast  (U32 v) { return __builtin_convertvector((I32)v, F);   }
    static U32 expand(U16 v) { return __builtin_convertvector(     v, U32); }
    static U32 expand(U8  v) { return __builtin_convertvector(     v, U32); }
#else
    static F   cast  (U32 v) { return   (F)v; }
    static U32 expand(U16 v) { return (U32)v; }
    static U32 expand(U8  v) { return (U32)v; }
#endif

template <typename V, typename T>
static inline V load(const T* src, size_t tail) {
#if defined(JUMPER)
    __builtin_assume(tail < kStride);
    if (__builtin_expect(tail, 0)) {
        V v{};  // Any inactive lanes are zeroed.
        switch (tail-1) {
            case 6: v[6] = src[6];
            case 5: v[5] = src[5];
            case 4: v[4] = src[4];
            case 3: v[3] = src[3];
            case 2: v[2] = src[2];
            case 1: v[1] = src[1];
            case 0: v[0] = src[0];
        }
        return v;
    }
#endif
    return unaligned_load<V>(src);
}

template <typename V, typename T>
static inline void store(T* dst, V v, size_t tail) {
#if defined(JUMPER)
    __builtin_assume(tail < kStride);
    if (__builtin_expect(tail, 0)) {
        switch (tail-1) {
            case 6: dst[6] = v[6];
            case 5: dst[5] = v[5];
            case 4: dst[4] = v[4];
            case 3: dst[3] = v[3];
            case 2: dst[2] = v[2];
            case 1: dst[1] = v[1];
            case 0: dst[0] = v[0];
        }
        return;
    }
#endif
    memcpy(dst, &v, sizeof(v));
}

#if 1 && defined(JUMPER) && defined(__AVX__)
    template <>
    inline U8 load(const uint8_t* src, size_t tail) {
        if (__builtin_expect(tail, 0)) {
            uint64_t v = 0;
            size_t shift = 0;
            #pragma nounroll
            while (tail --> 0) {
                v |= (uint64_t)*src++ << shift;
                shift += 8;
            }
            return unaligned_load<U8>(&v);
        }
        return unaligned_load<U8>(src);
    }
#endif

#if 1 && defined(JUMPER) && defined(__AVX2__)
    static inline U32 mask(size_t tail) {
        // It's easiest to build the mask as 8 8-bit values, either 0x00 or 0xff.
        // Start fully on, then shift away lanes from the top until we've got our mask.
        uint64_t mask = 0xffffffffffffffff >> 8*(kStride-tail);

        // Sign-extend each mask lane to its full width, 0x00000000 or 0xffffffff.
        return _mm256_cvtepi8_epi32(_mm_cvtsi64_si128((int64_t)mask));
    }

    template <>
    inline U32 load(const uint32_t* src, size_t tail) {
        __builtin_assume(tail < kStride);
        if (__builtin_expect(tail, 0)) {
            return _mm256_maskload_epi32((const int*)src, mask(tail));
        }
        return unaligned_load<U32>(src);
    }

    template <>
    inline void store(uint32_t* dst, U32 v, size_t tail) {
        __builtin_assume(tail < kStride);
        if (__builtin_expect(tail, 0)) {
            return _mm256_maskstore_epi32((int*)dst, mask(tail), v);
        }
        memcpy(dst, &v, sizeof(v));
    }
#endif


static F lerp(F from, F to, F t) {
    return mad(to-from, t, from);
}

static void from_565(U16 _565, F* r, F* g, F* b) {
    U32 wide = expand(_565);
    *r = cast(wide & C(31<<11)) * C(1.0f / (31<<11));
    *g = cast(wide & C(63<< 5)) * C(1.0f / (63<< 5));
    *b = cast(wide & C(31<< 0)) * C(1.0f / (31<< 0));
}

// Sometimes we want to work with 4 floats directly, regardless of the depth of the F vector.
#if defined(JUMPER)
    using F4 = float __attribute__((ext_vector_type(4)));
#else
    struct F4 {
        float vals[4];
        float operator[](int i) const { return vals[i]; }
    };
#endif

static void* load_and_inc(void**& program) {
#if defined(__GNUC__) && defined(__x86_64__)
    // Passing program as the second Stage argument makes it likely that it's in %rsi,
    // so this is usually a single instruction *program++.
    void* rax;
    asm("lodsq" : "=a"(rax), "+S"(program));  // Write-only %rax, read-write %rsi.
    return rax;
    // When a Stage uses its ctx pointer, this optimization typically cuts an instruction:
    //    mov    (%rsi), %rcx     // ctx  = program[0]
    //    ...
    //    mov 0x8(%rsi), %rax     // next = program[1]
    //    add $0x10, %rsi         // program += 2
    //    jmpq *%rax              // JUMP!
    // becomes
    //    lods   %ds:(%rsi),%rax  // ctx  = *program++;
    //    ...
    //    lods   %ds:(%rsi),%rax  // next = *program++;
    //    jmpq *%rax              // JUMP!
    //
    // When a Stage doesn't use its ctx pointer, it's 3 instructions either way,
    // but using lodsq (a 2-byte instruction) tends to trim a few bytes.
#else
    // On ARM *program++ compiles into a single instruction without any handholding.
    return *program++;
#endif
}

// Doesn't do anything unless you resolve it, either by casting to a pointer or calling load().
// This makes it free in stages that have no context pointer to load (i.e. built with nullptr).
struct LazyCtx {
    void*   ptr;
    void**& program;

    explicit LazyCtx(void**& p) : ptr(nullptr), program(p) {}

    template <typename T>
    operator T*() {
        if (!ptr) { ptr = load_and_inc(program); }
        return (T*)ptr;
    }

    template <typename T>
    T load() {
        if (!ptr) { ptr = load_and_inc(program); }
        return unaligned_load<T>(ptr);
    }
};

#if defined(JUMPER) && defined(__AVX__)
    // There's a big cost to switch between SSE and AVX+, so we do a little
    // extra work to handle even the jagged <kStride tail in AVX+ mode.
    using Stage = void(size_t x, void** program, K* k, size_t tail, F,F,F,F, F,F,F,F);

    #if defined(JUMPER) && defined(WIN)
    __attribute__((ms_abi))
    #endif
    extern "C" size_t WRAP(start_pipeline)(size_t x, void** program, K* k, size_t limit) {
        F v{};
        auto start = (Stage*)load_and_inc(program);
        while (x + kStride <= limit) {
            start(x,program,k,0,    v,v,v,v, v,v,v,v);
            x += kStride;
        }
        if (size_t tail = limit - x) {
            start(x,program,k,tail, v,v,v,v, v,v,v,v);
        }
        return limit;
    }

    #define STAGE(name)                                                           \
        static void name##_k(size_t x, LazyCtx ctx, K* k, size_t tail,            \
                             F& r, F& g, F& b, F& a, F& dr, F& dg, F& db, F& da); \
        extern "C" void WRAP(name)(size_t x, void** program, K* k, size_t tail,   \
                                   F r, F g, F b, F a, F dr, F dg, F db, F da) {  \
            LazyCtx ctx(program);                                                 \
            name##_k(x,ctx,k,tail, r,g,b,a, dr,dg,db,da);                         \
            auto next = (Stage*)load_and_inc(program);                            \
            next(x,program,k,tail, r,g,b,a, dr,dg,db,da);                         \
        }                                                                         \
        static void name##_k(size_t x, LazyCtx ctx, K* k, size_t tail,            \
                             F& r, F& g, F& b, F& a, F& dr, F& dg, F& db, F& da)

#else
    // Other instruction sets (SSE, NEON, portable) can fall back on narrower
    // pipelines cheaply, which frees us to always assume tail==0.

    // Stages tail call between each other by following program,
    // an interlaced sequence of Stage pointers and context pointers.
    using Stage = void(size_t x, void** program, K* k, F,F,F,F, F,F,F,F);

    #if defined(JUMPER) && defined(WIN)
    __attribute__((ms_abi))
    #endif
    extern "C" size_t WRAP(start_pipeline)(size_t x, void** program, K* k, size_t limit) {
        F v{};
        auto start = (Stage*)load_and_inc(program);
        while (x + kStride <= limit) {
            start(x,program,k, v,v,v,v, v,v,v,v);
            x += kStride;
        }
        return x;
    }

    #define STAGE(name)                                                           \
        static void name##_k(size_t x, LazyCtx ctx, K* k, size_t tail,            \
                             F& r, F& g, F& b, F& a, F& dr, F& dg, F& db, F& da); \
        extern "C" void WRAP(name)(size_t x, void** program, K* k,                \
                                   F r, F g, F b, F a, F dr, F dg, F db, F da) {  \
            LazyCtx ctx(program);                                                 \
            name##_k(x,ctx,k,0, r,g,b,a, dr,dg,db,da);                            \
            auto next = (Stage*)load_and_inc(program);                            \
            next(x,program,k, r,g,b,a, dr,dg,db,da);                              \
        }                                                                         \
        static void name##_k(size_t x, LazyCtx ctx, K* k, size_t tail,            \
                             F& r, F& g, F& b, F& a, F& dr, F& dg, F& db, F& da)
#endif

// Ends the chain of tail calls, returning back up to start_pipeline (and from there to the caller).
extern "C" void WRAP(just_return)(size_t, void**, K*, F,F,F,F, F,F,F,F) {}

// We can now define Stages!

// Some things to keep in mind while writing Stages:
//   - do not branch;                                           (i.e. avoid jmp)
//   - do not call functions that don't inline;                 (i.e. avoid call, ret)
//   - do not use constant literals other than 0, ~0 and 0.0f.  (i.e. avoid rip relative addressing)
//
// Some things that should work fine:
//   - 0, ~0, and 0.0f;
//   - arithmetic;
//   - functions of F and U32 that we've defined above;
//   - temporary values;
//   - lambdas;
//   - memcpy() with a compile-time constant size argument.

STAGE(seed_shader) {
    auto y = *(const int*)ctx;

    // It's important for speed to explicitly cast(x) and cast(y),
    // which has the effect of splatting them to vectors before converting to floats.
    // On Intel this breaks a data dependency on previous loop iterations' registers.
    r = cast(x) + 0.5_f + unaligned_load<F>(k->iota);
    g = cast(y) + 0.5_f;
    b = 1.0_f;
    a = 0;
    dr = dg = db = da = 0;
}

STAGE(constant_color) {
    auto rgba = ctx.load<F4>();
    r = rgba[0];
    g = rgba[1];
    b = rgba[2];
    a = rgba[3];
}

STAGE(clear) {
    r = g = b = a = 0;
}

STAGE(plus_) {
    r = r + dr;
    g = g + dg;
    b = b + db;
    a = a + da;
}

STAGE(srcover) {
    auto A = C(1.0f) - a;
    r = mad(dr, A, r);
    g = mad(dg, A, g);
    b = mad(db, A, b);
    a = mad(da, A, a);
}
STAGE(dstover) {
    auto DA = 1.0_f - da;
    r = mad(r, DA, dr);
    g = mad(g, DA, dg);
    b = mad(b, DA, db);
    a = mad(a, DA, da);
}

STAGE(clamp_0) {
    r = max(r, 0);
    g = max(g, 0);
    b = max(b, 0);
    a = max(a, 0);
}

STAGE(clamp_1) {
    r = min(r, 1.0_f);
    g = min(g, 1.0_f);
    b = min(b, 1.0_f);
    a = min(a, 1.0_f);
}

STAGE(clamp_a) {
    a = min(a, 1.0_f);
    r = min(r, a);
    g = min(g, a);
    b = min(b, a);
}

STAGE(set_rgb) {
    auto rgb = (const float*)ctx;
    r = rgb[0];
    g = rgb[1];
    b = rgb[2];
}
STAGE(swap_rb) {
    auto tmp = r;
    r = b;
    b = tmp;
}

STAGE(swap) {
    auto swap = [](F& v, F& dv) {
        auto tmp = v;
        v = dv;
        dv = tmp;
    };
    swap(r, dr);
    swap(g, dg);
    swap(b, db);
    swap(a, da);
}
STAGE(move_src_dst) {
    dr = r;
    dg = g;
    db = b;
    da = a;
}
STAGE(move_dst_src) {
    r = dr;
    g = dg;
    b = db;
    a = da;
}

STAGE(premul) {
    r = r * a;
    g = g * a;
    b = b * a;
}
STAGE(unpremul) {
    auto scale = if_then_else(a == 0, 0, 1.0_f / a);
    r = r * scale;
    g = g * scale;
    b = b * scale;
}

STAGE(from_srgb) {
    auto fn = [&](F s) {
        auto lo = s * C(1/12.92f);
        auto hi = mad(s*s, mad(s, 0.3000_f, 0.6975_f), 0.0025_f);
        return if_then_else(s < 0.055_f, lo, hi);
    };
    r = fn(r);
    g = fn(g);
    b = fn(b);
}
STAGE(to_srgb) {
    auto fn = [&](F l) {
        F sqrt = rcp  (rsqrt(l)),
          ftrt = rsqrt(rsqrt(l));
        auto lo = l * 12.46_f;
        auto hi = min(1.0_f, mad(0.411192_f, ftrt,
                             mad(0.689206_f, sqrt, -0.0988_f)));
        return if_then_else(l < 0.0043_f, lo, hi);
    };
    r = fn(r);
    g = fn(g);
    b = fn(b);
}

STAGE(scale_1_float) {
    auto c = *(const float*)ctx;

    r = r * c;
    g = g * c;
    b = b * c;
    a = a * c;
}
STAGE(scale_u8) {
    auto ptr = *(const uint8_t**)ctx + x;

    auto scales = load<U8>(ptr, tail);
    auto c = cast(expand(scales)) * C(1/255.0f);

    r = r * c;
    g = g * c;
    b = b * c;
    a = a * c;
}

STAGE(lerp_1_float) {
    auto c = *(const float*)ctx;

    r = lerp(dr, r, c);
    g = lerp(dg, g, c);
    b = lerp(db, b, c);
    a = lerp(da, a, c);
}
STAGE(lerp_u8) {
    auto ptr = *(const uint8_t**)ctx + x;

    auto scales = load<U8>(ptr, tail);
    auto c = cast(expand(scales)) * C(1/255.0f);

    r = lerp(dr, r, c);
    g = lerp(dg, g, c);
    b = lerp(db, b, c);
    a = lerp(da, a, c);
}
STAGE(lerp_565) {
    auto ptr = *(const uint16_t**)ctx + x;

    F cr,cg,cb;
    from_565(load<U16>(ptr, tail), &cr, &cg, &cb);

    r = lerp(dr, r, cr);
    g = lerp(dg, g, cg);
    b = lerp(db, b, cb);
    a = 1.0_f;
}

STAGE(load_tables) {
    struct Ctx {
        const uint32_t* src;
        const float *r, *g, *b;
    };
    auto c = (const Ctx*)ctx;

    auto px = load<U32>(c->src + x, tail);
    r = gather(c->r, (px      ) & 0xff_i);
    g = gather(c->g, (px >>  8) & 0xff_i);
    b = gather(c->b, (px >> 16) & 0xff_i);
    a = cast(        (px >> 24)) * C(1/255.0f);
}

STAGE(load_a8) {
    auto ptr = *(const uint8_t**)ctx + x;

    r = g = b = 0.0f;
    a = cast(expand(load<U8>(ptr, tail))) * C(1/255.0f);
}
STAGE(store_a8) {
    auto ptr = *(uint8_t**)ctx + x;

    U8 packed = pack(pack(round(a, 255.0_f)));
    store(ptr, packed, tail);
}

STAGE(load_565) {
    auto ptr = *(const uint16_t**)ctx + x;

    from_565(load<U16>(ptr, tail), &r,&g,&b);
    a = 1.0_f;
}
STAGE(store_565) {
    auto ptr = *(uint16_t**)ctx + x;

    U16 px = pack( round(r, 31.0_f) << 11
                 | round(g, 63.0_f) <<  5
                 | round(b, 31.0_f)      );
    store(ptr, px, tail);
}

STAGE(load_8888) {
    auto ptr = *(const uint32_t**)ctx + x;

    auto px = load<U32>(ptr, tail);
    r = cast((px      ) & 0xff_i) * C(1/255.0f);
    g = cast((px >>  8) & 0xff_i) * C(1/255.0f);
    b = cast((px >> 16) & 0xff_i) * C(1/255.0f);
    a = cast((px >> 24)         ) * C(1/255.0f);
}

STAGE(store_8888) {
    auto ptr = *(uint32_t**)ctx + x;

    U32 px = round(r, 255.0_f)
           | round(g, 255.0_f) <<  8
           | round(b, 255.0_f) << 16
           | round(a, 255.0_f) << 24;
    store(ptr, px, tail);
}

STAGE(load_f16) {
    auto ptr = *(const uint64_t**)ctx + x;

#if !defined(JUMPER)
    auto half_to_float = [&](int16_t h) {
        if (h < 0x0400) { h = 0; }            // Flush denorm and negative to zero.
        return bit_cast<F>(h << 13)           // Line up the mantissa,
             * bit_cast<F>(U32(0x77800000));  // then fix up the exponent.
    };
    auto rgba = (const int16_t*)ptr;
    r = half_to_float(rgba[0]);
    g = half_to_float(rgba[1]);
    b = half_to_float(rgba[2]);
    a = half_to_float(rgba[3]);
#elif defined(__aarch64__)
    auto halfs = vld4_f16((const float16_t*)ptr);
    r = vcvt_f32_f16(halfs.val[0]);
    g = vcvt_f32_f16(halfs.val[1]);
    b = vcvt_f32_f16(halfs.val[2]);
    a = vcvt_f32_f16(halfs.val[3]);
#elif defined(__arm__)
    auto rb_ga = vld2_f16((const float16_t*)ptr);
    auto rb = vcvt_f32_f16(rb_ga.val[0]),
         ga = vcvt_f32_f16(rb_ga.val[1]);
    r = {rb[0], rb[2]};
    g = {ga[0], ga[2]};
    b = {rb[1], rb[3]};
    a = {ga[1], ga[3]};
#elif defined(__AVX2__) && defined(__FMA__) && defined(__F16C__)
    __m128i _01, _23, _45, _67;
    if (__builtin_expect(tail,0)) {
        auto src = (const double*)ptr;
        _01 = _23 = _45 = _67 = _mm_setzero_si128();
        if (tail > 0) { _01 = _mm_loadl_pd(_01, src+0); }
        if (tail > 1) { _01 = _mm_loadh_pd(_01, src+1); }
        if (tail > 2) { _23 = _mm_loadl_pd(_23, src+2); }
        if (tail > 3) { _23 = _mm_loadh_pd(_23, src+3); }
        if (tail > 4) { _45 = _mm_loadl_pd(_45, src+4); }
        if (tail > 5) { _45 = _mm_loadh_pd(_45, src+5); }
        if (tail > 6) { _67 = _mm_loadl_pd(_67, src+6); }
    } else {
        _01 = _mm_loadu_si128(((__m128i*)ptr) + 0);
        _23 = _mm_loadu_si128(((__m128i*)ptr) + 1);
        _45 = _mm_loadu_si128(((__m128i*)ptr) + 2);
        _67 = _mm_loadu_si128(((__m128i*)ptr) + 3);
    }

    auto _02 = _mm_unpacklo_epi16(_01, _23),  // r0 r2 g0 g2 b0 b2 a0 a2
         _13 = _mm_unpackhi_epi16(_01, _23),  // r1 r3 g1 g3 b1 b3 a1 a3
         _46 = _mm_unpacklo_epi16(_45, _67),
         _57 = _mm_unpackhi_epi16(_45, _67);

    auto rg0123 = _mm_unpacklo_epi16(_02, _13),  // r0 r1 r2 r3 g0 g1 g2 g3
         ba0123 = _mm_unpackhi_epi16(_02, _13),  // b0 b1 b2 b3 a0 a1 a2 a3
         rg4567 = _mm_unpacklo_epi16(_46, _57),
         ba4567 = _mm_unpackhi_epi16(_46, _57);

    r = _mm256_cvtph_ps(_mm_unpacklo_epi64(rg0123, rg4567));
    g = _mm256_cvtph_ps(_mm_unpackhi_epi64(rg0123, rg4567));
    b = _mm256_cvtph_ps(_mm_unpacklo_epi64(ba0123, ba4567));
    a = _mm256_cvtph_ps(_mm_unpackhi_epi64(ba0123, ba4567));
#elif defined(__AVX__)
    __m128i _01, _23, _45, _67;
    if (__builtin_expect(tail,0)) {
        auto src = (const double*)ptr;
        _01 = _23 = _45 = _67 = _mm_setzero_si128();
        if (tail > 0) { _01 = _mm_loadl_pd(_01, src+0); }
        if (tail > 1) { _01 = _mm_loadh_pd(_01, src+1); }
        if (tail > 2) { _23 = _mm_loadl_pd(_23, src+2); }
        if (tail > 3) { _23 = _mm_loadh_pd(_23, src+3); }
        if (tail > 4) { _45 = _mm_loadl_pd(_45, src+4); }
        if (tail > 5) { _45 = _mm_loadh_pd(_45, src+5); }
        if (tail > 6) { _67 = _mm_loadl_pd(_67, src+6); }
    } else {
        _01 = _mm_loadu_si128(((__m128i*)ptr) + 0);
        _23 = _mm_loadu_si128(((__m128i*)ptr) + 1);
        _45 = _mm_loadu_si128(((__m128i*)ptr) + 2);
        _67 = _mm_loadu_si128(((__m128i*)ptr) + 3);
    }

    auto _02 = _mm_unpacklo_epi16(_01, _23),  // r0 r2 g0 g2 b0 b2 a0 a2
         _13 = _mm_unpackhi_epi16(_01, _23),  // r1 r3 g1 g3 b1 b3 a1 a3
         _46 = _mm_unpacklo_epi16(_45, _67),
         _57 = _mm_unpackhi_epi16(_45, _67);

    auto rg0123 = _mm_unpacklo_epi16(_02, _13),  // r0 r1 r2 r3 g0 g1 g2 g3
         ba0123 = _mm_unpackhi_epi16(_02, _13),  // b0 b1 b2 b3 a0 a1 a2 a3
         rg4567 = _mm_unpacklo_epi16(_46, _57),
         ba4567 = _mm_unpackhi_epi16(_46, _57);

    // half_to_float() slows down ~10x for denorm inputs, so we flush them to zero.
    // With a signed comparison this conveniently also flushes negative half floats to zero.
    auto ftz = [](__m128i v) {
        return _mm_andnot_si128(_mm_cmplt_epi16(v, _mm_set1_epi32(0x04000400_i)), v);
    };
    rg0123 = ftz(rg0123);
    ba0123 = ftz(ba0123);
    rg4567 = ftz(rg4567);
    ba4567 = ftz(ba4567);

    U32 R = _mm256_setr_m128i(_mm_unpacklo_epi16(rg0123, _mm_setzero_si128()),
                              _mm_unpacklo_epi16(rg4567, _mm_setzero_si128())),
        G = _mm256_setr_m128i(_mm_unpackhi_epi16(rg0123, _mm_setzero_si128()),
                              _mm_unpackhi_epi16(rg4567, _mm_setzero_si128())),
        B = _mm256_setr_m128i(_mm_unpacklo_epi16(ba0123, _mm_setzero_si128()),
                              _mm_unpacklo_epi16(ba4567, _mm_setzero_si128())),
        A = _mm256_setr_m128i(_mm_unpackhi_epi16(ba0123, _mm_setzero_si128()),
                              _mm_unpackhi_epi16(ba4567, _mm_setzero_si128()));

    auto half_to_float = [&](U32 h) {
        return bit_cast<F>(h << 13)             // Line up the mantissa,
             * bit_cast<F>(U32(0x77800000_i));  // then fix up the exponent.
    };

    r = half_to_float(R);
    g = half_to_float(G);
    b = half_to_float(B);
    a = half_to_float(A);

#elif defined(__SSE2__)
    auto _01 = _mm_loadu_si128(((__m128i*)ptr) + 0),
         _23 = _mm_loadu_si128(((__m128i*)ptr) + 1);

    auto _02 = _mm_unpacklo_epi16(_01, _23),  // r0 r2 g0 g2 b0 b2 a0 a2
         _13 = _mm_unpackhi_epi16(_01, _23);  // r1 r3 g1 g3 b1 b3 a1 a3

    auto rg = _mm_unpacklo_epi16(_02, _13),  // r0 r1 r2 r3 g0 g1 g2 g3
         ba = _mm_unpackhi_epi16(_02, _13);  // b0 b1 b2 b3 a0 a1 a2 a3

    // Same deal as AVX, flush denorms and negatives to zero.
    auto ftz = [](__m128i v) {
        return _mm_andnot_si128(_mm_cmplt_epi16(v, _mm_set1_epi32(0x04000400_i)), v);
    };
    rg = ftz(rg);
    ba = ftz(ba);

    auto half_to_float = [&](U32 h) {
        return bit_cast<F>(h << 13)             // Line up the mantissa,
             * bit_cast<F>(U32(0x77800000_i));  // then fix up the exponent.
    };

    r = half_to_float(_mm_unpacklo_epi16(rg, _mm_setzero_si128()));
    g = half_to_float(_mm_unpackhi_epi16(rg, _mm_setzero_si128()));
    b = half_to_float(_mm_unpacklo_epi16(ba, _mm_setzero_si128()));
    a = half_to_float(_mm_unpackhi_epi16(ba, _mm_setzero_si128()));
#endif
}

STAGE(store_f16) {
    auto ptr = *(uint64_t**)ctx + x;

#if !defined(JUMPER)
    auto float_to_half = [&](F f) {
        return bit_cast<U32>(f * bit_cast<F>(U32(0x07800000_i)))  // Fix up the exponent,
            >> 13;                                                // then line up the mantissa.
    };
    auto rgba = (int16_t*)ptr;
    rgba[0] = float_to_half(r);
    rgba[1] = float_to_half(g);
    rgba[2] = float_to_half(b);
    rgba[3] = float_to_half(a);
#elif defined(__aarch64__)
    float16x4x4_t halfs = {{
        vcvt_f16_f32(r),
        vcvt_f16_f32(g),
        vcvt_f16_f32(b),
        vcvt_f16_f32(a),
    }};
    vst4_f16((float16_t*)ptr, halfs);
#elif defined(__arm__)
    float16x4x2_t rb_ga = {{
        vcvt_f16_f32(float32x4_t{r[0], b[0], r[1], b[1]}),
        vcvt_f16_f32(float32x4_t{g[0], a[0], g[1], a[1]}),
    }};
    vst2_f16((float16_t*)ptr, rb_ga);
#elif defined(__AVX2__) && defined(__FMA__) && defined(__F16C__)
    auto R = _mm256_cvtps_ph(r, _MM_FROUND_CUR_DIRECTION),
         G = _mm256_cvtps_ph(g, _MM_FROUND_CUR_DIRECTION),
         B = _mm256_cvtps_ph(b, _MM_FROUND_CUR_DIRECTION),
         A = _mm256_cvtps_ph(a, _MM_FROUND_CUR_DIRECTION);

    auto rg0123 = _mm_unpacklo_epi16(R, G),  // r0 g0 r1 g1 r2 g2 r3 g3
         rg4567 = _mm_unpackhi_epi16(R, G),  // r4 g4 r5 g5 r6 g6 r7 g7
         ba0123 = _mm_unpacklo_epi16(B, A),
         ba4567 = _mm_unpackhi_epi16(B, A);

    auto _01 = _mm_unpacklo_epi32(rg0123, ba0123),
         _23 = _mm_unpackhi_epi32(rg0123, ba0123),
         _45 = _mm_unpacklo_epi32(rg4567, ba4567),
         _67 = _mm_unpackhi_epi32(rg4567, ba4567);

    if (__builtin_expect(tail,0)) {
        auto dst = (double*)ptr;
        if (tail > 0) { _mm_storel_pd(dst+0, _01); }
        if (tail > 1) { _mm_storeh_pd(dst+1, _01); }
        if (tail > 2) { _mm_storel_pd(dst+2, _23); }
        if (tail > 3) { _mm_storeh_pd(dst+3, _23); }
        if (tail > 4) { _mm_storel_pd(dst+4, _45); }
        if (tail > 5) { _mm_storeh_pd(dst+5, _45); }
        if (tail > 6) { _mm_storel_pd(dst+6, _67); }
    } else {
        _mm_storeu_si128((__m128i*)ptr + 0, _01);
        _mm_storeu_si128((__m128i*)ptr + 1, _23);
        _mm_storeu_si128((__m128i*)ptr + 2, _45);
        _mm_storeu_si128((__m128i*)ptr + 3, _67);
    }
#elif defined(__AVX__)
    auto float_to_half = [&](F f) {
        return bit_cast<U32>(f * bit_cast<F>(U32(0x07800000_i)))  // Fix up the exponent,
            >> 13;                                                // then line up the mantissa.
    };
    U32 R = float_to_half(r),
        G = float_to_half(g),
        B = float_to_half(b),
        A = float_to_half(a);
    auto r0123 = _mm256_extractf128_si256(R, 0),
         r4567 = _mm256_extractf128_si256(R, 1),
         g0123 = _mm256_extractf128_si256(G, 0),
         g4567 = _mm256_extractf128_si256(G, 1),
         b0123 = _mm256_extractf128_si256(B, 0),
         b4567 = _mm256_extractf128_si256(B, 1),
         a0123 = _mm256_extractf128_si256(A, 0),
         a4567 = _mm256_extractf128_si256(A, 1);
    auto rg0123 = r0123 | _mm_slli_si128(g0123,2),
         rg4567 = r4567 | _mm_slli_si128(g4567,2),
         ba0123 = b0123 | _mm_slli_si128(a0123,2),
         ba4567 = b4567 | _mm_slli_si128(a4567,2);

    auto _01 = _mm_unpacklo_epi32(rg0123, ba0123),
         _23 = _mm_unpackhi_epi32(rg0123, ba0123),
         _45 = _mm_unpacklo_epi32(rg4567, ba4567),
         _67 = _mm_unpackhi_epi32(rg4567, ba4567);

    if (__builtin_expect(tail,0)) {
        auto dst = (double*)ptr;
        if (tail > 0) { _mm_storel_pd(dst+0, _01); }
        if (tail > 1) { _mm_storeh_pd(dst+1, _01); }
        if (tail > 2) { _mm_storel_pd(dst+2, _23); }
        if (tail > 3) { _mm_storeh_pd(dst+3, _23); }
        if (tail > 4) { _mm_storel_pd(dst+4, _45); }
        if (tail > 5) { _mm_storeh_pd(dst+5, _45); }
        if (tail > 6) { _mm_storel_pd(dst+6, _67); }
    } else {
        _mm_storeu_si128((__m128i*)ptr + 0, _01);
        _mm_storeu_si128((__m128i*)ptr + 1, _23);
        _mm_storeu_si128((__m128i*)ptr + 2, _45);
        _mm_storeu_si128((__m128i*)ptr + 3, _67);
    }
#elif defined(__SSE2__)
    auto float_to_half = [&](F f) {
        return bit_cast<U32>(f * bit_cast<F>(U32(0x07800000_i)))  // Fix up the exponent,
            >> 13;                                                // then line up the mantissa.
    };
    U32 R = float_to_half(r),
        G = float_to_half(g),
        B = float_to_half(b),
        A = float_to_half(a);
    U32 rg = R | _mm_slli_si128(G,2),
        ba = B | _mm_slli_si128(A,2);
    _mm_storeu_si128((__m128i*)ptr + 0, _mm_unpacklo_epi32(rg, ba));
    _mm_storeu_si128((__m128i*)ptr + 1, _mm_unpackhi_epi32(rg, ba));
#endif
}

STAGE(store_f32) {
    auto ptr = *(float**)ctx + 4*x;

#if !defined(JUMPER)
    ptr[0] = r;
    ptr[1] = g;
    ptr[2] = b;
    ptr[3] = a;
#elif defined(__aarch64__)
    vst4q_f32(ptr, (float32x4x4_t{{r,g,b,a}}));
#elif defined(__arm__)
    vst4_f32(ptr, (float32x2x4_t{{r,g,b,a}}));
#elif defined(__AVX__)
    F rg0145 = _mm256_unpacklo_ps(r, g),  // r0 g0 r1 g1 | r4 g4 r5 g5
      rg2367 = _mm256_unpackhi_ps(r, g),  // r2 ...      | r6 ...
      ba0145 = _mm256_unpacklo_ps(b, a),  // b0 a0 b1 a1 | b4 a4 b5 a5
      ba2367 = _mm256_unpackhi_ps(b, a);  // b2 ...      | b6 ...

    F _04 = _mm256_unpacklo_pd(rg0145, ba0145),  // r0 g0 b0 a0 | r4 g4 b4 a4
      _15 = _mm256_unpackhi_pd(rg0145, ba0145),  // r1 ...      | r5 ...
      _26 = _mm256_unpacklo_pd(rg2367, ba2367),  // r2 ...      | r6 ...
      _37 = _mm256_unpackhi_pd(rg2367, ba2367);  // r3 ...      | r7 ...

    if (__builtin_expect(tail, 0)) {
        if (tail > 0) { _mm_storeu_ps(ptr+ 0, _mm256_extractf128_ps(_04, 0)); }
        if (tail > 1) { _mm_storeu_ps(ptr+ 4, _mm256_extractf128_ps(_15, 0)); }
        if (tail > 2) { _mm_storeu_ps(ptr+ 8, _mm256_extractf128_ps(_26, 0)); }
        if (tail > 3) { _mm_storeu_ps(ptr+12, _mm256_extractf128_ps(_37, 0)); }
        if (tail > 4) { _mm_storeu_ps(ptr+16, _mm256_extractf128_ps(_04, 1)); }
        if (tail > 5) { _mm_storeu_ps(ptr+20, _mm256_extractf128_ps(_15, 1)); }
        if (tail > 6) { _mm_storeu_ps(ptr+24, _mm256_extractf128_ps(_26, 1)); }
    } else {
        F _01 = _mm256_permute2f128_ps(_04, _15, 32),  // 32 == 0010 0000 == lo, lo
          _23 = _mm256_permute2f128_ps(_26, _37, 32),
          _45 = _mm256_permute2f128_ps(_04, _15, 49),  // 49 == 0011 0001 == hi, hi
          _67 = _mm256_permute2f128_ps(_26, _37, 49);
        _mm256_storeu_ps(ptr+ 0, _01);
        _mm256_storeu_ps(ptr+ 8, _23);
        _mm256_storeu_ps(ptr+16, _45);
        _mm256_storeu_ps(ptr+24, _67);
    }
#elif defined(__SSE2__)
    auto v0 = r, v1 = g, v2 = b, v3 = a;
    _MM_TRANSPOSE4_PS(v0, v1, v2, v3);
    memcpy(ptr+ 0, &v0, sizeof(v0));
    memcpy(ptr+ 4, &v1, sizeof(v1));
    memcpy(ptr+ 8, &v2, sizeof(v2));
    memcpy(ptr+12, &v3, sizeof(v3));
#endif
}

static F ulp_before(F v) {
    return bit_cast<F>(bit_cast<U32>(v) + U32(0xffffffff));
}
static F clamp(F v, float limit) {
    v = max(0, v);
    return min(v, ulp_before(limit));
}
static F repeat(F v, float limit) {
    v = v - floor_(v/limit)*limit;
    return min(v, ulp_before(limit));
}
static F mirror(F v, float limit) {
    v = abs_( (v-limit) - (limit+limit)*floor_((v-limit)/(limit+limit)) - limit );
    return min(v, ulp_before(limit));
}
STAGE(clamp_x)  { r = clamp (r, *(const float*)ctx); }
STAGE(clamp_y)  { g = clamp (g, *(const float*)ctx); }
STAGE(repeat_x) { r = repeat(r, *(const float*)ctx); }
STAGE(repeat_y) { g = repeat(g, *(const float*)ctx); }
STAGE(mirror_x) { r = mirror(r, *(const float*)ctx); }
STAGE(mirror_y) { g = mirror(g, *(const float*)ctx); }

STAGE(luminance_to_alpha) {
    a = r*0.2126_f + g*0.7152_f + b*0.0722_f;
    r = g = b = 0;
}

STAGE(matrix_2x3) {
    auto m = (const float*)ctx;

    auto R = mad(r,m[0], mad(g,m[2], m[4])),
         G = mad(r,m[1], mad(g,m[3], m[5]));
    r = R;
    g = G;
}
STAGE(matrix_3x4) {
    auto m = (const float*)ctx;

    auto R = mad(r,m[0], mad(g,m[3], mad(b,m[6], m[ 9]))),
         G = mad(r,m[1], mad(g,m[4], mad(b,m[7], m[10]))),
         B = mad(r,m[2], mad(g,m[5], mad(b,m[8], m[11])));
    r = R;
    g = G;
    b = B;
}
STAGE(matrix_4x5) {
    auto m = (const float*)ctx;

    auto R = mad(r,m[0], mad(g,m[4], mad(b,m[ 8], mad(a,m[12], m[16])))),
         G = mad(r,m[1], mad(g,m[5], mad(b,m[ 9], mad(a,m[13], m[17])))),
         B = mad(r,m[2], mad(g,m[6], mad(b,m[10], mad(a,m[14], m[18])))),
         A = mad(r,m[3], mad(g,m[7], mad(b,m[11], mad(a,m[15], m[19]))));
    r = R;
    g = G;
    b = B;
    a = A;
}
STAGE(matrix_perspective) {
    // N.B. Unlike the other matrix_ stages, this matrix is row-major.
    auto m = (const float*)ctx;

    auto R = mad(r,m[0], mad(g,m[1], m[2])),
         G = mad(r,m[3], mad(g,m[4], m[5])),
         Z = mad(r,m[6], mad(g,m[7], m[8]));
    r = R * rcp(Z);
    g = G * rcp(Z);
}

STAGE(linear_gradient_2stops) {
    struct Ctx { F4 c0, dc; };
    auto c = ctx.load<Ctx>();

    auto t = r;
    r = mad(t, c.dc[0], c.c0[0]);
    g = mad(t, c.dc[1], c.c0[1]);
    b = mad(t, c.dc[2], c.c0[2]);
    a = mad(t, c.dc[3], c.c0[3]);
}
