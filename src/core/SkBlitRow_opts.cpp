/*
 * Copyright 2023 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/private/base/SkFeatures.h"
#include "src/core/SkBlitRow.h"
#include "src/core/SkCpu.h"
#include "src/core/SkOptsTargets.h"

#define SK_OPTS_TARGET SK_OPTS_TARGET_DEFAULT
#include "src/opts/SkOpts_SetTarget.h"

#include "src/opts/SkBlitRow_opts.h"  // IWYU pragma: keep

#include "src/opts/SkOpts_RestoreTarget.h"

namespace SkOpts {
    DEFINE_DEFAULT(blit_row_color32);
    DEFINE_DEFAULT(blit_row_s32a_opaque);

    void Init_BlitRow_hsw();
    void Init_BlitRow_lasx();

    static bool init() {
    #if defined(SK_ENABLE_OPTIMIZE_SIZE)
        // All Init_foo functions are omitted when optimizing for size
    #elif defined(SK_CPU_X86)
        #if SK_CPU_SSE_LEVEL < SK_CPU_SSE_LEVEL_AVX2
            if (SkCpu::Supports(SkCpu::HSW)) { Init_BlitRow_hsw(); }
        #endif
    #elif defined(SK_CPU_LOONGARCH)
        #if SK_CPU_LSX_LEVEL < SK_CPU_LSX_LEVEL_LASX
            if (SkCpu::Supports(SkCpu::LOONGARCH_ASX)) { Init_BlitRow_lasx(); }
        #endif
    #endif
      return true;
    }

    void Init_BlitRow() {
        [[maybe_unused]] static bool gInitialized = init();
    }
}  // namespace SkOpts
