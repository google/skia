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

static SkStreamAsset* resource(const char path[]) {
    SkString fullPath = GetResourcePath(path);
    return SkStream::NewFromFile(fullPath.c_str());
}

static bool almost_equal(float a, float b) {
    return SkTAbs(a - b) < 0.001f;
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

    // No need to use almost equal here.  The color profile that we have extracted
    // actually has a table of gammas.  And our current implementation guesses 2.2f.
    SkFloat3 gammas = colorSpace->gamma();
    REPORTER_ASSERT(r, 2.2f == gammas.fVec[0]);
    REPORTER_ASSERT(r, 2.2f == gammas.fVec[1]);
    REPORTER_ASSERT(r, 2.2f == gammas.fVec[2]);

    // These nine values were extracted from the color profile in isolation (before
    // we embedded it in the png).  Here we check that we still extract the same values.
    SkFloat3x3 xyz = colorSpace->xyz();
    REPORTER_ASSERT(r, almost_equal(0.436066f, xyz.fMat[0]));
    REPORTER_ASSERT(r, almost_equal(0.222488f, xyz.fMat[1]));
    REPORTER_ASSERT(r, almost_equal(0.013916f, xyz.fMat[2]));
    REPORTER_ASSERT(r, almost_equal(0.385147f, xyz.fMat[3]));
    REPORTER_ASSERT(r, almost_equal(0.716873f, xyz.fMat[4]));
    REPORTER_ASSERT(r, almost_equal(0.0970764f, xyz.fMat[5]));
    REPORTER_ASSERT(r, almost_equal(0.143066f, xyz.fMat[6]));
    REPORTER_ASSERT(r, almost_equal(0.0606079f, xyz.fMat[7]));
    REPORTER_ASSERT(r, almost_equal(0.714096f, xyz.fMat[8]));
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

    // It's important to use almost equal here.  This profile sets gamma as
    // 563 / 256, which actually comes out to about 2.19922.
    SkFloat3 gammas = colorSpace->gamma();
    REPORTER_ASSERT(r, almost_equal(2.2f, gammas.fVec[0]));
    REPORTER_ASSERT(r, almost_equal(2.2f, gammas.fVec[1]));
    REPORTER_ASSERT(r, almost_equal(2.2f, gammas.fVec[2]));

    // These nine values were extracted from the color profile.  Until we know any
    // better, we'll assume these are the right values and test that we continue
    // to extract them properly.
    SkFloat3x3 xyz = colorSpace->xyz();
    REPORTER_ASSERT(r, almost_equal(0.385117f, xyz.fMat[0]));
    REPORTER_ASSERT(r, almost_equal(0.716904f, xyz.fMat[1]));
    REPORTER_ASSERT(r, almost_equal(0.0970612f, xyz.fMat[2]));
    REPORTER_ASSERT(r, almost_equal(0.143051f, xyz.fMat[3]));
    REPORTER_ASSERT(r, almost_equal(0.0606079f, xyz.fMat[4]));
    REPORTER_ASSERT(r, almost_equal(0.713913f, xyz.fMat[5]));
    REPORTER_ASSERT(r, almost_equal(0.436035f, xyz.fMat[6]));
    REPORTER_ASSERT(r, almost_equal(0.222488f, xyz.fMat[7]));
    REPORTER_ASSERT(r, almost_equal(0.013916f, xyz.fMat[8]));
}
