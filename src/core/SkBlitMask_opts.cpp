/*
 * Copyright 2023 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/private/base/SkFeatures.h"
#include "src/core/SkBlitMask.h"
#include "src/core/SkCpu.h"
#include "src/core/SkOptsTargets.h"

#define SK_OPTS_TARGET SK_OPTS_TARGET_DEFAULT
#include "src/opts/SkOpts_SetTarget.h"

#include "src/opts/SkBlitMask_opts.h"  // IWYU pragma: keep

#include "src/opts/SkOpts_RestoreTarget.h"

namespace SkOpts {
    DEFINE_DEFAULT(blit_mask_d32_a8);

    void Init_BlitMask_ssse3();

    static bool init() {
    #if defined(SK_ENABLE_OPTIMIZE_SIZE)
        // All Init_foo functions are omitted when optimizing for size
    #elif defined(SK_CPU_X86)
        #if SK_CPU_SSE_LEVEL < SK_CPU_SSE_LEVEL_SSSE3
            if (SkCpu::Supports(SkCpu::SSSE3)) { Init_BlitMask_ssse3(); }
        #endif
    #endif
      return true;
    }

    void Init_BlitMask() {
        [[maybe_unused]] static bool gInitialized = init();
    }
}  // namespace SkOpts
