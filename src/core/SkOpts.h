/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkOpts_DEFINED
#define SkOpts_DEFINED

#include "include/core/SkTypes.h"
#include "src/core/SkRasterPipeline.h"
#include "src/core/SkXfermodePriv.h"

struct SkBitmapProcState;

namespace SkOpts {
    // Call to replace pointers to portable functions with pointers to CPU-specific functions.
    // Thread-safe and idempotent.
    // Called by SkGraphics::Init().
    void Init();

    // Declare function pointers here...

    // May return nullptr if we haven't specialized the given Mode.
    extern SkXfermode* (*create_xfermode)(SkBlendMode);

    extern void (*blit_mask_d32_a8)(SkPMColor*, size_t, const SkAlpha*, size_t, SkColor, int, int);
    extern void (*blit_row_color32)(SkPMColor*, const SkPMColor*, int, SkPMColor);
    extern void (*blit_row_s32a_opaque)(SkPMColor*, const SkPMColor*, int, U8CPU);

    // Swizzle input into some sort of 8888 pixel, {premul,unpremul} x {rgba,bgra}.
    typedef void (*Swizzle_8888_u32)(uint32_t*, const uint32_t*, int);
    extern Swizzle_8888_u32 RGBA_to_BGRA,          // i.e. just swap RB
                            RGBA_to_rgbA,          // i.e. just premultiply
                            RGBA_to_bgrA,          // i.e. swap RB and premultiply
                            inverted_CMYK_to_RGB1, // i.e. convert color space
                            inverted_CMYK_to_BGR1; // i.e. convert color space

    typedef void (*Swizzle_8888_u8)(uint32_t*, const uint8_t*, int);
    extern Swizzle_8888_u8 RGB_to_RGB1,     // i.e. insert an opaque alpha
                           RGB_to_BGR1,     // i.e. swap RB and insert an opaque alpha
                           gray_to_RGB1,    // i.e. expand to color channels + an opaque alpha
                           grayA_to_RGBA,   // i.e. expand to color channels
                           grayA_to_rgbA;   // i.e. expand to color channels and premultiply

    extern void (*memset16)(uint16_t[], uint16_t, int);
    extern void SK_API (*memset32)(uint32_t[], uint32_t, int);
    extern void (*memset64)(uint64_t[], uint64_t, int);

    extern void (*rect_memset16)(uint16_t[], uint16_t, int, size_t, int);
    extern void (*rect_memset32)(uint32_t[], uint32_t, int, size_t, int);
    extern void (*rect_memset64)(uint64_t[], uint64_t, int, size_t, int);

    // The fastest high quality 32-bit hash we can provide on this platform.
    extern uint32_t (*hash_fn)(const void*, size_t, uint32_t seed);
    static inline uint32_t hash(const void* data, size_t bytes, uint32_t seed=0) {
        return hash_fn(data, bytes, seed);
    }

    // SkBitmapProcState optimized Shader, Sample, or Matrix procs.
    // This is the only one that can use anything past SSE2/NEON.
    extern void (*S32_alpha_D32_filter_DX)(const SkBitmapProcState&,
                                           const uint32_t* xy, int count, SkPMColor*);

#define M(st) +1
    // We can't necessarily express the type of SkJumper stage functions here,
    // so we just use this void(*)(void) as a stand-in.
    using StageFn = void(*)(void);
    extern StageFn stages_highp[SK_RASTER_PIPELINE_STAGES(M)], just_return_highp;
    extern StageFn stages_lowp [SK_RASTER_PIPELINE_STAGES(M)], just_return_lowp;

    extern void (*start_pipeline_highp)(size_t,size_t,size_t,size_t, void**);
    extern void (*start_pipeline_lowp )(size_t,size_t,size_t,size_t, void**);
#undef M
}

#endif//SkOpts_DEFINED
