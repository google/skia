/*
 * Copyright 2026 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "experimental/rust_jpeg/decoder/SkJpegRustDecoder.h"

#include <algorithm>
#include <cmath>
#include <cstring>
#include <functional>
#include <memory>
#include <utility>
#include <vector>

#include "include/codec/SkCodec.h"
#include "include/core/SkBitmap.h"
#include "include/core/SkColor.h"
#include "include/core/SkColorSpace.h"
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
#include "tools/DecodeUtils.h"
#include "tools/Resources.h"

#if defined(SK_CODEC_ENCODES_JPEG_WITH_RUST)
#include "experimental/rust_jpeg/encoder/SkJpegRustEncoder.h"
#include "include/encode/SkJpegEncoder.h"
#include "tools/ToolUtils.h"
#endif

#if defined(SK_CODEC_DECODES_JPEG_GAINMAPS)
#include "include/codec/SkAndroidCodec.h"
#include "include/private/SkGainmapInfo.h"
#endif

#if defined(SK_CODEC_DECODES_JPEG)
#include "include/codec/SkEncodedOrigin.h"
#include "include/codec/SkJpegDecoder.h"
#include "modules/skcms/skcms.h"
#endif

#define REPORTER_ASSERT_SUCCESSFUL_CODEC_RESULT(r, actualResult)           \
    REPORTER_ASSERT(r,                                                     \
                    (actualResult) == SkCodec::kSuccess,                   \
                    "actualResult=\"%s\" != kSuccess",                     \
                    SkCodec::ResultToString(actualResult))

// Helper wrapping a call to `SkJpegRustDecoder::Decode`.
static std::unique_ptr<SkCodec> decode_rust_jpeg(skiatest::Reporter* r,
                                                 const char* path) {
    skiatest::ReporterContext ctx(r, path);
    sk_sp<SkData> data = GetResourceAsData(path);
    if (!data) {
        ERRORF(r, "Missing resource: %s", path);
        return nullptr;
    }

    SkCodec::Result result;
    std::unique_ptr<SkCodec> codec =
            SkJpegRustDecoder::Decode(SkMemoryStream::Make(std::move(data)), &result);
    REPORTER_ASSERT_SUCCESSFUL_CODEC_RESULT(r, result);
    return codec;
}

// ========== Decoder Tests ==========

DEF_TEST(RustJpegCodec_IsJpeg, r) {
    sk_sp<SkData> jpegData = GetResourceAsData("images/color_wheel.jpg");
    if (!jpegData) {
        ERRORF(r, "Missing resource: images/color_wheel.jpg");
        return;
    }
    REPORTER_ASSERT(r, SkJpegRustDecoder::IsJpeg(jpegData->data(), jpegData->size()));

    sk_sp<SkData> pngData = GetResourceAsData("images/color_wheel.png");
    if (!pngData) {
        ERRORF(r, "Missing resource: images/color_wheel.png");
        return;
    }
    REPORTER_ASSERT(r, !SkJpegRustDecoder::IsJpeg(pngData->data(), pngData->size()));
}

// Decode a variety of JPEG images covering baseline, progressive, grayscale,
// different subsampling modes, ICC profiles, and other edge cases.
DEF_TEST(RustJpegCodec_decode_valid_jpgs, r) {
    struct TestCase {
        const char* path;
        SkISize expectedDimensions;
    };

    static const TestCase kTestCases[] = {
        {"images/color_wheel.jpg",       SkISize::Make(128, 128)},
        {"images/grayscale.jpg",         SkISize::Make(128, 128)},
        {"images/dog.jpg",               SkISize::Make(180, 180)},
        {"images/b78329453.jpeg",        SkISize::Make(635, 760)},
        {"images/mandrill_512_q075.jpg", SkISize::Make(512, 512)},
        {"images/ducky.jpg",             SkISize::Make(489, 537)},
        {"images/mandrill_h1v1.jpg",     SkISize::Make(512, 512)},
        {"images/mandrill_h2v1.jpg",     SkISize::Make(512, 512)},
        {"images/cropped_mandrill.jpg",  SkISize::Make(439, 154)},
        {"images/flutter_logo.jpg",      SkISize::Make(400, 400)},
        {"images/icc-v2-gbr.jpg",        SkISize::Make(275, 207)},
        {"images/randPixels.jpg",        SkISize::Make(8, 8)},
        {"images/crbug999986.jpeg",      SkISize::Make(2000, 1500)},
        {"images/CMYK.jpg",              SkISize::Make(642, 516)},
    };

    for (const auto& tc : kTestCases) {
        skiatest::ReporterContext ctx(r, tc.path);

        std::unique_ptr<SkCodec> codec = decode_rust_jpeg(r, tc.path);
        if (!codec) {
            continue;
        }

        REPORTER_ASSERT(r, codec->getFrameCount() == 1);
        REPORTER_ASSERT(r,
                        codec->dimensions() == tc.expectedDimensions,
                        "dimensions=%dx%d != expected=%dx%d",
                        codec->dimensions().width(), codec->dimensions().height(),
                        tc.expectedDimensions.width(), tc.expectedDimensions.height());

        auto [image, result] = codec->getImage();
        REPORTER_ASSERT_SUCCESSFUL_CODEC_RESULT(r, result);
        REPORTER_ASSERT(r, image);
    }
}

// Verify true incremental resumption with a HaltingStream: start decoding with
// incomplete input, observe kIncompleteInput, progressively expose more of the
// same stream, and complete without restarting the decoder.
DEF_TEST(RustJpeg_IncrementalDecode_PartialStreaming, r) {
    struct TestCase {
        const char* path;
        const char* description;
        bool expectProgressivePreview;
    };

    static const TestCase kTestCases[] = {
        {"images/color_wheel.jpg",       "baseline RGB 128x128", false},
        {"images/dog.jpg",               "RGB 180x180",          false},
        {"images/mandrill_512_q075.jpg", "RGB 512x512",          false},
        {"images/b78329453.jpeg",        "progressive RGB 635x760", true},
    };

    // JPEG headers (SOS, DQT, DHT tables...) can exceed 1 KB. This leaves
    // enough bytes to construct the codec while withholding image data.
    constexpr size_t kInitialBytes = 4 * 1024;
    constexpr size_t kChunkSize = 4 * 1024;
    constexpr int kMaxIterations = 1000;

    for (const auto& testCase : kTestCases) {
        skiatest::ReporterContext ctx(r, testCase.description);

        sk_sp<SkData> data = GetResourceAsData(testCase.path);
        if (!data) {
            ERRORF(r, "Missing resource: %s", testCase.path);
            continue;
        }

        // Decode the full image first so we can verify the incremental result.
        std::unique_ptr<SkCodec> refCodec =
                SkJpegRustDecoder::Decode(SkMemoryStream::Make(data), nullptr);
        if (!refCodec) {
            ERRORF(r, "Failed to create reference codec for %s", testCase.path);
            continue;
        }
        SkBitmap reference;
        reference.allocPixels(refCodec->getInfo().makeColorType(kN32_SkColorType));
        {
            SkCodec::Result refResult = refCodec->getPixels(reference.pixmap());
            REPORTER_ASSERT_SUCCESSFUL_CODEC_RESULT(r, refResult);
            if (refResult != SkCodec::kSuccess) {
                continue;
            }
        }

        REPORTER_ASSERT(r, data->size() > kInitialBytes,
                        "%s: test image must exceed the initial stream window",
                        testCase.description);
        if (data->size() <= kInitialBytes) {
            continue;
        }

        // Create a HaltingStream that starts with the headers and partial image data.
        const size_t initialBytes = kInitialBytes;
        auto haltingStream = std::make_unique<HaltingStream>(data, initialBytes);
        HaltingStream* streamPtr = haltingStream.get();

        SkCodec::Result result;
        std::unique_ptr<SkCodec> codec =
                SkJpegRustDecoder::Decode(std::move(haltingStream), &result);
        if (!codec) {
            ERRORF(r, "%s: failed to create streaming codec with %zu bytes: %s",
                   testCase.description, initialBytes, SkCodec::ResultToString(result));
            continue;
        }

        const SkImageInfo info = codec->getInfo().makeColorType(kN32_SkColorType);
        SkBitmap bitmap;
        if (!bitmap.tryAllocPixels(info)) {
            ERRORF(r, "%s: failed to allocate bitmap", testCase.description);
            continue;
        }
        memset(bitmap.getPixels(), 0xAB, bitmap.computeByteSize());

        result = codec->startIncrementalDecode(info, bitmap.getPixels(), bitmap.rowBytes());
        REPORTER_ASSERT_SUCCESSFUL_CODEC_RESULT(r, result);
        if (result != SkCodec::kSuccess) {
            continue;
        }

        int rowsDecoded = 0;
        int previousRowsDecoded = 0;
        int iterations = 0;
        bool sawIncompleteInput = false;
        bool sawPartialOutput = false;
        bool checkedNoNewDataRetry = false;
        int progressivePreviewCount = 0;
        std::vector<uint8_t> previousPixels(bitmap.computeByteSize(), 0xAB);

        while (iterations++ < kMaxIterations) {
            result = codec->incrementalDecode(&rowsDecoded);

            REPORTER_ASSERT(r, rowsDecoded >= previousRowsDecoded,
                            "%s: decoded row count regressed from %d to %d",
                            testCase.description, previousRowsDecoded, rowsDecoded);

            const uint8_t* pixels = static_cast<const uint8_t*>(bitmap.getPixels());
            if (!testCase.expectProgressivePreview && previousRowsDecoded > 0) {
                const size_t stableBytes = static_cast<size_t>(previousRowsDecoded) *
                                           bitmap.rowBytes();
                REPORTER_ASSERT(r, memcmp(pixels, previousPixels.data(), stableBytes) == 0,
                                "%s: previously reported stable rows changed",
                                testCase.description);
            }

            if (result == SkCodec::kSuccess) {
                break;
            }

            REPORTER_ASSERT(r, result == SkCodec::kIncompleteInput,
                            "%s: partial input should return kIncompleteInput, got %s",
                            testCase.description, SkCodec::ResultToString(result));
            if (result != SkCodec::kIncompleteInput) {
                break;
            }
            sawIncompleteInput = true;

            const bool pixelsChanged =
                    std::any_of(pixels,
                                pixels + bitmap.computeByteSize(),
                                [](uint8_t value) { return value != 0xAB; });
            if (testCase.expectProgressivePreview) {
                if (memcmp(pixels, previousPixels.data(), bitmap.computeByteSize()) != 0) {
                    progressivePreviewCount++;
                    sawPartialOutput = true;
                    REPORTER_ASSERT(r, rowsDecoded > 0,
                                    "%s: preview pixels were not reported as initialized",
                                    testCase.description);
                }
            } else if (rowsDecoded > 0) {
                sawPartialOutput = true;
                REPORTER_ASSERT(r, pixelsChanged,
                                "%s: reported stable rows were not written",
                                testCase.description);
            }

            for (int y = 0; y < rowsDecoded; ++y) {
                for (int x = 0; x < info.width(); ++x) {
                    REPORTER_ASSERT(r, SkColorGetA(bitmap.getColor(x, y)) == SK_AlphaOPAQUE,
                                    "%s: initialized pixel (%d,%d) is not opaque",
                                    testCase.description, x, y);
                }
            }

            previousRowsDecoded = rowsDecoded;
            memcpy(previousPixels.data(), pixels, bitmap.computeByteSize());

            if (!checkedNoNewDataRetry && sawPartialOutput) {
                int retryRowsDecoded = -1;
                const SkCodec::Result retryResult = codec->incrementalDecode(&retryRowsDecoded);
                REPORTER_ASSERT(r, retryResult == SkCodec::kIncompleteInput,
                                "%s: retry without new data returned %s",
                                testCase.description, SkCodec::ResultToString(retryResult));
                REPORTER_ASSERT(r, retryRowsDecoded == rowsDecoded,
                                "%s: retry changed initialized rows from %d to %d",
                                testCase.description, rowsDecoded, retryRowsDecoded);
                REPORTER_ASSERT(r,
                                memcmp(pixels, previousPixels.data(), bitmap.computeByteSize()) == 0,
                                "%s: retry without new data changed output",
                                testCase.description);
                checkedNoNewDataRetry = true;
            }

            if (streamPtr->isAllDataReceived()) {
                ERRORF(r, "%s: still incomplete after all data was available",
                       testCase.description);
                break;
            }

            // Do not call startIncrementalDecode() again. Each subsequent call
            // must resume the existing Rust/zune decoder after one more chunk
            // becomes available.
            streamPtr->addNewData(kChunkSize);
        }

        REPORTER_ASSERT(r, iterations < kMaxIterations,
                        "%s: decode loop exceeded max iterations", testCase.description);
        REPORTER_ASSERT(r, sawIncompleteInput,
                        "%s: decode never observed incomplete input", testCase.description);
        REPORTER_ASSERT(r, sawPartialOutput,
                "%s: no partial output was exposed before all input arrived",
                testCase.description);
        REPORTER_ASSERT(r, checkedNoNewDataRetry,
                "%s: retry without new input was not exercised",
                testCase.description);
        if (testCase.expectProgressivePreview) {
            REPORTER_ASSERT(r, progressivePreviewCount >= 2,
                    "%s: expected multiple progressive preview replacements, got %d",
                    testCase.description, progressivePreviewCount);
        }
        REPORTER_ASSERT(r, result == SkCodec::kSuccess,
                        "%s: resumed decode expected kSuccess, got %s",
                        testCase.description, SkCodec::ResultToString(result));
        REPORTER_ASSERT(r, rowsDecoded == info.height(),
                        "%s: resumed decode reported %d of %d rows",
                        testCase.description, rowsDecoded, info.height());

        // Verify dimensions match the reference.
        REPORTER_ASSERT(r,
                        bitmap.dimensions() == reference.dimensions(),
                        "%s: incremental dims %dx%d != reference %dx%d",
                        testCase.description,
                        bitmap.width(), bitmap.height(),
                        reference.width(), reference.height());

        // Both paths used the same Rust decoder, so pixel output should be
        // identical (zero tolerance).
        if (bitmap.dimensions() == reference.dimensions()) {
            const float tols[4] = {0, 0, 0, 0};
            auto error = std::function<ComparePixmapsErrorReporter>(
                    [&](int x, int y, const float diffs[4]) {
                        ERRORF(r,
                               "%s: pixel differs at (%d,%d): diffs=(%f,%f,%f,%f)",
                               testCase.description,
                               x, y,
                               diffs[0], diffs[1], diffs[2], diffs[3]);
                    });
            ComparePixels(bitmap.pixmap(), reference.pixmap(), tols, error);
        }
    }
}

DEF_TEST(RustJpeg_IncrementalDecode_RejectsSubset, r) {
    std::unique_ptr<SkCodec> codec = decode_rust_jpeg(r, "images/color_wheel.jpg");
    if (!codec) {
        return;
    }

    const SkImageInfo info = codec->getInfo().makeColorType(kN32_SkColorType);
    const SkIRect subset = SkIRect::MakeXYWH(0, 1, info.width(), info.height() - 1);
    SkBitmap bitmap;
    bitmap.allocPixels(info.makeWH(subset.width(), subset.height()));

    SkCodec::Options options;
    options.fSubset = &subset;
    const SkCodec::Result result = codec->startIncrementalDecode(
            info, bitmap.getPixels(), bitmap.rowBytes(), &options);
    REPORTER_ASSERT(r, result == SkCodec::kInvalidParameters,
                    "incremental subset decode returned %s",
                    SkCodec::ResultToString(result));
}

DEF_TEST(RustJpeg_IncrementalDecode_RejectsShortRowBytes, r) {
    std::unique_ptr<SkCodec> codec = decode_rust_jpeg(r, "images/color_wheel.jpg");
    if (!codec) {
        return;
    }

    const SkImageInfo info = codec->getInfo().makeColorType(kN32_SkColorType);
    SkBitmap bitmap;
    bitmap.allocPixels(info);
    const SkCodec::Result result = codec->startIncrementalDecode(
            info, bitmap.getPixels(), info.minRowBytes() - 1);
    REPORTER_ASSERT(r, result == SkCodec::kInvalidParameters,
                    "short incremental row bytes returned %s",
                    SkCodec::ResultToString(result));
}

DEF_TEST(RustJpeg_IncrementalDecode_AfterRewind, r) {
    std::unique_ptr<SkCodec> codec = decode_rust_jpeg(r, "images/color_wheel.jpg");
    if (!codec) {
        return;
    }

    const SkImageInfo info = codec->getInfo().makeColorType(kN32_SkColorType);
    SkBitmap firstIncremental;
    firstIncremental.allocPixels(info);
    REPORTER_ASSERT_SUCCESSFUL_CODEC_RESULT(
            r,
            codec->startIncrementalDecode(
                    info, firstIncremental.getPixels(), firstIncremental.rowBytes()));
    int rowsDecoded = 0;
    REPORTER_ASSERT_SUCCESSFUL_CODEC_RESULT(r, codec->incrementalDecode(&rowsDecoded));
    REPORTER_ASSERT(r, rowsDecoded == info.height());

    auto [fullImage, fullResult] = codec->getImage();
    REPORTER_ASSERT_SUCCESSFUL_CODEC_RESULT(r, fullResult);
    REPORTER_ASSERT(r, fullImage);

    SkBitmap secondIncremental;
    secondIncremental.allocPixels(info);
    REPORTER_ASSERT_SUCCESSFUL_CODEC_RESULT(
            r,
            codec->startIncrementalDecode(
                    info, secondIncremental.getPixels(), secondIncremental.rowBytes()));
    rowsDecoded = 0;
    REPORTER_ASSERT_SUCCESSFUL_CODEC_RESULT(r, codec->incrementalDecode(&rowsDecoded));
    REPORTER_ASSERT(r, rowsDecoded == info.height());

    REPORTER_ASSERT(r, firstIncremental.computeByteSize() == secondIncremental.computeByteSize());
    REPORTER_ASSERT(r,
                    memcmp(firstIncremental.getPixels(),
                           secondIncremental.getPixels(),
                           firstIncremental.computeByteSize()) == 0);
}

DEF_TEST(RustJpeg_IncrementalDecode_ColorTransformPaddedStride, r) {
    // This file has more than 32 KiB of metadata before its decodable scan data.
    constexpr size_t kInitialBytes = 34 * 1024;
    constexpr size_t kChunkSize = 4 * 1024;
    constexpr int kMaxIterations = 1000;

    sk_sp<SkData> data = GetResourceAsData("images/icc-v2-gbr.jpg");
    if (!data) {
        ERRORF(r, "Missing resource: images/icc-v2-gbr.jpg");
        return;
    }

    std::unique_ptr<SkCodec> referenceCodec =
            SkJpegRustDecoder::Decode(SkMemoryStream::Make(data), nullptr);
    REPORTER_ASSERT(r, referenceCodec);
    if (!referenceCodec) {
        return;
    }

    const SkImageInfo dstInfo = referenceCodec->getInfo()
                                        .makeColorType(kRGBA_8888_SkColorType)
                                        .makeColorSpace(SkColorSpace::MakeSRGB());
    SkBitmap reference;
    reference.allocPixels(dstInfo);
    REPORTER_ASSERT_SUCCESSFUL_CODEC_RESULT(
            r,
            referenceCodec->getPixels(dstInfo, reference.getPixels(), reference.rowBytes()));

    auto haltingStream = std::make_unique<HaltingStream>(data, kInitialBytes);
    HaltingStream* streamPtr = haltingStream.get();
    SkCodec::Result result;
    std::unique_ptr<SkCodec> codec =
            SkJpegRustDecoder::Decode(std::move(haltingStream), &result);
    REPORTER_ASSERT_SUCCESSFUL_CODEC_RESULT(r, result);
    REPORTER_ASSERT(r, codec);
    if (!codec) {
        return;
    }

    const size_t paddedRowBytes = dstInfo.minRowBytes() + 32;
    SkBitmap incremental;
    incremental.allocPixels(dstInfo, paddedRowBytes);
    memset(incremental.getPixels(), 0xAB, incremental.computeByteSize());

    REPORTER_ASSERT_SUCCESSFUL_CODEC_RESULT(
            r,
            codec->startIncrementalDecode(
                    dstInfo, incremental.getPixels(), incremental.rowBytes()));

    int rowsDecoded = 0;
    int iterations = 0;
    bool sawIncompleteInput = false;
    do {
        result = codec->incrementalDecode(&rowsDecoded);
        if (result == SkCodec::kIncompleteInput) {
            sawIncompleteInput = true;
            REPORTER_ASSERT(r, !streamPtr->isAllDataReceived());
            streamPtr->addNewData(kChunkSize);
        }
    } while (result == SkCodec::kIncompleteInput && iterations++ < kMaxIterations);

    REPORTER_ASSERT(r, sawIncompleteInput);
    REPORTER_ASSERT(r, iterations < kMaxIterations);
    REPORTER_ASSERT_SUCCESSFUL_CODEC_RESULT(r, result);
    REPORTER_ASSERT(r, rowsDecoded == dstInfo.height());

    const float tolerances[4] = {0, 0, 0, 0};
    auto reportError = std::function<ComparePixmapsErrorReporter>(
            [&](int x, int y, const float diffs[4]) {
                ERRORF(r,
                       "incremental transformed pixel differs at (%d,%d): "
                       "diffs=(%f,%f,%f,%f)",
                       x, y, diffs[0], diffs[1], diffs[2], diffs[3]);
            });
    ComparePixels(reference.pixmap(),
                  incremental.pixmap(),
                  tolerances,
                  reportError);

    const size_t pixelBytes = dstInfo.minRowBytes();
    for (int y = 0; y + 1 < dstInfo.height(); ++y) {
        const uint8_t* row = static_cast<const uint8_t*>(incremental.getAddr(0, y));
        REPORTER_ASSERT(r,
                        std::all_of(row + pixelBytes,
                                    row + paddedRowBytes,
                                    [](uint8_t value) { return value == 0xAB; }),
                        "incremental decode modified row %d padding", y);
    }
}

DEF_TEST(RustJpegCodec_rewind, r) {
    std::unique_ptr<SkCodec> codec = decode_rust_jpeg(r, "images/color_wheel.jpg");
    if (!codec) {
        return;
    }

    auto [image1, result1] = codec->getImage();
    REPORTER_ASSERT_SUCCESSFUL_CODEC_RESULT(r, result1);
    REPORTER_ASSERT(r, image1);

    auto [image2, result2] = codec->getImage();
    REPORTER_ASSERT_SUCCESSFUL_CODEC_RESULT(r, result2);
    REPORTER_ASSERT(r, image2);

    REPORTER_ASSERT(r, image1->dimensions() == image2->dimensions());
}

DEF_TEST(RustJpegCodec_incomplete_input, r) {
    sk_sp<SkData> data = GetResourceAsData("images/color_wheel.jpg");
    if (!data) {
        ERRORF(r, "Missing resource: images/color_wheel.jpg");
        return;
    }

    // Keep only the first third of the data.
    sk_sp<SkData> truncated = SkData::MakeSubset(data.get(), 0, data->size() / 3);

    SkCodec::Result result;
    std::unique_ptr<SkCodec> codec =
            SkJpegRustDecoder::Decode(SkMemoryStream::Make(std::move(truncated)), &result);
    if (!codec) {
        // Acceptable: codec construction failed.
        REPORTER_ASSERT(r, result != SkCodec::kSuccess);
        return;
    }

    // If codec constructed, decoding may succeed (Rust decoder is lenient with
    // truncated data) or fail - either outcome is acceptable.
    auto [image, decodeResult] = codec->getImage();
    (void)decodeResult;
}

DEF_TEST(RustJpegCodec_reject_non_jpeg, r) {
    sk_sp<SkData> data = GetResourceAsData("images/color_wheel.png");
    if (!data) {
        ERRORF(r, "Missing resource: images/color_wheel.png");
        return;
    }

    SkCodec::Result result;
    std::unique_ptr<SkCodec> codec =
            SkJpegRustDecoder::Decode(SkMemoryStream::Make(std::move(data)), &result);
    REPORTER_ASSERT(r, !codec, "Should reject PNG data as JPEG");
}

// Decode an ICC-profiled JPEG into an explicit sRGB color space, exercising the
// applyColorXform() path (xformOnDecode == true).
DEF_TEST(RustJpegCodec_decode_with_color_transform, r) {
    std::unique_ptr<SkCodec> codec = decode_rust_jpeg(r, "images/icc-v2-gbr.jpg");
    if (!codec) {
        return;
    }

    // The image has a non-sRGB ICC profile. Requesting sRGB forces a color transform.
    SkImageInfo dstInfo = codec->getInfo()
        .makeColorType(kRGBA_8888_SkColorType)
        .makeColorSpace(SkColorSpace::MakeSRGB());
    SkBitmap bitmap;
    bitmap.allocPixels(dstInfo);
    SkCodec::Result result = codec->getPixels(dstInfo, bitmap.getPixels(), bitmap.rowBytes());
    REPORTER_ASSERT_SUCCESSFUL_CODEC_RESULT(r, result);

    // Sanity: the decoded pixels should not be all zero (would indicate garbled output).
    bool allZero = true;
    const uint32_t* pixels = static_cast<const uint32_t*>(bitmap.getPixels());
    for (int i = 0; i < bitmap.width() * bitmap.height() && allZero; ++i) {
        if (pixels[i] != 0) {
            allZero = false;
        }
    }
    REPORTER_ASSERT(r, !allZero, "Color-transformed decode produced all-zero pixels");
}

// ========== Encoder Tests ==========

#if defined(SK_CODEC_ENCODES_JPEG_WITH_RUST)

// Encode smoke test: decode a JPEG, re-encode it, verify the output is
// non-empty, re-decodable, and preserves dimensions.
DEF_TEST(RustJpegEncode_smoke_test, r) {
    struct TestCase {
        const char* path;
        int expectedWidth;
        int expectedHeight;
        int quality;
    };

    static const TestCase kTestCases[] = {
        {"images/mandrill_512_q075.jpg", 512, 512, 85},
        {"images/dog.jpg",              180, 180, 90},
    };

    for (const auto& tc : kTestCases) {
        skiatest::ReporterContext ctx(r, tc.path);

        sk_sp<SkData> data = GetResourceAsData(tc.path);
        if (!data) {
            ERRORF(r, "Missing resource: %s", tc.path);
            continue;
        }

        SkCodec::Result result;
        std::unique_ptr<SkCodec> codec =
                SkJpegRustDecoder::Decode(SkMemoryStream::Make(data), &result);
        REPORTER_ASSERT_SUCCESSFUL_CODEC_RESULT(r, result);
        if (!codec) continue;

        auto [image, decodeResult] = codec->getImage();
        REPORTER_ASSERT_SUCCESSFUL_CODEC_RESULT(r, decodeResult);
        REPORTER_ASSERT(r, image);
        if (!image) continue;

        SkPixmap src;
        REPORTER_ASSERT(r, image->peekPixels(&src));

        SkDynamicMemoryWStream dst;
        SkJpegEncoder::Options options;
        options.fQuality = tc.quality;
        REPORTER_ASSERT(r, SkJpegRustEncoder::Encode(&dst, src, options));
        REPORTER_ASSERT(r, dst.bytesWritten() > 0);

        sk_sp<SkData> encoded = dst.detachAsData();
        REPORTER_ASSERT(r, SkJpegRustDecoder::IsJpeg(encoded->data(), encoded->size()));

        SkCodec::Result reDecodeResult;
        std::unique_ptr<SkCodec> reCodec =
                SkJpegRustDecoder::Decode(SkMemoryStream::Make(encoded), &reDecodeResult);
        REPORTER_ASSERT_SUCCESSFUL_CODEC_RESULT(r, reDecodeResult);
        REPORTER_ASSERT(r, reCodec);
        if (!reCodec) continue;

        auto [reImage, reImageResult] = reCodec->getImage();
        REPORTER_ASSERT_SUCCESSFUL_CODEC_RESULT(r, reImageResult);
        REPORTER_ASSERT(r, reImage);
        if (!reImage) continue;
        REPORTER_ASSERT(r, reImage->width() == tc.expectedWidth);
        REPORTER_ASSERT(r, reImage->height() == tc.expectedHeight);
    }
}

DEF_TEST(RustJpegEncode_quality_affects_size, r) {
    SkBitmap bitmap;
    if (!ToolUtils::GetResourceAsBitmap("images/mandrill_512_q075.jpg", &bitmap)) {
        ERRORF(r, "Failed to decode images/mandrill_512_q075.jpg");
        return;
    }

    SkPixmap src;
    REPORTER_ASSERT(r, bitmap.peekPixels(&src));

    SkDynamicMemoryWStream dstLow, dstHigh;
    SkJpegEncoder::Options lowOptions, highOptions;
    lowOptions.fQuality = 10;
    highOptions.fQuality = 95;

    REPORTER_ASSERT(r, SkJpegRustEncoder::Encode(&dstLow, src, lowOptions));
    REPORTER_ASSERT(r, SkJpegRustEncoder::Encode(&dstHigh, src, highOptions));

    REPORTER_ASSERT(r,
                    dstLow.bytesWritten() < dstHigh.bytesWritten(),
                    "Low quality (%zu bytes) should be smaller than high quality (%zu bytes)",
                    dstLow.bytesWritten(),
                    dstHigh.bytesWritten());
}

// Table-based test for encoding different pixel formats. Verifies that RGB,
// RGBA (alpha stripped or blended), BGRA, and grayscale all round-trip through
// the encoder without crashing and produce non-empty output.
DEF_TEST(RustJpegEncode_color_formats, r) {
    struct TestCase {
        const char* description;
        SkColorType colorType;
        SkAlphaType alphaType;
        SkColor fillColor;
        SkJpegEncoder::AlphaOption alphaOption;
    };

    static const TestCase kTestCases[] = {
        {"grayscale",
         kGray_8_SkColorType,   kOpaque_SkAlphaType,   SK_ColorGRAY,
         SkJpegEncoder::AlphaOption::kIgnore},
        {"RGBA ignore alpha",
         kRGBA_8888_SkColorType, kUnpremul_SkAlphaType, 0x80FF0000,
         SkJpegEncoder::AlphaOption::kIgnore},
        {"RGBA blend on black",
         kRGBA_8888_SkColorType, kUnpremul_SkAlphaType, 0x8000FF00,
         SkJpegEncoder::AlphaOption::kBlendOnBlack},
        {"BGRA",
         kBGRA_8888_SkColorType, kUnpremul_SkAlphaType, SK_ColorBLUE,
         SkJpegEncoder::AlphaOption::kIgnore},
        {"RGB_888x (4-byte opaque)",
         kRGB_888x_SkColorType, kOpaque_SkAlphaType, SK_ColorRED,
         SkJpegEncoder::AlphaOption::kIgnore},
    };

    for (const auto& tc : kTestCases) {
        skiatest::ReporterContext ctx(r, tc.description);

        SkImageInfo info = SkImageInfo::Make(64, 64, tc.colorType, tc.alphaType);
        SkBitmap bitmap;
        REPORTER_ASSERT(r, bitmap.tryAllocPixels(info));
        bitmap.eraseColor(tc.fillColor);

        SkPixmap src;
        REPORTER_ASSERT(r, bitmap.peekPixels(&src));

        SkDynamicMemoryWStream dst;
        SkJpegEncoder::Options options;
        options.fQuality = 85;
        options.fAlphaOption = tc.alphaOption;
        REPORTER_ASSERT(r, SkJpegRustEncoder::Encode(&dst, src, options));
        REPORTER_ASSERT(r, dst.bytesWritten() > 0);
    }
}

#endif  // defined(SK_CODEC_ENCODES_JPEG_WITH_RUST)

// ========== Gainmap / MPF Tests ==========

#if defined(SK_CODEC_DECODES_JPEG_GAINMAPS)

// Verify the Rust codec's gainmap pipeline: Rust segment scanning → MPF parsing
// → C++ metadata interpretation → gainmap codec extraction.  This exercises the
// ~190-line onGetGainmapInfo code path that is otherwise untested.
DEF_TEST(RustJpeg_gainmap_extraction, r) {
    // gainmap_iso21496_1.jpg is a synthetic test image with an ISO 21496-1
    // gainmap embedded via MPF.  The C++ codec's JpegGainmapTest suite already
    // validates the detailed SkGainmapInfo fields, so here we only verify that
    // the Rust codec successfully extracts the gainmap and can decode it.
    static const char* kGainmapImages[] = {
        "images/gainmap_iso21496_1.jpg",
        "images/gainmap_iso21496_1_adobe_gcontainer.jpg",
    };

    for (const char* path : kGainmapImages) {
        skiatest::ReporterContext ctx(r, path);
        sk_sp<SkData> data = GetResourceAsData(path);
        if (!data) {
            ERRORF(r, "Missing resource: %s", path);
            continue;
        }

        SkCodec::Result result;
        std::unique_ptr<SkCodec> codec =
                SkJpegRustDecoder::Decode(SkMemoryStream::Make(data), &result);
        REPORTER_ASSERT(r, codec);
        if (!codec) continue;

        auto androidCodec = SkAndroidCodec::MakeFromCodec(std::move(codec));
        REPORTER_ASSERT(r, androidCodec);
        if (!androidCodec) continue;

        SkGainmapInfo gainmapInfo;
        std::unique_ptr<SkAndroidCodec> gainmapCodec;
        bool hasGainmap = androidCodec->getGainmapAndroidCodec(&gainmapInfo, &gainmapCodec);
        REPORTER_ASSERT(r, hasGainmap, "Expected gainmap in %s", path);
        if (!hasGainmap) continue;

        REPORTER_ASSERT(r, gainmapCodec);
        // The gainmap image should have valid non-zero dimensions.
        REPORTER_ASSERT(r, gainmapCodec->getInfo().width() > 0);
        REPORTER_ASSERT(r, gainmapCodec->getInfo().height() > 0);

        // Actually decode the gainmap to verify the full pipeline.
        SkBitmap gainmapBitmap;
        gainmapBitmap.allocPixels(gainmapCodec->getInfo());
        REPORTER_ASSERT(r,
                        SkCodec::kSuccess ==
                                gainmapCodec->getAndroidPixels(gainmapBitmap.info(),
                                                               gainmapBitmap.getPixels(),
                                                               gainmapBitmap.rowBytes()));
    }
}

#endif  // defined(SK_CODEC_DECODES_JPEG_GAINMAPS)

// ========== Rust vs C++ JPEG Comparison Tests ==========

#if defined(SK_CODEC_DECODES_JPEG)

// Decode the same JPEG with both the Rust decoder and the C++ libjpeg-turbo
// decoder, then compare the outputs pixel-by-pixel within a tolerance.
// JPEG decoding is inherently lossy and different DCT implementations may
// produce slightly different values; a tolerance of ~8/255 per channel covers
// the typical inter-implementation variance.
static void compare_rust_vs_cpp_jpeg(skiatest::Reporter* r,
                                     const char* path,
                                     float tolerancePerChannel = 8.0f / 255.0f) {
    skiatest::ReporterContext ctx(r, path);
    sk_sp<SkData> data = GetResourceAsData(path);
    if (!data) {
        ERRORF(r, "Missing resource: %s", path);
        return;
    }

    SkCodec::Result rustResult;
    std::unique_ptr<SkCodec> rustCodec =
            SkJpegRustDecoder::Decode(SkMemoryStream::Make(data), &rustResult);
    if (!rustCodec) {
        // Some images may not be supported by the Rust decoder.
        return;
    }
    REPORTER_ASSERT_SUCCESSFUL_CODEC_RESULT(r, rustResult);

    SkCodec::Result cppResult;
    std::unique_ptr<SkCodec> cppCodec =
            SkJpegDecoder::Decode(SkMemoryStream::Make(data), &cppResult);
    if (!cppCodec) {
        return;
    }
    REPORTER_ASSERT_SUCCESSFUL_CODEC_RESULT(r, cppResult);

    auto [rustImage, rustDecodeResult] = rustCodec->getImage();
    REPORTER_ASSERT_SUCCESSFUL_CODEC_RESULT(r, rustDecodeResult);
    REPORTER_ASSERT(r, rustImage);

    auto [cppImage, cppDecodeResult] = cppCodec->getImage();
    REPORTER_ASSERT_SUCCESSFUL_CODEC_RESULT(r, cppDecodeResult);
    REPORTER_ASSERT(r, cppImage);

    if (!rustImage || !cppImage) {
        return;
    }

    REPORTER_ASSERT(r,
                    rustImage->dimensions() == cppImage->dimensions(),
                    "Rust dims %dx%d != C++ dims %dx%d",
                    rustImage->width(),
                    rustImage->height(),
                    cppImage->width(),
                    cppImage->height());
    if (rustImage->dimensions() != cppImage->dimensions()) {
        return;
    }

    SkBitmap rustBitmap, cppBitmap;
    REPORTER_ASSERT(r, rustBitmap.tryAllocPixels(rustImage->imageInfo()));
    REPORTER_ASSERT(r, cppBitmap.tryAllocPixels(cppImage->imageInfo()));
    REPORTER_ASSERT(r, rustImage->readPixels(nullptr, rustBitmap.pixmap(), 0, 0));
    REPORTER_ASSERT(r, cppImage->readPixels(nullptr, cppBitmap.pixmap(), 0, 0));

    const float tols[4] = {tolerancePerChannel, tolerancePerChannel, tolerancePerChannel, 0.0f};
    auto error = std::function<ComparePixmapsErrorReporter>(
            [&](int x, int y, const float diffs[4]) {
                ERRORF(r,
                       "%s: pixel differs at (%d, %d): diffs=(%f, %f, %f, %f)",
                       path,
                       x,
                       y,
                       diffs[0],
                       diffs[1],
                       diffs[2],
                       diffs[3]);
            });
    ComparePixels(rustBitmap.pixmap(), cppBitmap.pixmap(), tols, error);
}

// Table-based test comparing Rust and C++ JPEG decoders across many images.
// Each entry specifies a description, path, and optional per-channel tolerance
// (default 8/255). Unsupported formats are gracefully skipped when the Rust
// decoder returns nullptr.
DEF_TEST(RustJpeg_vs_CppJpeg, r) {
    struct TestCase {
        const char* description;
        const char* path;
        float tolerance;
    };

    static constexpr float kDefaultTol = 8.0f / 255.0f;

    static const TestCase kTestCases[] = {
        {"color wheel (baseline RGB)",    "images/color_wheel.jpg",       kDefaultTol},
        {"dog (180x180, RGB)",            "images/dog.jpg",               kDefaultTol},
        {"ducky",                         "images/ducky.jpg",             kDefaultTol},
        {"mandrill 512 q075",             "images/mandrill_512_q075.jpg", kDefaultTol},
        {"mandrill h1v1 subsampling",     "images/mandrill_h1v1.jpg",     kDefaultTol},
        {"mandrill h2v1 subsampling",     "images/mandrill_h2v1.jpg",     kDefaultTol},
        {"grayscale",                     "images/grayscale.jpg",         kDefaultTol},
        {"progressive (b78329453)",       "images/b78329453.jpeg",        kDefaultTol},
        // Progressive JPEGs can have slightly larger inter-implementation diffs.
        {"progressive (crbug999986)",     "images/crbug999986.jpeg",      10.0f / 255.0f},
        {"cropped mandrill",              "images/cropped_mandrill.jpg",  kDefaultTol},
        {"flutter logo",                  "images/flutter_logo.jpg",      kDefaultTol},
        {"ICC v2 GBR profile",            "images/icc-v2-gbr.jpg",        kDefaultTol},
        // randPixels has small DCT rounding differences between implementations.
        {"random pixels",                 "images/randPixels.jpg",        16.0f / 255.0f},
        // CMYK: different CMYK→RGB conversion paths (zune-jpeg vs libjpeg-turbo)
        // may produce slightly larger per-channel differences.
        {"CMYK",                              "images/CMYK.jpg",              16.0f / 255.0f},
    };

    for (const TestCase& tc : kTestCases) {
        compare_rust_vs_cpp_jpeg(r, tc.path, tc.tolerance);
    }
}

// Compare EXIF orientation reported by Rust and C++ JPEG decoders for the same
// image.  Both codecs ultimately use SkExif::Parse, so the values must match
// exactly.
DEF_TEST(RustJpeg_vs_CppJpeg_Orientation, r) {
    static const char* kImages[] = {
        "images/orientation/1_444.jpg",
        "images/orientation/2_444.jpg",
        "images/orientation/3_444.jpg",
        "images/orientation/4_444.jpg",
        "images/orientation/5_444.jpg",
        "images/orientation/6_444.jpg",
        "images/orientation/7_444.jpg",
        "images/orientation/8_444.jpg",
        "images/exif-orientation-2-ur.jpg",
        "images/orientation/exif.jpg",     // large EXIF block
        "images/orientation/subifd.jpg",   // EXIF with sub-IFD structure
        "images/ducky.jpg",
        "images/color_wheel.jpg",         // no EXIF → default orientation
    };

    for (const char* path : kImages) {
        skiatest::ReporterContext ctx(r, path);
        sk_sp<SkData> data = GetResourceAsData(path);
        if (!data) {
            ERRORF(r, "Missing resource: %s", path);
            continue;
        }

        SkCodec::Result rustResult;
        auto rustCodec = SkJpegRustDecoder::Decode(SkMemoryStream::Make(data), &rustResult);
        if (!rustCodec) continue;

        SkCodec::Result cppResult;
        auto cppCodec = SkJpegDecoder::Decode(SkMemoryStream::Make(data), &cppResult);
        if (!cppCodec) continue;

        REPORTER_ASSERT(r,
                        rustCodec->getOrigin() == cppCodec->getOrigin(),
                        "%s: Rust orientation %d != C++ orientation %d",
                        path,
                        static_cast<int>(rustCodec->getOrigin()),
                        static_cast<int>(cppCodec->getOrigin()));
    }
}

// Compare ICC profile data reported by Rust and C++ JPEG decoders.  Both
// profiles should round-trip to the same parsed representation.
DEF_TEST(RustJpeg_vs_CppJpeg_ICCProfile, r) {
    static const char* kImages[] = {
        "images/icc-v2-gbr.jpg",       // ICC v2 with GBR channel order
        "images/ducky.jpg",            // large ICC profile
        "images/color_wheel.jpg",      // no ICC → both should be nullptr
        "images/grayscale.jpg",        // no ICC
    };

    for (const char* path : kImages) {
        skiatest::ReporterContext ctx(r, path);
        sk_sp<SkData> data = GetResourceAsData(path);
        if (!data) {
            ERRORF(r, "Missing resource: %s", path);
            continue;
        }

        SkCodec::Result rustResult;
        auto rustCodec = SkJpegRustDecoder::Decode(SkMemoryStream::Make(data), &rustResult);
        if (!rustCodec) continue;

        SkCodec::Result cppResult;
        auto cppCodec = SkJpegDecoder::Decode(SkMemoryStream::Make(data), &cppResult);
        if (!cppCodec) continue;

        const skcms_ICCProfile* rustICC = rustCodec->getICCProfile();
        const skcms_ICCProfile* cppICC = cppCodec->getICCProfile();

        // Both should agree on presence.
        if (!rustICC && !cppICC) {
            continue;  // both null — OK
        }
        if (!rustICC || !cppICC) {
            ERRORF(r, "%s: ICC presence mismatch (Rust %s, C++ %s)",
                   path,
                   rustICC ? "present" : "absent",
                   cppICC ? "present" : "absent");
            continue;
        }

        // Compare data class and color space.
        REPORTER_ASSERT(r,
                        rustICC->data_color_space == cppICC->data_color_space,
                        "%s: ICC data_color_space mismatch", path);

        // Compare the TRC (transfer function) for each channel.
        for (int ch = 0; ch < 3; ++ch) {
            REPORTER_ASSERT(r,
                            skcms_TransferFunction_isSRGBish(&rustICC->trc[ch].parametric) ==
                            skcms_TransferFunction_isSRGBish(&cppICC->trc[ch].parametric),
                            "%s: ICC TRC[%d] sRGB-ish mismatch", path, ch);
        }

        // Compare the 3x3 toXYZD50 matrix.
        for (int row = 0; row < 3; ++row) {
            for (int col = 0; col < 3; ++col) {
                float rustVal = rustICC->toXYZD50.vals[row][col];
                float cppVal = cppICC->toXYZD50.vals[row][col];
                float diff = std::abs(rustVal - cppVal);
                REPORTER_ASSERT(r,
                                diff < 1e-5f,
                                "%s: ICC toXYZD50[%d][%d] differs: Rust=%f C++=%f",
                                path, row, col, rustVal, cppVal);
            }
        }
    }
}

#endif  // defined(SK_CODEC_DECODES_JPEG)
