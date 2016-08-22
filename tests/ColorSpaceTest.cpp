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
                       const SkColorSpace::GammaNamed expectedGamma) {

    REPORTER_ASSERT(r, expectedGamma == space->gammaNamed());


    SkMatrix44 mat = space->xyz();
    const float src[] = {
        1, 0, 0, 1,
        0, 1, 0, 1,
        0, 0, 1, 1,
    };
    float dst[4];
    for (int i = 0; i < 3; ++i) {
        mat.mapScalars(&src[i*4], dst);
        REPORTER_ASSERT(r, almost_equal(red[i],   dst[0]));
        REPORTER_ASSERT(r, almost_equal(green[i], dst[1]));
        REPORTER_ASSERT(r, almost_equal(blue[i],  dst[2]));
    }
}

const float g_sRGB_XYZ[] = { 0.4358f, 0.2224f, 0.0139f,   // R
                             0.3853f, 0.7170f, 0.0971f,   // G
                             0.1430f, 0.0606f, 0.7139f }; // B

DEF_TEST(ColorSpace_sRGB, r) {
    test_space(r, SkColorSpace::NewNamed(SkColorSpace::kSRGB_Named).get(),
               g_sRGB_XYZ, &g_sRGB_XYZ[3], &g_sRGB_XYZ[6], SkColorSpace::kSRGB_GammaNamed);

}

static SkStreamAsset* resource(const char path[]) {
    SkString fullPath = GetResourcePath(path);
    return SkStream::NewFromFile(fullPath.c_str());
}

DEF_TEST(ColorSpaceParsePngICCProfile, r) {
    SkAutoTDelete<SkStream> stream(resource("color_wheel_with_profile.png"));
    REPORTER_ASSERT(r, nullptr != stream);
    if (!stream) {
        return;
    }

    SkAutoTDelete<SkCodec> codec(SkCodec::NewFromStream(stream.release()));
    REPORTER_ASSERT(r, nullptr != codec);

#if (PNG_LIBPNG_VER_MAJOR > 1) || (PNG_LIBPNG_VER_MAJOR == 1 && PNG_LIBPNG_VER_MINOR >= 6)
    SkColorSpace* colorSpace = codec->getInfo().colorSpace();
    REPORTER_ASSERT(r, nullptr != colorSpace);

    test_space(r, colorSpace, &g_sRGB_XYZ[0], &g_sRGB_XYZ[3], &g_sRGB_XYZ[6],
               SkColorSpace::kSRGB_GammaNamed);
#endif
}

DEF_TEST(ColorSpaceParseJpegICCProfile, r) {
    SkAutoTDelete<SkStream> stream(resource("icc-v2-gbr.jpg"));
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
    REPORTER_ASSERT(r, nullptr != colorSpace);

    const float red[] = { 0.385117f, 0.716904f, 0.0970612f };
    const float green[] = { 0.143051f, 0.0606079f, 0.713913f };
    const float blue[] = { 0.436035f, 0.222488f, 0.013916f };
    test_space(r, colorSpace, red, green, blue, SkColorSpace::k2Dot2Curve_GammaNamed);
}

DEF_TEST(ColorSpaceSRGBCompare, r) {
    // Create an sRGB color space by name
    sk_sp<SkColorSpace> namedColorSpace = SkColorSpace::NewNamed(SkColorSpace::kSRGB_Named);

    // Create an sRGB color space by value
    SkMatrix44 srgbToxyzD50(SkMatrix44::kUninitialized_Constructor);
    srgbToxyzD50.set3x3RowMajorf(g_sRGB_XYZ);
    sk_sp<SkColorSpace> rgbColorSpace = SkColorSpace::NewRGB(SkColorSpace::kSRGB_GammaNamed,
                                                             srgbToxyzD50);
    REPORTER_ASSERT(r, rgbColorSpace == namedColorSpace);

    // Change a single value from the sRGB matrix
    srgbToxyzD50.set(2, 2, 0.5f);
    sk_sp<SkColorSpace> strangeColorSpace = SkColorSpace::NewRGB(SkColorSpace::kSRGB_GammaNamed,
                                                                 srgbToxyzD50);
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
    test_space(r, iccColorSpace.get(), g_sRGB_XYZ, &g_sRGB_XYZ[3], &g_sRGB_XYZ[6],
               SkColorSpace::k2Dot2Curve_GammaNamed);
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
    REPORTER_ASSERT(r, monitorSpace->xyz() == newMonitorSpace->xyz());
    REPORTER_ASSERT(r, monitorSpace->gammaNamed() == newMonitorSpace->gammaNamed());
}

DEF_TEST(ColorSpace_Named, r) {
    const struct {
        SkColorSpace::Named fNamed;
        bool fExpectedToSucceed;
        bool fIsSRGB;
    } recs[] {
        { SkColorSpace::kUnknown_Named,  false, false },
        { SkColorSpace::kSRGB_Named,     true,  true },
        { SkColorSpace::kAdobeRGB_Named, true,  false },
    };

    for (auto rec : recs) {
        auto cs = SkColorSpace::NewNamed(rec.fNamed);
        REPORTER_ASSERT(r, !cs == !rec.fExpectedToSucceed);
        if (cs) {
            if (rec.fIsSRGB) {
                REPORTER_ASSERT(r, SkColorSpace::kSRGB_GammaNamed == cs->gammaNamed());
            } else {
                REPORTER_ASSERT(r, SkColorSpace::k2Dot2Curve_GammaNamed == cs->gammaNamed());
            }
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
