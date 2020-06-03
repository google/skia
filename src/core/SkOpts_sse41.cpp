/*
 * Copyright 2020 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/core/SkOpts.h"

#if defined(SK_CPU_X86)

    // Turn on SSE4.1 feature set.
    #if defined(__clang__)
        #pragma clang attribute push(__attribute__((target("sse4.1"))), apply_to=function)
    #elif defined(__GNUC__)
        #pragma GCC push_options
        #pragma GCC target("sse4.1")
    #endif

    // Let our code in *_opts.h know we want SSE 4.1 features.
    #undef  SK_CPU_SSE_LEVEL
    #define SK_CPU_SSE_LEVEL SK_CPU_SSE_LEVEL_SSE41

    #if defined(__clang__) && defined(_MSC_VER)
        // clang-cl's immintrin.h is bizarrely annoying, not including the
        // various foointrin.h unless the __FOO__ flag is also defined (i.e.
        // you used command-line flags to set the features instead of attributes).
        // MSVC itself doesn't work this way, nor does non-_MSC_VER clang.  :/
        #define __SSE__ 1
        #define __SSE2__ 1
        #define __SSE3__ 1
        #define __SSSE3__ 1
        #define __SSE4_1__ 1
    #endif

    #define SK_OPTS_NS sse41
    #include "src/opts/SkBlitRow_opts.h"
    #include "src/opts/SkRasterPipeline_opts.h"

    // TODO: maybe cut this whole file to save code size?  Almost everything is HSW these days.
    namespace SkOpts {
        void Init_sse41() {
            blit_row_color32     = SK_OPTS_NS::blit_row_color32;
            blit_row_s32a_opaque = SK_OPTS_NS::blit_row_s32a_opaque;

        #define M(st) stages_highp[SkRasterPipeline::st] = (StageFn)SK_OPTS_NS::st;
            SK_RASTER_PIPELINE_STAGES(M)
            just_return_highp = (StageFn)SK_OPTS_NS::just_return;
            start_pipeline_highp = SK_OPTS_NS::start_pipeline;
        #undef M

        #define M(st) stages_lowp[SkRasterPipeline::st] = (StageFn)SK_OPTS_NS::lowp::st;
            SK_RASTER_PIPELINE_STAGES(M)
            just_return_lowp = (StageFn)SK_OPTS_NS::lowp::just_return;
            start_pipeline_lowp = SK_OPTS_NS::lowp::start_pipeline;
        #undef M
        }
    }

    #if defined(__clang__)
        #pragma clang attribute pop
    #elif defined(__GNUC__)
        #pragma GCC pop_options
    #endif

#endif//defined(SK_CPU_X86)

