/*
 * Copyright 2025 Google LLC.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "experimental/rust_bmp/decoder/SkBmpRustDecoder.h"

#include <memory>
#include <utility>

#include "include/codec/SkCodec.h"
#include "include/core/SkBitmap.h"
#include "include/core/SkColor.h"
#include "include/core/SkColorType.h"
#include "include/core/SkData.h"
#include "include/core/SkImage.h"
#include "include/core/SkImageInfo.h"
#include "include/core/SkPixmap.h"
#include "include/core/SkRefCnt.h"
#include "include/core/SkStream.h"
#include "tests/ComparePixels.h"
#include "tests/Test.h"
#include "tools/Resources.h"

// Helper to verify pixel color at a specific location.
static void assert_pixel_color(skiatest::Reporter* r,
                              const SkPixmap& pixmap,
                              int x,
                              int y,
                              SkColor expectedColor,
                              const char* description) {
    SkASSERT(r);
    SkASSERT(x >= 0);
    SkASSERT(y >= 0);
    SkASSERT(description);

    skiatest::ReporterContext ctx(r, description);
    REPORTER_ASSERT(r, x < pixmap.width(),
                    "x=%d >= width=%d", x, pixmap.width());
    REPORTER_ASSERT(r, y < pixmap.height(),
                    "y=%d >= height=%d", y, pixmap.height());
    REPORTER_ASSERT(r, kN32_SkColorType == pixmap.colorType(),
                    "kN32_SkColorType != pixmap.ColorType()=%d",
                    pixmap.colorType());

    SkColor actualColor = pixmap.getColor(x, y);
    REPORTER_ASSERT(r, actualColor == expectedColor,
                    "actualColor=0x%08X != expectedColor==0x%08X at (%d,%d)",
                    actualColor, expectedColor, x, y);
}

#define REPORTER_ASSERT_SUCCESSFUL_CODEC_RESULT(r, actualResult) \
    REPORTER_ASSERT(r, actualResult == SkCodec::kSuccess, \
                    "actualResult=\"%s\" != kSuccess", \
                    SkCodec::ResultToString(actualResult))

// Helper wrapping a call to `SkBmpRustDecoder::Decode`.
static std::unique_ptr<SkCodec> decode_bmp(skiatest::Reporter* r, const char* path) {
    skiatest::ReporterContext ctx(r, path);
    sk_sp<SkData> data = GetResourceAsData(path);
    if (!data) {
        ERRORF(r, "Missing resource: %s", path);
        return nullptr;
    }

    SkCodec::Result result;
    std::unique_ptr<SkCodec> codec =
            SkBmpRustDecoder::Decode(SkMemoryStream::Make(std::move(data)), &result);
    REPORTER_ASSERT_SUCCESSFUL_CODEC_RESULT(r, result);

    return codec;
}

// Table-based test for decoding valid BMP files.
DEF_TEST(RustBmpCodec_decode_valid_bmp, r) {
    auto test = [&r](const char* description, const char* file, SkISize expectedSize,
                     SkColorType expectedColorType,
                     SkCodec::SkScanlineOrder expectedScanlineOrder) {
        skiatest::ReporterContext ctx(r, description);
        std::unique_ptr<SkCodec> codec = decode_bmp(r, file);
        if (!codec) {
            return;
        }
        REPORTER_ASSERT(r, codec->dimensions() == expectedSize,
                        "dimensions=%dx%d != expected=%dx%d",
                        codec->dimensions().width(), codec->dimensions().height(),
                        expectedSize.width(), expectedSize.height());

        // Verify color type
        SkImageInfo info = codec->getInfo();
        REPORTER_ASSERT(r, info.colorType() == expectedColorType,
                        "colorType=%d != expected=%d",
                        info.colorType(), expectedColorType);

        auto [image, result] = codec->getImage();
        REPORTER_ASSERT_SUCCESSFUL_CODEC_RESULT(r, result);
        REPORTER_ASSERT(r, image);
        REPORTER_ASSERT(r, image->width() == expectedSize.width());
        REPORTER_ASSERT(r, image->height() == expectedSize.height());
    };

    test("basic 24-bit", "images/randPixels.bmp", {8, 8}, kN32_SkColorType,
        SkCodec::kBottomUp_SkScanlineOrder);
    test("rle compression", "images/rle.bmp", {320, 240}, kN32_SkColorType,
        SkCodec::kBottomUp_SkScanlineOrder);
    test("8 bit indexed", "images/bmp-size-32x32-8bpp.bmp", {32, 32}, kN32_SkColorType,
        SkCodec::kBottomUp_SkScanlineOrder);
    test("32 bit top-down", "images/32bpp-topdown-320x240.bmp", {320, 240}, kN32_SkColorType,
        SkCodec::kTopDown_SkScanlineOrder);
}

// Test that Decode handles nullptr for the Result parameter.
DEF_TEST(RustBmpCodec_nullptr_result, r) {
    sk_sp<SkData> data = GetResourceAsData("images/randPixels.bmp");
    REPORTER_ASSERT(r, data);

    // This should not crash even when result is nullptr
    std::unique_ptr<SkCodec> codec =
            SkBmpRustDecoder::Decode(SkMemoryStream::Make(std::move(data)), nullptr);
    REPORTER_ASSERT(r, codec);
    REPORTER_ASSERT(r, codec->dimensions() == SkISize::Make(8, 8));
}

// Test that SkBmpRustDecoder correctly rejects non-BMP data.
DEF_TEST(RustBmpCodec_reject_non_bmp, r) {
    sk_sp<SkData> data = GetResourceAsData("images/color_wheel.png");
    if (!data) {
        ERRORF(r, "Missing resource: images/color_wheel.png");
        return;
    }

    SkCodec::Result result;
    std::unique_ptr<SkCodec> codec =
            SkBmpRustDecoder::Decode(SkMemoryStream::Make(std::move(data)), &result);

    // Should fail to decode PNG data as BMP
    REPORTER_ASSERT(r, !codec, "SkBmpRustDecoder should reject PNG data");
}

// Test codec reuse functionality.
DEF_TEST(RustBmpCodec_rewind, r) {
    std::unique_ptr<SkCodec> codec = decode_bmp(r, "images/randPixels.bmp");
    if (!codec) {
        return;
    }

    // Decode first time
    auto [image1, result1] = codec->getImage();
    REPORTER_ASSERT_SUCCESSFUL_CODEC_RESULT(r, result1);
    REPORTER_ASSERT(r, image1);

    // Decode again using the same codec (tests reuse)
    auto [image2, result2] = codec->getImage();
    REPORTER_ASSERT_SUCCESSFUL_CODEC_RESULT(r, result2);
    REPORTER_ASSERT(r, image2);

    // Both images should have the same dimensions
    REPORTER_ASSERT(r,
                    image1->dimensions() == image2->dimensions(),
                    "Images after rewind should have same dimensions");

    // Verify pixel data is identical
    SkBitmap bm1, bm2;
    REPORTER_ASSERT(r, bm1.tryAllocPixels(image1->imageInfo()));
    REPORTER_ASSERT(r, bm2.tryAllocPixels(image2->imageInfo()));
    REPORTER_ASSERT(r, image1->readPixels(nullptr, bm1.pixmap(), 0, 0));
    REPORTER_ASSERT(r, image2->readPixels(nullptr, bm2.pixmap(), 0, 0));

    // Use zero tolerance for exact pixel match
    const float tols[4] = {0, 0, 0, 0};
    auto error = std::function<ComparePixmapsErrorReporter>(
            [&](int x, int y, const float diffs[4]) {
        ERRORF(r, "Pixels differ at (%d, %d) after rewind. Diffs: (%f, %f, %f, %f)",
               x, y, diffs[0], diffs[1], diffs[2], diffs[3]);
    });
    ComparePixels(bm1.pixmap(), bm2.pixmap(), tols, error);
}

// Test getPixels with a pre-allocated bitmap.
DEF_TEST(RustBmpCodec_getPixels, r) {
    std::unique_ptr<SkCodec> codec = decode_bmp(r, "images/randPixels.bmp");
    if (!codec) {
        return;
    }

    SkImageInfo info = codec->getInfo().makeColorType(kN32_SkColorType);
    SkBitmap bitmap;
    bitmap.allocPixels(info);

    SkCodec::Result result = codec->getPixels(bitmap.pixmap());
    REPORTER_ASSERT_SUCCESSFUL_CODEC_RESULT(r, result);

    // Verify bitmap dimensions
    REPORTER_ASSERT(r, bitmap.width() == 8);
    REPORTER_ASSERT(r, bitmap.height() == 8);

    // Spot check pixel values to verify correct decoding
    SkPixmap pixmap = bitmap.pixmap();
    assert_pixel_color(r, pixmap, 0, 0, SkColorSetARGB(0xFF, 0xBB, 0xA5, 0x70), "pixel (0,0)");
    assert_pixel_color(r, pixmap, 1, 0, SkColorSetARGB(0xFF, 0x39, 0x5F, 0x5D), "pixel (1,0)");
    assert_pixel_color(r, pixmap, 0, 1, SkColorSetARGB(0xFF, 0xBD, 0xA6, 0x0E), "pixel (0,1)");
    assert_pixel_color(r, pixmap, 1, 1, SkColorSetARGB(0xFF, 0xC0, 0x1D, 0xB6), "pixel (1,1)");
}

// Test that IsBmp correctly identifies BMP data.
DEF_TEST(RustBmpCodec_IsBmp_positive, r) {
    sk_sp<SkData> data = GetResourceAsData("images/randPixels.bmp");
    if (!data) {
        ERRORF(r, "Missing resource: images/randPixels.bmp");
        return;
    }

    bool isBmp = SkBmpRustDecoder::IsBmp(data->data(), data->size());
    REPORTER_ASSERT(r, isBmp, "IsBmp should return true for BMP data");
}

// Test that IsBmp correctly rejects non-BMP data.
DEF_TEST(RustBmpCodec_IsBmp_negative, r) {
    sk_sp<SkData> data = GetResourceAsData("images/color_wheel.png");
    if (!data) {
        ERRORF(r, "Missing resource: images/color_wheel.png");
        return;
    }

    bool isBmp = SkBmpRustDecoder::IsBmp(data->data(), data->size());
    REPORTER_ASSERT(r, !isBmp, "IsBmp should return false for PNG data");
}

// Test IsBmp with insufficient data.
DEF_TEST(RustBmpCodec_IsBmp_insufficient_data, r) {
    // BMP signature is "BM" (2 bytes), but header is larger
    const uint8_t shortData[] = {'B'};
    bool isBmp = SkBmpRustDecoder::IsBmp(shortData, sizeof(shortData));
    REPORTER_ASSERT(r, !isBmp, "IsBmp should return false for insufficient data");
}

// Table-based test for handling invalid/corrupted BMP files.
DEF_TEST(RustBmpCodec_invalid_bmp_handling, r) {
    auto test = [&r](const char* description, const char* file) {
        skiatest::ReporterContext ctx(r, description);
        sk_sp<SkData> data = GetResourceAsData(file);
        if (!data) {
            ERRORF(r, "Missing resource: %s", file);
            return;
        }

        SkCodec::Result result;
        std::unique_ptr<SkCodec> codec =
                SkBmpRustDecoder::Decode(SkMemoryStream::Make(std::move(data)), &result);

        // If we got a codec, try to decode to ensure we don't crash
        if (codec) {
            auto [image, decodeResult] = codec->getImage();
            // Any result is acceptable as long as we don't crash
            (void)image;
            (void)decodeResult;
        }
    };

    test("zero width", "empty_images/zero-width.bmp");
    test("zero height", "empty_images/zero-height.bmp");
    test("corrupted incomplete", "invalid_images/b33251605.bmp");
    test("invalid header", "invalid_images/b33651913.bmp");
    test("extreme dimensions", "invalid_images/b34778578.bmp");
    test("os2 fuzz", "invalid_images/osfuzz6288.bmp");
}

// Test explicit rewind through multiple getPixels calls.
DEF_TEST(RustBmpCodec_explicit_rewind, r) {
    std::unique_ptr<SkCodec> codec = decode_bmp(r, "images/randPixels.bmp");
    if (!codec) {
        return;
    }

    // First decode
    SkBitmap bitmap1;
    bitmap1.allocPixels(codec->getInfo());
    SkCodec::Result result1 = codec->getPixels(bitmap1.pixmap());
    REPORTER_ASSERT_SUCCESSFUL_CODEC_RESULT(r, result1);

    // Second decode (implicitly rewinds internally)
    SkBitmap bitmap2;
    bitmap2.allocPixels(codec->getInfo());
    SkCodec::Result result2 = codec->getPixels(bitmap2.pixmap());
    REPORTER_ASSERT_SUCCESSFUL_CODEC_RESULT(r, result2);

    // Verify dimensions match
    REPORTER_ASSERT(r, bitmap1.dimensions() == bitmap2.dimensions(),
                    "Dimensions should match after rewind");

    // Verify pixel data matches
    REPORTER_ASSERT(r, bitmap1.computeByteSize() == bitmap2.computeByteSize(),
                    "Byte sizes should match");
    REPORTER_ASSERT(r,
                    memcmp(bitmap1.getPixels(), bitmap2.getPixels(),
                           bitmap1.computeByteSize()) == 0,
                    "Pixel data should match after rewind");
}

