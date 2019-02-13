/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "Resources.h"
#include "SkCodec.h"
#include "SkColorSpace.h"
#include "SkColorSpacePriv.h"
#include "SkData.h"
#include "SkImageInfo.h"
#include "SkRefCnt.h"
#include "SkStream.h"
#include "SkTypes.h"
#include "Test.h"

#include "../third_party/skcms/skcms.h"
#include "png.h"

#include <memory>
#include <utility>

static bool almost_equal(float a, float b) {
    return SkTAbs(a - b) < 0.001f;
}

static void test_space(skiatest::Reporter* r, SkColorSpace* space,
                       const float red[], const float green[], const float blue[],
                       bool expectSRGB = false) {

    REPORTER_ASSERT(r, nullptr != space);
    REPORTER_ASSERT(r, expectSRGB == space->gammaCloseToSRGB());

    skcms_Matrix3x3 mat;
    space->toXYZD50(&mat);
    const float* ref[3] = { red, green, blue };
    for (int i = 0; i < 3; ++i) {
        REPORTER_ASSERT(r, almost_equal(ref[i][0], mat.vals[0][i]));
        REPORTER_ASSERT(r, almost_equal(ref[i][1], mat.vals[1][i]));
        REPORTER_ASSERT(r, almost_equal(ref[i][2], mat.vals[2][i]));
    }
}

static void test_path(skiatest::Reporter* r, const char* path,
                      const float red[], const float green[], const float blue[],
                      bool expectSRGB = false) {
    std::unique_ptr<SkStream> stream(GetResourceAsStream(path));
    REPORTER_ASSERT(r, nullptr != stream);
    if (!stream) {
        return;
    }

    std::unique_ptr<SkCodec> codec(SkCodec::MakeFromStream(std::move(stream)));
    REPORTER_ASSERT(r, nullptr != codec);
    if (!codec) {
        return;
    }

    auto colorSpace = codec->getInfo().refColorSpace();
    test_space(r, colorSpace.get(), red, green, blue, expectSRGB);
}

static constexpr float g_sRGB_R[]{ 0.4358f, 0.2224f, 0.0139f };
static constexpr float g_sRGB_G[]{ 0.3853f, 0.7170f, 0.0971f };
static constexpr float g_sRGB_B[]{ 0.1430f, 0.0606f, 0.7139f };

DEF_TEST(ColorSpace_sRGB, r) {
    test_space(r, sk_srgb_singleton(), g_sRGB_R, g_sRGB_G, g_sRGB_B, true);

}

DEF_TEST(ColorSpaceParseICCProfiles, r) {

#if (PNG_LIBPNG_VER_MAJOR > 1) || (PNG_LIBPNG_VER_MAJOR == 1 && PNG_LIBPNG_VER_MINOR >= 6)
    test_path(r, "images/color_wheel_with_profile.png", g_sRGB_R, g_sRGB_G, g_sRGB_B, true);
#endif

    const float red[]   = { 0.385117f, 0.716904f, 0.0970612f };
    const float green[] = { 0.143051f, 0.0606079f, 0.713913f };
    const float blue[]  = { 0.436035f, 0.222488f, 0.013916f };
    test_path(r, "images/icc-v2-gbr.jpg", red, green, blue);

    test_path(r, "images/webp-color-profile-crash.webp",
            red, green, blue);
    test_path(r, "images/webp-color-profile-lossless.webp",
            red, green, blue);
    test_path(r, "images/webp-color-profile-lossy.webp",
            red, green, blue);
    test_path(r, "images/webp-color-profile-lossy-alpha.webp",
            red, green, blue);
}

static void test_serialize(skiatest::Reporter* r, sk_sp<SkColorSpace> space, bool isNamed) {
    sk_sp<SkData> data1 = space->serialize();

    size_t bytes = space->writeToMemory(nullptr);
    sk_sp<SkData> data2 = SkData::MakeUninitialized(bytes);
    space->writeToMemory(data2->writable_data());

    sk_sp<SkColorSpace> newSpace1 = SkColorSpace::Deserialize(data1->data(), data1->size());
    sk_sp<SkColorSpace> newSpace2 = SkColorSpace::Deserialize(data2->data(), data2->size());

    REPORTER_ASSERT(r, SkColorSpace::Equals(space.get(), newSpace1.get()));
    REPORTER_ASSERT(r, SkColorSpace::Equals(space.get(), newSpace2.get()));
}

DEF_TEST(ColorSpace_Serialize, r) {
    test_serialize(r, SkColorSpace::MakeSRGB(), true);
    test_serialize(r, SkColorSpace::MakeSRGBLinear(), true);

    auto test = [&](const char* path) {
        sk_sp<SkData> data = GetResourceAsData(path);

        skcms_ICCProfile profile;
        REPORTER_ASSERT(r, skcms_Parse(data->data(), data->size(), &profile));

        sk_sp<SkColorSpace> space = SkColorSpace::Make(profile);
        REPORTER_ASSERT(r, space);

        test_serialize(r, space, false);
    };
    test("icc_profiles/HP_ZR30w.icc");
    test("icc_profiles/HP_Z32x.icc");

    skcms_TransferFunction fn;
    fn.a = 1.0f;
    fn.b = 0.0f;
    fn.c = 1.0f;
    fn.d = 0.5f;
    fn.e = 0.0f;
    fn.f = 0.0f;
    fn.g = 1.0f;
    skcms_Matrix3x3 toXYZ = {{
        { 1, 0, 0 },
        { 0, 1, 0 },
        { 0, 0, 1 },
    }};
    test_serialize(r, SkColorSpace::MakeRGB(fn, toXYZ), false);
}

DEF_TEST(ColorSpace_Equals, r) {
    sk_sp<SkColorSpace> srgb = SkColorSpace::MakeSRGB();

    auto parse = [&](const char* path) {
        sk_sp<SkData> data = GetResourceAsData(path);

        skcms_ICCProfile profile;
        REPORTER_ASSERT(r, skcms_Parse(data->data(), data->size(), &profile));

        sk_sp<SkColorSpace> space = SkColorSpace::Make(profile);
        REPORTER_ASSERT(r, space);

        return space;
    };
    sk_sp<SkColorSpace> z30 = parse("icc_profiles/HP_ZR30w.icc");
    sk_sp<SkColorSpace> z32 = parse("icc_profiles/HP_Z32x.icc");

    skcms_TransferFunction fn;
    fn.a = 1.0f;
    fn.b = 0.0f;
    fn.c = 1.0f;
    fn.d = 0.5f;
    fn.e = 0.0f;
    fn.f = 0.0f;
    fn.g = 1.0f;
    skcms_Matrix3x3 toXYZ = {{
        { 1, 0, 0 },
        { 0, 1, 0 },
        { 0, 0, 1 },
    }};
    sk_sp<SkColorSpace> rgb4 = SkColorSpace::MakeRGB(fn, toXYZ);

    REPORTER_ASSERT(r, SkColorSpace::Equals(nullptr, nullptr));
    REPORTER_ASSERT(r, SkColorSpace::Equals(srgb.get(), srgb.get()));
    REPORTER_ASSERT(r, SkColorSpace::Equals(z30.get(), z30.get()));
    REPORTER_ASSERT(r, SkColorSpace::Equals(z32.get(), z32.get()));
    REPORTER_ASSERT(r, SkColorSpace::Equals(rgb4.get(), rgb4.get()));

    REPORTER_ASSERT(r, !SkColorSpace::Equals(nullptr, srgb.get()));
    REPORTER_ASSERT(r, !SkColorSpace::Equals(srgb.get(), nullptr));
    REPORTER_ASSERT(r, !SkColorSpace::Equals(z30.get(), srgb.get()));
    REPORTER_ASSERT(r, !SkColorSpace::Equals(z32.get(), z30.get()));
    REPORTER_ASSERT(r, !SkColorSpace::Equals(z30.get(), rgb4.get()));
    REPORTER_ASSERT(r, !SkColorSpace::Equals(srgb.get(), rgb4.get()));
}

static inline bool matrix_almost_equal(const skcms_Matrix3x3& a, const skcms_Matrix3x3& b) {
    for (int r = 0; r < 3; ++r) {
        for (int c = 0; c < 3; ++c) {
            if (!almost_equal(a.vals[r][c], b.vals[r][c])) {
                return false;
            }
        }
    }
    return true;
}

static inline void check_primaries(skiatest::Reporter* r, const SkColorSpacePrimaries& primaries,
                                   const skcms_Matrix3x3& reference) {
    skcms_Matrix3x3 toXYZ;
    bool result = primaries.toXYZD50(&toXYZ);
    REPORTER_ASSERT(r, result);
    REPORTER_ASSERT(r, matrix_almost_equal(toXYZ, reference));
}

DEF_TEST(ColorSpace_Primaries, r) {
    // sRGB primaries (D65)
    skcms_Matrix3x3 srgbToXYZ;
    bool result = skcms_PrimariesToXYZD50(
        0.64f, 0.33f,
        0.30f, 0.60f,
        0.15f, 0.06f,
        0.3127f, 0.3290f,
        &srgbToXYZ);
    REPORTER_ASSERT(r, result);

    sk_sp<SkColorSpace> space = SkColorSpace::MakeRGB(SkNamedTransferFn::kSRGB, srgbToXYZ);
    REPORTER_ASSERT(r, SkColorSpace::MakeSRGB() == space);

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
    skcms_Matrix3x3 proToXYZ = {{
        { 0.7976749f, 0.1351917f, 0.0313534f },
        { 0.2880402f, 0.7118741f, 0.0000857f },
        { 0.0000000f, 0.0000000f, 0.8252100f },
    }};
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
    skcms_Matrix3x3 ntscToXYZ = {{
        {  0.6343706f, 0.1852204f, 0.1446290f },
        {  0.3109496f, 0.5915984f, 0.0974520f },
        { -0.0011817f, 0.0555518f, 0.7708399f }
    }};
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
    space = SkColorSpace::MakeRGB(SkNamedTransferFn::kSRGB, SkNamedGamut::kDCIP3);
    skcms_Matrix3x3 reference;
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
    space = SkColorSpace::MakeRGB(SkNamedTransferFn::kSRGB, SkNamedGamut::kRec2020);
    SkAssertResult(space->toXYZD50(&reference));
    check_primaries(r, rec2020, reference);
}

DEF_TEST(ColorSpace_MatrixHash, r) {
    sk_sp<SkColorSpace> srgb = SkColorSpace::MakeSRGB();

    skcms_TransferFunction fn;
    fn.a = 1.0f;
    fn.b = 0.0f;
    fn.c = 0.0f;
    fn.d = 0.0f;
    fn.e = 0.0f;
    fn.f = 0.0f;
    fn.g = 3.0f;

    sk_sp<SkColorSpace> strange = SkColorSpace::MakeRGB(fn, SkNamedGamut::kSRGB);

    REPORTER_ASSERT(r, srgb->toXYZD50Hash() == strange->toXYZD50Hash());
}

DEF_TEST(ColorSpace_IsSRGB, r) {
    sk_sp<SkColorSpace> srgb0 = SkColorSpace::MakeSRGB();

    skcms_TransferFunction fn;
    fn.a = 1.0f;
    fn.b = 0.0f;
    fn.c = 0.0f;
    fn.d = 0.0f;
    fn.e = 0.0f;
    fn.f = 0.0f;
    fn.g = 2.2f;
    sk_sp<SkColorSpace> twoDotTwo = SkColorSpace::MakeRGB(fn, SkNamedGamut::kSRGB);

    REPORTER_ASSERT(r, srgb0->isSRGB());
    REPORTER_ASSERT(r, !twoDotTwo->isSRGB());
}

DEF_TEST(ColorSpace_skcms_IsSRGB, r) {
    sk_sp<SkColorSpace> srgb = SkColorSpace::Make(*skcms_sRGB_profile());
    REPORTER_ASSERT(r, srgb->isSRGB());
}

DEF_TEST(ColorSpace_skcms_sRGB_exact, r) {
    skcms_ICCProfile profile;
    sk_srgb_singleton()->toProfile(&profile);

    REPORTER_ASSERT(r, 0 == memcmp(&profile, skcms_sRGB_profile(), sizeof(skcms_ICCProfile)));
}
