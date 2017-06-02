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

#define WRAP(name) sk_lowp_##name##_ssse3

using K = const SkJumper_constants;
static const size_t kStride = 8;

template <typename T> using V = T __attribute__((ext_vector_type(8)));
using U8  = V<uint8_t>;
using U16 = V<uint16_t>;

// See SkFixed15.h for details on this format and its operations.
struct F {
    U16 vec;

    F() = default;
    F(uint16_t bits) : vec(bits) {}

    F(U16 v) : vec(v) {}
    operator U16() const { return vec; }
};

SI F operator+(F x, F y) { return x.vec + y.vec; }
SI F operator-(F x, F y) { return x.vec - y.vec; }
SI F operator*(F x, F y) { return _mm_abs_epi16(_mm_mulhrs_epi16(x.vec, y.vec)); }
SI F mad(F f, F m, F a) { return f*m+a; }

SI F operator<<(F x, int bits) { return x.vec << bits; }
SI F operator>>(F x, int bits) { return x.vec >> bits; }

using Stage = void(K* k, void** program, size_t x, size_t y, size_t tail, F,F,F,F, F,F,F,F);

MAYBE_MSABI
extern "C" void WRAP(start_pipeline)(size_t x, size_t y, size_t limit, void** program, K* k) {
    F v{};
    auto start = (Stage*)load_and_inc(program);
    while (x + kStride <= limit) {
        start(k,program,x,y,0,    v,v,v,v, v,v,v,v);
        x += kStride;
    }
    if (size_t tail = limit - x) {
        start(k,program,x,y,tail, v,v,v,v, v,v,v,v);
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

// Stages!

STAGE(swap_rb) {
    auto tmp = r;
    r = b;
    b = tmp;
}
