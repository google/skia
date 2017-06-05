/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkJumper.h"
#include "SkJumper_misc.h"
#include <immintrin.h>

#if !defined(__SSSE3__) || !defined(__clang__) || !defined(__x86_64__)
    #error "We're starting with just SSSE3 x86-64 for now, and will always require Clang."
#endif

#define WRAP(name) sk_##name##_ssse3_lowp

using K = const SkJumper_constants;
static const size_t kStride = 8;

template <typename T> using V = T __attribute__((ext_vector_type(8)));
using U8  = V<uint8_t>;
using U16 = V<uint16_t>;
using U32 = V<uint32_t>;

// See SkFixed15.h for details on this format and its operations.
struct F {
    U16 vec;

    F() = default;
    F(float f) : vec((uint16_t)(f * 0x8000)) {}

    F(U16 v) : vec(v) {}
    operator U16() const { return vec; }
};

SI F operator+(F x, F y) { return x.vec + y.vec; }
SI F operator-(F x, F y) { return x.vec - y.vec; }
SI F operator*(F x, F y) { return _mm_abs_epi16(_mm_mulhrs_epi16(x.vec, y.vec)); }
SI F mad(F f, F m, F a) { return f*m+a; }
SI F inv(F v) { return 1.0f - v; }

SI F operator<<(F x, int bits) { return x.vec << bits; }
SI F operator>>(F x, int bits) { return x.vec >> bits; }

using Stage = void(K* k, void** program, size_t x, size_t y, size_t tail, F,F,F,F, F,F,F,F);

MAYBE_MSABI
extern "C" size_t WRAP(start_pipeline)(size_t x, size_t y, size_t limit, void** program, K* k) {
    F v{};
    auto start = (Stage*)load_and_inc(program);
    while (x + kStride <= limit) {
        start(k,program,x,y,0,    v,v,v,v, v,v,v,v);
        x += kStride;
    }
    if (size_t tail = limit - x) {
        start(k,program,x,y,tail, v,v,v,v, v,v,v,v);
    }
    return limit;
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
SI void store(T* dst, V v, size_t tail) {
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
    unaligned_store(dst, v);
}

SI void from_8888(U32 rgba, F* r, F* g, F* b, F* a) {
    // Split the 8 pixels into low and high halves, and reinterpret as vectors of 16-bit values.
    U16 lo = unaligned_load<U16>((const uint32_t*)&rgba + 0),
        hi = unaligned_load<U16>((const uint32_t*)&rgba + 4);

    // Shuffle so that the 4 bytes of each color channel are contiguous...
    lo = _mm_shuffle_epi8(lo, _mm_setr_epi8(0,4,8,12, 1,5,9,13, 2,6,10,14, 3,7,11,15));
    hi = _mm_shuffle_epi8(hi, _mm_setr_epi8(0,4,8,12, 1,5,9,13, 2,6,10,14, 3,7,11,15));

    // ...then get all 8 bytes of each color channel together into a single register.
    U16 rg = _mm_unpacklo_epi32(lo,hi),
        ba = _mm_unpackhi_epi32(lo,hi);

    // Unpack as 16-bit values into the high half of each 16-bit lane, to get a free *256.
    U16 R = _mm_unpacklo_epi8(U16(0), rg),
        G = _mm_unpackhi_epi8(U16(0), rg),
        B = _mm_unpacklo_epi8(U16(0), ba),
        A = _mm_unpackhi_epi8(U16(0), ba);

    // Now we scale from [0,255] to [0,32768].  Ideally that's 32768/255 = 128.50196,
    // but we can approximate that very cheaply as 256*32897/65536 = 128.50391.
    // 0 and 255 map to 0 and 32768 correctly, and nothing else is off by more than 1.
    *r = _mm_mulhi_epu16(R, U16(32897));
    *g = _mm_mulhi_epu16(G, U16(32897));
    *b = _mm_mulhi_epu16(B, U16(32897));
    *a = _mm_mulhi_epu16(A, U16(32897));
}
SI U32 to_8888(F r, F g, F b, F a) {
    // We want to interlace and pack these values from [0,32768] to [0,255].
    // Luckily the simplest possible thing works great: >>7, then saturate.
    // The 'u' in packus handles the saturation to [0,255] we need.
    U16 rb = _mm_packus_epi16(r>>7,b>>7), // r0 r1 r2 r3 r4 r5 r6 r7 b0 b1 b2 b3 b4 b5 b6 b7
        ga = _mm_packus_epi16(g>>7,a>>7);

    U16 rg = _mm_unpacklo_epi8(rb, ga),   // r0 g0 r1 g1 ...                           r7 g7
        ba = _mm_unpackhi_epi8(rb, ga);   // b0 a0       ...                           b7 a7

    U16 lo = _mm_unpacklo_epi16(rg, ba),  // r0 g0 b0 a0 ...                     r3 g3 b3 a3
        hi = _mm_unpackhi_epi16(rg, ba);  // r4 g4 b4 a4 ...                     r7 g7 b7 a7

    U32 px;
    memcpy((uint32_t*)&px + 0, &lo, sizeof(lo));
    memcpy((uint32_t*)&px + 4, &hi, sizeof(hi));
    return px;
}

// Stages!

STAGE(constant_color) {
    auto rgba = (const float*)ctx;
    r = rgba[0];
    g = rgba[1];
    b = rgba[2];
    a = rgba[3];
}

STAGE(load_8888) {
    auto ptr = *(const uint32_t**)ctx + x;
    from_8888(load<U32>(ptr, tail), &r,&g,&b,&a);
}
STAGE(store_8888) {
    auto ptr = *(uint32_t**)ctx + x;
    store(ptr, to_8888(r,g,b,a), tail);
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

BLEND_MODE(srcover) { return mad(d, inv(sa), s); }

#undef BLEND_MODE
