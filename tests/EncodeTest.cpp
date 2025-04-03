/*
 * Copyright 2017 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/codec/SkCodec.h"
#include "include/codec/SkEncodedImageFormat.h"
#include "include/codec/SkJpegDecoder.h"
#include "include/core/SkAlphaType.h"
#include "include/core/SkBitmap.h"
#include "include/core/SkCanvas.h"
#include "include/core/SkColor.h"
#include "include/core/SkColorType.h"
#include "include/core/SkData.h"
#include "include/core/SkDataTable.h"
#include "include/core/SkImage.h"
#include "include/core/SkImageInfo.h"
#include "include/core/SkPixmap.h"
#include "include/core/SkRefCnt.h"
#include "include/core/SkStream.h"
#include "include/core/SkSurface.h"
#include "include/core/SkTypes.h"
#include "include/encode/SkEncoder.h"
#include "include/encode/SkJpegEncoder.h"
#include "include/encode/SkPngEncoder.h"
#include "include/encode/SkWebpEncoder.h"
#include "include/private/base/SkAssert.h"
#include "include/private/base/SkMalloc.h"
#include "include/private/base/SkTemplates.h"
#include "modules/skcms/src/skcms_public.h"
#include "src/core/SkColorPriv.h"
#include "src/core/SkConvertPixels.h"
#include "src/core/SkImageInfoPriv.h"
#include "tests/Test.h"
#include "tools/DecodeUtils.h"

#if defined(SK_CODEC_DECODES_PNG_WITH_LIBPNG)
#include "include/codec/SkPngDecoder.h"
#endif

#if defined(SK_CODEC_DECODES_PNG_WITH_RUST)
#include "experimental/rust_png/decoder/SkPngRustDecoder.h"
#endif

#if defined(SK_CODEC_ENCODES_PNG_WITH_RUST)
#include "experimental/rust_png/encoder/SkPngRustEncoder.h"
#endif

#include <png.h>
#include <webp/decode.h>

#include <algorithm>
#include <cstddef>
#include <initializer_list>
#include <memory>
#include <string>
#include <vector>

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
    bool success = ToolUtils::GetResourceAsBitmap("images/mandrill_128.png", &bitmap);
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

void test_png_encoding_roundtrip_from_specific_source_format(skiatest::Reporter* r,
                                                             SkColorType colorType,
                                                             SkAlphaType alphaType,
                                                             int tolerance) {
    ///////////////////////////////////////////////////
    // Decode the test image into `originalBitmapRgba8`
    // (RGBA8, as the name implies).
    SkBitmap originalBitmapRgba8;
    {
        const char* resource = (kOpaque_SkAlphaType == alphaType) ? "images/color_wheel.jpg"
                                                                  : "images/color_wheel.png";
        sk_sp<SkData> data = GetResourceAsData(resource);
        if (!data) {
            return;
        }
        std::unique_ptr<SkCodec> codec = SkCodec::MakeFromData(data);
        REPORTER_ASSERT(r, !!codec);
        if (!codec) {
            return;
        }
        SkImageInfo dstInfo = codec->getInfo().makeColorType(kRGBA_8888_SkColorType);
        originalBitmapRgba8.allocPixels(dstInfo);
        SkCodec::Result result = codec->getPixels(
                dstInfo, originalBitmapRgba8.getPixels(), originalBitmapRgba8.rowBytes());
        REPORTER_ASSERT(r,
                        result == SkCodec::kSuccess,
                        "result=%s, color=%d, alpha=%d",
                        SkCodec::ResultToString(result),
                        static_cast<int>(colorType),
                        static_cast<int>(alphaType));
        if (result != SkCodec::kSuccess) {
            return;
        }
    }

    //////////////////////////////////////////////////////////////////
    // Transform `originalBitmapRgba8` into `originalBitmap` (into the
    // `colorType` / `alphaType` that this test cares about).
    SkBitmap originalBitmap;
    {
        SkImageInfo dstInfo =
                originalBitmapRgba8.info().makeColorType(colorType).makeAlphaType(alphaType);
        originalBitmap.allocPixels(dstInfo);

        bool success = SkConvertPixels(dstInfo, originalBitmap.getPixels(), dstInfo.minRowBytes(),
                                       originalBitmapRgba8.info(), originalBitmapRgba8.getPixels(),
                                       originalBitmapRgba8.info().minRowBytes());

        REPORTER_ASSERT(r, success);
        if (!success) {
            return;
        }
    }

    /////////////////////////////////////////////
    // Encode `originalBitmap` into `encodedPng`.
    sk_sp<SkData> encodedPng;
    {
        SkPixmap src;
        bool success = originalBitmap.peekPixels(&src);
        REPORTER_ASSERT(r, success);
        if (!success) {
            return;
        }
        SkDynamicMemoryWStream buf;
#if defined(Sk_CODEC_ENCODES_PNG_WITH_RUST)
        success = SkPngRustEncoder::Encode(&buf, src, SkPngRustEncoder::Options());
#else
        success = SkPngEncoder::Encode(&buf, src, SkPngEncoder::Options());
#endif
        REPORTER_ASSERT(r, success);
        if (!success) {
            return;
        }
        encodedPng = buf.detachAsData();
    }

    /////////////////////////////////////////////////////
    // Decode `encodedPng` into `roundtripBitmap` (RGBA8).
    SkBitmap roundtripBitmap;
    {
        std::unique_ptr<SkCodec> codec = SkCodec::MakeFromData(encodedPng);
        REPORTER_ASSERT(r, !!codec);
        if (!codec) {
            return;
        }
        SkImageInfo dstInfo = codec->getInfo().makeColorType(kRGBA_8888_SkColorType);
        roundtripBitmap.allocPixels(dstInfo);
        SkCodec::Result result =
                codec->getPixels(dstInfo, roundtripBitmap.getPixels(), roundtripBitmap.rowBytes());
        REPORTER_ASSERT(r,
                        result == SkCodec::kSuccess,
                        "result=%s, color=%d, alpha=%d",
                        SkCodec::ResultToString(result),
                        static_cast<int>(colorType),
                        static_cast<int>(alphaType));
        if (result != SkCodec::kSuccess) {
            return;
        }
    }

    //////////////////////////////////////////////////////////////////////////
    // Ensure that `originalBitmap` into `roundtripBitmap` are (almost) equal.
    // (We can't use the `almost_equals` overload which operates on `SkBitmap`s,
    // because our bitmaps may expectedly have differente alpha types.)
    if (originalBitmapRgba8.dimensions() != roundtripBitmap.dimensions()) {
        REPORTER_ASSERT(r, false);
        return;
    }
    for (int y = 0; y < originalBitmap.height(); y++) {
        for (int x = 0; x < originalBitmap.width(); x++) {
            SkColor originalColor = originalBitmap.getColor(x, y);
            SkColor roundtripColor = roundtripBitmap.getColor(x, y);
            SkPMColor originalPremulColor = SkPreMultiplyColor(originalColor);
            SkPMColor roundtripPremulColor = SkPreMultiplyColor(roundtripColor);
            bool almost_same = almost_equals(originalPremulColor, roundtripPremulColor, tolerance);
            REPORTER_ASSERT(r,
                            almost_same,
                            "x=%d, y=%d, original=0x%08x, roundtrip=0x%08x, color=%d, alpha=%d",
                            x,
                            y,
                            originalPremulColor,
                            roundtripPremulColor,
                            static_cast<int>(colorType),
                            static_cast<int>(alphaType));
            if (!almost_same) {
                return;
            }
        }
    }
}

DEF_TEST(Encode_png_roundtrip_for_different_source_formats, r) {
    test_png_encoding_roundtrip_from_specific_source_format(
            r, kN32_SkColorType, kOpaque_SkAlphaType, 0);
    test_png_encoding_roundtrip_from_specific_source_format(
            r, kN32_SkColorType, kUnpremul_SkAlphaType, 0);
    test_png_encoding_roundtrip_from_specific_source_format(
            r, kN32_SkColorType, kPremul_SkAlphaType, 0);
    test_png_encoding_roundtrip_from_specific_source_format(
            r, kRGB_565_SkColorType, kOpaque_SkAlphaType, 1);

    // PNG encoder used to narrow down `kRGBA_F16_SkColorType` from RGBA to RGB
    // (BE16) by skipping the alpha channel via `png_set_filler`.  But this
    // wasn't done quite right for `kRGBA_F32_SkColorType`, which motivated this
    // test.  See the code review comments of http://review.skia.org/922676 for
    // more details.
    test_png_encoding_roundtrip_from_specific_source_format(
            r, kRGBA_F16_SkColorType, kOpaque_SkAlphaType, 0);
    test_png_encoding_roundtrip_from_specific_source_format(
            r, kRGBA_F32_SkColorType, kOpaque_SkAlphaType, 0);
}

DEF_TEST(Encode_JPG, r) {
    auto image = ToolUtils::GetResourceAsImage("images/mandrill_128.png");
    if (!image) {
        return;
    }

    for (auto ct : { kRGBA_8888_SkColorType,
                     kBGRA_8888_SkColorType,
                     kRGB_565_SkColorType,
                     kARGB_4444_SkColorType,
                     kGray_8_SkColorType,
                     kRGBA_F16_SkColorType }) {
        for (auto at : { kPremul_SkAlphaType, kUnpremul_SkAlphaType, kOpaque_SkAlphaType }) {
            auto info = SkImageInfo::Make(image->width(), image->height(), ct, at);
            auto surface = SkSurfaces::Raster(info);
            auto canvas = surface->getCanvas();
            canvas->drawImage(image, 0, 0);

            SkBitmap bm;
            bm.allocPixels(info);
            if (!surface->makeImageSnapshot()->readPixels(nullptr, bm.pixmap(), 0, 0)) {
                ERRORF(r, "failed to readPixels! ct: %i\tat: %i\n", ct, at);
                continue;
            }
            for (auto alphaOption : { SkJpegEncoder::AlphaOption::kIgnore,
                                      SkJpegEncoder::AlphaOption::kBlendOnBlack }) {
                SkJpegEncoder::Options opts;
                opts.fAlphaOption = alphaOption;
                SkNullWStream ignored;
                if (!SkJpegEncoder::Encode(&ignored, bm.pixmap(), opts)) {
                    REPORTER_ASSERT(r, ct == kARGB_4444_SkColorType
                                    && alphaOption == SkJpegEncoder::AlphaOption::kBlendOnBlack);
                }
            }
        }
    }
}

DEF_TEST(Encode_JpegDownsample, r) {
    SkBitmap bitmap;
    bool success = ToolUtils::GetResourceAsBitmap("images/mandrill_128.png", &bitmap);
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
    SkImages::DeferredFromEncodedData(data0)->asLegacyBitmap(&bm0);
    SkImages::DeferredFromEncodedData(data1)->asLegacyBitmap(&bm1);
    SkImages::DeferredFromEncodedData(data2)->asLegacyBitmap(&bm2);
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
    bool success = ToolUtils::GetResourceAsBitmap("images/mandrill_128.png", &bitmap);
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
    SkImages::DeferredFromEncodedData(data0)->asLegacyBitmap(&bm0);
    SkImages::DeferredFromEncodedData(data1)->asLegacyBitmap(&bm1);
    SkImages::DeferredFromEncodedData(data2)->asLegacyBitmap(&bm2);
    REPORTER_ASSERT(r, almost_equals(bm0, bm1, 0));
    REPORTER_ASSERT(r, almost_equals(bm0, bm2, 0));
}

#ifndef SK_BUILD_FOR_GOOGLE3
DEF_TEST(Encode_WebpQuality, r) {
    SkBitmap bm;
    bm.allocN32Pixels(100, 100);
    bm.eraseColor(SK_ColorBLUE);

    SkWebpEncoder::Options opts;
    opts.fCompression = SkWebpEncoder::Compression::kLossless;
    SkDynamicMemoryWStream stream;
    SkASSERT_RELEASE(SkWebpEncoder::Encode(&stream, bm.pixmap(), opts));
    auto dataLossLess = stream.detachAsData();

    opts.fCompression = SkWebpEncoder::Compression::kLossy;
    opts.fQuality = 99;
    stream.reset();
    SkASSERT_RELEASE(SkWebpEncoder::Encode(&stream, bm.pixmap(), opts));
    auto dataLossy = stream.detachAsData();

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
    bool success = ToolUtils::GetResourceAsBitmap("images/google_chrome.ico", &bitmap);
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
    SkImages::DeferredFromEncodedData(data0)->asLegacyBitmap(&bm0);
    SkImages::DeferredFromEncodedData(data1)->asLegacyBitmap(&bm1);
    SkImages::DeferredFromEncodedData(data2)->asLegacyBitmap(&bm2);
    SkImages::DeferredFromEncodedData(data3)->asLegacyBitmap(&bm3);
    REPORTER_ASSERT(r, almost_equals(bm0, bm1, 0));
    REPORTER_ASSERT(r, almost_equals(bm0, bm2, 90));
    REPORTER_ASSERT(r, almost_equals(bm2, bm3, 50));
}

DEF_TEST(Encode_WebpAnimated, r) {
    const int frameCount = 3;
    const int width = 16;
    const int height = 16;
    auto info = SkImageInfo::MakeN32Premul(width, height);
    std::vector<SkBitmap> bitmaps(frameCount);
    std::vector<SkEncoder::Frame> frames(frameCount);
    std::vector<int> durations = {50, 100, 150};
    std::vector<SkColor> colors = {SK_ColorRED, SK_ColorBLUE, SK_ColorGREEN};

    for (int i = 0; i < frameCount; i++) {
        bitmaps[i].allocPixels(info);
        bitmaps[i].eraseColor(colors[i]);
        REPORTER_ASSERT(r, bitmaps[i].peekPixels(&frames[i].pixmap));
        frames[i].duration = durations[i];
    }

    SkDynamicMemoryWStream stream;
    SkWebpEncoder::Options options;
    options.fCompression = SkWebpEncoder::Compression::kLossless;
    options.fQuality = 100;

    REPORTER_ASSERT(r, SkWebpEncoder::EncodeAnimated(&stream, frames, options));

    auto codec = SkCodec::MakeFromData(stream.detachAsData());
    REPORTER_ASSERT(r, !!codec);

    std::vector<SkCodec::FrameInfo> frameInfos = codec->getFrameInfo();
    REPORTER_ASSERT(r, frameInfos.size() == frameCount);

    for (size_t i = 0; i < frameInfos.size(); ++i) {
        SkBitmap bitmap;
        bitmap.allocPixels(info);
        bitmap.eraseColor(0);

        SkCodec::Options codecOptions;
        codecOptions.fFrameIndex = (int)i;

        auto result = codec->getPixels(info, bitmap.getPixels(), bitmap.rowBytes(), &codecOptions);
        if (result != SkCodec::kSuccess) {
            ERRORF(r, "error in frame %zu: %s", i, SkCodec::ResultToString(result));
        }

        REPORTER_ASSERT(r, almost_equals(bitmap, bitmaps[i], 0));
        REPORTER_ASSERT(r, frameInfos[i].fDuration == durations[i]);
    }
}

DEF_TEST(Encode_WebpAnimated_FrameUnmatched, r) {
    // Create two frames with unmatched sizes and verify the encode should fail.
    SkEncoder::Frame frame1;
    SkBitmap bm1;
    bm1.allocPixels(SkImageInfo::MakeN32Premul(8, 8));
    bm1.eraseColor(SK_ColorYELLOW);
    REPORTER_ASSERT(r, bm1.peekPixels(&frame1.pixmap));
    frame1.duration = 200;

    SkEncoder::Frame frame2;
    SkBitmap bm2;
    bm2.allocPixels(SkImageInfo::MakeN32Premul(16, 16));
    bm2.eraseColor(SK_ColorYELLOW);
    REPORTER_ASSERT(r, bm2.peekPixels(&frame2.pixmap));
    frame2.duration = 200;

    SkDynamicMemoryWStream stream;
    SkWebpEncoder::Options options;
    options.fCompression = SkWebpEncoder::Compression::kLossy;
    options.fQuality = 100;
    std::vector<SkEncoder::Frame> frames = {frame1, frame2};
    bool output = SkWebpEncoder::EncodeAnimated(&stream, frames, options);
    REPORTER_ASSERT(r, !output);
}

DEF_TEST(Encode_Alpha, r) {
    // These formats have no sensible way to encode alpha images.
    for (auto format : { SkEncodedImageFormat::kJPEG,
                         SkEncodedImageFormat::kPNG,
                         SkEncodedImageFormat::kWEBP }) {
        for (int ctAsInt = kUnknown_SkColorType + 1; ctAsInt <= kLastEnum_SkColorType; ctAsInt++) {
            auto ct = static_cast<SkColorType>(ctAsInt);
            // Non-alpha-only colortypes are tested elsewhere.
            if (!SkColorTypeIsAlphaOnly(ct)) continue;
            SkBitmap bm;
            bm.allocPixels(SkImageInfo::Make(10, 10, ct, kPremul_SkAlphaType));
            sk_bzero(bm.getPixels(), bm.computeByteSize());
            SkDynamicMemoryWStream stream;
            bool success = false;
            if (format == SkEncodedImageFormat::kJPEG) {
                success = SkJpegEncoder::Encode(&stream, bm.pixmap(), {});
            } else if (format == SkEncodedImageFormat::kPNG) {
                success = SkPngEncoder::Encode(&stream, bm.pixmap(), {});
            } else {
                success = SkWebpEncoder::Encode(&stream, bm.pixmap(), {});
            }

            if ((format == SkEncodedImageFormat::kJPEG || format == SkEncodedImageFormat::kPNG) &&
                ct == kAlpha_8_SkColorType) {
                // We support encoding alpha8 to png and jpeg with our own private meaning.
                REPORTER_ASSERT(r, success);
                REPORTER_ASSERT(r, stream.bytesWritten() > 0);
            } else {
                REPORTER_ASSERT(r, !success);
                REPORTER_ASSERT(r, stream.bytesWritten() == 0);
            }
        }
    }
}

DEF_TEST(Encode_jpeg_blend_to_black, r) {
    SkBitmap originalBitmap;
    const char* resource = "images/rainbow-gradient.png";
    int jpeg_tolerance = 60;

    for (SkColorType colorType : {kRGBA_8888_SkColorType,
                                  kBGRA_8888_SkColorType,
                                  kRGBA_F16_SkColorType}) {
        for (SkAlphaType alphaType : {kUnpremul_SkAlphaType,
                                      kPremul_SkAlphaType}) {
            for (bool blendOnBlack : {true, false}) {
                skiatest::ReporterContext rc(r,
                                             SkStringPrintf(
                                                  "colorType=0x%x alphaType=0x%x blendOnBlack=%d",
                                                  unsigned(colorType),
                                                  unsigned(alphaType),
                                                  blendOnBlack));
                /////////////////////////////////////////////////////////////////
                // Decode the test image into `originalBitmap` into correct alpha
                // and color type.
                auto stream = GetResourceAsStream(resource, false);
                REPORTER_ASSERT(r, stream);
                if (!stream) {
                    return;
                }
                std::unique_ptr<SkCodec> codec;
                SkCodec::Result result = SkCodec::kSuccess;
#if defined(SK_CODEC_DECODES_PNG_WITH_LIBPNG)
                codec = SkPngDecoder::Decode(std::move(stream), &result);
#elif defined(SK_CODEC_DECODES_PNG_WITH_RUST)
                codec = SkPngRustDecoder::Decode(std::move(stream), &result);
#endif
                REPORTER_ASSERT(r, codec);
                if (!codec) {
                  return;
                }
                REPORTER_ASSERT(r,
                  result == SkCodec::kSuccess,
                  "result=%s", SkCodec::ResultToString(result));
                if (result != SkCodec::kSuccess) {
                    continue;
                }
                SkImageInfo dstInfo = codec->getInfo().makeAlphaType(alphaType)
                                                      .makeColorType(colorType);
                originalBitmap.allocPixels(dstInfo);
                result = codec->getPixels(
                    dstInfo, originalBitmap.getPixels(), originalBitmap.rowBytes());
                REPORTER_ASSERT(r,
                            result == SkCodec::kSuccess,
                            "result=%s", SkCodec::ResultToString(result));
                if (result != SkCodec::kSuccess) {
                    continue;
                }

                //////////////////////////////////////////////////////////////////////
                // Blend 'originalBitmap' onto a black background if needed, otherwise
                // disregard alpha and store in 'referenceBM'. This is the bitmap we
                // will be comparing to.
                SkBitmap referenceBM;
                bool success;
                if (blendOnBlack) {
                    SkImageInfo& referenceImageInfo = dstInfo;
                    referenceBM.allocPixels(referenceImageInfo);
                    referenceBM.eraseColor(SK_ColorBLACK);
                    SkCanvas blackCanvas(referenceBM);
                    blackCanvas.drawImage(originalBitmap.asImage(), 0, 0);
                } else {
                    SkImageInfo opaqueInfo = dstInfo.makeAlphaType(kOpaque_SkAlphaType)
                                                    .makeColorType(kRGB_888x_SkColorType);
                    referenceBM.allocPixels(opaqueInfo);
                    success = SkConvertPixels(opaqueInfo,
                                              referenceBM.getAddr(0,0),
                                              opaqueInfo.minRowBytes(),
                                              dstInfo,
                                              originalBitmap.getAddr(0, 0),
                                              dstInfo.minRowBytes());
                    REPORTER_ASSERT(r, success);
                    if (!success) { continue; }
                }

                ///////////////////////////////////////////////////////////////////////////
                // Encode 'originalBitmap' into JPEG. Then decode it into 'roundtripBitmap'.
                SkPixmap src;
                success = originalBitmap.peekPixels(&src);
                REPORTER_ASSERT(r, success);
                if (!success) { continue; }

                SkDynamicMemoryWStream buf;
                SkJpegEncoder::Options options;
                options.fAlphaOption = blendOnBlack ? SkJpegEncoder::AlphaOption::kBlendOnBlack
                                                    : SkJpegEncoder::AlphaOption::kIgnore;
                success = SkJpegEncoder::Encode(&buf, src, options);
                REPORTER_ASSERT(r, success);
                if (!success) { continue; }
                sk_sp<SkData> roundtripData = buf.detachAsData();
                sk_sp<SkImage> image = SkImages::DeferredFromEncodedData(roundtripData);

                SkBitmap roundtripBitmap;
                std::unique_ptr<SkCodec> roundtripCodec = SkJpegDecoder::Decode(roundtripData, &result);
                SkImageInfo roundtripInfo = roundtripCodec->getInfo();
                roundtripBitmap.allocPixels(roundtripInfo);
                roundtripCodec->getPixels(
                    roundtripInfo, roundtripBitmap.getPixels(), roundtripBitmap.rowBytes());

                //////////////////////////////////////////////////////////////////////
                // Ensure that `referenceBM` and `roundtripBitmap` are (almost) equal.
                // We give a certain tolerance due to lossyness.
                if (referenceBM.dimensions() != roundtripBitmap.dimensions()) {
                    REPORTER_ASSERT(r, false);
                    continue;
                }
                bool shouldContinue = false;
                for (int y = 0; y < referenceBM.height(); y++) {
                    if (shouldContinue) { break; }
                    for (int x = 0; x < referenceBM.width(); x++) {
                        SkColor originalColor = referenceBM.getColor(x, y);
                        SkColor roundtripColor = roundtripBitmap.getColor(x, y);
                        SkPMColor originalPremulColor = SkPreMultiplyColor(originalColor);
                        SkPMColor roundtripPremulColor = SkPreMultiplyColor(roundtripColor);
                        bool almost_same = almost_equals(originalPremulColor,
                                                         roundtripPremulColor,
                                                         jpeg_tolerance);
                        REPORTER_ASSERT(r,
                                        almost_same,
                                        "x=%d, y=%d, original=0x%08x, roundtrip=0x%08x, color=%d, alpha=%d",
                                        x,
                                        y,
                                        originalPremulColor,
                                        roundtripPremulColor,
                                        static_cast<int>(colorType),
                                        static_cast<int>(alphaType));
                        if (!almost_same) {
                            shouldContinue = true;
                            break;
                        }
                    }
                }
            }
        }
    }
}
