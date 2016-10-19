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

    #define STAGE(stage)                                                        \
        body[SkRasterPipeline::stage] = (SkOpts::VoidFn)SK_OPTS_NS::stage;      \
        tail[SkRasterPipeline::stage] = (SkOpts::VoidFn)SK_OPTS_NS::stage##_tail

        STAGE(store_565);
        STAGE(store_srgb);
        STAGE(store_f16);

        STAGE(load_s_565);
        STAGE(load_s_srgb);
        STAGE(load_s_f16);

        STAGE(load_d_565);
        STAGE(load_d_srgb);
        STAGE(load_d_f16);

        STAGE(scale_u8);

        STAGE(lerp_u8);
        STAGE(lerp_565);
    #undef STAGE

    #define STAGE(stage)                                                   \
        body[SkRasterPipeline::stage] = (SkOpts::VoidFn)SK_OPTS_NS::stage; \
        tail[SkRasterPipeline::stage] = (SkOpts::VoidFn)SK_OPTS_NS::stage

        STAGE(just_return);
        STAGE(swap_src_dst);
        STAGE(lerp_constant_float);
        STAGE(constant_color);

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

