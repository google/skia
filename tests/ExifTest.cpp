/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/codec/SkCodec.h"
#include "include/codec/SkEncodedOrigin.h"
#include "include/core/SkStream.h"
#include "include/private/SkExif.h"
#include "tests/Test.h"
#include "tools/Resources.h"

#include <memory>
#include <utility>

DEF_TEST(ExifOrientation, r) {
    std::unique_ptr<SkStream> stream(GetResourceAsStream("images/exif-orientation-2-ur.jpg"));
    REPORTER_ASSERT(r, nullptr != stream);
    if (!stream) {
        return;
    }

    std::unique_ptr<SkCodec> codec(SkCodec::MakeFromStream(std::move(stream)));
    REPORTER_ASSERT(r, nullptr != codec);
    SkEncodedOrigin origin = codec->getOrigin();
    REPORTER_ASSERT(r, kTopRight_SkEncodedOrigin == origin);

    codec = SkCodec::MakeFromStream(GetResourceAsStream("images/mandrill_512_q075.jpg"));
    REPORTER_ASSERT(r, nullptr != codec);
    origin = codec->getOrigin();
    REPORTER_ASSERT(r, kTopLeft_SkEncodedOrigin == origin);
}

DEF_TEST(ExifOrientationInExif, r) {
    std::unique_ptr<SkStream> stream(GetResourceAsStream("images/orientation/exif.jpg"));

    std::unique_ptr<SkCodec> codec = SkCodec::MakeFromStream(std::move(stream));
    REPORTER_ASSERT(r, nullptr != codec);
    SkEncodedOrigin origin = codec->getOrigin();
    REPORTER_ASSERT(r, kLeftBottom_SkEncodedOrigin == origin);
}

DEF_TEST(ExifOrientationInSubIFD, r) {
    std::unique_ptr<SkStream> stream(GetResourceAsStream("images/orientation/subifd.jpg"));

    std::unique_ptr<SkCodec> codec = SkCodec::MakeFromStream(std::move(stream));
    REPORTER_ASSERT(r, nullptr != codec);
    SkEncodedOrigin origin = codec->getOrigin();
    REPORTER_ASSERT(r, kLeftBottom_SkEncodedOrigin == origin);
}

static bool approx_eq(float x, float y, float epsilon) { return std::abs(x - y) < epsilon; }

DEF_TEST(ExifParse, r) {
    const float kEpsilon = 0.001f;

    {
        sk_sp<SkData> data = GetResourceAsData("images/test0-hdr.exif");
        REPORTER_ASSERT(r, nullptr != data);
        SkExifMetadata exif(data);
        float hdrHeadroom = 0.f;
        REPORTER_ASSERT(r, exif.getHdrHeadroom(&hdrHeadroom));
        REPORTER_ASSERT(r, approx_eq(hdrHeadroom, 3.755296f, kEpsilon));

        uint16_t resolutionUnit = 0;
        float xResolution = 0.f;
        float yResolution = 0.f;
        REPORTER_ASSERT(r, exif.getResolutionUnit(&resolutionUnit));
        REPORTER_ASSERT(r, 2 == resolutionUnit);
        REPORTER_ASSERT(r, exif.getXResolution(&xResolution));
        REPORTER_ASSERT(r, 72.f == xResolution);
        REPORTER_ASSERT(r, exif.getYResolution(&yResolution));
        REPORTER_ASSERT(r, 72.f == yResolution);

        uint32_t pixelXDimension = 0;
        uint32_t pixelYDimension = 0;
        REPORTER_ASSERT(r, exif.getPixelXDimension(&pixelXDimension));
        REPORTER_ASSERT(r, 4032 == pixelXDimension);
        REPORTER_ASSERT(r, exif.getPixelYDimension(&pixelYDimension));
        REPORTER_ASSERT(r, 3024 == pixelYDimension);
    }

    {
        sk_sp<SkData> data = GetResourceAsData("images/test1-pixel32.exif");
        REPORTER_ASSERT(r, nullptr != data);
        SkExifMetadata exif(data);
        float hdrHeadroom = 0.f;
        REPORTER_ASSERT(r, !exif.getHdrHeadroom(&hdrHeadroom));

        uint16_t resolutionUnit = 0;
        float xResolution = 0.f;
        float yResolution = 0.f;
        REPORTER_ASSERT(r, exif.getResolutionUnit(&resolutionUnit));
        REPORTER_ASSERT(r, 2 == resolutionUnit);
        REPORTER_ASSERT(r, exif.getXResolution(&xResolution));
        REPORTER_ASSERT(r, 72.f == xResolution);
        REPORTER_ASSERT(r, exif.getYResolution(&yResolution));
        REPORTER_ASSERT(r, 72.f == yResolution);

        uint32_t pixelXDimension = 0;
        uint32_t pixelYDimension = 0;
        REPORTER_ASSERT(r, exif.getPixelXDimension(&pixelXDimension));
        REPORTER_ASSERT(r, 200 == pixelXDimension);
        REPORTER_ASSERT(r, exif.getPixelYDimension(&pixelYDimension));
        REPORTER_ASSERT(r, 100 == pixelYDimension);
    }

    {
        sk_sp<SkData> data = GetResourceAsData("images/test2-nonuniform.exif");
        REPORTER_ASSERT(r, nullptr != data);
        SkExifMetadata exif(data);
        float hdrHeadroom = 0.f;
        REPORTER_ASSERT(r, !exif.getHdrHeadroom(&hdrHeadroom));

        uint16_t resolutionUnit = 0;
        float xResolution = 0.f;
        float yResolution = 0.f;
        REPORTER_ASSERT(r, exif.getResolutionUnit(&resolutionUnit));
        REPORTER_ASSERT(r, 2 == resolutionUnit);
        REPORTER_ASSERT(r, exif.getXResolution(&xResolution));
        REPORTER_ASSERT(r, 144.f == xResolution);
        REPORTER_ASSERT(r, exif.getYResolution(&yResolution));
        REPORTER_ASSERT(r, 36.f == yResolution);

        uint32_t pixelXDimension = 0;
        uint32_t pixelYDimension = 0;
        REPORTER_ASSERT(r, exif.getPixelXDimension(&pixelXDimension));
        REPORTER_ASSERT(r, 50 == pixelXDimension);
        REPORTER_ASSERT(r, exif.getPixelYDimension(&pixelYDimension));
        REPORTER_ASSERT(r, 100 == pixelYDimension);
    }

    {
        sk_sp<SkData> data = GetResourceAsData("images/test3-little-endian.exif");
        REPORTER_ASSERT(r, nullptr != data);
        SkExifMetadata exif(data);
        float hdrHeadroom = 0.f;
        REPORTER_ASSERT(r, !exif.getHdrHeadroom(&hdrHeadroom));

        uint16_t resolutionUnit = 0;
        float xResolution = 0.f;
        float yResolution = 0.f;
        REPORTER_ASSERT(r, exif.getResolutionUnit(&resolutionUnit));
        REPORTER_ASSERT(r, 2 == resolutionUnit);
        REPORTER_ASSERT(r, exif.getXResolution(&xResolution));
        REPORTER_ASSERT(r, 350.f == xResolution);
        REPORTER_ASSERT(r, exif.getYResolution(&yResolution));
        REPORTER_ASSERT(r, 350.f == yResolution);

        uint32_t pixelXDimension = 0;
        uint32_t pixelYDimension = 0;
        REPORTER_ASSERT(r, !exif.getPixelXDimension(&pixelXDimension));
        REPORTER_ASSERT(r, !exif.getPixelYDimension(&pixelYDimension));
    }

    {
        sk_sp<SkData> data = GetResourceAsData("images/test0-hdr.exif");

        // Zero out the denominators of signed and unsigned rationals, to verify that we do not
        // divide by zero.
        data = SkData::MakeWithCopy(data->bytes(), data->size());
        memset(static_cast<uint8_t*>(data->writable_data()) + 186, 0, 4);
        memset(static_cast<uint8_t*>(data->writable_data()) + 2171, 0, 4);
        memset(static_cast<uint8_t*>(data->writable_data()) + 2240, 0, 4);

        // Parse the corrupted Exif.
        SkExifMetadata exif(data);

        // HDR headroom signed denominators are destroyed.
        float hdrHeadroom = 0.f;
        REPORTER_ASSERT(r, exif.getHdrHeadroom(&hdrHeadroom));
        REPORTER_ASSERT(r, approx_eq(hdrHeadroom, 3.482202f, kEpsilon));

        // The X resolution should be zero.
        float xResolution = 0.f;
        float yResolution = 0.f;
        REPORTER_ASSERT(r, exif.getXResolution(&xResolution));
        REPORTER_ASSERT(r, 0.f == xResolution);
        REPORTER_ASSERT(r, exif.getYResolution(&yResolution));
        REPORTER_ASSERT(r, 72.f == yResolution);
    }
}
