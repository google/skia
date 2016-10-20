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
