/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "Resources.h"
#include "SkBitmap.h"
#include "SkCodec.h"
#include "SkMD5.h"
#include "SkRandom.h"
#include "SkScaledCodec.h"
#include "Test.h"

static SkStreamAsset* resource(const char path[]) {
    SkString fullPath = GetResourcePath(path);
    return SkStream::NewFromFile(fullPath.c_str());
}

static void md5(const SkBitmap& bm, SkMD5::Digest* digest) {
    SkAutoLockPixels autoLockPixels(bm);
    SkASSERT(bm.getPixels());
    SkMD5 md5;
    size_t rowLen = bm.info().bytesPerPixel() * bm.width();
    for (int y = 0; y < bm.height(); ++y) {
        md5.update(static_cast<uint8_t*>(bm.getAddr(0, y)), rowLen);
    }
    md5.finish(*digest);
}

/**
 *  Compute the digest for bm and compare it to a known good digest.
 *  @param r Reporter to assert that bm's digest matches goodDigest.
 *  @param goodDigest The known good digest to compare to.
 *  @param bm The bitmap to test.
 */
static void compare_to_good_digest(skiatest::Reporter* r, const SkMD5::Digest& goodDigest,
                           const SkBitmap& bm) {
    SkMD5::Digest digest;
    md5(bm, &digest);
    REPORTER_ASSERT(r, digest == goodDigest);
}

/**
 *  Test decoding an SkCodec to a particular SkImageInfo.
 *
 *  Calling getPixels(info) should return expectedResult, and if goodDigest is non nullptr,
 *  the resulting decode should match.
 */
static void test_info(skiatest::Reporter* r, SkCodec* codec, const SkImageInfo& info,
                      SkCodec::Result expectedResult, const SkMD5::Digest* goodDigest) {
    SkBitmap bm;
    bm.allocPixels(info);
    SkAutoLockPixels autoLockPixels(bm);

    SkCodec::Result result = codec->getPixels(info, bm.getPixels(), bm.rowBytes());
    REPORTER_ASSERT(r, result == expectedResult);

    if (goodDigest) {
        compare_to_good_digest(r, *goodDigest, bm);
    }
}

SkIRect generate_random_subset(SkRandom* rand, int w, int h) {
    SkIRect rect;
    do {
        rect.fLeft = rand->nextRangeU(0, w);
        rect.fTop = rand->nextRangeU(0, h);
        rect.fRight = rand->nextRangeU(0, w);
        rect.fBottom = rand->nextRangeU(0, h);
        rect.sort();
    } while (rect.isEmpty());
    return rect;
}

static void test_codec(skiatest::Reporter* r, SkCodec* codec, SkBitmap& bm, const SkImageInfo& info,
        const SkISize& size, bool supports565, SkMD5::Digest* digest,
        const SkMD5::Digest* goodDigest) {
    REPORTER_ASSERT(r, info.dimensions() == size);
    bm.allocPixels(info);
    SkAutoLockPixels autoLockPixels(bm);

    SkCodec::Result result = codec->getPixels(info, bm.getPixels(), bm.rowBytes());
    REPORTER_ASSERT(r, result == SkCodec::kSuccess);

    md5(bm, digest);
    if (goodDigest) {
        REPORTER_ASSERT(r, *digest == *goodDigest);
    }

    {
        // Test decoding to 565
        SkImageInfo info565 = info.makeColorType(kRGB_565_SkColorType);
        SkCodec::Result expected = (supports565 && info.alphaType() == kOpaque_SkAlphaType) ?
                SkCodec::kSuccess : SkCodec::kInvalidConversion;
        test_info(r, codec, info565, expected, nullptr);
    }

    // Verify that re-decoding gives the same result.  It is interesting to check this after
    // a decode to 565, since choosing to decode to 565 may result in some of the decode
    // options being modified.  These options should return to their defaults on another
    // decode to kN32, so the new digest should match the old digest.
    test_info(r, codec, info, SkCodec::kSuccess, digest);

    {
        // Check alpha type conversions
        if (info.alphaType() == kOpaque_SkAlphaType) {
            test_info(r, codec, info.makeAlphaType(kUnpremul_SkAlphaType),
                      SkCodec::kInvalidConversion, nullptr);
            test_info(r, codec, info.makeAlphaType(kPremul_SkAlphaType),
                      SkCodec::kInvalidConversion, nullptr);
        } else {
            // Decoding to opaque should fail
            test_info(r, codec, info.makeAlphaType(kOpaque_SkAlphaType),
                      SkCodec::kInvalidConversion, nullptr);
            SkAlphaType otherAt = info.alphaType();
            if (kPremul_SkAlphaType == otherAt) {
                otherAt = kUnpremul_SkAlphaType;
            } else {
                otherAt = kPremul_SkAlphaType;
            }
            // The other non-opaque alpha type should always succeed, but not match.
            test_info(r, codec, info.makeAlphaType(otherAt), SkCodec::kSuccess, nullptr);
        }
    }
}

static void check(skiatest::Reporter* r,
                  const char path[],
                  SkISize size,
                  bool supportsScanlineDecoding,
                  bool supportsSubsetDecoding,
                  bool supports565 = true) {

    SkAutoTDelete<SkStream> stream(resource(path));
    if (!stream) {
        SkDebugf("Missing resource '%s'\n", path);
        return;
    }
    SkAutoTDelete<SkCodec> codec(SkCodec::NewFromStream(stream.detach()));
    if (!codec) {
        ERRORF(r, "Unable to decode '%s'", path);
        return;
    }

    // Test full image decodes with SkCodec
    SkMD5::Digest codecDigest;
    SkImageInfo info = codec->getInfo().makeColorType(kN32_SkColorType);
    SkBitmap bm;
    test_codec(r, codec, bm, info, size, supports565, &codecDigest, nullptr);

    // Scanline decoding follows.
    // Need to call startScanlineDecode() first.
    REPORTER_ASSERT(r, codec->getScanlines(bm.getAddr(0, 0), 1, 0)
            == SkCodec::kScanlineDecodingNotStarted);
    REPORTER_ASSERT(r, codec->skipScanlines(1)
            == SkCodec::kScanlineDecodingNotStarted);

    const SkCodec::Result startResult = codec->startScanlineDecode(info);
    if (supportsScanlineDecoding) {
        bm.eraseColor(SK_ColorYELLOW);

        REPORTER_ASSERT(r, startResult == SkCodec::kSuccess);

        for (int y = 0; y < info.height(); y++) {
            SkCodec::Result result = codec->getScanlines(bm.getAddr(0, y), 1, 0);
            REPORTER_ASSERT(r, result == SkCodec::kSuccess);
        }
        // verify that scanline decoding gives the same result.
        if (SkCodec::kTopDown_SkScanlineOrder == codec->getScanlineOrder()) {
            compare_to_good_digest(r, codecDigest, bm);
        }

        // Cannot continue to decode scanlines beyond the end
        REPORTER_ASSERT(r, codec->getScanlines(bm.getAddr(0, 0), 1, 0)
                == SkCodec::kInvalidParameters);

        // Interrupting a scanline decode with a full decode starts from
        // scratch
        REPORTER_ASSERT(r, codec->startScanlineDecode(info) == SkCodec::kSuccess);
        REPORTER_ASSERT(r, codec->getScanlines(bm.getAddr(0, 0), 1, 0)
                == SkCodec::kSuccess);
        REPORTER_ASSERT(r, codec->getPixels(bm.info(), bm.getPixels(), bm.rowBytes())
                == SkCodec::kSuccess);
        REPORTER_ASSERT(r, codec->getScanlines(bm.getAddr(0, 0), 1, 0)
                == SkCodec::kScanlineDecodingNotStarted);
        REPORTER_ASSERT(r, codec->skipScanlines(1)
                == SkCodec::kScanlineDecodingNotStarted);
    } else {
        REPORTER_ASSERT(r, startResult == SkCodec::kUnimplemented);
    }

    // The rest of this function tests decoding subsets, and will decode an arbitrary number of
    // random subsets.
    // Do not attempt to decode subsets of an image of only once pixel, since there is no
    // meaningful subset.
    if (size.width() * size.height() == 1) {
        return;
    }

    SkRandom rand;
    SkIRect subset;
    SkCodec::Options opts;
    opts.fSubset = &subset;
    for (int i = 0; i < 5; i++) {
        subset = generate_random_subset(&rand, size.width(), size.height());
        SkASSERT(!subset.isEmpty());
        const bool supported = codec->getValidSubset(&subset);
        REPORTER_ASSERT(r, supported == supportsSubsetDecoding);

        SkImageInfo subsetInfo = info.makeWH(subset.width(), subset.height());
        SkBitmap bm;
        bm.allocPixels(subsetInfo);
        const SkCodec::Result result = codec->getPixels(bm.info(), bm.getPixels(), bm.rowBytes(),
                                                        &opts, nullptr, nullptr);

        if (supportsSubsetDecoding) {
            REPORTER_ASSERT(r, result == SkCodec::kSuccess);
            // Webp is the only codec that supports subsets, and it will have modified the subset
            // to have even left/top.
            REPORTER_ASSERT(r, SkIsAlign2(subset.fLeft) && SkIsAlign2(subset.fTop));
        } else {
            // No subsets will work.
            REPORTER_ASSERT(r, result == SkCodec::kUnimplemented);
        }
    }

    // SkScaledCodec tests
    if (supportsScanlineDecoding || supportsSubsetDecoding){
        SkAutoTDelete<SkStream> stream(resource(path));
        if (!stream) {
            SkDebugf("Missing resource '%s'\n", path);
            return;
        }
        SkAutoTDelete<SkCodec> codec(SkScaledCodec::NewFromStream(stream.detach()));
        if (!codec) {
            ERRORF(r, "Unable to decode '%s'", path);
            return;
        }

        SkBitmap bm;
        SkMD5::Digest scaledCodecDigest;
        test_codec(r, codec, bm, info, size, supports565, &scaledCodecDigest, &codecDigest);
    }
}

DEF_TEST(Codec, r) {
    // WBMP
    check(r, "mandrill.wbmp", SkISize::Make(512, 512), true, false);

    // WEBP
    check(r, "baby_tux.webp", SkISize::Make(386, 395), false, true);
    check(r, "color_wheel.webp", SkISize::Make(128, 128), false, true);
    check(r, "yellow_rose.webp", SkISize::Make(400, 301), false, true);

    // BMP
    check(r, "randPixels.bmp", SkISize::Make(8, 8), true, false);

    // ICO
    // These two tests examine interestingly different behavior:
    // Decodes an embedded BMP image
    check(r, "color_wheel.ico", SkISize::Make(128, 128), false, false);
    // Decodes an embedded PNG image
    check(r, "google_chrome.ico", SkISize::Make(256, 256), false, false);

    // GIF
    check(r, "box.gif", SkISize::Make(200, 55), true, false);
    check(r, "color_wheel.gif", SkISize::Make(128, 128), true, false);
    check(r, "randPixels.gif", SkISize::Make(8, 8), true, false);

    // JPG
    check(r, "CMYK.jpg", SkISize::Make(642, 516), true, false, false);
    check(r, "color_wheel.jpg", SkISize::Make(128, 128), true, false);
    check(r, "grayscale.jpg", SkISize::Make(128, 128), true, false);
    check(r, "mandrill_512_q075.jpg", SkISize::Make(512, 512), true, false);
    check(r, "randPixels.jpg", SkISize::Make(8, 8), true, false);

    // PNG
    check(r, "arrow.png", SkISize::Make(187, 312), true, false);
    check(r, "baby_tux.png", SkISize::Make(240, 246), true, false);
    check(r, "color_wheel.png", SkISize::Make(128, 128), true, false);
    check(r, "half-transparent-white-pixel.png", SkISize::Make(1, 1), true, false);
    check(r, "mandrill_128.png", SkISize::Make(128, 128), true, false);
    check(r, "mandrill_16.png", SkISize::Make(16, 16), true, false);
    check(r, "mandrill_256.png", SkISize::Make(256, 256), true, false);
    check(r, "mandrill_32.png", SkISize::Make(32, 32), true, false);
    check(r, "mandrill_512.png", SkISize::Make(512, 512), true, false);
    check(r, "mandrill_64.png", SkISize::Make(64, 64), true, false);
    check(r, "plane.png", SkISize::Make(250, 126), true, false);
    check(r, "plane_interlaced.png", SkISize::Make(250, 126), true, false);
    check(r, "randPixels.png", SkISize::Make(8, 8), true, false);
    check(r, "yellow_rose.png", SkISize::Make(400, 301), true, false);
}

// Test interlaced PNG in stripes, similar to DM's kStripe_Mode
DEF_TEST(Codec_stripes, r) {
    const char * path = "plane_interlaced.png";
    SkAutoTDelete<SkStream> stream(resource(path));
    if (!stream) {
        SkDebugf("Missing resource '%s'\n", path);
    }

    SkAutoTDelete<SkCodec> codec(SkCodec::NewFromStream(stream.detach()));
    REPORTER_ASSERT(r, codec);

    if (!codec) {
        return;
    }

    switch (codec->getScanlineOrder()) {
        case SkCodec::kBottomUp_SkScanlineOrder:
        case SkCodec::kOutOfOrder_SkScanlineOrder:
            ERRORF(r, "This scanline order will not match the original.");
            return;
        default:
            break;
    }

    // Baseline for what the image should look like, using N32.
    const SkImageInfo info = codec->getInfo().makeColorType(kN32_SkColorType);

    SkBitmap bm;
    bm.allocPixels(info);
    SkAutoLockPixels autoLockPixels(bm);
    SkCodec::Result result = codec->getPixels(info, bm.getPixels(), bm.rowBytes());
    REPORTER_ASSERT(r, result == SkCodec::kSuccess);

    SkMD5::Digest digest;
    md5(bm, &digest);

    // Now decode in stripes
    const int height = info.height();
    const int numStripes = 4;
    int stripeHeight;
    int remainingLines;
    SkTDivMod(height, numStripes, &stripeHeight, &remainingLines);

    bm.eraseColor(SK_ColorYELLOW);

    result = codec->startScanlineDecode(info);
    REPORTER_ASSERT(r, result == SkCodec::kSuccess);

    // Odd stripes
    for (int i = 1; i < numStripes; i += 2) {
        // Skip the even stripes
        result = codec->skipScanlines(stripeHeight);
        REPORTER_ASSERT(r, result == SkCodec::kSuccess);

        result = codec->getScanlines(bm.getAddr(0, i * stripeHeight), stripeHeight,
                                     bm.rowBytes());
        REPORTER_ASSERT(r, result == SkCodec::kSuccess);
    }

    // Even stripes
    result = codec->startScanlineDecode(info);
    REPORTER_ASSERT(r, result == SkCodec::kSuccess);

    for (int i = 0; i < numStripes; i += 2) {
        result = codec->getScanlines(bm.getAddr(0, i * stripeHeight), stripeHeight,
                                     bm.rowBytes());
        REPORTER_ASSERT(r, result == SkCodec::kSuccess);

        // Skip the odd stripes
        if (i + 1 < numStripes) {
            result = codec->skipScanlines(stripeHeight);
            REPORTER_ASSERT(r, result == SkCodec::kSuccess);
        }
    }

    // Remainder at the end
    if (remainingLines > 0) {
        result = codec->startScanlineDecode(info);
        REPORTER_ASSERT(r, result == SkCodec::kSuccess);

        result = codec->skipScanlines(height - remainingLines);
        REPORTER_ASSERT(r, result == SkCodec::kSuccess);

        result = codec->getScanlines(bm.getAddr(0, height - remainingLines),
                                     remainingLines, bm.rowBytes());
        REPORTER_ASSERT(r, result == SkCodec::kSuccess);
    }

    compare_to_good_digest(r, digest, bm);
}

static void test_invalid_stream(skiatest::Reporter* r, const void* stream, size_t len) {
    SkCodec* codec = SkCodec::NewFromStream(new SkMemoryStream(stream, len, false));
    // We should not have gotten a codec. Bots should catch us if we leaked anything.
    REPORTER_ASSERT(r, !codec);
}

// Ensure that SkCodec::NewFromStream handles freeing the passed in SkStream,
// even on failure. Test some bad streams.
DEF_TEST(Codec_leaks, r) {
    // No codec should claim this as their format, so this tests SkCodec::NewFromStream.
    const char nonSupportedStream[] = "hello world";
    // The other strings should look like the beginning of a file type, so we'll call some
    // internal version of NewFromStream, which must also delete the stream on failure.
    const unsigned char emptyPng[] = { 0x89, 0x50, 0x4e, 0x47, 0x0d, 0x0a, 0x1a, 0x0a };
    const unsigned char emptyJpeg[] = { 0xFF, 0xD8, 0xFF };
    const char emptyWebp[] = "RIFF1234WEBPVP";
    const char emptyBmp[] = { 'B', 'M' };
    const char emptyIco[] = { '\x00', '\x00', '\x01', '\x00' };
    const char emptyGif[] = "GIFVER";

    test_invalid_stream(r, nonSupportedStream, sizeof(nonSupportedStream));
    test_invalid_stream(r, emptyPng, sizeof(emptyPng));
    test_invalid_stream(r, emptyJpeg, sizeof(emptyJpeg));
    test_invalid_stream(r, emptyWebp, sizeof(emptyWebp));
    test_invalid_stream(r, emptyBmp, sizeof(emptyBmp));
    test_invalid_stream(r, emptyIco, sizeof(emptyIco));
    test_invalid_stream(r, emptyGif, sizeof(emptyGif));
}

static void test_dimensions(skiatest::Reporter* r, const char path[]) {
    // Create the codec from the resource file
    SkAutoTDelete<SkStream> stream(resource(path));
    if (!stream) {
        SkDebugf("Missing resource '%s'\n", path);
        return;
    }
    SkAutoTDelete<SkCodec> codec(SkScaledCodec::NewFromStream(stream.detach()));
    if (!codec) {
        ERRORF(r, "Unable to create codec '%s'", path);
        return;
    }

    // Check that the decode is successful for a variety of scales
    for (float scale = 0.05f; scale < 2.0f; scale += 0.05f) {
        // Scale the output dimensions
        SkISize scaledDims = codec->getScaledDimensions(scale);
        SkImageInfo scaledInfo = codec->getInfo()
                .makeWH(scaledDims.width(), scaledDims.height())
                .makeColorType(kN32_SkColorType);

        // Set up for the decode
        size_t rowBytes = scaledDims.width() * sizeof(SkPMColor);
        size_t totalBytes = scaledInfo.getSafeSize(rowBytes);
        SkAutoTMalloc<SkPMColor> pixels(totalBytes);

        SkCodec::Result result =
                codec->getPixels(scaledInfo, pixels.get(), rowBytes, nullptr, nullptr, nullptr);
        REPORTER_ASSERT(r, SkCodec::kSuccess == result);
    }
}

// Ensure that onGetScaledDimensions returns valid image dimensions to use for decodes
DEF_TEST(Codec_Dimensions, r) {
    // JPG
    test_dimensions(r, "CMYK.jpg");
    test_dimensions(r, "color_wheel.jpg");
    test_dimensions(r, "grayscale.jpg");
    test_dimensions(r, "mandrill_512_q075.jpg");
    test_dimensions(r, "randPixels.jpg");

    // Decoding small images with very large scaling factors is a potential
    // source of bugs and crashes.  We disable these tests in Gold because
    // tiny images are not very useful to look at.
    // Here we make sure that we do not crash or access illegal memory when
    // performing scaled decodes on small images.
    test_dimensions(r, "1x1.png");
    test_dimensions(r, "2x2.png");
    test_dimensions(r, "3x3.png");
    test_dimensions(r, "3x1.png");
    test_dimensions(r, "1x1.png");
    test_dimensions(r, "16x1.png");
    test_dimensions(r, "1x16.png");
    test_dimensions(r, "mandrill_16.png");

}

static void test_invalid(skiatest::Reporter* r, const char path[]) {
    SkAutoTDelete<SkStream> stream(resource(path));
    if (!stream) {
        SkDebugf("Missing resource '%s'\n", path);
        return;
    }
    SkAutoTDelete<SkCodec> codec(SkCodec::NewFromStream(stream.detach()));
    REPORTER_ASSERT(r, nullptr == codec);
}

DEF_TEST(Codec_Empty, r) {
    // Test images that should not be able to create a codec
    test_invalid(r, "empty_images/zero-dims.gif");
    test_invalid(r, "empty_images/zero-embedded.ico");
    test_invalid(r, "empty_images/zero-width.bmp");
    test_invalid(r, "empty_images/zero-height.bmp");
    test_invalid(r, "empty_images/zero-width.jpg");
    test_invalid(r, "empty_images/zero-height.jpg");
    test_invalid(r, "empty_images/zero-width.png");
    test_invalid(r, "empty_images/zero-height.png");
    test_invalid(r, "empty_images/zero-width.wbmp");
    test_invalid(r, "empty_images/zero-height.wbmp");
    // This image is an ico with an embedded mask-bmp.  This is illegal.
    test_invalid(r, "invalid_images/mask-bmp-ico.ico");
}

static void test_invalid_parameters(skiatest::Reporter* r, const char path[]) {
    SkAutoTDelete<SkStream> stream(resource(path));
    if (!stream) {
        SkDebugf("Missing resource '%s'\n", path);
        return;
    }
    SkAutoTDelete<SkCodec> decoder(SkCodec::NewFromStream(stream.detach()));
    
    // This should return kSuccess because kIndex8 is supported.
    SkPMColor colorStorage[256];
    int colorCount;
    SkCodec::Result result = decoder->startScanlineDecode(
        decoder->getInfo().makeColorType(kIndex_8_SkColorType), nullptr, colorStorage, &colorCount);
    REPORTER_ASSERT(r, SkCodec::kSuccess == result);
    // The rest of the test is uninteresting if kIndex8 is not supported
    if (SkCodec::kSuccess != result) {
        return;
    }

    // This should return kInvalidParameters because, in kIndex_8 mode, we must pass in a valid
    // colorPtr and a valid colorCountPtr.
    result = decoder->startScanlineDecode(
        decoder->getInfo().makeColorType(kIndex_8_SkColorType), nullptr, nullptr, nullptr);
    REPORTER_ASSERT(r, SkCodec::kInvalidParameters == result);
    result = decoder->startScanlineDecode(
        decoder->getInfo().makeColorType(kIndex_8_SkColorType));
    REPORTER_ASSERT(r, SkCodec::kInvalidParameters == result);
}

DEF_TEST(Codec_Params, r) {
    test_invalid_parameters(r, "index8.png");
    test_invalid_parameters(r, "mandrill.wbmp");
}
