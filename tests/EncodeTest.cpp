/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "tests/Test.h"
#include "tools/Resources.h"

#include "include/core/SkBitmap.h"
#include "include/core/SkColorPriv.h"
#include "include/core/SkEncodedImageFormat.h"
#include "include/core/SkImage.h"
#include "include/core/SkStream.h"
#include "include/encode/SkJpegEncoder.h"
#include "include/encode/SkPngEncoder.h"
#include "include/encode/SkWebpEncoder.h"

#include "png.h"

#include <algorithm>
#include <string>
#include <vector>

// FIXME: Update the Google3 build's dependencies so it can run this test.
#ifndef SK_BUILD_FOR_GOOGLE3
#include "webp/decode.h"
#endif

static bool encode(SkEncodedImageFormat format, SkWStream* dst, const SkPixmap& src) {
    switch (format) {
        case SkEncodedImageFormat::kJPEG:
            return SkJpegEncoder::Encode(dst, src, SkJpegEncoder::Options());
        case SkEncodedImageFormat::kPNG:
            return SkPngEncoder::Encode(dst, src, SkPngEncoder::Options());
        default:
            return false;
    }
}

static std::unique_ptr<SkEncoder> make(SkEncodedImageFormat format, SkWStream* dst,
                                       const SkPixmap& src) {
    switch (format) {
        case SkEncodedImageFormat::kJPEG:
            return SkJpegEncoder::Make(dst, src, SkJpegEncoder::Options());
        case SkEncodedImageFormat::kPNG:
            return SkPngEncoder::Make(dst, src, SkPngEncoder::Options());
        default:
            return nullptr;
    }
}

static void test_encode(skiatest::Reporter* r, SkEncodedImageFormat format) {
    SkBitmap bitmap;
    bool success = GetResourceAsBitmap("images/mandrill_128.png", &bitmap);
    if (!success) {
        return;
    }

    SkPixmap src;
    success = bitmap.peekPixels(&src);
    REPORTER_ASSERT(r, success);
    if (!success) {
        return;
    }

    SkDynamicMemoryWStream dst0, dst1, dst2, dst3;
    success = encode(format, &dst0, src);
    REPORTER_ASSERT(r, success);

    auto encoder1 = make(format, &dst1, src);
    for (int i = 0; i < src.height(); i++) {
        success = encoder1->encodeRows(1);
        REPORTER_ASSERT(r, success);
    }

    auto encoder2 = make(format, &dst2, src);
    for (int i = 0; i < src.height(); i+=3) {
        success = encoder2->encodeRows(3);
        REPORTER_ASSERT(r, success);
    }

    auto encoder3 = make(format, &dst3, src);
    success = encoder3->encodeRows(200);
    REPORTER_ASSERT(r, success);

    sk_sp<SkData> data0 = dst0.detachAsData();
    sk_sp<SkData> data1 = dst1.detachAsData();
    sk_sp<SkData> data2 = dst2.detachAsData();
    sk_sp<SkData> data3 = dst3.detachAsData();
    REPORTER_ASSERT(r, data0->equals(data1.get()));
    REPORTER_ASSERT(r, data0->equals(data2.get()));
    REPORTER_ASSERT(r, data0->equals(data3.get()));
}

DEF_TEST(Encode, r) {
    test_encode(r, SkEncodedImageFormat::kJPEG);
    test_encode(r, SkEncodedImageFormat::kPNG);
}

static inline bool almost_equals(SkPMColor a, SkPMColor b, int tolerance) {
    if (SkTAbs((int)SkGetPackedR32(a) - (int)SkGetPackedR32(b)) > tolerance) {
        return false;
    }

    if (SkTAbs((int)SkGetPackedG32(a) - (int)SkGetPackedG32(b)) > tolerance) {
        return false;
    }

    if (SkTAbs((int)SkGetPackedB32(a) - (int)SkGetPackedB32(b)) > tolerance) {
        return false;
    }

    if (SkTAbs((int)SkGetPackedA32(a) - (int)SkGetPackedA32(b)) > tolerance) {
        return false;
    }

    return true;
}

static inline bool almost_equals(const SkBitmap& a, const SkBitmap& b, int tolerance) {
    if (a.info() != b.info()) {
        return false;
    }

    SkASSERT(kN32_SkColorType == a.colorType());
    for (int y = 0; y < a.height(); y++) {
        for (int x = 0; x < a.width(); x++) {
            if (!almost_equals(*a.getAddr32(x, y), *b.getAddr32(x, y), tolerance)) {
                return false;
            }
        }
    }

    return true;
}

DEF_TEST(Encode_JpegDownsample, r) {
    SkBitmap bitmap;
    bool success = GetResourceAsBitmap("images/mandrill_128.png", &bitmap);
    if (!success) {
        return;
    }

    SkPixmap src;
    success = bitmap.peekPixels(&src);
    REPORTER_ASSERT(r, success);
    if (!success) {
        return;
    }

    SkDynamicMemoryWStream dst0, dst1, dst2;
    SkJpegEncoder::Options options;
    success = SkJpegEncoder::Encode(&dst0, src, options);
    REPORTER_ASSERT(r, success);

    options.fDownsample = SkJpegEncoder::Downsample::k422;
    success = SkJpegEncoder::Encode(&dst1, src, options);
    REPORTER_ASSERT(r, success);

    options.fDownsample = SkJpegEncoder::Downsample::k444;
    success = SkJpegEncoder::Encode(&dst2, src, options);
    REPORTER_ASSERT(r, success);

    sk_sp<SkData> data0 = dst0.detachAsData();
    sk_sp<SkData> data1 = dst1.detachAsData();
    sk_sp<SkData> data2 = dst2.detachAsData();
    REPORTER_ASSERT(r, data0->size() < data1->size());
    REPORTER_ASSERT(r, data1->size() < data2->size());

    SkBitmap bm0, bm1, bm2;
    SkImage::MakeFromEncoded(data0)->asLegacyBitmap(&bm0);
    SkImage::MakeFromEncoded(data1)->asLegacyBitmap(&bm1);
    SkImage::MakeFromEncoded(data2)->asLegacyBitmap(&bm2);
    REPORTER_ASSERT(r, almost_equals(bm0, bm1, 60));
    REPORTER_ASSERT(r, almost_equals(bm1, bm2, 60));
}

static inline void pushComment(
        std::vector<std::string>& comments, const char* keyword, const char* text) {
    comments.push_back(keyword);
    comments.push_back(text);
}

static void testPngComments(const SkPixmap& src, SkPngEncoder::Options& options,
        skiatest::Reporter* r) {
    std::vector<std::string> commentStrings;
    pushComment(commentStrings, "key", "text");
    pushComment(commentStrings, "test", "something");
    pushComment(commentStrings, "have some", "spaces in both");

    std::string longKey(PNG_KEYWORD_MAX_LENGTH, 'x');
#ifdef SK_DEBUG
    commentStrings.push_back(longKey);
#else
    // We call SkDEBUGFAILF it the key is too long so we'll only test this in release mode.
    commentStrings.push_back(longKey + "x");
#endif
    commentStrings.push_back("");

    std::vector<const char*> commentPointers;
    std::vector<size_t> commentSizes;
    for(auto& str : commentStrings) {
        commentPointers.push_back(str.c_str());
        commentSizes.push_back(str.length() + 1);
    }

    options.fComments = SkDataTable::MakeCopyArrays((void const *const *)commentPointers.data(),
            commentSizes.data(), commentStrings.size());


    SkDynamicMemoryWStream dst;
    bool success = SkPngEncoder::Encode(&dst, src, options);
    REPORTER_ASSERT(r, success);

    std::vector<char> output(dst.bytesWritten());
    dst.copyTo(output.data());

    // Each chunk is of the form length (4 bytes), chunk type (tEXt), data,
    // checksum (4 bytes).  Make sure we find all of them in the encoded
    // results.
    const char kExpected1[] =
        "\x00\x00\x00\x08tEXtkey\x00text\x9e\xe7\x66\x51";
    const char kExpected2[] =
        "\x00\x00\x00\x0etEXttest\x00something\x29\xba\xef\xac";
    const char kExpected3[] =
        "\x00\x00\x00\x18tEXthave some\x00spaces in both\x8d\x69\x34\x2d";
    std::string longKeyRecord = "tEXt" + longKey; // A snippet of our long key comment
    std::string tooLongRecord = "tExt" + longKey + "x"; // A snippet whose key is too long

    auto search1 = std::search(output.begin(), output.end(),
            kExpected1, kExpected1 + sizeof(kExpected1));
    auto search2 = std::search(output.begin(), output.end(),
            kExpected2, kExpected2 + sizeof(kExpected2));
    auto search3 = std::search(output.begin(), output.end(),
            kExpected3, kExpected3 + sizeof(kExpected3));
    auto search4 = std::search(output.begin(), output.end(),
            longKeyRecord.begin(), longKeyRecord.end());
    auto search5 = std::search(output.begin(), output.end(),
            tooLongRecord.begin(), tooLongRecord.end());

    REPORTER_ASSERT(r, search1 != output.end());
    REPORTER_ASSERT(r, search2 != output.end());
    REPORTER_ASSERT(r, search3 != output.end());
    REPORTER_ASSERT(r, search4 != output.end());
    REPORTER_ASSERT(r, search5 == output.end());
    // Comments test ends
}

DEF_TEST(Encode_PngOptions, r) {
    SkBitmap bitmap;
    bool success = GetResourceAsBitmap("images/mandrill_128.png", &bitmap);
    if (!success) {
        return;
    }

    SkPixmap src;
    success = bitmap.peekPixels(&src);
    REPORTER_ASSERT(r, success);
    if (!success) {
        return;
    }

    SkDynamicMemoryWStream dst0, dst1, dst2;
    SkPngEncoder::Options options;
    success = SkPngEncoder::Encode(&dst0, src, options);
    REPORTER_ASSERT(r, success);

    options.fFilterFlags = SkPngEncoder::FilterFlag::kUp;
    success = SkPngEncoder::Encode(&dst1, src, options);
    REPORTER_ASSERT(r, success);

    options.fZLibLevel = 3;
    success = SkPngEncoder::Encode(&dst2, src, options);
    REPORTER_ASSERT(r, success);

    testPngComments(src, options, r);

    sk_sp<SkData> data0 = dst0.detachAsData();
    sk_sp<SkData> data1 = dst1.detachAsData();
    sk_sp<SkData> data2 = dst2.detachAsData();
    REPORTER_ASSERT(r, data0->size() < data1->size());
    REPORTER_ASSERT(r, data1->size() < data2->size());

    SkBitmap bm0, bm1, bm2;
    SkImage::MakeFromEncoded(data0)->asLegacyBitmap(&bm0);
    SkImage::MakeFromEncoded(data1)->asLegacyBitmap(&bm1);
    SkImage::MakeFromEncoded(data2)->asLegacyBitmap(&bm2);
    REPORTER_ASSERT(r, almost_equals(bm0, bm1, 0));
    REPORTER_ASSERT(r, almost_equals(bm0, bm2, 0));
}

#ifndef SK_BUILD_FOR_GOOGLE3
DEF_TEST(Encode_WebpQuality, r) {
    SkBitmap bm;
    bm.allocN32Pixels(100, 100);
    bm.eraseColor(SK_ColorBLUE);

    auto dataLossy    = SkEncodeBitmap(bm, SkEncodedImageFormat::kWEBP, 99);
    auto dataLossLess = SkEncodeBitmap(bm, SkEncodedImageFormat::kWEBP, 100);

    enum Format {
        kMixed    = 0,
        kLossy    = 1,
        kLossless = 2,
    };

    auto test = [&r](const sk_sp<SkData>& data, Format expected) {
        auto printFormat = [](int f) {
            switch (f) {
                case kMixed:    return "mixed";
                case kLossy:    return "lossy";
                case kLossless: return "lossless";
                default:        return "unknown";
            }
        };

        if (!data) {
            ERRORF(r, "Failed to encode. Expected %s", printFormat(expected));
            return;
        }

        WebPBitstreamFeatures features;
        auto status = WebPGetFeatures(data->bytes(), data->size(), &features);
        if (status != VP8_STATUS_OK) {
            ERRORF(r, "Encode had an error %i. Expected %s", status, printFormat(expected));
            return;
        }

        if (expected != features.format) {
            ERRORF(r, "Expected %s encode, but got format %s", printFormat(expected),
                                                               printFormat(features.format));
        }
    };

    test(dataLossy,    kLossy);
    test(dataLossLess, kLossless);
}
#endif

DEF_TEST(Encode_WebpOptions, r) {
    SkBitmap bitmap;
    bool success = GetResourceAsBitmap("images/google_chrome.ico", &bitmap);
    if (!success) {
        return;
    }

    SkPixmap src;
    success = bitmap.peekPixels(&src);
    REPORTER_ASSERT(r, success);
    if (!success) {
        return;
    }

    SkDynamicMemoryWStream dst0, dst1, dst2, dst3;
    SkWebpEncoder::Options options;
    options.fCompression = SkWebpEncoder::Compression::kLossless;
    options.fQuality = 0.0f;
    success = SkWebpEncoder::Encode(&dst0, src, options);
    REPORTER_ASSERT(r, success);

    options.fQuality = 100.0f;
    success = SkWebpEncoder::Encode(&dst1, src, options);
    REPORTER_ASSERT(r, success);

    options.fCompression = SkWebpEncoder::Compression::kLossy;
    options.fQuality = 100.0f;
    success = SkWebpEncoder::Encode(&dst2, src, options);
    REPORTER_ASSERT(r, success);

    options.fCompression = SkWebpEncoder::Compression::kLossy;
    options.fQuality = 50.0f;
    success = SkWebpEncoder::Encode(&dst3, src, options);
    REPORTER_ASSERT(r, success);

    sk_sp<SkData> data0 = dst0.detachAsData();
    sk_sp<SkData> data1 = dst1.detachAsData();
    sk_sp<SkData> data2 = dst2.detachAsData();
    sk_sp<SkData> data3 = dst3.detachAsData();
    REPORTER_ASSERT(r, data0->size() > data1->size());
    REPORTER_ASSERT(r, data1->size() > data2->size());
    REPORTER_ASSERT(r, data2->size() > data3->size());

    SkBitmap bm0, bm1, bm2, bm3;
    SkImage::MakeFromEncoded(data0)->asLegacyBitmap(&bm0);
    SkImage::MakeFromEncoded(data1)->asLegacyBitmap(&bm1);
    SkImage::MakeFromEncoded(data2)->asLegacyBitmap(&bm2);
    SkImage::MakeFromEncoded(data3)->asLegacyBitmap(&bm3);
    REPORTER_ASSERT(r, almost_equals(bm0, bm1, 0));
    REPORTER_ASSERT(r, almost_equals(bm0, bm2, 90));
    REPORTER_ASSERT(r, almost_equals(bm2, bm3, 50));
}
