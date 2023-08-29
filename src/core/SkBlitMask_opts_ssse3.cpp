/*
 * Copyright 2023 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/private/base/SkFeatures.h"
#include "src/core/SkBlitMask.h"
#include "src/core/SkOptsTargets.h"

#if defined(SK_CPU_X86) && !defined(SK_ENABLE_OPTIMIZE_SIZE)

// The order of these includes is important:
// 1) Select the target CPU architecture by defining SK_OPTS_TARGET and including SkOpts_SetTarget
// 2) Include the code to compile, typically in a _opts.h file.
// 3) Include SkOpts_RestoreTarget to switch back to the default CPU architecture

#define SK_OPTS_TARGET SK_OPTS_TARGET_SSSE3
#include "src/opts/SkOpts_SetTarget.h"

#include "src/opts/SkBlitMask_opts.h"

#include "src/opts/SkOpts_RestoreTarget.h"

namespace SkOpts {
    void Init_BlitMask_ssse3() {
        blit_mask_d32_a8 = ssse3::blit_mask_d32_a8;
    }
}  // namespace SkOpts

#endif // SK_CPU_X86 && !SK_ENABLE_OPTIMIZE_SIZE
