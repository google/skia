/*
 * Copyright 2013 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/codec/SkCodec.h"
#include "include/codec/SkEncodedOrigin.h"
#include "include/core/SkColorType.h"
#include "include/core/SkData.h"
#include "include/core/SkImageInfo.h"
#include "include/core/SkPixmap.h"
#include "include/core/SkScalar.h"
#include "include/core/SkSize.h"
#include "include/core/SkStream.h"
#include "include/core/SkTypes.h"
#include "include/core/SkYUVAInfo.h"
#include "include/core/SkYUVAPixmaps.h"
#include "include/effects/SkColorMatrix.h"
#include "include/encode/SkJpegEncoder.h"
#include "include/private/base/SkTo.h"
#include "tests/Test.h"
#include "tools/Resources.h"

#include <cmath>
#include <cstdint>
#include <memory>
#include <utility>

static void codec_yuv(skiatest::Reporter* reporter,
                      const char path[],
                      const SkYUVAInfo* expectedInfo) {
    std::unique_ptr<SkStream> stream(GetResourceAsStream(path));
    if (!stream) {
        return;
    }
    std::unique_ptr<SkCodec> codec(SkCodec::MakeFromStream(std::move(stream)));
    REPORTER_ASSERT(reporter, codec);
    if (!codec) {
        return;
    }

    // Test queryYUBAInfo()
    SkYUVAPixmapInfo yuvaPixmapInfo;

    static constexpr auto kAllTypes = SkYUVAPixmapInfo::SupportedDataTypes::All();
    static constexpr auto kNoTypes  = SkYUVAPixmapInfo::SupportedDataTypes();

    // SkYUVAInfo param is required to be non-null.
    bool success = codec->queryYUVAInfo(kAllTypes, nullptr);
    REPORTER_ASSERT(reporter, !success);
    // Fails when there is no support for YUVA planes.
    success = codec->queryYUVAInfo(kNoTypes, &yuvaPixmapInfo);
    REPORTER_ASSERT(reporter, !success);

    success = codec->queryYUVAInfo(kAllTypes, &yuvaPixmapInfo);
    REPORTER_ASSERT(reporter, SkToBool(expectedInfo) == success);
    if (!success) {
        return;
    }
    REPORTER_ASSERT(reporter, *expectedInfo == yuvaPixmapInfo.yuvaInfo());

    int numPlanes = yuvaPixmapInfo.numPlanes();
    REPORTER_ASSERT(reporter, numPlanes <= SkYUVAInfo::kMaxPlanes);
    for (int i = 0; i < numPlanes; ++i) {
        const SkImageInfo& planeInfo = yuvaPixmapInfo.planeInfo(i);
        SkColorType planeCT = planeInfo.colorType();
        REPORTER_ASSERT(reporter, !planeInfo.isEmpty());
        REPORTER_ASSERT(reporter, planeCT != kUnknown_SkColorType);
        REPORTER_ASSERT(reporter, planeInfo.validRowBytes(yuvaPixmapInfo.rowBytes(i)));
        // Currently all planes must share a data type, gettable as SkYUVAPixmapInfo::dataType().
        auto [numChannels, planeDataType] = SkYUVAPixmapInfo::NumChannelsAndDataType(planeCT);
        REPORTER_ASSERT(reporter, planeDataType == yuvaPixmapInfo.dataType());
    }
    for (int i = numPlanes; i < SkYUVAInfo::kMaxPlanes; ++i) {
        const SkImageInfo& planeInfo = yuvaPixmapInfo.planeInfo(i);
        REPORTER_ASSERT(reporter, planeInfo.dimensions().isEmpty());
        REPORTER_ASSERT(reporter, planeInfo.colorType() == kUnknown_SkColorType);
        REPORTER_ASSERT(reporter, yuvaPixmapInfo.rowBytes(i) == 0);
    }

    // Allocate the memory for the YUV decode.
    auto pixmaps = SkYUVAPixmaps::Allocate(yuvaPixmapInfo);
    REPORTER_ASSERT(reporter, pixmaps.isValid());

    for (int i = 0; i < SkYUVAPixmaps::kMaxPlanes; ++i) {
        REPORTER_ASSERT(reporter, pixmaps.plane(i).info() == yuvaPixmapInfo.planeInfo(i));
    }
    for (int i = numPlanes; i < SkYUVAInfo::kMaxPlanes; ++i) {
        REPORTER_ASSERT(reporter, pixmaps.plane(i).rowBytes() == 0);
    }

    // Test getYUVAPlanes()
    REPORTER_ASSERT(reporter, SkCodec::kSuccess == codec->getYUVAPlanes(pixmaps));
}

DEF_TEST(Jpeg_YUV_Codec, r) {
    auto setExpectations = [](SkISize dims, SkYUVAInfo::Subsampling subsampling) {
        return SkYUVAInfo(dims,
                          SkYUVAInfo::PlaneConfig::kY_U_V,
                          subsampling,
                          kJPEG_Full_SkYUVColorSpace,
                          kTopLeft_SkEncodedOrigin,
                          SkYUVAInfo::Siting::kCentered,
                          SkYUVAInfo::Siting::kCentered);
    };

    SkYUVAInfo expectations = setExpectations({128, 128}, SkYUVAInfo::Subsampling::k420);
    codec_yuv(r, "images/color_wheel.jpg", &expectations);

    // H2V2
    expectations = setExpectations({512, 512}, SkYUVAInfo::Subsampling::k420);
    codec_yuv(r, "images/mandrill_512_q075.jpg", &expectations);

    // H1V1
    expectations = setExpectations({512, 512}, SkYUVAInfo::Subsampling::k444);
    codec_yuv(r, "images/mandrill_h1v1.jpg", &expectations);

    // H2V1
    expectations = setExpectations({512, 512}, SkYUVAInfo::Subsampling::k422);
    codec_yuv(r, "images/mandrill_h2v1.jpg", &expectations);

    // Non-power of two dimensions
    expectations = setExpectations({439, 154}, SkYUVAInfo::Subsampling::k420);
    codec_yuv(r, "images/cropped_mandrill.jpg", &expectations);

    expectations = setExpectations({8, 8}, SkYUVAInfo::Subsampling::k420);
    codec_yuv(r, "images/randPixels.jpg", &expectations);

    // Progressive images
    expectations = setExpectations({512, 512}, SkYUVAInfo::Subsampling::k444);
    codec_yuv(r, "images/brickwork-texture.jpg", &expectations);
    codec_yuv(r, "images/brickwork_normal-map.jpg", &expectations);

    // A CMYK encoded image should fail.
    codec_yuv(r, "images/CMYK.jpg", nullptr);
    // A grayscale encoded image should fail.
    codec_yuv(r, "images/grayscale.jpg", nullptr);
    // A PNG should fail.
    codec_yuv(r, "images/arrow.png", nullptr);
}

SkYUVAPixmaps decode_yuva(skiatest::Reporter* r, std::unique_ptr<SkStream> stream) {
    static constexpr auto kAllTypes = SkYUVAPixmapInfo::SupportedDataTypes::All();
    SkYUVAPixmaps result;

    std::unique_ptr<SkCodec> codec(SkCodec::MakeFromStream(std::move(stream)));
    REPORTER_ASSERT(r, codec);

    SkYUVAPixmapInfo yuvaPixmapInfo;
    REPORTER_ASSERT(r, codec->queryYUVAInfo(kAllTypes, &yuvaPixmapInfo));
    result = SkYUVAPixmaps::Allocate(yuvaPixmapInfo);
    REPORTER_ASSERT(r, result.isValid());
    REPORTER_ASSERT(r, SkCodec::kSuccess == codec->getYUVAPlanes(result));
    return result;
}

static void verify_same(skiatest::Reporter* r, const SkYUVAPixmaps a, const SkYUVAPixmaps& b) {
    REPORTER_ASSERT(r, a.yuvaInfo() == b.yuvaInfo());
    REPORTER_ASSERT(r, a.numPlanes() == b.numPlanes());
    for (int plane = 0; plane < a.numPlanes(); ++plane) {
        const SkPixmap& aPlane = a.plane(plane);
        const SkPixmap& bPlane = b.plane(plane);
        REPORTER_ASSERT(r, aPlane.computeByteSize() == bPlane.computeByteSize());
        const uint8_t* aData = reinterpret_cast<const uint8_t*>(aPlane.addr());
        const uint8_t* bData = reinterpret_cast<const uint8_t*>(bPlane.addr());
        for (int row = 0; row < aPlane.height(); ++row) {
            for (int col = 0; col < aPlane.width() * aPlane.info().bytesPerPixel(); ++col) {
                int32_t aByte = aData[col];
                int32_t bByte = bData[col];
                // Allow at most one bit of difference.
                REPORTER_ASSERT(r, std::abs(aByte - bByte) <= 1);
            }
            aData += aPlane.rowBytes();
            bData += bPlane.rowBytes();
        }
    }
}

DEF_TEST(Jpeg_YUV_Encode, r) {
    const char* paths[] = {
            "images/color_wheel.jpg",
            "images/mandrill_512_q075.jpg",
            "images/mandrill_h1v1.jpg",
            "images/mandrill_h2v1.jpg",
            "images/cropped_mandrill.jpg",
            "images/randPixels.jpg",
    };
    for (const auto* path : paths) {
        SkYUVAPixmaps decoded;
        {
            std::unique_ptr<SkStream> stream(GetResourceAsStream(path));
            decoded = decode_yuva(r, std::move(stream));
        }

        SkYUVAPixmaps roundtrip;
        {
            SkJpegEncoder::Options options;
            SkDynamicMemoryWStream encodeStream;
            REPORTER_ASSERT(r, SkJpegEncoder::Encode(&encodeStream, decoded, nullptr, options));
            auto encodedData = encodeStream.detachAsData();
            roundtrip = decode_yuva(r, SkMemoryStream::Make(encodedData));
        }

        verify_same(r, decoded, roundtrip);
    }
}

// Be sure that the two matrices are inverses of each other
// (i.e. rgb2yuv and yuv2rgb
DEF_TEST(YUVMath, reporter) {
    const SkYUVColorSpace spaces[] = {
        kJPEG_SkYUVColorSpace,
        kRec601_SkYUVColorSpace,
        kRec709_SkYUVColorSpace,
        kBT2020_SkYUVColorSpace,
        kIdentity_SkYUVColorSpace,
    };

    // Not sure what the theoretical precision we can hope for is, so pick a big value that
    // passes (when I think we're correct).
    const float tolerance = 1.0f/(1 << 18);

    for (auto cs : spaces) {
        SkColorMatrix r2ym = SkColorMatrix::RGBtoYUV(cs),
                      y2rm = SkColorMatrix::YUVtoRGB(cs);
        r2ym.postConcat(y2rm);

        float tmp[20];
        r2ym.getRowMajor(tmp);
        for (int i = 0; i < 20; ++i) {
            float expected = 0;
            if (i % 6 == 0) {   // diagonal
                expected = 1;
            }
            REPORTER_ASSERT(reporter, SkScalarNearlyEqual(tmp[i], expected, tolerance));
        }
    }
}
