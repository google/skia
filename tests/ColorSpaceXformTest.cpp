/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "Resources.h"
#include "SkCodec.h"
#include "SkColorPriv.h"
#include "SkColorSpace.h"
#include "SkColorSpace_Base.h"
#include "SkColorSpaceXform.h"
#include "Test.h"

class ColorSpaceXformTest {
public:
    static SkDefaultXform* CreateDefaultXform(const sk_sp<SkGammas>& srcGamma,
            const SkMatrix44& srcToDst, const sk_sp<SkGammas>& dstGamma) {
        return new SkDefaultXform(srcGamma, srcToDst, dstGamma);
    }
};

static void test_xform(skiatest::Reporter* r, const sk_sp<SkGammas>& gammas) {
    // Arbitrary set of 10 pixels
    constexpr int width = 10;
    constexpr uint32_t srcPixels[width] = {
            0xFFABCDEF, 0xFF146829, 0xFF382759, 0xFF184968, 0xFFDE8271,
            0xFF32AB52, 0xFF0383BC, 0xFF000000, 0xFFFFFFFF, 0xFFDDEEFF, };
    uint32_t dstPixels[width];

    // Identity matrix
    SkMatrix44 srcToDst = SkMatrix44::I();

    // Create and perform xform
    std::unique_ptr<SkColorSpaceXform> xform(
            ColorSpaceXformTest::CreateDefaultXform(gammas, srcToDst, gammas));
    xform->xform_RGB1_8888(dstPixels, srcPixels, width);

    // Since the matrix is the identity, and the gamma curves match, the pixels
    // should be unchanged.
    for (int i = 0; i < width; i++) {
        // TODO (msarett):
        // As the implementation changes, we may want to use a tolerance here.
        REPORTER_ASSERT(r, ((srcPixels[i] >>  0) & 0xFF) == SkGetPackedR32(dstPixels[i]));
        REPORTER_ASSERT(r, ((srcPixels[i] >>  8) & 0xFF) == SkGetPackedG32(dstPixels[i]));
        REPORTER_ASSERT(r, ((srcPixels[i] >> 16) & 0xFF) == SkGetPackedB32(dstPixels[i]));
        REPORTER_ASSERT(r, ((srcPixels[i] >> 24) & 0xFF) == SkGetPackedA32(dstPixels[i]));
    }
}

DEF_TEST(ColorSpaceXform_TableGamma, r) {
    // Lookup-table based gamma curves
    SkGammaCurve red, green, blue;
    constexpr size_t tableSize = 10;
    red.fTable = std::unique_ptr<float[]>(new float[tableSize]);
    green.fTable = std::unique_ptr<float[]>(new float[tableSize]);
    blue.fTable = std::unique_ptr<float[]>(new float[tableSize]);
    red.fTableSize = green.fTableSize = blue.fTableSize = 10;
    red.fTable[0] = green.fTable[0] = blue.fTable[0] = 0.00f;
    red.fTable[1] = green.fTable[1] = blue.fTable[1] = 0.05f;
    red.fTable[2] = green.fTable[2] = blue.fTable[2] = 0.10f;
    red.fTable[3] = green.fTable[3] = blue.fTable[3] = 0.15f;
    red.fTable[4] = green.fTable[4] = blue.fTable[4] = 0.25f;
    red.fTable[5] = green.fTable[5] = blue.fTable[5] = 0.35f;
    red.fTable[6] = green.fTable[6] = blue.fTable[6] = 0.45f;
    red.fTable[7] = green.fTable[7] = blue.fTable[7] = 0.60f;
    red.fTable[8] = green.fTable[8] = blue.fTable[8] = 0.75f;
    red.fTable[9] = green.fTable[9] = blue.fTable[9] = 1.00f;
    sk_sp<SkGammas> gammas =
            sk_make_sp<SkGammas>(std::move(red), std::move(green), std::move(blue));
    test_xform(r, gammas);
}

DEF_TEST(ColorSpaceXform_ParametricGamma, r) {
    // Parametric gamma curves
    SkGammaCurve red, green, blue;

    // Interval, switch xforms at 0.5f
    red.fD = green.fD = blue.fD = 0.5f;

    // First equation, Y = 0.5f * X
    red.fE = green.fE = blue.fE = 0.5f;

    // Second equation, Y = ((1.0f * X) + 0.0f) ^ 3.0f + 0.125f
    // Note that the function is continuous:
    // 0.5f * 0.5f = ((1.0f * 0.5f) + 0.0f) ^ 3.0f + 0.125f = 0.25f
    red.fA = green.fA = blue.fA = 1.0f;
    red.fB = green.fB = blue.fB = 0.0f;
    red.fC = green.fC = blue.fC = 0.125f;
    red.fG = green.fG = blue.fG = 3.0f;
    sk_sp<SkGammas> gammas = sk_make_sp<SkGammas>(std::move(red), std::move(green), std::move(blue));
    test_xform(r, gammas);
}

DEF_TEST(ColorSpaceXform_ExponentialGamma, r) {
    // Exponential gamma curves
    SkGammaCurve red, green, blue;
    red.fValue = green.fValue = blue.fValue = 4.0f;
    sk_sp<SkGammas> gammas =
            sk_make_sp<SkGammas>(std::move(red), std::move(green), std::move(blue));
    test_xform(r, gammas);
}
