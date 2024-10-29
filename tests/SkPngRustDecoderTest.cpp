/*
 * Copyright 2024 Google LLC.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "experimental/rust_png/SkPngRustDecoder.h"
#include "include/codec/SkCodec.h"
#include "include/codec/SkCodecAnimation.h"
#include "include/core/SkColor.h"
#include "include/core/SkData.h"
#include "include/core/SkImage.h"
#include "include/core/SkImageInfo.h"
#include "include/core/SkPixmap.h"
#include "include/core/SkRefCnt.h"
#include "include/core/SkStream.h"
#include "tests/Test.h"
#include "tools/Resources.h"

#include <memory>
#include <utility>

#define REPORTER_ASSERT_SUCCESSFUL_CODEC_RESULT(r, actualResult) \
    REPORTER_ASSERT(r,                                           \
                    actualResult == SkCodec::kSuccess,           \
                    "actualResult=\"%s\" != kSuccess",           \
                    SkCodec::ResultToString(actualResult))

// Helper wrapping a call to `SkPngRustDecoder::Decode`.
std::unique_ptr<SkCodec> SkPngRustDecoderDecode(skiatest::Reporter* r, const char* path) {
    sk_sp<SkData> data(GetResourceAsData(path));
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
                      const SkImage& image,
                      int x,
                      int y,
                      SkColor expectedColor,
                      const char* description) {
    SkASSERT(r);
    SkASSERT(x >= 0);
    SkASSERT(y >= 0);
    SkASSERT(description);

    REPORTER_ASSERT(r, x < image.width(), "x=%d >= width=%d", x, image.width());
    REPORTER_ASSERT(r, y < image.height(), "y=%d >= height=%d", y, image.height());
    REPORTER_ASSERT(r,
                    kN32_SkColorType == image.colorType(),
                    "kN32_SkColorType != image.ColorType()=%d",
                    image.colorType());

    SkPixmap pixmap;
    REPORTER_ASSERT(r, image.peekPixels(&pixmap));

    SkColor actualColor = pixmap.getColor(x, y);
    REPORTER_ASSERT(r,
                    actualColor == expectedColor,
                    "actualColor=0x%08X != expectedColor==0x%08X (%s)",
                    actualColor,
                    expectedColor,
                    description);
}

void AssertGreenPixel(skiatest::Reporter* r,
                      const SkImage& image,
                      int x,
                      int y,
                      const char* description = "Expecting a green pixel") {
    AssertPixelColor(r, image, x, y, SkColorSetRGB(0x00, 0xFF, 0x00), description);
}

void AssertRedPixel(skiatest::Reporter* r,
                    const SkImage& image,
                    int x,
                    int y,
                    const char* description = "Expecting a red pixel") {
    AssertPixelColor(r, image, x, y, SkColorSetRGB(0xFF, 0x00, 0x00), description);
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

    AssertGreenPixel(r, *image, 0, 0);
    AssertGreenPixel(r, *image, expectedWidth / 2, expectedHeight / 2);
}

sk_sp<SkImage> DecodeLastFrame(skiatest::Reporter* r, const char* resourcePath) {
    std::unique_ptr<SkCodec> codec = SkPngRustDecoderDecode(r, resourcePath);
    if (!codec) {
        return nullptr;
    }

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
DEF_TEST(Codec_apng_basic_trivial_static_image, r) {
    AssertSingleGreenFrame(r, 128, 64, "images/apng-test-suite--basic--trivial-static-image.png");
}

// Test based on
// https://philip.html5.org/tests/apng/tests.html#trivial-animated-image-one-frame-using-default-image
DEF_TEST(Codec_apng_basic_using_default_image, r) {
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
DEF_TEST(Codec_apng_basic_ignoring_default_image, r) {
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
// * TODO(https://crbug.com/356922876): Skia client (e.g. Blink; or here the
//   testcase) is expected to handle `SkCodecAnimation::DisposalMethod` and
//   populate the target buffer (and `SkCodec::Options::fPriorFrame`) with the
//   expected pixels.  OTOH, `SkPngRustCodec` needs to handle
//   `SkCodecAnimation::Blend` - without this the final frame in this test will
//   contain red pixels.
DEF_TEST(Codec_apng_dispose_op_none_basic, r) {
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
    AssertRedPixel(r, *image, 0, 0, "Frame #0 should be red");

    // Validate contents of the second frame.
    SkPixmap pixmap;
    REPORTER_ASSERT(r, image->peekPixels(&pixmap));
    SkCodec::Options options;
    options.fZeroInitialized = SkCodec::kNo_ZeroInitialized;
    options.fSubset = nullptr;
    options.fFrameIndex = 1;  // We want to decode the second frame.
    options.fPriorFrame = 0;  // `pixmap` contains the first frame before `getPixels` call.
    REPORTER_ASSERT_SUCCESSFUL_CODEC_RESULT(r, codec->getPixels(pixmap, &options));
    AssertGreenPixel(r, *image, 0, 0, "Frame #1 should be green");

    // Validate contents of the third frame.
    options.fFrameIndex = 2;  // We want to decode the second frame.
    options.fPriorFrame = 1;  // `pixmap` contains the second frame before `getPixels` call.
    REPORTER_ASSERT_SUCCESSFUL_CODEC_RESULT(r, codec->getPixels(pixmap, &options));
    AssertGreenPixel(r, *image, 0, 0, "Frame #2 should be green");
}

// Test based on
// https://philip.html5.org/tests/apng/tests.html#apng-blend-op-source-on-solid-colour
DEF_TEST(Codec_apng_blend_ops_source_on_solid, r) {
    sk_sp<SkImage> image =
            DecodeLastFrame(r, "images/apng-test-suite--blend-ops--source-on-solid.png");
    if (!image) {
        return;
    }

    AssertGreenPixel(r, *image, 0, 0);
}

// Test based on
// https://philip.html5.org/tests/apng/tests.html#apng-blend-op-source-on-transparent-colour
DEF_TEST(Codec_apng_blend_ops_source_on_transparent, r) {
    sk_sp<SkImage> image =
            DecodeLastFrame(r, "images/apng-test-suite--blend-ops--source-on-transparent.png");
    if (!image) {
        return;
    }

    AssertPixelColor(r,
                     *image,
                     0,
                     0,
                     SkColorSetARGB(0x00, 0x00, 0x00, 0x00),
                     "Expecting a transparent pixel");
}

// Test based on
// https://philip.html5.org/tests/apng/tests.html#apng-blend-op-source-on-nearly-transparent-colour
DEF_TEST(Codec_apng_blend_ops_source_on_nearly_transparent, r) {
    sk_sp<SkImage> image = DecodeLastFrame(
            r, "images/apng-test-suite--blend-ops--source-on-nearly-transparent.png");
    if (!image) {
        return;
    }

    AssertPixelColor(r,
                     *image,
                     0,
                     0,
                     SkColorSetARGB(0x02, 0x00, 0xFF, 0x00),
                     "Expecting a nearly transparent pixel");
}

// Test based on
// https://philip.html5.org/tests/apng/tests.html#apng-blend-op-over-on-solid-and-transparent-colours
DEF_TEST(Codec_apng_blend_ops_over_on_solid_and_transparent, r) {
    sk_sp<SkImage> image = DecodeLastFrame(
            r, "images/apng-test-suite--blend-ops--over-on-solid-and-transparent.png");
    if (!image) {
        return;
    }

    AssertGreenPixel(r, *image, 0, 0);
}

// Test based on
// https://philip.html5.org/tests/apng/tests.html#apng-blend-op-over-repeatedly-with-nearly-transparent-colours
DEF_TEST(Codec_apng_blend_ops_over_repeatedly, r) {
    sk_sp<SkImage> image =
            DecodeLastFrame(r, "images/apng-test-suite--blend-ops--over-repeatedly.png");
    if (!image) {
        return;
    }

    AssertGreenPixel(r, *image, 0, 0);
}

// Test based on
// https://philip.html5.org/tests/apng/tests.html#apng-dispose-op-none-in-region
DEF_TEST(Codec_apng_regions_dispose_op_none, r) {
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
    for (int y = 0; y < image->height(); y++) {
        for (int x = 0; x < image->width(); x++) {
            AssertGreenPixel(r, *image, x, y);
        }
    }
}

// Test based on
// https://philip.html5.org/tests/apng/tests.html#num-plays-0
DEF_TEST(Codec_apng_num_plays_0, r) {
    AssertAnimationRepetitionCount(
            r, SkCodec::kRepetitionCountInfinite, "images/apng-test-suite--num-plays--0.png");
}

// Test based on
// https://philip.html5.org/tests/apng/tests.html#num-plays-1
DEF_TEST(Codec_apng_num_plays_1, r) {
    AssertAnimationRepetitionCount(r, 0, "images/apng-test-suite--num-plays--1.png");
}

// Test based on
// https://philip.html5.org/tests/apng/tests.html#num-plays-2
DEF_TEST(Codec_apng_num_plays_2, r) {
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
DEF_TEST(Codec_apng_invalid_num_frames_outside_valid_range, r) {
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

DEF_TEST(Codec_png_swizzling_target_unimplemented, r) {
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
