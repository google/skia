/*
 * Copyright 2020 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "src/core/SkOpts.h"

#if defined(SK_CPU_ARM64)

    // Turn on CRC32 feature set.
    #if defined(__clang__)
        #pragma clang attribute push(__attribute__((target("crc"))), apply_to=function)
    #elif defined(__GNUC__)
        #pragma GCC push_options
        #pragma GCC target("crc")
    #endif

    // Let our code in *_opts.h know we want CRC32 features.
    #define SK_ARM_HAS_CRC32

    // Let the arm_acle.h headers know we will be using the CRC32 intrinsics.
    #define __ARM_FEATURE_CRC32 1

    #define SK_OPTS_NS crc32
    #include "src/opts/SkChecksum_opts.h"

    namespace SkOpts {
        void Init_crc32() {
            hash_fn = SK_OPTS_NS::hash_fn;
        }
    }

    #if defined(__clang__)
        #pragma clang attribute pop
    #elif defined(__GNUC__)
        #pragma GCC pop_options
    #endif

#endif//defined(SK_CPU_ARM64)
