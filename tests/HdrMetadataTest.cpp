/*
 * Copyright 2025 Google LLC.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/core/SkBitmap.h"
#include "include/core/SkCanvas.h"
#include "include/core/SkColorFilter.h"
#include "include/core/SkData.h"
#include "include/core/SkImage.h"
#include "include/core/SkScalar.h"
#include "include/private/SkHdrMetadata.h"
#include "src/codec/SkHdrAgtmPriv.h"
#include "tests/Test.h"

DEF_TEST(HdrMetadata_ParseSerialize_ContentLightLevelInformation, r) {
    uint8_t data[] = {
        0x03, 0xE8,
        0x00, 0xFA,
    };
    // Data taken from:
    // https://www.w3.org/TR/png-3/#example-13
    // https://www.w3.org/TR/png-3/#example-14
    uint8_t dataPng[] = {
        0x00, 0x98, 0x96, 0x80,
        0x00, 0x26, 0x25, 0xA0,
    };
    skhdr::ContentLightLevelInformation clliExpected = {
        1000.f, 250.f,
    };
    auto skData = SkData::MakeWithoutCopy(data, sizeof(data));
    auto skDataPng = SkData::MakeWithoutCopy(dataPng, sizeof(dataPng));

    skhdr::ContentLightLevelInformation clli;
    REPORTER_ASSERT(r, clli.parse(skData.get()));
    REPORTER_ASSERT(r, clli == clliExpected);
    REPORTER_ASSERT(r, skData->equals(clli.serialize().get()));

    skhdr::ContentLightLevelInformation clliPng;
    REPORTER_ASSERT(r, clliPng.parsePngChunk(skDataPng.get()));
    REPORTER_ASSERT(r, clliPng == clliExpected);
    REPORTER_ASSERT(r, skDataPng->equals(clli.serializePngChunk().get()));
}

DEF_TEST(HdrMetadata_ParseSerialize_MasteringDisplayColorVolume, r) {
    // Data taken from:
    // https://www.w3.org/TR/png-3/#example-5
    // https://www.w3.org/TR/png-3/#example-6
    // https://www.w3.org/TR/png-3/#example-7
    // https://www.w3.org/TR/png-3/#example-8
    uint8_t data[] = {
        0x8A, 0x48, 0x39, 0x08, // Red
        0x21, 0x34, 0x9B, 0xAA, // Green
        0x19, 0x96, 0x08, 0xFC, // Blue
        0x3D, 0x13, 0x40, 0x42, // White
        0x02, 0x62, 0x5A, 0x00, // Maximum luminance
        0x00, 0x00, 0x00, 0x05, // Minimum luminance
    };
    skhdr::MasteringDisplayColorVolume mdcvExpected = {
        {0.708f, 0.292f, 0.17f, 0.797f, 0.131f, 0.046f, 0.3127f, 0.329f},
        4000.f, 0.0005f,
    };
    auto skData = SkData::MakeWithoutCopy(data, sizeof(data));

    skhdr::MasteringDisplayColorVolume mdcv;
    REPORTER_ASSERT(r, mdcv.parse(skData.get()));
    REPORTER_ASSERT(r, mdcv == mdcvExpected);
    REPORTER_ASSERT(r, skData->equals(mdcv.serialize().get()));
}

DEF_TEST(HdrMetadata_Agtm_Cubic, r) {
    skhdr::AgtmImpl::PiecewiseCubicFunction cubic = {
        10,
        {0.10720647f, 0.76246667f, 1.39535723f, 2.17572099f, 2.47834070f,
         3.14288223f, 3.35428070f, 4.24864607f, 4.59087493f, 4.80373641f},
        {0.37384606f, 0.93143060f, 0.f,         1.23009354f, 1.25542898f,
         2.22460677f, 2.69226748f, 3.45838813f, 4.44597502f, 5.19196203f},
        {0.},
    };
    cubic.populateSlopeFromPCHIP();

    const float mExpected[10] = { 2.03242568f, 0.f,         0.f,         0.14042951f, 0.14250506f,
                                  1.82245618f, 1.35855757f, 1.43703564f, 3.18918733f, 3.74186390f};
    for (size_t i = 0; i < 10; ++i) {
        REPORTER_ASSERT(r, SkScalarNearlyEqual(cubic.fM[i], mExpected[i], 0.0001f));
    }

    const float yExpected[11] = {
        0.37384606f, 0.86280187f, 0.63630745f, 0.05871820f, 1.05625216f,
        1.26009455f, 1.95243885f, 2.85680727f, 3.19521825f, 4.14318213f,
        5.13419092f};
    for (size_t i = 0; i < 11; ++i) {
        const float x = i / 2.f;
        const float y = cubic.evaluate(x);
        REPORTER_ASSERT(r, SkScalarNearlyEqual(y, yExpected[i], 0.0001f));
    }
}

DEF_TEST(HdrMetadata_Agtm_Mix, r) {
    auto test = [&r](const std::string& name, skhdr::AgtmImpl::ComponentMixingFunction mix,
                     SkColor4f input, SkColor4f expected) {
        skiatest::ReporterContext ctx(r, name);
        SkColor4f actual = mix.evaluate(input);
        REPORTER_ASSERT(r, actual.fR == expected.fR);
        REPORTER_ASSERT(r, actual.fG == expected.fG);
        REPORTER_ASSERT(r, actual.fB == expected.fB);
        REPORTER_ASSERT(r, actual.fA == expected.fA);
        REPORTER_ASSERT(r, actual.fA == input.fA);
    };

    test("Red only",
         skhdr::AgtmImpl::ComponentMixingFunction({.fRed=1.f}),
         SkColor4f({0.5f, 0.75f, 0.25f, 1.f}),
         SkColor4f({0.5f, 0.5f,  0.5f,  1.f}));

    test("Green only",
         skhdr::AgtmImpl::ComponentMixingFunction({.fGreen=1.f}),
         SkColor4f({0.75f, 0.5f, 0.25f, 1.f}),
         SkColor4f({0.5f,  0.5f, 0.5f,  1.f}));

    test("Blue only",
         skhdr::AgtmImpl::ComponentMixingFunction({.fBlue=1.f}),
         SkColor4f({0.75f, 0.25f, 0.5f, 1.f}),
         SkColor4f({0.5f,  0.5f,  0.5f,  1.f}));

    test("Max only",
         skhdr::AgtmImpl::ComponentMixingFunction({.fMax=1.f}),
         SkColor4f({0.75f, 0.5f,  0.25f, 1.f}),
         SkColor4f({0.75f, 0.75f, 0.75f, 1.f}));

    test("Min only",
         skhdr::AgtmImpl::ComponentMixingFunction({.fMin=1.f}),
         SkColor4f({0.75f, 0.5f,  0.25f, 1.f}),
         SkColor4f({0.25f, 0.25f, 0.25f, 1.f}));

    test("Component only",
         skhdr::AgtmImpl::ComponentMixingFunction({.fComponent=1.f}),
         SkColor4f({0.75f, 0.5f, 0.25f, 1.f}),
         SkColor4f({0.75f, 0.5f, 0.25f, 1.f}));

    test("CIE Y (luminance)",
         skhdr::AgtmImpl::ComponentMixingFunction({.fRed=0.2627f, .fGreen=0.6780f, .fBlue=0.0593f}),
         SkColor4f({0.75f,    0.5f,     0.25f,    0.125f}),
         SkColor4f({0.55085f, 0.55085f, 0.55085f, 0.125f}));

    test("max-component",
         skhdr::AgtmImpl::ComponentMixingFunction({.fMax=0.75f, .fComponent=0.25f}),
         SkColor4f({0.75f, 0.5f,    0.25f,   0.125f}),
         SkColor4f({0.75f, 0.6875f, 0.6250f, 0.125f}));

}

DEF_TEST(HdrMetadata_Agtm_RWTMO, r) {
    skhdr::AgtmImpl agtm;
    agtm.fBaselineHdrHeadroom = 1.f;
    agtm.populateUsingRwtmo();

    REPORTER_ASSERT(r, memcmp(&agtm.fGainApplicationSpacePrimaries, &SkNamedPrimaries::kRec2020,
                              sizeof(SkColorSpacePrimaries)) == 0);
    REPORTER_ASSERT(r, agtm.fNumAlternateImages == 2);
    REPORTER_ASSERT(r, agtm.fAlternateHdrHeadroom[0] == 0.f);
    REPORTER_ASSERT(r, SkScalarNearlyEqual(agtm.fAlternateHdrHeadroom[1], 0.6151137835929048f));

    const float xExpected[2][8] = {
        {1.00000f, 1.06461f, 1.15531f, 1.27209f, 1.41494f, 1.58388f, 1.77890f, 2.00000f},
        {1.00000f, 1.10504f, 1.22269f, 1.35294f, 1.49580f, 1.65126f, 1.81933f, 2.00000f},
    };
    const float yExpected[2][8] = {
        {-0.35356f, -0.37367f, -0.42913f, -0.51246f, -0.61663f, -0.73563f, -0.86465f, -1.00000f},
        { 0.00000f, -0.01253f, -0.04583f, -0.09477f, -0.15559f, -0.22550f, -0.30244f, -0.38489f},
    };
    const float mExpected[2][8] = {
        {0.00000f, -0.50266f, -0.68079f, -0.73059f, -0.72159f, -0.68535f, -0.63784f, -0.58742f},
        {0.00000f, -0.21470f, -0.33759f, -0.40581f, -0.44088f, -0.45573f, -0.45828f, -0.45351f},
    };

    for (size_t a = 0; a < 2; ++a) {
        const auto& cubic = agtm.fGainFunction[a].fPiecewiseCubic;
        REPORTER_ASSERT(r, cubic.fNumControlPoints == 8);
        for (size_t c = 0; c < 8; ++c) {
            REPORTER_ASSERT(r, SkScalarNearlyEqual(xExpected[a][c], cubic.fX[c]));
            REPORTER_ASSERT(r, SkScalarNearlyEqual(yExpected[a][c], cubic.fY[c]));
            REPORTER_ASSERT(r, SkScalarNearlyEqual(mExpected[a][c], cubic.fM[c]));
        }
    }
}

DEF_TEST(HdrMetadata_Agtm_Weighting, r) {
    skhdr::AgtmImpl agtm;

    auto test = [&r, &agtm](const std::string& name,
                            float targetedHdrHeadroom,
                            const skhdr::AgtmImpl::Weighting& wExpected) {
        skiatest::ReporterContext ctx(r, name);
        skhdr::AgtmImpl::Weighting w = agtm.computeWeighting(targetedHdrHeadroom);
        REPORTER_ASSERT(r, w.fWeight[0] == wExpected.fWeight[0]);
        REPORTER_ASSERT(r, w.fWeight[1] == wExpected.fWeight[1]);
        REPORTER_ASSERT(r, w.fAlternateImageIndex[0] == wExpected.fAlternateImageIndex[0]);
        REPORTER_ASSERT(r, w.fAlternateImageIndex[1] == wExpected.fAlternateImageIndex[1]);
    };

    // Tests with a single baseline representation.
    agtm.fBaselineHdrHeadroom = 1.f;
    test("base-1, target-0", 0.f,
         {{skhdr::AgtmImpl::Weighting::kInvalidIndex, skhdr::AgtmImpl::Weighting::kInvalidIndex},
          {0.f, 0.f}});

    test("base-1, target-1", 1.f,
         {{skhdr::AgtmImpl::Weighting::kInvalidIndex, skhdr::AgtmImpl::Weighting::kInvalidIndex},
          {0.f, 0.f}});

    test("base-2, target-2", 2.f,
         {{skhdr::AgtmImpl::Weighting::kInvalidIndex, skhdr::AgtmImpl::Weighting::kInvalidIndex},
          {0.f, 0.f}});

    // Tests with a baseline and an alternate representation.
    agtm.fBaselineHdrHeadroom = 1.f;
    agtm.fNumAlternateImages = 1;
    agtm.fAlternateHdrHeadroom[0] = 0.f;

    test("base-1-alt0, target-0", 0.f,
         {{0, skhdr::AgtmImpl::Weighting::kInvalidIndex},
          {1.f, 0.f}});

    test("base-1-alt0, target-0.25", 0.25f,
         {{0, skhdr::AgtmImpl::Weighting::kInvalidIndex},
          {0.75f, 0.f}});

    test("base-1-alt0, target-1", 1.f,
         {{skhdr::AgtmImpl::Weighting::kInvalidIndex, skhdr::AgtmImpl::Weighting::kInvalidIndex},
          {0.f, 0.f}});

    test("base-1-alt0, target-1.25", 1.25f,
         {{skhdr::AgtmImpl::Weighting::kInvalidIndex, skhdr::AgtmImpl::Weighting::kInvalidIndex},
          {0.f, 0.f}});

    // Two alternate representations.
    agtm.fBaselineHdrHeadroom = 1.f;
    agtm.fNumAlternateImages = 2;
    agtm.fAlternateHdrHeadroom[0] = 0.f;
    agtm.fAlternateHdrHeadroom[1] = 2.f;

    test("base-1-alt0-alt2, target-0", 0.f,
         {{0, skhdr::AgtmImpl::Weighting::kInvalidIndex},
          {1.f, 0.f}});

    test("base-1-alt0-alt2, target-0.25", 0.25f,
         {{0, skhdr::AgtmImpl::Weighting::kInvalidIndex},
          {0.75f, 0.f}});

    test("base-1-alt0-alt2, target-1", 1.f,
         {{skhdr::AgtmImpl::Weighting::kInvalidIndex, skhdr::AgtmImpl::Weighting::kInvalidIndex},
          {0.f, 0.f}});

    test("base-1-alt0-alt2, target-1.25", 1.25f,
         {{1, skhdr::AgtmImpl::Weighting::kInvalidIndex},
          {0.25f, 0.f}});

    test("base-1-alt0-alt2, target-2", 2.f,
         {{1, skhdr::AgtmImpl::Weighting::kInvalidIndex},
          {1.f, 0.f}});

    test("base-1-alt0-alt2, target-3", 3.f,
         {{1, skhdr::AgtmImpl::Weighting::kInvalidIndex},
          {1.f, 0.f}});

    // Two alternate representations again, now mix-able.
    agtm.fBaselineHdrHeadroom = 2.f;
    agtm.fNumAlternateImages = 2;
    agtm.fAlternateHdrHeadroom[0] = 0.f;
    agtm.fAlternateHdrHeadroom[1] = 1.f;

    test("base-2-alt0-alt1, target-0.25", 0.25f,
         {{0, 1},
          {0.75f, 0.25f}});
}

static bool operator==(const SkColorSpacePrimaries& a, const SkColorSpacePrimaries& b) {
    return memcmp(&a, &b, sizeof(a)) == 0;
}

static void assert_agtms_equal(skiatest::Reporter* r,
                               const skhdr::AgtmImpl& agtmIn,
                               const skhdr::AgtmImpl& agtmOut) {
    // Allow error for headrooms, x, and y to twice the their encoding step.
    constexpr float kHeadroomError = 2.f * 1.f / 10000.f;
    constexpr float kXError = 2.f * 1.f / 1000.f;
    constexpr float kYError = 2.f * 1.f / 4000.f;
    // Allow a wider error for slope because its encoding is non-uniform.
    constexpr float kMError = 0.005f;

    REPORTER_ASSERT(r, agtmIn.fType == agtmOut.fType);
    REPORTER_ASSERT(r, agtmIn.fHdrReferenceWhite == agtmOut.fHdrReferenceWhite);
    REPORTER_ASSERT(r, SkScalarNearlyEqual(
            agtmIn.fBaselineHdrHeadroom, agtmOut.fBaselineHdrHeadroom, kHeadroomError));
    REPORTER_ASSERT(r, agtmIn.fGainApplicationSpacePrimaries ==
                       agtmOut.fGainApplicationSpacePrimaries);
    REPORTER_ASSERT(r, agtmIn.fNumAlternateImages == agtmOut.fNumAlternateImages);
    if (agtmIn.fNumAlternateImages != agtmOut.fNumAlternateImages) {
        return;
    }
    for (uint8_t a = 0; a < agtmIn.fNumAlternateImages; ++a) {
        skiatest::ReporterContext ctxA(r, SkStringPrintf("AlternateImage:a=%u", a));

        REPORTER_ASSERT(r, SkScalarNearlyEqual(
            agtmIn.fAlternateHdrHeadroom[a], agtmOut.fAlternateHdrHeadroom[a], kHeadroomError));

        auto& mixIn = agtmIn.fGainFunction[a].fComponentMixing;
        auto& mixOut = agtmOut.fGainFunction[a].fComponentMixing;

        REPORTER_ASSERT(r, mixIn.fRed == mixOut.fRed);
        REPORTER_ASSERT(r, mixIn.fGreen == mixOut.fGreen);
        REPORTER_ASSERT(r, mixIn.fBlue == mixOut.fBlue);
        REPORTER_ASSERT(r, mixIn.fMax == mixOut.fMax);
        REPORTER_ASSERT(r, mixIn.fMin == mixOut.fMin);
        REPORTER_ASSERT(r, mixIn.fComponent == mixOut.fComponent);

        auto& curveIn = agtmIn.fGainFunction[a].fPiecewiseCubic;
        auto& curveOut = agtmOut.fGainFunction[a].fPiecewiseCubic;
        REPORTER_ASSERT(r, curveIn.fNumControlPoints == curveOut.fNumControlPoints);
        if (curveIn.fNumControlPoints != curveOut.fNumControlPoints) {
            return;
        }
        for (uint8_t c = 0; c < curveIn.fNumControlPoints; ++c) {
            skiatest::ReporterContext ctxC(r, SkStringPrintf("ControlPoint:c=%u", c));
            REPORTER_ASSERT(r, SkScalarNearlyEqual(curveIn.fX[c], curveOut.fX[c], kXError));
            REPORTER_ASSERT(r, SkScalarNearlyEqual(curveIn.fY[c], curveOut.fY[c], kYError));
            REPORTER_ASSERT(r, SkScalarNearlyEqual(curveIn.fM[c], curveOut.fM[c], kMError));
        }
    }
}

// Test round-trip serialization of AGTM metadata.
DEF_TEST(HdrMetadata_Agtm_Serialize, r) {
    {
        skiatest::ReporterContext ctx(r, "NoAdaptiveToneMap");

        skhdr::AgtmImpl agtmIn;
        agtmIn.fHdrReferenceWhite = 123.f;
        agtmIn.fType = skhdr::AgtmImpl::Type::kNone;

        skhdr::AgtmImpl agtmOut;
        REPORTER_ASSERT(r, agtmOut.parse(agtmIn.serialize().get()));

        assert_agtms_equal(r, agtmIn, agtmOut);
    }

    {
        skiatest::ReporterContext ctx(r, "RWTMO");

        skhdr::AgtmImpl agtmIn;
        agtmIn.fBaselineHdrHeadroom = 1.f;
        agtmIn.populateUsingRwtmo();

        skhdr::AgtmImpl agtmOut;
        REPORTER_ASSERT(r, agtmOut.parse(agtmIn.serialize().get()));

        assert_agtms_equal(r, agtmIn, agtmOut);
    }

    {
        skiatest::ReporterContext ctx(r, "ClampInRec601");

        skhdr::AgtmImpl agtmIn;
        agtmIn.fType = skhdr::AgtmImpl::Type::kCustom;
        agtmIn.fHdrReferenceWhite = 100.f;
        agtmIn.fBaselineHdrHeadroom = 2.f;
        agtmIn.fGainApplicationSpacePrimaries = SkNamedPrimaries::kRec601;
        agtmIn.fNumAlternateImages = 0;

        skhdr::AgtmImpl agtmOut;
        REPORTER_ASSERT(r, agtmOut.parse(agtmIn.serialize().get()));

        assert_agtms_equal(r, agtmIn, agtmOut);
    }

    {
        skiatest::ReporterContext ctx(r, "OneAlternates");

        skhdr::AgtmImpl agtmIn;
        agtmIn.fType = skhdr::AgtmImpl::Type::kCustom;
        agtmIn.fHdrReferenceWhite = 400.f;
        agtmIn.fBaselineHdrHeadroom = 4.f;
        agtmIn.fGainApplicationSpacePrimaries = SkNamedPrimaries::kSMPTE_EG_432_1;
        agtmIn.fNumAlternateImages = 1;
        agtmIn.fAlternateHdrHeadroom[0] = 0.f;
        agtmIn.fGainFunction[0] = {
            .fComponentMixing = {
                .fMax = 1.f,
            },
            .fPiecewiseCubic = {
                .fNumControlPoints = 2u,
                .fX = {1.f, 16.f},
                .fY = {0.f, -4.f},
                .fM = {0.f,  0.f},
            },
        };

        skhdr::AgtmImpl agtmOut;
        REPORTER_ASSERT(r, agtmOut.parse(agtmIn.serialize().get()));

        assert_agtms_equal(r, agtmIn, agtmOut);
    }

    {
        skiatest::ReporterContext ctx(r, "FourAlternates");

        skhdr::AgtmImpl agtmIn;
        agtmIn.fType = skhdr::AgtmImpl::Type::kCustom;
        agtmIn.fHdrReferenceWhite = 400.f;
        agtmIn.fBaselineHdrHeadroom = 2.f;
        agtmIn.fGainApplicationSpacePrimaries = SkNamedPrimaries::kSMPTE_EG_432_1;
        agtmIn.fNumAlternateImages = 4;
        agtmIn.fAlternateHdrHeadroom[0] = 0.f;
        agtmIn.fAlternateHdrHeadroom[1] = 1.f;
        agtmIn.fAlternateHdrHeadroom[2] = 3.f;
        agtmIn.fAlternateHdrHeadroom[3] = 4.f;
        agtmIn.fGainFunction[0] = {
            .fComponentMixing = {
                .fMax = 0.75f,
                .fMin = 0.25f
            },
            .fPiecewiseCubic = {
                .fNumControlPoints = 1u,
                .fX = {0.f},
                .fY = {1.f},
                .fM = {0.f},
            },
        };
        agtmIn.fGainFunction[1] = {
            .fComponentMixing = {
                .fMax = 1.f,
            },
            .fPiecewiseCubic = {
                .fNumControlPoints = 4u,
                .fX = {0.f, 1.f,  2.f,  3.f},
                .fY = {1.f, 0.5f, 0.4f, 0.3f},
                .fM = {0.f, 0.1f, 0.2f, 0.3f},
            },
        };
        agtmIn.fGainFunction[2] = {
            .fComponentMixing = {
                .fComponent = 1.f,
            },
            .fPiecewiseCubic = {
                .fNumControlPoints = 2u,
                .fX = {0.f, 1.f},
                .fY = {1.f, 0.5f},
                .fM = {0.f, 0.1f},
            },
        };
        agtmIn.fGainFunction[3] = {
            .fComponentMixing = {
                .fRed   = 0.3f,
                .fGreen = 0.6f,
                .fBlue  = 0.1f,
            },
            .fPiecewiseCubic = {
                .fNumControlPoints = 3u,
                .fX = {0.f, 1.f,  2.f},
                .fY = {1.f, 0.5f, 0.4f},
                .fM = {0.f, 0.1f, 0.5},
            },
        };

        skhdr::AgtmImpl agtmOut;
        REPORTER_ASSERT(r, agtmOut.parse(agtmIn.serialize().get()));

        assert_agtms_equal(r, agtmIn, agtmOut);
    }
}

// Test the logic to apply the AGTM tone mapping.
DEF_TEST(HdrMetadata_Agtm_Apply_and_Shader, r) {
    // This will tone map several input colors to different targeted HDR headrooms using this
    // RWTMO metadata.
    skhdr::AgtmImpl agtm;
    agtm.fBaselineHdrHeadroom = 2;
    agtm.populateUsingRwtmo();
    agtm.populateGainCurvesXYM();

    // We will use the following input pixel values in gain application color space. These include
    // monochrome and non-monochrome values, as well as values that are less than white (less than
    // 1) and brighter than white (greater than 1).
    constexpr size_t kNumTestColors = 6;
    SkColor4f inputTestColors[kNumTestColors] = {
        {1.00f, 1.00f, 1.00f, 1.f},
        {1.00f, 0.50f, 0.25f, 1.f},
        {4.00f, 4.00f, 4.00f, 1.f},
        {1.00f, 2.00f, 4.00f, 1.f},
        {0.50f, 0.50f, 0.50f, 1.f},
        {2.00f, 2.00f, 2.00f, 1.f},
    };

    // We will test applying the gain for the following targetd HDR headroom values.
    constexpr size_t kNumTests = 5;
    const float testTargetedHdrHeadrooms[kNumTests] = {
        0.f,
        1.f,
        agtm.fAlternateHdrHeadroom[1],
        std::log2(3.f),
        2.f,
    };

    // These are the expected output pixel values for each of the targted HDR headrooms.
    SkColor4f expectedTestColors[kNumTests][kNumTestColors] = {
        {
            {0.565302f, 0.565302f, 0.565302f, 1.f},
            {0.565302f, 0.282651f, 0.141326f, 1.f},
            {1.000000f, 1.000000f, 1.000000f, 1.f},
            {0.250000f, 0.500000f, 1.000000f, 1.f},
            {0.282651f, 0.282651f, 0.282651f, 1.f},
            {0.815278f, 0.815278f, 0.815278f, 1.f},
        },
        {
            {0.898755f, 0.898755f, 0.898755f, 1.f},
            {0.898755f, 0.449377f, 0.224689f, 1.f},
            {2.000000f, 2.000000f, 2.000000f, 1.f},
            {0.500000f, 1.000000f, 2.000000f, 1.f},
            {0.449377f, 0.449377f, 0.449377f, 1.f},
            {1.471569f, 1.471569f, 1.471569f, 1.f},
        },
        {
            {1.000000f, 1.000000f, 1.000000f, 1.f},
            {1.000000f, 0.500000f, 0.250000f, 1.f},
            {2.346040f, 2.346040f, 2.346040f, 1.f},
            {0.586510f, 1.173020f, 2.346040f, 1.f},
            {0.500000f, 0.500000f, 0.500000f, 1.f},
            {1.685886f, 1.685886f, 1.685886f, 1.f},
        },
        {
            {1.000000f, 1.000000f, 1.000000f, 1.f},
            {1.000000f, 0.500000f, 0.250000f, 1.f},
            {3.000000f, 3.000000f, 3.000000f, 1.f},
            {0.750000f, 1.500000f, 3.000000f, 1.f},
            {0.500000f, 0.500000f, 0.500000f, 1.f},
            {1.823991f, 1.823991f, 1.823991f, 1.f},
        },
        {
            {1.00f, 1.00f, 1.00f, 1.f},
            {1.00f, 0.50f, 0.25f, 1.f},
            {4.00f, 4.00f, 4.00f, 1.f},
            {1.00f, 2.00f, 4.00f, 1.f},
            {0.50f, 0.50f, 0.50f, 1.f},
            {2.00f, 2.00f, 2.00f, 1.f},
        },
    };

    // All of the math is done with at least half-precision. Given the range of values we are in
    // (not far from 1), we should maintain at least ten bit precision.
    constexpr float kEpsilon = 1.f/1024.f;

    // Test the Agtm::applyGain function.
    for (size_t t = 0; t < kNumTests; ++t) {
        const auto targetedHdrHeadroom = testTargetedHdrHeadrooms[t];
        skiatest::ReporterContext ctx(r, SkStringPrintf("AgtmImpl::applyGain, targetedHdrHeadroom:%f", targetedHdrHeadroom));

        // Copy the inputTextColors to outputTestColors (because applyGain works in-place).
        SkColor4f outputTestColors[kNumTestColors];
        for (size_t i = 0; i < kNumTestColors; ++i) {
            outputTestColors[i] = inputTestColors[i];
        }

        // Apply the tone mapping gain in-place on outputTestColors.
        agtm.applyGain(SkSpan<SkColor4f>(outputTestColors, kNumTestColors), targetedHdrHeadroom);

        // Verify the result matches expectations.
        for (size_t i = 0; i < kNumTestColors; ++i) {
            const auto& output = outputTestColors[i];
            const auto& expected = expectedTestColors[t][i];

            REPORTER_ASSERT(r, SkScalarNearlyEqual(output.fR, expected.fR, kEpsilon));
            REPORTER_ASSERT(r, SkScalarNearlyEqual(output.fG, expected.fG, kEpsilon));
            REPORTER_ASSERT(r, SkScalarNearlyEqual(output.fB, expected.fB, kEpsilon));
            REPORTER_ASSERT(r, SkScalarNearlyEqual(output.fA, expected.fA, kEpsilon));
        }
    }

    // Test using an SkColorFilter to apply the gain.
    for (size_t t = 0; t < kNumTests; ++t) {
        const auto targetedHdrHeadroom = testTargetedHdrHeadrooms[t];
        skiatest::ReporterContext ctx(r, SkStringPrintf("Agtm::makeColorFilter, targetedHdrHeadroom:%f", targetedHdrHeadroom));

        // The input and output images will be kNumTestColors-by-1.
        const auto info = SkImageInfo::Make(
            kNumTestColors, 1,
            kRGBA_F32_SkColorType, kPremul_SkAlphaType,
            agtm.getGainApplicationSpace());

        // Create an SkImage that references the inputTestColors array directly.
        sk_sp<SkImage> inputImage = SkImages::RasterFromData(
            info,
            SkData::MakeWithoutCopy(inputTestColors, sizeof(inputTestColors)),
            info.minRowBytes());

        // Create an output SkBitmap to draw into.
        SkBitmap bm;
        bm.allocPixels(info);

        // Call drawImage, using the color filter created by Agtm::makeColorFilter.
        {
            SkPaint paint;
            auto colorFilter = agtm.makeColorFilter(targetedHdrHeadroom);
            SkASSERT(colorFilter);
            paint.setColorFilter(colorFilter);
            auto canvas = SkCanvas::MakeRasterDirect(bm.info(), bm.getPixels(), bm.rowBytes());
            canvas->drawImage(inputImage.get(), 0, 0, SkSamplingOptions(), &paint);
        }

        // Verify that the pixels written into the SkBitmap match the expected values.
        for (size_t i = 0; i < kNumTestColors; ++i) {
            const auto& output = *reinterpret_cast<const SkColor4f*>(bm.getAddr(i, 0));
            const auto& expected = expectedTestColors[t][i];
            REPORTER_ASSERT(r, SkScalarNearlyEqual(output.fR, expected.fR, kEpsilon));
            REPORTER_ASSERT(r, SkScalarNearlyEqual(output.fG, expected.fG, kEpsilon));
            REPORTER_ASSERT(r, SkScalarNearlyEqual(output.fB, expected.fB, kEpsilon));
            REPORTER_ASSERT(r, SkScalarNearlyEqual(output.fA, expected.fA, kEpsilon));
        }
    }
}

