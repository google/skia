/*
 * Copyright 2024 Google LLC.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/codec/SkPngRustDecoder.h"

#include <memory>
#include <utility>

#include "include/codec/SkCodec.h"
#include "include/codec/SkCodecAnimation.h"
#include "include/core/SkBitmap.h"
#include "include/core/SkColor.h"
#include "include/core/SkColorType.h"
#include "include/core/SkData.h"
#include "include/core/SkImage.h"
#include "include/core/SkImageInfo.h"
#include "include/core/SkPixmap.h"
#include "include/core/SkRefCnt.h"
#include "include/core/SkStream.h"
#include "tests/FakeStreams.h"
#include "tests/Test.h"
#include "tools/Resources.h"

#define REPORTER_ASSERT_SUCCESSFUL_CODEC_RESULT(r, actualResult) \
    REPORTER_ASSERT(r,                                           \
                    actualResult == SkCodec::kSuccess,           \
                    "actualResult=\"%s\" != kSuccess",           \
                    SkCodec::ResultToString(actualResult))

// Helper wrapping a call to `SkPngRustDecoder::Decode`.
std::unique_ptr<SkCodec> SkPngRustDecoderDecode(skiatest::Reporter* r, const char* path) {
    sk_sp<SkData> data = GetResourceAsData(path);
    if (!data) {
        ERRORF(r, "Missing resource: %s", path);
        return nullptr;
    }

    SkCodec::Result result;
    std::unique_ptr<SkCodec> codec =
            SkPngRustDecoder::Decode(std::make_unique<SkMemoryStream>(std::move(data)), &result);
    REPORTER_ASSERT_SUCCESSFUL_CODEC_RESULT(r, result);

    return codec;
}

void AssertPixelColor(skiatest::Reporter* r,
                      const SkPixmap& pixmap,
                      int x,
                      int y,
                      SkColor expectedColor,
                      const char* description) {
    SkASSERT(r);
    SkASSERT(x >= 0);
    SkASSERT(y >= 0);
    SkASSERT(description);

    REPORTER_ASSERT(r, x < pixmap.width(), "x=%d >= width=%d", x, pixmap.width());
    REPORTER_ASSERT(r, y < pixmap.height(), "y=%d >= height=%d", y, pixmap.height());
    REPORTER_ASSERT(r,
                    kN32_SkColorType == pixmap.colorType(),
                    "kN32_SkColorType != pixmap.ColorType()=%d",
                    pixmap.colorType());

    SkColor actualColor = pixmap.getColor(x, y);
    REPORTER_ASSERT(r,
                    actualColor == expectedColor,
                    "actualColor=0x%08X != expectedColor==0x%08X at (%d,%d) (%s)",
                    actualColor,
                    expectedColor,
                    x,
                    y,
                    description);
}

void AssertGreenPixel(skiatest::Reporter* r,
                      const SkPixmap& pixmap,
                      int x,
                      int y,
                      const char* description = "Expecting a green pixel") {
    AssertPixelColor(r, pixmap, x, y, SkColorSetRGB(0x00, 0xFF, 0x00), description);
}

void AssertRedPixel(skiatest::Reporter* r,
                    const SkPixmap& pixmap,
                    int x,
                    int y,
                    const char* description = "Expecting a red pixel") {
    AssertPixelColor(r, pixmap, x, y, SkColorSetRGB(0xFF, 0x00, 0x00), description);
}

void AssertBluePixel(skiatest::Reporter* r,
                     const SkPixmap& pixmap,
                     int x,
                     int y,
                     const char* description = "Expecting a blue pixel") {
    AssertPixelColor(r, pixmap, x, y, SkColorSetRGB(0x00, 0x00, 0xFF), description);
}

void AssertSingleGreenFrame(skiatest::Reporter* r,
                            int expectedWidth,
                            int expectedHeight,
                            const char* resourcePath) {
    std::unique_ptr<SkCodec> codec = SkPngRustDecoderDecode(r, resourcePath);
    if (!codec) {
        return;
    }

    REPORTER_ASSERT(r, codec->getFrameCount() == 1);
    REPORTER_ASSERT(r, codec->getRepetitionCount() == 0);

    SkCodec::FrameInfo info;
    REPORTER_ASSERT(r, codec->getFrameInfo(0, &info));
    REPORTER_ASSERT(r, info.fBlend == SkCodecAnimation::Blend::kSrc);
    REPORTER_ASSERT(r, info.fDisposalMethod == SkCodecAnimation::DisposalMethod::kKeep);
    REPORTER_ASSERT(r, info.fFrameRect == SkIRect::MakeWH(expectedWidth, expectedHeight));
    REPORTER_ASSERT(r, info.fRequiredFrame == SkCodec::kNoFrame);

    auto [image, result] = codec->getImage();
    REPORTER_ASSERT_SUCCESSFUL_CODEC_RESULT(r, result);

    REPORTER_ASSERT(r, image);
    REPORTER_ASSERT(r,
                    image->width() == expectedWidth,
                    "actualWidth=%d != expectedWidth=%d",
                    image->width(),
                    expectedWidth);
    REPORTER_ASSERT(r,
                    image->height() == expectedHeight,
                    "actualHeight=%d != expectedHeight=%d",
                    image->height(),
                    expectedHeight);

    SkPixmap pixmap;
    REPORTER_ASSERT(r, image->peekPixels(&pixmap));

    AssertGreenPixel(r, pixmap, 0, 0);
    AssertGreenPixel(r, pixmap, expectedWidth / 2, expectedHeight / 2);
}

sk_sp<SkImage> DecodeLastFrame(skiatest::Reporter* r, SkCodec* codec) {
    int frameCount = codec->getFrameCount();
    sk_sp<SkImage> image;
    SkCodec::Result result = SkCodec::kSuccess;
    for (int i = 0; i < frameCount; i++) {
        SkCodec::FrameInfo info;
        REPORTER_ASSERT(r, codec->getFrameInfo(i, &info));

        // This test method only supports `kKeep` disposal method.
        SkASSERT(info.fDisposalMethod == SkCodecAnimation::DisposalMethod::kKeep);

        if (!image) {
            std::tie(image, result) = codec->getImage();
            REPORTER_ASSERT_SUCCESSFUL_CODEC_RESULT(r, result);
            if (result != SkCodec::kSuccess) {
                return nullptr;
            }
        } else {
            SkPixmap pixmap;
            REPORTER_ASSERT(r, image->peekPixels(&pixmap));

            SkCodec::Options options;
            options.fZeroInitialized = SkCodec::kNo_ZeroInitialized;
            options.fSubset = nullptr;
            options.fFrameIndex = i;
            options.fPriorFrame = i - 1;

            result = codec->getPixels(pixmap, &options);
            REPORTER_ASSERT_SUCCESSFUL_CODEC_RESULT(r, result);
            if (result != SkCodec::kSuccess) {
                return nullptr;
            }
        }
    }

    return image;
}

sk_sp<SkImage> DecodeLastFrame(skiatest::Reporter* r, const char* resourcePath) {
    std::unique_ptr<SkCodec> codec = SkPngRustDecoderDecode(r, resourcePath);
    if (!codec) {
        return nullptr;
    }

    return DecodeLastFrame(r, codec.get());
}

void AssertAnimationRepetitionCount(skiatest::Reporter* r,
                                    int expectedRepetitionCount,
                                    const char* resourcePath) {
    std::unique_ptr<SkCodec> codec = SkPngRustDecoderDecode(r, resourcePath);
    if (!codec) {
        return;
    }

    int actualRepetitionCount = codec->getRepetitionCount();
    REPORTER_ASSERT(r,
                    actualRepetitionCount == expectedRepetitionCount,
                    "actualRepetitionCount=%d != expectedRepetitionCount=%d",
                    actualRepetitionCount,
                    expectedRepetitionCount);
}

// Test based on
// https://philip.html5.org/tests/apng/tests.html#trivial-static-image
DEF_TEST(RustPngCodec_apng_basic_trivial_static_image, r) {
    AssertSingleGreenFrame(r, 128, 64, "images/apng-test-suite--basic--trivial-static-image.png");
}

// Test based on
// https://philip.html5.org/tests/apng/tests.html#trivial-animated-image-one-frame-using-default-image
DEF_TEST(RustPngCodec_apng_basic_using_default_image, r) {
    AssertSingleGreenFrame(r, 128, 64, "images/apng-test-suite--basic--using-default-image.png");
}

// Test based on
// https://philip.html5.org/tests/apng/tests.html#trivial-animated-image-one-frame-ignoring-default-image
//
// The input file contains the following PNG chunks: IHDR, acTL, IDAT, fcTL,
// fdAT, IEND.  Presence of acTL chunk + no fcTL chunk before IDAT means that
// the IDAT chunk is *not* part of the animation (i.e. APNG-aware decoders
// should ignore IDAT frame and start with fdAT frame).  This test will fail
// with non-APNG-aware decoders (e.g. with `SkPngCodec`), because the `IDAT`
// chunk represents a red image (the `fdAT` chunk represents a green image).
DEF_TEST(RustPngCodec_apng_basic_ignoring_default_image, r) {
    AssertSingleGreenFrame(r, 128, 64, "images/apng-test-suite--basic--ignoring-default-image.png");
}

// Test based on
// https://philip.html5.org/tests/apng/tests.html#apng-dispose-op-none-basic
//
// This test covers two aspects of `SkPngRustCodec` implementation:
//
// * Blink expects that `onGetFrameCount` returns the total frame count when
//   the complete image resource is available (i.e. a lower frame count should
//   only happen upon `SkCodec::kIncompleteInput`).  Before http://review.skia.org/911038
//   `SkPngRustCodec::onGetFrameCount` would not discover additional frames if
//   previous frames haven't been decoded yet.
// * Skia client (e.g. Blink; or here the testcase) is expected to handle
//   `SkCodecAnimation::DisposalMethod` and populate the target buffer (and
//   `SkCodec::Options::fPriorFrame`) with the expected pixels.  OTOH,
//   `SkPngRustCodec` needs to handle `SkCodecAnimation::Blend` - without this
//   the final frame in this test will contain red pixels.
DEF_TEST(RustPngCodec_apng_dispose_op_none_basic, r) {
    std::unique_ptr<SkCodec> codec =
            SkPngRustDecoderDecode(r, "images/apng-test-suite--dispose-ops--none-basic.png");
    if (!codec) {
        return;
    }

    REPORTER_ASSERT(r, codec->getFrameCount() == 3);
    REPORTER_ASSERT(r, codec->getRepetitionCount() == 0);

    // We should have `FrameInfo` for all 3 frames.
    SkCodec::FrameInfo info[3];
    REPORTER_ASSERT(r, codec->getFrameInfo(0, &info[0]));
    REPORTER_ASSERT(r, codec->getFrameInfo(1, &info[1]));
    REPORTER_ASSERT(r, codec->getFrameInfo(2, &info[2]));

    // The codec should realize that the `SkStream` contains all the data of the
    // first 2 frames.  Currently `SkPngRustCodec::onGetFrameCount` stops after
    // parsing the final, 3rd `fcTL` chunk and therefore it can't tell if the
    // subsequent `fdAT` chunk has been fully received or not.
    REPORTER_ASSERT(r, info[0].fFullyReceived);
    REPORTER_ASSERT(r, info[1].fFullyReceived);
    REPORTER_ASSERT(r, !info[2].fFullyReceived);

    // Spot-check frame metadata.
    REPORTER_ASSERT(r, info[1].fAlphaType == kUnpremul_SkAlphaType);
    REPORTER_ASSERT(r, info[1].fBlend == SkCodecAnimation::Blend::kSrcOver);
    REPORTER_ASSERT(r, info[1].fDisposalMethod == SkCodecAnimation::DisposalMethod::kKeep);
    REPORTER_ASSERT(r, info[1].fDuration == 100, "dur = %d", info[1].fDuration);
    REPORTER_ASSERT(r, info[1].fFrameRect == SkIRect::MakeWH(128, 64));
    REPORTER_ASSERT(r, info[1].fHasAlphaWithinBounds);
    REPORTER_ASSERT(r, info[1].fRequiredFrame == 0);

    // Validate contents of the first frame.
    auto [image, result] = codec->getImage();
    REPORTER_ASSERT_SUCCESSFUL_CODEC_RESULT(r, result);
    REPORTER_ASSERT(r, image);
    REPORTER_ASSERT(r, image->width() == 128, "width %d != 128", image->width());
    REPORTER_ASSERT(r, image->height() == 64, "height %d != 64", image->height());
    SkPixmap pixmap;
    REPORTER_ASSERT(r, image->peekPixels(&pixmap));
    AssertRedPixel(r, pixmap, 0, 0, "Frame #0 should be red");

    // Validate contents of the second frame.
    SkCodec::Options options;
    options.fZeroInitialized = SkCodec::kNo_ZeroInitialized;
    options.fSubset = nullptr;
    options.fFrameIndex = 1;  // We want to decode the second frame.
    options.fPriorFrame = 0;  // `pixmap` contains the first frame before `getPixels` call.
    REPORTER_ASSERT_SUCCESSFUL_CODEC_RESULT(r, codec->getPixels(pixmap, &options));
    AssertGreenPixel(r, pixmap, 0, 0, "Frame #1 should be green");

    // Validate contents of the third frame.
    options.fFrameIndex = 2;  // We want to decode the second frame.
    options.fPriorFrame = 1;  // `pixmap` contains the second frame before `getPixels` call.
    REPORTER_ASSERT_SUCCESSFUL_CODEC_RESULT(r, codec->getPixels(pixmap, &options));
    AssertGreenPixel(r, pixmap, 0, 0, "Frame #2 should be green");
}

// This test covers an incomplete input scenario:
//
// * Only half of 1st frame is available during `onGetFrameCount`.
//   In this situation `onGetFrameCount` may consume the whole input in a
//   (futile in this case) attempt to discover `fcTL` chunks for 2nd and 3rd
//   frame.  This will mean that the input stream is in the middle of the 1st
//   frame - no longer positioned correctly for decoding the 1st frame.
// * Full input is available when subsequently decoding 1st frame.
DEF_TEST(RustPngCodec_apng_dispose_op_none_basic_incomplete_input1, r) {
    const char* path = "images/apng-test-suite--dispose-ops--none-basic.png";
    sk_sp<SkData> data = GetResourceAsData(path);
    if (!data) {
        ERRORF(r, "Missing resource: %s", path);
        return;
    }
    size_t fullLength = data->size();

    // Initially expose roughly middle of `IDAT` chunk (in this image `fcTL` is
    // present before the `IDAT` chunk and therefore the `IDAT` chunk is part of
    // the animated image).
    constexpr size_t kInitialBytes = 0xAD;
    auto streamForCodec = std::make_unique<HaltingStream>(std::move(data), kInitialBytes);
    HaltingStream* retainedStream = streamForCodec.get();

    SkCodec::Result result;
    std::unique_ptr<SkCodec> codec = SkPngRustDecoder::Decode(std::move(streamForCodec), &result);
    REPORTER_ASSERT_SUCCESSFUL_CODEC_RESULT(r, result);
    if (!codec) {
        return;
    }

    SkBitmap bitmap;
    if (!bitmap.tryAllocN32Pixels(codec->dimensions().width(), codec->dimensions().height())) {
        ERRORF(r, "Failed to allocate SkBitmap");
        return;
    }

    // Try to provoke the codec to consume the currently-available part of the
    // input stream.
    //
    // At this point only the metadata for the first frame is available.
    int frameCount = codec->getFrameCount();
    REPORTER_ASSERT(r, frameCount == 1);

    // Make the rest of the input available to the codec.
    retainedStream->addNewData(fullLength);

    // Try to decode the first frame and check its contents.
    sk_sp<SkImage> image;
    std::tie(image, result) = codec->getImage();
    REPORTER_ASSERT_SUCCESSFUL_CODEC_RESULT(r, result);
    REPORTER_ASSERT(r, image);
    REPORTER_ASSERT(r, image->width() == 128, "width %d != 128", image->width());
    REPORTER_ASSERT(r, image->height() == 64, "height %d != 64", image->height());
    SkPixmap pixmap;
    REPORTER_ASSERT(r, image->peekPixels(&pixmap));
    AssertRedPixel(r, pixmap, 0, 0, "Frame #0 should be red");
}

// This test covers an incomplete input scenario:
//
// * Only half of 1st frame is available during the initial `incrementalDecode`.
// * Before retrying, `getFrameCount` is called.  This should *not* reposition
//   the stream while in the middle of an active incremental decode.
// * Then we retry `incrementalDecode`.
DEF_TEST(RustPngCodec_apng_dispose_op_none_basic_incomplete_input2, r) {
    const char* path = "images/apng-test-suite--dispose-ops--none-basic.png";
    sk_sp<SkData> data = GetResourceAsData(path);
    if (!data) {
        ERRORF(r, "Missing resource: %s", path);
        return;
    }
    size_t fullLength = data->size();

    constexpr size_t kInitialBytes = 0x8D;  // Roughly middle of IDAT chunk.
    auto streamForCodec = std::make_unique<HaltingStream>(std::move(data), kInitialBytes);
    HaltingStream* retainedStream = streamForCodec.get();

    SkCodec::Result result;
    std::unique_ptr<SkCodec> codec = SkPngRustDecoder::Decode(std::move(streamForCodec), &result);
    REPORTER_ASSERT_SUCCESSFUL_CODEC_RESULT(r, result);
    if (!codec) {
        return;
    }

    SkBitmap bitmap;
    if (!bitmap.tryAllocN32Pixels(codec->dimensions().width(), codec->dimensions().height())) {
        ERRORF(r, "Failed to allocate SkBitmap");
        return;
    }
    const SkPixmap& pixmap = bitmap.pixmap();

    // Fill the `bitmap` with blue pixels to detect which pixels have been filled
    // by the codec during a partially-successful `incrementalDecode`.
    //
    // (The first frame has all red pixels.)
    bitmap.erase(SkColorSetRGB(0, 0, 0xFF), bitmap.bounds());
    AssertBluePixel(r, pixmap, 0, 0);
    AssertBluePixel(r, pixmap, 40, 29);
    AssertBluePixel(r, pixmap, 80, 35);
    AssertBluePixel(r, pixmap, 127, 63);

    // Decode partially-available, incomplete image.
    SkCodec::Options options;
    options.fZeroInitialized = SkCodec::kNo_ZeroInitialized;
    options.fSubset = nullptr;
    options.fFrameIndex = 0;
    options.fPriorFrame = SkCodec::kNoFrame;
    result = codec->startIncrementalDecode(bitmap.pixmap().info(),
                                           bitmap.pixmap().writable_addr(),
                                           bitmap.pixmap().rowBytes(),
                                           &options);
    REPORTER_ASSERT(r, result == SkCodec::kSuccess);
    int rowsDecoded = -1;
    result = codec->incrementalDecode(&rowsDecoded);
    REPORTER_ASSERT(r, result == SkCodec::kIncompleteInput);
    REPORTER_ASSERT(r, rowsDecoded == 10, "actual rowsDecoded = %d", rowsDecoded);
    AssertRedPixel(r, pixmap, 0, 0);
    AssertBluePixel(r, pixmap, 40, 29);
    AssertBluePixel(r, pixmap, 80, 35);
    AssertBluePixel(r, pixmap, 127, 63);

    // Make the rest of the input available to the codec.
    retainedStream->addNewData(fullLength);

    // Try to provoke the codec to consume further into the input stream (doing
    // this would loose the position inside the currently active incremental
    // decode).
    //
    // At this point metadata of all the frames is available, but the codec
    // shouldn't read the other two `fcTL` chunks during an active incremental
    // decode.
    int frameCount = codec->getFrameCount();
    REPORTER_ASSERT(r, frameCount == 1);

    // Check that all the pixels of the first frame got decoded.
    rowsDecoded = -1;
    result = codec->incrementalDecode(&rowsDecoded);
    REPORTER_ASSERT(r, result == SkCodec::kSuccess);
    REPORTER_ASSERT(r, rowsDecoded == -1);  // Not set when `kSuccess`.
    AssertRedPixel(r, pixmap, 0, 0);
    AssertRedPixel(r, pixmap, 40, 29);
    AssertRedPixel(r, pixmap, 80, 35);
    AssertRedPixel(r, pixmap, 127, 63);
}

// Test based on
// https://philip.html5.org/tests/apng/tests.html#apng-blend-op-source-on-solid-colour
DEF_TEST(RustPngCodec_apng_blend_ops_source_on_solid, r) {
    std::unique_ptr<SkCodec> codec =
            SkPngRustDecoderDecode(r, "images/apng-test-suite--blend-ops--source-on-solid.png");
    if (!codec) {
        return;
    }
    sk_sp<SkImage> image = DecodeLastFrame(r, codec.get());
    if (!image) {
        return;
    }

    SkPixmap pixmap;
    REPORTER_ASSERT(r, image->peekPixels(&pixmap));
    AssertGreenPixel(r, pixmap, 0, 0);

    SkCodec::FrameInfo info;
    REPORTER_ASSERT(r, codec->getFrameInfo(1, &info));
    REPORTER_ASSERT(r, info.fBlend == SkCodecAnimation::Blend::kSrc);
    REPORTER_ASSERT(r, info.fRequiredFrame == SkCodec::kNoFrame);
}

// Test based on
// https://philip.html5.org/tests/apng/tests.html#apng-blend-op-source-on-nearly-transparent-colour
DEF_TEST(RustPngCodec_apng_blend_ops_source_on_nearly_transparent, r) {
    sk_sp<SkImage> image = DecodeLastFrame(
            r, "images/apng-test-suite--blend-ops--source-on-nearly-transparent.png");
    if (!image) {
        return;
    }

    SkPixmap pixmap;
    REPORTER_ASSERT(r, image->peekPixels(&pixmap));
    AssertPixelColor(r,
                     pixmap,
                     0,
                     0,
                     SkColorSetARGB(0x02, 0x00, 0xFF, 0x00),
                     "Expecting a nearly transparent pixel");
}

// Test based on
// https://philip.html5.org/tests/apng/tests.html#apng-blend-op-over-on-solid-and-transparent-colours
DEF_TEST(RustPngCodec_apng_blend_ops_over_on_solid_and_transparent, r) {
    sk_sp<SkImage> image = DecodeLastFrame(
            r, "images/apng-test-suite--blend-ops--over-on-solid-and-transparent.png");
    if (!image) {
        return;
    }

    SkPixmap pixmap;
    REPORTER_ASSERT(r, image->peekPixels(&pixmap));
    AssertGreenPixel(r, pixmap, 0, 0);
}

// Test based on
// https://philip.html5.org/tests/apng/tests.html#apng-blend-op-over-repeatedly-with-nearly-transparent-colours
DEF_TEST(RustPngCodec_apng_blend_ops_over_repeatedly, r) {
    sk_sp<SkImage> image =
            DecodeLastFrame(r, "images/apng-test-suite--blend-ops--over-repeatedly.png");
    if (!image) {
        return;
    }

    SkPixmap pixmap;
    REPORTER_ASSERT(r, image->peekPixels(&pixmap));
    AssertGreenPixel(r, pixmap, 0, 0);
}

// Test based on
// https://philip.html5.org/tests/apng/tests.html#apng-dispose-op-none-in-region
DEF_TEST(RustPngCodec_apng_regions_dispose_op_none, r) {
    sk_sp<SkImage> image =
            DecodeLastFrame(r, "images/apng-test-suite--regions--dispose-op-none.png");
    if (!image) {
        return;
    }

    // Check all pixels.
    //
    // * The image (and the first frame) is 128x64
    // * The 2nd frame is 64x32 at offset (32,16)
    // * The 3rd frame is 1x1 at offset (0,0)
    SkPixmap pixmap;
    REPORTER_ASSERT(r, image->peekPixels(&pixmap));
    for (int y = 0; y < pixmap.height(); y++) {
        for (int x = 0; x < pixmap.width(); x++) {
            AssertGreenPixel(r, pixmap, x, y);
        }
    }
}

// Test based on
// https://philip.html5.org/tests/apng/tests.html#num-plays-0
DEF_TEST(RustPngCodec_apng_num_plays_0, r) {
    AssertAnimationRepetitionCount(
            r, SkCodec::kRepetitionCountInfinite, "images/apng-test-suite--num-plays--0.png");
}

// Test based on
// https://philip.html5.org/tests/apng/tests.html#num-plays-1
DEF_TEST(RustPngCodec_apng_num_plays_1, r) {
    AssertAnimationRepetitionCount(r, 0, "images/apng-test-suite--num-plays--1.png");
}

// Test based on
// https://philip.html5.org/tests/apng/tests.html#num-plays-2
DEF_TEST(RustPngCodec_apng_num_plays_2, r) {
    AssertAnimationRepetitionCount(r, 1, "images/apng-test-suite--num-plays--2.png");
}

// Test based on
// https://philip.html5.org/tests/apng/tests.html#num-frames-outside-valid-range
//
// In this test the `acTL` chunk sets `num_frames` to `2147483649u` (or `0x80000001u`):
//
// * AFAICT version 1.0 of the APNG spec only says that "0 is not a valid value"
// * The test suite webpage says that at one point the APNG spec said that
//   `num_frames` shall be "limited to the range 0 to (2^31)-1"
DEF_TEST(RustPngCodec_apng_invalid_num_frames_outside_valid_range, r) {
    std::unique_ptr<SkCodec> codec = SkPngRustDecoderDecode(
            r, "images/apng-test-suite--invalid--num-frames-outside-valid-range.png");
    if (!codec) {
        return;
    }

    // Calling `codec->getFrameCount` exercises the code used to discover and
    // parse `fcTL` chunks.  With the initial implementation of
    // `SkPngRustCodec::getRawFrameCount` the call below would have failed
    // `SkASSERT(fFrameAtCurrentStreamPosition < this->getRawFrameCount())`
    // in `SkPngRustCodec::readToStartOfNextFrame`.
    //
    // Note that `SkPngRustCodec::onGetFrameCount` expectedly returns the number
    // of successfully parsed `fcTL` chunks (1 chunk in the test input) rather
    // than returning the raw `acTL.num_frames`.
    REPORTER_ASSERT(r, codec->getFrameCount() == 1);
}

DEF_TEST(RustPngCodec_png_swizzling_target_unimplemented, r) {
    std::unique_ptr<SkCodec> codec =
            SkPngRustDecoderDecode(r, "images/apng-test-suite--basic--ignoring-default-image.png");
    if (!codec) {
        return;
    }
    REPORTER_ASSERT(r, codec->getFrameCount() == 1);

    // Ask to decode into an esoteric `SkColorType`:
    //
    // * Unsupported by `SkSwizzler`.
    // * Supported by `SkCodec::conversionSupported`.
    SkImageInfo dstInfo = SkImageInfo::Make(
            codec->dimensions(), kBGRA_10101010_XR_SkColorType, kPremul_SkAlphaType);

    auto [image, result] = codec->getImage(dstInfo);
    REPORTER_ASSERT(r, result == SkCodec::kUnimplemented);
    REPORTER_ASSERT(r, !image);
}

DEF_TEST(RustPngCodec_png_was_encoded_with_16_bits_or_more_per_component, r) {
    struct Test {
        const char* fFilename;
        bool fEncodedWith16bits;
    };
    const std::array<Test, 4> kTests = {
            Test{"images/pngsuite/basn0g04.png", false},  // 4 bit (16 level) grayscale
            Test{"images/pngsuite/basn2c08.png", false},  // 3x8 bits rgb color
            Test{"images/pngsuite/basn2c16.png", true},   // 3x16 bits rgb color
            Test{"images/pngsuite/basn3p01.png", false}   // 1 bit (2 color) paletted
    };
    for (const auto& test : kTests) {
        std::unique_ptr<SkCodec> codec = SkPngRustDecoderDecode(r, test.fFilename);
        if (codec) {
            REPORTER_ASSERT(r, codec->hasHighBitDepthEncodedData() == test.fEncodedWith16bits);
        }
    }
}

DEF_TEST(RustPngCodec_png_cicp, r) {
    std::unique_ptr<SkCodec> codec = SkPngRustDecoderDecode(r, "images/cicp_pq.png");
    if (!codec) {
        return;
    }

    const skcms_ICCProfile* profile = codec->getICCProfile();
    REPORTER_ASSERT(r, profile);
    if (!profile) {
        return;
    }
    auto cs = SkColorSpace::Make(*profile);
    skcms_TransferFunction tf;
    cs->transferFn(&tf);

    REPORTER_ASSERT(r, skcms_TransferFunction_isPQish(&tf) ||
                       skcms_TransferFunction_isPQ(&tf));
}

DEF_TEST(RustPngCodec_green15x15, r) {
    std::unique_ptr<SkCodec> codec = SkPngRustDecoderDecode(r, "images/green15x15.png");
    if (!codec) {
        return;
    }

    SkImageInfo dstInfo = codec->getInfo();
    dstInfo = dstInfo.makeColorSpace(SkColorSpace::MakeSRGB());
    auto [image, result] = codec->getImage(dstInfo);
    REPORTER_ASSERT_SUCCESSFUL_CODEC_RESULT(r, result);
    if (result != SkCodec::kSuccess) {
        return;
    }

    SkPixmap pixmap;
    REPORTER_ASSERT(r, image->peekPixels(&pixmap));
    const SkColor kExpectedColor = SkColorSetARGB(0xFF, 0x00, 0x80, 0x00);
    AssertPixelColor(r, pixmap, 0, 0, kExpectedColor, "Expecting a dark green pixel");
}

DEF_TEST(RustPngCodec_exif_orientation, r) {
    std::unique_ptr<SkCodec> codec = SkPngRustDecoderDecode(r, "images/F-exif-chunk-early.png");
    if (!codec) {
        return;
    }

    REPORTER_ASSERT(r, codec->getOrigin() == kRightTop_SkEncodedOrigin);
}

DEF_TEST(RustPngCodec_f16_trc_tables, r) {
    std::unique_ptr<SkCodec> codec = SkPngRustDecoderDecode(r, "images/f16-trc-tables.png");
    REPORTER_ASSERT(r, codec);

    const SkImageInfo info = codec->getInfo();
    REPORTER_ASSERT(r, info.colorSpace());

    // Decoding to F16 without color space conversion.
    const SkImageInfo dstInfo = info.makeColorType(kRGBA_F16_SkColorType)
                                    .makeColorSpace(nullptr);
    // This should not crash.
    auto [image, result] = codec->getImage(dstInfo);
    REPORTER_ASSERT_SUCCESSFUL_CODEC_RESULT(r, result);
}

DEF_TEST(RustPngCodec_crbug445556737, r) {
    sk_sp<SkImage> image = DecodeLastFrame(r, "images/crbug445556737.png");
    if (!image) {
        return;
    }

    // The main test verification is that there are no assertion failures nor
    // other crashes.  Cursory verification below is supplementary/secondary.
    REPORTER_ASSERT(r, image->height() == 5);
    REPORTER_ASSERT(r, image->width() == 5);
}

DEF_TEST(RustPngCodec_invalid_profile, r) {
    // This image has an gamma value of 0. For parity with Blink, we want to disregard
    // the ICC profile in this case and create the codec without it. This is different
    // than libpng SkPngCodec behavior, which will default to an SRGB icc profile.
    std::unique_ptr<SkCodec> codec =
        SkPngRustDecoderDecode(r, "images/png-zero-gamma-color-profile.png");
    REPORTER_ASSERT(r, codec);

    // There should be no ICC profile.
    REPORTER_ASSERT(r, !codec->getICCProfile());

    // This should not crash.
    auto [image, result] = codec->getImage(codec->getInfo());
    REPORTER_ASSERT_SUCCESSFUL_CODEC_RESULT(r, result);
}

static bool bitmaps_equal(const SkBitmap& actual, const SkBitmap& expected) {
    for (int y = 0; y < actual.height(); ++y) {
        for (int x = 0; x < actual.width(); ++x) {
            SkColor c1 = actual.getColor(x, y);
            SkColor c2 = expected.getColor(x, y);
            SkPMColor actualPMColor = SkPreMultiplyColor(c1);
            SkPMColor expectedPMColor = SkPreMultiplyColor(c2);

            if (actualPMColor != expectedPMColor) {
                return false;
            }
        }
    }
    return true;
}

static void test_subset_decode(skiatest::Reporter* r, const char* resource) {
    skiatest::ReporterContext context(r, resource);
    std::unique_ptr<SkStream> stream(GetResourceAsStream(resource));
    REPORTER_ASSERT(r, stream);

    std::unique_ptr<SkCodec> codec = SkCodec::MakeFromStream(std::move(stream));
    REPORTER_ASSERT(r, codec);

    const SkImageInfo info = codec->getInfo();

    SkBitmap bm;
    bm.allocPixels(info);
    codec->getPixels(info, bm.getPixels(), bm.rowBytes());

    SkBitmap tiledBM;
    tiledBM.allocPixels(info);

    const int height = info.height();
    const int width = info.width();
    // Note that if numStripes does not evenly divide height there will be an extra
    // stripe.
    const int numStripes = 4;
    const int numVerticalStripes = 2;

    if (numStripes > height || numVerticalStripes > width) {
        // Image is too small.
        return;
    }

    const int stripeHeight = height / numStripes;
    const int stripeWidth = width / numVerticalStripes;

    // Iterate through the image twice. Once to decode odd stripes, and once for even.
    for (int oddEven = 1; oddEven >= 0; oddEven--) {
        for (int y = oddEven * stripeHeight; y < height; y += 2 * stripeHeight) {
            for (int xStripe = 0; xStripe < numVerticalStripes; xStripe++) {
                // Calculate all four bounds for the grid section
                const int top = y;
                const int bottom = std::min(y + stripeHeight, height);
                const int left = xStripe * stripeWidth;
                const int right = std::min((xStripe + 1) * stripeWidth, width);

                SkIRect subset = SkIRect::MakeLTRB(left, top, right, bottom);
                SkCodec::Options options;
                options.fSubset = &subset;

                REPORTER_ASSERT(r, SkCodec::kSuccess == codec->startIncrementalDecode(
                    info, tiledBM.getAddr(left, top), tiledBM.rowBytes(), &options));
                REPORTER_ASSERT(r, SkCodec::kSuccess == codec->incrementalDecode());
            }
        }
    }

    REPORTER_ASSERT(r, bitmaps_equal(bm, tiledBM));
}

DEF_TEST(RustPngCodec_subset, r) {
    // Tests subsets by splitting the image into 8 or 10 different tiles and decoding
    // those each separately, then comparing to the full image decoded.
    test_subset_decode(r, "images/baby_tux.png");
    test_subset_decode(r, "images/plane_interlaced.png");
}

DEF_TEST(RustPngCodec_interlaced_animated_blending, r) {
    std::unique_ptr<SkCodec> codec =
        SkPngRustDecoderDecode(r, "images/interlaced-multiframe-with-blending.png");
    REPORTER_ASSERT(r, codec);

    // Use incrementalDecode for each frame of this image. This should not crash.
    SkBitmap bm;
    SkImageInfo info = codec->getInfo();
    bm.allocPixels(info);
    REPORTER_ASSERT(r, codec->getFrameCount() == 4);
    for (int i = 0; i < codec->getFrameCount(); ++i) {
        SkCodec::Options options;
        options.fFrameIndex = i;
        options.fPriorFrame = i - 1;
        SkCodec::Result result;
        result = codec->startIncrementalDecode(info, bm.getPixels(), bm.rowBytes(), &options);
        REPORTER_ASSERT_SUCCESSFUL_CODEC_RESULT(r, result);
        codec->incrementalDecode();
        REPORTER_ASSERT_SUCCESSFUL_CODEC_RESULT(r, result);
    }
}
