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
#if 1
    #define STAGE(stage, kCallNext) \
        stages_4  [SkRasterPipeline::stage] = stage_4  <SK_OPTS_NS::stage, kCallNext>; \
        stages_1_3[SkRasterPipeline::stage] = stage_1_3<SK_OPTS_NS::stage, kCallNext>

        STAGE(store_565 , false);
        STAGE(store_srgb, false);
        STAGE(store_f16 , false);

        STAGE(load_s_565 , true);
        STAGE(load_s_srgb, true);
        STAGE(load_s_f16 , true);

        STAGE(load_d_565 , true);
        STAGE(load_d_srgb, true);
        STAGE(load_d_f16 , true);

        STAGE(scale_u8, true);

        STAGE(lerp_u8            , true);
        STAGE(lerp_565           , true);
        STAGE(lerp_constant_float, true);

        STAGE(constant_color, true);

        STAGE(srcover, true);
    #undef STAGE
#endif
    }
}

