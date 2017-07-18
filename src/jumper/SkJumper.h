/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkJumper_DEFINED
#define SkJumper_DEFINED

// This file contains definitions shared by SkJumper.cpp (compiled normally as part of Skia)
// and SkJumper_stages.cpp (compiled into Skia _and_ offline into SkJumper_generated.h).
// Keep it simple!

// Externally facing functions (start_pipeline) are called a little specially on Windows.
#if defined(JUMPER) && defined(WIN) && defined(__x86_64__)
    #define MAYBE_MSABI __attribute__((ms_abi))                   // Use MS' ABI, not System V.
#elif defined(JUMPER) && defined(WIN) && defined(__i386__)
    #define MAYBE_MSABI __attribute__((force_align_arg_pointer))  // Re-align stack 4 -> 16 bytes.
#else
    #define MAYBE_MSABI
#endif

#if defined(JUMPER) && (defined(__aarch64__) || defined(__arm__))
    // To reduce SkJumper's dependency on the Android NDK,
    // we provide what we need from <string.h>, <stdint.h>, and <stddef.h> ourselves.
    #define memcpy __builtin_memcpy

    using  int8_t  =   signed char;
    using uint8_t  = unsigned char;
    using  int16_t =   signed short;
    using uint16_t = unsigned short;
    using  int32_t =   signed int;
    using uint32_t = unsigned int;
    #if defined(__aarch64__)
        using  int64_t =   signed long;
        using uint64_t = unsigned long;
        using size_t = uint64_t;
    #else
        using  int64_t =   signed long long;
        using uint64_t = unsigned long long;
        using size_t = uint32_t;
    #endif

    // Now pretend we've included <stdint.h> (or it'll be included again by <arm_neon.h>).
    #define __CLANG_STDINT_H
    #define _STDINT_H_
#else
    #include <string.h>
    #include <stdint.h>
#endif

static const int SkJumper_kMaxStride = 8;

struct SkJumper_constants {
    float    iota_F  [SkJumper_kMaxStride];   //  0,1,2,3,4,...
    uint32_t iota_U32[SkJumper_kMaxStride];   //  0,1,2,3,4,...
};

struct SkJumper_MemoryCtx {
    void* pixels;
    int   stride;
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
    float    fCoeffA,
             fInvCoeffA,
             fR0,
             fDR;
};

#endif//SkJumper_DEFINED
