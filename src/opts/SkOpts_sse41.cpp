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
        run_pipeline         = sse41::run_pipeline;
    }
}
