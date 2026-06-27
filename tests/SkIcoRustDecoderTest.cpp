/*
 * Copyright 2025 Google LLC.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "experimental/rust_ico/decoder/SkIcoRustDecoder.h"

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
#include "tests/FakeStreams.h"
#include "tests/Test.h"
#include "tools/Resources.h"

#define REPORTER_ASSERT_SUCCESSFUL_CODEC_RESULT(r, actualResult) \
    REPORTER_ASSERT(r, actualResult == SkCodec::kSuccess, \
                    "actualResult=\"%s\" != kSuccess", \
                    SkCodec::ResultToString(actualResult))

// Helper wrapping a call to `SkIcoRustDecoder::Decode`.
static std::unique_ptr<SkCodec> decode_ico(skiatest::Reporter* r, const char* path) {
    skiatest::ReporterContext ctx(r, path);
    sk_sp<SkData> data = GetResourceAsData(path);
    if (!data) {
        ERRORF(r, "Missing resource: %s", path);
        return nullptr;
    }

    SkCodec::Result result;
    std::unique_ptr<SkCodec> codec =
            SkIcoRustDecoder::Decode(SkMemoryStream::Make(std::move(data)), &result);
    REPORTER_ASSERT_SUCCESSFUL_CODEC_RESULT(r, result);

    return codec;
}

// Test decoding a valid ICO file with multiple BMP images.
DEF_TEST(RustIcoCodec_decode_multi_bmp, r) {
    std::unique_ptr<SkCodec> codec = decode_ico(r, "images/color_wheel.ico");
    if (!codec) {
        return;
    }

    // color_wheel.ico has 5 images, largest should be reported
    SkISize dimensions = codec->dimensions();
    REPORTER_ASSERT(r, dimensions.width() > 0, "width=%d", dimensions.width());
    REPORTER_ASSERT(r, dimensions.height() > 0, "height=%d", dimensions.height());

    auto [image, result] = codec->getImage();
    REPORTER_ASSERT_SUCCESSFUL_CODEC_RESULT(r, result);
    REPORTER_ASSERT(r, image);
}

// Test decoding a valid ICO file with PNG images (google_chrome.ico has PNG).
DEF_TEST(RustIcoCodec_decode_with_png, r) {
    std::unique_ptr<SkCodec> codec = decode_ico(r, "images/google_chrome.ico");
    if (!codec) {
        return;
    }

    // google_chrome.ico has 9 images including 256x256 PNG
    SkISize dimensions = codec->dimensions();
    REPORTER_ASSERT(r, dimensions.width() == 256,
                    "Expected 256, got width=%d", dimensions.width());
    REPORTER_ASSERT(r, dimensions.height() == 256,
                    "Expected 256, got height=%d", dimensions.height());

    auto [image, result] = codec->getImage();
    REPORTER_ASSERT_SUCCESSFUL_CODEC_RESULT(r, result);
    REPORTER_ASSERT(r, image);
    REPORTER_ASSERT(r, image->width() == 256);
    REPORTER_ASSERT(r, image->height() == 256);
}

// Test that Decode handles nullptr for the Result parameter.
DEF_TEST(RustIcoCodec_nullptr_result, r) {
    sk_sp<SkData> data = GetResourceAsData("images/color_wheel.ico");
    REPORTER_ASSERT(r, data);

    // This should not crash even when result is nullptr
    std::unique_ptr<SkCodec> codec =
            SkIcoRustDecoder::Decode(SkMemoryStream::Make(std::move(data)), nullptr);
    REPORTER_ASSERT(r, codec);
}

// Test that SkIcoRustDecoder correctly rejects non-ICO data.
DEF_TEST(RustIcoCodec_reject_non_ico, r) {
    sk_sp<SkData> data = GetResourceAsData("images/color_wheel.png");
    if (!data) {
        ERRORF(r, "Missing resource: images/color_wheel.png");
        return;
    }

    SkCodec::Result result;
    std::unique_ptr<SkCodec> codec =
            SkIcoRustDecoder::Decode(SkMemoryStream::Make(std::move(data)), &result);

    // Should fail to decode PNG data as ICO
    REPORTER_ASSERT(r, !codec, "SkIcoRustDecoder should reject PNG data");
}

// Test codec reuse functionality.
DEF_TEST(RustIcoCodec_rewind, r) {
    std::unique_ptr<SkCodec> codec = decode_ico(r, "images/color_wheel.ico");
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

// Test that IsIco correctly identifies ICO data.
DEF_TEST(RustIcoCodec_IsIco_positive, r) {
    sk_sp<SkData> data = GetResourceAsData("images/color_wheel.ico");
    if (!data) {
        ERRORF(r, "Missing resource: images/color_wheel.ico");
        return;
    }

    bool isIco = SkIcoRustDecoder::IsIco(data->data(), data->size());
    REPORTER_ASSERT(r, isIco, "IsIco should return true for ICO data");
}

// Test that IsIco correctly rejects non-ICO data.
DEF_TEST(RustIcoCodec_IsIco_negative, r) {
    sk_sp<SkData> data = GetResourceAsData("images/color_wheel.png");
    if (!data) {
        ERRORF(r, "Missing resource: images/color_wheel.png");
        return;
    }

    bool isIco = SkIcoRustDecoder::IsIco(data->data(), data->size());
    REPORTER_ASSERT(r, !isIco, "IsIco should return false for PNG data");
}

// Test IsIco with insufficient data.
DEF_TEST(RustIcoCodec_IsIco_insufficient_data, r) {
    // ICO signature needs at least 4 bytes (00 00 01 00 or 00 00 02 00)
    const uint8_t shortData[] = {0x00, 0x00, 0x01};
    bool isIco = SkIcoRustDecoder::IsIco(shortData, sizeof(shortData));
    REPORTER_ASSERT(r, !isIco, "IsIco should return false for insufficient data");
}

// Test IsIco recognizes CUR files (cursor format, similar to ICO).
DEF_TEST(RustIcoCodec_IsIco_cursor, r) {
    // CUR signature: 00 00 02 00
    const uint8_t curData[] = {0x00, 0x00, 0x02, 0x00, 0x01, 0x00};
    bool isIco = SkIcoRustDecoder::IsIco(curData, sizeof(curData));
    REPORTER_ASSERT(r, isIco, "IsIco should return true for CUR data");
}

// Table-based test for handling invalid/corrupted ICO files.
DEF_TEST(RustIcoCodec_invalid_ico_handling, r) {
    auto test = [&r](const char* description, const char* file) {
        skiatest::ReporterContext ctx(r, description);
        sk_sp<SkData> data = GetResourceAsData(file);
        if (!data) {
            ERRORF(r, "Missing resource: %s", file);
            return;
        }

        SkCodec::Result result;
        std::unique_ptr<SkCodec> codec =
                SkIcoRustDecoder::Decode(SkMemoryStream::Make(std::move(data)), &result);

        // If we got a codec, try to decode to ensure we don't crash
        if (codec) {
            auto [image, decodeResult] = codec->getImage();
            // Any result is acceptable as long as we don't crash
            (void)image;
            (void)decodeResult;
        }
    };

    test("zero embedded", "empty_images/zero-embedded.ico");
    test("fuzz0", "invalid_images/ico_fuzz0.ico");
    test("fuzz1", "invalid_images/ico_fuzz1.ico");
    test("leak01", "invalid_images/ico_leak01.ico");
    test("int_overflow", "invalid_images/int_overflow.ico");
    test("mask-bmp-ico", "invalid_images/mask-bmp-ico.ico");
    test("sigabort_favicon", "invalid_images/sigabort_favicon.ico");
    test("sigsegv_favicon", "invalid_images/sigsegv_favicon.ico");
    test("sigsegv_favicon_2", "invalid_images/sigsegv_favicon_2.ico");
    test("b37623797", "invalid_images/b37623797.ico");
    test("b38116746", "invalid_images/b38116746.ico");
}

// Test getPixels with a pre-allocated bitmap.
DEF_TEST(RustIcoCodec_getPixels, r) {
    std::unique_ptr<SkCodec> codec = decode_ico(r, "images/color_wheel.ico");
    if (!codec) {
        return;
    }

    SkImageInfo info = codec->getInfo().makeColorType(kN32_SkColorType);
    SkBitmap bitmap;
    bitmap.allocPixels(info);

    SkCodec::Result result = codec->getPixels(bitmap.pixmap());
    REPORTER_ASSERT_SUCCESSFUL_CODEC_RESULT(r, result);

    // Verify bitmap dimensions match codec dimensions
    REPORTER_ASSERT(r, bitmap.width() == codec->dimensions().width());
    REPORTER_ASSERT(r, bitmap.height() == codec->dimensions().height());
}

// Test explicit rewind through multiple getPixels calls.
DEF_TEST(RustIcoCodec_explicit_rewind, r) {
    std::unique_ptr<SkCodec> codec = decode_ico(r, "images/color_wheel.ico");
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

// Test decoding ICO files with non-standard (non-square) dimensions.
// These files have non-square aspect ratios, which is unusual for ICO files.
DEF_TEST(RustIcoCodec_nonstandard_dimensions, r) {
    struct TestCase {
        const char* file;
        int expectedWidth;
        int expectedHeight;
    };
    const TestCase testCases[] = {
        {"images/ico_nonsquare_48x32.ico", 48, 32},
        {"images/ico_nonsquare_64x48.ico", 64, 48},
        {"images/ico_nonsquare_128x96.ico", 128, 96},
        {"images/ico_nonsquare_200x150.ico", 200, 150},
    };

    for (const auto& testCase : testCases) {
        skiatest::ReporterContext ctx(r, testCase.file);
        std::unique_ptr<SkCodec> codec = decode_ico(r, testCase.file);
        if (!codec) {
            continue;
        }

        SkISize dimensions = codec->dimensions();
        REPORTER_ASSERT(r, dimensions.width() == testCase.expectedWidth,
                        "Expected width=%d, got width=%d",
                        testCase.expectedWidth, dimensions.width());
        REPORTER_ASSERT(r, dimensions.height() == testCase.expectedHeight,
                        "Expected height=%d, got height=%d",
                        testCase.expectedHeight, dimensions.height());

        auto [image, result] = codec->getImage();
        REPORTER_ASSERT_SUCCESSFUL_CODEC_RESULT(r, result);
        REPORTER_ASSERT(r, image);
        REPORTER_ASSERT(r, image->width() == testCase.expectedWidth);
        REPORTER_ASSERT(r, image->height() == testCase.expectedHeight);
    }
}

// Test that ICO files with oversized dimensions (>256, requiring PNG payload) are rejected.
// The Rust ICO decoder currently only supports BMP payloads with dimensions <= 256.
DEF_TEST(RustIcoCodec_reject_oversized, r) {
    sk_sp<SkData> data = GetResourceAsData("images/ico_oversized_512x384.ico");
    if (!data) {
        ERRORF(r, "Missing resource: images/ico_oversized_512x384.ico");
        return;
    }

    SkCodec::Result result;
    std::unique_ptr<SkCodec> codec =
            SkIcoRustDecoder::Decode(SkMemoryStream::Make(std::move(data)), &result);

    // Should fail to decode ICO with PNG payload (oversized dimensions)
    REPORTER_ASSERT(r, !codec,
                    "SkIcoRustDecoder should reject ICO files with oversized dimensions");
}

// Test incremental decoding with partial data using HaltingStream.
// This simulates network streaming where data arrives incrementally.
// Note: ICO codec reads the entire file upfront to parse the directory,
// so it cannot truly resume mid-stream. However, this test verifies that
// the codec correctly handles partial data and succeeds when full data is available.
DEF_TEST(RustIcoCodec_IncrementalDecode_PartialStreaming, r) {
    const char* path = "images/color_wheel.ico";
    sk_sp<SkData> data = GetResourceAsData(path);
    if (!data) {
        ERRORF(r, "Missing resource: %s", path);
        return;
    }
    const size_t fullLength = data->size();

    // Start with only 1/4 of the data available
    const size_t initialBytes = fullLength / 4;
    auto streamForCodec = std::make_unique<HaltingStream>(data, initialBytes);
    HaltingStream* retainedStream = streamForCodec.get();

    // ICO codec needs the full file to parse the directory and embedded images,
    // so with partial data we expect the decode to fail initially.
    SkCodec::Result result;
    std::unique_ptr<SkCodec> codec =
            SkIcoRustDecoder::Decode(std::move(streamForCodec), &result);

    // With only partial data, codec creation should fail
    if (codec) {
        // If codec was created with partial data, it means the initial bytes
        // were enough to parse the directory. Try to decode - it should fail
        // or return incomplete.
        SkBitmap bitmap;
        bitmap.allocPixels(codec->getInfo().makeColorType(kN32_SkColorType));
        SkCodec::Result decodeResult = codec->getPixels(bitmap.pixmap());
        // The decode might succeed if the initial bytes contained enough data,
        // or fail with incomplete input
        (void)decodeResult;
    } else {
        // Codec creation failed with partial data - this is expected.
        REPORTER_ASSERT(r, result == SkCodec::kIncompleteInput ||
                           result == SkCodec::kInvalidInput,
                        "Expected kIncompleteInput or kInvalidInput with partial data, got %s",
                        SkCodec::ResultToString(result));

        // Now add the remaining data to the stream
        retainedStream->addNewData(fullLength - initialBytes);
        REPORTER_ASSERT(r, retainedStream->isAllDataReceived(),
                        "Stream should have all data after addNewData");

        // Rewind the stream to start fresh
        REPORTER_ASSERT(r, retainedStream->rewind(), "Stream should be rewindable");

        // Create a new codec with the now-complete stream
        // Note: We need to wrap the retained stream since Decode takes ownership
        // We'll create a new stream with the full data instead
        auto fullStream = SkMemoryStream::Make(data);
        std::unique_ptr<SkCodec> fullCodec =
                SkIcoRustDecoder::Decode(std::move(fullStream), &result);
        REPORTER_ASSERT_SUCCESSFUL_CODEC_RESULT(r, result);
        REPORTER_ASSERT(r, fullCodec, "Codec creation should succeed with full data");

        if (fullCodec) {
            auto [image, decodeResult] = fullCodec->getImage();
            REPORTER_ASSERT_SUCCESSFUL_CODEC_RESULT(r, decodeResult);
            REPORTER_ASSERT(r, image, "Decoding should succeed with full data");
        }
    }
}

// Test that incremental decode API works for ICO codec.
// ICO codec delegates to embedded codecs which may or may not support incremental decode.
DEF_TEST(RustIcoCodec_IncrementalDecode_API, r) {
    std::unique_ptr<SkCodec> codec = decode_ico(r, "images/color_wheel.ico");
    if (!codec) {
        return;
    }

    SkImageInfo info = codec->getInfo().makeColorType(kN32_SkColorType);
    SkBitmap bitmap;
    bitmap.allocPixels(info);

    SkCodec::Options options;
    options.fZeroInitialized = SkCodec::kNo_ZeroInitialized;
    options.fSubset = nullptr;
    options.fFrameIndex = 0;
    options.fPriorFrame = SkCodec::kNoFrame;

    // Try to start incremental decode
    SkCodec::Result result = codec->startIncrementalDecode(
            info, bitmap.getPixels(), bitmap.rowBytes(), &options);

    // ICO codec may return kUnimplemented if the embedded codec doesn't support
    // incremental decode, or kSuccess if it does.
    if (result == SkCodec::kSuccess) {
        // If incremental decode started, complete it
        int rowsDecoded = -1;
        result = codec->incrementalDecode(&rowsDecoded);
        REPORTER_ASSERT(r, result == SkCodec::kSuccess ||
                           result == SkCodec::kIncompleteInput,
                        "incrementalDecode should succeed or return incomplete, got %s",
                        SkCodec::ResultToString(result));
    } else if (result == SkCodec::kUnimplemented) {
        // Incremental decode not supported - this is acceptable for BMP-based ICO
        // Fall back to regular decode
        result = codec->getPixels(bitmap.pixmap());
        REPORTER_ASSERT_SUCCESSFUL_CODEC_RESULT(r, result);
    } else {
        ERRORF(r, "Unexpected result from startIncrementalDecode: %s",
               SkCodec::ResultToString(result));
    }

    // Verify we got valid pixel data
    REPORTER_ASSERT(r, bitmap.width() == codec->dimensions().width());
    REPORTER_ASSERT(r, bitmap.height() == codec->dimensions().height());
}

// Test that an ICO file truncated after the directory (so all entry data is missing)
// fails to create a codec. The directory is intact but none of the embedded images
// are present.
DEF_TEST(RustIcoCodec_truncated_all_entries_missing, r) {
    sk_sp<SkData> data = GetResourceAsData("images/color_wheel.ico");
    if (!data) {
        ERRORF(r, "Missing resource: images/color_wheel.ico");
        return;
    }

    // color_wheel.ico has 5 BMP images. The directory is 6 + 5*16 = 86 bytes.
    // Truncate to just the directory so no embedded image data is present.
    const size_t directorySize = 6 + 5 * 16;
    REPORTER_ASSERT(r, data->size() > directorySize);
    sk_sp<SkData> truncated = SkData::MakeSubset(data.get(), 0, directorySize);

    SkCodec::Result result;
    std::unique_ptr<SkCodec> codec =
            SkIcoRustDecoder::Decode(SkMemoryStream::Make(std::move(truncated)), &result);

    // No embedded images can be decoded, so codec creation should fail.
    REPORTER_ASSERT(r, !codec,
                    "Codec should not be created when all entries are truncated");
    REPORTER_ASSERT(r, result == SkCodec::kInvalidInput,
                    "Expected kInvalidInput, got %s", SkCodec::ResultToString(result));
}

// Test that an ICO file truncated so that only some embedded images are present
// returns kIncompleteInput (not kSuccess), while still producing a usable codec
// for the entries that are available.
DEF_TEST(RustIcoCodec_truncated_some_entries_missing, r) {
    sk_sp<SkData> data = GetResourceAsData("images/color_wheel.ico");
    if (!data) {
        ERRORF(r, "Missing resource: images/color_wheel.ico");
        return;
    }

    // color_wheel.ico has 5 BMP images. Truncate to roughly half the file so
    // some entries are fully present but later ones are cut off.
    const size_t truncatedSize = data->size() / 2;
    sk_sp<SkData> truncated = SkData::MakeSubset(data.get(), 0, truncatedSize);

    SkCodec::Result result;
    std::unique_ptr<SkCodec> codec =
            SkIcoRustDecoder::Decode(SkMemoryStream::Make(std::move(truncated)), &result);

    if (codec) {
        // Some entries decoded successfully but not all — should be kIncompleteInput.
        REPORTER_ASSERT(r, result == SkCodec::kIncompleteInput,
                        "Expected kIncompleteInput for partially truncated ICO, got %s",
                        SkCodec::ResultToString(result));

        // The codec should still be usable for the entries that are present.
        int frameCount = codec->getFrameCount();
        REPORTER_ASSERT(r, frameCount > 0, "Should have at least one decodable frame");
        REPORTER_ASSERT(r, frameCount < 5,
                        "Should have fewer than 5 frames (original has 5), got %d", frameCount);

        // Verify the available frames can be decoded.
        auto [image, decodeResult] = codec->getImage();
        REPORTER_ASSERT(r, image, "Should be able to decode available frames");
    } else {
        // If no entries decoded at all, kInvalidInput is acceptable.
        REPORTER_ASSERT(r, result == SkCodec::kInvalidInput ||
                           result == SkCodec::kIncompleteInput,
                        "Expected kInvalidInput or kIncompleteInput, got %s",
                        SkCodec::ResultToString(result));
    }
}
