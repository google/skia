/*
 * Copyright 2020 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/core/SkOpts.h"

#if defined(SK_CPU_X86)

    // Turn on SKX feature set.
    #if defined(__clang__)
        #pragma clang attribute push(__attribute__((target("avx512f,avx512dq,avx512cd,avx512bw,avx512vl"))), apply_to=function)
    #elif defined(__GNUC__)
        #pragma GCC push_options
        #pragma GCC target("avx512f,avx512dq,avx512cd,avx512bw,avx512vl")
    #endif

    // Let our code in *_opts.h know we want SKX features.
    #undef  SK_CPU_SSE_LEVEL
    #define SK_CPU_SSE_LEVEL SK_CPU_SSE_LEVEL_SKX

    #define SK_OPTS_NS skx
    #include "src/opts/SkRasterPipeline_opts.h"
    #include "src/opts/SkVM_opts.h"

    namespace SkOpts {
        void Init_skx() {
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

            interpret_skvm = SK_OPTS_NS::interpret_skvm;
        }
    }

    #if defined(__clang__)
        #pragma clang attribute pop
    #elif defined(__GNUC__)
        #pragma GCC pop_options
    #endif

#endif//defined(SK_CPU_X86)
