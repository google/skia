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
#include "src/core/SkRasterPipelineOpList.h"
#include "src/core/SkXfermodePriv.h"

/**
 * SkOpts (short for SkOptimizations) is a mechanism where we can ship with multiple implementations
 * of a set of functions and dynamically choose the best one at runtime (e.g. the call to
 * SkGraphics::Init(), which calls SkOpts::Init()) depending on the detected CPU features. This is
 * also referred to as having "specializations" of a given function.
 *
 * For example, Skia might be compiled to support CPUs that only have the sse2 instruction set
 * (https://en.wikipedia.org/wiki/X86_instruction_listings#SSE2_instructions)
 * but may be run on a more modern CPU that supports sse42 instructions.
 * (https://en.wikipedia.org/wiki/SSE4)
 * SkOpts allow Skia to have two versions of a CRC32 checksum function, one that uses normal C++
 * code (e.g. loops, bit operations, table lookups) and one that makes use of the _mm_crc32_u64
 * intrinsic function which uses the SSE4.2 crc32 machine instruction under the hood. This hash
 * function is declared here in the SkOpts namespace, and then the implementation (see SkOpts.cpp)
 * is deferred to a function of the same name in the sse2:: namespace (the minimum Skia is compiled
 * with) using DEFINE_DEFAULT.
 *
 * All implementations of this hash function are done in a header file file in //src/opts
 * (e.g. //src/opts/SkChecksum_opts.h). ifdefs guard each of the implementations, such that only
 * one implementation is possible for a given SK_CPU_SSE_LEVEL. This header will be compiled
 * *multiple* times with a different SK_CPU_SSE_LEVEL each compilation.
 *
 * Each CPU instruction set that we want specializations for has a .cpp file in //src/opts which
 * defines an Init() function that replaces the function pointers in the SkOpts namespace with the
 * ones from the specialized namespace (e.g. sse42::). These .cpp files don't implement the
 * specializations, they just refer to the specialization created in the header files (e.g.
 * SkChecksum_opts.h).
 *
 * At compile time:
 *   - SkOpts.cpp is compiled with the minimum CPU level (e.g. SSE2). Because this
 *     file includes all the headers in //src/opts/, those headers add "the default implementation"
 *     of all their functions to the SK_OPTS_NS namespace (e.g. sse2::hash_fn).
 *   - Each of the specialized .cpp files in //src/opts/ are compiled with their respective
 *     compiler flags. Because the specialized .cpp file includes the headers that implement the
 *     functions using intrinsics or other CPU-specific code, those specialized functions end up
 *     in the specialized namespace, e.g. (sse42::hash_fn).
 *
 * At link time, the default implementations and all specializations of all SkOpts functions are
 * included in the resulting library/binary file.
 *
 * At runtime, SkOpts::Init() will run the appropriate Init functions that the current CPU level
 * supports specializations for (e.g. Init_sse42, Init_ssse3). Note multiple Init functions can
 * be called as CPU instruction sets are typically super sets of older instruction sets
 */

struct SkBitmapProcState;
struct SkRasterPipelineStage;
namespace skvm {
struct InterpreterInstruction;
}
namespace SkSL {
class TraceHook;
}

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
    extern void (*memset32)(uint32_t[], uint32_t, int);
    extern void (*memset64)(uint64_t[], uint64_t, int);

    extern void (*rect_memset16)(uint16_t[], uint16_t, int, size_t, int);
    extern void (*rect_memset32)(uint32_t[], uint32_t, int, size_t, int);
    extern void (*rect_memset64)(uint64_t[], uint64_t, int, size_t, int);

    extern float (*cubic_solver)(float, float, float, float);

    static inline uint32_t hash(const void* data, size_t bytes, uint32_t seed=0) {
        // hash_fn is defined in SkOpts_spi.h so it can be used by //modules
        return hash_fn(data, bytes, seed);
    }

    // SkBitmapProcState optimized Shader, Sample, or Matrix procs.
    extern void (*S32_alpha_D32_filter_DX)(const SkBitmapProcState&,
                                           const uint32_t* xy, int count, SkPMColor*);
    extern void (*S32_alpha_D32_filter_DXDY)(const SkBitmapProcState&,
                                             const uint32_t* xy, int count, SkPMColor*);

    // We can't necessarily express the type of SkRasterPipeline stage functions here,
    // so we just use this void(*)(void) as a stand-in.
    using StageFn = void(*)(void);
    extern StageFn ops_highp[kNumRasterPipelineHighpOps], just_return_highp;
    extern StageFn ops_lowp [kNumRasterPipelineLowpOps ], just_return_lowp;

    extern void (*start_pipeline_highp)(size_t,size_t,size_t,size_t, SkRasterPipelineStage*);
    extern void (*start_pipeline_lowp )(size_t,size_t,size_t,size_t, SkRasterPipelineStage*);

    extern size_t raster_pipeline_lowp_stride;
    extern size_t raster_pipeline_highp_stride;

    extern void (*interpret_skvm)(const skvm::InterpreterInstruction insts[], int ninsts,
                                  int nregs, int loop, const int strides[],
                                  SkSL::TraceHook* traceHooks[], int nTraceHooks,
                                  int nargs, int n, void* args[]);
}  // namespace SkOpts

#endif // SkOpts_DEFINED
