/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/core/SkTypes.h"

#include <cmath>

#include "include/core/SkICC.h"
#include "include/core/SkString.h"
#include "modules/skcms/skcms.h"
#include "src/core/SkColorSpacePriv.h"
#include "tests/Test.h"
#include "tools/Resources.h"

DEF_TEST(AdobeRGB, r) {
    if (sk_sp<SkData> profile = GetResourceAsData("icc_profiles/AdobeRGB1998.icc")) {
        skcms_ICCProfile parsed;
        REPORTER_ASSERT(r, skcms_Parse(profile->data(), profile->size(), &parsed));
        REPORTER_ASSERT(r, !parsed.has_CICP);

        auto got  = SkColorSpace::Make(parsed);
        auto want = SkColorSpace::MakeRGB(SkNamedTransferFn::k2Dot2, SkNamedGamut::kAdobeRGB);
        REPORTER_ASSERT(r, SkColorSpace::Equals(got.get(), want.get()));
    }
}

DEF_TEST(HDR_ICC, r) {
    constexpr size_t kTestCount = 2;
    SK_API sk_sp<SkData> profile[kTestCount] = {
            SkWriteICCProfile(SkNamedTransferFn::kPQ, SkNamedGamut::kRec2020),
            SkWriteICCProfile(SkNamedTransferFn::kHLG, SkNamedGamut::kDisplayP3),
    };

    SK_API sk_sp<SkData> dst_profile[kTestCount] = {
            SkWriteICCProfile(SkNamedTransferFn::kLinear, SkNamedGamut::kRec2020),
            SkWriteICCProfile(SkNamedTransferFn::kLinear, SkNamedGamut::kDisplayP3),
    };

    constexpr size_t kPixelCount = 6;

    // clang-format off
    float pixels[kPixelCount][3]{

            { 0.0f, 0.0f, 0.0f, },
            { 0.5f, 0.5f, 0.5f, },
            { 0.5f, 0.0f, 0.0f, },
            { 0.0f, 0.5f, 0.0f, },
            { 0.0f, 0.0f, 0.5f, },
            { 1.0f, 1.0f, 1.0f, },
    };
    float dst_pixels_expected[kTestCount][kPixelCount][3] = {
            {
                    { 0.0000f, 0.0000f, 0.0000f, },
                    { 0.3126f, 0.3125f, 0.3125f, },
                    { 0.4061f, 0.0000f, 0.0000f, },
                    { 0.0000f, 0.3475f, 0.0000f, },
                    { 0.0000f, 0.0000f, 0.4426f, },
                    { 1.0000f, 1.0000f, 1.0000f, },
            },
            {
                    { 0.0000f, 0.0000f, 0.0000f, },
                    { 0.1044f, 0.1044f, 0.1044f, },
                    { 0.1194f, 0.0000f, 0.0000f, },
                    { 0.0000f, 0.1080f, 0.0000f, },
                    { 0.0000f, 0.0000f, 0.1315f, },
                    { 1.0000f, 1.0000f, 1.0000f, },
            },
    };
    // clang-format on
    uint32_t cicp_primaries_expected[kTestCount] = {9, 12};
    uint32_t cicp_trfn_expected[kTestCount] = {16, 18};

    for (size_t test = 0; test < kTestCount; ++test) {
        skcms_ICCProfile parsed;
        REPORTER_ASSERT(r, skcms_Parse(profile[test]->data(), profile[test]->size(), &parsed));
        REPORTER_ASSERT(r, parsed.has_CICP);
        REPORTER_ASSERT(r, parsed.CICP.color_primaries == cicp_primaries_expected[test]);
        REPORTER_ASSERT(r, parsed.CICP.transfer_characteristics == cicp_trfn_expected[test]);
        REPORTER_ASSERT(r, parsed.CICP.matrix_coefficients == 0);
        REPORTER_ASSERT(r, parsed.CICP.video_full_range_flag == 1);

        skcms_ICCProfile dst_parsed;
        REPORTER_ASSERT(
                r, skcms_Parse(dst_profile[test]->data(), dst_profile[test]->size(), &dst_parsed));

        for (size_t pixel = 0; pixel < kPixelCount; ++pixel) {
            float dst_pixel_actual[3]{
                    0.f,
            };
            bool xform_result = skcms_Transform(pixels[pixel],
                                                skcms_PixelFormat_RGB_fff,
                                                skcms_AlphaFormat_Opaque,
                                                &parsed,
                                                dst_pixel_actual,
                                                skcms_PixelFormat_RGB_fff,
                                                skcms_AlphaFormat_Opaque,
                                                &dst_parsed,
                                                1);
            REPORTER_ASSERT(r, xform_result);

            auto approx_equal = [=](float x, float y) { return std::abs(x - y) < 1e-3f; };
            for (size_t i = 0; i < 3; ++i) {
                REPORTER_ASSERT(
                        r, approx_equal(dst_pixel_actual[i], dst_pixels_expected[test][pixel][i]));
            }
            printf("\n");
        }
    }
}
