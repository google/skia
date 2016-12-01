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
#include "SkColorSpace_A2B.h"
#include "SkColorSpace_Base.h"
#include "SkColorSpace_XYZ.h"
#include "SkColorSpaceXform_Base.h"
#include "Test.h"

class ColorSpaceXformTest {
public:
    static std::unique_ptr<SkColorSpaceXform> CreateIdentityXform(const sk_sp<SkGammas>& gammas) {
        // Logically we can pass any matrix here.  For simplicty, pass I(), i.e. D50 XYZ gamut.
        sk_sp<SkColorSpace> space(new SkColorSpace_XYZ(
                kNonStandard_SkGammaNamed, gammas, SkMatrix::I(), nullptr));

        // Use special testing entry point, so we don't skip the xform, even though src == dst.
        return SlowIdentityXform(static_cast<SkColorSpace_XYZ*>(space.get()));
    }

    static std::unique_ptr<SkColorSpaceXform> CreateIdentityXform_A2B(
            SkGammaNamed gammaNamed, const sk_sp<SkGammas>& gammas) {
        std::vector<SkColorSpace_A2B::Element> srcElements;
        // sRGB
        const float values[16] = {
            0.4358f, 0.3853f, 0.1430f, 0.0f,
            0.2224f, 0.7170f, 0.0606f, 0.0f,
            0.0139f, 0.0971f, 0.7139f, 0.0f,
            0.0000f, 0.0000f, 0.0000f, 1.0f
        };
        SkMatrix44 arbitraryMatrix{SkMatrix44::kUninitialized_Constructor};
        arbitraryMatrix.setRowMajorf(values);
        if (kNonStandard_SkGammaNamed == gammaNamed) {
            srcElements.push_back(SkColorSpace_A2B::Element(gammas));
        } else {
            srcElements.push_back(SkColorSpace_A2B::Element(gammaNamed));
        }
        srcElements.push_back(SkColorSpace_A2B::Element(arbitraryMatrix));
        auto srcSpace = ColorSpaceXformTest::CreateA2BSpace(SkColorSpace_A2B::PCS::kXYZ,
                                                            std::move(srcElements));
        sk_sp<SkColorSpace> dstSpace(new SkColorSpace_XYZ(gammaNamed, gammas, arbitraryMatrix,
                                                          nullptr));

        return SkColorSpaceXform::New(static_cast<SkColorSpace_A2B*>(srcSpace.get()),
                                      static_cast<SkColorSpace_XYZ*>(dstSpace.get()));
    }

    static sk_sp<SkColorSpace> CreateA2BSpace(SkColorSpace_A2B::PCS pcs,
                                              std::vector<SkColorSpace_A2B::Element> elements) {
        return sk_sp<SkColorSpace>(new SkColorSpace_A2B(pcs, nullptr, std::move(elements)));
    }
};

static bool almost_equal(int x, int y) {
    return SkTAbs(x - y) <= 1 ;
}

static void test_identity_xform(skiatest::Reporter* r, const sk_sp<SkGammas>& gammas,
                                bool repeat) {
    // Arbitrary set of 10 pixels
    constexpr int width = 10;
    constexpr uint32_t srcPixels[width] = {
            0xFFABCDEF, 0xFF146829, 0xFF382759, 0xFF184968, 0xFFDE8271,
            0xFF32AB52, 0xFF0383BC, 0xFF000102, 0xFFFFFFFF, 0xFFDDEEFF, };
    uint32_t dstPixels[width];

    // Create and perform an identity xform.
    std::unique_ptr<SkColorSpaceXform> xform = ColorSpaceXformTest::CreateIdentityXform(gammas);
    bool result = xform->apply(select_xform_format(kN32_SkColorType), dstPixels,
                               SkColorSpaceXform::kBGRA_8888_ColorFormat, srcPixels, width,
                               kOpaque_SkAlphaType);
    REPORTER_ASSERT(r, result);

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

    if (repeat) {
        // We should cache part of the transform after the run.  So it is interesting
        // to make sure it still runs correctly the second time.
        test_identity_xform(r, gammas, false);
    }
}

static void test_identity_xform_A2B(skiatest::Reporter* r, SkGammaNamed gammaNamed,
                                    const sk_sp<SkGammas>& gammas, bool repeat) {
    // Arbitrary set of 10 pixels
    constexpr int width = 10;
    constexpr uint32_t srcPixels[width] = {
            0xFFABCDEF, 0xFF146829, 0xFF382759, 0xFF184968, 0xFFDE8271,
            0xFF32AB52, 0xFF0383BC, 0xFF000102, 0xFFFFFFFF, 0xFFDDEEFF, };
    uint32_t dstPixels[width];

    // Create and perform an identity xform.
    auto xform = ColorSpaceXformTest::CreateIdentityXform_A2B(gammaNamed, gammas);
    bool result = xform->apply(select_xform_format(kN32_SkColorType), dstPixels,
                               SkColorSpaceXform::kBGRA_8888_ColorFormat, srcPixels, width,
                               kOpaque_SkAlphaType);
    REPORTER_ASSERT(r, result);

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

    if (repeat) {
        // We should cache part of the transform after the run.  So it is interesting
        // to make sure it still runs correctly the second time.
        test_identity_xform_A2B(r, gammaNamed, gammas, false);
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
    test_identity_xform(r, gammas, true);
    test_identity_xform_A2B(r, kNonStandard_SkGammaNamed, gammas, true);
}

DEF_TEST(ColorSpaceXform_ParametricGamma, r) {
    // Parametric gamma curves
    void* memory = sk_malloc_throw(sizeof(SkGammas) + sizeof(SkColorSpaceTransferFn));
    sk_sp<SkGammas> gammas = sk_sp<SkGammas>(new (memory) SkGammas());
    gammas->fRedType = gammas->fGreenType = gammas->fBlueType = SkGammas::Type::kParam_Type;
    gammas->fRedData.fParamOffset = gammas->fGreenData.fParamOffset =
            gammas->fBlueData.fParamOffset = 0;
    SkColorSpaceTransferFn* params = SkTAddOffset<SkColorSpaceTransferFn>
            (memory, sizeof(SkGammas));

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
    test_identity_xform(r, gammas, true);
    test_identity_xform_A2B(r, kNonStandard_SkGammaNamed, gammas, true);
}

DEF_TEST(ColorSpaceXform_ExponentialGamma, r) {
    // Exponential gamma curves
    sk_sp<SkGammas> gammas = sk_sp<SkGammas>(new SkGammas());
    gammas->fRedType = gammas->fGreenType = gammas->fBlueType = SkGammas::Type::kValue_Type;
    gammas->fRedData.fValue = gammas->fGreenData.fValue = gammas->fBlueData.fValue = 1.4f;
    test_identity_xform(r, gammas, true);
    test_identity_xform_A2B(r, kNonStandard_SkGammaNamed, gammas, true);
}

DEF_TEST(ColorSpaceXform_NamedGamma, r) {
    sk_sp<SkGammas> gammas = sk_sp<SkGammas>(new SkGammas());
    gammas->fRedType = gammas->fGreenType = gammas->fBlueType = SkGammas::Type::kNamed_Type;
    gammas->fRedData.fNamed = kSRGB_SkGammaNamed;
    gammas->fGreenData.fNamed = k2Dot2Curve_SkGammaNamed;
    gammas->fBlueData.fNamed = kLinear_SkGammaNamed;
    test_identity_xform(r, gammas, true);
    test_identity_xform_A2B(r, kNonStandard_SkGammaNamed, gammas, true);
    test_identity_xform_A2B(r, kSRGB_SkGammaNamed, nullptr, true);
    test_identity_xform_A2B(r, k2Dot2Curve_SkGammaNamed, nullptr, true);
    test_identity_xform_A2B(r, kLinear_SkGammaNamed, nullptr, true);
}

DEF_TEST(ColorSpaceXform_NonMatchingGamma, r) {
    constexpr size_t tableSize = 10;
    void* memory = sk_malloc_throw(sizeof(SkGammas) + sizeof(float) * tableSize +
                                   sizeof(SkColorSpaceTransferFn));
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

    SkColorSpaceTransferFn* params = SkTAddOffset<SkColorSpaceTransferFn>(memory,
            sizeof(SkGammas) + sizeof(float) * tableSize);
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

    test_identity_xform(r, gammas, true);
    test_identity_xform_A2B(r, kNonStandard_SkGammaNamed, gammas, true);
}

DEF_TEST(ColorSpaceXform_A2BCLUT, r) {
    constexpr int inputChannels = 3;
    constexpr int gp            = 4; // # grid points

    constexpr int numEntries    = gp*gp*gp*3;
    uint8_t gridPoints[3] = {gp, gp, gp};
    void* memory = sk_malloc_throw(sizeof(SkColorLookUpTable) + sizeof(float) * numEntries);
    sk_sp<SkColorLookUpTable> colorLUT(new (memory) SkColorLookUpTable(inputChannels, gridPoints));
    // make a CLUT that rotates R, G, and B ie R->G, G->B, B->R
    float* table = SkTAddOffset<float>(memory, sizeof(SkColorLookUpTable));
    for (int r = 0; r < gp; ++r) {
        for (int g = 0; g < gp; ++g) {
            for (int b = 0; b < gp; ++b) {
                table[3*(gp*gp*r + gp*g + b) + 0] = g * (1.f / (gp - 1.f));
                table[3*(gp*gp*r + gp*g + b) + 1] = b * (1.f / (gp - 1.f));
                table[3*(gp*gp*r + gp*g + b) + 2] = r * (1.f / (gp - 1.f));
            }
        }
    }

    // build an even distribution of pixels every (7 / 255) steps
    // to test the xform on
    constexpr int pixelgp   = 7;
    constexpr int numPixels = pixelgp*pixelgp*pixelgp;
    SkAutoTMalloc<uint32_t> srcPixels(numPixels);
    int srcIndex = 0;
    for (int r = 0; r < pixelgp; ++r) {
        for (int g = 0; g < pixelgp; ++g) {
            for (int b = 0; b < pixelgp; ++b) {
                const int red   = (int) (r * (255.f / (pixelgp - 1.f)));
                const int green = (int) (g * (255.f / (pixelgp - 1.f)));
                const int blue  = (int) (b * (255.f / (pixelgp - 1.f)));
                srcPixels[srcIndex] = SkColorSetRGB(red, green, blue);
                ++srcIndex;
            }
        }
    }
    SkAutoTMalloc<uint32_t> dstPixels(numPixels);

    // src space is identity besides CLUT
    std::vector<SkColorSpace_A2B::Element> srcElements;
    srcElements.push_back(SkColorSpace_A2B::Element(std::move(colorLUT)));
    auto srcSpace = ColorSpaceXformTest::CreateA2BSpace(SkColorSpace_A2B::PCS::kXYZ,
                                                        std::move(srcElements));
    // dst space is entirely identity
    auto dstSpace = SkColorSpace::MakeRGB(SkColorSpace::kLinear_RenderTargetGamma, SkMatrix44::I());
    auto xform = SkColorSpaceXform::New(srcSpace.get(), dstSpace.get());
    bool result = xform->apply(SkColorSpaceXform::kRGBA_8888_ColorFormat, dstPixels.get(),
                               SkColorSpaceXform::kRGBA_8888_ColorFormat, srcPixels.get(),
                               numPixels, kOpaque_SkAlphaType);
    REPORTER_ASSERT(r, result);

    for (int i = 0; i < numPixels; ++i) {
        REPORTER_ASSERT(r, almost_equal(SkColorGetR(srcPixels[i]),
                                        SkColorGetG(dstPixels[i])));
        REPORTER_ASSERT(r, almost_equal(SkColorGetG(srcPixels[i]),
                                        SkColorGetB(dstPixels[i])));
        REPORTER_ASSERT(r, almost_equal(SkColorGetB(srcPixels[i]),
                                        SkColorGetR(dstPixels[i])));
    }
}

