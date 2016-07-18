/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkCpu.h"
#include "SkHalf.h"
#include "SkOnce.h"
#include "SkOpts.h"

#define SK_OPTS_NS sk_default
#include "SkBlend_opts.h"
#include "SkBlitMask_opts.h"
#include "SkBlitRow_opts.h"
#include "SkBlurImageFilter_opts.h"
#include "SkColorCubeFilter_opts.h"
#include "SkColorXform_opts.h"
#include "SkMorphologyImageFilter_opts.h"
#include "SkSwizzler_opts.h"
#include "SkTextureCompressor_opts.h"
#include "SkXfermode_opts.h"

namespace SK_OPTS_NS {
    static void float_to_half(uint16_t dst[], const float src[], int n) {
        while (n-->0) {
            *dst++ = SkFloatToHalf(*src++);
        }
    }
    static void half_to_float(float dst[], const uint16_t src[], int n) {
        while (n-->0) {
            *dst++ = SkHalfToFloat(*src++);
        }
    }
}

namespace SkOpts {

    // Define default function pointer values here...
    // If our global compile options are set high enough, these defaults might even be
    // CPU-specialized, e.g. a typical x86-64 machine might start with SSE2 defaults.
    // They'll still get a chance to be replaced with even better ones, e.g. using SSE4.1.
    decltype(create_xfermode) create_xfermode = sk_default::create_xfermode;
    decltype(color_cube_filter_span) color_cube_filter_span = sk_default::color_cube_filter_span;

    decltype(box_blur_xx) box_blur_xx = sk_default::box_blur_xx;
    decltype(box_blur_xy) box_blur_xy = sk_default::box_blur_xy;
    decltype(box_blur_yx) box_blur_yx = sk_default::box_blur_yx;

    decltype(dilate_x) dilate_x = sk_default::dilate_x;
    decltype(dilate_y) dilate_y = sk_default::dilate_y;
    decltype( erode_x)  erode_x = sk_default::erode_x;
    decltype( erode_y)  erode_y = sk_default::erode_y;

    decltype(texture_compressor)       texture_compressor = sk_default::texture_compressor;
    decltype(fill_block_dimensions) fill_block_dimensions = sk_default::fill_block_dimensions;

    decltype(blit_mask_d32_a8) blit_mask_d32_a8 = sk_default::blit_mask_d32_a8;

    decltype(blit_row_color32)     blit_row_color32     = sk_default::blit_row_color32;
    decltype(blit_row_s32a_opaque) blit_row_s32a_opaque = sk_default::blit_row_s32a_opaque;

    decltype(RGBA_to_BGRA)          RGBA_to_BGRA          = sk_default::RGBA_to_BGRA;
    decltype(RGBA_to_rgbA)          RGBA_to_rgbA          = sk_default::RGBA_to_rgbA;
    decltype(RGBA_to_bgrA)          RGBA_to_bgrA          = sk_default::RGBA_to_bgrA;
    decltype(RGB_to_RGB1)           RGB_to_RGB1           = sk_default::RGB_to_RGB1;
    decltype(RGB_to_BGR1)           RGB_to_BGR1           = sk_default::RGB_to_BGR1;
    decltype(gray_to_RGB1)          gray_to_RGB1          = sk_default::gray_to_RGB1;
    decltype(grayA_to_RGBA)         grayA_to_RGBA         = sk_default::grayA_to_RGBA;
    decltype(grayA_to_rgbA)         grayA_to_rgbA         = sk_default::grayA_to_rgbA;
    decltype(inverted_CMYK_to_RGB1) inverted_CMYK_to_RGB1 = sk_default::inverted_CMYK_to_RGB1;
    decltype(inverted_CMYK_to_BGR1) inverted_CMYK_to_BGR1 = sk_default::inverted_CMYK_to_BGR1;

    decltype(half_to_float) half_to_float = sk_default::half_to_float;
    decltype(float_to_half) float_to_half = sk_default::float_to_half;

    decltype(srcover_srgb_srgb) srcover_srgb_srgb = sk_default::srcover_srgb_srgb;

    decltype(color_xform_RGB1_srgb_to_2dot2)  color_xform_RGB1_srgb_to_2dot2  =
            sk_default::color_xform_RGB1_srgb_to_2dot2;
    decltype(color_xform_RGB1_2dot2_to_2dot2) color_xform_RGB1_2dot2_to_2dot2 =
            sk_default::color_xform_RGB1_2dot2_to_2dot2;
    decltype(color_xform_RGB1_srgb_to_srgb)   color_xform_RGB1_srgb_to_srgb   =
            sk_default::color_xform_RGB1_srgb_to_srgb;
    decltype(color_xform_RGB1_2dot2_to_srgb)  color_xform_RGB1_2dot2_to_srgb  =
            sk_default::color_xform_RGB1_2dot2_to_srgb;

    // Each Init_foo() is defined in src/opts/SkOpts_foo.cpp.
    void Init_ssse3();
    void Init_sse41();
    void Init_sse42() {}
    void Init_avx();
    void Init_avx2() {}

    static void init() {
    #if defined(SK_CPU_X86) && !defined(SK_BUILD_NO_OPTS)
        if (SkCpu::Supports(SkCpu::SSSE3)) { Init_ssse3(); }
        if (SkCpu::Supports(SkCpu::SSE41)) { Init_sse41(); }
        if (SkCpu::Supports(SkCpu::SSE42)) { Init_sse42(); }
        if (SkCpu::Supports(SkCpu::AVX  )) { Init_avx();   }
        if (SkCpu::Supports(SkCpu::AVX2 )) { Init_avx2();  }
    #endif
    }

    void Init() {
        static SkOnce once;
        once(init);
    }
}  // namespace SkOpts
