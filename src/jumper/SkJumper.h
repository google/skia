/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkJumper_DEFINED
#define SkJumper_DEFINED

#include <stddef.h>
#include <stdint.h>

// This file contains definitions shared by SkJumper.cpp (compiled normally as part of Skia)
// and SkJumper_stages.cpp (compiled into Skia _and_ offline into SkJumper_generated.h).
// Keep it simple!

// Externally facing functions (start_pipeline) are called a little specially on Windows.
#if defined(JUMPER_IS_OFFLINE) && defined(WIN) && defined(__x86_64__)
    #define MAYBE_MSABI __attribute__((ms_abi))                   // Use MS' ABI, not System V.
#elif defined(JUMPER_IS_OFFLINE) && defined(WIN) && defined(__i386__)
    #define MAYBE_MSABI __attribute__((force_align_arg_pointer))  // Re-align stack 4 -> 16 bytes.
#else
    #define MAYBE_MSABI
#endif

// Any custom ABI to use for all non-externally-facing stage functions.
#if defined(__ARM_NEON) && defined(__arm__)
    // This lets us pass vectors more efficiently on 32-bit ARM.
    #define ABI __attribute__((pcs("aapcs-vfp")))
#else
    #define ABI
#endif

// On ARM we expect that you're using Clang if you want SkJumper to be fast.
// If you are, the baseline float stages will use NEON, and lowp stages will
// also be available. (If somehow you're building for ARM not using Clang,
// you'll get scalar baseline stages and no lowp support.)
#if defined(__clang__) && defined(__ARM_NEON)
    #define JUMPER_HAS_NEON_LOWP
#endif

static const int SkJumper_kMaxStride = 16;

struct SkJumper_MemoryCtx {
    void* pixels;
    int   stride;
};

struct SkJumper_GatherCtx {
    const void* pixels;
    int         stride;
    float       width;
    float       height;
};

// State shared by save_xy, accumulate, and bilinear_* / bicubic_*.
struct SkJumper_SamplerCtx {
    float      x[SkJumper_kMaxStride];
    float      y[SkJumper_kMaxStride];
    float     fx[SkJumper_kMaxStride];
    float     fy[SkJumper_kMaxStride];
    float scalex[SkJumper_kMaxStride];
    float scaley[SkJumper_kMaxStride];
};

struct SkJumper_TileCtx {
    float scale;
    float invScale; // cache of 1/scale
};

struct SkJumper_CallbackCtx {
    MAYBE_MSABI void (*fn)(SkJumper_CallbackCtx* self, int active_pixels/*<= SkJumper_kMaxStride*/);

    // When called, fn() will have our active pixels available in rgba.
    // When fn() returns, the pipeline will read back those active pixels from read_from.
    float rgba[4*SkJumper_kMaxStride];
    float* read_from = rgba;
};

struct SkJumper_LoadTablesCtx {
    const void* src;
    const float *r, *g, *b;
};

struct SkJumper_TableCtx {
    const float* table;
    int          size;
};

// This should line up with the memory layout of SkColorSpaceTransferFn.
struct SkJumper_ParametricTransferFunction {
    float G, A,B,C,D,E,F;
};

struct SkJumper_GradientCtx {
    size_t stopCount;
    float* fs[4];
    float* bs[4];
    float* ts;
};

struct SkJumper_2PtConicalCtx {
    uint32_t fMask[SkJumper_kMaxStride];
    float    fP0,
             fP1;
};

struct SkJumper_UniformColorCtx {
    float r,g,b,a;
    uint16_t rgba[4];  // [0,255] in a 16-bit lane.
};

struct SkJumper_ColorLookupTableCtx {
    const float* table;
    int limits[4];
};

#endif//SkJumper_DEFINED
