/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/codec/SkCodec.h"
#include "include/codec/SkEncodedOrigin.h"
#include "include/codec/SkWebpDecoder.h"
#include "include/core/SkBitmap.h"
#include "include/core/SkImage.h"
#include "include/core/SkSize.h"
#include "include/core/SkStream.h"
#include "include/encode/SkJpegEncoder.h"
#include "include/private/SkExif.h"
#include "tests/Test.h"
#include "tools/Resources.h"

#include <memory>
#include <tuple>
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

DEF_TEST(GetImageRespectsExif, r) {
    std::unique_ptr<SkStream> stream(GetResourceAsStream("images/orientation/6.webp"));
    REPORTER_ASSERT(r, nullptr != stream);
    if (!stream) {
        return;
    }

    std::unique_ptr<SkCodec> codec(SkWebpDecoder::Decode(std::move(stream), nullptr));
    REPORTER_ASSERT(r, nullptr != codec);
    SkEncodedOrigin origin = codec->getOrigin();
    REPORTER_ASSERT(r, kRightTop_SkEncodedOrigin == origin,
                    "Actual origin %d", origin);

    auto result = codec->getImage();
    REPORTER_ASSERT(r, std::get<1>(result) == SkCodec::Result::kSuccess,
                    "Not success %d", std::get<1>(result));
    sk_sp<SkImage> frame = std::get<0>(result);
    REPORTER_ASSERT(r, frame);
    SkISize dims = frame->dimensions();
    REPORTER_ASSERT(r, dims.fWidth == 100, "width %d != 100", dims.fWidth);
    REPORTER_ASSERT(r, dims.fHeight == 80, "height %d != 80", dims.fHeight);
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
        SkExif::Metadata exif;
        SkExif::Parse(exif, data.get());
        REPORTER_ASSERT(r, exif.fHdrHeadroom.has_value());
        REPORTER_ASSERT(r, approx_eq(exif.fHdrHeadroom.value(), 3.755296f, kEpsilon));

        REPORTER_ASSERT(r, exif.fResolutionUnit.has_value());
        REPORTER_ASSERT(r, 2 == exif.fResolutionUnit.value());
        REPORTER_ASSERT(r, exif.fXResolution.has_value());
        REPORTER_ASSERT(r, 72.f == exif.fXResolution.value());
        REPORTER_ASSERT(r, exif.fYResolution.has_value());
        REPORTER_ASSERT(r, 72.f == exif.fYResolution.value());

        REPORTER_ASSERT(r, exif.fPixelXDimension.has_value());
        REPORTER_ASSERT(r, 4032 == exif.fPixelXDimension.value());
        REPORTER_ASSERT(r, exif.fPixelYDimension.has_value());
        REPORTER_ASSERT(r, 3024 == exif.fPixelYDimension.value());
    }

    {
        sk_sp<SkData> data = GetResourceAsData("images/test1-pixel32.exif");
        REPORTER_ASSERT(r, nullptr != data);
        SkExif::Metadata exif;
        SkExif::Parse(exif, data.get());
        REPORTER_ASSERT(r, !exif.fHdrHeadroom.has_value());

        REPORTER_ASSERT(r, exif.fResolutionUnit.value());
        REPORTER_ASSERT(r, 2 == exif.fResolutionUnit.value());
        REPORTER_ASSERT(r, exif.fXResolution.has_value());
        REPORTER_ASSERT(r, 72.f == exif.fXResolution.value());
        REPORTER_ASSERT(r, exif.fYResolution.has_value());
        REPORTER_ASSERT(r, 72.f == exif.fYResolution.value());

        REPORTER_ASSERT(r, exif.fPixelXDimension.has_value());
        REPORTER_ASSERT(r, 200 == exif.fPixelXDimension.value());
        REPORTER_ASSERT(r, exif.fPixelYDimension.has_value());
        REPORTER_ASSERT(r, 100 == exif.fPixelYDimension.value());
    }

    {
        sk_sp<SkData> data = GetResourceAsData("images/test2-nonuniform.exif");
        REPORTER_ASSERT(r, nullptr != data);
        SkExif::Metadata exif;
        SkExif::Parse(exif, data.get());
        REPORTER_ASSERT(r, !exif.fHdrHeadroom.has_value());

        REPORTER_ASSERT(r, exif.fResolutionUnit.value());
        REPORTER_ASSERT(r, 2 == exif.fResolutionUnit.value());
        REPORTER_ASSERT(r, exif.fXResolution.has_value());
        REPORTER_ASSERT(r, 144.f == exif.fXResolution.value());
        REPORTER_ASSERT(r, exif.fYResolution.has_value());
        REPORTER_ASSERT(r, 36.f == exif.fYResolution.value());

        REPORTER_ASSERT(r, exif.fPixelXDimension.has_value());
        REPORTER_ASSERT(r, 50 == exif.fPixelXDimension.value());
        REPORTER_ASSERT(r, exif.fPixelYDimension.has_value());
        REPORTER_ASSERT(r, 100 == exif.fPixelYDimension.value());
    }

    {
        sk_sp<SkData> data = GetResourceAsData("images/test3-little-endian.exif");
        REPORTER_ASSERT(r, nullptr != data);
        SkExif::Metadata exif;
        SkExif::Parse(exif, data.get());
        REPORTER_ASSERT(r, !exif.fHdrHeadroom.has_value());

        REPORTER_ASSERT(r, exif.fResolutionUnit.value());
        REPORTER_ASSERT(r, 2 == exif.fResolutionUnit.value());
        REPORTER_ASSERT(r, exif.fXResolution.has_value());
        REPORTER_ASSERT(r, 350.f == exif.fXResolution.value());
        REPORTER_ASSERT(r, exif.fYResolution.has_value());
        REPORTER_ASSERT(r, 350.f == exif.fYResolution.value());

        REPORTER_ASSERT(r, !exif.fPixelXDimension.has_value());
        REPORTER_ASSERT(r, !exif.fPixelYDimension.has_value());
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
        SkExif::Metadata exif;
        SkExif::Parse(exif, data.get());

        // HDR headroom signed denominators are destroyed.
        REPORTER_ASSERT(r, exif.fHdrHeadroom.has_value());
        REPORTER_ASSERT(r, approx_eq(exif.fHdrHeadroom.value(), 3.482202f, kEpsilon));

        // The X resolution should be zero.
        REPORTER_ASSERT(r, exif.fXResolution.has_value());
        REPORTER_ASSERT(r, 0.f == exif.fXResolution.value());
        REPORTER_ASSERT(r, exif.fYResolution.has_value());
        REPORTER_ASSERT(r, 72.f == exif.fYResolution.value());
    }
}

DEF_TEST(ExifTruncate, r) {
    sk_sp<SkData> data = GetResourceAsData("images/test0-hdr.exif");

    // At 545 bytes, we do not have either value yet.
    {
        SkExif::Metadata exif;
        SkExif::Parse(exif, SkData::MakeWithCopy(data->bytes(), 545).get());
        REPORTER_ASSERT(r, !exif.fPixelXDimension.has_value());
        REPORTER_ASSERT(r, !exif.fPixelYDimension.has_value());
    }

    // At 546 bytes, we have one.
    {
        SkExif::Metadata exif;
        SkExif::Parse(exif, SkData::MakeWithCopy(data->bytes(), 546).get());
        REPORTER_ASSERT(r, exif.fPixelXDimension.has_value());
        REPORTER_ASSERT(r, !exif.fPixelYDimension.has_value());
    }

    // At 558 bytes (12 bytes later, one tag), we have both.
    {
        SkExif::Metadata exif;
        SkExif::Parse(exif, SkData::MakeWithCopy(data->bytes(), 558).get());
        REPORTER_ASSERT(r, exif.fPixelXDimension.has_value());
        REPORTER_ASSERT(r, exif.fPixelYDimension.has_value());
    }
}

bool metadata_are_equal(const SkExif::Metadata& m1, const SkExif::Metadata& m2) {
   return m1.fOrigin == m2.fOrigin &&
          m1.fHdrHeadroom == m2.fHdrHeadroom &&
          m1.fResolutionUnit == m2.fResolutionUnit &&
          m1.fXResolution == m2.fXResolution &&
          m1.fYResolution == m2.fYResolution &&
          m1.fPixelXDimension == m2.fPixelXDimension &&
          m1.fPixelYDimension == m2.fPixelYDimension;
}

DEF_TEST(ExifWriteOrientation, r) {
    SkBitmap bm;
    bm.allocPixels(SkImageInfo::MakeN32Premul(100, 100));
    bm.eraseColor(SK_ColorBLUE);
    SkPixmap pm;
    if (!bm.peekPixels(&pm)) {
        ERRORF(r, "failed to peek pixels");
        return;
    }
    for (auto o : { kTopLeft_SkEncodedOrigin,
                    kTopRight_SkEncodedOrigin,
                    kBottomRight_SkEncodedOrigin,
                    kBottomLeft_SkEncodedOrigin,
                    kLeftTop_SkEncodedOrigin,
                    kRightTop_SkEncodedOrigin,
                    kRightBottom_SkEncodedOrigin,
                    kLeftBottom_SkEncodedOrigin }) {
        SkDynamicMemoryWStream stream;
        SkJpegEncoder::Options options;
        options.fOrigin = o;
        if (!SkJpegEncoder::Encode(&stream, pm, options)) {
            ERRORF(r, "Failed to encode with orientation %i", o);
            return;
        }

        auto data = stream.detachAsData();
        auto codec = SkCodec::MakeFromData(std::move(data));
        if (!codec) {
            ERRORF(r, "Failed to create a codec with orientation %i", o);
            return;
        }
        REPORTER_ASSERT(r, codec->getOrigin() == o);
    }
}

DEF_TEST(ExifWriteTest, r) {
  {
    // Parse exif data
    sk_sp<SkData> data = GetResourceAsData("images/test2-nonuniform.exif");
    REPORTER_ASSERT(r, nullptr != data);
    SkExif::Metadata exif;
    SkExif::Parse(exif, data.get());

    // Write parsed data back to exif
    sk_sp<SkData> writeData = SkExif::WriteExif(exif);
    REPORTER_ASSERT(r, nullptr != writeData);

    // Parse the new exif data that was written
    SkExif::Metadata writeExif;
    SkExif::Parse(writeExif, writeData.get());

    REPORTER_ASSERT(r, metadata_are_equal(writeExif, exif));
  }
}

// We are not able to write HDRheadroom data from a Metadata instance so WriteExif
// should fail and return nullptr when fHdrHeadroom is present.
DEF_TEST(ExifWriteFailsHDRheadroom, r) {
    sk_sp<SkData> data = GetResourceAsData("images/test0-hdr.exif");
    REPORTER_ASSERT(r, nullptr != data);
    SkExif::Metadata exif;
    SkExif::Parse(exif, data.get());

    // Write parsed data back to exif
    sk_sp<SkData> writeData = SkExif::WriteExif(exif);
    REPORTER_ASSERT(r, nullptr == writeData);
}

