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

using U32  = uint32_t __attribute__((ext_vector_type( 4)));
using U8x4 = uint8_t  __attribute__((ext_vector_type(16)));

union V {
    U32  u32;
    U8x4 u8x4;
};
static const size_t kStride = sizeof(V) / sizeof(uint32_t);

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
