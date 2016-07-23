/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkOpts.h"
#define SK_OPTS_NS sk_ssse3
#include "SkBlitMask_opts.h"
#include "SkColorCubeFilter_opts.h"
#include "SkSwizzler_opts.h"
#include "SkXfermode_opts.h"

namespace SkOpts {
    void Init_ssse3() {
        create_xfermode = sk_ssse3::create_xfermode;
        blit_mask_d32_a8 = sk_ssse3::blit_mask_d32_a8;
        color_cube_filter_span = sk_ssse3::color_cube_filter_span;

        RGBA_to_BGRA          = sk_ssse3::RGBA_to_BGRA;
        RGBA_to_rgbA          = sk_ssse3::RGBA_to_rgbA;
        RGBA_to_bgrA          = sk_ssse3::RGBA_to_bgrA;
        RGB_to_RGB1           = sk_ssse3::RGB_to_RGB1;
        RGB_to_BGR1           = sk_ssse3::RGB_to_BGR1;
        gray_to_RGB1          = sk_ssse3::gray_to_RGB1;
        grayA_to_RGBA         = sk_ssse3::grayA_to_RGBA;
        grayA_to_rgbA         = sk_ssse3::grayA_to_rgbA;
        inverted_CMYK_to_RGB1 = sk_ssse3::inverted_CMYK_to_RGB1;
        inverted_CMYK_to_BGR1 = sk_ssse3::inverted_CMYK_to_BGR1;
    }
}
