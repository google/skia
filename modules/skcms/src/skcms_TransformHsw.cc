/*
 * Copyright 2018 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "skcms_public.h"     // NO_G3_REWRITE
#include "skcms_internals.h"  // NO_G3_REWRITE
#include "skcms_Transform.h"  // NO_G3_REWRITE
#include <assert.h>
#include <float.h>
#include <limits.h>
#include <stdlib.h>
#include <string.h>

#if defined(__ARM_NEON)
    #include <arm_neon.h>
#elif defined(__SSE__)
    #include <immintrin.h>

    #if defined(__clang__)
        // That #include <immintrin.h> is usually enough, but Clang's headers
        // avoid #including the whole kitchen sink when _MSC_VER is defined,
        // because lots of programs on Windows would include that and it'd be
        // a lot slower. But we want all those headers included, so we can use
        // their features (after making runtime checks).
        #include <smmintrin.h>
        #include <avxintrin.h>
        #include <avx2intrin.h>
        #include <avx512fintrin.h>
        #include <avx512dqintrin.h>
    #endif
#endif

namespace skcms_private {
namespace hsw {

#if defined(SKCMS_DISABLE_HSW)

void run_program(const Op* program, const void** contexts, ptrdiff_t programSize,
                 const char* src, char* dst, int n,
                 const size_t src_bpp, const size_t dst_bpp) {
    skcms_private::baseline::run_program(program, contexts, programSize,
                                         src, dst, n, src_bpp, dst_bpp);
}

#else

#define USING_AVX
#define USING_AVX_F16C
#define USING_AVX2
#define N 8
template <typename T> using V = skcms_private::Vec<N,T>;

#include "Transform_inl.h"

#endif

}  // namespace hsw
}  // namespace skcms_private
