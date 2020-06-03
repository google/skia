/*
 * Copyright 2020 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/core/SkOpts.h"

#if defined(SK_CPU_X86)

    // Turn on SSE4.2 feature set.
    #if defined(__clang__)
        #pragma clang attribute push(__attribute__((target("sse4.2"))), apply_to=function)
    #elif defined(__GNUC__)
        #pragma GCC push_options
        #pragma GCC target("sse4.2")
    #endif

    // Let our code in *_opts.h know we want SSE 4.2 features.
    #undef  SK_CPU_SSE_LEVEL
    #define SK_CPU_SSE_LEVEL SK_CPU_SSE_LEVEL_SSE42

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
    #endif

    #define SK_OPTS_NS sse42
    #include "src/opts/SkChecksum_opts.h"

    namespace SkOpts {
        void Init_sse42() {
            hash_fn = SK_OPTS_NS::hash_fn;
        }
    }

    #if defined(__clang__)
        #pragma clang attribute pop
    #elif defined(__GNUC__)
        #pragma GCC pop_options
    #endif

#endif//defined(SK_CPU_X86)
