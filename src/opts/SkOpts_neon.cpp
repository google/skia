/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkOpts.h"

#define SK_OPTS_NS neon
#include "SkBlurImageFilter_opts.h"
#include "SkFloatingPoint_opts.h"
#include "SkMorphologyImageFilter_opts.h"
#include "SkTextureCompressor_opts.h"
#include "SkUtils_opts.h"
#include "SkXfermode_opts.h"

namespace SkOpts {
    void Init_neon() {
        rsqrt           = neon::rsqrt;
        memset16        = neon::memset16;
        memset32        = neon::memset32;
        create_xfermode = SkCreate4pxXfermode;

        box_blur_xx = neon::box_blur_xx;
        box_blur_xy = neon::box_blur_xy;
        box_blur_yx = neon::box_blur_yx;

        dilate_x = neon::dilate_x;
        dilate_y = neon::dilate_y;
         erode_x = neon::erode_x;
         erode_y = neon::erode_y;

        texture_compressor    = neon::texture_compressor;
        fill_block_dimensions = neon::fill_block_dimensions;
    }
}
