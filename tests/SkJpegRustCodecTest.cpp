/*
 * Copyright 2026 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "experimental/rust_jpeg/decoder/SkJpegRustDecoder.h"
#include "experimental/rust_jpeg/decoder/impl/SkJpegRustCodec.h"

#include <algorithm>
#include <array>
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
#include "include/core/SkYUVAInfo.h"
#include "include/core/SkYUVAPixmaps.h"
#include "include/private/SkFeatures.h"
#include "rust/common/SkStreamAdapter.h"
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

#define REPORTER_ASSERT_SUCCESSFUL_CODEC_RESULT(r, actualResult) \
    REPORTER_ASSERT(r,                                           \
                    (actualResult) == SkCodec::kSuccess,         \
                    "actualResult=\"%s\" != kSuccess",           \
                    SkCodec::ResultToString(actualResult))

// Helper wrapping a call to `SkJpegRustDecoder::Decode`.
static std::unique_ptr<SkCodec> decode_rust_jpeg(skiatest::Reporter* r, const char* path) {
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

static std::unique_ptr<SkCodec> decode_rust_jpeg_data(sk_sp<SkData> data,
                                                      SkCodec::Result* result = nullptr) {
    return SkJpegRustDecoder::Decode(SkMemoryStream::Make(std::move(data)), result);
}

static rust::Box<rust_jpeg::Reader> make_rust_jpeg_reader_for_test(SkStream* stream,
                                                                   uint64_t streamLength) {
    auto reader = rust_jpeg::new_reader(std::make_unique<rust::stream::SkStreamAdapter>(stream),
                                        streamLength);
    SkASSERT_RELEASE(reader->initialization_result() == rust_jpeg::DecodingResult::Success);
    return reader;
}

struct RustJpegReaderForTest {
    explicit RustJpegReaderForTest(std::unique_ptr<SkStream> stream, uint64_t streamLength)
            : fStream(std::move(stream))
            , fReader(make_rust_jpeg_reader_for_test(fStream.get(), streamLength)) {}

    std::unique_ptr<SkStream> fStream;
    rust::Box<rust_jpeg::Reader> fReader;
};

class SkJpegRustCodecTestPriv {
public:
    static bool SupportsStreamingFullDecode(const SkJpegRustCodec& codec) {
        return codec.supportsStreamingFullDecodeForTesting();
    }
    static uint64_t FullImageBufferCapacity(const SkJpegRustCodec& codec) {
        return codec.fullImageBufferCapacityForTesting();
    }
    static size_t ScanlineBufferSize(const SkJpegRustCodec& codec) {
        return codec.scanlineBufferSizeForTesting();
    }
    static int LastFullDecodeRows(const SkJpegRustCodec& codec) {
        return codec.lastFullDecodeRowsForTesting();
    }
};

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
            {"images/color_wheel.jpg", SkISize::Make(128, 128)},
            {"images/grayscale.jpg", SkISize::Make(128, 128)},
            {"images/dog.jpg", SkISize::Make(180, 180)},
            {"images/b78329453.jpeg", SkISize::Make(635, 760)},
            {"images/mandrill_512_q075.jpg", SkISize::Make(512, 512)},
            {"images/ducky.jpg", SkISize::Make(489, 537)},
            {"images/mandrill_h1v1.jpg", SkISize::Make(512, 512)},
            {"images/mandrill_h2v1.jpg", SkISize::Make(512, 512)},
            {"images/cropped_mandrill.jpg", SkISize::Make(439, 154)},
            {"images/flutter_logo.jpg", SkISize::Make(400, 400)},
            {"images/icc-v2-gbr.jpg", SkISize::Make(275, 207)},
            {"images/randPixels.jpg", SkISize::Make(8, 8)},
            {"images/crbug999986.jpeg", SkISize::Make(2000, 1500)},
            {"images/CMYK.jpg", SkISize::Make(642, 516)},
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
                        codec->dimensions().width(),
                        codec->dimensions().height(),
                        tc.expectedDimensions.width(),
                        tc.expectedDimensions.height());

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
            {"images/color_wheel.jpg", "baseline RGB 128x128", false},
            {"images/dog.jpg", "RGB 180x180", false},
            {"images/mandrill_512_q075.jpg", "RGB 512x512", false},
            {"images/b78329453.jpeg", "progressive RGB 635x760", true},
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

        REPORTER_ASSERT(r,
                        data->size() > kInitialBytes,
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
            ERRORF(r,
                   "%s: failed to create streaming codec with %zu bytes: %s",
                   testCase.description,
                   initialBytes,
                   SkCodec::ResultToString(result));
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

            REPORTER_ASSERT(r,
                            rowsDecoded >= previousRowsDecoded,
                            "%s: decoded row count regressed from %d to %d",
                            testCase.description,
                            previousRowsDecoded,
                            rowsDecoded);

            const uint8_t* pixels = static_cast<const uint8_t*>(bitmap.getPixels());
            if (!testCase.expectProgressivePreview && previousRowsDecoded > 0) {
                const size_t stableBytes =
                        static_cast<size_t>(previousRowsDecoded) * bitmap.rowBytes();
                REPORTER_ASSERT(r,
                                memcmp(pixels, previousPixels.data(), stableBytes) == 0,
                                "%s: previously reported stable rows changed",
                                testCase.description);
            }

            if (result == SkCodec::kSuccess) {
                break;
            }

            REPORTER_ASSERT(r,
                            result == SkCodec::kIncompleteInput,
                            "%s: partial input should return kIncompleteInput, got %s",
                            testCase.description,
                            SkCodec::ResultToString(result));
            if (result != SkCodec::kIncompleteInput) {
                break;
            }
            sawIncompleteInput = true;

            const bool pixelsChanged = std::any_of(pixels,
                                                   pixels + bitmap.computeByteSize(),
                                                   [](uint8_t value) { return value != 0xAB; });
            if (testCase.expectProgressivePreview) {
                if (memcmp(pixels, previousPixels.data(), bitmap.computeByteSize()) != 0) {
                    progressivePreviewCount++;
                    sawPartialOutput = true;
                    REPORTER_ASSERT(r,
                                    rowsDecoded > 0,
                                    "%s: preview pixels were not reported as initialized",
                                    testCase.description);
                }
            } else if (rowsDecoded > 0) {
                sawPartialOutput = true;
                REPORTER_ASSERT(r,
                                pixelsChanged,
                                "%s: reported stable rows were not written",
                                testCase.description);
            }

            for (int y = 0; y < rowsDecoded; ++y) {
                for (int x = 0; x < info.width(); ++x) {
                    REPORTER_ASSERT(r,
                                    SkColorGetA(bitmap.getColor(x, y)) == SK_AlphaOPAQUE,
                                    "%s: initialized pixel (%d,%d) is not opaque",
                                    testCase.description,
                                    x,
                                    y);
                }
            }

            previousRowsDecoded = rowsDecoded;
            memcpy(previousPixels.data(), pixels, bitmap.computeByteSize());

            if (!checkedNoNewDataRetry && sawPartialOutput) {
                int retryRowsDecoded = -1;
                const SkCodec::Result retryResult = codec->incrementalDecode(&retryRowsDecoded);
                REPORTER_ASSERT(r,
                                retryResult == SkCodec::kIncompleteInput,
                                "%s: retry without new data returned %s",
                                testCase.description,
                                SkCodec::ResultToString(retryResult));
                REPORTER_ASSERT(r,
                                retryRowsDecoded == rowsDecoded,
                                "%s: retry changed initialized rows from %d to %d",
                                testCase.description,
                                rowsDecoded,
                                retryRowsDecoded);
                REPORTER_ASSERT(
                        r,
                        memcmp(pixels, previousPixels.data(), bitmap.computeByteSize()) == 0,
                        "%s: retry without new data changed output",
                        testCase.description);
                checkedNoNewDataRetry = true;
            }

            if (streamPtr->isAllDataReceived()) {
                ERRORF(r,
                       "%s: still incomplete after all data was available",
                       testCase.description);
                break;
            }

            // Do not call startIncrementalDecode() again. Each subsequent call
            // must resume the existing Rust/zune decoder after one more chunk
            // becomes available.
            streamPtr->addNewData(kChunkSize);
        }

        REPORTER_ASSERT(r,
                        iterations < kMaxIterations,
                        "%s: decode loop exceeded max iterations",
                        testCase.description);
        REPORTER_ASSERT(r,
                        sawIncompleteInput,
                        "%s: decode never observed incomplete input",
                        testCase.description);
        REPORTER_ASSERT(r,
                        sawPartialOutput,
                        "%s: no partial output was exposed before all input arrived",
                        testCase.description);
        REPORTER_ASSERT(r,
                        checkedNoNewDataRetry,
                        "%s: retry without new input was not exercised",
                        testCase.description);
        if (testCase.expectProgressivePreview) {
            REPORTER_ASSERT(r,
                            progressivePreviewCount >= 2,
                            "%s: expected multiple progressive preview replacements, got %d",
                            testCase.description,
                            progressivePreviewCount);
        }
        REPORTER_ASSERT(r,
                        result == SkCodec::kSuccess,
                        "%s: resumed decode expected kSuccess, got %s",
                        testCase.description,
                        SkCodec::ResultToString(result));
        REPORTER_ASSERT(r,
                        rowsDecoded == info.height(),
                        "%s: resumed decode reported %d of %d rows",
                        testCase.description,
                        rowsDecoded,
                        info.height());

        // Verify dimensions match the reference.
        REPORTER_ASSERT(r,
                        bitmap.dimensions() == reference.dimensions(),
                        "%s: incremental dims %dx%d != reference %dx%d",
                        testCase.description,
                        bitmap.width(),
                        bitmap.height(),
                        reference.width(),
                        reference.height());

        // Both paths used the same Rust decoder, so pixel output should be
        // identical (zero tolerance).
        if (bitmap.dimensions() == reference.dimensions()) {
            const float tols[4] = {0, 0, 0, 0};
            auto error = std::function<ComparePixmapsErrorReporter>(
                    [&](int x, int y, const float diffs[4]) {
                        ERRORF(r,
                               "%s: pixel differs at (%d,%d): diffs=(%f,%f,%f,%f)",
                               testCase.description,
                               x,
                               y,
                               diffs[0],
                               diffs[1],
                               diffs[2],
                               diffs[3]);
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
    const SkCodec::Result result =
            codec->startIncrementalDecode(info, bitmap.getPixels(), bitmap.rowBytes(), &options);
    REPORTER_ASSERT(r,
                    result == SkCodec::kInvalidParameters,
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
    const SkCodec::Result result =
            codec->startIncrementalDecode(info, bitmap.getPixels(), info.minRowBytes() - 1);
    REPORTER_ASSERT(r,
                    result == SkCodec::kInvalidParameters,
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

        SkBitmap scanlines;
        scanlines.allocPixels(info);
        REPORTER_ASSERT_SUCCESSFUL_CODEC_RESULT(r, codec->startScanlineDecode(info));
        REPORTER_ASSERT(
            r,
            codec->getScanlines(scanlines.getPixels(), info.height(), scanlines.rowBytes()) ==
                info.height());
        REPORTER_ASSERT(r,
                memcmp(firstIncremental.getPixels(),
                   scanlines.getPixels(),
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
            r, referenceCodec->getPixels(dstInfo, reference.getPixels(), reference.rowBytes()));

    auto haltingStream = std::make_unique<HaltingStream>(data, kInitialBytes);
    HaltingStream* streamPtr = haltingStream.get();
    SkCodec::Result result;
    std::unique_ptr<SkCodec> codec = SkJpegRustDecoder::Decode(std::move(haltingStream), &result);
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
    auto reportError =
            std::function<ComparePixmapsErrorReporter>([&](int x, int y, const float diffs[4]) {
                ERRORF(r,
                       "incremental transformed pixel differs at (%d,%d): "
                       "diffs=(%f,%f,%f,%f)",
                       x,
                       y,
                       diffs[0],
                       diffs[1],
                       diffs[2],
                       diffs[3]);
            });
    ComparePixels(reference.pixmap(), incremental.pixmap(), tolerances, reportError);

    const size_t pixelBytes = dstInfo.minRowBytes();
    for (int y = 0; y + 1 < dstInfo.height(); ++y) {
        const uint8_t* row = static_cast<const uint8_t*>(incremental.getAddr(0, y));
        REPORTER_ASSERT(r,
                        std::all_of(row + pixelBytes,
                                    row + paddedRowBytes,
                                    [](uint8_t value) { return value == 0xAB; }),
                        "incremental decode modified row %d padding",
                        y);
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

DEF_TEST(RustJpegCodec_short_input_classification, r) {
    const auto decode = [](sk_sp<SkData> data, SkCodec::Result* result) {
        return SkJpegRustDecoder::Decode(SkMemoryStream::Make(std::move(data)), result);
    };

    SkCodec::Result result;
    std::unique_ptr<SkCodec> codec = decode(SkData::MakeEmpty(), &result);
    REPORTER_ASSERT(r, !codec);
    REPORTER_ASSERT(r, result == SkCodec::kIncompleteInput);

    constexpr uint8_t kPartialSignature[] = {0xFF};
    codec = decode(SkData::MakeWithCopy(kPartialSignature, sizeof(kPartialSignature)), &result);
    REPORTER_ASSERT(r, !codec);
    REPORTER_ASSERT(r, result == SkCodec::kIncompleteInput);

    constexpr uint8_t kInvalidSignature[] = {0x00};
    codec = decode(SkData::MakeWithCopy(kInvalidSignature, sizeof(kInvalidSignature)), &result);
    REPORTER_ASSERT(r, !codec);
    REPORTER_ASSERT(r, result == SkCodec::kErrorInInput);

    constexpr uint8_t kStartOfImage[] = {0xFF, 0xD8};
    codec = decode(SkData::MakeWithCopy(kStartOfImage, sizeof(kStartOfImage)), &result);
    REPORTER_ASSERT(r, !codec);
    REPORTER_ASSERT(r, result == SkCodec::kIncompleteInput);

    sk_sp<SkData> data = GetResourceAsData("images/color_wheel.jpg");
    REPORTER_ASSERT(r, data);
    if (!data) {
        return;
    }

    codec = decode(std::move(data), &result);
    REPORTER_ASSERT_SUCCESSFUL_CODEC_RESULT(r, result);
    REPORTER_ASSERT(r, codec);
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

#if defined(SK_CODEC_DECODES_JPEG)

DEF_TEST(RustJpegCodec_RawYUVAParity, r) {
    struct TestCase {
        const char* path;
        SkYUVAInfo::Subsampling subsampling;
    };
    const TestCase testCases[] = {
            {"images/orientation/1_444.jpg", SkYUVAInfo::Subsampling::k444},
            {"images/orientation/1_422.jpg", SkYUVAInfo::Subsampling::k422},
            {"images/orientation/1_420.jpg", SkYUVAInfo::Subsampling::k420},
            {"images/orientation/1_440.jpg", SkYUVAInfo::Subsampling::k440},
            {"images/orientation/1_411.jpg", SkYUVAInfo::Subsampling::k411},
            {"images/orientation/1_410.jpg", SkYUVAInfo::Subsampling::k410},
            {"images/cropped_mandrill.jpg", SkYUVAInfo::Subsampling::k420},
            {"images/brickwork-texture.jpg", SkYUVAInfo::Subsampling::k444},
            {"images/non_interleaved_420_64x64.jpg", SkYUVAInfo::Subsampling::k420},
            {"images/four-colors-limited-range-420-8bpc.jpg", SkYUVAInfo::Subsampling::k420},
            {"images/b78329453.jpeg", SkYUVAInfo::Subsampling::k420},
    };
    constexpr auto kAllTypes = SkYUVAPixmapInfo::SupportedDataTypes::All();
    int maximumDeviation = 0;

    for (const TestCase& testCase : testCases) {
        skiatest::ReporterContext context(r, testCase.path);
        sk_sp<SkData> data = GetResourceAsData(testCase.path);
        REPORTER_ASSERT(r, data);
        if (!data) {
            continue;
        }

        std::unique_ptr<SkCodec> rustCodec = decode_rust_jpeg_data(data);
        std::unique_ptr<SkCodec> jpegCodec =
                SkJpegDecoder::Decode(SkMemoryStream::Make(data), nullptr);
        REPORTER_ASSERT(r, rustCodec && jpegCodec);
        if (!rustCodec || !jpegCodec) {
            continue;
        }

        SkYUVAPixmapInfo rustInfo;
        SkYUVAPixmapInfo jpegInfo;
        REPORTER_ASSERT(r, rustCodec->queryYUVAInfo(kAllTypes, &rustInfo));
        REPORTER_ASSERT(r, jpegCodec->queryYUVAInfo(kAllTypes, &jpegInfo));
        if (!rustInfo.isValid() || !jpegInfo.isValid()) {
            continue;
        }
        REPORTER_ASSERT(r, rustInfo.yuvaInfo() == jpegInfo.yuvaInfo());
        REPORTER_ASSERT(r, rustInfo.yuvaInfo().subsampling() == testCase.subsampling);
        REPORTER_ASSERT(r, rustInfo.yuvaInfo().yuvColorSpace() == kJPEG_Full_SkYUVColorSpace);
        REPORTER_ASSERT(r, rustInfo.yuvaInfo().origin() == rustCodec->getOrigin());

        std::array<std::vector<uint8_t>, SkYUVAPixmaps::kMaxPlanes> storage;
        std::array<SkPixmap, SkYUVAPixmaps::kMaxPlanes> pixmaps;
        for (int plane = 0; plane < rustInfo.numPlanes(); ++plane) {
            const SkImageInfo& info = rustInfo.planeInfo(plane);
            const size_t rowBytes = static_cast<size_t>(info.width()) + 13;
            const size_t exactSize = (static_cast<size_t>(info.height()) - 1) * rowBytes +
                                     static_cast<size_t>(info.width());
            storage[plane].assign(exactSize, 0xCD);
            pixmaps[plane] = SkPixmap(info, storage[plane].data(), rowBytes);
        }
        SkYUVAPixmaps rustPlanes =
                SkYUVAPixmaps::FromExternalPixmaps(rustInfo.yuvaInfo(), pixmaps.data());
        SkYUVAPixmaps jpegPlanes = SkYUVAPixmaps::Allocate(jpegInfo);
        REPORTER_ASSERT(r, rustPlanes.isValid() && jpegPlanes.isValid());
        REPORTER_ASSERT_SUCCESSFUL_CODEC_RESULT(r, rustCodec->getYUVAPlanes(rustPlanes));
        REPORTER_ASSERT_SUCCESSFUL_CODEC_RESULT(r, jpegCodec->getYUVAPlanes(jpegPlanes));

        if (&testCase == &testCases[0] ||
            !strcmp(testCase.path, "images/four-colors-limited-range-420-8bpc.jpg")) {
            SkYUVAPixmaps replay = SkYUVAPixmaps::Allocate(rustInfo);
            REPORTER_ASSERT_SUCCESSFUL_CODEC_RESULT(r, rustCodec->getYUVAPlanes(replay));
            for (int plane = 0; plane < rustPlanes.numPlanes(); ++plane) {
                for (int y = 0; y < rustPlanes.plane(plane).height(); ++y) {
                    REPORTER_ASSERT(
                            r,
                            memcmp(rustPlanes.plane(plane).addr(0, y),
                                   replay.plane(plane).addr(0, y),
                                   static_cast<size_t>(rustPlanes.plane(plane).width())) == 0);
                }
            }
        }

        for (int plane = 0; plane < rustPlanes.numPlanes(); ++plane) {
            const SkPixmap& actual = rustPlanes.plane(plane);
            const SkPixmap& expected = jpegPlanes.plane(plane);
            REPORTER_ASSERT(r, actual.dimensions() == expected.dimensions());
            for (int y = 0; y < actual.height(); ++y) {
                const auto* actualRow = static_cast<const uint8_t*>(actual.addr(0, y));
                const auto* expectedRow = static_cast<const uint8_t*>(expected.addr(0, y));
                for (int x = 0; x < actual.width(); ++x) {
                    const int deviation = std::abs(static_cast<int>(actualRow[x]) -
                                                   static_cast<int>(expectedRow[x]));
                    maximumDeviation = std::max(maximumDeviation, deviation);
                    REPORTER_ASSERT(r,
                                    deviation <= 2,
                                    "%s plane %d sample (%d,%d) deviation %d exceeds 2",
                                    testCase.path,
                                    plane,
                                    x,
                                    y,
                                    deviation);
                }
                if (y + 1 < actual.height()) {
                    REPORTER_ASSERT(r,
                                    std::all_of(actualRow + actual.width(),
                                                actualRow + actual.rowBytes(),
                                                [](uint8_t value) { return value == 0xCD; }),
                                    "%s plane %d row %d padding was modified",
                                    testCase.path,
                                    plane,
                                    y);
                }
            }
        }
    }
    INFOF(r, "Rust JPEG raw-plane maximum libjpeg-turbo deviation: %d", maximumDeviation);

    for (const char* path : {"images/grayscale.jpg", "images/CMYK.jpg"}) {
        std::unique_ptr<SkCodec> codec = decode_rust_jpeg(r, path);
        SkYUVAPixmapInfo info;
        REPORTER_ASSERT(r, codec && !codec->queryYUVAInfo(kAllTypes, &info));
    }

    sk_sp<SkData> source = GetResourceAsData("images/orientation/1_444.jpg");
    REPORTER_ASSERT(r, source);
    if (source) {
        auto makeMutatedSOF = [&](size_t componentOffset, uint8_t value) {
            sk_sp<SkData> mutated = SkData::MakeUninitialized(source->size());
            memcpy(mutated->writable_data(), source->data(), source->size());
            auto* bytes = static_cast<uint8_t*>(mutated->writable_data());
            for (size_t index = 0; index + 16 < mutated->size(); ++index) {
                if (bytes[index] == 0xFF && bytes[index + 1] == 0xC0) {
                    bytes[index + componentOffset] = value;
                    break;
                }
            }
            if (componentOffset == 10) {
                for (size_t index = 0; index + 6 < mutated->size(); ++index) {
                    if (bytes[index] == 0xFF && bytes[index + 1] == 0xDA) {
                        bytes[index + 5] = value;
                        break;
                    }
                }
            }
            return mutated;
        };
        struct Mutation {
            const char* description;
            sk_sp<SkData> data;
        };
        Mutation mutations[] = {{"component ID", makeMutatedSOF(10, 4)},
                                {"chroma sampling", makeMutatedSOF(14, 0x21)}};
        for (Mutation& mutation : mutations) {
            skiatest::ReporterContext context(r, mutation.description);
            std::unique_ptr<SkCodec> codec = decode_rust_jpeg_data(std::move(mutation.data));
            SkYUVAPixmapInfo info;
            REPORTER_ASSERT(r, codec && !codec->queryYUVAInfo(kAllTypes, &info));
        }
    }
}

#endif

DEF_TEST(RustJpegCodec_ProgressiveRawYUVRetry, r) {
    sk_sp<SkData> data = GetResourceAsData("images/four-colors-limited-range-420-8bpc.jpg");
    REPORTER_ASSERT(r, data);
    if (!data) {
        return;
    }

    auto stream = std::make_unique<HaltingStream>(data, data->size() / 2);
    HaltingStream* streamPtr = stream.get();
    SkCodec::Result result;
    std::unique_ptr<SkCodec> codec = SkJpegRustDecoder::Decode(std::move(stream), &result);
    REPORTER_ASSERT_SUCCESSFUL_CODEC_RESULT(r, result);
    REPORTER_ASSERT(r, codec);
    if (!codec) {
        return;
    }

    constexpr auto kAllTypes = SkYUVAPixmapInfo::SupportedDataTypes::All();
    SkYUVAPixmapInfo info;
    REPORTER_ASSERT(r, codec->queryYUVAInfo(kAllTypes, &info));
    REPORTER_ASSERT(r, info.yuvaInfo().planeConfig() == SkYUVAInfo::PlaneConfig::kY_U_V);
    REPORTER_ASSERT(r, info.yuvaInfo().subsampling() == SkYUVAInfo::Subsampling::k420);

    std::array<std::vector<uint8_t>, SkYUVAPixmaps::kMaxPlanes> storage;
    std::array<SkPixmap, SkYUVAPixmaps::kMaxPlanes> pixmaps;
    for (int plane = 0; plane < info.numPlanes(); ++plane) {
        const SkImageInfo& planeInfo = info.planeInfo(plane);
        const size_t rowBytes = static_cast<size_t>(planeInfo.width()) + 13;
        storage[plane].assign(rowBytes * static_cast<size_t>(planeInfo.height()), 0xCD);
        pixmaps[plane] = SkPixmap(planeInfo, storage[plane].data(), rowBytes);
    }
    SkYUVAPixmaps planes = SkYUVAPixmaps::FromExternalPixmaps(info.yuvaInfo(), pixmaps.data());
    REPORTER_ASSERT(r, planes.isValid());

    REPORTER_ASSERT(r, codec->getYUVAPlanes(planes) == SkCodec::kIncompleteInput);
    for (int plane = 0; plane < planes.numPlanes(); ++plane) {
        REPORTER_ASSERT(
                r, std::all_of(storage[plane].begin(), storage[plane].end(), [](uint8_t value) {
                    return value == 0xCD;
                }));
    }

    streamPtr->addNewData(data->size());
    REPORTER_ASSERT_SUCCESSFUL_CODEC_RESULT(r, codec->getYUVAPlanes(planes));
    for (int plane = 0; plane < planes.numPlanes(); ++plane) {
        REPORTER_ASSERT(
                r, std::any_of(storage[plane].begin(), storage[plane].end(), [](uint8_t value) {
                    return value != 0xCD;
                }));
    }
}

DEF_TEST(RustJpegCodec_RawDirectPlaneContracts, r) {
    constexpr uint8_t kSentinel = 0xCD;
    sk_sp<SkData> data = GetResourceAsData("images/color_wheel.jpg");
    REPORTER_ASSERT(r, data);
    if (!data) {
        return;
    }

    auto makeReader = [&]() {
        return RustJpegReaderForTest(SkMemoryStream::Make(data), data->size());
    };
    auto reader = makeReader();
    REPORTER_ASSERT(r, reader.fReader->read_metadata() == rust_jpeg::DecodingResult::Success);
    REPORTER_ASSERT(r, reader.fReader->raw_component_count() == 3);

    std::array<rust_jpeg::RawPlaneInfo, 3> infos = {
            reader.fReader->raw_plane_info(0),
            reader.fReader->raw_plane_info(1),
            reader.fReader->raw_plane_info(2),
    };
    std::array<size_t, 3> strides;
    std::array<size_t, 3> rows;
    std::array<size_t, 3> required;
    for (int plane = 0; plane < 3; ++plane) {
        strides[plane] = static_cast<size_t>(infos[plane].width) + 7 + plane * 3;
        rows[plane] = std::min(static_cast<size_t>(infos[plane].height),
                               static_cast<size_t>(infos[plane].vertical_sampling_factor) * 8);
        required[plane] =
                rows[plane] == 0 ? 0 : (rows[plane] - 1) * strides[plane] + infos[plane].width;
    }

    std::array<std::vector<uint8_t>, 3> expected;
    for (int plane = 0; plane < 3; ++plane) {
        expected[plane].assign(required[plane], kSentinel);
    }
    rust_jpeg::RawOutputResult expectedResult = reader.fReader->read_raw_imcu_row(
            rust::Slice<uint8_t>(expected[0].data(), expected[0].size()),
            static_cast<uint32_t>(strides[0]),
            rust::Slice<uint8_t>(expected[1].data(), expected[1].size()),
            static_cast<uint32_t>(strides[1]),
            rust::Slice<uint8_t>(expected[2].data(), expected[2].size()),
            static_cast<uint32_t>(strides[2]));
    REPORTER_ASSERT(r, expectedResult.status == rust_jpeg::OutputStatus::Ready);
    rust_jpeg::ScanlineOutputResult conflictingStart =
            reader.fReader->start_scanline_output(rust_jpeg::PackedOutputFormat::Source);
    REPORTER_ASSERT(r, conflictingStart.status == rust_jpeg::OutputStatus::Error);
    REPORTER_ASSERT(r, conflictingStart.result == rust_jpeg::DecodingResult::ParameterError);
    reader.fReader->reset_decode_state();
    rust_jpeg::ScanlineOutputResult scanlineStart =
            reader.fReader->start_scanline_output(rust_jpeg::PackedOutputFormat::Source);
    REPORTER_ASSERT(r, scanlineStart.status == rust_jpeg::OutputStatus::Ready);
    const std::array<uint32_t, 3> expectedRows = {
            expectedResult.y_rows, expectedResult.u_rows, expectedResult.v_rows};
    for (int plane = 0; plane < 3; ++plane) {
        REPORTER_ASSERT(r, expectedRows[plane] == rows[plane]);
        for (size_t row = 0; row + 1 < rows[plane]; ++row) {
            REPORTER_ASSERT(
                    r,
                    std::all_of(expected[plane].begin() + row * strides[plane] + infos[plane].width,
                                expected[plane].begin() + (row + 1) * strides[plane],
                                [](uint8_t value) { return value == kSentinel; }));
        }
    }

    for (int shortPlane = 0; shortPlane < 3; ++shortPlane) {
        auto shortReader = makeReader();
        REPORTER_ASSERT(r,
                        shortReader.fReader->read_metadata() == rust_jpeg::DecodingResult::Success);
        std::array<std::vector<uint8_t>, 3> storage;
        for (int plane = 0; plane < 3; ++plane) {
            storage[plane].assign(required[plane] - (plane == shortPlane), kSentinel);
        }
        rust_jpeg::RawOutputResult result = shortReader.fReader->read_raw_imcu_row(
                rust::Slice<uint8_t>(storage[0].data(), storage[0].size()),
                static_cast<uint32_t>(strides[0]),
                rust::Slice<uint8_t>(storage[1].data(), storage[1].size()),
                static_cast<uint32_t>(strides[1]),
                rust::Slice<uint8_t>(storage[2].data(), storage[2].size()),
                static_cast<uint32_t>(strides[2]));
        REPORTER_ASSERT(r, result.status == rust_jpeg::OutputStatus::Error);
        for (const auto& plane : storage) {
            REPORTER_ASSERT(r, std::all_of(plane.begin(), plane.end(), [](uint8_t value) {
                                return value == kSentinel;
                            }));
        }
    }

    size_t entropyStart = 0;
    const uint8_t* bytes = data->bytes();
    for (size_t offset = 0; offset + 4 < data->size(); ++offset) {
        if (bytes[offset] == 0xFF && bytes[offset + 1] == 0xDA) {
            const size_t markerLength = static_cast<size_t>(bytes[offset + 2]) << 8 |
                                        static_cast<size_t>(bytes[offset + 3]);
            entropyStart = offset + 2 + markerLength;
            break;
        }
    }
    REPORTER_ASSERT(r, entropyStart > 0 && entropyStart < data->size());

    int suspensionsTested = 0;
    for (size_t visible = entropyStart;
         visible < std::min(data->size(), entropyStart + 128) && suspensionsTested < 3;
         ++visible) {
        auto stream = std::make_unique<HaltingStream>(data, visible);
        HaltingStream* streamPtr = stream.get();
        RustJpegReaderForTest partialReader(std::move(stream), 0);
        REPORTER_ASSERT(
                r, partialReader.fReader->read_metadata() == rust_jpeg::DecodingResult::Success);
        std::array<std::vector<uint8_t>, 3> storage;
        for (int plane = 0; plane < 3; ++plane) {
            storage[plane].assign(required[plane], kSentinel);
        }
        rust_jpeg::RawOutputResult result = partialReader.fReader->read_raw_imcu_row(
                rust::Slice<uint8_t>(storage[0].data(), storage[0].size()),
                static_cast<uint32_t>(strides[0]),
                rust::Slice<uint8_t>(storage[1].data(), storage[1].size()),
                static_cast<uint32_t>(strides[1]),
                rust::Slice<uint8_t>(storage[2].data(), storage[2].size()),
                static_cast<uint32_t>(strides[2]));
        if (result.status != rust_jpeg::OutputStatus::NeedMoreInput) {
            continue;
        }
        ++suspensionsTested;
        for (const auto& plane : storage) {
            REPORTER_ASSERT(r, std::all_of(plane.begin(), plane.end(), [](uint8_t value) {
                                return value == kSentinel;
                            }));
        }

        streamPtr->addNewData(data->size());
        result = partialReader.fReader->read_raw_imcu_row(
                rust::Slice<uint8_t>(storage[0].data(), storage[0].size()),
                static_cast<uint32_t>(strides[0]),
                rust::Slice<uint8_t>(storage[1].data(), storage[1].size()),
                static_cast<uint32_t>(strides[1]),
                rust::Slice<uint8_t>(storage[2].data(), storage[2].size()),
                static_cast<uint32_t>(strides[2]));
        REPORTER_ASSERT(r, result.status == rust_jpeg::OutputStatus::Ready);
        REPORTER_ASSERT(r, result.y_rows == expectedRows[0]);
        REPORTER_ASSERT(r, result.u_rows == expectedRows[1]);
        REPORTER_ASSERT(r, result.v_rows == expectedRows[2]);
        REPORTER_ASSERT(r, storage == expected);
    }
    REPORTER_ASSERT(r, suspensionsTested == 3);
}

DEF_TEST(RustJpegCodec_ScanlineWorkflows, r) {
    const char* paths[] = {
            "images/color_wheel.jpg",
            "images/cropped_mandrill.jpg",
            "images/b78329453.jpeg",
            "images/non_interleaved_420_64x64.jpg",
    };
    for (const char* path : paths) {
        skiatest::ReporterContext context(r, path);
        sk_sp<SkData> data = GetResourceAsData(path);
        REPORTER_ASSERT(r, data);
        if (!data) {
            continue;
        }

        std::unique_ptr<SkCodec> referenceCodec = decode_rust_jpeg_data(data);
        std::unique_ptr<SkCodec> codec = decode_rust_jpeg_data(data);
        REPORTER_ASSERT(r, referenceCodec && codec);
        if (!referenceCodec || !codec) {
            continue;
        }
        const SkImageInfo info = codec->getInfo().makeColorType(kN32_SkColorType);
        SkBitmap reference;
        reference.allocPixels(info);
        REPORTER_ASSERT_SUCCESSFUL_CODEC_RESULT(
                r, referenceCodec->getPixels(info, reference.getPixels(), reference.rowBytes()));

        const size_t rowBytes = info.minRowBytes() + 17;
        std::vector<uint8_t> output(rowBytes * static_cast<size_t>(info.height()), 0xCD);
        std::vector<bool> decodedRows(static_cast<size_t>(info.height()), false);
        REPORTER_ASSERT_SUCCESSFUL_CODEC_RESULT(r, codec->startScanlineDecode(info));

        int row = 0;
        REPORTER_ASSERT(r, codec->getScanlines(output.data(), 1, rowBytes) == 1);
        decodedRows[row++] = true;
        const int skip = std::min(3, info.height() - row);
        REPORTER_ASSERT(r, codec->skipScanlines(skip));
        row += skip;
        while (row < info.height()) {
            const int requested = std::min(7, info.height() - row);
            const int decoded =
                    codec->getScanlines(output.data() + row * rowBytes, requested, rowBytes);
            REPORTER_ASSERT(r, decoded == requested);
            for (int index = 0; index < decoded; ++index) {
                decodedRows[row + index] = true;
            }
            row += requested;
        }

        for (int y = 0; y < info.height(); ++y) {
            const uint8_t* actual = output.data() + y * rowBytes;
            const uint8_t* expected = static_cast<const uint8_t*>(reference.getAddr(0, y));
            if (decodedRows[y]) {
                REPORTER_ASSERT(r,
                                memcmp(actual, expected, info.minRowBytes()) == 0,
                                "%s row %d differs from full decode",
                                path,
                                y);
            } else {
                REPORTER_ASSERT(r,
                                std::all_of(actual,
                                            actual + info.minRowBytes(),
                                            [](uint8_t value) { return value == 0xCD; }),
                                "%s skipped row %d was modified",
                                path,
                                y);
            }
            REPORTER_ASSERT(r,
                            std::all_of(actual + info.minRowBytes(),
                                        actual + rowBytes,
                                        [](uint8_t value) { return value == 0xCD; }),
                            "%s row %d padding was modified",
                            path,
                            y);
        }

        std::fill(output.begin(), output.end(), 0xCD);
        REPORTER_ASSERT_SUCCESSFUL_CODEC_RESULT(r, codec->startScanlineDecode(info));
        REPORTER_ASSERT(
                r, codec->getScanlines(output.data(), info.height(), rowBytes) == info.height());
        for (int y = 0; y < info.height(); ++y) {
            REPORTER_ASSERT(r,
                            memcmp(output.data() + y * rowBytes,
                                   reference.getAddr(0, y),
                                   info.minRowBytes()) == 0);
        }
    }

    std::unique_ptr<SkCodec> codec = decode_rust_jpeg(r, "images/color_wheel.jpg");
    if (codec) {
        const SkImageInfo rgbaInfo = codec->getInfo().makeColorType(kRGBA_8888_SkColorType);
        SkIRect subset = SkIRect::MakeXYWH(1, 0, rgbaInfo.width() - 1, rgbaInfo.height());
        SkCodec::Options options;
        options.fSubset = &subset;
        REPORTER_ASSERT(r,
                        codec->startScanlineDecode(rgbaInfo, &options) == SkCodec::kUnimplemented);
        REPORTER_ASSERT(
                r,
                codec->startScanlineDecode(rgbaInfo.makeWH(
                        rgbaInfo.width() / 2, rgbaInfo.height() / 2)) == SkCodec::kInvalidScale);

        const SkImageInfo bgraInfo = codec->getInfo().makeColorType(kBGRA_8888_SkColorType);
        std::vector<uint8_t> bgra(bgraInfo.computeMinByteSize());
        REPORTER_ASSERT_SUCCESSFUL_CODEC_RESULT(r, codec->startScanlineDecode(bgraInfo));
        REPORTER_ASSERT(
                r,
                codec->getScanlines(bgra.data(), bgraInfo.height(), bgraInfo.minRowBytes()) ==
                        bgraInfo.height());
        std::unique_ptr<SkCodec> bgraReferenceCodec = decode_rust_jpeg(r, "images/color_wheel.jpg");
        std::vector<uint8_t> bgraReference(bgraInfo.computeMinByteSize());
        REPORTER_ASSERT(r, bgraReferenceCodec);
        if (bgraReferenceCodec) {
            REPORTER_ASSERT_SUCCESSFUL_CODEC_RESULT(
                    r,
                    bgraReferenceCodec->getPixels(
                            bgraInfo, bgraReference.data(), bgraInfo.minRowBytes()));
            REPORTER_ASSERT(r, bgra == bgraReference);
        }
    }
}


static void assert_row_padding_untouched(skiatest::Reporter* r,
                                         const std::vector<uint8_t>& pixels,
                                         const SkImageInfo& info,
                                         size_t rowBytes) {
    for (int row = 0; row < info.height(); ++row) {
        const uint8_t* rowStart = pixels.data() + static_cast<size_t>(row) * rowBytes;
        REPORTER_ASSERT(r,
                        std::all_of(rowStart + info.minRowBytes(),
                                    rowStart + rowBytes,
                                    [](uint8_t value) { return value == 0xCD; }),
                        "row %d padding was modified",
                        row);
    }
}

#if defined(SK_CODEC_DECODES_JPEG)

DEF_TEST(RustJpegCodec_GetPixelsUsesScanlines, r) {
    struct TestCase {
        const char* path;
        int cppTolerance;
        bool forceSRGB;
    };
    // zune-jpeg and libjpeg-turbo use different centered-chroma edge
    // extension for the final subsampled pixel. The 4:2:2 and 4:2:0
    // orientation fixtures observe a maximum right-edge deviation of 30;
    // getPixels and the explicit Rust scanline path must still match exactly.
    const TestCase testCases[] = {
            {"images/orientation/1_444.jpg", 8, false},
            {"images/orientation/1_440.jpg", 8, false},
            {"images/orientation/1_422.jpg", 30, false},
            {"images/orientation/1_420.jpg", 30, false},
            {"images/CMYK.jpg", 16, false},
            {"images/cropped_mandrill.jpg", 8, false},
            {"images/icc-v2-gbr.jpg", 8, true},
            {"images/non_interleaved_420_64x64.jpg", 8, false},
    };

    for (const TestCase& testCase : testCases) {
        skiatest::ReporterContext context(r, testCase.path);
        sk_sp<SkData> data = GetResourceAsData(testCase.path);
        REPORTER_ASSERT(r, data);
        if (!data) {
            continue;
        }

        SkCodec::Result makeResult;
        std::unique_ptr<SkJpegRustCodec> fullCodec =
                SkJpegRustCodec::MakeFromStream(SkMemoryStream::Make(data), &makeResult);
        REPORTER_ASSERT_SUCCESSFUL_CODEC_RESULT(r, makeResult);
        REPORTER_ASSERT(r,
                fullCodec &&
                    SkJpegRustCodecTestPriv::SupportsStreamingFullDecode(*fullCodec));
        if (!fullCodec) {
            continue;
        }

        SkImageInfo info = fullCodec->getInfo().makeColorType(kN32_SkColorType);
        if (testCase.forceSRGB) {
            info = info.makeColorSpace(SkColorSpace::MakeSRGB());
        }
        const size_t rowBytes = info.minRowBytes() + 19;
        std::vector<uint8_t> fullPixels(rowBytes * static_cast<size_t>(info.height()), 0xCD);
        REPORTER_ASSERT_SUCCESSFUL_CODEC_RESULT(
                r, fullCodec->getPixels(info, fullPixels.data(), rowBytes));
        REPORTER_ASSERT(r, SkJpegRustCodecTestPriv::LastFullDecodeRows(*fullCodec) == info.height());
        REPORTER_ASSERT(r, SkJpegRustCodecTestPriv::FullImageBufferCapacity(*fullCodec) == 0);
        REPORTER_ASSERT(r,
                SkJpegRustCodecTestPriv::ScanlineBufferSize(*fullCodec) <=
                    info.minRowBytes() * 36);
        assert_row_padding_untouched(r, fullPixels, info, rowBytes);

        std::unique_ptr<SkCodec> scanlineCodec = decode_rust_jpeg_data(data);
        std::vector<uint8_t> scanlinePixels(rowBytes * static_cast<size_t>(info.height()), 0xCD);
        REPORTER_ASSERT_SUCCESSFUL_CODEC_RESULT(r, scanlineCodec->startScanlineDecode(info));
        REPORTER_ASSERT(r,
                        scanlineCodec->getScanlines(
                                scanlinePixels.data(), info.height(), rowBytes) == info.height());
        assert_row_padding_untouched(r, scanlinePixels, info, rowBytes);

        SkCodec::Result cppResult;
        std::unique_ptr<SkCodec> cppCodec =
                SkJpegDecoder::Decode(SkMemoryStream::Make(data), &cppResult);
        REPORTER_ASSERT_SUCCESSFUL_CODEC_RESULT(r, cppResult);
        std::vector<uint8_t> cppPixels(rowBytes * static_cast<size_t>(info.height()), 0xCD);
        REPORTER_ASSERT_SUCCESSFUL_CODEC_RESULT(
                r, cppCodec->getPixels(info, cppPixels.data(), rowBytes));
        assert_row_padding_untouched(r, cppPixels, info, rowBytes);

        int maximumCppDeviation = 0;
        for (int row = 0; row < info.height(); ++row) {
            const uint8_t* fullRow = fullPixels.data() + static_cast<size_t>(row) * rowBytes;
            const uint8_t* scanlineRow =
                    scanlinePixels.data() + static_cast<size_t>(row) * rowBytes;
            const uint8_t* cppRow = cppPixels.data() + static_cast<size_t>(row) * rowBytes;
            REPORTER_ASSERT(r, memcmp(fullRow, scanlineRow, info.minRowBytes()) == 0);
            for (size_t column = 0; column < info.minRowBytes(); ++column) {
                maximumCppDeviation = std::max(maximumCppDeviation,
                                               std::abs(static_cast<int>(fullRow[column]) -
                                                        static_cast<int>(cppRow[column])));
            }
        }
        REPORTER_ASSERT(r,
                        maximumCppDeviation <= testCase.cppTolerance,
                        "%s maximum C++ deviation %d exceeds %d",
                        testCase.path,
                        maximumCppDeviation,
                        testCase.cppTolerance);

        if (&testCase == &testCases[0]) {
            std::vector<uint8_t> replay(rowBytes * static_cast<size_t>(info.height()), 0xCD);
            REPORTER_ASSERT_SUCCESSFUL_CODEC_RESULT(
                    r, fullCodec->getPixels(info, replay.data(), rowBytes));
            REPORTER_ASSERT(r, replay == fullPixels);
            REPORTER_ASSERT(r, SkJpegRustCodecTestPriv::FullImageBufferCapacity(*fullCodec) == 0);
        }
    }
}

#if defined(SK_CODEC_ENCODES_JPEG_WITH_RUST)

DEF_TEST(RustJpegCodec_GetPixelsBaselineGrayscale, r) {
    const SkImageInfo sourceInfo =
            SkImageInfo::Make(65, 63, kGray_8_SkColorType, kOpaque_SkAlphaType);
    SkBitmap source;
    source.allocPixels(sourceInfo);
    for (int row = 0; row < source.height(); ++row) {
        memset(source.getAddr8(0, row), row * 4, source.rowBytes());
    }
    SkDynamicMemoryWStream encodedStream;
    SkJpegEncoder::Options encodeOptions;
    encodeOptions.fQuality = 90;
    REPORTER_ASSERT(r, SkJpegRustEncoder::Encode(&encodedStream, source.pixmap(), encodeOptions));
    sk_sp<SkData> data = encodedStream.detachAsData();

    SkCodec::Result makeResult;
    std::unique_ptr<SkJpegRustCodec> fullCodec =
            SkJpegRustCodec::MakeFromStream(SkMemoryStream::Make(data), &makeResult);
    REPORTER_ASSERT_SUCCESSFUL_CODEC_RESULT(r, makeResult);
    REPORTER_ASSERT(r,
                    fullCodec &&
                            SkJpegRustCodecTestPriv::SupportsStreamingFullDecode(*fullCodec));
    if (!fullCodec) {
        return;
    }
    const SkImageInfo info = fullCodec->getInfo().makeColorType(kRGBA_8888_SkColorType);
    SkBitmap fullPixels;
    SkBitmap scanlinePixels;
    SkBitmap cppPixels;
    fullPixels.allocPixels(info);
    scanlinePixels.allocPixels(info);
    cppPixels.allocPixels(info);
    REPORTER_ASSERT_SUCCESSFUL_CODEC_RESULT(r, fullCodec->getPixels(fullPixels.pixmap()));
    REPORTER_ASSERT(r, SkJpegRustCodecTestPriv::FullImageBufferCapacity(*fullCodec) == 0);

    std::unique_ptr<SkCodec> scanlineCodec = decode_rust_jpeg_data(data);
    REPORTER_ASSERT_SUCCESSFUL_CODEC_RESULT(r, scanlineCodec->startScanlineDecode(info));
    REPORTER_ASSERT(r,
                    scanlineCodec->getScanlines(scanlinePixels.getPixels(),
                                                info.height(),
                                                scanlinePixels.rowBytes()) == info.height());
    REPORTER_ASSERT(r,
                    memcmp(fullPixels.getPixels(),
                           scanlinePixels.getPixels(),
                           fullPixels.computeByteSize()) == 0);

    std::unique_ptr<SkCodec> cppCodec =
            SkJpegDecoder::Decode(SkMemoryStream::Make(data), &makeResult);
    REPORTER_ASSERT_SUCCESSFUL_CODEC_RESULT(r, makeResult);
    REPORTER_ASSERT_SUCCESSFUL_CODEC_RESULT(r, cppCodec->getPixels(cppPixels.pixmap()));
    const uint8_t* fullBytes = static_cast<const uint8_t*>(fullPixels.getPixels());
    const uint8_t* cppBytes = static_cast<const uint8_t*>(cppPixels.getPixels());
    int maximumDeviation = 0;
    for (size_t index = 0; index < fullPixels.computeByteSize(); ++index) {
        maximumDeviation = std::max(
                maximumDeviation,
                std::abs(static_cast<int>(fullBytes[index]) - static_cast<int>(cppBytes[index])));
    }
    REPORTER_ASSERT(r, maximumDeviation <= 8, "maximum C++ deviation %d", maximumDeviation);
}

#endif  // defined(SK_CODEC_ENCODES_JPEG_WITH_RUST)
#endif  // defined(SK_CODEC_DECODES_JPEG)

DEF_TEST(RustJpegCodec_GetPixelsProgressiveFallback, r) {
    sk_sp<SkData> data = GetResourceAsData("images/b78329453.jpeg");
    REPORTER_ASSERT(r, data);
    if (!data) {
        return;
    }
    SkCodec::Result makeResult;
    std::unique_ptr<SkJpegRustCodec> codec =
            SkJpegRustCodec::MakeFromStream(SkMemoryStream::Make(data), &makeResult);
    REPORTER_ASSERT_SUCCESSFUL_CODEC_RESULT(r, makeResult);
    REPORTER_ASSERT(r,
                    codec && !SkJpegRustCodecTestPriv::SupportsStreamingFullDecode(*codec));
    if (!codec) {
        return;
    }
    const SkImageInfo info = codec->getInfo().makeColorType(kN32_SkColorType);
    SkBitmap pixels;
    pixels.allocPixels(info);
    REPORTER_ASSERT_SUCCESSFUL_CODEC_RESULT(r, codec->getPixels(pixels.pixmap()));
    REPORTER_ASSERT(r, SkJpegRustCodecTestPriv::LastFullDecodeRows(*codec) == info.height());
    REPORTER_ASSERT(r,
                        SkJpegRustCodecTestPriv::FullImageBufferCapacity(*codec) >=
                            static_cast<uint64_t>(info.width()) * info.height() * 3);
                REPORTER_ASSERT(r, SkJpegRustCodecTestPriv::ScanlineBufferSize(*codec) == 0);
}

DEF_TEST(RustJpegCodec_GetPixelsExtendedSequential, r) {
    sk_sp<SkData> baseline = GetResourceAsData("images/orientation/1_420.jpg");
    REPORTER_ASSERT(r, baseline);
    if (!baseline) {
        return;
    }
    sk_sp<SkData> extended = SkData::MakeUninitialized(baseline->size());
    memcpy(extended->writable_data(), baseline->data(), baseline->size());
    auto* bytes = static_cast<uint8_t*>(extended->writable_data());
    for (size_t index = 0; index + 1 < extended->size(); ++index) {
        if (bytes[index] == 0xFF && bytes[index + 1] == 0xC0) {
            bytes[index + 1] = 0xC1;
            break;
        }
    }

    SkCodec::Result makeResult;
    std::unique_ptr<SkJpegRustCodec> codec =
            SkJpegRustCodec::MakeFromStream(SkMemoryStream::Make(extended), &makeResult);
    REPORTER_ASSERT_SUCCESSFUL_CODEC_RESULT(r, makeResult);
    REPORTER_ASSERT(r,
                    codec && SkJpegRustCodecTestPriv::SupportsStreamingFullDecode(*codec));
    if (codec) {
        SkBitmap pixels;
        pixels.allocPixels(codec->getInfo().makeColorType(kN32_SkColorType));
        REPORTER_ASSERT_SUCCESSFUL_CODEC_RESULT(r, codec->getPixels(pixels.pixmap()));
        REPORTER_ASSERT(r, SkJpegRustCodecTestPriv::FullImageBufferCapacity(*codec) == 0);
    }
}

DEF_TEST(RustJpegCodec_GetPixelsTruncatedRows, r) {
    constexpr size_t kInitialBytes = 4 * 1024;
    sk_sp<SkData> data = GetResourceAsData("images/color_wheel.jpg");
    REPORTER_ASSERT(r, data && data->size() > kInitialBytes);
    if (!data || data->size() <= kInitialBytes) {
        return;
    }

    auto fullStream = std::make_unique<HaltingStream>(data, kInitialBytes);
    SkCodec::Result makeResult;
    std::unique_ptr<SkJpegRustCodec> fullCodec =
            SkJpegRustCodec::MakeFromStream(std::move(fullStream), &makeResult);
    REPORTER_ASSERT_SUCCESSFUL_CODEC_RESULT(r, makeResult);
    REPORTER_ASSERT(r, fullCodec);
    if (!fullCodec) {
        return;
    }
    const SkImageInfo info = fullCodec->getInfo().makeColorType(kN32_SkColorType);
    const size_t rowBytes = info.minRowBytes() + 11;
    std::vector<uint8_t> fullPixels(rowBytes * static_cast<size_t>(info.height()), 0xCD);
    for (int row = 0; row < info.height(); ++row) {
        memset(fullPixels.data() + static_cast<size_t>(row) * rowBytes, 0, info.minRowBytes());
    }
    SkCodec::Options options;
    options.fZeroInitialized = SkCodec::kYes_ZeroInitialized;
    const SkCodec::Result fullResult =
            fullCodec->getPixels(info, fullPixels.data(), rowBytes, &options);
    REPORTER_ASSERT(r, fullResult == SkCodec::kIncompleteInput);

    auto scanlineStream = std::make_unique<HaltingStream>(data, kInitialBytes);
    std::unique_ptr<SkCodec> scanlineCodec =
            SkJpegRustDecoder::Decode(std::move(scanlineStream), &makeResult);
    REPORTER_ASSERT_SUCCESSFUL_CODEC_RESULT(r, makeResult);
    std::vector<uint8_t> scanlinePixels(rowBytes * static_cast<size_t>(info.height()), 0xCD);
    for (int row = 0; row < info.height(); ++row) {
        memset(scanlinePixels.data() + static_cast<size_t>(row) * rowBytes, 0, info.minRowBytes());
    }
    REPORTER_ASSERT_SUCCESSFUL_CODEC_RESULT(r, scanlineCodec->startScanlineDecode(info, &options));
    const int scanlineRows =
            scanlineCodec->getScanlines(scanlinePixels.data(), info.height(), rowBytes);
    REPORTER_ASSERT(r, scanlineRows > 0 && scanlineRows < info.height());
    REPORTER_ASSERT(r, SkJpegRustCodecTestPriv::LastFullDecodeRows(*fullCodec) == scanlineRows);
    for (int row = 0; row < scanlineRows; ++row) {
        REPORTER_ASSERT(r,
                        memcmp(fullPixels.data() + static_cast<size_t>(row) * rowBytes,
                               scanlinePixels.data() + static_cast<size_t>(row) * rowBytes,
                               info.minRowBytes()) == 0);
    }
    assert_row_padding_untouched(r, fullPixels, info, rowBytes);
    REPORTER_ASSERT(r, SkJpegRustCodecTestPriv::FullImageBufferCapacity(*fullCodec) == 0);
}

DEF_TEST(RustJpegCodec_GetPixelsMissingEoiAfterAllRows, r) {
    sk_sp<SkData> data = GetResourceAsData("images/color_wheel.jpg");
    REPORTER_ASSERT(r, data && data->size() > 2);
    if (!data || data->size() <= 2) {
        return;
    }
    const uint8_t* bytes = data->bytes();
    REPORTER_ASSERT(r, bytes[data->size() - 2] == 0xFF && bytes[data->size() - 1] == 0xD9);
    sk_sp<SkData> missingEoi = SkData::MakeUninitialized(data->size() + 2);
    auto* missingEoiBytes = static_cast<uint8_t*>(missingEoi->writable_data());
    memcpy(missingEoiBytes, bytes, data->size() - 2);
    const uint8_t emptyAppSegment[] = {0xFF, 0xE0, 0x00, 0x02};
    memcpy(missingEoiBytes + data->size() - 2, emptyAppSegment, sizeof(emptyAppSegment));

    SkCodec::Result makeResult;
    std::unique_ptr<SkJpegRustCodec> fullCodec =
            SkJpegRustCodec::MakeFromStream(SkMemoryStream::Make(missingEoi), &makeResult);
    REPORTER_ASSERT_SUCCESSFUL_CODEC_RESULT(r, makeResult);
    REPORTER_ASSERT(r, fullCodec);
    if (!fullCodec) {
        return;
    }
    const SkImageInfo info = fullCodec->getInfo().makeColorType(kN32_SkColorType);
    SkBitmap fullPixels;
    fullPixels.allocPixels(info);
    REPORTER_ASSERT_SUCCESSFUL_CODEC_RESULT(r, fullCodec->getPixels(fullPixels.pixmap()));
    REPORTER_ASSERT(r, SkJpegRustCodecTestPriv::LastFullDecodeRows(*fullCodec) == info.height());

    std::unique_ptr<SkCodec> scanlineCodec = decode_rust_jpeg_data(std::move(missingEoi));
    SkBitmap scanlinePixels;
    scanlinePixels.allocPixels(info);
    REPORTER_ASSERT_SUCCESSFUL_CODEC_RESULT(r, scanlineCodec->startScanlineDecode(info));
    REPORTER_ASSERT(r,
                    scanlineCodec->getScanlines(scanlinePixels.getPixels(),
                                                info.height(),
                                                scanlinePixels.rowBytes()) == info.height());
    REPORTER_ASSERT(r,
                    memcmp(fullPixels.getPixels(),
                           scanlinePixels.getPixels(),
                           fullPixels.computeByteSize()) == 0);
}

DEF_TEST(RustJpegCodec_StreamingOutputRetry, r) {
    constexpr size_t kInitialBytes = 1024;
    sk_sp<SkData> data = GetResourceAsData("images/color_wheel.jpg");
    REPORTER_ASSERT(r, data && data->size() > kInitialBytes);
    if (!data || data->size() <= kInitialBytes) {
        return;
    }

    {
        auto stream = std::make_unique<HaltingStream>(data, kInitialBytes);
        HaltingStream* streamPtr = stream.get();
        SkCodec::Result result;
        std::unique_ptr<SkCodec> codec = SkJpegRustDecoder::Decode(std::move(stream), &result);
        REPORTER_ASSERT_SUCCESSFUL_CODEC_RESULT(r, result);
        REPORTER_ASSERT(r, codec);
        if (codec) {
            constexpr auto kAllTypes = SkYUVAPixmapInfo::SupportedDataTypes::All();
            SkYUVAPixmapInfo info;
            REPORTER_ASSERT(r, codec->queryYUVAInfo(kAllTypes, &info));
            SkYUVAPixmaps planes = SkYUVAPixmaps::Allocate(info);
            result = codec->getYUVAPlanes(planes);
            REPORTER_ASSERT(r,
                            result == SkCodec::kIncompleteInput,
                            "raw partial decode returned %s",
                            SkCodec::ResultToString(result));
            streamPtr->addNewData(data->size());
            REPORTER_ASSERT_SUCCESSFUL_CODEC_RESULT(r, codec->getYUVAPlanes(planes));

            std::unique_ptr<SkCodec> referenceCodec = decode_rust_jpeg_data(data);
            SkYUVAPixmapInfo referenceInfo;
            REPORTER_ASSERT(r, referenceCodec->queryYUVAInfo(kAllTypes, &referenceInfo));
            SkYUVAPixmaps referencePlanes = SkYUVAPixmaps::Allocate(referenceInfo);
            REPORTER_ASSERT_SUCCESSFUL_CODEC_RESULT(r,
                                                    referenceCodec->getYUVAPlanes(referencePlanes));

#if defined(SK_CODEC_DECODES_JPEG)
            std::unique_ptr<SkCodec> cppCodec =
                    SkJpegDecoder::Decode(SkMemoryStream::Make(data), nullptr);
            SkYUVAPixmapInfo cppInfo;
            REPORTER_ASSERT(r, cppCodec->queryYUVAInfo(kAllTypes, &cppInfo));
            SkYUVAPixmaps cppPlanes = SkYUVAPixmaps::Allocate(cppInfo);
            REPORTER_ASSERT_SUCCESSFUL_CODEC_RESULT(r, cppCodec->getYUVAPlanes(cppPlanes));
#endif

            for (int plane = 0; plane < planes.numPlanes(); ++plane) {
                for (int row = 0; row < planes.plane(plane).height(); ++row) {
                    const auto* actual =
                            static_cast<const uint8_t*>(planes.plane(plane).addr(0, row));
                    const auto* expected =
                            static_cast<const uint8_t*>(referencePlanes.plane(plane).addr(0, row));
                    REPORTER_ASSERT(r,
                                    memcmp(actual,
                                           expected,
                                           static_cast<size_t>(planes.plane(plane).width())) == 0);
#if defined(SK_CODEC_DECODES_JPEG)
                    const auto* cpp =
                            static_cast<const uint8_t*>(cppPlanes.plane(plane).addr(0, row));
                    for (int column = 0; column < planes.plane(plane).width(); ++column) {
                        REPORTER_ASSERT(r,
                                        std::abs(static_cast<int>(actual[column]) -
                                                 static_cast<int>(cpp[column])) <= 2);
                    }
#endif
                }
            }
        }
    }

    {
        auto stream = std::make_unique<HaltingStream>(data, kInitialBytes);
        HaltingStream* streamPtr = stream.get();
        SkCodec::Result result;
        std::unique_ptr<SkCodec> codec = SkJpegRustDecoder::Decode(std::move(stream), &result);
        REPORTER_ASSERT_SUCCESSFUL_CODEC_RESULT(r, result);
        REPORTER_ASSERT(r, codec);
        if (!codec) {
            return;
        }

        const SkImageInfo info = codec->getInfo().makeColorType(kRGBA_8888_SkColorType);
        SkCodec::Options options;
        options.fZeroInitialized = SkCodec::kYes_ZeroInitialized;
        REPORTER_ASSERT_SUCCESSFUL_CODEC_RESULT(r, codec->startScanlineDecode(info, &options));
        std::vector<uint8_t> row(info.minRowBytes(), 0xCD);
        bool suspended = false;
        for (int y = 0; y < info.height(); ++y) {
            std::fill(row.begin(), row.end(), 0xCD);
            if (codec->getScanlines(row.data(), 1, row.size()) == 0) {
                suspended = true;
                REPORTER_ASSERT(r, std::all_of(row.begin(), row.end(), [](uint8_t value) {
                                    return value == 0xCD;
                                }));
                break;
            }
        }
        REPORTER_ASSERT(r, suspended);

        streamPtr->addNewData(data->size());
        std::vector<uint8_t> scanlines(info.computeMinByteSize(), 0xCD);
        REPORTER_ASSERT_SUCCESSFUL_CODEC_RESULT(r, codec->startScanlineDecode(info));
        REPORTER_ASSERT(r,
                        codec->getScanlines(scanlines.data(), info.height(), info.minRowBytes()) ==
                                info.height());

        std::unique_ptr<SkCodec> referenceCodec = decode_rust_jpeg_data(data);
        std::vector<uint8_t> reference(info.computeMinByteSize());
        REPORTER_ASSERT(r, referenceCodec);
        if (referenceCodec) {
            REPORTER_ASSERT_SUCCESSFUL_CODEC_RESULT(
                    r, referenceCodec->getPixels(info, reference.data(), info.minRowBytes()));
            REPORTER_ASSERT(r, scanlines == reference);
        }
    }
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
            {"images/dog.jpg", 180, 180, 90},
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
             kGray_8_SkColorType,
             kOpaque_SkAlphaType,
             SK_ColorGRAY,
             SkJpegEncoder::AlphaOption::kIgnore},
            {"RGBA ignore alpha",
             kRGBA_8888_SkColorType,
             kUnpremul_SkAlphaType,
             0x80FF0000,
             SkJpegEncoder::AlphaOption::kIgnore},
            {"RGBA blend on black",
             kRGBA_8888_SkColorType,
             kUnpremul_SkAlphaType,
             0x8000FF00,
             SkJpegEncoder::AlphaOption::kBlendOnBlack},
            {"BGRA",
             kBGRA_8888_SkColorType,
             kUnpremul_SkAlphaType,
             SK_ColorBLUE,
             SkJpegEncoder::AlphaOption::kIgnore},
            {"RGB_888x (4-byte opaque)",
             kRGB_888x_SkColorType,
             kOpaque_SkAlphaType,
             SK_ColorRED,
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
        REPORTER_ASSERT(
                r,
                SkCodec::kSuccess == gainmapCodec->getAndroidPixels(gainmapBitmap.info(),
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
    auto error =
            std::function<ComparePixmapsErrorReporter>([&](int x, int y, const float diffs[4]) {
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
            {"color wheel (baseline RGB)", "images/color_wheel.jpg", kDefaultTol},
            {"dog (180x180, RGB)", "images/dog.jpg", kDefaultTol},
            {"ducky", "images/ducky.jpg", kDefaultTol},
            {"mandrill 512 q075", "images/mandrill_512_q075.jpg", kDefaultTol},
            {"mandrill h1v1 subsampling", "images/mandrill_h1v1.jpg", kDefaultTol},
            {"mandrill h2v1 subsampling", "images/mandrill_h2v1.jpg", kDefaultTol},
            {"grayscale", "images/grayscale.jpg", kDefaultTol},
            {"progressive (b78329453)", "images/b78329453.jpeg", kDefaultTol},
            // Progressive JPEGs can have slightly larger inter-implementation diffs.
            {"progressive (crbug999986)", "images/crbug999986.jpeg", 10.0f / 255.0f},
            {"cropped mandrill", "images/cropped_mandrill.jpg", kDefaultTol},
            {"flutter logo", "images/flutter_logo.jpg", kDefaultTol},
            {"ICC v2 GBR profile", "images/icc-v2-gbr.jpg", kDefaultTol},
            // randPixels has small DCT rounding differences between implementations.
            {"random pixels", "images/randPixels.jpg", 16.0f / 255.0f},
            // CMYK: different CMYK→RGB conversion paths (zune-jpeg vs libjpeg-turbo)
            // may produce slightly larger per-channel differences.
            {"CMYK", "images/CMYK.jpg", 16.0f / 255.0f},
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
            "images/orientation/exif.jpg",    // large EXIF block
            "images/orientation/subifd.jpg",  // EXIF with sub-IFD structure
            "images/ducky.jpg",
            "images/color_wheel.jpg",  // no EXIF → default orientation
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
            "images/icc-v2-gbr.jpg",   // ICC v2 with GBR channel order
            "images/ducky.jpg",        // large ICC profile
            "images/color_wheel.jpg",  // no ICC → both should be nullptr
            "images/grayscale.jpg",    // no ICC
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
            ERRORF(r,
                   "%s: ICC presence mismatch (Rust %s, C++ %s)",
                   path,
                   rustICC ? "present" : "absent",
                   cppICC ? "present" : "absent");
            continue;
        }

        // Compare data class and color space.
        REPORTER_ASSERT(r,
                        rustICC->data_color_space == cppICC->data_color_space,
                        "%s: ICC data_color_space mismatch",
                        path);

        // Compare the TRC (transfer function) for each channel.
        for (int ch = 0; ch < 3; ++ch) {
            REPORTER_ASSERT(r,
                            skcms_TransferFunction_isSRGBish(&rustICC->trc[ch].parametric) ==
                                    skcms_TransferFunction_isSRGBish(&cppICC->trc[ch].parametric),
                            "%s: ICC TRC[%d] sRGB-ish mismatch",
                            path,
                            ch);
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
                                path,
                                row,
                                col,
                                rustVal,
                                cppVal);
            }
        }
    }
}

#endif  // defined(SK_CODEC_DECODES_JPEG)
