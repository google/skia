/*
 * Copyright 2020 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/core/SkOpts.h"

#if defined(SK_CPU_X86)

    // Turn on AVX feature set.
    #if defined(__clang__)
        #pragma clang attribute push(__attribute__((target("avx"))), apply_to=function)
    #elif defined(__GNUC__)
        #pragma GCC push_options
        #pragma GCC target("avx")
    #endif

    // Let our code in *_opts.h know we want AVX features.
    #undef  SK_CPU_SSE_LEVEL
    #define SK_CPU_SSE_LEVEL SK_CPU_SSE_LEVEL_AVX

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
        #define __SSE4_2__ 1
        #define __AVX__ 1
    #endif

    #define SK_OPTS_NS avx
    #include "src/opts/SkRasterPipeline_opts.h"
    #include "src/opts/SkUtils_opts.h"

    namespace SkOpts {
        void Init_avx() {
            memset16 = SK_OPTS_NS::memset16;
            memset32 = SK_OPTS_NS::memset32;
            memset64 = SK_OPTS_NS::memset64;

            rect_memset16 = SK_OPTS_NS::rect_memset16;
            rect_memset32 = SK_OPTS_NS::rect_memset32;
            rect_memset64 = SK_OPTS_NS::rect_memset64;

            // TODO: maybe cut these stages to save code size?  Almost everything is HSW these days.
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
