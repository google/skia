/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkOpts_DEFINED
#define SkOpts_DEFINED

#include "SkRasterPipeline.h"
#include "SkTypes.h"
#include "SkXfermodePriv.h"

struct ProcCoeff;

namespace SkOpts {
    // Call to replace pointers to portable functions with pointers to CPU-specific functions.
    // Thread-safe and idempotent.
    // Called by SkGraphics::Init().
    void Init();

    // Declare function pointers here...

    // May return nullptr if we haven't specialized the given Mode.
    extern SkXfermode* (*create_xfermode)(SkBlendMode);

    typedef void (*Morph)(const SkPMColor*, SkPMColor*, int, int, int, int, int);
    extern Morph dilate_x, dilate_y, erode_x, erode_y;

    extern void (*blit_mask_d32_a8)(SkPMColor*, size_t, const SkAlpha*, size_t, SkColor, int, int);
    extern void (*blit_row_color32)(SkPMColor*, const SkPMColor*, int, SkPMColor);
    extern void (*blit_row_s32a_opaque)(SkPMColor*, const SkPMColor*, int, U8CPU);

    // Swizzle input into some sort of 8888 pixel, {premul,unpremul} x {rgba,bgra}.
    typedef void (*Swizzle_8888)(uint32_t*, const void*, int);
    extern Swizzle_8888 RGBA_to_BGRA,          // i.e. just swap RB
                        RGBA_to_rgbA,          // i.e. just premultiply
                        RGBA_to_bgrA,          // i.e. swap RB and premultiply
                        RGB_to_RGB1,           // i.e. insert an opaque alpha
                        RGB_to_BGR1,           // i.e. swap RB and insert an opaque alpha
                        gray_to_RGB1,          // i.e. expand to color channels + an opaque alpha
                        grayA_to_RGBA,         // i.e. expand to color channels
                        grayA_to_rgbA,         // i.e. expand to color channels and premultiply
                        inverted_CMYK_to_RGB1, // i.e. convert color space
                        inverted_CMYK_to_BGR1; // i.e. convert color space

    extern void (*memset16)(uint16_t[], uint16_t, int);
    extern void SK_API (*memset32)(uint32_t[], uint32_t, int);
    extern void (*memset64)(uint64_t[], uint64_t, int);

    // The fastest high quality 32-bit hash we can provide on this platform.
    extern uint32_t (*hash_fn)(const void*, size_t, uint32_t seed);
    static inline uint32_t hash(const void* data, size_t bytes, uint32_t seed=0) {
        return hash_fn(data, bytes, seed);
    }
}

#endif//SkOpts_DEFINED
