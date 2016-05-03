/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "Resources.h"
#include "SkCodec.h"
#include "SkColorSpace.h"
#include "Test.h"

#include "png.h"

static bool almost_equal(float a, float b) {
    return SkTAbs(a - b) < 0.001f;
}

static void test_space(skiatest::Reporter* r, SkColorSpace* space,
                       const float red[], const float green[], const float blue[],
                       const float expectedGammas[]) {
#ifdef SK_DEBUG
    const SkColorSpace::SkGammas& gammas = space->gammas();
    REPORTER_ASSERT(r, almost_equal(expectedGammas[0], gammas.red()));
    REPORTER_ASSERT(r, almost_equal(expectedGammas[1], gammas.green()));
    REPORTER_ASSERT(r, almost_equal(expectedGammas[2], gammas.blue()));
#endif

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

DEF_TEST(ColorSpace_sRGB, r) {
    const float srgb_r[] = { 0.4358f, 0.2224f, 0.0139f };
    const float srgb_g[] = { 0.3853f, 0.7170f, 0.0971f };
    const float srgb_b[] = { 0.1430f, 0.0606f, 0.7139f };
    const float srgb_gamma[] = { 2.2f, 2.2f, 2.2f };
    test_space(r, SkColorSpace::NewNamed(SkColorSpace::kSRGB_Named).get(),
               srgb_r, srgb_g, srgb_b, srgb_gamma);

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
    SkColorSpace* colorSpace = codec->getColorSpace();
    REPORTER_ASSERT(r, nullptr != colorSpace);

    const float red[] = { 0.436066f, 0.222488f, 0.013916f };
    const float green[] = { 0.385147f, 0.716873f, 0.0970764f };
    const float blue[] = { 0.143066f, 0.0606079f, 0.714096f };
    const float gamma[] = { 0, 0, 0 };  // table-based gamma returns 0 from this its float-getter
    test_space(r, colorSpace, red, green, blue, gamma);
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

    SkColorSpace* colorSpace = codec->getColorSpace();
    REPORTER_ASSERT(r, nullptr != colorSpace);

    const float red[] = { 0.385117f, 0.716904f, 0.0970612f };
    const float green[] = { 0.143051f, 0.0606079f, 0.713913f };
    const float blue[] = { 0.436035f, 0.222488f, 0.013916f };
    const float gamma[] = { 2.2f, 2.2f, 2.2f };
    test_space(r, colorSpace, red, green, blue, gamma);
}
