/*
 * Copyright 2025 Google LLC.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "experimental/rust_ico/decoder/SkIcoRustDecoder.h"

#include <cstdint>
#include <cstring>
#include <iterator>
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
#include "include/core/SkRect.h"
#include "include/core/SkRefCnt.h"
#include "include/core/SkSize.h"
#include "include/core/SkStream.h"
#include "include/core/SkString.h"
#include "include/private/SkTo.h"
#if defined(SK_CODEC_DECODES_ICO)
#include "src/codec/SkIcoCodec.h"
#endif
#include "tests/ComparePixels.h"
#include "tests/FakeStreams.h"
#include "tests/Test.h"
#include "tools/Resources.h"
#include "tools/ToolUtils.h"

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

// ICO entries are alternative representations of one logical image. Verify
// that each native size can be selected by destination dimensions and exposed
// through the indexed frame API used by Blink.
DEF_TEST(RustIcoCodec_alternative_dimensions, r) {
    std::unique_ptr<SkCodec> codec = decode_ico(r, "images/color_wheel.ico");
    if (!codec) {
        return;
    }

    // The top-level (canvas) size is the largest entry.
    REPORTER_ASSERT(r, codec->dimensions() == SkISize::Make(128, 128),
                    "Expected 128x128 canvas, got %dx%d",
                    codec->dimensions().width(), codec->dimensions().height());

    REPORTER_ASSERT(r, codec->getFrameCount() == 5,
                    "Expected five compatibility frames, got %d", codec->getFrameCount());
    for (int i = 0; i < codec->getFrameCount(); ++i) {
        SkCodec::FrameInfo frameInfo;
        REPORTER_ASSERT(r, codec->getFrameInfo(i, &frameInfo));
        REPORTER_ASSERT(r, frameInfo.fRequiredFrame == SkCodec::kNoFrame);
    }

    const int kExpectedSizes[] = {128, 64, 48, 32, 16};
    const float kExpectedScales[] = {1.0f, 0.25f, 0.140625f, 0.0625f, 0.015625f};
    for (int expected : kExpectedSizes) {
        skiatest::ReporterContext ctx(r, SkStringPrintf("size %d", expected).c_str());
        SkImageInfo info = codec->getInfo()
                                   .makeWH(expected, expected)
                                   .makeColorType(kN32_SkColorType);
        SkBitmap bitmap;
        REPORTER_ASSERT(r, bitmap.tryAllocPixels(info));
        bitmap.eraseColor(SK_ColorTRANSPARENT);

        SkCodec::Result result = codec->getPixels(bitmap.pixmap());
        REPORTER_ASSERT_SUCCESSFUL_CODEC_RESULT(r, result);
        REPORTER_ASSERT(r, bitmap.width() == expected && bitmap.height() == expected,
                        "decoded %dx%d, expected %dx%d",
                        bitmap.width(), bitmap.height(), expected, expected);
    }

    for (size_t i = 0; i < sizeof(kExpectedSizes) / sizeof(kExpectedSizes[0]); i++) {
        REPORTER_ASSERT(r,
                        codec->getScaledDimensions(kExpectedScales[i]) ==
                                SkISize::Make(kExpectedSizes[i], kExpectedSizes[i]),
                        "Scale %g did not select %dx%d",
                        kExpectedScales[i], kExpectedSizes[i], kExpectedSizes[i]);
    }

    SkBitmap bitmap;
    REPORTER_ASSERT(r, bitmap.tryAllocPixels(codec->getInfo().makeColorType(kN32_SkColorType)));
    SkCodec::Options options;
    options.fFrameIndex = 1;
    REPORTER_ASSERT_SUCCESSFUL_CODEC_RESULT(r, codec->getPixels(bitmap.pixmap(), &options));
}

// Verify that callers can decode an indexed entry into a larger canvas when
// they explicitly request an alpha-capable destination for the padding.
DEF_TEST(RustIcoCodec_indexed_full_canvas, r) {
    std::unique_ptr<SkCodec> codec = decode_ico(r, "images/color_wheel.ico");
    if (!codec) {
        return;
    }

    const int kExpectedSizes[] = {128, 64, 48, 32, 16};
    REPORTER_ASSERT(r, codec->getFrameCount() == std::size(kExpectedSizes));
    for (int i = 0; i < codec->getFrameCount(); ++i) {
        SkCodec::FrameInfo frameInfo;
        REPORTER_ASSERT(r, codec->getFrameInfo(i, &frameInfo));
        REPORTER_ASSERT(r, frameInfo.fFrameRect ==
                                   SkIRect::MakeWH(kExpectedSizes[i], kExpectedSizes[i]));
    }

    constexpr int kFrameIndex = 1;
    constexpr int kFrameSize = 64;
    const SkImageInfo canvasInfo = codec->getInfo()
                                           .makeColorType(kN32_SkColorType)
                                           .makeAlphaType(kPremul_SkAlphaType);
    const SkImageInfo nativeInfo = canvasInfo.makeWH(kFrameSize, kFrameSize);

    SkBitmap expected;
    REPORTER_ASSERT(r, expected.tryAllocPixels(nativeInfo));
    REPORTER_ASSERT_SUCCESSFUL_CODEC_RESULT(r, codec->getPixels(expected.pixmap()));

    SkCodec::Options options;
    options.fFrameIndex = kFrameIndex;
    SkBitmap canvas;
    REPORTER_ASSERT(r, canvas.tryAllocPixels(canvasInfo));
    canvas.eraseColor(SK_ColorMAGENTA);
    REPORTER_ASSERT_SUCCESSFUL_CODEC_RESULT(r, codec->getPixels(canvas.pixmap(), &options));

    bool matches = true;
    for (int y = 0; y < kFrameSize && matches; ++y) {
        for (int x = 0; x < kFrameSize; ++x) {
            if (canvas.getColor(x, y) != expected.getColor(x, y)) {
                matches = false;
                break;
            }
        }
    }
    REPORTER_ASSERT(r, matches);
    REPORTER_ASSERT(r, SkColorGetA(canvas.getColor(127, 127)) == 0);

    std::unique_ptr<SkCodec> incremental = decode_ico(r, "images/color_wheel.ico");
    if (!incremental) {
        return;
    }
    SkBitmap incrementalCanvas;
    REPORTER_ASSERT(r, incrementalCanvas.tryAllocPixels(canvasInfo));
    REPORTER_ASSERT_SUCCESSFUL_CODEC_RESULT(
            r, incremental->startIncrementalDecode(
                       canvasInfo, incrementalCanvas.getPixels(),
                       incrementalCanvas.rowBytes(), &options));
    REPORTER_ASSERT_SUCCESSFUL_CODEC_RESULT(r, incremental->incrementalDecode());
    REPORTER_ASSERT(r, ToolUtils::equal_pixels(canvas.pixmap(), incrementalCanvas.pixmap()));

    SkBitmap undersized;
    REPORTER_ASSERT(r, undersized.tryAllocPixels(canvasInfo.makeWH(16, 16)));
    REPORTER_ASSERT(r, codec->getPixels(undersized.pixmap(), &options) ==
                               SkCodec::kInvalidScale);
}

#if defined(SK_CODEC_DECODES_ICO)
DEF_TEST(RustIcoCodec_matches_legacy_size_selection, r) {
    sk_sp<SkData> data = GetResourceAsData("images/color_wheel.ico");
    if (!data) {
        ERRORF(r, "Missing resource: images/color_wheel.ico");
        return;
    }

    SkCodec::Result rustCreateResult;
    SkCodec::Result legacyCreateResult;
    std::unique_ptr<SkCodec> rustCodec =
            SkIcoRustDecoder::Decode(SkMemoryStream::Make(data), &rustCreateResult);
    std::unique_ptr<SkCodec> legacyCodec =
            SkIcoCodec::MakeFromStream(SkMemoryStream::Make(data), &legacyCreateResult);
    REPORTER_ASSERT_SUCCESSFUL_CODEC_RESULT(r, rustCreateResult);
    REPORTER_ASSERT_SUCCESSFUL_CODEC_RESULT(r, legacyCreateResult);
    REPORTER_ASSERT(r, rustCodec && legacyCodec);
    if (!rustCodec || !legacyCodec) {
        return;
    }

    REPORTER_ASSERT(r, rustCodec->dimensions() == legacyCodec->dimensions());
    REPORTER_ASSERT(r, rustCodec->getEncodedFormat() == legacyCodec->getEncodedFormat());

    const int kSizes[] = {128, 64, 48, 32, 16, 24};
    for (int size : kSizes) {
        skiatest::ReporterContext ctx(r, SkStringPrintf("size %d", size).c_str());
        const SkImageInfo info = SkImageInfo::MakeN32Premul(size, size);
        SkBitmap rustBitmap;
        SkBitmap legacyBitmap;
        REPORTER_ASSERT(r, rustBitmap.tryAllocPixels(info));
        REPORTER_ASSERT(r, legacyBitmap.tryAllocPixels(info));

        const SkCodec::Result rustResult = rustCodec->getPixels(rustBitmap.pixmap());
        const SkCodec::Result legacyResult = legacyCodec->getPixels(legacyBitmap.pixmap());
        REPORTER_ASSERT(r, rustResult == legacyResult,
                        "Rust returned %s, legacy returned %s",
                        SkCodec::ResultToString(rustResult),
                        SkCodec::ResultToString(legacyResult));
        if (rustResult == SkCodec::kSuccess) {
            REPORTER_ASSERT(r, ToolUtils::equal_pixels(rustBitmap.pixmap(),
                                                       legacyBitmap.pixmap()));
        }
    }
}
#endif

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

DEF_TEST(RustIcoCodec_cursor_hotspots, r) {
    sk_sp<SkData> data = GetResourceAsData("images/color_wheel.ico");
    REPORTER_ASSERT(r, data);
    if (!data) {
        return;
    }

    SkCodec::Result result;
    std::unique_ptr<SkCodec> icoCodec =
            SkIcoRustDecoder::Decode(SkMemoryStream::Make(data), &result);
    REPORTER_ASSERT_SUCCESSFUL_CODEC_RESULT(r, result);
    REPORTER_ASSERT(r, icoCodec);

    SkIPoint hotSpot;
    REPORTER_ASSERT(r, !SkIcoRustDecoder::GetHotSpot(nullptr, 0, &hotSpot));
    REPORTER_ASSERT(r, !SkIcoRustDecoder::GetHotSpot(icoCodec.get(), 0, &hotSpot));

    sk_sp<SkData> cursorData = GetResourceAsData("images/color_wheel.cur");
    REPORTER_ASSERT(r, cursorData);
    if (!cursorData) {
        return;
    }
    const auto* bytes = static_cast<const uint8_t*>(cursorData->data());

    constexpr size_t kDirectoryHeaderSize = 6;
    constexpr size_t kDirectoryEntrySize = 16;
    const size_t entryCount = static_cast<size_t>(bytes[4]) | (static_cast<size_t>(bytes[5]) << 8);
    REPORTER_ASSERT(r, entryCount == 5);
    REPORTER_ASSERT(r,
                    cursorData->size() >= kDirectoryHeaderSize + entryCount * kDirectoryEntrySize);
    if (entryCount != 5 ||
        cursorData->size() < kDirectoryHeaderSize + entryCount * kDirectoryEntrySize) {
        return;
    }

    std::unique_ptr<SkCodec> cursorCodec =
            SkIcoRustDecoder::Decode(SkMemoryStream::Make(cursorData), &result);
    REPORTER_ASSERT_SUCCESSFUL_CODEC_RESULT(r, result);
    REPORTER_ASSERT(r, cursorCodec);
    if (!cursorCodec) {
        return;
    }
    REPORTER_ASSERT(r, cursorCodec->getFrameCount() == static_cast<int>(entryCount));
    for (int frameIndex = 0; frameIndex < cursorCodec->getFrameCount(); ++frameIndex) {
        const int originalEntryIndex = static_cast<int>(entryCount) - frameIndex - 1;
        REPORTER_ASSERT(r, SkIcoRustDecoder::GetHotSpot(cursorCodec.get(), frameIndex, &hotSpot));
        REPORTER_ASSERT(r, hotSpot.x() == originalEntryIndex + 1);
        REPORTER_ASSERT(r, hotSpot.y() == originalEntryIndex + 6);
        SkCodec::FrameInfo frameInfo;
        REPORTER_ASSERT(r, cursorCodec->getFrameInfo(frameIndex, &frameInfo));
        const size_t entryOffset = kDirectoryHeaderSize + originalEntryIndex * kDirectoryEntrySize;
        const int width = bytes[entryOffset] == 0 ? 256 : bytes[entryOffset];
        const int height = bytes[entryOffset + 1] == 0 ? 256 : bytes[entryOffset + 1];
        REPORTER_ASSERT(r, frameInfo.fFrameRect == SkIRect::MakeWH(width, height));
    }

    REPORTER_ASSERT(r, !SkIcoRustDecoder::GetHotSpot(cursorCodec.get(), -1, &hotSpot));
    REPORTER_ASSERT(r,
                    !SkIcoRustDecoder::GetHotSpot(
                            cursorCodec.get(), cursorCodec->getFrameCount(), &hotSpot));
    REPORTER_ASSERT(r, !SkIcoRustDecoder::GetHotSpot(cursorCodec.get(), 0, nullptr));

    const auto readU32 = [](const uint8_t* ptr) {
        return static_cast<uint32_t>(ptr[0]) | (static_cast<uint32_t>(ptr[1]) << 8) |
               (static_cast<uint32_t>(ptr[2]) << 16) | (static_cast<uint32_t>(ptr[3]) << 24);
    };
    const auto writeU32 = [](uint8_t* ptr, uint32_t value) {
        ptr[0] = static_cast<uint8_t>(value);
        ptr[1] = static_cast<uint8_t>(value >> 8);
        ptr[2] = static_cast<uint8_t>(value >> 16);
        ptr[3] = static_cast<uint8_t>(value >> 24);
    };
    const uint8_t* firstEntry = bytes + kDirectoryHeaderSize;
    const size_t firstPayloadEnd = readU32(firstEntry + 12) + readU32(firstEntry + 8);
    REPORTER_ASSERT(r, firstPayloadEnd <= cursorData->size());
    sk_sp<SkData> partialData = SkData::MakeSubset(cursorData.get(), 0, firstPayloadEnd);
    std::unique_ptr<SkCodec> partialCodec =
            SkIcoRustDecoder::Decode(SkMemoryStream::Make(std::move(partialData)), &result);
    REPORTER_ASSERT(r, !partialCodec);
    REPORTER_ASSERT(r, result == SkCodec::kIncompleteInput);
    if (partialCodec || result != SkCodec::kIncompleteInput) {
        return;
    }

    std::unique_ptr<SkCodec> completeAfterPartial =
            SkIcoRustDecoder::Decode(SkMemoryStream::Make(cursorData), &result);
    REPORTER_ASSERT_SUCCESSFUL_CODEC_RESULT(r, result);
    REPORTER_ASSERT(r, completeAfterPartial);
    if (!completeAfterPartial) {
        return;
    }
    REPORTER_ASSERT(r, completeAfterPartial->getFrameCount() == static_cast<int>(entryCount));
    REPORTER_ASSERT(r, SkIcoRustDecoder::GetHotSpot(completeAfterPartial.get(), 0, &hotSpot));
    REPORTER_ASSERT(r, hotSpot == SkIPoint::Make(5, 10));

    // Build two equal-quality entries whose payload offsets are the reverse of
    // their directory order. Stable sorting retains payload-offset order, and
    // each frame must retain the hotspot from its own directory record.
    const uint8_t* sourceEntry = bytes + kDirectoryHeaderSize;
    const uint32_t sourcePayloadSize = readU32(sourceEntry + 8);
    const uint32_t sourcePayloadOffset = readU32(sourceEntry + 12);
    const size_t sourcePayloadEnd = static_cast<size_t>(sourcePayloadOffset) + sourcePayloadSize;
    REPORTER_ASSERT(r, sourcePayloadEnd <= cursorData->size());
    if (sourcePayloadEnd > cursorData->size()) {
        return;
    }
    constexpr size_t kEqualEntryCount = 2;
    const size_t equalDirectorySize = kDirectoryHeaderSize + kEqualEntryCount * kDirectoryEntrySize;
    const size_t earlierPayloadOffset = equalDirectorySize;
    const size_t laterPayloadOffset = earlierPayloadOffset + sourcePayloadSize;
    sk_sp<SkData> equalQualityData =
            SkData::MakeUninitialized(laterPayloadOffset + sourcePayloadSize);
    auto* equalBytes = static_cast<uint8_t*>(equalQualityData->writable_data());
    std::memcpy(equalBytes, bytes, kDirectoryHeaderSize);
    equalBytes[4] = kEqualEntryCount;
    equalBytes[5] = 0;
    uint8_t* firstEqualEntry = equalBytes + kDirectoryHeaderSize;
    uint8_t* secondEqualEntry = firstEqualEntry + kDirectoryEntrySize;
    std::memcpy(firstEqualEntry, sourceEntry, kDirectoryEntrySize);
    std::memcpy(secondEqualEntry, sourceEntry, kDirectoryEntrySize);
    firstEqualEntry[4] = 40;
    firstEqualEntry[6] = 41;
    secondEqualEntry[4] = 20;
    secondEqualEntry[6] = 21;
    writeU32(firstEqualEntry + 12, SkToU32(laterPayloadOffset));
    writeU32(secondEqualEntry + 12, SkToU32(earlierPayloadOffset));
    std::memcpy(equalBytes + earlierPayloadOffset, bytes + sourcePayloadOffset, sourcePayloadSize);
    std::memcpy(equalBytes + laterPayloadOffset, bytes + sourcePayloadOffset, sourcePayloadSize);

    std::unique_ptr<SkCodec> equalQualityCodec =
            SkIcoRustDecoder::Decode(SkMemoryStream::Make(std::move(equalQualityData)), &result);
    REPORTER_ASSERT_SUCCESSFUL_CODEC_RESULT(r, result);
    REPORTER_ASSERT(r, equalQualityCodec);
    if (!equalQualityCodec) {
        return;
    }
    REPORTER_ASSERT(r, equalQualityCodec->getFrameCount() == 2);
    REPORTER_ASSERT(r, SkIcoRustDecoder::GetHotSpot(equalQualityCodec.get(), 0, &hotSpot));
    REPORTER_ASSERT(r, hotSpot == SkIPoint::Make(20, 21));
    REPORTER_ASSERT(r, SkIcoRustDecoder::GetHotSpot(equalQualityCodec.get(), 1, &hotSpot));
    REPORTER_ASSERT(r, hotSpot == SkIPoint::Make(40, 41));

    sk_sp<SkData> skippedEntryData = SkData::MakeWithCopy(cursorData->data(), cursorData->size());
    auto* skippedBytes = static_cast<uint8_t*>(skippedEntryData->writable_data());
    constexpr size_t kSkippedDirectoryIndex = 4;
    const uint8_t* skippedDirectoryEntry =
            skippedBytes + kDirectoryHeaderSize + kSkippedDirectoryIndex * kDirectoryEntrySize;
    const size_t skippedPayloadOffset = readU32(skippedDirectoryEntry + 12);
    REPORTER_ASSERT(r, skippedPayloadOffset + 4 <= skippedEntryData->size());
    if (skippedPayloadOffset + 4 > skippedEntryData->size()) {
        return;
    }
    std::memset(skippedBytes + skippedPayloadOffset, 0, 4);

    std::unique_ptr<SkCodec> skippedEntryCodec =
            SkIcoRustDecoder::Decode(SkMemoryStream::Make(std::move(skippedEntryData)), &result);
    REPORTER_ASSERT_SUCCESSFUL_CODEC_RESULT(r, result);
    REPORTER_ASSERT(r, skippedEntryCodec);
    if (!skippedEntryCodec) {
        return;
    }
    const SkIPoint kExpectedHotSpots[] = {
            SkIPoint::Make(4, 9),
            SkIPoint::Make(3, 8),
            SkIPoint::Make(2, 7),
            SkIPoint::Make(1, 6),
    };
    REPORTER_ASSERT(
            r,
            skippedEntryCodec->getFrameCount() == static_cast<int>(std::size(kExpectedHotSpots)));
    SkCodec::FrameInfo fallbackFrameInfo;
    REPORTER_ASSERT(r, skippedEntryCodec->getFrameInfo(0, &fallbackFrameInfo));
    REPORTER_ASSERT(r, fallbackFrameInfo.fFrameRect == SkIRect::MakeWH(64, 64));
    for (int frameIndex = 0; frameIndex < skippedEntryCodec->getFrameCount(); ++frameIndex) {
        REPORTER_ASSERT(
                r, SkIcoRustDecoder::GetHotSpot(skippedEntryCodec.get(), frameIndex, &hotSpot));
        REPORTER_ASSERT(r, hotSpot == kExpectedHotSpots[frameIndex]);
    }
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

DEF_TEST(RustIcoCodec_accept_directory_payload_dimension_mismatch, r) {
    sk_sp<SkData> data = GetResourceAsData("images/wrong-frame-dimensions.ico");
    REPORTER_ASSERT(r, data);
    if (!data) {
        return;
    }

    SkCodec::Result result;
    std::unique_ptr<SkCodec> codec =
            SkIcoRustDecoder::Decode(SkMemoryStream::Make(std::move(data)), &result);
    REPORTER_ASSERT_SUCCESSFUL_CODEC_RESULT(r, result);
    REPORTER_ASSERT(r, codec);
    if (!codec) {
        return;
    }

    REPORTER_ASSERT(r, codec->dimensions() == SkISize::Make(256, 256));
    REPORTER_ASSERT(r, codec->getFrameCount() == 4);

    constexpr SkISize kNativeSize = SkISize::Make(128, 128);
    constexpr int kFrameIndex = 2;
    SkCodec::FrameInfo frameInfo;
    REPORTER_ASSERT(r, codec->getFrameInfo(kFrameIndex, &frameInfo));
    REPORTER_ASSERT(r, frameInfo.fFrameRect == SkIRect::MakeWH(256, 256));

    const SkImageInfo canvasInfo =
            codec->getInfo().makeColorType(kN32_SkColorType).makeAlphaType(kPremul_SkAlphaType);
    SkBitmap canvas;
    REPORTER_ASSERT(r, canvas.tryAllocPixels(canvasInfo));
    canvas.eraseColor(SK_ColorMAGENTA);
    SkCodec::Options options;
    options.fFrameIndex = kFrameIndex;
    REPORTER_ASSERT_SUCCESSFUL_CODEC_RESULT(r, codec->getPixels(canvas.pixmap(), &options));

    SkBitmap native;
    REPORTER_ASSERT(r, native.tryAllocPixels(canvasInfo.makeDimensions(kNativeSize)));
    REPORTER_ASSERT_SUCCESSFUL_CODEC_RESULT(r, codec->getPixels(native.pixmap()));

    SkBitmap canvasSubset;
    REPORTER_ASSERT(r, canvas.extractSubset(&canvasSubset, SkIRect::MakeSize(kNativeSize)));
    REPORTER_ASSERT(r, ToolUtils::equal_pixels(canvasSubset.pixmap(), native.pixmap()));
    REPORTER_ASSERT(r, SkColorGetA(canvas.getColor(255, 255)) == 0);
}

DEF_TEST(RustIcoCodec_partial_directory_payload_dimension_mismatch, r) {
    sk_sp<SkData> data = GetResourceAsData("images/wrong-frame-dimensions.ico");
    REPORTER_ASSERT(r, data);
    if (!data) {
        return;
    }

    constexpr size_t kFirstPayloadEnd = 1376;
    REPORTER_ASSERT(r, data->size() >= kFirstPayloadEnd);
    if (data->size() < kFirstPayloadEnd) {
        return;
    }
    sk_sp<SkData> partial = SkData::MakeSubset(data.get(), 0, kFirstPayloadEnd);

    SkCodec::Result result;
    std::unique_ptr<SkCodec> codec =
            SkIcoRustDecoder::Decode(SkMemoryStream::Make(std::move(partial)), &result);
    REPORTER_ASSERT_SUCCESSFUL_CODEC_RESULT(r, result);
    REPORTER_ASSERT(r, codec);
    if (!codec) {
        return;
    }

    REPORTER_ASSERT(r, codec->dimensions() == SkISize::Make(256, 256));
    REPORTER_ASSERT(r, codec->getFrameCount() == 1);
    SkCodec::FrameInfo frameInfo;
    REPORTER_ASSERT(r, codec->getFrameInfo(0, &frameInfo));
    REPORTER_ASSERT(r, frameInfo.fFrameRect == SkIRect::MakeWH(256, 256));

    const SkImageInfo canvasInfo =
            codec->getInfo().makeColorType(kN32_SkColorType).makeAlphaType(kPremul_SkAlphaType);
    SkBitmap bitmap;
    REPORTER_ASSERT(r, bitmap.tryAllocPixels(canvasInfo));
    bitmap.eraseColor(SK_ColorMAGENTA);
    SkCodec::Options options;
    options.fFrameIndex = 0;
    REPORTER_ASSERT_SUCCESSFUL_CODEC_RESULT(r, codec->getPixels(bitmap.pixmap(), &options));
    REPORTER_ASSERT(r, SkColorGetA(bitmap.getColor(255, 255)) == 0);
}

DEF_TEST(RustIcoCodec_incomplete_before_first_payload, r) {
    sk_sp<SkData> data = GetResourceAsData("images/wrong-frame-dimensions.ico");
    REPORTER_ASSERT(r, data);
    if (!data) {
        return;
    }

    constexpr size_t kPartialSize = 100;
    REPORTER_ASSERT(r, data->size() > kPartialSize);
    sk_sp<SkData> partial = SkData::MakeSubset(data.get(), 0, kPartialSize);

    SkCodec::Result result;
    std::unique_ptr<SkCodec> codec =
            SkIcoRustDecoder::Decode(SkMemoryStream::Make(std::move(partial)), &result);

    REPORTER_ASSERT(r, !codec);
    REPORTER_ASSERT(r, result == SkCodec::kIncompleteInput);
}

DEF_TEST(RustIcoCodec_metadata_only_entry_preserves_size, r) {
    sk_sp<SkData> data = GetResourceAsData("images/color_wheel.ico");
    REPORTER_ASSERT(r, data);
    if (!data) {
        return;
    }

    sk_sp<SkData> mutated = SkData::MakeUninitialized(data->size());
    std::memcpy(mutated->writable_data(), data->data(), data->size());

    constexpr size_t kDirectoryHeaderSize = 6;
    constexpr size_t kDirectoryEntrySize = 16;
    REPORTER_ASSERT(r, data->size() >= kDirectoryHeaderSize + kDirectoryEntrySize);
    if (data->size() < kDirectoryHeaderSize + kDirectoryEntrySize) {
        return;
    }
    auto* bytes = static_cast<uint8_t*>(mutated->writable_data());
    bytes[4] = 1;
    bytes[5] = 0;
    bytes[kDirectoryHeaderSize] = 1;
    bytes[kDirectoryHeaderSize + 1] = 1;

    SkCodec::Result result;
    std::unique_ptr<SkCodec> codec =
            SkIcoRustDecoder::Decode(SkMemoryStream::Make(std::move(mutated)), &result);

    REPORTER_ASSERT_SUCCESSFUL_CODEC_RESULT(r, result);
    REPORTER_ASSERT(r, codec);
    if (!codec) {
        return;
    }

    REPORTER_ASSERT(r, codec->dimensions() == SkISize::Make(1, 1));
    REPORTER_ASSERT(r, codec->getFrameCount() == 1);
    SkCodec::FrameInfo frameInfo;
    REPORTER_ASSERT(r, codec->getFrameInfo(0, &frameInfo));
    REPORTER_ASSERT(r, frameInfo.fFrameRect == SkIRect::MakeWH(1, 1));

    constexpr uint32_t kSentinel = 0xA5A5A5A5;
    uint32_t pixels[] = {kSentinel, kSentinel};
    const SkImageInfo info = codec->getInfo().makeColorType(kN32_SkColorType);
    REPORTER_ASSERT(r,
                    codec->getPixels(info, pixels, info.minRowBytes()) == SkCodec::kInvalidScale);
    REPORTER_ASSERT(r, pixels[0] == kSentinel);
    REPORTER_ASSERT(r, pixels[1] == kSentinel);
}

DEF_TEST(RustIcoCodec_mixed_metadata_only_entry, r) {
    sk_sp<SkData> data = GetResourceAsData("images/color_wheel.ico");
    REPORTER_ASSERT(r, data);
    if (!data) {
        return;
    }

    sk_sp<SkData> mutated = SkData::MakeUninitialized(data->size());
    std::memcpy(mutated->writable_data(), data->data(), data->size());
    auto* bytes = static_cast<uint8_t*>(mutated->writable_data());
    constexpr size_t kDirectoryHeaderSize = 6;
    constexpr size_t kDirectoryEntrySize = 16;
    constexpr size_t kLargestEntryIndex = 4;
    const size_t largestEntryOffset =
            kDirectoryHeaderSize + kLargestEntryIndex * kDirectoryEntrySize;
    bytes[largestEntryOffset] = 1;
    bytes[largestEntryOffset + 1] = 1;

    SkCodec::Result result;
    std::unique_ptr<SkCodec> codec =
            SkIcoRustDecoder::Decode(SkMemoryStream::Make(std::move(mutated)), &result);
    REPORTER_ASSERT_SUCCESSFUL_CODEC_RESULT(r, result);
    REPORTER_ASSERT(r, codec);
    if (!codec) {
        return;
    }

    REPORTER_ASSERT(r, codec->dimensions() == SkISize::Make(64, 64));
    REPORTER_ASSERT(r, codec->getFrameCount() == 5);
    REPORTER_ASSERT(r, codec->getScaledDimensions(1.0f) == SkISize::Make(64, 64));

    SkCodec::FrameInfo frameInfo;
    REPORTER_ASSERT(r, codec->getFrameInfo(0, &frameInfo));
    REPORTER_ASSERT(r, frameInfo.fFrameRect == SkIRect::MakeWH(64, 64));
    constexpr int kMetadataOnlyFrameIndex = 4;
    REPORTER_ASSERT(r, codec->getFrameInfo(kMetadataOnlyFrameIndex, &frameInfo));
    REPORTER_ASSERT(r, frameInfo.fFrameRect == SkIRect::MakeWH(1, 1));

    const SkImageInfo info = codec->getInfo().makeColorType(kN32_SkColorType);
    SkBitmap actual;
    SkBitmap expected;
    REPORTER_ASSERT(r, actual.tryAllocPixels(info));
    REPORTER_ASSERT(r, expected.tryAllocPixels(info));
    actual.eraseColor(SK_ColorMAGENTA);
    expected.eraseColor(SK_ColorMAGENTA);

    SkCodec::Options options;
    options.fFrameIndex = kMetadataOnlyFrameIndex;
    REPORTER_ASSERT(r, codec->getPixels(actual.pixmap(), &options) == SkCodec::kInvalidScale);
    REPORTER_ASSERT(r, ToolUtils::equal_pixels(actual.pixmap(), expected.pixmap()));

    options.fFrameIndex = 0;
    REPORTER_ASSERT_SUCCESSFUL_CODEC_RESULT(r, codec->getPixels(actual.pixmap(), &options));
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

// Test that an oversized payload is retained for metadata but cannot be decoded.
DEF_TEST(RustIcoCodec_oversized_payload_is_metadata_only, r) {
    sk_sp<SkData> data = GetResourceAsData("images/ico_oversized_512x384.ico");
    if (!data) {
        ERRORF(r, "Missing resource: images/ico_oversized_512x384.ico");
        return;
    }

    SkCodec::Result result;
    std::unique_ptr<SkCodec> codec =
            SkIcoRustDecoder::Decode(SkMemoryStream::Make(std::move(data)), &result);

    REPORTER_ASSERT_SUCCESSFUL_CODEC_RESULT(r, result);
    REPORTER_ASSERT(r, codec);
    if (!codec) {
        return;
    }

    REPORTER_ASSERT(r, codec->dimensions() == SkISize::Make(256, 256));
    SkBitmap bitmap;
    REPORTER_ASSERT(r, bitmap.tryAllocPixels(codec->getInfo().makeColorType(kN32_SkColorType)));
    REPORTER_ASSERT(r, codec->getPixels(bitmap.pixmap()) == SkCodec::kInvalidScale);
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

    // No embedded images can be decoded yet, so callers should retry with more data.
    REPORTER_ASSERT(r, !codec,
                    "Codec should not be created when all entries are truncated");
    REPORTER_ASSERT(r, result == SkCodec::kIncompleteInput,
                    "Expected kIncompleteInput, got %s", SkCodec::ResultToString(result));
}

// Test that an ICO file truncated so that only some embedded images are present
// still reports kSuccess (matching SkIcoCodec) and produces a usable codec whose
// reported size is stable, derived from the ICO directory rather than from the
// subset of entries that happen to be decodable so far.
//
// This stability is required by progressive/deferred clients such as Blink: they
// lock the image size from the first successful parse, so a size that grew as
// more data arrived would leave earlier buffers undersized and overflow on a
// later decode. Reporting kIncompleteInput here would instead leave such clients
// with no size at all. See SkIcoRustCodec::MakeFromStream for details.
DEF_TEST(RustIcoCodec_truncated_some_entries_missing, r) {
    sk_sp<SkData> data = GetResourceAsData("images/color_wheel.ico");
    if (!data) {
        ERRORF(r, "Missing resource: images/color_wheel.ico");
        return;
    }
    // Reported size with the full, well-formed file, for the stability check.
    SkCodec::Result fullResult;
    std::unique_ptr<SkCodec> fullCodec =
            SkIcoRustDecoder::Decode(SkMemoryStream::Make(data), &fullResult);
    REPORTER_ASSERT(r, fullCodec, "Codec creation should succeed with full data");
    const SkISize fullDims = fullCodec ? fullCodec->dimensions() : SkISize::MakeEmpty();

    // color_wheel.ico has 5 BMP images. Truncate to roughly half the file so
    // some entries are fully present but later ones are cut off.
    const size_t truncatedSize = data->size() / 2;
    sk_sp<SkData> truncated = SkData::MakeSubset(data.get(), 0, truncatedSize);

    SkCodec::Result result;
    std::unique_ptr<SkCodec> codec =
            SkIcoRustDecoder::Decode(SkMemoryStream::Make(std::move(truncated)), &result);

    if (codec) {
        // As long as we have a usable codec we report kSuccess, mirroring
        // SkIcoCodec.
        REPORTER_ASSERT(r, result == SkCodec::kSuccess,
                        "Expected kSuccess for partially truncated ICO with usable "
                        "entries, got %s",
                        SkCodec::ResultToString(result));

        REPORTER_ASSERT(r, codec->getFrameCount() > 0);
        REPORTER_ASSERT(r, codec->getFrameCount() < fullCodec->getFrameCount());

        // The reported size must match the full-file size even though fewer
        // entries are decodable -- it comes from the directory, not the decodable
        // subset. This is the property that prevents progressive-decode overflows.
        REPORTER_ASSERT(r, codec->dimensions() == fullDims,
                        "Partial-data size (%dx%d) should equal full-data size (%dx%d)",
                        codec->dimensions().width(), codec->dimensions().height(),
                        fullDims.width(), fullDims.height());

        // getInfo() must describe a non-empty image so callers can size buffers.
        REPORTER_ASSERT(r, !codec->getInfo().isEmpty(),
                        "Codec info should describe a non-empty image");

        SkBitmap bitmap;
        REPORTER_ASSERT(r, bitmap.tryAllocPixels(
                                   codec->getInfo().makeColorType(kN32_SkColorType)));
        SkCodec::Options options;
        options.fFrameIndex = 0;
        REPORTER_ASSERT_SUCCESSFUL_CODEC_RESULT(
                r, codec->getPixels(bitmap.pixmap(), &options));
    } else {
        // If no entries decoded at all, kInvalidInput is acceptable.
        REPORTER_ASSERT(r, result == SkCodec::kInvalidInput ||
                           result == SkCodec::kIncompleteInput,
                        "Expected kInvalidInput or kIncompleteInput, got %s",
                        SkCodec::ResultToString(result));
    }
}
