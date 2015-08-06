/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkOpts_DEFINED
#define SkOpts_DEFINED

#include "SkTextureCompressor.h"
#include "SkTypes.h"
#include "SkXfermode.h"

struct ProcCoeff;

namespace SkOpts {
    // Call to replace pointers to portable functions with pointers to CPU-specific functions.
    // Thread-safe and idempotent.
    // Called by SkGraphics::Init(), and automatically #if SK_ALLOW_STATIC_GLOBAL_INITIALIZERS.
    void Init();

    // Declare function pointers here...

    // Returns a fast approximation of 1.0f/sqrtf(x).
    extern float (*rsqrt)(float);

    // See SkUtils.h
    extern void (*memset16)(uint16_t[], uint16_t, int);
    extern void (*memset32)(uint32_t[], uint32_t, int);

    // May return nullptr if we haven't specialized the given Mode.
    extern SkXfermode* (*create_xfermode)(const ProcCoeff&, SkXfermode::Mode);

    typedef void (*BoxBlur)(const SkPMColor*, int, SkPMColor*, int, int, int, int, int);
    extern BoxBlur box_blur_xx, box_blur_xy, box_blur_yx;

    typedef void (*Morph)(const SkPMColor*, SkPMColor*, int, int, int, int, int);
    extern Morph dilate_x, dilate_y, erode_x, erode_y;

    typedef bool (*TextureCompressor)(uint8_t* dst, const uint8_t* src,
                                      int width, int height, size_t rowBytes);
    extern TextureCompressor (*texture_compressor)(SkColorType, SkTextureCompressor::Format);
    extern bool (*fill_block_dimensions)(SkTextureCompressor::Format, int* x, int* y);

}

#endif//SkOpts_DEFINED
