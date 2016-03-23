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
#include "SkMatrix_opts.h"
#include "SkMorphologyImageFilter_opts.h"
#include "SkSwizzler_opts.h"
#include "SkTextureCompressor_opts.h"
#include "SkXfermode_opts.h"

namespace SkOpts {
    void Init_neon() {
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

        blit_row_color32     = sk_neon::blit_row_color32;
        blit_row_s32a_opaque = sk_neon::blit_row_s32a_opaque;

        color_cube_filter_span = sk_neon::color_cube_filter_span;

        matrix_translate       = sk_neon::matrix_translate;
        matrix_scale_translate = sk_neon::matrix_scale_translate;
        matrix_affine          = sk_neon::matrix_affine;

        RGBA_to_BGRA          = sk_neon::RGBA_to_BGRA;
        RGBA_to_rgbA          = sk_neon::RGBA_to_rgbA;
        RGBA_to_bgrA          = sk_neon::RGBA_to_bgrA;
        RGB_to_RGB1           = sk_neon::RGB_to_RGB1;
        RGB_to_BGR1           = sk_neon::RGB_to_BGR1;
        gray_to_RGB1          = sk_neon::gray_to_RGB1;
        grayA_to_RGBA         = sk_neon::grayA_to_RGBA;
        grayA_to_rgbA         = sk_neon::grayA_to_rgbA;
        inverted_CMYK_to_RGB1 = sk_neon::inverted_CMYK_to_RGB1;
        inverted_CMYK_to_BGR1 = sk_neon::inverted_CMYK_to_BGR1;
    }
}
