/*
 * Copyright 2023 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

// Include guards are intentionally omitted

#include "include/private/base/SkFeatures.h"

#if !defined(SK_OPTS_TARGET)
    #error Define SK_OPTS_TARGET before including SkOpts_SetTarget
#endif

#if defined(SK_OPTS_NS)
    // For testing define SK_OPTS_NS before including.
#elif SK_OPTS_TARGET == SK_OPTS_TARGET_DEFAULT

    #if defined(SK_ARM_HAS_NEON)
        #define SK_OPTS_NS neon
    #elif SK_CPU_SSE_LEVEL >= SK_CPU_SSE_LEVEL_SKX
        #define SK_OPTS_NS skx
    #elif SK_CPU_SSE_LEVEL >= SK_CPU_SSE_LEVEL_AVX2
        #define SK_OPTS_NS avx2
    #elif SK_CPU_SSE_LEVEL >= SK_CPU_SSE_LEVEL_AVX
        #define SK_OPTS_NS avx
    #elif SK_CPU_SSE_LEVEL >= SK_CPU_SSE_LEVEL_SSE42
        #define SK_OPTS_NS sse42
    #elif SK_CPU_SSE_LEVEL >= SK_CPU_SSE_LEVEL_SSE41
        #define SK_OPTS_NS sse41
    #elif SK_CPU_SSE_LEVEL >= SK_CPU_SSE_LEVEL_SSSE3
        #define SK_OPTS_NS ssse3
    #elif SK_CPU_SSE_LEVEL >= SK_CPU_SSE_LEVEL_SSE3
        #define SK_OPTS_NS sse3
    #elif SK_CPU_SSE_LEVEL >= SK_CPU_SSE_LEVEL_SSE2
        #define SK_OPTS_NS sse2
    #elif SK_CPU_SSE_LEVEL >= SK_CPU_SSE_LEVEL_SSE1
        #define SK_OPTS_NS sse
    #elif SK_CPU_LSX_LEVEL >= SK_CPU_LSX_LEVEL_LASX
        #define SK_OPTS_NS lasx
    #elif SK_CPU_LSX_LEVEL >= SK_CPU_LSX_LEVEL_LSX
        #define SK_OPTS_NS lsx
    #else
        #define SK_OPTS_NS portable
    #endif

    #define DEFINE_DEFAULT(name) decltype(name) name = SK_OPTS_NS::name

#else  // SK_OPTS_TARGET != SK_OPTS_TARGET_DEFAULT

    #if defined(SK_OLD_CPU_SSE_LEVEL)
        #error Include SkOpts_RestoreTarget before re-including SkOpts_SetTarget
    #endif

    #define SK_OLD_CPU_SSE_LEVEL SK_CPU_SSE_LEVEL
    #undef SK_CPU_SSE_LEVEL
    #undef SK_CPU_LSX_LEVEL

    // NOTE: Below, we automatically include arch-specific intrinsic headers when we've detected
    // that the compiler is clang-cl. Clang's headers skip including "unsupported" intrinsics (via
    // defines like __AVX__ etc.), but only when _MSC_VER is also defined. To get around that, we
    // directly include the headers for any intrinsics that ought to be present in each opts target.
    //
    // Each of the specific intrinsic headers also checks to ensure that immintrin.h has been
    // included, so do that here, first.
    #if defined(__clang__) && defined(_MSC_VER)
        #define __RTMINTRIN_H  // Workaround for https://github.com/llvm/llvm-project/issues/95133
        #include <immintrin.h>
    #endif

    // Why not put the target string in a #define, and remove the boilerplate? Because GCC doesn't
    // allow the target() option to be expanded by the preprocessor - it must be a literal string.
    #if SK_OPTS_TARGET == SK_OPTS_TARGET_SSSE3

        #define SK_CPU_SSE_LEVEL SK_CPU_SSE_LEVEL_SSSE3
        #define SK_OPTS_NS ssse3

        #if defined(__clang__)
            #pragma clang attribute push(__attribute__((target("sse2,ssse3"))), apply_to=function)
        #elif defined(__GNUC__)
            #pragma GCC push_options
            #pragma GCC target("sse2,ssse3")
        #endif

        #if defined(__clang__) && defined(_MSC_VER)
            #include <pmmintrin.h>
            #include <tmmintrin.h>
        #endif

    #elif SK_OPTS_TARGET == SK_OPTS_TARGET_AVX

        #define SK_CPU_SSE_LEVEL SK_CPU_SSE_LEVEL_AVX
        #define SK_OPTS_NS avx

        #if defined(__clang__)
            #pragma clang attribute push(__attribute__((target("sse2,ssse3,sse4.1,sse4.2,avx"))), apply_to=function)
        #elif defined(__GNUC__)
            #pragma GCC push_options
            #pragma GCC target("sse2,ssse3,sse4.1,sse4.2,avx")
        #endif

        #if defined(__clang__) && defined(_MSC_VER)
            #include <pmmintrin.h>
            #include <tmmintrin.h>
            #include <smmintrin.h>
            #include <avxintrin.h>
        #endif

    #elif SK_OPTS_TARGET == SK_OPTS_TARGET_HSW

        #define SK_CPU_SSE_LEVEL SK_CPU_SSE_LEVEL_AVX2
        #define SK_OPTS_NS hsw

        #if defined(__clang__)
            #pragma clang attribute push(__attribute__((target("sse2,ssse3,sse4.1,sse4.2,avx,avx2,bmi,bmi2,f16c,fma"))), apply_to=function)
        #elif defined(__GNUC__)
            #pragma GCC push_options
            #pragma GCC target("sse2,ssse3,sse4.1,sse4.2,avx,avx2,bmi,bmi2,f16c,fma")
        #endif

        #if defined(__clang__) && defined(_MSC_VER)
            #include <pmmintrin.h>
            #include <tmmintrin.h>
            #include <smmintrin.h>
            #include <avxintrin.h>
            #include <avx2intrin.h>
            #include <f16cintrin.h>
            #include <bmi2intrin.h>
            #include <fmaintrin.h>
        #endif

    #elif SK_OPTS_TARGET == SK_OPTS_TARGET_LASX

        #define SK_CPU_LSX_LEVEL SK_CPU_LSX_LEVEL_LASX
        #define SK_OPTS_NS lasx
        // The intrinsic from lasxintrin.h is wrapped by the __loongarch_asx macro, so we need to define it.
        #define __loongarch_asx

        #if defined(__clang__)
          #pragma clang attribute push(__attribute__((target("lasx"))), apply_to=function)
        #endif

    #else
        #error Unexpected value of SK_OPTS_TARGET

    #endif

#endif  // !SK_OPTS_TARGET_DEFAULT
