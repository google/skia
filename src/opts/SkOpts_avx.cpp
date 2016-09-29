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
        stages_8  [SkRasterPipeline::stage] = stage_8  <SK_OPTS_NS::stage, kCallNext>; \
        stages_1_7[SkRasterPipeline::stage] = stage_1_7<SK_OPTS_NS::stage, kCallNext>

        STAGE(constant_color, true);
        STAGE(lerp_constant_float , true);
        STAGE(srcover , true);

        STAGE(scale_u8, true);

    #undef STAGE
    }
}
