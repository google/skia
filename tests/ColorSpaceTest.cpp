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
#include "SkColorSpace_XYZ.h"
#include "SkColorSpacePriv.h"
#include "Test.h"

#include "png.h"

static bool almost_equal(float a, float b) {
    return SkTAbs(a - b) < 0.001f;
}

static void test_space(skiatest::Reporter* r, SkColorSpace* space,
                       const float red[], const float green[], const float blue[],
                       const SkGammaNamed expectedGamma) {

    REPORTER_ASSERT(r, nullptr != space);
    SkASSERT(SkColorSpace_Base::Type::kXYZ == as_CSB(space)->type());
    SkColorSpace_XYZ* csXYZ = static_cast<SkColorSpace_XYZ*>(space);
    REPORTER_ASSERT(r, expectedGamma == csXYZ->gammaNamed());

    const SkMatrix44& mat = *csXYZ->toXYZD50();
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
    std::unique_ptr<SkStream> stream(GetResourceAsStream(path));
    REPORTER_ASSERT(r, nullptr != stream);
    if (!stream) {
        return;
    }

    std::unique_ptr<SkCodec> codec(SkCodec::NewFromStream(stream.release()));
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
    test_space(r, SkColorSpace::MakeSRGB().get(),
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
    sk_sp<SkColorSpace> namedColorSpace = SkColorSpace::MakeSRGB();

    // Create an sRGB color space by value
    SkMatrix44 srgbToxyzD50(SkMatrix44::kUninitialized_Constructor);
    srgbToxyzD50.set3x3RowMajorf(g_sRGB_XYZ);
    sk_sp<SkColorSpace> rgbColorSpace =
            SkColorSpace::MakeRGB(SkColorSpace::kSRGB_RenderTargetGamma, srgbToxyzD50);
    REPORTER_ASSERT(r, rgbColorSpace == namedColorSpace);

    SkColorSpaceTransferFn srgbFn;
    srgbFn.fA = (1.0f / 1.055f);
    srgbFn.fB = (0.055f / 1.055f);
    srgbFn.fC = (1.0f / 12.92f);
    srgbFn.fD = 0.04045f;
    srgbFn.fE = 0.0f;
    srgbFn.fF = 0.0f;
    srgbFn.fG = 2.4f;
    sk_sp<SkColorSpace> rgbColorSpace2 = SkColorSpace::MakeRGB(srgbFn, srgbToxyzD50);
    REPORTER_ASSERT(r, rgbColorSpace2 == namedColorSpace);

    // Change a single value from the sRGB matrix
    srgbToxyzD50.set(2, 2, 0.5f);
    sk_sp<SkColorSpace> strangeColorSpace =
            SkColorSpace::MakeRGB(SkColorSpace::kSRGB_RenderTargetGamma, srgbToxyzD50);
    REPORTER_ASSERT(r, strangeColorSpace != namedColorSpace);
}

DEF_TEST(ColorSpaceSRGBLinearCompare, r) {
    // Create the linear sRGB color space by name
    sk_sp<SkColorSpace> namedColorSpace = SkColorSpace::MakeSRGBLinear();

    // Create the linear sRGB color space via the sRGB color space's makeLinearGamma()
    auto srgb = SkColorSpace::MakeSRGB();
    auto srgbXYZ = static_cast<SkColorSpace_XYZ*>(srgb.get());
    sk_sp<SkColorSpace> viaSrgbColorSpace = srgbXYZ->makeLinearGamma();
    REPORTER_ASSERT(r, namedColorSpace == viaSrgbColorSpace);

    // Create a linear sRGB color space by value
    SkMatrix44 srgbToxyzD50(SkMatrix44::kUninitialized_Constructor);
    srgbToxyzD50.set3x3RowMajorf(g_sRGB_XYZ);
    sk_sp<SkColorSpace> rgbColorSpace =
        SkColorSpace::MakeRGB(SkColorSpace::kLinear_RenderTargetGamma, srgbToxyzD50);
    REPORTER_ASSERT(r, rgbColorSpace == namedColorSpace);

    SkColorSpaceTransferFn linearExpFn;
    linearExpFn.fA = 1.0f;
    linearExpFn.fB = 0.0f;
    linearExpFn.fC = 0.0f;
    linearExpFn.fD = 0.0f;
    linearExpFn.fE = 0.0f;
    linearExpFn.fF = 0.0f;
    linearExpFn.fG = 1.0f;
    sk_sp<SkColorSpace> rgbColorSpace2 = SkColorSpace::MakeRGB(linearExpFn, srgbToxyzD50);
    REPORTER_ASSERT(r, rgbColorSpace2 == namedColorSpace);

    SkColorSpaceTransferFn linearFn;
    linearFn.fA = 0.0f;
    linearFn.fB = 0.0f;
    linearFn.fC = 1.0f;
    linearFn.fD = 1.0f;
    linearFn.fE = 0.0f;
    linearFn.fF = 0.0f;
    linearFn.fG = 0.0f;
    sk_sp<SkColorSpace> rgbColorSpace3 = SkColorSpace::MakeRGB(linearFn, srgbToxyzD50);
    REPORTER_ASSERT(r, rgbColorSpace3 == namedColorSpace);

    // Change a single value from the sRGB matrix
    srgbToxyzD50.set(2, 2, 0.5f);
    sk_sp<SkColorSpace> strangeColorSpace =
        SkColorSpace::MakeRGB(SkColorSpace::kLinear_RenderTargetGamma, srgbToxyzD50);
    REPORTER_ASSERT(r, strangeColorSpace != namedColorSpace);
}

DEF_TEST(ColorSpaceAdobeCompare, r) {
    // Create an sRGB color space by name
    sk_sp<SkColorSpace> namedColorSpace =
            SkColorSpace_Base::MakeNamed(SkColorSpace_Base::kAdobeRGB_Named);

    // Create an sRGB color space by value
    SkMatrix44 adobeToxyzD50(SkMatrix44::kUninitialized_Constructor);
    adobeToxyzD50.set3x3RowMajorf(gAdobeRGB_toXYZD50);

    SkColorSpaceTransferFn fn;
    fn.fA = 1.0f;
    fn.fB = 0.0f;
    fn.fC = 0.0f;
    fn.fD = 0.0f;
    fn.fE = 0.0f;
    fn.fF = 0.0f;
    fn.fG = 2.2f;
    sk_sp<SkColorSpace> rgbColorSpace = SkColorSpace::MakeRGB(fn, adobeToxyzD50);
    REPORTER_ASSERT(r, rgbColorSpace == namedColorSpace);
}

DEF_TEST(ColorSpace_Named, r) {
    const struct {
        SkColorSpace_Base::Named fNamed;
        SkGammaNamed             fExpectedGamma;
    } recs[] {
        { SkColorSpace_Base::kSRGB_Named,       kSRGB_SkGammaNamed },
        { SkColorSpace_Base::kAdobeRGB_Named,   k2Dot2Curve_SkGammaNamed },
        { SkColorSpace_Base::kSRGBLinear_Named, kLinear_SkGammaNamed },
    };

    for (auto rec : recs) {
        auto cs = SkColorSpace_Base::MakeNamed(rec.fNamed);
        REPORTER_ASSERT(r, cs);
        if (cs) {
            SkASSERT(SkColorSpace_Base::Type::kXYZ == as_CSB(cs)->type());
            SkColorSpace_XYZ* csXYZ = static_cast<SkColorSpace_XYZ*>(cs.get());
            REPORTER_ASSERT(r, rec.fExpectedGamma == csXYZ->gammaNamed());
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
    test_serialize(r, SkColorSpace::MakeSRGB().get(), true);
    test_serialize(r, SkColorSpace_Base::MakeNamed(SkColorSpace_Base::kAdobeRGB_Named).get(), true);
    test_serialize(r, SkColorSpace::MakeSRGBLinear().get(), true);

    sk_sp<SkData> monitorData = SkData::MakeFromFileName(
            GetResourcePath("icc_profiles/HP_ZR30w.icc").c_str());
    test_serialize(r, SkColorSpace::MakeICC(monitorData->data(), monitorData->size()).get(), false);
    monitorData = SkData::MakeFromFileName( GetResourcePath("icc_profiles/HP_Z32x.icc").c_str());
    test_serialize(r, SkColorSpace::MakeICC(monitorData->data(), monitorData->size()).get(), false);
    monitorData = SkData::MakeFromFileName(GetResourcePath("icc_profiles/upperLeft.icc").c_str());
    test_serialize(r, SkColorSpace::MakeICC(monitorData->data(), monitorData->size()).get(), false);
    monitorData = SkData::MakeFromFileName(GetResourcePath("icc_profiles/upperRight.icc").c_str());
    test_serialize(r, SkColorSpace::MakeICC(monitorData->data(), monitorData->size()).get(), false);

    SkColorSpaceTransferFn fn;
    fn.fA = 1.0f;
    fn.fB = 0.0f;
    fn.fC = 1.0f;
    fn.fD = 0.5f;
    fn.fE = 0.0f;
    fn.fF = 0.0f;
    fn.fG = 1.0f;
    SkMatrix44 toXYZ(SkMatrix44::kIdentity_Constructor);
    test_serialize(r, SkColorSpace::MakeRGB(fn, toXYZ).get(), false);
}

DEF_TEST(ColorSpace_Equals, r) {
    sk_sp<SkColorSpace> srgb = SkColorSpace::MakeSRGB();
    sk_sp<SkColorSpace> adobe = SkColorSpace_Base::MakeNamed(SkColorSpace_Base::kAdobeRGB_Named);
    sk_sp<SkData> data = SkData::MakeFromFileName(
            GetResourcePath("icc_profiles/HP_ZR30w.icc").c_str());
    sk_sp<SkColorSpace> z30 = SkColorSpace::MakeICC(data->data(), data->size());
    data = SkData::MakeFromFileName( GetResourcePath("icc_profiles/HP_Z32x.icc").c_str());
    sk_sp<SkColorSpace> z32 = SkColorSpace::MakeICC(data->data(), data->size());
    data = SkData::MakeFromFileName(GetResourcePath("icc_profiles/upperLeft.icc").c_str());
    sk_sp<SkColorSpace> upperLeft = SkColorSpace::MakeICC(data->data(), data->size());
    data = SkData::MakeFromFileName(GetResourcePath("icc_profiles/upperRight.icc").c_str());
    sk_sp<SkColorSpace> upperRight = SkColorSpace::MakeICC(data->data(), data->size());

    SkColorSpaceTransferFn fn;
    fn.fA = 1.0f;
    fn.fB = 0.0f;
    fn.fC = 1.0f;
    fn.fD = 0.5f;
    fn.fE = 0.0f;
    fn.fF = 0.0f;
    fn.fG = 1.0f;
    SkMatrix44 toXYZ(SkMatrix44::kIdentity_Constructor);
    sk_sp<SkColorSpace> rgb4 = SkColorSpace::MakeRGB(fn, toXYZ);

    REPORTER_ASSERT(r, SkColorSpace::Equals(nullptr, nullptr));
    REPORTER_ASSERT(r, SkColorSpace::Equals(srgb.get(), srgb.get()));
    REPORTER_ASSERT(r, SkColorSpace::Equals(adobe.get(), adobe.get()));
    REPORTER_ASSERT(r, SkColorSpace::Equals(z30.get(), z30.get()));
    REPORTER_ASSERT(r, SkColorSpace::Equals(z32.get(), z32.get()));
    REPORTER_ASSERT(r, SkColorSpace::Equals(upperLeft.get(), upperLeft.get()));
    REPORTER_ASSERT(r, SkColorSpace::Equals(upperRight.get(), upperRight.get()));
    REPORTER_ASSERT(r, SkColorSpace::Equals(rgb4.get(), rgb4.get()));

    REPORTER_ASSERT(r, !SkColorSpace::Equals(nullptr, srgb.get()));
    REPORTER_ASSERT(r, !SkColorSpace::Equals(srgb.get(), nullptr));
    REPORTER_ASSERT(r, !SkColorSpace::Equals(adobe.get(), srgb.get()));
    REPORTER_ASSERT(r, !SkColorSpace::Equals(z30.get(), srgb.get()));
    REPORTER_ASSERT(r, !SkColorSpace::Equals(z32.get(), z30.get()));
    REPORTER_ASSERT(r, !SkColorSpace::Equals(upperLeft.get(), srgb.get()));
    REPORTER_ASSERT(r, !SkColorSpace::Equals(upperLeft.get(), upperRight.get()));
    REPORTER_ASSERT(r, !SkColorSpace::Equals(z30.get(), upperRight.get()));
    REPORTER_ASSERT(r, !SkColorSpace::Equals(upperRight.get(), adobe.get()));
    REPORTER_ASSERT(r, !SkColorSpace::Equals(z30.get(), rgb4.get()));
    REPORTER_ASSERT(r, !SkColorSpace::Equals(srgb.get(), rgb4.get()));
}

static inline bool matrix_almost_equal(const SkMatrix44& a, const SkMatrix44& b) {
    return almost_equal(a.get(0, 0), b.get(0, 0)) &&
           almost_equal(a.get(0, 1), b.get(0, 1)) &&
           almost_equal(a.get(0, 2), b.get(0, 2)) &&
           almost_equal(a.get(0, 3), b.get(0, 3)) &&
           almost_equal(a.get(1, 0), b.get(1, 0)) &&
           almost_equal(a.get(1, 1), b.get(1, 1)) &&
           almost_equal(a.get(1, 2), b.get(1, 2)) &&
           almost_equal(a.get(1, 3), b.get(1, 3)) &&
           almost_equal(a.get(2, 0), b.get(2, 0)) &&
           almost_equal(a.get(2, 1), b.get(2, 1)) &&
           almost_equal(a.get(2, 2), b.get(2, 2)) &&
           almost_equal(a.get(2, 3), b.get(2, 3)) &&
           almost_equal(a.get(3, 0), b.get(3, 0)) &&
           almost_equal(a.get(3, 1), b.get(3, 1)) &&
           almost_equal(a.get(3, 2), b.get(3, 2)) &&
           almost_equal(a.get(3, 3), b.get(3, 3));
}

static inline void check_primaries(skiatest::Reporter* r, const SkColorSpacePrimaries& primaries,
                                   const SkMatrix44& reference) {
    SkMatrix44 toXYZ(SkMatrix44::kUninitialized_Constructor);
    bool result = primaries.toXYZD50(&toXYZ);
    REPORTER_ASSERT(r, result);
    REPORTER_ASSERT(r, matrix_almost_equal(toXYZ, reference));
}

DEF_TEST(ColorSpace_Primaries, r) {
    // sRGB primaries (D65)
    SkColorSpacePrimaries srgb;
    srgb.fRX = 0.64f;
    srgb.fRY = 0.33f;
    srgb.fGX = 0.30f;
    srgb.fGY = 0.60f;
    srgb.fBX = 0.15f;
    srgb.fBY = 0.06f;
    srgb.fWX = 0.3127f;
    srgb.fWY = 0.3290f;
    SkMatrix44 srgbToXYZ(SkMatrix44::kUninitialized_Constructor);
    bool result = srgb.toXYZD50(&srgbToXYZ);
    REPORTER_ASSERT(r, result);

    sk_sp<SkColorSpace> space = SkColorSpace::MakeRGB(SkColorSpace::kSRGB_RenderTargetGamma,
                                                      srgbToXYZ);
    REPORTER_ASSERT(r, SkColorSpace::MakeSRGB() == space);

    // AdobeRGB primaries (D65)
    SkColorSpacePrimaries adobe;
    adobe.fRX = 0.64f;
    adobe.fRY = 0.33f;
    adobe.fGX = 0.21f;
    adobe.fGY = 0.71f;
    adobe.fBX = 0.15f;
    adobe.fBY = 0.06f;
    adobe.fWX = 0.3127f;
    adobe.fWY = 0.3290f;
    SkMatrix44 adobeToXYZ(SkMatrix44::kUninitialized_Constructor);
    result = adobe.toXYZD50(&adobeToXYZ);
    REPORTER_ASSERT(r, result);

    SkColorSpaceTransferFn fn;
    fn.fA = 1.0f;
    fn.fB = fn.fC = fn.fD = fn.fE = fn.fF = 0.0f;
    fn.fG = 2.2f;
    space = SkColorSpace::MakeRGB(fn, adobeToXYZ);
    REPORTER_ASSERT(r, SkColorSpace_Base::MakeNamed(SkColorSpace_Base::kAdobeRGB_Named) == space);

    // ProPhoto (D50)
    SkColorSpacePrimaries proPhoto;
    proPhoto.fRX = 0.7347f;
    proPhoto.fRY = 0.2653f;
    proPhoto.fGX = 0.1596f;
    proPhoto.fGY = 0.8404f;
    proPhoto.fBX = 0.0366f;
    proPhoto.fBY = 0.0001f;
    proPhoto.fWX = 0.34567f;
    proPhoto.fWY = 0.35850f;
    SkMatrix44 proToXYZ(SkMatrix44::kUninitialized_Constructor);
    proToXYZ.set3x3(0.7976749f, 0.2880402f, 0.0000000f,
                    0.1351917f, 0.7118741f, 0.0000000f,
                    0.0313534f, 0.0000857f, 0.8252100f);
    check_primaries(r, proPhoto, proToXYZ);

    // NTSC (C)
    SkColorSpacePrimaries ntsc;
    ntsc.fRX = 0.67f;
    ntsc.fRY = 0.33f;
    ntsc.fGX = 0.21f;
    ntsc.fGY = 0.71f;
    ntsc.fBX = 0.14f;
    ntsc.fBY = 0.08f;
    ntsc.fWX = 0.31006f;
    ntsc.fWY = 0.31616f;
    SkMatrix44 ntscToXYZ(SkMatrix44::kUninitialized_Constructor);
    ntscToXYZ.set3x3(0.6343706f, 0.3109496f, -0.0011817f,
                     0.1852204f, 0.5915984f, 0.0555518f,
                     0.1446290f, 0.0974520f, 0.7708399f);
    check_primaries(r, ntsc, ntscToXYZ);

    // DCI P3 (D65)
    SkColorSpacePrimaries p3;
    p3.fRX = 0.680f;
    p3.fRY = 0.320f;
    p3.fGX = 0.265f;
    p3.fGY = 0.690f;
    p3.fBX = 0.150f;
    p3.fBY = 0.060f;
    p3.fWX = 0.3127f;
    p3.fWY = 0.3290f;
    space = SkColorSpace::MakeRGB(SkColorSpace::kSRGB_RenderTargetGamma,
                                  SkColorSpace::kDCIP3_D65_Gamut);
    SkMatrix44 reference(SkMatrix44::kUninitialized_Constructor);
    SkAssertResult(space->toXYZD50(&reference));
    check_primaries(r, p3, reference);

    // Rec 2020 (D65)
    SkColorSpacePrimaries rec2020;
    rec2020.fRX = 0.708f;
    rec2020.fRY = 0.292f;
    rec2020.fGX = 0.170f;
    rec2020.fGY = 0.797f;
    rec2020.fBX = 0.131f;
    rec2020.fBY = 0.046f;
    rec2020.fWX = 0.3127f;
    rec2020.fWY = 0.3290f;
    space = SkColorSpace::MakeRGB(SkColorSpace::kSRGB_RenderTargetGamma,
                                  SkColorSpace::kRec2020_Gamut);
    SkAssertResult(space->toXYZD50(&reference));
    check_primaries(r, rec2020, reference);
}

DEF_TEST(ColorSpace_InvalidICC, r) {
    // This color space has a matrix that is not D50.
    sk_sp<SkData> data = GetResourceAsData("icc_profiles/SM2333SW.icc");
    if (!data) {
        return;
    }
    sk_sp<SkColorSpace> cs = SkColorSpace::MakeICC(data->data(), data->size());
    REPORTER_ASSERT(r, !cs);

    // The color space has a color lut with only one entry in each dimension.
    data = GetResourceAsData("icc_profiles/invalid_color_lut.icc");
    if (!data) {
        return;
    }

    cs = SkColorSpace::MakeICC(data->data(), data->size());
    REPORTER_ASSERT(r, !cs);
}

DEF_TEST(ColorSpace_MatrixHash, r) {
    sk_sp<SkColorSpace> srgb = SkColorSpace::MakeSRGB();

    SkColorSpaceTransferFn fn;
    fn.fA = 1.0f;
    fn.fB = 0.0f;
    fn.fC = 0.0f;
    fn.fD = 0.0f;
    fn.fE = 0.0f;
    fn.fF = 0.0f;
    fn.fG = 3.0f;

    SkMatrix44 srgbMat(SkMatrix44::kUninitialized_Constructor);
    srgbMat.set3x3RowMajorf(gSRGB_toXYZD50);
    sk_sp<SkColorSpace> strange = SkColorSpace::MakeRGB(fn, srgbMat);

    REPORTER_ASSERT(r, *as_CSB(srgb)->toXYZD50() == *as_CSB(strange)->toXYZD50());
    REPORTER_ASSERT(r, as_CSB(srgb)->toXYZD50Hash() == as_CSB(strange)->toXYZD50Hash());
}

DEF_TEST(ColorSpace_IsSRGB, r) {
    sk_sp<SkColorSpace> srgb0 = SkColorSpace::MakeSRGB();

    SkColorSpaceTransferFn fn;
    fn.fA = 1.0f;
    fn.fB = 0.0f;
    fn.fC = 0.0f;
    fn.fD = 0.0f;
    fn.fE = 0.0f;
    fn.fF = 0.0f;
    fn.fG = 2.2f;
    sk_sp<SkColorSpace> twoDotTwo = SkColorSpace::MakeRGB(fn, SkColorSpace::kSRGB_Gamut);

    REPORTER_ASSERT(r, srgb0->isSRGB());
    REPORTER_ASSERT(r, !twoDotTwo->isSRGB());
}
