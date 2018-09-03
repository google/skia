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

// This file contains definitions shared by SkJumper.cpp/SkJumper_stages.cpp
// and the rest of Skia.  It is important to keep the interface to SkJumper
// limited and simple to avoid serious ODR violation pitfalls, especially when
// using Microsoft's <math.h> and similar headers with inline-but-not-static
// function definitions.

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

struct SkJumper_DecalTileCtx {
    uint32_t mask[SkJumper_kMaxStride];
    float    limit_x;
    float    limit_y;
};

struct SkJumper_CallbackCtx {
    void (*fn)(SkJumper_CallbackCtx* self, int active_pixels/*<= SkJumper_kMaxStride*/);

    // When called, fn() will have our active pixels available in rgba.
    // When fn() returns, the pipeline will read back those active pixels from read_from.
    float rgba[4*SkJumper_kMaxStride];
    float* read_from = rgba;
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

#endif//SkJumper_DEFINED
