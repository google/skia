/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkOpts.h"

#define SK_OPTS_NS sk_neon
#include "SkBlitMask_opts.h"
#include "SkBlitRow_opts.h"
#include "SkBlurImageFilter_opts.h"
#include "SkColorCubeFilter_opts.h"
#include "SkFloatingPoint_opts.h"
#include "SkMatrix_opts.h"
#include "SkMorphologyImageFilter_opts.h"
#include "SkSwizzler_opts.h"
#include "SkTextureCompressor_opts.h"
#include "SkUtils_opts.h"
#include "SkXfermode_opts.h"

namespace SkOpts {
    void Init_neon() {
        rsqrt           = sk_neon::rsqrt;
        memset16        = sk_neon::memset16;
        memset32        = sk_neon::memset32;
        create_xfermode = sk_neon::create_xfermode;

        box_blur_xx = sk_neon::box_blur_xx;
        box_blur_xy = sk_neon::box_blur_xy;
        box_blur_yx = sk_neon::box_blur_yx;

        dilate_x = sk_neon::dilate_x;
        dilate_y = sk_neon::dilate_y;
         erode_x = sk_neon::erode_x;
         erode_y = sk_neon::erode_y;

        texture_compressor    = sk_neon::texture_compressor;
        fill_block_dimensions = sk_neon::fill_block_dimensions;

        blit_mask_d32_a8 = sk_neon::blit_mask_d32_a8;

        blit_row_color32 = sk_neon::blit_row_color32;

        color_cube_filter_span = sk_neon::color_cube_filter_span;

        matrix_translate       = sk_neon::matrix_translate;
        matrix_scale_translate = sk_neon::matrix_scale_translate;
        matrix_affine          = sk_neon::matrix_affine;

        premul_xxxa        = sk_neon::premul_xxxa;
        premul_swaprb_xxxa = sk_neon::premul_swaprb_xxxa;
        swaprb_xxxa        = sk_neon::swaprb_xxxa;
    }
}
