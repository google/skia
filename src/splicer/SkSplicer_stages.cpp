/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkSplicer_shared.h"
#include <immintrin.h>
#include <string.h>

#if !defined(__clang__) || !defined(__AVX2__) || !defined(__FMA__) || !defined(__F16C__)
    #error This file is not like the rest of Skia.
    #error It must be compiled with clang with -mavx2 -mfma -mf16c.
#endif

// We have very specific inlining requirements.  It helps to just use always_inline.
#define AI __attribute__((always_inline)) inline

// We'll be compiling this file to an object file, then extracting parts of it into
// SkSplicer_stages.h.  It's easier to do if the function names are not C++ mangled.
#define C extern "C"

// Since we know we're using Clang, we can use its vector extensions.
// These are __m256 and __m256i, but friendlier and strongly-typed.
using F   = float    __attribute__((ext_vector_type(8)));
using U32 = uint32_t __attribute__((ext_vector_type(8)));

// We need to polyfill a few routines Clang doesn't build in to ext_vector_types.
AI static F   cast(U32 v) { return _mm256_cvtepi32_ps(v); }
AI static U32 cast(F   v) { return _mm256_cvtps_epi32(v); }
AI static F min(F a, F b) { return _mm256_min_ps(a,b); }
AI static F max(F a, F b) { return _mm256_max_ps(a,b); }
AI static F fma(F f, F m, F a) { return _mm256_fmadd_ps(f,m,a); }


// Stages all fit a common interface that allows SkSplicer to splice them together.
using K = SkSplicer_constants;
using Stage = void(size_t x, size_t unused, void* ctx, K* constants, F,F,F,F, F,F,F,F);

// Stage's arguments act as the working set of registers within the final spliced function.
// Here's a litte primer on the ABI:
//   x:         rdi         TODO: explain
//   unused:    rsi         Unused for now, but a handy register because of lodsq.
//   ctx:       rdx         Look for movabsq_rdx in SkSplicer to see how this works.
//   constants: rcx         TODO: explain
//   vectors:   ymm0-ymm7


// done() is the keystone to this entire splicing approach.
// It has the same signature as Stage, but simply consumes its arguments, keeping them live.
// Every stage should end in a simple jmp (i.e. tail-call) into done() (hence, noinline).
// That's how we guarantee each stage is spliceable.

__attribute__((noinline))
C void done(size_t x, size_t unused, void* ctx, K* k,
            F r, F g, F b, F a, F dr, F dg, F db, F da) {
    asm("" :: "r"(x), "r"(unused), "r"(ctx), "r"(k),
              "x"(r), "x"(g), "x"(b), "x"(a), "x"(dr), "x"(dg), "x"(db), "x"(da));
}

// This should feel familiar to anyone who's read SkRasterPipeline_opts.h.
// It's just a convenience to make a valid, spliceable Stage, nothing magic.
#define STAGE(name)                                                              \
    AI static void name##_k(size_t x, size_t unused, void* ctx, K* k,            \
                            F& r, F& g, F& b, F& a, F& dr, F& dg, F& db, F& da); \
    C void name(size_t x, size_t unused, void* ctx, K* k,                        \
                F r, F g, F b, F a, F dr, F dg, F db, F da) {                    \
        name##_k(x,unused,ctx,k, r,g,b,a, dr,dg,db,da);                          \
        done    (x,unused,ctx,k, r,g,b,a, dr,dg,db,da);                          \
    }                                                                            \
    AI static void name##_k(size_t x, size_t unused, void* ctx, K* k,            \
                            F& r, F& g, F& b, F& a, F& dr, F& dg, F& db, F& da)

// We can now define Stages!

// Some things to keep in mind while writing Stages:
//   - do not branch;                                       (i.e. avoid jmp)
//   - do not call functions that don't inline;             (i.e. avoid call, ret, stack use)
//   - do not use constant literals other than 0 and 0.0f.  (i.e. avoid rip relative addressing)
//
// Some things that should work fine:
//   - 0 and 0.0f;
//   - arithmetic;
//   - functions on F and U32 defined above;
//   - temporary values;
//   - lambdas;
//   - memcpy() with a constant size argument.

STAGE(clear) {
    r = 0;
    g = 0;
    b = 0;
    a = 0;
}

STAGE(plus) {
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
    a = fma(db, A, a);
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
    auto swap = [](F& f, F& df) {
        auto tmp = f;
        f = df;
        df = tmp;
    };
    swap(r, dr);
    swap(g, dg);
    swap(b, db);
    swap(a, da);
}

STAGE(load_8888) {
    auto ptr = *(const uint32_t**)ctx + x;

    U32 px;
    memcpy(&px, ptr, sizeof(px));

    r = cast((px      ) & k->_0x000000ff) * k->_1_255;
    g = cast((px >>  8) & k->_0x000000ff) * k->_1_255;
    b = cast((px >> 16) & k->_0x000000ff) * k->_1_255;
    a = cast((px >> 24)                 ) * k->_1_255;
}

STAGE(store_8888) {
    auto ptr = *(uint32_t**)ctx + x;

    U32 px = cast(r * k->_255)
           | cast(g * k->_255) <<  8
           | cast(b * k->_255) << 16
           | cast(a * k->_255) << 24;
    memcpy(ptr, &px, sizeof(px));
}

