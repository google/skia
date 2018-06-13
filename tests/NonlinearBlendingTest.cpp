/*
 * Copyright 2018 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "../third_party/skcms/skcms.h"
#include "SkCanvas.h"
#include "SkColorSpace.h"
#include "SkHalf.h"
#include "SkSurface.h"
#include "Test.h"

DEF_TEST(NonlinearBlending, r) {

    auto srgb = SkColorSpace::MakeSRGB();
    auto dp3  = SkColorSpace::MakeRGB(SkColorSpace::kSRGB_RenderTargetGamma,
                                      SkColorSpace::kDCIP3_D65_Gamut);
    sk_sp<SkColorSpace> color_spaces[] = { srgb, dp3 };

    // The cross-product of these colors provides a good spread of interesting test cases.
    SkColor colors[] = {
        0xffff0000, 0x7fff0000, 0x7f7f0000,
        0xff00ff00, 0x7f00ff00, 0x7f007f00,
    };

    skcms_ICCProfile srgb_profile;
    srgb->toProfile(&srgb_profile);

    for (auto src_color : colors)
    for (auto dst_color : colors) {
        for (auto dst_cs : color_spaces) {

            skcms_ICCProfile dst_profile;
            dst_cs->toProfile(&dst_profile);

            float src_float[4],
                  dst_float[4];

            auto bgra = skcms_PixelFormat_BGRA_8888,
                  f16 = skcms_PixelFormat_RGBA_hhhh,
                  f32 = skcms_PixelFormat_RGBA_ffff;
            auto unpremul = skcms_AlphaFormat_Unpremul,
                   premul = skcms_AlphaFormat_PremulAsEncoded;

            skcms_Transform(&src_color, bgra, unpremul, &srgb_profile,
                            &src_float,  f32,   premul,  &dst_profile,
                            1);
            skcms_Transform(&dst_color, bgra, unpremul, &srgb_profile,
                            &dst_float,  f32,   premul,  &dst_profile,
                            1);

            // srcover (nonlinear) blend
            float nonlinear_blend[4] = {
                src_float[0] + dst_float[0]*(1 - src_float[3]),
                src_float[1] + dst_float[1]*(1 - src_float[3]),
                src_float[2] + dst_float[2]*(1 - src_float[3]),
                src_float[3] + dst_float[3]*(1 - src_float[3]),
            }, back_in_srgb[4];
            skcms_Transform(nonlinear_blend, f32, premul,  &dst_profile,
                            back_in_srgb,    f32, premul, &srgb_profile,
                            1);

            auto info = SkImageInfo::Make(1,1, kRGBA_F16_SkColorType
                                             , kPremul_SkAlphaType
                                             , dst_cs);
            auto surf = SkSurface::MakeRaster(info);
            surf->getCanvas()->clear    (dst_color);
            surf->getCanvas()->drawColor(src_color);

            SkHalf drawn[4];
            REPORTER_ASSERT(r, surf->readPixels(info, &drawn, sizeof(drawn), 0,0));

            float drawn_srgb[4];
            skcms_Transform(&drawn    , f16, premul, &dst_profile,
                            drawn_srgb, f32, premul, &srgb_profile,
                            1);

            SkDebugf("%.3f %.3f %.3f %.3f vs. %.3f %.3f %.3f %.3f\n",
                     back_in_srgb[0], back_in_srgb[1], back_in_srgb[2], back_in_srgb[3],
                       drawn_srgb[0],   drawn_srgb[1],   drawn_srgb[2],   drawn_srgb[3]);
        }
        SkDebugf("~~~~~~~~~~~~~~\n");
    }

}
