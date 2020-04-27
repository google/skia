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
        #define __AVX512F__ 1
        #define __AVX512DQ__ 1
        #define __AVX512CD__ 1
        #define __AVX512BW__ 1
        #define __AVX512VL__ 1
    #endif

    #define SK_OPTS_NS skx
    #include "src/opts/SkBlitRow_opts.h"
    #include "src/opts/SkVM_opts.h"

    namespace SkOpts {
        void Init_skx() {
            blit_row_s32a_opaque = SK_OPTS_NS::blit_row_s32a_opaque;
            interpret_skvm = SK_OPTS_NS::interpret_skvm;
        }
    }

    #if defined(__clang__)
        #pragma clang attribute pop
    #elif defined(__GNUC__)
        #pragma GCC pop_options
    #endif

#endif//defined(SK_CPU_X86)
