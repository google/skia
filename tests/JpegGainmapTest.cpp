/*
 * Copyright 2023 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/codec/SkAndroidCodec.h"
#include "include/codec/SkCodec.h"
#include "include/core/SkBitmap.h"
#include "include/core/SkCanvas.h"
#include "include/core/SkColor.h"
#include "include/core/SkImage.h"
#include "include/core/SkShader.h"
#include "include/core/SkSize.h"
#include "include/core/SkStream.h"
#include "include/core/SkTypes.h"
#include "include/encode/SkJpegEncoder.h"
#include "include/private/SkGainmapInfo.h"
#include "include/private/SkGainmapShader.h"
#include "include/private/SkJpegGainmapEncoder.h"
#include "src/codec/SkJpegCodec.h"
#include "src/codec/SkJpegConstants.h"
#include "src/codec/SkJpegMultiPicture.h"
#include "src/codec/SkJpegSegmentScan.h"
#include "src/codec/SkJpegSourceMgr.h"
#include "tests/Test.h"
#include "tools/Resources.h"

#include <cstdint>
#include <cstring>
#include <memory>
#include <utility>
#include <vector>

namespace {

// Return true if the relative difference between x and y is less than epsilon.
static bool approx_eq(float x, float y, float epsilon) {
    float numerator = std::abs(x - y);
    // To avoid being too sensitive around zero, set the minimum denominator to epsilon.
    float denominator = std::max(std::min(std::abs(x), std::abs(y)), epsilon);
    if (numerator / denominator > epsilon) {
        return false;
    }
    return true;
}

static bool approx_eq(const SkColor4f& x, const SkColor4f& y, float epsilon) {
    return approx_eq(x.fR, y.fR, epsilon) && approx_eq(x.fG, y.fG, epsilon) &&
           approx_eq(x.fB, y.fB, epsilon);
}

template <typename Reporter>
void expect_approx_eq_info(Reporter& r, const SkGainmapInfo& a, const SkGainmapInfo& b) {
    float kEpsilon = 1e-4f;
    REPORTER_ASSERT(r, approx_eq(a.fGainmapRatioMin, b.fGainmapRatioMin, kEpsilon));
    REPORTER_ASSERT(r, approx_eq(a.fGainmapRatioMin, b.fGainmapRatioMin, kEpsilon));
    REPORTER_ASSERT(r, approx_eq(a.fGainmapGamma, b.fGainmapGamma, kEpsilon));
    REPORTER_ASSERT(r, approx_eq(a.fEpsilonSdr, b.fEpsilonSdr, kEpsilon));
    REPORTER_ASSERT(r, approx_eq(a.fEpsilonHdr, b.fEpsilonHdr, kEpsilon));
    REPORTER_ASSERT(r, approx_eq(a.fDisplayRatioSdr, b.fDisplayRatioSdr, kEpsilon));
    REPORTER_ASSERT(r, approx_eq(a.fDisplayRatioHdr, b.fDisplayRatioHdr, kEpsilon));
    REPORTER_ASSERT(r, a.fType == b.fType);
    REPORTER_ASSERT(r, a.fBaseImageType == b.fBaseImageType);

    REPORTER_ASSERT(r, !!a.fGainmapMathColorSpace == !!b.fGainmapMathColorSpace);
    if (a.fGainmapMathColorSpace) {
        skcms_TransferFunction a_fn;
        skcms_Matrix3x3 a_m;
        a.fGainmapMathColorSpace->transferFn(&a_fn);
        a.fGainmapMathColorSpace->toXYZD50(&a_m);
        skcms_TransferFunction b_fn;
        skcms_Matrix3x3 b_m;
        b.fGainmapMathColorSpace->transferFn(&b_fn);
        b.fGainmapMathColorSpace->toXYZD50(&b_m);

        REPORTER_ASSERT(r, approx_eq(a_fn.g, b_fn.g, kEpsilon));
        REPORTER_ASSERT(r, approx_eq(a_fn.a, b_fn.a, kEpsilon));
        REPORTER_ASSERT(r, approx_eq(a_fn.b, b_fn.b, kEpsilon));
        REPORTER_ASSERT(r, approx_eq(a_fn.c, b_fn.c, kEpsilon));
        REPORTER_ASSERT(r, approx_eq(a_fn.d, b_fn.d, kEpsilon));
        REPORTER_ASSERT(r, approx_eq(a_fn.e, b_fn.e, kEpsilon));
        REPORTER_ASSERT(r, approx_eq(a_fn.f, b_fn.f, kEpsilon));

        // The round-trip of the color space through the ICC profile loses significant precision.
        // Use a larger epsilon for it.
        const float kMatrixEpsilon = 1e-2f;
        for (int i = 0; i < 3; ++i) {
            for (int j = 0; j < 3; ++j) {
                REPORTER_ASSERT(r, approx_eq(a_m.vals[i][j], b_m.vals[i][j], kMatrixEpsilon));
            }
        }
    }
}

// A test stream to stress the different SkJpegSourceMgr sub-classes.
class TestStream : public SkStream {
public:
    enum class Type {
        kUnseekable,    // SkJpegUnseekableSourceMgr
        kSeekable,      // SkJpegBufferedSourceMgr
        kMemoryMapped,  // SkJpegMemorySourceMgr
    };
    TestStream(Type type, SkStream* stream)
            : fStream(stream)
            , fSeekable(type != Type::kUnseekable)
            , fMemoryMapped(type == Type::kMemoryMapped) {}
    ~TestStream() override {}

    size_t read(void* buffer, size_t size) override { return fStream->read(buffer, size); }
    size_t peek(void* buffer, size_t size) const override { return fStream->peek(buffer, size); }
    bool isAtEnd() const override { return fStream->isAtEnd(); }
    bool rewind() override {
        if (!fSeekable) {
            return false;
        }
        return fStream->rewind();
    }
    bool hasPosition() const override {
        if (!fSeekable) {
            return false;
        }
        return fStream->hasPosition();
    }
    size_t getPosition() const override {
        if (!fSeekable) {
            return 0;
        }
        return fStream->hasPosition();
    }
    bool seek(size_t position) override {
        if (!fSeekable) {
            return 0;
        }
        return fStream->seek(position);
    }
    bool move(long offset) override {
        if (!fSeekable) {
            return 0;
        }
        return fStream->move(offset);
    }
    bool hasLength() const override {
        if (!fMemoryMapped) {
            return false;
        }
        return fStream->hasLength();
    }
    size_t getLength() const override {
        if (!fMemoryMapped) {
            return 0;
        }
        return fStream->getLength();
    }
    const void* getMemoryBase() override {
        if (!fMemoryMapped) {
            return nullptr;
        }
        return fStream->getMemoryBase();
    }

private:
    SkStream* const fStream;
    bool fSeekable = false;
    bool fMemoryMapped = false;
};

}  // namespace

DEF_TEST(Codec_jpegSegmentScan, r) {
    const struct Rec {
        const char* path;
        size_t sosSegmentCount;
        size_t eoiSegmentCount;
        size_t testSegmentIndex;
        uint8_t testSegmentMarker;
        size_t testSegmentOffset;
        uint16_t testSegmentParameterLength;
    } recs[] = {
            {"images/wide_gamut_yellow_224_224_64.jpeg", 11, 15, 10, 0xda, 9768, 12},
            {"images/CMYK.jpg", 7, 8, 1, 0xee, 2, 14},
            {"images/b78329453.jpeg", 10, 23, 3, 0xe2, 154, 540},
            {"images/brickwork-texture.jpg", 8, 28, 12, 0xc4, 34183, 42},
            {"images/brickwork_normal-map.jpg", 8, 28, 27, 0xd9, 180612, 0},
            {"images/cmyk_yellow_224_224_32.jpg", 19, 23, 2, 0xed, 854, 2828},
            {"images/color_wheel.jpg", 10, 11, 2, 0xdb, 20, 67},
            {"images/cropped_mandrill.jpg", 10, 11, 4, 0xc0, 158, 17},
            {"images/dog.jpg", 10, 11, 5, 0xc4, 177, 28},
            {"images/ducky.jpg", 12, 13, 10, 0xc4, 3718, 181},
            {"images/exif-orientation-2-ur.jpg", 11, 12, 2, 0xe1, 20, 130},
            {"images/flutter_logo.jpg", 9, 27, 21, 0xda, 5731, 8},
            {"images/grayscale.jpg", 6, 16, 9, 0xda, 327, 8},
            {"images/icc-v2-gbr.jpg", 12, 25, 24, 0xd9, 43832, 0},
            {"images/mandrill_512_q075.jpg", 10, 11, 7, 0xc4, 393, 31},
            {"images/mandrill_cmyk.jpg", 19, 35, 16, 0xdd, 574336, 4},
            {"images/mandrill_h1v1.jpg", 10, 11, 1, 0xe0, 2, 16},
            {"images/mandrill_h2v1.jpg", 10, 11, 0, 0xd8, 0, 0},
            {"images/randPixels.jpg", 10, 11, 6, 0xc4, 200, 30},
            {"images/wide_gamut_yellow_224_224_64.jpeg", 11, 15, 10, 0xda, 9768, 12},
    };

    for (const auto& rec : recs) {
        auto stream = GetResourceAsStream(rec.path);
        if (!stream) {
            continue;
        }

        // Scan all the way to EndOfImage.
        auto sourceMgr = SkJpegSourceMgr::Make(stream.get());
        const auto& segments = sourceMgr->getAllSegments();

        // Verify we got the expected number of segments at EndOfImage
        REPORTER_ASSERT(r, rec.eoiSegmentCount == segments.size());

        // Verify we got the expected number of segments before StartOfScan
        for (size_t i = 0; i < segments.size(); ++i) {
            if (segments[i].marker == kJpegMarkerStartOfScan) {
                REPORTER_ASSERT(r, rec.sosSegmentCount == i + 1);
                break;
            }
        }

        // Verify the values for a randomly pre-selected segment index.
        const auto& segment = segments[rec.testSegmentIndex];
        REPORTER_ASSERT(r, rec.testSegmentMarker == segment.marker);
        REPORTER_ASSERT(r, rec.testSegmentOffset == segment.offset);
        REPORTER_ASSERT(r, rec.testSegmentParameterLength == segment.parameterLength);
    }
}

static bool find_mp_params_segment(SkStream* stream,
                                   std::unique_ptr<SkJpegMultiPictureParameters>* outMpParams,
                                   SkJpegSegment* outMpParamsSegment) {
    auto sourceMgr = SkJpegSourceMgr::Make(stream);
    for (const auto& segment : sourceMgr->getAllSegments()) {
        if (segment.marker != kMpfMarker) {
            continue;
        }
        auto parameterData = sourceMgr->getSegmentParameters(segment);
        if (!parameterData) {
            continue;
        }
        *outMpParams = SkJpegMultiPictureParameters::Make(parameterData);
        if (*outMpParams) {
            *outMpParamsSegment = segment;
            return true;
        }
    }
    return false;
}

DEF_TEST(Codec_multiPictureParams, r) {
    // Little-endian test.
    {
        const uint8_t bytes[] = {
                0x4d, 0x50, 0x46, 0x00, 0x49, 0x49, 0x2a, 0x00, 0x08, 0x00, 0x00, 0x00, 0x03,
                0x00, 0x00, 0xb0, 0x07, 0x00, 0x04, 0x00, 0x00, 0x00, 0x30, 0x31, 0x30, 0x30,
                0x01, 0xb0, 0x04, 0x00, 0x01, 0x00, 0x00, 0x00, 0x02, 0x00, 0x00, 0x00, 0x02,
                0xb0, 0x07, 0x00, 0x20, 0x00, 0x00, 0x00, 0x32, 0x00, 0x00, 0x00, 0x00, 0x00,
                0x00, 0x00, 0x00, 0x00, 0x03, 0x00, 0x20, 0xcf, 0x49, 0x00, 0x00, 0x00, 0x00,
                0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xee, 0x28, 0x01, 0x00,
                0xf9, 0xb7, 0x3c, 0x00, 0x00, 0x00, 0x00, 0x00,
        };
        auto mpParams =
                SkJpegMultiPictureParameters::Make(SkData::MakeWithoutCopy(bytes, sizeof(bytes)));
        REPORTER_ASSERT(r, mpParams);
        REPORTER_ASSERT(r, mpParams->images.size() == 2);
        REPORTER_ASSERT(r, mpParams->images[0].dataOffset == 0);
        REPORTER_ASSERT(r, mpParams->images[0].size == 4837152);
        REPORTER_ASSERT(r, mpParams->images[1].dataOffset == 3979257);
        REPORTER_ASSERT(r, mpParams->images[1].size == 76014);
    }

    // Big-endian test.
    {
        const uint8_t bytes[] = {
                0x4d, 0x50, 0x46, 0x00, 0x4d, 0x4d, 0x00, 0x2a, 0x00, 0x00, 0x00, 0x08, 0x00,
                0x03, 0xb0, 0x00, 0x00, 0x07, 0x00, 0x00, 0x00, 0x04, 0x30, 0x31, 0x30, 0x30,
                0xb0, 0x01, 0x00, 0x04, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0x02, 0xb0,
                0x02, 0x00, 0x07, 0x00, 0x00, 0x00, 0x20, 0x00, 0x00, 0x00, 0x32, 0x00, 0x00,
                0x00, 0x00, 0x20, 0x03, 0x00, 0x00, 0x00, 0x56, 0xda, 0x2f, 0x00, 0x00, 0x00,
                0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x14, 0xc6, 0x01,
                0x00, 0x55, 0x7c, 0x1f, 0x00, 0x00, 0x00, 0x00,
        };
        auto mpParams =
                SkJpegMultiPictureParameters::Make(SkData::MakeWithoutCopy(bytes, sizeof(bytes)));
        REPORTER_ASSERT(r, mpParams);
        REPORTER_ASSERT(r, mpParams->images.size() == 2);
        REPORTER_ASSERT(r, mpParams->images[0].dataOffset == 0);
        REPORTER_ASSERT(r, mpParams->images[0].size == 5691951);
        REPORTER_ASSERT(r, mpParams->images[1].dataOffset == 5602335);
        REPORTER_ASSERT(r, mpParams->images[1].size == 1361409);
    }

    // Three entry test.
    {
        const uint8_t bytes[] = {
                0x4d, 0x50, 0x46, 0x00, 0x4d, 0x4d, 0x00, 0x2a, 0x00, 0x00, 0x00, 0x08, 0x00,
                0x03, 0xb0, 0x00, 0x00, 0x07, 0x00, 0x00, 0x00, 0x04, 0x30, 0x31, 0x30, 0x30,
                0xb0, 0x01, 0x00, 0x04, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0x03, 0xb0,
                0x02, 0x00, 0x07, 0x00, 0x00, 0x00, 0x30, 0x00, 0x00, 0x00, 0x32, 0x00, 0x00,
                0x00, 0x00, 0x00, 0x03, 0x00, 0x00, 0x00, 0x1f, 0x1c, 0xc2, 0x00, 0x00, 0x00,
                0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x03, 0x05, 0xb0,
                0x00, 0x1f, 0x12, 0xec, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
                0x00, 0x96, 0x6b, 0x00, 0x22, 0x18, 0x9c, 0x00, 0x00, 0x00, 0x00,
        };
        auto mpParams =
                SkJpegMultiPictureParameters::Make(SkData::MakeWithoutCopy(bytes, sizeof(bytes)));
        REPORTER_ASSERT(r, mpParams);
        REPORTER_ASSERT(r, mpParams->images.size() == 3);
        REPORTER_ASSERT(r, mpParams->images[0].dataOffset == 0);
        REPORTER_ASSERT(r, mpParams->images[0].size == 2038978);
        REPORTER_ASSERT(r, mpParams->images[1].dataOffset == 2036460);
        REPORTER_ASSERT(r, mpParams->images[1].size == 198064);
        REPORTER_ASSERT(r, mpParams->images[2].dataOffset == 2234524);
        REPORTER_ASSERT(r, mpParams->images[2].size == 38507);
    }

    // Inserting various corrupt values.
    {
        const uint8_t bytes[] = {
                0x4d, 0x50, 0x46, 0x00,  // 0: {'M', 'P', 'F',   0} signature
                0x4d, 0x4d, 0x00, 0x2a,  // 4: {'M', 'M',   0, '*'} big-endian
                0x00, 0x00, 0x00, 0x08,  // 8: Index IFD offset
                0x00, 0x03,              // 12: Number of tags
                0xb0, 0x00,              // 14: Version tag
                0x00, 0x07,              // 16: Undefined type
                0x00, 0x00, 0x00, 0x04,  // 18: Size
                0x30, 0x31, 0x30, 0x30,  // 22: Value
                0xb0, 0x01,              // 26: Number of images
                0x00, 0x04,              // 28: Unsigned long type
                0x00, 0x00, 0x00, 0x01,  // 30: Count
                0x00, 0x00, 0x00, 0x02,  // 34: Value
                0xb0, 0x02,              // 38: MP entry tag
                0x00, 0x07,              // 40: Undefined type
                0x00, 0x00, 0x00, 0x20,  // 42: Size
                0x00, 0x00, 0x00, 0x32,  // 46: Value (offset)
                0x00, 0x00, 0x00, 0x00,  // 50: Next IFD offset (null)
                0x20, 0x03, 0x00, 0x00,  // 54: MP Entry 0 attributes
                0x00, 0x56, 0xda, 0x2f,  // 58: MP Entry 0 size (5691951)
                0x00, 0x00, 0x00, 0x00,  // 62: MP Entry 0 offset (0)
                0x00, 0x00, 0x00, 0x00,  // 66: MP Entry 0 dependencies
                0x00, 0x00, 0x00, 0x00,  // 70: MP Entry 1 attributes.
                0x00, 0x14, 0xc6, 0x01,  // 74: MP Entry 1 size (1361409)
                0x00, 0x55, 0x7c, 0x1f,  // 78: MP Entry 1 offset (5602335)
                0x00, 0x00, 0x00, 0x00,  // 82: MP Entry 1 dependencies
        };

        // Verify the offsets labeled above.
        REPORTER_ASSERT(r, bytes[22] == 0x30);
        REPORTER_ASSERT(r, bytes[26] == 0xb0);
        REPORTER_ASSERT(r, bytes[38] == 0xb0);
        REPORTER_ASSERT(r, bytes[54] == 0x20);
        REPORTER_ASSERT(r, bytes[81] == 0x1f);

        {
            // Change the version to {'0', '1', '0', '1'}.
            auto bytesInvalid = SkData::MakeWithCopy(bytes, sizeof(bytes));
            REPORTER_ASSERT(r, bytes[25] == '0');
            reinterpret_cast<uint8_t*>(bytesInvalid->writable_data())[25] = '1';
            REPORTER_ASSERT(r, SkJpegMultiPictureParameters::Make(bytesInvalid) == nullptr);
        }

        {
            // Change the number of images to be undefined type instead of unsigned long type.
            auto bytesInvalid = SkData::MakeWithCopy(bytes, sizeof(bytes));
            REPORTER_ASSERT(r, bytes[29] == 0x04);
            reinterpret_cast<uint8_t*>(bytesInvalid->writable_data())[29] = 0x07;
            REPORTER_ASSERT(r, SkJpegMultiPictureParameters::Make(bytesInvalid) == nullptr);
        }

        {
            // Make the MP entries point off of the end of the buffer.
            auto bytesInvalid = SkData::MakeWithCopy(bytes, sizeof(bytes));
            REPORTER_ASSERT(r, bytes[49] == 0x32);
            reinterpret_cast<uint8_t*>(bytesInvalid->writable_data())[49] = 0xFE;
            REPORTER_ASSERT(r, SkJpegMultiPictureParameters::Make(bytesInvalid) == nullptr);
        }

        {
            // Make the MP entries too small.
            auto bytesInvalid = SkData::MakeWithCopy(bytes, sizeof(bytes));
            REPORTER_ASSERT(r, bytes[45] == 0x20);
            reinterpret_cast<uint8_t*>(bytesInvalid->writable_data())[45] = 0x1F;
            REPORTER_ASSERT(r, SkJpegMultiPictureParameters::Make(bytesInvalid) == nullptr);
        }
    }
}

DEF_TEST(Codec_jpegMultiPicture, r) {
    const char* path = "images/iphone_13_pro.jpeg";
    auto stream = GetResourceAsStream(path);
    REPORTER_ASSERT(r, stream);

    // Search and parse the MPF header.
    std::unique_ptr<SkJpegMultiPictureParameters> mpParams;
    SkJpegSegment mpParamsSegment;
    REPORTER_ASSERT(r, find_mp_params_segment(stream.get(), &mpParams, &mpParamsSegment));

    // Verify that we get the same parameters when we re-serialize and de-serialize them
    {
        auto mpParamsSerialized = mpParams->serialize();
        REPORTER_ASSERT(r, mpParamsSerialized);
        auto mpParamsRoundTripped = SkJpegMultiPictureParameters::Make(mpParamsSerialized);
        REPORTER_ASSERT(r, mpParamsRoundTripped);
        REPORTER_ASSERT(r, mpParamsRoundTripped->images.size() == mpParams->images.size());
        for (size_t i = 0; i < mpParamsRoundTripped->images.size(); ++i) {
            REPORTER_ASSERT(r, mpParamsRoundTripped->images[i].size == mpParams->images[i].size);
            REPORTER_ASSERT(
                    r,
                    mpParamsRoundTripped->images[i].dataOffset == mpParams->images[i].dataOffset);
        }
    }

    const struct Rec {
        const TestStream::Type streamType;
        const bool skipFirstImage;
        const size_t bufferSize;
    } recs[] = {
            {TestStream::Type::kMemoryMapped, false, 1024},
            {TestStream::Type::kMemoryMapped, true, 1024},
            {TestStream::Type::kSeekable, false, 1024},
            {TestStream::Type::kSeekable, true, 1024},
            {TestStream::Type::kSeekable, false, 7},
            {TestStream::Type::kSeekable, true, 13},
            {TestStream::Type::kSeekable, true, 1024 * 1024 * 16},
            {TestStream::Type::kUnseekable, false, 1024},
            {TestStream::Type::kUnseekable, true, 1024},
            {TestStream::Type::kUnseekable, false, 1},
            {TestStream::Type::kUnseekable, true, 1},
            {TestStream::Type::kUnseekable, false, 7},
            {TestStream::Type::kUnseekable, true, 13},
            {TestStream::Type::kUnseekable, false, 1024 * 1024 * 16},
            {TestStream::Type::kUnseekable, true, 1024 * 1024 * 16},
    };
    for (const auto& rec : recs) {
        stream->rewind();
        TestStream testStream(rec.streamType, stream.get());
        auto sourceMgr = SkJpegSourceMgr::Make(&testStream, rec.bufferSize);

        // Decode the images into bitmaps.
        size_t numberOfImages = mpParams->images.size();
        std::vector<SkBitmap> bitmaps(numberOfImages);
        for (size_t i = 0; i < numberOfImages; ++i) {
            if (i == 0) {
                REPORTER_ASSERT(r, mpParams->images[i].dataOffset == 0);
                continue;
            }
            if (i == 1 && rec.skipFirstImage) {
                continue;
            }
            auto imageData = sourceMgr->getSubsetData(
                    SkJpegMultiPictureParameters::GetAbsoluteOffset(mpParams->images[i].dataOffset,
                                                                    mpParamsSegment.offset),
                    mpParams->images[i].size);
            REPORTER_ASSERT(r, imageData);

            std::unique_ptr<SkCodec> codec =
                    SkCodec::MakeFromStream(SkMemoryStream::Make(imageData));
            REPORTER_ASSERT(r, codec);

            SkBitmap bm;
            bm.allocPixels(codec->getInfo());
            REPORTER_ASSERT(r,
                            SkCodec::kSuccess ==
                                    codec->getPixels(bm.info(), bm.getPixels(), bm.rowBytes()));
            bitmaps[i] = bm;
        }

        // Spot-check the image size and pixels.
        if (!rec.skipFirstImage) {
            REPORTER_ASSERT(r, bitmaps[1].dimensions() == SkISize::Make(1512, 2016));
            REPORTER_ASSERT(r, bitmaps[1].getColor(0, 0) == 0xFF3B3B3B);
            REPORTER_ASSERT(r, bitmaps[1].getColor(1511, 2015) == 0xFF101010);
        }
        REPORTER_ASSERT(r, bitmaps[2].dimensions() == SkISize::Make(576, 768));
        REPORTER_ASSERT(r, bitmaps[2].getColor(0, 0) == 0xFF010101);
        REPORTER_ASSERT(r, bitmaps[2].getColor(575, 767) == 0xFFB5B5B5);
    }
}

// Decode an image and its gainmap.
template <typename Reporter>
void decode_all(Reporter& r,
                std::unique_ptr<SkStream> stream,
                SkBitmap& baseBitmap,
                SkBitmap& gainmapBitmap,
                SkGainmapInfo& gainmapInfo) {
    // Decode the base bitmap.
    SkCodec::Result result = SkCodec::kSuccess;
    std::unique_ptr<SkCodec> baseCodec = SkJpegCodec::MakeFromStream(std::move(stream), &result);
    REPORTER_ASSERT(r, baseCodec);
    baseBitmap.allocPixels(baseCodec->getInfo());
    REPORTER_ASSERT(r,
                    SkCodec::kSuccess == baseCodec->getPixels(baseBitmap.info(),
                                                              baseBitmap.getPixels(),
                                                              baseBitmap.rowBytes()));
    std::unique_ptr<SkAndroidCodec> androidCodec =
            SkAndroidCodec::MakeFromCodec(std::move(baseCodec));
    REPORTER_ASSERT(r, androidCodec);

    // Extract the gainmap info and stream.
    std::unique_ptr<SkStream> gainmapStream;
    REPORTER_ASSERT(r, androidCodec->getAndroidGainmap(&gainmapInfo, &gainmapStream));
    REPORTER_ASSERT(r, gainmapStream);

    // Decode the gainmap bitmap.
    std::unique_ptr<SkCodec> gainmapCodec = SkCodec::MakeFromStream(std::move(gainmapStream));
    REPORTER_ASSERT(r, gainmapCodec);
    SkBitmap bm;
    bm.allocPixels(gainmapCodec->getInfo());
    gainmapBitmap.allocPixels(gainmapCodec->getInfo());
    REPORTER_ASSERT(r,
                    SkCodec::kSuccess == gainmapCodec->getPixels(gainmapBitmap.info(),
                                                                 gainmapBitmap.getPixels(),
                                                                 gainmapBitmap.rowBytes()));
}

DEF_TEST(AndroidCodec_jpegGainmapDecode, r) {
    const struct Rec {
        const char* path;
        SkISize dimensions;
        SkColor originColor;
        SkColor farCornerColor;
        SkGainmapInfo info;
    } recs[] = {
            {"images/iphone_13_pro.jpeg",
             SkISize::Make(1512, 2016),
             0xFF3B3B3B,
             0xFF101010,
             {{1.f, 1.f, 1.f, 1.f},
              {3.482202f, 3.482202f, 3.482202f, 1.f},
              {1.f, 1.f, 1.f, 1.f},
              {0.f, 0.f, 0.f, 1.f},
              {0.f, 0.f, 0.f, 1.f},
              1.f,
              3.482202f,
              SkGainmapInfo::BaseImageType::kSDR,
              SkGainmapInfo::Type::kApple,
              nullptr}},
            {"images/iphone_15.jpeg",
             SkISize::Make(2016, 1512),
             0xFF5C5C5C,
             0xFF656565,
             {{1.f, 1.f, 1.f, 1.f},
              {3.755272f, 3.755272f, 3.755272f, 1.f},
              {1.f, 1.f, 1.f, 1.f},
              {0.f, 0.f, 0.f, 1.f},
              {0.f, 0.f, 0.f, 1.f},
              1.f,
              3.755272f,
              SkGainmapInfo::BaseImageType::kSDR,
              SkGainmapInfo::Type::kApple,
              nullptr}},
            {"images/gainmap_gcontainer_only.jpg",
             SkISize::Make(32, 32),
             0xffffffff,
             0xffffffff,
             {{25.f, 0.5f, 1.f, 1.f},
              {2.f, 4.f, 8.f, 1.f},
              {0.5, 1.f, 2.f, 1.f},
              {0.01f, 0.001f, 0.0001f, 1.f},
              {0.0001f, 0.001f, 0.01f, 1.f},
              2.f,
              4.f,
              SkGainmapInfo::BaseImageType::kSDR,
              SkGainmapInfo::Type::kDefault,
              nullptr}},
            {"images/gainmap_iso21496_1_adobe_gcontainer.jpg",
             SkISize::Make(32, 32),
             0xffffffff,
             0xff000000,
             {{25.f, 0.5f, 1.f, 1.f},
              {2.f, 4.f, 8.f, 1.f},
              {0.5, 1.f, 2.f, 1.f},
              {0.01f, 0.001f, 0.0001f, 1.f},
              {0.0001f, 0.001f, 0.01f, 1.f},
              2.f,
              4.f,
              SkGainmapInfo::BaseImageType::kSDR,
              SkGainmapInfo::Type::kDefault,
              nullptr}},
            {"images/gainmap_iso21496_1.jpg",
             SkISize::Make(32, 32),
             0xffffffff,
             0xff000000,
             {{25.f, 0.5f, 1.f, 1.f},
              {2.f, 4.f, 8.f, 1.f},
              {0.5, 1.f, 2.f, 1.f},
              {0.01f, 0.001f, 0.0001f, 1.f},
              {0.0001f, 0.001f, 0.01f, 1.f},
              2.f,
              4.f,
              SkGainmapInfo::BaseImageType::kHDR,
              SkGainmapInfo::Type::kDefault,
              SkColorSpace::MakeRGB(SkNamedTransferFn::kSRGB, SkNamedGamut::kRec2020)}},
    };

    TestStream::Type kStreamTypes[] = {
            TestStream::Type::kUnseekable,
            TestStream::Type::kSeekable,
            TestStream::Type::kMemoryMapped,
    };
    for (const auto& streamType : kStreamTypes) {
        bool useFileStream = streamType != TestStream::Type::kMemoryMapped;
        for (const auto& rec : recs) {
            auto stream = GetResourceAsStream(rec.path, useFileStream);
            REPORTER_ASSERT(r, stream);
            auto testStream = std::make_unique<TestStream>(streamType, stream.get());

            SkBitmap baseBitmap;
            SkBitmap gainmapBitmap;
            SkGainmapInfo gainmapInfo;
            decode_all(r, std::move(testStream), baseBitmap, gainmapBitmap, gainmapInfo);

            // Spot-check the image size and pixels.
            REPORTER_ASSERT(r, gainmapBitmap.dimensions() == rec.dimensions);
            REPORTER_ASSERT(r, gainmapBitmap.getColor(0, 0) == rec.originColor);
            REPORTER_ASSERT(
                    r,
                    gainmapBitmap.getColor(rec.dimensions.fWidth - 1, rec.dimensions.fHeight - 1) ==
                            rec.farCornerColor);

            // Verify the gainmap rendering parameters.
            expect_approx_eq_info(r, rec.info, gainmapInfo);
        }
    }
}

DEF_TEST(AndroidCodec_jpegNoGainmap, r) {
    // This test image has a large APP16 segment that will stress the various SkJpegSourceMgrs'
    // data skipping paths.
    const char* path = "images/icc-v2-gbr.jpg";

    TestStream::Type kStreamTypes[] = {
            TestStream::Type::kUnseekable,
            TestStream::Type::kSeekable,
            TestStream::Type::kMemoryMapped,
    };
    for (const auto& streamType : kStreamTypes) {
        bool useFileStream = streamType != TestStream::Type::kMemoryMapped;
        auto stream = GetResourceAsStream(path, useFileStream);
        REPORTER_ASSERT(r, stream);
        auto testStream = std::make_unique<TestStream>(streamType, stream.get());

        // Decode the base bitmap.
        SkCodec::Result result = SkCodec::kSuccess;
        std::unique_ptr<SkCodec> baseCodec =
                SkJpegCodec::MakeFromStream(std::move(testStream), &result);
        REPORTER_ASSERT(r, baseCodec);
        SkBitmap baseBitmap;
        baseBitmap.allocPixels(baseCodec->getInfo());
        REPORTER_ASSERT(r,
                        SkCodec::kSuccess == baseCodec->getPixels(baseBitmap.info(),
                                                                  baseBitmap.getPixels(),
                                                                  baseBitmap.rowBytes()));

        std::unique_ptr<SkAndroidCodec> androidCodec =
                SkAndroidCodec::MakeFromCodec(std::move(baseCodec));
        REPORTER_ASSERT(r, androidCodec);

        // Try to extract the gainmap info and stream. It should fail.
        SkGainmapInfo gainmapInfo;
        std::unique_ptr<SkStream> gainmapStream;
        REPORTER_ASSERT(r, !androidCodec->getAndroidGainmap(&gainmapInfo, &gainmapStream));
    }
}

#if !defined(SK_ENABLE_NDK_IMAGES)

DEF_TEST(AndroidCodec_gainmapInfoEncode, r) {
    SkDynamicMemoryWStream encodeStream;

    constexpr size_t kNumTests = 4;

    SkBitmap baseBitmap;
    baseBitmap.allocPixels(SkImageInfo::MakeN32Premul(16, 16));

    SkBitmap gainmapBitmaps[kNumTests];
    gainmapBitmaps[0].allocPixels(SkImageInfo::MakeN32Premul(16, 16));
    gainmapBitmaps[1].allocPixels(SkImageInfo::MakeN32Premul(8, 8));
    gainmapBitmaps[2].allocPixels(
            SkImageInfo::Make(4, 4, kAlpha_8_SkColorType, kPremul_SkAlphaType));
    gainmapBitmaps[3].allocPixels(
            SkImageInfo::Make(8, 8, kGray_8_SkColorType, kPremul_SkAlphaType));

    SkGainmapInfo infos[kNumTests] = {
            // Multi-channel, UltraHDR-compatible.
            {{1.f, 2.f, 4.f, 1.f},
             {8.f, 16.f, 32.f, 1.f},
             {64.f, 128.f, 256.f, 1.f},
             {1 / 10.f, 1 / 11.f, 1 / 12.f, 1.f},
             {1 / 13.f, 1 / 14.f, 1 / 15.f, 1.f},
             4.f,
             32.f,
             SkGainmapInfo::BaseImageType::kSDR,
             SkGainmapInfo::Type::kDefault,
             nullptr},
            // Multi-channel, not UltraHDR-compatible.
            {{1.f, 2.f, 4.f, 1.f},
             {8.f, 16.f, 32.f, 1.f},
             {64.f, 128.f, 256.f, 1.f},
             {1 / 10.f, 1 / 11.f, 1 / 12.f, 1.f},
             {1 / 13.f, 1 / 14.f, 1 / 15.f, 1.f},
             4.f,
             32.f,
             SkGainmapInfo::BaseImageType::kSDR,
             SkGainmapInfo::Type::kDefault,
             SkColorSpace::MakeRGB(SkNamedTransferFn::kSRGB, SkNamedGamut::kDisplayP3)},
            // Single-channel, UltraHDR-compatible.
            {{1.f, 1.f, 1.f, 1.f},
             {8.f, 8.f, 8.f, 1.f},
             {64.f, 64.f, 64.f, 1.f},
             {1 / 128.f, 1 / 128.f, 1 / 128.f, 1.f},
             {1 / 256.f, 1 / 256.f, 1 / 256.f, 1.f},
             4.f,
             32.f,
             SkGainmapInfo::BaseImageType::kSDR,
             SkGainmapInfo::Type::kDefault,
             nullptr},
            // Single-channel, not UltraHDR-compatible.
            {{1.f, 1.f, 1.f, 1.f},
             {8.f, 8.f, 8.f, 1.f},
             {64.f, 64.f, 64.f, 1.f},
             {1 / 128.f, 1 / 128.f, 1 / 128.f, 1.f},
             {1 / 256.f, 1 / 256.f, 1 / 256.f, 1.f},
             4.f,
             32.f,
             SkGainmapInfo::BaseImageType::kHDR,
             SkGainmapInfo::Type::kDefault,
             SkColorSpace::MakeRGB(SkNamedTransferFn::kSRGB, SkNamedGamut::kDisplayP3)},
    };

    for (size_t i = 0; i < kNumTests; ++i) {
        // Encode |gainmapInfo|.
        bool encodeResult = SkJpegGainmapEncoder::EncodeHDRGM(&encodeStream,
                                                              baseBitmap.pixmap(),
                                                              SkJpegEncoder::Options(),
                                                              gainmapBitmaps[i].pixmap(),
                                                              SkJpegEncoder::Options(),
                                                              infos[i]);
        REPORTER_ASSERT(r, encodeResult);

        // Decode into |decodedGainmapInfo|.
        SkGainmapInfo decodedGainmapInfo;
        SkBitmap decodedBaseBitmap;
        SkBitmap decodedGainmapBitmap;
        auto decodeStream = std::make_unique<SkMemoryStream>(encodeStream.detachAsData());
        decode_all(r,
                   std::move(decodeStream),
                   decodedBaseBitmap,
                   decodedGainmapBitmap,
                   decodedGainmapInfo);

        // Verify that the decode reproducd the input.
        expect_approx_eq_info(r, infos[i], decodedGainmapInfo);
    }
}

// Render an applied gainmap.
static SkBitmap render_gainmap(const SkImageInfo& renderInfo,
                               float renderHdrRatio,
                               const SkBitmap& baseBitmap,
                               const SkBitmap& gainmapBitmap,
                               const SkGainmapInfo& gainmapInfo,
                               int x,
                               int y) {
    SkRect baseRect = SkRect::MakeXYWH(x, y, renderInfo.width(), renderInfo.height());

    float scaleX = gainmapBitmap.width() / static_cast<float>(baseBitmap.width());
    float scaleY = gainmapBitmap.height() / static_cast<float>(baseBitmap.height());
    SkRect gainmapRect = SkRect::MakeXYWH(baseRect.x() * scaleX,
                                          baseRect.y() * scaleY,
                                          baseRect.width() * scaleX,
                                          baseRect.height() * scaleY);

    SkRect dstRect = SkRect::Make(renderInfo.dimensions());

    sk_sp<SkImage> baseImage = SkImages::RasterFromBitmap(baseBitmap);
    sk_sp<SkImage> gainmapImage = SkImages::RasterFromBitmap(gainmapBitmap);
    sk_sp<SkShader> shader = SkGainmapShader::Make(baseImage,
                                                   baseRect,
                                                   SkSamplingOptions(),
                                                   gainmapImage,
                                                   gainmapRect,
                                                   SkSamplingOptions(),
                                                   gainmapInfo,
                                                   dstRect,
                                                   renderHdrRatio,
                                                   renderInfo.refColorSpace());

    SkBitmap result;
    result.allocPixels(renderInfo);
    result.eraseColor(SK_ColorTRANSPARENT);
    SkCanvas canvas(result);

    SkPaint paint;
    paint.setShader(shader);
    canvas.drawRect(dstRect, paint);

    return result;
}

// Render a single pixel of an applied gainmap and return it.
static SkColor4f render_gainmap_pixel(float renderHdrRatio,
                                      const SkBitmap& baseBitmap,
                                      const SkBitmap& gainmapBitmap,
                                      const SkGainmapInfo& gainmapInfo,
                                      int x,
                                      int y) {
    SkImageInfo testPixelInfo = SkImageInfo::Make(
            /*width=*/1,
            /*height=*/1,
            kRGBA_F16_SkColorType,
            kPremul_SkAlphaType,
            SkColorSpace::MakeSRGB());
    SkBitmap testPixelBitmap = render_gainmap(
            testPixelInfo, renderHdrRatio, baseBitmap, gainmapBitmap, gainmapInfo, x, y);
    return testPixelBitmap.getColor4f(0, 0);
}

DEF_TEST(AndroidCodec_jpegGainmapTranscode, r) {
    const char* path = "images/iphone_13_pro.jpeg";
    SkBitmap baseBitmap[2];
    SkBitmap gainmapBitmap[2];
    SkGainmapInfo gainmapInfo[2];

    // Decode an MPF-based gainmap image.
    decode_all(r, GetResourceAsStream(path), baseBitmap[0], gainmapBitmap[0], gainmapInfo[0]);

    // This test was written before SkGainmapShader added support for kApple type. Strip the
    // type out.
    gainmapInfo[0].fType = SkGainmapInfo::Type::kDefault;

    constexpr float kEpsilon = 1e-2f;
    {
        SkDynamicMemoryWStream encodeStream;

        // Transcode to UltraHDR.
        bool encodeResult = SkJpegGainmapEncoder::EncodeHDRGM(&encodeStream,
                                                              baseBitmap[0].pixmap(),
                                                              SkJpegEncoder::Options(),
                                                              gainmapBitmap[0].pixmap(),
                                                              SkJpegEncoder::Options(),
                                                              gainmapInfo[0]);
        REPORTER_ASSERT(r, encodeResult);
        auto encodeData = encodeStream.detachAsData();

        // Decode the just-encoded image.
        auto decodeStream = std::make_unique<SkMemoryStream>(encodeData);
        decode_all(r, std::move(decodeStream), baseBitmap[1], gainmapBitmap[1], gainmapInfo[1]);

        // HDRGM will have the same rendering parameters.
        expect_approx_eq_info(r, gainmapInfo[0], gainmapInfo[1]);

        // Render a few pixels and verify that they come out the same. Rendering requires SkSL.
        const struct Rec {
            int x;
            int y;
            float hdrRatio;
            SkColor4f expectedColor;
            SkColorType forcedColorType;
        } recs[] = {
                {1446, 1603, 1.05f, {0.984375f, 1.004883f, 1.008789f, 1.f}, kUnknown_SkColorType},
                {1446, 1603, 100.f, {1.147461f, 1.170898f, 1.174805f, 1.f}, kUnknown_SkColorType},
                {1446, 1603, 100.f, {1.147461f, 1.170898f, 1.174805f, 1.f}, kGray_8_SkColorType},
                {1446, 1603, 100.f, {1.147461f, 1.170898f, 1.174805f, 1.f}, kAlpha_8_SkColorType},
                {1446, 1603, 100.f, {1.147461f, 1.170898f, 1.174805f, 1.f}, kR8_unorm_SkColorType},
        };

        for (const auto& rec : recs) {
            SkBitmap gainmapBitmap0;
            SkASSERT(gainmapBitmap[0].colorType() == kGray_8_SkColorType);

            // Force various different single-channel formats, to ensure that they all work. Note
            // that when the color type is forced to kAlpha_8_SkColorType, the shader will always
            // read (0,0,0,1) if the alpha type is kOpaque_SkAlphaType.
            if (rec.forcedColorType == kUnknown_SkColorType) {
                gainmapBitmap0 = gainmapBitmap[0];
            } else {
                gainmapBitmap0.installPixels(gainmapBitmap[0]
                                                     .info()
                                                     .makeColorType(rec.forcedColorType)
                                                     .makeAlphaType(kPremul_SkAlphaType),
                                             gainmapBitmap[0].getPixels(),
                                             gainmapBitmap[0].rowBytes());
            }
            SkColor4f p0 = render_gainmap_pixel(
                    rec.hdrRatio, baseBitmap[0], gainmapBitmap0, gainmapInfo[0], rec.x, rec.y);
            SkColor4f p1 = render_gainmap_pixel(
                    rec.hdrRatio, baseBitmap[1], gainmapBitmap[1], gainmapInfo[1], rec.x, rec.y);

            REPORTER_ASSERT(r, approx_eq(p0, p1, kEpsilon));
        }
    }
}

DEF_TEST(AndroidCodec_gainmapInfoParse, r) {
    const uint8_t versionData[] = {
            0x00,  // Minimum version
            0x00,
            0x00,  // Writer version
            0x00,
    };
    const uint8_t data[] = {
            0x00, 0x00,                                      // Minimum version
            0x00, 0x00,                                      // Writer version
            0xc0,                                            // Flags
            0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01,  // Base HDR headroom
            0x00, 0x01, 0x45, 0x3e, 0x00, 0x00, 0x80, 0x00,  // Altr HDR headroom
            0xfc, 0x23, 0x05, 0x14, 0x40, 0x00, 0x00, 0x00,  // Red: Gainmap min
            0x00, 0x01, 0x1f, 0xe1, 0x00, 0x00, 0x80, 0x00,  // Red: Gainmap max
            0x10, 0x4b, 0x9f, 0x0a, 0x40, 0x00, 0x00, 0x00,  // Red: Gamma
            0x01, 0x00, 0x00, 0x00, 0x40, 0x00, 0x00, 0x00,  // Red: Base offset
            0x01, 0x00, 0x00, 0x00, 0x40, 0x00, 0x00, 0x00,  // Red: Altr offset
            0xfd, 0xdb, 0x68, 0x04, 0x40, 0x00, 0x00, 0x00,  // Green: Gainmap min
            0x00, 0x01, 0x11, 0x68, 0x00, 0x00, 0x80, 0x00,  // Green: Gainmap max
            0x10, 0x28, 0xf9, 0x53, 0x40, 0x00, 0x00, 0x00,  // Green: Gamma
            0x01, 0x00, 0x00, 0x00, 0x40, 0x00, 0x00, 0x00,  // Green: Base offset
            0x01, 0x00, 0x00, 0x00, 0x40, 0x00, 0x00, 0x00,  // Green: Altr offset
            0xf7, 0x16, 0x7b, 0x90, 0x40, 0x00, 0x00, 0x00,  // Blue: Gainmap min
            0x00, 0x01, 0x0f, 0x9a, 0x00, 0x00, 0x80, 0x00,  // Blue: Gainmap max
            0x12, 0x95, 0xa8, 0x3f, 0x40, 0x00, 0x00, 0x00,  // Blue: Gamma
            0x01, 0x00, 0x00, 0x00, 0x40, 0x00, 0x00, 0x00,  // Blue: Base offset
            0x01, 0x00, 0x00, 0x00, 0x40, 0x00, 0x00, 0x00,  // Blue: Altr offset
    };
    SkGainmapInfo kExpectedInfo = {{0.959023f, 0.977058f, 0.907989f, 1.f},
                                   {4.753710f, 4.395375f, 4.352630f, 1.f},
                                   {3.927490f, 3.960382f, 3.443712f, 1.f},
                                   {0.015625f, 0.015625f, 0.015625f, 1.f},
                                   {0.015625f, 0.015625f, 0.015625f, 1.f},
                                   1.000000f,
                                   5.819739f,
                                   SkGainmapInfo::BaseImageType::kSDR,
                                   SkGainmapInfo::Type::kDefault,
                                   nullptr};
    SkGainmapInfo kSingleChannelInfo = {{0.1234567e-4f, 0.1234567e-4f, 0.1234567e-4f, 1.f},
                                        {-0.1234567e-4f, -0.1234567e-4f, -0.1234567e-4f, 1.f},
                                        {0.1234567e+0f, 0.1234567e+0f, 0.1234567e+0f, 1.f},
                                        {0.1234567e+4f, 0.1234567e+4f, 0.1234567e+4f, 1.f},
                                        {0.1234567e+4f, 0.1234567e+4f, 0.1234567e+4f, 1.f},
                                        1.,
                                        4.f,
                                        SkGainmapInfo::BaseImageType::kHDR,
                                        SkGainmapInfo::Type::kDefault,
                                        SkColorSpace::MakeSRGB()};

    // Verify the version from data.
    REPORTER_ASSERT(r,
                    SkGainmapInfo::ParseVersion(
                            SkData::MakeWithoutCopy(versionData, sizeof(versionData)).get()));

    // Verify the SkGainmapInfo from data.
    SkGainmapInfo info;
    REPORTER_ASSERT(r,
                    SkGainmapInfo::Parse(SkData::MakeWithoutCopy(data, sizeof(data)).get(), info));
    expect_approx_eq_info(r, info, kExpectedInfo);

    // Verify the parsed version.
    REPORTER_ASSERT(r, SkGainmapInfo::ParseVersion(SkGainmapInfo::SerializeVersion().get()));

    // Verify the round-trip SkGainmapInfo.
    auto dataInfo = info.serialize();
    SkGainmapInfo infoRoundTrip;
    REPORTER_ASSERT(r, SkGainmapInfo::Parse(dataInfo.get(), infoRoundTrip));
    expect_approx_eq_info(r, info, infoRoundTrip);

    // Serialize a single-channel SkGainmapInfo. The serialized data should be smaller.
    auto dataSingleChannelInfo = kSingleChannelInfo.serialize();
    REPORTER_ASSERT(r, dataSingleChannelInfo->size() < dataInfo->size());
    SkGainmapInfo singleChannelInfoRoundTrip;
    REPORTER_ASSERT(r,
                    SkGainmapInfo::Parse(dataSingleChannelInfo.get(), singleChannelInfoRoundTrip));
    expect_approx_eq_info(r, singleChannelInfoRoundTrip, kSingleChannelInfo);
}

#endif  // !defined(SK_ENABLE_NDK_IMAGES)
