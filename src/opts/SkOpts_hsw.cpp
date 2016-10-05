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

    run_pipeline = SK_OPTS_NS::run_pipeline;

    #define STAGE(stage, kCallNext) \
        body[SkRasterPipeline::stage] = stage_b<SK_OPTS_NS::stage, kCallNext>; \
        tail[SkRasterPipeline::stage] = stage_t<SK_OPTS_NS::stage, kCallNext>

        STAGE(just_return, false);

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

    #undef STAGE

    #define STAGE(stage) \
        body[SkRasterPipeline::stage] = SK_OPTS_NS::stage; \
        tail[SkRasterPipeline::stage] = SK_OPTS_NS::stage

        STAGE(dst);
        STAGE(dstatop);
        STAGE(dstin);
        STAGE(dstout);
        STAGE(dstover);
        STAGE(srcatop);
        STAGE(srcin);
        STAGE(srcout);
        STAGE(srcover);
        STAGE(clear);
        STAGE(modulate);
        STAGE(multiply);
        STAGE(plus_);
        STAGE(screen);
        STAGE(xor_);
        STAGE(colorburn);
        STAGE(colordodge);
        STAGE(darken);
        STAGE(difference);
        STAGE(exclusion);
        STAGE(hardlight);
        STAGE(lighten);
        STAGE(overlay);
        STAGE(softlight);
    #undef STAGE
    }
}

