/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "Resources.h"
#include "SkCodec.h"
#include "SkCodecPriv.h"
#include "SkColorPriv.h"
#include "SkColorSpace.h"
#include "SkColorSpace_Base.h"
#include "SkColorSpaceXform.h"
#include "Test.h"

class ColorSpaceXformTest {
public:
    static std::unique_ptr<SkColorSpaceXform> CreateIdentityXform(const sk_sp<SkGammas>& gammas) {
        // Logically we can pass any matrix here.  For simplicty, pass I(), i.e. D50 XYZ gamut.
        sk_sp<SkColorSpace> space(new SkColorSpace_Base(
                nullptr, kNonStandard_SkGammaNamed, gammas, SkMatrix::I(), nullptr));

        // Use special testing entry point, so we don't skip the xform, even though src == dst.
        return SlowIdentityXform(space.get());
    }
};

static bool almost_equal(int x, int y) {
    return SkTAbs(x - y) <= 1;
}

static void test_identity_xform(skiatest::Reporter* r, const sk_sp<SkGammas>& gammas) {
    // Arbitrary set of 10 pixels
    constexpr int width = 10;
    constexpr uint32_t srcPixels[width] = {
            0xFFABCDEF, 0xFF146829, 0xFF382759, 0xFF184968, 0xFFDE8271,
            0xFF32AB52, 0xFF0383BC, 0xFF000102, 0xFFFFFFFF, 0xFFDDEEFF, };
    uint32_t dstPixels[width];

    // Create and perform an identity xform.
    std::unique_ptr<SkColorSpaceXform> xform = ColorSpaceXformTest::CreateIdentityXform(gammas);
    xform->apply(dstPixels, srcPixels, width, select_xform_format(kN32_SkColorType),
                 SkColorSpaceXform::kBGRA_8888_ColorFormat, kOpaque_SkAlphaType);

    // Since the src->dst matrix is the identity, and the gamma curves match,
    // the pixels should be unchanged.
    for (int i = 0; i < width; i++) {
        REPORTER_ASSERT(r, almost_equal(((srcPixels[i] >>  0) & 0xFF),
                                        SkGetPackedB32(dstPixels[i])));
        REPORTER_ASSERT(r, almost_equal(((srcPixels[i] >>  8) & 0xFF),
                                        SkGetPackedG32(dstPixels[i])));
        REPORTER_ASSERT(r, almost_equal(((srcPixels[i] >> 16) & 0xFF),
                                        SkGetPackedR32(dstPixels[i])));
        REPORTER_ASSERT(r, almost_equal(((srcPixels[i] >> 24) & 0xFF),
                                        SkGetPackedA32(dstPixels[i])));
    }
}

DEF_TEST(ColorSpaceXform_TableGamma, r) {
    // Lookup-table based gamma curves
    constexpr size_t tableSize = 10;
    void* memory = sk_malloc_throw(sizeof(SkGammas) + sizeof(float) * tableSize);
    sk_sp<SkGammas> gammas = sk_sp<SkGammas>(new (memory) SkGammas());
    gammas->fRedType = gammas->fGreenType = gammas->fBlueType = SkGammas::Type::kTable_Type;
    gammas->fRedData.fTable.fSize = gammas->fGreenData.fTable.fSize =
            gammas->fBlueData.fTable.fSize = tableSize;
    gammas->fRedData.fTable.fOffset = gammas->fGreenData.fTable.fOffset =
            gammas->fBlueData.fTable.fOffset = 0;
    float* table = SkTAddOffset<float>(memory, sizeof(SkGammas));

    table[0] = 0.00f;
    table[1] = 0.05f;
    table[2] = 0.10f;
    table[3] = 0.15f;
    table[4] = 0.25f;
    table[5] = 0.35f;
    table[6] = 0.45f;
    table[7] = 0.60f;
    table[8] = 0.75f;
    table[9] = 1.00f;
    test_identity_xform(r, gammas);
}

DEF_TEST(ColorSpaceXform_ParametricGamma, r) {
    // Parametric gamma curves
    void* memory = sk_malloc_throw(sizeof(SkGammas) + sizeof(SkGammas::Params));
    sk_sp<SkGammas> gammas = sk_sp<SkGammas>(new (memory) SkGammas());
    gammas->fRedType = gammas->fGreenType = gammas->fBlueType = SkGammas::Type::kParam_Type;
    gammas->fRedData.fParamOffset = gammas->fGreenData.fParamOffset =
            gammas->fBlueData.fParamOffset = 0;
    SkGammas::Params* params = SkTAddOffset<SkGammas::Params>(memory, sizeof(SkGammas));

    // Interval, switch xforms at 0.0031308f
    params->fD = 0.04045f;

    // First equation:
    params->fE = 1.0f / 12.92f;
    params->fF = 0.0f;

    // Second equation:
    // Note that the function is continuous (it's actually sRGB).
    params->fA = 1.0f / 1.055f;
    params->fB = 0.055f / 1.055f;
    params->fC = 0.0f;
    params->fG = 2.4f;
    test_identity_xform(r, gammas);
}

DEF_TEST(ColorSpaceXform_ExponentialGamma, r) {
    // Exponential gamma curves
    sk_sp<SkGammas> gammas = sk_sp<SkGammas>(new SkGammas());
    gammas->fRedType = gammas->fGreenType = gammas->fBlueType = SkGammas::Type::kValue_Type;
    gammas->fRedData.fValue = gammas->fGreenData.fValue = gammas->fBlueData.fValue = 1.4f;
    test_identity_xform(r, gammas);
}

DEF_TEST(ColorSpaceXform_NamedGamma, r) {
    sk_sp<SkGammas> gammas = sk_sp<SkGammas>(new SkGammas());
    gammas->fRedType = gammas->fGreenType = gammas->fBlueType = SkGammas::Type::kNamed_Type;
    gammas->fRedData.fNamed = kSRGB_SkGammaNamed;
    gammas->fGreenData.fNamed = k2Dot2Curve_SkGammaNamed;
    gammas->fBlueData.fNamed = kLinear_SkGammaNamed;
    test_identity_xform(r, gammas);
}

DEF_TEST(ColorSpaceXform_NonMatchingGamma, r) {
    constexpr size_t tableSize = 10;
    void* memory = sk_malloc_throw(sizeof(SkGammas) + sizeof(float) * tableSize +
                                   sizeof(SkGammas::Params));
    sk_sp<SkGammas> gammas = sk_sp<SkGammas>(new (memory) SkGammas());

    float* table = SkTAddOffset<float>(memory, sizeof(SkGammas));
    table[0] = 0.00f;
    table[1] = 0.15f;
    table[2] = 0.20f;
    table[3] = 0.25f;
    table[4] = 0.35f;
    table[5] = 0.45f;
    table[6] = 0.55f;
    table[7] = 0.70f;
    table[8] = 0.85f;
    table[9] = 1.00f;

    SkGammas::Params* params = SkTAddOffset<SkGammas::Params>(memory, sizeof(SkGammas) +
                                                              sizeof(float) * tableSize);
    params->fA = 1.0f / 1.055f;
    params->fB = 0.055f / 1.055f;
    params->fC = 0.0f;
    params->fD = 0.04045f;
    params->fE = 1.0f / 12.92f;
    params->fF = 0.0f;
    params->fG = 2.4f;

    gammas->fRedType = SkGammas::Type::kValue_Type;
    gammas->fRedData.fValue = 1.2f;

    gammas->fGreenType = SkGammas::Type::kTable_Type;
    gammas->fGreenData.fTable.fSize = tableSize;
    gammas->fGreenData.fTable.fOffset = 0;

    gammas->fBlueType = SkGammas::Type::kParam_Type;
    gammas->fBlueData.fParamOffset = sizeof(float) * tableSize;

    test_identity_xform(r, gammas);
}
