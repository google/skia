/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkJumper.h"
#include "SkJumper_misc.h"
#include <immintrin.h>

#if !defined(__clang__) || !defined(__x86_64__)
    #error "We're starting with just x86-64 for now, and will always require Clang."
#endif

using K = const SkJumper_constants;

#if defined(__AVX2__)
    #define WRAP(name) sk_##name##_hsw_lowp
    template <typename T> using V = T __attribute__((ext_vector_type(16)));
    static const size_t kStride = 16;
#else
    #define WRAP(name) sk_##name##_ssse3_lowp
    template <typename T> using V = T __attribute__((ext_vector_type(8)));
    static const size_t kStride = 8;
#endif

using U8  = V<uint8_t>;
using U16 = V<uint16_t>;
using U32 = V<uint32_t>;

// See SkFixed15.h for details on this format and its operations.
struct F {
    U16 vec;

    F() = default;
    F(float f) {
        // After adding 256.0f, the SkFixed15 value is the bottom two bytes of the float.
        f += 256.0f;
        vec = unaligned_load<uint16_t>(&f);
    }

    F(U16 v) : vec(v) {}
    operator U16() const { return vec; }
};

SI F operator+(F x, F y) { return x.vec + y.vec; }
SI F operator-(F x, F y) { return x.vec - y.vec; }
SI F operator*(F x, F y) {
#if defined(__AVX2__)
    return _mm256_abs_epi16(_mm256_mulhrs_epi16(x.vec, y.vec));
#else
    return _mm_abs_epi16(_mm_mulhrs_epi16(x.vec, y.vec));
#endif
}

SI F mad(F f, F m, F a) { return f*m+a; }
SI F inv(F v) { return 1.0f - v; }
SI F two(F v) { return v + v; }
SI F lerp(F from, F to, F t) { return to*t + from*inv(t); }

SI F operator<<(F x, int bits) { return x.vec << bits; }
SI F operator>>(F x, int bits) { return x.vec >> bits; }

using Stage = void(K* k, void** program, size_t x, size_t y, size_t tail, F,F,F,F, F,F,F,F);

#if defined(__AVX__)
    // We really want to make sure all paths go through this function's (implicit) vzeroupper.
    // If they don't, we'll experience severe slowdowns when we first use SSE instructions again.
    __attribute__((disable_tail_calls))
#endif
MAYBE_MSABI
extern "C" void WRAP(start_pipeline)(size_t x, size_t y, size_t limit, void** program, K* k) {
    F v;
    auto start = (Stage*)load_and_inc(program);
    while (x + kStride <= limit) {
        start(k,program,x,y,0,    v,v,v,v, v,v,v,v);
        x += kStride;
    }
    if (size_t tail = limit - x) {
        start(k,program,x,y,tail, v,v,v,v, v,v,v,v);
    }
}

#if defined(__AVX__)
    // We really want to make sure all paths go through this function's (implicit) vzeroupper.
    // If they don't, we'll experience severe slowdowns when we first use SSE instructions again.
    __attribute__((disable_tail_calls))
#endif
__attribute__((flatten))  // Force-inline the call to start_pipeline().
MAYBE_MSABI
extern "C" void WRAP(start_pipeline_2d)(size_t x, size_t y, size_t xlimit, size_t ylimit,
                                        void** program, K* k) {
    for (; y < ylimit; y++) {
        WRAP(start_pipeline)(x,y,xlimit, program, k);
    }
}

extern "C" void WRAP(just_return)(K*, void**, size_t,size_t,size_t, F,F,F,F, F,F,F,F) {}

#define STAGE(name)                                                                   \
    SI void name##_k(K* k, LazyCtx ctx, size_t x, size_t y, size_t tail,              \
                     F& r, F& g, F& b, F& a, F& dr, F& dg, F& db, F& da);             \
    extern "C" void WRAP(name)(K* k, void** program, size_t x, size_t y, size_t tail, \
                               F r, F g, F b, F a, F dr, F dg, F db, F da) {          \
        LazyCtx ctx(program);                                                         \
        name##_k(k,ctx,x,y,tail, r,g,b,a, dr,dg,db,da);                               \
        auto next = (Stage*)load_and_inc(program);                                    \
        next(k,program,x,y,tail, r,g,b,a, dr,dg,db,da);                               \
    }                                                                                 \
    SI void name##_k(K* k, LazyCtx ctx, size_t x, size_t y, size_t tail,              \
                     F& r, F& g, F& b, F& a, F& dr, F& dg, F& db, F& da)


// Helper functions used by multiple stages.

template <typename V, typename T>
SI V load(const T* src, size_t tail) {
    __builtin_assume(tail < kStride);
    if (__builtin_expect(tail, 0)) {
        V v{};  // Any inactive lanes are zeroed.
        switch (tail) {
            case 15: v[14] = src[14];
            case 14: v[13] = src[13];
            case 13: v[12] = src[12];
            case 12: memcpy(&v, src, 12*sizeof(T)); break;
            case 11: v[10] = src[10];
            case 10: v[ 9] = src[ 9];
            case  9: v[ 8] = src[ 8];
            case  8: memcpy(&v, src,  8*sizeof(T)); break;
            case  7: v[6] = src[6];
            case  6: v[5] = src[5];
            case  5: v[4] = src[4];
            case  4: memcpy(&v, src,  4*sizeof(T)); break;
            case  3: v[2] = src[2];
            case  2: memcpy(&v, src,  2*sizeof(T)); break;
            case  1: memcpy(&v, src,  1*sizeof(T)); break;
        }
        return v;
    }
    return unaligned_load<V>(src);
}

template <typename V, typename T>
SI void store(T* dst, V v, size_t tail) {
    __builtin_assume(tail < kStride);
    if (__builtin_expect(tail, 0)) {
        switch (tail) {
            case 15: dst[14] = v[14];
            case 14: dst[13] = v[13];
            case 13: dst[12] = v[12];
            case 12: memcpy(dst, &v, 12*sizeof(T)); break;
            case 11: dst[10] = v[10];
            case 10: dst[ 9] = v[ 9];
            case  9: dst[ 8] = v[ 8];
            case  8: memcpy(dst, &v,  8*sizeof(T)); break;
            case  7: dst[6] = v[6];
            case  6: dst[5] = v[5];
            case  5: dst[4] = v[4];
            case  4: memcpy(dst, &v,  4*sizeof(T)); break;
            case  3: dst[2] = v[2];
            case  2: memcpy(dst, &v,  2*sizeof(T)); break;
            case  1: memcpy(dst, &v,  1*sizeof(T)); break;
        }
        return;
    }
    unaligned_store(dst, v);
}

// TODO: mask loads and stores with AVX2

// Scale from [0,255] up to [0,32768].
SI F from_wide_byte(U16 bytes) {
    // Ideally we'd scale by 32768/255 = 128.50196, but instead we'll approximate
    // that a little more cheaply as 256*32897/65536 = 128.50391.
    // 0 and 255 map to 0 and 32768 correctly, and nothing else is off by more than 1 bit.
#if defined(__AVX2__)
    return _mm256_mulhi_epu16(bytes << 8, U16(32897));
#else
    return    _mm_mulhi_epu16(bytes << 8, U16(32897));
#endif
}
SI F from_byte(U8 bytes) {
    return from_wide_byte(__builtin_convertvector(bytes, U16));
}

// Pack from [0,32768] down to [0,255].
SI U16 to_wide_byte(F v) {
    // The simplest thing works great: divide by 128 and saturate.
#if defined(__AVX2__)
    return _mm256_min_epi16(v >> 7, U16(255));
#else
    return    _mm_min_epi16(v >> 7, U16(255));
#endif
}
SI U8 to_byte(F v) {
    // Like to_wide_byte(), but we'll bake the saturation into the 16->8 bit pack.
#if defined(__AVX2__)
    return _mm_packus_epi16(_mm256_extracti128_si256(v >> 7, 0),
                            _mm256_extracti128_si256(v >> 7, 1));
#else
    // Only the bottom 8 bytes are of interest... it doesn't matter what we pack on top.
    __m128i packed = _mm_packus_epi16(v >> 7, v >> 7);
    return unaligned_load<U8>(&packed);
#endif
}

SI void from_8888(U32 rgba, F* r, F* g, F* b, F* a) {
    *r = from_wide_byte(__builtin_convertvector((rgba >>  0) & 0xff, U16));
    *g = from_wide_byte(__builtin_convertvector((rgba >>  8) & 0xff, U16));
    *b = from_wide_byte(__builtin_convertvector((rgba >> 16) & 0xff, U16));
    *a = from_wide_byte(__builtin_convertvector((rgba >> 24) & 0xff, U16));
}

SI U32 to_8888(F r, F g, F b, F a) {
    return __builtin_convertvector(to_wide_byte(r), U32) <<  0
         | __builtin_convertvector(to_wide_byte(g), U32) <<  8
         | __builtin_convertvector(to_wide_byte(b), U32) << 16
         | __builtin_convertvector(to_wide_byte(a), U32) << 24;
}

// Stages!

STAGE(uniform_color) {
    // We're converting to fixed point, which lets us play some IEEE representation tricks,
    // replacing a naive *32768 and float->int conversion with a simple float add.
    using F32x4 = float    __attribute__((ext_vector_type(4)));
    using U16x8 = uint16_t __attribute__((ext_vector_type(8)));
    auto bits = (U16x8)(unaligned_load<F32x4>((const float*)ctx) + 256.0f);
    r = (U16)bits[0];
    g = (U16)bits[2];
    b = (U16)bits[4];
    a = (U16)bits[6];
}

STAGE(black_color) {
    r = g = b = 0.0f;
    a = 1.0f;
}

STAGE(white_color) {
    r = g = b = a = 1.0f;
}

STAGE(set_rgb) {
    auto rgb = (const float*)ctx;
    r = rgb[0];
    g = rgb[1];
    b = rgb[2];
}

STAGE(premul) {
    r = r * a;
    g = g * a;
    b = b * a;
}

STAGE(load_8888) {
    auto ptr = *(const uint32_t**)ctx + x;
    from_8888(load<U32>(ptr, tail), &r,&g,&b,&a);
}
STAGE(load_8888_dst) {
    auto ptr = *(const uint32_t**)ctx + x;
    from_8888(load<U32>(ptr, tail), &dr,&dg,&db,&da);
}
STAGE(store_8888) {
    auto ptr = *(uint32_t**)ctx + x;
    store(ptr, to_8888(r,g,b,a), tail);
}

STAGE(load_bgra) {
    auto ptr = *(const uint32_t**)ctx + x;
    from_8888(load<U32>(ptr, tail), &b,&g,&r,&a);
}
STAGE(load_bgra_dst) {
    auto ptr = *(const uint32_t**)ctx + x;
    from_8888(load<U32>(ptr, tail), &db,&dg,&dr,&da);
}
STAGE(store_bgra) {
    auto ptr = *(uint32_t**)ctx + x;
    store(ptr, to_8888(b,g,r,a), tail);
}

STAGE(load_a8) {
    auto ptr = *(const uint8_t**)ctx + x;
    r = g = b = 0.0f;
    a = from_byte(load<U8>(ptr, tail));
}
STAGE(load_a8_dst) {
    auto ptr = *(const uint8_t**)ctx + x;
    dr = dg = db = 0.0f;
    da = from_byte(load<U8>(ptr, tail));
}
STAGE(store_a8) {
    auto ptr = *(uint8_t**)ctx + x;
    store(ptr, to_byte(a), tail);
}

STAGE(load_g8) {
    auto ptr = *(const uint8_t**)ctx + x;
    r = g = b = from_byte(load<U8>(ptr, tail));
    a = 1.0f;
}

STAGE(load_g8_dst) {
    auto ptr = *(const uint8_t**)ctx + x;
    dr = dg = db = from_byte(load<U8>(ptr, tail));
    da = 1.0f;
}

STAGE(srcover_rgba_8888) {
    auto ptr = *(uint32_t**)ctx + x;

    from_8888(load<U32>(ptr, tail), &dr,&dg,&db,&da);

    r = mad(dr, inv(a), r);
    g = mad(dg, inv(a), g);
    b = mad(db, inv(a), b);
    a = mad(da, inv(a), a);

    store(ptr, to_8888(r,g,b,a), tail);
}

STAGE(scale_1_float) {
    float c = *(const float*)ctx;

    r = r * c;
    g = g * c;
    b = b * c;
    a = a * c;
}
STAGE(scale_u8) {
    auto ptr = *(const uint8_t**)ctx + x;

    U8 scales = load<U8>(ptr, tail);
    F c = from_byte(scales);

    r = r * c;
    g = g * c;
    b = b * c;
    a = a * c;
}

STAGE(lerp_1_float) {
    float c = *(const float*)ctx;

    r = lerp(dr, r, c);
    g = lerp(dg, g, c);
    b = lerp(db, b, c);
    a = lerp(da, a, c);
}
STAGE(lerp_u8) {
    auto ptr = *(const uint8_t**)ctx + x;

    U8 scales = load<U8>(ptr, tail);
    F c = from_byte(scales);

    r = lerp(dr, r, c);
    g = lerp(dg, g, c);
    b = lerp(db, b, c);
    a = lerp(da, a, c);
}

STAGE(swap_rb) {
    auto tmp = r;
    r = b;
    b = tmp;
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

// Most blend modes apply the same logic to each channel.
#define BLEND_MODE(name)                       \
    SI F name##_channel(F s, F d, F sa, F da); \
    STAGE(name) {                              \
        r = name##_channel(r,dr,a,da);         \
        g = name##_channel(g,dg,a,da);         \
        b = name##_channel(b,db,a,da);         \
        a = name##_channel(a,da,a,da);         \
    }                                          \
    SI F name##_channel(F s, F d, F sa, F da)

BLEND_MODE(clear)    { return 0.0f; }
BLEND_MODE(srcatop)  { return s*da + d*inv(sa); }
BLEND_MODE(dstatop)  { return d*sa + s*inv(da); }
BLEND_MODE(srcin)    { return s * da; }
BLEND_MODE(dstin)    { return d * sa; }
BLEND_MODE(srcout)   { return s * inv(da); }
BLEND_MODE(dstout)   { return d * inv(sa); }
BLEND_MODE(srcover)  { return mad(d, inv(sa), s); }
BLEND_MODE(dstover)  { return mad(s, inv(da), d); }

BLEND_MODE(modulate) { return s*d; }
BLEND_MODE(multiply) { return s*inv(da) + d*inv(sa) + s*d; }
BLEND_MODE(screen)   { return s + inv(s)*d; }
BLEND_MODE(xor_)     { return s*inv(da) + d*inv(sa); }

#undef BLEND_MODE
