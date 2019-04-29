/*
 * Copyright 2018 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/core/SkColorSpace.h"
#include "include/third_party/skcms/skcms.h"
#include "src/core/SkColorSpaceXformSteps.h"
#include "tests/Test.h"

DEF_TEST(SkColorSpaceXformSteps_vs_skcms, r) {
    auto srgb = SkColorSpace::MakeSRGB();
    auto dp3  = SkColorSpace::MakeRGB(SkNamedTransferFn::kSRGB, SkNamedGamut::kDCIP3);

    skcms_ICCProfile srgb_profile;
    srgb->toProfile(&srgb_profile);
    skcms_ICCProfile dp3_profile;
    dp3->toProfile(&dp3_profile);

    // These colors provide a good spread of interesting test cases.
    SkColor colors[] = {
        0xffff0000, 0x7fff0000, 0x7f7f0000,
        0xff00ff00, 0x7f00ff00, 0x7f007f00,
    };

    for (auto color : colors) {
        auto bgra = skcms_PixelFormat_BGRA_8888,
              f32 = skcms_PixelFormat_RGBA_ffff;
        auto unpremul = skcms_AlphaFormat_Unpremul,
               premul = skcms_AlphaFormat_PremulAsEncoded;

        float via_skcms[4];
        skcms_Transform(&color,     bgra, unpremul, &srgb_profile,
                        &via_skcms,  f32,   premul,  &dp3_profile,
                        1);

        SkColorSpaceXformSteps steps(srgb.get(), kUnpremul_SkAlphaType,
                                      dp3.get(),   kPremul_SkAlphaType);
        float via_steps[4] = {
            SkColorGetR(color) * (1 / 255.0f),
            SkColorGetG(color) * (1 / 255.0f),
            SkColorGetB(color) * (1 / 255.0f),
            SkColorGetA(color) * (1 / 255.0f),
        };
        steps.apply(via_steps);

        REPORTER_ASSERT(r, fabsf(via_skcms[0] - via_steps[0]) <= 0.005f);
        REPORTER_ASSERT(r, fabsf(via_skcms[1] - via_steps[1]) <= 0.005f);
        REPORTER_ASSERT(r, fabsf(via_skcms[2] - via_steps[2]) <= 0.005f);
        REPORTER_ASSERT(r, fabsf(via_skcms[3] - via_steps[3]) <= 0.0f);

        // Now go back using the other method's inverse transform
        float steps_to_skcms[4];
        skcms_Transform(via_steps,      f32, premul, &dp3_profile,
                        steps_to_skcms, f32, premul, &srgb_profile,
                        1);

        float skcms_to_steps[4] = { via_skcms[0], via_skcms[1], via_skcms[2], via_skcms[3] };
        SkColorSpaceXformSteps inv_steps(dp3.get(), kPremul_SkAlphaType,
                                        srgb.get(), kPremul_SkAlphaType);
        inv_steps.apply(skcms_to_steps);

        REPORTER_ASSERT(r, fabsf(skcms_to_steps[0] - steps_to_skcms[0]) <= 0.005f);
        REPORTER_ASSERT(r, fabsf(skcms_to_steps[1] - steps_to_skcms[1]) <= 0.005f);
        REPORTER_ASSERT(r, fabsf(skcms_to_steps[2] - steps_to_skcms[2]) <= 0.005f);
        REPORTER_ASSERT(r, fabsf(skcms_to_steps[3] - steps_to_skcms[3]) <= 0.0f);
    }
}
