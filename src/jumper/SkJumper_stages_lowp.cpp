/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

// This restricted SkJumper backend works on 8-bit per channel pixels stored in
// 16-bit channels.  This is a last attempt to write a performant low-precision
// backend with stage definitions that can be shared by x86 and ARM.

#include "SkJumper.h"
#include "SkJumper_misc.h"

#if defined(__clang__)  // This file is empty when not compiled by Clang.

#if defined(__ARM_NEON)
    #include <arm_neon.h>
#elif defined(__SSE2__)
    #include <immintrin.h>
#else
    #include <math.h>
#endif

#if !defined(JUMPER_IS_OFFLINE)
    #define WRAP(name) sk_##name##_lowp
#elif defined(__AVX2__)
    #define WRAP(name) sk_##name##_hsw_lowp
#elif defined(__SSE4_1__)
    #define WRAP(name) sk_##name##_sse41_lowp
#elif defined(__SSE2__)
    #define WRAP(name) sk_##name##_sse2_lowp
#endif

#if defined(__AVX2__)
    using U8  = uint8_t  __attribute__((ext_vector_type(16)));
    using U16 = uint16_t __attribute__((ext_vector_type(16)));
    using I16 =  int16_t __attribute__((ext_vector_type(16)));
    using I32 =  int32_t __attribute__((ext_vector_type(16)));
    using U32 = uint32_t __attribute__((ext_vector_type(16)));
    using F   = float    __attribute__((ext_vector_type(16)));
#else
    using U8  = uint8_t  __attribute__((ext_vector_type(8)));
    using U16 = uint16_t __attribute__((ext_vector_type(8)));
    using I16 =  int16_t __attribute__((ext_vector_type(8)));
    using I32 =  int32_t __attribute__((ext_vector_type(8)));
    using U32 = uint32_t __attribute__((ext_vector_type(8)));
    using F   = float    __attribute__((ext_vector_type(8)));
#endif

static const size_t N = sizeof(U16) / sizeof(uint16_t);

// We pass program as the second argument so that load_and_inc() will find it in %rsi on x86-64.
using Stage = void (ABI*)(size_t tail, void** program, size_t dx, size_t dy,
                          U16  r, U16  g, U16  b, U16  a,
                          U16 dr, U16 dg, U16 db, U16 da);

extern "C" MAYBE_MSABI void WRAP(start_pipeline)(const size_t x0,
                                                 const size_t y0,
                                                 const size_t xlimit,
                                                 const size_t ylimit,
                                                 void** program) {
    auto start = (Stage)load_and_inc(program);
    for (size_t dy = y0; dy < ylimit; dy++) {
        size_t dx = x0;
        for (; dx + N <= xlimit; dx += N) {
            start(   0,program,dx,dy, 0,0,0,0, 0,0,0,0);
        }
        if (size_t tail = xlimit - dx) {
            start(tail,program,dx,dy, 0,0,0,0, 0,0,0,0);
        }
    }
}

extern "C" ABI void WRAP(just_return)(size_t,void**,size_t,size_t,
                                      U16,U16,U16,U16, U16,U16,U16,U16) {}

// All stages use the same function call ABI to chain into each other, but there are three types:
//   GG: geometry in, geometry out  -- think, a matrix
//   GP: geometry in, pixels out.   -- think, a memory gather
//   PP: pixels in, pixels out.     -- think, a blend mode
//
// (Some stages ignore their inputs or produce no logical output.  That's perfectly fine.)
//
// These three STAGE_ macros let you define each type of stage,
// and will have (x,y) geometry and/or (r,g,b,a, dr,dg,db,da) pixel arguments as appropriate.

#define STAGE_GG(name, ...)                                                            \
    SI void name##_k(__VA_ARGS__, size_t dx, size_t dy, size_t tail, F& x, F& y);      \
    extern "C" ABI void WRAP(name)(size_t tail, void** program, size_t dx, size_t dy,  \
                                   U16  r, U16  g, U16  b, U16  a,                     \
                                   U16 dr, U16 dg, U16 db, U16 da) {                   \
        auto x = join<F>(r,g),                                                         \
             y = join<F>(b,a);                                                         \
        name##_k(Ctx{program}, dx,dy,tail, x,y);                                       \
        split(x, &r,&g);                                                               \
        split(y, &b,&a);                                                               \
        auto next = (Stage)load_and_inc(program);                                      \
        next(tail,program,dx,dy, r,g,b,a, dr,dg,db,da);                                \
    }                                                                                  \
    SI void name##_k(__VA_ARGS__, size_t dx, size_t dy, size_t tail, F& x, F& y)

#define STAGE_GP(name, ...)                                                            \
    SI void name##_k(__VA_ARGS__, size_t dx, size_t dy, size_t tail, F x, F y,         \
                     U16&  r, U16&  g, U16&  b, U16&  a,                               \
                     U16& dr, U16& dg, U16& db, U16& da);                              \
    extern "C" ABI void WRAP(name)(size_t tail, void** program, size_t dx, size_t dy,  \
                                   U16  r, U16  g, U16  b, U16  a,                     \
                                   U16 dr, U16 dg, U16 db, U16 da) {                   \
        auto x = join<F>(r,g),                                                         \
             y = join<F>(b,a);                                                         \
        name##_k(Ctx{program}, dx,dy,tail, x,y, r,g,b,a, dr,dg,db,da);                 \
        auto next = (Stage)load_and_inc(program);                                      \
        next(tail,program,dx,dy, r,g,b,a, dr,dg,db,da);                                \
    }                                                                                  \
    SI void name##_k(__VA_ARGS__, size_t dx, size_t dy, size_t tail, F x, F y,         \
                     U16&  r, U16&  g, U16&  b, U16&  a,                               \
                     U16& dr, U16& dg, U16& db, U16& da)

#define STAGE_PP(name, ...)                                                            \
    SI void name##_k(__VA_ARGS__, size_t dx, size_t dy, size_t tail,                   \
                     U16&  r, U16&  g, U16&  b, U16&  a,                               \
                     U16& dr, U16& dg, U16& db, U16& da);                              \
    extern "C" ABI void WRAP(name)(size_t tail, void** program, size_t dx, size_t dy,  \
                                   U16  r, U16  g, U16  b, U16  a,                     \
                                   U16 dr, U16 dg, U16 db, U16 da) {                   \
        name##_k(Ctx{program}, dx,dy,tail, r,g,b,a, dr,dg,db,da);                      \
        auto next = (Stage)load_and_inc(program);                                      \
        next(tail,program,dx,dy, r,g,b,a, dr,dg,db,da);                                \
    }                                                                                  \
    SI void name##_k(__VA_ARGS__, size_t dx, size_t dy, size_t tail,                   \
                     U16&  r, U16&  g, U16&  b, U16&  a,                               \
                     U16& dr, U16& dg, U16& db, U16& da)

// ~~~~~~ Commonly used helper functions ~~~~~~ //

SI U16 div255(U16 v) {
#if 0
    return (v+127)/255;  // The ideal rounding divide by 255.
#else
    return (v+255)/256;  // A good approximation of (v+127)/255.
#endif
}

SI U16 inv(U16 v) { return 255-v; }

SI U16 if_then_else(I16 c, U16 t, U16 e) { return (t & c) | (e & ~c); }
SI U32 if_then_else(I32 c, U32 t, U32 e) { return (t & c) | (e & ~c); }

SI U16 max(U16 x, U16 y) { return if_then_else(x < y, y, x); }
SI U16 min(U16 x, U16 y) { return if_then_else(x < y, x, y); }
SI U16 max(U16 x, U16 y, U16 z) { return max(x, max(y, z)); }
SI U16 min(U16 x, U16 y, U16 z) { return min(x, min(y, z)); }

SI U16 from_float(float f) { return f * 255.0f + 0.5f; }

SI U16 lerp(U16 from, U16 to, U16 t) { return div255( from*inv(t) + to*t ); }

template <typename D, typename S>
SI D cast(S src) {
    return __builtin_convertvector(src, D);
}

template <typename D, typename S>
SI void split(S v, D* lo, D* hi) {
    static_assert(2*sizeof(D) == sizeof(S), "");
    memcpy(lo, (const char*)&v + 0*sizeof(D), sizeof(D));
    memcpy(hi, (const char*)&v + 1*sizeof(D), sizeof(D));
}
template <typename D, typename S>
SI D join(S lo, S hi) {
    static_assert(sizeof(D) == 2*sizeof(S), "");
    D v;
    memcpy((char*)&v + 0*sizeof(S), &lo, sizeof(S));
    memcpy((char*)&v + 1*sizeof(S), &hi, sizeof(S));
    return v;
}
template <typename V, typename H>
SI V map(V v, H (*fn)(H)) {
    H lo,hi;
    split(v, &lo,&hi);
    lo = fn(lo);
    hi = fn(hi);
    return join<V>(lo,hi);
}

// TODO: do we need platform-specific intrinsics for any of these?
SI F if_then_else(I32 c, F t, F e) {
    return bit_cast<F>( (bit_cast<I32>(t) & c) | (bit_cast<I32>(e) & ~c) );
}
SI F max(F x, F y) { return if_then_else(x < y, y, x); }
SI F min(F x, F y) { return if_then_else(x < y, x, y); }

SI F mad(F f, F m, F a) { return f*m+a; }
SI U32 trunc_(F x) { return (U32)cast<I32>(x); }

SI F rcp(F x) {
#if defined(__AVX2__)
    return map(x, _mm256_rcp_ps);
#elif defined(__SSE__)
    return map(x, _mm_rcp_ps);
#elif defined(__ARM_NEON)
    return map(x, +[](float32x4_t v) {
        auto est = vrecpeq_f32(v);
        return vrecpsq_f32(v,est)*est;
    });
#else
    return 1.0f / x;
#endif
}
SI F sqrt_(F x) {
#if defined(__AVX2__)
    return map(x, _mm256_sqrt_ps);
#elif defined(__SSE__)
    return map(x, _mm_sqrt_ps);
#elif defined(__aarch64__)
    return map(x, vsqrtq_f32);
#elif defined(__ARM_NEON)
    return map(x, +[](float32x4_t v) {
        auto est = vrsqrteq_f32(v);  // Estimate and two refinement steps for est = rsqrt(v).
        est *= vrsqrtsq_f32(v,est*est);
        est *= vrsqrtsq_f32(v,est*est);
        return v*est;                // sqrt(v) == v*rsqrt(v).
    });
#else
    return F{
        sqrtf(x[0]), sqrtf(x[1]), sqrtf(x[2]), sqrtf(x[3]),
        sqrtf(x[4]), sqrtf(x[5]), sqrtf(x[6]), sqrtf(x[7]),
    };
#endif
}

SI F floor_(F x) {
#if defined(__aarch64__)
    return map(x, vrndmq_f32);
#elif defined(__AVX2__)
    return map(x, +[](__m256 v){ return _mm256_floor_ps(v); });  // _mm256_floor_ps is a macro...
#elif defined(__SSE4_1__)
    return map(x, +[](__m128 v){ return    _mm_floor_ps(v); });  // _mm_floor_ps() is a macro too.
#else
    F roundtrip = cast<F>(cast<I32>(x));
    return roundtrip - if_then_else(roundtrip > x, F(1), F(0));
#endif
}
SI F abs_(F x) { return bit_cast<F>( bit_cast<I32>(x) & 0x7fffffff ); }

// ~~~~~~ Basic / misc. stages ~~~~~~ //

STAGE_GG(seed_shader, const float* iota) {
    x = cast<F>(I32(dx)) + unaligned_load<F>(iota);
    y = cast<F>(I32(dy)) + 0.5f;
}

STAGE_GG(matrix_translate, const float* m) {
    x += m[0];
    y += m[1];
}
STAGE_GG(matrix_scale_translate, const float* m) {
    x = mad(x,m[0], m[2]);
    y = mad(y,m[1], m[3]);
}
STAGE_GG(matrix_2x3, const float* m) {
    auto X = mad(x,m[0], mad(y,m[2], m[4])),
         Y = mad(x,m[1], mad(y,m[3], m[5]));
    x = X;
    y = Y;
}
STAGE_GG(matrix_perspective, const float* m) {
    // N.B. Unlike the other matrix_ stages, this matrix is row-major.
    auto X = mad(x,m[0], mad(y,m[1], m[2])),
         Y = mad(x,m[3], mad(y,m[4], m[5])),
         Z = mad(x,m[6], mad(y,m[7], m[8]));
    x = X * rcp(Z);
    y = Y * rcp(Z);
}

STAGE_PP(uniform_color, const SkJumper_UniformColorCtx* c) {
    r = c->rgba[0];
    g = c->rgba[1];
    b = c->rgba[2];
    a = c->rgba[3];
}
STAGE_PP(black_color, Ctx::None) { r = g = b =   0; a = 255; }
STAGE_PP(white_color, Ctx::None) { r = g = b = 255; a = 255; }

STAGE_PP(set_rgb, const float rgb[3]) {
    r = from_float(rgb[0]);
    g = from_float(rgb[1]);
    b = from_float(rgb[2]);
}

STAGE_PP(clamp_a, Ctx::None) {
    r = min(r, a);
    g = min(g, a);
    b = min(b, a);
}
STAGE_PP(clamp_a_dst, Ctx::None) {
    dr = min(dr, da);
    dg = min(dg, da);
    db = min(db, da);
}

STAGE_PP(premul, Ctx::None) {
    r = div255(r * a);
    g = div255(g * a);
    b = div255(b * a);
}
STAGE_PP(premul_dst, Ctx::None) {
    dr = div255(dr * da);
    dg = div255(dg * da);
    db = div255(db * da);
}

STAGE_PP(swap_rb, Ctx::None) {
    auto tmp = r;
    r = b;
    b = tmp;
}

STAGE_PP(move_src_dst, Ctx::None) {
    dr = r;
    dg = g;
    db = b;
    da = a;
}

STAGE_PP(move_dst_src, Ctx::None) {
    r = dr;
    g = dg;
    b = db;
    a = da;
}

STAGE_PP(invert, Ctx::None) {
    r = inv(r);
    g = inv(g);
    b = inv(b);
    a = inv(a);
}

// ~~~~~~ Blend modes ~~~~~~ //

// The same logic applied to all 4 channels.
#define BLEND_MODE(name)                                 \
    SI U16 name##_channel(U16 s, U16 d, U16 sa, U16 da); \
    STAGE_PP(name, Ctx::None) {                          \
        r = name##_channel(r,dr,a,da);                   \
        g = name##_channel(g,dg,a,da);                   \
        b = name##_channel(b,db,a,da);                   \
        a = name##_channel(a,da,a,da);                   \
    }                                                    \
    SI U16 name##_channel(U16 s, U16 d, U16 sa, U16 da)

    BLEND_MODE(clear)    { return 0; }
    BLEND_MODE(srcatop)  { return div255( s*da + d*inv(sa) ); }
    BLEND_MODE(dstatop)  { return div255( d*sa + s*inv(da) ); }
    BLEND_MODE(srcin)    { return div255( s*da ); }
    BLEND_MODE(dstin)    { return div255( d*sa ); }
    BLEND_MODE(srcout)   { return div255( s*inv(da) ); }
    BLEND_MODE(dstout)   { return div255( d*inv(sa) ); }
    BLEND_MODE(srcover)  { return s + div255( d*inv(sa) ); }
    BLEND_MODE(dstover)  { return d + div255( s*inv(da) ); }
    BLEND_MODE(modulate) { return div255( s*d ); }
    BLEND_MODE(multiply) { return div255( s*inv(da) + d*inv(sa) + s*d ); }
    BLEND_MODE(plus_)    { return min(s+d, 255); }
    BLEND_MODE(screen)   { return s + d - div255( s*d ); }
    BLEND_MODE(xor_)     { return div255( s*inv(da) + d*inv(sa) ); }
#undef BLEND_MODE

// The same logic applied to color, and srcover for alpha.
#define BLEND_MODE(name)                                 \
    SI U16 name##_channel(U16 s, U16 d, U16 sa, U16 da); \
    STAGE_PP(name, Ctx::None) {                          \
        r = name##_channel(r,dr,a,da);                   \
        g = name##_channel(g,dg,a,da);                   \
        b = name##_channel(b,db,a,da);                   \
        a = a + div255( da*inv(a) );                     \
    }                                                    \
    SI U16 name##_channel(U16 s, U16 d, U16 sa, U16 da)

    BLEND_MODE(darken)     { return s + d -   div255( max(s*da, d*sa) ); }
    BLEND_MODE(lighten)    { return s + d -   div255( min(s*da, d*sa) ); }
    BLEND_MODE(difference) { return s + d - 2*div255( min(s*da, d*sa) ); }
    BLEND_MODE(exclusion)  { return s + d - 2*div255( s*d ); }

    BLEND_MODE(hardlight) {
        return div255( s*inv(da) + d*inv(sa) +
                       if_then_else(2*s <= sa, 2*s*d, sa*da - 2*(sa-s)*(da-d)) );
    }
    BLEND_MODE(overlay) {
        return div255( s*inv(da) + d*inv(sa) +
                       if_then_else(2*d <= da, 2*s*d, sa*da - 2*(sa-s)*(da-d)) );
    }
#undef BLEND_MODE

// ~~~~~~ Helpers for interacting with memory ~~~~~~ //

template <typename T>
SI T* ptr_at_xy(const SkJumper_MemoryCtx* ctx, size_t dx, size_t dy) {
    return (T*)ctx->pixels + dy*ctx->stride + dx;
}

template <typename T>
SI U32 ix_and_ptr(T** ptr, const SkJumper_GatherCtx* ctx, F x, F y) {
    auto clamp = [](F v, F limit) {
        limit = bit_cast<F>( bit_cast<U32>(limit) - 1 );  // Exclusive -> inclusive.
        return min(max(0, v), limit);
    };
    x = clamp(x, ctx->width);
    y = clamp(y, ctx->height);

    *ptr = (const T*)ctx->pixels;
    return trunc_(y)*ctx->stride + trunc_(x);
}

template <typename V, typename T>
SI V load(const T* ptr, size_t tail) {
    V v = 0;
    switch (tail & (N-1)) {
        case  0: memcpy(&v, ptr, sizeof(v)); break;
    #if defined(__AVX2__)
        case 15: v[14] = ptr[14];
        case 14: v[13] = ptr[13];
        case 13: v[12] = ptr[12];
        case 12: memcpy(&v, ptr, 12*sizeof(T)); break;
        case 11: v[10] = ptr[10];
        case 10: v[ 9] = ptr[ 9];
        case  9: v[ 8] = ptr[ 8];
        case  8: memcpy(&v, ptr,  8*sizeof(T)); break;
    #endif
        case  7: v[ 6] = ptr[ 6];
        case  6: v[ 5] = ptr[ 5];
        case  5: v[ 4] = ptr[ 4];
        case  4: memcpy(&v, ptr,  4*sizeof(T)); break;
        case  3: v[ 2] = ptr[ 2];
        case  2: memcpy(&v, ptr,  2*sizeof(T)); break;
        case  1: v[ 0] = ptr[ 0];
    }
    return v;
}
template <typename V, typename T>
SI void store(T* ptr, size_t tail, V v) {
    switch (tail & (N-1)) {
        case  0: memcpy(ptr, &v, sizeof(v)); break;
    #if defined(__AVX2__)
        case 15: ptr[14] = v[14];
        case 14: ptr[13] = v[13];
        case 13: ptr[12] = v[12];
        case 12: memcpy(ptr, &v, 12*sizeof(T)); break;
        case 11: ptr[10] = v[10];
        case 10: ptr[ 9] = v[ 9];
        case  9: ptr[ 8] = v[ 8];
        case  8: memcpy(ptr, &v,  8*sizeof(T)); break;
    #endif
        case  7: ptr[ 6] = v[ 6];
        case  6: ptr[ 5] = v[ 5];
        case  5: ptr[ 4] = v[ 4];
        case  4: memcpy(ptr, &v,  4*sizeof(T)); break;
        case  3: ptr[ 2] = v[ 2];
        case  2: memcpy(ptr, &v,  2*sizeof(T)); break;
        case  1: ptr[ 0] = v[ 0];
    }
}

#if defined(__AVX2__)
    template <typename V, typename T>
    SI V gather(const T* ptr, U32 ix) {
        return V{ ptr[ix[ 0]], ptr[ix[ 1]], ptr[ix[ 2]], ptr[ix[ 3]],
                  ptr[ix[ 4]], ptr[ix[ 5]], ptr[ix[ 6]], ptr[ix[ 7]],
                  ptr[ix[ 8]], ptr[ix[ 9]], ptr[ix[10]], ptr[ix[11]],
                  ptr[ix[12]], ptr[ix[13]], ptr[ix[14]], ptr[ix[15]], };
    }

    template<>
    F gather(const float* p, U32 ix) {
        __m256i lo, hi;
        split(ix, &lo, &hi);

        return join<F>(_mm256_i32gather_ps(p, lo, 4),
                       _mm256_i32gather_ps(p, hi, 4));
    }

    template<>
    U32 gather(const uint32_t* p, U32 ix) {
        __m256i lo, hi;
        split(ix, &lo, &hi);

        return join<U32>(_mm256_i32gather_epi32(p, lo, 4),
                         _mm256_i32gather_epi32(p, hi, 4));
    }
#else
    template <typename V, typename T>
    SI V gather(const T* ptr, U32 ix) {
        return V{ ptr[ix[ 0]], ptr[ix[ 1]], ptr[ix[ 2]], ptr[ix[ 3]],
                  ptr[ix[ 4]], ptr[ix[ 5]], ptr[ix[ 6]], ptr[ix[ 7]], };
    }
#endif


// ~~~~~~ 32-bit memory loads and stores ~~~~~~ //

SI void from_8888(U32 rgba, U16* r, U16* g, U16* b, U16* a) {
#if 1 && defined(__AVX2__)
    // Swap the middle 128-bit lanes to make _mm256_packus_epi32() in cast_U16() work out nicely.
    __m256i _01,_23;
    split(rgba, &_01, &_23);
    __m256i _02 = _mm256_permute2x128_si256(_01,_23, 0x20),
            _13 = _mm256_permute2x128_si256(_01,_23, 0x31);
    rgba = join<U32>(_02, _13);

    auto cast_U16 = [](U32 v) -> U16 {
        __m256i _02,_13;
        split(v, &_02,&_13);
        return _mm256_packus_epi32(_02,_13);
    };
#else
    auto cast_U16 = [](U32 v) -> U16 {
        return cast<U16>(v);
    };
#endif
    *r = cast_U16(rgba & 65535) & 255;
    *g = cast_U16(rgba & 65535) >>  8;
    *b = cast_U16(rgba >>   16) & 255;
    *a = cast_U16(rgba >>   16) >>  8;
}

SI void load_8888(const uint32_t* ptr, size_t tail, U16* r, U16* g, U16* b, U16* a) {
#if 1 && defined(__ARM_NEON)
    uint8x8x4_t rgba;
    switch (tail & (N-1)) {
        case 0: rgba = vld4_u8     ((const uint8_t*)(ptr+0)         ); break;
        case 7: rgba = vld4_lane_u8((const uint8_t*)(ptr+6), rgba, 6);
        case 6: rgba = vld4_lane_u8((const uint8_t*)(ptr+5), rgba, 5);
        case 5: rgba = vld4_lane_u8((const uint8_t*)(ptr+4), rgba, 4);
        case 4: rgba = vld4_lane_u8((const uint8_t*)(ptr+3), rgba, 3);
        case 3: rgba = vld4_lane_u8((const uint8_t*)(ptr+2), rgba, 2);
        case 2: rgba = vld4_lane_u8((const uint8_t*)(ptr+1), rgba, 1);
        case 1: rgba = vld4_lane_u8((const uint8_t*)(ptr+0), rgba, 0);
    }
    *r = cast<U16>(rgba.val[0]);
    *g = cast<U16>(rgba.val[1]);
    *b = cast<U16>(rgba.val[2]);
    *a = cast<U16>(rgba.val[3]);
#else
    from_8888(load<U32>(ptr, tail), r,g,b,a);
#endif
}
SI void store_8888(uint32_t* ptr, size_t tail, U16 r, U16 g, U16 b, U16 a) {
#if 1 && defined(__ARM_NEON)
    uint8x8x4_t rgba = {{
        cast<U8>(r),
        cast<U8>(g),
        cast<U8>(b),
        cast<U8>(a),
    }};
    switch (tail & (N-1)) {
        case 0: vst4_u8     ((uint8_t*)(ptr+0), rgba   ); break;
        case 7: vst4_lane_u8((uint8_t*)(ptr+6), rgba, 6);
        case 6: vst4_lane_u8((uint8_t*)(ptr+5), rgba, 5);
        case 5: vst4_lane_u8((uint8_t*)(ptr+4), rgba, 4);
        case 4: vst4_lane_u8((uint8_t*)(ptr+3), rgba, 3);
        case 3: vst4_lane_u8((uint8_t*)(ptr+2), rgba, 2);
        case 2: vst4_lane_u8((uint8_t*)(ptr+1), rgba, 1);
        case 1: vst4_lane_u8((uint8_t*)(ptr+0), rgba, 0);
    }
#else
    store(ptr, tail, cast<U32>(r | (g<<8)) <<  0
                   | cast<U32>(b | (a<<8)) << 16);
#endif
}

STAGE_PP(load_8888, const SkJumper_MemoryCtx* ctx) {
    load_8888(ptr_at_xy<const uint32_t>(ctx, dx,dy), tail, &r,&g,&b,&a);
}
STAGE_PP(load_8888_dst, const SkJumper_MemoryCtx* ctx) {
    load_8888(ptr_at_xy<const uint32_t>(ctx, dx,dy), tail, &dr,&dg,&db,&da);
}
STAGE_PP(store_8888, const SkJumper_MemoryCtx* ctx) {
    store_8888(ptr_at_xy<uint32_t>(ctx, dx,dy), tail, r,g,b,a);
}

STAGE_PP(load_bgra, const SkJumper_MemoryCtx* ctx) {
    load_8888(ptr_at_xy<const uint32_t>(ctx, dx,dy), tail, &b,&g,&r,&a);
}
STAGE_PP(load_bgra_dst, const SkJumper_MemoryCtx* ctx) {
    load_8888(ptr_at_xy<const uint32_t>(ctx, dx,dy), tail, &db,&dg,&dr,&da);
}
STAGE_PP(store_bgra, const SkJumper_MemoryCtx* ctx) {
    store_8888(ptr_at_xy<uint32_t>(ctx, dx,dy), tail, b,g,r,a);
}

STAGE_GP(gather_8888, const SkJumper_GatherCtx* ctx) {
    const uint32_t* ptr;
    U32 ix = ix_and_ptr(&ptr, ctx, x,y);
    from_8888(gather<U32>(ptr, ix), &r, &g, &b, &a);
}
STAGE_GP(gather_bgra, const SkJumper_GatherCtx* ctx) {
    const uint32_t* ptr;
    U32 ix = ix_and_ptr(&ptr, ctx, x,y);
    from_8888(gather<U32>(ptr, ix), &b, &g, &r, &a);
}

// ~~~~~~ 16-bit memory loads and stores ~~~~~~ //

SI void from_565(U16 rgb, U16* r, U16* g, U16* b) {
    // Format for 565 buffers: 15|rrrrr gggggg bbbbb|0
    U16 R = (rgb >> 11) & 31,
        G = (rgb >>  5) & 63,
        B = (rgb >>  0) & 31;

    // These bit replications are the same as multiplying by 255/31 or 255/63 to scale to 8-bit.
    *r = (R << 3) | (R >> 2);
    *g = (G << 2) | (G >> 4);
    *b = (B << 3) | (B >> 2);
}
SI void load_565(const uint16_t* ptr, size_t tail, U16* r, U16* g, U16* b) {
    from_565(load<U16>(ptr, tail), r,g,b);
}
SI void store_565(uint16_t* ptr, size_t tail, U16 r, U16 g, U16 b) {
    // Select the top 5,6,5 bits.
    U16 R = r >> 3,
        G = g >> 2,
        B = b >> 3;
    // Pack them back into 15|rrrrr gggggg bbbbb|0.
    store(ptr, tail, R << 11
                   | G <<  5
                   | B <<  0);
}

STAGE_PP(load_565, const SkJumper_MemoryCtx* ctx) {
    load_565(ptr_at_xy<const uint16_t>(ctx, dx,dy), tail, &r,&g,&b);
    a = 255;
}
STAGE_PP(load_565_dst, const SkJumper_MemoryCtx* ctx) {
    load_565(ptr_at_xy<const uint16_t>(ctx, dx,dy), tail, &dr,&dg,&db);
    da = 255;
}
STAGE_PP(store_565, const SkJumper_MemoryCtx* ctx) {
    store_565(ptr_at_xy<uint16_t>(ctx, dx,dy), tail, r,g,b);
}
STAGE_GP(gather_565, const SkJumper_GatherCtx* ctx) {
    const uint16_t* ptr;
    U32 ix = ix_and_ptr(&ptr, ctx, x,y);
    from_565(gather<U16>(ptr, ix), &r, &g, &b);
    a = 255;
}

SI void from_4444(U16 rgba, U16* r, U16* g, U16* b, U16* a) {
    // Format for 4444 buffers: 15|rrrr gggg bbbb aaaa|0.
    U16 R = (rgba >> 12) & 15,
        G = (rgba >>  8) & 15,
        B = (rgba >>  4) & 15,
        A = (rgba >>  0) & 15;

    // Scale [0,15] to [0,255].
    *r = (R << 4) | R;
    *g = (G << 4) | G;
    *b = (B << 4) | B;
    *a = (A << 4) | A;
}
SI void load_4444(const uint16_t* ptr, size_t tail, U16* r, U16* g, U16* b, U16* a) {
    from_4444(load<U16>(ptr, tail), r,g,b,a);
}
SI void store_4444(uint16_t* ptr, size_t tail, U16 r, U16 g, U16 b, U16 a) {
    // Select the top 4 bits of each.
    U16 R = r >> 4,
        G = g >> 4,
        B = b >> 4,
        A = a >> 4;
    // Pack them back into 15|rrrr gggg bbbb aaaa|0.
    store(ptr, tail, R << 12
                   | G <<  8
                   | B <<  4
                   | A <<  0);
}

STAGE_PP(load_4444, const SkJumper_MemoryCtx* ctx) {
    load_4444(ptr_at_xy<const uint16_t>(ctx, dx,dy), tail, &r,&g,&b,&a);
}
STAGE_PP(load_4444_dst, const SkJumper_MemoryCtx* ctx) {
    load_4444(ptr_at_xy<const uint16_t>(ctx, dx,dy), tail, &dr,&dg,&db,&da);
}
STAGE_PP(store_4444, const SkJumper_MemoryCtx* ctx) {
    store_4444(ptr_at_xy<uint16_t>(ctx, dx,dy), tail, r,g,b,a);
}
STAGE_GP(gather_4444, const SkJumper_GatherCtx* ctx) {
    const uint16_t* ptr;
    U32 ix = ix_and_ptr(&ptr, ctx, x,y);
    from_4444(gather<U16>(ptr, ix), &r,&g,&b,&a);
}

// ~~~~~~ 8-bit memory loads and stores ~~~~~~ //

SI U16 load_8(const uint8_t* ptr, size_t tail) {
    return cast<U16>(load<U8>(ptr, tail));
}
SI void store_8(uint8_t* ptr, size_t tail, U16 v) {
    store(ptr, tail, cast<U8>(v));
}

STAGE_PP(load_a8, const SkJumper_MemoryCtx* ctx) {
    r = g = b = 0;
    a = load_8(ptr_at_xy<const uint8_t>(ctx, dx,dy), tail);
}
STAGE_PP(load_a8_dst, const SkJumper_MemoryCtx* ctx) {
    dr = dg = db = 0;
    da = load_8(ptr_at_xy<const uint8_t>(ctx, dx,dy), tail);
}
STAGE_PP(store_a8, const SkJumper_MemoryCtx* ctx) {
    store_8(ptr_at_xy<uint8_t>(ctx, dx,dy), tail, a);
}
STAGE_GP(gather_a8, const SkJumper_GatherCtx* ctx) {
    const uint8_t* ptr;
    U32 ix = ix_and_ptr(&ptr, ctx, x,y);
    r = g = b = 0;
    a = cast<U16>(gather<U8>(ptr, ix));
}

STAGE_PP(load_g8, const SkJumper_MemoryCtx* ctx) {
    r = g = b = load_8(ptr_at_xy<const uint8_t>(ctx, dx,dy), tail);
    a = 255;
}
STAGE_PP(load_g8_dst, const SkJumper_MemoryCtx* ctx) {
    dr = dg = db = load_8(ptr_at_xy<const uint8_t>(ctx, dx,dy), tail);
    da = 255;
}
STAGE_PP(luminance_to_alpha, Ctx::None) {
    a = (r*54 + g*183 + b*19)/256;  // 0.2126, 0.7152, 0.0722 with 256 denominator.
    r = g = b = 0;
}
STAGE_GP(gather_g8, const SkJumper_GatherCtx* ctx) {
    const uint8_t* ptr;
    U32 ix = ix_and_ptr(&ptr, ctx, x,y);
    r = g = b = cast<U16>(gather<U8>(ptr, ix));
    a = 255;
}

// ~~~~~~ Coverage scales / lerps ~~~~~~ //

STAGE_PP(scale_1_float, const float* f) {
    U16 c = from_float(*f);
    r = div255( r * c );
    g = div255( g * c );
    b = div255( b * c );
    a = div255( a * c );
}
STAGE_PP(lerp_1_float, const float* f) {
    U16 c = from_float(*f);
    r = lerp(dr, r, c);
    g = lerp(dg, g, c);
    b = lerp(db, b, c);
    a = lerp(da, a, c);
}

STAGE_PP(scale_u8, const SkJumper_MemoryCtx* ctx) {
    U16 c = load_8(ptr_at_xy<const uint8_t>(ctx, dx,dy), tail);
    r = div255( r * c );
    g = div255( g * c );
    b = div255( b * c );
    a = div255( a * c );
}
STAGE_PP(lerp_u8, const SkJumper_MemoryCtx* ctx) {
    U16 c = load_8(ptr_at_xy<const uint8_t>(ctx, dx,dy), tail);
    r = lerp(dr, r, c);
    g = lerp(dg, g, c);
    b = lerp(db, b, c);
    a = lerp(da, a, c);
}

// Derive alpha's coverage from rgb coverage and the values of src and dst alpha.
SI U16 alpha_coverage_from_rgb_coverage(U16 a, U16 da, U16 cr, U16 cg, U16 cb) {
    return if_then_else(a < da, min(cr,cg,cb)
                              , max(cr,cg,cb));
}
STAGE_PP(scale_565, const SkJumper_MemoryCtx* ctx) {
    U16 cr,cg,cb;
    load_565(ptr_at_xy<const uint16_t>(ctx, dx,dy), tail, &cr,&cg,&cb);
    U16 ca = alpha_coverage_from_rgb_coverage(a,da, cr,cg,cb);

    r = div255( r * cr );
    g = div255( g * cg );
    b = div255( b * cb );
    a = div255( a * ca );
}
STAGE_PP(lerp_565, const SkJumper_MemoryCtx* ctx) {
    U16 cr,cg,cb;
    load_565(ptr_at_xy<const uint16_t>(ctx, dx,dy), tail, &cr,&cg,&cb);
    U16 ca = alpha_coverage_from_rgb_coverage(a,da, cr,cg,cb);

    r = lerp(dr, r, cr);
    g = lerp(dg, g, cg);
    b = lerp(db, b, cb);
    a = lerp(da, a, ca);
}

// ~~~~~~ Gradient stages ~~~~~~ //

// Clamp x to [0,1], both sides inclusive (think, gradients).
// Even repeat and mirror funnel through a clamp to handle bad inputs like +Inf, NaN.
SI F clamp_01(F v) { return min(max(0, v), 1); }

STAGE_GG(clamp_x_1 , Ctx::None) { x = clamp_01(x); }
STAGE_GG(repeat_x_1, Ctx::None) { x = clamp_01(x - floor_(x)); }
STAGE_GG(mirror_x_1, Ctx::None) {
    auto two = [](F x){ return x+x; };
    x = clamp_01(abs_( (x-1.0f) - two(floor_((x-1.0f)*0.5f)) - 1.0f ));
}

SI U16 round_F_to_U16(F x) { return cast<U16>(x * 255.0f + 0.5f); }

SI void gradient_lookup(const SkJumper_GradientCtx* c, U32 idx, F t,
                        U16* r, U16* g, U16* b, U16* a) {

    F fr, fg, fb, fa, br, bg, bb, ba;
#if defined(__AVX2__)
    if (c->stopCount <=8) {
        __m256i lo, hi;
        split(idx, &lo, &hi);

        fr = join<F>(_mm256_permutevar8x32_ps(_mm256_loadu_ps(c->fs[0]), lo),
                     _mm256_permutevar8x32_ps(_mm256_loadu_ps(c->fs[0]), hi));
        br = join<F>(_mm256_permutevar8x32_ps(_mm256_loadu_ps(c->bs[0]), lo),
                     _mm256_permutevar8x32_ps(_mm256_loadu_ps(c->bs[0]), hi));
        fg = join<F>(_mm256_permutevar8x32_ps(_mm256_loadu_ps(c->fs[1]), lo),
                     _mm256_permutevar8x32_ps(_mm256_loadu_ps(c->fs[1]), hi));
        bg = join<F>(_mm256_permutevar8x32_ps(_mm256_loadu_ps(c->bs[1]), lo),
                     _mm256_permutevar8x32_ps(_mm256_loadu_ps(c->bs[1]), hi));
        fb = join<F>(_mm256_permutevar8x32_ps(_mm256_loadu_ps(c->fs[2]), lo),
                     _mm256_permutevar8x32_ps(_mm256_loadu_ps(c->fs[2]), hi));
        bb = join<F>(_mm256_permutevar8x32_ps(_mm256_loadu_ps(c->bs[2]), lo),
                     _mm256_permutevar8x32_ps(_mm256_loadu_ps(c->bs[2]), hi));
        fa = join<F>(_mm256_permutevar8x32_ps(_mm256_loadu_ps(c->fs[3]), lo),
                     _mm256_permutevar8x32_ps(_mm256_loadu_ps(c->fs[3]), hi));
        ba = join<F>(_mm256_permutevar8x32_ps(_mm256_loadu_ps(c->bs[3]), lo),
                     _mm256_permutevar8x32_ps(_mm256_loadu_ps(c->bs[3]), hi));
    } else
#endif
    {
        fr = gather<F>(c->fs[0], idx);
        fg = gather<F>(c->fs[1], idx);
        fb = gather<F>(c->fs[2], idx);
        fa = gather<F>(c->fs[3], idx);
        br = gather<F>(c->bs[0], idx);
        bg = gather<F>(c->bs[1], idx);
        bb = gather<F>(c->bs[2], idx);
        ba = gather<F>(c->bs[3], idx);
    }
    *r = round_F_to_U16(mad(t, fr, br));
    *g = round_F_to_U16(mad(t, fg, bg));
    *b = round_F_to_U16(mad(t, fb, bb));
    *a = round_F_to_U16(mad(t, fa, ba));
}

STAGE_GP(gradient, const SkJumper_GradientCtx* c) {
    auto t = x;
    U32 idx = 0;

    // N.B. The loop starts at 1 because idx 0 is the color to use before the first stop.
    for (size_t i = 1; i < c->stopCount; i++) {
        idx += if_then_else(t >= c->ts[i], U32(1), U32(0));
    }

    gradient_lookup(c, idx, t, &r, &g, &b, &a);
}

STAGE_GP(evenly_spaced_gradient, const SkJumper_GradientCtx* c) {
    auto t = x;
    auto idx = trunc_(t * (c->stopCount-1));
    gradient_lookup(c, idx, t, &r, &g, &b, &a);
}

STAGE_GP(evenly_spaced_2_stop_gradient, const void* ctx) {
    // TODO: Rename Ctx SkJumper_EvenlySpaced2StopGradientCtx.
    struct Ctx { float f[4], b[4]; };
    auto c = (const Ctx*)ctx;

    auto t = x;
    r = round_F_to_U16(mad(t, c->f[0], c->b[0]));
    g = round_F_to_U16(mad(t, c->f[1], c->b[1]));
    b = round_F_to_U16(mad(t, c->f[2], c->b[2]));
    a = round_F_to_U16(mad(t, c->f[3], c->b[3]));
}

STAGE_GG(xy_to_unit_angle, Ctx::None) {
    F xabs = abs_(x),
      yabs = abs_(y);

    F slope = min(xabs, yabs)/max(xabs, yabs);
    F s = slope * slope;

    // Use a 7th degree polynomial to approximate atan.
    // This was generated using sollya.gforge.inria.fr.
    // A float optimized polynomial was generated using the following command.
    // P1 = fpminimax((1/(2*Pi))*atan(x),[|1,3,5,7|],[|24...|],[2^(-40),1],relative);
    F phi = slope
             * (0.15912117063999176025390625f     + s
             * (-5.185396969318389892578125e-2f   + s
             * (2.476101927459239959716796875e-2f + s
             * (-7.0547382347285747528076171875e-3f))));

    phi = if_then_else(xabs < yabs, 1.0f/4.0f - phi, phi);
    phi = if_then_else(x < 0.0f   , 1.0f/2.0f - phi, phi);
    phi = if_then_else(y < 0.0f   , 1.0f - phi     , phi);
    phi = if_then_else(phi != phi , 0              , phi);  // Check for NaN.
    x = phi;
}
STAGE_GG(xy_to_radius, Ctx::None) {
    x = sqrt_(x*x + y*y);
}

// ~~~~~~ Compound stages ~~~~~~ //

STAGE_PP(srcover_rgba_8888, const SkJumper_MemoryCtx* ctx) {
    auto ptr = ptr_at_xy<uint32_t>(ctx, dx,dy);

    load_8888(ptr, tail, &dr,&dg,&db,&da);
    r = r + div255( dr*inv(a) );
    g = g + div255( dg*inv(a) );
    b = b + div255( db*inv(a) );
    a = a + div255( da*inv(a) );
    store_8888(ptr, tail, r,g,b,a);
}
STAGE_PP(srcover_bgra_8888, const SkJumper_MemoryCtx* ctx) {
    auto ptr = ptr_at_xy<uint32_t>(ctx, dx,dy);

    load_8888(ptr, tail, &db,&dg,&dr,&da);
    r = r + div255( dr*inv(a) );
    g = g + div255( dg*inv(a) );
    b = b + div255( db*inv(a) );
    a = a + div255( da*inv(a) );
    store_8888(ptr, tail, b,g,r,a);
}

#endif//defined(__clang__)
