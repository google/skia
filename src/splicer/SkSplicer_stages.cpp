/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkSplicer_shared.h"
#include <string.h>

// It's tricky to relocate code referencing ordinary constants, so we read them from this struct.
using K = const SkSplicer_constants;

#if !defined(SPLICER) && !defined(JUMPER)
    // This path should lead to portable code that can be compiled directly into Skia.
    // (All other paths are compiled offline by Clang into SkSplicer_generated.h.)
    #include <math.h>

    using F   = float;
    using I32 =  int32_t;
    using U32 = uint32_t;
    using U8  = uint8_t;

    static F   fma(F f, F m, F a)  { return f*m+a; }
    static F   min(F a, F b)       { return fminf(a,b); }
    static F   max(F a, F b)       { return fmaxf(a,b); }
    static F   rcp  (F v)          { return 1.0f / v; }
    static F   rsqrt(F v)          { return 1.0f / sqrtf(v); }
    static U32 round(F v, F scale) { return (uint32_t)(v*scale); }

    static F if_then_else(I32 c, F t, F e) { return c ? t : e; }

    static F gather(const float* p, U32 ix) { return p[ix]; }

#elif defined(__aarch64__)
    #include <arm_neon.h>

    // Since we know we're using Clang, we can use its vector extensions.
    using F   = float    __attribute__((ext_vector_type(4)));
    using I32 =  int32_t __attribute__((ext_vector_type(4)));
    using U32 = uint32_t __attribute__((ext_vector_type(4)));
    using U8  = uint8_t  __attribute__((ext_vector_type(4)));

    // We polyfill a few routines that Clang doesn't build into ext_vector_types.
    static F   fma(F f, F m, F a)                   { return vfmaq_f32(a,f,m);        }
    static F   min(F a, F b)                        { return vminq_f32(a,b);          }
    static F   max(F a, F b)                        { return vmaxq_f32(a,b);          }
    static F   rcp  (F v) { auto e = vrecpeq_f32 (v); return vrecpsq_f32 (v,e  ) * e; }
    static F   rsqrt(F v) { auto e = vrsqrteq_f32(v); return vrsqrtsq_f32(v,e*e) * e; }
    static U32 round(F v, F scale)                  { return vcvtnq_u32_f32(v*scale); }

    static F if_then_else(I32 c, F t, F e) { return vbslq_f32((U32)c,t,e); }

    static F gather(const float* p, U32 ix) { return {p[ix[0]], p[ix[1]], p[ix[2]], p[ix[3]]}; }

#elif defined(__ARM_NEON__)
    #if defined(__thumb2__) || !defined(__ARM_ARCH_7A__) || !defined(__ARM_VFPV4__)
        #error On ARMv7, compile with -march=armv7-a -mfpu=neon-vfp4, without -mthumb.
    #endif
    #include <arm_neon.h>

    // We can pass {s0-s15} as arguments under AAPCS-VFP.  We'll slice that as 8 d-registers.
    using F   = float    __attribute__((ext_vector_type(2)));
    using I32 =  int32_t __attribute__((ext_vector_type(2)));
    using U32 = uint32_t __attribute__((ext_vector_type(2)));
    using U8  = uint8_t  __attribute__((ext_vector_type(2)));

    static F   fma(F f, F m, F a)                  { return vfma_f32(a,f,m);        }
    static F   min(F a, F b)                       { return vmin_f32(a,b);          }
    static F   max(F a, F b)                       { return vmax_f32(a,b);          }
    static F   rcp  (F v) { auto e = vrecpe_f32 (v); return vrecps_f32 (v,e  ) * e; }
    static F   rsqrt(F v) { auto e = vrsqrte_f32(v); return vrsqrts_f32(v,e*e) * e; }
    static U32 round(F v, F scale)                 { return vcvt_u32_f32(fma(v,scale,0.5f)); }

    static F if_then_else(I32 c, F t, F e) { return vbsl_f32((U32)c,t,e); }

    static F gather(const float* p, U32 ix) { return {p[ix[0]], p[ix[1]]}; }

#elif defined(__AVX2__) && defined(__FMA__) && defined(__F16C__)
    #include <immintrin.h>

    // These are __m256 and __m256i, but friendlier and strongly-typed.
    using F   = float    __attribute__((ext_vector_type(8)));
    using I32 =  int32_t __attribute__((ext_vector_type(8)));
    using U32 = uint32_t __attribute__((ext_vector_type(8)));
    using U8  = uint8_t  __attribute__((ext_vector_type(8)));

    static F   fma(F f, F m, F a)  { return _mm256_fmadd_ps(f,m,a);}
    static F   min(F a, F b)       { return _mm256_min_ps(a,b);    }
    static F   max(F a, F b)       { return _mm256_max_ps(a,b);    }
    static F   rcp  (F v)          { return _mm256_rcp_ps  (v);    }
    static F   rsqrt(F v)          { return _mm256_rsqrt_ps(v);    }
    static U32 round(F v, F scale) { return _mm256_cvtps_epi32(v*scale); }

    static F if_then_else(I32 c, F t, F e) { return _mm256_blendv_ps(e,t,c); }

    static F gather(const float* p, U32 ix) { return _mm256_i32gather_ps(p, ix, 4); }

#elif defined(__SSE2__)
    #include <immintrin.h>

    using F   = float    __attribute__((ext_vector_type(4)));
    using I32 =  int32_t __attribute__((ext_vector_type(4)));
    using U32 = uint32_t __attribute__((ext_vector_type(4)));
    using U8  = uint8_t  __attribute__((ext_vector_type(4)));

    static F   fma(F f, F m, F a)  { return f*m+a;           }
    static F   min(F a, F b)       { return _mm_min_ps(a,b); }
    static F   max(F a, F b)       { return _mm_max_ps(a,b); }
    static F   rcp  (F v)          { return _mm_rcp_ps  (v); }
    static F   rsqrt(F v)          { return _mm_rsqrt_ps(v); }
    static U32 round(F v, F scale) { return _mm_cvtps_epi32(v*scale); }

    static F if_then_else(I32 c, F t, F e) {
    #if defined(__SSE4_1__)
        return _mm_blendv_ps(e,t,c);
    #else
        return _mm_or_ps(_mm_and_ps(c, t), _mm_andnot_ps(c, e));
    #endif
    }

    static F gather(const float* p, U32 ix) { return {p[ix[0]], p[ix[1]], p[ix[2]], p[ix[3]]}; }
#endif

// We need to be a careful with casts.
// (F)x means cast x to float in the portable path, but bit_cast x to float in the others.
// These named casts and bit_cast() are always what they seem to be.
#if !defined(SPLICER) && !defined(JUMPER)
    static F   cast  (U32 v) { return (F)v; }
    static U32 expand(U8  v) { return (U32)v; }
#else
    static F   cast  (U32 v) { return __builtin_convertvector((I32)v, F);   }
    static U32 expand(U8  v) { return __builtin_convertvector(     v, U32); }
#endif

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

// Sometimes we want to work with 4 floats directly, regardless of the depth of the F vector.
#if !defined(SPLICER) && !defined(JUMPER)
    struct F4 {
        float vals[4];
        float operator[](int i) const { return vals[i]; }
    };
#else
    using F4 = float __attribute__((ext_vector_type(4)));
#endif

// We'll be compiling this file to an object file, then extracting parts of it into
// SkSplicer_generated.h.  It's easier to do if the function names are not C++ mangled.
#define C extern "C"

#if defined(SPLICER)
    // Splicer Stages all fit a common interface that allows SkSplicer to splice them together.
    // (This is just for reference... nothing uses this type when we're in Splicer mode.)
    using Stage = void(size_t x, size_t limit, void* ctx, K* k, F,F,F,F, F,F,F,F);

    // Stage's arguments act as the working set of registers within the final spliced function.
    // Here's a little primer on the x86-64/aarch64 ABIs:
    //   x:         rdi/x0   x and limit work to drive the loop, see loop_start in SkSplicer.cpp.
    //   limit:     rsi/x1
    //   ctx:       rdx/x2   Look for set_ctx in SkSplicer.cpp to see how this works.
    //   k:         rcx/x3
    //   vectors:   ymm0-ymm7/v0-v7

    // done() is the key to this entire splicing strategy.
    //
    // It matches the signature of Stage, so all the registers are kept live.
    // Every Stage calls done() and so will end in a single jmp (i.e. tail-call) into done(),
    // which marks the point where we can splice one Stage onto the next.
    //
    // The lovely bit is that we don't have to define done(), just declare it.
    C void done(size_t, size_t, void*, K*, F,F,F,F, F,F,F,F);

    // This should feel familiar to anyone who's read SkRasterPipeline_opts.h.
    // It's just a convenience to make a valid, spliceable Stage, nothing magic.
    #define STAGE(name)                                                           \
        static void name##_k(size_t& x, size_t limit, void* ctx, K* k,            \
                             F& r, F& g, F& b, F& a, F& dr, F& dg, F& db, F& da); \
        C void name(size_t x, size_t limit, void* ctx, K* k,                      \
                    F r, F g, F b, F a, F dr, F dg, F db, F da) {                 \
            name##_k(x,limit,ctx,k, r,g,b,a, dr,dg,db,da);                        \
            done    (x,limit,ctx,k, r,g,b,a, dr,dg,db,da);                        \
        }                                                                         \
        static void name##_k(size_t& x, size_t limit, void* ctx, K* k,            \
                             F& r, F& g, F& b, F& a, F& dr, F& dg, F& db, F& da)
#else
    // Jumper and portable Stages tail call between each other by following
    // program, an interlaced sequence of Stage pointers and context pointers.
    using Stage = void(size_t x, void** program, K* k, F,F,F,F, F,F,F,F);

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

    #define STAGE(name)                                                           \
        static void name##_k(size_t& x, void* ctx, K* k,                          \
                             F& r, F& g, F& b, F& a, F& dr, F& dg, F& db, F& da); \
        C void name(size_t x, void** program, K* k,                               \
                    F r, F g, F b, F a, F dr, F dg, F db, F da) {                 \
            auto ctx = load_and_inc(program);                                     \
            name##_k(x,ctx,k, r,g,b,a, dr,dg,db,da);                              \
            auto next = (Stage*)load_and_inc(program);                            \
            next(x,program,k, r,g,b,a, dr,dg,db,da);                              \
        }                                                                         \
        static void name##_k(size_t& x, void* ctx, K* k,                          \
                             F& r, F& g, F& b, F& a, F& dr, F& dg, F& db, F& da)
#endif

// We can now define Stages!

// Some things to keep in mind while writing Stages:
//   - do not branch;                                       (i.e. avoid jmp)
//   - do not call functions that don't inline;             (i.e. avoid call, ret)
//   - do not use constant literals other than 0 and 0.0f.  (i.e. avoid rip relative addressing)
//
// Some things that should work fine:
//   - 0, ~0, and 0.0f;
//   - arithmetic;
//   - functions of F and U32 that we've defined above;
//   - temporary values;
//   - lambdas;
//   - memcpy() with a compile-time constant size argument.

STAGE(inc_x) {
    x += sizeof(F) / sizeof(float);
}

STAGE(seed_shader) {
    auto y = *(const int*)ctx;

    // It's important for speed to explicitly cast(x) and cast(y),
    // which has the effect of splatting them to vectors before converting to floats.
    // On Intel this breaks a data dependency on previous loop iterations' registers.

    r = cast(x) + k->_0_5 + unaligned_load<F>(k->iota);
    g = cast(y) + k->_0_5;
    b = k->_1;
    a = 0;
    dr = dg = db = da = 0;
}

STAGE(constant_color) {
    auto rgba = unaligned_load<F4>(ctx);
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
    auto A = k->_1 - a;
    r = fma(dr, A, r);
    g = fma(dg, A, g);
    b = fma(db, A, b);
    a = fma(da, A, a);
}
STAGE(dstover) {
    auto DA = k->_1 - da;
    r = fma(r, DA, dr);
    g = fma(g, DA, dg);
    b = fma(b, DA, db);
    a = fma(a, DA, da);
}

STAGE(clamp_0) {
    r = max(r, 0);
    g = max(g, 0);
    b = max(b, 0);
    a = max(a, 0);
}

STAGE(clamp_1) {
    r = min(r, k->_1);
    g = min(g, k->_1);
    b = min(b, k->_1);
    a = min(a, k->_1);
}

STAGE(clamp_a) {
    a = min(a, k->_1);
    r = min(r, a);
    g = min(g, a);
    b = min(b, a);
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
    auto scale = if_then_else(a == 0, 0, k->_1 / a);
    r = r * scale;
    g = g * scale;
    b = b * scale;
}

STAGE(from_srgb) {
    auto fn = [&](F s) {
        auto lo = s * k->_1_1292;
        auto hi = fma(s*s, fma(s, k->_03000, k->_06975), k->_00025);
        return if_then_else(s < k->_0055, lo, hi);
    };
    r = fn(r);
    g = fn(g);
    b = fn(b);
}
STAGE(to_srgb) {
    auto fn = [&](F l) {
        F sqrt = rcp  (rsqrt(l)),
          ftrt = rsqrt(rsqrt(l));
        auto lo = l * k->_1246;
        auto hi = min(k->_1, fma(k->_0411192, ftrt,
                             fma(k->_0689206, sqrt,
                                 k->n_00988)));
        return if_then_else(l < k->_00043, lo, hi);
    };
    r = fn(r);
    g = fn(g);
    b = fn(b);
}

STAGE(scale_u8) {
    auto ptr = *(const uint8_t**)ctx + x;

    auto scales = unaligned_load<U8>(ptr);
    auto c = cast(expand(scales)) * k->_1_255;

    r = r * c;
    g = g * c;
    b = b * c;
    a = a * c;
}

STAGE(load_tables) {
    struct Ctx {
        const uint32_t* src;
        const float *r, *g, *b;
    };
    auto c = (const Ctx*)ctx;

    auto px = unaligned_load<U32>(c->src + x);
    r = gather(c->r, (px      ) & k->_0x000000ff);
    g = gather(c->g, (px >>  8) & k->_0x000000ff);
    b = gather(c->b, (px >> 16) & k->_0x000000ff);
    a = cast(        (px >> 24)) * k->_1_255;
}

STAGE(load_8888) {
    auto ptr = *(const uint32_t**)ctx + x;

    auto px = unaligned_load<U32>(ptr);
    r = cast((px      ) & k->_0x000000ff) * k->_1_255;
    g = cast((px >>  8) & k->_0x000000ff) * k->_1_255;
    b = cast((px >> 16) & k->_0x000000ff) * k->_1_255;
    a = cast((px >> 24)                 ) * k->_1_255;
}

STAGE(store_8888) {
    auto ptr = *(uint32_t**)ctx + x;

    U32 px = round(r, k->_255)
           | round(g, k->_255) <<  8
           | round(b, k->_255) << 16
           | round(a, k->_255) << 24;
    memcpy(ptr, &px, sizeof(px));
}

STAGE(load_f16) {
    auto ptr = *(const uint64_t**)ctx + x;

#if !defined(SPLICER) && !defined(JUMPER)
    // TODO:
#elif defined(__aarch64__)
    auto halfs = vld4_f16((const float16_t*)ptr);
    r = vcvt_f32_f16(halfs.val[0]);
    g = vcvt_f32_f16(halfs.val[1]);
    b = vcvt_f32_f16(halfs.val[2]);
    a = vcvt_f32_f16(halfs.val[3]);
#elif defined(__ARM_NEON__)
    auto rb_ga = vld2_f16((const float16_t*)ptr);
    auto rb = vcvt_f32_f16(rb_ga.val[0]),
         ga = vcvt_f32_f16(rb_ga.val[1]);
    r = {rb[0], rb[2]};
    g = {ga[0], ga[2]};
    b = {rb[1], rb[3]};
    a = {ga[1], ga[3]};
#elif defined(__AVX2__) && defined(__FMA__) && defined(__F16C__)
    auto _01 = _mm_loadu_si128(((__m128i*)ptr) + 0),
         _23 = _mm_loadu_si128(((__m128i*)ptr) + 1),
         _45 = _mm_loadu_si128(((__m128i*)ptr) + 2),
         _67 = _mm_loadu_si128(((__m128i*)ptr) + 3);

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
#elif defined(__SSE2__)
    auto _01 = _mm_loadu_si128(((__m128i*)ptr) + 0),
         _23 = _mm_loadu_si128(((__m128i*)ptr) + 1);

    auto _02 = _mm_unpacklo_epi16(_01, _23),  // r0 r2 g0 g2 b0 b2 a0 a2
         _13 = _mm_unpackhi_epi16(_01, _23);  // r1 r3 g1 g3 b1 b3 a1 a3

    auto rg = _mm_unpacklo_epi16(_02, _13),  // r0 r1 r2 r3 g0 g1 g2 g3
         ba = _mm_unpackhi_epi16(_02, _13);  // b0 b1 b2 b3 a0 a1 a2 a3

    auto half_to_float = [&](U32 h) {
        return bit_cast<F>(h << 13)               // Line up the mantissa,
             * bit_cast<F>(U32(k->_0x77800000));  // then fix up the exponent.
    };

    r = half_to_float(_mm_unpacklo_epi16(rg, _mm_setzero_si128()));
    g = half_to_float(_mm_unpackhi_epi16(rg, _mm_setzero_si128()));
    b = half_to_float(_mm_unpacklo_epi16(ba, _mm_setzero_si128()));
    a = half_to_float(_mm_unpackhi_epi16(ba, _mm_setzero_si128()));
#endif
}

STAGE(store_f16) {
    auto ptr = *(uint64_t**)ctx + x;

#if !defined(SPLICER) && !defined(JUMPER)
    // TODO:
#elif defined(__aarch64__)
    float16x4x4_t halfs = {{
        vcvt_f16_f32(r),
        vcvt_f16_f32(g),
        vcvt_f16_f32(b),
        vcvt_f16_f32(a),
    }};
    vst4_f16((float16_t*)ptr, halfs);
#elif defined(__ARM_NEON__)
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

    _mm_storeu_si128((__m128i*)ptr + 0, _mm_unpacklo_epi32(rg0123, ba0123));
    _mm_storeu_si128((__m128i*)ptr + 1, _mm_unpackhi_epi32(rg0123, ba0123));
    _mm_storeu_si128((__m128i*)ptr + 2, _mm_unpacklo_epi32(rg4567, ba4567));
    _mm_storeu_si128((__m128i*)ptr + 3, _mm_unpackhi_epi32(rg4567, ba4567));
#elif defined(__SSE2__)
    auto float_to_half = [&](F f) {
        return bit_cast<U32>(f * bit_cast<F>(U32(k->_0x07800000)))  // Fix up the exponent,
            >> 13;                                                  // then line up the mantissa.
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

static F clamp(const F& v, float limit) {
    F l = bit_cast<F>(bit_cast<U32>(F(limit)) + U32(0xffffffff));  // limit - 1 ulp
    return max(0, min(v, l));
}
STAGE(clamp_x) { r = clamp(r, *(const float*)ctx); }
STAGE(clamp_y) { g = clamp(g, *(const float*)ctx); }

STAGE(matrix_2x3) {
    auto m = (const float*)ctx;

    auto R = fma(r,m[0], fma(g,m[2], m[4])),
         G = fma(r,m[1], fma(g,m[3], m[5]));
    r = R;
    g = G;
}
STAGE(matrix_3x4) {
    auto m = (const float*)ctx;

    auto R = fma(r,m[0], fma(g,m[3], fma(b,m[6], m[ 9]))),
         G = fma(r,m[1], fma(g,m[4], fma(b,m[7], m[10]))),
         B = fma(r,m[2], fma(g,m[5], fma(b,m[8], m[11])));
    r = R;
    g = G;
    b = B;
}

STAGE(linear_gradient_2stops) {
    struct Ctx { F4 c0, dc; };
    auto c = unaligned_load<Ctx>(ctx);

    auto t = r;
    r = fma(t, c.dc[0], c.c0[0]);
    g = fma(t, c.dc[1], c.c0[1]);
    b = fma(t, c.dc[2], c.c0[2]);
    a = fma(t, c.dc[3], c.c0[3]);
}
