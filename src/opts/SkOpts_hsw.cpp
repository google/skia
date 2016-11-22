/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkOpts.h"

#define SK_OPTS_NS hsw
#include "SkRasterPipeline_opts.h"

#if defined(_INC_MATH) && !defined(SkSafe_math_DEFINED)
    #error We have included ucrt\math.h without protecting it against ODR violation.
#endif

namespace SkOpts {
    void Init_hsw() {
        compile_pipeline = hsw::compile_pipeline;
    }
}

