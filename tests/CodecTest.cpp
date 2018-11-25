/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "FakeStreams.h"
#include "Resources.h"
#include "SkAndroidCodec.h"
#include "SkAutoMalloc.h"
#include "SkBitmap.h"
#include "SkCodec.h"
#include "SkCodecImageGenerator.h"
#include "SkColorSpace_XYZ.h"
#include "SkColorSpacePriv.h"
#include "SkData.h"
#include "SkFrontBufferedStream.h"
#include "SkImageEncoder.h"
#include "SkImageEncoderPriv.h"
#include "SkMakeUnique.h"
#include "SkMD5.h"
#include "SkOSPath.h"
#include "SkJpegEncoder.h"
#include "SkPngChunkReader.h"
#include "SkPngEncoder.h"
#include "SkRandom.h"
#include "SkStream.h"
#include "SkStreamPriv.h"
#include "SkWebpEncoder.h"
#include "Test.h"

#include "png.h"

#include "sk_tool_utils.h"

#if PNG_LIBPNG_VER_MAJOR == 1 && PNG_LIBPNG_VER_MINOR < 5
    // FIXME (scroggo): Google3 needs to be updated to use a newer version of libpng. In
    // the meantime, we had to break some pieces of SkPngCodec in order to support Google3.
    // The parts that are broken are likely not used by Google3.
    #define SK_PNG_DISABLE_TESTS
#endif

static void md5(const SkBitmap& bm, SkMD5::Digest* digest) {
    SkASSERT(bm.getPixels());
    SkMD5 md5;
    size_t rowLen = bm.info().bytesPerPixel() * bm.width();
    for (int y = 0; y < bm.height(); ++y) {
        md5.write(bm.getAddr(0, y), rowLen);
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
template<typename Codec>
static void test_info(skiatest::Reporter* r, Codec* codec, const SkImageInfo& info,
                      SkCodec::Result expectedResult, const SkMD5::Digest* goodDigest) {
    SkBitmap bm;
    bm.allocPixels(info);

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

static void test_incremental_decode(skiatest::Reporter* r, SkCodec* codec, const SkImageInfo& info,
        const SkMD5::Digest& goodDigest) {
    SkBitmap bm;
    bm.allocPixels(info);

    REPORTER_ASSERT(r, SkCodec::kSuccess == codec->startIncrementalDecode(info, bm.getPixels(),
                                                                          bm.rowBytes()));

    REPORTER_ASSERT(r, SkCodec::kSuccess == codec->incrementalDecode());

    compare_to_good_digest(r, goodDigest, bm);
}

// Test in stripes, similar to DM's kStripe_Mode
static void test_in_stripes(skiatest::Reporter* r, SkCodec* codec, const SkImageInfo& info,
                            const SkMD5::Digest& goodDigest) {
    SkBitmap bm;
    bm.allocPixels(info);
    bm.eraseColor(SK_ColorYELLOW);

    const int height = info.height();
    // Note that if numStripes does not evenly divide height there will be an extra
    // stripe.
    const int numStripes = 4;

    if (numStripes > height) {
        // Image is too small.
        return;
    }

    const int stripeHeight = height / numStripes;

    // Iterate through the image twice. Once to decode odd stripes, and once for even.
    for (int oddEven = 1; oddEven >= 0; oddEven--) {
        for (int y = oddEven * stripeHeight; y < height; y += 2 * stripeHeight) {
            SkIRect subset = SkIRect::MakeLTRB(0, y, info.width(),
                                               SkTMin(y + stripeHeight, height));
            SkCodec::Options options;
            options.fSubset = &subset;
            if (SkCodec::kSuccess != codec->startIncrementalDecode(info, bm.getAddr(0, y),
                        bm.rowBytes(), &options)) {
                ERRORF(r, "failed to start incremental decode!\ttop: %i\tbottom%i\n",
                       subset.top(), subset.bottom());
                return;
            }
            if (SkCodec::kSuccess != codec->incrementalDecode()) {
                ERRORF(r, "failed incremental decode starting from line %i\n", y);
                return;
            }
        }
    }

    compare_to_good_digest(r, goodDigest, bm);
}

template<typename Codec>
static void test_codec(skiatest::Reporter* r, Codec* codec, SkBitmap& bm, const SkImageInfo& info,
        const SkISize& size, SkCodec::Result expectedResult, SkMD5::Digest* digest,
        const SkMD5::Digest* goodDigest) {

    REPORTER_ASSERT(r, info.dimensions() == size);
    bm.allocPixels(info);

    SkCodec::Result result = codec->getPixels(info, bm.getPixels(), bm.rowBytes());
    REPORTER_ASSERT(r, result == expectedResult);

    md5(bm, digest);
    if (goodDigest) {
        REPORTER_ASSERT(r, *digest == *goodDigest);
    }

    {
        // Test decoding to 565
        SkImageInfo info565 = info.makeColorType(kRGB_565_SkColorType);
        if (info.alphaType() == kOpaque_SkAlphaType) {
            // Decoding to 565 should succeed.
            SkBitmap bm565;
            bm565.allocPixels(info565);

            // This will allow comparison even if the image is incomplete.
            bm565.eraseColor(SK_ColorBLACK);

            REPORTER_ASSERT(r, expectedResult == codec->getPixels(info565,
                    bm565.getPixels(), bm565.rowBytes()));

            SkMD5::Digest digest565;
            md5(bm565, &digest565);

            // A dumb client's request for non-opaque should also succeed.
            for (auto alpha : { kPremul_SkAlphaType, kUnpremul_SkAlphaType }) {
                info565 = info565.makeAlphaType(alpha);
                test_info(r, codec, info565, expectedResult, &digest565);
            }
        } else {
            test_info(r, codec, info565, SkCodec::kInvalidConversion, nullptr);
        }
    }

    if (codec->getInfo().colorType() == kGray_8_SkColorType) {
        SkImageInfo grayInfo = codec->getInfo();
        SkBitmap grayBm;
        grayBm.allocPixels(grayInfo);

        grayBm.eraseColor(SK_ColorBLACK);

        REPORTER_ASSERT(r, expectedResult == codec->getPixels(grayInfo,
                grayBm.getPixels(), grayBm.rowBytes()));

        SkMD5::Digest grayDigest;
        md5(grayBm, &grayDigest);

        for (auto alpha : { kPremul_SkAlphaType, kUnpremul_SkAlphaType }) {
            grayInfo = grayInfo.makeAlphaType(alpha);
            test_info(r, codec, grayInfo, expectedResult, &grayDigest);
        }
    }

    // Verify that re-decoding gives the same result.  It is interesting to check this after
    // a decode to 565, since choosing to decode to 565 may result in some of the decode
    // options being modified.  These options should return to their defaults on another
    // decode to kN32, so the new digest should match the old digest.
    test_info(r, codec, info, expectedResult, digest);

    {
        // Check alpha type conversions
        if (info.alphaType() == kOpaque_SkAlphaType) {
            test_info(r, codec, info.makeAlphaType(kUnpremul_SkAlphaType),
                      expectedResult, digest);
            test_info(r, codec, info.makeAlphaType(kPremul_SkAlphaType),
                      expectedResult, digest);
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
            test_info(r, codec, info.makeAlphaType(otherAt), expectedResult, nullptr);
        }
    }
}

static bool supports_partial_scanlines(const char path[]) {
    static const char* const exts[] = {
        "jpg", "jpeg", "png", "webp"
        "JPG", "JPEG", "PNG", "WEBP"
    };

    for (uint32_t i = 0; i < SK_ARRAY_COUNT(exts); i++) {
        if (SkStrEndsWith(path, exts[i])) {
            return true;
        }
    }
    return false;
}

// FIXME: Break up this giant function
static void check(skiatest::Reporter* r,
                  const char path[],
                  SkISize size,
                  bool supportsScanlineDecoding,
                  bool supportsSubsetDecoding,
                  bool supportsIncomplete,
                  bool supportsNewScanlineDecoding = false) {

    std::unique_ptr<SkStream> stream(GetResourceAsStream(path));
    if (!stream) {
        return;
    }

    std::unique_ptr<SkCodec> codec(nullptr);
    bool isIncomplete = supportsIncomplete;
    if (isIncomplete) {
        size_t size = stream->getLength();
        codec = SkCodec::MakeFromData(SkData::MakeFromStream(stream.get(), 2 * size / 3));
    } else {
        codec = SkCodec::MakeFromStream(std::move(stream));
    }
    if (!codec) {
        ERRORF(r, "Unable to decode '%s'", path);
        return;
    }

    // Test full image decodes with SkCodec
    SkMD5::Digest codecDigest;
    const SkImageInfo info = codec->getInfo().makeColorType(kN32_SkColorType);
    SkBitmap bm;
    SkCodec::Result expectedResult = isIncomplete ? SkCodec::kIncompleteInput : SkCodec::kSuccess;
    test_codec(r, codec.get(), bm, info, size, expectedResult, &codecDigest, nullptr);

    // Scanline decoding follows.

    if (supportsNewScanlineDecoding && !isIncomplete) {
        test_incremental_decode(r, codec.get(), info, codecDigest);
        // This is only supported by codecs that use incremental decoding to
        // support subset decodes - png and jpeg (once SkJpegCodec is
        // converted).
        if (SkStrEndsWith(path, "png") || SkStrEndsWith(path, "PNG")) {
            test_in_stripes(r, codec.get(), info, codecDigest);
        }
    }

    // Need to call startScanlineDecode() first.
    REPORTER_ASSERT(r, codec->getScanlines(bm.getAddr(0, 0), 1, 0) == 0);
    REPORTER_ASSERT(r, !codec->skipScanlines(1));
    const SkCodec::Result startResult = codec->startScanlineDecode(info);
    if (supportsScanlineDecoding) {
        bm.eraseColor(SK_ColorYELLOW);

        REPORTER_ASSERT(r, startResult == SkCodec::kSuccess);

        for (int y = 0; y < info.height(); y++) {
            const int lines = codec->getScanlines(bm.getAddr(0, y), 1, 0);
            if (!isIncomplete) {
                REPORTER_ASSERT(r, 1 == lines);
            }
        }
        // verify that scanline decoding gives the same result.
        if (SkCodec::kTopDown_SkScanlineOrder == codec->getScanlineOrder()) {
            compare_to_good_digest(r, codecDigest, bm);
        }

        // Cannot continue to decode scanlines beyond the end
        REPORTER_ASSERT(r, codec->getScanlines(bm.getAddr(0, 0), 1, 0)
                == 0);

        // Interrupting a scanline decode with a full decode starts from
        // scratch
        REPORTER_ASSERT(r, codec->startScanlineDecode(info) == SkCodec::kSuccess);
        const int lines = codec->getScanlines(bm.getAddr(0, 0), 1, 0);
        if (!isIncomplete) {
            REPORTER_ASSERT(r, lines == 1);
        }
        REPORTER_ASSERT(r, codec->getPixels(bm.info(), bm.getPixels(), bm.rowBytes())
                == expectedResult);
        REPORTER_ASSERT(r, codec->getScanlines(bm.getAddr(0, 0), 1, 0)
                == 0);
        REPORTER_ASSERT(r, codec->skipScanlines(1)
                == 0);

        // Test partial scanline decodes
        if (supports_partial_scanlines(path) && info.width() >= 3) {
            SkCodec::Options options;
            int width = info.width();
            int height = info.height();
            SkIRect subset = SkIRect::MakeXYWH(2 * (width / 3), 0, width / 3, height);
            options.fSubset = &subset;

            const auto partialStartResult = codec->startScanlineDecode(info, &options);
            REPORTER_ASSERT(r, partialStartResult == SkCodec::kSuccess);

            for (int y = 0; y < height; y++) {
                const int lines = codec->getScanlines(bm.getAddr(0, y), 1, 0);
                if (!isIncomplete) {
                    REPORTER_ASSERT(r, 1 == lines);
                }
            }
        }
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
        const auto result = codec->getPixels(bm.info(), bm.getPixels(), bm.rowBytes(), &opts);

        if (supportsSubsetDecoding) {
            if (expectedResult == SkCodec::kSuccess) {
                REPORTER_ASSERT(r, result == expectedResult);
            }
            // Webp is the only codec that supports subsets, and it will have modified the subset
            // to have even left/top.
            REPORTER_ASSERT(r, SkIsAlign2(subset.fLeft) && SkIsAlign2(subset.fTop));
        } else {
            // No subsets will work.
            REPORTER_ASSERT(r, result == SkCodec::kUnimplemented);
        }
    }

    // SkAndroidCodec tests
    if (supportsScanlineDecoding || supportsSubsetDecoding || supportsNewScanlineDecoding) {

        std::unique_ptr<SkStream> stream(GetResourceAsStream(path));
        if (!stream) {
            return;
        }

        auto androidCodec = SkAndroidCodec::MakeFromCodec(std::move(codec));
        if (!androidCodec) {
            ERRORF(r, "Unable to decode '%s'", path);
            return;
        }

        SkBitmap bm;
        SkMD5::Digest androidCodecDigest;
        test_codec(r, androidCodec.get(), bm, info, size, expectedResult, &androidCodecDigest,
                   &codecDigest);
    }

    if (!isIncomplete) {
        // Test SkCodecImageGenerator
        std::unique_ptr<SkStream> stream(GetResourceAsStream(path));
        sk_sp<SkData> fullData(SkData::MakeFromStream(stream.get(), stream->getLength()));
        std::unique_ptr<SkImageGenerator> gen(
                SkCodecImageGenerator::MakeFromEncodedCodec(fullData));
        SkBitmap bm;
        bm.allocPixels(info);
        REPORTER_ASSERT(r, gen->getPixels(info, bm.getPixels(), bm.rowBytes()));
        compare_to_good_digest(r, codecDigest, bm);

#ifndef SK_PNG_DISABLE_TESTS
        // Test using SkFrontBufferedStream, as Android does
        auto bufferedStream = SkFrontBufferedStream::Make(
                      SkMemoryStream::Make(std::move(fullData)), SkCodec::MinBufferedBytesNeeded());
        REPORTER_ASSERT(r, bufferedStream);
        codec = SkCodec::MakeFromStream(std::move(bufferedStream));
        REPORTER_ASSERT(r, codec);
        if (codec) {
            test_info(r, codec.get(), info, SkCodec::kSuccess, &codecDigest);
        }
#endif
    }

    // If we've just tested incomplete decodes, let's run the same test again on full decodes.
    if (isIncomplete) {
        check(r, path, size, supportsScanlineDecoding, supportsSubsetDecoding, false,
              supportsNewScanlineDecoding);
    }
}

DEF_TEST(Codec_wbmp, r) {
    check(r, "images/mandrill.wbmp", SkISize::Make(512, 512), true, false, true);
}

DEF_TEST(Codec_webp, r) {
    check(r, "images/baby_tux.webp", SkISize::Make(386, 395), false, true, true);
    check(r, "images/color_wheel.webp", SkISize::Make(128, 128), false, true, true);
    check(r, "images/yellow_rose.webp", SkISize::Make(400, 301), false, true, true);
}

DEF_TEST(Codec_bmp, r) {
    check(r, "images/randPixels.bmp", SkISize::Make(8, 8), true, false, true);
    check(r, "images/rle.bmp", SkISize::Make(320, 240), true, false, true);
}

DEF_TEST(Codec_ico, r) {
    // FIXME: We are not ready to test incomplete ICOs
    // These two tests examine interestingly different behavior:
    // Decodes an embedded BMP image
    check(r, "images/color_wheel.ico", SkISize::Make(128, 128), true, false, false);
    // Decodes an embedded PNG image
    check(r, "images/google_chrome.ico", SkISize::Make(256, 256), false, false, false, true);
}

DEF_TEST(Codec_gif, r) {
    check(r, "images/box.gif", SkISize::Make(200, 55), false, false, true, true);
    check(r, "images/color_wheel.gif", SkISize::Make(128, 128), false, false, true, true);
    // randPixels.gif is too small to test incomplete
    check(r, "images/randPixels.gif", SkISize::Make(8, 8), false, false, false, true);
}

DEF_TEST(Codec_jpg, r) {
    check(r, "images/CMYK.jpg", SkISize::Make(642, 516), true, false, true);
    check(r, "images/color_wheel.jpg", SkISize::Make(128, 128), true, false, true);
    // grayscale.jpg is too small to test incomplete
    check(r, "images/grayscale.jpg", SkISize::Make(128, 128), true, false, false);
    check(r, "images/mandrill_512_q075.jpg", SkISize::Make(512, 512), true, false, true);
    // randPixels.jpg is too small to test incomplete
    check(r, "images/randPixels.jpg", SkISize::Make(8, 8), true, false, false);
}

DEF_TEST(Codec_png, r) {
    check(r, "images/arrow.png", SkISize::Make(187, 312), false, false, true, true);
    check(r, "images/baby_tux.png", SkISize::Make(240, 246), false, false, true, true);
    check(r, "images/color_wheel.png", SkISize::Make(128, 128), false, false, true, true);
    // half-transparent-white-pixel.png is too small to test incomplete
    check(r, "images/half-transparent-white-pixel.png", SkISize::Make(1, 1), false, false, false, true);
    check(r, "images/mandrill_128.png", SkISize::Make(128, 128), false, false, true, true);
    check(r, "images/mandrill_16.png", SkISize::Make(16, 16), false, false, true, true);
    check(r, "images/mandrill_256.png", SkISize::Make(256, 256), false, false, true, true);
    check(r, "images/mandrill_32.png", SkISize::Make(32, 32), false, false, true, true);
    check(r, "images/mandrill_512.png", SkISize::Make(512, 512), false, false, true, true);
    check(r, "images/mandrill_64.png", SkISize::Make(64, 64), false, false, true, true);
    check(r, "images/plane.png", SkISize::Make(250, 126), false, false, true, true);
    check(r, "images/plane_interlaced.png", SkISize::Make(250, 126), false, false, true, true);
    check(r, "images/randPixels.png", SkISize::Make(8, 8), false, false, true, true);
    check(r, "images/yellow_rose.png", SkISize::Make(400, 301), false, false, true, true);
}

// Disable RAW tests for Win32.
#if defined(SK_CODEC_DECODES_RAW) && (!defined(_WIN32))
DEF_TEST(Codec_raw, r) {
    check(r, "images/sample_1mp.dng", SkISize::Make(600, 338), false, false, false);
    check(r, "images/sample_1mp_rotated.dng", SkISize::Make(600, 338), false, false, false);
    check(r, "images/dng_with_preview.dng", SkISize::Make(600, 338), true, false, false);
}
#endif

static void test_invalid_stream(skiatest::Reporter* r, const void* stream, size_t len) {
    // Neither of these calls should return a codec. Bots should catch us if we leaked anything.
    REPORTER_ASSERT(r, !SkCodec::MakeFromStream(
                                        skstd::make_unique<SkMemoryStream>(stream, len, false)));
    REPORTER_ASSERT(r, !SkAndroidCodec::MakeFromStream(
                                        skstd::make_unique<SkMemoryStream>(stream, len, false)));
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

DEF_TEST(Codec_null, r) {
    // Attempting to create an SkCodec or an SkAndroidCodec with null should not
    // crash.
    REPORTER_ASSERT(r, !SkCodec::MakeFromStream(nullptr));
    REPORTER_ASSERT(r, !SkAndroidCodec::MakeFromStream(nullptr));
}

static void test_dimensions(skiatest::Reporter* r, const char path[]) {
    // Create the codec from the resource file
    std::unique_ptr<SkStream> stream(GetResourceAsStream(path));
    if (!stream) {
        return;
    }
    std::unique_ptr<SkAndroidCodec> codec(SkAndroidCodec::MakeFromStream(std::move(stream)));
    if (!codec) {
        ERRORF(r, "Unable to create codec '%s'", path);
        return;
    }

    // Check that the decode is successful for a variety of scales
    for (int sampleSize = 1; sampleSize < 32; sampleSize++) {
        // Scale the output dimensions
        SkISize scaledDims = codec->getSampledDimensions(sampleSize);
        SkImageInfo scaledInfo = codec->getInfo()
                .makeWH(scaledDims.width(), scaledDims.height())
                .makeColorType(kN32_SkColorType);

        // Set up for the decode
        size_t rowBytes = scaledDims.width() * sizeof(SkPMColor);
        size_t totalBytes = scaledInfo.computeByteSize(rowBytes);
        SkAutoTMalloc<SkPMColor> pixels(totalBytes);

        SkAndroidCodec::AndroidOptions options;
        options.fSampleSize = sampleSize;
        SkCodec::Result result =
                codec->getAndroidPixels(scaledInfo, pixels.get(), rowBytes, &options);
        REPORTER_ASSERT(r, SkCodec::kSuccess == result);
    }
}

// Ensure that onGetScaledDimensions returns valid image dimensions to use for decodes
DEF_TEST(Codec_Dimensions, r) {
    // JPG
    test_dimensions(r, "images/CMYK.jpg");
    test_dimensions(r, "images/color_wheel.jpg");
    test_dimensions(r, "images/grayscale.jpg");
    test_dimensions(r, "images/mandrill_512_q075.jpg");
    test_dimensions(r, "images/randPixels.jpg");

    // Decoding small images with very large scaling factors is a potential
    // source of bugs and crashes.  We disable these tests in Gold because
    // tiny images are not very useful to look at.
    // Here we make sure that we do not crash or access illegal memory when
    // performing scaled decodes on small images.
    test_dimensions(r, "images/1x1.png");
    test_dimensions(r, "images/2x2.png");
    test_dimensions(r, "images/3x3.png");
    test_dimensions(r, "images/3x1.png");
    test_dimensions(r, "images/1x1.png");
    test_dimensions(r, "images/16x1.png");
    test_dimensions(r, "images/1x16.png");
    test_dimensions(r, "images/mandrill_16.png");

    // RAW
// Disable RAW tests for Win32.
#if defined(SK_CODEC_DECODES_RAW) && (!defined(_WIN32))
    test_dimensions(r, "images/sample_1mp.dng");
    test_dimensions(r, "images/sample_1mp_rotated.dng");
    test_dimensions(r, "images/dng_with_preview.dng");
#endif
}

static void test_invalid(skiatest::Reporter* r, const char path[]) {
    std::unique_ptr<SkStream> stream(GetResourceAsStream(path));
    if (!stream) {
        return;
    }
    REPORTER_ASSERT(r, !SkCodec::MakeFromStream(std::move(stream)));
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
    // It is illegal for a webp frame to not be fully contained by the canvas.
    test_invalid(r, "invalid_images/invalid-offset.webp");
#if defined(SK_CODEC_DECODES_RAW) && (!defined(_WIN32))
    test_invalid(r, "empty_images/zero_height.tiff");
#endif
    test_invalid(r, "invalid_images/b37623797.ico");
}

#ifdef PNG_READ_UNKNOWN_CHUNKS_SUPPORTED

#ifndef SK_PNG_DISABLE_TESTS   // reading chunks does not work properly with older versions.
                               // It does not appear that anyone in Google3 is reading chunks.

static void codex_test_write_fn(png_structp png_ptr, png_bytep data, png_size_t len) {
    SkWStream* sk_stream = (SkWStream*)png_get_io_ptr(png_ptr);
    if (!sk_stream->write(data, len)) {
        png_error(png_ptr, "sk_write_fn Error!");
    }
}

DEF_TEST(Codec_pngChunkReader, r) {
    // Create a dummy bitmap. Use unpremul RGBA for libpng.
    SkBitmap bm;
    const int w = 1;
    const int h = 1;
    const SkImageInfo bmInfo = SkImageInfo::Make(w, h, kRGBA_8888_SkColorType,
                                                 kUnpremul_SkAlphaType);
    bm.setInfo(bmInfo);
    bm.allocPixels();
    bm.eraseColor(SK_ColorBLUE);
    SkMD5::Digest goodDigest;
    md5(bm, &goodDigest);

    // Write to a png file.
    png_structp png = png_create_write_struct(PNG_LIBPNG_VER_STRING, nullptr, nullptr, nullptr);
    REPORTER_ASSERT(r, png);
    if (!png) {
        return;
    }

    png_infop info = png_create_info_struct(png);
    REPORTER_ASSERT(r, info);
    if (!info) {
        png_destroy_write_struct(&png, nullptr);
        return;
    }

    if (setjmp(png_jmpbuf(png))) {
        ERRORF(r, "failed writing png");
        png_destroy_write_struct(&png, &info);
        return;
    }

    SkDynamicMemoryWStream wStream;
    png_set_write_fn(png, (void*) (&wStream), codex_test_write_fn, nullptr);

    png_set_IHDR(png, info, (png_uint_32)w, (png_uint_32)h, 8,
                 PNG_COLOR_TYPE_RGB_ALPHA, PNG_INTERLACE_NONE,
                 PNG_COMPRESSION_TYPE_DEFAULT, PNG_FILTER_TYPE_DEFAULT);

    // Create some chunks that match the Android framework's use.
    static png_unknown_chunk gUnknowns[] = {
        { "npOl", (png_byte*)"outline", sizeof("outline"), PNG_HAVE_IHDR },
        { "npLb", (png_byte*)"layoutBounds", sizeof("layoutBounds"), PNG_HAVE_IHDR },
        { "npTc", (png_byte*)"ninePatchData", sizeof("ninePatchData"), PNG_HAVE_IHDR },
    };

    png_set_keep_unknown_chunks(png, PNG_HANDLE_CHUNK_ALWAYS, (png_byte*)"npOl\0npLb\0npTc\0", 3);
    png_set_unknown_chunks(png, info, gUnknowns, SK_ARRAY_COUNT(gUnknowns));
#if PNG_LIBPNG_VER < 10600
    /* Deal with unknown chunk location bug in 1.5.x and earlier */
    png_set_unknown_chunk_location(png, info, 0, PNG_HAVE_IHDR);
    png_set_unknown_chunk_location(png, info, 1, PNG_HAVE_IHDR);
#endif

    png_write_info(png, info);

    for (int j = 0; j < h; j++) {
        png_bytep row = (png_bytep)(bm.getAddr(0, j));
        png_write_rows(png, &row, 1);
    }
    png_write_end(png, info);
    png_destroy_write_struct(&png, &info);

    class ChunkReader : public SkPngChunkReader {
    public:
        ChunkReader(skiatest::Reporter* r)
            : fReporter(r)
        {
            this->reset();
        }

        bool readChunk(const char tag[], const void* data, size_t length) override {
            for (size_t i = 0; i < SK_ARRAY_COUNT(gUnknowns); ++i) {
                if (!strcmp(tag, (const char*) gUnknowns[i].name)) {
                    // Tag matches. This should have been the first time we see it.
                    REPORTER_ASSERT(fReporter, !fSeen[i]);
                    fSeen[i] = true;

                    // Data and length should match
                    REPORTER_ASSERT(fReporter, length == gUnknowns[i].size);
                    REPORTER_ASSERT(fReporter, !strcmp((const char*) data,
                                                       (const char*) gUnknowns[i].data));
                    return true;
                }
            }
            ERRORF(fReporter, "Saw an unexpected unknown chunk.");
            return true;
        }

        bool allHaveBeenSeen() {
            bool ret = true;
            for (auto seen : fSeen) {
                ret &= seen;
            }
            return ret;
        }

        void reset() {
            sk_bzero(fSeen, sizeof(fSeen));
        }

    private:
        skiatest::Reporter* fReporter;  // Unowned
        bool fSeen[3];
    };

    ChunkReader chunkReader(r);

    // Now read the file with SkCodec.
    std::unique_ptr<SkCodec> codec(SkCodec::MakeFromData(wStream.detachAsData(), &chunkReader));
    REPORTER_ASSERT(r, codec);
    if (!codec) {
        return;
    }

    // Now compare to the original.
    SkBitmap decodedBm;
    decodedBm.setInfo(codec->getInfo());
    decodedBm.allocPixels();
    SkCodec::Result result = codec->getPixels(codec->getInfo(), decodedBm.getPixels(),
                                              decodedBm.rowBytes());
    REPORTER_ASSERT(r, SkCodec::kSuccess == result);

    if (decodedBm.colorType() != bm.colorType()) {
        SkBitmap tmp;
        bool success = sk_tool_utils::copy_to(&tmp, bm.colorType(), decodedBm);
        REPORTER_ASSERT(r, success);
        if (!success) {
            return;
        }

        tmp.swap(decodedBm);
    }

    compare_to_good_digest(r, goodDigest, decodedBm);
    REPORTER_ASSERT(r, chunkReader.allHaveBeenSeen());

    // Decoding again will read the chunks again.
    chunkReader.reset();
    REPORTER_ASSERT(r, !chunkReader.allHaveBeenSeen());
    result = codec->getPixels(codec->getInfo(), decodedBm.getPixels(), decodedBm.rowBytes());
    REPORTER_ASSERT(r, SkCodec::kSuccess == result);
    REPORTER_ASSERT(r, chunkReader.allHaveBeenSeen());
}
#endif // SK_PNG_DISABLE_TESTS
#endif // PNG_READ_UNKNOWN_CHUNKS_SUPPORTED

// Stream that can only peek up to a limit
class LimitedPeekingMemStream : public SkStream {
public:
    LimitedPeekingMemStream(sk_sp<SkData> data, size_t limit)
        : fStream(std::move(data))
        , fLimit(limit) {}

    size_t peek(void* buf, size_t bytes) const override {
        return fStream.peek(buf, SkTMin(bytes, fLimit));
    }
    size_t read(void* buf, size_t bytes) override {
        return fStream.read(buf, bytes);
    }
    bool rewind() override {
        return fStream.rewind();
    }
    bool isAtEnd() const override {
        return fStream.isAtEnd();
    }
private:
    SkMemoryStream fStream;
    const size_t   fLimit;
};

// Disable RAW tests for Win32.
#if defined(SK_CODEC_DECODES_RAW) && (!defined(_WIN32))
// Test that the RawCodec works also for not asset stream. This will test the code path using
// SkRawBufferedStream instead of SkRawAssetStream.
DEF_TEST(Codec_raw_notseekable, r) {
    constexpr char path[] = "images/dng_with_preview.dng";
    sk_sp<SkData> data(GetResourceAsData(path));
    if (!data) {
        SkDebugf("Missing resource '%s'\n", path);
        return;
    }

    std::unique_ptr<SkCodec> codec(SkCodec::MakeFromStream(
                                           skstd::make_unique<NotAssetMemStream>(std::move(data))));
    REPORTER_ASSERT(r, codec);

    test_info(r, codec.get(), codec->getInfo(), SkCodec::kSuccess, nullptr);
}
#endif

// Test that even if webp_parse_header fails to peek enough, it will fall back to read()
// + rewind() and succeed.
DEF_TEST(Codec_webp_peek, r) {
    constexpr char path[] = "images/baby_tux.webp";
    auto data = GetResourceAsData(path);
    if (!data) {
        SkDebugf("Missing resource '%s'\n", path);
        return;
    }

    // The limit is less than webp needs to peek or read.
    std::unique_ptr<SkCodec> codec(SkCodec::MakeFromStream(
                                           skstd::make_unique<LimitedPeekingMemStream>(data, 25)));
    REPORTER_ASSERT(r, codec);

    test_info(r, codec.get(), codec->getInfo(), SkCodec::kSuccess, nullptr);

    // Similarly, a stream which does not peek should still succeed.
    codec = SkCodec::MakeFromStream(skstd::make_unique<LimitedPeekingMemStream>(data, 0));
    REPORTER_ASSERT(r, codec);

    test_info(r, codec.get(), codec->getInfo(), SkCodec::kSuccess, nullptr);
}

// SkCodec's wbmp decoder was initially unnecessarily restrictive.
// It required the second byte to be zero. The wbmp specification allows
// a couple of bits to be 1 (so long as they do not overlap with 0x9F).
// Test that SkCodec now supports an image with these bits set.
DEF_TEST(Codec_wbmp_restrictive, r) {
    const char* path = "images/mandrill.wbmp";
    std::unique_ptr<SkStream> stream(GetResourceAsStream(path));
    if (!stream) {
        return;
    }

    // Modify the stream to contain a second byte with some bits set.
    auto data = SkCopyStreamToData(stream.get());
    uint8_t* writeableData = static_cast<uint8_t*>(data->writable_data());
    writeableData[1] = static_cast<uint8_t>(~0x9F);

    // SkCodec should support this.
    std::unique_ptr<SkCodec> codec(SkCodec::MakeFromData(data));
    REPORTER_ASSERT(r, codec);
    if (!codec) {
        return;
    }
    test_info(r, codec.get(), codec->getInfo(), SkCodec::kSuccess, nullptr);
}

// wbmp images have a header that can be arbitrarily large, depending on the
// size of the image. We cap the size at 65535, meaning we only need to look at
// 8 bytes to determine whether we can read the image. This is important
// because SkCodec only passes a limited number of bytes to SkWbmpCodec to
// determine whether the image is a wbmp.
DEF_TEST(Codec_wbmp_max_size, r) {
    const unsigned char maxSizeWbmp[] = { 0x00, 0x00,           // Header
                                          0x83, 0xFF, 0x7F,     // W: 65535
                                          0x83, 0xFF, 0x7F };   // H: 65535
    std::unique_ptr<SkStream> stream(new SkMemoryStream(maxSizeWbmp, sizeof(maxSizeWbmp), false));
    std::unique_ptr<SkCodec> codec(SkCodec::MakeFromStream(std::move(stream)));

    REPORTER_ASSERT(r, codec);
    if (!codec) return;

    REPORTER_ASSERT(r, codec->getInfo().width() == 65535);
    REPORTER_ASSERT(r, codec->getInfo().height() == 65535);

    // Now test an image which is too big. Any image with a larger header (i.e.
    // has bigger width/height) is also too big.
    const unsigned char tooBigWbmp[] = { 0x00, 0x00,           // Header
                                         0x84, 0x80, 0x00,     // W: 65536
                                         0x84, 0x80, 0x00 };   // H: 65536
    stream.reset(new SkMemoryStream(tooBigWbmp, sizeof(tooBigWbmp), false));
    codec = SkCodec::MakeFromStream(std::move(stream));

    REPORTER_ASSERT(r, !codec);
}

DEF_TEST(Codec_jpeg_rewind, r) {
    const char* path = "images/mandrill_512_q075.jpg";
    sk_sp<SkData> data(GetResourceAsData(path));
    if (!data) {
        return;
    }

    data = SkData::MakeSubset(data.get(), 0, data->size() / 2);
    std::unique_ptr<SkAndroidCodec> codec(SkAndroidCodec::MakeFromData(data));
    if (!codec) {
        ERRORF(r, "Unable to create codec '%s'.", path);
        return;
    }

    const int width = codec->getInfo().width();
    const int height = codec->getInfo().height();
    size_t rowBytes = sizeof(SkPMColor) * width;
    SkAutoMalloc pixelStorage(height * rowBytes);

    // Perform a sampled decode.
    SkAndroidCodec::AndroidOptions opts;
    opts.fSampleSize = 12;
    auto sampledInfo = codec->getInfo().makeWH(width / 12, height / 12);
    auto result = codec->getAndroidPixels(sampledInfo, pixelStorage.get(), rowBytes, &opts);
    REPORTER_ASSERT(r, SkCodec::kIncompleteInput == result);

    // Rewind the codec and perform a full image decode.
    result = codec->getPixels(codec->getInfo(), pixelStorage.get(), rowBytes);
    REPORTER_ASSERT(r, SkCodec::kIncompleteInput == result);

    // Now perform a subset decode.
    {
        opts.fSampleSize = 1;
        SkIRect subset = SkIRect::MakeWH(100, 100);
        opts.fSubset = &subset;
        result = codec->getAndroidPixels(codec->getInfo().makeWH(100, 100), pixelStorage.get(),
                                         rowBytes, &opts);
        // Though we only have half the data, it is enough to decode this subset.
        REPORTER_ASSERT(r, SkCodec::kSuccess == result);
    }

    // Perform another full image decode.  ASAN will detect if we look at the subset when it is
    // out of scope.  This would happen if we depend on the old state in the codec.
    // This tests two layers of bugs: both SkJpegCodec::readRows and SkCodec::fillIncompleteImage
    // used to look at the old subset.
    opts.fSubset = nullptr;
    result = codec->getAndroidPixels(codec->getInfo(), pixelStorage.get(), rowBytes, &opts);
    REPORTER_ASSERT(r, SkCodec::kIncompleteInput == result);
}

static void check_color_xform(skiatest::Reporter* r, const char* path) {
    std::unique_ptr<SkAndroidCodec> codec(SkAndroidCodec::MakeFromStream(GetResourceAsStream(path)));

    SkAndroidCodec::AndroidOptions opts;
    opts.fSampleSize = 3;
    const int subsetWidth = codec->getInfo().width() / 2;
    const int subsetHeight = codec->getInfo().height() / 2;
    SkIRect subset = SkIRect::MakeWH(subsetWidth, subsetHeight);
    opts.fSubset = &subset;

    const int dstWidth = subsetWidth / opts.fSampleSize;
    const int dstHeight = subsetHeight / opts.fSampleSize;
    sk_sp<SkData> data = GetResourceAsData("icc_profiles/HP_ZR30w.icc");
    sk_sp<SkColorSpace> colorSpace = SkColorSpace::MakeICC(data->data(), data->size());
    SkImageInfo dstInfo = codec->getInfo().makeWH(dstWidth, dstHeight)
                                          .makeColorType(kN32_SkColorType)
                                          .makeColorSpace(colorSpace);

    size_t rowBytes = dstInfo.minRowBytes();
    SkAutoMalloc pixelStorage(dstInfo.computeByteSize(rowBytes));
    SkCodec::Result result = codec->getAndroidPixels(dstInfo, pixelStorage.get(), rowBytes, &opts);
    REPORTER_ASSERT(r, SkCodec::kSuccess == result);
}

DEF_TEST(Codec_ColorXform, r) {
    check_color_xform(r, "images/mandrill_512_q075.jpg");
    check_color_xform(r, "images/mandrill_512.png");
}

static bool color_type_match(SkColorType origColorType, SkColorType codecColorType) {
    switch (origColorType) {
        case kRGBA_8888_SkColorType:
        case kBGRA_8888_SkColorType:
            return kRGBA_8888_SkColorType == codecColorType ||
                   kBGRA_8888_SkColorType == codecColorType;
        default:
            return origColorType == codecColorType;
    }
}

static bool alpha_type_match(SkAlphaType origAlphaType, SkAlphaType codecAlphaType) {
    switch (origAlphaType) {
        case kUnpremul_SkAlphaType:
        case kPremul_SkAlphaType:
            return kUnpremul_SkAlphaType == codecAlphaType ||
                    kPremul_SkAlphaType == codecAlphaType;
        default:
            return origAlphaType == codecAlphaType;
    }
}

static void check_round_trip(skiatest::Reporter* r, SkCodec* origCodec, const SkImageInfo& info) {
    SkBitmap bm1;
    bm1.allocPixels(info);
    SkCodec::Result result = origCodec->getPixels(info, bm1.getPixels(), bm1.rowBytes());
    REPORTER_ASSERT(r, SkCodec::kSuccess == result);

    // Encode the image to png.
    sk_sp<SkData> data =
            sk_sp<SkData>(sk_tool_utils::EncodeImageToData(bm1, SkEncodedImageFormat::kPNG, 100));

    std::unique_ptr<SkCodec> codec(SkCodec::MakeFromData(data));
    REPORTER_ASSERT(r, color_type_match(info.colorType(), codec->getInfo().colorType()));
    REPORTER_ASSERT(r, alpha_type_match(info.alphaType(), codec->getInfo().alphaType()));

    SkBitmap bm2;
    bm2.allocPixels(info);
    result = codec->getPixels(info, bm2.getPixels(), bm2.rowBytes());
    REPORTER_ASSERT(r, SkCodec::kSuccess == result);

    SkMD5::Digest d1, d2;
    md5(bm1, &d1);
    md5(bm2, &d2);
    REPORTER_ASSERT(r, d1 == d2);
}

DEF_TEST(Codec_PngRoundTrip, r) {
    auto codec = SkCodec::MakeFromStream(GetResourceAsStream("images/mandrill_512_q075.jpg"));

    SkColorType colorTypesOpaque[] = {
            kRGB_565_SkColorType, kRGBA_8888_SkColorType, kBGRA_8888_SkColorType
    };
    for (SkColorType colorType : colorTypesOpaque) {
        SkImageInfo newInfo = codec->getInfo().makeColorType(colorType);
        check_round_trip(r, codec.get(), newInfo);
    }

    codec = SkCodec::MakeFromStream(GetResourceAsStream("images/grayscale.jpg"));
    check_round_trip(r, codec.get(), codec->getInfo());

    codec = SkCodec::MakeFromStream(GetResourceAsStream("images/yellow_rose.png"));

    SkColorType colorTypesWithAlpha[] = {
            kRGBA_8888_SkColorType, kBGRA_8888_SkColorType
    };
    SkAlphaType alphaTypes[] = {
            kUnpremul_SkAlphaType, kPremul_SkAlphaType
    };
    for (SkColorType colorType : colorTypesWithAlpha) {
        for (SkAlphaType alphaType : alphaTypes) {
            // Set color space to nullptr because color correct premultiplies do not round trip.
            SkImageInfo newInfo = codec->getInfo().makeColorType(colorType)
                                                  .makeAlphaType(alphaType)
                                                  .makeColorSpace(nullptr);
            check_round_trip(r, codec.get(), newInfo);
        }
    }

    codec = SkCodec::MakeFromStream(GetResourceAsStream("images/index8.png"));

    for (SkAlphaType alphaType : alphaTypes) {
        SkImageInfo newInfo = codec->getInfo().makeAlphaType(alphaType)
                                              .makeColorSpace(nullptr);
        check_round_trip(r, codec.get(), newInfo);
    }
}

static void test_conversion_possible(skiatest::Reporter* r, const char* path,
                                     bool supportsScanlineDecoder,
                                     bool supportsIncrementalDecoder) {
    std::unique_ptr<SkStream> stream(GetResourceAsStream(path));
    if (!stream) {
        return;
    }

    std::unique_ptr<SkCodec> codec(SkCodec::MakeFromStream(std::move(stream)));
    if (!codec) {
        ERRORF(r, "failed to create a codec for %s", path);
        return;
    }

    SkImageInfo infoF16 = codec->getInfo().makeColorType(kRGBA_F16_SkColorType);

    SkBitmap bm;
    bm.allocPixels(infoF16);
    SkCodec::Result result = codec->getPixels(infoF16, bm.getPixels(), bm.rowBytes());
    REPORTER_ASSERT(r, SkCodec::kInvalidConversion == result);

    result = codec->startScanlineDecode(infoF16);
    if (supportsScanlineDecoder) {
        REPORTER_ASSERT(r, SkCodec::kInvalidConversion == result);
    } else {
        REPORTER_ASSERT(r, SkCodec::kUnimplemented == result
                        || SkCodec::kInvalidConversion == result);
    }

    result = codec->startIncrementalDecode(infoF16, bm.getPixels(), bm.rowBytes());
    if (supportsIncrementalDecoder) {
        REPORTER_ASSERT(r, SkCodec::kInvalidConversion == result);
    } else {
        REPORTER_ASSERT(r, SkCodec::kUnimplemented == result
                        || SkCodec::kInvalidConversion == result);
    }

    infoF16 = infoF16.makeColorSpace(infoF16.colorSpace()->makeLinearGamma());
    result = codec->getPixels(infoF16, bm.getPixels(), bm.rowBytes());
    REPORTER_ASSERT(r, SkCodec::kSuccess == result);
    result = codec->startScanlineDecode(infoF16);
    if (supportsScanlineDecoder) {
        REPORTER_ASSERT(r, SkCodec::kSuccess == result);
    } else {
        REPORTER_ASSERT(r, SkCodec::kUnimplemented == result);
    }

    result = codec->startIncrementalDecode(infoF16, bm.getPixels(), bm.rowBytes());
    if (supportsIncrementalDecoder) {
        REPORTER_ASSERT(r, SkCodec::kSuccess == result);
    } else {
        REPORTER_ASSERT(r, SkCodec::kUnimplemented == result);
    }
}

DEF_TEST(Codec_F16ConversionPossible, r) {
    test_conversion_possible(r, "images/color_wheel.webp", false, false);
    test_conversion_possible(r, "images/mandrill_512_q075.jpg", true, false);
    test_conversion_possible(r, "images/yellow_rose.png", false, true);
}

static void decode_frame(skiatest::Reporter* r, SkCodec* codec, size_t frame) {
    SkBitmap bm;
    auto info = codec->getInfo().makeColorType(kN32_SkColorType);
    bm.allocPixels(info);

    SkCodec::Options opts;
    opts.fFrameIndex = frame;
    REPORTER_ASSERT(r, SkCodec::kSuccess == codec->getPixels(info,
            bm.getPixels(), bm.rowBytes(), &opts));
}

// For an animated GIF, we should only read enough to decode frame 0 if the
// client never calls getFrameInfo and only decodes frame 0.
DEF_TEST(Codec_skipFullParse, r) {
    auto path = "images/test640x479.gif";
    auto streamObj = GetResourceAsStream(path);
    if (!streamObj) {
        return;
    }
    SkStream* stream = streamObj.get();

    // Note that we cheat and hold on to the stream pointer, but SkCodec will
    // take ownership. We will not refer to the stream after the SkCodec
    // deletes it.
    std::unique_ptr<SkCodec> codec(SkCodec::MakeFromStream(std::move(streamObj)));
    if (!codec) {
        ERRORF(r, "Failed to create codec for %s", path);
        return;
    }

    REPORTER_ASSERT(r, stream->hasPosition());
    const size_t sizePosition = stream->getPosition();
    REPORTER_ASSERT(r, stream->hasLength() && sizePosition < stream->getLength());

    // This should read more of the stream, but not the whole stream.
    decode_frame(r, codec.get(), 0);
    const size_t positionAfterFirstFrame = stream->getPosition();
    REPORTER_ASSERT(r, positionAfterFirstFrame > sizePosition
                       && positionAfterFirstFrame < stream->getLength());

    // There is more data in the stream.
    auto frameInfo = codec->getFrameInfo();
    REPORTER_ASSERT(r, frameInfo.size() == 4);
    REPORTER_ASSERT(r, stream->getPosition() > positionAfterFirstFrame);
}

// Only rewinds up to a limit.
class LimitedRewindingStream : public SkStream {
public:
    static std::unique_ptr<SkStream> Make(const char path[], size_t limit) {
        auto stream = GetResourceAsStream(path);
        if (!stream) {
            return nullptr;
        }
        return std::unique_ptr<SkStream>(new LimitedRewindingStream(std::move(stream), limit));
    }

    size_t read(void* buffer, size_t size) override {
        const size_t bytes = fStream->read(buffer, size);
        fPosition += bytes;
        return bytes;
    }

    bool isAtEnd() const override {
        return fStream->isAtEnd();
    }

    bool rewind() override {
        if (fPosition <= fLimit && fStream->rewind()) {
            fPosition = 0;
            return true;
        }

        return false;
    }

private:
    std::unique_ptr<SkStream> fStream;
    const size_t              fLimit;
    size_t                    fPosition;

    LimitedRewindingStream(std::unique_ptr<SkStream> stream, size_t limit)
        : fStream(std::move(stream))
        , fLimit(limit)
        , fPosition(0)
    {
        SkASSERT(fStream);
    }
};

DEF_TEST(Codec_fallBack, r) {
    // SkAndroidCodec needs to be able to fall back to scanline decoding
    // if incremental decoding does not work. Make sure this does not
    // require a rewind.

    // Formats that currently do not support incremental decoding
    auto files = {
            "images/CMYK.jpg",
            "images/color_wheel.ico",
            "images/mandrill.wbmp",
            "images/randPixels.bmp",
            };
    for (auto file : files) {
        auto stream = LimitedRewindingStream::Make(file, SkCodec::MinBufferedBytesNeeded());
        if (!stream) {
            SkDebugf("Missing resources (%s). Set --resourcePath.\n", file);
            return;
        }

        std::unique_ptr<SkCodec> codec(SkCodec::MakeFromStream(std::move(stream)));
        if (!codec) {
            ERRORF(r, "Failed to create codec for %s,", file);
            continue;
        }

        SkImageInfo info = codec->getInfo().makeColorType(kN32_SkColorType);
        SkBitmap bm;
        bm.allocPixels(info);

        if (SkCodec::kUnimplemented != codec->startIncrementalDecode(info, bm.getPixels(),
                bm.rowBytes())) {
            ERRORF(r, "Is scanline decoding now implemented for %s?", file);
            continue;
        }

        // Scanline decoding should not require a rewind.
        SkCodec::Result result = codec->startScanlineDecode(info);
        if (SkCodec::kSuccess != result) {
            ERRORF(r, "Scanline decoding failed for %s with %i", file, result);
        }
    }
}

// This test verifies that we fixed an assert statement that fired when reusing a png codec
// after scaling.
DEF_TEST(Codec_reusePng, r) {
    std::unique_ptr<SkStream> stream(GetResourceAsStream("images/plane.png"));
    if (!stream) {
        return;
    }

    std::unique_ptr<SkAndroidCodec> codec(SkAndroidCodec::MakeFromStream(std::move(stream)));
    if (!codec) {
        ERRORF(r, "Failed to create codec\n");
        return;
    }

    SkAndroidCodec::AndroidOptions opts;
    opts.fSampleSize = 5;
    auto size = codec->getSampledDimensions(opts.fSampleSize);
    auto info = codec->getInfo().makeWH(size.fWidth, size.fHeight).makeColorType(kN32_SkColorType);
    SkBitmap bm;
    bm.allocPixels(info);
    auto result = codec->getAndroidPixels(info, bm.getPixels(), bm.rowBytes(), &opts);
    REPORTER_ASSERT(r, result == SkCodec::kSuccess);

    info = codec->getInfo().makeColorType(kN32_SkColorType);
    bm.allocPixels(info);
    opts.fSampleSize = 1;
    result = codec->getAndroidPixels(info, bm.getPixels(), bm.rowBytes(), &opts);
    REPORTER_ASSERT(r, result == SkCodec::kSuccess);
}

DEF_TEST(Codec_rowsDecoded, r) {
    auto file = "images/plane_interlaced.png";
    std::unique_ptr<SkStream> stream(GetResourceAsStream(file));
    if (!stream) {
        return;
    }

    // This is enough to read the header etc, but no rows.
    std::unique_ptr<SkCodec> codec(SkCodec::MakeFromData(SkData::MakeFromStream(stream.get(), 99)));
    if (!codec) {
        ERRORF(r, "Failed to create codec\n");
        return;
    }

    auto info = codec->getInfo().makeColorType(kN32_SkColorType);
    SkBitmap bm;
    bm.allocPixels(info);
    auto result = codec->startIncrementalDecode(info, bm.getPixels(), bm.rowBytes());
    REPORTER_ASSERT(r, result == SkCodec::kSuccess);

    // This is an arbitrary value. The important fact is that it is not zero, and rowsDecoded
    // should get set to zero by incrementalDecode.
    int rowsDecoded = 77;
    result = codec->incrementalDecode(&rowsDecoded);
    REPORTER_ASSERT(r, result == SkCodec::kIncompleteInput);
    REPORTER_ASSERT(r, rowsDecoded == 0);
}

static void test_invalid_images(skiatest::Reporter* r, const char* path,
                                SkCodec::Result expectedResult) {
    auto stream = GetResourceAsStream(path);
    if (!stream) {
        return;
    }

    std::unique_ptr<SkCodec> codec(SkCodec::MakeFromStream(std::move(stream)));
    REPORTER_ASSERT(r, codec);

    test_info(r, codec.get(), codec->getInfo().makeColorType(kN32_SkColorType), expectedResult,
              nullptr);
}

DEF_TEST(Codec_InvalidImages, r) {
    // ASAN will complain if there is an issue.
    test_invalid_images(r, "invalid_images/skbug5887.gif", SkCodec::kErrorInInput);
    test_invalid_images(r, "invalid_images/many-progressive-scans.jpg", SkCodec::kInvalidInput);
    test_invalid_images(r, "invalid_images/b33251605.bmp", SkCodec::kIncompleteInput);
    test_invalid_images(r, "invalid_images/bad_palette.png", SkCodec::kInvalidInput);
}

static void test_invalid_header(skiatest::Reporter* r, const char* path) {
    auto data = GetResourceAsData(path);
    if (!data) {
        return;
    }
    std::unique_ptr<SkStreamAsset> stream(new SkMemoryStream(std::move(data)));
    if (!stream) {
        return;
    }
    std::unique_ptr<SkCodec> codec(SkCodec::MakeFromStream(std::move(stream)));
    REPORTER_ASSERT(r, !codec);
}

DEF_TEST(Codec_InvalidHeader, r) {
    test_invalid_header(r, "invalid_images/int_overflow.ico");

    // These files report values that have caused problems with SkFILEStreams.
    // They are invalid, and should not create SkCodecs.
    test_invalid_header(r, "invalid_images/b33651913.bmp");
    test_invalid_header(r, "invalid_images/b34778578.bmp");
}

DEF_TEST(Codec_InvalidAnimated, r) {
    // ASAN will complain if there is an issue.
    auto path = "invalid_images/skbug6046.gif";
    auto stream = GetResourceAsStream(path);
    if (!stream) {
        return;
    }

    std::unique_ptr<SkCodec> codec(SkCodec::MakeFromStream(std::move(stream)));
    REPORTER_ASSERT(r, codec);
    if (!codec) {
        return;
    }

    const auto info = codec->getInfo().makeColorType(kN32_SkColorType);
    SkBitmap bm;
    bm.allocPixels(info);

    auto frameInfos = codec->getFrameInfo();
    SkCodec::Options opts;
    for (int i = 0; static_cast<size_t>(i) < frameInfos.size(); i++) {
        opts.fFrameIndex = i;
        const auto reqFrame = frameInfos[i].fRequiredFrame;
        opts.fPriorFrame = reqFrame == i - 1 ? reqFrame : SkCodec::kNone;
        auto result = codec->startIncrementalDecode(info, bm.getPixels(), bm.rowBytes(), &opts);
        if (result != SkCodec::kSuccess) {
            ERRORF(r, "Failed to start decoding frame %i (out of %i) with error %i\n", i,
                   frameInfos.size(), result);
            continue;
        }

        codec->incrementalDecode();
    }
}

static void encode_format(SkDynamicMemoryWStream* stream, const SkPixmap& pixmap,
                          SkTransferFunctionBehavior unpremulBehavior,
                          SkEncodedImageFormat format) {
    SkPngEncoder::Options pngOptions;
    SkWebpEncoder::Options webpOptions;
    pngOptions.fUnpremulBehavior = unpremulBehavior;
    webpOptions.fUnpremulBehavior = unpremulBehavior;
    switch (format) {
        case SkEncodedImageFormat::kPNG:
            SkPngEncoder::Encode(stream, pixmap, pngOptions);
            break;
        case SkEncodedImageFormat::kJPEG:
            SkJpegEncoder::Encode(stream, pixmap, SkJpegEncoder::Options());
            break;
        case SkEncodedImageFormat::kWEBP:
            SkWebpEncoder::Encode(stream, pixmap, webpOptions);
            break;
        default:
            SkASSERT(false);
            break;
    }
}

static void test_encode_icc(skiatest::Reporter* r, SkEncodedImageFormat format,
                            SkTransferFunctionBehavior unpremulBehavior) {
    // Test with sRGB color space.
    SkBitmap srgbBitmap;
    SkImageInfo srgbInfo = SkImageInfo::MakeS32(1, 1, kOpaque_SkAlphaType);
    srgbBitmap.allocPixels(srgbInfo);
    *srgbBitmap.getAddr32(0, 0) = 0;
    SkPixmap pixmap;
    srgbBitmap.peekPixels(&pixmap);
    SkDynamicMemoryWStream srgbBuf;
    encode_format(&srgbBuf, pixmap, unpremulBehavior, format);
    sk_sp<SkData> srgbData = srgbBuf.detachAsData();
    std::unique_ptr<SkCodec> srgbCodec(SkCodec::MakeFromData(srgbData));
    REPORTER_ASSERT(r, srgbCodec->getInfo().colorSpace() == SkColorSpace::MakeSRGB().get());

    // Test with P3 color space.
    SkDynamicMemoryWStream p3Buf;
    sk_sp<SkColorSpace> p3 = SkColorSpace::MakeRGB(SkColorSpace::kSRGB_RenderTargetGamma,
                                                   SkColorSpace::kDCIP3_D65_Gamut);
    pixmap.setColorSpace(p3);
    encode_format(&p3Buf, pixmap, unpremulBehavior, format);
    sk_sp<SkData> p3Data = p3Buf.detachAsData();
    std::unique_ptr<SkCodec> p3Codec(SkCodec::MakeFromData(p3Data));
    REPORTER_ASSERT(r, p3Codec->getInfo().colorSpace()->gammaCloseToSRGB());
    SkMatrix44 mat0(SkMatrix44::kUninitialized_Constructor);
    SkMatrix44 mat1(SkMatrix44::kUninitialized_Constructor);
    bool success = p3->toXYZD50(&mat0);
    REPORTER_ASSERT(r, success);
    success = p3Codec->getInfo().colorSpace()->toXYZD50(&mat1);
    REPORTER_ASSERT(r, success);

    for (int i = 0; i < 4; i++) {
        for (int j = 0; j < 4; j++) {
            REPORTER_ASSERT(r, color_space_almost_equal(mat0.get(i, j), mat1.get(i, j)));
        }
    }
}

DEF_TEST(Codec_EncodeICC, r) {
    test_encode_icc(r, SkEncodedImageFormat::kPNG, SkTransferFunctionBehavior::kRespect);
    test_encode_icc(r, SkEncodedImageFormat::kJPEG, SkTransferFunctionBehavior::kRespect);
    test_encode_icc(r, SkEncodedImageFormat::kWEBP, SkTransferFunctionBehavior::kRespect);
    test_encode_icc(r, SkEncodedImageFormat::kPNG, SkTransferFunctionBehavior::kIgnore);
    test_encode_icc(r, SkEncodedImageFormat::kJPEG, SkTransferFunctionBehavior::kIgnore);
    test_encode_icc(r, SkEncodedImageFormat::kWEBP, SkTransferFunctionBehavior::kIgnore);
}

DEF_TEST(Codec_webp_rowsDecoded, r) {
    const char* path = "images/baby_tux.webp";
    sk_sp<SkData> data(GetResourceAsData(path));
    if (!data) {
        return;
    }

    // Truncate this file so that the header is available but no rows can be
    // decoded. This should create a codec but fail to decode.
    size_t truncatedSize = 5000;
    sk_sp<SkData> subset = SkData::MakeSubset(data.get(), 0, truncatedSize);
    std::unique_ptr<SkCodec> codec = SkCodec::MakeFromData(std::move(subset));
    if (!codec) {
        ERRORF(r, "Failed to create a codec for %s truncated to only %lu bytes",
               path, truncatedSize);
        return;
    }

    test_info(r, codec.get(), codec->getInfo(), SkCodec::kInvalidInput, nullptr);
}
