/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

// This file is very similar to SkSplicer_stages.cpp, and you will want to read through that file
// first before trying to understand this one.  We'll note only key differences here.

#include "SkSplicer_shared.h"
#include <string.h>

#if !defined(__clang__)
    #error This file is not like the rest of Skia.  It must be compiled with clang.
#endif

#define AI __attribute__((always_inline)) inline

#if defined(__aarch64__)
    #include <arm_neon.h>

    // In this file, F is a vector of SkFixed15.
    // See SkFixed15.h for notes on its various operations.
    struct F {
        using V = uint16_t __attribute__((ext_vector_type(8)));

        V vec;

        F(uint16x8_t v) : vec(v) {}
        operator V() const { return vec; }

        F() = default;
        F(uint16_t v) : vec(v) {}

        F operator+(F o) const { return vqaddq_u16(vec, o.vec); }
        F operator-(F o) const { return vqsubq_u16(vec, o.vec); }
        F operator*(F o) const {
            return vsraq_n_u16(vabsq_s16(vqrdmulhq_s16(vec, o.vec)),
                               vandq_s16(vec, o.vec), 15);
        }
        F operator>>(int k) const { return vec >> k; }
        F operator<<(int k) const { return vec << k; }
    };
    AI static F min(F a, F b) { return vminq_u16(a,b); }
    AI static F max(F a, F b) { return vmaxq_u16(a,b); }

#elif defined(__ARM_NEON__)
    #if defined(__thumb2__) || !defined(__ARM_ARCH_7A__) || !defined(__ARM_VFPV4__)
        #error On ARMv7, compile with -march=armv7-a -mfpu=neon-vfp4, without -mthumb.
    #endif
    #include <arm_neon.h>

    struct F {
        using V = uint16_t __attribute__((ext_vector_type(4)));

        V vec;

        F(uint16x4_t v) : vec(v) {}
        operator V() const { return vec; }

        F() = default;
        F(uint16_t v) : vec(v) {}

        F operator+(F o) const { return vqadd_u16(vec, o.vec); }
        F operator-(F o) const { return vqsub_u16(vec, o.vec); }
        F operator*(F o) const {
            return vsra_n_u16(vabs_s16(vqrdmulh_s16(vec, o.vec)),
                              vand_s16(vec, o.vec), 15);
        }
        F operator>>(int k) const { return vec >> k; }
        F operator<<(int k) const { return vec << k; }
    };
    AI static F min(F a, F b) { return vmin_u16(a,b); }
    AI static F max(F a, F b) { return vmax_u16(a,b); }

#else
    #if !defined(__AVX2__) || !defined(__FMA__) || !defined(__F16C__)
        #error On x86, compile with -mavx2 -mfma -mf16c.
    #endif
    #include <immintrin.h>

    struct F {
        using V = uint16_t __attribute__((ext_vector_type(16)));

        V vec;

        F(__m256 v) : vec(v) {}
        operator V() const { return vec; }

        F() = default;
        F(uint16_t v) : vec(v) {}

        F operator+(F o) const { return _mm256_adds_epu16(vec, o.vec); }
        F operator-(F o) const { return _mm256_subs_epu16(vec, o.vec); }
        F operator*(F o) const { return _mm256_abs_epi16(_mm256_mulhrs_epi16(vec, o.vec)); }
        F operator>>(int k) const { return vec >> k; }
        F operator<<(int k) const { return vec << k; }
    };
    AI static F min(F a, F b) { return _mm256_min_epu16(a,b); }
    AI static F max(F a, F b) { return _mm256_max_epu16(a,b); }
#endif

// No platform actually supports FMA for SkFixed15.
// This fma() method just makes it easier to port stages to lowp.
AI static F fma(F f, F m, F a) { return f*m+a; }

#if defined(__ARM_NEON__)
    #define C extern "C" __attribute__((pcs("aapcs-vfp")))
#else
    #define C extern "C"
#endif

// We use a set of constants suitable for SkFixed15 math.
using K = const SkSplicer_constants_lowp;
using Stage = void(size_t x, size_t limit, void* ctx, K* k, F,F,F,F, F,F,F,F);

C void done(size_t, size_t, void*, K*, F,F,F,F, F,F,F,F);

// The armv7 aapcs-vfp calling convention makes us pass F::V instead of F if we want them in
// registers.  This shouldn't affect performance or how you write STAGEs in any way.

#define STAGE(name)                                                              \
    AI static void name##_k(size_t x, size_t limit, void* ctx, K* k,             \
                            F& r, F& g, F& b, F& a, F& dr, F& dg, F& db, F& da); \
    C void name##_lowp(size_t x, size_t limit, void* ctx, K* k,                  \
                       F::V  R, F::V  G, F::V  B, F::V  A,                       \
                       F::V DR, F::V DG, F::V DB, F::V DA) {                     \
        F r = R, g = G, b = B, a = A, dr = DR, dg = DG, db = DB, da = DA;        \
        name##_k(x,limit,ctx,k, r,g,b,a, dr,dg,db,da);                           \
        done    (x,limit,ctx,k, r,g,b,a, dr,dg,db,da);                           \
    }                                                                            \
    AI static void name##_k(size_t x, size_t limit, void* ctx, K* k,             \
                            F& r, F& g, F& b, F& a, F& dr, F& dg, F& db, F& da)

STAGE(clear) {
    r = g = b = a = 0;
}

STAGE(plus) {
    r = r + dr;
    g = g + dg;
    b = b + db;
    a = a + da;
}
STAGE(multiply) {
    r = r * dr;
    g = g * dg;
    b = b * db;
    a = a * da;
}

STAGE(srcover) {
    auto A = F(k->_1) - a;
    r = fma(dr, A, r);
    g = fma(dg, A, g);
    b = fma(db, A, b);
    a = fma(da, A, a);
}
STAGE(dstover) { srcover_k(x,limit,ctx,k, dr,dg,db,da, r,g,b,a); }

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

STAGE(load_8888) {
    auto ptr = *(const uint32_t**)ctx + x;

#if defined(__aarch64__)
    auto to_fixed15 = [](uint8x8_t u8) {
        // u8 * (32768/255) == u8 * 128.50196... == u8*128 + u8/2 + (u8+1)>>8  ( see SkFixed15.h)
        F u16 = vmovl_u8(u8);  // TODO: tighten up by starting off with vshll_n_u8(u8, 7)?
        return (u16 << 7) + (u16 >> 1) + ((u16+1)>>8);
    };

    uint8x8x4_t rgba = vld4_u8((const uint8_t*)ptr);
    r = to_fixed15(rgba.val[0]);
    g = to_fixed15(rgba.val[1]);
    b = to_fixed15(rgba.val[2]);
    a = to_fixed15(rgba.val[3]);

#elif defined(__ARM_NEON__)
    // TODO

#else
    auto to_fixed15 = [k](__m128i u8) {
        F u16 = _mm256_cvtepu8_epi16(u8);
        return (u16 << 7) + (u16 >> 1) + ((u16+k->_0x0001)>>8);
    };

    // TODO: shorter, more confusing, faster with 256-bit loads and shuffles

    // Load 16 interplaced pixels.
    auto _0123 = _mm_loadu_si128((const __m128i*)ptr + 0),
         _4567 = _mm_loadu_si128((const __m128i*)ptr + 1),
         _89AB = _mm_loadu_si128((const __m128i*)ptr + 2),
         _CDEF = _mm_loadu_si128((const __m128i*)ptr + 3);

    // We've got an awful lot of unpacking to do to transpose this...
    auto _0415 = _mm_unpacklo_epi8(_0123, _4567),  // r04 g04 b04 a04  r15 g15 b15 a15
         _2637 = _mm_unpackhi_epi8(_0123, _4567),  // r26 g26 b26 a26  r37 g37 b37 a37
         _8C9D = _mm_unpacklo_epi8(_89AB, _CDEF),
         _AEBF = _mm_unpackhi_epi8(_89AB, _CDEF);

    auto _0246 = _mm_unpacklo_epi8(_0415, _2637),  // r0246 g0246  b0246 a0246
         _1357 = _mm_unpackhi_epi8(_0415, _2637),  // r1357 g1357  b1357 a1357
         _8ACE = _mm_unpacklo_epi8(_8C9D, _AEBF),
         _9BDF = _mm_unpackhi_epi8(_8C9D, _AEBF);

    auto rg_01234567 = _mm_unpacklo_epi8(_0246, _1357),  // r01234567 g01234567
         ba_01234567 = _mm_unpackhi_epi8(_0246, _1357),  // b01234567 a01234567
         rg_89ABCDEF = _mm_unpacklo_epi8(_8ACE, _9BDF),  // r89ABCDEF g89ABCDEF
         ba_89ABCDEF = _mm_unpackhi_epi8(_8ACE, _9BDF);  // b89ABCDEF a89ABCDEF

    r = to_fixed15(_mm_unpacklo_epi64(rg_01234567, rg_89ABCDEF));
    g = to_fixed15(_mm_unpackhi_epi64(rg_01234567, rg_89ABCDEF));
    b = to_fixed15(_mm_unpacklo_epi64(ba_01234567, ba_89ABCDEF));
    a = to_fixed15(_mm_unpackhi_epi64(ba_01234567, ba_89ABCDEF));
#endif
}

STAGE(store_8888) {
    auto ptr = *(uint32_t**)ctx + x;

#if defined(__aarch64__)
    auto from_fixed15 = [](F v) {
        // (v - (v>>8)) >> 7  (see SkFixed15.h)
        return vshrn_n_u16(v - (v>>8), 7);
    };

    uint8x8x4_t rgba = {{
        from_fixed15(r),
        from_fixed15(g),
        from_fixed15(b),
        from_fixed15(a),
    }};
    vst4_u8((uint8_t*)ptr, rgba);
#elif defined(__ARM_NEON__)
    // TODO

#else
    auto from_fixed15 = [](F v) {
        v = (v - (v>>8)) >> 7;
        return _mm_packus_epi16(_mm256_extracti128_si256(v, 0),
                                _mm256_extracti128_si256(v, 1));
    };

    auto R = from_fixed15(r),
         G = from_fixed15(g),
         B = from_fixed15(b),
         A = from_fixed15(a);

    auto rg_01234567 = _mm_unpacklo_epi8(R,G),  // rg0 rg1 rg2 ... rg7
         rg_89ABCDEF = _mm_unpackhi_epi8(R,G),  // rg8 rg9 rgA ... rgF
         ba_01234567 = _mm_unpacklo_epi8(B,A),
         ba_89ABCDEF = _mm_unpackhi_epi8(B,A);
    _mm_storeu_si128((__m128i*)ptr + 0, _mm_unpacklo_epi16(rg_01234567, ba_01234567));
    _mm_storeu_si128((__m128i*)ptr + 1, _mm_unpackhi_epi16(rg_01234567, ba_01234567));
    _mm_storeu_si128((__m128i*)ptr + 2, _mm_unpacklo_epi16(rg_89ABCDEF, ba_89ABCDEF));
    _mm_storeu_si128((__m128i*)ptr + 3, _mm_unpackhi_epi16(rg_89ABCDEF, ba_89ABCDEF));
#endif
}
