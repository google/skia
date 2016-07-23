/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkOpts.h"

#define SK_OPTS_NS sk_sse41
#include "SkBlurImageFilter_opts.h"
#include "SkBlitRow_opts.h"
#include "SkBlend_opts.h"
#include "SkColorXform_opts.h"

namespace SkOpts {
    void Init_sse41() {
        box_blur_xx          = sk_sse41::box_blur_xx;
        box_blur_xy          = sk_sse41::box_blur_xy;
        box_blur_yx          = sk_sse41::box_blur_yx;
        srcover_srgb_srgb    = sk_sse41::srcover_srgb_srgb;
        blit_row_s32a_opaque = sk_sse41::blit_row_s32a_opaque;

        color_xform_RGB1_srgb_to_2dot2  = sk_sse41::color_xform_RGB1_srgb_to_2dot2;
        color_xform_RGB1_2dot2_to_2dot2 = sk_sse41::color_xform_RGB1_2dot2_to_2dot2;
        color_xform_RGB1_srgb_to_srgb   = sk_sse41::color_xform_RGB1_srgb_to_srgb;
        color_xform_RGB1_2dot2_to_srgb  = sk_sse41::color_xform_RGB1_2dot2_to_srgb;
    }
}
