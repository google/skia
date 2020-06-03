/*
 * Copyright 2020 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/core/SkOpts.h"

#if defined(SK_CPU_X86)

    // Turn on SSSE3 feature set.
    #if defined(__clang__)
        #pragma clang attribute push(__attribute__((target("ssse3"))), apply_to=function)
    #elif defined(__GNUC__)
        #pragma GCC push_options
        #pragma GCC target("ssse3")
    #endif

    // Let our code in *_opts.h know we want SSSE3 features.
    #undef  SK_CPU_SSE_LEVEL
    #define SK_CPU_SSE_LEVEL SK_CPU_SSE_LEVEL_SSSE3

    #if defined(__clang__) && defined(_MSC_VER)
        // clang-cl's immintrin.h is bizarrely annoying, not including the
        // various foointrin.h unless the __FOO__ flag is also defined (i.e.
        // you used command-line flags to set the features instead of attributes).
        // MSVC itself doesn't work this way, nor does non-_MSC_VER clang.  :/
        #define __SSE__ 1
        #define __SSE2__ 1
        #define __SSE3__ 1
        #define __SSSE3__ 1
    #endif

    #define SK_OPTS_NS ssse3
    #include "src/opts/SkBitmapProcState_opts.h"
    #include "src/opts/SkBlitMask_opts.h"
    #include "src/opts/SkSwizzler_opts.h"
    #include "src/opts/SkXfermode_opts.h"

    namespace SkOpts {
        void Init_ssse3() {
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

            // TODO: maybe cut these three to save code size?  Almost everything is HSW these days.
            create_xfermode = SK_OPTS_NS::create_xfermode;
            blit_mask_d32_a8 = SK_OPTS_NS::blit_mask_d32_a8;
            S32_alpha_D32_filter_DX  = SK_OPTS_NS::S32_alpha_D32_filter_DX;
        }
    }

    #if defined(__clang__)
        #pragma clang attribute pop
    #elif defined(__GNUC__)
        #pragma GCC pop_options
    #endif

#endif//defined(SK_CPU_X86)
