/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkOpts.h"

#define SK_OPTS_NS hsw
#include "SkRasterPipeline_opts.h"

namespace SkOpts {
    void Init_hsw() {
        compile_pipeline = hsw::compile_pipeline;
    }
}

