/*
 * Copyright 2023 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/private/base/SkFeatures.h"
#include "src/core/SkCpu.h"
#include "src/core/SkMemset.h"

#define SK_OPTS_TARGET SK_OPTS_TARGET_DEFAULT
#include "src/opts/SkOpts_SetTarget.h"

#include "src/opts/SkMemset_opts.h"  // IWYU pragma: keep

#include "src/opts/SkOpts_RestoreTarget.h"

namespace SkOpts {
    DEFINE_DEFAULT(memset16);
    DEFINE_DEFAULT(memset32);
    DEFINE_DEFAULT(memset64);

    DEFINE_DEFAULT(rect_memset16);
    DEFINE_DEFAULT(rect_memset32);
    DEFINE_DEFAULT(rect_memset64);

    void Init_Memset_avx();
    void Init_Memset_erms();

    static bool init() {
    #if defined(SK_ENABLE_OPTIMIZE_SIZE)
        // All Init_foo functions are omitted when optimizing for size
    #elif defined(SK_CPU_X86)
        #if SK_CPU_SSE_LEVEL < SK_CPU_SSE_LEVEL_AVX
            if (SkCpu::Supports(SkCpu::AVX)) { Init_Memset_avx(); }
        #endif

        if (SkCpu::Supports(SkCpu::ERMS)) { Init_Memset_erms(); }
    #endif
      return true;
    }

    void Init_Memset() {
        [[maybe_unused]] static bool gInitialized = init();
    }
}  // namespace SkOpts
