/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkOpts.h"

#define SK_OPTS_NS sse41
#include "SkBlurImageFilter_opts.h"
#include "SkBlitRow_opts.h"
#include "SkBlend_opts.h"
#include "SkRasterPipeline_opts.h"

namespace SkOpts {
    void Init_sse41() {
        box_blur_xx          = sse41::box_blur_xx;
        box_blur_xy          = sse41::box_blur_xy;
        box_blur_yx          = sse41::box_blur_yx;
        srcover_srgb_srgb    = sse41::srcover_srgb_srgb;
        blit_row_s32a_opaque = sse41::blit_row_s32a_opaque;

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

    }
}
