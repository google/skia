
/*
 * Copyright 2023 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/private/base/SkFeatures.h"
#include "src/core/SkOpts.h"
#include "src/core/SkSwizzlePriv.h"

#if defined(SK_CPU_X86) && !defined(SK_ENABLE_OPTIMIZE_SIZE)

// The order of these includes is important:
// 1) Select the target CPU architecture by defining SK_OPTS_TARGET and including SkOpts_SetTarget
// 2) Include the code to compile, typically in a _opts.h file.
// 3) Include SkOpts_RestoreTarget to switch back to the default CPU architecture

#define SK_OPTS_TARGET SK_OPTS_TARGET_SSSE3
#include "src/opts/SkOpts_SetTarget.h"

#include "src/opts/SkSwizzler_opts.h"

#include "src/opts/SkOpts_RestoreTarget.h"

namespace SkOpts {
    void Init_Swizzler_ssse3() {
        RGBA_to_BGRA          = ssse3::RGBA_to_BGRA;
        RGBA_to_rgbA          = ssse3::RGBA_to_rgbA;
        RGBA_to_bgrA          = ssse3::RGBA_to_bgrA;
        RGB_to_RGB1           = ssse3::RGB_to_RGB1;
        RGB_to_BGR1           = ssse3::RGB_to_BGR1;
        gray_to_RGB1          = ssse3::gray_to_RGB1;
        grayA_to_RGBA         = ssse3::grayA_to_RGBA;
        grayA_to_rgbA         = ssse3::grayA_to_rgbA;
        inverted_CMYK_to_RGB1 = ssse3::inverted_CMYK_to_RGB1;
        inverted_CMYK_to_BGR1 = ssse3::inverted_CMYK_to_BGR1;
    }
}  // namespace SkOpts

#endif // SK_CPU_X86 && !SK_ENABLE_OPTIMIZE_SIZE
