/*
 * Copyright 2020 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/core/SkOpts.h"

#if defined(SK_CPU_X86)

    // Turn on HSW feature set.
    #if defined(__clang__)
        #pragma clang attribute push(__attribute__((target("avx2,f16c,fma"))), apply_to=function)
    #elif defined(__GNUC__)
        #pragma GCC push_options
        #pragma GCC target("avx2,f16c,fma")
    #endif

    // Let our code in *_opts.h know we want HSW features.
    #undef  SK_CPU_SSE_LEVEL
    #define SK_CPU_SSE_LEVEL SK_CPU_SSE_LEVEL_AVX2

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
        #define __F16C__ 1
        #define __AVX2__ 1
        #define __FMA__ 1
    #endif

    #define SK_OPTS_NS hsw
    #include "src/core/SkCubicSolver.h"
    #include "src/opts/SkBitmapProcState_opts.h"
    #include "src/opts/SkBlitRow_opts.h"
    #include "src/opts/SkRasterPipeline_opts.h"
    #include "src/opts/SkSwizzler_opts.h"
    #include "src/opts/SkUtils_opts.h"
    #include "src/opts/SkVM_opts.h"

    namespace SkOpts {
        void Init_hsw() {
            blit_row_color32     = SK_OPTS_NS::blit_row_color32;
            blit_row_s32a_opaque = SK_OPTS_NS::blit_row_s32a_opaque;

            S32_alpha_D32_filter_DX  = SK_OPTS_NS::S32_alpha_D32_filter_DX;

            cubic_solver = SK_OPTS_NS::cubic_solver;

            RGBA_to_BGRA          = SK_OPTS_NS::RGBA_to_BGRA;
            RGBA_to_rgbA          = SK_OPTS_NS::RGBA_to_rgbA;
            RGBA_to_bgrA          = SK_OPTS_NS::RGBA_to_bgrA;
            RGB_to_RGB1           = SK_OPTS_NS::RGB_to_RGB1;
            RGB_to_BGR1           = SK_OPTS_NS::RGB_to_BGR1;
            gray_to_RGB1          = SK_OPTS_NS::gray_to_RGB1;
            grayA_to_RGBA         = SK_OPTS_NS::grayA_to_RGBA;
            grayA_to_rgbA         = SK_OPTS_NS::grayA_to_rgbA;
            inverted_CMYK_to_RGB1 = SK_OPTS_NS::inverted_CMYK_to_RGB1;
            inverted_CMYK_to_BGR1 = SK_OPTS_NS::inverted_CMYK_to_BGR1;

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
