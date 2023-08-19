/*
 * Copyright 2023 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

// Include guards are intentionally omitted

#include "include/private/base/SkFeatures.h"

#if SK_OPTS_TARGET == SK_OPTS_TARGET_DEFAULT
    // Nothing to do here
#else

    #if !defined(SK_OLD_CPU_SSE_LEVEL)
        #error Include SkOpts_SetTarget before including SkOpts_RestoreTarget
    #endif

    #undef SK_CPU_SSE_LEVEL
    #define SK_CPU_SSE_LEVEL SK_OLD_CPU_SSE_LEVEL
    #undef SK_OLD_CPU_SSE_LEVEL

    #if defined(__clang__)
        #pragma clang attribute pop
    #elif defined(__GNUC__)
        #pragma GCC pop_options
    #endif

#endif
