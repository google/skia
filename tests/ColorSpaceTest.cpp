/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "Resources.h"
#include "SkCodec.h"
#include "SkColorSpace.h"
#include "SkColorSpace_Base.h"
#include "Test.h"

#include "png.h"

static bool almost_equal(float a, float b) {
    return SkTAbs(a - b) < 0.001f;
}

static void test_space(skiatest::Reporter* r, SkColorSpace* space,
                       const float red[], const float green[], const float blue[],
                       const SkGammaNamed expectedGamma) {

    REPORTER_ASSERT(r, nullptr != space);
    REPORTER_ASSERT(r, expectedGamma == as_CSB(space)->gammaNamed());

    const SkMatrix44& mat = space->toXYZD50();
    const float src[] = {
        1, 0, 0, 1,
        0, 1, 0, 1,
        0, 0, 1, 1,
    };
    const float* ref[3] = { red, green, blue };
    float dst[4];
    for (int i = 0; i < 3; ++i) {
        mat.mapScalars(&src[i*4], dst);
        REPORTER_ASSERT(r, almost_equal(ref[i][0], dst[0]));
        REPORTER_ASSERT(r, almost_equal(ref[i][1], dst[1]));
        REPORTER_ASSERT(r, almost_equal(ref[i][2], dst[2]));
    }
}

static void test_path(skiatest::Reporter* r, const char* path,
                      const float red[], const float green[], const float blue[],
                      const SkGammaNamed expectedGamma) {
    SkAutoTDelete<SkStream> stream(GetResourceAsStream(path));
    REPORTER_ASSERT(r, nullptr != stream);
    if (!stream) {
        return;
    }

    SkAutoTDelete<SkCodec> codec(SkCodec::NewFromStream(stream.release()));
    REPORTER_ASSERT(r, nullptr != codec);
    if (!codec) {
        return;
    }

    SkColorSpace* colorSpace = codec->getInfo().colorSpace();
    test_space(r, colorSpace, red, green, blue, expectedGamma);
}

static constexpr float g_sRGB_XYZ[]{
    0.4358f, 0.3853f, 0.1430f,    // Rx, Gx, Bx
    0.2224f, 0.7170f, 0.0606f,    // Ry, Gy, Gz
    0.0139f, 0.0971f, 0.7139f,    // Rz, Gz, Bz
};

static constexpr float g_sRGB_R[]{ 0.4358f, 0.2224f, 0.0139f };
static constexpr float g_sRGB_G[]{ 0.3853f, 0.7170f, 0.0971f };
static constexpr float g_sRGB_B[]{ 0.1430f, 0.0606f, 0.7139f };

DEF_TEST(ColorSpace_sRGB, r) {
    test_space(r, SkColorSpace::NewNamed(SkColorSpace::kSRGB_Named).get(),
               g_sRGB_R, g_sRGB_G, g_sRGB_B, kSRGB_SkGammaNamed);

}

DEF_TEST(ColorSpaceParseICCProfiles, r) {

#if (PNG_LIBPNG_VER_MAJOR > 1) || (PNG_LIBPNG_VER_MAJOR == 1 && PNG_LIBPNG_VER_MINOR >= 6)
    test_path(r, "color_wheel_with_profile.png", g_sRGB_R, g_sRGB_G, g_sRGB_B,
              kSRGB_SkGammaNamed);
#endif

    const float red[] = { 0.385117f, 0.716904f, 0.0970612f };
    const float green[] = { 0.143051f, 0.0606079f, 0.713913f };
    const float blue[] = { 0.436035f, 0.222488f, 0.013916f };
    test_path(r, "icc-v2-gbr.jpg", red, green, blue, k2Dot2Curve_SkGammaNamed);

    test_path(r, "webp-color-profile-crash.webp",
            red, green, blue, kNonStandard_SkGammaNamed);
    test_path(r, "webp-color-profile-lossless.webp",
            red, green, blue, kNonStandard_SkGammaNamed);
    test_path(r, "webp-color-profile-lossy.webp",
            red, green, blue, kNonStandard_SkGammaNamed);
    test_path(r, "webp-color-profile-lossy-alpha.webp",
            red, green, blue, kNonStandard_SkGammaNamed);
}

DEF_TEST(ColorSpaceSRGBCompare, r) {
    // Create an sRGB color space by name
    sk_sp<SkColorSpace> namedColorSpace = SkColorSpace::NewNamed(SkColorSpace::kSRGB_Named);

    // Create an sRGB color space by value
    SkMatrix44 srgbToxyzD50(SkMatrix44::kUninitialized_Constructor);
    srgbToxyzD50.set3x3RowMajorf(g_sRGB_XYZ);
    sk_sp<SkColorSpace> rgbColorSpace =
            SkColorSpace::NewRGB(SkColorSpace::kSRGB_RenderTargetGamma, srgbToxyzD50);
    REPORTER_ASSERT(r, rgbColorSpace == namedColorSpace);

    // Change a single value from the sRGB matrix
    srgbToxyzD50.set(2, 2, 0.5f);
    sk_sp<SkColorSpace> strangeColorSpace =
            SkColorSpace::NewRGB(SkColorSpace::kSRGB_RenderTargetGamma, srgbToxyzD50);
    REPORTER_ASSERT(r, strangeColorSpace != namedColorSpace);
}

DEF_TEST(ColorSpaceSRGBLinearCompare, r) {
    // Create the linear sRGB color space by name
    sk_sp<SkColorSpace> namedColorSpace = SkColorSpace::NewNamed(SkColorSpace::kSRGBLinear_Named);

    // Create the linear sRGB color space via the sRGB color space's makeLinearGamma()
    sk_sp<SkColorSpace> viaSrgbColorSpace =
        SkColorSpace::NewNamed(SkColorSpace::kSRGB_Named)->makeLinearGamma();
    REPORTER_ASSERT(r, namedColorSpace == viaSrgbColorSpace);

    // Create a linear sRGB color space by value
    SkMatrix44 srgbToxyzD50(SkMatrix44::kUninitialized_Constructor);
    srgbToxyzD50.set3x3RowMajorf(g_sRGB_XYZ);
    sk_sp<SkColorSpace> rgbColorSpace =
        SkColorSpace::NewRGB(SkColorSpace::kLinear_RenderTargetGamma, srgbToxyzD50);
    REPORTER_ASSERT(r, rgbColorSpace == namedColorSpace);

    // Change a single value from the sRGB matrix
    srgbToxyzD50.set(2, 2, 0.5f);
    sk_sp<SkColorSpace> strangeColorSpace =
        SkColorSpace::NewRGB(SkColorSpace::kLinear_RenderTargetGamma, srgbToxyzD50);
    REPORTER_ASSERT(r, strangeColorSpace != namedColorSpace);
}

class ColorSpaceTest {
public:
    static sk_sp<SkData> WriteToICC(SkColorSpace* space) {
        return as_CSB(space)->writeToICC();
    }
};

DEF_TEST(ColorSpaceWriteICC, r) {
    // Test writing a new ICC profile
    sk_sp<SkColorSpace> namedColorSpace = SkColorSpace::NewNamed(SkColorSpace::kSRGB_Named);
    sk_sp<SkData> namedData = ColorSpaceTest::WriteToICC(namedColorSpace.get());
    sk_sp<SkColorSpace> iccColorSpace = SkColorSpace::NewICC(namedData->data(), namedData->size());
    test_space(r, iccColorSpace.get(), g_sRGB_R, g_sRGB_G, g_sRGB_B, k2Dot2Curve_SkGammaNamed);
    // FIXME (msarett): Test disabled.  sRGB profiles are written approximately as 2.2f curves.
    // REPORTER_ASSERT(r, iccColorSpace == namedColorSpace);

    // Test saving the original ICC data
    sk_sp<SkData> monitorData = SkData::MakeFromFileName(
            GetResourcePath("icc_profiles/HP_ZR30w.icc").c_str());
    REPORTER_ASSERT(r, monitorData);
    if (!monitorData) {
        return;
    }
    sk_sp<SkColorSpace> monitorSpace = SkColorSpace::NewICC(monitorData->data(),
                                                            monitorData->size());
    sk_sp<SkData> newMonitorData = ColorSpaceTest::WriteToICC(monitorSpace.get());
    sk_sp<SkColorSpace> newMonitorSpace = SkColorSpace::NewICC(newMonitorData->data(),
                                                               newMonitorData->size());
    REPORTER_ASSERT(r, monitorSpace->toXYZD50() == newMonitorSpace->toXYZD50());
    REPORTER_ASSERT(r, as_CSB(monitorSpace)->gammaNamed() == as_CSB(newMonitorSpace)->gammaNamed());
}

DEF_TEST(ColorSpace_Named, r) {
    const struct {
        SkColorSpace::Named fNamed;
        SkGammaNamed fExpectedGamma;
    } recs[] {
        { SkColorSpace::kSRGB_Named,       kSRGB_SkGammaNamed },
        { SkColorSpace::kAdobeRGB_Named,   k2Dot2Curve_SkGammaNamed },
        { SkColorSpace::kSRGBLinear_Named, kLinear_SkGammaNamed },
    };

    for (auto rec : recs) {
        auto cs = SkColorSpace::NewNamed(rec.fNamed);
        REPORTER_ASSERT(r, cs);
        if (cs) {
            REPORTER_ASSERT(r, rec.fExpectedGamma == as_CSB(cs)->gammaNamed());
        }
    }

    SkImageInfo info = SkImageInfo::MakeS32(10, 10, kPremul_SkAlphaType);
    REPORTER_ASSERT(r, info.gammaCloseToSRGB());
}

static void test_serialize(skiatest::Reporter* r, SkColorSpace* space, bool isNamed) {
    sk_sp<SkData> data1 = space->serialize();

    size_t bytes = space->writeToMemory(nullptr);
    sk_sp<SkData> data2 = SkData::MakeUninitialized(bytes);
    space->writeToMemory(data2->writable_data());

    sk_sp<SkColorSpace> newSpace1 = SkColorSpace::Deserialize(data1->data(), data1->size());
    sk_sp<SkColorSpace> newSpace2 = SkColorSpace::Deserialize(data2->data(), data2->size());

    if (isNamed) {
        REPORTER_ASSERT(r, space == newSpace1.get());
        REPORTER_ASSERT(r, space == newSpace2.get());
    } else {
        REPORTER_ASSERT(r, SkColorSpace::Equals(space, newSpace1.get()));
        REPORTER_ASSERT(r, SkColorSpace::Equals(space, newSpace2.get()));
    }
}

DEF_TEST(ColorSpace_Serialize, r) {
    test_serialize(r, SkColorSpace::NewNamed(SkColorSpace::kSRGB_Named).get(), true);
    test_serialize(r, SkColorSpace::NewNamed(SkColorSpace::kAdobeRGB_Named).get(), true);
    test_serialize(r, SkColorSpace::NewNamed(SkColorSpace::kSRGBLinear_Named).get(), true);

    sk_sp<SkData> monitorData = SkData::MakeFromFileName(
            GetResourcePath("icc_profiles/HP_ZR30w.icc").c_str());
    test_serialize(r, SkColorSpace::NewICC(monitorData->data(), monitorData->size()).get(), false);
    monitorData = SkData::MakeFromFileName( GetResourcePath("icc_profiles/HP_Z32x.icc").c_str());
    test_serialize(r, SkColorSpace::NewICC(monitorData->data(), monitorData->size()).get(), false);
    monitorData = SkData::MakeFromFileName(GetResourcePath("icc_profiles/upperLeft.icc").c_str());
    test_serialize(r, SkColorSpace::NewICC(monitorData->data(), monitorData->size()).get(), false);
    monitorData = SkData::MakeFromFileName(GetResourcePath("icc_profiles/upperRight.icc").c_str());
    test_serialize(r, SkColorSpace::NewICC(monitorData->data(), monitorData->size()).get(), false);

    const float gammas[] = { 1.1f, 1.2f, 1.7f, };
    SkMatrix44 toXYZ(SkMatrix44::kIdentity_Constructor);
    test_serialize(r, SkColorSpace_Base::NewRGB(gammas, toXYZ).get(), false);
}

DEF_TEST(ColorSpace_Equals, r) {
    sk_sp<SkColorSpace> srgb = SkColorSpace::NewNamed(SkColorSpace::kSRGB_Named);
    sk_sp<SkColorSpace> adobe = SkColorSpace::NewNamed(SkColorSpace::kAdobeRGB_Named);
    sk_sp<SkData> data = SkData::MakeFromFileName(
            GetResourcePath("icc_profiles/HP_ZR30w.icc").c_str());
    sk_sp<SkColorSpace> z30 = SkColorSpace::NewICC(data->data(), data->size());
    data = SkData::MakeFromFileName( GetResourcePath("icc_profiles/HP_Z32x.icc").c_str());
    sk_sp<SkColorSpace> z32 = SkColorSpace::NewICC(data->data(), data->size());
    data = SkData::MakeFromFileName(GetResourcePath("icc_profiles/upperLeft.icc").c_str());
    sk_sp<SkColorSpace> upperLeft = SkColorSpace::NewICC(data->data(), data->size());
    data = SkData::MakeFromFileName(GetResourcePath("icc_profiles/upperRight.icc").c_str());
    sk_sp<SkColorSpace> upperRight = SkColorSpace::NewICC(data->data(), data->size());
    const float gammas1[] = { 1.1f, 1.2f, 1.3f, };
    const float gammas2[] = { 1.1f, 1.2f, 1.7f, };
    SkMatrix44 toXYZ(SkMatrix44::kIdentity_Constructor);
    sk_sp<SkColorSpace> rgb1 = SkColorSpace_Base::NewRGB(gammas1, toXYZ);
    sk_sp<SkColorSpace> rgb2 = SkColorSpace_Base::NewRGB(gammas2, toXYZ);
    sk_sp<SkColorSpace> rgb3 = SkColorSpace_Base::NewRGB(gammas1, toXYZ);

    REPORTER_ASSERT(r, SkColorSpace::Equals(nullptr, nullptr));
    REPORTER_ASSERT(r, SkColorSpace::Equals(srgb.get(), srgb.get()));
    REPORTER_ASSERT(r, SkColorSpace::Equals(adobe.get(), adobe.get()));
    REPORTER_ASSERT(r, SkColorSpace::Equals(z30.get(), z30.get()));
    REPORTER_ASSERT(r, SkColorSpace::Equals(z32.get(), z32.get()));
    REPORTER_ASSERT(r, SkColorSpace::Equals(upperLeft.get(), upperLeft.get()));
    REPORTER_ASSERT(r, SkColorSpace::Equals(upperRight.get(), upperRight.get()));
    REPORTER_ASSERT(r, SkColorSpace::Equals(rgb1.get(), rgb1.get()));
    REPORTER_ASSERT(r, SkColorSpace::Equals(rgb1.get(), rgb3.get()));

    REPORTER_ASSERT(r, !SkColorSpace::Equals(nullptr, srgb.get()));
    REPORTER_ASSERT(r, !SkColorSpace::Equals(srgb.get(), nullptr));
    REPORTER_ASSERT(r, !SkColorSpace::Equals(adobe.get(), srgb.get()));
    REPORTER_ASSERT(r, !SkColorSpace::Equals(z30.get(), srgb.get()));
    REPORTER_ASSERT(r, !SkColorSpace::Equals(z32.get(), z30.get()));
    REPORTER_ASSERT(r, !SkColorSpace::Equals(upperLeft.get(), srgb.get()));
    REPORTER_ASSERT(r, !SkColorSpace::Equals(upperLeft.get(), upperRight.get()));
    REPORTER_ASSERT(r, !SkColorSpace::Equals(z30.get(), upperRight.get()));
    REPORTER_ASSERT(r, !SkColorSpace::Equals(upperRight.get(), adobe.get()));
    REPORTER_ASSERT(r, !SkColorSpace::Equals(rgb1.get(), rgb2.get()));
}
