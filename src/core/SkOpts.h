/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkOpts_DEFINED
#define SkOpts_DEFINED

#include "include/core/SkTypes.h"
#include "include/private/SkOpts_spi.h"
#include "src/core/SkRasterPipeline.h"
#include "src/core/SkXfermodePriv.h"

struct SkBitmapProcState;
namespace skvm { struct InterpreterInstruction; }

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
    extern void SK_SPI(*memset32)(uint32_t[], uint32_t, int);
    extern void (*memset64)(uint64_t[], uint64_t, int);

    extern void (*rect_memset16)(uint16_t[], uint16_t, int, size_t, int);
    extern void (*rect_memset32)(uint32_t[], uint32_t, int, size_t, int);
    extern void (*rect_memset64)(uint64_t[], uint64_t, int, size_t, int);

    extern float (*cubic_solver)(float, float, float, float);

    static inline uint32_t hash(const void* data, size_t bytes, uint32_t seed=0) {
        return hash_fn(data, bytes, seed);
    }

    // SkBitmapProcState optimized Shader, Sample, or Matrix procs.
    extern void (*S32_alpha_D32_filter_DX)(const SkBitmapProcState&,
                                           const uint32_t* xy, int count, SkPMColor*);
    extern void (*S32_alpha_D32_filter_DXDY)(const SkBitmapProcState&,
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

    extern void (*interpret_skvm)(const skvm::InterpreterInstruction insts[], int ninsts,
                                  int nregs, int loop, const int strides[], int nargs,
                                  int n, void* args[]);
}  // namespace SkOpts

/** Similar to memset(), but it assigns a 16, 32, or 64-bit value into the buffer.
    @param buffer   The memory to have value copied into it
    @param value    The value to be copied into buffer
    @param count    The number of times value should be copied into the buffer.
*/
static inline void sk_memset16(uint16_t buffer[], uint16_t value, int count) {
    SkOpts::memset16(buffer, value, count);
}
static inline void sk_memset32(uint32_t buffer[], uint32_t value, int count) {
    SkOpts::memset32(buffer, value, count);
}
static inline void sk_memset64(uint64_t buffer[], uint64_t value, int count) {
    SkOpts::memset64(buffer, value, count);
}

#endif//SkOpts_DEFINED
