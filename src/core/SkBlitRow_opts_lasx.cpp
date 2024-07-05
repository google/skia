/*
 * Copyright 2023 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/private/base/SkFeatures.h" // IWYU pragma: keep
#include "src/core/SkBlitRow.h" // IWYU pragma: keep
#include "src/core/SkOptsTargets.h" // IWYU pragma: keep

#if defined(SK_CPU_LOONGARCH) && !defined(SK_ENABLE_OPTIMIZE_SIZE)

// The order of these includes is important:
// 1) Select the target CPU architecture by defining SK_OPTS_TARGET and including SkOpts_SetTarget
// 2) Include the code to compile, typically in a _opts.h file.
// 3) Include SkOpts_RestoreTarget to switch back to the default CPU architecture

#define SK_OPTS_TARGET SK_OPTS_TARGET_LASX
#include "src/opts/SkOpts_SetTarget.h"

#include "src/opts/SkBlitRow_opts.h"

#include "src/opts/SkOpts_RestoreTarget.h"

namespace SkOpts {
    void Init_BlitRow_lasx() {
        blit_row_color32     = lasx::blit_row_color32;
        blit_row_s32a_opaque = lasx::blit_row_s32a_opaque;
    }
}  // namespace SkOpts

#endif // SK_CPU_LOONGARCH && !SK_ENABLE_OPTIMIZE_SIZE
