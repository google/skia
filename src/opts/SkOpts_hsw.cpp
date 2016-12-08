/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkSafe_math.h"   // Keep this first.
#include "SkOpts.h"

#define SK_OPTS_NS hsw
#include "SkBitmapFilter_opts.h"
#include "SkRasterPipeline_opts.h"

#if defined(_INC_MATH) && !defined(INC_MATH_IS_SAFE_NOW)
    #error We have included ucrt\math.h without protecting it against ODR violation.
#endif

namespace SkOpts {
    void Init_hsw() {
        run_pipeline     = hsw::run_pipeline;
        compile_pipeline = hsw::compile_pipeline;
        convolve_vertically = hsw::convolve_vertically;
    }
}

