/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkJumper.h"
#include "SkJumper_misc.h"

// We're going to try going even lower precision than _lowp.cpp,
// 8-bit per channel, and while we're at it keep our pixels interlaced.
// This is the natural format for kN32_SkColorType buffers, and we hope
// the stages in this file can replace many custom legacy routines.

// To start we'll keep things simple and target SSE2 only.
// TODO: SSE4.1, AVX2, NEON
// (Most legacy routines cap out at SSE4.1, so AVX2 should be a nice win.)

#if !defined(JUMPER)
    #error "This file must be pre-compiled."
#elif defined(__SSE2__)
    #define WRAP(name) sk_##name##_sse2_8bit
#endif

using U32   = uint32_t __attribute__((ext_vector_type( 4)));
using U8x4  = uint8_t  __attribute__((ext_vector_type(16)));
using U16x4 = uint16_t __attribute__((ext_vector_type(16)));

union V {
    U32  u32;
    U8x4 u8x4;

    V() = default;
    V(U32  v) : u32 (v) {}
    V(U8x4 v) : u8x4(v) {}
    V(int  v) : u8x4(v) {}
};
static const size_t kStride = sizeof(V) / sizeof(uint32_t);

SI V alpha(V v) {
    return __builtin_shufflevector(v.u8x4,v.u8x4, 3,3,3,3, 7,7,7,7, 11,11,11,11, 15,15,15,15);
}

SI V operator+(V x, V y) { return x.u8x4 + y.u8x4; }
SI V operator-(V x, V y) { return x.u8x4 - y.u8x4; }
SI V operator*(V x, V y) {
    // (x*y + x)/256 is a very good approximation of (x*y + 127)/255.
    U16x4 X = __builtin_convertvector(x.u8x4, U16x4),
          Y = __builtin_convertvector(y.u8x4, U16x4);
    return __builtin_convertvector((X*Y + X)>>8, U8x4);
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

STAGE(clear)       { src.u32 = 0x00000000; }
STAGE(black_color) { src.u32 = 0xff000000; }
STAGE(white_color) { src.u32 = 0xffffffff; }

STAGE(srcover) { src = src + (0xff - alpha(src))*dst; }

STAGE(uniform_color) {
    auto c = (const float*)ctx;

    src.u32 = (uint32_t)(c[0] * 255) << 0
            | (uint32_t)(c[1] * 255) << 8
            | (uint32_t)(c[2] * 255) << 16
            | (uint32_t)(c[3] * 255) << 24;
}

#if 0
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
#endif
