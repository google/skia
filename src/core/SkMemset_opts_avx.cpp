/*
 * Copyright 2023 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/private/base/SkFeatures.h"
#include "src/core/SkMemset.h"
#include "src/core/SkOptsTargets.h"

#if defined(SK_CPU_X86) && !defined(SK_ENABLE_OPTIMIZE_SIZE)

// The order of these includes is important:
// 1) Select the target CPU architecture by defining SK_OPTS_TARGET and including SkOpts_SetTarget
// 2) Include the code to compile, typically in a _opts.h file.
// 3) Include SkOpts_RestoreTarget to switch back to the default CPU architecture

#define SK_OPTS_TARGET SK_OPTS_TARGET_AVX
#include "src/opts/SkOpts_SetTarget.h"

#include "src/opts/SkMemset_opts.h"

#include "src/opts/SkOpts_RestoreTarget.h"

namespace SkOpts {
    void Init_Memset_avx() {
        memset16 = avx::memset16;
        memset32 = avx::memset32;
        memset64 = avx::memset64;

        rect_memset16 = avx::rect_memset16;
        rect_memset32 = avx::rect_memset32;
        rect_memset64 = avx::rect_memset64;
    }
}  // namespace SkOpts

#endif // SK_CPU_X86 && !SK_ENABLE_OPTIMIZE_SIZE
