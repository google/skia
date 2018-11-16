/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkOpts.h"
#define SK_OPTS_NS ssse3
#include "SkBitmapProcState_opts.h"
#include "SkBlitMask_opts.h"
#include "SkSwizzler_opts.h"
#include "SkXfermode_opts.h"

namespace SkOpts {
    void Init_ssse3() {
        create_xfermode = ssse3::create_xfermode;
        blit_mask_d32_a8 = ssse3::blit_mask_d32_a8;

        RGBA_to_BGRA          = ssse3::RGBA_to_BGRA;
        RGBA_to_rgbA          = ssse3::RGBA_to_rgbA;
        RGBA_to_bgrA          = ssse3::RGBA_to_bgrA;
        RGB_to_RGB1           = ssse3::RGB_to_RGB1;
        RGB_to_BGR1           = ssse3::RGB_to_BGR1;
        gray_to_RGB1          = ssse3::gray_to_RGB1;
        grayA_to_RGBA         = ssse3::grayA_to_RGBA;
        grayA_to_rgbA         = ssse3::grayA_to_rgbA;
        inverted_CMYK_to_RGB1 = ssse3::inverted_CMYK_to_RGB1;
        inverted_CMYK_to_BGR1 = ssse3::inverted_CMYK_to_BGR1;

        S32_alpha_D32_filter_DX  = ssse3::S32_alpha_D32_filter_DX;
    }
}
