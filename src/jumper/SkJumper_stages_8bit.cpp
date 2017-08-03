/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkJumper.h"
#include "SkJumper_misc.h"

#if defined(__SSE2__)
    #include <immintrin.h>
#endif

// We're going to try going even lower precision than _lowp.cpp,
// 8-bit per channel, and while we're at it keep our pixels interlaced.
// This is the natural format for kN32_SkColorType buffers, and we hope
// the stages in this file can replace many custom legacy routines.

#if !defined(JUMPER)
    #error "This file must be pre-compiled."
#elif defined(__aarch64__)
    #define WRAP(name) sk_##name##_aarch64_8bit
#elif defined(__arm__)
    #define WRAP(name) sk_##name##_vfp4_8bit
#elif defined(__AVX2__)
    #define WRAP(name) sk_##name##_hsw_8bit
#elif defined(__SSE4_1__)
    #define WRAP(name) sk_##name##_sse41_8bit
#elif defined(__SSE2__)
    #define WRAP(name) sk_##name##_sse2_8bit
#endif

#if defined(__AVX2__)
    using U8    = uint8_t  __attribute__((ext_vector_type( 8)));
    using U32   = uint32_t __attribute__((ext_vector_type( 8)));
    using U8x4  = uint8_t  __attribute__((ext_vector_type(32)));
    using U16x4 = uint16_t __attribute__((ext_vector_type(32)));
#else
    using U8    = uint8_t  __attribute__((ext_vector_type( 4)));
    using U32   = uint32_t __attribute__((ext_vector_type( 4)));
    using U8x4  = uint8_t  __attribute__((ext_vector_type(16)));
    using U16x4 = uint16_t __attribute__((ext_vector_type(16)));
#endif

union V {
    U32  u32;
    U8x4 u8x4;

    V() = default;
    V(U32   v) : u32 (v) {}
    V(U8x4  v) : u8x4(v) {}
    V(int   v) : u8x4(v) {}
    V(float v) : u8x4(v*255) {}
};
static const size_t kStride = sizeof(V) / sizeof(uint32_t);

// Usually __builtin_convertvector() is pretty good, but sometimes we can do better.
SI U8x4 pack(U16x4 v) {
#if defined(__AVX2__)
    static_assert(sizeof(v) == 64, "");
    auto lo = unaligned_load<__m256i>((char*)&v +  0),
         hi = unaligned_load<__m256i>((char*)&v + 32);

    auto _02 = _mm256_permute2x128_si256(lo,hi, 0x20),
         _13 = _mm256_permute2x128_si256(lo,hi, 0x31);
    return _mm256_packus_epi16(_02, _13);
#elif defined(__SSE2__)
    static_assert(sizeof(v) == 32, "");
    auto lo = unaligned_load<__m128i>((char*)&v +  0),
         hi = unaligned_load<__m128i>((char*)&v + 16);
    return _mm_packus_epi16(lo,hi);
#else
    return __builtin_convertvector(v, U8x4);
#endif
}

SI V operator+(V x, V y) { return x.u8x4 + y.u8x4; }
SI V operator-(V x, V y) { return x.u8x4 - y.u8x4; }
SI V operator*(V x, V y) {
    // (x*y + x)/256 is a very good approximation of (x*y + 127)/255.
    U16x4 X = __builtin_convertvector(x.u8x4, U16x4),
          Y = __builtin_convertvector(y.u8x4, U16x4);
    return pack((X*Y + X)>>8);
}

SI V inv(V v) { return 0xff - v; }
SI V lerp(V from, V to, V t) { return to*t + from*inv(t); }

SI V alpha(V v) {
#if defined(__AVX2__)
    return __builtin_shufflevector(v.u8x4,v.u8x4,
                                    3, 3, 3, 3,  7, 7, 7, 7, 11,11,11,11, 15,15,15,15,
                                   19,19,19,19, 23,23,23,23, 27,27,27,27, 31,31,31,31);
#else
    return __builtin_shufflevector(v.u8x4,v.u8x4, 3,3,3,3, 7,7,7,7, 11,11,11,11, 15,15,15,15);
#endif
}

SI V swap_rb(V v) {
#if defined(__AVX2__)
    return __builtin_shufflevector(v.u8x4,v.u8x4,
                                    2, 1, 0, 3,  6, 5, 4, 7, 10, 9, 8,11, 14,13,12,15,
                                   18,17,16,19, 22,21,20,23, 26,25,24,27, 30,29,28,31);
#else
    return __builtin_shufflevector(v.u8x4,v.u8x4, 2,1,0,3, 6,5,4,7, 10,9,8,11, 14,13,12,15);
#endif
}

struct Params {
    size_t x,y,tail;
};

using Stage = void(const Params* params, void** program, V src, V dst);

#if defined(__AVX__)
    // We really want to make sure all paths go through this function's (implicit) vzeroupper.
    // If they don't, we'll experience severe slowdowns when we first use SSE instructions again.
    __attribute__((disable_tail_calls))
#endif
MAYBE_MSABI
extern "C" void WRAP(start_pipeline)(size_t x, size_t y, size_t xlimit, size_t ylimit,
                                     void** program, const SkJumper_constants*) {
    V v;
    auto start = (Stage*)load_and_inc(program);
    for (; y < ylimit; y++) {
        Params params = { x,y,0 };
        while (params.x + kStride <= xlimit) {
            start(&params,program, v,v);
            params.x += kStride;
        }
        if (size_t tail = xlimit - params.x) {
            params.tail = tail;
            start(&params,program, v,v);
        }
    }
}

extern "C" void WRAP(just_return)(const Params*, void**, V,V) {}

#define STAGE(name)                                                                   \
    SI void name##_k(LazyCtx ctx, size_t x, size_t y, size_t tail, V& src, V& dst);   \
    extern "C" void WRAP(name)(const Params* params, void** program, V src, V dst) {  \
        LazyCtx ctx(program);                                                         \
        name##_k(ctx, params->x, params->y, params->tail, src, dst);                  \
        auto next = (Stage*)load_and_inc(program);                                    \
        next(params,program, src,dst);                                                \
    }                                                                                 \
    SI void name##_k(LazyCtx ctx, size_t x, size_t y, size_t tail, V& src, V& dst)

template <typename V, typename T>
SI V load(const T* src, size_t tail) {
    __builtin_assume(tail < kStride);
    if (__builtin_expect(tail, 0)) {
        V v = 0;
        switch (tail) {
            case 7: v[6] = src[6];
            case 6: v[5] = src[5];
            case 5: v[4] = src[4];
            case 4: memcpy(&v, src, 4*sizeof(T)); break;
            case 3: v[2] = src[2];
            case 2: memcpy(&v, src, 2*sizeof(T)); break;
            case 1: memcpy(&v, src, 1*sizeof(T)); break;
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
            case 7: dst[6] = v[6];
            case 6: dst[5] = v[5];
            case 5: dst[4] = v[4];
            case 4: memcpy(dst, &v, 4*sizeof(T)); break;
            case 3: dst[2] = v[2];
            case 2: memcpy(dst, &v, 2*sizeof(T)); break;
            case 1: memcpy(dst, &v, 1*sizeof(T)); break;
        }
        return;
    }
    unaligned_store(dst, v);
}

#if 1 && defined(__AVX2__)
    SI U32 mask(size_t tail) {
        // We go a little out of our way to avoid needing large constant values here.

        // It's easiest to build the mask as 8 8-bit values, either 0x00 or 0xff.
        // Start fully on, then shift away lanes from the top until we've got our mask.
        uint64_t mask = 0xffffffffffffffff >> 8*(kStride-tail);

        // Sign-extend each mask lane to its full width, 0x00000000 or 0xffffffff.
        using S8  = int8_t  __attribute__((ext_vector_type(8)));
        using S32 = int32_t __attribute__((ext_vector_type(8)));
        return (U32)__builtin_convertvector(unaligned_load<S8>(&mask), S32);
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
        unaligned_store(dst, v);
    }
#endif

// Used by load_ and store_ stages to get to the right (x,y) starting point of contiguous memory.
template <typename T>
SI T* ptr_at_xy(const SkJumper_MemoryCtx* ctx, int x, int y) {
    return (T*)ctx->pixels + y*ctx->stride + x;
}

STAGE(uniform_color) {
    auto c = (const SkJumper_UniformColorCtx*)ctx;
    src.u32 = c->rgba;
}

STAGE(set_rgb) {
    auto c = (const float*)ctx;

    src.u32 = (uint32_t)(c[0] * 255) << 0
            | (uint32_t)(c[1] * 255) << 8
            | (uint32_t)(c[2] * 255) << 16
            | (src.u32 & 0xff000000);
}

STAGE(premul) {
    // I.e. rgb *= a, a *= 1.0f.
    src = src * (alpha(src).u32 | 0xff000000);
}
STAGE(swap_rb) {
    src = swap_rb(src);
}

STAGE(load_8888) {
    auto ptr = ptr_at_xy<const uint32_t>(ctx, x,y);
    src = load<U32>(ptr, tail);
}
STAGE(load_8888_dst) {
    auto ptr = ptr_at_xy<const uint32_t>(ctx, x,y);
    dst = load<U32>(ptr, tail);
}
STAGE(store_8888) {
    auto ptr = ptr_at_xy<uint32_t>(ctx, x,y);
    store(ptr, src.u32, tail);
}

STAGE(load_bgra) {
    auto ptr = ptr_at_xy<const uint32_t>(ctx, x,y);
    src = swap_rb(load<U32>(ptr, tail));
}
STAGE(load_bgra_dst) {
    auto ptr = ptr_at_xy<const uint32_t>(ctx, x,y);
    dst = swap_rb(load<U32>(ptr, tail));
}
STAGE(store_bgra) {
    auto ptr = ptr_at_xy<uint32_t>(ctx, x,y);
    store(ptr, swap_rb(src).u32, tail);
}

STAGE(load_a8) {
    auto ptr = ptr_at_xy<const uint8_t>(ctx, x,y);
    src = __builtin_convertvector(load<U8>(ptr, tail), U32) << 24;
}
STAGE(load_a8_dst) {
    auto ptr = ptr_at_xy<const uint8_t>(ctx, x,y);
    dst = __builtin_convertvector(load<U8>(ptr, tail), U32) << 24;
}
STAGE(store_a8) {
    auto ptr = ptr_at_xy<uint8_t>(ctx, x,y);
    store(ptr, __builtin_convertvector(src.u32 >> 24, U8), tail);
}

STAGE(load_g8) {
    auto ptr = ptr_at_xy<const uint8_t>(ctx, x,y);
    src = (__builtin_convertvector(load<U8>(ptr, tail), U32) * 0x010101) | 0xff000000;
}
STAGE(load_g8_dst) {
    auto ptr = ptr_at_xy<const uint8_t>(ctx, x,y);
    dst = (__builtin_convertvector(load<U8>(ptr, tail), U32) * 0x010101) | 0xff000000;
}

STAGE(srcover_rgba_8888) {
    auto ptr = ptr_at_xy<uint32_t>(ctx, x,y);

    V d = load<U32>(ptr, tail);
    V b = src + (d - d*alpha(src));

    store(ptr, b.u32, tail);
}

STAGE(scale_1_float) {
    float c = *(const float*)ctx;
    src = src * c;
}
STAGE(scale_u8) {
    auto ptr = ptr_at_xy<const uint8_t>(ctx, x,y);

    V c = __builtin_convertvector(load<U8>(ptr, tail), U32) << 24;
    src = src * alpha(c);
}

STAGE(lerp_1_float) {
    float c = *(const float*)ctx;
    src = lerp(dst, src, c);
}
STAGE(lerp_u8) {
    auto ptr = ptr_at_xy<const uint8_t>(ctx, x,y);

    V c = __builtin_convertvector(load<U8>(ptr, tail), U32) << 24;
    src = lerp(dst, src, alpha(c));
}

STAGE(move_src_dst) { dst = src; }
STAGE(move_dst_src) { src = dst; }

STAGE(black_color) { src.u32 = 0xff000000; }
STAGE(white_color) { src.u32 = 0xffffffff; }
STAGE(clear)       { src.u32 = 0x00000000; }

STAGE(srcatop)  { src = src*alpha(dst) + dst*inv(alpha(src)); }
STAGE(dstatop)  { src = dst*alpha(src) + src*inv(alpha(dst)); }
STAGE(srcin)    { src = src * alpha(dst); }
STAGE(dstin)    { src = dst * alpha(src); }
STAGE(srcout)   { src = src * inv(alpha(dst)); }
STAGE(dstout)   { src = dst * inv(alpha(src)); }
STAGE(srcover)  { src = src + (dst - dst*alpha(src)); }
STAGE(dstover)  { src = dst + (src - src*alpha(dst)); }
STAGE(modulate) { src = src*dst; }
STAGE(multiply) { src = src*inv(alpha(dst)) + dst*inv(alpha(src)) + src*dst; }
STAGE(screen)   { src = src + inv(src)*dst; }
STAGE(xor_)     { src = src*inv(alpha(dst)) + dst*inv(alpha(src)); }
