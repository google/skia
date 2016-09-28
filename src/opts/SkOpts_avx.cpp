/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkOpts.h"

#define SK_OPTS_NS avx
#include "SkRasterPipeline_opts.h"

namespace SkOpts {
    void Init_avx() {
    #define STAGE(stage, kCallNext) \
        stage = body<SK_OPTS_NS::stage, kCallNext>

        STAGE(srcover, true);
    #undef STAGE

    #define STAGE(stage, kCallNext) \
        stage##_body = body<SK_OPTS_NS::stage, kCallNext>; \
        stage##_tail = tail<SK_OPTS_NS::stage, kCallNext>

        STAGE(load_d_srgb, true);
        STAGE(load_s_srgb, true);
        STAGE( store_srgb, false);
    #undef STAGE
    }
}
