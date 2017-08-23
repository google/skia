/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkSafe_math.h"   // Keep this first.
#include "SkOpts.h"

#if defined(_INC_MATH) && !defined(INC_MATH_IS_SAFE_NOW)
    #error We have included ucrt\math.h without protecting it against ODR violation.
#endif

#define SK_OPTS_NS avx
#include "SkRasterPipeline_opts.h"
#include "SkUtils_opts.h"

namespace SkOpts {
    void Init_avx() {
        memset16               = avx::memset16;
        memset32               = avx::memset32;
        memset64               = avx::memset64;
        raster_pipeline_stages = avx::raster_pipeline_stages;
    }
}
