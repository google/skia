/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkOpts_DEFINED
#define SkOpts_DEFINED

#include "include/private/base/SkSpan_impl.h"
#include "src/core/SkRasterPipelineOpList.h"

#include <cstddef>
#include <cstdint>

namespace SkRasterPipelineContexts {
struct MemoryCtxPatch;
}

/**
 * SkOpts (short for SkOptimizations) is a mechanism where we can ship with multiple implementations
 * of a set of functions and dynamically choose the best one at runtime (e.g. the call to
 * SkGraphics::Init(), which calls SkOpts::Init()) depending on the detected CPU features. This is
 * also referred to as having "specializations" of a given function.
 *
 * For example, Skia might be compiled to support CPUs that only have the sse2 instruction set
 * (https://en.wikipedia.org/wiki/X86_instruction_listings#SSE2_instructions)
 * but may be run on a more modern CPU that supports AVX2 instructions.
 * (https://en.wikipedia.org/wiki/Advanced_Vector_Extensions)
 * SkOpts allow Skia to have two versions of a row-blitting function, one that uses normal C++
 * code (e.g. loops, scalar integer math) and one that makes use of the AVX2 vector types and
 * intrinsic functions. This function is declared here in the SkOpts namespace, and then the
 * implementation (see SkOpts.cpp) is deferred to a function of the same name in the sse2::
 * namespace (the minimum Skia is compiled with) using DEFINE_DEFAULT.
 *
 * All implementations of this blit function are done in a header file file in //src/opts
 * (e.g. //src/opts/SkBlitRow_opts.h). ifdefs guard each of the implementations, such that only
 * one implementation is possible for a given SK_CPU_SSE_LEVEL. This header will be compiled
 * *multiple* times with a different SK_CPU_SSE_LEVEL each compilation.
 *
 * Each CPU instruction set that we want specializations for has a .cpp file in //src/opts which
 * defines an Init() function that replaces the function pointers in the SkOpts namespace with the
 * ones from the specialized namespace (e.g. hsw::). These .cpp files don't implement the
 * specializations, they just refer to the specialization created in the header files (e.g.
 * SkBlitRow_opts.h).
 *
 * At compile time:
 *   - SkOpts.cpp is compiled with the minimum CPU level (e.g. SSE2). Because this
 *     file includes all the headers in //src/opts/, those headers add "the default implementation"
 *     of all their functions to the SK_OPTS_NS namespace (e.g. sse2::blit_row_color32).
 *   - Each of the specialized .cpp files in //src/opts/ are compiled with their respective
 *     compiler flags. Because the specialized .cpp file includes the headers that implement the
 *     functions using intrinsics or other CPU-specific code, those specialized functions end up
 *     in the specialized namespace, e.g. (hsw::blit_row_color32).
 *
 * At link time, the default implementations and all specializations of all SkOpts functions are
 * included in the resulting library/binary file.
 *
 * At runtime, SkOpts::Init() will run the appropriate Init functions that the current CPU level
 * supports specializations for (e.g. Init_hsw, Init_ssse3). Note multiple Init functions can
 * be called as CPU instruction sets are typically super sets of older instruction sets
 */

struct SkRasterPipelineStage;

namespace SkOpts {
    // Call to replace pointers to portable functions with pointers to CPU-specific functions.
    // Thread-safe and idempotent.
    // Called by SkGraphics::Init().
    void Init();

    // We can't necessarily express the type of SkRasterPipeline stage functions here,
    // so we just use this void(*)(void) as a stand-in.
    using StageFn = void(*)(void);
    extern StageFn ops_highp[kNumRasterPipelineHighpOps], just_return_highp;
    extern StageFn ops_lowp [kNumRasterPipelineLowpOps ], just_return_lowp;

    extern void (*start_pipeline_highp)(size_t,size_t,size_t,size_t, SkRasterPipelineStage*,
                                        SkSpan<SkRasterPipelineContexts::MemoryCtxPatch>,
                                        uint8_t*);
    extern void (*start_pipeline_lowp )(size_t,size_t,size_t,size_t, SkRasterPipelineStage*,
                                        SkSpan<SkRasterPipelineContexts::MemoryCtxPatch>,
                                        uint8_t*);

    extern size_t raster_pipeline_lowp_stride;
    extern size_t raster_pipeline_highp_stride;
}  // namespace SkOpts

#endif // SkOpts_DEFINED
