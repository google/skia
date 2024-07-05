
/*
 * Copyright 2023 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/private/base/SkFeatures.h" // IWYU pragma: keep
#include "src/core/SkOptsTargets.h" // IWYU pragma: keep
#include "src/core/SkSwizzlePriv.h" // IWYU pragma: keep

#if defined(SK_CPU_LOONGARCH) && !defined(SK_ENABLE_OPTIMIZE_SIZE)

// The order of these includes is important:
// 1) Select the target CPU architecture by defining SK_OPTS_TARGET and including SkOpts_SetTarget
// 2) Include the code to compile, typically in a _opts.inc file.
// 3) Include SkOpts_RestoreTarget to switch back to the default CPU architecture

#define SK_OPTS_TARGET SK_OPTS_TARGET_LASX
#include "src/opts/SkOpts_SetTarget.h"

#include "src/opts/SkSwizzler_opts.inc"

#include "src/opts/SkOpts_RestoreTarget.h"

namespace SkOpts {
    void Init_Swizzler_lasx() {
        RGBA_to_BGRA          = lasx::RGBA_to_BGRA;
        RGBA_to_rgbA          = lasx::RGBA_to_rgbA;
        RGBA_to_bgrA          = lasx::RGBA_to_bgrA;
        gray_to_RGB1          = lasx::gray_to_RGB1;
        grayA_to_RGBA         = lasx::grayA_to_RGBA;
        grayA_to_rgbA         = lasx::grayA_to_rgbA;
        inverted_CMYK_to_RGB1 = lasx::inverted_CMYK_to_RGB1;
        inverted_CMYK_to_BGR1 = lasx::inverted_CMYK_to_BGR1;
    }
}  // namespace SkOpts

#endif // SK_CPU_LOONGARCH && !SK_ENABLE_OPTIMIZE_SIZE
